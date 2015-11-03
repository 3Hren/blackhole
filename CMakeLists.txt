cmake_minimum_required(VERSION 2.6)

# Choose new behaviour for CMP0042.
# See http://www.cmake.org/cmake/help/v3.0/policy/CMP0042.html for more details.
if (POLICY CMP0042)
	cmake_policy(SET CMP0042 NEW)
endif (POLICY CMP0042)

OPTION(ENABLE_TESTING "Build the library with tests" OFF)
OPTION(ENABLE_BENCHMARKING "Build the library with benchmarks" OFF)

set(LIBRARY_NAME blackhole)

project(${LIBRARY_NAME})

include_directories(BEFORE SYSTEM
	${PROJECT_SOURCE_DIR}/foreign)

include_directories(${PROJECT_SOURCE_DIR}/include)

# add_library(cppformat STATIC foreign/cppformat/format)

add_library(${LIBRARY_NAME} SHARED
	src/attribute
	src/logger
)

add_definitions(-std=c++14)

target_compile_options(${LIBRARY_NAME} PRIVATE
	-Weverything
	-Wno-padded
	-Wno-shadow
	-Wno-c++98-compat
	-Wno-weak-vtables
	-pedantic
	-pedantic-errors)

# target_link_libraries(
# 	cppformat
# )

if (ENABLE_TESTING)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/foreign/modules")
    include(DownloadGoogleTesting)

	find_package(Threads)

	prepare_google_testing()

    add_executable(${LIBRARY_NAME}-tests
        tests/logger
    )

	enable_google_testing(${LIBRARY_NAME}-tests)

	target_link_libraries(${LIBRARY_NAME}-tests
		${LIBRARY_NAME}
		${CMAKE_THREAD_LIBS_INIT}
		gmock
		gtest
		gtest_main
	)
endif (ENABLE_TESTING)

if (ENABLE_BENCHMARKING)
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/foreign/modules")
	include(PrepareGoogleBenchmark)

	set(CMAKE_BUILD_TYPE Release)

	prepare_google_benchmarking()

	add_executable(${LIBRARY_NAME}-benchmarks
		bench/cpp14formatter
		bench/logger
		bench/main
	)

	enable_google_benchmarking(${LIBRARY_NAME}-benchmarks)

	target_link_libraries(${LIBRARY_NAME}-benchmarks
        ${LIBRARY_NAME}
        benchmark
	)
endif (ENABLE_BENCHMARKING)
