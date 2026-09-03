// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <baba-is-auto/Rules/Rule.hpp>

#include <utility>

namespace baba_is_auto
{
bool RuleCondition::operator==(const RuleCondition& rhs) const
{
    return op == rhs.op && targets == rhs.targets && negated == rhs.negated;
}

Rule::Rule(Object obj1, Object obj2, Object obj3,
           std::vector<RuleCondition> ruleConditions, bool isNegated)
    : conditions(std::move(ruleConditions)), negated(isNegated)
{
    objects = { std::move(obj1), std::move(obj2), std::move(obj3) };
}

bool Rule::operator==(const Rule& rhs) const
{
    return objects == rhs.objects && conditions == rhs.conditions &&
           negated == rhs.negated;
}
}  // namespace baba_is_auto
