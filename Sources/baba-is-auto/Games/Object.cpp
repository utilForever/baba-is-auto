#include <baba-is-auto/Games/Object.hpp>

namespace baba_is_auto
{
Object::Object(ObjectType type) : m_type(type)
{
    // Do nothing
}

bool Object::operator==(const Object& rhs) const
{
    return m_type == rhs.m_type;
}

ObjectType Object::GetType() const
{
    return m_type;
}
}  // namespace baba_is_auto