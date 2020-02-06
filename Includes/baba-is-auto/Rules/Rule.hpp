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
    //! Constructs a rule.
    //! \param _noun A noun word.
    //! \param _op An operator word.
    //! \param _property A property word.
    Rule(Noun _noun, Operator _op, Property _property)
        : type(RuleType::NOUN_VERB_PROPERTY),
          noun1(_noun),
          op(_op),
          property(_property)
    {
        // Do nothing
    }

    //! Constructs a rule.
    //! \param _noun1 A first noun word.
    //! \param _op An operator word.
    //! \param _noun2 A second noun word.
    Rule(Noun _noun1, Operator _op, Noun _noun2)
        : type(RuleType::NOUN_VERB_NOUN), noun1(_noun1), noun2(_noun2), op(_op)
    {
        // Do nothing
    }

    //! Operator overloading for ==.
    bool operator==(const Rule& rhs) const
    {
        if (type == RuleType::NOUN_VERB_PROPERTY)
        {
            return noun1 == rhs.noun1 && op == rhs.op &&
                   property == rhs.property;
        }
        else if (type == RuleType::NOUN_VERB_NOUN)
        {
            return noun1 == rhs.noun1 && op == rhs.op && noun2 == rhs.noun2;
        }

        return false;
    }

    RuleType type{};

    Noun noun1{};
    Noun noun2{};
    Operator op{};
    Property property{};
};
}  // namespace baba_is_auto

#endif