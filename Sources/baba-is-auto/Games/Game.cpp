// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <baba-is-auto/Games/Game.hpp>

#include <algorithm>
#include <utility>

namespace baba_is_auto
{
namespace
{
std::pair<int, int> MovedPosition(std::size_t x, std::size_t y, Direction dir)
{
    auto movedX = static_cast<int>(x);
    auto movedY = static_cast<int>(y);

    switch (dir)
    {
        case Direction::UP:
            --movedY;
            break;
        case Direction::DOWN:
            ++movedY;
            break;
        case Direction::LEFT:
            --movedX;
            break;
        case Direction::RIGHT:
            ++movedX;
            break;
        case Direction::NONE:
            break;
    }

    return { movedX, movedY };
}

bool HasIconProperty(RuleManager& rules, const std::vector<ObjectType>& types,
                     ObjectType property)
{
    return std::any_of(
        types.begin(), types.end(), [&rules, &property](ObjectType type) {
            return !IsTextType(type) && rules.HasProperty({ type }, property);
        });
}
}  // namespace

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

    ProcessSink();
    ProcessHotMelt();
    ProcessDefeat();

    CheckPlayState();
    ParseRules();
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

    const bool horizontal = direction == RuleDirection::HORIZONTAL;
    const std::size_t remaining = horizontal ? width - x : height - y;

    const auto At = [this, &x, &y, &horizontal](std::size_t offset) -> Object& {
        return horizontal ? m_map.At(x + offset, y) : m_map.At(x, y + offset);
    };

    if (remaining < 3 || !At(0).HasNounType() ||
        ((horizontal ? x : y) > 1 &&
         (horizontal ? m_map.At(x - 1, y) : m_map.At(x, y - 1))
             .HasType(ObjectType::AND) &&
         !(horizontal ? m_map.At(x - 1, y) : m_map.At(x, y - 1))
              .HasVerbType() &&
         (horizontal ? m_map.At(x - 2, y) : m_map.At(x, y - 2)).HasNounType()))
    {
        return;
    }

    std::vector<ObjectType> subjects = At(0).GetTypes();
    std::size_t offset = 1;

    while (offset + 1 < remaining && At(offset).HasType(ObjectType::AND) &&
           !At(offset).HasVerbType() && At(offset + 1).HasNounType())
    {
        const auto types = At(offset + 1).GetTypes();
        subjects.insert(subjects.end(), types.begin(), types.end());
        offset += 2;
    }

    if (offset >= remaining || !At(offset).HasVerbType())
    {
        return;
    }

    const std::size_t verb = offset++;

    if (offset >= remaining ||
        (!At(offset).HasNounType() && !At(offset).HasPropertyType()))
    {
        return;
    }

    std::vector<ObjectType> predicates = At(offset++).GetTypes();

    while (offset + 1 < remaining && At(offset).HasType(ObjectType::AND) &&
           (At(offset + 1).HasNounType() || At(offset + 1).HasPropertyType()))
    {
        const auto types = At(offset + 1).GetTypes();
        predicates.insert(predicates.end(), types.begin(), types.end());
        offset += 2;
    }

    for (const ObjectType subject : subjects)
    {
        if (!IsNounType(subject))
        {
            continue;
        }

        for (const ObjectType op : At(verb).GetTypes())
        {
            if (!IsVerbType(op))
            {
                continue;
            }

            for (const ObjectType predicate : predicates)
            {
                if (IsNounType(predicate) || IsPropertyType(predicate))
                {
                    m_ruleManager.AddRule(
                        { Object({ subject }), Object({ op }),
                          Object({ predicate }) });
                }
            }
        }
    }

    for (std::size_t i = 0; i < offset; ++i)
    {
        At(i).isRule = true;
    }
}

bool Game::CanMove(std::size_t x, std::size_t y, Direction dir)
{
    const auto [_x, _y] = MovedPosition(x, y, dir);

    const auto width = static_cast<int>(m_map.GetWidth());
    const auto height = static_cast<int>(m_map.GetHeight());

    // Check boundary
    if (_x < 0 || _x >= width || _y < 0 || _y >= height)
    {
        return false;
    }

    const std::vector<ObjectType> types = m_map.At(_x, _y).GetTypes();

    // Check the icon has property 'STOP'.
    if (HasIconProperty(m_ruleManager, types, ObjectType::STOP))
    {
        return false;
    }

    if (HasIconProperty(m_ruleManager, types, ObjectType::PUSH) ||
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
    const auto [_x, _y] = MovedPosition(x, y, dir);

    if (const std::vector<ObjectType> destinationTypes =
            m_map.At(_x, _y).GetTypes();
        HasIconProperty(m_ruleManager, destinationTypes, ObjectType::PUSH) ||
        m_map.At(_x, _y).HasTextType())
    {
        ProcessPush(_x, _y, dir);
    }

    for (const ObjectType type : movingTypes)
    {
        m_map.AddObject(_x, _y, type);
        m_map.RemoveObject(x, y, type);
    }
}

void Game::ProcessPush(std::size_t x, std::size_t y, Direction dir)
{
    const auto [targetX, targetY] = MovedPosition(x, y, dir);
    const std::vector<ObjectType> targetTypes =
        m_map.At(targetX, targetY).GetTypes();

    if (const bool pushesNext =
            HasIconProperty(m_ruleManager, targetTypes, ObjectType::PUSH) ||
            m_map.At(targetX, targetY).HasTextType();
        pushesNext)
    {
        ProcessPush(targetX, targetY, dir);
    }

    const std::vector<ObjectType> sourceTypes = m_map.At(x, y).GetTypes();

    for (const ObjectType pushedType : sourceTypes)
    {
        if (IsTextType(pushedType) ||
            m_ruleManager.HasProperty({ pushedType }, ObjectType::PUSH))
        {
            m_map.AddObject(targetX, targetY, pushedType);
            m_map.RemoveObject(x, y, pushedType);
        }
    }
}

void Game::ProcessSink()
{
    for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
    {
        for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
        {
            const std::vector<ObjectType> types = m_map.At(x, y).GetTypes();

            if (types.size() < 2 ||
                !HasIconProperty(m_ruleManager, types, ObjectType::SINK))
            {
                continue;
            }

            for (const ObjectType type : types)
            {
                m_map.RemoveObject(x, y, type);
            }
        }
    }
}

void Game::ProcessHotMelt()
{
    for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
    {
        for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
        {
            std::vector<ObjectType> iconTypes;

            for (const ObjectType type : m_map.At(x, y).GetTypes())
            {
                if (!IsTextType(type))
                {
                    iconTypes.emplace_back(type);
                }
            }

            if (!m_ruleManager.HasProperty(iconTypes, ObjectType::HOT) ||
                !m_ruleManager.HasProperty(iconTypes, ObjectType::MELT))
            {
                continue;
            }

            for (const ObjectType type : iconTypes)
            {
                if (m_ruleManager.HasProperty({ type }, ObjectType::MELT))
                {
                    m_map.RemoveObject(x, y, type);
                }
            }
        }
    }
}

void Game::ProcessDefeat()
{
    for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
    {
        for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
        {
            const std::vector<ObjectType> types =
                m_map.At(x, y).GetTypes();

            if (!HasIconProperty(m_ruleManager, types, ObjectType::DEFEAT))
            {
                continue;
            }

            for (const ObjectType type : types)
            {
                if (std::find(m_playerIcons.begin(), m_playerIcons.end(),
                              type) != m_playerIcons.end())
                {
                    m_map.RemoveObject(x, y, type);
                }
            }
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

        for (const auto& [x, y] : positions)
        {
            if (HasIconProperty(m_ruleManager, m_map.At(x, y).GetTypes(),
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
