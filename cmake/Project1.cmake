# Project 1 integration gates. Keep later projects out of this assignment's checks.
add_custom_target(build-p1-tests DEPENDS ${BUSTUB_P1_TEST_TARGETS})

add_custom_target(check-p1
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --no-tests=error -L "^p1$"
        DEPENDS build-p1-tests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        USES_TERMINAL
        VERBATIM
        )

set(BUSTUB_P1_CHECK_FILES ${P1_FILES} ${BUSTUB_P1_TEST_SOURCES})
add_custom_target(check-p1-format
        COMMAND ${CLANG_FORMAT_BIN} --dry-run --Werror ${BUSTUB_P1_CHECK_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        VERBATIM
        )

add_custom_target(check-p1-lint
        COMMAND ${Python3_EXECUTABLE} ${CPPLINT_BIN}
        --verbose=2 --quiet --linelength=120
        --filter=-legal/copyright,-build/header_guard,-runtime/references
        ${BUSTUB_P1_CHECK_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        VERBATIM
        )
