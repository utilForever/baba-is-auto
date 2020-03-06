#include <Games/Game.hpp>
#include <baba-is-auto/Games/Game.hpp>

#include <pybind11/pybind11.h>

using namespace baba_is_auto;

void AddGame(pybind11::module& m)
{
    pybind11::class_<Game>(m, "Game")
        .def(pybind11::init<std::string_view>())
        .def("GetMap", &Game::GetMap)
        .def("GetRuleManager", &Game::GetRuleManager)
        .def("GetPlayState", &Game::GetPlayState)
        .def("GetPlayerIcon", &Game::GetPlayerIcon)
        .def("MovePlayer", &Game::MovePlayer);
}