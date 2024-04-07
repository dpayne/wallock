# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

# ---- Add source files ----

# Note: globbing sources is considered bad practice as CMake's generators may not detect new files
# automatically. Keep that in mind when changing files, or explicitly mention them here.
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../include/*.hpp")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# ---- Create library ----

# Note: for header-only libraries change all PUBLIC flags to INTERFACE and create an interface
# target: add_library(${ROOT_PROJECT_NAME} INTERFACE)
add_library(${PROJECT_LIB_NAME} ${headers} ${sources})

if(ENABLE_IWYU)
  find_program(iwyu_path NAMES include-what-you-use iwyu REQUIRED)
  set(iwyu_path_and_options ${iwyu_path} -Xiwyu --no_comments)

  set_property(
    TARGET ${PROJECT_LIB_NAME} PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path_and_options}
  )
endif()

set_target_properties(${PROJECT_LIB_NAME} PROPERTIES CXX_STANDARD ${PROJECT_CXX_STD_VERSION})

add_build_flags(${PROJECT_LIB_NAME})

# ------ Link dependencies --------

add_dependencies(${PROJECT_LIB_NAME} wayland-gen-protocols)
target_link_libraries(${PROJECT_LIB_NAME} PRIVATE spdlog wayland-gen-protocols)
target_link_libraries(${PROJECT_LIB_NAME} PUBLIC ${PROJECT_LIBRARIES})

target_include_directories(
  ${PROJECT_LIB_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                             $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

target_include_directories(
  ${PROJECT_LIB_NAME} PRIVATE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated>
                              $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
)
