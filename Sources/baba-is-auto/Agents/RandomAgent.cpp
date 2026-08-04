// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <baba-is-auto/Agents/RandomAgent.hpp>

#include <chrono>
#include <cstdint>
#include <random>

namespace baba_is_auto
{
Direction RandomAgent::GetAction([[maybe_unused]] const Game& state)
{
    static std::mt19937 engine = [] {
        std::seed_seq seed{
            static_cast<std::uintmax_t>(std::random_device{}()),
            static_cast<std::uintmax_t>(
                std::chrono::steady_clock::now().time_since_epoch().count())
        };
        return std::mt19937{ seed };
    }();
    static std::uniform_int_distribution<int> action{
        0, static_cast<int>(Direction::RIGHT)
    };

    return static_cast<Direction>(action(engine));
}
}  // namespace baba_is_auto
