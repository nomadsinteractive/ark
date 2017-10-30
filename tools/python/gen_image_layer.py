#!/usr/bin/env python

import getopt
import sys

from PIL import Image
from PIL import ImageDraw

import acg

gPx = 0.5
gPy = 0.0
gSx = 1.0
gSy = 1.0


class Block(object):
    def __init__(self, typeid, width, height):
        self._type_id = typeid
        self._left = width
        self._top = height
        self._right = 0
        self._bottom = 0

    @property
    def type(self):
        return self._type_id

    @type.setter
    def type(self, value):
        self._type_id = value

    @property
    def left(self):
        return self._left

    @property
    def top(self):
        return self._top

    @property
    def right(self):
        return self._right

    @property
    def bottom(self):
        return self._bottom

    def flood(self, x, y):
        if self._left > x:
            self._left = x
        if self._right < x:
            self._right = x
        if self._top > y:
            self._top = y
        if self._bottom < y:
            self._bottom = y

    def item(self):
        return '<item type="%d" rect="%d, %d, %d, %d"/>' % (self._type_id, self._left, self._top, self._right + 1, self._bottom + 1)

    def render_object(self):
        width = self._right - self._left + 1
        height = self._bottom - self._top + 1
        return '<render-object type="%d" size="%d, %d" transform="pivot: (%.1f, %.1f)"/>' % (self._type_id, width * gSx, height * gSy, width * gPx, height * gPy)


class BlockWrapper(object):
    def __init__(self, block):
        self._block = block
        
    @property
    def type(self):
        return self._block.type
    
    @property
    def left(self):
        return self._block.left
    
    @property
    def top(self):
        return self._block.top
    
    @property
    def right(self):
        return self._block.right
    
    @property
    def bottom(self):
        return self._block.bottom

    def flood(self, x, y):
        self._block.flood(x, y)
    
    def merge(self, otherblock):
        self._block.type = min(self._block.type, otherblock.type)
        self._block.flood(otherblock.left, otherblock.top)
        self._block.flood(otherblock.right, otherblock.bottom)
        
    def item(self):
        return self._block.item()

    def render_object(self):
        return self._block.render_object()

def find_neighbour(x, scanline, row):
    neighbours = scanline[x - 1 : x + 2]
    for i in neighbours:
        if i:
            return i
    for i, j in enumerate(row[x + 1:]):
        alpha = j[3]
        if alpha == 0:
            break
        neighbour = scanline[x + i] or scanline[x + i + 1]
        if neighbour:
            return neighbour
    return None


def main():
    global gPx, gPy, gSx, gSy
    opts, args = getopt.getopt(sys.argv[1:], 'o:p:s:v')
    params = dict((i.lstrip('-'), j) for i, j in opts)
    if len(args) == 0:
        print('Usage: %s [-o outputfile] [-p px, py] [-s sx, sy] [-v] imagefile ...' % sys.argv[0])
        sys.exit(-1)

    output_file = params['o'] if 'o' in params else None
    pivot = params['p'] if 'p' in params else None
    scale = params['s'] if 's' in params else None
    show = 'v' in params
    if pivot:
        ps = pivot.split(',')
        gPx = float(ps[0])
        gPy = float(ps[1])
    if scale:
        ps = scale.split(',')
        gSx = float(ps[0])
        gSy = float(ps[1]) if len(ps) > 1 else gSx
    image = Image.open(args[0])
    pixels = list(image.getdata())
    pixelslen = len(pixels)
    imagewidth = image.size[0]
    imageheight = image.size[1]
    scanline = [None] * imagewidth
    blocks = []
    seq = 0
    for y, i in enumerate(range(0, pixelslen, imagewidth)):
        row = pixels[i: i + imagewidth]
        scan = []
        block = None
        for x, k in enumerate(row):
            alpha = k[3]
            if alpha > 0:
                n2 = find_neighbour(x, scanline, row)
                neighbour = block or n2
                if neighbour and n2 and neighbour is not n2:
                    neighbour.merge(n2)
                    if n2 in blocks:
                        blocks.remove(n2)
                if not neighbour:
                    block = BlockWrapper(Block(seq, imagewidth, imageheight))
                    seq += 1
                    blocks.append(block)
                else:
                    block = neighbour
                block.flood(x, y)
            else:
                block = None
            scan.append(block)
        scanline = scan
    canvas = ImageDraw.Draw(image)
    color = (255, 0, 0, 100)
    for i in blocks:
        canvas.text((i.left, i.top), str(i.type), fill=color)
        canvas.rectangle((i.left, i.top, i.right, i.bottom), outline=color)

    content = '    ' + '\n    '.join(i.item() for i in blocks) + '\n\n    ' + '\n    '.join(i.render_object() for i in blocks)
    acg.write_to_file(output_file, content)
    if show:
        image.show()

if __name__ == '__main__':
    main()
