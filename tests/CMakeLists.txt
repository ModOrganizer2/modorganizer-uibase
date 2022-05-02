cmake_minimum_required(VERSION 3.16)

add_executable(uibase_tests)
mo2_configure_tests(uibase_tests
    WARNINGS OFF DEPENDS uibase)
