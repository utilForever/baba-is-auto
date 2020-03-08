// Copyright (c) 2020 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <Agents/Preprocess.hpp>
#include <baba-is-auto/Agents/Preprocess.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

using namespace baba_is_auto;

void AddPreprocess(pybind11::module& m)
{
    pybind11::class_<Preprocess>(m, "Preprocess")
        .def_static("StateToTensor", &Preprocess::StateToTensor)
        .def_readonly_static("TENSOR_DIM", &Preprocess::TENSOR_DIM);
}