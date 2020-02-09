#include <baba-is-auto/Games/Game.hpp>

namespace baba_is_auto
{
Game::Game(std::string_view filename)
{
    m_map.Load(filename);
}
}  // namespace baba_is_auto