// Copyright (c) 2020 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include <baba-is-auto/Agents/Preprocess.hpp>

#include <map>

namespace baba_is_auto
{
std::map<ObjectType, std::size_t> TENSOR_DIM_MAP = {
    { ObjectType::BABA, 0 },      { ObjectType::IS, 1 },
    { ObjectType::YOU, 2 },       { ObjectType::ICON_EMPTY, 3 },
    { ObjectType::FLAG, 4 },      { ObjectType::WIN, 5 },
    { ObjectType::ICON_WALL, 6 }, { ObjectType::ICON_ROCK, 7 },
    { ObjectType::ICON_BABA, 8 }, { ObjectType::ICON_FLAG, 9 },
    { ObjectType::WALL, 10 },     { ObjectType::STOP, 11 },
    { ObjectType::ROCK, 12 },     { ObjectType::PUSH, 13 }
};

std::vector<float> Preprocess::StateToTensor(const Game& game)
{
    const std::size_t width = game.GetMap().GetWidth();
    const std::size_t height = game.GetMap().GetHeight();

    std::vector<float> tensor(TENSOR_DIM * width * height, 0.0f);

    const auto ToIndex = [width, height](std::size_t x, std::size_t y,
                                         std::size_t c) {
        return (c * width * height) + (y * width) + x;
    };

    for (std::size_t y = 0; y < height; ++y)
    {
        for (std::size_t x = 0; x < width; ++x)
        {
            const auto objs = game.GetMap().At(x, y).GetTypes();

            if (!objs.empty())
            {
                bool isTextType = false;

                for (auto& obj : objs)
                {
                    tensor[ToIndex(x, y, TENSOR_DIM_MAP[obj])] = 1.0f;

                    if (IsTextType(obj))
                    {
                        isTextType = true;
                    }
                }

                tensor[ToIndex(x, y, TENSOR_DIM - 2)] =
                    isTextType ? 1.0f : 0.0f;
                tensor[ToIndex(x, y, TENSOR_DIM - 1)] =
                    game.GetMap().At(x, y).isRule ? 1.0f : 0.0f;
            }
        }
    }

    return tensor;
}
}  // namespace baba_is_auto