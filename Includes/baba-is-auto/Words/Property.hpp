#ifndef BABA_IS_AUTO_PROPERTY_HPP
#define BABA_IS_AUTO_PROPERTY_HPP

#include <baba-is-auto/Enums/WordEnums.hpp>

namespace baba_is_auto
{
//!
//! \brief Property struct.
//!
//! This struct represents property. A property is something that can be
//! attached to noun words to alter their behavior.
//!
struct Property
{
    //! Default constructor.
    Property() = default;

    //! Constructs a property.
    //! \param _type The property type.
    Property(PropertyType _type) : type(_type)
    {
        // Do nothing
    }

    //! Operator overloading for ==.
    bool operator==(const Property& rhs) const
    {
        return type == rhs.type;
    }

    PropertyType type;
};
}  // namespace baba_is_auto

#endif