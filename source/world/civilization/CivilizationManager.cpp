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

    return id;
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
