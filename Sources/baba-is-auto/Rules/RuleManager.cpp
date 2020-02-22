#include <baba-is-auto/Rules/RuleManager.hpp>

#include <algorithm>
#include <tuple>

namespace baba_is_auto
{
void RuleManager::Add(Rule rule)
{
    m_rules.emplace_back(rule);
}

void RuleManager::Remove(Rule rule)
{
    const auto iter = std::find(m_rules.begin(), m_rules.end(), rule);
    if (iter != m_rules.end())
    {
        m_rules.erase(iter);
    }
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
            ObjectType obj = std::get<0>(rule.objects).GetTypes()[0];
            return static_cast<ObjectType>(
                static_cast<int>(obj) +
                static_cast<int>(ObjectType::ICON_TYPE));
        }
    }

    return ObjectType::ICON_EMPTY;
}

bool RuleManager::HasProperty(const std::vector<ObjectType>& types,
                              ObjectType property)
{
    const int iconTypeVal = static_cast<int>(ObjectType::ICON_TYPE);

    for (auto type : types)
    {
        const int nounVal = static_cast<int>(type);

        // Check object is an icon
        if (nounVal > iconTypeVal)
        {
            type = static_cast<ObjectType>(nounVal - iconTypeVal);

            for (auto& rule : m_rules)
            {
                if (std::get<0>(rule.objects).HasType(type) &&
                    std::get<2>(rule.objects).HasType(property))
                {
                    return true;
                }
            }
        }
    }

    return false;
}
}  // namespace baba_is_auto