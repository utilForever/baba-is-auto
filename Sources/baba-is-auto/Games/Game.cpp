// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <baba-is-auto/Games/Game.hpp>

#include <algorithm>
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

bool HasIconProperty(RuleManager& rules, const std::vector<ObjectType>& types,
                     ObjectType property)
{
    return std::any_of(
        types.begin(), types.end(), [&rules, &property](ObjectType type) {
            return !IsTextType(type) && rules.HasProperty({ type }, property);
        });
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

    m_playState = PlayState::PLAYING;
}

void Game::Reset()
{
    m_map.Reset();
    m_allNouns.clear();

    UpdateAllNouns();
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
    m_map.AssignMissingObjectIDs();

    if (dir != Direction::NONE)
    {
        ProcessPlayerMove(dir);
    }

    ParseRules();
    ProcessMoveProperty();
    ParseRules();
    ProcessTransformations();
    ParseRules();

    ProcessSink();
    ProcessHotMelt();
    ProcessDefeat();

    CheckPlayState();
    ParseRules();
}

void Game::ProcessPlayerMove(Direction dir)
{
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
        std::vector<ObjectID> playerIDs;

        for (const ObjectInstance& instance : m_map.At(x, y).GetInstances())
        {
            if (std::find(m_playerIcons.begin(), m_playerIcons.end(),
                          instance.type) != m_playerIcons.end())
            {
                playerIDs.emplace_back(instance.id);
                m_map.SetDirection(instance.id, dir);
            }
        }

        if (!playerIDs.empty() && CanMove(x, y, dir))
        {
            ProcessMove(x, y, dir, playerIDs);
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

    for (const Rule& rule : m_ruleManager.GetRules(ObjectType::YOU))
    {
        if (!rule.conditions.empty())
        {
            continue;
        }

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
    for (const Rule& rule : m_ruleManager.GetRules(property))
    {
        if (!std::get<2>(rule.objects).HasType(property))
        {
            continue;
        }

        for (const ObjectType subject : std::get<0>(rule.objects).GetTypes())
        {
            if (SubjectMatches(subject, instance.type) &&
                MatchesConditions(instance, rule.conditions))
            {
                return true;
            }
        }
    }

    return false;
}

bool Game::MatchesConditions(const ObjectInstance& instance,
                             const std::vector<RuleCondition>& conditions) const
{
    return std::all_of(conditions.begin(), conditions.end(),
                       [this, &instance](const RuleCondition& condition) {
                           return MatchesCondition(instance, condition);
                       });
}

bool Game::MatchesCondition(const ObjectInstance& instance,
                            const RuleCondition& condition) const
{
    const auto position = m_map.GetPosition(instance.id);

    if (!position)
    {
        return false;
    }

    const auto [x, y] = *position;
    bool matches;

    if (condition.op == ObjectType::LONELY)
    {
        matches = m_map.At(x, y).GetInstances().size() == 1;
    }
    else
    {
        std::vector<const ObjectInstance*> candidates;

        const auto AddCandidates = [this, &candidates, &instance](
                                       int candidateX, int candidateY) {
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
                if (candidate.id != instance.id)
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
                targets.emplace_back(ObjectType::TEXT);
                targets.emplace_back(ObjectType::EMPTY);
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
                    const Direction direction =
                        target == ObjectType::UP     ? Direction::UP
                        : target == ObjectType::DOWN ? Direction::DOWN
                        : target == ObjectType::LEFT ? Direction::LEFT
                                                     : Direction::RIGHT;
                    return instance.direction == direction;
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

void Game::ProcessMoveProperty()
{
    struct MoveState
    {
        ObjectID id;
        std::size_t attempts;
        bool turned = false;
    };

    const auto rules = m_ruleManager.GetRules(ObjectType::MOVE);
    std::vector<MoveState> moving;
    std::size_t rounds = 0;

    for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
    {
        for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
        {
            for (const ObjectInstance& instance : m_map.At(x, y).GetInstances())
            {
                std::size_t attempts = 0;

                for (const Rule& rule : rules)
                {
                    if (!std::get<2>(rule.objects).HasType(ObjectType::MOVE) ||
                        !MatchesConditions(instance, rule.conditions))
                    {
                        continue;
                    }

                    const auto subjects = std::get<0>(rule.objects).GetTypes();

                    if (std::any_of(subjects.begin(), subjects.end(),
                                    [&instance](ObjectType subject) {
                                        return SubjectMatches(subject,
                                                              instance.type);
                                    }))
                    {
                        ++attempts;
                    }
                }

                if (attempts != 0)
                {
                    moving.push_back({ instance.id, attempts });
                    rounds = std::max(rounds, attempts);
                }
            }
        }
    }

    for (std::size_t round = 0; round < rounds; ++round)
    {
        for (MoveState& state : moving)
        {
            if (round >= state.attempts)
            {
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
    };

    struct Result
    {
        bool matched = false;
        bool protectedByIdentity = false;
        std::vector<ObjectType> targets;
    };

    UpdateAllNouns();

    std::vector<Transformation> transformations;

    for (const Rule& rule : m_ruleManager.GetRules(ObjectType::IS))
    {
        if (!rule.conditions.empty() ||
            !std::get<1>(rule.objects).HasType(ObjectType::IS))
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

                transformations.push_back({ subject, predicate });
            }
        }
    }

    const auto Resolve = [this, &transformations](ObjectType source) {
        Result result;

        for (const Transformation& transformation : transformations)
        {
            if (!SubjectMatches(transformation.subject, source))
            {
                continue;
            }

            result.matched = true;

            if (transformation.predicate == ObjectType::ALL)
            {
                for (const ObjectType noun : m_allNouns)
                {
                    if (IsSpawnableAllNoun(noun))
                    {
                        result.targets.emplace_back(ConvertTextToIcon(noun));
                    }
                }

                continue;
            }

            const ObjectType target =
                transformation.predicate == ObjectType::TEXT
                    ? (IsTextType(source) ? source : ConvertIconToText(source))
                    : ConvertTextToIcon(transformation.predicate);

            result.targets.emplace_back(target);
            result.protectedByIdentity |= target == source;
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

    for (const ObjectInstance& snapshot : instances)
    {
        const Result result = Resolve(snapshot.type);

        if (!result.matched || result.protectedByIdentity)
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
        const auto position = m_map.GetPosition(snapshot.id);

        if (instance == nullptr || !position)
        {
            continue;
        }

        instance->type = targets.front();

        for (auto target = std::next(targets.begin()); target != targets.end();
             ++target)
        {
            m_map.AddObject(position->first, position->second, *target,
                            snapshot.direction);
        }
    }

    const Result emptyResult = Resolve(ObjectType::ICON_EMPTY);

    if (!emptyResult.matched || emptyResult.protectedByIdentity)
    {
        return;
    }

    for (const auto& [x, y] : emptyTiles)
    {
        for (const ObjectType target : emptyResult.targets)
        {
            if (target != ObjectType::ICON_EMPTY)
            {
                m_map.AddObject(x, y, target);
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

    const std::vector<ObjectType> types = m_map.At(_x, _y).GetTypes();

    // Check the icon has property 'STOP'.
    if (HasIconProperty(m_ruleManager, types, ObjectType::STOP))
    {
        return false;
    }

    for (const ObjectInstance& instance :
         m_map.At(static_cast<std::size_t>(_x), static_cast<std::size_t>(_y))
             .GetInstances())
    {
        if ((IsTextType(instance.type) ||
             HasProperty(instance, ObjectType::PUSH)) &&
            HasProperty(instance, LockedProperty(dir)))
        {
            return false;
        }
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
                       const std::vector<ObjectID>& movingIDs)
{
    const auto [_x, _y] = MovedPosition(x, y, dir);

    if (const std::vector<ObjectType> destinationTypes =
            m_map.At(_x, _y).GetTypes();
        HasIconProperty(m_ruleManager, destinationTypes, ObjectType::PUSH) ||
        m_map.At(_x, _y).HasTextType())
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

    std::vector<ObjectID> pushedIDs;

    for (const ObjectInstance& instance : m_map.At(x, y).GetInstances())
    {
        if (IsTextType(instance.type) ||
            m_ruleManager.HasProperty({ instance.type }, ObjectType::PUSH))
        {
            pushedIDs.emplace_back(instance.id);
        }
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
    const auto RemoveDefeatedPlayers = [this](std::size_t x, std::size_t y) {
        const std::vector<ObjectType> types = m_map.At(x, y).GetTypes();

        if (!HasIconProperty(m_ruleManager, types, ObjectType::DEFEAT))
        {
            return;
        }

        for (const ObjectType type : types)
        {
            if (std::find(m_playerIcons.begin(), m_playerIcons.end(), type) !=
                m_playerIcons.end())
            {
                m_map.RemoveObject(x, y, type);
            }
        }
    };

    for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
    {
        for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
        {
            RemoveDefeatedPlayers(x, y);
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
