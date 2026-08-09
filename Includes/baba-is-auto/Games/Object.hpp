// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#ifndef BABA_IS_AUTO_NOUN_HPP
#define BABA_IS_AUTO_NOUN_HPP

#include <baba-is-auto/Enums/GameEnums.hpp>

#include <cstdint>
#include <vector>

namespace baba_is_auto
{
using ObjectID = std::uint64_t;

//!
//! \brief Object instance struct.
//!
//! This struct represents an instance of an object in the game. It contains the
//! object's ID, type, and direction.
//!
struct ObjectInstance
{
    ObjectID id = 0;
    ObjectType type = ObjectType::ICON_EMPTY;
    Direction direction = Direction::RIGHT;
};

//!
//! \brief Object class.
//!
//! This class represents objects such as nouns, operators, and properties.
//! A noun is a word that corresponds to any possible in-game sprite. A few
//! nouns like STAR have multiple corresponding sprites, while VIOLET and
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

    //! Adds an object instance.
    //! \param type An object type to add.
    //! \param direction The direction the object faces.
    //! \param id The stable map object ID, or zero outside a map.
    void Add(ObjectType type, Direction direction, ObjectID id);

    //! Adds an existing object instance.
    //! \param instance The instance to add.
    void Add(const ObjectInstance& instance);

    //! Removes an object type.
    //! \param type An object type to remove.
    void Remove(ObjectType type);

    //! Removes the object with the given stable ID.
    //! \param id The stable map object ID.
    //! \return Whether an object was removed.
    bool Remove(ObjectID id);

    //! Gets a list of object types, including duplicate stacked objects.
    //! \return A list of object types.
    std::vector<ObjectType> GetTypes() const;

    //! Gets the object instances in insertion order.
    //! \return The object instances.
    const std::vector<ObjectInstance>& GetInstances() const;

    //! Gets a writable object instance by ID.
    //! \param id The stable map object ID.
    //! \return The instance, or nullptr when it is absent.
    ObjectInstance* GetInstance(ObjectID id);

    //! Gets an object instance by ID.
    //! \param id The stable map object ID.
    //! \return The instance, or nullptr when it is absent.
    const ObjectInstance* GetInstance(ObjectID id) const;

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

    //! Whether this object is part of an active rule.
    bool isRule = false;

 private:
    std::vector<ObjectInstance> m_instances;
};
}  // namespace baba_is_auto

#endif
