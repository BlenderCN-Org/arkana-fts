This file is part of the
```
_____________   ____
\_   _____/  |_/ __
|   ___) \  __\____
|  |     |  |   __ \
\__|     |__|  ____/
```

Project which is distributed under the terms of the GPL license v2.0
Pompei2


# General
This is an early release of Arkana - FTS so that is not yet a full game ;
currently "only" the following things can be looked at/played with:
* go trough a complete menu (options, look for updates, ...).
* loading animated 3D Models.
* loading a simple map without objects. No navigation.
* subscribing, logging in, chatting, creating, searching and joining games online.

# Install
If you see this and are on windows, you probably already have the game installed.

If you still want to compile it from source, read a complete guide in our wiki.

## Linux 
### Requirements

The following libraries and applications are NECESSARY to compile Arkana-FTS:
* CMake version 3.9 or above
* A threading library like pthreads (libpthreads) - For the threads
* The SDL library (libSDL) - For the windows
* OpenGL, OpenGL Utilities (libGL, libGLU) - For the 3D stuff

It is known to compile in GCC 7.2.

The following 3rd party libraries will be compiled if they are not found on your system:
* The OpenAL software and OpenAl Utility Toolkit library (libopenal-soft) - For the sound
* The Ogg and Vorbis libraries (libogg, libvorbis) - For compressed music files
* A modified version of CEGUI 0.5, along with:
  * PCRE (libpcre)
  * The FreeType library (libfreetype)
* A modified version of Cal3D
* The DaoVM - For the scripting language.

### How to compile Arkana-FTS: the easy way

The most easy way to install Arkana-FTS is to call the build script:
```
  > sh build.sh
```
And if you have all necessary librarys installed, everything will be
done automatically. After the installation, you will have the fts executable
in the fts directory and can just run it
```
  > ./fts
```
and enjoy the game. For more options, see "Advanced Options" below.

### How to compile Arkana-FTS: the longer way

Change into the directory "linux", by typing
```
  > cd linux
```
There you must call CMake to check your system and create the Makefiles
```
  > cmake ..
```
Or to enable debug mode you call CMake like this
```
  > cmake -D CMAKE_BUILD_TYPE:STRING=Debug ..
```
If everything went fine, you should now have a Makefile in your directory
and thus be able to run make
```
  > make
```
If everything went fine, you should have an executable called "fts"
in the "build" directory. Copy it over one directory back using
```
  > cp fts ..
```
And go back to run fts
```
  > cd ..
  > ./fts
```

Enjoy the game.

### Advanced Options

*Debug mode:*

The first possible fine-tuning option you can do is compile Arkana-FTS in
debug mode. To do this, you just need to call the build script with
the `-d` switch turned on
```
  > sh build.sh -d
```
And Arkana-FTS will be compiled in debug mode.

Note that it will run (much) slower in debug mode then in release mode.
Debug mode should be used only for developers.

Also note that the `-d` (debug, see below) switch is only needed during the
configure step. If you activate the '-n' switch, there is no need to
activate the `-d` switch as well.

*Make clean:*

If you need some disk space or you just want to rebuild everything because
I-don't-know-what-a-b*sh*t-happened, you can run something like a 'make clean'
by calling the build script with the '-c' switch turned on
```
  > sh build.sh -c
```
And all object files, configure caches and whatsoever will be deleted.

### Links

If you have any questions, visit the forum that you will find on the official fts page: http://arkana-fts.org
Developers can also visit the Docu-Wiki where they can find a lot of developer informations.

## Windows
Create and change into the directory "win64", by typing
```
  > mkdir win64
  > cd win64
```
There you must call CMake to check your system and create the solution and project files.

```
  > cmake -G "Visual Studio 15 2017 Win64" ..
```
Now you can build the game with the Visual Studio IDE by loading the `fts.sln` file.

# Keys
Key | Function
----|-------
up/left/right/down| move around.
Ctrl+up/left/right/down| look (rotate) around.
1  |   reset the camera to it's default position.
2  |   let the camera look at the centre (0,0,0).
F9 |  show a scripting console.
F11| switch the display mode (full, wireframe, dots).
F12| show some internal informations.
ESC| quit the game.
Ctrl+x| cut the currently selected text out.
Ctrl+c| copy the currently selected text.
Ctrl+v| paste the cut or copied text into the currently selected place.

    Keys for developers (in debug mode, only temporary):
Key|Function
---|--------
F2|  Show the terrain normals.
F3|  Show the terrain coordinate system.


# Bugs
If you encounter any bug, please report it here: http://arkana-fts.com/board/
Thank you !

# Play around
If you love to take risks :) you can try to create your own maps already, using the dev's map maker.
It is located in the tools directory. You can edit the three conf files and then run the map maker,
it will create a file named test.ftst which you have to copy over in the maps directory to test it ;)

If you make a nice map, we will be proud to see screen shots of it !

Have Fun & GOOD LUCK :p

# BlaBla
I hope you will like the little you can see now and maybe look for new versions later (later doesn't mean today ;)

Pompei2 AT gmail DOT com


# MAIN CHANGES
Here are the main changes in the versions. Of course there are always a lot
of bug fixes and minor changes, that I won't list here. If you are still interested
in them, you can take a look at our forum or at the git log.

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
