include(Compiler)
include(ExternalProject)

# Builds at least on GCC 4.8 and Apple clang-703.0.31.
function(download_google_testing)
    set(GOOGLETEST_GIT_REPO "https://github.com/abseil/googletest.git")
    set(GOOGLETEST_GIT_TAG 8fbf9d16a63a8b0cd629b24089f9470deef75120)

    # Using a specific git tag violates the Abseil/Google Test Live At Head philosophy.
    # https://abseil.io/about/philosophy
    # But in case the blackhole tests fail to build, uncommenting the GIT_TAG line will
    # use a Google Test version, which was working.

    set_directory_properties(properties EP_PREFIX "${CMAKE_BINARY_DIR}/foreign")
    ExternalProject_ADD(googlemock
        GIT_REPOSITORY ${GOOGLETEST_GIT_REPO}
#        GIT_TAG  ${GOOGLETEST_GIT_TAG}
        SOURCE_DIR "${CMAKE_BINARY_DIR}/foreign/googlemock"
        CMAKE_ARGS "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}" "-DCMAKE_CXX_FLAGS=-fPIC"
        INSTALL_COMMAND "")
    ExternalProject_GET_PROPERTY(googlemock SOURCE_DIR)
    ExternalProject_GET_PROPERTY(googlemock BINARY_DIR)

    set(GOOGLETEST_INCLUDE_DIR ${SOURCE_DIR}/googletest/include PARENT_SCOPE)
    set(GOOGLEMOCK_INCLUDE_DIR ${SOURCE_DIR}/googlemock/include PARENT_SCOPE)
    set(GOOGLEMOCK_BINARY_DIR ${BINARY_DIR}/lib PARENT_SCOPE)
endfunction()

function(prepare_google_testing)
    download_google_testing()

    include_directories(SYSTEM PARENT_SCOPE
        ${GOOGLETEST_INCLUDE_DIR}
        ${GOOGLEMOCK_INCLUDE_DIR})
    link_directories(${GOOGLEMOCK_BINARY_DIR})
endfunction()
