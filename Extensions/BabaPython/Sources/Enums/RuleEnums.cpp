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