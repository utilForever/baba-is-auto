// Copyright (c) 2020-2023 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <baba-is-auto/Games/Object.hpp>

#include <algorithm>

namespace baba_is_auto
{
Object::Object(std::vector<ObjectType> types)
{
    for (const auto& type : types)
    {
        m_types.emplace(type, 1);
    }
}

bool Object::operator==(const Object& rhs) const
{
    return m_types == rhs.m_types;
}

void Object::Add(ObjectType type, bool isBoundary)
{
    if (m_types.find(type) != m_types.end())
    {
        if (isBoundary)
        {
            m_types[type] = 1;
        }
        else
        {
            m_types[type] += 1;
        }
    }
    else
    {
        m_types.emplace(type, 1);
    }
}

void Object::Remove(ObjectType type)
{
    if (auto iter = m_types.find(type); iter != m_types.end())
    {
        if (m_types[type] == 1)
        {
            m_types.erase(iter);
        }
        else
        {
            m_types[type] -= 1;
        }
    }

    if (m_types.empty())
    {
        m_types.emplace(ObjectType::ICON_EMPTY, 1);
    }
}

std::vector<ObjectType> Object::GetTypes() const
{
    std::vector<ObjectType> ret;

    for (const auto& type : m_types)
    {
        ret.emplace_back(type.first);
    }

    return ret;
}

bool Object::HasType(ObjectType type) const
{
    return m_types.find(type) != m_types.end();
}

bool Object::HasTextType() const
{
    for (const auto& type : m_types)
    {
        if (static_cast<int>(type.first) <=
            static_cast<int>(ObjectType::ICON_TYPE))
        {
            return true;
        }
    }

    return false;
}

bool Object::HasNounType() const
{
    for (auto& type : m_types)
    {
        if (IsNounType(type.first))
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
        if (IsVerbType(type.first))
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
        if (IsPropertyType(type.first))
        {
            return true;
        }
    }

    return false;
}
}  // namespace baba_is_auto