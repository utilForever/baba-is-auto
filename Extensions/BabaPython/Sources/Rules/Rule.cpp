// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <Rules/Rule.hpp>
#include <baba-is-auto/Rules/Rule.hpp>

#include <pybind11/pybind11.h>

using namespace baba_is_auto;

void AddRule(pybind11::module& m)
{
    pybind11::class_<Rule>(m, "Rule", "Represents a three-tile rule.")
        .def(pybind11::init<Object, Object, Object>(),
             "Creates a three-tile rule.")
        .def("__eq__",
             [](const Rule& left, const Rule& right) { return left == right; },
             "Returns whether two rules contain the same three objects.");
}
