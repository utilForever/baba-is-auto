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
    //! \param noun A noun word.
    //! \param op An operator word.
    //! \param property A property word.
    void Add(Noun noun, Operator op, Property property);

    //! Adds a rule.
    //! \param noun1 A first noun word.
    //! \param op An operator word.
    //! \param noun1 A second noun word.
    void Add(Noun noun1, Operator op, Noun noun2);

    //! Removes a rule.
    //! \param noun A noun word.
    //! \param op An operator word.
    //! \param property A property word.
    void Remove(Noun noun, Operator op, Property property);

    //! Removes a rule.
    //! \param noun1 A first noun word.
    //! \param op An operator word.
    //! \param noun1 A second noun word.
    void Remove(Noun noun1, Operator op, Noun noun2);

    //! Gets the number of rules.
    //! \return The number of rules.
    std::size_t GetNumRules() const;

 private:
    std::vector<Rule> m_rules;
};
}  // namespace baba_is_auto

#endif