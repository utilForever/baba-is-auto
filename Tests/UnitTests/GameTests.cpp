// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#ifdef NEAR
#undef NEAR
#endif

#include <baba-is-auto/Agents/Preprocess.hpp>
#include <baba-is-auto/Agents/RandomAgent.hpp>
#include <baba-is-auto/Games/Game.hpp>
#include <baba-is-auto/Games/Map.hpp>
#include <baba-is-auto/Rules/RuleManager.hpp>

#include <algorithm>
#include <array>
#include <string_view>

using namespace baba_is_auto;

namespace
{
void Move(Game& game, std::string_view actions)
{
    constexpr std::string_view keys = "UDLR";
    constexpr std::array directions = { Direction::UP, Direction::DOWN,
                                        Direction::LEFT, Direction::RIGHT };

    for (const char action : actions)
    {
        const auto index = keys.find(action);
        CHECK(index != std::string_view::npos);
        game.MovePlayer(directions[index]);
    }
}

void AddRule(Game& game, ObjectType subject, ObjectType predicate,
             std::size_t y = 3)
{
    game.GetMap().AddObject(0, y, subject);
    game.GetMap().AddObject(1, y, ObjectType::IS);
    game.GetMap().AddObject(2, y, predicate);
}
}  // namespace

TEST_CASE("Game - Basic")
{
    Game game(MAPS_DIR "baba_is_you.txt");
    CHECK(game.GetMap().At(1, 4).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(9, 4).HasType(ObjectType::ICON_FLAG));
    CHECK(game.GetRuleManager().GetNumRules() == 4);
    CHECK(game.GetPlayerIcon() == ObjectType::ICON_BABA);

    auto pos = game.GetMap().GetPositions(game.GetPlayerIcon());
    CHECK(pos.size() == 1);
    CHECK(pos[0].first == 1);
    CHECK(pos[0].second == 4);

    game.MovePlayer(Direction::UP);
    CHECK(game.GetMap().At(1, 3).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(1, 4).HasType(ObjectType::ICON_EMPTY));

    game.MovePlayer(Direction::UP);
    CHECK(game.GetMap().At(1, 3).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(1, 2).HasType(ObjectType::ICON_WALL));

    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::RIGHT);
    CHECK(game.GetMap().At(4, 3).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(3, 3).HasType(ObjectType::ICON_TILE));

    game.MovePlayer(Direction::RIGHT);
    CHECK(game.GetMap().At(5, 3).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(6, 3).HasType(ObjectType::ICON_ROCK));
    CHECK(game.GetMap().At(4, 3).HasType(ObjectType::ICON_TILE));

    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::DOWN);
    CHECK(game.GetPlayState() == PlayState::PLAYING);

    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::RIGHT);
    CHECK(game.GetPlayState() == PlayState::WON);

    game.Reset();
    CHECK(game.GetMap().At(1, 4).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(9, 4).HasType(ObjectType::ICON_FLAG));
    CHECK(game.GetRuleManager().GetNumRules() == 4);
    CHECK(game.GetPlayerIcon() == ObjectType::ICON_BABA);
    CHECK(game.GetPlayState() == PlayState::PLAYING);
}

TEST_CASE("Game - HAS does not grant properties")
{
    Game game(MAPS_DIR "has_property.txt");

    CHECK(game.GetPlayerIcon() == ObjectType::ICON_EMPTY);
    CHECK(game.GetRuleManager().FindPlayer() == ObjectType::ICON_EMPTY);
    CHECK_FALSE(game.GetRuleManager().HasProperty({ ObjectType::ICON_BABA },
                                                  ObjectType::YOU));
}

TEST_CASE("Game - Grass Yard")
{
    Game game(MAPS_DIR "grass_yard.txt");
    CHECK(game.GetMap().GetWidth() == 24);
    CHECK(game.GetMap().GetHeight() == 14);
    CHECK(game.GetRuleManager().GetNumRules() == 2);
    CHECK(game.GetMap().At(9, 7).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(12, 7).HasType(ObjectType::ICON_GRASS));

    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::RIGHT);
    CHECK(game.GetMap().At(11, 7).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(12, 7).HasType(ObjectType::ICON_GRASS));

    game.Reset();
    CHECK(game.GetMap().At(9, 7).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetRuleManager().GetNumRules() == 2);
    CHECK(game.GetPlayState() == PlayState::PLAYING);

    constexpr std::string_view solution =
        "URRRRDRDRRRDRUUUURULLLLLLLULDRDLDRRDDRDDRDLLLDLUUUUUUULLRR"
        "UURULLLULDRRRDDRRRRR";
    Move(game, solution);

    CHECK(game.GetPlayState() == PlayState::WON);
}

TEST_CASE("Game - Arbitrary AND Chains")
{
    Game game(MAPS_DIR "and_chains.txt");
    RuleManager& rules = game.GetRuleManager();

    const Rule babaYou{ Object({ ObjectType::BABA }),
                        Object({ ObjectType::IS }),
                        Object({ ObjectType::YOU }) };
    const auto youRules = rules.GetRules(ObjectType::YOU);
    CHECK(rules.GetNumRules() == 10);
    CHECK(std::find(youRules.begin(), youRules.end(), babaYou) !=
          youRules.end());
    CHECK(rules.HasProperty({ ObjectType::ICON_BABA }, ObjectType::YOU));
    CHECK(rules.HasProperty({ ObjectType::ICON_WALL }, ObjectType::YOU));
    CHECK(rules.HasProperty({ ObjectType::ICON_ROCK }, ObjectType::YOU));
    CHECK(rules.HasProperty({ ObjectType::ICON_BABA }, ObjectType::PUSH));
    CHECK(rules.HasProperty({ ObjectType::ICON_BABA }, ObjectType::SINK));
    CHECK(game.GetMap().At(3, 1).isRule);
    CHECK(game.GetMap().At(10, 9).isRule);
}

TEST_CASE("Game - Rule after leading AND")
{
    Game game(MAPS_DIR "and_chains.txt");
    CHECK(game.GetRuleManager().HasProperty({ ObjectType::ICON_ICE },
                                            ObjectType::STOP));
}

TEST_CASE("Game - Stacked AND and verb")
{
    Game game(MAPS_DIR "and_chains.txt");
    CHECK(game.GetRuleManager().GetRules(ObjectType::JELLY).size() == 1);
}

TEST_CASE("Game - Rule after stacked AND and verb")
{
    Game game(MAPS_DIR "and_chains.txt");
    CHECK(game.GetRuleManager().HasProperty({ ObjectType::ICON_ICE },
                                            ObjectType::PUSH));
}

TEST_CASE("Game - Icy Waters")
{
    SUBCASE("Loads and resets")
    {
        Game game(MAPS_DIR "icy_waters.txt");
        CHECK(game.GetMap().GetWidth() == 28);
        CHECK(game.GetMap().GetHeight() == 16);
        CHECK(game.GetRuleManager().GetNumRules() == 5);
        CHECK(game.GetMap().At(7, 5).HasType(ObjectType::ICON_BABA));
        CHECK(game.GetMap().At(24, 4).HasType(ObjectType::ICON_FLAG));
        CHECK(game.GetRuleManager().HasProperty({ ObjectType::ICON_BABA },
                                                ObjectType::SINK));

        game.MovePlayer(Direction::DOWN);
        game.Reset();
        CHECK(game.GetMap().At(7, 5).HasType(ObjectType::ICON_BABA));
        CHECK(game.GetRuleManager().GetNumRules() == 5);
        CHECK(game.GetPlayState() == PlayState::PLAYING);
    }

    SUBCASE("Wall stops movement")
    {
        Game game(MAPS_DIR "icy_waters.txt");

        Move(game, "UU");
        CHECK(game.GetMap().At(7, 4).HasType(ObjectType::ICON_BABA));
        CHECK(game.GetMap().At(7, 3).HasType(ObjectType::ICON_WALL));
    }

    SUBCASE("Breaks BABA IS YOU AND SINK")
    {
        Game game(MAPS_DIR "icy_waters.txt");

        Move(game, "DDDDRRRRRDD");
        CHECK(game.GetMap().At(12, 11).HasType(ObjectType::ICON_BABA));
        CHECK(game.GetMap().At(12, 12).HasType(ObjectType::SINK));
        CHECK(game.GetRuleManager().HasProperty({ ObjectType::ICON_BABA },
                                                ObjectType::YOU));
        CHECK_FALSE(game.GetRuleManager().HasProperty({ ObjectType::ICON_BABA },
                                                      ObjectType::SINK));
        CHECK_FALSE(game.GetMap().At(11, 11).isRule);
    }

    SUBCASE("Broken SINK rule does not resolve the new overlap")
    {
        Game game(MAPS_DIR "icy_waters.txt");
        game.GetMap().AddObject(12, 11, ObjectType::ICON_ICE);

        Move(game, "DDDDRRRRRDD");
        CHECK(game.GetMap().At(12, 11).HasType(ObjectType::ICON_BABA));
        CHECK(game.GetMap().At(12, 11).HasType(ObjectType::ICON_ICE));
        CHECK(game.GetPlayState() == PlayState::PLAYING);
    }

    SUBCASE("Ice is passable, pushed WALL and JELLY sink, then Baba wins")
    {
        Game game(MAPS_DIR "icy_waters.txt");

        Move(game, "DDDDRRRRRDD");
        Move(game, "UUUURRRRRRRRRRR");
        CHECK(game.GetMap().At(23, 7).HasType(ObjectType::ICON_BABA));
        CHECK(game.GetMap().At(24, 7).HasType(ObjectType::WALL));

        Move(game, "DRU");
        CHECK_FALSE(game.GetMap().At(24, 6).HasType(ObjectType::WALL));
        CHECK_FALSE(game.GetMap().At(24, 6).HasType(ObjectType::ICON_JELLY));
        CHECK(game.GetPlayState() == PlayState::PLAYING);

        Move(game, "UUU");
        CHECK(game.GetPlayState() == PlayState::WON);
    }

    SUBCASE("Moving SINK destroys both objects")
    {
        Game game(MAPS_DIR "icy_waters.txt");
        game.GetMap().AddObject(7, 6, ObjectType::ICON_ICE);

        game.MovePlayer(Direction::DOWN);
        CHECK_FALSE(game.GetMap().At(7, 6).HasType(ObjectType::ICON_BABA));
        CHECK_FALSE(game.GetMap().At(7, 6).HasType(ObjectType::ICON_ICE));
        CHECK(game.GetPlayState() == PlayState::LOST);
    }

    SUBCASE("Destination SINK destroys both objects")
    {
        Game game(MAPS_DIR "icy_waters.txt");

        Move(game, "DDDDRRRRRDD");

        game.GetMap().AddObject(12, 10, ObjectType::ICON_JELLY);
        game.MovePlayer(Direction::UP);
        CHECK_FALSE(game.GetMap().At(12, 10).HasType(ObjectType::ICON_BABA));
        CHECK_FALSE(game.GetMap().At(12, 10).HasType(ObjectType::ICON_JELLY));
        CHECK(game.GetPlayState() == PlayState::LOST);
    }
}

TEST_CASE("Game - Turns")
{
    SUBCASE("Loads and resets")
    {
        Game game(MAPS_DIR "turns.txt");
        CHECK(game.GetMap().GetWidth() == 28);
        CHECK(game.GetMap().GetHeight() == 16);
        CHECK(game.GetRuleManager().GetNumRules() == 6);
        CHECK(game.GetMap().At(23, 8).HasType(ObjectType::ICON_BABA));
        CHECK(game.GetMap().At(2, 6).HasType(ObjectType::ICON_CRAB));
        CHECK(game.GetMap().At(10, 6).HasType(ObjectType::ICON_STAR));
        CHECK(game.GetMap().At(8, 6).HasType(ObjectType::ICON_SKULL));

        game.MovePlayer(Direction::LEFT);
        game.Reset();
        CHECK(game.GetMap().At(23, 8).HasType(ObjectType::ICON_BABA));
        CHECK(game.GetMap().At(2, 6).HasType(ObjectType::ICON_CRAB));
        CHECK(game.GetRuleManager().GetNumRules() == 6);
        CHECK(game.GetPlayState() == PlayState::PLAYING);
    }

    SUBCASE("Chains STAR properties, controls both YOU types, and wins")
    {
        Game game(MAPS_DIR "turns.txt");

        Move(game, "LULLLDDDLLLLULURDRUUULLDRRDRUU");
        CHECK(game.GetRuleManager().HasProperty({ ObjectType::ICON_STAR },
                                                ObjectType::PUSH));
        CHECK(game.GetRuleManager().HasProperty({ ObjectType::ICON_STAR },
                                                ObjectType::SINK));

        Move(game, "LLLLLLLL");
        CHECK(game.GetMap().GetPositions(ObjectType::ICON_STAR).empty());
        CHECK(game.GetMap().GetPositions(ObjectType::ICON_SKULL).empty());

        Move(game, "LLDLURRRRRRRRRRRUULDRDLLLLLLLLLLUULDD");
        CHECK(game.GetRuleManager().HasProperty({ ObjectType::ICON_CRAB },
                                                ObjectType::YOU));
        CHECK(game.GetRuleManager().HasProperty({ ObjectType::ICON_BABA },
                                                ObjectType::YOU));
        CHECK(game.GetMap().At(2, 6).HasType(ObjectType::ICON_CRAB));
        CHECK(game.GetMap().At(6, 6).HasType(ObjectType::ICON_BABA));

        game.MovePlayer(Direction::RIGHT);
        CHECK(game.GetMap().At(3, 6).HasType(ObjectType::ICON_CRAB));
        CHECK(game.GetMap().At(7, 6).HasType(ObjectType::ICON_BABA));
        CHECK_FALSE(game.GetMap().At(2, 6).HasType(ObjectType::ICON_CRAB));
        CHECK_FALSE(game.GetMap().At(6, 6).HasType(ObjectType::ICON_BABA));

        Move(game, "DDDDD");
        CHECK(game.GetMap().At(3, 11).HasType(ObjectType::ICON_CRAB));
        CHECK(game.GetPlayState() == PlayState::WON);
    }
}

TEST_CASE("Game - Editor Smoke Map")
{
    Game game(MAPS_DIR "editor_smoke.txt");
    CHECK(game.GetMap().GetWidth() == 9);
    CHECK(game.GetMap().GetHeight() == 7);
    CHECK(game.GetMap().At(1, 3).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(7, 3).HasType(ObjectType::ICON_FLAG));

    for (int i = 0; i < 6; ++i)
    {
        game.MovePlayer(Direction::RIGHT);
    }

    CHECK(game.GetPlayState() == PlayState::WON);
}

TEST_CASE("Game - Layered Map")
{
    Game game(MAPS_DIR "layered_map.txt");
    CHECK(game.GetMap().At(0, 0).HasType(ObjectType::BABA));
    CHECK(game.GetMap().At(0, 0).HasType(ObjectType::WALL));
    CHECK(game.GetMap().At(0, 1).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(0, 1).HasType(ObjectType::ICON_WALL));
    CHECK(game.GetMap().At(0, 1).HasType(ObjectType::ICON_ROCK));

    game.MovePlayer(Direction::RIGHT);
    CHECK(game.GetMap().At(0, 1).HasType(ObjectType::ICON_ROCK));
    CHECK(game.GetMap().At(1, 1).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(1, 1).HasType(ObjectType::ICON_WALL));
    CHECK(game.GetMap().At(2, 1).HasType(ObjectType::FLAG));
    CHECK(game.GetMap().At(2, 1).HasType(ObjectType::ROCK));
    CHECK(game.GetPlayState() == PlayState::PLAYING);
}

TEST_CASE("Game - Layered Movement")
{
    Game game(MAPS_DIR "layered_movement.txt");

    game.MovePlayer(Direction::RIGHT);
    CHECK(game.GetMap().At(1, 4).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(2, 4).HasType(ObjectType::ICON_ROCK));
    CHECK_FALSE(game.GetMap().At(3, 4).HasType(ObjectType::ICON_ROCK));
    CHECK_FALSE(game.GetMap().At(1, 5).HasType(ObjectType::ICON_BABA));
    CHECK_FALSE(game.GetMap().At(1, 5).HasType(ObjectType::ICON_WATER));
    CHECK(game.GetMap().At(2, 5).HasType(ObjectType::FLAG));
}

TEST_CASE("Game - Stacked Push Edges")
{
    Game game(MAPS_DIR "stacked_push.txt");

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(0, 3).HasType(ObjectType::ICON_BABA));

    game.MovePlayer(Direction::RIGHT);
    CHECK(game.GetMap().At(1, 3).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(2, 3).HasType(ObjectType::ICON_ROCK));
    CHECK(game.GetMap().At(3, 3).HasType(ObjectType::ICON_ROCK));
    CHECK(game.GetMap().At(3, 4).HasType(ObjectType::ICON_BABA));

    Game upward(MAPS_DIR "stacked_push.txt");
    upward.MovePlayer(Direction::UP);
    CHECK(upward.GetMap().At(0, 2).HasType(ObjectType::ICON_BABA));
    CHECK(upward.GetMap().At(3, 3).HasType(ObjectType::ICON_BABA));
}

TEST_CASE("Game - Won")
{
    Game game(MAPS_DIR "off_limits_bug.txt");

    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::DOWN);
    CHECK(game.GetPlayState() == PlayState::WON);
}

TEST_CASE("Game - Lost")
{
    Game game(MAPS_DIR "simple_map.txt");

    CHECK(game.GetMap().At(0, 2).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetRuleManager().GetNumRules() == 1);
    CHECK(game.GetPlayState() == PlayState::PLAYING);

    game.MovePlayer(Direction::UP);
    CHECK(game.GetRuleManager().GetNumRules() == 0);
    CHECK(game.GetPlayState() == PlayState::LOST);
}

TEST_CASE("Game - Sink")
{
    Game game(MAPS_DIR "out_of_reach.txt");
    CHECK(game.GetMap().At(9, 3).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetRuleManager().GetNumRules() == 5);
    CHECK(game.GetPlayerIcon() == ObjectType::ICON_BABA);

    game.MovePlayer(Direction::UP);
    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::DOWN);
    game.MovePlayer(Direction::LEFT);
    game.MovePlayer(Direction::LEFT);
    game.MovePlayer(Direction::UP);
    game.MovePlayer(Direction::LEFT);
    game.MovePlayer(Direction::DOWN);
    game.MovePlayer(Direction::DOWN);
    game.MovePlayer(Direction::DOWN);
    CHECK(game.GetMap().At(10, 5).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(10, 6).HasType(ObjectType::ICON_ROCK));

    game.MovePlayer(Direction::DOWN);
    CHECK(game.GetMap().At(10, 6).HasType(ObjectType::ICON_BABA));
    CHECK_FALSE(game.GetMap().At(10, 7).HasType(ObjectType::ICON_ROCK));
    CHECK_FALSE(game.GetMap().At(10, 7).HasType(ObjectType::ICON_WATER));

    game.MovePlayer(Direction::DOWN);
    CHECK(game.GetMap().At(10, 7).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetPlayState() == PlayState::PLAYING);
}

TEST_CASE("Game - Hot Melt")
{
    SUBCASE("Blocked overlap")
    {
        Game game(MAPS_DIR "volcano.txt");
        game.GetMap().AddObject(14, 1, ObjectType::ICON_LAVA);
        game.GetMap().AddObject(13, 1, ObjectType::ICON_WALL);

        game.MovePlayer(Direction::LEFT);
        CHECK_FALSE(game.GetMap().At(14, 1).HasType(ObjectType::ICON_BABA));
        CHECK(game.GetMap().At(14, 1).HasType(ObjectType::ICON_LAVA));
        CHECK(game.GetPlayState() == PlayState::LOST);
    }

    SUBCASE("Player")
    {
        Game game(MAPS_DIR "volcano.txt");
        game.GetMap().AddObject(15, 1, ObjectType::ICON_LAVA);

        game.MovePlayer(Direction::RIGHT);
        CHECK_FALSE(game.GetMap().At(15, 1).HasType(ObjectType::ICON_BABA));
        CHECK(game.GetMap().At(15, 1).HasType(ObjectType::ICON_LAVA));
        CHECK(game.GetPlayState() == PlayState::LOST);
    }

    SUBCASE("Pushed object")
    {
        Game game(MAPS_DIR "volcano.txt");
        game.GetMap().AddObject(13, 1, ObjectType::ICON_ROCK);
        game.GetMap().AddObject(12, 1, ObjectType::ICON_LAVA);
        game.GetMap().RemoveObject(25, 14, ObjectType::FLAG);
        game.GetMap().AddObject(25, 14, ObjectType::ROCK);
        game.GetMap().RemoveObject(27, 14, ObjectType::WIN);
        game.GetMap().AddObject(27, 14, ObjectType::MELT);

        game.MovePlayer(Direction::LEFT);
        CHECK(game.GetMap().At(13, 1).HasType(ObjectType::ICON_BABA));
        CHECK_FALSE(game.GetMap().At(12, 1).HasType(ObjectType::ICON_ROCK));
        CHECK(game.GetMap().At(12, 1).HasType(ObjectType::ICON_LAVA));
    }
}

TEST_CASE("Game - Empty Sink")
{
    Game game(MAPS_DIR "empty_sink.txt");

    game.MovePlayer(Direction::RIGHT);
    CHECK(game.GetMap().At(1, 2).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetPlayState() == PlayState::PLAYING);
}

TEST_CASE("Game - Newly active Sink")
{
    Game game(MAPS_DIR "sink_rule_timing.txt");

    game.MovePlayer(Direction::UP);
    CHECK_FALSE(game.GetMap().At(4, 2).HasType(ObjectType::ICON_ROCK));
    CHECK_FALSE(game.GetMap().At(4, 2).HasType(ObjectType::ICON_ICE));
}

TEST_CASE("Game - SINKed Rule Text Is Reparsed")
{
    Game game(MAPS_DIR "sink_rule_reparse.txt");

    game.MovePlayer(Direction::UP);
    CHECK_FALSE(game.GetMap().At(0, 0).HasType(ObjectType::FLAG));
    CHECK_FALSE(game.GetMap().At(0, 0).HasType(ObjectType::ICON_JELLY));
    CHECK_FALSE(game.GetRuleManager().HasProperty({ ObjectType::ICON_FLAG },
                                                  ObjectType::WIN));
    CHECK(game.GetRuleManager().GetNumRules() == 2);
}

TEST_CASE("Game - Defeat")
{
    Game game(MAPS_DIR "off_limits_bug.txt");
    CHECK(game.GetMap().At(12, 1).HasType(ObjectType::ICON_SKULL));

    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::RIGHT);
    CHECK(game.GetMap().At(12, 1).HasType(ObjectType::ICON_SKULL));
}

TEST_CASE("Game - Defeat ignores pushed objects")
{
    Game game(MAPS_DIR "defeat_push.txt");

    game.MovePlayer(Direction::RIGHT);
    CHECK(game.GetMap().At(2, 2).HasType(ObjectType::ICON_ROCK));
    CHECK(game.GetMap().At(2, 2).HasType(ObjectType::ICON_ICE));
}

TEST_CASE("Map - Basic")
{
    Map map(5, 5);

    map.AddObject(4, 3, ObjectType::BABA);
    CHECK(map.At(3, 3).HasType(ObjectType::ICON_EMPTY));
    CHECK(map.At(4, 3).HasType(ObjectType::BABA));
}

TEST_CASE("Map - Per-object facing")
{
    Map map(2, 1);
    CHECK_THROWS_AS(map.AddObject(0, 0, ObjectType::ICON_KEKE, Direction::NONE),
                    std::invalid_argument);

    map.AddObject(0, 0, ObjectType::ICON_KEKE, Direction::UP);
    map.AddObject(0, 0, ObjectType::ICON_KEKE, Direction::DOWN);

    const auto& instances = map.At(0, 0).GetInstances();
    CHECK(instances.size() == 2);
    CHECK(instances[0].id != instances[1].id);
    CHECK(instances[0].direction == Direction::UP);
    CHECK(instances[1].direction == Direction::DOWN);
    CHECK_FALSE(map.SetDirection(instances[0].id, Direction::NONE));
    CHECK(map.GetDirection(instances[0].id) == Direction::UP);

    map.AddObject(1, 0, ObjectType::ICON_LOVE);
    const auto& legacy = map.At(1, 0).GetInstances();
    CHECK(legacy.size() == 1);
    CHECK(legacy[0].direction == Direction::RIGHT);

    const ObjectID keke = instances[0].id;
    CHECK_FALSE(map.MoveObject(keke, 2, 0));
    CHECK_FALSE(map.MoveObject(keke, 0, 1));
    CHECK(map.GetPosition(keke) == Position{ 0, 0 });
    CHECK(map.GetDirection(keke) == Direction::UP);

    CHECK(map.MoveObject(keke, 1, 0));
    CHECK(map.GetPosition(keke) == Position{ 1, 0 });
    CHECK(map.GetDirection(keke) == Direction::UP);
}

TEST_CASE("Map - ID zero is never addressable")
{
    Map map(2, 1);

    CHECK_FALSE(map.GetPosition(0));
    CHECK(map.GetInstance(0) == nullptr);
    CHECK_FALSE(map.GetDirection(0));
    CHECK_FALSE(map.SetDirection(0, Direction::RIGHT));
    CHECK_FALSE(map.MoveObject(0, 1, 0));
    CHECK_FALSE(map.RemoveObject(0));
}

TEST_CASE("Map - Direction section")
{
    Map map;
    map.Load(MAPS_DIR "directions.txt");

    const auto& instances = map.At(0, 0).GetInstances();
    CHECK(instances.size() == 2);
    CHECK(instances[0].direction == Direction::UP);
    CHECK(instances[1].direction == Direction::LEFT);

    map.Reset();
    const auto& reset = map.At(0, 0).GetInstances();
    CHECK(reset.size() == 2);
    CHECK(reset[0].direction == Direction::UP);
    CHECK(reset[1].direction == Direction::LEFT);
}

TEST_CASE("Map - Invalid direction section")
{
    Map map;
    CHECK_THROWS_AS(map.Load(MAPS_DIR "invalid_directions.txt"),
                    std::runtime_error);
}

TEST_CASE("Enums - Directional LOCKED preserves map values")
{
    CHECK(static_cast<int>(ObjectType::BABA) == 4);
    CHECK(static_cast<int>(ObjectType::MOVE) == 83);
    CHECK(static_cast<int>(ObjectType::ICON_BABA) == 114);
    CHECK(static_cast<int>(ObjectType::ICON_WATER) == 175);

    CHECK(IsTextType(ObjectType::LOCKED_UP));
    CHECK(IsTextType(ObjectType::LOCKED_DOWN));
    CHECK(IsPropertyType(ObjectType::LOCKED_LEFT));
    CHECK(IsPropertyType(ObjectType::LOCKED_RIGHT));
    CHECK(ConvertIconToText(ObjectType::LOCKED_UP) == ObjectType::LOCKED_UP);
    CHECK(ConvertTextToIcon(ObjectType::LOCKED_RIGHT) ==
          ObjectType::LOCKED_RIGHT);
}

TEST_CASE("Game - Conditional MOVE rules")
{
    Game game(MAPS_DIR "move_conditions.txt");

    const auto rules = game.GetRuleManager().GetRules(ObjectType::MOVE);
    CHECK(rules.size() == 6);
    CHECK(game.GetPlayerIcon() == ObjectType::ICON_EMPTY);
    CHECK(game.GetRuleManager().FindPlayer() == ObjectType::ICON_EMPTY);
    CHECK_FALSE(game.GetRuleManager().HasProperty({ ObjectType::ICON_KEKE },
                                                  ObjectType::MOVE));

    const std::array expectedOps = { ObjectType::LONELY, ObjectType::ON,
                                     ObjectType::NEAR, ObjectType::FACING };

    for (std::size_t i = 0; i < expectedOps.size(); ++i)
    {
        CHECK(rules[i].conditions.size() == 1);
        CHECK(rules[i].conditions[0].op == expectedOps[i]);
        CHECK_FALSE(rules[i].conditions[0].negated);
    }

    CHECK(rules[4].conditions.size() == 2);
    CHECK(rules[4].conditions[0].op == ObjectType::ON);
    CHECK(rules[4].conditions[0].negated);
    CHECK(rules[4].conditions[0].targets == std::vector{ ObjectType::LOVE });
    CHECK(rules[4].conditions[1].op == ObjectType::NEAR);
    CHECK_FALSE(rules[4].conditions[1].negated);

    CHECK(rules[5].conditions.size() == 1);
    CHECK(rules[5].conditions[0].op == ObjectType::LONELY);
    CHECK(rules[5].conditions[0].negated);
}

TEST_CASE("Game - MOVE special noun conditions")
{
    Game game(MAPS_DIR "move_special_conditions.txt");

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(2, 5).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(9, 5).HasType(ObjectType::ICON_LOVE));
    CHECK(game.GetMap().At(3, 8).HasType(ObjectType::ICON_KEKE));
}

TEST_CASE("Game - MOVE ALL conditions exclude special nouns")
{
    Game game(MAPS_DIR "move_all_condition.txt");

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(1, 2).HasType(ObjectType::ICON_EMPTY));

    const auto& instances = game.GetMap().At(2, 2).GetInstances();
    CHECK(
        std::count_if(instances.begin(), instances.end(), [](const auto& obj) {
            return obj.type == ObjectType::ICON_BABA;
        }) == 2);
}

TEST_CASE("Game - Conditional PUSH observes directional lock")
{
    Game game(MAPS_DIR "conditional_push_scope.txt");

    game.MovePlayer(Direction::RIGHT);
    CHECK(game.GetMap().At(1, 3).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(2, 3).HasType(ObjectType::ICON_ROCK));
}

TEST_CASE("Game - YOU cannot move through a directional lock")
{
    Game game(MAPS_DIR "locked_you.txt");
    const auto baba = game.GetMap().At(1, 2).GetInstances().front().id;

    game.MovePlayer(Direction::RIGHT);
    CHECK(game.GetMap().GetPosition(baba) == Position{ 1, 2 });
    CHECK(game.GetMap().GetDirection(baba) == Direction::RIGHT);
}

TEST_CASE("Game - PUSH overrides STOP on the same instance")
{
    Game game(MAPS_DIR "conditional_push_stop.txt");

    game.MovePlayer(Direction::RIGHT);
    CHECK(game.GetMap().At(2, 3).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(3, 3).HasType(ObjectType::ICON_ROCK));
}

TEST_CASE("Game - Conditional YOU applies per instance")
{
    Game game(MAPS_DIR "conditional_you.txt");

    game.MovePlayer(Direction::RIGHT);
    CHECK(game.GetMap().At(2, 1).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(1, 1).HasType(ObjectType::ICON_ROCK));
}

TEST_CASE("Game - Conditional transformations apply per instance")
{
    Game game(MAPS_DIR "conditional_transformation.txt");

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(1, 1).HasType(ObjectType::ICON_KEKE));
    CHECK(game.GetMap().At(1, 1).HasType(ObjectType::ICON_LOVE));
    CHECK_FALSE(game.GetMap().At(1, 1).HasType(ObjectType::ICON_BABA));
}

TEST_CASE("Game - Conditional transformations use a phase snapshot")
{
    Game game(MAPS_DIR "conditional_transformation_snapshot.txt");

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(1, 2).HasType(ObjectType::ICON_KEKE));
    CHECK(game.GetMap().At(1, 2).HasType(ObjectType::ICON_ROCK));
    CHECK_FALSE(game.GetMap().At(1, 2).HasType(ObjectType::ICON_BABA));
    CHECK_FALSE(game.GetMap().At(1, 2).HasType(ObjectType::ICON_LOVE));
}

TEST_CASE("Game - Conditional EMPTY transformations apply per position")
{
    Game game(MAPS_DIR "conditional_empty_transformation.txt");

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(2, 1).HasType(ObjectType::ICON_KEKE));
    CHECK(game.GetMap().At(3, 1).HasType(ObjectType::ICON_KEKE));
    CHECK(game.GetMap().At(4, 1).HasType(ObjectType::ICON_KEKE));
    CHECK(game.GetMap().At(2, 2).HasType(ObjectType::ICON_KEKE));
    CHECK(game.GetMap().At(4, 2).HasType(ObjectType::ICON_KEKE));
    CHECK(game.GetMap().At(6, 2).HasType(ObjectType::ICON_EMPTY));
}

TEST_CASE("Game - Directionless EMPTY does not satisfy FACING")
{
    Game game(MAPS_DIR "conditional_empty_facing.txt");

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(2, 3).HasType(ObjectType::ICON_EMPTY));
    CHECK_FALSE(game.GetMap().At(2, 3).HasType(ObjectType::ICON_KEKE));
    CHECK_FALSE(game.GetMap().At(2, 3).HasType(ObjectType::ICON_LOVE));
}

TEST_CASE("Game - Later direction rules set EMPTY FACING direction")
{
    Game game(MAPS_DIR "conditional_empty_assigned_facing.txt");

    game.MovePlayer(Direction::NONE);

    const auto& instances = game.GetMap().At(2, 3).GetInstances();
    CHECK(instances.size() == 1);
    CHECK(instances.front().type == ObjectType::ICON_KEKE);
    CHECK(instances.front().direction == Direction::UP);
}

TEST_CASE("Game - Conditional overlap effects apply per instance")
{
    Game game(MAPS_DIR "conditional_sink.txt");

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(1, 1).HasType(ObjectType::ICON_EMPTY));
}

TEST_CASE("Game - SINK conditions use one effect-phase snapshot")
{
    Game game(MAPS_DIR "conditional_sink_snapshot.txt");

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(3, 2).HasType(ObjectType::ICON_EMPTY));
    CHECK(game.GetMap().At(4, 2).HasType(ObjectType::ICON_EMPTY));
}

TEST_CASE("Game - HOT and MELT conditions use one effect-phase snapshot")
{
    Game game(MAPS_DIR "conditional_hot_melt_snapshot.txt");

    game.MovePlayer(Direction::NONE);
    CHECK_FALSE(game.GetMap().At(3, 3).HasType(ObjectType::ICON_BABA));
    CHECK_FALSE(game.GetMap().At(4, 3).HasType(ObjectType::ICON_LOVE));
    CHECK(game.GetMap().At(3, 3).HasType(ObjectType::ICON_WATER));
    CHECK(game.GetMap().At(4, 3).HasType(ObjectType::ICON_WATER));
}

TEST_CASE("Game - DEFEAT conditions use one effect-phase snapshot")
{
    Game game(MAPS_DIR "conditional_defeat_snapshot.txt");

    game.MovePlayer(Direction::NONE);
    CHECK_FALSE(game.GetMap().At(3, 4).HasType(ObjectType::ICON_BABA));
    CHECK_FALSE(game.GetMap().At(4, 4).HasType(ObjectType::ICON_KEKE));
    CHECK(game.GetMap().At(3, 4).HasType(ObjectType::ICON_SKULL));
    CHECK(game.GetMap().At(4, 4).HasType(ObjectType::ICON_WATER));
}

TEST_CASE("Game - MOVE uses phase snapshots and stacked counts")
{
    Game game(MAPS_DIR "move_conditions.txt");
    game.GetMap().AddObject(0, 5, ObjectType::ICON_LOVE);

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(2, 5).HasType(ObjectType::ICON_KEKE));
    CHECK(game.GetMap().At(1, 8).HasType(ObjectType::ICON_KEKE));
    CHECK(game.GetMap().At(4, 11).HasType(ObjectType::ICON_KEKE));
}

TEST_CASE("Game - MOVE wait, bounce, WEAK, and directional locks")
{
    Game game(MAPS_DIR "move_rules.txt");

    const auto rock = game.GetMap().At(5, 11).GetInstances().front().id;
    CHECK(game.GetMap().SetDirection(rock, Direction::LEFT));

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(11, 8).HasType(ObjectType::ICON_KEKE));
    CHECK(game.GetMap().At(5, 8).HasType(ObjectType::ICON_KEKE));
    CHECK(game.GetMap().At(5, 8).GetInstances().front().direction ==
          Direction::LEFT);
    CHECK(game.GetMap().At(6, 8).HasType(ObjectType::ICON_ROBOT));
    CHECK(game.GetMap().At(5, 9).HasType(ObjectType::ICON_LOVE));
    CHECK(game.GetMap().At(5, 9).GetInstances().front().direction ==
          Direction::LEFT);
    CHECK(game.GetMap().GetPositions(ObjectType::ICON_ALGAE).empty());
    CHECK(game.GetMap().At(6, 11).HasType(ObjectType::ICON_ROCK));
    CHECK(game.GetMap().GetDirection(rock) == Direction::RIGHT);
}

TEST_CASE("Game - MOVE cannot start in a locked direction")
{
    Game game(MAPS_DIR "move_rules.txt");
    const auto rock = game.GetMap().At(5, 11).GetInstances().front().id;

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().GetPosition(rock) == Position{ 5, 11 });
    CHECK(game.GetMap().GetDirection(rock) == Direction::RIGHT);
}

TEST_CASE("Game - MOVE ignores invalid NONE facing")
{
    Game game(MAPS_DIR "move_rules.txt");
    const auto keke = game.GetMap().At(10, 8).GetInstances().front().id;

    game.GetMap().AddObject(10, 8, ObjectType::BABA);

    auto* instance = game.GetMap().GetInstance(keke);
    CHECK(instance != nullptr);

    instance->direction = Direction::NONE;

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().GetPosition(keke) == Position{ 10, 8 });
    CHECK(game.GetMap().GetDirection(keke) == Direction::NONE);
}

TEST_CASE("Game - MOVE assigns IDs to writable map insertions")
{
    Game game(MAPS_DIR "move_rules.txt");

    game.GetMap().At(1, 12).Add(ObjectType::ICON_KEKE);
    CHECK(game.GetMap().At(1, 12).GetInstances().front().id == 0);

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(1, 12).HasType(ObjectType::ICON_EMPTY));
    CHECK(game.GetMap().At(2, 12).HasType(ObjectType::ICON_KEKE));
    CHECK(game.GetMap().At(2, 12).GetInstances().front().id != 0);
}

TEST_CASE("Game - MOVE resolves each stack in rounds")
{
    Game game(MAPS_DIR "move_order.txt");

    const auto keke = game.GetMap().At(2, 4).GetInstances().front().id;
    CHECK(game.GetMap().SetDirection(keke, Direction::UP));

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(3, 4).HasType(ObjectType::ICON_LOVE));
    CHECK(game.GetMap().At(4, 4).HasType(ObjectType::ICON_KEKE));
    CHECK(game.GetMap().GetDirection(keke) == Direction::RIGHT);
}

TEST_CASE("Game - MOVE follows column-major object priority")
{
    Game game(MAPS_DIR "move_priority.txt");

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(2, 3).HasType(ObjectType::ICON_ROCK));
    CHECK(game.GetMap().At(1, 4).HasType(ObjectType::ICON_EMPTY));
    CHECK(std::count_if(game.GetMap().At(1, 3).GetInstances().begin(),
                        game.GetMap().At(1, 3).GetInstances().end(),
                        [](const ObjectInstance& instance) {
                            return instance.type == ObjectType::ICON_KEKE;
                        }) == 2);
}

TEST_CASE("Game - MOVE keeps priority after new objects are added")
{
    Game game(MAPS_DIR "move_priority.txt");

    Map& map = game.GetMap();
    map.RemoveObject(1, 2, ObjectType::ICON_KEKE);
    map.RemoveObject(0, 3, ObjectType::ICON_KEKE);
    map.RemoveObject(1, 3, ObjectType::ICON_ROCK);
    map.AddObject(2, 3, ObjectType::ICON_KEKE, Direction::LEFT);
    map.AddObject(1, 3, ObjectType::ICON_ROCK);
    map.AddObject(0, 3, ObjectType::ICON_KEKE, Direction::RIGHT);

    game.MovePlayer(Direction::NONE);
    CHECK(map.At(0, 3).HasType(ObjectType::ICON_ROCK));
    CHECK(map.At(2, 3).HasType(ObjectType::ICON_EMPTY));
}

TEST_CASE("Game - MOVE follows rule priority before object priority")
{
    Game game(MAPS_DIR "move_rule_priority.txt");

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(1, 4).HasType(ObjectType::ICON_ROCK));
    CHECK(game.GetMap().At(2, 3).HasType(ObjectType::ICON_EMPTY));
    CHECK(game.GetMap().At(2, 4).HasType(ObjectType::ICON_KEKE));
    CHECK(game.GetMap().At(2, 4).HasType(ObjectType::ICON_LOVE));
}

TEST_CASE("Game - direction properties use their stacked counts")
{
    Game game(MAPS_DIR "special_transformations.txt");

    Map& map = game.GetMap();
    map.AddObject(0, 1, ObjectType::KEKE);
    map.AddObject(1, 1, ObjectType::IS);
    map.AddObject(2, 1, ObjectType::UP);
    map.AddObject(3, 1, ObjectType::AND);
    map.AddObject(4, 1, ObjectType::UP);
    map.AddObject(5, 1, ObjectType::AND);
    map.AddObject(6, 1, ObjectType::LEFT);

    AddRule(game, ObjectType::KEKE, ObjectType::MOVE, 2);

    map.AddObject(7, 3, ObjectType::ICON_KEKE, Direction::RIGHT);

    game.MovePlayer(Direction::NONE);
    CHECK(map.At(7, 2).HasType(ObjectType::ICON_KEKE));
    CHECK(map.At(7, 2).GetInstances().front().direction == Direction::UP);
}

TEST_CASE("Game - tied direction properties choose clockwise from facing")
{
    Game game(MAPS_DIR "special_transformations.txt");
    Map& map = game.GetMap();

    map.AddObject(0, 1, ObjectType::KEKE);
    map.AddObject(1, 1, ObjectType::IS);
    map.AddObject(2, 1, ObjectType::UP);
    map.AddObject(3, 1, ObjectType::AND);
    map.AddObject(4, 1, ObjectType::LEFT);
    AddRule(game, ObjectType::KEKE, ObjectType::MOVE, 2);
    map.AddObject(7, 3, ObjectType::ICON_KEKE, Direction::LEFT);

    game.MovePlayer(Direction::NONE);
    CHECK(map.At(7, 2).HasType(ObjectType::ICON_KEKE));
    CHECK(map.At(7, 2).GetInstances().front().direction == Direction::UP);
}

TEST_CASE("Game - stacked EMPTY MOVE advances each step")
{
    Game game(MAPS_DIR "special_transformations.txt");
    Map& map = game.GetMap();

    AddRule(game, ObjectType::EMPTY, ObjectType::UP, 1);
    map.AddObject(0, 2, ObjectType::EMPTY);
    map.AddObject(1, 2, ObjectType::IS);
    map.AddObject(2, 2, ObjectType::MOVE);
    map.AddObject(3, 2, ObjectType::AND);
    map.AddObject(4, 2, ObjectType::MOVE);
    map.AddObject(7, 0, ObjectType::ICON_WALL);
    map.AddObject(7, 1, ObjectType::ICON_WALL);
    map.AddObject(7, 2, ObjectType::BABA);

    game.MovePlayer(Direction::NONE);
    CHECK(map.At(7, 0).HasType(ObjectType::BABA));
    CHECK(map.At(7, 1).HasType(ObjectType::ICON_WALL));
    CHECK(map.At(7, 2).HasType(ObjectType::ICON_EMPTY));
}

TEST_CASE("Game - EMPTY MOVE turns before moving away from an obstacle")
{
    Game game(MAPS_DIR "special_transformations.txt");
    Map& map = game.GetMap();

    AddRule(game, ObjectType::EMPTY, ObjectType::UP, 1);
    map.AddObject(0, 2, ObjectType::EMPTY);
    map.AddObject(1, 2, ObjectType::NEAR);
    map.AddObject(2, 2, ObjectType::LOVE);
    map.AddObject(3, 2, ObjectType::IS);
    map.AddObject(4, 2, ObjectType::MOVE);
    map.AddObject(6, 0, ObjectType::ICON_LOVE);
    map.AddObject(7, 1, ObjectType::BABA);

    game.MovePlayer(Direction::NONE);
    CHECK(map.At(7, 1).HasType(ObjectType::BABA));
    CHECK(map.At(7, 2).HasType(ObjectType::ICON_EMPTY));
}

TEST_CASE("Game - EMPTY MOVE cannot start in a locked direction")
{
    Game game(MAPS_DIR "special_transformations.txt");
    Map& map = game.GetMap();

    AddRule(game, ObjectType::EMPTY, ObjectType::UP, 1);
    AddRule(game, ObjectType::EMPTY, ObjectType::LOCKED_UP, 2);
    map.AddObject(0, 3, ObjectType::EMPTY);
    map.AddObject(1, 3, ObjectType::NEAR);
    map.AddObject(2, 3, ObjectType::LOVE);
    map.AddObject(3, 3, ObjectType::IS);
    map.AddObject(4, 3, ObjectType::MOVE);
    map.AddObject(6, 2, ObjectType::ICON_LOVE);
    map.AddObject(7, 1, ObjectType::BABA);

    game.MovePlayer(Direction::NONE);
    CHECK(map.At(7, 1).HasType(ObjectType::BABA));
    CHECK(map.At(7, 0).HasType(ObjectType::ICON_EMPTY));
}

TEST_CASE("Game - EMPTY MOVE recalculates positions between steps")
{
    Game game(MAPS_DIR "empty_move_recalculation.txt");

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(9, 0).HasType(ObjectType::BABA));
    CHECK(game.GetMap().At(9, 1).HasType(ObjectType::ICON_EMPTY));
}

TEST_CASE("Game - EMPTY properties are evaluated at their position")
{
    Game game(MAPS_DIR "special_transformations.txt");
    Map& map = game.GetMap();

    AddRule(game, ObjectType::EMPTY, ObjectType::STOP, 1);
    AddRule(game, ObjectType::BABA, ObjectType::YOU, 2);
    map.AddObject(7, 3, ObjectType::ICON_BABA);

    game.MovePlayer(Direction::NONE);
    game.MovePlayer(Direction::UP);
    CHECK(map.At(7, 3).HasType(ObjectType::ICON_BABA));
    CHECK(map.At(7, 2).HasType(ObjectType::ICON_EMPTY));
}

TEST_CASE("Game - Transformations use one snapshot")
{
    Game game(MAPS_DIR "transformations.txt");

    const auto keke = game.GetMap().At(0, 9).GetInstances().front().id;
    CHECK(game.GetMap().SetDirection(keke, Direction::UP));

    game.MovePlayer(Direction::NONE);

    for (const std::size_t x : { 0u, 1u })
    {
        CHECK(game.GetMap().At(x, 9).HasType(ObjectType::ICON_LOVE));
        CHECK(game.GetMap().At(x, 9).HasType(ObjectType::ICON_ROCK));
        CHECK(std::count_if(game.GetMap().At(x, 9).GetInstances().begin(),
                            game.GetMap().At(x, 9).GetInstances().end(),
                            [](const ObjectInstance& instance) {
                                return instance.type == ObjectType::ICON_LOVE;
                            }) == 2);
    }

    const auto* transformed = game.GetMap().GetInstance(keke);
    CHECK(transformed != nullptr);
    CHECK(transformed->type == ObjectType::ICON_LOVE);
    CHECK(transformed->direction == Direction::UP);

    const auto rock = std::find_if(
        game.GetMap().At(0, 9).GetInstances().begin(),
        game.GetMap().At(0, 9).GetInstances().end(), [](const auto& instance) {
            return instance.type == ObjectType::ICON_ROCK;
        });
    CHECK(rock != game.GetMap().At(0, 9).GetInstances().end());
    CHECK(rock->direction == Direction::UP);

    CHECK(game.GetMap().At(3, 9).HasType(ObjectType::ICON_ALGAE));
    CHECK_FALSE(game.GetMap().At(3, 9).HasType(ObjectType::ICON_LOVE));
    CHECK(game.GetMap().At(5, 9).HasType(ObjectType::ICON_WALL));
    CHECK(game.GetMap().At(6, 9).HasType(ObjectType::ICON_ROCK));
    CHECK(game.GetMap().At(8, 9).HasType(ObjectType::ICON_BIRD));
    CHECK(game.GetMap().At(9, 9).HasType(ObjectType::ICON_BOG));
    CHECK(game.GetMap().At(10, 9).HasType(ObjectType::ICON_BAT));
    CHECK(game.GetMap().At(12, 9).HasType(ObjectType::ICON_EMPTY));
    CHECK(game.GetMap().At(19, 10).HasType(ObjectType::ICON_FLOWER));
}

TEST_CASE("Game - Special noun TEXT subject transforms every text")
{
    Game game(MAPS_DIR "special_transformations.txt");
    AddRule(game, ObjectType::TEXT, ObjectType::ROCK);

    game.MovePlayer(Direction::NONE);

    for (const std::size_t x : { 0u, 1u, 2u })
    {
        CHECK(game.GetMap().At(x, 3).HasType(ObjectType::ICON_ROCK));
        CHECK_FALSE(game.GetMap().At(x, 3).HasTextType());
    }
}

TEST_CASE("Game - Special noun TEXT predicate writes the source noun")
{
    Game game(MAPS_DIR "special_transformations.txt");
    AddRule(game, ObjectType::BABA, ObjectType::TEXT);

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(0, 0).HasType(ObjectType::BABA));
    CHECK_FALSE(game.GetMap().At(0, 0).HasType(ObjectType::ICON_BABA));
    CHECK_FALSE(game.GetMap().At(0, 0).HasType(ObjectType::ICON_TEXT));
}

TEST_CASE("Game - Unsupported GROUP predicate does not create an icon")
{
    Game game(MAPS_DIR "special_transformations.txt");
    AddRule(game, ObjectType::BABA, ObjectType::GROUP);

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(0, 0).HasType(ObjectType::ICON_BABA));
    CHECK_FALSE(game.GetMap().At(0, 0).HasType(ObjectType::ICON_GROUP));
}

TEST_CASE("Game - Special noun ALL subject transforms every member")
{
    Game game(MAPS_DIR "special_transformations.txt");
    AddRule(game, ObjectType::ALL, ObjectType::LOVE);
    AddRule(game, ObjectType::LOVE, ObjectType::ROCK, 2);

    game.MovePlayer(Direction::NONE);

    for (const std::size_t x : { 0u, 2u, 4u })
    {
        CHECK(game.GetMap().At(x, 0).HasType(ObjectType::ICON_LOVE));
        CHECK(game.GetMap().At(x, 0).GetInstances().size() == 1);
    }
}

TEST_CASE("Game - Special noun ALL predicate expands without a self guard")
{
    Game game(MAPS_DIR "special_transformations.txt");
    AddRule(game, ObjectType::BABA, ObjectType::ALL);

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(0, 0).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(0, 0).HasType(ObjectType::ICON_ROCK));
    CHECK(game.GetMap().At(0, 0).HasType(ObjectType::ICON_LOVE));
    CHECK(game.GetMap().At(0, 0).GetInstances().size() == 3);
}

TEST_CASE("Game - Special noun ALL does not duplicate types on its tile")
{
    Game game(MAPS_DIR "special_transformations.txt");
    game.GetMap().AddObject(0, 0, ObjectType::ICON_ROCK);
    AddRule(game, ObjectType::BABA, ObjectType::ALL);

    game.MovePlayer(Direction::NONE);
    CHECK(std::count_if(game.GetMap().At(0, 0).GetInstances().begin(),
                        game.GetMap().At(0, 0).GetInstances().end(),
                        [](const ObjectInstance& instance) {
                            return instance.type == ObjectType::ICON_ROCK;
                        }) == 1);
    CHECK(game.GetMap().At(0, 0).GetInstances().size() == 3);
}

TEST_CASE("Game - Identity rule blocks ALL expansion")
{
    Game game(MAPS_DIR "special_transformations.txt");
    AddRule(game, ObjectType::BABA, ObjectType::ALL, 1);
    AddRule(game, ObjectType::BABA, ObjectType::BABA, 2);

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(0, 0).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(0, 0).GetInstances().size() == 1);
}

TEST_CASE("Game - EMPTY identity rule blocks ALL expansion")
{
    Game game(MAPS_DIR "special_transformations.txt");
    AddRule(game, ObjectType::EMPTY, ObjectType::ALL, 1);
    AddRule(game, ObjectType::EMPTY, ObjectType::EMPTY, 2);

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(7, 0).HasType(ObjectType::ICON_EMPTY));
}

TEST_CASE("Game - Duplicate EMPTY IS ALL creates each type once")
{
    Game game(MAPS_DIR "special_transformations.txt");
    AddRule(game, ObjectType::EMPTY, ObjectType::ALL, 1);
    AddRule(game, ObjectType::EMPTY, ObjectType::ALL, 2);

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(7, 0).GetInstances().size() == 3);
}

TEST_CASE("Game - directionless EMPTY transformation is not fixed to RIGHT")
{
    Game game(MAPS_DIR "special_transformations.txt");
    game.SetRandomSeed(0);
    AddRule(game, ObjectType::EMPTY, ObjectType::KEKE);

    game.MovePlayer(Direction::NONE);

    const auto positions = game.GetMap().GetPositions(ObjectType::ICON_KEKE);
    CHECK(positions.size() > 10);
    CHECK(
        std::any_of(positions.begin(), positions.end(), [&game](Position pos) {
            return game.GetMap()
                       .At(pos.first, pos.second)
                       .GetInstances()
                       .front()
                       .direction != Direction::RIGHT;
        }));
}

TEST_CASE("Game - random seed repeats EMPTY transformation directions")
{
    Game first(MAPS_DIR "special_transformations.txt");
    Game second(MAPS_DIR "special_transformations.txt");
    AddRule(first, ObjectType::EMPTY, ObjectType::KEKE);
    AddRule(second, ObjectType::EMPTY, ObjectType::KEKE);

    first.SetRandomSeed(12345);
    second.SetRandomSeed(12345);
    first.MovePlayer(Direction::NONE);
    second.MovePlayer(Direction::NONE);

    const auto Directions = [](const Game& game) {
        std::vector<Direction> result;

        for (std::size_t y = 0; y < game.GetMap().GetHeight(); ++y)
        {
            for (std::size_t x = 0; x < game.GetMap().GetWidth(); ++x)
            {
                for (const ObjectInstance& instance :
                     game.GetMap().At(x, y).GetInstances())
                {
                    if (instance.type == ObjectType::ICON_KEKE)
                    {
                        result.emplace_back(instance.direction);
                    }
                }
            }
        }

        return result;
    };

    const auto firstDirections = Directions(first);
    CHECK_FALSE(firstDirections.empty());
    CHECK(firstDirections == Directions(second));
}

TEST_CASE("Game - EMPTY IS ALL gives every object one valid direction")
{
    Game game(MAPS_DIR "special_transformations.txt");
    game.SetRandomSeed(12345);

    AddRule(game, ObjectType::EMPTY, ObjectType::ALL);
    game.MovePlayer(Direction::NONE);

    const auto& instances = game.GetMap().At(7, 3).GetInstances();
    CHECK(instances.size() == 3);

    const Direction direction = instances.front().direction;
    CHECK(direction != Direction::NONE);
    CHECK(std::all_of(instances.begin(), instances.end(),
                      [direction](const ObjectInstance& instance) {
                          return instance.direction == direction;
                      }));
}

TEST_CASE("Game - Special noun ALL to ALL expands every member")
{
    Game game(MAPS_DIR "special_transformations.txt");
    AddRule(game, ObjectType::ALL, ObjectType::ALL);

    game.MovePlayer(Direction::NONE);

    for (const std::size_t x : { 0u, 2u, 4u })
    {
        CHECK(game.GetMap().At(x, 0).HasType(ObjectType::ICON_BABA));
        CHECK(game.GetMap().At(x, 0).HasType(ObjectType::ICON_ROCK));
        CHECK(game.GetMap().At(x, 0).HasType(ObjectType::ICON_LOVE));
        CHECK(game.GetMap().At(x, 0).GetInstances().size() == 3);
    }
}

TEST_CASE("Game - Special noun ALL membership persists after removal")
{
    Game game(MAPS_DIR "special_transformations.txt");
    game.GetMap().RemoveObject(2, 0, ObjectType::ICON_ROCK);
    AddRule(game, ObjectType::BABA, ObjectType::ALL);

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(0, 0).HasType(ObjectType::ICON_ROCK));
}

TEST_CASE("Game - Transformation timing precedes overlap effects")
{
    Game game(MAPS_DIR "transformation_timing.txt");

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(2, 4).HasType(ObjectType::ICON_WATER));
    CHECK_FALSE(game.GetMap().At(5, 4).HasType(ObjectType::ICON_KEKE));
    CHECK_FALSE(game.GetMap().At(5, 4).HasType(ObjectType::ICON_LOVE));
    CHECK_FALSE(game.GetMap().At(5, 4).HasType(ObjectType::ICON_WATER));
}

TEST_CASE("Game - STOP WEAK allows entry and is destroyed")
{
    Game game(MAPS_DIR "baba_is_you.txt");

    Map& map = game.GetMap();
    map.RemoveObject(1, 4, ObjectType::ICON_BABA);
    map.AddObject(1, 1, ObjectType::ICON_BABA);
    map.AddObject(2, 1, ObjectType::ICON_WALL);

    AddRule(game, ObjectType::WALL, ObjectType::WEAK, 7);

    game.MovePlayer(Direction::NONE);
    game.MovePlayer(Direction::RIGHT);
    CHECK(map.At(2, 1).HasType(ObjectType::ICON_BABA));
    CHECK_FALSE(map.At(2, 1).HasType(ObjectType::ICON_WALL));
}

TEST_CASE("Game - blocked PUSH WEAK is destroyed")
{
    Game game(MAPS_DIR "baba_is_you.txt");

    Map& map = game.GetMap();
    map.RemoveObject(1, 4, ObjectType::ICON_BABA);
    map.AddObject(1, 1, ObjectType::ICON_BABA);
    map.AddObject(2, 1, ObjectType::ICON_ROCK);
    map.AddObject(3, 1, ObjectType::ICON_WALL);

    AddRule(game, ObjectType::ROCK, ObjectType::WEAK, 7);

    game.MovePlayer(Direction::NONE);
    game.MovePlayer(Direction::RIGHT);
    CHECK(map.At(2, 1).HasType(ObjectType::ICON_BABA));
    CHECK_FALSE(map.At(2, 1).HasType(ObjectType::ICON_ROCK));
    CHECK(map.At(3, 1).HasType(ObjectType::ICON_WALL));
}

TEST_CASE("Game - locked PUSH WEAK allows entry without pushing ahead")
{
    Game game(MAPS_DIR "baba_is_you.txt");

    Map& map = game.GetMap();
    map.RemoveObject(1, 4, ObjectType::ICON_BABA);
    map.AddObject(1, 1, ObjectType::ICON_BABA);
    map.AddObject(2, 1, ObjectType::ICON_ROCK);
    map.AddObject(3, 1, ObjectType::ICON_LOVE);

    map.AddObject(6, 1, ObjectType::LOVE);
    map.AddObject(7, 1, ObjectType::IS);
    map.AddObject(8, 1, ObjectType::PUSH);
    map.AddObject(0, 7, ObjectType::ROCK);
    map.AddObject(1, 7, ObjectType::IS);
    map.AddObject(2, 7, ObjectType::WEAK);
    map.AddObject(3, 7, ObjectType::AND);
    map.AddObject(4, 7, ObjectType::LOCKED_RIGHT);

    game.MovePlayer(Direction::NONE);
    game.MovePlayer(Direction::RIGHT);
    CHECK(map.At(2, 1).HasType(ObjectType::ICON_BABA));
    CHECK_FALSE(map.At(2, 1).HasType(ObjectType::ICON_ROCK));
    CHECK(map.At(3, 1).HasType(ObjectType::ICON_LOVE));
    CHECK(map.At(4, 1).HasType(ObjectType::ICON_EMPTY));
}

TEST_CASE("Game - blocked YOU WEAK is destroyed")
{
    Game game(MAPS_DIR "baba_is_you.txt");

    Map& map = game.GetMap();
    map.RemoveObject(1, 4, ObjectType::ICON_BABA);
    map.AddObject(0, 1, ObjectType::ICON_BABA);

    AddRule(game, ObjectType::BABA, ObjectType::WEAK, 7);

    game.MovePlayer(Direction::NONE);
    game.MovePlayer(Direction::LEFT);
    CHECK(map.GetPositions(ObjectType::ICON_BABA).empty());
}

TEST_CASE("Game - Affection layout and idle movement")
{
    Game game(MAPS_DIR "affection.txt");
    CHECK(game.GetMap().GetWidth() == 24);
    CHECK(game.GetMap().GetHeight() == 14);
    CHECK(game.GetRuleManager().GetNumRules() == 5);
    CHECK(game.GetMap().At(3, 9).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(16, 7).HasType(ObjectType::ICON_LOVE));
    CHECK(game.GetMap().At(14, 5).HasType(ObjectType::ICON_ALGAE));
    CHECK(game.GetMap().At(14, 4).HasType(ObjectType::ICON_TILE));

    const auto right = game.GetMap().At(6, 3).GetInstances().front().id;
    const auto up = game.GetMap().At(7, 7).GetInstances().front().id;
    const auto down = game.GetMap().At(12, 10).GetInstances().front().id;
    CHECK(game.GetMap().GetDirection(right) == Direction::RIGHT);
    CHECK(game.GetMap().GetDirection(up) == Direction::UP);
    CHECK(game.GetMap().GetDirection(down) == Direction::DOWN);

    game.MovePlayer(Direction::NONE);
    CHECK(game.GetMap().At(7, 3).HasType(ObjectType::ICON_KEKE));
    CHECK(game.GetMap().At(7, 6).HasType(ObjectType::ICON_KEKE));
    CHECK(game.GetMap().At(12, 11).HasType(ObjectType::ICON_KEKE));

    game.Reset();
    CHECK(game.GetMap().GetDirection(up) == Direction::UP);
    CHECK(game.GetMap().GetDirection(down) == Direction::DOWN);
}

TEST_CASE("Game - Affection MOVE solution")
{
    Game game(MAPS_DIR "affection.txt");

    Move(game, "RRRRRUUUUURRRDDDD");

    game.MovePlayer(Direction::NONE);
    game.MovePlayer(Direction::NONE);

    Move(game, "DUUUUURRRRRRRRDDD");
    CHECK(game.GetPlayState() == PlayState::WON);
}

TEST_CASE("Game - Affection transformation solution")
{
    Game game(MAPS_DIR "affection.txt");

    Move(game, "URRRRRRRRDDUULDDUULLDRRDRUUUURUURU");
    CHECK(game.GetPlayState() == PlayState::WON);
}

TEST_CASE("Map - Boundary Duplicate Stack")
{
    Map map(3, 3);

    map.AddObject(0, 1, ObjectType::ICON_BABA);
    map.AddObject(0, 1, ObjectType::ICON_BABA);
    CHECK(map.At(0, 1).GetTypes().size() == 2);
    CHECK(map.GetPositions(ObjectType::ICON_BABA).size() == 1);
}

TEST_CASE("Object - Duplicate Stack")
{
    const Object empty(std::vector<ObjectType>{});
    Object object;

    object.Add(ObjectType::ICON_BABA);
    object.Add(ObjectType::ICON_BABA);
    CHECK(empty.HasType(ObjectType::ICON_EMPTY));
    CHECK(object.GetTypes().size() == 2);

    object.Remove(ObjectType::ICON_BABA);
    CHECK(object.HasType(ObjectType::ICON_BABA));

    object.Remove(ObjectType::ICON_BABA);
    CHECK(object.HasType(ObjectType::ICON_EMPTY));
}

TEST_CASE("RuleManager - Basic")
{
    RuleManager ruleManager;

    const Rule rule1{ Object(std::vector{ ObjectType::BABA }),
                      Object(std::vector{ ObjectType::IS }),
                      Object(std::vector{ ObjectType::YOU }) };
    const Rule rule2{ Object(std::vector{ ObjectType::KEKE }),
                      Object(std::vector{ ObjectType::IS }),
                      Object(std::vector{ ObjectType::STOP }) };
    CHECK(ruleManager.FindPlayer() == ObjectType::ICON_EMPTY);

    ruleManager.AddRule(rule1);
    CHECK(ruleManager.GetNumRules() == 1);
    CHECK(ruleManager.FindPlayer() == ObjectType::ICON_BABA);
    CHECK(ruleManager.HasProperty({ ObjectType::BABA }, ObjectType::YOU));

    ruleManager.AddRule(rule2);
    CHECK(ruleManager.GetNumRules() == 2);

    ruleManager.RemoveRule(rule2);
    CHECK(ruleManager.GetNumRules() == 1);
}

TEST_CASE("RuleCondition - Equality compares every field")
{
    const RuleCondition condition{ ObjectType::ON, { ObjectType::LOVE }, true };
    CHECK(condition ==
          RuleCondition{ ObjectType::ON, { ObjectType::LOVE }, true });
    CHECK_FALSE(condition ==
                RuleCondition{ ObjectType::NEAR, { ObjectType::LOVE }, true });
    CHECK_FALSE(condition ==
                RuleCondition{ ObjectType::ON, { ObjectType::ROCK }, true });
    CHECK_FALSE(condition ==
                RuleCondition{ ObjectType::ON, { ObjectType::LOVE }, false });
}

TEST_CASE("Map - Icon Vanishing")
{
    Game game(MAPS_DIR "off_limits_bug.txt");
    CHECK(game.GetMap().At(14, 1).HasType(ObjectType::ICON_WALL));

    game.MovePlayer(Direction::RIGHT);
    CHECK(game.GetMap().At(14, 1).HasType(ObjectType::ICON_WALL));
}

TEST_CASE("Map - Icon Spread")
{
    Game game(MAPS_DIR "off_limits_bug.txt");

    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::LEFT);
    game.MovePlayer(Direction::LEFT);
    CHECK(game.GetMap().At(21, 3).HasType(ObjectType::ICON_WALL));
    CHECK(game.GetMap().At(21, 4).HasType(ObjectType::ICON_WALL));
    CHECK(game.GetMap().At(21, 5).HasType(ObjectType::ICON_WALL));
    CHECK(game.GetMap().At(22, 3).HasType(ObjectType::ICON_EMPTY));
    CHECK(game.GetMap().At(22, 4).HasType(ObjectType::ICON_EMPTY));
    CHECK(game.GetMap().At(22, 5).HasType(ObjectType::ICON_FLOWER));
    CHECK(game.GetMap().At(23, 3).HasType(ObjectType::ICON_EMPTY));
    CHECK(game.GetMap().At(23, 4).HasType(ObjectType::ICON_EMPTY));
    CHECK(game.GetMap().At(23, 5).HasType(ObjectType::ICON_EMPTY));
}

TEST_CASE("Preprocess - Basic")
{
    Game game(MAPS_DIR "baba_is_you.txt");

    const std::vector<float> tensor = Preprocess::StateToTensor(game);
    CHECK_EQ(tensor.size(), Preprocess::TENSOR_DIM * game.GetMap().GetWidth() *
                                game.GetMap().GetHeight());

    const auto ToIndex = [](std::size_t x, std::size_t y, std::size_t c) {
        return (c * 11 * 9) + (y * 11) + x;
    };

    CHECK_EQ(tensor[ToIndex(0, 0, 0)], 1.0f);
    CHECK_EQ(tensor[ToIndex(1, 0, 1)], 1.0f);
    CHECK_EQ(tensor[ToIndex(2, 0, 2)], 1.0f);

    CHECK_EQ(tensor[ToIndex(0, 0, 14)], 1.0f);
    CHECK_EQ(tensor[ToIndex(1, 0, 14)], 1.0f);
    CHECK_EQ(tensor[ToIndex(2, 0, 14)], 1.0f);
    CHECK_EQ(tensor[ToIndex(3, 0, 14)], 0.0f);

    CHECK_EQ(tensor[ToIndex(0, 0, 15)], 1.0f);
    CHECK_EQ(tensor[ToIndex(1, 0, 15)], 1.0f);
    CHECK_EQ(tensor[ToIndex(2, 0, 15)], 1.0f);
    CHECK_EQ(tensor[ToIndex(3, 0, 15)], 0.0f);
}

TEST_CASE("RandomAgent - Basic")
{
    const Game game(MAPS_DIR "baba_is_you.txt");
    RandomAgent agent;

    std::vector actions = { Direction::UP, Direction::DOWN, Direction::LEFT,
                            Direction::RIGHT, Direction::NONE };
    const Direction action = agent.GetAction(game);
    CHECK_NE(std::find(begin(actions), end(actions), action), end(actions));
}
