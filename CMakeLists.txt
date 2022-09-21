#
# To build:
# cd ./build/
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
project(TUXMINO VERSION 1.0)

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

target_link_libraries(tuxmino PRIVATE
    raylib
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(glfw)
    find_package(m)
    target_link_libraries(tuxmino PRIVATE
        raylib
        glfw
        m
    )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    # find_package(opengl32)
    # find_package(gdi32)
    # find_package(lwinmm)
    target_link_libraries(tuxmino PRIVATE
        raylib
        opengl32
        gdi32
        winmm
    )
endif()