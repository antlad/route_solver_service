#!/bin/bash
cwd=$(pwd)
ninja_build=../eclipse_ninja_create_route
mkdir -p ${ninja_build}
pushd ${ninja_build}
cmake -DCMAKE_BUILD_TYPE=Debug -G "Eclipse CDT4 - Ninja" -DCMAKE_ECLIPSE_VERSION=4.5 -DENABLE_UNIT_TESTS=ON ${cwd}
popd 

