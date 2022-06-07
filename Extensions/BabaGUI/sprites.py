import pygame
import pyBaba


class MapSprite(pygame.sprite.Sprite):
    def __init__(self, image, x, y, is_icon):
        if is_icon:
            self.image = pygame.image.load('./sprites/icon/{}.gif'.format(image))
        else:
            self.image = pygame.image.load('./sprites/text/{}.gif'.format(image))
        self.rect = self.image.get_rect()
        self.rect.x = x
        self.rect.y = y

        pygame.sprite.Sprite.__init__(self)


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
