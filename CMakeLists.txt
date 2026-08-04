# CMake version
cmake_minimum_required(VERSION 3.31.6 FATAL_ERROR)

# Include cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

# Use vcpkg automatically when the environment exposes it.
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{CMAKE_TOOLCHAIN_FILE})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{CMAKE_TOOLCHAIN_FILE}" CACHE STRING "CMake toolchain file")
elseif(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

# Build options for level editor
option(BABA_BUILD_EDITOR "Build the C++ ImGui level editor" OFF)
option(BUILD_COVERAGE "Build code coverage" OFF)
if (BABA_BUILD_EDITOR)
    list(APPEND VCPKG_MANIFEST_FEATURES editor)
endif()

# Declare project
project(baba-is-auto)

# Set output directories
set(DEFAULT_CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

# Set enable output of compile commands during generation
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Includes
include_directories(Includes)
include_directories(Extensions/BabaPython/Includes)

# Find packages
find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED)
find_package(doctest CONFIG REQUIRED)
if (BUILD_FROM_PIP)
    set(PYBIND11_NOPYTHON ON)
else()
    set(PYBIND11_FINDPYTHON ON)
endif()
find_package(pybind11 CONFIG REQUIRED)
find_package(effolkronium_random CONFIG REQUIRED)

# Compile options
include(CMake/CompileOptions.cmake)

# Build type - Release by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Overrides
set(CMAKE_MACOSX_RPATH ON)

# Set resources
set(MAPS_DIR ${PROJECT_SOURCE_DIR}/Resources/Maps)

# Project modules
add_subdirectory(Sources/baba-is-auto)
add_subdirectory(Tests/UnitTests)
if (BABA_BUILD_EDITOR)
    add_subdirectory(Extensions/BabaEditor)
endif()

if (BUILD_FROM_PIP)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${DEFAULT_CMAKE_LIBRARY_OUTPUT_DIRECTORY})
endif()
add_subdirectory(Extensions/BabaPython)
