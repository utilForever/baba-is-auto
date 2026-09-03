// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#ifndef BABA_IS_AUTO_GAME_ENUMS_HPP
#define BABA_IS_AUTO_GAME_ENUMS_HPP

namespace baba_is_auto
{
//! \brief An enumerator for identifying the object.
enum class ObjectType
{
    NOUN_TYPE,
#define X(a) a,
#include "NounType.def"
#undef X
    OP_TYPE,
#define X(a) a,
#include "OpType.def"
#undef X
    PROPERTY_TYPE,
#define X(a) a,
#include "PropertyType.def"
#undef X
    ICON_TYPE,
#define X(a) a,
#include "IconType.def"
#undef X
    LOCKED_UP,
    LOCKED_DOWN,
    LOCKED_LEFT,
    LOCKED_RIGHT,
};

//! Checks \p type is a directional LOCKED property.
//! \param type The object type.
//! \return The flag that indicates it is a directional LOCKED property.
constexpr bool IsLockedType(ObjectType type)
{
    return type >= ObjectType::LOCKED_UP &&
           type <= ObjectType::LOCKED_RIGHT;
}

//! Checks \p type is text type.
//! \param type The object type.
//! \return The flag that indicates it is text type.
constexpr bool IsTextType(ObjectType type)
{
    return type < ObjectType::ICON_TYPE || IsLockedType(type);
}

//! Checks \p type is noun type.
//! \param type The object type.
//! \return The flag that indicates it is noun type.
constexpr bool IsNounType(ObjectType type)
{
    return (type > ObjectType::NOUN_TYPE && type < ObjectType::OP_TYPE);
}

//! Checks \p type is op type.
//! \param type The object type.
//! \return The flag that indicates it is op type.
constexpr bool IsOpType(ObjectType type)
{
    return (type > ObjectType::OP_TYPE && type < ObjectType::PROPERTY_TYPE);
}

//! Checks \p type is verb type.
//! \param type The object type.
//! \return The flag that indicates it is verb type.
constexpr bool IsVerbType(ObjectType type)
{
    return (type == ObjectType::IS || type == ObjectType::HAS ||
            type == ObjectType::MAKE);
}

//! Checks \p type is property type.
//! \param type The object type.
//! \return The flag that indicates it is property type.
constexpr bool IsPropertyType(ObjectType type)
{
    return (type > ObjectType::PROPERTY_TYPE &&
            type < ObjectType::ICON_TYPE) ||
           IsLockedType(type);
}

//! Checks \p type is an in-game icon type.
//! \param type The object type.
//! \return The flag that indicates it is an in-game icon type.
constexpr bool IsIconType(ObjectType type)
{
    return type > ObjectType::ICON_TYPE && type <= ObjectType::ICON_WATER;
}

//! Converts icon type to text type.
//! \param type The icon type to convert.
//! \return The converted text type.
constexpr ObjectType ConvertIconToText(ObjectType type)
{
    const auto typeVal = static_cast<int>(type);
    const auto iconTypeVal = static_cast<int>(ObjectType::ICON_TYPE);

    if (!IsIconType(type))
    {
        return type;
    }

    const int convertedVal = typeVal - iconTypeVal;
    return static_cast<ObjectType>(convertedVal);
}

//! Converts text type to icon type.
//! \param type The text type to convert.
//! \return The converted icon type.
constexpr ObjectType ConvertTextToIcon(ObjectType type)
{
    const auto typeVal = static_cast<int>(type);
    const auto iconTypeVal = static_cast<int>(ObjectType::ICON_TYPE);

    if (!IsNounType(type))
    {
        return type;
    }

    const int convertedVal = typeVal + iconTypeVal;
    return static_cast<ObjectType>(convertedVal);
}

//! Checks whether a type belongs to the ordinary noun set selected by ALL.
//! \param type The text or icon type to check.
//! \return Whether ALL selects the type.
constexpr bool IsAllNoun(ObjectType type)
{
    const ObjectType noun = ConvertIconToText(type);
    return IsNounType(noun) && noun != ObjectType::TEXT &&
           noun != ObjectType::EMPTY && noun != ObjectType::LEVEL;
}

//! Checks whether a rule subject selects an object instance type.
//! \param subject The rule subject.
//! \param type The object instance type.
//! \return Whether the subject selects the instance.
constexpr bool SubjectMatches(ObjectType subject, ObjectType type)
{
    if (subject == ObjectType::TEXT)
    {
        return IsTextType(type);
    }

    if (subject == ObjectType::ALL)
    {
        return IsIconType(type) && IsAllNoun(type);
    }

    return ConvertTextToIcon(subject) == type;
}

//! \brief An enumerator for identifying the play state.
enum class PlayState
{
    INVALID,
    PLAYING,
    WON,
    LOST
};

//! \brief An enumerator for identifying the direction.
enum class Direction
{
    NONE,
    UP,
    DOWN,
    LEFT,
    RIGHT
};
}  // namespace baba_is_auto

#endif  // BABA_IS_AUTO_WORD_ENUMS_HPP
