#pragma once

#include <stdint.h>
#include <map>
#include <string>

#include "world/civilization/CivilizationTypes.hpp"
#include "world/level/TilePos.hpp"

class CivilizationPersistence;

class Settlement
{
public:
    Settlement(int32_t id, FactionId faction, const std::string& name, const TilePos& center);

    int32_t getId() const { return m_id; }
    FactionId getFaction() const { return m_faction; }
    const std::string& getName() const { return m_name; }
    const TilePos& getCenter() const { return m_center; }

    int getPopulation() const { return m_population; }
    void setPopulation(int population);

    ResourceStockpile& resources() { return m_resources; }
    const ResourceStockpile& resources() const { return m_resources; }

    void setWorkers(int farmers, int lumberWorkers, int quarryWorkers);
    void strategicTick();

    int getRelation(int32_t otherSettlementId) const;
    void setRelation(int32_t otherSettlementId, int score);
    void adjustRelation(int32_t otherSettlementId, int delta);
    DiplomaticStatus getDiplomaticStatus(int32_t otherSettlementId) const;

private:
    friend class CivilizationPersistence;

    static int clampRelation(int score);
    static int clampNonNegative(int value);

    int32_t m_id;
    FactionId m_faction;
    std::string m_name;
    TilePos m_center;
    int m_population;
    int m_farmers;
    int m_lumberWorkers;
    int m_quarryWorkers;
    ResourceStockpile m_resources;
    std::map<int32_t, int> m_relations;
};
