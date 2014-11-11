CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(PROJECT blackhole-tests)

# The next line enforces google testing framework to use own tuple implementation.
ADD_DEFINITIONS(-DGTEST_USE_OWN_TR1_TUPLE=1)
ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/foreign/gmock ${CMAKE_BINARY_DIR}/gmock)

ENABLE_TESTING()

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")

IF(ENABLE_ELASTICSEARCH)
    ADD_DEFINITIONS(-DURDL_HEADER_ONLY=1)
    ADD_DEFINITIONS(-DURDL_DISABLE_SSL=1)
ENDIF()

ADD_DEFINITIONS(-DBLACKHOLE_TESTING)

IF(ENABLE_ELASTICSEARCH_DEBUG)
    ADD_DEFINITIONS(-DENABLE_ELASTICSEARCH_DEBUG=1)
ENDIF()

file(GLOB_RECURSE BLACKHOLE_HEADERS . ../blackhole/*.[hi]pp)
file(GLOB_RECURSE BLACKHOLE_TESTING_HEADERS . *.hpp)
file(GLOB_RECURSE BLACKHOLE_DOCS . ../../doc/*md)

SET(HEADERS
    ${BLACKHOLE_DOCS}
    ${CMAKE_SOURCE_DIR}/debian/changelog
    ${CMAKE_SOURCE_DIR}/blackhole-bf.spec
    ${BLACKHOLE_HEADERS}
    ${BLACKHOLE_TESTING_HEADERS}
)

SET(SOURCE_MAIN main)

SET(SOURCES_BASE
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
    test_Keyword
    test_Logger
    test_Macro
    test_Manual
    test_MsgpackFormatter
    test_NullSink
    test_ScopedAttributes
    test_StreamSink
    test_SyslogSink
    test_Traits
    test_Utils
    utils/test_GenericStringFormatter
    Dynamic
    Record
    Repository
    TcpSink
    Trace
    UdpSink
    View
    Wrapper
)

SET(SOURCES_FORMATTER
    formatter/parser
    formatter/string
)

IF(ENABLE_ELASTICSEARCH)
    SET(SOURCES_ES
        extra/ElasticsearchSink
        extra/elasticsearch/Factory
        extra/elasticsearch/Transport
        extra/elasticsearch/Requests
        extra/elasticsearch/Utils
        extra/Resolver
        extra/UrlFetch
    )
ENDIF()

SET(SOURCES_DEVELOP
    attribute/combined
    logger/verbose
)

SET(SOURCES
    ${SOURCE_MAIN}
    ${SOURCES_BASE}
    ${SOURCES_FORMATTER}
    ${SOURCES_ES}
    ${SOURCES_DEVELOP}
)

SET(Boost_USE_MULTITHREADED ON)

SET(BOOST_COMPONENTS
    system
    filesystem
    thread
    date_time
)

PROJECT(${PROJECT})

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/foreign/rapidjson/include
    SYSTEM ${CMAKE_SOURCE_DIR}/foreign/gmock/include
    SYSTEM ${CMAKE_SOURCE_DIR}/foreign/gtest/include
)

LINK_DIRECTORIES(
    ${CMAKE_BINARY_DIR}/gmock/gtest
    ${CMAKE_BINARY_DIR}/gmock
)

FIND_PACKAGE(Boost COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

ADD_EXECUTABLE(${PROJECT} ${HEADERS} ${SOURCES})

TARGET_LINK_LIBRARIES(${PROJECT}
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    gmock
    gtest
    msgpack
)

IF(ENABLE_ELASTICSEARCH)
    INCLUDE_DIRECTORIES(
        SYSTEM ${CMAKE_SOURCE_DIR}/foreign/urdl/include
    )
ENDIF()

ADD_TEST(NAME blackhole-tests COMMAND blackhole-tests)

ADD_DEFINITIONS(-std=c++0x)

