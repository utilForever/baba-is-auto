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
    constexpr std::string_view actions = "UDLR";
    constexpr std::array directions = { Direction::UP, Direction::DOWN,
                                        Direction::LEFT, Direction::RIGHT };

    for (const char action : solution)
    {
        const auto index = actions.find(action);
        REQUIRE(index != std::string_view::npos);
        game.MovePlayer(directions[index]);
    }

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
    const auto Move = [](Game& game, std::string_view actions) {
        for (const char action : actions)
        {
            if (action == 'U')
            {
                game.MovePlayer(Direction::UP);
            }
            else if (action == 'D')
            {
                game.MovePlayer(Direction::DOWN);
            }
            else if (action == 'L')
            {
                game.MovePlayer(Direction::LEFT);
            }
            else if (action == 'R')
            {
                game.MovePlayer(Direction::RIGHT);
            }
        }
    };

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
