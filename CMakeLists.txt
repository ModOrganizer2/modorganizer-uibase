cmake_minimum_required(VERSION 3.16)

include(CMakePackageConfigHelpers)

project(uibase)

find_package(mo2-cmake CONFIG REQUIRED)

add_subdirectory(src)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/mo2-uibase-config.cmake"
  INSTALL_DESTINATION "lib/cmake/mo2-uibase"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/src/version.rc" uibase_version)
string(REGEX MATCH "#define VER_FILEVERSION\\s*([0-9]+),([0-9]+),([0-9]+)" _ ${uibase_version})
set(uibase_version_major ${CMAKE_MATCH_1})
set(uibase_version_minor ${CMAKE_MATCH_2})
set(uibase_version_patch ${CMAKE_MATCH_3})

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/mo2-uibase-config-version.cmake"
  VERSION "${uibase_version_major}.${uibase_version_minor}.${uibase_version_patch}"
  COMPATIBILITY AnyNewerVersion
  ARCH_INDEPENDENT
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/mo2-uibase-config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/mo2-uibase-config-version.cmake
  DESTINATION lib/cmake/mo2-uibase
)


set(BUILD_TESTING ${BUILD_TESTING} CACHE BOOL "build tests for uibase")
if (BUILD_TESTING)
	enable_testing()
	add_subdirectory(tests)
endif()
