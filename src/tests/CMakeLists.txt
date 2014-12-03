CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Prepare testing infrastructure.
INCLUDE(DownloadGoogleTesting)

DOWNLOAD_GOOGLE_TESTING()

INCLUDE_DIRECTORIES(
    SYSTEM ${GTEST_INCLUDE_DIR}
    SYSTEM ${GMOCK_INCLUDE_DIR}
)

LINK_DIRECTORIES(${GTEST_BINARY_DIR} ${GMOCK_BINARY_DIR})

# Configure some handy variables.
SET(PROJECT blackhole-tests)
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

file(GLOB_RECURSE BLACKHOLE_DOCS . ../../doc/*md)
file(GLOB_RECURSE BLACKHOLE_HEADERS . ../blackhole/*.[hi]pp)
file(GLOB_RECURSE BLACKHOLE_TESTING_HEADERS . *.hpp)

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
    msgpack
    gmock
    gtest
)

IF(ENABLE_ELASTICSEARCH)
    INCLUDE_DIRECTORIES(
        SYSTEM ${CMAKE_SOURCE_DIR}/foreign/urdl/include
    )
ENDIF()


ADD_DEFINITIONS(-std=c++0x)
