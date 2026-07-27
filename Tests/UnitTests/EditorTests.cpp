// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <doctest/doctest.h>

#include <LevelFile.hpp>

#include <baba-is-auto/Games/Game.hpp>

#include <filesystem>
#include <fstream>

using namespace baba_is_auto;
using namespace baba_is_auto::editor;

TEST_CASE("Editor - Level File Round Trip")
{
    namespace fs = std::filesystem;

    const fs::path path =
        fs::current_path() / "baba-is-auto-editor-round-trip.txt";
    fs::path predictableTemporary = path;
    predictableTemporary += ".tmp";
    std::error_code error;
    fs::remove(path, error);
    fs::remove(predictableTemporary, error);

    LevelFile source;
    REQUIRE(LoadLevelFile(MAPS_DIR "editor_smoke.txt", source));
    CHECK(source.tiles[0][0] == ObjectType::BABA);
    CHECK(source.tiles[0][1] == ObjectType::ICON_EMPTY);
    CHECK(source.tiles[0][2] == ObjectType::ICON_EMPTY);

    {
        std::ofstream reserved(predictableTemporary);
        reserved << "reserved";
    }

    REQUIRE(SaveLevelFile(path, source));

    {
        std::ifstream reserved(predictableTemporary);
        std::string contents;
        reserved >> contents;
        CHECK(contents == "reserved");
    }

    source.tiles[0] = { ObjectType::ICON_BABA, ObjectType::ICON_EMPTY,
                        ObjectType::ROCK };
    source.tiles[1] = { ObjectType::ICON_EMPTY, ObjectType::ICON_WALL,
                        ObjectType::ICON_EMPTY };
    REQUIRE(SaveLevelFile(path, source));

    LevelFile loaded;
    REQUIRE(LoadLevelFile(path, loaded));
    CHECK(loaded.width == source.width);
    CHECK(loaded.height == source.height);
    CHECK(loaded.tiles == source.tiles);

    LevelFile invalidSave = source;
    invalidSave.tiles[0][1] = ObjectType::OP_TYPE;
    CHECK_FALSE(SaveLevelFile(path, invalidSave));

    {
        std::ofstream truncated(path, std::ios::trunc);
        truncated << "3 2\n1 2\n";
    }

    CHECK_FALSE(LoadLevelFile(path, loaded));

    {
        std::ofstream invalid(path, std::ios::trunc);
        invalid << "3 1\n-1 -1 -1\n";
    }

    CHECK_FALSE(LoadLevelFile(path, loaded));
    fs::remove(path, error);
    fs::remove(predictableTemporary, error);
}
