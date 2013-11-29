CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_BUILD_TYPE "Release")

SET(PROJECT blackhole-tests)

SET(HEADERS
    Global.hpp
    Mocks.hpp
)

SET(SOURCES
    test_StringFormatter
    main
)

ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)

SET(Boost_USE_MULTITHREADED ON)

SET(BOOST_COMPONENTS
)

SET(GOOGLE_LIBRARIES
    gtest
    gmock
)

PROJECT(${PROJECT})

INCLUDE_DIRECTORIES(../
    /Users/esafronov/sandbox/Celero/include
)

FIND_PACKAGE(Boost COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

ADD_EXECUTABLE(${PROJECT} ${HEADERS} ${SOURCES})

TARGET_LINK_LIBRARIES(${PROJECT}
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${GOOGLE_LIBRARIES}
    blackhole
    /Users/esafronov/sandbox/Celero/build/libcelero.dylib
)

ADD_DEFINITIONS(-std=c++11)
