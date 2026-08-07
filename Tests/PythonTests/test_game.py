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
