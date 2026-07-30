// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <Games/Game.hpp>
#include <baba-is-auto/Games/Game.hpp>

#include <pybind11/pybind11.h>

using namespace baba_is_auto;

void AddGame(pybind11::module& m)
{
    pybind11::class_<Game>(
        m, "Game", "Runs a Baba Is You game loaded from a map file.")
        .def(pybind11::init<std::string_view>(),
             "Loads a map file, parses its rules, and starts a game.")
        .def("Reset", &Game::Reset,
             "Restores the loaded map and recalculates its initial rules and "
             "play state.")
        .def("GetMap", static_cast<Map& (Game::*)()>(&Game::GetMap),
             "Returns a copy of the map owned by the game.")
        .def("GetMap",
             static_cast<const Map& (Game::*)() const>(&Game::GetMap),
             "Returns a copy of the map owned by the game.")
        .def("GetRuleManager", &Game::GetRuleManager,
             "Returns a copy of the rule manager owned by the game.")
        .def("GetPlayState", &Game::GetPlayState,
             "Returns the current PlayState.")
        .def("GetPlayerIcon", &Game::GetPlayerIcon,
             "Returns the icon type selected by the active YOU rule.")
        .def("MovePlayer", &Game::MovePlayer,
             "Moves every object controlled by the active YOU rule in the "
             "given direction.");
}
