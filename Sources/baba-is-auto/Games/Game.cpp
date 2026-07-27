// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <baba-is-auto/Games/Game.hpp>

#include <algorithm>

namespace baba_is_auto
{
Game::Game(std::string_view filename)
{
    m_map.Load(filename);

    ParseRules();

    m_playState = PlayState::PLAYING;
}

void Game::Reset()
{
    m_map.Reset();

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
    if (dir == Direction::NONE)
    {
        return;
    }

    std::vector<Position> positions;

    for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
    {
        for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
        {
            const auto hasPlayer =
                std::any_of(m_playerIcons.begin(), m_playerIcons.end(),
                            [this, x, y](ObjectType type) {
                                return m_map.At(x, y).HasType(type);
                            });

            if (hasPlayer)
            {
                positions.emplace_back(x, y);
            }
        }
    }

    std::sort(positions.begin(), positions.end(),
              [dir](const Position& lhs, const Position& rhs) {
                  if (dir == Direction::LEFT)
                  {
                      return lhs.first < rhs.first;
                  }
                  if (dir == Direction::RIGHT)
                  {
                      return lhs.first > rhs.first;
                  }
                  if (dir == Direction::UP)
                  {
                      return lhs.second < rhs.second;
                  }

                  return lhs.second > rhs.second;
              });

    for (const auto& [x, y] : positions)
    {
        std::vector<ObjectType> playerTypes;

        for (const ObjectType type : m_map.At(x, y).GetTypes())
        {
            if (std::find(m_playerIcons.begin(), m_playerIcons.end(), type) !=
                m_playerIcons.end())
            {
                playerTypes.emplace_back(type);
            }
        }

        if (!playerTypes.empty() && CanMove(x, y, dir))
        {
            ProcessMove(x, y, dir, playerTypes);
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
            m_map.At(x, y).isRule = false;
        }
    }

    for (std::size_t y = 0; y < height; ++y)
    {
        for (std::size_t x = 0; x < width; ++x)
        {
            ParseRule(x, y, RuleDirection::HORIZONTAL);
            ParseRule(x, y, RuleDirection::VERTICAL);
        }
    }

    m_playerIcons.clear();

    for (const Rule& rule : m_ruleManager.GetRules(ObjectType::YOU))
    {
        for (const ObjectType type : std::get<0>(rule.objects).GetTypes())
        {
            const ObjectType icon = ConvertTextToIcon(type);

            if (IsNounType(type) &&
                std::find(m_playerIcons.begin(), m_playerIcons.end(), icon) ==
                    m_playerIcons.end())
            {
                m_playerIcons.emplace_back(icon);
            }
        }
    }

    m_playerIcon =
        m_playerIcons.empty() ? ObjectType::ICON_EMPTY : m_playerIcons.front();
}

void Game::ParseRule(std::size_t x, std::size_t y, RuleDirection direction)
{
    const std::size_t width = m_map.GetWidth();
    const std::size_t height = m_map.GetHeight();

    if (direction == RuleDirection::HORIZONTAL)
    {
        if (x + 2 >= width)
        {
            return;
        }

        if (m_map.At(x, y).HasNounType() && m_map.At(x + 1, y).HasVerbType() &&
            (m_map.At(x + 2, y).HasNounType() ||
             m_map.At(x + 2, y).HasPropertyType()))
        {
            m_ruleManager.AddRule(
                { m_map.At(x, y), m_map.At(x + 1, y), m_map.At(x + 2, y) });

            m_map.At(x, y).isRule = true;
            m_map.At(x + 1, y).isRule = true;
            m_map.At(x + 2, y).isRule = true;
        }
    }
    else if (direction == RuleDirection::VERTICAL)
    {
        if (y + 2 >= height)
        {
            return;
        }

        if (m_map.At(x, y).HasNounType() && m_map.At(x, y + 1).HasVerbType() &&
            (m_map.At(x, y + 2).HasNounType() ||
             m_map.At(x, y + 2).HasPropertyType()))
        {
            m_ruleManager.AddRule(
                { m_map.At(x, y), m_map.At(x, y + 1), m_map.At(x, y + 2) });

            m_map.At(x, y).isRule = true;
            m_map.At(x, y + 1).isRule = true;
            m_map.At(x, y + 2).isRule = true;
        }
    }
}

bool Game::CanMove(std::size_t x, std::size_t y, Direction dir)
{
    int _x = static_cast<int>(x);
    int _y = static_cast<int>(y);

    const auto width = static_cast<int>(m_map.GetWidth());
    const auto height = static_cast<int>(m_map.GetHeight());

    int dx = 0, dy = 0;
    if (dir == Direction::UP)
    {
        dy = -1;
    }
    else if (dir == Direction::DOWN)
    {
        dy = 1;
    }
    else if (dir == Direction::LEFT)
    {
        dx = -1;
    }
    else if (dir == Direction::RIGHT)
    {
        dx = 1;
    }

    _x += dx;
    _y += dy;

    // Check boundary
    if (_x < 0 || _x >= width || _y < 0 || _y >= height)
    {
        return false;
    }

    const std::vector<ObjectType> types = m_map.At(_x, _y).GetTypes();

    // Check the icon has property 'STOP'.
    if (m_ruleManager.HasProperty(types, ObjectType::STOP))
    {
        return false;
    }

    if (m_ruleManager.HasProperty(types, ObjectType::PUSH) ||
        m_map.At(_x, _y).HasTextType())
    {
        if (!CanMove(_x, _y, dir))
        {
            return false;
        }
    }

    return true;
}

void Game::ProcessMove(std::size_t x, std::size_t y, Direction dir,
                       const std::vector<ObjectType>& movingTypes)
{
    int _x = static_cast<int>(x);
    int _y = static_cast<int>(y);

    int dx = 0, dy = 0;
    if (dir == Direction::UP)
    {
        dy = -1;
    }
    else if (dir == Direction::DOWN)
    {
        dy = 1;
    }
    else if (dir == Direction::LEFT)
    {
        dx = -1;
    }
    else if (dir == Direction::RIGHT)
    {
        dx = 1;
    }

    _x += dx;
    _y += dy;

    std::vector<ObjectType> destinationTypes = m_map.At(_x, _y).GetTypes();

    if (m_ruleManager.HasProperty(destinationTypes, ObjectType::PUSH) ||
        m_map.At(_x, _y).HasTextType())
    {
        ProcessPush(_x, _y, dir);
        destinationTypes = m_map.At(_x, _y).GetTypes();
    }

    if (m_ruleManager.HasProperty(destinationTypes, ObjectType::SINK) ||
        m_ruleManager.HasProperty(destinationTypes, ObjectType::DEFEAT))
    {
        for (const ObjectType type : movingTypes)
        {
            m_map.RemoveObject(x, y, type);
        }

        return;
    }

    for (const ObjectType type : movingTypes)
    {
        m_map.AddObject(_x, _y, type);
        m_map.RemoveObject(x, y, type);
    }
}

void Game::ProcessPush(std::size_t x, std::size_t y, Direction dir)
{
    int targetX = static_cast<int>(x);
    int targetY = static_cast<int>(y);

    if (dir == Direction::UP)
    {
        --targetY;
    }
    else if (dir == Direction::DOWN)
    {
        ++targetY;
    }
    else if (dir == Direction::LEFT)
    {
        --targetX;
    }
    else if (dir == Direction::RIGHT)
    {
        ++targetX;
    }

    const std::vector<ObjectType> targetTypes =
        m_map.At(targetX, targetY).GetTypes();
    const bool pushesNext =
        m_ruleManager.HasProperty(targetTypes, ObjectType::PUSH) ||
        m_map.At(targetX, targetY).HasTextType();

    if (pushesNext)
    {
        ProcessPush(targetX, targetY, dir);
    }

    const std::vector<ObjectType> remainingTargetTypes =
        m_map.At(targetX, targetY).GetTypes();
    const bool destroysStack =
        m_ruleManager.HasProperty(remainingTargetTypes, ObjectType::SINK) ||
        m_ruleManager.HasProperty(remainingTargetTypes, ObjectType::DEFEAT);
    const std::vector<ObjectType> sourceTypes = m_map.At(x, y).GetTypes();

    for (const ObjectType pushedType : sourceTypes)
    {
        if (IsTextType(pushedType) ||
            m_ruleManager.HasProperty({ pushedType }, ObjectType::PUSH))
        {
            if (!destroysStack)
            {
                m_map.AddObject(targetX, targetY, pushedType);
            }

            m_map.RemoveObject(x, y, pushedType);
        }
    }
}

void Game::CheckPlayState()
{
    if (m_playerIcons.empty())
    {
        m_playState = PlayState::LOST;
        return;
    }

    bool hasPlayer = false;

    for (const ObjectType playerIcon : m_playerIcons)
    {
        const auto positions = m_map.GetPositions(playerIcon);
        hasPlayer = hasPlayer || !positions.empty();

        for (const auto& pos : positions)
        {
            if (m_ruleManager.HasProperty(
                    m_map.At(pos.first, pos.second).GetTypes(),
                    ObjectType::WIN))
            {
                m_playState = PlayState::WON;
                return;
            }
        }
    }

    if (!hasPlayer)
    {
        m_playState = PlayState::LOST;
    }
}
}  // namespace baba_is_auto
