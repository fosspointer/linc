#!/bin/sh

cd "$(dirname "$0")"
SCRIPT_DIR="$(pwd)"

if ! command -v cmake >/dev/null 2>&1; then
    echo "Error: CMake is not installed. Please install it and then run this script."
    exit 1
fi

if command -v ninja >/dev/null 2>&1; then
    build_system="ninja"
elif command -v make >/dev/null 2>&1; then
    build_system="make"
else
    echo "Error: No appropriate build system was found (Makefile/Ninja). Maybe try compiling this manually?"
    exit 1
fi

mkdir -p build
cd build

if [ -f "Makefile" ]; then 
    build_system="make"
elif [ -f "build.ninja" ]; then
    build_system="ninja"
fi

if "$build_system" = "make"; then
    CXX=g++ CC=gcc cmake -G"Unix Makefiles" ..
    sudo make install
else 
    CXX=g++ CC=gcc cmake -GNinja ..
    sudo ninja install
fi