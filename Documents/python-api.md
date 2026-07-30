@page python_api Python API Reference

# Install

```sh
python -m pip install .
```

# Quick start

```python
import pyBaba

game = pyBaba.Game("Resources/Maps/baba_is_you.txt")
game.MovePlayer(pyBaba.Direction.RIGHT)
print(game.GetPlayState())
```

Map coordinates are zero-based `(x, y)` pairs.

# Games

| API                                   | Description                                                                                                |
| ------------------------------------- | ---------------------------------------------------------------------------------------------------------- |
| `Game(filename)`                      | Loads `filename`, parses its rules, and starts a game. Raises `RuntimeError` when the map data is invalid. |
| `Game.Reset()`                        | Restores the loaded map and recalculates its initial rules and play state.                                 |
| `Game.GetMap()`                       | Returns the live map owned by the game.                                                                    |
| `Game.GetRuleManager()`               | Returns the live rule manager owned by the game.                                                           |
| `Game.GetPlayState()`                 | Returns the current `PlayState`.                                                                           |
| `Game.GetPlayerIcon()`                | Returns the icon type selected by the active `YOU` rule.                                                   |
| `Game.MovePlayer(direction)`          | Moves every object controlled by the active `YOU` rule in `direction`, then rebuilds rules and play state. |
| `Map()`                               | Creates an empty zero-sized map.                                                                           |
| `Map(width, height)`                  | Creates an empty map with the given dimensions.                                                            |
| `Map.Reset()`                         | Restores the objects recorded when the map was loaded.                                                     |
| `Map.GetWidth()`                      | Returns the map width.                                                                                     |
| `Map.GetHeight()`                     | Returns the map height.                                                                                    |
| `Map.Load(filename)`                  | Loads a one-to-three-layer text map. Raises `RuntimeError` when the map data is invalid.                   |
| `Map.AddObject(x, y, object_type)`    | Adds one `object_type` to the cell at zero-based `(x, y)`.                                                 |
| `Map.RemoveObject(x, y, object_type)` | Removes one `object_type` from the cell at zero-based `(x, y)`.                                            |
| `Map.At(x, y)`                        | Returns the live `Object` stored at zero-based `(x, y)`.                                                   |
| `Map.GetPositions(object_type)`       | Returns all zero-based `(x, y)` positions containing `object_type`.                                        |
| `Object()`                            | Creates an empty cell object.                                                                              |
| `Object(object_types)`                | Creates a cell object containing the supplied object types.                                                |
| `Object.__eq__(other)`                | Returns whether two cell objects contain the same types.                                                   |
| `Object.Add(object_type)`             | Adds one object type to the cell.                                                                          |
| `Object.Remove(object_type)`          | Removes one matching object type from the cell.                                                            |
| `Object.GetTypes()`                   | Returns all object types, including duplicate stacked values.                                              |
| `Object.HasType(object_type)`         | Returns whether the cell contains `object_type`.                                                           |
| `Object.HasTextType()`                | Returns whether the cell contains any text tile.                                                           |
| `Object.HasNounType()`                | Returns whether the cell contains any noun text tile.                                                      |
| `Object.HasVerbType()`                | Returns whether the cell contains any verb text tile.                                                      |
| `Object.HasPropertyType()`            | Returns whether the cell contains any property text tile.                                                  |

# Rules

| API                                                    | Description                                                           |
| ------------------------------------------------------ | --------------------------------------------------------------------- |
| `Rule(subject, operator, predicate)`                   | Creates a three-tile rule.                                            |
| `Rule.__eq__(other)`                                   | Returns whether two rules contain the same three objects.             |
| `RuleManager()`                                        | Creates an empty rule manager.                                        |
| `RuleManager.AddRule(rule)`                            | Adds `rule` to the active rule list.                                  |
| `RuleManager.RemoveRule(rule)`                         | Removes `rule` when it is present.                                    |
| `RuleManager.ClearRules()`                             | Removes every active rule.                                            |
| `RuleManager.GetRules(object_type)`                    | Returns rules containing `object_type` in any of their three objects. |
| `RuleManager.GetNumRules()`                            | Returns the number of active rules.                                   |
| `RuleManager.FindPlayer()`                             | Returns the icon type selected by the first active `YOU` rule.        |
| `RuleManager.HasProperty(object_types, property_type)` | Returns whether any supplied object type has the active property.     |

# Agents

| API                              | Description                                                    |
| -------------------------------- | -------------------------------------------------------------- |
| `IAgent`                         | Interface for agents that choose moves from game states.       |
| `RandomAgent()`                  | Creates an agent that chooses random `Direction` values.       |
| `RandomAgent.GetAction(game)`    | Returns a random `Direction` value for `game`.                 |
| `Preprocess`                     | Utilities that convert game states for reinforcement learning. |
| `Preprocess.TENSOR_DIM`          | Number of feature values used for each map cell.               |
| `Preprocess.StateToTensor(game)` | Flattens `game` into a feature tensor.                         |

# Enums

| API             | Description                                                                                                                                                                    |
| --------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `ObjectType`    | Identifies text tiles and in-game icons. Values use the exact names in `NounType.def`, `OpType.def`, `PropertyType.def`, and `IconType.def`, plus the four category sentinels. |
| `PlayState`     | Identifies the current game outcome: `INVALID`, `PLAYING`, `WON`, or `LOST`.                                                                                                   |
| `Direction`     | Identifies a movement direction: `NONE`, `UP`, `DOWN`, `LEFT`, or `RIGHT`.                                                                                                     |
| `RuleDirection` | Identifies a rule orientation: `HORIZONTAL` or `VERTICAL`.                                                                                                                     |

# Enum helpers

| API                              | Description                                                                           |
| -------------------------------- | ------------------------------------------------------------------------------------- |
| `IsTextType(object_type)`        | Returns whether an object type represents a text tile.                                |
| `IsNounType(object_type)`        | Returns whether an object type represents noun text.                                  |
| `IsOpType(object_type)`          | Returns whether an object type represents operator text.                              |
| `IsVerbType(object_type)`        | Returns whether an object type represents a supported verb.                           |
| `IsPropertyType(object_type)`    | Returns whether an object type represents property text.                              |
| `ConvertIconToText(object_type)` | Converts an icon type to its matching text type; other values are returned unchanged. |
| `ConvertTextToIcon(object_type)` | Converts a text type to its matching icon type; icon values are returned unchanged.   |
