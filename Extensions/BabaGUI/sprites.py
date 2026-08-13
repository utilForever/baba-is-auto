import pygame
import pyBaba


def _locked_text_image(direction):
    image = pygame.Surface((24, 24), pygame.SRCALPHA)
    if not pygame.font.get_init():
        pygame.font.init()
    font = pygame.font.Font(None, 8)

    for label, y in (("LOCKED", 3), (direction, 12)):
        line = font.render(label, False, (241, 120, 242))
        image.blit(line, ((24 - line.get_width()) // 2, y))

    return image


class MapSprite(pygame.sprite.Sprite):
    def __init__(self, image, x, y, is_icon, rotation=0):
        if is_icon:
            self.image = pygame.image.load("./sprites/icon/{}.gif".format(image))
        elif image.startswith("LOCKED_"):
            self.image = _locked_text_image(image.split("_", 1)[1])
        else:
            self.image = pygame.image.load("./sprites/text/{}.gif".format(image))

        if rotation:
            self.image = pygame.transform.rotate(self.image, rotation)

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
            self.image = pygame.transform.scale(
                pygame.image.load("./sprites/won.png"), (self.size, self.size)
            )
            self.rect = self.image.get_rect()
            self.rect.center = (screen_size[0] // 2, screen_size[1] // 2)
        else:
            self.size = max(screen_size[0], screen_size[1]) // 2
            self.image = pygame.transform.scale(
                pygame.image.load("./sprites/lost.png"), (self.size, self.size)
            )
            self.rect = self.image.get_rect()
            self.rect.center = (screen_size[0] // 2, screen_size[1] // 2)
