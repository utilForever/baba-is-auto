// Copyright (c) 2020 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#undef NEAR

#include <baba-is-auto/Agents/Preprocess.hpp>
#include <baba-is-auto/Agents/RandomAgent.hpp>
#include <baba-is-auto/Games/Game.hpp>
#include <baba-is-auto/Games/Map.hpp>
#include <baba-is-auto/Rules/RuleManager.hpp>

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

    game.MovePlayer(Direction::DOWN);
    CHECK(game.GetPlayState() == PlayState::LOST);
}

TEST_CASE("Map - Basic")
{
    Map map(5, 5);

    map.AddObject(4, 3, ObjectType::BABA);
    CHECK(map.At(3, 3).HasType(ObjectType::ICON_EMPTY));
    CHECK(map.At(4, 3).HasType(ObjectType::BABA));
}

TEST_CASE("RuleManager - Basic")
{
    RuleManager ruleManager;

    const Rule rule1{ Object(std::vector<ObjectType>{ ObjectType::BABA }),
                      Object(std::vector<ObjectType>{ ObjectType::IS }),
                      Object(std::vector<ObjectType>{ ObjectType::YOU }) };
    const Rule rule2{ Object(std::vector<ObjectType>{ ObjectType::KEKE }),
                      Object(std::vector<ObjectType>{ ObjectType::IS }),
                      Object(std::vector<ObjectType>{ ObjectType::STOP }) };

    ruleManager.AddRule(rule1);
    CHECK(ruleManager.GetNumRules() == 1);

    ruleManager.AddRule(rule2);
    CHECK(ruleManager.GetNumRules() == 2);

    ruleManager.RemoveRule(rule2);
    CHECK(ruleManager.GetNumRules() == 1);
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

    std::vector<Direction> actions = { Direction::UP, Direction::DOWN,
                                       Direction::LEFT, Direction::RIGHT,
                                       Direction::NONE };
    const Direction action = agent.GetAction(game);

    CHECK_NE(std::find(begin(actions), end(actions), action), end(actions));
}