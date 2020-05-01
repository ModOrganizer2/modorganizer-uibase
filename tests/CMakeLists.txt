cmake_minimum_required(VERSION 3.16)

include(../../cmake_common/src.cmake)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
requires_library(gtest)