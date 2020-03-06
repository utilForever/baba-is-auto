"""
Copyright (c) 2020 Chris Ohk

I am making my contributions/submissions to this project solely in our
personal capacity and am not conveying any rights to any intellectual
property of any third parties.
"""

import pyBaba

def test_map_basic():
    map = pyBaba.Map(5, 5)
    map.AddObject(3, 4, pyBaba.ObjectType.BABA)
    assert map.At(3, 3).HasType(pyBaba.ObjectType.ICON_EMPTY)
    assert map.At(3, 4).HasType(pyBaba.ObjectType.BABA)
