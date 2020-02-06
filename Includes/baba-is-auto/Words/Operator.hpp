#ifndef BABA_IS_AUTO_OPERATOR_HPP
#define BABA_IS_AUTO_OPERATOR_HPP

#include <baba-is-auto/Enums/WordEnums.hpp>

namespace baba_is_auto
{
//!
//! \brief Operator class.
//!
//! This class represents operator. An operator is a word that goes in between
//! properties and nouns to show the relation between them.
//!
class Operator
{
 public:
    //! Default constructor.
    Operator() = default;

    //! Constructs a operator.
    //! \param _type The operator type.
    Operator(OpType _type);

    //! Operator overloading for ==.
    //! \param rhs A right side of Rule object.
    //! \return The value that indicates two objects are equal.
    bool operator==(const Operator& rhs) const;

    OpType type;
};
}  // namespace baba_is_auto

#endif