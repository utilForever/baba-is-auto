// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <Enums/GameEnums.hpp>
#include <baba-is-auto/Enums/GameEnums.hpp>

#include <pybind11/pybind11.h>

using namespace baba_is_auto;

void AddGameEnums(pybind11::module& m)
{
#define X(val) .value(#val, ObjectType::val)
    pybind11::enum_<ObjectType>(
        m, "ObjectType", "Identifies text tiles and in-game icons.")
        .value("NOUN_TYPE", ObjectType::NOUN_TYPE)
#include <baba-is-auto/Enums/NounType.def>
        .value("OP_TYPE", ObjectType::OP_TYPE)
#include <baba-is-auto/Enums/OpType.def>
        .value("PROPERTY_TYPE", ObjectType::PROPERTY_TYPE)
#include <baba-is-auto/Enums/PropertyType.def>
        .value("ICON_TYPE", ObjectType::ICON_TYPE)
#include <baba-is-auto/Enums/IconType.def>
        .export_values();
#undef X

    pybind11::enum_<PlayState>(m, "PlayState",
                               "Identifies the current game outcome.")
        .value("INVALID", PlayState::INVALID)
        .value("PLAYING", PlayState::PLAYING)
        .value("WON", PlayState::WON)
        .value("LOST", PlayState::LOST)
        .export_values();

    pybind11::enum_<Direction>(m, "Direction",
                               "Identifies a movement direction.")
        .value("NONE", Direction::NONE)
        .value("UP", Direction::UP)
        .value("DOWN", Direction::DOWN)
        .value("LEFT", Direction::LEFT)
        .value("RIGHT", Direction::RIGHT)
        .export_values();
}

void AddGameEnumUtils(pybind11::module& m)
{
    m.def("IsTextType", IsTextType,
          "Returns whether an object type represents a text tile.");
    m.def("IsNounType", IsNounType,
          "Returns whether an object type represents noun text.");
    m.def("IsOpType", IsOpType,
          "Returns whether an object type represents operator text.");
    m.def("IsVerbType", IsVerbType,
          "Returns whether an object type represents a supported verb.");
    m.def("IsPropertyType", IsPropertyType,
          "Returns whether an object type represents property text.");

    m.def("ConvertIconToText", ConvertIconToText,
          "Converts an icon type to its matching text type; other values are "
          "returned unchanged.");
    m.def("ConvertTextToIcon", ConvertTextToIcon,
          "Converts a text type to its matching icon type; icon values are "
          "returned unchanged.");
}
