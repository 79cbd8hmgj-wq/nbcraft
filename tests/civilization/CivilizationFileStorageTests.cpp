#include <assert.h>
#include <stdio.h>

#include "common/Utils.hpp"
#include "world/civilization/CivilizationFileStorage.hpp"
#include "world/civilization/CivilizationManager.hpp"

int main()
{
    const std::string directory = "build/civilization-file-storage-test";
    createFolderIfNotExists(directory.c_str());

    CivilizationManager source;
    const int32_t romanId = source.foundSettlement(
        FACTION_ROMAN,
        "Roma",
        TilePos(0, 64, 0)
    );

    Settlement* roma = source.getSettlement(romanId);
    assert(roma != 0);
    roma->setPopulation(25);
    roma->setWorkers(6, 3, 2);
    roma->resources().food = 120;
    roma->resources().wood = 60;
    roma->resources().stone = 40;
    roma->resources().wealth = 15;

    assert(CivilizationFileStorage::save(directory, source));

    CivilizationManager loaded;
    assert(CivilizationFileStorage::load(directory, loaded));

    const Settlement* loadedRoma = loaded.getSettlement(romanId);
    assert(loadedRoma != 0);
    assert(loadedRoma->getPopulation() == 25);
    assert(loadedRoma->resources().food == 120);
    assert(loadedRoma->resources().wood == 60);
    assert(loadedRoma->resources().stone == 40);
    assert(loadedRoma->resources().wealth == 15);

    remove((directory + "/civilizations.dat").c_str());
    return 0;
}
