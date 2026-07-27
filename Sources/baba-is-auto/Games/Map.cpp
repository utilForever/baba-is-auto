// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <baba-is-auto/Games/Map.hpp>

#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace baba_is_auto
{
namespace
{
constexpr std::size_t MAX_MAP_LAYERS = 3;

bool IsValidMapTile(int value)
{
    const auto type = static_cast<ObjectType>(value);
    return value > static_cast<int>(ObjectType::NOUN_TYPE) &&
           value <= static_cast<int>(ObjectType::ICON_WATER) &&
           type != ObjectType::OP_TYPE && type != ObjectType::PROPERTY_TYPE &&
           type != ObjectType::ICON_TYPE;
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
        width > std::numeric_limits<std::size_t>::max() / height)
    {
        throw std::runtime_error("Invalid map dimensions");
    }

    const std::size_t tileCount = width * height;
    std::vector<int> values;

    for (int value = 0; mapFile >> value;)
    {
        values.emplace_back(value);
    }

    if (!mapFile.eof() || values.empty() || values.size() % tileCount != 0 ||
        values.size() / tileCount > MAX_MAP_LAYERS)
    {
        throw std::runtime_error("Invalid map tile data");
    }

    std::vector<std::vector<ObjectType>> tileTypes(tileCount);

    for (std::size_t i = 0; i < values.size(); ++i)
    {
        if (!IsValidMapTile(values[i]))
        {
            throw std::runtime_error("Invalid map object type");
        }

        tileTypes[i % tileCount].emplace_back(
            static_cast<ObjectType>(values[i]));
    }

    std::vector<Object> objects;
    objects.reserve(tileCount);

    for (auto& types : tileTypes)
    {
        objects.emplace_back(std::move(types));
    }

    m_width = width;
    m_height = height;
    m_initObjects = objects;
    m_objects = std::move(objects);
}

void Map::AddObject(std::size_t x, std::size_t y, ObjectType type)
{
    m_objects.at(y * m_width + x).Add(type);
}

void Map::RemoveObject(std::size_t x, std::size_t y, ObjectType type)
{
    m_objects.at(y * m_width + x).Remove(type);
}

Object& Map::At(std::size_t x, std::size_t y)
{
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
            for (const ObjectType tileType : At(x, y).GetTypes())
            {
                if (tileType == type)
                {
                    res.emplace_back(std::make_pair(x, y));
                }
            }
        }
    }

    return res;
}

}  // namespace baba_is_auto
