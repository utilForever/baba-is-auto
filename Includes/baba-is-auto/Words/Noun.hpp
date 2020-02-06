#ifndef BABA_IS_AUTO_NOUN_HPP
#define BABA_IS_AUTO_NOUN_HPP

#include <baba-is-auto/Enums/WordEnums.hpp>

namespace baba_is_auto
{
//!
//! \brief Noun class.
//!
//! This class represents noun. A noun is a word that corresponds to any
//! possible in-game sprite. A few nouns like STAR) have multiple corresponding
//! sprites, while VIOLET and FLOWER words have the same corresponding sprites.
//! A noun can be used as a NOUN IS VERB statement e.g. BABA IS YOU to give it a
//! property or as a NOUN IS NOUN statement e.g. WALL IS WATER to turn an object
//! into another object.
//!
class Noun
{
 public:
    //! Default constructor.
    Noun() = default;

    //! Constructs a noun.
    //! \param _type The noun type.
    explicit Noun(NounType _type);

    //! Operator overloading for ==.
    //! \param rhs A right side of Rule object.
    //! \return The value that indicates two objects are equal.
    bool operator==(const Noun& rhs) const;

    NounType type;
};
}  // namespace baba_is_auto

#endif