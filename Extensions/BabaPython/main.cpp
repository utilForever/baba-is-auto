#include <Enums/GameEnums.hpp>

#include <pybind11/pybind11.h>

PYBIND11_MODULE(pyBaba, m)
{
    m.doc() =
        R"pbdoc(Baba Is You simulator with some reinforcement learning)pbdoc";

    AddGameEnums(m);
    AddGameEnumUtils(m);
}