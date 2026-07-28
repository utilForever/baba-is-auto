"""
Copyright (c) 2020-2026 Chris Ohk

I am making my contributions/submissions to this project solely in our
personal capacity and am not conveying any rights to any intellectual
property of any third parties.
"""

import pyBaba


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
