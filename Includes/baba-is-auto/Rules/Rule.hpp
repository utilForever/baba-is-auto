// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#ifndef BABA_IS_AUTO_RULE_HPP
#define BABA_IS_AUTO_RULE_HPP

#include <baba-is-auto/Games/Object.hpp>

#include <tuple>
#include <vector>

namespace baba_is_auto
{
struct RuleCondition
{
    ObjectType op = ObjectType::ON;
    std::vector<ObjectType> targets;
    bool negated = false;

    bool operator==(const RuleCondition& rhs) const;
};

//!
//! \brief Rule class.
//!
//! This class represents rule. The game centers around the manipulation of
//! "rules"--represented by tiles with words written on them--in order to allow
//! the titular character Baba (or some other object) to reach a specified goal.
//!
class Rule
{
 public:
    //! Constructs a rule.
    //! \param obj1 A first object.
    //! \param obj2 A second object.
    //! \param obj3 A third object.
    //! \param ruleConditions Optional conditions restricting subject instances.
    //! \param isNegated Whether the predicate is negated.
    Rule(Object obj1, Object obj2, Object obj3,
         std::vector<RuleCondition> ruleConditions = {},
         bool isNegated = false);

    //! Operator overloading for ==.
    //! \param rhs A right side of Rule object.
    //! \return The value that indicates two rules are equal.
    bool operator==(const Rule& rhs) const;

    //! The subject, operator, and predicate objects.
    std::tuple<Object, Object, Object> objects;

    //! Optional conditions restricting which subject instances match.
    std::vector<RuleCondition> conditions;

    //! Whether the predicate is negated.
    bool negated = false;
};
}  // namespace baba_is_auto

#endif
