#ifndef BABA_IS_AUTO_MAP_HPP
#define BABA_IS_AUTO_MAP_HPP

#include <baba-is-auto/Enums/GameEnums.hpp>
#include <baba-is-auto/Games/Object.hpp>

#include <string_view>
#include <vector>

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
    //! Default constructor.
    Map() = default;

    //! Constructs map with given \p width and \p height.
    //! \param width The size of the width.
    //! \param height The size of the height.
    Map(std::size_t width, std::size_t height);

    //! Loads the data of the map.
    //! \param filename The file name to load.
    void Load(std::string_view filename);

    //! Shows the map.
    void Show() const;

    //! Parses the rules.
    void ParseRules();

    //! Assigns an object to the map.
    //! \param row The number of row.
    //! \param col The number of column.
    //! \param type An object type to assign to the map.
    void Assign(std::size_t row, std::size_t col, ObjectType type);

    //! Assigns an object to the map.
    //! \param row The number of row.
    //! \param col The number of column.
    //! \return An object at row and column.
    Object At(std::size_t row, std::size_t col);

 private:
    std::size_t m_width = 0;
    std::size_t m_height = 0;

    std::vector<Object> m_objects;
};
}  // namespace baba_is_auto

#endif