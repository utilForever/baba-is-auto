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

    if (m_instances.empty())
    {
        Add(ObjectType::ICON_EMPTY);
    }
}

bool Object::operator==(const Object& rhs) const
{
    return GetTypes() == rhs.GetTypes();
}

void Object::Add(ObjectType type)
{
    Add(type, Direction::RIGHT, 0);
}

void Object::Add(ObjectType type, Direction direction, ObjectID id)
{
    if (type == ObjectType::ICON_EMPTY)
    {
        if (m_instances.empty())
        {
            m_instances.push_back({ 0, type, Direction::RIGHT });
        }

        return;
    }

    m_instances.erase(std::remove_if(m_instances.begin(), m_instances.end(),
                                     [](const ObjectInstance& instance) {
                                         return instance.type ==
                                                ObjectType::ICON_EMPTY;
                                     }),
                      m_instances.end());
    m_instances.push_back({ id, type, direction });
}

void Object::Add(const ObjectInstance& instance)
{
    Add(instance.type, instance.direction, instance.id);
}

void Object::Remove(ObjectType type)
{
    const auto iter = std::find_if(
        m_instances.begin(), m_instances.end(),
        [type](const auto& instance) { return instance.type == type; });

    if (iter != m_instances.end())
    {
        m_instances.erase(iter);
    }

    if (m_instances.empty())
    {
        Add(ObjectType::ICON_EMPTY);
    }
}

bool Object::Remove(ObjectID id)
{
    const auto iter = std::find_if(
        m_instances.begin(), m_instances.end(),
        [id](const ObjectInstance& instance) { return instance.id == id; });

    if (iter == m_instances.end() || iter->type == ObjectType::ICON_EMPTY)
    {
        return false;
    }

    m_instances.erase(iter);

    if (m_instances.empty())
    {
        Add(ObjectType::ICON_EMPTY);
    }

    return true;
}

std::vector<ObjectType> Object::GetTypes() const
{
    std::vector<ObjectType> ret;

    for (const ObjectInstance& instance : m_instances)
    {
        ret.emplace_back(instance.type);
    }

    std::sort(ret.begin(), ret.end());

    return ret;
}

const std::vector<ObjectInstance>& Object::GetInstances() const
{
    return m_instances;
}

ObjectInstance* Object::GetInstance(ObjectID id)
{
    const auto iter = std::find_if(
        m_instances.begin(), m_instances.end(),
        [id](const ObjectInstance& instance) { return instance.id == id; });
    return iter == m_instances.end() ? nullptr : &*iter;
}

const ObjectInstance* Object::GetInstance(ObjectID id) const
{
    const auto iter = std::find_if(
        m_instances.begin(), m_instances.end(),
        [id](const ObjectInstance& instance) { return instance.id == id; });
    return iter == m_instances.end() ? nullptr : &*iter;
}

bool Object::HasType(ObjectType type) const
{
    return std::any_of(m_instances.begin(), m_instances.end(),
                       [type](const ObjectInstance& instance) {
                           return instance.type == type;
                       });
}

bool Object::HasTextType() const
{
    return std::any_of(m_instances.begin(), m_instances.end(),
                       [](const ObjectInstance& instance) {
                           return IsTextType(instance.type);
                       });
}

bool Object::HasNounType() const
{
    return std::any_of(
        m_instances.begin(), m_instances.end(),
        [](const auto& instance) { return IsNounType(instance.type); });
}

bool Object::HasVerbType() const
{
    return std::any_of(
        m_instances.begin(), m_instances.end(),
        [](const auto& instance) { return IsVerbType(instance.type); });
}

bool Object::HasPropertyType() const
{
    return std::any_of(
        m_instances.begin(), m_instances.end(),
        [](const auto& instance) { return IsPropertyType(instance.type); });
}
}  // namespace baba_is_auto
