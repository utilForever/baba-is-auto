#include <baba-is-auto/Games/Map.hpp>

namespace baba_is_auto
{
Map::Map(std::size_t width, std::size_t height)
    : m_width(width), m_height(height)
{
    m_objects.reserve(m_width * m_height);

    for (std::size_t i = 0; i < m_width * m_height; ++i)
    {
        m_objects.emplace_back(ObjectType::EMPTY);
    }
}

void Map::Assign(std::size_t row, std::size_t col, ObjectType type)
{
    m_objects.at(row * m_width + col) = Object(type);
}

Object Map::At(std::size_t row, std::size_t col)
{
    return m_objects.at(row * m_width + col);
}
}  // namespace baba_is_auto