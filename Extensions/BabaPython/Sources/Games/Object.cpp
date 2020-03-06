// Copyright (c) 2020 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <Games/Object.hpp>
#include <baba-is-auto/Games/Object.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

using namespace baba_is_auto;

void AddObject(pybind11::module& m)
{
    pybind11::class_<Object>(m, "Object")
        .def(pybind11::init<>())
        .def(pybind11::init<std::vector<ObjectType>>())
        .def("__eq__",
             [](const Object& left, const Object& right) {
                 return left.GetTypes() == right.GetTypes();
             })
        .def("Add", &Object::Add)
        .def("Remove", &Object::Remove)
        .def("GetTypes", &Object::GetTypes)
        .def("HasType", &Object::HasType)
        .def("HasTextType", &Object::HasTextType)
        .def("HasNounType", &Object::HasNounType)
        .def("HasVerbType", &Object::HasVerbType)
        .def("HasPropertyType", &Object::HasPropertyType);
}