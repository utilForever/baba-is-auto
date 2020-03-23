import pygame
import pyBaba
from images import GIFImage


class SpriteLoader:
    def __init__(self):
        self.icon_images = {pyBaba.ObjectType.ICON_BABA: 'BABA',
                            pyBaba.ObjectType.ICON_FLAG: 'FLAG',
                            pyBaba.ObjectType.ICON_WALL: 'WALL',
                            pyBaba.ObjectType.ICON_ROCK: 'ROCK',
                            pyBaba.ObjectType.ICON_TILE: 'TILE',
                            pyBaba.ObjectType.ICON_WATER: 'WATER',
                            pyBaba.ObjectType.ICON_GRASS: 'GRASS',
                            pyBaba.ObjectType.ICON_LAVA: 'LAVA'}

        for i in self.icon_images:
            self.icon_images[i] = GIFImage(
                './sprites/icon/{}.gif'.format(self.icon_images[i]))
            self.icon_images[i].scale(1.0)

        self.text_images = {pyBaba.ObjectType.BABA: 'BABA',
                            pyBaba.ObjectType.IS: 'IS',
                            pyBaba.ObjectType.YOU: 'YOU',
                            pyBaba.ObjectType.FLAG: 'FLAG',
                            pyBaba.ObjectType.WIN: 'WIN',
                            pyBaba.ObjectType.WALL: 'WALL',
                            pyBaba.ObjectType.STOP: 'STOP',
                            pyBaba.ObjectType.ROCK: 'ROCK',
                            pyBaba.ObjectType.PUSH: 'PUSH',
                            pyBaba.ObjectType.WATER: 'WATER',
                            pyBaba.ObjectType.SINK: 'SINK',
                            pyBaba.ObjectType.LAVA: 'LAVA',
                            pyBaba.ObjectType.MELT: 'MELT',
                            pyBaba.ObjectType.HOT: 'HOT'}

        for i in self.text_images:
            self.text_images[i] = GIFImage(
                './sprites/text/{}.gif'.format(self.text_images[i]))
            self.text_images[i].scale(1.0)


class ResultImage(pygame.sprite.Sprite):
    def __init__(self):
        pygame.sprite.Sprite.__init__(self)

    def update(self, status, screen_size):
        if status == pyBaba.PlayState.WON:
            self.size = max(screen_size[0], screen_size[1]) // 2
            self.image = pygame.transform.scale(pygame.image.load(
                './sprites/won.png'), (self.size, self.size))
            self.rect = self.image.get_rect()
            self.rect.center = (screen_size[0] // 2, screen_size[1] // 2)
        else:
            self.size = max(screen_size[0], screen_size[1]) // 2
            self.image = pygame.transform.scale(pygame.image.load(
                './sprites/lost.png'), (self.size, self.size))
            self.rect = self.image.get_rect()
            self.rect.center = (screen_size[0] // 2, screen_size[1] // 2)
