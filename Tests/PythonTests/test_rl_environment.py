"""
Copyright (c) 2020-2026 Chris Ohk

I am making my contributions/submissions to this project solely in our
personal capacity and am not conveying any rights to any intellectual
property of any third parties.
"""

import importlib.util
import sys
from pathlib import Path
from types import SimpleNamespace

import gym
import pyBaba
import pytest


@pytest.mark.parametrize(
    "name",
    [
        "baba-babaisyou-v0",
        "baba-outofreach-v0",
        "baba-volcano-v0",
    ],
)
def test_rl_environment_uses_gym_spaces(monkeypatch, name):
    directory = Path(__file__).parents[2] / "Extensions" / "BabaRL" / name
    monkeypatch.chdir(directory)
    monkeypatch.setitem(
        sys.modules, "rendering", SimpleNamespace(Renderer=lambda game: None)
    )

    spec = importlib.util.spec_from_file_location(
        "environment", directory / "environment.py"
    )
    module = importlib.util.module_from_spec(spec)
    monkeypatch.setitem(sys.modules, "environment", module)
    spec.loader.exec_module(module)
    env = gym.make(name)

    assert isinstance(env.action_space, gym.spaces.Discrete)
    assert env.action_space.n == 4
    assert isinstance(env.observation_space, gym.spaces.Box)
    assert env.unwrapped.actions == [
        pyBaba.Direction.UP,
        pyBaba.Direction.DOWN,
        pyBaba.Direction.LEFT,
        pyBaba.Direction.RIGHT,
    ]
    assert env.observation_space.contains(env.reset())
    assert env.observation_space.contains(env.step(env.action_space.sample())[0])
