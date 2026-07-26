import gym
from gym import spaces
from gym.utils import seeding
from gym.envs.registration import register
import numpy as np

import pyBaba
import rendering


class BabaEnv(gym.Env):
    metadata = {'render.modes': ['human', 'rgb_array']}

    def __init__(self, enable_render=True):
        super(BabaEnv, self).__init__()

        self.path = '../../../Resources/Maps/volcano.txt'
        self.game = pyBaba.Game(self.path)
        self.renderer = rendering.Renderer(self.game)

        self.actions = [
            pyBaba.Direction.UP,
            pyBaba.Direction.DOWN,
            pyBaba.Direction.LEFT,
            pyBaba.Direction.RIGHT
        ]
        self.action_space = spaces.Discrete(len(self.actions))
        self.observation_space = spaces.Box(
            low=0, high=1,
            shape=(pyBaba.Preprocess.TENSOR_DIM,
                   self.game.GetMap().GetHeight(),
                   self.game.GetMap().GetWidth()),
            dtype=np.float32)
        self.action_size = self.action_space.n

        self.seed()
        self.reset()

    def seed(self, seed=None):
        self.np_random, seed = seeding.np_random(seed)

        return [seed]

    def reset(self):
        self.game.Reset()
        self.done = False

        return self.get_obs()

    def step(self, action):
        self.game.MovePlayer(self.actions[action])

        result = self.game.GetPlayState()

        if result == pyBaba.PlayState.LOST:
            self.done = True
            reward = -100
        elif result == pyBaba.PlayState.WON:
            self.done = True
            reward = 200
        else:
            reward = -0.5

        return self.get_obs(), reward, self.done, {}

    def render(self, mode='human', close=False):
        if close:
            self.renderer.quit_game()

        return self.renderer.render(self.game.GetMap(), mode)

    def get_obs(self):
        return np.array(
            pyBaba.Preprocess.StateToTensor(self.game),
            dtype=np.float32).reshape(-1, self.game.GetMap().GetHeight(), self.game.GetMap().GetWidth())


register(
    id='baba-volcano-v0',
    entry_point='environment:BabaEnv',
    max_episode_steps=200,
    nondeterministic=True
)
