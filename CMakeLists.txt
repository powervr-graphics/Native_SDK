cmake_minimum_required(VERSION 3.3)
project(PowerVR_SDK)
include(cmake/Common.cmake)

# Define a cache string variable restricted to a specific set of values.
function(choice variable_name default_value description)
    set(choices ${ARGN})
    set(${variable_name} ${default_value} CACHE STRING "${description}. Supported values: ${choices}")
    set_property(CACHE ${variable_name} PROPERTY STRINGS ${choices})
    if(NOT ${default_value} IN_LIST choices)
        message(SEND_ERROR "Value ${${variable_name}} in variable ${variable_name} is not within the range of valid choices: ${choices}")
    endif()
endfunction()

# Allow toggling between vulkan and opengles
choice(PVR_GRAPHICS_LIBRARY OPENGLES "The underlying graphics library" OPENGLES VULKAN)

# Allows to differentiate between when we build this repository in isolation (by itself) and
# when it is included via `add_subdirectory()` by some downstream project.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(STANDALONE_BUILD ON)
endif()

add_library(PowerVR_SDK INTERFACE)
target_include_directories(PowerVR_SDK INTERFACE
    ${SDK_ROOT}/include
    ${SDK_ROOT}/framework
)

target_link_libraries(PowerVR_SDK INTERFACE ${EXTRA_LIBS})

# TODO: Implement platform logic to choose correct shared libraries for linker usage

add_subdirectory(external)
add_subdirectory(framework)

# Do not build examples if we're not a standalone build
if (NOT BUILD_OPENGLES_EXAMPLES AND NOT BUILD_VULKAN_EXAMPLES AND STANDALONE_BUILD)
	if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/examples/Vulkan/CMakeLists.txt)
		set (BUILD_VULKAN_EXAMPLES 1)
	endif()
	if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/examples/OpenGLES/CMakeLists.txt)
		set (BUILD_OPENGLES_EXAMPLES 1)
	endif()
endif()

if (BUILD_VULKAN_EXAMPLES)
	add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/examples/Vulkan")
endif()
if (BUILD_OPENGLES_EXAMPLES)
	add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/examples/OpenGLES")
endif()
