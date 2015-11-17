cmake_minimum_required(VERSION 2.6)

# Choose new behaviour for CMP0042.
# See http://www.cmake.org/cmake/help/v3.0/policy/CMP0042.html for more details.
if (POLICY CMP0042)
	cmake_policy(SET CMP0042 NEW)
endif (POLICY CMP0042)

OPTION(ENABLE_TESTING "Build the library with tests" OFF)
OPTION(ENABLE_BENCHMARKING "Build the library with benchmarks" OFF)
OPTION(ENABLE_TESTING_THREADSAFETY "Build the thread-safety testing suite" OFF)

set(LIBRARY_NAME blackhole)

project(${LIBRARY_NAME})

find_package(Boost 1.46 REQUIRED COMPONENTS
	system
	thread)

include_directories(BEFORE SYSTEM
	${PROJECT_SOURCE_DIR}/foreign)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(cppformat STATIC foreign/cppformat/format)

add_library(${LIBRARY_NAME} SHARED
	src/attribute
	src/formatter/string/error
	# src/formatter/string/parser
	src/logger
	src/record
	src/root
	src/scoped
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
		COMPILE_FLAGS "-pedantic-errors")
endif ()

target_link_libraries(${LIBRARY_NAME}
	${Boost_LIBRARIES}
	cppformat
)

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

	include_directories(${PROJECT_SOURCE_DIR}/tests/include)

    add_executable(${LIBRARY_NAME}-tests
		tests/attribute
		tests/facade
		tests/formatter/parser
		tests/formatter/string
		tests/record
		tests/src/mocks/handler
		tests/src/mocks/logger
		tests/wrapper
        tests/root)

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

	set(CMAKE_BUILD_TYPE Release)

	prepare_google_benchmarking()

	add_executable(${LIBRARY_NAME}-benchmarks
		bench/cpp14formatter
		bench/logger
		bench/clock
		bench/main
		bench/record
	)

	enable_google_benchmarking(${LIBRARY_NAME}-benchmarks)

	target_link_libraries(${LIBRARY_NAME}-benchmarks
		${LIBRARY_NAME}
        benchmark
		${CMAKE_THREAD_LIBS_INIT}
	)
endif (ENABLE_BENCHMARKING)
