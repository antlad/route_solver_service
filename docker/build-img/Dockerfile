FROM ubuntu:16.04
RUN apt-get update && apt-get install gcc g++ cmake make git wget mc --force-yes --assume-yes
RUN apt-get update && apt-get install libeigen3-dev  libboost-all-dev golang ccache --force-yes --assume-yes 
RUN apt-get update && apt-get install libluabind-dev --force-yes git --assume-yes 
RUN git clone https://github.com/Project-OSRM/osrm-backend.git
RUN apt-get update && apt-get install libtbb-dev libstxxl-dev libbz2-dev --force-yes git --assume-yes 
RUN apt-get update && apt-get install zlib1g-dev --force-yes git --assume-yes 
RUN cd osrm-backend && git checkout v5.5.2 && mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_LTO=OFF .. && make install
RUN apt-get update && apt-get install wget --force-yes git --assume-yes 
RUN wget https://cmake.org/files/v3.7/cmake-3.7.1-Linux-x86_64.tar.gz && \
tar -C /usr/local -xvzf cmake-3.7.1-Linux-x86_64.tar.gz && \
update-alternatives --install /usr/bin/cmake cmake /usr/local/cmake-3.7.1-Linux-x86_64/bin/cmake 1 --force
RUN apt-get update && apt-get install curl htop --force-yes --assume-yes
RUN export MAKEFLAGS=-j5
VOLUME /src-in

