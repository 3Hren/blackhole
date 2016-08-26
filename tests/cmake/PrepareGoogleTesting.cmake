include(Compiler)
include(ExternalProject)

# Builds at least on GCC 4.8 and Apple clang-703.0.31.
function(download_google_testing)
    set(GOOGLEMOCK_URL "https://github.com/google/googletest/archive/release-1.8.0.zip")
    set(GOOGLEMOCK_URL_MD5 adfafc8512ab65fd3cf7955ef0100ff5)

    set_directory_properties(properties EP_PREFIX "${CMAKE_BINARY_DIR}/foreign")
    ExternalProject_ADD(googlemock
        URL ${GOOGLEMOCK_URL}
        URL_MD5 ${GOOGLEMOCK_URL_MD5}
        SOURCE_DIR "${CMAKE_BINARY_DIR}/foreign/googlemock"
        CMAKE_ARGS "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}" "-DCMAKE_CXX_FLAGS=-fPIC"
        INSTALL_COMMAND "")
    ExternalProject_GET_PROPERTY(googlemock SOURCE_DIR)
    ExternalProject_GET_PROPERTY(googlemock BINARY_DIR)

    set(GOOGLETEST_INCLUDE_DIR ${SOURCE_DIR}/googletest/include PARENT_SCOPE)
    set(GOOGLEMOCK_INCLUDE_DIR ${SOURCE_DIR}/googlemock/include PARENT_SCOPE)
    set(GOOGLETEST_BINARY_DIR ${BINARY_DIR}/googlemock/gtest PARENT_SCOPE)
    set(GOOGLEMOCK_BINARY_DIR ${BINARY_DIR}/googlemock PARENT_SCOPE)
endfunction()

function(prepare_google_testing)
    download_google_testing()

    include_directories(SYSTEM PARENT_SCOPE
        ${GOOGLETEST_INCLUDE_DIR}
        ${GOOGLEMOCK_INCLUDE_DIR})
    link_directories(${GOOGLETEST_BINARY_DIR} ${GOOGLEMOCK_BINARY_DIR})
endfunction()
