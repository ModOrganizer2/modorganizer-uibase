cmake_minimum_required(VERSION 3.16)

project(uibase)
set(project_type dll)
set(create_translations OFF)

include(../cmake_common/project.cmake)
add_subdirectory(src)
