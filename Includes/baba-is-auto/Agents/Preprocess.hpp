// Copyright (c) 2020 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#ifndef BABA_IS_AUTO_PREPROCESS_HPP
#define BABA_IS_AUTO_PREPROCESS_HPP

#include <baba-is-auto/Games/Game.hpp>

#include <vector>

namespace baba_is_auto
{
//!
//! \brief Preprocess class.
//!
//! This class contains utility functions for preprocess.
//!
class Preprocess
{
 public:
    //! The dimension of the tensor. (14 + 2)
    constexpr static int TENSOR_DIM = 16;

    //! Converts the game state to the tensor.
    //! \param game The game state.
    //! \return The converted tensor.
    static std::vector<float> StateToTensor(const Game& game);
};
}  // namespace baba_is_auto

#endif