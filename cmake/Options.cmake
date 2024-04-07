# always enable test for debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  set(ENABLE_TEST_DEFAULT ON)
else()
  set(ENABLE_TEST_DEFAULT OFF)
endif()

# options
option(ENABLE_TEST "Enable unit tests" ${ENABLE_TEST_DEFAULT})
option(ENABLE_IWYU "Enable include-what-you-use check" OFF)
option(ENABLE_DOCS "Enable documentation generation" ON)
option(ENABLE_MAIN "Enable main executable" ON)
option(ENABLE_BENCHMARK "Enable benchmark tests" OFF)
option(ENABLE_LINTER "Enable linter" OFF)
option(USE_CCACHE_BY_DEFAULT "Enable ccache" ON)
option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)
option(ENABLE_TEST_COVERAGE_REPORT "Enable test coverage report" OFF)
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

# external dependencies
set(EXTERNAL_PROJECTS_DIR ${CMAKE_BINARY_DIR}/_deps)
set(EXTERNAL_PROJECTS_INCLUDE_DIR ${EXTERNAL_PROJECTS_DIR}/include)
set(EXTERNAL_PROJECTS_LIB_DIR ${EXTERNAL_PROJECTS_DIR}/lib)

if(ENABLE_TEST_COVERAGE_REPORT)
  set(ENABLE_TEST_COVERAGE ON)
endif()

# by default enable clang tidy
if(ENABLE_LINTER)
  set(USE_STATIC_ANALYZER clang-tidy)
endif()

# enable ccache by default if available
if(USE_CCACHE_BY_DEFAULT)
  find_program(CCACHE_FOUND ccache)
  if(CCACHE_FOUND)
    set(USE_CCACHE
        ON
        CACHE BOOL "" FORCE
    )
  endif(CCACHE_FOUND)
endif()
