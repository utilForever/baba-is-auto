// Copyright (c) 2020 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <Agents/IAgent.hpp>
#include <baba-is-auto/Agents/IAgent.hpp>

#include <pybind11/pybind11.h>

using namespace baba_is_auto;

void AddIAgent(pybind11::module& m)
{
    pybind11::class_<IAgent>(m, "IAgent");
}