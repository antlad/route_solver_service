#!/bin/bash
docker run -v /home/vlad/data/osrm-docker:/osrm -p 7779:7779  --rm vladtroinich/routing /usr/local/bin/create_route_backend --osrm_path=/osrm/RU-MOW.osrm