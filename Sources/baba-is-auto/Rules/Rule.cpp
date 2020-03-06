// Copyright (c) 2020 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <baba-is-auto/Rules/Rule.hpp>

namespace baba_is_auto
{
Rule::Rule(Object obj1, Object obj2, Object obj3)
{
    objects = { obj1, obj2, obj3 };
}

bool Rule::operator==(const Rule& rhs) const
{
    return objects == rhs.objects;
}
}  // namespace baba_is_auto