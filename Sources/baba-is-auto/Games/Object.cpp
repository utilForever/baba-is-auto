// Copyright (c) 2020-2026 Chris Ohk

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
        Add(type);
    }

    if (m_types.empty())
    {
        m_types.emplace(ObjectType::ICON_EMPTY, 1);
    }
}

bool Object::operator==(const Object& rhs) const
{
    return m_types == rhs.m_types;
}

void Object::Add(ObjectType type)
{
    if (type == ObjectType::ICON_EMPTY)
    {
        if (m_types.empty())
        {
            m_types.emplace(type, 1);
        }

        return;
    }

    m_types.erase(ObjectType::ICON_EMPTY);
    ++m_types[type];
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

    for (const auto& [type, count] : m_types)
    {
        for (std::size_t i = 0; i < count; ++i)
        {
            ret.emplace_back(type);
        }
    }

    return ret;
}

bool Object::HasType(ObjectType type) const
{
    return m_types.find(type) != m_types.end();
}

bool Object::HasTextType() const
{
    return std::any_of(m_types.begin(), m_types.end(), [](const auto& entry) {
        return entry.first <= ObjectType::ICON_TYPE;
    });
}

bool Object::HasNounType() const
{
    return std::any_of(m_types.begin(), m_types.end(), [](const auto& entry) {
        return IsNounType(entry.first);
    });
}

bool Object::HasVerbType() const
{
    return std::any_of(m_types.begin(), m_types.end(), [](const auto& entry) {
        return IsVerbType(entry.first);
    });
}

bool Object::HasPropertyType() const
{
    return std::any_of(m_types.begin(), m_types.end(), [](const auto& entry) {
        return IsPropertyType(entry.first);
    });
}
}  // namespace baba_is_auto
