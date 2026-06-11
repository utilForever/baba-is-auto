# Target name
set(target pyBaba)

# Define
set(root_dir ${CMAKE_CURRENT_SOURCE_DIR}/../..)

# Sources
file(GLOB_RECURSE sources
		${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# Add Pybind11 module
if (BUILD_FROM_PIP)
	add_library(${target} MODULE ${sources})
	target_include_directories(${target}
			PRIVATE
			${BABA_PYTHON_INCLUDE_DIR})
	target_link_libraries(${target}
			PRIVATE
			pybind11::headers)
	if (BABA_PYTHON_LIBRARY)
		target_link_libraries(${target}
				PRIVATE
				${BABA_PYTHON_LIBRARY})
	elseif (APPLE)
		target_link_options(${target}
				PRIVATE
				"-undefined"
				"dynamic_lookup")
	endif()
	set_target_properties(${target}
			PROPERTIES
			PREFIX ""
			SUFFIX "${BABA_PYTHON_EXTENSION_SUFFIX}"
			LIBRARY_OUTPUT_DIRECTORY "${BABA_PYTHON_OUTPUT_DIRECTORY}"
			RUNTIME_OUTPUT_DIRECTORY "${BABA_PYTHON_OUTPUT_DIRECTORY}"
			LIBRARY_OUTPUT_DIRECTORY_DEBUG "${BABA_PYTHON_OUTPUT_DIRECTORY}"
			RUNTIME_OUTPUT_DIRECTORY_DEBUG "${BABA_PYTHON_OUTPUT_DIRECTORY}"
			LIBRARY_OUTPUT_DIRECTORY_RELEASE "${BABA_PYTHON_OUTPUT_DIRECTORY}"
			RUNTIME_OUTPUT_DIRECTORY_RELEASE "${BABA_PYTHON_OUTPUT_DIRECTORY}"
			LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${BABA_PYTHON_OUTPUT_DIRECTORY}"
			RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${BABA_PYTHON_OUTPUT_DIRECTORY}"
			LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${BABA_PYTHON_OUTPUT_DIRECTORY}"
			RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${BABA_PYTHON_OUTPUT_DIRECTORY}")
else()
	find_package(pybind11 CONFIG REQUIRED)
	pybind11_add_module(${target} ${sources})
endif()

# Project options
set_target_properties(${target}
		PROPERTIES
		${DEFAULT_PROJECT_OPTIONS})

# Link libraries
target_link_libraries(${target}
		PRIVATE
		${DEFAULT_LINKER_OPTIONS}
		baba-is-auto)
