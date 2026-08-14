# Ancient Empires Foundation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add the first playable Ancient Empires vertical slice to NBCraft: persistent settlements, three ancient factions, strategic resource simulation, territory ownership, a Roman player settlement, AI rival settlements, and a reusable humanoid civilization NPC base.

**Architecture:** `Level` owns a `CivilizationManager`, but civilization rules live under `source/world/civilization` and remain independent of rendering/UI. Strategic simulation runs at settlement level on a throttled tick; nearby individuals remain ordinary Minecraft entities. Civilization state is saved separately as `civilizations.dat` through the existing NBT/storage stack so the original `level.dat` format stays untouched.

**Tech Stack:** C++98-compatible NBCraft core, CMake/CTest, existing `Level`/`PathfinderMob`/NBT systems, existing humanoid renderer.

## Global Constraints

- PC is the initial target; Xbox deployment is out of scope.
- Keep core civilization logic C++98-compatible because non-Windows NBCraft builds default to C++98.
- Do not change the original `level.dat` schema for the first milestone.
- Distant civilizations are settlement-stat simulations; do not keep hundreds of distant NPC entities active.
- Initial factions are Roman, Egyptian, and Persian.
- The first player-founded settlement is Roman; faction-selection UI is a later milestone.
- Use custom entity IDs at 1000+; `AddMobPacket` and entity NBT already use 32-bit IDs.
- Use tile ID 202 for the first custom Founding Standard (`200` and `201` are already used by NBCraft custom blocks).

---

### Task 1: Add a lightweight civilization test target

**Files:**
- Modify: `CMakeLists.txt`
- Create: `tests/CMakeLists.txt`
- Create: `tests/civilization/CivilizationModelTests.cpp`

**Interfaces:**
- Produces: CTest target `nbcraft-civilization-tests`.
- The target initially compiles only civilization-domain files so tests do not require a graphics/platform executable.

- [ ] **Step 1: Add the failing test source**

```cpp
#include <assert.h>
#include "world/civilization/Settlement.hpp"

int main()
{
    Settlement settlement(1, FACTION_ROMAN, "Roma", TilePos(0, 64, 0));
    assert(settlement.getDiplomaticStatus(2) == DIPLOMACY_NEUTRAL);
    settlement.adjustRelation(2, -70);
    assert(settlement.getDiplomaticStatus(2) == DIPLOMACY_HOSTILE);
    return 0;
}
```

- [ ] **Step 2: Wire CTest**

Add near the top-level project declaration:

```cmake
include(CTest)
```

Add after `add_subdirectory(source)`:

```cmake
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
```

Create `tests/CMakeLists.txt`:

```cmake
add_executable(nbcraft-civilization-tests
    civilization/CivilizationModelTests.cpp
    ../source/world/civilization/CivilizationTypes.cpp
    ../source/world/civilization/Settlement.cpp
)

target_include_directories(nbcraft-civilization-tests PRIVATE ../source)
add_test(NAME nbcraft-civilization-tests COMMAND nbcraft-civilization-tests)
```

- [ ] **Step 3: Run the test and verify RED**

Run:

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --target nbcraft-civilization-tests
```

Expected: compilation fails because `Settlement.hpp` and civilization types do not exist.

- [ ] **Step 4: Commit the failing test**

```sh
git add CMakeLists.txt tests
 git commit -m "test: add civilization model test target"
```

---

### Task 2: Implement civilization domain types and settlement diplomacy

**Files:**
- Create: `source/world/civilization/CivilizationTypes.hpp`
- Create: `source/world/civilization/CivilizationTypes.cpp`
- Create: `source/world/civilization/Settlement.hpp`
- Create: `source/world/civilization/Settlement.cpp`
- Modify: `source/CMakeLists.txt`

**Interfaces:**
- Produces:
  - `enum FactionId { FACTION_NONE, FACTION_ROMAN, FACTION_EGYPTIAN, FACTION_PERSIAN }`
  - `enum DiplomaticStatus { DIPLOMACY_HOSTILE, DIPLOMACY_UNFRIENDLY, DIPLOMACY_NEUTRAL, DIPLOMACY_FRIENDLY, DIPLOMACY_ALLIED }`
  - `struct ResourceStockpile { int food, wood, stone, wealth; }`
  - `class Settlement`

`Settlement` public API:

```cpp
Settlement(int32_t id, FactionId faction, const std::string& name, const TilePos& center);
int32_t getId() const;
FactionId getFaction() const;
const std::string& getName() const;
const TilePos& getCenter() const;
int getPopulation() const;
void setPopulation(int population);
ResourceStockpile& resources();
const ResourceStockpile& resources() const;
void setWorkers(int farmers, int lumberWorkers, int quarryWorkers);
void strategicTick();
int getRelation(int32_t otherSettlementId) const;
void setRelation(int32_t otherSettlementId, int score);
void adjustRelation(int32_t otherSettlementId, int delta);
DiplomaticStatus getDiplomaticStatus(int32_t otherSettlementId) const;
```

Diplomacy thresholds:

```text
<= -60 hostile
-59..-20 unfriendly
-19..19 neutral
20..59 friendly
>= 60 allied
```

Strategic resource rule for milestone 1:

```text
food  += farmers * 2 - ((population + 9) / 10)
wood  += lumberWorkers
stone += quarryWorkers
all resources clamp at 0
```

- [ ] **Step 1: Implement the minimum types/API above.**
- [ ] **Step 2: Add source files to `source/CMakeLists.txt`.**
- [ ] **Step 3: Build and run `ctest --test-dir build -R nbcraft-civilization-tests --output-on-failure`; verify GREEN.**
- [ ] **Step 4: Extend the test with resource production:**

```cpp
settlement.setPopulation(20);
settlement.setWorkers(5, 2, 1);
settlement.strategicTick();
assert(settlement.resources().food == 8);
assert(settlement.resources().wood == 2);
assert(settlement.resources().stone == 1);
```

- [ ] **Step 5: Re-run tests and commit.**

```sh
git add source/world/civilization source/CMakeLists.txt tests/civilization/CivilizationModelTests.cpp
 git commit -m "feat: add settlement domain model"
```

---

### Task 3: Add CivilizationManager and chunk-based territory

**Files:**
- Create: `source/world/civilization/CivilizationManager.hpp`
- Create: `source/world/civilization/CivilizationManager.cpp`
- Create: `tests/civilization/CivilizationManagerTests.cpp`
- Modify: `tests/CMakeLists.txt`
- Modify: `source/CMakeLists.txt`

**Interfaces:**

```cpp
class CivilizationManager {
public:
    CivilizationManager();
    int32_t foundSettlement(FactionId faction, const std::string& name, const TilePos& center);
    Settlement* getSettlement(int32_t id);
    const Settlement* getSettlement(int32_t id) const;
    Settlement* getSettlementAt(const ChunkPos& chunk);
    const Settlement* getSettlementAt(const ChunkPos& chunk) const;
    bool claimChunk(int32_t settlementId, const ChunkPos& chunk);
    bool isChunkClaimed(const ChunkPos& chunk) const;
    void strategicTick();
    const std::map<int32_t, Settlement>& getSettlements() const;
};
```

Rules:
- New settlements claim their center chunk immediately.
- A chunk can have only one owner.
- Settlement IDs start at 1 and increase monotonically.
- `strategicTick()` calls every settlement's `strategicTick()` exactly once.

- [ ] **Step 1: Write failing tests for ID allocation, center-chunk claim, conflicting claims, and one strategic tick.**
- [ ] **Step 2: Run and verify RED.**
- [ ] **Step 3: Implement the minimal manager.**
- [ ] **Step 4: Run and verify GREEN.**
- [ ] **Step 5: Commit.**

```sh
git commit -am "feat: add civilization manager and territory claims"
```

---

### Task 4: Serialize civilization state with existing NBT

**Files:**
- Create: `source/world/civilization/CivilizationPersistence.hpp`
- Create: `source/world/civilization/CivilizationPersistence.cpp`
- Create: `tests/civilization/CivilizationPersistenceTests.cpp`
- Modify: `tests/CMakeLists.txt`
- Modify: `source/CMakeLists.txt`

**Interfaces:**

```cpp
class CivilizationPersistence {
public:
    static CompoundTag* save(const CivilizationManager& manager);
    static bool load(const CompoundTag& root, CivilizationManager& manager);
};
```

NBT schema:

```text
Version: int32 = 1
NextSettlementId: int32
Settlements: list<compound>
  Id: int32
  Faction: int32
  Name: string
  X/Y/Z: int32
  Population: int32
  Farmers/LumberWorkers/QuarryWorkers: int32
  Food/Wood/Stone/Wealth: int32
  Relations: list<compound> { OtherId: int32, Score: int32 }
Claims: list<compound> { X: int32, Z: int32, SettlementId: int32 }
```

- [ ] **Step 1: Write a failing round-trip test creating two settlements, hostile diplomacy, resources, and two claims.**
- [ ] **Step 2: Run and verify RED.**
- [ ] **Step 3: Implement NBT serialization with `CompoundTag` and `ListTag`.**
- [ ] **Step 4: Run and verify GREEN, including equality of IDs, centers, resources, relations, and claims after round trip.**
- [ ] **Step 5: Commit.**

```sh
git commit -am "feat: serialize civilization state with NBT"
```

---

### Task 5: Integrate civilization state into Level and world saves

**Files:**
- Modify: `source/world/level/Level.hpp`
- Modify: `source/world/level/Level.cpp`
- Modify: `source/world/level/storage/LevelStorage.hpp`
- Modify: `source/world/level/storage/LevelStorage.cpp`
- Modify: `source/world/level/storage/ExternalFileLevelStorage.hpp`
- Modify: `source/world/level/storage/ExternalFileLevelStorage.cpp`

**Interfaces:**

Add to `LevelStorage` with default no-op implementations:

```cpp
virtual bool loadCivilizations(CivilizationManager& manager);
virtual bool saveCivilizations(const CivilizationManager& manager);
```

Add to `Level`:

```cpp
CivilizationManager* getCivilizationManager() const;
```

And members:

```cpp
CivilizationManager* m_pCivilizationManager;
int m_civilizationTickCounter;
```

Lifecycle:
- Construct manager after `LevelData` is prepared.
- Call `m_pLevelStorage->loadCivilizations(*m_pCivilizationManager)` during Level construction.
- Delete manager in `Level::~Level()`.
- Every 100 `Level::tick()` calls, run one `strategicTick()`.
- Call `saveCivilizations()` from `Level::saveGame()`.

External storage format:
- File: `<world>/civilizations.dat`
- Use `RakNet::BitStream`, `RakDataInput`/`RakDataOutput`, and `NbtIo` exactly like existing entity/player NBT storage.
- Write to `civilizations.dat.tmp`, close successfully, then replace `civilizations.dat` to avoid partially written saves.
- Missing file means an empty manager and is not an error.

- [ ] **Step 1: Add a storage-level round-trip test using a temporary directory if the current test platform supports filesystem tests; otherwise keep persistence tests at the NBT layer and verify through a debug build.**
- [ ] **Step 2: Implement LevelStorage extension and ExternalFileLevelStorage file I/O.**
- [ ] **Step 3: Integrate manager ownership/ticking/saving into `Level`.**
- [ ] **Step 4: Build full NBCraft and run all tests.**
- [ ] **Step 5: Launch a world, save, quit, reopen, and verify a manually injected settlement survives.**
- [ ] **Step 6: Commit.**

```sh
git commit -am "feat: persist civilization state with worlds"
```

---

### Task 6: Add the Founding Standard block

**Files:**
- Modify: `source/common/Utils.hpp`
- Create: `source/world/tile/FoundingStandardTile.hpp`
- Create: `source/world/tile/FoundingStandardTile.cpp`
- Modify: `source/world/tile/Tile.hpp`
- Modify: `source/world/tile/Tile.cpp`
- Modify: `source/CMakeLists.txt`

**Interfaces:**

Add:

```cpp
TILE_FOUNDING_STANDARD = 202
```

Add static tile pointer:

```cpp
static Tile* foundingStandard;
```

`FoundingStandardTile::use` behavior for milestone 1:
- If the chunk is already claimed, do nothing and return `true`.
- Otherwise found `FACTION_ROMAN` at the standard position with name `"Roma"`.
- Initial state: population `10`, farmers `4`, lumber workers `2`, quarry workers `2`, food `50`, wood `25`, stone `25`, wealth `10`.
- Claim the center chunk.

Use an existing banner/cloth texture for the prototype; custom art comes after mechanics are stable.

- [ ] **Step 1: Add a failing manager-level test containing the exact initial settlement values expected from a founding helper.**
- [ ] **Step 2: Implement tile ID/registration and tile class.**
- [ ] **Step 3: Add it to the creative inventory using the existing tile-item registration path.**
- [ ] **Step 4: Build and launch NBCraft; place and activate the standard; verify one Roman settlement is created and duplicate activation does not duplicate it.**
- [ ] **Step 5: Save/reload and verify it persists.**
- [ ] **Step 6: Commit.**

```sh
git commit -am "feat: add Roman founding standard"
```

---

### Task 7: Add the reusable EmpireHuman NPC base

**Files:**
- Modify: `source/world/entity/EntityType.hpp`
- Modify: `source/world/entity/EntityTypeDescriptor.hpp`
- Modify: `source/world/entity/EntityTypeDescriptor.cpp`
- Modify: `source/world/entity/MobFactory.cpp`
- Create: `source/world/entity/EmpireHuman.hpp`
- Create: `source/world/entity/EmpireHuman.cpp`
- Modify: `source/CMakeLists.txt`

**Interfaces:**

Add custom ID:

```cpp
EMPIRE_HUMAN = 1000
```

Add roles:

```cpp
enum EmpireRole {
    EMPIRE_ROLE_WORKER = 0,
    EMPIRE_ROLE_SOLDIER = 1,
    EMPIRE_ROLE_MERCHANT = 2
};
```

`EmpireHuman` derives from `PathfinderMob` and stores:

```cpp
int32_t m_settlementId;
FactionId m_faction;
EmpireRole m_role;
```

Constructor defaults:

```text
renderer: RENDER_HUMANOID
texture: mob/char.png for prototype
run speed: 0.5
```

Persistence keys written through `addAdditionalSaveData` / `readAdditionalSaveData`:

```text
SettlementId: int32
Faction: int32
EmpireRole: int32
```

AI for milestone 1:
- Worker: random stroll biased toward the settlement center; never seeks attack targets.
- Soldier: seeks hostile `EmpireHuman` entities inside 16 blocks; does not attack same-settlement or allied entities.
- Merchant: random stroll only.

- [ ] **Step 1: Write serialization and faction-hostility tests for the non-rendering behavior.**
- [ ] **Step 2: Verify RED.**
- [ ] **Step 3: Register descriptor/factory and implement minimal entity behavior.**
- [ ] **Step 4: Reuse the existing `RENDER_HUMANOID` renderer; no new renderer is required for this milestone.**
- [ ] **Step 5: Build, spawn one worker and one soldier, save/reload, and verify role/faction/settlement survive.**
- [ ] **Step 6: Commit.**

```sh
git commit -am "feat: add persistent empire human entity"
```

---

### Task 8: Seed two autonomous rival settlements for the vertical slice

**Files:**
- Modify: `source/world/civilization/CivilizationManager.hpp`
- Modify: `source/world/civilization/CivilizationManager.cpp`
- Modify: `source/world/level/Level.cpp`
- Extend: `tests/civilization/CivilizationManagerTests.cpp`

**Interfaces:**

```cpp
void CivilizationManager::seedPrototypeRivals(const TilePos& playerSpawn);
```

Rules:
- Only runs when there are zero settlements.
- Egyptian settlement starts roughly 24 chunks east of spawn.
- Persian settlement starts roughly 24 chunks west of spawn.
- Their exact centers are snapped to the top solid block at the selected X/Z by the Level integration layer.
- Each begins with population 20, food 100, wood 50, stone 50, wealth 25.
- Egypt and Persia begin neutral to each other and unfriendly (`-30`) toward the Roman settlement once Roma is founded.
- Do not instantiate distant citizens yet; these rivals exist first as strategic settlements.

- [ ] **Step 1: Write tests proving seeding is idempotent and creates exactly two factions.**
- [ ] **Step 2: Verify RED, implement, verify GREEN.**
- [ ] **Step 3: Integrate terrain-height resolution in `Level` without forcing distant entity activity.**
- [ ] **Step 4: Full build/test and gameplay smoke test.**
- [ ] **Step 5: Commit.**

```sh
git commit -am "feat: seed Egyptian and Persian rival settlements"
```

---

## Milestone acceptance criteria

The foundation milestone is complete when:

1. NBCraft still builds and launches on PC.
2. Activating the Founding Standard creates exactly one Roman settlement.
3. Egypt and Persia exist as autonomous strategic settlements.
4. Settlements own chunks and conflicting claims are rejected.
5. Population/work assignments alter food, wood, and stone on strategic ticks.
6. Diplomatic scores map to hostile/unfriendly/neutral/friendly/allied states.
7. Civilization state survives save/quit/reload in `civilizations.dat`.
8. `EmpireHuman` entities survive save/reload with faction, settlement and role intact.
9. Empire simulation does not require distant NPC entities to remain active.
10. All civilization tests pass under CTest.

## Deferred intentionally

- Culture-selection UI when founding.
- Prefab building placement and settlement upgrades.
- Caravans and trade routes.
- Raids and army-group movement.
- Dynamic borders beyond explicit chunk claims.
- Custom Roman/Egyptian/Persian textures/models.
- Diplomacy screen and kingdom-management screen.
- Walls, roads, barracks, farms, temples and other structure templates.
- Vassals, taxation, treaties, sieges and annexation.
