#ifndef BABA_IS_AUTO_GAME_HPP
#define BABA_IS_AUTO_GAME_HPP

#include <baba-is-auto/Games/Map.hpp>

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
    //! Constructs game with given \p width and \p height.
    //! \param width The size of the width.
    //! \param height The size of the height.
    Game(std::size_t width, std::size_t height);

 private:
    Map m_map;
};
}  // namespace baba_is_auto

#endif