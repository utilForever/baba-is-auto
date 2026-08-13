// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#ifndef BABA_IS_AUTO_MAP_HPP
#define BABA_IS_AUTO_MAP_HPP

#include <baba-is-auto/Enums/GameEnums.hpp>
#include <baba-is-auto/Games/Object.hpp>

#include <optional>
#include <string_view>
#include <vector>

namespace baba_is_auto
{
//! A zero-based (x, y) map position.
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
    //! \throws std::runtime_error If the map data is invalid.
    void Load(std::string_view filename);

    //! Adds an object to the map.
    //! \param x The x position.
    //! \param y The y position.
    //! \param type An object type to add to the map.
    void AddObject(std::size_t x, std::size_t y, ObjectType type);

    //! Adds an object with an initial facing direction.
    //! \param x The x position.
    //! \param y The y position.
    //! \param type An object type to add to the map.
    //! \param direction An initial facing direction of the object.
    void AddObject(std::size_t x, std::size_t y, ObjectType type,
                   Direction direction);

    //! Removes an object from the map.
    //! \param x The x position.
    //! \param y The y position.
    //! \param type An object type to remove from the map.
    void RemoveObject(std::size_t x, std::size_t y, ObjectType type);

    //! Removes an object by stable ID.
    //! \param id The stable ID of the object to remove.
    //! \return True if the object was removed successfully, false otherwise.
    bool RemoveObject(ObjectID id);

    //! Moves an object by stable ID, preserving its state.
    //! \param id The stable ID of the object to move.
    //! \param x The new x position.
    //! \param y The new y position.
    //! \return True if the object was moved successfully, false otherwise.
    bool MoveObject(ObjectID id, std::size_t x, std::size_t y);

    //! Gets an object's current position.
    //! \param id The stable ID of the object.
    //! \return The current position of the object.
    std::optional<Position> GetPosition(ObjectID id) const;

    //! Gets an object by stable ID.
    //! \param id The stable ID of the object.
    //! \return A pointer to the object instance, or nullptr if not found.
    ObjectInstance* GetInstance(ObjectID id);

    //! Gets an object by stable ID.
    //! \param id The stable ID of the object.
    //! \return A pointer to the object instance, or nullptr if not found.
    const ObjectInstance* GetInstance(ObjectID id) const;

    //! Updates an object's facing direction.
    //! \param id The stable ID of the object.
    //! \param direction The new facing direction of the object.
    //! \return True if the direction was updated successfully, false otherwise.
    bool SetDirection(ObjectID id, Direction direction);

    //! Gets an object's facing direction.
    //! \param id The stable ID of the object.
    //! \return The facing direction of the object, or std::nullopt if not
    //! found.
    std::optional<Direction> GetDirection(ObjectID id) const;

    //! Gets a writable object reference from the map.
    //! \param x The x position.
    //! \param y The y position.
    //! \return A writable object reference at the zero-based (x, y) position.
    Object& At(std::size_t x, std::size_t y);

    //! Gets an object from the map.
    //! \param x The x position.
    //! \param y The y position.
    //! \return An object at row and column.
    const Object& At(std::size_t x, std::size_t y) const;

    //! Gets a list of icon positions.
    //! \param type An object type to get a list of positions.
    //! \return A list of icon positions.
    std::vector<Position> GetPositions(ObjectType type) const;

 private:
    friend class Game;

    //! Adds an internally generated object after pending IDs are assigned.
    //! \param x The x position.
    //! \param y The y position.
    //! \param type An object type to add to the map.
    //! \param direction An initial facing direction of the object.
    void AddGeneratedObject(std::size_t x, std::size_t y, ObjectType type,
                            Direction direction);

    //! Assigns missing object IDs to objects.
    void AssignMissingObjectIDs();

    std::size_t m_width = 0;
    std::size_t m_height = 0;

    ObjectID m_initNextObjectID = 1;
    ObjectID m_nextObjectID = 1;

    bool m_mayHaveMissingObjectIDs = false;

    std::vector<Object> m_initObjects;
    std::vector<Object> m_objects;
};
}  // namespace baba_is_auto

#endif
