// Copyright (c) 2020-2026 Chris Ohk

#ifndef BABA_EDITOR_LEVEL_FILE_HPP
#define BABA_EDITOR_LEVEL_FILE_HPP

#include <baba-is-auto/Enums/GameEnums.hpp>

#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
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
#else
#include <cerrno>
#include <cstdlib>
#include <unistd.h>
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

//! Returns true when the object type can be stored in the numeric map format.
inline bool IsValidLevelTile(ObjectType type)
{
    const int value = static_cast<int>(type);
    return value > static_cast<int>(ObjectType::NOUN_TYPE) &&
           value <= static_cast<int>(ObjectType::ICON_WATER) &&
           type != ObjectType::OP_TYPE && type != ObjectType::PROPERTY_TYPE &&
           type != ObjectType::ICON_TYPE;
}

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

        const auto type = static_cast<ObjectType>(value);

        if (!IsValidLevelTile(type))
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

//! Creates a unique temporary file beside the destination and writes its
//! contents through the exclusively created native file handle.
inline bool WriteTemporaryLevelFile(const fs::path& destination,
                                    const std::string& contents,
                                    fs::path& temporary)
{
#ifdef _WIN32
    std::random_device random;

    for (unsigned int attempt = 0; attempt < 64; ++attempt)
    {
        temporary = destination;
        temporary += L".tmp.";
        temporary += std::to_wstring(GetCurrentProcessId());
        temporary += L".";
        temporary += std::to_wstring(random());
        temporary += L".";
        temporary += std::to_wstring(attempt);

        HANDLE file = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr,
                                  CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY, nullptr);

        if (file == INVALID_HANDLE_VALUE)
        {
            const DWORD error = GetLastError();

            if (error == ERROR_FILE_EXISTS || error == ERROR_ALREADY_EXISTS)
            {
                continue;
            }

            return false;
        }

        bool success = true;
        std::size_t offset = 0;

        while (offset < contents.size())
        {
            DWORD written = 0;
            const DWORD remaining =
                static_cast<DWORD>(contents.size() - offset);

            if (!WriteFile(file, contents.data() + offset, remaining, &written,
                           nullptr) ||
                written == 0)
            {
                success = false;
                break;
            }

            offset += written;
        }

        success = success && FlushFileBuffers(file) != 0;
        success = CloseHandle(file) != 0 && success;

        if (!success)
        {
            std::error_code error;
            fs::remove(temporary, error);
        }

        return success;
    }

    return false;
#else
    std::string pattern = destination.native() + ".tmp.XXXXXX";
    std::vector<char> path(pattern.begin(), pattern.end());
    path.push_back('\0');

    const int file = ::mkstemp(path.data());

    if (file == -1)
    {
        return false;
    }

    temporary = fs::path(path.data());
    bool success = true;
    std::size_t offset = 0;

    while (offset < contents.size())
    {
        const ssize_t written =
            ::write(file, contents.data() + offset, contents.size() - offset);

        if (written == -1 && errno == EINTR)
        {
            continue;
        }

        if (written <= 0)
        {
            success = false;
            break;
        }

        offset += static_cast<std::size_t>(written);
    }

    success = success && ::fsync(file) == 0;
    success = ::close(file) == 0 && success;

    if (!success)
    {
        std::error_code error;
        fs::remove(temporary, error);
    }

    return success;
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

    for (const ObjectType tile : level.tiles)
    {
        if (!IsValidLevelTile(tile))
        {
            return false;
        }
    }

    std::ostringstream file;
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

    fs::path temporary;

    if (!file || !WriteTemporaryLevelFile(filename, file.str(), temporary) ||
        !ReplaceLevelFile(temporary, filename))
    {
        std::error_code error;
        fs::remove(temporary, error);
        return false;
    }

    return true;
}
}  // namespace baba_is_auto::editor

#endif
