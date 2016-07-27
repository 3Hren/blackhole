FROM ubuntu:trusty

RUN apt-get -y -qq update && \
    apt-get -y -qq install build-essential devscripts equivs

COPY . /build/blackhole
RUN cd /build/blackhole && \
    DEBIAN_FRONTEND=noninteractive mk-build-deps -ir -t "apt-get -qq --no-install-recommends"
RUN cd /build/blackhole && \
    yes | debuild -e CC -e CXX -uc -us -j$(cat /proc/cpuinfo | fgrep -c processor) && \
    debi

# Test.
RUN cd /tmp && mkdir build && cd build && \
    cmake -DENABLE_TESTING=ON /build/blackhole && make && \
    ./blackhole-tests

# Cleanup
RUN DEBIAN_FRONTEND=noninteractive apt-get -qq purge blackhole-build-deps && \
    DEBIAN_FRONTEND=noninteractive apt-get -qq purge build-essential devscripts equivs && \
    DEBIAN_FRONTEND=noninteractive apt-get -qq autoremove --purge && \
    rm -rf build && \
    rm -rf /tmp
