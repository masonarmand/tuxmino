#
# To build:
# mkdir ./build
# cd build
# cmake ..
# cmake --build .
#
# Note: On windows, you may want to run
# cmake .. -G "MinGW Makefiles"
# instead of
# cmake ..
# to force cmake to use MinGW's gcc instead of whatever default shit it uses.
# If you get an error about not being able to find/open raylib.lib, this is probably why.
#
# Also, you will need to copy the res/folder to the same directory as the executable.

cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(TUXMINO VERSION 0.1)

# Specify C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Add an executable to project as a compilation target
add_executable(tuxmino
    src/draw.c
    src/engine.c
    src/main.c
    src/piece.c
    src/timer.c
)

target_include_directories(tuxmino PUBLIC
    ${PROJECT_BINARY_DIR}
    include/
)

target_link_directories(tuxmino PUBLIC
    lib/
)

# Compile raylib if needed. Basically, just call the CMakeLists.txt file that raylib comes with, but output the files one directory higher (in tuxmino/dependencies/raylib/). Pretty sure CMake actually moves everything to tuxmino/build/ which is even cleaner. CMake also seems to handle target_link_libraries(raylib) directly.
add_subdirectory(./dependencies/raylib/raylib-4.2.0/ ./dependencies/raylib)
target_link_libraries(tuxmino PRIVATE
    raylib
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(tuxmino PRIVATE
        glfw
        m
        pthread
    )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(tuxmino PRIVATE
        opengl32
        gdi32
        winmm
    )
endif()