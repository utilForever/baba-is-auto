import torch
from torch import nn, optim
import torch.nn.functional as F

from collections import namedtuple
import random
import numpy as np

import gym
import environment
import pyBaba

from tensorboardX import SummaryWriter

device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
env = gym.make('baba-outofreach-v0')

Transition = namedtuple(
    'Transition', ('state', 'action', 'next_state', 'reward'))


class ReplayMemory:
    def __init__(self, capacity):
        self.capacity = capacity
        self.memory = []
        self.position = 0

    def push(self, *args):
        if len(self.memory) < self.capacity:
            self.memory.append(None)

        self.memory[self.position] = Transition(*args)
        self.position = (self.position + 1) % self.capacity

    def sample(self, batch_size):
        return random.sample(self.memory, batch_size)

    def __len__(self):
        return len(self.memory)


class Network(nn.Module):
    def __init__(self):
        super(Network, self).__init__()

        self.conv1 = nn.Conv2d(pyBaba.Preprocess.TENSOR_DIM,
                               64, 3, padding=1, bias=False)
        self.bn1 = nn.BatchNorm2d(64)
        self.conv2 = nn.Conv2d(64, 64, 3, padding=1, bias=False)
        self.bn2 = nn.BatchNorm2d(64)
        self.conv3 = nn.Conv2d(64, 64, 3, padding=1, bias=False)
        self.bn3 = nn.BatchNorm2d(64)
        self.conv4 = nn.Conv2d(64, 1, 1, padding=0, bias=False)
        self.bn4 = nn.BatchNorm2d(1)

        self.fc = nn.Linear(352, 4)

    def forward(self, x):
        x = F.relu(self.bn1(self.conv1(x)))
        x = F.relu(self.bn2(self.conv2(x)))
        x = F.relu(self.bn3(self.conv3(x)))
        x = F.relu(self.bn4(self.conv4(x)))

        x = x.view(x.data.size(0), -1)
        return self.fc(x)


BATCH_SIZE = 128
GAMMA = 0.99
EPSILON = 0.9
EPSILON_DECAY = 0.99
MIN_EPSILON = 0.01
TARGET_UPDATE = 10

net = Network().to(device)
target_net = Network().to(device)

target_net.load_state_dict(net.state_dict())
target_net.eval()

opt = optim.Adam(net.parameters())
memory = ReplayMemory(10000)


def get_action(state):
    if random.random() > EPSILON:
        with torch.no_grad():
            return env.action_space[net(state).max(1)[1].view(1)]
    else:
        return random.choice(env.action_space)


def train():
    if len(memory) < BATCH_SIZE:
        return

    transitions = memory.sample(BATCH_SIZE)
    batch = Transition(*zip(*transitions))

    actions = tuple((map(lambda a: torch.tensor([[int(a) - 1]]), batch.action)))
    rewards = tuple(
        (map(lambda r: torch.tensor([r], dtype=torch.float32), batch.reward)))

    non_final_mask = torch.tensor(tuple(
        map(lambda s: s is not None, batch.next_state)), device=device, dtype=torch.bool)
    non_final_next_states = torch.cat(
        [s for s in batch.next_state if s is not None])

    state_batch = torch.cat(batch.state).to(device)
    action_batch = torch.cat(actions).to(device)
    reward_batch = torch.cat(rewards).to(device)

    q_values = net(state_batch).gather(1, action_batch)

    next_q_values = torch.zeros(BATCH_SIZE, device=device)
    next_q_values[non_final_mask] = target_net(
        non_final_next_states).max(1)[0].detach()

    expected_state_action_values = (next_q_values * GAMMA) + reward_batch

    loss = F.smooth_l1_loss(
        q_values, expected_state_action_values.unsqueeze(1))

    opt.zero_grad()
    loss.backward()

    for param in net.parameters():
        param.grad.data.clamp_(-1, 1)

    opt.step()


if __name__ == '__main__':
    writer = SummaryWriter()

    global_step = 0

    scores = []
    for e in range(10000):
        score = 0

        state = env.reset().reshape(1, -1, 16, 22)
        state = torch.tensor(state).to(device)

        step = 0
        while step < 200:
            global_step += 1

            action = get_action(state)

            env.render()

            next_state, reward, done, _ = env.step(action)
            next_state = next_state.reshape(1, -1, 16, 22)
            next_state = torch.tensor(next_state).to(device)

            memory.push(state, action, next_state, reward)
            score += reward
            state = next_state

            step += 1

            train()

            if env.done:
                break

        writer.add_scalar('Reward', score, e)
        writer.add_scalar('Step', step, e)
        writer.add_scalar('Epsilon', EPSILON, e)

        scores.append(score)

        print(
            f'Episode {e}: score: {score:.3f} time_step: {global_step} step: {step} epsilon: {EPSILON}')

        if np.mean(scores[-min(50, len(scores)):]) > 180:
            print('Solved!')
            torch.save(net.state_dict(), 'dqn_agent.bin')
            break

        if e % TARGET_UPDATE == 0:
            target_net.load_state_dict(net.state_dict())

            EPSILON *= EPSILON_DECAY
            EPSILON = max(EPSILON, MIN_EPSILON)
