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
#define X(a) a,
#include "NounType.def"
#undef X
#define X(a) a,
#include "OpType.def"
#undef X
#define X(a) a,
#include "PropertyType.def"
#undef X
};
}  // namespace baba_is_auto

#endif  // BABA_IS_AUTO_WORD_ENUMS_HPP