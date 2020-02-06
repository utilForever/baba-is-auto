#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <baba-is-auto/Rules/RuleManager.hpp>

using namespace baba_is_auto;

TEST_CASE("RuleManager - Basic")
{
    RuleManager ruleManager;

    const Rule rule1{ Noun(NounType::BABA), Operator(OpType::IS),
                      Property(PropertyType::YOU) };
    const Rule rule2{ Noun(NounType::KEKE), Operator(OpType::IS),
                      Property(PropertyType::STOP) };

    ruleManager.Add(rule1);
    CHECK(ruleManager.GetNumRules() == 1);

    ruleManager.Add(rule2);
    CHECK(ruleManager.GetNumRules() == 2);

    ruleManager.Remove(rule2);
    CHECK(ruleManager.GetNumRules() == 1);
}