#include "CivilizationManager.hpp"

CivilizationManager::CivilizationManager()
    : m_nextSettlementId(1)
{
}

int32_t CivilizationManager::foundSettlement(FactionId faction, const std::string& name, const TilePos& center)
{
    const int32_t id = m_nextSettlementId++;
    m_settlements.insert(std::make_pair(id, Settlement(id, faction, name, center)));

    const ChunkPos centerChunk(
        ChunkPos::ToChunkCoordinate(center.x),
        ChunkPos::ToChunkCoordinate(center.z)
    );
    claimChunk(id, centerChunk);

    if (faction == FACTION_ROMAN)
    {
        Settlement* romanSettlement = getSettlement(id);
        std::map<int32_t, Settlement>::iterator it = m_settlements.begin();
        for (; it != m_settlements.end(); ++it)
        {
            if (it->first == id)
                continue;

            const FactionId otherFaction = it->second.getFaction();
            if (otherFaction != FACTION_EGYPTIAN && otherFaction != FACTION_PERSIAN)
                continue;

            romanSettlement->setRelation(it->first, -30);
            it->second.setRelation(id, -30);
        }
    }

    return id;
}

void CivilizationManager::seedPrototypeRivals(const TilePos& playerSpawn)
{
    if (!m_settlements.empty())
        return;

    const int rivalDistanceBlocks = 24 * 16;

    const int32_t egyptianId = foundSettlement(
        FACTION_EGYPTIAN,
        "Memphis",
        TilePos(playerSpawn.x + rivalDistanceBlocks, playerSpawn.y, playerSpawn.z)
    );
    const int32_t persianId = foundSettlement(
        FACTION_PERSIAN,
        "Pasargadae",
        TilePos(playerSpawn.x - rivalDistanceBlocks, playerSpawn.y, playerSpawn.z)
    );

    Settlement* egypt = getSettlement(egyptianId);
    Settlement* persia = getSettlement(persianId);

    if (egypt)
    {
        egypt->setPopulation(20);
        egypt->resources().food = 100;
        egypt->resources().wood = 50;
        egypt->resources().stone = 50;
        egypt->resources().wealth = 25;
    }

    if (persia)
    {
        persia->setPopulation(20);
        persia->resources().food = 100;
        persia->resources().wood = 50;
        persia->resources().stone = 50;
        persia->resources().wealth = 25;
    }
}

Settlement* CivilizationManager::getSettlement(int32_t id)
{
    std::map<int32_t, Settlement>::iterator it = m_settlements.find(id);
    if (it == m_settlements.end())
        return 0;
    return &it->second;
}

const Settlement* CivilizationManager::getSettlement(int32_t id) const
{
    std::map<int32_t, Settlement>::const_iterator it = m_settlements.find(id);
    if (it == m_settlements.end())
        return 0;
    return &it->second;
}

Settlement* CivilizationManager::getSettlementAt(const ChunkPos& chunk)
{
    std::map<ChunkPos, int32_t>::iterator it = m_claims.find(chunk);
    if (it == m_claims.end())
        return 0;
    return getSettlement(it->second);
}

const Settlement* CivilizationManager::getSettlementAt(const ChunkPos& chunk) const
{
    std::map<ChunkPos, int32_t>::const_iterator it = m_claims.find(chunk);
    if (it == m_claims.end())
        return 0;
    return getSettlement(it->second);
}

bool CivilizationManager::claimChunk(int32_t settlementId, const ChunkPos& chunk)
{
    if (!getSettlement(settlementId))
        return false;

    std::map<ChunkPos, int32_t>::iterator it = m_claims.find(chunk);
    if (it != m_claims.end())
        return it->second == settlementId;

    m_claims.insert(std::make_pair(chunk, settlementId));
    return true;
}

bool CivilizationManager::isChunkClaimed(const ChunkPos& chunk) const
{
    return m_claims.find(chunk) != m_claims.end();
}

void CivilizationManager::strategicTick()
{
    std::map<int32_t, Settlement>::iterator it = m_settlements.begin();
    for (; it != m_settlements.end(); ++it)
        it->second.strategicTick();
}
