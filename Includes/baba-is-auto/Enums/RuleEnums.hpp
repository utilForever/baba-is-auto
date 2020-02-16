#ifndef BABA_IS_AUTO_RULE_ENUMS_HPP
#define BABA_IS_AUTO_RULE_ENUMS_HPP

namespace baba_is_auto
{
//! \brief An enumerator for identifying the rule.
enum class RuleType
{
    NOUN_VERB_PROPERTY,
    NOUN_VERB_NOUN,
};

//! \brief An enumerator for identifying the direction of rule.
enum class RuleDirection
{
    HORIZONTAL,
    VERTICAL,
};
}  // namespace baba_is_auto

#endif