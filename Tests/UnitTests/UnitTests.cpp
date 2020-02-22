#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#undef NEAR

#include <baba-is-auto/Games/Game.hpp>
#include <baba-is-auto/Games/Map.hpp>
#include <baba-is-auto/Rules/RuleManager.hpp>

using namespace baba_is_auto;

TEST_CASE("Game - Basic")
{
    Game game(MAPS_DIR "BabaIsYou.txt");

    CHECK_NOTHROW(game.GetMap().Show());

    CHECK(game.GetMap().At(4, 1).GetType() == ObjectType::ICON_BABA);
    CHECK(game.GetMap().At(4, 9).GetType() == ObjectType::ICON_FLAG);

    CHECK(game.GetPlayerIcon() == ObjectType::ICON_EMPTY);

    game.Initialize();

    CHECK(game.GetRuleManager().GetNumRules() == 4);

    CHECK(game.GetPlayerIcon() == ObjectType::ICON_BABA);

    auto pos = game.GetMap().GetPositions(game.GetPlayerIcon());
    CHECK(pos.size() == 1);
    CHECK(pos[0].first == 4);
    CHECK(pos[0].second == 1);

    game.MovePlayer(Direction::UP);
    CHECK(game.GetMap().At(3, 1).GetType() == ObjectType::ICON_BABA);
    CHECK(game.GetMap().At(4, 1).GetType() == ObjectType::ICON_EMPTY);

    game.MovePlayer(Direction::UP);
    CHECK(game.GetMap().At(3, 1).GetType() == ObjectType::ICON_BABA);
    CHECK(game.GetMap().At(2, 1).GetType() == ObjectType::ICON_WALL);

    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::RIGHT);
    CHECK(game.GetMap().At(3, 4).GetType() == ObjectType::ICON_BABA);
    CHECK(game.GetMap().At(3, 3).GetType() == ObjectType::ICON_EMPTY);

    game.MovePlayer(Direction::RIGHT);
    CHECK(game.GetMap().At(3, 5).GetType() == ObjectType::ICON_BABA);
    CHECK(game.GetMap().At(3, 6).GetType() == ObjectType::ICON_ROCK);
    CHECK(game.GetMap().At(3, 4).GetType() == ObjectType::ICON_EMPTY);
}

TEST_CASE("Map - Basic")
{
    Map map(5, 5);

    map.Assign(3, 4, ObjectType::BABA);
    CHECK(map.At(3, 3).GetType() == ObjectType::EMPTY);
    CHECK(map.At(3, 4).GetType() == ObjectType::BABA);
}

TEST_CASE("RuleManager - Basic")
{
    RuleManager ruleManager;

    const Rule rule1{ Object(ObjectType::BABA), Object(ObjectType::IS),
                      Object(ObjectType::YOU) };
    const Rule rule2{ Object(ObjectType::KEKE), Object(ObjectType::IS),
                      Object(ObjectType::STOP) };

    ruleManager.Add(rule1);
    CHECK(ruleManager.GetNumRules() == 1);

    ruleManager.Add(rule2);
    CHECK(ruleManager.GetNumRules() == 2);

    ruleManager.Remove(rule2);
    CHECK(ruleManager.GetNumRules() == 1);
}