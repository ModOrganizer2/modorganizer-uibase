cmake_minimum_required(VERSION 3.16)

project(uibase)

if(DEFINED DEPENDENCIES_DIR)
	include(${DEPENDENCIES_DIR}/modorganizer_super/cmake_common/mo2.cmake)
else()
	include(../cmake_common/mo2.cmake)
endif()

add_subdirectory(src)
add_subdirectory(tests)
