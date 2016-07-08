FROM ubuntu:xenial

RUN apt-get update

RUN apt-get -y install cmake
RUN apt-get -y install g++
RUN apt-get -y install git
RUN apt-get -y install libboost-dev
RUN apt-get -y install libboost-system-dev
RUN apt-get -y install libboost-thread-dev
RUN apt-get -y install make
RUN apt-get -y install valgrind

COPY . /code/blackhole/

RUN dpkg -l | grep g++
RUN dpkg -l | grep boost

RUN mkdir -p /code/blackhole/build
WORKDIR /code/blackhole/build

RUN rm -rf *
RUN cmake -DENABLE_TESTING=ON ..
RUN make -j1
RUN valgrind ./blackhole-tests
