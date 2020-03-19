// Copyright (c) 2020 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#ifndef BABA_IS_AUTO_GAME_HPP
#define BABA_IS_AUTO_GAME_HPP

#include <baba-is-auto/Enums/RuleEnums.hpp>
#include <baba-is-auto/Games/Map.hpp>
#include <baba-is-auto/Rules/RuleManager.hpp>

#include <string>

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
    //! \return The flag indicates that an object can move.
    bool CanMove(std::size_t x, std::size_t y, Direction dir);

    //! Processes the move of the player.
    //! \param x The x position.
    //! \param y The y position.
    //! \param dir The direction to move.
    //! \param type The object type to move.
    void ProcessMove(std::size_t x, std::size_t y, Direction dir,
                     ObjectType type);

    //! Checks the play state of the game.
    void CheckPlayState();

    Map m_map;
    RuleManager m_ruleManager;

    PlayState m_playState = PlayState::INVALID;
    ObjectType m_playerIcon = ObjectType::ICON_EMPTY;
};
}  // namespace baba_is_auto

#endif