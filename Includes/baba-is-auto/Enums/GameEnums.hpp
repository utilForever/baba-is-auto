// Copyright (c) 2020 Chris Ohk

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
};

//! Checks \p type is text type.
//! \param type The object type.
//! \return The flag that indicates it is text type.
constexpr bool IsTextType(ObjectType type)
{
    return type < ObjectType::ICON_TYPE;
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
    return (type > ObjectType::PROPERTY_TYPE && type < ObjectType::ICON_TYPE);
}

//! Converts icon type to text type.
//! \param type The icon type to convert.
//! \return The converted text type.
constexpr ObjectType ConvertIconToText(ObjectType type)
{
    const auto typeVal = static_cast<int>(type);
    const auto iconTypeVal = static_cast<int>(ObjectType::ICON_TYPE);

    if (typeVal <= iconTypeVal)
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

    if (typeVal > iconTypeVal)
    {
        return type;
    }

    const int convertedVal = typeVal + iconTypeVal;
    return static_cast<ObjectType>(convertedVal);
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