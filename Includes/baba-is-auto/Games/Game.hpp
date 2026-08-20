// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#ifndef BABA_IS_AUTO_GAME_HPP
#define BABA_IS_AUTO_GAME_HPP

#include <baba-is-auto/Enums/RuleEnums.hpp>
#include <baba-is-auto/Games/Map.hpp>
#include <baba-is-auto/Rules/RuleManager.hpp>

#include <cstdint>
#include <random>
#include <string_view>
#include <vector>

namespace baba_is_auto
{
//!
//! \brief Game class.
//!
//! This class represents game. A game is a structured form of Baba Is You.
//!
class Game
{
 public:
    //! Constructs game with given \p mapFileName.
    //! \param filename The file name to load a map.
    explicit Game(std::string_view filename);

    //! Resets map and rule data.
    void Reset();

    //! Sets the random seed used for directionless EMPTY behavior.
    //! \param seed The random seed.
    void SetRandomSeed(std::uint32_t seed);

    //! Gets a map object.
    //! \return A map object.
    Map& GetMap();

    //! Gets a map object.
    //! \return A map object.
    const Map& GetMap() const;

    //! Gets a rule manager object.
    //! \return A rule manager object.
    RuleManager& GetRuleManager();

    //! Gets the play state of the game.
    //! \return The play state of the game.
    PlayState GetPlayState() const;

    //! Gets an icon type that represents player.
    //! \return An icon type that represents player.
    ObjectType GetPlayerIcon() const;

    //! Moves the icon that represents player.
    //! \param dir The direction to move the player.
    void MovePlayer(Direction dir);

 private:
    //! Parses a list of rules.
    void ParseRules();

    //! Parses a rule that satisfies the condition.
    //! \param x The x position.
    //! \param y The y position.
    //! \param direction The direction to check the rule.
    void ParseRule(std::size_t x, std::size_t y, RuleDirection direction);

    //! Checks an object can move.
    //! \param x The x position.
    //! \param y The y position.
    //! \param dir The direction to move.
    //! \param movingIDs The object IDs to move together.
    //! \return The flag indicates that an object can move.
    bool CanMove(std::size_t x, std::size_t y, Direction dir,
                 const std::vector<ObjectID>& movingIDs = {});

    //! Moves objects with the active YOU property.
    //! \param dir The direction to move.
    void ProcessPlayerMove(Direction dir);

    //! Gets the active YOU object IDs at a position.
    //! \param position The position to inspect.
    //! \return The active YOU object IDs at the position.
    std::vector<ObjectID> GetPlayerIDsAt(const Position& position) const;

    //! Resolves movement for one stack of YOU objects.
    //! \param position The position of the stack.
    //! \param playerIDs The YOU object IDs in the stack.
    //! \param dir The direction to move.
    void ProcessPlayerStack(const Position& position,
                            const std::vector<ObjectID>& playerIDs,
                            Direction dir);

    //! Resolves all MOVE attempts in round order.
    void ProcessMoveProperty();

    //! Tracks one object or EMPTY tile while resolving MOVE.
    struct MoveState
    {
        //! The object ID, or zero for EMPTY.
        ObjectID id = 0;

        //! The current EMPTY position; unused for object states.
        Position position{};

        //! The current EMPTY direction; unused for object states.
        Direction direction = Direction::NONE;

        //! The number of MOVE attempts to process.
        std::size_t attempts = 0;

        //! Whether a blocked MOVE attempt has already turned around.
        bool turned = false;

        //! Whether this state remains eligible for processing.
        bool active = true;
    };

    //! Adds the MOVE attempts produced by a rule.
    //! \param rule The MOVE rule to evaluate.
    //! \param moving The MOVE states to update.
    //! \param rounds The maximum number of MOVE rounds to update.
    void AddMoveRuleAttempts(const Rule& rule, std::vector<MoveState>& moving,
                             std::size_t& rounds);

    //! Adds objects at a position that match a MOVE rule.
    //! \param rule The MOVE rule whose conditions to evaluate.
    //! \param subjects The subjects accepted by the rule.
    //! \param position The position to inspect.
    //! \param matching The matching object IDs to update.
    void AppendMatchingMoveObjects(const Rule& rule,
                                   const std::vector<ObjectType>& subjects,
                                   const Position& position,
                                   std::vector<ObjectID>& matching) const;

    //! Registers a matching EMPTY MOVE attempt at a position.
    //! \param rule The EMPTY MOVE rule to evaluate.
    //! \param position The EMPTY position to inspect.
    //! \param moving The MOVE states to update.
    //! \param rounds The maximum number of MOVE rounds to update.
    void RegisterEmptyMoveAttemptAt(const Rule& rule, const Position& position,
                                    std::vector<MoveState>& moving,
                                    std::size_t& rounds);

    //! Registers one MOVE attempt, combining repeated attempts.
    //! \param moving The MOVE states to update.
    //! \param id The object ID, or zero for EMPTY.
    //! \param position The EMPTY position when \p id is zero.
    //! \param direction The initial movement direction.
    //! \param rounds The maximum number of MOVE rounds to update.
    static void RegisterMoveAttempt(std::vector<MoveState>& moving, ObjectID id,
                                    const Position& position,
                                    Direction direction, std::size_t& rounds);

    //! Processes all eligible MOVE states in one round.
    //! \param moving The MOVE states to process.
    //! \param round The zero-based MOVE round.
    void ProcessMoveRound(std::vector<MoveState>& moving, std::size_t round);

    //! Processes one EMPTY MOVE state.
    //! \param moving The MOVE states to update.
    //! \param index The index of the EMPTY state to process.
    void ProcessEmptyMoveState(std::vector<MoveState>& moving,
                               std::size_t index);

    //! Merges EMPTY MOVE states that reach the same position.
    //! \param moving The MOVE states to update.
    //! \param index The index of the state that just moved.
    static void MergeEmptyMoveStates(std::vector<MoveState>& moving,
                                     std::size_t index);

    //! Processes one object MOVE state.
    //! \param state The MOVE state to update.
    void ProcessObjectMoveState(MoveState& state);

    //! Applies active directional properties to object facing.
    void ProcessDirectionProperties();

    //! Applies active directional rules to one object instance.
    //! \param instance The object instance whose direction to update.
    //! \param position The position of the object instance.
    //! \param rules The active IS rules to evaluate.
    void ApplyDirectionProperties(ObjectInstance& instance,
                                  const Position& position,
                                  const std::vector<Rule>& rules) const;

    //! Builds the EMPTY instance state at a position for this turn.
    //! \return An object instance representing the EMPTY state at a position.
    ObjectInstance EmptyAt(const Position& position) const;

    //! Applies active noun transformations from one board snapshot.
    void ProcessTransformations();

    //! Describes one active noun transformation.
    struct Transformation
    {
        //! The noun matched by the transformation.
        ObjectType subject;

        //! The noun produced by the transformation.
        ObjectType predicate;

        //! The conditions that the source must satisfy.
        std::vector<RuleCondition> conditions;
    };

    //! Stores the transformations resolved for one snapshot object.
    struct TransformationResult
    {
        //! Whether at least one transformation matched the source.
        bool matched = false;

        //! Whether a direct noun transformation matched the source.
        bool transforms = false;

        //! Whether an ALL transformation matched the source.
        bool expandsAll = false;

        //! Whether an identity transformation preserves the source.
        bool protectedByIdentity = false;

        //! The targets produced by direct noun transformations.
        std::vector<ObjectType> targets;

        //! The targets produced by expanding ALL.
        std::vector<ObjectType> allTargets;
    };

    //! Pairs a snapshot object with its board position.
    struct LocatedInstance
    {
        //! The snapshot object.
        ObjectInstance instance;

        //! The object's snapshot position.
        Position position;
    };

    //! Stores a pending transformation for one EMPTY tile.
    struct EmptyTransformation
    {
        //! The EMPTY tile position.
        Position position;

        //! The EMPTY direction, or NONE when no direction was assigned.
        Direction direction;

        //! The transformation resolved from the snapshot.
        TransformationResult result;
    };

    //! Gets the active noun transformations.
    //! \return The active noun transformations.
    std::vector<Transformation> FindTransformations() const;

    //! Adds valid predicate transformations for one subject.
    //! \param rule The IS rule to inspect.
    //! \param subject The rule subject to expand.
    //! \param transformations The transformations to update.
    void AppendSubjectTransformations(
        const Rule& rule, ObjectType subject,
        std::vector<Transformation>& transformations) const;

    //! Resolves all transformations for one snapshot object.
    //! \param source The snapshot object and its position.
    //! \param transformations The active transformations to evaluate.
    //! \return The resolved transformation result.
    TransformationResult ResolveTransformation(
        const LocatedInstance& source,
        const std::vector<Transformation>& transformations) const;

    //! Adds spawnable ALL nouns to a transformation result.
    //! \param result The transformation result to update.
    void AppendAllTargets(TransformationResult& result) const;

    //! Resolves the icon type produced by a transformation.
    //! \param source The source object type.
    //! \param predicate The transformation predicate.
    //! \return The transformed object type.
    static ObjectType ResolveTransformationTarget(ObjectType source,
                                                  ObjectType predicate);

    //! Collects the object and EMPTY transformation snapshots.
    //! \param instances The positioned object snapshots to update.
    //! \param emptyTiles The EMPTY positions to update.
    void CollectTransformationSnapshot(std::vector<LocatedInstance>& instances,
                                       std::vector<Position>& emptyTiles) const;

    //! Adds object snapshots found at a position.
    //! \param position The position to inspect.
    //! \param instances The positioned object snapshots to update.
    void AppendTransformationInstancesAt(
        const Position& position,
        std::vector<LocatedInstance>& instances) const;

    //! Applies a resolved transformation to one snapshot object.
    //! \param source The snapshot object and its position.
    //! \param result The resolved transformation to apply.
    void ApplyTransformation(const LocatedInstance& source,
                             const TransformationResult& result);

    //! Spawns missing ALL targets at a position.
    //! \param position The position at which to spawn targets.
    //! \param direction The direction assigned to spawned objects.
    //! \param targets The object types to spawn.
    //! \return Whether at least one target was spawned.
    bool SpawnAllTargets(const Position& position, Direction direction,
                         const std::vector<ObjectType>& targets);

    //! Applies a resolved transformation to an EMPTY position.
    //! \param pending The EMPTY transformation to apply.
    void ApplyEmptyTransformation(const EmptyTransformation& pending);

    //! Adds nouns currently present in the level to the persistent ALL set.
    void UpdateAllNouns();

    //! Chooses a random cardinal direction.
    //! \return A randomly chosen cardinal direction (UP, DOWN, LEFT, RIGHT).
    Direction RandomDirection();

    //! Checks an object instance has a property at a known position.
    bool HasPropertyAtPosition(const ObjectInstance& instance,
                               const Position& position,
                               ObjectType property) const;

    //! Checks whether any object at a position has a property.
    //! \param x The x position.
    //! \param y The y position.
    //! \param property The property to check.
    //! \return The flag indicates that an object at a position has a property.
    bool HasPropertyAt(std::size_t x, std::size_t y, ObjectType property) const;

    //! Checks all conditions attached to a rule at a known position.
    //! \param instance The object instance to check.
    //! \param position The position at which to evaluate the conditions.
    //! \param conditions The conditions to check.
    //! \return The flag indicates that an object instance matches all
    //! conditions.
    bool MatchesConditionsAt(
        const ObjectInstance& instance, const Position& position,
        const std::vector<RuleCondition>& conditions) const;

    //! Checks one condition attached to a rule at a known position.
    //! \param instance The object instance to check.
    //! \param position The position at which to evaluate the condition.
    //! \param condition The condition to check.
    //! \return The flag indicates that an object instance matches the
    //! condition.
    bool MatchesConditionAt(const ObjectInstance& instance,
                            const Position& position,
                            const RuleCondition& condition) const;

    //! Tracks matched and destroyed IDs at one OPEN/SHUT movement boundary.
    struct OpenShutInteraction
    {
        //! The object IDs that matched the OPEN/SHUT interaction.
        std::vector<ObjectID> matchedIDs;
        //! The object IDs that were destroyed by the OPEN/SHUT interaction.
        std::vector<ObjectID> destroyedIDs;
    };

    //! Gets the OPEN/SHUT interaction at one movement boundary.
    //! \param source The source position of the moving objects.
    //! \param movingIDs The object IDs that are moving.
    //! \param destination The destination position of the moving objects.
    //! \return The OPEN/SHUT interaction at the movement boundary.
    OpenShutInteraction GetOpenShutInteraction(
        const Position& source, const std::vector<ObjectID>& movingIDs,
        const Position& destination) const;

    //! Destroys objects and spawns their active HAS results in place.
    //! \param ids The object IDs to destroy.
    //! \return The IDs of the generated HAS objects.
    std::vector<ObjectID> DestroyOpenShutObjects(
        const std::vector<ObjectID>& ids);

    //! Processes the move of the player.
    //! \param x The x position.
    //! \param y The y position.
    //! \param dir The direction to move.
    //! \param ids The object instances to move together.
    void ProcessMove(std::size_t x, std::size_t y, Direction dir,
                     const std::vector<ObjectID>& ids);

    //! Pushes every movable object stacked at a position as one group.
    //! \param x The x position.
    //! \param y The y position.
    //! \param dir The direction to move.
    //! \param ids The object instances selected before collision effects.
    void ProcessPush(std::size_t x, std::size_t y, Direction dir,
                     const std::vector<ObjectID>& ids);

    //! Resolves OPEN and SHUT objects that already overlap.
    void ProcessOpenShut();

    //! Removes all objects where a SINK object overlaps another object.
    void ProcessSink();

    //! Removes WEAK objects that overlap another object.
    void ProcessWeak();

    //! Removes MELT objects overlapping HOT objects.
    void ProcessHotMelt();

    //! Removes YOU objects overlapping DEFEAT objects.
    void ProcessDefeat();

    //! Checks the play state of the game.
    void CheckPlayState();

    Map m_map;
    RuleManager m_ruleManager;

    PlayState m_playState = PlayState::INVALID;
    ObjectType m_playerIcon = ObjectType::ICON_EMPTY;

    std::vector<ObjectType> m_playerIcons;
    std::vector<ObjectType> m_allNouns;
    std::vector<ObjectID> m_movementGeneratedIDs;
    std::mt19937 m_randomEngine{ std::random_device{}() };
};
}  // namespace baba_is_auto

#endif
