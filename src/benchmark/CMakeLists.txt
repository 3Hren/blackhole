CMAKE_MINIMUM_REQUIRED(VERSION 2.8.5)

# Prepare benchmarking infrastructure.
INCLUDE(DownloadBenchmarking)
DOWNLOAD_BENCHMARKING()
INCLUDE_DIRECTORIES(${EPICMETER_INCLUDE_DIR})
LINK_DIRECTORIES(${EPICMETER_BINARY_DIR})

# Configure benchmarking target.
SET(PROJECT blackhole-benchmark)
SET(CMAKE_BUILD_TYPE "Release")
ADD_DEFINITIONS(-DBOOST_DISABLE_ASSERTS=1)

SET(HEADERS
    util.hpp
)

SET(SOURCES
#    attribute_set
    detail/datetime
    formatter/string
    logger/batch
    logger/logger
#    logger/thread
#    experimental
)

PROJECT(${PROJECT})

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/foreign/rapidjson/include
)

SET(Boost_USE_MULTITHREADED ON)

SET(BOOST_COMPONENTS
    system
    filesystem
    thread
)

FIND_PACKAGE(Boost COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

ADD_EXECUTABLE(${PROJECT} ${HEADERS} ${SOURCES})
ADD_DEPENDENCIES(${PROJECT} em)

TARGET_LINK_LIBRARIES(${PROJECT}
    ${Boost_LIBRARIES}
    epicmeter
    epicmeter-main
)

ADD_DEFINITIONS(-std=c++0x)
