# Target name
set(target baba-is-auto)

# Define
set(root_dir ${CMAKE_CURRENT_SOURCE_DIR}/../..)

# Includes
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Sources
file(GLOB header_dir
    ${root_dir}/Includes)

file(GLOB_RECURSE headers
    ${header_dir}/*.hpp)
set(baba_is_auto_header ${header_dir}/baba-is-auto/baba-is-auto.hpp)
list(REMOVE_ITEM headers ${baba_is_auto_header})

file(GLOB_RECURSE sources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# Custom-build event
if (BUILD_FROM_PIP)
    if (NOT DEFINED BABA_PYTHON_EXECUTABLE OR "${BABA_PYTHON_EXECUTABLE}" STREQUAL "")
        message(FATAL_ERROR "BUILD_FROM_PIP requires BABA_PYTHON_EXECUTABLE to be set")
    endif ()
    set(baba_is_auto_python ${BABA_PYTHON_EXECUTABLE})
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(baba_is_auto_python ${Python3_EXECUTABLE})
else ()
    set(baba_is_auto_python python3)
endif ()

set(baba_is_auto_header_gen_py ${root_dir}/Scripts/header_gen.py)
add_custom_target(baba_is_auto_header_gen_py ALL
    COMMAND ${baba_is_auto_python} ${baba_is_auto_header_gen_py}
    DEPENDS ${headers} ${baba_is_auto_header_gen_py}
)

# Build library
add_library(${target}
    ${sources})
add_dependencies(${target} baba_is_auto_header_gen_py)

# Project options
set_target_properties(${target}
    PROPERTIES
    ${DEFAULT_PROJECT_OPTIONS}
)

# Compile options
target_compile_options(${target}
    PRIVATE

    PUBLIC
    ${DEFAULT_COMPILE_OPTIONS}

    INTERFACE
)

target_link_libraries(${target}
    PRIVATE

    PUBLIC
    ${DEFAULT_LINKER_OPTIONS}
	${DEFAULT_LIBRARIES}
    effolkronium_random

    INTERFACE
)

# Install
install(TARGETS ${target} DESTINATION lib)
install(DIRECTORY ${header_dir} DESTINATION include)
