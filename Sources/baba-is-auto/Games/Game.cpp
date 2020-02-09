#include <baba-is-auto/Games/Game.hpp>

namespace baba_is_auto
{
Game::Game(std::string_view filename)
{
    m_map.Load(filename);
}

Map& Game::GetMap()
{
    return m_map;
}
}  // namespace baba_is_auto