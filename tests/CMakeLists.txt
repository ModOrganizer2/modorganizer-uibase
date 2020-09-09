cmake_minimum_required(VERSION 3.16)

project(uibasetests)
set(project_type tests)

include(GoogleTest)

if(DEFINED DEPENDENCIES_DIR)
	include(${DEPENDENCIES_DIR}/modorganizer_super/cmake_common/src.cmake)
    include(${DEPENDENCIES_DIR}/modorganizer_super/cmake_common/project.cmake)
else()
	include(../../cmake_common/src.cmake)
    include(../cmake_common/project.cmake)
endif()

requires_project(uibase)