#!/bin/bash
cd $(dirname $0)
rm -f ${PWD}/../build/*.deb
#rm -f ${PWD}/../build-size2d/golang/size-mes-2d-backend
#can't do this by single build command because cmake deb package generator
docker run --rm -v "${PWD}/../:/src-in" -t create_route_build bash -c "cd /src-in && mkdir -p build && cd build && export MAKEFLAGS=-j5 && export CGO_LDFLAGS=-L/src-in/build/src/lib:/src-in/build/src/wrapper && cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_BACKEND_BUILD=ON -DBUILD_ENABLE_CPACK=ON .. && make && make package "



