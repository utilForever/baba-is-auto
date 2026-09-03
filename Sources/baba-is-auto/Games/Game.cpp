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
    switch (type)
    {
        case ObjectType::UP:
            return Direction::UP;
        case ObjectType::DOWN:
            return Direction::DOWN;
        case ObjectType::LEFT:
            return Direction::LEFT;
        case ObjectType::RIGHT:
            return Direction::RIGHT;
        default:
            return Direction::NONE;
    }
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
    switch (direction)
    {
        case Direction::UP:
            return 0;
        case Direction::RIGHT:
            return 1;
        case Direction::DOWN:
            return 2;
        case Direction::LEFT:
        case Direction::NONE:
            return 3;
    }

    return 3;
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

constexpr bool IsSpawnableAllNoun(ObjectType type)
{
    const ObjectType noun = ConvertIconToText(type);
    return IsAllNoun(noun) && noun != ObjectType::ALL &&
           noun != ObjectType::GROUP;
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
                         const std::vector<std::pair<ObjectType, bool>>&
                             predicates,
                         const std::vector<RuleCondition>& conditions)
{
    for (const ObjectType subject : subjects)
    {
        for (const ObjectType verb : verbs)
        {
            for (const auto& [predicate, negated] : predicates)
            {
                if (negated && verb != ObjectType::IS)
                {
                    continue;
                }

                rules.AddRule({ Object({ subject }), Object({ verb }),
                                Object({ predicate }), conditions, negated });
            }
        }
    }
}

using PlayerStack = std::pair<Position, std::vector<ObjectID>>;

bool PlayerStackComesFirst(const Position& lhs, const Position& rhs,
                           Direction direction)
{
    switch (direction)
    {
        case Direction::LEFT:
            return lhs.first < rhs.first;
        case Direction::RIGHT:
            return lhs.first > rhs.first;
        case Direction::UP:
            return lhs.second < rhs.second;
        case Direction::DOWN:
        case Direction::NONE:
            return lhs.second > rhs.second;
    }

    return false;
}

class RuleLine
{
 public:
    RuleLine(Map& map, std::size_t x, std::size_t y, RuleDirection direction)
        : m_map(map),
          m_x(x),
          m_y(y),
          m_horizontal(direction == RuleDirection::HORIZONTAL)
    {
        // Do nothing
    }

    std::size_t Remaining() const
    {
        return m_horizontal ? m_map.GetWidth() - m_x : m_map.GetHeight() - m_y;
    }

    std::size_t Coordinate() const
    {
        return m_horizontal ? m_x : m_y;
    }

    Object& At(std::size_t offset) const
    {
        return m_horizontal ? m_map.At(m_x + offset, m_y)
                            : m_map.At(m_x, m_y + offset);
    }

    Object& Before(std::size_t offset) const
    {
        return m_horizontal ? m_map.At(m_x - offset, m_y)
                            : m_map.At(m_x, m_y - offset);
    }

 private:
    Map& m_map;
    std::size_t m_x;
    std::size_t m_y;
    bool m_horizontal;
};

bool StartsInsidePreviousRule(const RuleLine& line)
{
    const std::size_t coordinate = line.Coordinate();

    if (coordinate > 0 &&
        (line.Before(1).HasType(ObjectType::LONELY) ||
         FindType(line.Before(1), IsInfixCondition).has_value() ||
         (line.At(0).HasType(ObjectType::LONELY) &&
          line.Before(1).HasType(ObjectType::NOT))))
    {
        return true;
    }

    return coordinate > 1 && line.Before(1).HasType(ObjectType::AND) &&
           !line.Before(1).HasVerbType() && line.Before(2).HasNounType();
}

void ReadLonelyCondition(const RuleLine& line, std::size_t& offset,
                         std::vector<RuleCondition>& conditions)
{
    if (line.At(offset).HasType(ObjectType::LONELY))
    {
        conditions.push_back({ ObjectType::LONELY, {}, false });
        ++offset;
    }
    else if (line.At(offset).HasType(ObjectType::NOT) &&
             offset + 1 < line.Remaining() &&
             line.At(offset + 1).HasType(ObjectType::LONELY))
    {
        conditions.push_back({ ObjectType::LONELY, {}, true });
        offset += 2;
    }
}

bool ReadRuleSubjects(const RuleLine& line, std::size_t& offset,
                      std::vector<ObjectType>& subjects)
{
    if (offset >= line.Remaining() || !line.At(offset).HasNounType())
    {
        return false;
    }

    subjects = line.At(offset++).GetTypes();

    while (offset + 1 < line.Remaining() &&
           line.At(offset).HasType(ObjectType::AND) &&
           !line.At(offset).HasVerbType() && line.At(offset + 1).HasNounType())
    {
        const auto types = line.At(offset + 1).GetTypes();
        subjects.insert(subjects.end(), types.begin(), types.end());
        offset += 2;
    }

    return true;
}

bool StartsInfixCondition(const RuleLine& line, std::size_t offset)
{
    return offset < line.Remaining() &&
           FindType(line.At(offset), IsInfixCondition).has_value();
}

void RetainConditionTargets(std::vector<ObjectType>& targets,
                            ObjectType condition)
{
    RetainTypes(targets, [condition](ObjectType type) {
        return IsNounType(type) ||
               (condition == ObjectType::FACING && IsDirectionType(type));
    });
}

bool ReadConditionTargets(const RuleLine& line, std::size_t& offset,
                          ObjectType condition,
                          std::vector<ObjectType>& targets)
{
    if (offset >= line.Remaining())
    {
        return false;
    }

    targets = line.At(offset++).GetTypes();
    RetainConditionTargets(targets, condition);

    if (targets.empty())
    {
        return false;
    }

    while (offset + 1 < line.Remaining() &&
           line.At(offset).HasType(ObjectType::AND))
    {
        if (StartsInfixCondition(line, offset + 1) ||
            (line.At(offset + 1).HasType(ObjectType::NOT) &&
             StartsInfixCondition(line, offset + 2)))
        {
            ++offset;
            break;
        }

        std::vector<ObjectType> moreTargets = line.At(offset + 1).GetTypes();
        RetainConditionTargets(moreTargets, condition);

        if (moreTargets.empty())
        {
            break;
        }

        targets.insert(targets.end(), moreTargets.begin(), moreTargets.end());
        offset += 2;
    }

    return true;
}

bool ReadRuleConditions(const RuleLine& line, std::size_t& offset,
                        std::vector<RuleCondition>& conditions)
{
    while (offset < line.Remaining())
    {
        bool negated = false;

        if (line.At(offset).HasType(ObjectType::NOT) &&
            StartsInfixCondition(line, offset + 1))
        {
            negated = true;
            ++offset;
        }

        const auto condition = FindType(line.At(offset), IsInfixCondition);
        if (!condition.has_value())
        {
            break;
        }

        ++offset;

        std::vector<ObjectType> targets;

        if (!ReadConditionTargets(line, offset, *condition, targets))
        {
            return false;
        }

        conditions.push_back({ *condition, std::move(targets), negated });
    }

    return true;
}

struct ParsedPredicates
{
    std::size_t offset;
    std::vector<std::pair<ObjectType, bool>> values;
};

std::vector<ParsedPredicates> ReadRulePredicateOptions(const RuleLine& line,
                                                       std::size_t offset,
                                                       bool allowNegated)
{
    std::vector<ParsedPredicates> options;
    if (offset >= line.Remaining())
    {
        return options;
    }

    std::vector<std::pair<ObjectType, bool>> positive;
    for (const ObjectType type : line.At(offset).GetTypes())
    {
        if (IsRulePredicate(type))
        {
            positive.emplace_back(type, false);
        }
    }

    if (!positive.empty())
    {
        options.push_back({ offset + 1, std::move(positive) });
    }

    if (!allowNegated || !line.At(offset).HasType(ObjectType::NOT) ||
        offset + 1 >= line.Remaining())
    {
        return options;
    }

    std::vector<std::pair<ObjectType, bool>> negative;
    for (const ObjectType type : line.At(offset + 1).GetTypes())
    {
        if (IsPropertyType(type))
        {
            negative.emplace_back(type, true);
        }
    }

    if (!negative.empty())
    {
        options.push_back({ offset + 2, std::move(negative) });
    }

    return options;
}

void ContinueRulePredicates(const RuleLine& line, ParsedPredicates parsed,
                            std::vector<ParsedPredicates>& results,
                            bool allowNegated)
{
    if (parsed.offset + 1 >= line.Remaining() ||
        !line.At(parsed.offset).HasType(ObjectType::AND))
    {
        results.emplace_back(std::move(parsed));
        return;
    }

    auto options =
        ReadRulePredicateOptions(line, parsed.offset + 1, allowNegated);
    if (options.empty())
    {
        results.emplace_back(std::move(parsed));
        return;
    }

    for (ParsedPredicates& option : options)
    {
        ParsedPredicates next = parsed;
        next.offset = option.offset;
        next.values.insert(next.values.end(), option.values.begin(),
                           option.values.end());
        ContinueRulePredicates(line, std::move(next), results, allowNegated);
    }
}

std::vector<ParsedPredicates> ReadRulePredicates(const RuleLine& line,
                                                 std::size_t offset,
                                                 bool allowNegated)
{
    std::vector<ParsedPredicates> results;
    for (ParsedPredicates& option :
         ReadRulePredicateOptions(line, offset, allowNegated))
    {
        ContinueRulePredicates(line, std::move(option), results,
                               allowNegated);
    }

    return results;
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
    m_movementGeneratedIDs.clear();

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
    ProcessOpenShut();

    CheckPlayState();
    ParseRules();
}

void Game::ProcessPlayerMove(Direction dir)
{
    std::vector<PlayerStack> players;

    for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
    {
        for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
        {
            const Position position{ x, y };
            std::vector<ObjectID> ids = GetPlayerIDsAt(position);

            if (!ids.empty())
            {
                players.emplace_back(position, std::move(ids));
            }
        }
    }

    std::sort(players.begin(), players.end(),
              [dir](const PlayerStack& lhs, const PlayerStack& rhs) {
                  return PlayerStackComesFirst(lhs.first, rhs.first, dir);
              });

    for (const auto& [position, playerIDs] : players)
    {
        ProcessPlayerStack(position, playerIDs, dir);
    }
}

std::vector<ObjectID> Game::GetPlayerIDsAt(const Position& position) const
{
    std::vector<ObjectID> ids;

    for (const ObjectInstance& instance :
         m_map.At(position.first, position.second).GetInstances())
    {
        if (HasPropertyAtPosition(instance, position, ObjectType::YOU))
        {
            ids.emplace_back(instance.id);
        }
    }

    return ids;
}

void Game::ProcessPlayerStack(const Position& position,
                              const std::vector<ObjectID>& playerIDs,
                              Direction dir)
{
    std::vector<ObjectID> movableIDs;

    for (const ObjectID id : playerIDs)
    {
        m_map.SetDirection(id, dir);

        const ObjectInstance* instance =
            m_map.At(position.first, position.second).GetInstance(id);

        if (instance != nullptr &&
            !HasPropertyAtPosition(*instance, position, LockedProperty(dir)))
        {
            movableIDs.emplace_back(id);
        }
    }

    if (movableIDs.empty())
    {
        return;
    }

    if (CanMove(position.first, position.second, dir, movableIDs))
    {
        ProcessMove(position.first, position.second, dir, movableIDs);
        return;
    }

    for (const ObjectID id : movableIDs)
    {
        const ObjectInstance* instance =
            m_map.At(position.first, position.second).GetInstance(id);

        if (instance != nullptr &&
            HasPropertyAtPosition(*instance, position, ObjectType::WEAK))
        {
            m_map.RemoveObject(id);
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
            const Position position{ x, y };

            for (const ObjectInstance& instance : m_map.At(x, y).GetInstances())
            {
                if (HasPropertyAtPosition(instance, position,
                                          ObjectType::YOU) &&
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
    const RuleLine line(m_map, x, y, direction);

    if (line.Remaining() < 3 || StartsInsidePreviousRule(line))
    {
        return;
    }

    std::vector<RuleCondition> conditions;
    std::size_t offset = 0;

    ReadLonelyCondition(line, offset, conditions);

    std::vector<ObjectType> subjects;
    if (!ReadRuleSubjects(line, offset, subjects) ||
        !ReadRuleConditions(line, offset, conditions) ||
        offset >= line.Remaining() || !line.At(offset).HasVerbType())
    {
        return;
    }

    const std::size_t verb = offset++;
    auto parsedPredicates = ReadRulePredicates(
        line, offset, line.At(verb).HasType(ObjectType::IS));
    if (parsedPredicates.empty())
    {
        return;
    }

    std::vector<ObjectType> verbs = line.At(verb).GetTypes();

    RetainTypes(subjects, IsNounType);
    RetainTypes(verbs, IsVerbType);
    for (const ParsedPredicates& parsed : parsedPredicates)
    {
        AddRuleCombinations(m_ruleManager, subjects, verbs, parsed.values,
                            conditions);

        for (std::size_t i = 0; i < parsed.offset; ++i)
        {
            line.At(i).isRule = true;
        }
    }
}

bool Game::HasPropertyAtPosition(const ObjectInstance& instance,
                                 const Position& position,
                                 ObjectType property) const
{
    const ObjectInstance atPosition =
        instance.type == ObjectType::ICON_EMPTY ? EmptyAt(position) : instance;

    return HasPropertyForInstanceAtPosition(atPosition, position, property);
}

bool Game::HasPropertyForInstanceAtPosition(const ObjectInstance& instance,
                                            const Position& position,
                                            ObjectType property) const
{
    bool hasProperty =
        IsTextType(instance.type) && property == ObjectType::PUSH;

    for (const Rule& rule : m_ruleManager.GetRules(property))
    {
        if (!std::get<1>(rule.objects).HasType(ObjectType::IS) ||
            !std::get<2>(rule.objects).HasType(property))
        {
            continue;
        }

        for (const ObjectType subject : std::get<0>(rule.objects).GetTypes())
        {
            if (SubjectMatches(subject, instance.type) &&
                MatchesConditionsAt(instance, position, rule.conditions))
            {
                if (rule.negated)
                {
                    return false;
                }

                hasProperty = true;
            }
        }
    }

    return hasProperty;
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

namespace
{
using ConditionCandidates = std::vector<const ObjectInstance*>;

void AddConditionCandidates(const Map& map, const ObjectInstance& source,
                            const Position& sourcePosition, int candidateX,
                            int candidateY, ConditionCandidates& candidates)
{
    if (candidateX < 0 || candidateY < 0 ||
        candidateX >= static_cast<int>(map.GetWidth()) ||
        candidateY >= static_cast<int>(map.GetHeight()))
    {
        return;
    }

    for (const ObjectInstance& candidate :
         map.At(static_cast<std::size_t>(candidateX),
                static_cast<std::size_t>(candidateY))
             .GetInstances())
    {
        if (candidate.id != source.id ||
            candidateX != static_cast<int>(sourcePosition.first) ||
            candidateY != static_cast<int>(sourcePosition.second))
        {
            candidates.emplace_back(&candidate);
        }
    }
}

ConditionCandidates CollectConditionCandidates(const Map& map,
                                               const ObjectInstance& source,
                                               const Position& position,
                                               ObjectType condition)
{
    ConditionCandidates candidates;
    const auto [x, y] = position;

    if (condition == ObjectType::ON)
    {
        AddConditionCandidates(map, source, position, static_cast<int>(x),
                               static_cast<int>(y), candidates);
    }
    else if (condition == ObjectType::NEAR)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                AddConditionCandidates(map, source, position,
                                       static_cast<int>(x) + dx,
                                       static_cast<int>(y) + dy, candidates);
            }
        }
    }
    else if (condition == ObjectType::FACING)
    {
        const auto [candidateX, candidateY] =
            MovedPosition(x, y, source.direction);
        AddConditionCandidates(map, source, position, candidateX, candidateY,
                               candidates);
    }

    return candidates;
}

std::vector<ObjectType> ExpandConditionTargets(
    const std::vector<ObjectType>& conditionTargets,
    const std::vector<ObjectType>& allNouns)
{
    std::vector<ObjectType> targets;

    for (const ObjectType target : conditionTargets)
    {
        if (target == ObjectType::ALL)
        {
            std::copy_if(allNouns.begin(), allNouns.end(),
                         std::back_inserter(targets), IsSpawnableAllNoun);
        }
        else
        {
            targets.emplace_back(target);
        }
    }

    return targets;
}

bool MatchesConditionTargets(const ObjectInstance& source,
                             const ConditionCandidates& candidates,
                             const std::vector<ObjectType>& targets)
{
    ConditionCandidates used;

    for (const ObjectType target : targets)
    {
        if (IsDirectionType(target))
        {
            if (source.direction != ToDirection(target))
            {
                return false;
            }

            continue;
        }

        const auto found =
            std::find_if(candidates.begin(), candidates.end(),
                         [target, &used](const ObjectInstance* candidate) {
                             return SubjectMatches(target, candidate->type) &&
                                    std::find(used.begin(), used.end(),
                                              candidate) == used.end();
                         });

        if (found == candidates.end())
        {
            return false;
        }

        used.emplace_back(*found);
    }

    return true;
}
}  // namespace

bool Game::MatchesConditionAt(const ObjectInstance& instance,
                              const Position& position,
                              const RuleCondition& condition) const
{
    const auto [x, y] = position;
    bool matches = m_map.At(x, y).GetInstances().size() == 1;

    if (condition.op != ObjectType::LONELY)
    {
        const auto candidates =
            CollectConditionCandidates(m_map, instance, position, condition.op);
        const auto targets =
            ExpandConditionTargets(condition.targets, m_allNouns);

        matches = MatchesConditionTargets(instance, candidates, targets);
    }

    return condition.negated ? !matches : matches;
}

ObjectInstance Game::EmptyAt(const Position& position) const
{
    ObjectInstance empty{ 0, ObjectType::ICON_EMPTY, Direction::NONE };

    for (const Rule& rule : m_ruleManager.GetRules(ObjectType::IS))
    {
        if (const auto subjects = std::get<0>(rule.objects).GetTypes();
            rule.negated ||
            !std::get<1>(rule.objects).HasType(ObjectType::IS) ||
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
            if (IsDirectionType(predicate) &&
                HasPropertyForInstanceAtPosition(empty, position, predicate))
            {
                empty.direction = ToDirection(predicate);
            }
        }
    }

    return empty;
}

namespace
{
using LocatedObject = std::pair<Position, ObjectID>;

void AppendObjectsAt(const Map& map, const Position& position,
                     std::vector<LocatedObject>& objects)
{
    for (const ObjectInstance& instance :
         map.At(position.first, position.second).GetInstances())
    {
        if (instance.type != ObjectType::ICON_EMPTY)
        {
            objects.emplace_back(position, instance.id);
        }
    }
}

std::vector<LocatedObject> FindObjects(const Map& map)
{
    std::vector<LocatedObject> objects;

    for (std::size_t x = 0; x < map.GetWidth(); ++x)
    {
        for (std::size_t y = 0; y < map.GetHeight(); ++y)
        {
            AppendObjectsAt(map, { x, y }, objects);
        }
    }

    return objects;
}
}  // namespace

void Game::ProcessDirectionProperties()
{
    const auto rules = m_ruleManager.GetRules(ObjectType::IS);
    const auto objects = FindObjects(m_map);

    for (const auto& [position, id] : objects)
    {
        ObjectInstance* instance =
            m_map.At(position.first, position.second).GetInstance(id);

        if (instance == nullptr)
        {
            continue;
        }

        ApplyDirectionProperties(*instance, position, rules);
    }
}

void Game::ApplyDirectionProperties(ObjectInstance& instance,
                                    const Position& position,
                                    const std::vector<Rule>& rules) const
{
    constexpr std::array directions = { Direction::UP, Direction::RIGHT,
                                        Direction::DOWN, Direction::LEFT };
    std::array<std::size_t, 4> counts{};

    for (const Rule& rule : rules)
    {
        if (const auto subjects = std::get<0>(rule.objects).GetTypes();
            rule.negated ||
            !std::get<1>(rule.objects).HasType(ObjectType::IS) ||
            !std::any_of(subjects.begin(), subjects.end(),
                         [&instance](ObjectType subject) {
                             return SubjectMatches(subject, instance.type);
                         }) ||
            !MatchesConditionsAt(instance, position, rule.conditions))
        {
            continue;
        }

        for (const ObjectType predicate : std::get<2>(rule.objects).GetTypes())
        {
            if (IsDirectionType(predicate) &&
                HasPropertyAtPosition(instance, position, predicate))
            {
                ++counts[DirectionIndex(ToDirection(predicate))];
            }
        }
    }

    const std::size_t highest = *std::max_element(counts.begin(), counts.end());
    if (highest == 0)
    {
        return;
    }

    const std::size_t current = instance.direction == Direction::NONE
                                    ? directions.size() - 1
                                    : DirectionIndex(instance.direction);

    for (std::size_t offset = 1; offset <= directions.size(); ++offset)
    {
        const std::size_t index = (current + offset) % directions.size();

        if (counts[index] == highest)
        {
            instance.direction = directions[index];
            return;
        }
    }
}

void Game::ProcessMoveProperty()
{
    const auto rules = m_ruleManager.GetRules(ObjectType::MOVE);
    std::vector<MoveState> moving;
    std::size_t rounds = 0;

    for (const Rule& rule : rules)
    {
        AddMoveRuleAttempts(rule, moving, rounds);
    }

    for (std::size_t round = 0; round < rounds; ++round)
    {
        ProcessMoveRound(moving, round);
    }
}

void Game::AddMoveRuleAttempts(const Rule& rule, std::vector<MoveState>& moving,
                               std::size_t& rounds)
{
    if (rule.negated ||
        !std::get<2>(rule.objects).HasType(ObjectType::MOVE))
    {
        return;
    }

    const auto subjects = std::get<0>(rule.objects).GetTypes();
    std::vector<ObjectID> matching;

    for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
    {
        for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
        {
            AppendMatchingMoveObjects(rule, subjects, { x, y }, matching);
        }
    }

    std::sort(matching.begin(), matching.end());

    for (const ObjectID id : matching)
    {
        RegisterMoveAttempt(moving, id, {}, Direction::NONE, rounds);
    }

    if (!std::any_of(subjects.begin(), subjects.end(), [](ObjectType subject) {
            return SubjectMatches(subject, ObjectType::ICON_EMPTY);
        }))
    {
        return;
    }

    for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
    {
        for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
        {
            RegisterEmptyMoveAttemptAt(rule, { x, y }, moving, rounds);
        }
    }
}

void Game::AppendMatchingMoveObjects(const Rule& rule,
                                     const std::vector<ObjectType>& subjects,
                                     const Position& position,
                                     std::vector<ObjectID>& matching) const
{
    for (const ObjectInstance& instance :
         m_map.At(position.first, position.second).GetInstances())
    {
        if (instance.type == ObjectType::ICON_EMPTY ||
            !MatchesConditionsAt(instance, position, rule.conditions) ||
            !HasPropertyAtPosition(instance, position, ObjectType::MOVE))
        {
            continue;
        }

        if (std::any_of(subjects.begin(), subjects.end(),
                        [&instance](ObjectType subject) {
                            return SubjectMatches(subject, instance.type);
                        }))
        {
            matching.emplace_back(instance.id);
        }
    }
}

void Game::RegisterEmptyMoveAttemptAt(const Rule& rule,
                                      const Position& position,
                                      std::vector<MoveState>& moving,
                                      std::size_t& rounds)
{
    if (!m_map.At(position.first, position.second)
             .HasType(ObjectType::ICON_EMPTY))
    {
        return;
    }

    const ObjectInstance empty = EmptyAt(position);

    if (!MatchesConditionsAt(empty, position, rule.conditions) ||
        !HasPropertyForInstanceAtPosition(empty, position, ObjectType::MOVE))
    {
        return;
    }

    RegisterMoveAttempt(moving, 0, position, empty.direction, rounds);
}

void Game::RegisterMoveAttempt(std::vector<MoveState>& moving, ObjectID id,
                               const Position& position, Direction direction,
                               std::size_t& rounds)
{
    auto found = std::find_if(
        moving.begin(), moving.end(), [id, &position](const MoveState& state) {
            return state.id == id && (id != 0 || state.position == position);
        });

    if (found == moving.end())
    {
        moving.push_back({ id, position, direction });
        found = std::prev(moving.end());
    }

    rounds = std::max(rounds, ++found->attempts);
}

void Game::ProcessMoveRound(std::vector<MoveState>& moving, std::size_t round)
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
            ProcessEmptyMoveState(moving, index);
        }
        else
        {
            ProcessObjectMoveState(state);
        }
    }
}

void Game::ProcessEmptyMoveState(std::vector<MoveState>& moving,
                                 std::size_t index)
{
    MoveState& state = moving[index];

    if (!m_map.At(state.position.first, state.position.second)
             .HasType(ObjectType::ICON_EMPTY))
    {
        return;
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
        return;
    }

    if (CanMove(state.position.first, state.position.second, direction))
    {
        ProcessMove(state.position.first, state.position.second, direction, {});

        state.position = MovedPosition(state.position.first,
                                       state.position.second, direction);

        MergeEmptyMoveStates(moving, index);
    }
    else if (!state.turned)
    {
        state.direction = Opposite(direction);
        state.turned = true;
    }
}

void Game::MergeEmptyMoveStates(std::vector<MoveState>& moving,
                                std::size_t index)
{
    MoveState& state = moving[index];

    for (std::size_t duplicateIndex = 0; duplicateIndex < moving.size();
         ++duplicateIndex)
    {
        MoveState& duplicate = moving[duplicateIndex];

        if (duplicateIndex == index || !duplicate.active || duplicate.id != 0 ||
            duplicate.position != state.position)
        {
            continue;
        }

        if (duplicateIndex < index)
        {
            duplicate.attempts = std::max(duplicate.attempts, state.attempts);
            state.active = false;
        }
        else
        {
            state.attempts = std::max(state.attempts, duplicate.attempts);
            duplicate.active = false;
        }

        return;
    }
}

void Game::ProcessObjectMoveState(MoveState& state)
{
    const auto position = m_map.GetPosition(state.id);

    if (!position.has_value())
    {
        return;
    }

    const ObjectInstance* instance =
        m_map.At(position->first, position->second).GetInstance(state.id);

    if (instance == nullptr || instance->direction == Direction::NONE)
    {
        return;
    }

    const Direction direction = instance->direction;
    if (!state.turned &&
        HasPropertyAtPosition(*instance, *position, LockedProperty(direction)))
    {
        return;
    }

    if (CanMove(position->first, position->second, direction, { state.id }))
    {
        ProcessMove(position->first, position->second, direction, { state.id });
        return;
    }

    if (!state.turned)
    {
        m_map.SetDirection(state.id, Opposite(direction));
        state.turned = true;
    }
    else if (HasPropertyAtPosition(*instance, *position, ObjectType::WEAK))
    {
        m_map.RemoveObject(state.id);
    }
}

void Game::ProcessTransformations()
{
    UpdateAllNouns();

    const auto transformations = FindTransformations();
    std::vector<LocatedInstance> instances;
    std::vector<Position> emptyTiles;

    CollectTransformationSnapshot(instances, emptyTiles);

    std::vector<std::pair<LocatedInstance, TransformationResult>> pending;
    pending.reserve(instances.size());

    for (const LocatedInstance& snapshot : instances)
    {
        pending.emplace_back(snapshot,
                             ResolveTransformation(snapshot, transformations));
    }

    std::vector<EmptyTransformation> pendingEmpty;
    pendingEmpty.reserve(emptyTiles.size());

    for (const Position& position : emptyTiles)
    {
        const ObjectInstance empty = EmptyAt(position);
        const LocatedInstance source{ empty, position };

        pendingEmpty.push_back(
            { position, empty.direction,
              ResolveTransformation(source, transformations) });
    }

    for (const auto& [source, result] : pending)
    {
        ApplyTransformation(source, result);
    }

    for (const EmptyTransformation& empty : pendingEmpty)
    {
        ApplyEmptyTransformation(empty);
    }
}

std::vector<Game::Transformation> Game::FindTransformations() const
{
    std::vector<Transformation> transformations;

    for (const Rule& rule : m_ruleManager.GetRules(ObjectType::IS))
    {
        if (!std::get<1>(rule.objects).HasType(ObjectType::IS))
        {
            continue;
        }

        for (const ObjectType subject : std::get<0>(rule.objects).GetTypes())
        {
            AppendSubjectTransformations(rule, subject, transformations);
        }
    }

    return transformations;
}

void Game::AppendSubjectTransformations(
    const Rule& rule, ObjectType subject,
    std::vector<Transformation>& transformations) const
{
    if (!IsNounType(subject) || subject == ObjectType::GROUP)
    {
        return;
    }

    for (const ObjectType predicate : std::get<2>(rule.objects).GetTypes())
    {
        if (IsNounType(predicate) && predicate != ObjectType::GROUP)
        {
            transformations.push_back({ subject, predicate, rule.conditions });
        }
    }
}

Game::TransformationResult Game::ResolveTransformation(
    const LocatedInstance& source,
    const std::vector<Transformation>& transformations) const
{
    TransformationResult result;

    for (const Transformation& transformation : transformations)
    {
        if (!SubjectMatches(transformation.subject, source.instance.type) ||
            !MatchesConditionsAt(source.instance, source.position,
                                 transformation.conditions))
        {
            continue;
        }

        result.matched = true;

        if (transformation.predicate == ObjectType::ALL)
        {
            result.expandsAll = true;
            AppendAllTargets(result);
            continue;
        }

        result.transforms = true;

        const ObjectType target = ResolveTransformationTarget(
            source.instance.type, transformation.predicate);

        result.targets.emplace_back(target);
        result.protectedByIdentity |= target == source.instance.type;
    }

    return result;
}

void Game::AppendAllTargets(TransformationResult& result) const
{
    for (const ObjectType noun : m_allNouns)
    {
        if (IsSpawnableAllNoun(noun))
        {
            result.allTargets.emplace_back(ConvertTextToIcon(noun));
        }
    }
}

ObjectType Game::ResolveTransformationTarget(ObjectType source,
                                             ObjectType predicate)
{
    if (predicate != ObjectType::TEXT)
    {
        return ConvertTextToIcon(predicate);
    }

    return IsTextType(source) ? source : ConvertIconToText(source);
}

void Game::CollectTransformationSnapshot(
    std::vector<LocatedInstance>& instances,
    std::vector<Position>& emptyTiles) const
{
    for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
    {
        for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
        {
            const Position position{ x, y };

            if (m_map.At(x, y).HasType(ObjectType::ICON_EMPTY))
            {
                emptyTiles.emplace_back(position);
            }

            AppendTransformationInstancesAt(position, instances);
        }
    }
}

void Game::AppendTransformationInstancesAt(
    const Position& position, std::vector<LocatedInstance>& instances) const
{
    for (const ObjectInstance& instance :
         m_map.At(position.first, position.second).GetInstances())
    {
        if (instance.type != ObjectType::ICON_EMPTY &&
            std::find(m_movementGeneratedIDs.begin(),
                      m_movementGeneratedIDs.end(),
                      instance.id) == m_movementGeneratedIDs.end())
        {
            instances.push_back({ instance, position });
        }
    }
}

void Game::ApplyTransformation(const LocatedInstance& source,
                               const TransformationResult& result)
{
    if (!result.matched || result.protectedByIdentity)
    {
        return;
    }

    if (m_map.At(source.position.first, source.position.second)
            .GetInstance(source.instance.id) == nullptr)
    {
        return;
    }

    if (const bool spawned = SpawnAllTargets(
            source.position, source.instance.direction, result.allTargets);
        result.expandsAll && spawned &&
        (IsTextType(source.instance.type) ||
         source.instance.type == ObjectType::ICON_LEVEL))
    {
        m_map.RemoveObject(source.instance.id);
        return;
    }

    if (!result.transforms)
    {
        return;
    }

    std::vector<ObjectType> targets = result.targets;

    targets.erase(
        std::remove(targets.begin(), targets.end(), ObjectType::ICON_EMPTY),
        targets.end());

    if (targets.empty())
    {
        m_map.RemoveObject(source.instance.id);
        return;
    }

    ObjectInstance* instance =
        m_map.At(source.position.first, source.position.second)
            .GetInstance(source.instance.id);

    if (instance == nullptr)
    {
        return;
    }

    instance->type = targets.front();

    for (auto target = std::next(targets.begin()); target != targets.end();
         ++target)
    {
        m_map.AddGeneratedObject(source.position.first, source.position.second,
                                 *target, source.instance.direction);
    }
}

bool Game::SpawnAllTargets(const Position& position, Direction direction,
                           const std::vector<ObjectType>& targets)
{
    bool spawned = false;

    for (const ObjectType target : targets)
    {
        if (!m_map.At(position.first, position.second).HasType(target))
        {
            m_map.AddGeneratedObject(position.first, position.second, target,
                                     direction);
            spawned = true;
        }
    }

    return spawned;
}

void Game::ApplyEmptyTransformation(const EmptyTransformation& pending)
{
    const auto& [position, direction, result] = pending;

    if (!result.matched || result.protectedByIdentity)
    {
        return;
    }

    const Direction spawnDirection =
        direction == Direction::NONE ? RandomDirection() : direction;

    SpawnAllTargets(position, spawnDirection, result.allTargets);

    for (const ObjectType target : result.targets)
    {
        if (target != ObjectType::ICON_EMPTY)
        {
            m_map.AddGeneratedObject(position.first, position.second, target,
                                     spawnDirection);
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

Game::OpenShutInteraction Game::GetOpenShutInteraction(
    const Position& source, const std::vector<ObjectID>& movingIDs,
    const Position& destination) const
{
    OpenShutInteraction interaction;
    const std::vector<ObjectInstance> targets =
        m_map.At(destination.first, destination.second).GetInstances();

    const auto AddUnique = [](std::vector<ObjectID>& ids, ObjectID id) {
        if (std::find(ids.begin(), ids.end(), id) == ids.end())
        {
            ids.emplace_back(id);
        }
    };

    for (const ObjectID movingID : movingIDs)
    {
        const ObjectInstance* moving =
            m_map.At(source.first, source.second).GetInstance(movingID);

        if (moving == nullptr)
        {
            continue;
        }

        const bool movingFloats =
            HasPropertyAtPosition(*moving, source, ObjectType::FLOAT);
        const bool movingIsSafe =
            HasPropertyAtPosition(*moving, source, ObjectType::SAFE);
        const bool movingIsOpen =
            HasPropertyAtPosition(*moving, source, ObjectType::OPEN);
        const bool movingIsShut =
            !movingIsOpen &&
            HasPropertyAtPosition(*moving, source, ObjectType::SHUT);

        for (const ObjectInstance& target : targets)
        {
            if (std::find(interaction.destroyedIDs.begin(),
                          interaction.destroyedIDs.end(),
                          target.id) != interaction.destroyedIDs.end() ||
                movingFloats != HasPropertyAtPosition(target, destination,
                                                      ObjectType::FLOAT) ||
                !((movingIsOpen && HasPropertyAtPosition(target, destination,
                                                         ObjectType::SHUT)) ||
                  (movingIsShut && HasPropertyAtPosition(target, destination,
                                                         ObjectType::OPEN))))
            {
                continue;
            }

            const bool targetIsSafe =
                HasPropertyAtPosition(target, destination, ObjectType::SAFE);

            if (movingIsSafe && targetIsSafe)
            {
                continue;
            }

            AddUnique(interaction.matchedIDs, movingID);
            AddUnique(interaction.matchedIDs, target.id);

            if (!movingIsSafe)
            {
                AddUnique(interaction.destroyedIDs, movingID);
            }

            if (!targetIsSafe)
            {
                AddUnique(interaction.destroyedIDs, target.id);
            }

            if (!movingIsSafe)
            {
                break;
            }
        }
    }

    return interaction;
}

std::vector<ObjectID> Game::DestroyOpenShutObjects(
    const std::vector<ObjectID>& ids)
{
    struct Spawn
    {
        Position position;
        ObjectType type;
        Direction direction;
    };

    std::vector<Spawn> spawns;
    const auto hasRules = m_ruleManager.GetRules(ObjectType::HAS);

    for (const ObjectID id : ids)
    {
        const auto position = m_map.GetPosition(id);
        const ObjectInstance* instance = m_map.GetInstance(id);

        if (!position.has_value() || instance == nullptr)
        {
            continue;
        }

        for (const Rule& rule : hasRules)
        {
            if (!std::get<1>(rule.objects).HasType(ObjectType::HAS) ||
                !MatchesConditionsAt(*instance, *position, rule.conditions))
            {
                continue;
            }

            for (const ObjectType subject :
                 std::get<0>(rule.objects).GetTypes())
            {
                if (!SubjectMatches(subject, instance->type))
                {
                    continue;
                }

                for (const ObjectType predicate :
                     std::get<2>(rule.objects).GetTypes())
                {
                    if (IsNounType(predicate))
                    {
                        const ObjectType target = ResolveTransformationTarget(
                            instance->type, predicate);

                        if (target != ObjectType::ICON_EMPTY)
                        {
                            spawns.push_back(
                                { *position, target, instance->direction });
                        }
                    }
                }
            }
        }
    }

    for (const ObjectID id : ids)
    {
        m_map.RemoveObject(id);
    }

    std::vector<ObjectID> generatedIDs;

    for (const Spawn& spawn : spawns)
    {
        m_map.AddGeneratedObject(spawn.position.first, spawn.position.second,
                                 spawn.type, spawn.direction);
        generatedIDs.emplace_back(
            m_map.At(spawn.position.first, spawn.position.second)
                .GetInstances()
                .back()
                .id);
    }

    return generatedIDs;
}

bool Game::CanMove(std::size_t x, std::size_t y, Direction dir,
                   const std::vector<ObjectID>& movingIDs)
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
    const auto interaction =
        GetOpenShutInteraction({ x, y }, movingIDs, destinationPosition);

    const auto IsMatched = [&interaction](ObjectID id) {
        return std::find(interaction.matchedIDs.begin(),
                         interaction.matchedIDs.end(),
                         id) != interaction.matchedIDs.end();
    };

    // Check the icon has property 'STOP'.
    if (std::any_of(
            instances.begin(), instances.end(),
            [this, &destinationPosition,
             &IsMatched](const ObjectInstance& instance) {
                return !IsMatched(instance.id) &&
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
        if (!IsMatched(instance.id) &&
            HasPropertyAtPosition(instance, destinationPosition,
                                  ObjectType::PUSH) &&
            HasPropertyAtPosition(instance, destinationPosition,
                                  LockedProperty(dir)) &&
            !HasPropertyAtPosition(instance, destinationPosition,
                                   ObjectType::WEAK))
        {
            return false;
        }
    }

    if (std::any_of(instances.begin(), instances.end(),
                    [this, &destinationPosition,
                     &IsMatched](const ObjectInstance& instance) {
                        return !IsMatched(instance.id) &&
                               HasPropertyAtPosition(instance,
                                                     destinationPosition,
                                                     ObjectType::PUSH);
                    }))
    {
        std::vector<ObjectID> pushedIDs;

        for (const ObjectInstance& instance : instances)
        {
            if (!IsMatched(instance.id) &&
                HasPropertyAtPosition(instance, destinationPosition,
                                      ObjectType::PUSH) &&
                !HasPropertyAtPosition(instance, destinationPosition,
                                       LockedProperty(dir)))
            {
                pushedIDs.emplace_back(instance.id);
            }
        }

        if (!CanMove(_x, _y, dir, pushedIDs))
        {
            return std::all_of(
                instances.begin(), instances.end(),
                [this, &destinationPosition,
                 &IsMatched](const ObjectInstance& instance) {
                    return IsMatched(instance.id) ||
                           !HasPropertyAtPosition(instance,
                                                  destinationPosition,
                                                  ObjectType::PUSH) ||
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
    const Position destination{ static_cast<std::size_t>(_x),
                                static_cast<std::size_t>(_y) };

    const auto interaction =
        GetOpenShutInteraction({ x, y }, movingIDs, destination);
    std::vector<ObjectID> pushedIDs;

    const auto IsMatched = [&interaction](ObjectID id) {
        return std::find(interaction.matchedIDs.begin(),
                         interaction.matchedIDs.end(),
                         id) != interaction.matchedIDs.end();
    };

    for (const ObjectInstance& instance :
         m_map.At(destination.first, destination.second).GetInstances())
    {
        if (!IsMatched(instance.id) &&
            HasPropertyAtPosition(instance, destination, ObjectType::PUSH) &&
            !HasPropertyAtPosition(instance, destination, LockedProperty(dir)))
        {
            pushedIDs.emplace_back(instance.id);
        }
    }

    if (!pushedIDs.empty())
    {
        ProcessPush(destination.first, destination.second, dir, pushedIDs);
    }

    const auto generatedIDs = DestroyOpenShutObjects(interaction.destroyedIDs);
    m_movementGeneratedIDs.insert(m_movementGeneratedIDs.end(),
                                  generatedIDs.begin(), generatedIDs.end());

    for (const ObjectID id : movingIDs)
    {
        m_map.SetDirection(id, dir);
        m_map.MoveObject(id, destination.first, destination.second);
    }
}

void Game::ProcessPush(std::size_t x, std::size_t y, Direction dir,
                       const std::vector<ObjectID>& pushedIDs)
{
    const Position position{ x, y };

    if (!CanMove(x, y, dir, pushedIDs))
    {
        std::vector<ObjectID> weakIDs;

        for (const ObjectID id : pushedIDs)
        {
            const ObjectInstance* instance = m_map.At(x, y).GetInstance(id);

            if (instance != nullptr &&
                HasPropertyAtPosition(*instance, position, ObjectType::WEAK))
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

    ProcessMove(x, y, dir, pushedIDs);
}

void Game::ProcessOpenShut()
{
    std::vector<ObjectID> destroyedIDs;
    const auto openRules = m_ruleManager.GetRules(ObjectType::OPEN);

    const auto AddUnique = [&destroyedIDs](ObjectID id) {
        if (std::find(destroyedIDs.begin(), destroyedIDs.end(), id) ==
            destroyedIDs.end())
        {
            destroyedIDs.emplace_back(id);
        }
    };

    for (std::size_t y = 0; y < m_map.GetHeight(); ++y)
    {
        for (std::size_t x = 0; x < m_map.GetWidth(); ++x)
        {
            const Position position{ x, y };
            const std::vector<ObjectInstance> instances =
                m_map.At(x, y).GetInstances();

            for (const ObjectInstance& closer : instances)
            {
                if (!HasPropertyAtPosition(closer, position, ObjectType::SHUT))
                {
                    continue;
                }

                const bool closerFloats =
                    HasPropertyAtPosition(closer, position, ObjectType::FLOAT);
                const bool closerIsSafe =
                    HasPropertyAtPosition(closer, position, ObjectType::SAFE);

                for (const Rule& rule : openRules)
                {
                    if (rule.negated ||
                        !std::get<1>(rule.objects).HasType(ObjectType::IS) ||
                        !std::get<2>(rule.objects).HasType(ObjectType::OPEN))
                    {
                        continue;
                    }

                    const auto& subjects = std::get<0>(rule.objects).GetTypes();
                    const auto opener = std::find_if(
                        instances.begin(), instances.end(),
                        [this, &position, &rule, &subjects,
                         closerFloats](const ObjectInstance& candidate) {
                            return closerFloats == HasPropertyAtPosition(
                                                       candidate, position,
                                                       ObjectType::FLOAT) &&
                                   HasPropertyAtPosition(candidate, position,
                                                         ObjectType::OPEN) &&
                                   std::any_of(
                                       subjects.begin(), subjects.end(),
                                       [&candidate](ObjectType subject) {
                                           return SubjectMatches(
                                               subject, candidate.type);
                                       }) &&
                                   MatchesConditionsAt(candidate, position,
                                                       rule.conditions);
                        });

                    if (opener == instances.end())
                    {
                        continue;
                    }

                    const bool openerIsSafe = HasPropertyAtPosition(
                        *opener, position, ObjectType::SAFE);

                    if (closerIsSafe && openerIsSafe)
                    {
                        continue;
                    }

                    if (!closerIsSafe)
                    {
                        AddUnique(closer.id);
                    }

                    if (opener->id != closer.id && !openerIsSafe)
                    {
                        AddUnique(opener->id);
                    }
                }
            }
        }
    }

    DestroyOpenShutObjects(destroyedIDs);
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
                if (std::find(m_movementGeneratedIDs.begin(),
                              m_movementGeneratedIDs.end(),
                              instance.id) == m_movementGeneratedIDs.end() &&
                    HasPropertyAtPosition(instance, { x, y }, ObjectType::WEAK))
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
            const Position position{ x, y };

            if (const bool hasHot = std::any_of(
                    instances.begin(), instances.end(),
                    [this, &position](const ObjectInstance& instance) {
                        return HasPropertyAtPosition(instance, position,
                                                     ObjectType::HOT);
                    });
                !hasHot)
            {
                continue;
            }

            for (const ObjectInstance& instance : instances)
            {
                if (HasPropertyAtPosition(instance, position, ObjectType::MELT))
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
                if (HasPropertyAtPosition(instance, { x, y }, ObjectType::YOU))
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
                if (!HasPropertyAtPosition(instance, { x, y }, ObjectType::YOU))
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
