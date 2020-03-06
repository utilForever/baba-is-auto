// Copyright (c) 2020 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

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
    if (m_types.size() == 1 && m_types.at(0) == ObjectType::ICON_EMPTY)
    {
        m_types.clear();
    }

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

    if (m_types.empty())
    {
        m_types.emplace_back(ObjectType::ICON_EMPTY);
    }
}

std::vector<ObjectType> Object::GetTypes() const
{
    return m_types;
}

bool Object::HasType(ObjectType type) const
{
    return std::find(m_types.begin(), m_types.end(), type) != m_types.end();
}

bool Object::HasTextType() const
{
    if (m_types.size() != 1)
    {
        return false;
    }

    return static_cast<int>(m_types[0]) <=
           static_cast<int>(ObjectType::ICON_TYPE);
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