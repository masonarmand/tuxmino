<img src='https://raw.githubusercontent.com/masonarmand/tuxmino/main/screenshots/tuxmino-gameplay.webp'>

# tuxmino
Highly moddable free open source falling block puzzle game. Written in C using the
[raylib](https://github.com/raysan5/raylib) game library.

The game is still in its early stages and is missing some features.

For documentation, tutorials, info on contributing, etc visit [tuxmino.org](https://tuxmino.org).

## Features
- Lua support for making your own custom game modes.
- Mechanics that attempt to replicate old arcade stackers.
- All the necessary speed timings (Lock delay, appearance delay, etc).
- Two rotation systems
    - Classic: Harder and more limiting rotation rule. Less kicks.
    - World: Rotation system with lots of kicks, think of this as "easy mode".

## Todo list & known issues
View the [todo list](TODO.md).

## Compiling

Please make sure the required dependencies are met:
- [raylib dependencies linux](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux)

Then simply either run `./build.sh` or `build.bat` depending on what OS you are on.
The executable and all required files will be output to the `build/bin/` folder.
