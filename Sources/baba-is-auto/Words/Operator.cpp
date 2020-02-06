#include <baba-is-auto/Words/Operator.hpp>

namespace baba_is_auto
{
Operator::Operator(OpType _type) : type(_type)
{
    // Do nothing
}

bool Operator::operator==(const Operator& rhs) const
{
    return type == rhs.type;
}
}  // namespace baba_is_auto