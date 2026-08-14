#include <assert.h>

#include "world/civilization/Settlement.hpp"

int main()
{
    Settlement settlement(1, FACTION_ROMAN, "Roma", TilePos(0, 64, 0));

    assert(settlement.getDiplomaticStatus(2) == DIPLOMACY_NEUTRAL);

    settlement.adjustRelation(2, -70);
    assert(settlement.getDiplomaticStatus(2) == DIPLOMACY_HOSTILE);

    settlement.setPopulation(20);
    settlement.setWorkers(5, 2, 1);
    settlement.strategicTick();

    assert(settlement.resources().food == 8);
    assert(settlement.resources().wood == 2);
    assert(settlement.resources().stone == 1);

    return 0;
}
