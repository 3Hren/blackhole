SET(PROJECT blackhole-benchmark)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.5)

ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/foreign/epicmeter ${CMAKE_BINARY_DIR}/epicmeter)

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
    ${CMAKE_SOURCE_DIR}/foreign/epicmeter/include
)

LINK_DIRECTORIES(
    ${CMAKE_BINARY_DIR}/epicmeter
)

SET(Boost_USE_MULTITHREADED ON)

SET(BOOST_COMPONENTS
    system
    filesystem
    thread
)

FIND_PACKAGE(Boost COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

ADD_EXECUTABLE(${PROJECT} ${HEADERS} ${SOURCES})

TARGET_LINK_LIBRARIES(${PROJECT}
    ${Boost_LIBRARIES}
    epicmeter
    epicmeter-main
)

ADD_DEFINITIONS(-std=c++0x)
