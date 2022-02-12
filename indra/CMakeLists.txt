# -*- cmake -*-

# cmake_minimum_required should appear before any
# other commands to guarantee full compatibility
# with the version specified
## 3.8 added VS_DEBUGGER_WORKING_DIRECTORY support
cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)

set(ROOT_PROJECT_NAME "SecondLife" CACHE STRING
    "The root project/makefile/solution name. Defaults to SecondLife.")
project(${ROOT_PROJECT_NAME})

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(Variables)
include(BuildVersion)

set(LEGACY_STDIO_LIBS)
if (WINDOWS)
      set(LEGACY_STDIO_LIBS legacy_stdio_definitions)
endif (WINDOWS)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Build type.  One of: Debug Release RelWithDebInfo" FORCE)
endif (NOT CMAKE_BUILD_TYPE)

#<FS:AW optional opensim support>
option(OPENSIM "OpenSim support" OFF)
if (OPENSIM)
  add_definitions(-DOPENSIM=1)
  if (SINGLEGRID)
    add_definitions(-DSINGLEGRID=1 -DSINGLEGRID_URI=\"${SINGLEGRID_URI}\")
    message(STATUS "Compiling with OpenSim support - Single Grid version (${SINGLEGRID_URI})")
  else (SINGLEGRID)
    message(STATUS "Compiling with OpenSim support")
  endif (SINGLEGRID)
else (OPENSIM)
  if (SINGLEGRID)
    message(WARNING "Value for SINGLEGRID is unused during Havok/SecondLife builds" )
  endif()
  message(STATUS "Compiling without OpenSim support")
endif (OPENSIM)

if (HAVOK_TPV)
  add_definitions(-DHAVOK_TPV=1)
  message(STATUS "Compiling with Havok libraries")
endif (HAVOK_TPV)
#</FS:AW optional opensim support>

#<FS:Ansariel> Support for test builds
option(TESTBUILD "Generating test build" OFF)
if(TESTBUILD AND TESTBUILDPERIOD)
  add_definitions(-DTESTBUILD=1 -DTESTBUILDPERIOD=${TESTBUILDPERIOD})
  message(STATUS "Creating test build version; test period: ${TESTBUILDPERIOD} days")
endif(TESTBUILD AND TESTBUILDPERIOD)
#</FS:Ansariel>

# <FS:Ansariel> [AVX Optimization]
option(USE_AVX_OPTIMIZATION "AVX optimization support" OFF)
option(USE_AVX2_OPTIMIZATION "AVX2 optimization support" OFF)
if (USE_AVX_OPTIMIZATION)
  if (USE_AVX2_OPTIMIZATION)
    message(FATAL_ERROR "You cannot use AVX and AVX2 at the same time!")
  else (USE_AVX2_OPTIMIZATION)
    add_definitions(-DUSE_AVX_OPTIMIZATION=1)
    message(STATUS "Compiling with AVX optimizations")
  endif (USE_AVX2_OPTIMIZATION)
elseif (USE_AVX2_OPTIMIZATION)
  add_definitions(-DUSE_AVX2_OPTIMIZATION=1)
  message(STATUS "Compiling with AVX2 optimizations")
else (USE_AVX_OPTIMIZATION)
  message(STATUS "Compiling without AVX optimizations")
endif (USE_AVX_OPTIMIZATION)
# </FS:Ansariel> [AVX Optimization]

add_subdirectory(cmake)

# <FS:Beq> Tracy Profiler support
option(USE_TRACY_PROFILER "Tracy Profiler support" OFF)
if (USE_TRACY_PROFILER)
  message(STATUS "Compiling with Tracy profiler")
else (USE_TRACY_PROFILER)
  message(STATUS "Compiling without Tracy profiler")
endif (USE_TRACY_PROFILER)
# </FS:Beq> Tracy Profiler support

add_subdirectory(${LIBS_OPEN_PREFIX}llcommon)
add_subdirectory(${LIBS_OPEN_PREFIX}llcorehttp)
add_subdirectory(${LIBS_OPEN_PREFIX}llinventory)
add_subdirectory(${LIBS_OPEN_PREFIX}llmath)
add_subdirectory(${LIBS_OPEN_PREFIX}llmessage)
add_subdirectory(${LIBS_OPEN_PREFIX}llprimitive)
add_subdirectory(${LIBS_OPEN_PREFIX}llfilesystem)
add_subdirectory(${LIBS_OPEN_PREFIX}llxml)

if (WINDOWS AND EXISTS ${LIBS_CLOSED_DIR}copy_win_scripts)
  add_subdirectory(${LIBS_CLOSED_PREFIX}copy_win_scripts)
endif (WINDOWS AND EXISTS ${LIBS_CLOSED_DIR}copy_win_scripts)

add_subdirectory(${LIBS_OPEN_PREFIX}lscript)

if (LL_TESTS)
  # Define after the custom targets are created so
  # individual apps can add themselves as dependencies
  add_subdirectory(${INTEGRATION_TESTS_PREFIX}integration_tests)
endif (LL_TESTS)

