add_custom_target(
  ${PROJECT_NAME}_coverage_report ALL
  COMMAND lcov --capture --directory . --output-file coverage.info --ignore-errors gcov,gcov
          --ignore-errors mismatch
  COMMAND lcov --remove coverage.info '/usr/*' --output-file coverage.info --ignore-errors unused
  COMMAND lcov --remove coverage.info "${CMAKE_BINARY_DIR}" --output-file coverage.info
          --ignore-errors unused
  COMMAND lcov --remove coverage.info "*/custom_libs/*" --output-file coverage.info --ignore-errors
          unused
  COMMAND lcov --remove coverage.info "${PROJECT_SOURCE_DIR}/test" --output-file coverage.info
          --ignore-errors unused
  COMMAND lcov --remove coverage.info "${PROJECT_SOURCE_DIR}/main" --output-file coverage.info
          --ignore-errors unused
  COMMAND genhtml coverage.info --output-directory coverage_report
  DEPENDS ${PROJECT_TEST_NAME}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Generating coverage report"
)
