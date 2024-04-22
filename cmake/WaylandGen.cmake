find_package(PkgConfig REQUIRED)

# Find wayland-scanner
find_program(WAYLAND_SCANNER NAMES wayland-scanner)
message(STATUS "WAYLAND_SCANNER: ${WAYLAND_SCANNER}")

# Find wayland-protocols
pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols)
if(WAYLAND_PROTOCOLS_FOUND)
  pkg_get_variable(WAYLAND_PROTOCOLS_DIR wayland-protocols pkgdatadir)
else()
  message(FATAL_ERROR "wayland-protocols not found")
endif()

find_path(WAYLAND_INCLUDE_DIR wayland-client.h PATHS ${WAYLAND_INCLUDEDIR})
pkg_check_modules(
  wayland-egl
  REQUIRED
  IMPORTED_TARGET
  wayland-client
  wayland-egl
  wayland-cursor
  egl
  gl
)

set(WAYLAND_PROTOCOLS_FILES
    ${WAYLAND_PROTOCOLS_DIR}/staging/ext-session-lock/ext-session-lock-v1.xml
    ${WAYLAND_PROTOCOLS_DIR}/staging/fractional-scale/fractional-scale-v1.xml
    ${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml
    ${WAYLAND_PROTOCOLS_DIR}/stable/viewporter/viewporter.xml
    ${PROJECT_SOURCE_DIR}/wayland-protocols/wlr-layer-shell-unstable-v1.xml
)

set(PROTOCOL_HEADERS "")
set(PROTOCOL_SOURCES "")

function(generate_wayland_protocol protocol xml_file)
  set(protocol_xml ${xml_file})
  set(protocol_h ${PROJECT_BINARY_DIR}/generated/${protocol}-protocol.h)
  set(protocol_c ${PROJECT_BINARY_DIR}/generated/${protocol}-protocol.c)
  add_custom_command(
    OUTPUT "${protocol_h}" "${protocol_c}"
    COMMAND mkdir -p "${PROJECT_BINARY_DIR}/generated"
    COMMAND "${WAYLAND_SCANNER}" client-header "${protocol_xml}" "${protocol_h}"
    COMMAND "${WAYLAND_SCANNER}" private-code "${protocol_xml}" "${protocol_c}"
    DEPENDS "${WAYLAND_SCANNER}" "${protocol_xml}"
    COMMENT "Generating ${protocol} C wrappers"
  )
  set(PROTOCOL_HEADERS
      ${PROTOCOL_HEADERS} ${protocol_h}
      PARENT_SCOPE
  )
  set(PROTOCOL_SOURCES
      ${PROTOCOL_SOURCES} ${protocol_c}
      PARENT_SCOPE
  )
endfunction()

foreach(protocol_xml ${WAYLAND_PROTOCOLS_FILES})
  message(STATUS "Generating wayland protocol: ${protocol_xml}")
  get_filename_component(protocol_name ${protocol_xml} NAME_WE)
  generate_wayland_protocol(${protocol_name} ${protocol_xml})
endforeach()

message(STATUS "Generated wayland protocol: ${PROTOCOL_HEADERS} ${PROTOCOL_SOURCES}")

set(wayland_generated_sources ${PROTOCOL_HEADERS} ${PROTOCOL_SOURCES})
add_custom_target(generate-wayland-extra-protocols DEPENDS ${WAYLAND_PROTOCOLS_FILES})

add_library(wayland-gen-protocols ${wayland_generated_sources})
set_target_properties(wayland-gen-protocols PROPERTIES LINKER_LANGUAGE C)
target_include_directories(
  wayland-gen-protocols PRIVATE ${WAYLAND_INCLUDE_DIR} ${WAYLAND_EXTRA_PROTOCOL_GENERATED_DIR}
)
add_dependencies(wayland-gen-protocols generate-wayland-extra-protocols)
message(STATUS "WAYLAND_LIBRARIES: ${WAYLAND}")
set(PROJECT_LIBRARIES ${PROJECT_LIBRARIES} wayland-gen-protocols PkgConfig::wayland-egl)
