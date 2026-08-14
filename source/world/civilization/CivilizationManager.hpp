#pragma once

#include <stdint.h>
#include <map>
#include <string>

#include "world/civilization/Settlement.hpp"
#include "world/level/levelgen/chunk/ChunkPos.hpp"

class CivilizationManager
{
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

    const std::map<int32_t, Settlement>& getSettlements() const { return m_settlements; }

private:
    int32_t m_nextSettlementId;
    std::map<int32_t, Settlement> m_settlements;
    std::map<ChunkPos, int32_t> m_claims;
};
