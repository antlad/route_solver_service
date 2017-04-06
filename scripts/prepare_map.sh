#!/bin/bash
docker run -v /home/vlad/data/osrm-docker:/osrm --rm vladtroinich/routing /usr/local/bin/osrm-extract /osrm/RU-MOW.osm.pbf -p /osrm/profiles/car.lua 
docker run -v /home/vlad/data/osrm-docker:/osrm --rm vladtroinich/routing /usr/local/bin/osrm-contract /osrm/RU-MOW.osrm
