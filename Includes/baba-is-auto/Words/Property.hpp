#ifndef BABA_IS_AUTO_PROPERTY_HPP
#define BABA_IS_AUTO_PROPERTY_HPP

#include <baba-is-auto/Enums/WordEnums.hpp>

namespace baba_is_auto
{
//!
//! \brief Property class.
//!
//! This class represents property. A property is something that can be
//! attached to noun words to alter their behavior.
//!
class Property
{
 public:
    //! Default constructor.
    Property() = default;

    //! Constructs a property.
    //! \param _type The property type.
    explicit Property(PropertyType _type);

    //! Operator overloading for ==.
    //! \param rhs A right side of Rule object.
    //! \return The value that indicates two objects are equal.
    bool operator==(const Property& rhs) const;

    PropertyType type;
};
}  // namespace baba_is_auto

#endif