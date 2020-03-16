import pygame
import pyBaba
import sprites

BLOCK_SIZE = 48
COLOR_BACKGROUND = pygame.Color(0, 0, 0)


class Renderer():
    def __init__(self, game):
        pygame.init()
        pygame.display.set_caption('OpenAI Gym - baba-babaisyou-v0')

        self.game = game
        self.screen_size = (game.GetMap().GetWidth() * BLOCK_SIZE,
                            game.GetMap().GetHeight() * BLOCK_SIZE)
        self.screen = pygame.display.set_mode(
            (self.screen_size[0], self.screen_size[1]), pygame.DOUBLEBUF)

        self.sprite_loader = sprites.SpriteLoader()
        self.draw(game.GetMap())

    def draw_obj(self, map, x_pos, y_pos):
        objects = map.At(y_pos, x_pos)

        for obj_type in objects.GetTypes():
            if pyBaba.IsTextType(obj_type):
                obj_image = self.sprite_loader.text_images[obj_type]
            else:
                if obj_type == pyBaba.ObjectType.ICON_EMPTY:
                    continue
                obj_image = self.sprite_loader.icon_images[obj_type]
            obj_rect = obj_image.get_rect()
            obj_rect.topleft = (x_pos * BLOCK_SIZE, y_pos * BLOCK_SIZE)
            self.screen.blit(obj_image, obj_rect)

    def draw(self, map):
        for y_pos in range(map.GetHeight()):
            for x_pos in range(map.GetWidth()):
                self.draw_obj(map, x_pos, y_pos)

    def render(self, map, mode='human'):
        if mode == 'human':
            self.draw(map)
            pygame.display.flip()
