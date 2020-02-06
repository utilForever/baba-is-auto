#ifndef BABA_IS_AUTO_RULE_HPP
#define BABA_IS_AUTO_RULE_HPP

#include <baba-is-auto/Enums/RuleEnums.hpp>
#include <baba-is-auto/Words/Noun.hpp>
#include <baba-is-auto/Words/Operator.hpp>
#include <baba-is-auto/Words/Property.hpp>

namespace baba_is_auto
{
//!
//! \brief Rule struct.
//!
//! This class represents rule. The game centers around the manipulation of
//! "rules"--represented by tiles with words written on them--in order to allow
//! the titular character Baba (or some other object) to reach a specified goal.
//!
struct Rule
{
    RuleType type;

    Noun noun1;
    Noun noun2;
    Operator op;
    Property property;
};
}  // namespace baba_is_auto

#endif