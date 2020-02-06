#include <baba-is-auto/Rules/RuleManager.hpp>

#include <algorithm>

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
}  // namespace baba_is_auto