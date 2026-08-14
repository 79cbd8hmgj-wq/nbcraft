#include <assert.h>

#include "world/civilization/CivilizationManager.hpp"

int main()
{
    CivilizationManager manager;

    const int32_t romanId = manager.foundSettlement(
        FACTION_ROMAN,
        "Roma",
        TilePos(0, 64, 0)
    );
    const int32_t egyptianId = manager.foundSettlement(
        FACTION_EGYPTIAN,
        "Memphis",
        TilePos(32, 64, 0)
    );

    assert(romanId == 1);
    assert(egyptianId == 2);

    assert(manager.isChunkClaimed(ChunkPos(0, 0)));
    assert(manager.getSettlementAt(ChunkPos(0, 0))->getId() == romanId);
    assert(manager.getSettlementAt(ChunkPos(2, 0))->getId() == egyptianId);

    assert(!manager.claimChunk(egyptianId, ChunkPos(0, 0)));
    assert(manager.claimChunk(romanId, ChunkPos(1, 0)));
    assert(manager.getSettlementAt(ChunkPos(1, 0))->getId() == romanId);

    Settlement* roma = manager.getSettlement(romanId);
    assert(roma != 0);
    roma->setPopulation(20);
    roma->setWorkers(5, 2, 1);

    manager.strategicTick();

    assert(roma->resources().food == 8);
    assert(roma->resources().wood == 2);
    assert(roma->resources().stone == 1);

    return 0;
}
