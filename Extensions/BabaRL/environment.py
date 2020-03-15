import gym
from gym.utils import seeding
from gym.envs.registration import register
import numpy as np

import pyBaba


class BabaEnv(gym.Env):
    metadata = {'render.modes': ['human']}

    def __init__(self, path):
        super(BabaEnv, self).__init__()

        self.path = path

        self.action_space = [
            pyBaba.Direction.UP,
            pyBaba.Direction.DOWN,
            pyBaba.Direction.LEFT,
            pyBaba.Direction.RIGHT,
            pyBaba.Direction.NONE
        ]

        self.action_size = len(self.action_space)
        self.seed()

    def seed(self, seed=None):
        self.np_random, seed = seeding.np_random(seed)

        return [seed]

    def reset(self):
        self.game = pyBaba.Game(self.path)
        self.done = False

        return self._get_obs()

    def step(self, action):
        self.game.MovePlayer(action)

        result = self.game.GetPlayState()

        if result == pyBaba.PlayState.LOST:
            self.done = True
            reward = -100
        elif result == pyBaba.PlayState.WON:
            self.done = True
            reward = 200
        else:
            reward = -0.1

        return self._get_obs(), reward, self.done, {}

    def render(self, mode='human', close=False):
        pass

    def _get_obs(self):
        return np.array(
            pyBaba.Preprocess.StateToTensor(self.game),
            dtype=np.float32).reshape(-1, self.game.GetMap().GetHeight(), self.game.GetMap().GetWidth())


class BabaEnvBabaIsYou(BabaEnv):
    def __init__(self):
        super(BabaEnvBabaIsYou, self).__init__(
            "../../Resources/Maps/BabaIsYou.txt")


register(
    id='baba-babaisyou-v0',
    entry_point='environment:BabaEnvBabaIsYou',
    max_episode_steps=1000,
    nondeterministic=True
)
