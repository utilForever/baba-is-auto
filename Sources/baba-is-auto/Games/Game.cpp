#include <baba-is-auto/Games/Game.hpp>

namespace baba_is_auto
{
Game::Game(std::string_view filename)
{
    m_map.Load(filename);
}

void Game::Initialize()
{
    const std::size_t width = m_map.GetWidth();
    const std::size_t height = m_map.GetHeight();

    for (std::size_t y = 0; y < height; ++y)
    {
        for (std::size_t x = 0; x < width; ++x)
        {
            ParseRule(y, x, RuleDirection::HORIZONTAL);
            ParseRule(y, x, RuleDirection::VERTICAL);
        }
    }

    m_playerIcon = m_ruleManager.GetPlayer();
}

Map& Game::GetMap()
{
    return m_map;
}

RuleManager& Game::GetRuleManager()
{
    return m_ruleManager;
}

ObjectType Game::GetPlayerIcon() const
{
    return m_playerIcon;
}

void Game::ParseRule(std::size_t row, std::size_t col, RuleDirection direction)
{
    const std::size_t width = m_map.GetWidth();
    const std::size_t height = m_map.GetHeight();

    if (direction == RuleDirection::HORIZONTAL)
    {
        if (col + 2 >= width)
        {
            return;
        }

        if (IsNounType(m_map.At(row, col).GetType()) &&
            IsVerbType(m_map.At(row, col + 1).GetType()) &&
            (IsNounType(m_map.At(row, col + 2).GetType()) ||
             IsPropertyType(m_map.At(row, col + 2).GetType())))
        {
            m_ruleManager.Add({ m_map.At(row, col), m_map.At(row, col + 1),
                                m_map.At(row, col + 2) });
        }
    }
    else if (direction == RuleDirection::VERTICAL)
    {
        if (row + 2 >= height)
        {
            return;
        }

        if (IsNounType(m_map.At(row, col).GetType()) &&
            IsVerbType(m_map.At(row + 1, col).GetType()) &&
            (IsNounType(m_map.At(row + 2, col).GetType()) ||
             IsPropertyType(m_map.At(row + 2, col).GetType())))
        {
            m_ruleManager.Add({ m_map.At(row, col), m_map.At(row + 1, col),
                                m_map.At(row + 2, col) });
        }
    }
}
}  // namespace baba_is_auto