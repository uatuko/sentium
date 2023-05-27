add_library(err INTERFACE)

target_sources(err
	INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/basic_error.h>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/errors.h>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/fixed_string.h>
)

# tests
add_executable(err_tests
	basic_error_test.cpp
)

target_link_libraries(err_tests
	PRIVATE err
	PRIVATE gtest_main
)

include(GoogleTest)
gtest_discover_tests(err_tests)
