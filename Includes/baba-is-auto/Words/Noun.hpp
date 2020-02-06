#ifndef BABA_IS_AUTO_NOUN_HPP
#define BABA_IS_AUTO_NOUN_HPP

#include <baba-is-auto/Enums/WordEnums.hpp>

namespace baba_is_auto
{
//!
//! \brief Noun struct.
//!
//! This struct represents noun. A noun is a word that corresponds to any
//! possible in-game sprite. A few nouns like STAR) have multiple corresponding
//! sprites, while VIOLET and FLOWER words have the same corresponding sprites.
//! A noun can be used as a NOUN IS VERB statement e.g. BABA IS YOU to give it a
//! property or as a NOUN IS NOUN statement e.g. WALL IS WATER to turn an object
//! into another object.
//!
struct Noun
{
    //! Default constructor.
    Noun() = default;

    //! Constructs a noun.
    //! \param _type The noun type.
    Noun(NounType _type) : type(_type)
    {
        // Do nothing
    }

    //! Operator overloading for ==.
    bool operator==(const Noun& rhs) const
    {
        return type == rhs.type;
    }

    NounType type;
};
}  // namespace baba_is_auto

#endif