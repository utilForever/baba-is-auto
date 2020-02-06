#include <baba-is-auto/Words/Property.hpp>

namespace baba_is_auto
{
Property::Property(PropertyType _type) : type(_type)
{
    // Do nothing
}

bool Property::operator==(const Property& rhs) const
{
    return type == rhs.type;
}
}  // namespace baba_is_auto