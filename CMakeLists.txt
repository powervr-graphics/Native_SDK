cmake_minimum_required(VERSION 3.3)
project(PowerVR_SDK)

set(CMAKE_CXX_STANDARD 14)

if(TARGET PowerVR_SDK)
	return()
endif()

include(cmake/Common.cmake)

add_library(PowerVR_SDK INTERFACE)
target_include_directories(PowerVR_SDK INTERFACE
    ${SDK_ROOT}/include
    ${SDK_ROOT}/framework)
target_compile_definitions(PowerVR_SDK INTERFACE $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1 RELEASE=1>)

option(BUILD_FRAMEWORK "Build the PowerVR Framework" ON)
option(USE_PREBUILT_DEPENDENCIES "Indicates that the PowerVR Framework and its dependencies have been prebuilt. Libraries will not be built and will instead be imported. The Examples will still be built" OFF)

set(GLSLANG_INSTALL_DIR "GLSLANG-NOTFOUND" CACHE PATH "Path to a prebuilt glslang")
set(GLSLANG_VALIDATOR_INSTALL_DIR "GLSLANG_VALIDATOR-NOTFOUND" CACHE PATH "Path to a prebuilt glslangValidator")

# Check environment variable also
if(NOT GLSLANG_INSTALL_DIR)
	set(GLSLANG_INSTALL_DIR $ENV{GLSLANG_INSTALL_DIR})
endif()

if(NOT GLSLANG_VALIDATOR_INSTALL_DIR)
	set(GLSLANG_VALIDATOR_INSTALL_DIR $ENV{GLSLANG_VALIDATOR_INSTALL_DIR})
endif()

# If the location for a prebuilt glslang has been provided but a location for a prebuilt glslangValidator has not been provided
# then use the glslang location for both
if(GLSLANG_INSTALL_DIR AND NOT GLSLANG_VALIDATOR_INSTALL_DIR)
	set(GLSLANG_VALIDATOR_INSTALL_DIR ${GLSLANG_INSTALL_DIR})
endif()

# Setup the external modules required
add_subdirectory(external EXCLUDE_FROM_ALL)

if(BUILD_FRAMEWORK)
	add_subdirectory(framework EXCLUDE_FROM_ALL)
endif()

get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT)
	option(BUILD_EXAMPLES "Build the PowerVR SDK Examples" OFF)
else()
	option(BUILD_EXAMPLES "Build the PowerVR SDK Examples" ON)
endif()
if(BUILD_EXAMPLES)
	option(BUILD_OPENGLES_EXAMPLES "Build the OpenGLES PowerVR SDK Examples - BUILD_EXAMPLES must also be enabled" OFF)
	option(BUILD_VULKAN_EXAMPLES "Build the Vulkan PowerVR SDK Examples - BUILD_EXAMPLES must also be enabled" OFF)

	if (NOT BUILD_OPENGLES_EXAMPLES AND NOT BUILD_VULKAN_EXAMPLES)
		if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/examples/Vulkan/CMakeLists.txt)
			set (BUILD_VULKAN_EXAMPLES 1)
		endif()
		if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/examples/OpenGLES/CMakeLists.txt)
			set (BUILD_OPENGLES_EXAMPLES 1)
		endif()
	endif()
	
	add_subdirectory(examples)
endif()