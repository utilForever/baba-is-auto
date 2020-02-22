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
}  // namespace baba_is_auto