// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <doctest/doctest.h>

#include <LevelFile.hpp>

#include <baba-is-auto/Games/Game.hpp>

#include <filesystem>
#include <fstream>
#include <limits>

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
    CHECK(LoadLevelFile(MAPS_DIR "editor_smoke.txt", source));
    CHECK(source.tiles[0][0] == ObjectType::BABA);
    CHECK(source.tiles[0][1] == ObjectType::ICON_EMPTY);
    CHECK(source.tiles[0][2] == ObjectType::ICON_EMPTY);
    CHECK(source.directions[0][0] == Direction::RIGHT);

    {
        std::ofstream reserved(predictableTemporary);
        reserved << "reserved";
    }

    CHECK(SaveLevelFile(path, source));

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
    source.directions[0][0] = Direction::UP;
    source.directions[1][1] = Direction::LEFT;
    CHECK(SaveLevelFile(path, source));

    LevelFile loaded;
    CHECK(LoadLevelFile(path, loaded));
    CHECK(loaded.width == source.width);
    CHECK(loaded.height == source.height);
    CHECK(loaded.tiles == source.tiles);
    CHECK(loaded.directions == source.directions);

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

    Map map;

    {
        std::ofstream invalid(path, std::ios::trunc);
        invalid << "0 1\n";
    }

    CHECK_THROWS_AS(map.Load(path.string()), std::runtime_error);

    {
        std::ofstream invalid(path, std::ios::trunc);
        invalid << "3 1\n132 132\n";
    }

    CHECK_THROWS_AS(map.Load(path.string()), std::runtime_error);

    {
        std::ofstream invalid(path, std::ios::trunc);
        invalid << "3 1\n0 132 132\n";
    }

    CHECK_THROWS_AS(map.Load(path.string()), std::runtime_error);
    fs::remove(path, error);
    fs::remove(predictableTemporary, error);
}

TEST_CASE("Map - Load Dimension Limits")
{
    namespace fs = std::filesystem;

    const fs::path path =
        fs::current_path() / "baba-is-auto-map-dimension-limits.txt";
    const auto maxCoordinate =
        static_cast<std::size_t>(std::numeric_limits<int>::max());
    std::error_code error;
    Map map;

    {
        std::ofstream boundary(path);
        boundary << maxCoordinate << " 1\n";
    }

    CHECK_THROWS_WITH(map.Load(path.string()), "Invalid map tile data");

    {
        std::ofstream oversized(path, std::ios::trunc);
        oversized << maxCoordinate + 1 << " 1\n";
    }

    CHECK_THROWS_WITH(map.Load(path.string()), "Invalid map dimensions");
    fs::remove(path, error);
}

TEST_CASE("Editor - Affection direction round trip")
{
    namespace fs = std::filesystem;

    LevelFile affection;
    CHECK(LoadLevelFile(MAPS_DIR "affection.txt", affection));
    CHECK(affection.width == 24);
    CHECK(affection.height == 14);
    CHECK(affection.tiles[3 * affection.width + 6][0] == ObjectType::ICON_KEKE);
    CHECK(affection.directions[3 * affection.width + 6][0] == Direction::RIGHT);
    CHECK(affection.directions[7 * affection.width + 7][0] == Direction::UP);
    CHECK(affection.directions[10 * affection.width + 12][0] ==
          Direction::DOWN);

    const fs::path path =
        fs::current_path() / "baba-is-auto-affection-round-trip.txt";
    std::error_code error;
    CHECK(SaveLevelFile(path, affection));

    LevelFile loaded;
    CHECK(LoadLevelFile(path, loaded));
    CHECK(loaded.tiles == affection.tiles);
    CHECK(loaded.directions == affection.directions);

    fs::remove(path, error);
}

TEST_CASE("Editor - Affection sprite assets")
{
    namespace fs = std::filesystem;

    const fs::path root = (fs::path(MAPS_DIR) / "../..").lexically_normal();

    for (const char* path : {
             "Extensions/BabaGUI/sprites/text/KEKE.gif",
             "Extensions/BabaGUI/sprites/text/LOVE.gif",
             "Extensions/BabaGUI/sprites/text/ALGAE.gif",
             "Extensions/BabaGUI/sprites/text/MOVE.gif",
             "Extensions/BabaGUI/sprites/icon/KEKE.gif",
             "Extensions/BabaGUI/sprites/icon/LOVE.gif",
             "Extensions/BabaGUI/sprites/icon/ALGAE.gif",
         })
    {
        CHECK(fs::is_regular_file(root / path));
    }
}

TEST_CASE("Editor - Pillar Yard round trip and sprite assets")
{
    namespace fs = std::filesystem;

    LevelFile pillarYard;
    REQUIRE(LoadLevelFile(MAPS_DIR "pillar_yard.txt", pillarYard));
    CHECK(pillarYard.width == 24);
    CHECK(pillarYard.height == 14);
    CHECK(pillarYard.tiles[2 * pillarYard.width + 5][0] == ObjectType::PILLAR);
    CHECK(pillarYard.tiles[7 * pillarYard.width + 8][0] ==
          ObjectType::ICON_PILLAR);
    CHECK(pillarYard.tiles[4 * pillarYard.width + 3][0] ==
          ObjectType::ICON_BRICK);

    const fs::path path =
        fs::current_path() / "baba-is-auto-pillar-yard-round-trip.txt";
    std::error_code error;
    CHECK(SaveLevelFile(path, pillarYard));

    LevelFile loaded;
    REQUIRE(LoadLevelFile(path, loaded));
    CHECK(loaded.tiles == pillarYard.tiles);
    CHECK(loaded.directions == pillarYard.directions);

    fs::remove(path, error);

    const fs::path root = (fs::path(MAPS_DIR) / "../..").lexically_normal();

    for (const char* asset : {
             "Extensions/BabaGUI/sprites/text/PILLAR.gif",
             "Extensions/BabaGUI/sprites/icon/PILLAR.gif",
             "Extensions/BabaGUI/sprites/icon/BRICK.gif",
         })
    {
        CHECK(fs::is_regular_file(root / asset));
    }
}

TEST_CASE("Editor - Brick Wall round trip and sprite assets")
{
    namespace fs = std::filesystem;

    LevelFile brickWall;
    REQUIRE(LoadLevelFile(MAPS_DIR "brick_wall.txt", brickWall));
    CHECK(brickWall.width == 15);
    CHECK(brickWall.height == 8);
    CHECK(brickWall.tiles[4 * brickWall.width + 10][0] ==
          ObjectType::ICON_BRICK);
    CHECK(brickWall.tiles[4 * brickWall.width + 11][0] ==
          ObjectType::ICON_FLAG);
    CHECK(brickWall.tiles[4 * brickWall.width + 11][1] ==
          ObjectType::ICON_EMPTY);

    const fs::path path =
        fs::current_path() / "baba-is-auto-brick-wall-round-trip.txt";
    std::error_code error;
    CHECK(SaveLevelFile(path, brickWall));

    LevelFile loaded;
    REQUIRE(LoadLevelFile(path, loaded));
    CHECK(loaded.tiles == brickWall.tiles);
    CHECK(loaded.directions == brickWall.directions);

    fs::remove(path, error);

    const fs::path root = (fs::path(MAPS_DIR) / "../..").lexically_normal();

    for (const char* asset : {
             "Extensions/BabaGUI/sprites/text/BRICK.gif",
             "Extensions/BabaGUI/sprites/icon/BRICK.gif",
         })
    {
        CHECK(fs::is_regular_file(root / asset));
    }
}

TEST_CASE("Editor - Layer tile direction updates")
{
    LevelFile::LayerTile tiles{ ObjectType::ICON_EMPTY, ObjectType::ICON_EMPTY,
                                ObjectType::ICON_EMPTY };
    LevelFile::LayerDirections directions{ Direction::RIGHT, Direction::RIGHT,
                                           Direction::RIGHT };

    CHECK(SetLevelLayerTile(tiles, directions, 1, ObjectType::ICON_KEKE,
                            Direction::UP));
    CHECK(tiles[1] == ObjectType::ICON_KEKE);
    CHECK(directions[1] == Direction::UP);
    CHECK_FALSE(SetLevelLayerTile(tiles, directions, 1, ObjectType::ICON_KEKE,
                                  Direction::NONE));
    CHECK(directions[1] == Direction::UP);
    CHECK(SetLevelLayerTile(tiles, directions, 1, ObjectType::ICON_EMPTY,
                            Direction::DOWN));
    CHECK(tiles[1] == ObjectType::ICON_EMPTY);
    CHECK(directions[1] == Direction::RIGHT);
}
