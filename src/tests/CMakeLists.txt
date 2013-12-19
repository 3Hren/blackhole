CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_BUILD_TYPE "Debug")

SET(PROJECT blackhole-tests)

ENABLE_TESTING()

SET(HEADERS
    Global.hpp
    Mocks.hpp
)

SET(SOURCES
    main
    test_StringFormatter
    test_FileSink
    test_Logger
    test_VerboseLogger
    test_Attribute
    test_Filter
    test_StringPatternParser
    test_UdpSink
    test_Manual
    test_SyslogSink
    test_Functional
    test_FactoryInitialization
    test_JsonFormatter
    test_ExceptionTrap.cpp
)

ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)

SET(Boost_USE_MULTITHREADED ON)

SET(BOOST_COMPONENTS
    system
    filesystem
)

SET(GOOGLE_LIBRARIES
    gtest
    gmock
)

PROJECT(${PROJECT})

INCLUDE_DIRECTORIES(
    ../
)

FIND_PACKAGE(Boost COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

ADD_EXECUTABLE(${PROJECT} ${HEADERS} ${SOURCES})

TARGET_LINK_LIBRARIES(${PROJECT}
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${GOOGLE_LIBRARIES}
    blackhole
)

ADD_TEST(NAME blackhole-tests COMMAND blackhole-tests)

ADD_DEFINITIONS(-std=c++0x)
