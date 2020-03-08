// Copyright (c) 2020 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <Agents/IAgent.hpp>
#include <Agents/Preprocess.hpp>
#include <Agents/RandomAgent.hpp>
#include <Enums/GameEnums.hpp>
#include <Enums/RuleEnums.hpp>
#include <Games/Game.hpp>
#include <Games/Map.hpp>
#include <Games/Object.hpp>
#include <Rules/Rule.hpp>
#include <Rules/RuleManager.hpp>

#include <pybind11/pybind11.h>

PYBIND11_MODULE(pyBaba, m)
{
    m.doc() =
        R"pbdoc(Baba Is You simulator with some reinforcement learning)pbdoc";

    AddIAgent(m);
    AddPreprocess(m);
    AddRandomAgent(m);

    AddGameEnums(m);
    AddGameEnumUtils(m);
    AddRuleEnums(m);

    AddGame(m);
    AddMap(m);
    AddObject(m);

    AddRule(m);
    AddRuleManager(m);
}