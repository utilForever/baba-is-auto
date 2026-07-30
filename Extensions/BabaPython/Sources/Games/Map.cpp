// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <Games/Map.hpp>
#include <baba-is-auto/Games/Map.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

using namespace baba_is_auto;

void AddMap(pybind11::module& m)
{
    pybind11::class_<Map>(m, "Map",
                          "Stores and edits a two-dimensional game board.")
        .def(pybind11::init<>(), "Creates an empty zero-sized map.")
        .def(pybind11::init<std::size_t, std::size_t>(),
             "Creates an empty map with the given dimensions.")
        .def("Reset", &Map::Reset,
             "Restores the objects recorded when the map was loaded.")
        .def("GetWidth", &Map::GetWidth, "Returns the map width.")
        .def("GetHeight", &Map::GetHeight, "Returns the map height.")
        .def("Load", &Map::Load,
             "Loads a one-to-three-layer text map.")
        .def("AddObject", &Map::AddObject,
             "Adds one object type to the cell at zero-based (x, y).")
        .def("RemoveObject", &Map::RemoveObject,
             "Removes one object type from the cell at zero-based (x, y).")
        .def("At",
             static_cast<Object& (Map::*)(std::size_t, std::size_t)>(&Map::At),
             "Returns the live Object stored at zero-based (x, y).")
        .def(
            "At",
            static_cast<const Object& (Map::*)(std::size_t, std::size_t) const>(
                &Map::At),
            "Returns the live Object stored at zero-based (x, y).")
        .def("GetPositions", &Map::GetPositions,
             "Returns all zero-based (x, y) positions containing the object "
             "type.");
}
