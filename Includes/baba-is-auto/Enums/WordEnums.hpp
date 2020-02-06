// Copyright (c) 2019 Chris Ohk, Hyojang Kim

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#ifndef BABA_IS_AUTO_WORD_ENUMS_HPP
#define BABA_IS_AUTO_WORD_ENUMS_HPP

#include <string>
#include <string_view>

namespace baba_is_auto
{
//! \brief An enumerator for identifying the noun of the word.
enum class NounType
{
#define X(a) a,
#include "NounType.def"
#undef X
};

const std::string NOUN_TYPE_STR[] = {
#define X(a) #a,
#include "NounType.def"
#undef X
};

//! \brief An enumerator for identifying the operation of the word.
enum class OpType
{
#define X(a) a,
#include "OpType.def"
#undef X
    NEAR
};

const std::string OP_TYPE_STR[] = {
#define X(a) #a,
#include "OpType.def"
#undef X
    "NEAR"
};

//! \brief An enumerator for identifying the property of the word.
enum class PropertyType
{
#define X(a) a,
#include "PropertyType.def"
#undef X
};

const std::string PROPERTY_TYPE_STR[] = {
#define X(a) #a,
#include "PropertyType.def"
#undef X
};

template <class T>
T StrToEnum(const std::string_view&);
template <class T>
std::string_view EnumToStr(T);

#define STR2ENUM(TYPE, ARRAY)                                                \
    template <>                                                              \
    inline TYPE StrToEnum<TYPE>(const std::string_view& str)                 \
    {                                                                        \
        for (std::size_t i = 0; i < (sizeof(ARRAY) / sizeof(ARRAY[0])); ++i) \
        {                                                                    \
            if (ARRAY[i] == str)                                             \
            {                                                                \
                return TYPE(i);                                              \
            }                                                                \
        }                                                                    \
                                                                             \
        return TYPE(0);                                                      \
    }

#define ENUM2STR(TYPE, ARRAY)                       \
    template <>                                     \
    inline std::string_view EnumToStr<TYPE>(TYPE v) \
    {                                               \
        return ARRAY[static_cast<int>(v)];          \
    }

#define ENUM_AND_STR(TYPE, ARRAY) \
    STR2ENUM(TYPE, ARRAY)         \
    ENUM2STR(TYPE, ARRAY)

ENUM_AND_STR(NounType, NOUN_TYPE_STR)
ENUM_AND_STR(OpType, OP_TYPE_STR)
ENUM_AND_STR(PropertyType, PROPERTY_TYPE_STR)
}  // namespace baba_is_auto

#endif  // BABA_IS_AUTO_WORD_ENUMS_HPP