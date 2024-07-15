# voxelands

Clone of voxelands for correcting bugs made by :
------------------------------------------------
Pierre Brochard : pierre.brochard.1982@m4x.org

There is some threading problems which are corrected here.
And also some cleaning.
It seems to work without segfaults in some multi-hours sessions testing.

The original is taken from :
----------------------------
https://gitlab.com/voxelands/voxelands
========================================================================
				README
========================================================================

ABOUT:
Voxelands is a sandbox construction game based on Minetest, which was
 inspired by earlier "voxel world" games such as Infiniminer. Gameplay
 puts players in a fully destructible 3D game world where they can place
 and dig numerous types of blocks, items and creatures using a variety
 of tools. Inside the game world players can build structures, artworks
 and anything else their creativity can think of on multiplayer servers
 and singleplayer worlds across multiple game modes.

DEPENDENCIES:
Voxelands requires the standard C++ library, irrlicht 1.8.5 or later,
 freetype, openAL, vorbisfile, ogg, zlib. CMake, make, and g++ (or other
 C++ compiler) and bzip2 and jpeg are required to build from source.

INSTALL:
If you're reading this then you've already unpacked the tarball, so
 just:
1) cd /path/to/voxelands
2) cmake -DRUN_IN_PLACE=1 .
3) make -j3

Other optional cmake parameters:
1) -DBUILD_SERVER=0 (default 1) builds the dedicated server
2) -DBUILD_CLIENT=0 (default 1) builds the rich client
3) -DENABLE_AUDIO=0 (default 1) enables audio support

The option -DRUN_IN_PLACE expects the user to run voxelands from the extracted source directory.

WINDOWS:
Voxelands can be cross compiled with mingw, or built natively with MSVC,
 instructions for the latter are available on the forums:
 http://forum.voxelands.com

USE:
Simply run bin/voxelands from any console/terminal.

CONTROLS:
Controls can be set in the Settings tab of the main menu

Main controls:

    W, S, A, D - Walk forward backward, left, and right.
    Mouse - Look / Aim.
    Mouse wheel - Change wielded item.
    1 through 8 - Change wielded item.
    Right Mouse Button - Place item.
    Left Mouse Button - Dig / Punch / Attack.
    Shift - Sneak.
    Space - Jump.
    R - Move Up (ladders, swimming, flying)
    F - Move Down (ladders, swimming, flying)
    E - Run.

Action controls:

    I - Enter inventory.
    Q - Examine pointed block.
    H - Use item in hand.
=======
    T - Talk / commands.
    K - Toggle fly mode.
    F1 - Hide / show hud.
    F2 - Hide / show chat.
    F3 - Disable / enable fog.
    F4 - Disable / enable camera update.
    F5 - Toggle through debug information.
    F6 - Toggle through profiler (info on FPS, chunk updates, etc.)
    F12 - Take a screenshot.


LICENSE:
Code:
Voxelands is licensed under the GPLv3 or later.
Copyright (c) 2013-2016 Lisa 'darkrose' Milne <lisa@ltmnet.com>
Forked from Minetest 0.3.x
Copyright (c) 2010-2011 Perttu Ahola <celeron55@gmail.com>

Textures, Sounds, and Models:

This does not apply to texture packs made by others.

Attribution-ShareAlike 3.0 Unported (CC BY-SA 3.0)
http://creativecommons.org/licenses/by-sa/3.0/

To satisfy the attribution clause, any project using Voxelands' assets
 must provide a prominent notice as part of each of said project's
 credits notices (such as in documentation, on a website, and/or any
 other credits screen associated with the project) showing both the
 CC-BY-SA licensing, the ownership by Voxelands of the asset, and a link
 to the Voxelands' project at http://www.voxelands.com/.

Artists include:
sdzen
darkrose
sapier
Tom Peter
Telaron
juskiddink
With special thanks to http://www.opengameart.org/

DEVELOPMENT:
Voxelands is under active development, the latest updates can be found
 at:
  https://gitlab.com/voxelands/voxelands
 or on IRC at:
  irc.entropynet.net #voxelands

CHANGES:
See doc/changelog.txt
