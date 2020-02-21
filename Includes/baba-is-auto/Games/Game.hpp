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
    Game(std::string_view filename);

    //! Initializes game to parse initial rules.
    void Initialize();

    //! Gets a map object.
    //! \return A map object.
    Map& GetMap();

    //! Gets a rule manager object.
    //! \return A rule manager object.
    RuleManager& GetRuleManager();

    //! Gets an icon type that represents player.
    //! \return An icon type that represents player.
    ObjectType GetPlayerIcon() const;

 private:
    //! Parses a rule that satisfies the condition.
    //! \param row The number of row.
    //! \param col The number of column.
    //! \param direction The direction to check the rule.
    void ParseRule(std::size_t row, std::size_t col, RuleDirection direction);

    Map m_map;
    RuleManager m_ruleManager;

    ObjectType m_playerIcon = ObjectType::ICON_EMPTY;
};
}  // namespace baba_is_auto

#endif