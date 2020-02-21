#ifndef BABA_IS_AUTO_RULE_MANAGER_HPP
#define BABA_IS_AUTO_RULE_MANAGER_HPP

#include <baba-is-auto/Rules/Rule.hpp>

#include <vector>

namespace baba_is_auto
{
//!
//! \brief RuleManager class.
//!
//! This class manages a list of rules.
//!
class RuleManager
{
 public:
    //! Adds a rule.
    //! \param rule The rule.
    void Add(Rule rule);

    //! Removes a rule.
    //! \param rule The rule.
    void Remove(Rule rule);

    //! Gets the number of rules.
    //! \return The number of rules.
    std::size_t GetNumRules() const;

    //! Gets the object type for player.
    //! \return The object type for player.
    ObjectType FindPlayer() const;

 private:
    std::vector<Rule> m_rules;
};
}  // namespace baba_is_auto

#endif