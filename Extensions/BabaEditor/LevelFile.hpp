// Copyright (c) 2020-2026 Chris Ohk

#ifndef BABA_EDITOR_LEVEL_FILE_HPP
#define BABA_EDITOR_LEVEL_FILE_HPP

#include <baba-is-auto/Enums/GameEnums.hpp>

#include <filesystem>
#include <fstream>
#include <system_error>
#include <utility>
#include <vector>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace baba_is_auto::editor
{
namespace fs = std::filesystem;

constexpr std::size_t MIN_LEVEL_WIDTH = 3;
constexpr std::size_t MAX_LEVEL_WIDTH = 33;
constexpr std::size_t MIN_LEVEL_HEIGHT = 1;
constexpr std::size_t MAX_LEVEL_HEIGHT = 18;

//! The single-layer numeric map format consumed by baba-is-auto.
struct LevelFile
{
    std::size_t width = 0;
    std::size_t height = 0;
    std::vector<ObjectType> tiles;
};

//! Loads a level file from the given path into the provided LevelFile object.
inline bool LoadLevelFile(const fs::path& filename, LevelFile& level)
{
    std::ifstream file(filename);
    LevelFile loaded;

    if (!(file >> loaded.width >> loaded.height) ||
        loaded.width < MIN_LEVEL_WIDTH || loaded.width > MAX_LEVEL_WIDTH ||
        loaded.height < MIN_LEVEL_HEIGHT || loaded.height > MAX_LEVEL_HEIGHT)
    {
        return false;
    }

    loaded.tiles.resize(loaded.width * loaded.height);

    for (ObjectType& tile : loaded.tiles)
    {
        int value = 0;

        if (!(file >> value))
        {
            return false;
        }

        const ObjectType type = static_cast<ObjectType>(value);

        if (value <= static_cast<int>(ObjectType::NOUN_TYPE) ||
            value > static_cast<int>(ObjectType::ICON_WATER) ||
            type == ObjectType::OP_TYPE || type == ObjectType::PROPERTY_TYPE ||
            type == ObjectType::ICON_TYPE)
        {
            return false;
        }

        tile = type;
    }

    level = std::move(loaded);
    return true;
}

//! Replaces the destination file with the temporary file, ensuring atomicity.
inline bool ReplaceLevelFile(const fs::path& temporary,
                             const fs::path& destination)
{
#ifdef _WIN32
    return MoveFileExW(temporary.c_str(), destination.c_str(),
                       MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    std::error_code error;
    fs::rename(temporary, destination, error);
    return !error;
#endif
}

//! Saves the provided LevelFile object to the given path, ensuring atomicity.
inline bool SaveLevelFile(const fs::path& filename, const LevelFile& level)
{
    if (level.width < MIN_LEVEL_WIDTH || level.width > MAX_LEVEL_WIDTH ||
        level.height < MIN_LEVEL_HEIGHT || level.height > MAX_LEVEL_HEIGHT ||
        level.tiles.size() != level.width * level.height)
    {
        return false;
    }

    fs::path temporary = filename;
    temporary += ".tmp";

    std::ofstream file(temporary, std::ios::trunc);

    if (!file)
    {
        return false;
    }

    file << level.width << ' ' << level.height << '\n';

    for (std::size_t y = 0; y < level.height; ++y)
    {
        for (std::size_t x = 0; x < level.width; ++x)
        {
            file << static_cast<int>(level.tiles[y * level.width + x]);

            if (x + 1 < level.width)
            {
                file << ' ';
            }
        }

        file << '\n';
    }

    file.close();

    if (!file || !ReplaceLevelFile(temporary, filename))
    {
        std::error_code error;
        fs::remove(temporary, error);
        return false;
    }

    return true;
}
}  // namespace baba_is_auto::editor

#endif
