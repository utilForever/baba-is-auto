import pyBaba


def test_public_api_has_descriptive_docstrings():
    assert "C++17 Baba Is You simulator" in pyBaba.__doc__
    assert "loaded from a map file" in pyBaba.Game.__doc__
    assert "active YOU rule" in pyBaba.Game.MovePlayer.__doc__
    assert "text tiles and in-game icons" in pyBaba.ObjectType.__doc__
    assert "current game state" in pyBaba.PlayState.__doc__
    assert "represents a text tile" in pyBaba.IsTextType.__doc__

    documented_apis = (
        (pyBaba.Map, "two-dimensional game board"),
        (pyBaba.Object, "stacked in one map cell"),
        (pyBaba.Rule, "three-tile rule"),
        (pyBaba.RuleManager, "active rules"),
        (pyBaba.IAgent, "choose moves from game states"),
        (pyBaba.Preprocess, "reinforcement learning"),
        (pyBaba.Preprocess.StateToTensor, "feature tensor"),
        (pyBaba.RandomAgent, "random movement direction"),
        (pyBaba.Direction, "movement direction"),
        (pyBaba.RuleDirection, "orientation of a rule"),
        (pyBaba.IsNounType, "represents noun text"),
        (pyBaba.IsOpType, "represents operator text"),
        (pyBaba.IsVerbType, "represents a supported verb"),
        (pyBaba.IsPropertyType, "represents property text"),
        (pyBaba.ConvertIconToText, "matching text type"),
        (pyBaba.ConvertTextToIcon, "matching icon type"),
    )

    for api, description in documented_apis:
        assert description in api.__doc__
