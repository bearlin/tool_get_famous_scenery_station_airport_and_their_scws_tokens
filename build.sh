#!/bin/bash

# Default build settings
BUILD_TYPE="-DCMAKE_BUILD_TYPE=Release"

# Update build settings
for argument in $@
do
  if [ "$argument" == "clean" ]; then
    rm -rf build
    rm -rf ./outputs/taiwan/*.txt
    rm -rf ./outputs/china/*.txt
    rm inputs/stations/*.dump
    rm inputs/scenery/*.dump
    rm inputs/airports/*.dump
    exit 0;
  fi
  if [ "$argument" == "debug" ]; then
    BUILD_TYPE="-DCMAKE_BUILD_TYPE=Debug"
  fi
  if [ "$argument" == "build-data" ]; then
    # gen_data.sh
    chmod +x gen_data.sh
    ./gen_data.sh
  fi
done

# build
echo "BUILD_TYPE=$BUILD_TYPE"
mkdir -p build && cd build && rm -rf && cmake $BUILD_TYPE .. && make && cd -

