// Copyright (c) 2020 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

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
    void AddRule(const Rule& rule);

    //! Removes a rule.
    //! \param rule The rule.
    void RemoveRule(const Rule& rule);

    //! Clears a list of rules.
    void ClearRules();

    //! Gets a list of rules that has specific type.
    //! \param type The object type to find a rule.
    //! \return A list of rules that has specific type.
    std::vector<Rule> GetRules(ObjectType type) const;

    //! Gets the number of rules.
    //! \return The number of rules.
    std::size_t GetNumRules() const;

    //! Gets the object type for player.
    //! \return The object type for player.
    ObjectType FindPlayer() const;

    //! Checks an object has specific property.
    //! \param types A list of object types to check it has property.
    //! \param property The property to check.
    //! \return The flag indicates that an object has specific property.
    bool HasProperty(const std::vector<ObjectType>& types, ObjectType property);

 private:
    std::vector<Rule> m_rules;
};
}  // namespace baba_is_auto

#endif