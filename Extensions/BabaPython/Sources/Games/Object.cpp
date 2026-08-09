// Copyright (c) 2020-2026 Chris Ohk

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
    pybind11::class_<ObjectInstance>(
        m, "ObjectInstance", "One object with stable identity and facing.")
        .def_readonly("id", &ObjectInstance::id)
        .def_readonly("type", &ObjectInstance::type)
        .def_readonly("direction", &ObjectInstance::direction);

    pybind11::class_<Object>(m, "Object",
                             "Stores the object types stacked in one map cell.")
        .def(pybind11::init<>(), "Creates an empty cell object.")
        .def(pybind11::init<std::vector<ObjectType>>(),
             "Creates a cell object containing the supplied object types.")
        .def(
            "__eq__",
            [](const Object& left, const Object& right) {
                return left.GetTypes() == right.GetTypes();
            },
            "Returns whether two cell objects contain the same types.")
        .def("Add", static_cast<void (Object::*)(ObjectType)>(&Object::Add),
             "Adds one object type to the cell facing right.")
        .def("Remove",
             static_cast<void (Object::*)(ObjectType)>(&Object::Remove),
             "Removes one matching object type from the cell.")
        .def("GetTypes", &Object::GetTypes,
             "Returns all object types, including duplicate stacked values.")
        .def("GetInstances", &Object::GetInstances,
             "Returns object instances with stable IDs and facing.")
        .def("HasType", &Object::HasType,
             "Returns whether the cell contains the object type.")
        .def("HasTextType", &Object::HasTextType,
             "Returns whether the cell contains any text tile.")
        .def("HasNounType", &Object::HasNounType,
             "Returns whether the cell contains any noun text tile.")
        .def("HasVerbType", &Object::HasVerbType,
             "Returns whether the cell contains any verb text tile.")
        .def("HasPropertyType", &Object::HasPropertyType,
             "Returns whether the cell contains any property text tile.");
}
