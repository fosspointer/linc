#! /bin/sh

cd "$(dirname "$0")"
SCRIPT_DIR="$(pwd)"
mkdir -p build
source ./environment.sh
cd build && CXX=g++ CC=gcc cmake -GNinja .. && ninja