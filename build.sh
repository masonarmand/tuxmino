#!/bin/bash
# check if build directory exists
if [ ! -d "build" ]; then
  mkdir build
fi
# check if include directory exists
if [ ! -d "include" ]; then
  mkdir include
fi
cd build
cmake ..
make
