#ifndef BABA_IS_AUTO_OPERATOR_HPP
#define BABA_IS_AUTO_OPERATOR_HPP

#include <baba-is-auto/Enums/WordEnums.hpp>

namespace baba_is_auto
{
//!
//! \brief Operator struct.
//!
//! This struct represents operator. An operator is a word that goes in between
//! properties and nouns to show the relation between them.
//!
struct Operator
{
    //! Default constructor.
    Operator() = default;

    //! Constructs a operator.
    //! \param _type The operator type.
    Operator(OpType _type) : type(_type)
    {
        // Do nothing
    }

    //! Operator overloading for ==.
    bool operator==(const Operator& rhs) const
    {
        return type == rhs.type;
    }

    OpType type;
};
}  // namespace baba_is_auto

#endif