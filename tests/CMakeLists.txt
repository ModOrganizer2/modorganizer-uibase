cmake_minimum_required(VERSION 3.16)

add_executable(uibase-tests EXCLUDE_FROM_ALL)
mo2_configure_tests(uibase-tests WARNINGS OFF)
target_link_libraries(uibase-tests PRIVATE uibase)
