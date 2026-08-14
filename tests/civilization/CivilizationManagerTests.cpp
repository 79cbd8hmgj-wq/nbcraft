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

    CivilizationManager rivals;
    rivals.seedPrototypeRivals(TilePos(100, 64, 100));
    rivals.seedPrototypeRivals(TilePos(100, 64, 100));

    assert(rivals.getSettlements().size() == 2);

    const Settlement* egypt = rivals.getSettlement(1);
    const Settlement* persia = rivals.getSettlement(2);
    assert(egypt != 0);
    assert(persia != 0);
    assert(egypt->getFaction() == FACTION_EGYPTIAN);
    assert(persia->getFaction() == FACTION_PERSIAN);
    assert(egypt->getCenter() == TilePos(484, 64, 100));
    assert(persia->getCenter() == TilePos(-284, 64, 100));
    assert(egypt->getPopulation() == 20);
    assert(persia->getPopulation() == 20);
    assert(egypt->resources().food == 100);
    assert(egypt->resources().wood == 50);
    assert(egypt->resources().stone == 50);
    assert(egypt->resources().wealth == 25);

    const int32_t rivalRomanId = rivals.foundSettlement(
        FACTION_ROMAN,
        "Roma",
        TilePos(100, 64, 100)
    );
    assert(rivalRomanId == 3);
    assert(rivals.getSettlement(1)->getRelation(rivalRomanId) == -30);
    assert(rivals.getSettlement(2)->getRelation(rivalRomanId) == -30);
    assert(rivals.getSettlement(rivalRomanId)->getRelation(1) == -30);
    assert(rivals.getSettlement(rivalRomanId)->getRelation(2) == -30);

    return 0;
}
