#include <assert.h>

#include "nbt/CompoundTag.hpp"
#include "world/civilization/CivilizationManager.hpp"
#include "world/civilization/CivilizationPersistence.hpp"

int main()
{
    CivilizationManager source;

    const int32_t romanId = source.foundSettlement(
        FACTION_ROMAN,
        "Roma",
        TilePos(0, 64, 0)
    );
    const int32_t egyptianId = source.foundSettlement(
        FACTION_EGYPTIAN,
        "Memphis",
        TilePos(32, 65, 0)
    );

    Settlement* roma = source.getSettlement(romanId);
    assert(roma != 0);
    roma->setPopulation(20);
    roma->setWorkers(5, 2, 1);
    roma->resources().food = 100;
    roma->resources().wood = 50;
    roma->resources().stone = 25;
    roma->resources().wealth = 10;
    roma->setRelation(egyptianId, -70);

    assert(source.claimChunk(romanId, ChunkPos(1, 0)));

    CompoundTag* saved = CivilizationPersistence::save(source);
    assert(saved != 0);

    CivilizationManager loaded;
    assert(CivilizationPersistence::load(*saved, loaded));

    Settlement* loadedRoma = loaded.getSettlement(romanId);
    assert(loadedRoma != 0);
    assert(loadedRoma->getFaction() == FACTION_ROMAN);
    assert(loadedRoma->getName() == "Roma");
    assert(loadedRoma->getCenter() == TilePos(0, 64, 0));
    assert(loadedRoma->getPopulation() == 20);
    assert(loadedRoma->resources().food == 100);
    assert(loadedRoma->resources().wood == 50);
    assert(loadedRoma->resources().stone == 25);
    assert(loadedRoma->resources().wealth == 10);
    assert(loadedRoma->getRelation(egyptianId) == -70);
    assert(loaded.getSettlementAt(ChunkPos(1, 0))->getId() == romanId);

    loaded.strategicTick();
    assert(loadedRoma->resources().food == 108);
    assert(loadedRoma->resources().wood == 52);
    assert(loadedRoma->resources().stone == 26);

    const int32_t persianId = loaded.foundSettlement(
        FACTION_PERSIAN,
        "Pasargadae",
        TilePos(64, 64, 0)
    );
    assert(persianId == 3);

    saved->deleteChildren();
    delete saved;

    return 0;
}
