// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <baba-is-auto/Games/Map.hpp>

#include <charconv>
#include <fstream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace baba_is_auto
{
namespace
{
constexpr std::size_t MAX_MAP_LAYERS = 3;
constexpr std::size_t MAX_MAP_DIMENSION =
    static_cast<std::size_t>(std::numeric_limits<int>::max());

bool IsValidMapTile(int value)
{
    const auto type = static_cast<ObjectType>(value);
    return value > static_cast<int>(ObjectType::NOUN_TYPE) &&
           value <= static_cast<int>(ObjectType::LOCKED_RIGHT) &&
           type != ObjectType::OP_TYPE && type != ObjectType::PROPERTY_TYPE &&
           type != ObjectType::ICON_TYPE;
}

std::optional<int> ParseInt(std::string_view token)
{
    int value = 0;
    const auto [end, error] =
        std::from_chars(token.data(), token.data() + token.size(), value);

    if (error != std::errc{} || end != token.data() + token.size())
    {
        return std::nullopt;
    }

    return value;
}

std::optional<Direction> ParseDirection(int value)
{
    switch (value)
    {
        case 0:
            return Direction::RIGHT;
        case 1:
            return Direction::UP;
        case 2:
            return Direction::LEFT;
        case 3:
            return Direction::DOWN;
        default:
            return std::nullopt;
    }
}

using TileInstances = std::vector<std::vector<ObjectInstance>>;

void ReadMapValues(std::istream& file, std::vector<int>& values,
                   std::vector<int>& directions, bool& readingDirections)
{
    std::string token;

    while (file >> token)
    {
        if (token == "DIRECTIONS")
        {
            if (readingDirections)
            {
                throw std::runtime_error("Invalid map direction data");
            }

            readingDirections = true;
            continue;
        }

        const auto value = ParseInt(token);

        if (!value.has_value())
        {
            throw std::runtime_error("Invalid map tile data");
        }

        (readingDirections ? directions : values).emplace_back(*value);
    }
}

TileInstances BuildTileInstances(const std::vector<int>& values,
                                 const std::vector<int>& directions,
                                 bool hasDirections, std::size_t tileCount)
{
    TileInstances tileInstances(tileCount);

    for (std::size_t i = 0; i < values.size(); ++i)
    {
        if (!IsValidMapTile(values[i]))
        {
            throw std::runtime_error("Invalid map object type");
        }

        const auto type = static_cast<ObjectType>(values[i]);
        const auto direction = hasDirections
                                   ? ParseDirection(directions[i])
                                   : std::optional{ Direction::RIGHT };

        if (!direction.has_value())
        {
            throw std::runtime_error("Invalid map direction data");
        }

        if (type != ObjectType::ICON_EMPTY)
        {
            tileInstances[i % tileCount].push_back({ 0, type, *direction });
        }
    }

    return tileInstances;
}

ObjectID AssignObjectIDs(TileInstances& tiles, std::size_t width,
                         std::size_t height)
{
    ObjectID nextObjectID = 1;

    for (std::size_t x = 0; x < width; ++x)
    {
        for (std::size_t y = 0; y < height; ++y)
        {
            for (ObjectInstance& instance : tiles[y * width + x])
            {
                instance.id = nextObjectID++;
            }
        }
    }

    return nextObjectID;
}

std::vector<Object> BuildObjects(const TileInstances& tiles)
{
    std::vector<Object> objects;
    objects.reserve(tiles.size());

    for (const auto& instances : tiles)
    {
        Object object;

        for (const ObjectInstance& instance : instances)
        {
            object.Add(instance);
        }

        if (instances.empty())
        {
            object.Add(ObjectType::ICON_EMPTY);
        }

        objects.emplace_back(std::move(object));
    }

    return objects;
}
}  // namespace

Map::Map(std::size_t width, std::size_t height)
    : m_width(width), m_height(height)
{
    m_initObjects.reserve(m_width * m_height);
    m_objects.reserve(m_width * m_height);

    for (std::size_t i = 0; i < m_width * m_height; ++i)
    {
        m_initObjects.emplace_back(
            std::vector<ObjectType>{ ObjectType::ICON_EMPTY });
        m_objects.emplace_back(
            std::vector<ObjectType>{ ObjectType::ICON_EMPTY });
    }
}

void Map::Reset()
{
    m_objects = m_initObjects;
    m_nextObjectID = m_initNextObjectID;
}

std::size_t Map::GetWidth() const
{
    return m_width;
}

std::size_t Map::GetHeight() const
{
    return m_height;
}

void Map::Load(std::string_view filename)
{
    std::ifstream mapFile{ std::string(filename) };
    std::size_t width = 0;
    std::size_t height = 0;

    if (!(mapFile >> width >> height) || width == 0 || height == 0 ||
        width > MAX_MAP_DIMENSION || height > MAX_MAP_DIMENSION ||
        width > std::numeric_limits<std::size_t>::max() / height)
    {
        throw std::runtime_error("Invalid map dimensions");
    }

    const std::size_t tileCount = width * height;
    std::vector<int> values;
    std::vector<int> directionValues;
    bool readingDirections = false;

    ReadMapValues(mapFile, values, directionValues, readingDirections);

    if (values.empty() || values.size() % tileCount != 0 ||
        values.size() / tileCount > MAX_MAP_LAYERS ||
        (readingDirections && directionValues.size() != values.size()))
    {
        throw std::runtime_error("Invalid map tile data");
    }

    auto tileInstances = BuildTileInstances(values, directionValues,
                                            readingDirections, tileCount);
    const ObjectID nextObjectID = AssignObjectIDs(tileInstances, width, height);
    std::vector<Object> objects = BuildObjects(tileInstances);

    m_width = width;
    m_height = height;
    m_initObjects = objects;
    m_objects = std::move(objects);
    m_initNextObjectID = nextObjectID;
    m_nextObjectID = nextObjectID;
}

void Map::AddObject(std::size_t x, std::size_t y, ObjectType type)
{
    AddObject(x, y, type, Direction::RIGHT);
}

void Map::AddObject(std::size_t x, std::size_t y, ObjectType type,
                    Direction direction)
{
    if (direction == Direction::NONE)
    {
        throw std::invalid_argument("Object direction cannot be NONE");
    }

    if (m_mayHaveMissingObjectIDs)
    {
        AssignMissingObjectIDs();
    }

    AddGeneratedObject(x, y, type, direction);
}

void Map::AddGeneratedObject(std::size_t x, std::size_t y, ObjectType type,
                             Direction direction)
{
    const ObjectID id = type == ObjectType::ICON_EMPTY ? 0 : m_nextObjectID++;
    m_objects.at(y * m_width + x).Add(type, direction, id);
}

void Map::AssignMissingObjectIDs()
{
    for (Object& object : m_objects)
    {
        ObjectInstance* instance = object.GetInstance(0);

        while (instance != nullptr && instance->type != ObjectType::ICON_EMPTY)
        {
            instance->id = m_nextObjectID++;
            instance = object.GetInstance(0);
        }
    }
}

void Map::RemoveObject(std::size_t x, std::size_t y, ObjectType type)
{
    m_objects.at(y * m_width + x).Remove(type);
}

bool Map::RemoveObject(ObjectID id)
{
    for (Object& object : m_objects)
    {
        if (object.Remove(id))
        {
            return true;
        }
    }

    return false;
}

bool Map::MoveObject(ObjectID id, std::size_t x, std::size_t y)
{
    if (x >= m_width || y >= m_height)
    {
        return false;
    }

    const auto position = GetPosition(id);

    if (!position)
    {
        return false;
    }

    Object& source = At(position->first, position->second);
    const ObjectInstance* found = source.GetInstance(id);

    if (found == nullptr)
    {
        return false;
    }

    const ObjectInstance instance = *found;

    if (!source.Remove(id))
    {
        return false;
    }

    At(x, y).Add(instance);

    return true;
}

std::optional<Position> Map::GetPosition(ObjectID id) const
{
    if (id == 0)
    {
        return std::nullopt;
    }

    for (std::size_t y = 0; y < m_height; ++y)
    {
        for (std::size_t x = 0; x < m_width; ++x)
        {
            if (At(x, y).GetInstance(id) != nullptr)
            {
                return Position{ x, y };
            }
        }
    }

    return std::nullopt;
}

ObjectInstance* Map::GetInstance(ObjectID id)
{
    const auto position = GetPosition(id);
    return position ? At(position->first, position->second).GetInstance(id)
                    : nullptr;
}

const ObjectInstance* Map::GetInstance(ObjectID id) const
{
    const auto position = GetPosition(id);
    return position ? At(position->first, position->second).GetInstance(id)
                    : nullptr;
}

bool Map::SetDirection(ObjectID id, Direction direction)
{
    if (direction == Direction::NONE)
    {
        return false;
    }

    ObjectInstance* object = GetInstance(id);

    if (object == nullptr)
    {
        return false;
    }

    object->direction = direction;
    return true;
}

std::optional<Direction> Map::GetDirection(ObjectID id) const
{
    const ObjectInstance* object = GetInstance(id);
    return object == nullptr ? std::nullopt
                             : std::optional{ object->direction };
}

Object& Map::At(std::size_t x, std::size_t y)
{
    m_mayHaveMissingObjectIDs = true;
    return m_objects.at(y * m_width + x);
}

const Object& Map::At(std::size_t x, std::size_t y) const
{
    return m_objects.at(y * m_width + x);
}

std::vector<Position> Map::GetPositions(ObjectType type) const
{
    std::vector<Position> res;

    for (std::size_t y = 0; y < m_height; ++y)
    {
        for (std::size_t x = 0; x < m_width; ++x)
        {
            if (At(x, y).HasType(type))
            {
                res.emplace_back(x, y);
            }
        }
    }

    return res;
}

}  // namespace baba_is_auto
