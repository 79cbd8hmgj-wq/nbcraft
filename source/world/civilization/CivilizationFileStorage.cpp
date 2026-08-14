#include "CivilizationFileStorage.hpp"

#include <stdio.h>
#include <stdint.h>

#include "nbt/CompoundTag.hpp"
#include "nbt/NbtIo.hpp"
#include "network/RakIO.hpp"
#include "world/civilization/CivilizationManager.hpp"
#include "world/civilization/CivilizationPersistence.hpp"

namespace
{
    const char CIVILIZATION_MAGIC[4] = { 'C', 'I', 'V', '\0' };
    const int32_t CIVILIZATION_FILE_VERSION = 1;

    unsigned int getRemainingBytes(FILE* file)
    {
        const long current = ftell(file);
        if (current < 0)
            return 0;

        if (fseek(file, 0, SEEK_END) != 0)
            return 0;

        const long end = ftell(file);
        if (end < current)
            return 0;

        if (fseek(file, current, SEEK_SET) != 0)
            return 0;

        return static_cast<unsigned int>(end - current);
    }
}

bool CivilizationFileStorage::load(const std::string& levelDirectory, CivilizationManager& manager)
{
    const std::string fileName = levelDirectory + "/civilizations.dat";
    FILE* file = fopen(fileName.c_str(), "rb");
    if (!file)
        return false;

    char magic[4];
    if (fread(magic, 1, sizeof(magic), file) != sizeof(magic))
    {
        fclose(file);
        return false;
    }

    if (magic[0] != CIVILIZATION_MAGIC[0]
        || magic[1] != CIVILIZATION_MAGIC[1]
        || magic[2] != CIVILIZATION_MAGIC[2])
    {
        fclose(file);
        return false;
    }

    int32_t fileVersion = 0;
    if (fread(&fileVersion, sizeof(fileVersion), 1, file) != 1
        || fileVersion != CIVILIZATION_FILE_VERSION)
    {
        fclose(file);
        return false;
    }

    uint32_t size = 0;
    if (fread(&size, sizeof(size), 1, file) != 1
        || size == 0
        || size > getRemainingBytes(file))
    {
        fclose(file);
        return false;
    }

    uint8_t* data = new uint8_t[size];
    if (fread(data, 1, size, file) != size)
    {
        delete[] data;
        fclose(file);
        return false;
    }

    fclose(file);

    RakNet::BitStream bitStream(data, size, false);
    RakDataInput input(bitStream);
    CompoundTag* root = NbtIo::read(input);

    delete[] data;

    if (!root)
        return false;

    const bool loaded = CivilizationPersistence::load(*root, manager);
    root->deleteChildren();
    delete root;
    return loaded;
}

bool CivilizationFileStorage::save(const std::string& levelDirectory, const CivilizationManager& manager)
{
    CompoundTag* root = CivilizationPersistence::save(manager);
    if (!root)
        return false;

    RakNet::BitStream bitStream;
    RakDataOutput output(bitStream);
    NbtIo::write(*root, output);

    root->deleteChildren();
    delete root;

    const uint32_t size = bitStream.GetNumberOfBytesUsed();
    if (size == 0)
        return false;

    const std::string fileName = levelDirectory + "/civilizations.dat";
    const std::string temporaryFileName = fileName + ".tmp";

    FILE* file = fopen(temporaryFileName.c_str(), "wb");
    if (!file)
        return false;

    bool writeSucceeded = true;
    writeSucceeded = writeSucceeded && fwrite(CIVILIZATION_MAGIC, 1, sizeof(CIVILIZATION_MAGIC), file) == sizeof(CIVILIZATION_MAGIC);
    writeSucceeded = writeSucceeded && fwrite(&CIVILIZATION_FILE_VERSION, sizeof(CIVILIZATION_FILE_VERSION), 1, file) == 1;
    writeSucceeded = writeSucceeded && fwrite(&size, sizeof(size), 1, file) == 1;
    writeSucceeded = writeSucceeded && fwrite(bitStream.GetData(), 1, size, file) == size;

    if (fclose(file) != 0)
        writeSucceeded = false;

    if (!writeSucceeded)
    {
        remove(temporaryFileName.c_str());
        return false;
    }

    remove(fileName.c_str());
    if (rename(temporaryFileName.c_str(), fileName.c_str()) != 0)
    {
        remove(temporaryFileName.c_str());
        return false;
    }

    return true;
}
