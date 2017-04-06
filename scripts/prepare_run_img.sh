#!/bin/bash
cd $(dirname $0)
pushd ${PWD}/../docker/run-img/
rm -f *.deb
cp ../../build/*.deb build.deb
docker build -t vladtroinich/routing:latest .
popd


