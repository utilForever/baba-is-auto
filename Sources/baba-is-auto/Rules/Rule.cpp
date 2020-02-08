#include <baba-is-auto/Rules/Rule.hpp>

namespace baba_is_auto
{
Rule::Rule(Object obj1, Object obj2, Object obj3)
{
    m_objects = { obj1, obj2, obj3 };
}

bool Rule::operator==(const Rule& rhs) const
{
    return m_objects == rhs.m_objects;
}
}  // namespace baba_is_auto