#include <baba-is-auto/Games/Game.hpp>

namespace baba_is_auto
{
Game::Game(std::string_view filename)
{
    m_map.Load(filename);
}

void Game::Initialize()
{
    m_map.ParseRules();
}

Map& Game::GetMap()
{
    return m_map;
}

RuleManager& Game::GetRuleManager()
{
    return m_ruleManager;
}
}  // namespace baba_is_auto