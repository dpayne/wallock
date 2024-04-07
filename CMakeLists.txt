# this script adds all subprojects to a single build to allow IDEs understand the full project
# structure.
set(ROOT_PROJECT_NAME "wallock")
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# set module path
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules;${CMAKE_MODULE_PATH}")

include(cmake/Options.cmake)
# ---- Project ----

# Note: update this to your new project's name and version
project(
  ${ROOT_PROJECT_NAME}
  VERSION 1.0
  LANGUAGES C CXX
)

set(PROJECT_CXX_STD_VERSION 23)
set(PROJECT_MAIN_NAME ${ROOT_PROJECT_NAME})
set(PROJECT_LIB_NAME ${ROOT_PROJECT_NAME}_lib)
set(PROJECT_TEST_NAME ${ROOT_PROJECT_NAME}_test)

# ---- General Compile Time Flags ----
include(versions.cmake)
include(cmake/BuildFlags.cmake)

# ---- Add dependencies via CPM ----

include(cmake/CPM.cmake)
include(cmake/tools.cmake)
include(cmake/ProjectLibraries.cmake)

include(cmake/WaylandGen.cmake)

# My default build all
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src ${CMAKE_BINARY_DIR}/src)
if(${ENABLE_MAIN})
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/main ${CMAKE_BINARY_DIR}/main)
endif()

if(${ENABLE_TEST})
  # needed to generate test target
  enable_testing()
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/test ${CMAKE_BINARY_DIR}/test)
endif()

if(${ENABLE_TEST_COVERAGE_REPORT})
  include(cmake/CodeCoverageReport.cmake)
endif()
