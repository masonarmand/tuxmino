# file: CMakeLists.txt
# Author: dawnvoid (https://github.com/dawnvoid)
# Contributors:
#   - Mason Armand: Added lua library
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
# to force cmake to use MinGW's gcc instead of whatever default stuff it uses.
# If you get an error about not being able to find/open raylib.lib, this is probably why.
#
# Also, you will need to copy the res/folder to the same directory as the executable.

cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(TUXMINO VERSION 0.1)

# Specify C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Set the executable output path
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# Get all .c files in the src directory
file(GLOB SOURCES "src/*.c")

# Add an executable to project as a compilation target
add_executable(tuxmino ${SOURCES})

target_compile_options(tuxmino PRIVATE
    -Wpedantic
    -pedantic-errors
    -Werror
    -Wold-style-definition
    -Wcast-align
    -Wformat=2
    -Wlogical-op
    -Wmissing-include-dirs
    -Wnested-externs
    -Wredundant-decls
    -Wshadow
    -Wswitch
    -Wundef
    -Wunreachable-code
    -Wdisabled-optimization
    -Wunsafe-loop-optimizations
    -Wfree-nonheap-object
)

target_include_directories(tuxmino PUBLIC
    ${PROJECT_BINARY_DIR}
    include/
    ./dependencies/lua/
)

target_link_directories(tuxmino PUBLIC
    lib/
)

# Compile raylib if needed. Basically, just call the CMakeLists.txt file that raylib comes with,
# but output the files one directory higher (in tuxmino/dependencies/raylib/). Pretty sure CMake
# actually moves everything to tuxmino/build/ which is even cleaner. CMake also seems to handle
# target_link_libraries(raylib) directly.
add_subdirectory(./dependencies/raylib)
add_subdirectory(./dependencies/lua)
target_link_libraries(tuxmino PRIVATE
    raylib
    lua::lib
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
)
endif()

# Copy resource files to the output directory
file(COPY res/ DESTINATION ${EXECUTABLE_OUTPUT_PATH}/res)
file(COPY gamemodes/ DESTINATION ${EXECUTABLE_OUTPUT_PATH}/gamemodes)
file(COPY config.ini DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY screenshots/ DESTINATION ${EXECUTABLE_OUTPUT_PATH}/screenshots)
