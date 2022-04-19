cmake_minimum_required(VERSION 3.16)

project(uibasetests)

find_package(GTest REQUIRED)

add_executable(uibasetests)
mo2_configure_tests(uibasetests TRANSLATIONS OFF)
