import pyBaba


def test_public_api_has_descriptive_docstrings():
    assert "C++17 Baba Is You simulator" in pyBaba.__doc__
    assert "loaded from a map file" in pyBaba.Game.__doc__
    assert "active YOU rule" in pyBaba.Game.MovePlayer.__doc__
    assert "text tiles and in-game icons" in pyBaba.ObjectType.__doc__
    assert "current game state" in pyBaba.PlayState.__doc__
    assert "represents a text tile" in pyBaba.IsTextType.__doc__
