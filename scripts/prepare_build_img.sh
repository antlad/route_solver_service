#!/bin/bash
cd $(dirname $0)
pushd ../docker/build-img
docker build -t create_route_build .
popd
