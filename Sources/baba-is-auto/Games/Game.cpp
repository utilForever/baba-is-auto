// Copyright (c) 2020 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <baba-is-auto/Games/Game.hpp>

namespace baba_is_auto
{
Game::Game(std::string_view filename)
{
    m_map.Load(filename);

    ParseRules();

    m_playState = PlayState::PLAYING;
}

Map& Game::GetMap()
{
    return m_map;
}

const Map& Game::GetMap() const
{
    return m_map;
}

RuleManager& Game::GetRuleManager()
{
    return m_ruleManager;
}

PlayState Game::GetPlayState() const
{
    return m_playState;
}

ObjectType Game::GetPlayerIcon() const
{
    return m_playerIcon;
}

void Game::MovePlayer(Direction dir)
{
    auto positions = GetMap().GetPositions(m_playerIcon);

    for (auto& [row, col] : positions)
    {
        if (CanMove(row, col, dir))
        {
            ProcessMove(row, col, dir, m_playerIcon);
        }
    }

    ParseRules();
    CheckPlayState();
}

void Game::ParseRules()
{
    m_ruleManager.ClearRules();

    const std::size_t width = m_map.GetWidth();
    const std::size_t height = m_map.GetHeight();

    for (std::size_t y = 0; y < height; ++y)
    {
        for (std::size_t x = 0; x < width; ++x)
        {
            m_map.At(y, x).isRule = false;

            ParseRule(y, x, RuleDirection::HORIZONTAL);
            ParseRule(y, x, RuleDirection::VERTICAL);
        }
    }

    m_playerIcon = m_ruleManager.FindPlayer();
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

        if (m_map.At(row, col).HasNounType() &&
            m_map.At(row, col + 1).HasVerbType() &&
            (m_map.At(row, col + 2).HasNounType() ||
             m_map.At(row, col + 2).HasPropertyType()))
        {
            m_ruleManager.AddRule({ m_map.At(row, col), m_map.At(row, col + 1),
                                    m_map.At(row, col + 2) });

            m_map.At(row, col).isRule = true;
            m_map.At(row, col + 1).isRule = true;
            m_map.At(row, col + 2).isRule = true;
        }
    }
    else if (direction == RuleDirection::VERTICAL)
    {
        if (row + 2 >= height)
        {
            return;
        }

        if (m_map.At(row, col).HasNounType() &&
            m_map.At(row + 1, col).HasVerbType() &&
            (m_map.At(row + 2, col).HasNounType() ||
             m_map.At(row + 2, col).HasPropertyType()))
        {
            m_ruleManager.AddRule({ m_map.At(row, col), m_map.At(row + 1, col),
                                    m_map.At(row + 2, col) });

            m_map.At(row, col).isRule = true;
            m_map.At(row + 1, col).isRule = true;
            m_map.At(row + 2, col).isRule = true;
        }
    }
}

bool Game::CanMove(std::size_t _row, std::size_t _col, Direction dir)
{
    int row = static_cast<int>(_row);
    int col = static_cast<int>(_col);

    const auto width = static_cast<int>(m_map.GetWidth());
    const auto height = static_cast<int>(m_map.GetHeight());

    int dRow = 0, dCol = 0;
    if (dir == Direction::UP)
    {
        dRow = -1;
    }
    else if (dir == Direction::DOWN)
    {
        dRow = 1;
    }
    else if (dir == Direction::LEFT)
    {
        dCol = -1;
    }
    else if (dir == Direction::RIGHT)
    {
        dCol = 1;
    }

    row += dRow;
    col += dCol;

    // Check boundary
    if (row < 0 || row >= height || col < 0 || col >= width)
    {
        return false;
    }

    const std::vector<ObjectType> types = m_map.At(row, col).GetTypes();

    // Check the icon has property 'STOP'.
    if (m_ruleManager.HasProperty(types, ObjectType::STOP))
    {
        return false;
    }

    if (m_ruleManager.HasProperty(types, ObjectType::PUSH) ||
        m_map.At(row, col).HasTextType())
    {
        if (!CanMove(row, col, dir))
        {
            return false;
        }
    }

    return true;
}

void Game::ProcessMove(std::size_t _row, std::size_t _col, Direction dir,
                       ObjectType type)
{
    int row = static_cast<int>(_row);
    int col = static_cast<int>(_col);

    int dRow = 0, dCol = 0;
    if (dir == Direction::UP)
    {
        dRow = -1;
    }
    else if (dir == Direction::DOWN)
    {
        dRow = 1;
    }
    else if (dir == Direction::LEFT)
    {
        dCol = -1;
    }
    else if (dir == Direction::RIGHT)
    {
        dCol = 1;
    }

    row += dRow;
    col += dCol;

    const std::vector<ObjectType> types = m_map.At(row, col).GetTypes();

    if (m_ruleManager.HasProperty(types, ObjectType::PUSH))
    {
        auto rules = m_ruleManager.GetRules(ObjectType::PUSH);

        for (auto& rule : rules)
        {
            const ObjectType nounType = std::get<0>(rule.objects).GetTypes()[0];
            ProcessMove(row, col, dir, ConvertTextToIcon(nounType));
        }
    }
    else if (m_map.At(row, col).HasTextType())
    {
        ProcessMove(row, col, dir, types[0]);
    }

    m_map.AddObject(row, col, type);
    m_map.RemoveObject(_row, _col, type);
}

void Game::CheckPlayState()
{
    const auto youRules = m_ruleManager.GetRules(ObjectType::YOU);
    if (youRules.empty())
    {
        m_playState = PlayState::LOST;
        return;
    }

    auto winRules = m_ruleManager.GetRules(ObjectType::WIN);
    auto positions = m_map.GetPositions(m_playerIcon);
    for (auto& pos : positions)
    {
        for (auto& rule : winRules)
        {
            const ObjectType type = std::get<0>(rule.objects).GetTypes()[0];

            if (m_map.At(pos.first, pos.second)
                    .HasType(ConvertTextToIcon(type)))
            {
                m_playState = PlayState::WON;
            }
        }
    }
}
}  // namespace baba_is_auto