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

    //! Checks an object has specific property.
    //! \param object The object to check it has property.
    //! \param property The property to check.
    //! \return The flag indicates that an object has specific property.
    bool HasProperty(ObjectType object, ObjectType property);

 private:
    std::vector<Rule> m_rules;
};
}  // namespace baba_is_auto

#endif