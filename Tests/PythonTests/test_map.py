"""
Copyright (c) 2020-2026 Chris Ohk

I am making my contributions/submissions to this project solely in our
personal capacity and am not conveying any rights to any intellectual
property of any third parties.
"""

import pyBaba
import pytest


def test_map_basic():
    game_map = pyBaba.Map(5, 5)
    game_map.AddObject(3, 4, pyBaba.ObjectType.BABA)
    assert game_map.At(3, 3).HasType(pyBaba.ObjectType.ICON_EMPTY)
    assert game_map.At(3, 4).HasType(pyBaba.ObjectType.BABA)


def test_object_duplicate_stack():
    obj = pyBaba.Object()
    obj.Add(pyBaba.ObjectType.ICON_BABA)
    obj.Add(pyBaba.ObjectType.ICON_BABA)

    assert obj.GetTypes() == [
        pyBaba.ObjectType.ICON_BABA,
        pyBaba.ObjectType.ICON_BABA,
    ]


def test_map_object_identity_and_facing():
    game_map = pyBaba.Map(2, 1)

    with pytest.raises(ValueError):
        game_map.AddObject(
            0, 0, pyBaba.ObjectType.ICON_KEKE, pyBaba.Direction.NONE
        )

    game_map.AddObject(
        0, 0, pyBaba.ObjectType.ICON_KEKE, pyBaba.Direction.UP
    )
    game_map.AddObject(1, 0, pyBaba.ObjectType.ICON_LOVE)

    keke = game_map.At(0, 0).GetInstances()[0]
    love = game_map.At(1, 0).GetInstances()[0]
    assert keke.id != love.id
    assert keke.direction == pyBaba.Direction.UP
    assert love.direction == pyBaba.Direction.RIGHT
    assert game_map.GetPosition(keke.id) == (0, 0)
    assert game_map.GetDirection(keke.id) == pyBaba.Direction.UP
    assert game_map.SetDirection(keke.id, pyBaba.Direction.LEFT)
    assert game_map.GetDirection(keke.id) == pyBaba.Direction.LEFT
    assert not game_map.SetDirection(keke.id, pyBaba.Direction.NONE)
    assert game_map.GetDirection(keke.id) == pyBaba.Direction.LEFT
