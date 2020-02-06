#include <baba-is-auto/Words/Noun.hpp>

namespace baba_is_auto
{
Noun::Noun(NounType _type) : type(_type)
{
    // Do nothing
}

bool Noun::operator==(const Noun& rhs) const
{
    return type == rhs.type;
}
}  // namespace baba_is_auto