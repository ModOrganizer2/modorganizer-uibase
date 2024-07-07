cmake_minimum_required(VERSION 3.16)

project(uibase)

find_package(mo2-cmake CONFIG REQUIRED)

add_subdirectory(src)

set(BUILD_TESTING ${BUILD_TESTING} CACHE BOOL "build tests for uibase")
if (BUILD_TESTING)
	enable_testing()
	add_subdirectory(tests)
endif()
