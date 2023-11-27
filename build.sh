#! /bin/sh

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
mkdir -p build
source ./environment.sh
cd build && CXX=g++ CC=gcc cmake -GNinja .. && ninja