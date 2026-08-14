#pragma once

enum FactionId
{
    FACTION_NONE = 0,
    FACTION_ROMAN,
    FACTION_EGYPTIAN,
    FACTION_PERSIAN
};

enum DiplomaticStatus
{
    DIPLOMACY_HOSTILE = 0,
    DIPLOMACY_UNFRIENDLY,
    DIPLOMACY_NEUTRAL,
    DIPLOMACY_FRIENDLY,
    DIPLOMACY_ALLIED
};

struct ResourceStockpile
{
    int food;
    int wood;
    int stone;
    int wealth;

    ResourceStockpile() : food(0), wood(0), stone(0), wealth(0) {}
};
