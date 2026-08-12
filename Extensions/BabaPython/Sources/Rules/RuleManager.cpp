// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <Rules/RuleManager.hpp>
#include <baba-is-auto/Rules/RuleManager.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

using namespace baba_is_auto;

void AddRuleManager(pybind11::module& m)
{
    pybind11::class_<RuleManager>(
        m, "RuleManager", "Stores and queries active rules.")
        .def(pybind11::init<>(), "Creates an empty rule manager.")
        .def("AddRule", &RuleManager::AddRule,
             "Adds the rule to the active rule list.")
        .def("RemoveRule", &RuleManager::RemoveRule,
             "Removes the rule when it is present.")
        .def("ClearRules", &RuleManager::ClearRules,
             "Removes every active rule.")
        .def("GetRules", &RuleManager::GetRules,
             "Returns rules containing the object type in any of their three "
             "objects.")
        .def("GetNumRules", &RuleManager::GetNumRules,
             "Returns the number of active rules.")
        .def("FindPlayer", &RuleManager::FindPlayer,
             "Returns the icon type selected by the first unconditional YOU "
             "rule.")
        .def("HasProperty", &RuleManager::HasProperty,
             "Returns whether any supplied object type has the unconditional "
             "property.");
}
