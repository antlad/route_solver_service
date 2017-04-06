To build project you should you docker. Here some helper scripts to easy get package and docker images.

- scripts/prepare_build_img.sh -prepare docker build image to get dpkg
- scripts/prepare_deb.sh - build dpkg with help of docker build image
- scripts/prepare_run_img.sh - create service runtime docker image

Some examples:

- scripts/prepare_map.sh - examples of commands to prepare osrm maps that will be used by runtime docker image
- scripts/run.sh - how to run docker runtime image