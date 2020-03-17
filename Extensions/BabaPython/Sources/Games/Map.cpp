// Copyright (c) 2020 Chris Ohk

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
    pybind11::class_<Map>(m, "Map")
        .def(pybind11::init<>())
        .def(pybind11::init<std::size_t, std::size_t>())
        .def("Reset", &Map::Reset)
        .def("GetWidth", &Map::GetWidth)
        .def("GetHeight", &Map::GetHeight)
        .def("Load", &Map::Load)
        .def("AddObject", &Map::AddObject)
        .def("RemoveObject", &Map::RemoveObject)
        .def("At",
             static_cast<Object& (Map::*)(std::size_t, std::size_t)>(&Map::At))
        .def(
            "At",
            static_cast<const Object& (Map::*)(std::size_t, std::size_t) const>(
                &Map::At))
        .def("GetPositions", &Map::GetPositions);
}