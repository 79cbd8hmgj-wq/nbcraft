#include "CivilizationPersistence.hpp"

#include "nbt/CompoundTag.hpp"
#include "nbt/ListTag.hpp"
#include "world/civilization/CivilizationManager.hpp"

CompoundTag* CivilizationPersistence::save(const CivilizationManager& manager)
{
    CompoundTag* root = new CompoundTag();
    root->putInt32("Version", 1);
    root->putInt32("NextSettlementId", manager.m_nextSettlementId);

    ListTag* settlements = new ListTag(Tag::TAG_TYPE_COMPOUND);
    std::map<int32_t, Settlement>::const_iterator settlementIt = manager.m_settlements.begin();
    for (; settlementIt != manager.m_settlements.end(); ++settlementIt)
    {
        const Settlement& settlement = settlementIt->second;
        CompoundTag* settlementTag = new CompoundTag();
        settlementTag->putInt32("Id", settlement.m_id);
        settlementTag->putInt32("Faction", settlement.m_faction);
        settlementTag->putString("Name", settlement.m_name);
        settlementTag->putInt32("X", settlement.m_center.x);
        settlementTag->putInt32("Y", settlement.m_center.y);
        settlementTag->putInt32("Z", settlement.m_center.z);
        settlementTag->putInt32("Population", settlement.m_population);
        settlementTag->putInt32("Farmers", settlement.m_farmers);
        settlementTag->putInt32("LumberWorkers", settlement.m_lumberWorkers);
        settlementTag->putInt32("QuarryWorkers", settlement.m_quarryWorkers);
        settlementTag->putInt32("Food", settlement.m_resources.food);
        settlementTag->putInt32("Wood", settlement.m_resources.wood);
        settlementTag->putInt32("Stone", settlement.m_resources.stone);
        settlementTag->putInt32("Wealth", settlement.m_resources.wealth);

        ListTag* relations = new ListTag(Tag::TAG_TYPE_COMPOUND);
        std::map<int32_t, int>::const_iterator relationIt = settlement.m_relations.begin();
        for (; relationIt != settlement.m_relations.end(); ++relationIt)
        {
            CompoundTag* relationTag = new CompoundTag();
            relationTag->putInt32("OtherId", relationIt->first);
            relationTag->putInt32("Score", relationIt->second);
            relations->add(relationTag);
        }
        settlementTag->put("Relations", relations);
        settlements->add(settlementTag);
    }
    root->put("Settlements", settlements);

    ListTag* claims = new ListTag(Tag::TAG_TYPE_COMPOUND);
    std::map<ChunkPos, int32_t>::const_iterator claimIt = manager.m_claims.begin();
    for (; claimIt != manager.m_claims.end(); ++claimIt)
    {
        CompoundTag* claimTag = new CompoundTag();
        claimTag->putInt32("X", claimIt->first.x);
        claimTag->putInt32("Z", claimIt->first.z);
        claimTag->putInt32("SettlementId", claimIt->second);
        claims->add(claimTag);
    }
    root->put("Claims", claims);

    return root;
}

bool CivilizationPersistence::load(const CompoundTag& root, CivilizationManager& manager)
{
    if (root.getInt32("Version") != 1)
        return false;

    manager.m_settlements.clear();
    manager.m_claims.clear();

    int32_t maxSettlementId = 0;
    const ListTag* settlements = root.getList("Settlements");
    if (settlements)
    {
        for (size_t i = 0; i < settlements->size(); ++i)
        {
            const CompoundTag* settlementTag = settlements->getCompound(i);
            if (!settlementTag)
                continue;

            const int32_t id = settlementTag->getInt32("Id");
            if (id <= 0)
                continue;

            Settlement settlement(
                id,
                static_cast<FactionId>(settlementTag->getInt32("Faction")),
                settlementTag->getString("Name"),
                TilePos(
                    settlementTag->getInt32("X"),
                    settlementTag->getInt32("Y"),
                    settlementTag->getInt32("Z")
                )
            );

            settlement.m_population = Settlement::clampNonNegative(settlementTag->getInt32("Population"));
            settlement.m_farmers = Settlement::clampNonNegative(settlementTag->getInt32("Farmers"));
            settlement.m_lumberWorkers = Settlement::clampNonNegative(settlementTag->getInt32("LumberWorkers"));
            settlement.m_quarryWorkers = Settlement::clampNonNegative(settlementTag->getInt32("QuarryWorkers"));
            settlement.m_resources.food = Settlement::clampNonNegative(settlementTag->getInt32("Food"));
            settlement.m_resources.wood = Settlement::clampNonNegative(settlementTag->getInt32("Wood"));
            settlement.m_resources.stone = Settlement::clampNonNegative(settlementTag->getInt32("Stone"));
            settlement.m_resources.wealth = Settlement::clampNonNegative(settlementTag->getInt32("Wealth"));

            const ListTag* relations = settlementTag->getList("Relations");
            if (relations)
            {
                for (size_t relationIndex = 0; relationIndex < relations->size(); ++relationIndex)
                {
                    const CompoundTag* relationTag = relations->getCompound(relationIndex);
                    if (!relationTag)
                        continue;

                    settlement.setRelation(
                        relationTag->getInt32("OtherId"),
                        relationTag->getInt32("Score")
                    );
                }
            }

            manager.m_settlements.insert(std::make_pair(id, settlement));
            if (id > maxSettlementId)
                maxSettlementId = id;
        }
    }

    const ListTag* claims = root.getList("Claims");
    if (claims)
    {
        for (size_t i = 0; i < claims->size(); ++i)
        {
            const CompoundTag* claimTag = claims->getCompound(i);
            if (!claimTag)
                continue;

            const int32_t settlementId = claimTag->getInt32("SettlementId");
            if (!manager.getSettlement(settlementId))
                continue;

            manager.m_claims.insert(std::make_pair(
                ChunkPos(claimTag->getInt32("X"), claimTag->getInt32("Z")),
                settlementId
            ));
        }
    }

    const int32_t savedNextId = root.getInt32("NextSettlementId");
    manager.m_nextSettlementId = savedNextId > maxSettlementId ? savedNextId : maxSettlementId + 1;
    if (manager.m_nextSettlementId < 1)
        manager.m_nextSettlementId = 1;

    return true;
}
