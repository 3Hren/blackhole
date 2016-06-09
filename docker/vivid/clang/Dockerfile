FROM ubuntu:vivid

RUN apt-get update

RUN apt-get -y install cmake
RUN apt-get -y install g++
RUN apt-get -y install git
RUN apt-get -y install libboost-dev
RUN apt-get -y install libboost-system-dev
RUN apt-get -y install libboost-thread-dev
RUN apt-get -y install make
RUN apt-get -y install valgrind

RUN apt-get -y install clang++-3.6
RUN update-alternatives --install /usr/bin/cc cc /usr/bin/clang-3.6 100
RUN update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-3.6 100

COPY . /code/blackhole/

RUN dpkg -l | grep clang
RUN dpkg -l | grep boost

RUN mkdir -p /code/blackhole/build
WORKDIR /code/blackhole/build

RUN rm -rf *
RUN cmake -DENABLE_TESTING=ON ..
RUN make -j1
RUN valgrind ./blackhole-tests
