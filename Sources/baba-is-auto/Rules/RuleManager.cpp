#include <baba-is-auto/Rules/RuleManager.hpp>

#include <algorithm>

namespace baba_is_auto
{
void RuleManager::Add(Noun noun, Operator op, Property property)
{
    Rule rule{};

    rule.type = RuleType::NOUN_VERB_PROPERTY;
    rule.noun1 = noun;
    rule.op = op;
    rule.property = property;

    m_rules.emplace_back(rule);
}

void RuleManager::Add(Noun noun1, Operator op, Noun noun2)
{
    Rule rule{};

    rule.type = RuleType::NOUN_VERB_NOUN;
    rule.noun1 = noun1;
    rule.op = op;
    rule.noun2 = noun2;

    m_rules.emplace_back(rule);
}

void RuleManager::Remove(Noun noun, Operator op, Property property)
{
    Rule rule{};

    rule.type = RuleType::NOUN_VERB_PROPERTY;
    rule.noun1 = noun;
    rule.op = op;
    rule.property = property;

    const auto iter = std::find(m_rules.begin(), m_rules.end(), rule);
    if (iter != m_rules.end())
    {
        m_rules.erase(iter);
    }
}

void RuleManager::Remove(Noun noun1, Operator op, Noun noun2)
{
    Rule rule{};

    rule.type = RuleType::NOUN_VERB_NOUN;
    rule.noun1 = noun1;
    rule.op = op;
    rule.noun2 = noun2;

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