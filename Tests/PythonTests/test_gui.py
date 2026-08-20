"""Behavior checks for GUI-owned direction decisions."""

import importlib.util
from pathlib import Path
import sys

import pyBaba

from Extensions.BabaGUI.orientation import oriented_instances
from Extensions.BabaGUI.orientation import rotation_for_direction


def test_keke_rotation_matches_facing_direction():
    assert rotation_for_direction(pyBaba.Direction.RIGHT) == 0
    assert rotation_for_direction(pyBaba.Direction.UP) == 90
    assert rotation_for_direction(pyBaba.Direction.LEFT) == 180
    assert rotation_for_direction(pyBaba.Direction.DOWN) == -90


def test_directionless_keke_uses_unrotated_sprite():
    assert rotation_for_direction(pyBaba.Direction.NONE) == 0


def test_render_input_preserves_each_keke_direction():
    game_map = pyBaba.Map(1, 1)
    game_map.AddObject(0, 0, pyBaba.ObjectType.ICON_KEKE, pyBaba.Direction.UP)
    game_map.AddObject(0, 0, pyBaba.ObjectType.ICON_KEKE, pyBaba.Direction.DOWN)

    rendered = list(oriented_instances(game_map.At(0, 0)))
    assert [(instance.type, angle) for instance, angle in rendered] == [
        (pyBaba.ObjectType.ICON_KEKE, 90),
        (pyBaba.ObjectType.ICON_KEKE, -90),
    ]


def test_render_input_preserves_directional_lock_markers():
    game_map = pyBaba.Map(1, 1)

    for marker in (
        pyBaba.ObjectType.LOCKED_UP,
        pyBaba.ObjectType.LOCKED_DOWN,
        pyBaba.ObjectType.LOCKED_LEFT,
        pyBaba.ObjectType.LOCKED_RIGHT,
    ):
        game_map.AddObject(0, 0, marker)

    rendered = list(oriented_instances(game_map.At(0, 0)))
    assert [(instance.type, angle) for instance, angle in rendered] == [
        (pyBaba.ObjectType.LOCKED_UP, 0),
        (pyBaba.ObjectType.LOCKED_DOWN, 0),
        (pyBaba.ObjectType.LOCKED_LEFT, 0),
        (pyBaba.ObjectType.LOCKED_RIGHT, 0),
    ]


def _gif_palette_starts(data):
    assert data[:6] in (b"GIF87a", b"GIF89a")
    position = 13
    packed = data[10]

    if packed & 0x80:
        yield position
        position += 3 * (1 << ((packed & 7) + 1))

    while data[position] != 0x3B:
        marker = data[position]
        position += 1

        if marker == 0x21:
            position += 1
        elif marker == 0x2C:
            packed = data[position + 8]
            position += 9
            if packed & 0x80:
                yield position
                position += 3 * (1 << ((packed & 7) + 1))
            position += 1
        else:
            raise AssertionError(f"unexpected GIF marker: {marker:#x}")

        while data[position]:
            position += data[position] + 1
        position += 1


def test_sprite_palettes_keep_colored_pixels_opaque():
    root = Path(__file__).parents[2]
    sprites = {
        "Extensions/BabaGUI/sprites/text/ALGAE.gif": (((84, 150, 64),), 1),
        "Extensions/BabaGUI/sprites/text/KEKE.gif": (((196, 90, 117),), 1),
        "Extensions/BabaGUI/sprites/text/LOVE.gif": (((241, 120, 242),), 1),
        "Extensions/BabaGUI/sprites/text/MOVE.gif": (((119, 171, 63),), 1),
        "Extensions/BabaGUI/sprites/text/BRICK.gif": (((144, 103, 62),), 1),
        "Extensions/BabaGUI/sprites/icon/ALGAE.gif": (((84, 150, 64),), 1),
        "Extensions/BabaGUI/sprites/icon/KEKE.gif": (((196, 90, 117),), 2),
        "Extensions/BabaGUI/sprites/icon/LOVE.gif": (((241, 120, 242),), 1),
        "Extensions/BabaGUI/sprites/text/PILLAR.gif": (((81, 111, 148),), 1),
        "Extensions/BabaGUI/sprites/icon/PILLAR.gif": (
            ((81, 111, 148), (36, 36, 36)),
            2,
        ),
        "Extensions/BabaGUI/sprites/icon/BRICK.gif": (((42, 47, 39),), 1),
        "Extensions/BabaGUI/sprites/text/KEY.gif": (((224, 240, 157),), 1),
        "Extensions/BabaGUI/sprites/text/DOOR.gif": (((196, 90, 117),), 1),
        "Extensions/BabaGUI/sprites/text/OPEN.gif": (((224, 240, 157),), 1),
        "Extensions/BabaGUI/sprites/text/SHUT.gif": (((196, 90, 117),), 1),
        "Extensions/BabaGUI/sprites/icon/KEY.gif": (((224, 240, 157),), 1),
        "Extensions/BabaGUI/sprites/icon/DOOR.gif": (
            ((196, 90, 117), (36, 36, 36)),
            2,
        ),
    }

    for relative, (colors, transparent_index) in sprites.items():
        data = (root / relative).read_bytes()
        assert int.from_bytes(data[6:8], "little") == 24
        assert int.from_bytes(data[8:10], "little") == 24

        palettes = list(_gif_palette_starts(data))
        assert len(palettes) == 3

        controls = [
            index
            for index in range(len(data))
            if data.startswith(b"\x21\xf9\x04", index)
        ]
        assert len(controls) == 3
        assert all(data[index + 3] & 1 for index in controls)
        assert all(data[index + 6] == transparent_index for index in controls)

        for start in palettes:
            for palette_index, color in enumerate(colors):
                offset = start + palette_index * 3
                assert tuple(data[offset : offset + 3]) == color

            for palette_index in range(len(colors), transparent_index + 1):
                offset = start + palette_index * 3
                assert max(data[offset : offset + 3]) <= 8
