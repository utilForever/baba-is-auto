// Copyright (c) 2020 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#ifndef BABA_IS_AUTO_IAGENT_HPP
#define BABA_IS_AUTO_IAGENT_HPP

#include <baba-is-auto/Enums/GameEnums.hpp>
#include <baba-is-auto/Games/Game.hpp>

namespace baba_is_auto
{
//!
//! \brief IAgent class.
//!
//! This class is an interface of agent classes.
//!
class IAgent
{
 public:
    //! Default virtual destructor.
    virtual ~IAgent() = default;

    //! Gets an action of agent.
    //! \param state The current game state.
    //! \return An action of agent.
    virtual Direction GetAction(const Game& state) = 0;
};
}  // namespace baba_is_auto

#endif