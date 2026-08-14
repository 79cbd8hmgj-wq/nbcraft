#pragma once

#include <string>

class CivilizationManager;

class CivilizationFileStorage
{
public:
    static bool load(const std::string& levelDirectory, CivilizationManager& manager);
    static bool save(const std::string& levelDirectory, const CivilizationManager& manager);
};
