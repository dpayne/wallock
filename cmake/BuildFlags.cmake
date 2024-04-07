if(NOT DEFINED BUILD_FLAGS OR BUILD_FLAGS STREQUAL "")
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(BUILD_FLAGS "-O2")
  else()
    set(BUILD_FLAGS "-ggdb -g3 -O0")
  endif()
endif()

if(ENABLE_TEST_COVERAGE)
  set(BUILD_FLAGS "${BUILD_FLAGS} -O0 -g -fprofile-arcs -ftest-coverage")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${BUILD_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${BUILD_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${BUILD_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${BUILD_FLAGS}")

set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} ${BUILD_FLAGS}")
set(CMAKE_LD_FLAGS_DEBUG "${CMAKE_LD_FLAGS_DEBUG} ${BUILD_FLAGS}")
set(CMAKE_LD_FLAGS_RELEASE "${CMAKE_LD_FLAGS_RELEASE} ${BUILD_FLAGS}")

function(add_build_flags target_name)
  # being a cross-platform target, we enforce standards conformance on MSVC
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
      target_compile_options(
        ${target_name}
        PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->"
               -fno-strict-aliasing
               -DNDEBUG
               -Wall
               -O2
               -Wno-variadic-macros
               -fdiagnostics-color=always
      )
    else()
      target_compile_options(
        ${target_name} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->"
      )
    endif()
    target_compile_definitions(${target_name} PUBLIC NDEBUG)
    target_compile_definitions(${target_name} PRIVATE SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)
  else()

    if(ENABLE_TEST_COVERAGE)
      target_compile_options(${target_name} PRIVATE -fprofile-arcs -ftest-coverage)
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
      target_compile_options(
        ${target_name}
        PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->"
               -DDEBUG
               -fdiagnostics-color=always
               -fno-strict-aliasing
               -Wall
               -Warray-bounds
               -Wdate-time
               -Wdisabled-optimization
               -Werror
               -Wextra
               -Wformat=2
               -Wformat-security
               -Wimplicit-fallthrough
               -Winvalid-pch
               -Wmissing-declarations
               -Wnull-dereference
               -Woverloaded-virtual
               -Wpacked
               -Wshadow
               -Wshift-overflow
               -Wunknown-pragmas
               -Wunused-parameter
               -Wvla
      )
    else()
      target_compile_options(
        ${target_name} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->"
      )
    endif()
    target_compile_definitions(${target_name} PUBLIC DEBUG)
    target_compile_definitions(${target_name} PRIVATE SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
  endif()
endfunction()
