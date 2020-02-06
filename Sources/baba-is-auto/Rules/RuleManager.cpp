#include <baba-is-auto/Rules/RuleManager.hpp>

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
}  // namespace baba_is_auto