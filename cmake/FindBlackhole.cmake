#  Try to find Blackhole
#  Once done, this will define
#
#  Blackhole_FOUND - system has Blackhole
#  Blackhole_INCLUDE_DIRS - the Blackhole include directories

find_package(PkgConfig)

find_path(BLACKHOLE_INCLUDE_DIR
    NAMES blackhole/blackhole.hpp
    HINTS ${PC_BLACKHOLE_INCLUDEDIR}
          ${PC_BLACKHOLE_INCLUDE_DIRS}
    PATH_SUFFIXES blackhole
)

# Currently Blackhole is header-only library
# find_library(BLACKHOLE_LIBRARIES
#     NAMES blackhole
#     HINTS ${PC_BLACKHOLE_LIBDIR}
#           ${PC_BLACKHOLE_LIBRARY_DIRS}
# )

set(BLACKHOLE_INCLUDE_DIRS ${BLACKHOLE_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
# Handle the QUIETLY and REQUIRED arguments and set BLACKHOLE_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LibBlackhole  DEFAULT_MSG
                                  BLACKHOLE_INCLUDE_DIR)

mark_as_advanced(BLACKHOLE_INCLUDE_DIR)
