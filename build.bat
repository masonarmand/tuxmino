@echo off
IF NOT EXIST "build" (
  mkdir build
)
IF NOT EXIST "include" (
  mkdir include
)
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
