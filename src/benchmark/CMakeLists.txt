CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
SET(PROJECT blackhole-benchmark)

ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/foreign/ticktack ${CMAKE_BINARY_DIR}/ticktack)

SET(CMAKE_BUILD_TYPE "Release")

SET(HEADERS
)

SET(SOURCES
    attribute_set
    detail/datetime
    formatter/string
    logger/logger
)

PROJECT(${PROJECT})

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/foreign/ticktack/include
)

LINK_DIRECTORIES(
    ${CMAKE_BINARY_DIR}/ticktack
)

SET(Boost_USE_MULTITHREADED ON)

SET(BOOST_COMPONENTS
    system
    thread
)

FIND_PACKAGE(Boost COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

ADD_EXECUTABLE(${PROJECT} ${HEADERS} ${SOURCES})

TARGET_LINK_LIBRARIES(${PROJECT}
    ${Boost_LIBRARIES}
    ticktack
    ticktack-main
)

ADD_DEFINITIONS(-std=c++0x)
