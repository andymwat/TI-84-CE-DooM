#!/usr/bin/python
import math
import os
from PIL import Image

# Trig tables

trig = open("trig.h", "w")
trig.write("int24_t short_sine_table[256] = {")
for x in range(256):
        deg = x * 360.0 / 1024.0
        rad = math.radians(deg)
        sine = int(65535*math.sin(rad))
        if sine == 0:
                sine = 1
        if x != 0:
                trig.write(",")
        trig.write(str(sine) + "\n")
trig.write("};\n")
trig.write("int24_t secants[128] = {")
for x in range(128):
        deg = ((x - 64) * 360.0) / 1024.0
        rad = math.radians(deg)
        if x != 0:
                trig.write(",")
        trig.write(str(int(9900 / math.cos(rad))))
trig.write("};\n")

trig.write("int24_t short_tan_table[128] = {")
for x in range(128):
        deg = x * 360.0 / 1024.0
        rad = math.radians(deg)
        if x != 0:
                trig.write(",")
        trig.write(str(int(65535*math.tan(rad))))
trig.write("};\n")

trig.write("int24_t short_cot_table[128] = {0x7FFFFF")
for x in range(1,128):
        deg = x * 360.0 / 1024.0
        rad = math.radians(deg)
        trig.write("," + str(int(65535/(32*math.tan(rad)))))
trig.write("};\n")

trig.write("uint8_t arctan_table[257] = {")
for x in range(257):
        rad = math.atan(float(x) / 256.0)
        deg = math.degrees(rad)
        if x != 0:
                trig.write(",")
        trig.write(str(int(deg * 1023 / 360)))
trig.write("};\n")


# Square root table

sqr = open("sqrt.h", "w")
sqr.write("int24_t square_root_table[768] = {")
for x in range(256,1024):
        s = math.sqrt(x << 14)
        if x != 256:
                sqr.write(",")
        sqr.write(str(int(s)))
sqr.write("};")

# Palette

paldata = [0, 0x333333, 0x666666, 0x999999, 0xCCCCCC, 0xFFFFFF, 0xFF0000, 0xFF5500, 0x33FFFF, 0xFFAA00, 0xFFFF00, 0x990000, 0x00FF00, 0x0000FF, 0x009900, 0x000099]
pal = open("palette.h" ,"w")
pal.write("uint16_t palette[16] = {")
for x in range(16):
        rgb = paldata[x]
        r = rgb >> 16
        g = (rgb >> 8) & 255
        b = rgb & 255
        r = r >> 3
        g = g >> 3
        b = b >> 3
        if x != 0:
                pal.write(",")
        pal.write(hex((r << 10) + (g << 5) + b))
pal.write("};")

# Wall textures

def dist(palcolor, rgb):
        palrgb = paldata[palcolor]
        r = palrgb >> 16
        g = (palrgb >> 8) & 255
        b = palrgb & 255
        return abs(r - rgb[0]) + abs(g - rgb[1]) + abs(b - rgb[2])

def process_bitmap(img, outfile, width, height):
        for x in range(width):
                for y in range(height):
                        bestcolor = 0
                        bestdist = 99999999999
                        for testcolor in range(16):
                                testdist = dist(testcolor, img.getpixel((x, y)))
                                if testdist < bestdist:
                                        bestdist = testdist
                                        bestcolor = testcolor

                        outfile.write(" db " + str(17 * bestcolor) + "\n")

walls = open("walls.inc", "w")
NUM_WALLS = 6
for wall in range(NUM_WALLS):
        i = Image.open("wall" + str(wall) + ".png")
        process_bitmap(i, walls, 64, 64)

# Character images

inchars = open("..\\calcuzap\\chars.i", "r")
first = True
chars = open("chars.inc", "w")
for line in inchars:
        for x in range(len(line)):
                if line[x] == "$":
                        val = int(line[x+1:x+3], 16)
                        chars.write(" db " + str(val) + "\n")
                        first = False