#ifndef BABA_IS_AUTO_RULE_HPP
#define BABA_IS_AUTO_RULE_HPP

#include <baba-is-auto/Enums/RuleEnums.hpp>
#include <baba-is-auto/Words/Noun.hpp>
#include <baba-is-auto/Words/Operator.hpp>
#include <baba-is-auto/Words/Property.hpp>

namespace baba_is_auto
{
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
    //! \param _noun A noun word.
    //! \param _op An operator word.
    //! \param _property A property word.
    Rule(Noun _noun, Operator _op, Property _property);

    //! Constructs a rule.
    //! \param _noun1 A first noun word.
    //! \param _op An operator word.
    //! \param _noun2 A second noun word.
    Rule(Noun _noun1, Operator _op, Noun _noun2);

    //! Operator overloading for ==.
    //! \param rhs A right side of Rule object.
    //! \return The value that indicates two objects are equal.
    bool operator==(const Rule& rhs) const;

    RuleType type{};

    Noun noun1{};
    Noun noun2{};
    Operator op{};
    Property property{};
};
}  // namespace baba_is_auto

#endif