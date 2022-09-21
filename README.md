# tuxmino
A falling block puzzle game inspired by the [TGM series](https://tetris.wiki/Tetris_The_Grand_Master_3_Terror-Instinct).  
Written in C using the raylib library.  
The game does not completely replicate the exact mechanics of the [Tetris Grand Master series](https://tetris.wiki/Tetris_The_Grand_Master_3_Terror-Instinct), this was just made as a fun project.  

## Features
- Several game modes (Master, Easy, Death, Invisible, 20G Practice)
- Speed timings such as ARR, DAS, Lock Delay, etc
- TGM inspired mechanics such as [20G gravity](https://tetris.wiki/20G) and sonic drop
- [ARS rotation](https://tetris.wiki/Arika_Rotation_System)

## Controls
- `Left and Right arrows` = move left and right
- `Up arrow` = sonic drop
- `Down arrow` = soft drop/lock piece
- `Space` = hold
- `Z` = rotates left
- `X` = rotatates right
- `Escape` = pause
- `R` = restart/back to main menu
- `S` = take a screenshot (saved in screenshot folder)

## Todo
- Add options menu to configure things like sound, key mapping, etc
- Add scoring system and timer
- Finish making Death mode, Mater mode, and Easy mode
- Add more modes?

## Screenshots
<img src="https://github.com/Scarbyte/tuxmino/blob/main/screenshots/screenshot.png?raw=true" width=600>
<img src="https://github.com/Scarbyte/tuxmino/blob/main/screenshots/menu.png?raw=true" width=600>

<br>

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

## Compiling on Linux Manually
### Required dependecies
- [raylib](https://github.com/raysan5/raylib)
- [glfw](https://github.com/glfw/glfw)

`gcc ./src/*.c -lraylib -lglfw -lm -pthread -o tuxmino`

<br>

To compile statically make sure to link to the respective static libraries.  
Example (assuming raylib is in the same directory as the project):  
```gcc ./src/*c -I./raylib//include `pkg-config --cflags glfw3` -o tuxmino -L./raylib/lib -lraylib `pkg-config --static --libs glfw3` -lm -pthread```

<br>

## Credits
- Background images from NASA
- Click sound (res/snd/lock.wav) courtesy of [lebaston100](https://freesound.org/people/lebaston100/sounds/192276/)
