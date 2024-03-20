
# Copyright 2020 UT-Battelle, LLC and SCALE developers.
# See the top-level COPYRIGHT file for details.
#-----------------------------------------------------------------------------#

# Helper scripts, see CMakePackageConfigHelpers
set(SCALE_VERSION 7.0.0)

####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was SCALEConfig.modern.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

#-----------------------------------------------------------------------------#
# Helper scripts
#-----------------------------------------------------------------------------#

get_filename_component(SCALE_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)
list(APPEND CMAKE_MODULE_PATH
  "${SCALE_CMAKE_DIR}"
)
if(CMAKE_VERSION VERSION_LESS 3.18)
  list(APPEND CMAKE_MODULE_PATH
    "${SCALE_CMAKE_DIR}/backport/3.18"
  )
endif()
if(CMAKE_VERSION VERSION_LESS 3.19)
  list(APPEND CMAKE_MODULE_PATH
    "${SCALE_CMAKE_DIR}/backport/3.19"
  )
endif()

#-----------------------------------------------------------------------------#
# Variables
#-----------------------------------------------------------------------------#

set(SCALE_BINARY_DIR "/home/aarog/code_installations/scale_master/build")
set(SCALE_BUILD_EXAMPLES "OFF")
set(SCALE_BUILD_FULCRUM_HELP "OFF")
set(SCALE_BUILD_MADRE "OFF")
set(SCALE_BUILD_OMNIBUS_DOCS "OFF")
set(SCALE_BUILD_PACKAGE "OFF")
set(SCALE_BUILD_REGRESSION "OFF")
set(SCALE_BUILD_RELEASE_INFO "OFF")
set(SCALE_BUILD_TESTS "ON")
set(SCALE_BUILTIN_Celeritas "ON")
set(SCALE_BUILTIN_Flibcpp "ON")
set(SCALE_BUILTIN_GNDS "ON")
set(SCALE_BUILTIN_GTest "ON")
set(SCALE_BUILTIN_gcem "ON")
set(SCALE_BUILTIN_nlohmann_json "ON")
set(SCALE_BUILTIN_pugixml "ON")
set(SCALE_BUILTIN_statslib "ON")
set(SCALE_BUILTIN_superlu "ON")
set(SCALE_CEDATA_PRECISION "mixed")
set(SCALE_CXX_WARNING_FLAGS "-Wall
-pedantic
-Wno-unused-parameter
-Wno-unused-variable
-Wno-unused-local-typedefs")
set(SCALE_C_WARNING_FLAGS "-Wall
-pedantic")
set(SCALE_DATA_AUTO "OFF")
set(SCALE_DATA_DIR "/home/aarog/code_installations/scale_master/scale_data")
set(SCALE_DEBUG "ON")
set(SCALE_DEBUG_OUTPUT "OFF")
set(SCALE_ENV_Fortran "GFORTRAN_CONVERT_UNIT=big_endian:21-26,33,77-80,88,89")
set(SCALE_Fortran_WARNING_FLAGS "-Wall
-pedantic
-Wno-unused-dummy-argument
-Wno-unused-value
-Wno-unused-variable
-Wno-unused-function
-Wno-tabs
-Wno-conversion")
set(SCALE_GIT_SUBMODULE "OFF")
set(SCALE_HPC_DATA_DIR "SCALE_HPC_DATA_DIR-NOTFOUND")
set(SCALE_IS_TOP_LEVEL "ON")
set(SCALE_NOWARN_CUDA "OFF")
set(SCALE_NOWARN_DAGMC "OFF")
set(SCALE_NOWARN_Geant4 "OFF")
set(SCALE_NOWARN_HIP "OFF")
set(SCALE_NOWARN_Java "OFF")
set(SCALE_NOWARN_MCNP "OFF")
set(SCALE_NOWARN_Qt "OFF")
set(SCALE_NOWARN_pint "ON")
set(SCALE_PYTHONPATH "/home/aarog/spack/var/spack/environments/scale_7/.spack-env/view/lib/python3.11/site-packages")
set(SCALE_Python_WARNING_FLAGS "-W
once")
set(SCALE_RELEASE_AGGRESSIVE "FALSE")
set(SCALE_REPOSITORY_BRANCH "master")
set(SCALE_RUN_IMPL_SCRIPT "/home/aarog/code_installations/scale_master/src/cmake/ScaleRunQuietCommand/ScaleRunQuietImpl.cmake")
set(SCALE_SOURCE_DIR "/home/aarog/code_installations/scale_master/src")
set(SCALE_TESTING_TRACK "")
set(SCALE_TRILINOS_UNSUPPORTED "TRUE")
set(SCALE_ULTRALITE "OFF")
set(SCALE_USE_CUDA "OFF")
set(SCALE_USE_DAGMC "OFF")
set(SCALE_USE_FLEX "OFF")
set(SCALE_USE_Fortran "ON")
set(SCALE_USE_Fortran_MPI "TRUE")
set(SCALE_USE_GNDS "ON")
set(SCALE_USE_Geant4 "OFF")
set(SCALE_USE_Git_LFS "TRUE")
set(SCALE_USE_HIP "OFF")
set(SCALE_USE_Java "OFF")
set(SCALE_USE_MCNP "OFF")
set(SCALE_USE_MPI "ON")
set(SCALE_USE_OpenMP "ON")
set(SCALE_USE_Qt "OFF")
set(SCALE_USE_SWIG_Fortran "OFF")
set(SCALE_USE_SWIG_Python "ON")
set(SCALE_USE_Silo "ON")
set(SCALE_USE_SuperLU "ON")
set(SCALE_BUILD_SHARED_LIBS "ON")
set(SCALE_Trilinos_VERSION "14.2")
set(SCALE_INSTALL_GTEST "ON")
set_and_check(HDF5_ROOT "/home/aarog/spack/var/spack/environments/scale_7/.spack-env/view")
set_and_check(MPIEXEC_EXECUTABLE "/home/aarog/spack/var/spack/environments/scale_7/.spack-env/view/bin/mpiexec")
set_and_check(Trilinos_DIR "/home/aarog/spack/var/spack/environments/scale_7/.spack-env/view/lib/cmake/Trilinos")

#-----------------------------------------------------------------------------#
# Dependencies
#-----------------------------------------------------------------------------#

include(CMakeFindDependencyMacro)

enable_language(C)

find_dependency(HDF5 COMPONENTS C HL)
find_dependency(PythonInterp)
if(SCALE_DATA_DIR)
  find_dependency(SCALEData)
endif()

if(SCALE_USE_MPI)
  find_dependency(MPI COMPONENTS C CXX)
endif()

if(NOT DEFINED CMAKE_CXX_EXTENSIONS)
  # Tell Kokkos to STFU
  set(CMAKE_CXX_EXTENSIONS "")
endif()
find_dependency(Trilinos ${SCALE_Trilinos_VERSION})

find_dependency(BLAS)
find_dependency(LAPACK)

if(SCALE_BUILD_TESTS)
  if(SCALE_BUILTIN_GTest)
    # GTest targets should be installed alongside SCALE
    include("${SCALE_CMAKE_DIR}/../GTest/GTestConfig.cmake")
  else()
    if(CMAKE_VERSION VERSION_LESS 3.20)
      # First look for standard CMake installation
      # (automatically done for CMake >= 3.20)
      find_dependency(GTest QUIET NO_MODULE)
    endif()
    if(NOT GTest_FOUND)
      # If not found, try again
      find_dependency(GTest)
    endif()
  endif()
endif()

if(SCALE_USE_Fortran)
  find_dependency(Flibcpp)
endif()

# PugiXML may be installed externally or vendored by SCALE
if(SCALE_BUILTIN_pugixml)
  include("${SCALE_CMAKE_DIR}/../pugixml/pugixml-config.cmake")
else()
  find_dependency(pugixml)
endif()

# Celeritas is used by ORANGE
if(SCALE_BUILTIN_Celeritas)
  include("${SCALE_CMAKE_DIR}/../Celeritas/CeleritasConfig.cmake")
else()
  find_dependency(Celeritas)
endif()

# GNDS is used by AMPX and may be installed externally or vendored by SCALE
if(SCALE_BUILTIN_GNDS)
  include("${SCALE_CMAKE_DIR}/../GNDS/GNDSConfig.cmake")
else()
  find_dependency(GNDS)
endif()

# SuperLU is used by Origen and may be installed externally or vendored by SCALE
if(SCALE_BUILTIN_superlu)
  # SuperLU should be installed alongside SCALE
  include("${SCALE_CMAKE_DIR}/../superlu/superluConfig.cmake")
else()
  find_dependency(superlu)
endif()

# gcem is used by statslib and may be installed externally or vendored by SCALE
if(NOT SCALE_BUILTIN_gcem)
  find_dependency(gcem)
endif()

if(SCALE_USE_MCNP)
  if(SCALE_BUILTIN_Lava)
    # Lava will be installed alongside SCALE
    include("${SCALE_CMAKE_DIR}/../Lava/LavaConfig.cmake")
  else()
    find_dependency(Lava)
  endif()
endif()

#-----------------------------------------------------------------------------#
# Targets
#-----------------------------------------------------------------------------#

if(NOT TARGET SCALE::nemesis)
  include("${SCALE_CMAKE_DIR}/SCALETargets.cmake")
endif()

#-----------------------------------------------------------------------------#

check_required_components(SCALE)
set(SCALE_FOUND TRUE)
foreach(comp ${SCALE_FIND_COMPONENTS})
  list(APPEND SCALE_LIBRARIES ${CMAKE_CURRENT_LIST_DIR}/../../lib${comp}.so)
endforeach()
set(SCALE_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/../../../include)

#-----------------------------------------------------------------------------#
