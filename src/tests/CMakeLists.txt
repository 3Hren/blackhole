CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(PROJECT blackhole-tests)

INCLUDE(Compiler)

# Use gmock 1.6 for ancient compilers and systems.
IF(CMAKE_VERSION VERSION_LESS 2.8.5 OR (DEFINED GCC_COMPILER_VERSION AND GCC_COMPILER_VERSION VERSION_LESS 4.8))
    # Hack for cmake (< 2.8.5), which cannot extract zip archives.
    # Also gmock doesn't compile on GCC (< 4.8).
    SET(GMOCK_URL "https://s3-us-west-2.amazonaws.com/blackhole.testing/gmock-1.6.0.tar.gz")
ELSE()
    SET(GMOCK_URL "https://googlemock.googlecode.com/files/gmock-1.7.0.zip")
    SET(GMOCK_URL_MD5 073b984d8798ea1594f5e44d85b20d66)
ENDIF()

INCLUDE(ExternalProject)
SET_DIRECTORY_PROPERTIES(properties EP_PREFIX "${CMAKE_BINARY_DIR}/foreign")
ExternalProject_ADD(googlemock
    URL ${GMOCK_URL}
    SOURCE_DIR "${CMAKE_BINARY_DIR}/foreign/gmock"
    CMAKE_ARGS "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}" "-DCMAKE_CXX_FLAGS=-fPIC"
    INSTALL_COMMAND ""
)
ExternalProject_GET_PROPERTY(googlemock SOURCE_DIR)
ExternalProject_GET_PROPERTY(googlemock BINARY_DIR)
SET(GMOCK_SOURCE_DIR ${SOURCE_DIR})
SET(GMOCK_BINARY_DIR ${BINARY_DIR})

INCLUDE_DIRECTORIES(
    SYSTEM ${GMOCK_SOURCE_DIR}/include
    SYSTEM ${GMOCK_SOURCE_DIR}/gtest/include
)

LINK_DIRECTORIES(${GMOCK_BINARY_DIR} ${GMOCK_BINARY_DIR}/gtest)

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
    attribute/combined
    datetime/test_DayOfYear
    datetime/test_DayOfMonth
    datetime/test_DayOfWeek
    datetime/test_Month
    datetime/test_Other
    datetime/test_Time
    datetime/test_Year
    datetime/test_Week
    logger/composite
    logger/verbose
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

IF(ENABLE_TRACING_FRAMEWORK)
    SET(SOURCES_TRACING
        Trace
    )
ENDIF()

IF(ENABLE_TESTING_THREAD)
    SET(SOURCES_FUNCTIONAL
        functional/threading/logger
    )
ENDIF()

SET(SOURCES_DEVELOP
)

SET(SOURCES
    ${SOURCE_MAIN}
    ${SOURCES_BASE}
    ${SOURCES_FUNCTIONAL}
    ${SOURCES_FORMATTER}
    ${SOURCES_ES}
    ${SOURCES_TRACING}
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
)

FIND_PACKAGE(Boost COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

ADD_EXECUTABLE(${PROJECT} ${HEADERS} ${SOURCES})

ADD_DEPENDENCIES(${PROJECT} googlemock)

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

ENABLE_TESTING()
ADD_TEST(NAME blackhole-tests COMMAND blackhole-tests)

ADD_DEFINITIONS(-std=c++0x)
