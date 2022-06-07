import pygame
import pyBaba
import config
import sys
import sprites

icon_images = {pyBaba.ObjectType.ICON_BABA: 'BABA',
               pyBaba.ObjectType.ICON_FLAG: 'FLAG',
               pyBaba.ObjectType.ICON_WALL: 'WALL',
               pyBaba.ObjectType.ICON_ROCK: 'ROCK',
               pyBaba.ObjectType.ICON_TILE: 'TILE',
               pyBaba.ObjectType.ICON_WATER: 'WATER',
               pyBaba.ObjectType.ICON_GRASS: 'GRASS',
               pyBaba.ObjectType.ICON_LAVA: 'LAVA',
               pyBaba.ObjectType.ICON_SKULL: 'SKULL',
               pyBaba.ObjectType.ICON_FLOWER: 'FLOWER'}

text_images = {pyBaba.ObjectType.BABA: 'BABA',
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
               pyBaba.ObjectType.HOT: 'HOT',
               pyBaba.ObjectType.SKULL: 'SKULL',
               pyBaba.ObjectType.DEFEAT: 'DEFEAT'}

game = pyBaba.Game("../../Resources/Maps/off_limits_bug.txt")
screen_size = (game.GetMap().GetWidth() * config.BLOCK_SIZE,
               game.GetMap().GetHeight() * config.BLOCK_SIZE)
screen = pygame.display.set_mode(
    (screen_size[0], screen_size[1]), pygame.DOUBLEBUF)

map_sprite_group = pygame.sprite.Group()

result_image = sprites.ResultImage()
result_image_group = pygame.sprite.Group()
result_image_group.add(result_image)


def draw_obj(x_pos, y_pos):
    objects = game.GetMap().At(x_pos, y_pos)
    is_icon = False

    for obj_type in objects.GetTypes():
        if pyBaba.IsTextType(obj_type):
            obj_image = text_images[obj_type]
        else:
            if obj_type == pyBaba.ObjectType.ICON_EMPTY:
                continue
            obj_image = icon_images[obj_type]
            is_icon = True
        map_sprite = sprites.MapSprite(obj_image, x_pos * config.BLOCK_SIZE, y_pos * config.BLOCK_SIZE, is_icon)
        map_sprite_group.add(map_sprite)


def draw():
    map_sprite_group.empty()

    for y_pos in range(game.GetMap().GetHeight()):
        for x_pos in range(game.GetMap().GetWidth()):
            draw_obj(x_pos, y_pos)

    map_sprite_group.draw(screen)


if __name__ == '__main__':
    pygame.init()
    pygame.font.init()

    action_dic = {"Direction.UP": pyBaba.Direction.UP, "Direction.DOWN": pyBaba.Direction.DOWN,
                  "Direction.LEFT": pyBaba.Direction.LEFT, "Direction.RIGHT": pyBaba.Direction.RIGHT, "Direction.NONE": pyBaba.Direction.NONE}
    action_file = open("./action.txt", 'r')
    actions = action_file.read().splitlines()
    action_file.close()

    clock = pygame.time.Clock()

    pygame.time.set_timer(pygame.USEREVENT, 200)

    game_over = False
    time_step = 0

    while True:
        if game_over:
            for event in pygame.event.get():
                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_ESCAPE:
                        pygame.quit()
                        sys.exit()
            if game.GetPlayState() == pyBaba.PlayState.WON:
                result_image_group.update(pyBaba.PlayState.WON, screen_size)
                result_image_group.draw(screen)
            else:
                result_image_group.update(pyBaba.PlayState.LOST, screen_size)
                result_image_group.draw(screen)
            pygame.display.flip()
            continue

        for event in pygame.event.get():
            if event.type == pygame.USEREVENT:
                if time_step < len(actions):
                    game.MovePlayer(action_dic[actions[time_step]])
                    time_step += 1
                else:
                    pass
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    pygame.quit()
                    sys.exit()

        if game.GetPlayState() == pyBaba.PlayState.WON or game.GetPlayState() == pyBaba.PlayState.LOST:
            game_over = True

        screen.fill(config.COLOR_BACKGROUND)
        draw()
        pygame.display.flip()

        clock.tick(config.FPS)
