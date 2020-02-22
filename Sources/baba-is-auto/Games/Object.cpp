#include <baba-is-auto/Games/Object.hpp>

namespace baba_is_auto
{
Object::Object(std::vector<ObjectType> types) : m_types(std::move(types))
{
    // Do nothing
}

bool Object::operator==(const Object& rhs) const
{
    return m_types == rhs.m_types;
}

std::vector<ObjectType> Object::GetTypes() const
{
    return m_types;
}

bool Object::HasType(ObjectType type) const
{
    return std::find(m_types.begin(), m_types.end(), type) != m_types.end();
}
}  // namespace baba_is_auto