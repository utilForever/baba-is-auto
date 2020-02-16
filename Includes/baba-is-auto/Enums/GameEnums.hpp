// Copyright (c) 2019 Chris Ohk, Hyojang Kim

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
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
}  // namespace baba_is_auto

#endif  // BABA_IS_AUTO_WORD_ENUMS_HPP