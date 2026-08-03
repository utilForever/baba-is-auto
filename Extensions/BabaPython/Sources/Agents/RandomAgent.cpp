// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <Agents/RandomAgent.hpp>
#include <baba-is-auto/Agents/RandomAgent.hpp>

#include <pybind11/pybind11.h>

using namespace baba_is_auto;

void AddRandomAgent(pybind11::module& m)
{
    pybind11::class_<RandomAgent>(
        m, "RandomAgent", "Agent that chooses a random movement direction.")
        .def(pybind11::init<>(),
             "Creates an agent that chooses random Direction values.")
        .def("GetAction", &RandomAgent::GetAction,
             "Returns a random Direction value for the game.");
}
