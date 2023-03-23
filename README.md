<img src='https://raw.githubusercontent.com/masonarmand/tuxmino/v1/screenshots/tuxmino-gameplay.webp'>

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

### Compiling on Windows using CMake
```
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

After compiling make sure your compiled executable is in the same folder as the `res` folder.

<br>

### Compiling on Linux using CMake
```
git clone https://github.com/Scarbyte/tuxmino.git
cd tuxmino
mkdir build
cd build
cmake ..
cmake --build .
```

Then make sure your compiled binary is in the same folder as the `res` folder.

<br>

### Using the makefile
If lua5.4 and raylib are installed/compiled as shared libraries, you can use the makefile to quickly
compile the game for development purposes. The makefile enables a lot of compiler warnings and also
automatically launches the game in gdb for debugging. If you do not wish to launch the game in gdb
you can edit the makefile and change `tuxmino debug clean` to `tuxmino run clean`. To compile the
game using the makefile, simply run the command `make` in the project directory.
