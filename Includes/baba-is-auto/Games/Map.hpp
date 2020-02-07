#ifndef BABA_IS_AUTO_MAP_HPP
#define BABA_IS_AUTO_MAP_HPP

#include <cstdlib>

namespace baba_is_auto
{
//!
//! \brief Map class.
//!
//! This class represents map. A map is a board of the game.
//!
class Map
{
 public:
    //! Constructs map with given \p width and \p height.
    //! \param width The size of the width.
    //! \param height The size of the height.
    Map(std::size_t width, std::size_t height);

 private:
    std::size_t m_width = 0;
    std::size_t m_height = 0;
};
}  // namespace baba_is_auto

#endif