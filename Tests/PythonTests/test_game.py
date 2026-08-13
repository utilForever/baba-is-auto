"""
Copyright (c) 2020-2026 Chris Ohk

I am making my contributions/submissions to this project solely in our
personal capacity and am not conveying any rights to any intellectual
property of any third parties.
"""

from pathlib import Path

import pyBaba


def _move(game, actions):
    directions = {
        "U": pyBaba.Direction.UP,
        "D": pyBaba.Direction.DOWN,
        "L": pyBaba.Direction.LEFT,
        "R": pyBaba.Direction.RIGHT,
    }

    for action in actions:
        game.MovePlayer(directions[action])


def _write_level(source, destination, replacements):
    values = source.read_text().split()
    width, height = map(int, values[:2])
    tiles = values[2:]

    for x, y, object_type in replacements:
        tiles[y * width + x] = str(object_type.value)

    rows = (" ".join(tiles[y * width : (y + 1) * width]) for y in range(height))

    destination.write_text(f"{width} {height}\n" + "\n".join(rows))
    return destination


def test_game_basic():
    game = pyBaba.Game("Resources/Maps/baba_is_you.txt")
    assert game.GetMap().At(1, 4).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetMap().At(9, 4).HasType(pyBaba.ObjectType.ICON_FLAG)
    assert game.GetRuleManager().GetNumRules() == 4
    assert game.GetPlayerIcon() == pyBaba.ObjectType.ICON_BABA

    pos = game.GetMap().GetPositions(game.GetPlayerIcon())
    assert len(pos) == 1
    assert pos[0][0] == 1
    assert pos[0][1] == 4

    game.MovePlayer(pyBaba.Direction.UP)
    assert game.GetMap().At(1, 3).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetMap().At(1, 4).HasType(pyBaba.ObjectType.ICON_EMPTY)

    game.MovePlayer(pyBaba.Direction.UP)
    assert game.GetMap().At(1, 3).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetMap().At(1, 2).HasType(pyBaba.ObjectType.ICON_WALL)

    game.MovePlayer(pyBaba.Direction.RIGHT)
    game.MovePlayer(pyBaba.Direction.RIGHT)
    game.MovePlayer(pyBaba.Direction.RIGHT)
    assert game.GetMap().At(4, 3).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetMap().At(3, 3).HasType(pyBaba.ObjectType.ICON_TILE)

    game.MovePlayer(pyBaba.Direction.RIGHT)
    assert game.GetMap().At(5, 3).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetMap().At(6, 3).HasType(pyBaba.ObjectType.ICON_ROCK)
    assert game.GetMap().At(4, 3).HasType(pyBaba.ObjectType.ICON_TILE)

    game.MovePlayer(pyBaba.Direction.RIGHT)
    game.MovePlayer(pyBaba.Direction.DOWN)
    assert game.GetPlayState() == pyBaba.PlayState.PLAYING

    game.MovePlayer(pyBaba.Direction.RIGHT)
    game.MovePlayer(pyBaba.Direction.RIGHT)
    game.MovePlayer(pyBaba.Direction.RIGHT)
    assert game.GetPlayState() == pyBaba.PlayState.WON

    game.Reset()
    assert game.GetMap().At(1, 4).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetMap().At(9, 4).HasType(pyBaba.ObjectType.ICON_FLAG)
    assert game.GetRuleManager().GetNumRules() == 4
    assert game.GetPlayerIcon() == pyBaba.ObjectType.ICON_BABA
    assert game.GetPlayState() == pyBaba.PlayState.PLAYING


def test_game_random_seed_repeats_empty_directions(tmp_path):
    level = _write_level(
        Path("Resources/Maps/special_transformations.txt"),
        tmp_path / "seeded_empty.txt",
        [
            (0, 1, pyBaba.ObjectType.EMPTY),
            (1, 1, pyBaba.ObjectType.IS),
            (2, 1, pyBaba.ObjectType.KEKE),
        ],
    )
    first = pyBaba.Game(str(level))
    second = pyBaba.Game(str(level))

    for game in (first, second):
        game.SetRandomSeed(12345)
        game.MovePlayer(pyBaba.Direction.NONE)

    def directions(game):
        game_map = game.GetMap()
        return [
            instance.direction
            for y in range(game_map.GetHeight())
            for x in range(game_map.GetWidth())
            for instance in game_map.At(x, y).GetInstances()
            if instance.type == pyBaba.ObjectType.ICON_KEKE
        ]

    first_directions = directions(first)
    assert first_directions
    assert first_directions == directions(second)


def test_game_stacked_empty_move_advances_each_step(tmp_path):
    level = _write_level(
        Path("Resources/Maps/special_transformations.txt"),
        tmp_path / "empty_move_stacks.txt",
        [
            (0, 1, pyBaba.ObjectType.EMPTY),
            (1, 1, pyBaba.ObjectType.IS),
            (2, 1, pyBaba.ObjectType.UP),
            (0, 2, pyBaba.ObjectType.EMPTY),
            (1, 2, pyBaba.ObjectType.IS),
            (2, 2, pyBaba.ObjectType.MOVE),
            (3, 2, pyBaba.ObjectType.AND),
            (4, 2, pyBaba.ObjectType.MOVE),
            (7, 0, pyBaba.ObjectType.ICON_WALL),
            (7, 1, pyBaba.ObjectType.ICON_WALL),
            (7, 2, pyBaba.ObjectType.BABA),
        ],
    )
    game = pyBaba.Game(str(level))

    game.MovePlayer(pyBaba.Direction.NONE)

    game_map = game.GetMap()
    assert game_map.At(7, 0).HasType(pyBaba.ObjectType.BABA)
    assert game_map.At(7, 1).HasType(pyBaba.ObjectType.ICON_WALL)
    assert game_map.At(7, 2).HasType(pyBaba.ObjectType.ICON_EMPTY)


def test_game_empty_move_turns_before_moving_away_from_obstacle(tmp_path):
    level = _write_level(
        Path("Resources/Maps/special_transformations.txt"),
        tmp_path / "empty_move_bounce.txt",
        [
            (0, 1, pyBaba.ObjectType.EMPTY),
            (1, 1, pyBaba.ObjectType.IS),
            (2, 1, pyBaba.ObjectType.UP),
            (0, 2, pyBaba.ObjectType.EMPTY),
            (1, 2, pyBaba.ObjectType.NEAR),
            (2, 2, pyBaba.ObjectType.LOVE),
            (3, 2, pyBaba.ObjectType.IS),
            (4, 2, pyBaba.ObjectType.MOVE),
            (6, 0, pyBaba.ObjectType.ICON_LOVE),
            (7, 1, pyBaba.ObjectType.BABA),
        ],
    )
    game = pyBaba.Game(str(level))

    game.MovePlayer(pyBaba.Direction.NONE)

    game_map = game.GetMap()
    assert game_map.At(7, 1).HasType(pyBaba.ObjectType.BABA)
    assert game_map.At(7, 2).HasType(pyBaba.ObjectType.ICON_EMPTY)


def test_game_empty_move_cannot_start_in_locked_direction(tmp_path):
    level = _write_level(
        Path("Resources/Maps/special_transformations.txt"),
        tmp_path / "empty_move_locked.txt",
        [
            (0, 1, pyBaba.ObjectType.EMPTY),
            (1, 1, pyBaba.ObjectType.IS),
            (2, 1, pyBaba.ObjectType.UP),
            (0, 2, pyBaba.ObjectType.EMPTY),
            (1, 2, pyBaba.ObjectType.IS),
            (2, 2, pyBaba.ObjectType.LOCKED_UP),
            (0, 3, pyBaba.ObjectType.EMPTY),
            (1, 3, pyBaba.ObjectType.NEAR),
            (2, 3, pyBaba.ObjectType.LOVE),
            (3, 3, pyBaba.ObjectType.IS),
            (4, 3, pyBaba.ObjectType.MOVE),
            (6, 2, pyBaba.ObjectType.ICON_LOVE),
            (7, 1, pyBaba.ObjectType.BABA),
        ],
    )
    game = pyBaba.Game(str(level))

    game.MovePlayer(pyBaba.Direction.NONE)

    game_map = game.GetMap()
    assert game_map.At(7, 1).HasType(pyBaba.ObjectType.BABA)
    assert game_map.At(7, 0).HasType(pyBaba.ObjectType.ICON_EMPTY)


def test_game_empty_move_recalculates_positions_between_steps():
    game = pyBaba.Game("Resources/Maps/empty_move_recalculation.txt")

    game.MovePlayer(pyBaba.Direction.NONE)

    game_map = game.GetMap()
    assert game_map.At(9, 0).HasType(pyBaba.ObjectType.BABA)
    assert game_map.At(9, 1).HasType(pyBaba.ObjectType.ICON_EMPTY)


def test_game_grass_yard():
    game = pyBaba.Game("Resources/Maps/grass_yard.txt")
    assert game.GetMap().GetWidth() == 24
    assert game.GetMap().GetHeight() == 14
    assert game.GetRuleManager().GetNumRules() == 2
    assert game.GetMap().At(9, 7).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetMap().At(12, 7).HasType(pyBaba.ObjectType.ICON_GRASS)

    for _ in range(3):
        game.MovePlayer(pyBaba.Direction.RIGHT)
    assert game.GetMap().At(11, 7).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetMap().At(12, 7).HasType(pyBaba.ObjectType.ICON_GRASS)

    game.Reset()
    assert game.GetMap().At(9, 7).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetRuleManager().GetNumRules() == 2
    assert game.GetPlayState() == pyBaba.PlayState.PLAYING

    directions = {
        "U": pyBaba.Direction.UP,
        "D": pyBaba.Direction.DOWN,
        "L": pyBaba.Direction.LEFT,
        "R": pyBaba.Direction.RIGHT,
    }
    solution = (
        "URRRRDRDRRRDRUUUURULLLLLLLULDRDLDRRDDRDDRDLLLDLUUUUUUULLRR"
        "UURULLLULDRRRDDRRRRR"
    )

    for action in solution:
        game.MovePlayer(directions[action])

    assert game.GetPlayState() == pyBaba.PlayState.WON


def test_game_arbitrary_and_chains():
    game = pyBaba.Game("Resources/Maps/and_chains.txt")
    rules = game.GetRuleManager()
    baba_you = pyBaba.Rule(
        pyBaba.Object([pyBaba.ObjectType.BABA]),
        pyBaba.Object([pyBaba.ObjectType.IS]),
        pyBaba.Object([pyBaba.ObjectType.YOU]),
    )

    assert rules.GetNumRules() == 10
    assert baba_you in rules.GetRules(pyBaba.ObjectType.YOU)
    assert rules.HasProperty([pyBaba.ObjectType.ICON_BABA], pyBaba.ObjectType.YOU)
    assert rules.HasProperty([pyBaba.ObjectType.BABA], pyBaba.ObjectType.YOU)
    assert rules.HasProperty([pyBaba.ObjectType.ICON_WALL], pyBaba.ObjectType.YOU)
    assert rules.HasProperty([pyBaba.ObjectType.ICON_ROCK], pyBaba.ObjectType.YOU)
    assert rules.HasProperty([pyBaba.ObjectType.ICON_BABA], pyBaba.ObjectType.PUSH)
    assert rules.HasProperty([pyBaba.ObjectType.ICON_BABA], pyBaba.ObjectType.SINK)


def test_rule_conditions_are_visible_in_python():
    assert hasattr(pyBaba, "RuleCondition")

    default = pyBaba.RuleCondition()
    assert default.op == pyBaba.ObjectType.ON
    assert default.targets == []
    assert not default.negated

    condition = pyBaba.RuleCondition()
    condition.op = pyBaba.ObjectType.ON
    condition.targets = [pyBaba.ObjectType.LOVE]
    condition.negated = True

    same = pyBaba.RuleCondition()
    same.op = pyBaba.ObjectType.ON
    same.targets = [pyBaba.ObjectType.LOVE]
    same.negated = True
    assert condition == same
    assert condition != default

    rule = pyBaba.Rule(
        pyBaba.Object([pyBaba.ObjectType.BABA]),
        pyBaba.Object([pyBaba.ObjectType.IS]),
        pyBaba.Object([pyBaba.ObjectType.MOVE]),
        [condition],
    )

    assert rule.conditions == [condition]
    assert rule != pyBaba.Rule(
        pyBaba.Object([pyBaba.ObjectType.BABA]),
        pyBaba.Object([pyBaba.ObjectType.IS]),
        pyBaba.Object([pyBaba.ObjectType.MOVE]),
    )


def test_game_rule_after_leading_and():
    game = pyBaba.Game("Resources/Maps/and_chains.txt")
    assert game.GetRuleManager().HasProperty(
        [pyBaba.ObjectType.ICON_ICE], pyBaba.ObjectType.STOP
    )


def test_game_stacked_and_and_verb():
    game = pyBaba.Game("Resources/Maps/and_chains.txt")
    assert len(game.GetRuleManager().GetRules(pyBaba.ObjectType.JELLY)) == 1


def test_game_rule_after_stacked_and_and_verb():
    game = pyBaba.Game("Resources/Maps/and_chains.txt")
    assert game.GetRuleManager().HasProperty(
        [pyBaba.ObjectType.ICON_ICE], pyBaba.ObjectType.PUSH
    )


def test_game_icy_waters_load_stop_and_reset():
    game = pyBaba.Game("Resources/Maps/icy_waters.txt")
    assert game.GetMap().GetWidth() == 28
    assert game.GetMap().GetHeight() == 16
    assert game.GetRuleManager().GetNumRules() == 5
    assert game.GetMap().At(7, 5).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetMap().At(24, 4).HasType(pyBaba.ObjectType.ICON_FLAG)

    _move(game, "UU")
    assert game.GetMap().At(7, 4).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetMap().At(7, 3).HasType(pyBaba.ObjectType.ICON_WALL)

    game.Reset()
    assert game.GetMap().At(7, 5).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetPlayState() == pyBaba.PlayState.PLAYING


def test_game_icy_waters_rule_sink_and_win():
    game = pyBaba.Game("Resources/Maps/icy_waters.txt")

    _move(game, "DDDDRRRRRDD")
    assert game.GetMap().At(12, 11).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetMap().At(12, 12).HasType(pyBaba.ObjectType.SINK)
    assert game.GetRuleManager().HasProperty(
        [pyBaba.ObjectType.ICON_BABA], pyBaba.ObjectType.YOU
    )
    assert not game.GetRuleManager().HasProperty(
        [pyBaba.ObjectType.ICON_BABA], pyBaba.ObjectType.SINK
    )

    _move(game, "UUUURRRRRRRRRRR")
    assert game.GetMap().At(23, 7).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetMap().At(24, 7).HasType(pyBaba.ObjectType.WALL)

    _move(game, "DRU")
    assert not game.GetMap().At(24, 6).HasType(pyBaba.ObjectType.WALL)
    assert not game.GetMap().At(24, 6).HasType(pyBaba.ObjectType.ICON_JELLY)

    _move(game, "UUU")
    assert game.GetPlayState() == pyBaba.PlayState.WON


def test_game_icy_waters_sink_from_either_side(tmp_path):
    source = Path("Resources/Maps/icy_waters.txt")
    moving_level = _write_level(
        source,
        tmp_path / "moving_sink.txt",
        [(7, 6, pyBaba.ObjectType.ICON_ICE)],
    )
    moving_sink = pyBaba.Game(str(moving_level))

    moving_sink.MovePlayer(pyBaba.Direction.DOWN)
    assert not moving_sink.GetMap().At(7, 6).HasType(pyBaba.ObjectType.ICON_BABA)
    assert not moving_sink.GetMap().At(7, 6).HasType(pyBaba.ObjectType.ICON_ICE)
    assert moving_sink.GetPlayState() == pyBaba.PlayState.LOST

    destination_level = _write_level(
        source,
        tmp_path / "destination_sink.txt",
        [(12, 10, pyBaba.ObjectType.ICON_JELLY)],
    )
    destination_sink = pyBaba.Game(str(destination_level))
    _move(destination_sink, "DDDDRRRRRDD")

    destination_sink.MovePlayer(pyBaba.Direction.UP)
    assert not destination_sink.GetMap().At(12, 10).HasType(pyBaba.ObjectType.ICON_BABA)
    assert (
        not destination_sink.GetMap().At(12, 10).HasType(pyBaba.ObjectType.ICON_JELLY)
    )
    assert destination_sink.GetPlayState() == pyBaba.PlayState.LOST


def test_game_turns_load_and_reset():
    game = pyBaba.Game("Resources/Maps/turns.txt")
    assert game.GetMap().GetWidth() == 28
    assert game.GetMap().GetHeight() == 16
    assert game.GetRuleManager().GetNumRules() == 6
    assert game.GetMap().At(23, 8).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetMap().At(2, 6).HasType(pyBaba.ObjectType.ICON_CRAB)
    assert game.GetMap().At(10, 6).HasType(pyBaba.ObjectType.ICON_STAR)
    assert game.GetMap().At(8, 6).HasType(pyBaba.ObjectType.ICON_SKULL)

    game.MovePlayer(pyBaba.Direction.LEFT)
    game.Reset()
    assert game.GetMap().At(23, 8).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetMap().At(2, 6).HasType(pyBaba.ObjectType.ICON_CRAB)
    assert game.GetRuleManager().GetNumRules() == 6
    assert game.GetPlayState() == pyBaba.PlayState.PLAYING


def test_game_turns_rules_multiple_you_and_win():
    game = pyBaba.Game("Resources/Maps/turns.txt")

    _move(game, "LULLLDDDLLLLULURDRUUULLDRRDRUU")
    assert game.GetRuleManager().HasProperty(
        [pyBaba.ObjectType.ICON_STAR], pyBaba.ObjectType.PUSH
    )
    assert game.GetRuleManager().HasProperty(
        [pyBaba.ObjectType.ICON_STAR], pyBaba.ObjectType.SINK
    )

    _move(game, "LLLLLLLL")
    assert not game.GetMap().GetPositions(pyBaba.ObjectType.ICON_STAR)
    assert not game.GetMap().GetPositions(pyBaba.ObjectType.ICON_SKULL)

    _move(game, "LLDLURRRRRRRRRRRUULDRDLLLLLLLLLLUULDD")
    assert game.GetRuleManager().HasProperty(
        [pyBaba.ObjectType.ICON_CRAB], pyBaba.ObjectType.YOU
    )
    assert game.GetRuleManager().HasProperty(
        [pyBaba.ObjectType.ICON_BABA], pyBaba.ObjectType.YOU
    )
    assert game.GetMap().At(2, 6).HasType(pyBaba.ObjectType.ICON_CRAB)
    assert game.GetMap().At(6, 6).HasType(pyBaba.ObjectType.ICON_BABA)

    game.MovePlayer(pyBaba.Direction.RIGHT)
    assert game.GetMap().At(3, 6).HasType(pyBaba.ObjectType.ICON_CRAB)
    assert game.GetMap().At(7, 6).HasType(pyBaba.ObjectType.ICON_BABA)
    assert not game.GetMap().At(2, 6).HasType(pyBaba.ObjectType.ICON_CRAB)
    assert not game.GetMap().At(6, 6).HasType(pyBaba.ObjectType.ICON_BABA)

    _move(game, "DDDDD")
    assert game.GetMap().At(3, 11).HasType(pyBaba.ObjectType.ICON_CRAB)
    assert game.GetPlayState() == pyBaba.PlayState.WON


def test_game_sink_reparses_destroyed_rule_text():
    game = pyBaba.Game("Resources/Maps/sink_rule_reparse.txt")

    game.MovePlayer(pyBaba.Direction.UP)
    assert not game.GetMap().At(0, 0).HasType(pyBaba.ObjectType.FLAG)
    assert not game.GetMap().At(0, 0).HasType(pyBaba.ObjectType.ICON_JELLY)
    assert not game.GetRuleManager().HasProperty(
        [pyBaba.ObjectType.ICON_FLAG], pyBaba.ObjectType.WIN
    )
    assert game.GetRuleManager().GetNumRules() == 2


def test_game_lost():
    game = pyBaba.Game("Resources/Maps/simple_map.txt")
    assert game.GetMap().At(0, 2).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetRuleManager().GetNumRules() == 1
    assert game.GetPlayState() == pyBaba.PlayState.PLAYING

    game.MovePlayer(pyBaba.Direction.UP)
    assert game.GetRuleManager().GetNumRules() == 0
    assert game.GetPlayState() == pyBaba.PlayState.LOST


def test_game_layered_map():
    game = pyBaba.Game("Resources/Maps/layered_map.txt")
    assert game.GetMap().At(0, 0).HasType(pyBaba.ObjectType.BABA)
    assert game.GetMap().At(0, 0).HasType(pyBaba.ObjectType.WALL)
    assert game.GetMap().At(0, 1).HasType(pyBaba.ObjectType.ICON_ROCK)

    game.MovePlayer(pyBaba.Direction.RIGHT)
    assert game.GetMap().At(0, 1).HasType(pyBaba.ObjectType.ICON_ROCK)
    assert game.GetMap().At(1, 1).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetMap().At(1, 1).HasType(pyBaba.ObjectType.ICON_WALL)
    assert game.GetMap().At(2, 1).HasType(pyBaba.ObjectType.FLAG)
    assert game.GetMap().At(2, 1).HasType(pyBaba.ObjectType.ROCK)


def test_game_sink():
    game = pyBaba.Game("Resources/Maps/out_of_reach.txt")
    assert game.GetMap().At(9, 3).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetRuleManager().GetNumRules() == 5
    assert game.GetPlayerIcon() == pyBaba.ObjectType.ICON_BABA

    game.MovePlayer(pyBaba.Direction.UP)
    game.MovePlayer(pyBaba.Direction.RIGHT)
    game.MovePlayer(pyBaba.Direction.RIGHT)
    game.MovePlayer(pyBaba.Direction.RIGHT)
    game.MovePlayer(pyBaba.Direction.RIGHT)
    game.MovePlayer(pyBaba.Direction.DOWN)
    game.MovePlayer(pyBaba.Direction.LEFT)
    game.MovePlayer(pyBaba.Direction.LEFT)
    game.MovePlayer(pyBaba.Direction.UP)
    game.MovePlayer(pyBaba.Direction.LEFT)
    game.MovePlayer(pyBaba.Direction.DOWN)
    game.MovePlayer(pyBaba.Direction.DOWN)
    game.MovePlayer(pyBaba.Direction.DOWN)
    assert game.GetMap().At(10, 5).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetMap().At(10, 6).HasType(pyBaba.ObjectType.ICON_ROCK)

    game.MovePlayer(pyBaba.Direction.DOWN)
    assert game.GetMap().At(10, 6).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetMap().At(10, 7).HasType(pyBaba.ObjectType.ICON_ROCK) is False
    assert game.GetMap().At(10, 7).HasType(pyBaba.ObjectType.ICON_WATER) is False

    game.MovePlayer(pyBaba.Direction.DOWN)
    assert game.GetMap().At(10, 7).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetPlayState() == pyBaba.PlayState.PLAYING


def test_game_newly_active_sink():
    game = pyBaba.Game("Resources/Maps/sink_rule_timing.txt")

    game.MovePlayer(pyBaba.Direction.UP)
    assert not game.GetMap().At(4, 2).HasType(pyBaba.ObjectType.ICON_ROCK)
    assert not game.GetMap().At(4, 2).HasType(pyBaba.ObjectType.ICON_ICE)


def test_game_empty_sink_is_inert():
    game = pyBaba.Game("Resources/Maps/empty_sink.txt")

    game.MovePlayer(pyBaba.Direction.RIGHT)
    assert game.GetMap().At(1, 2).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetPlayState() == pyBaba.PlayState.PLAYING


def test_game_defeat_ignores_pushed_objects():
    game = pyBaba.Game("Resources/Maps/defeat_push.txt")

    game.MovePlayer(pyBaba.Direction.RIGHT)
    assert game.GetMap().At(2, 2).HasType(pyBaba.ObjectType.ICON_ROCK)
    assert game.GetMap().At(2, 2).HasType(pyBaba.ObjectType.ICON_ICE)


def test_game_hot_melt(tmp_path):
    level = _write_level(
        Path("Resources/Maps/volcano.txt"),
        tmp_path / "volcano.txt",
        (
            (12, 1, pyBaba.ObjectType.ICON_LAVA),
            (13, 1, pyBaba.ObjectType.ICON_ROCK),
            (25, 14, pyBaba.ObjectType.ROCK),
            (26, 14, pyBaba.ObjectType.IS),
            (27, 14, pyBaba.ObjectType.MELT),
        ),
    )
    game = pyBaba.Game(str(level))

    game.MovePlayer(pyBaba.Direction.LEFT)
    assert game.GetMap().At(13, 1).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetMap().At(12, 1).HasType(pyBaba.ObjectType.ICON_ROCK) is False
    assert game.GetMap().At(12, 1).HasType(pyBaba.ObjectType.ICON_LAVA)

    game.MovePlayer(pyBaba.Direction.LEFT)
    assert game.GetMap().At(12, 1).HasType(pyBaba.ObjectType.ICON_BABA) is False
    assert game.GetMap().At(12, 1).HasType(pyBaba.ObjectType.ICON_LAVA)
    assert game.GetPlayState() == pyBaba.PlayState.LOST


def test_game_move_wait_stacks_weak_and_locks():
    game = pyBaba.Game("Resources/Maps/move_rules.txt")
    rock = game.GetMap().At(5, 11).GetInstances()[0].id

    game.MovePlayer(pyBaba.Direction.NONE)
    assert game.GetMap().At(11, 8).HasType(pyBaba.ObjectType.ICON_KEKE)
    assert game.GetMap().At(5, 8).HasType(pyBaba.ObjectType.ICON_KEKE)
    assert game.GetMap().At(6, 8).HasType(pyBaba.ObjectType.ICON_ROBOT)
    assert not game.GetMap().GetPositions(pyBaba.ObjectType.ICON_ALGAE)
    assert game.GetMap().GetPosition(rock) == (5, 11)
    assert game.GetMap().GetDirection(rock) == pyBaba.Direction.RIGHT


def test_game_move_special_noun_conditions():
    game = pyBaba.Game("Resources/Maps/move_special_conditions.txt")

    game.MovePlayer(pyBaba.Direction.NONE)
    assert game.GetMap().At(2, 5).HasType(pyBaba.ObjectType.ICON_BABA)
    assert game.GetMap().At(9, 5).HasType(pyBaba.ObjectType.ICON_LOVE)
    assert game.GetMap().At(3, 8).HasType(pyBaba.ObjectType.ICON_KEKE)


def test_game_later_empty_direction_rule_controls_facing():
    game = pyBaba.Game("Resources/Maps/conditional_empty_assigned_facing.txt")

    game.MovePlayer(pyBaba.Direction.NONE)
    instances = game.GetMap().At(2, 3).GetInstances()
    assert len(instances) == 1
    assert instances[0].type == pyBaba.ObjectType.ICON_KEKE
    assert instances[0].direction == pyBaba.Direction.UP


def test_game_noun_transformations_are_snapshotted():
    game = pyBaba.Game("Resources/Maps/transformations.txt")
    keke = game.GetMap().At(0, 9).GetInstances()[0].id

    game.MovePlayer(pyBaba.Direction.NONE)
    assert game.GetMap().At(0, 9).HasType(pyBaba.ObjectType.ICON_LOVE)
    assert game.GetMap().At(0, 9).HasType(pyBaba.ObjectType.ICON_ROCK)
    assert game.GetMap().GetDirection(keke) == pyBaba.Direction.RIGHT
    assert game.GetMap().At(3, 9).HasType(pyBaba.ObjectType.ICON_ALGAE)
    assert game.GetMap().At(5, 9).HasType(pyBaba.ObjectType.ICON_WALL)
    assert game.GetMap().At(6, 9).HasType(pyBaba.ObjectType.ICON_ROCK)
    assert game.GetMap().At(12, 9).HasType(pyBaba.ObjectType.ICON_EMPTY)
    assert game.GetMap().At(19, 10).HasType(pyBaba.ObjectType.ICON_FLOWER)


def test_game_text_transformation_writes_the_source_noun(tmp_path):
    level = _write_level(
        Path("Resources/Maps/special_transformations.txt"),
        tmp_path / "text_transformation.txt",
        (
            (0, 3, pyBaba.ObjectType.BABA),
            (1, 3, pyBaba.ObjectType.IS),
            (2, 3, pyBaba.ObjectType.TEXT),
        ),
    )
    game = pyBaba.Game(str(level))

    game.MovePlayer(pyBaba.Direction.NONE)
    assert game.GetMap().At(0, 0).HasType(pyBaba.ObjectType.BABA)
    assert not game.GetMap().At(0, 0).HasType(pyBaba.ObjectType.ICON_TEXT)


def test_game_all_transformation_expands_without_a_self_guard(tmp_path):
    level = _write_level(
        Path("Resources/Maps/special_transformations.txt"),
        tmp_path / "all_transformation.txt",
        (
            (0, 3, pyBaba.ObjectType.BABA),
            (1, 3, pyBaba.ObjectType.IS),
            (2, 3, pyBaba.ObjectType.ALL),
        ),
    )
    game = pyBaba.Game(str(level))

    game.MovePlayer(pyBaba.Direction.NONE)

    result = game.GetMap().At(0, 0)
    assert result.HasType(pyBaba.ObjectType.ICON_BABA)
    assert result.HasType(pyBaba.ObjectType.ICON_ROCK)
    assert result.HasType(pyBaba.ObjectType.ICON_LOVE)
    assert len(result.GetInstances()) == 3


def test_game_affection_layout_directions_and_wait():
    game = pyBaba.Game("Resources/Maps/affection.txt")
    assert game.GetMap().GetWidth() == 24
    assert game.GetMap().GetHeight() == 14

    right = game.GetMap().At(6, 3).GetInstances()[0].id
    up = game.GetMap().At(7, 7).GetInstances()[0].id
    down = game.GetMap().At(12, 10).GetInstances()[0].id
    assert game.GetMap().GetDirection(right) == pyBaba.Direction.RIGHT
    assert game.GetMap().GetDirection(up) == pyBaba.Direction.UP
    assert game.GetMap().GetDirection(down) == pyBaba.Direction.DOWN

    game.MovePlayer(pyBaba.Direction.NONE)
    assert game.GetMap().At(7, 3).HasType(pyBaba.ObjectType.ICON_KEKE)
    assert game.GetMap().At(7, 6).HasType(pyBaba.ObjectType.ICON_KEKE)
    assert game.GetMap().At(12, 11).HasType(pyBaba.ObjectType.ICON_KEKE)


def test_game_affection_move_solution():
    game = pyBaba.Game("Resources/Maps/affection.txt")

    _move(game, "RRRRRUUUUURRRDDDD")

    game.MovePlayer(pyBaba.Direction.NONE)
    game.MovePlayer(pyBaba.Direction.NONE)

    _move(game, "DUUUUURRRRRRRRDDD")
    assert game.GetPlayState() == pyBaba.PlayState.WON


def test_game_affection_transformation_solution():
    game = pyBaba.Game("Resources/Maps/affection.txt")

    _move(game, "URRRRRRRRDDUULDDUULLDRRDRUUUURUURU")
    assert game.GetPlayState() == pyBaba.PlayState.WON
