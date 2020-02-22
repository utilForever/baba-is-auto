#include <baba-is-auto/Games/Object.hpp>

#include <algorithm>

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

void Object::Add(ObjectType type)
{
    if (std::find(m_types.begin(), m_types.end(), type) == m_types.end())
    {
        m_types.emplace_back(type);
    }
}

void Object::Remove(ObjectType type)
{
    m_types.erase(
        std::remove_if(m_types.begin(), m_types.end(),
                       [&](ObjectType& _type) { return type == _type; }),
        m_types.end());
}

std::vector<ObjectType> Object::GetTypes() const
{
    return m_types;
}

bool Object::HasType(ObjectType type) const
{
    return std::find(m_types.begin(), m_types.end(), type) != m_types.end();
}

bool Object::HasNounType() const
{
    for (auto& type : m_types)
    {
        if (IsNounType(type))
        {
            return true;
        }
    }

    return false;
}

bool Object::HasVerbType() const
{
    for (auto& type : m_types)
    {
        if (IsVerbType(type))
        {
            return true;
        }
    }

    return false;
}

bool Object::HasPropertyType() const
{
    for (auto& type : m_types)
    {
        if (IsPropertyType(type))
        {
            return true;
        }
    }

    return false;
}
}  // namespace baba_is_auto