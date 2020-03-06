// Copyright (c) 2020 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <Rules/RuleManager.hpp>
#include <baba-is-auto/Rules/RuleManager.hpp>

#include <pybind11/pybind11.h>

using namespace baba_is_auto;

void AddRuleManager(pybind11::module& m)
{
    pybind11::class_<RuleManager>(m, "RuleManager")
        .def(pybind11::init<>())
        .def("AddRule", &RuleManager::AddRule)
        .def("RemoveRule", &RuleManager::RemoveRule)
        .def("ClearRules", &RuleManager::ClearRules)
        .def("GetRules", &RuleManager::GetRules)
        .def("GetNumRules", &RuleManager::GetNumRules)
        .def("FindPlayer", &RuleManager::FindPlayer)
        .def("HasProperty", &RuleManager::HasProperty);
}