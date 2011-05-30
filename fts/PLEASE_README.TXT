This file is part of the
_____________   ____
\_   _____/  |_/ __
|   ___) \  __\____
|  |     |  |   __ \
\__|     |__|  ____/

Project wich is distributed under the terms of the GPL license v2.0
Pompei2


General:
========
    This is an early release of Arkana - FTS so that is not yet a full game ;
    currently "only" the following things can be looked at/played with:
        * go trough a complete menu (options, look for updates, ...).
        * loading animated 3D Models.
        * loading a simple map without objects. No navigation.
        * subscribing, logging in, chatting, creating, searching and joining games online.

Install:
========
    If you see this and are on windows, you probably already have the game installed.
    If you still want to compile it from source, read a complete guide in our wiki.

    Linux users, please read the INSTALL.txt for instructions.

Keys:
=====
    up/left/right/down:      move around.
    Ctrl+up/left/right/down: look (rotate) around.

    1:   reset the camera to it's default position.
    2:   let the camera look at the centre (0,0,0).
    F9:  show a scripting console.
    F11: switch the display mode (full, wireframe, dots).
    F12: show some internal informations.
    ESC: quit the game.

    Ctrl+x: cut the currently selected text out.
    Ctrl+c: copy the currently selected text.
    Ctrl+v: paste the cut or copied text into the currently selected place.

    Keys for developers (in debug mode, only temporary):
    ----------------------------------------------------

        F2:  Show the terrain normals.
        F3:  Show the terrain coordinate system.


Bugs:
=====
    If you encounter any bug, please report it here: http://arkana-fts.org/board/
    Thank you !

Play around:
============
    If you love to take risks :) you can try to create your own maps already, using the dev's map maker.
    It is located in the tools directory. You can edit the three conf files and then run the map maker,
    it will create a file named test.ftst wich you have to copy over in the maps directory to test it ;)

    If you make a nice map, we will be proud to see screenshots of it !

    Have Fun & GOOD LUCK :p

BlaBla:
=======
    I hope you will like the little you can see now and maybe look for new versions later (later doesn't mean today ;)

    Pompei2 AT gmail DOT com


MAIN CHANGES:
=============
    Here are the main changes in the versions. Of course there are always a lot
    of bugfixes and minor changes, that I won't list here. If you are still interested
    in them, you can take a look at our forum or at the svn changelog.

    version 0.0.4:
    --------------
    - Got rid of most dependencies, like Cg, GLEE, lib3ds, libjpeg, libpng, libz, libarchive, ...
    - Added skeletal animated models based on Cal3D.
    - Use of shaders and other modern OpenGL techniques.
    - Added a scripting language named Dao.
    - Set up a master-server infrastructure with gamepage etc.

    version 0.0.3:
    --------------
    - Loading of terrain (with normals etc).
    - Automatic generating of tilesets.
    - Support for OpenGL Extensions trough GLEE.
    - Loading (animated) .3ds files trough lib3ds.
    - Prepared support for Cg shaders.
    - Added a sound system.
    - Added lights.
    - Improved overall performance.
    - Added all online-gaming stuff like chatting, creating and joining games and channels.

    Version 0.0.2:
    --------------
    - Added support for accounts.
    - Live change of screen mode and all other options.
