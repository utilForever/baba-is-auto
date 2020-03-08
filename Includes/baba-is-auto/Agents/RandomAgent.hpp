// Copyright (c) 2020 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#ifndef BABA_IS_AUTO_RANDOM_AGENT_HPP
#define BABA_IS_AUTO_RANDOM_AGENT_HPP

#include <baba-is-auto/Agents/IAgent.hpp>

namespace baba_is_auto
{
//!
//! \brief RandomAgent class.
//!
//! This class is an agent that plays an action at random.
//!
class RandomAgent final : public IAgent
{
 public:
    //! Default virtual destructor.
    virtual ~RandomAgent() = default;

    //! Gets an action of agent.
    //! \param state The current game state.
    //! \return An action of agent.
    Direction GetAction(const Game& state) override;
};
}  // namespace baba_is_auto

#endif