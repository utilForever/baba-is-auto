// Copyright (c) 2020 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <Enums/RuleEnums.hpp>
#include <baba-is-auto/Enums/RuleEnums.hpp>

#include <pybind11/pybind11.h>

using namespace baba_is_auto;

void AddRuleEnums(pybind11::module& m)
{
    pybind11::enum_<RuleDirection>(m, "RuleDirection")
        .value("HORIZONTAL", RuleDirection::HORIZONTAL)
        .value("VERTICAL", RuleDirection::VERTICAL)
        .export_values();
}