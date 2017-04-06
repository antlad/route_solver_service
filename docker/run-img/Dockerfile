FROM ubuntu:16.04
RUN apt-get update 
ADD build.deb build.deb
RUN dpkg -i build.deb;exit 0
RUN apt-get install create_route --force-yes --assume-yes;exit 0
RUN apt-get -f install --force-yes --assume-yes
RUN rm build.deb
EXPOSE 7779
VOLUME /osrm/
# ENTRYPOINT ["/usr/local/bin/create_route_backend"]
CMD []

