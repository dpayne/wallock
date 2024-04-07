# ---- Dependencies ----
CPMAddPackage(
  NAME googletest
  GITHUB_REPOSITORY google/googletest
  VERSION ${gtest_version}
  OPTIONS "INSTALL_GTEST OFF" "gtest_force_shared_crt"
)

# ---- Create binary ----

file(GLOB_RECURSE test_headers CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
file(GLOB_RECURSE test_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_TEST_NAME} ${test_headers} ${test_sources})
target_link_libraries(${PROJECT_TEST_NAME} PRIVATE spdlog wayland-gen-protocols)
target_link_libraries(${PROJECT_TEST_NAME} PUBLIC gtest gtest_main gmock dl ${PROJECT_LIB_NAME})
target_include_directories(
  ${PROJECT_TEST_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/../src
                               $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated>
)
set_target_properties(${PROJECT_TEST_NAME} PROPERTIES CXX_STANDARD ${PROJECT_CXX_STD_VERSION})

add_build_flags(${PROJECT_TEST_NAME})

# ---- Add ${PROJECT_TEST_NAME}Tests ----

enable_testing()

# Note: doctest and similar testing frameworks can automatically configure CMake tests. For other
# testing frameworks add the tests target instead: add_test(NAME projectNameTests COMMAND
# ${PROJECT_TEST_NAME})

include(GoogleTest)
gtest_discover_tests(${PROJECT_TEST_NAME} DISCOVERY_TIMEOUT 300)

add_custom_target(
  ${PROJECT_TEST_NAME}_link ALL
  COMMAND ln -sf "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_TEST_NAME}"
  DEPENDS ${PROJECT_TEST_NAME}
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMENT "Linking executable shortcut ${CMAKE_CURRENT_BINARY_DIR}"
)
