#pragma once

class CompoundTag;
class CivilizationManager;

class CivilizationPersistence
{
public:
    static CompoundTag* save(const CivilizationManager& manager);
    static bool load(const CompoundTag& root, CivilizationManager& manager);
};
