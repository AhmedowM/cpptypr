cmake_minimum_required(VERSION 3.25)

# ---- Compiler Settings (must be before project() for CMake 3.25+ CXX detection) ----
set(CMAKE_CXX_STANDARD 23 CACHE STRING "")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(cpptypr LANGUAGES C CXX)

# ---- Version ----
execute_process(
    COMMAND git describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(GIT_TAG "${GIT_TAG}" CACHE STRING "Latest git tag for versioning")

if(GIT_TAG MATCHES "^v([0-9]+)\\.([0-9]+)\\.([0-9]+)$")
    set(PROJECT_VERSION_MAJOR "${CMAKE_MATCH_1}")
    set(PROJECT_VERSION_MINOR "${CMAKE_MATCH_2}")
    set(PROJECT_VERSION_PATCH "${CMAKE_MATCH_3}")
    set(PROJECT_VERSION "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")
else()
    set(PROJECT_VERSION_MAJOR "0")
    set(PROJECT_VERSION_MINOR "1")
    set(PROJECT_VERSION_PATCH "0")
    set(PROJECT_VERSION "0.1.0")
endif()

# ---- Version Header ----
set(CPPTYPR_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPPTYPR_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPPTYPR_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPPTYPR_VERSION "${PROJECT_VERSION}")
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpptypr/version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/cpptypr/version.hpp
)

# ---- External Dependencies ----
include(FetchContent)
set(saved_build_testing "${BUILD_TESTING}")
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    ctypr
    GIT_REPOSITORY https://github.com/AhmedowM/ctypr.git
    GIT_TAG v1.2.0
)
FetchContent_MakeAvailable(ctypr)
set(BUILD_TESTING "${saved_build_testing}" CACHE BOOL "" FORCE)

# ---- Compiler Settings ----
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- Headers in Build Tree ----
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/cpptypr.hpp ${CMAKE_CURRENT_BINARY_DIR}/include/cpptypr.hpp COPYONLY)

file(GLOB_RECURSE PUBLIC_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/cpptypr/*.hpp)
foreach(header ${PUBLIC_HEADERS})
    file(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR}/src/cpptypr ${header})
    configure_file(${header} ${CMAKE_CURRENT_BINARY_DIR}/include/cpptypr/${rel} COPYONLY)
endforeach()

# ---- Library ----
file(GLOB CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/cpptypr/*.cpp)

add_library(cpptypr STATIC ${CPP_SOURCES})

target_include_directories(cpptypr PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(cpptypr PUBLIC ctypr)

# ---- Tests ----
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# ---- Installation ----
set(CMAKE_INSTALL_INCLUDEDIR "include")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/cpptypr.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/cpptypr/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cpptypr
    FILES_MATCHING PATTERN "*.hpp"
)

install(TARGETS cpptypr
    EXPORT cpptyprTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# ---- CMake Config ----
include(CMakePackageConfigHelpers)

install(EXPORT cpptyprTargets
    FILE cpptyprTargets.cmake
    NAMESPACE cpptypr::
    DESTINATION lib/cmake/cpptypr
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpptyprConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cpptyprConfig.cmake
    INSTALL_DESTINATION lib/cmake/cpptypr
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/cpptyprConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/cpptyprConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cpptyprConfigVersion.cmake
    DESTINATION lib/cmake/cpptypr
)

# ---- CPack ----
set(CPACK_PACKAGE_NAME "cpptypr")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++ typing practice library")
set(CPACK_PACKAGE_VENDOR "cpptypr")
string(TOLOWER "${CMAKE_SYSTEM_NAME}" CPACK_SYSTEM_NAME)
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}")
set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
set(CPACK_GENERATOR "TGZ;ZIP")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
endif()

# ---- Remove ctypr C headers from install ----
# ctypr is a build-time dependency; its C headers should not
# be distributed with the C++ package.
install(CODE [[
    file(REMOVE_RECURSE
        "${CMAKE_INSTALL_PREFIX}/include/ctypr"
        "${CMAKE_INSTALL_PREFIX}/include/ctypr.h"
    )
]])

include(CPack)
