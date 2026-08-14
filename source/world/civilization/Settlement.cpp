#include "Settlement.hpp"

Settlement::Settlement(int32_t id, FactionId faction, const std::string& name, const TilePos& center)
    : m_id(id),
      m_faction(faction),
      m_name(name),
      m_center(center),
      m_population(0),
      m_farmers(0),
      m_lumberWorkers(0),
      m_quarryWorkers(0)
{
}

int Settlement::clampRelation(int score)
{
    if (score < -100)
        return -100;
    if (score > 100)
        return 100;
    return score;
}

int Settlement::clampNonNegative(int value)
{
    return value < 0 ? 0 : value;
}

void Settlement::setPopulation(int population)
{
    m_population = clampNonNegative(population);
}

void Settlement::setWorkers(int farmers, int lumberWorkers, int quarryWorkers)
{
    m_farmers = clampNonNegative(farmers);
    m_lumberWorkers = clampNonNegative(lumberWorkers);
    m_quarryWorkers = clampNonNegative(quarryWorkers);
}

void Settlement::strategicTick()
{
    const int foodUpkeep = (m_population + 9) / 10;

    m_resources.food = clampNonNegative(m_resources.food + m_farmers * 2 - foodUpkeep);
    m_resources.wood = clampNonNegative(m_resources.wood + m_lumberWorkers);
    m_resources.stone = clampNonNegative(m_resources.stone + m_quarryWorkers);
}

int Settlement::getRelation(int32_t otherSettlementId) const
{
    std::map<int32_t, int>::const_iterator it = m_relations.find(otherSettlementId);
    if (it == m_relations.end())
        return 0;
    return it->second;
}

void Settlement::setRelation(int32_t otherSettlementId, int score)
{
    m_relations[otherSettlementId] = clampRelation(score);
}

void Settlement::adjustRelation(int32_t otherSettlementId, int delta)
{
    setRelation(otherSettlementId, getRelation(otherSettlementId) + delta);
}

DiplomaticStatus Settlement::getDiplomaticStatus(int32_t otherSettlementId) const
{
    const int relation = getRelation(otherSettlementId);

    if (relation <= -60)
        return DIPLOMACY_HOSTILE;
    if (relation <= -20)
        return DIPLOMACY_UNFRIENDLY;
    if (relation < 20)
        return DIPLOMACY_NEUTRAL;
    if (relation < 60)
        return DIPLOMACY_FRIENDLY;
    return DIPLOMACY_ALLIED;
}
