// Copyright (c) 2020 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#ifndef BABA_IS_AUTO_MAP_HPP
#define BABA_IS_AUTO_MAP_HPP

#include <baba-is-auto/Enums/GameEnums.hpp>
#include <baba-is-auto/Games/Object.hpp>

#include <string_view>
#include <vector>

namespace baba_is_auto
{
using Position = std::pair<std::size_t, std::size_t>;

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

    //! Resets map data.
    void Reset();

    //! Gets the width of the map.
    //! \return The width of the map.
    std::size_t GetWidth() const;

    //! Gets the height of the map.
    //! \return The height of the map.
    std::size_t GetHeight() const;

    //! Loads the data of the map.
    //! \param filename The file name to load.
    void Load(std::string_view filename);

    //! Adds an object to the map.
    //! \param x The x position.
    //! \param y The y position.
    //! \param type An object type to add to the map.
    void AddObject(std::size_t x, std::size_t y, ObjectType type);

    //! Removes an object from the map.
    //! \param x The x position.
    //! \param y The y position.
    //! \param type An object type to remove from the map.
    void RemoveObject(std::size_t x, std::size_t y, ObjectType type);

    //! Assigns an object to the map.
    //! \param x The x position.
    //! \param y The y position.
    //! \return An object at row and column.
    Object& At(std::size_t x, std::size_t y);

    //! Assigns an object to the map.
    //! \param x The x position.
    //! \param y The y position.
    //! \return An object at row and column.
    const Object& At(std::size_t x, std::size_t y) const;

    //! Gets a list of icon positions.
    //! \param type An object type to get a list of positions.
    //! \return A list of icon positions.
    std::vector<Position> GetPositions(ObjectType type) const;

 private:
    std::size_t m_width = 0;
    std::size_t m_height = 0;

    std::vector<Object> m_initObjects;
    std::vector<Object> m_objects;
};
}  // namespace baba_is_auto

#endif