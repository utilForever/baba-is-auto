import pyBaba
import config
from images import GIFImage


class SpriteLoader:
    def __init__(self):
        self.icon_images = {pyBaba.ObjectType.ICON_BABA: 'BABA',
                            pyBaba.ObjectType.ICON_FLAG: 'FLAG',
                            pyBaba.ObjectType.ICON_WALL: 'WALL',
                            pyBaba.ObjectType.ICON_ROCK: 'ROCK',
                            pyBaba.ObjectType.ICON_TILE: 'TILE'}

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
                            pyBaba.ObjectType.PUSH: 'PUSH'}

        for i in self.text_images:
            self.text_images[i] = GIFImage(
                './sprites/text/{}.gif'.format(self.text_images[i]))
            self.text_images[i].scale(1.0)
