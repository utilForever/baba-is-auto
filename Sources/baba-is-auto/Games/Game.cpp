// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <baba-is-auto/Games/Game.hpp>

#include <algorithm>
#include <array>
#include <iterator>
#include <optional>
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

constexpr bool IsRulePredicate(ObjectType type)
{
    return IsNounType(type) || IsPropertyType(type);
}

constexpr bool IsInfixCondition(ObjectType type)
{
    return type == ObjectType::ON || type == ObjectType::NEAR ||
           type == ObjectType::FACING;
}

std::optional<ObjectType> FindType(const Object& object,
                                   bool (*predicate)(ObjectType))
{
    for (const ObjectType type : object.GetTypes())
    {
        if (predicate(type))
        {
            return type;
        }
    }

    return std::nullopt;
}

constexpr bool IsDirectionType(ObjectType type)
{
    return type == ObjectType::UP || type == ObjectType::DOWN ||
           type == ObjectType::LEFT || type == ObjectType::RIGHT;
}

constexpr Direction ToDirection(ObjectType type)
{
    return type == ObjectType::UP      ? Direction::UP
           : type == ObjectType::DOWN  ? Direction::DOWN
           : type == ObjectType::LEFT  ? Direction::LEFT
           : type == ObjectType::RIGHT ? Direction::RIGHT
                                       : Direction::NONE;
}

constexpr Direction Opposite(Direction direction)
{
    switch (direction)
    {
        case Direction::UP:
            return Direction::DOWN;
        case Direction::DOWN:
            return Direction::UP;
        case Direction::LEFT:
            return Direction::RIGHT;
        case Direction::RIGHT:
            return Direction::LEFT;
        case Direction::NONE:
            return Direction::NONE;
    }

    return Direction::NONE;
}

constexpr std::size_t DirectionIndex(Direction direction)
{
    return direction == Direction::UP      ? 0
           : direction == Direction::RIGHT ? 1
           : direction == Direction::DOWN  ? 2
                                           : 3;
}

constexpr ObjectType LockedProperty(Direction direction)
{
    switch (direction)
    {
        case Direction::UP:
            return ObjectType::LOCKED_UP;
        case Direction::DOWN:
            return ObjectType::LOCKED_DOWN;
        case Direction::LEFT:
            return ObjectType::LOCKED_LEFT;
        case Direction::RIGHT:
            return ObjectType::LOCKED_RIGHT;
        case Direction::NONE:
            return ObjectType::ICON_EMPTY;
    }

    return ObjectType::ICON_EMPTY;
}

constexpr bool IsAllNoun(ObjectType type)
{
    const ObjectType noun = ConvertIconToText(type);
    return IsNounType(noun) && noun != ObjectType::TEXT &&
           noun != ObjectType::EMPTY && noun != ObjectType::LEVEL;
}

constexpr bool IsSpawnableAllNoun(ObjectType type)
{
    const ObjectType noun = ConvertIconToText(type);
    return IsAllNoun(noun) && noun != ObjectType::ALL &&
           noun != ObjectType::GROUP;
}

constexpr bool SubjectMatches(ObjectType subject, ObjectType type)
{
    if (subject == ObjectType::TEXT)
    {
        return IsTextType(type);
    }

    if (subject == ObjectType::ALL)
    {
        return IsIconType(type) && IsAllNoun(type);
    }

    return ConvertTextToIcon(subject) == type;
}

template <typename Predicate>
void RetainTypes(std::vector<ObjectType>& types, Predicate predicate)
{
    types.erase(std::remove_if(
                    types.begin(), types.end(),
                    [predicate](ObjectType type) { return !predicate(type); }),
                types.end());
}

void AddRuleCombinations(RuleManager& rules,
                         const std::vector<ObjectType>& subjects,
                         const std::vector<ObjectType>& verbs,
                         const std::vector<ObjectType>& predicates,
                         const std::vector<RuleCondition>& conditions)
{
    for (const ObjectType subject : subjects)
    {
        for (const ObjectType verb : verbs)
        {
            for (const ObjectType predicate : predicates)
            {
                rules.AddRule({ Object({ subject }), Object({ verb }),
                                Object({ predicate }), conditions });
            }
        }
    }
}
}  // namespace

Game::Game(std::string_view filename)
{
    m_map.Load(filename);

    UpdateAllNouns();
    ParseRules();
    ProcessDirectionProperties();

    m_playState = PlayState::PLAYING;
}

void Game::Reset()
{
    m_map.Reset();
    m_allNouns.clear();

    UpdateAllNouns();
    ParseRules();
    ProcessDirectionProperties();

    m_playState = PlayState::PLAYING;
}

void Game::SetRandomSeed(std::uint32_t seed)
{
    m_randomEngine.seed(seed);
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
    m_map.AssignMissingObjectIDs();

    if (dir != Direction::NONE)
    {
        ProcessPlayerMove(dir);
    }

    ParseRules();
    ProcessDirectionProperties();
    ProcessMoveProperty();
    ParseRules();
    ProcessTransformations();
    ParseRules();

    ProcessSink();
    ProcessWeak();
    ProcessHotMelt();
    ProcessDefeat();

    CheckPlayState();
    ParseRules();
}

void Game::ProcessPlayerMove(Direction dir)
{
    using PlayerStack = std::pair<Position, std::vector<ObjectID>>;

    std::vector<PlayerStack> players;

    for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
    {
        for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
        {
            std::vector<ObjectID> ids;

            for (const ObjectInstance& instance : m_map.At(x, y).GetInstances())
            {
                if (HasProperty(instance, ObjectType::YOU))
                {
                    ids.emplace_back(instance.id);
                }
            }

            if (!ids.empty())
            {
                players.push_back({ { x, y }, ids });
            }
        }
    }

    std::sort(players.begin(), players.end(),
              [dir](const PlayerStack& lhs, const PlayerStack& rhs) {
                  const Position& lhsPosition = lhs.first;
                  const Position& rhsPosition = rhs.first;

                  if (dir == Direction::LEFT)
                  {
                      return lhsPosition.first < rhsPosition.first;
                  }

                  if (dir == Direction::RIGHT)
                  {
                      return lhsPosition.first > rhsPosition.first;
                  }

                  if (dir == Direction::UP)
                  {
                      return lhsPosition.second < rhsPosition.second;
                  }

                  return lhsPosition.second > rhsPosition.second;
              });

    for (const auto& [position, playerIDs] : players)
    {
        std::vector<ObjectID> movableIDs;

        for (const ObjectID id : playerIDs)
        {
            m_map.SetDirection(id, dir);

            const ObjectInstance* instance = m_map.GetInstance(id);

            if (instance != nullptr &&
                !HasProperty(*instance, LockedProperty(dir)))
            {
                movableIDs.emplace_back(id);
            }
        }

        if (movableIDs.empty())
        {
            continue;
        }

        if (CanMove(position.first, position.second, dir))
        {
            ProcessMove(position.first, position.second, dir, movableIDs);
        }
        else
        {
            for (const ObjectID id : movableIDs)
            {
                const ObjectInstance* instance = m_map.GetInstance(id);

                if (instance != nullptr &&
                    HasProperty(*instance, ObjectType::WEAK))
                {
                    m_map.RemoveObject(id);
                }
            }
        }
    }
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

    for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
    {
        for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
        {
            for (const ObjectInstance& instance : m_map.At(x, y).GetInstances())
            {
                if (HasProperty(instance, ObjectType::YOU) &&
                    std::find(m_playerIcons.begin(), m_playerIcons.end(),
                              instance.type) == m_playerIcons.end())
                {
                    m_playerIcons.emplace_back(instance.type);
                }
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
    const auto Before = [this, &x, &y,
                         &horizontal](std::size_t offset) -> Object& {
        return horizontal ? m_map.At(x - offset, y) : m_map.At(x, y - offset);
    };

    if (remaining < 3)
    {
        return;
    }

    const std::size_t coordinate = horizontal ? x : y;

    if (coordinate > 0 && (Before(1).HasType(ObjectType::LONELY) ||
                           FindType(Before(1), IsInfixCondition) ||
                           (At(0).HasType(ObjectType::LONELY) &&
                            Before(1).HasType(ObjectType::NOT))))
    {
        return;
    }

    const bool continuesPrevious =
        coordinate > 1 && Before(1).HasType(ObjectType::AND) &&
        !Before(1).HasVerbType() && Before(2).HasNounType();

    if (continuesPrevious)
    {
        return;
    }

    std::vector<RuleCondition> conditions;
    std::size_t offset = 0;

    if (At(offset).HasType(ObjectType::LONELY))
    {
        conditions.push_back({ ObjectType::LONELY, {}, false });
        ++offset;
    }
    else if (At(offset).HasType(ObjectType::NOT) && remaining > 1 &&
             At(offset + 1).HasType(ObjectType::LONELY))
    {
        conditions.push_back({ ObjectType::LONELY, {}, true });
        offset += 2;
    }

    if (offset >= remaining || !At(offset).HasNounType())
    {
        return;
    }

    std::vector<ObjectType> subjects = At(offset).GetTypes();
    ++offset;

    while (offset + 1 < remaining && At(offset).HasType(ObjectType::AND) &&
           !At(offset).HasVerbType() && At(offset + 1).HasNounType())
    {
        const auto types = At(offset + 1).GetTypes();
        subjects.insert(subjects.end(), types.begin(), types.end());
        offset += 2;
    }

    while (offset < remaining)
    {
        bool negated = false;

        if (At(offset).HasType(ObjectType::NOT) && offset + 1 < remaining &&
            FindType(At(offset + 1), IsInfixCondition))
        {
            negated = true;
            ++offset;
        }

        const auto condition = FindType(At(offset), IsInfixCondition);

        if (!condition)
        {
            break;
        }

        ++offset;

        if (offset >= remaining)
        {
            return;
        }

        std::vector<ObjectType> targets = At(offset).GetTypes();

        RetainTypes(targets, [condition](ObjectType type) {
            return IsNounType(type) ||
                   (*condition == ObjectType::FACING && IsDirectionType(type));
        });

        if (targets.empty())
        {
            return;
        }

        ++offset;

        while (offset + 1 < remaining && At(offset).HasType(ObjectType::AND))
        {
            if (FindType(At(offset + 1), IsInfixCondition) ||
                (At(offset + 1).HasType(ObjectType::NOT) &&
                 offset + 2 < remaining &&
                 FindType(At(offset + 2), IsInfixCondition)))
            {
                ++offset;
                break;
            }

            std::vector<ObjectType> moreTargets = At(offset + 1).GetTypes();

            RetainTypes(moreTargets, [condition](ObjectType type) {
                return IsNounType(type) || (*condition == ObjectType::FACING &&
                                            IsDirectionType(type));
            });

            if (moreTargets.empty())
            {
                break;
            }

            targets.insert(targets.end(), moreTargets.begin(),
                           moreTargets.end());
            offset += 2;
        }

        conditions.push_back({ *condition, std::move(targets), negated });
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

    std::vector<ObjectType> verbs = At(verb).GetTypes();
    RetainTypes(subjects, IsNounType);
    RetainTypes(verbs, IsVerbType);
    RetainTypes(predicates, IsRulePredicate);
    AddRuleCombinations(m_ruleManager, subjects, verbs, predicates, conditions);

    for (std::size_t i = 0; i < offset; ++i)
    {
        At(i).isRule = true;
    }
}

bool Game::HasProperty(const ObjectInstance& instance,
                       ObjectType property) const
{
    const auto position = m_map.GetPosition(instance.id);
    return position && HasPropertyAtPosition(instance, *position, property);
}

bool Game::HasPropertyAtPosition(const ObjectInstance& instance,
                                 const Position& position,
                                 ObjectType property) const
{
    const ObjectInstance atPosition =
        instance.type == ObjectType::ICON_EMPTY ? EmptyAt(position) : instance;

    for (const Rule& rule : m_ruleManager.GetRules(property))
    {
        if (!std::get<2>(rule.objects).HasType(property))
        {
            continue;
        }

        for (const ObjectType subject : std::get<0>(rule.objects).GetTypes())
        {
            if (SubjectMatches(subject, atPosition.type) &&
                MatchesConditionsAt(atPosition, position, rule.conditions))
            {
                return true;
            }
        }
    }

    return false;
}

bool Game::HasPropertyAt(std::size_t x, std::size_t y,
                         ObjectType property) const
{
    const Position position{ x, y };
    const auto& instances = m_map.At(x, y).GetInstances();

    return std::any_of(
        instances.begin(), instances.end(),
        [this, &position, property](const ObjectInstance& instance) {
            return HasPropertyAtPosition(instance, position, property);
        });
}

bool Game::MatchesConditions(const ObjectInstance& instance,
                             const std::vector<RuleCondition>& conditions) const
{
    const auto position = m_map.GetPosition(instance.id);
    return position && MatchesConditionsAt(instance, *position, conditions);
}

bool Game::MatchesConditionsAt(
    const ObjectInstance& instance, const Position& position,
    const std::vector<RuleCondition>& conditions) const
{
    return std::all_of(
        conditions.begin(), conditions.end(),
        [this, &instance, &position](const RuleCondition& condition) {
            return MatchesConditionAt(instance, position, condition);
        });
}

bool Game::MatchesConditionAt(const ObjectInstance& instance,
                              const Position& position,
                              const RuleCondition& condition) const
{
    const auto [x, y] = position;
    bool matches;

    if (condition.op == ObjectType::LONELY)
    {
        matches = m_map.At(x, y).GetInstances().size() == 1;
    }
    else
    {
        std::vector<const ObjectInstance*> candidates;

        const auto AddCandidates = [this, &candidates, &instance, _x = x,
                                    _y = y](int candidateX, int candidateY) {
            if (candidateX < 0 || candidateY < 0 ||
                candidateX >= static_cast<int>(m_map.GetWidth()) ||
                candidateY >= static_cast<int>(m_map.GetHeight()))
            {
                return;
            }

            for (const ObjectInstance& candidate :
                 m_map
                     .At(static_cast<std::size_t>(candidateX),
                         static_cast<std::size_t>(candidateY))
                     .GetInstances())
            {
                if (candidate.id != instance.id ||
                    candidateX != static_cast<int>(_x) ||
                    candidateY != static_cast<int>(_y))
                {
                    candidates.emplace_back(&candidate);
                }
            }
        };

        if (condition.op == ObjectType::ON)
        {
            AddCandidates(static_cast<int>(x), static_cast<int>(y));
        }
        else if (condition.op == ObjectType::NEAR)
        {
            for (int dy = -1; dy <= 1; ++dy)
            {
                for (int dx = -1; dx <= 1; ++dx)
                {
                    AddCandidates(static_cast<int>(x) + dx,
                                  static_cast<int>(y) + dy);
                }
            }
        }
        else if (condition.op == ObjectType::FACING)
        {
            const auto [candidateX, candidateY] =
                MovedPosition(x, y, instance.direction);
            AddCandidates(candidateX, candidateY);
        }

        std::vector<ObjectType> targets;

        for (const ObjectType target : condition.targets)
        {
            if (target == ObjectType::ALL)
            {
                std::copy_if(m_allNouns.begin(), m_allNouns.end(),
                             std::back_inserter(targets), IsSpawnableAllNoun);
            }
            else
            {
                targets.emplace_back(target);
            }
        }

        std::vector<const ObjectInstance*> used;

        matches = std::all_of(
            targets.begin(), targets.end(),
            [&instance, &candidates, &used](ObjectType target) {
                if (target == ObjectType::UP || target == ObjectType::DOWN ||
                    target == ObjectType::LEFT || target == ObjectType::RIGHT)
                {
                    return instance.direction == ToDirection(target);
                }

                const auto found = std::find_if(
                    candidates.begin(), candidates.end(),
                    [target, &used](const ObjectInstance* candidate) {
                        return SubjectMatches(target, candidate->type) &&
                               std::find(used.begin(), used.end(), candidate) ==
                                   used.end();
                    });

                if (found == candidates.end())
                {
                    return false;
                }

                used.emplace_back(*found);

                return true;
            });
    }

    return condition.negated ? !matches : matches;
}

ObjectInstance Game::EmptyAt(const Position& position) const
{
    ObjectInstance empty{ 0, ObjectType::ICON_EMPTY, Direction::NONE };

    for (const Rule& rule : m_ruleManager.GetRules(ObjectType::IS))
    {
        const auto subjects = std::get<0>(rule.objects).GetTypes();

        if (!std::get<1>(rule.objects).HasType(ObjectType::IS) ||
            !std::any_of(subjects.begin(), subjects.end(),
                         [&empty](ObjectType subject) {
                             return SubjectMatches(subject, empty.type);
                         }) ||
            !MatchesConditionsAt(empty, position, rule.conditions))
        {
            continue;
        }

        for (const ObjectType predicate : std::get<2>(rule.objects).GetTypes())
        {
            if (IsDirectionType(predicate))
            {
                empty.direction = ToDirection(predicate);
            }
        }
    }

    return empty;
}

void Game::ProcessDirectionProperties()
{
    constexpr std::array directions = { Direction::UP, Direction::RIGHT,
                                        Direction::DOWN, Direction::LEFT };
    const auto rules = m_ruleManager.GetRules(ObjectType::IS);
    std::vector<ObjectID> ids;

    for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
    {
        for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
        {
            for (const ObjectInstance& instance : m_map.At(x, y).GetInstances())
            {
                if (instance.type != ObjectType::ICON_EMPTY)
                {
                    ids.emplace_back(instance.id);
                }
            }
        }
    }

    for (const ObjectID id : ids)
    {
        ObjectInstance* instance = m_map.GetInstance(id);

        if (instance == nullptr)
        {
            continue;
        }

        std::array<std::size_t, 4> counts{};

        for (const Rule& rule : rules)
        {
            const auto subjects = std::get<0>(rule.objects).GetTypes();

            if (!std::get<1>(rule.objects).HasType(ObjectType::IS) ||
                !std::any_of(subjects.begin(), subjects.end(),
                             [instance](ObjectType subject) {
                                 return SubjectMatches(subject, instance->type);
                             }) ||
                !MatchesConditions(*instance, rule.conditions))
            {
                continue;
            }

            for (const ObjectType predicate :
                 std::get<2>(rule.objects).GetTypes())
            {
                if (IsDirectionType(predicate))
                {
                    ++counts[DirectionIndex(ToDirection(predicate))];
                }
            }
        }

        const std::size_t highest =
            *std::max_element(counts.begin(), counts.end());

        if (highest == 0)
        {
            continue;
        }

        const std::size_t current = instance->direction == Direction::NONE
                                        ? directions.size() - 1
                                        : DirectionIndex(instance->direction);

        for (std::size_t offset = 1; offset <= directions.size(); ++offset)
        {
            const std::size_t index = (current + offset) % directions.size();

            if (counts[index] == highest)
            {
                instance->direction = directions[index];
                break;
            }
        }
    }
}

void Game::ProcessMoveProperty()
{
    struct MoveState
    {
        ObjectID id = 0;
        Position position{};
        Direction direction = Direction::NONE;
        std::size_t attempts = 0;
        bool turned = false;
        bool active = true;
    };

    const auto rules = m_ruleManager.GetRules(ObjectType::MOVE);
    std::vector<MoveState> moving;
    std::size_t rounds = 0;

    for (const Rule& rule : rules)
    {
        if (!std::get<2>(rule.objects).HasType(ObjectType::MOVE))
        {
            continue;
        }

        const auto subjects = std::get<0>(rule.objects).GetTypes();
        std::vector<ObjectID> matching;

        for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
        {
            for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
            {
                for (const ObjectInstance& instance :
                     m_map.At(x, y).GetInstances())
                {
                    if (instance.type == ObjectType::ICON_EMPTY ||
                        !MatchesConditions(instance, rule.conditions) ||
                        !std::any_of(subjects.begin(), subjects.end(),
                                     [&instance](ObjectType subject) {
                                         return SubjectMatches(subject,
                                                               instance.type);
                                     }))
                    {
                        continue;
                    }

                    matching.emplace_back(instance.id);
                }
            }
        }

        std::sort(matching.begin(), matching.end());

        for (const ObjectID id : matching)
        {
            auto found = std::find_if(
                moving.begin(), moving.end(),
                [id](const MoveState& state) { return state.id == id; });

            if (found == moving.end())
            {
                moving.push_back({ id });
                found = std::prev(moving.end());
            }

            rounds = std::max(rounds, ++found->attempts);
        }

        if (std::any_of(
                subjects.begin(), subjects.end(), [](ObjectType subject) {
                    return SubjectMatches(subject, ObjectType::ICON_EMPTY);
                }))
        {
            for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
            {
                for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
                {
                    if (!m_map.At(x, y).HasType(ObjectType::ICON_EMPTY))
                    {
                        continue;
                    }

                    const Position position{ x, y };
                    const ObjectInstance empty = EmptyAt(position);

                    if (!MatchesConditionsAt(empty, position, rule.conditions))
                    {
                        continue;
                    }

                    auto found = std::find_if(
                        moving.begin(), moving.end(),
                        [&position](const MoveState& state) {
                            return state.id == 0 && state.position == position;
                        });

                    if (found == moving.end())
                    {
                        moving.push_back({ 0, position, empty.direction });
                        found = std::prev(moving.end());
                    }

                    rounds = std::max(rounds, ++found->attempts);
                }
            }
        }
    }

    for (std::size_t round = 0; round < rounds; ++round)
    {
        for (std::size_t index = 0; index < moving.size(); ++index)
        {
            MoveState& state = moving[index];

            if (!state.active || round >= state.attempts)
            {
                continue;
            }

            if (state.id == 0)
            {
                if (!m_map.At(state.position.first, state.position.second)
                         .HasType(ObjectType::ICON_EMPTY))
                {
                    continue;
                }

                if (state.direction == Direction::NONE)
                {
                    state.direction = RandomDirection();
                }

                const Direction direction = state.direction;

                if (!state.turned &&
                    HasPropertyAt(state.position.first, state.position.second,
                                  LockedProperty(direction)))
                {
                    continue;
                }

                if (CanMove(state.position.first, state.position.second,
                            direction))
                {
                    ProcessMove(state.position.first, state.position.second,
                                direction, {});

                    state.position = MovedPosition(
                        state.position.first, state.position.second, direction);

                    for (std::size_t duplicateIndex = 0;
                         duplicateIndex < moving.size(); ++duplicateIndex)
                    {
                        MoveState& duplicate = moving[duplicateIndex];

                        if (duplicateIndex == index || !duplicate.active ||
                            duplicate.id != 0 ||
                            duplicate.position != state.position)
                        {
                            continue;
                        }

                        if (duplicateIndex < index)
                        {
                            duplicate.attempts =
                                std::max(duplicate.attempts, state.attempts);
                            state.active = false;
                        }
                        else
                        {
                            state.attempts =
                                std::max(state.attempts, duplicate.attempts);
                            duplicate.active = false;
                        }

                        break;
                    }
                }
                else if (!state.turned)
                {
                    state.direction = Opposite(direction);
                    state.turned = true;
                }

                continue;
            }

            const ObjectInstance* instance = m_map.GetInstance(state.id);
            const auto position = m_map.GetPosition(state.id);

            if (instance == nullptr || !position)
            {
                continue;
            }

            const Direction direction = instance->direction;

            if (direction == Direction::NONE)
            {
                continue;
            }

            if (!state.turned &&
                HasProperty(*instance, LockedProperty(direction)))
            {
                continue;
            }

            if (CanMove(position->first, position->second, direction))
            {
                ProcessMove(position->first, position->second, direction,
                            { state.id });
            }
            else if (!state.turned)
            {
                m_map.SetDirection(state.id, Opposite(direction));
                state.turned = true;
            }
            else if (HasProperty(*instance, ObjectType::WEAK))
            {
                m_map.RemoveObject(state.id);
            }
        }
    }
}

void Game::ProcessTransformations()
{
    struct Transformation
    {
        ObjectType subject;
        ObjectType predicate;
        std::vector<RuleCondition> conditions;
    };

    struct Result
    {
        bool matched = false;
        bool transforms = false;
        bool expandsAll = false;
        bool protectedByIdentity = false;
        std::vector<ObjectType> targets;
        std::vector<ObjectType> allTargets;
    };

    UpdateAllNouns();

    std::vector<Transformation> transformations;

    for (const Rule& rule : m_ruleManager.GetRules(ObjectType::IS))
    {
        if (!std::get<1>(rule.objects).HasType(ObjectType::IS))
        {
            continue;
        }

        for (const ObjectType subject : std::get<0>(rule.objects).GetTypes())
        {
            if (!IsNounType(subject) || subject == ObjectType::GROUP)
            {
                continue;
            }

            for (const ObjectType predicate :
                 std::get<2>(rule.objects).GetTypes())
            {
                if (!IsNounType(predicate) || predicate == ObjectType::GROUP)
                {
                    continue;
                }

                transformations.push_back(
                    { subject, predicate, rule.conditions });
            }
        }
    }

    const auto Resolve = [this, &transformations](
                             const ObjectInstance& source,
                             const Position* position = nullptr) {
        Result result;

        for (const Transformation& transformation : transformations)
        {
            if (!SubjectMatches(transformation.subject, source.type))
            {
                continue;
            }

            const bool matches =
                position == nullptr
                    ? MatchesConditions(source, transformation.conditions)
                    : MatchesConditionsAt(source, *position,
                                          transformation.conditions);

            if (!matches)
            {
                continue;
            }

            result.matched = true;

            if (transformation.predicate == ObjectType::ALL)
            {
                result.expandsAll = true;

                for (const ObjectType noun : m_allNouns)
                {
                    if (IsSpawnableAllNoun(noun))
                    {
                        result.allTargets.emplace_back(ConvertTextToIcon(noun));
                    }
                }

                continue;
            }

            result.transforms = true;

            const ObjectType target =
                transformation.predicate == ObjectType::TEXT
                    ? (IsTextType(source.type) ? source.type
                                               : ConvertIconToText(source.type))
                    : ConvertTextToIcon(transformation.predicate);

            result.targets.emplace_back(target);
            result.protectedByIdentity |= target == source.type;
        }

        return result;
    };

    std::vector<ObjectInstance> instances;
    std::vector<Position> emptyTiles;

    for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
    {
        for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
        {
            if (m_map.At(x, y).HasType(ObjectType::ICON_EMPTY))
            {
                emptyTiles.emplace_back(x, y);
            }

            for (const ObjectInstance& instance : m_map.At(x, y).GetInstances())
            {
                if (instance.type != ObjectType::ICON_EMPTY)
                {
                    instances.emplace_back(instance);
                }
            }
        }
    }

    std::vector<std::pair<ObjectInstance, Result>> pending;
    pending.reserve(instances.size());

    for (const ObjectInstance& snapshot : instances)
    {
        pending.emplace_back(snapshot, Resolve(snapshot));
    }

    struct EmptyResult
    {
        Position position;
        Direction direction;
        Result result;
    };

    std::vector<EmptyResult> pendingEmpty;
    pendingEmpty.reserve(emptyTiles.size());

    for (const Position& position : emptyTiles)
    {
        const ObjectInstance empty = EmptyAt(position);
        pendingEmpty.push_back(
            { position, empty.direction, Resolve(empty, &position) });
    }

    for (const auto& [snapshot, result] : pending)
    {
        if (!result.matched || result.protectedByIdentity)
        {
            continue;
        }

        const auto position = m_map.GetPosition(snapshot.id);

        if (!position)
        {
            continue;
        }

        bool spawned = false;

        for (const ObjectType target : result.allTargets)
        {
            if (!m_map.At(position->first, position->second).HasType(target))
            {
                m_map.AddGeneratedObject(position->first, position->second,
                                         target, snapshot.direction);
                spawned = true;
            }
        }

        if (result.expandsAll && spawned &&
            (IsTextType(snapshot.type) ||
             snapshot.type == ObjectType::ICON_LEVEL))
        {
            m_map.RemoveObject(snapshot.id);
            continue;
        }

        if (!result.transforms)
        {
            continue;
        }

        std::vector<ObjectType> targets = result.targets;

        targets.erase(
            std::remove(targets.begin(), targets.end(), ObjectType::ICON_EMPTY),
            targets.end());

        if (targets.empty())
        {
            m_map.RemoveObject(snapshot.id);
            continue;
        }

        ObjectInstance* instance = m_map.GetInstance(snapshot.id);

        if (instance == nullptr)
        {
            continue;
        }

        instance->type = targets.front();

        for (auto target = std::next(targets.begin()); target != targets.end();
             ++target)
        {
            m_map.AddGeneratedObject(position->first, position->second, *target,
                                     snapshot.direction);
        }
    }

    for (const auto& [position, direction, result] : pendingEmpty)
    {
        if (!result.matched || result.protectedByIdentity)
        {
            continue;
        }

        for (const ObjectType target : result.allTargets)
        {
            if (!m_map.At(position.first, position.second).HasType(target))
            {
                m_map.AddGeneratedObject(position.first, position.second,
                                         target, Direction::NONE);
            }
        }

        for (const ObjectType target : result.targets)
        {
            if (target != ObjectType::ICON_EMPTY)
            {
                if (direction == Direction::NONE)
                {
                    m_map.AddGeneratedObject(position.first, position.second,
                                             target, RandomDirection());
                }
                else
                {
                    m_map.AddGeneratedObject(position.first, position.second,
                                             target, direction);
                }
            }
        }
    }
}

void Game::UpdateAllNouns()
{
    for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
    {
        for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
        {
            for (const ObjectInstance& instance : m_map.At(x, y).GetInstances())
            {
                const ObjectType noun = ConvertIconToText(instance.type);

                if (!IsAllNoun(noun) ||
                    std::find(m_allNouns.begin(), m_allNouns.end(), noun) !=
                        m_allNouns.end())
                {
                    continue;
                }

                m_allNouns.emplace_back(noun);
            }
        }
    }
}

Direction Game::RandomDirection()
{
    constexpr std::array directions = { Direction::UP, Direction::RIGHT,
                                        Direction::DOWN, Direction::LEFT };
    return directions[m_randomEngine() % directions.size()];
}

bool Game::CanMove(std::size_t x, std::size_t y, Direction dir)
{
    if (dir == Direction::NONE)
    {
        return false;
    }

    const auto [_x, _y] = MovedPosition(x, y, dir);

    const auto width = static_cast<int>(m_map.GetWidth());
    const auto height = static_cast<int>(m_map.GetHeight());

    // Check boundary
    if (_x < 0 || _x >= width || _y < 0 || _y >= height)
    {
        return false;
    }

    const auto& destination =
        m_map.At(static_cast<std::size_t>(_x), static_cast<std::size_t>(_y));
    const auto& instances = destination.GetInstances();
    const Position destinationPosition{ static_cast<std::size_t>(_x),
                                        static_cast<std::size_t>(_y) };

    // Check the icon has property 'STOP'.
    if (std::any_of(
            instances.begin(), instances.end(),
            [this, &destinationPosition](const ObjectInstance& instance) {
                return !IsTextType(instance.type) &&
                       HasPropertyAtPosition(instance, destinationPosition,
                                             ObjectType::STOP) &&
                       !HasPropertyAtPosition(instance, destinationPosition,
                                              ObjectType::PUSH) &&
                       !HasPropertyAtPosition(instance, destinationPosition,
                                              ObjectType::WEAK);
            }))
    {
        return false;
    }

    for (const ObjectInstance& instance : instances)
    {
        if ((IsTextType(instance.type) ||
             HasPropertyAtPosition(instance, destinationPosition,
                                   ObjectType::PUSH)) &&
            HasPropertyAtPosition(instance, destinationPosition,
                                  LockedProperty(dir)) &&
            !HasPropertyAtPosition(instance, destinationPosition,
                                   ObjectType::WEAK))
        {
            return false;
        }
    }

    if (std::any_of(
            instances.begin(), instances.end(),
            [this, &destinationPosition](const ObjectInstance& instance) {
                return IsTextType(instance.type) ||
                       HasPropertyAtPosition(instance, destinationPosition,
                                             ObjectType::PUSH);
            }))
    {
        if (!CanMove(_x, _y, dir))
        {
            return std::all_of(
                instances.begin(), instances.end(),
                [this, &destinationPosition](const ObjectInstance& instance) {
                    return (!IsTextType(instance.type) &&
                            !HasPropertyAtPosition(instance,
                                                   destinationPosition,
                                                   ObjectType::PUSH)) ||
                           HasPropertyAtPosition(instance, destinationPosition,
                                                 ObjectType::WEAK);
                });
        }
    }

    return true;
}

void Game::ProcessMove(std::size_t x, std::size_t y, Direction dir,
                       const std::vector<ObjectID>& movingIDs)
{
    const auto [_x, _y] = MovedPosition(x, y, dir);
    const auto& instances = m_map.At(_x, _y).GetInstances();

    if (std::any_of(instances.begin(), instances.end(),
                    [this](const ObjectInstance& instance) {
                        return IsTextType(instance.type) ||
                               HasProperty(instance, ObjectType::PUSH);
                    }))
    {
        ProcessPush(_x, _y, dir);
    }

    for (const ObjectID id : movingIDs)
    {
        m_map.SetDirection(id, dir);
        m_map.MoveObject(id, static_cast<std::size_t>(_x),
                         static_cast<std::size_t>(_y));
    }
}

void Game::ProcessPush(std::size_t x, std::size_t y, Direction dir)
{
    std::vector<ObjectID> pushedIDs;

    for (const ObjectInstance& instance : m_map.At(x, y).GetInstances())
    {
        if ((IsTextType(instance.type) ||
             HasProperty(instance, ObjectType::PUSH)) &&
            !HasProperty(instance, LockedProperty(dir)))
        {
            pushedIDs.emplace_back(instance.id);
        }
    }

    if (pushedIDs.empty())
    {
        return;
    }

    if (!CanMove(x, y, dir))
    {
        std::vector<ObjectID> weakIDs;

        for (const ObjectID id : pushedIDs)
        {
            const ObjectInstance* instance = m_map.GetInstance(id);

            if (instance != nullptr && HasProperty(*instance, ObjectType::WEAK))
            {
                weakIDs.emplace_back(id);
            }
        }

        for (const ObjectID id : weakIDs)
        {
            m_map.RemoveObject(id);
        }

        return;
    }

    const auto [targetX, targetY] = MovedPosition(x, y, dir);
    const auto& targetInstances = m_map.At(targetX, targetY).GetInstances();

    if (const bool pushesNext =
            std::any_of(targetInstances.begin(), targetInstances.end(),
                        [this](const ObjectInstance& instance) {
                            return IsTextType(instance.type) ||
                                   HasProperty(instance, ObjectType::PUSH);
                        });
        pushesNext)
    {
        ProcessPush(targetX, targetY, dir);
    }

    for (const ObjectID id : pushedIDs)
    {
        m_map.SetDirection(id, dir);
        m_map.MoveObject(id, static_cast<std::size_t>(targetX),
                         static_cast<std::size_t>(targetY));
    }
}

void Game::ProcessSink()
{
    std::vector<ObjectID> sinkIDs;

    for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
    {
        for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
        {
            const auto& instances = m_map.At(x, y).GetInstances();

            if (instances.size() < 2 || !HasPropertyAt(x, y, ObjectType::SINK))
            {
                continue;
            }

            for (const ObjectInstance& instance : instances)
            {
                sinkIDs.emplace_back(instance.id);
            }
        }
    }

    for (const ObjectID id : sinkIDs)
    {
        m_map.RemoveObject(id);
    }
}

void Game::ProcessWeak()
{
    std::vector<ObjectID> weakIDs;

    for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
    {
        for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
        {
            const auto& instances = m_map.At(x, y).GetInstances();

            if (instances.size() < 2)
            {
                continue;
            }

            for (const ObjectInstance& instance : instances)
            {
                if (HasProperty(instance, ObjectType::WEAK))
                {
                    weakIDs.emplace_back(instance.id);
                }
            }
        }
    }

    for (const ObjectID id : weakIDs)
    {
        m_map.RemoveObject(id);
    }
}

void Game::ProcessHotMelt()
{
    std::vector<ObjectID> meltIDs;

    for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
    {
        for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
        {
            const std::vector<ObjectInstance> instances =
                m_map.At(x, y).GetInstances();

            const bool hasHot =
                std::any_of(instances.begin(), instances.end(),
                            [this](const ObjectInstance& instance) {
                                return HasProperty(instance, ObjectType::HOT);
                            });

            if (!hasHot)
            {
                continue;
            }

            for (const ObjectInstance& instance : instances)
            {
                if (HasProperty(instance, ObjectType::MELT))
                {
                    meltIDs.emplace_back(instance.id);
                }
            }
        }
    }

    for (const ObjectID id : meltIDs)
    {
        m_map.RemoveObject(id);
    }
}

void Game::ProcessDefeat()
{
    std::vector<ObjectID> playerIDs;

    for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
    {
        for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
        {
            if (!HasPropertyAt(x, y, ObjectType::DEFEAT))
            {
                continue;
            }

            for (const ObjectInstance& instance : m_map.At(x, y).GetInstances())
            {
                if (HasProperty(instance, ObjectType::YOU))
                {
                    playerIDs.emplace_back(instance.id);
                }
            }
        }
    }

    for (const ObjectID id : playerIDs)
    {
        m_map.RemoveObject(id);
    }
}

void Game::CheckPlayState()
{
    bool hasPlayer = false;

    for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
    {
        for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
        {
            for (const ObjectInstance& instance : m_map.At(x, y).GetInstances())
            {
                if (!HasProperty(instance, ObjectType::YOU))
                {
                    continue;
                }

                hasPlayer = true;

                if (HasPropertyAt(x, y, ObjectType::WIN))
                {
                    m_playState = PlayState::WON;
                    return;
                }
            }
        }
    }

    if (!hasPlayer)
    {
        m_playState = PlayState::LOST;
    }
}
}  // namespace baba_is_auto
