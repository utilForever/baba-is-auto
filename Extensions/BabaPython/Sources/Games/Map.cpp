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
        .def("GetWidth", &Map::GetWidth)
        .def("GetHeight", &Map::GetHeight)
        .def("Load", &Map::Load)
        .def("AddObject", &Map::AddObject)
        .def("RemoveObject", &Map::RemoveObject)
        .def("At", &Map::At)
        .def("GetPositions", &Map::GetPositions);
}