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

    CHECK(game.GetMap().At(4, 1).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(4, 9).HasType(ObjectType::ICON_FLAG));

    CHECK(game.GetPlayerIcon() == ObjectType::ICON_EMPTY);

    game.Initialize();

    CHECK(game.GetRuleManager().GetNumRules() == 4);

    CHECK(game.GetPlayerIcon() == ObjectType::ICON_BABA);

    auto pos = game.GetMap().GetPositions(game.GetPlayerIcon());
    CHECK(pos.size() == 1);
    CHECK(pos[0].first == 4);
    CHECK(pos[0].second == 1);

    game.MovePlayer(Direction::UP);
    CHECK(game.GetMap().At(3, 1).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(4, 1).HasType(ObjectType::ICON_EMPTY));

    game.MovePlayer(Direction::UP);
    CHECK(game.GetMap().At(3, 1).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(2, 1).HasType(ObjectType::ICON_WALL));

    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::RIGHT);
    game.MovePlayer(Direction::RIGHT);
    CHECK(game.GetMap().At(3, 4).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(3, 3).HasType(ObjectType::ICON_EMPTY));

    game.MovePlayer(Direction::RIGHT);
    CHECK(game.GetMap().At(3, 5).HasType(ObjectType::ICON_BABA));
    CHECK(game.GetMap().At(3, 6).HasType(ObjectType::ICON_ROCK));
    CHECK(game.GetMap().At(3, 4).HasType(ObjectType::ICON_EMPTY));
}

TEST_CASE("Map - Basic")
{
    Map map(5, 5);

    map.AddObject(3, 4, ObjectType::BABA);
    CHECK(map.At(3, 3).HasType(ObjectType::ICON_EMPTY));
    CHECK(map.At(3, 4).HasType(ObjectType::BABA));
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