CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")

SET(PROJECT blackhole-tests)

ENABLE_TESTING()

SET(HEADERS
    global.hpp
    mocks.hpp
    mocks/files.hpp
    mocks/logger.hpp
    mocks/socket.hpp
    mocks/syslog.hpp
    datetime/common.hpp
)

SET(SOURCES
    main
    datetime/test_DayOfYear
    datetime/test_DayOfMonth
    datetime/test_DayOfWeek
    datetime/test_Month
    datetime/test_Other
    datetime/test_Time
    datetime/test_Year
    datetime/test_Week
    test_AttachableStream
    test_Attribute
    test_Config
    test_ConfigJsonInitialization
    test_ExceptionTrap
    test_FactoryInitialization
    test_FileRotation
    test_FileRotationNaming
    test_FileBackend
    test_FileRotationSizeWatcher
    test_FileRotationDateTimeWatcher
    test_FileSink
    test_Filter
    test_Functional
    test_JsonFormatter
    test_Logger
    test_Macro
    test_Manual
    test_MsgpackFormatter
    test_NullSink
    test_StreamSink
    test_StringFormatter
    test_StringPatternParser
    test_SyslogSink
    test_Traits
    test_UdpSink
    test_Utils
    test_VerboseLogger
)

SET(Boost_USE_MULTITHREADED ON)

SET(BOOST_COMPONENTS
    system
    filesystem
    thread
)

SET(GOOGLE_LIBRARIES
    gtest
    gmock
)

PROJECT(${PROJECT})

INCLUDE_DIRECTORIES(
    ../
    ../../foreign/rapidjson/include
)

FIND_PACKAGE(Boost COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

ADD_EXECUTABLE(${PROJECT} ${HEADERS} ${SOURCES})

TARGET_LINK_LIBRARIES(${PROJECT}
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${GOOGLE_LIBRARIES}
    blackhole
    msgpack
)

ADD_TEST(NAME blackhole-tests COMMAND blackhole-tests)

ADD_DEFINITIONS(-std=c++0x)
