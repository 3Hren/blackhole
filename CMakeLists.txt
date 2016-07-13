cmake_minimum_required(VERSION 2.6)

# Choose new behaviour for CMP0042.
# See http://www.cmake.org/cmake/help/v3.0/policy/CMP0042.html for more details.
if (POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif (POLICY CMP0042)

OPTION(ENABLE_TESTING "Build the library with tests" OFF)
OPTION(ENABLE_EXAMPLES "Build examples" OFF)
OPTION(ENABLE_BENCHMARKING "Build the library with benchmarks" OFF)
OPTION(ENABLE_TESTING_THREADSAFETY "Build the thread-safety testing suite" OFF)

set(LIBRARY_NAME blackhole)

project(${LIBRARY_NAME})

find_package(Boost 1.46 REQUIRED COMPONENTS
    system
    thread)

include_directories(BEFORE SYSTEM
    ${PROJECT_SOURCE_DIR}/foreign/libcds
    ${PROJECT_SOURCE_DIR}/foreign/rapidjson/include)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(${LIBRARY_NAME} SHARED
    src/attribute
    src/config/factory
    src/config/json
    src/config/node
    src/config/option
    src/datetime/generator.linux
    src/datetime/generator.other
    src/essentials.cpp
    src/format
    src/formatter/json
    src/formatter/mod
    src/formatter/string.cpp
    src/formatter/string/error
    src/formatter/string/grammar
    src/formatter/string/parser
    src/formatter/string/token
    src/handler.cpp
    src/handler/blocking
    src/logger
    src/procname
    src/record
    src/filter/severity.cpp
    src/registry
    src/root
    src/scope/holder
    src/scope/manager
    src/scope/watcher
    src/sink/asynchronous
    src/sink/asynchronous.p
    src/sink/console
    src/sink/file
    src/sink/null
    src/sink/socket/tcp
    src/sink/socket/udp
    src/sink/syslog
    src/termcolor.cpp
    src/wrapper)

# Set the Standard version.
include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-std=c++14" COMPILER_SUPPORTS_CXX14)
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
check_cxx_compiler_flag("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

if (COMPILER_SUPPORTS_CXX14)
    add_definitions(-std=c++14)
elseif (COMPILER_SUPPORTS_CXX11)
    add_definitions(-std=c++11)
elseif (COMPILER_SUPPORTS_CXX0X)
    add_definitions(-std=c++0x)
else ()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif ()

# Set maximum warnings level depending on compiler.
# TODO: target_maximum_warnings(${LIBRARY_NAME})
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options(${LIBRARY_NAME} PRIVATE
        -Weverything
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-padded
        -Wno-shadow
        -Wno-weak-vtables
        -pedantic
        -pedantic-errors)
else ()
    set_target_properties(${LIBRARY_NAME} PROPERTIES
        COMPILE_FLAGS "-Wall"
        COMPILE_FLAGS "-Wextra"
        COMPILE_FLAGS "-Waddress"
        COMPILE_FLAGS "-Warray-bounds"
        COMPILE_FLAGS "-Wbuiltin-macro-redefined"
        COMPILE_FLAGS "-Wconversion"
        COMPILE_FLAGS "-Wctor-dtor-privacy"
        COMPILE_FLAGS "-Winit-self"
        COMPILE_FLAGS "-Wnon-virtual-dtor"
        COMPILE_FLAGS "-Wold-style-cast"
        COMPILE_FLAGS "-Woverloaded-virtual"
        COMPILE_FLAGS "-Wsuggest-attribute=const"
        COMPILE_FLAGS "-Wsuggest-attribute=noreturn"
        COMPILE_FLAGS "-Wsuggest-attribute=pure"
        COMPILE_FLAGS "-Wswitch"
        COMPILE_FLAGS "-Wunreachable-code"
        COMPILE_FLAGS "-pedantic"
        COMPILE_FLAGS "-pedantic-errors"
        LINK_FLAGS -Wl,--version-script=${PROJECT_SOURCE_DIR}/libblackhole1.version)
endif ()

target_link_libraries(${LIBRARY_NAME}
        ${Boost_LIBRARIES}
)

# The rule is that: any breakage of the ABI must be indicated by incrementing the SOVERSION.
# So, adding e.g. functions is no problem, modifying argument lists or removing functions would
# required the SOVERSION to be incremented. Similar rules hold of course for non-opaque
# data-structures.
set_target_properties(${LIBRARY_NAME} PROPERTIES VERSION 1.0.0)
set_target_properties(${LIBRARY_NAME} PROPERTIES SOVERSION 1)

if (ENABLE_TESTING_THREADSAFETY)
    add_executable(${LIBRARY_NAME}-tests-rc-assign
        tests/rc/assign)

    target_link_libraries(${LIBRARY_NAME}-tests-rc-assign
        ${LIBRARY_NAME}
        ${CMAKE_THREAD_LIBS_INIT}
        ${Boost_LIBRARIES})

    add_executable(${LIBRARY_NAME}-tests-rc-filter
        tests/rc/filter)

    target_link_libraries(${LIBRARY_NAME}-tests-rc-filter
        ${LIBRARY_NAME}
        ${CMAKE_THREAD_LIBS_INIT}
        ${Boost_LIBRARIES})
endif (ENABLE_TESTING_THREADSAFETY)

if (ENABLE_TESTING)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/foreign/modules")
    include(DownloadGoogleTesting)

    find_package(Threads)

    prepare_google_testing()

    include_directories(BEFORE SYSTEM
        ${PROJECT_SOURCE_DIR}/foreign/rapidjson/include
        ${PROJECT_SOURCE_DIR}/foreign/libcds)

    include_directories(
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/tests/include)

    if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
        include(CodeCoverage)

        find_program(GCOV_PATH gcov)
        find_program(LCOV_PATH lcov)
        find_program(GENHTML_PATH genhtml)

        if (NOT(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "Coverage"))
                message(WARNING "Code coverage results with an optimized (non-Debug) build may be misleading")
        endif ()

        function(setup_target_for_coverage _targetname _testrunner _outputname)
            ADD_CUSTOM_TARGET(${_targetname}
                # Reset all execution counts to zero.
                ${LCOV_PATH} --directory . --zerocounters

                # Run tests.
                COMMAND ${_testrunner} ${ARGV3}

                # Capture coverage data.
                COMMAND ${LCOV_PATH} --compat-libtool --directory . --capture --output-file ${_outputname}.info
                COMMAND ${LCOV_PATH} --remove ${_outputname}.info 'tests/*' '/usr/*' 'foreign/*' --output-file ${_outputname}.info.cleaned

                # Generating the report.
                COMMAND ${GENHTML_PATH} -o ${_outputname} ${_outputname}.info.cleaned
                # COMMAND ${CMAKE_COMMAND} -E remove ${_outputname}.info ${_outputname}.info.cleaned
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Resetting code coverage counters to zero.\nProcessing code coverage counters and generating report.")

            # Show info where to find the report
            ADD_CUSTOM_COMMAND(TARGET ${_targetname} POST_BUILD
                COMMAND ;
                COMMENT "Open ./${_outputname}/index.html in your browser to view the coverage report.")
        ENDFUNCTION() # SETUP_TARGET_FOR_COVERAGE
        setup_target_for_coverage(coverage ${LIBRARY_NAME}-tests .coverage)

        set(CMAKE_C_FLAGS   "-g -O0 -fprofile-arcs -ftest-coverage")
        set(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
    endif ()

  add_executable(${LIBRARY_NAME}-tests
        tests/attribute
        tests/config/json
        tests/config/option
        tests/datetime
        tests/facade
        tests/record
        tests/registry
        tests/root
        tests/severity
        tests/src/mocks/formatter
        tests/src/mocks/handler
        tests/src/mocks/logger
        tests/src/mocks/sink
        tests/src/unit/stdext/string_view
        tests/src/unit/detail/formatter/string/parser.cpp
        tests/src/unit/detail/handler/blocking.cpp
        tests/src/unit/detail/mpsc
        tests/src/unit/detail/record
        tests/src/unit/formatter/grammar
        tests/src/unit/formatter/json
        tests/src/unit/formatter/string.cpp
        tests/src/unit/formatter/token
        tests/src/unit/sink/asynchronous
        tests/src/unit/sink/console.cpp
        tests/src/unit/sink/console/builder.cpp
        tests/src/unit/sink/file.cpp
        tests/src/unit/sink/file/flusher/bytecount.cpp
        tests/src/unit/sink/file/flusher/repeat.cpp
        tests/src/unit/sink/file/stream.cpp
        tests/src/unit/sink/null
        tests/src/unit/sink/syslog
        tests/src/unit/sink/tcp
        tests/src/unit/sink/udp.cpp
        tests/src/unit/termcolor.cpp
        tests/wrapper)

    enable_google_testing(${LIBRARY_NAME}-tests)

    target_link_libraries(${LIBRARY_NAME}-tests
        ${LIBRARY_NAME}
        ${CMAKE_THREAD_LIBS_INIT}
        gmock
        gtest
        gtest_main)
endif (ENABLE_TESTING)

if (ENABLE_BENCHMARKING)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/foreign/modules")
    include(PrepareGoogleBenchmark)

    find_package(Threads)

    prepare_google_benchmarking()

    include_directories(
        ${PROJECT_SOURCE_DIR}/bench)

    add_executable(${LIBRARY_NAME}-benchmarks
        bench/attribute
        bench/clock
        bench/cpp14formatter
        bench/datetime
        bench/formatter/json
        bench/formatter/string
        bench/logger
        bench/main
        bench/queue
        bench/record
        bench/recordbuf
        bench/system/thread)

    enable_google_benchmarking(${LIBRARY_NAME}-benchmarks)

    target_link_libraries(${LIBRARY_NAME}-benchmarks
        ${LIBRARY_NAME}
        benchmark
        ${CMAKE_THREAD_LIBS_INIT})
endif (ENABLE_BENCHMARKING)

function(enable_all_warnings TARGET)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_compile_options(${TARGET} PRIVATE
            -Weverything
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-padded
            -Wno-shadow
            -Wno-weak-vtables
            -pedantic
            -pedantic-errors)
    endif ()
endfunction()

if (ENABLE_EXAMPLES)
    # Configuration using manual builder.
    add_executable(2-simple
        examples/2.simple)

    enable_all_warnings(2-simple)

    target_link_libraries(2-simple
            blackhole)

    # Configuration using JSON config file.
    add_executable(3-config
        examples/3.config)

    enable_all_warnings(3-config)

    target_link_libraries(3-config
        blackhole)

    # Configuration using JSON config file with facade.
    add_executable(4-config_facade
        examples/4.config_facade)

    enable_all_warnings(4-config_facade)

    target_link_libraries(4-config_facade
        blackhole)

    file(COPY examples/3.config.json DESTINATION .)
endif (ENABLE_EXAMPLES)

install(
    TARGETS
        blackhole
    LIBRARY DESTINATION lib COMPONENT runtime
        ARCHIVE DESTINATION lib COMPONENT development)

install(
    DIRECTORY
        include/
    DESTINATION include
    COMPONENT development
    PATTERN "detail" EXCLUDE)
