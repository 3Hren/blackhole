FROM drydock/u12:prod

RUN apt-get update

RUN apt-get -y install cmake
RUN apt-get -y install git
RUN apt-get -y install libboost-dev
RUN apt-get -y install libboost-system-dev
RUN apt-get -y install libboost-thread-dev

COPY . /code/blackhole

RUN mkdir -p /code/blackhole/build
WORKDIR /code/blackhole/build

RUN rm -rf *
RUN cmake -DENABLE_TESTING=ON ..
RUN make install
