#ifndef BABA_IS_AUTO_GAME_HPP
#define BABA_IS_AUTO_GAME_HPP

#include <baba-is-auto/Games/Map.hpp>

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

 private:
    Map m_map;
};
}  // namespace baba_is_auto

#endif