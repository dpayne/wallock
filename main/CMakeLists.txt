# --- Import dependencies ----

# ---- Create standalone executable ----

file(GLOB main_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB main_headers CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)

add_executable(${PROJECT_MAIN_NAME} ${main_headers} ${main_sources})

set_target_properties(${PROJEC_MAIN_NAME} PROPERTIES CXX_STANDARD ${PROJECT_CXX_STD_VERSION})
add_build_flags(${PROJECT_MAIN_NAME})

target_link_libraries(${PROJECT_MAIN_NAME} PRIVATE spdlog)
target_link_libraries(${PROJECT_MAIN_NAME} PUBLIC ${PROJECT_LIB_NAME} dl)

add_custom_target(
  ${PROJECT_NAME}_link ALL
  COMMAND ln -sf "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_MAIN_NAME}"
          "${PROJECT_SOURCE_DIR}/${PROJECT_MAIN_NAME}"
  DEPENDS ${PROJECT_MAIN_NAME}
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMENT "Linking executable shortcut ${CMAKE_CURRENT_BINARY_DIR}"
)

install(FILES "${PROJECT_SOURCE_DIR}/pam/${PROJECT_MAIN_NAME}" DESTINATION "/etc/pam.d/")
install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_MAIN_NAME}"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/"
)
