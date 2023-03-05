# tuxmino
Highly moddable free open source falling block puzzle game. Written in C.

[Website](https://tuxmino.org)

## Features
- Lua support for making your own custom game modes.
- Mechanics that attempt to replicate old arcade stackers.
- All the necessary speed timings (Lock delay, appearance delay, etc).
- Two rotation systems
    - Classic: Harder and more limiting rotation rule. Less kicks.
    - World: Rotation system with lots of kicks, think of this as "easy mode".

## Todo
- Update compilation instructions.
- Finish implementing lua support.
- Support for animated backgrounds (.GIF format).
- Finish website


## Compiling on Windows using CMake
`mkdir build`
`cd build`
`cmake .. -G "MinGW Makefiles"`
`cmake --build .`

After compiling make sure your compiled executable is in the same folder as the `res` folder.

<br>

## Compiling on Linux using CMake
`git clone https://github.com/Scarbyte/tuxmino.git`
`cd tuxmino`
`mkdir build`
`cd build`
`cmake ..`
`cmake --build .`

Then make sure your compiled binary is in the same folder as the `res` folder.

<br>

## Using the makefile
