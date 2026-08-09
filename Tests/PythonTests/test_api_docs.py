import inspect

import pyBaba


def test_public_api_has_descriptive_docstrings():
    assert "C++17 Baba Is You simulator" in pyBaba.__doc__
    assert "loaded from a map file" in pyBaba.Game.__doc__
    assert "active YOU rule" in pyBaba.Game.MovePlayer.__doc__
    assert "text tiles and in-game icons" in pyBaba.ObjectType.__doc__
    assert "current game state" in pyBaba.PlayState.__doc__
    assert "represents a text tile" in pyBaba.IsTextType.__doc__

    documented_apis = (
        (pyBaba.Game.__init__, "Loads a map file"),
        (pyBaba.Game.Reset, "Restores the loaded map"),
        (pyBaba.Game.GetMap, "copy of the map owned by the game"),
        (pyBaba.Game.GetRuleManager, "copy of the rule manager"),
        (pyBaba.Game.GetPlayState, "current PlayState"),
        (pyBaba.Game.GetPlayerIcon, "icon type selected by the active YOU rule"),
        (pyBaba.Map, "two-dimensional game board"),
        (pyBaba.Map.__init__, "Creates an empty zero-sized map"),
        (pyBaba.Map.__init__, "empty map with the given dimensions"),
        (pyBaba.Map.Reset, "Restores the objects recorded"),
        (pyBaba.Map.GetWidth, "Returns the map width"),
        (pyBaba.Map.GetHeight, "Returns the map height"),
        (pyBaba.Map.Load, "one-to-three-layer text map"),
        (pyBaba.Map.AddObject, "cell at zero-based (x, y)"),
        (pyBaba.Map.RemoveObject, "cell at zero-based (x, y)"),
        (pyBaba.Map.At, "Object stored at zero-based (x, y)"),
        (pyBaba.Map.GetPositions, "positions containing the object type"),
        (pyBaba.Map.GetPosition, "position for an object ID"),
        (pyBaba.Map.SetDirection, "facing direction for an object ID"),
        (pyBaba.Map.GetDirection, "facing direction for an object ID"),
        (pyBaba.ObjectInstance, "stable identity and facing"),
        (pyBaba.Object, "stacked in one map cell"),
        (pyBaba.Object.__init__, "Creates an empty cell object"),
        (pyBaba.Object.__init__, "supplied object types"),
        (pyBaba.Object.__eq__, "same types"),
        (pyBaba.Object.Add, "Adds one object type"),
        (pyBaba.Object.Remove, "Removes one matching object type"),
        (pyBaba.Object.GetTypes, "duplicate stacked values"),
        (pyBaba.Object.GetInstances, "stable IDs and facing"),
        (pyBaba.Object.HasType, "contains the object type"),
        (pyBaba.Object.HasTextType, "contains any text tile"),
        (pyBaba.Object.HasNounType, "contains any noun text tile"),
        (pyBaba.Object.HasVerbType, "contains any verb text tile"),
        (pyBaba.Object.HasPropertyType, "contains any property text tile"),
        (pyBaba.Rule, "three-tile rule"),
        (pyBaba.Rule.__init__, "Creates a three-tile rule"),
        (pyBaba.Rule.__eq__, "same three objects"),
        (pyBaba.RuleManager, "active rules"),
        (pyBaba.RuleManager.__init__, "Creates an empty rule manager"),
        (pyBaba.RuleManager.AddRule, "Adds the rule"),
        (pyBaba.RuleManager.RemoveRule, "Removes the rule"),
        (pyBaba.RuleManager.ClearRules, "Removes every active rule"),
        (pyBaba.RuleManager.GetRules, "rules containing the object type"),
        (pyBaba.RuleManager.GetNumRules, "number of active rules"),
        (pyBaba.RuleManager.FindPlayer, "first active YOU rule"),
        (pyBaba.RuleManager.HasProperty, "has the active property"),
        (pyBaba.IAgent, "choose moves from game states"),
        (pyBaba.Preprocess, "reinforcement learning"),
        (
            inspect.getattr_static(pyBaba.Preprocess, "TENSOR_DIM"),
            "Number of feature values used for each map cell",
        ),
        (pyBaba.Preprocess.StateToTensor, "feature tensor"),
        (pyBaba.RandomAgent, "random movement direction"),
        (pyBaba.RandomAgent.__init__, "random Direction values"),
        (pyBaba.RandomAgent.GetAction, "random Direction value"),
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
