// Copyright (c) 2020 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#ifndef BABA_IS_AUTO_NOUN_HPP
#define BABA_IS_AUTO_NOUN_HPP

#include <baba-is-auto/Enums/GameEnums.hpp>

#include <vector>

namespace baba_is_auto
{
//!
//! \brief Noun class.
//!
//! This class represents object such as noun, operator and property.
//! A noun is a word that corresponds to any possible in-game sprite. A few
//! nouns like STAR) have multiple corresponding sprites, while VIOLET and
//! FLOWER words have the same corresponding sprites. A noun can be used as a
//! NOUN IS VERB statement e.g. BABA IS YOU to give it a property or as a NOUN
//! IS NOUN statement e.g. WALL IS WATER to turn an object into another object.
//! An operator is a word that goes in between properties and nouns to show the
//! relation between them.
//! A property is something that can be attached to noun words to alter their
//! behavior.
//!
class Object
{
 public:
    //! Default constructor.
    Object() = default;

    //! Constructs an object.
    //! \param types A list of object types.
    explicit Object(std::vector<ObjectType> types);

    //! Operator overloading for ==.
    //! \param rhs A right side of Object object.
    //! \return The value that indicates two objects are equal.
    bool operator==(const Object& rhs) const;

    //! Adds an object type.
    //! \param type An object type to add.
    void Add(ObjectType type);

    //! Removes an object type.
    //! \param type An object type to remove.
    void Remove(ObjectType type);

    //! Gets a list of object types.
    //! \return A list of object types.
    std::vector<ObjectType> GetTypes() const;

    //! Checks the object has specific type.
    //! \param type An object type to check.
    //! \return The flag indicates that the object has specific type.
    bool HasType(ObjectType type) const;

    //! Checks the object has text type.
    //! \return The flag indicates that the object has text type.
    bool HasTextType() const;

    //! Checks the object has noun type.
    //! \return The flag indicates that the object has noun type.
    bool HasNounType() const;

    //! Checks the object has verb type.
    //! \return The flag indicates that the object has verb type.
    bool HasVerbType() const;

    //! Checks the object has property type.
    //! \return The flag indicates that the object has property type.
    bool HasPropertyType() const;

    bool isRule = false;

 private:
    std::vector<ObjectType> m_types;
};
}  // namespace baba_is_auto

#endif