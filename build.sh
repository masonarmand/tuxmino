#!/bin/bash
# check if build directory exists
if [ ! -d "build" ]; then
  mkdir build
fi
cd build
cmake ..
make
