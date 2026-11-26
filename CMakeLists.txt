cmake_minimum_required(VERSION 3.14)

if(POLICY CMP0135)
	cmake_policy(SET CMP0135 NEW)
endif()
project(chipstack CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
	ftxui
	URL https://github.com/ArthurSonzogni/FTXUI/archive/refs/tags/v5.0.0.tar.gz
)
FetchContent_MakeAvailable(ftxui)

file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS src/core/*.cpp src/ai/*.cpp)
add_library(chipstack_core ${CORE_SOURCES})
target_include_directories(chipstack_core PUBLIC src)
set(WARNINGS -Wall -Wextra -Wno-unused-parameter)
target_compile_options(chipstack_core PRIVATE ${WARNINGS})

file(GLOB_RECURSE UI_SOURCES CONFIGURE_DEPENDS src/ui/*.cpp)
add_executable(chipstack src/main.cpp ${UI_SOURCES})
target_compile_options(chipstack PRIVATE ${WARNINGS})
target_link_libraries(chipstack
	PRIVATE chipstack_core
	PRIVATE ftxui::screen
	PRIVATE ftxui::dom
	PRIVATE ftxui::component
)

option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
	enable_testing()
	find_package(GTest QUIET)
	if(NOT GTest_FOUND)
		FetchContent_Declare(
			googletest
			URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
		)
		set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
		FetchContent_MakeAvailable(googletest)
	endif()

	file(GLOB TEST_SOURCES CONFIGURE_DEPENDS tests/*.cpp)
	add_executable(chipstack_tests ${TEST_SOURCES})
	target_link_libraries(chipstack_tests
		PRIVATE chipstack_core
		PRIVATE GTest::gtest_main
	)
	include(GoogleTest)
	gtest_discover_tests(chipstack_tests)
endif()
