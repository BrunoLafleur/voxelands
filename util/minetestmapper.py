#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This program is free software. It comes without any warranty, to
# the extent permitted by applicable law. You can redistribute it
# and/or modify it under the terms of the Do What The Fuck You Want
# To Public License, Version 2, as published by Sam Hocevar. See
# COPYING for more details.

# Made by Jogge, modified by celeron55
# 2011-05-29: j0gge: initial release
# 2011-05-30: celeron55: simultaneous support for sectors/sectors2, removed
# 2011-06-02: j0gge: command line parameters, coordinates, players, ...
# 2011-06-04: celeron55: added #!/usr/bin/python2 and converted \r\n to \n
#                        to make it easily executable on Linux
# 2011-07-30: WF: Support for content types extension, refactoring
# 2011-07-30: erlehmann: PEP 8 compliance.

# Requires Python Imaging Library: http://www.pythonware.com/products/pil/

# Some speed-up: ...lol, actually it slows it down.
#import psyco ; psyco.full()
#from psyco.classes import *

import zlib
import os
import string
import time
import getopt
import sys
import array
import cStringIO
from PIL import Image, ImageDraw, ImageFont, ImageColor

TRANSLATION_TABLE = {
    1: 0x800,  # CONTENT_GRASS
    4: 0x801,  # CONTENT_TREE
    5: 0x802,  # CONTENT_LEAVES
    6: 0x803,  # CONTENT_GRASS_FOOTSTEPS
    7: 0x804,  # CONTENT_MESE
    8: 0x805,  # CONTENT_MUD
    10: 0x806,  # CONTENT_CLOUD
    11: 0x807,  # CONTENT_COALSTONE
    12: 0x808,  # CONTENT_WOOD
    13: 0x809,  # CONTENT_SAND
    18: 0x80a,  # CONTENT_COBBLE
    19: 0x80b,  # CONTENT_STEEL
    20: 0x80c,  # CONTENT_GLASS
    22: 0x80d,  # CONTENT_MOSSYCOBBLE
    23: 0x80e,  # CONTENT_GRAVEL
    24: 0x80f,  # CONTENT_SANDSTONE
    25: 0x810,  # CONTENT_CACTUS
    26: 0x811,  # CONTENT_BRICK
    27: 0x812,  # CONTENT_CLAY
    28: 0x813,  # CONTENT_PAPYRUS
    29: 0x814}  # CONTENT_BOOKSHELF


def hex_to_int(h):
    i = int(h, 16)
    if(i > 2047):
        i -= 4096
    return i


def hex4_to_int(h):
    i = int(h, 16)
    if(i > 32767):
        i -= 65536
    return i


def int_to_hex3(i):
    if(i < 0):
        return "%03X" % (i + 4096)
    else:
        return "%03X" % i


def int_to_hex4(i):
    if(i < 0):
        return "%04X" % (i + 65536)
    else:
        return "%04X" % i


def getBlockAsInteger(p):
    return p[2]*16777216 + p[1]*4096 + p[0]

def unsignedToSigned(i, max_positive):
    if i < max_positive:
        return i
    else:
        return i - 2*max_positive

def getIntegerAsBlock(i):
    x = unsignedToSigned(i % 4096, 2048)
    i = int((i - x) / 4096)
    y = unsignedToSigned(i % 4096, 2048)
    i = int((i - y) / 4096)
    z = unsignedToSigned(i % 4096, 2048)
    return x,y,z

def limit(i, l, h):
    if(i > h):
        i = h
    if(i < l):
        i = l
    return i


def usage():
    print("TODO: Help")
try:
    opts, args = getopt.getopt(sys.argv[1:], "hi:o:", ["help", "input=",
        "output=", "bgcolor=", "scalecolor=", "origincolor=",
        "playercolor=", "draworigin", "drawplayers", "drawscale",
        "drawunderground"])
except getopt.GetoptError as err:
    # print help information and exit:
    print(str(err))  # will print something like "option -a not recognized"
    usage()
    sys.exit(2)

path = "../world/"
output = "map.png"
border = 0
scalecolor = "black"
bgcolor = "white"
origincolor = "red"
playercolor = "red"
drawscale = False
drawplayers = False
draworigin = False
drawunderground = False

sector_xmin = -1500 / 16
sector_xmax = 1500 / 16
sector_zmin = -1500 / 16
sector_zmax = 1500 / 16

for o, a in opts:
    if o in ("-h", "--help"):
        usage()
        sys.exit()
    elif o in ("-i", "--input"):
        path = a
    elif o in ("-o", "--output"):
        output = a
    elif o == "--bgcolor":
        bgcolor = ImageColor.getrgb(a)
    elif o == "--scalecolor":
        scalecolor = ImageColor.getrgb(a)
    elif o == "--playercolor":
        playercolor = ImageColor.getrgb(a)
    elif o == "--origincolor":
        origincolor = ImageColor.getrgb(a)
    elif o == "--drawscale":
        drawscale = True
        border = 40
    elif o == "--drawplayers":
        drawplayers = True
    elif o == "--draworigin":
        draworigin = True
    elif o == "--drawunderground":
        drawunderground = True
    else:
        assert False, "unhandled option"

if path[-1:] != "/" and path[-1:] != "\\":
    path = path + "/"

# Load color information for the blocks.
colors = {}
try:
    f = file("colors.txt")
except IOError:
    f = file(os.path.join(os.path.dirname(__file__), "colors.txt"))
for line in f:
    values = string.split(line)
    colors[int(values[0], 16)] = (
        int(values[1]),
        int(values[2]),
        int(values[3]))
f.close()

xlist = []
zlist = []

# List all sectors to memory and calculate the width and heigth of the
# resulting picture.

conn = None
cur = None
if os.path.exists(path + "map.sqlite"):
    import sqlite3
    conn = sqlite3.connect(path + "map.sqlite")
    cur = conn.cursor()

    cur.execute("SELECT `pos` FROM `blocks`")
    while True:
        r = cur.fetchone()
        if not r:
            break

        x, y, z = getIntegerAsBlock(r[0])

        if x < sector_xmin or x > sector_xmax:
            continue
        if z < sector_zmin or z > sector_zmax:
            continue

        xlist.append(x)
        zlist.append(z)

if os.path.exists(path + "sectors2"):
    for filename in os.listdir(path + "sectors2"):
        for filename2 in os.listdir(path + "sectors2/" + filename):
            x = hex_to_int(filename)
            z = hex_to_int(filename2)
            if x < sector_xmin or x > sector_xmax:
                continue
            if z < sector_zmin or z > sector_zmax:
                continue
            xlist.append(x)
            zlist.append(z)

if os.path.exists(path + "sectors"):
    for filename in os.listdir(path + "sectors"):
        x = hex4_to_int(filename[:4])
        z = hex4_to_int(filename[-4:])
        if x < sector_xmin or x > sector_xmax:
            continue
        if z < sector_zmin or z > sector_zmax:
            continue
        xlist.append(x)
        zlist.append(z)

# Get rid of doubles
xlist, zlist = zip(*sorted(set(zip(xlist, zlist))))

minx = min(xlist)
minz = min(zlist)
maxx = max(xlist)
maxz = max(zlist)

w = (maxx - minx) * 16 + 16
h = (maxz - minz) * 16 + 16

print("w=" + str(w) + " h=" + str(h))

im = Image.new("RGB", (w + border, h + border), bgcolor)
draw = ImageDraw.Draw(im)
impix = im.load()

stuff = {}

starttime = time.time()

CONTENT_WATER = 2

def content_is_water(d):
    return d in [2, 9]

def content_is_air(d):
    return d in [126, 127, 254]

def read_content(mapdata, version, datapos):
    if version == 20:
        if mapdata[datapos] < 0x80:
            return mapdata[datapos]
        else:
            return (mapdata[datapos] << 4) | (mapdata[datapos + 0x2000] >> 4)
    elif 16 <= version < 20:
        return TRANSLATION_TABLE.get(mapdata[datapos], mapdata[datapos])
    else:
        raise Exception("Unsupported map format: " + str(version))


def read_mapdata(f, version, pixellist, water, day_night_differs):
    global stuff  # oh my :-)

    dec_o = zlib.decompressobj()
    try:
        mapdata = array.array("B", dec_o.decompress(f.read()))
    except:
        mapdata = []

    f.close()

    if(len(mapdata) < 4096):
        print("bad: " + xhex + "/" + zhex + "/" + yhex + " " + \
            str(len(mapdata)))
    else:
        chunkxpos = xpos * 16
        chunkypos = ypos * 16
        chunkzpos = zpos * 16
        content = 0
        datapos = 0
        for (x, z) in reversed(pixellist):
            for y in reversed(range(16)):
                datapos = x + y * 16 + z * 256
                content = read_content(mapdata, version, datapos)
                if content_is_air(content):
                    pass
                elif content_is_water(content):
                    water[(x, z)] += 1
                    # Add dummy stuff for drawing sea without seabed
                    stuff[(chunkxpos + x, chunkzpos + z)] = (
                        chunkypos + y, content, water[(x, z)], day_night_differs)
                elif content in colors:
                    # Memorize information on the type and height of
                    # the block and for drawing the picture.
                    stuff[(chunkxpos + x, chunkzpos + z)] = (
                        chunkypos + y, content, water[(x, z)], day_night_differs)
                    pixellist.remove((x, z))
                    break
                else:
                    print("strange block: %s/%s/%s x: %d y: %d z: %d \
block id: %x" % (xhex, zhex, yhex, x, y, z, content))

# Go through all sectors.
for n in range(len(xlist)):
    #if n > 500:
    #   break
    if n % 200 == 0:
        nowtime = time.time()
        dtime = nowtime - starttime
        try:
            n_per_second = 1.0 * n / dtime
        except ZeroDivisionError:
            n_per_second = 0
        if n_per_second != 0:
            seconds_per_n = 1.0 / n_per_second
            time_guess = seconds_per_n * len(xlist)
            remaining_s = time_guess - dtime
            remaining_minutes = int(remaining_s / 60)
            remaining_s -= remaining_minutes * 60
            print("Processing sector " + str(n) + " of " + str(len(xlist))
                    + " (" + str(round(100.0 * n / len(xlist), 1)) + "%)"
                    + " (ETA: " + str(remaining_minutes) + "m "
                    + str(int(remaining_s)) + "s)")

    xpos = xlist[n]
    zpos = zlist[n]

    xhex = int_to_hex3(xpos)
    zhex = int_to_hex3(zpos)
    xhex4 = int_to_hex4(xpos)
    zhex4 = int_to_hex4(zpos)

    sector1 = xhex4.lower() + zhex4.lower()
    sector2 = xhex.lower() + "/" + zhex.lower()

    ylist = []

    sectortype = ""

    if cur:
        psmin = getBlockAsInteger((xpos, -2048, zpos))
        psmax = getBlockAsInteger((xpos, 2047, zpos))
        cur.execute("SELECT `pos` FROM `blocks` WHERE `pos`>=? AND `pos`<=? AND (`pos` - ?) % 4096 = 0", (psmin, psmax, psmin))
        while True:
            r = cur.fetchone()
            if not r:
                break
            pos = getIntegerAsBlock(r[0])[1]
            ylist.append(pos)
            sectortype = "sqlite"
    try:
        for filename in os.listdir(path + "sectors/" + sector1):
            if(filename != "meta"):
                pos = int(filename, 16)
                if(pos > 32767):
                    pos -= 65536
                ylist.append(pos)
                sectortype = "old"
    except OSError:
        pass

    if sectortype == "":
        try:
            for filename in os.listdir(path + "sectors2/" + sector2):
                if(filename != "meta"):
                    pos = int(filename, 16)
                    if(pos > 32767):
                        pos -= 65536
                    ylist.append(pos)
                    sectortype = "new"
        except OSError:
            pass

    if sectortype == "":
        continue

    ylist.sort()

    # Make a list of pixels of the sector that are to be looked for.
    pixellist = []
    water = {}
    for x in range(16):
        for z in range(16):
            pixellist.append((x, z))
            water[(x, z)] = 0

    # Go through the Y axis from top to bottom.
    for ypos in reversed(ylist):

        yhex = int_to_hex4(ypos)

        if sectortype == "sqlite":
            ps = getBlockAsInteger((xpos, ypos, zpos))
            cur.execute("SELECT `data` FROM `blocks` WHERE `pos`==? LIMIT 1", (ps,))
            r = cur.fetchone()
            if not r:
                continue
            f = cStringIO.StringIO(r[0])
        else:
            if sectortype == "old":
                filename = path + "sectors/" + sector1 + "/" + yhex.lower()
            else:
                filename = path + "sectors2/" + sector2 + "/" + yhex.lower()
            f = file(filename, "rb")

        # Let's just memorize these even though it's not really necessary.
        version = ord(f.read(1))
        flags = f.read(1)

        # Checking day and night differs -flag
        day_night_differs = ((ord(flags) & 2) != 0)

        read_mapdata(f, version, pixellist, water, day_night_differs)

        # After finding all the pixels in the sector, we can move on to
        # the next sector without having to continue the Y axis.
        if(len(pixellist) == 0):
            break

print("Drawing image")
# Drawing the picture
starttime = time.time()
n = 0
for (x, z) in stuff.iterkeys():
    if n % 500000 == 0:
        nowtime = time.time()
        dtime = nowtime - starttime
        try:
            n_per_second = 1.0 * n / dtime
        except ZeroDivisionError:
            n_per_second = 0
        if n_per_second != 0:
            listlen = len(stuff)
            seconds_per_n = 1.0 / n_per_second
            time_guess = seconds_per_n * listlen
            remaining_s = time_guess - dtime
            remaining_minutes = int(remaining_s / 60)
            remaining_s -= remaining_minutes * 60
            print("Drawing pixel " + str(n) + " of " + str(listlen)
                    + " (" + str(round(100.0 * n / listlen, 1)) + "%)"
                    + " (ETA: " + str(remaining_minutes) + "m "
                    + str(int(remaining_s)) + "s)")
    n += 1

    (r, g, b) = colors[stuff[(x, z)][1]]

    dnd = stuff[(x, z)][3]  # day/night differs?
    if not dnd and not drawunderground:
        if stuff[(x, z)][2] > 0:  # water
            (r, g, b) = colors[CONTENT_WATER]
        else:
            continue

    # Comparing heights of a couple of adjacent blocks and changing
    # brightness accordingly.
    try:
        c = stuff[(x, z)][1]
        c1 = stuff[(x - 1, z)][1]
        c2 = stuff[(x, z + 1)][1]
        dnd1 = stuff[(x - 1, z)][3]
        dnd2 = stuff[(x, z + 1)][3]
        if not dnd:
            d = -69
        elif not content_is_water(c1) and not content_is_water(c2) and \
            not content_is_water(c):
            y = stuff[(x, z)][0]
            y1 = stuff[(x - 1, z)][0] if dnd1 else y
            y2 = stuff[(x, z + 1)][0] if dnd2 else y
            d = ((y - y1) + (y - y2)) * 12
        else:
            d = 0

        if(d > 36):
            d = 36

        r = limit(r + d, 0, 255)
        g = limit(g + d, 0, 255)
        b = limit(b + d, 0, 255)
    except:
        pass

    # Water
    if(stuff[(x, z)][2] > 0):
        r = int(r * .15 + colors[2][0] * .85)
        g = int(g * .15 + colors[2][1] * .85)
        b = int(b * .15 + colors[2][2] * .85)

    impix[x - minx * 16 + border, h - 1 - (z - minz * 16) + border] = (r, g, b)


if draworigin:
    draw.ellipse((minx * -16 - 5 + border, h - minz * -16 - 6 + border,
        minx * -16 + 5 + border, h - minz * -16 + 4 + border),
        outline=origincolor)

font = ImageFont.load_default()

if drawscale:
    draw.text((24, 0), "X", font=font, fill=scalecolor)
    draw.text((2, 24), "Z", font=font, fill=scalecolor)

    for n in range(int(minx / -4) * -4, maxx, 4):
        draw.text((minx * -16 + n * 16 + 2 + border, 0), str(n * 16),
            font=font, fill=scalecolor)
        draw.line((minx * -16 + n * 16 + border, 0,
            minx * -16 + n * 16 + border, border - 1), fill=scalecolor)

    for n in range(int(maxz / 4) * 4, minz, -4):
        draw.text((2, h - 1 - (n * 16 - minz * 16) + border), str(n * 16),
            font=font, fill=scalecolor)
        draw.line((0, h - 1 - (n * 16 - minz * 16) + border, border - 1,
            h - 1 - (n * 16 - minz * 16) + border), fill=scalecolor)

if drawplayers:
    try:
        for filename in os.listdir(path + "players"):
            f = file(path + "players/" + filename)
            lines = f.readlines()
            name = ""
            position = []
            for line in lines:
                p = string.split(line)
                if p[0] == "name":
                    name = p[2]
                    print(filename + ": name = " + name)
                if p[0] == "position":
                    position = string.split(p[2][1:-1], ",")
                    print(filename + ": position = " + p[2])
            if len(name) > 0 and len(position) == 3:
                x = (int(float(position[0]) / 10 - minx * 16))
                z = int(h - (float(position[2]) / 10 - minz * 16))
                draw.ellipse((x - 2 + border, z - 2 + border,
                    x + 2 + border, z + 2 + border), outline=playercolor)
                draw.text((x + 2 + border, z + 2 + border), name,
                    font=font, fill=playercolor)
            f.close()
    except OSError:
        pass

print("Saving")
im.save(output)
