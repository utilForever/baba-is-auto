#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <baba-is-auto/Rules/RuleManager.hpp>

using namespace baba_is_auto;

TEST_CASE("RuleManager - Add")
{
    RuleManager ruleManager;

    Noun baba{};
    baba.type = NounType::BABA;

    Operator is{};
    is.type = OpType::IS;

    Property you{};
    you.type = PropertyType::YOU;

    ruleManager.Add(baba, is, you);
}