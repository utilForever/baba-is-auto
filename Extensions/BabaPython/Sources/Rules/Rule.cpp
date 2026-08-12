// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <Rules/Rule.hpp>
#include <baba-is-auto/Rules/Rule.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

using namespace baba_is_auto;

void AddRule(pybind11::module& m)
{
    pybind11::class_<RuleCondition>(
        m, "RuleCondition", "Restricts a rule to matching subject instances.")
        .def(pybind11::init<>(), "Creates an ON condition with no targets.")
        .def_readwrite("op", &RuleCondition::op,
                       "The LONELY, ON, NEAR, or FACING operator.")
        .def_readwrite("targets", &RuleCondition::targets,
                       "Object types required by the condition.")
        .def_readwrite("negated", &RuleCondition::negated,
                       "Whether the condition is negated.")
        .def("__eq__",
             [](const RuleCondition& left, const RuleCondition& right) {
                 return left == right;
             },
             "Returns whether two conditions contain the same values.");

    pybind11::class_<Rule>(m, "Rule",
                           "Represents a three-tile rule with optional "
                           "conditions.")
        .def(pybind11::init<Object, Object, Object>(),
             "Creates a three-tile rule.")
        .def(pybind11::init<Object, Object, Object,
                            std::vector<RuleCondition>>(),
             "Creates a three-tile rule with subject conditions.")
        .def_readonly("conditions", &Rule::conditions,
                      "Conditions restricting matching subject instances.")
        .def("__eq__",
             [](const Rule& left, const Rule& right) { return left == right; },
             "Returns whether two rules contain the same objects and "
             "conditions.");
}
