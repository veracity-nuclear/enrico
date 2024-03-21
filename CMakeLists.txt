cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0074 OLD)
project(enrico Fortran C CXX)

#===============================================================================
# RPATH and install information
#===============================================================================

include(GNUInstallDirs)

# NekRS must be installed (via `make install`) to have all headers/libs available.
# For ENRICO, we'll install in the build directory by default
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "" FORCE)

    # NekRS also does this same trick to set the prefix to .local/nekrs; prevent
    # that from happening
    set(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT FALSE)
endif ()

# This block of code ensures that dynamic libraries can be found via the RPATH
# whether the executable is the original one from the build directory or the
# installed one in CMAKE_INSTALL_PREFIX. Ref:
# https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/RPATH-handling

# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)


# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(CMAKE_INSTALL_RPATH
  "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}:${CMAKE_INSTALL_PREFIX}/lib")

# =============================================================================
# Check for SCALE (optional)
# =============================================================================
if (Trilinos_DIR)
  find_package(Trilinos PATHS ${Trilinos_DIR} COMPONENTS Tpetra)
else()
  find_package(Trilinos PATHS ${SCALE_DIR} COMPONENTS Tpetra)
endif ()

if (Trilinos_FOUND)
  MESSAGE(STATUS "Trilinos Found!")
  MESSAGE(STATUS "Trilinos_DIR = ${Trilinos_DIR}")
endif ()

find_package(SCALE PATHS ${SCALE_DIR} COMPONENTS nemesis ampx geometria omnibus shift)

if (SCALE_FOUND)
    MESSAGE(STATUS "SCALE found!")
    MESSAGE(STATUS "SCALE_DIR = ${SCALE_DIR}")
    MESSAGE(STATUS "SCALE_VERSION = ${SCALE_VERSION}")
endif ()

# =============================================================================
# set pugixml library as a variable
# =============================================================================

if (TARGET pugixml::pugixml)
    set(LIBPUGIXML pugixml::pugixml)
else ()
    set(LIBPUGIXML pugixml)
endif ()

# =============================================================================
# Headers for all targets
# =============================================================================
if (Trilinos_FOUND)
    include_directories(${Trilinos_INCLUDE_DIRS})
    include_directories(${Trilinos_TPL_INCLUDE_DIRS})
endif ()

if (SCALE_FOUND)
    include_directories(${SCALE_INCLUDE_DIRS})
    include_directories(${SCALE_TPL_INCLUDE_DIRS})
endif ()

# =============================================================================
# Build enrico driver
# =============================================================================
add_executable(enrico src/main.cpp)
target_link_libraries(enrico PUBLIC ${LIBPUGIXML}
                                    ${Trilinos_LIBRARIES}
                                    ${Trilinos_TPL_LIBRARIES}
                                    ${SCALE_LIBRARIES}
                                    ${SCALE_TPL_LIBRARIES})

# Ensure C++17 standard is used
set_target_properties(
        enrico
        PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF)


#################################################################################
# Install targets
#################################################################################

set(INSTALL_TARGETS
  enrico)

install(TARGETS ${INSTALL_TARGETS}
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
)
