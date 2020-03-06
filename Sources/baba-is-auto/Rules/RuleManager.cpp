// Copyright (c) 2020 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <baba-is-auto/Rules/RuleManager.hpp>

#include <algorithm>
#include <tuple>

namespace baba_is_auto
{
void RuleManager::AddRule(const Rule& rule)
{
    m_rules.emplace_back(rule);
}

void RuleManager::RemoveRule(const Rule& rule)
{
    const auto iter = std::find(m_rules.begin(), m_rules.end(), rule);
    if (iter != m_rules.end())
    {
        m_rules.erase(iter);
    }
}

void RuleManager::ClearRules()
{
    m_rules.clear();
}

std::vector<Rule> RuleManager::GetRules(ObjectType type) const
{
    std::vector<Rule> ret;

    for (auto& rule : m_rules)
    {
        if (std::get<0>(rule.objects).HasType(type) ||
            std::get<1>(rule.objects).HasType(type) ||
            std::get<2>(rule.objects).HasType(type))
        {
            ret.emplace_back(rule);
        }
    }

    return ret;
}

std::size_t RuleManager::GetNumRules() const
{
    return m_rules.size();
}

ObjectType RuleManager::FindPlayer() const
{
    for (auto& rule : m_rules)
    {
        if (std::get<2>(rule.objects).HasType(ObjectType::YOU))
        {
            const ObjectType type = std::get<0>(rule.objects).GetTypes()[0];
            return ConvertTextToIcon(type);
        }
    }

    return ObjectType::ICON_EMPTY;
}

bool RuleManager::HasProperty(const std::vector<ObjectType>& types,
                              ObjectType property)
{
    for (auto type : types)
    {
        type = ConvertIconToText(type);

        for (auto& rule : m_rules)
        {
            if (std::get<0>(rule.objects).HasType(type) &&
                std::get<2>(rule.objects).HasType(property))
            {
                return true;
            }
        }
    }

    return false;
}
}  // namespace baba_is_auto