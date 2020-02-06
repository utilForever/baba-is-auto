#include <baba-is-auto/Rules/Rule.hpp>

namespace baba_is_auto
{
Rule::Rule(Noun _noun, Operator _op, Property _property)
    : type(RuleType::NOUN_VERB_PROPERTY),
      noun1(_noun),
      op(_op),
      property(_property)
{
    // Do nothing
}

Rule::Rule(Noun _noun1, Operator _op, Noun _noun2)
    : type(RuleType::NOUN_VERB_NOUN), noun1(_noun1), noun2(_noun2), op(_op)
{
    // Do nothing
}

bool Rule::operator==(const Rule& rhs) const
{
    if (type == RuleType::NOUN_VERB_PROPERTY)
    {
        return noun1 == rhs.noun1 && op == rhs.op && property == rhs.property;
    }
    else if (type == RuleType::NOUN_VERB_NOUN)
    {
        return noun1 == rhs.noun1 && op == rhs.op && noun2 == rhs.noun2;
    }

    return false;
}
}  // namespace baba_is_auto