// Copyright (c) 2020 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <baba-is-auto/Agents/RandomAgent.hpp>

#include <effolkronium/random.hpp>

namespace baba_is_auto
{
Direction RandomAgent::GetAction([[maybe_unused]] const Game& state)
{
    using Random = effolkronium::random_static;

    return static_cast<Direction>(
        Random::get(0, static_cast<int>(Direction::RIGHT)));
}
}  // namespace baba_is_auto