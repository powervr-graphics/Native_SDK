cmake_minimum_required(VERSION 3.3)

project(PowerVR_SDK)

include(GNUInstallDirs)

# Include a set of reusable functions
include("${CMAKE_CURRENT_LIST_DIR}/cmake/Functions.cmake")

# Include a set of reusable functions specific for the examples
include("${CMAKE_CURRENT_LIST_DIR}/examples/ExampleFunctions.cmake")

# Avoids the SDK being added multiple times
if(TARGET PowerVR_SDK)
	return()
endif()
add_library(PowerVR_SDK INTERFACE)

# Set PROJECT_ARCH to either 32 or 64 (bits). Used to set the framework folders for Windows.
if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
	set(PROJECT_ARCH 64 CACHE INTERNAL "")
else()
	set(PROJECT_ARCH 32 CACHE INTERNAL "")
endif()

# CMAKE_MODULE_PATH is used by find_package
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/modules" CACHE INTERNAL "")
# Default to release if the user passes nothing.
if(NOT CMAKE_BUILD_TYPE) 
	message("-DCMAKE_BUILD_TYPE not defined. Assuming Release")
	set(CMAKE_BUILD_TYPE "Release" CACHE INTERNAL "CMAKE_BUILD_TYPE - Specifies the build type on single-configuration generators")
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR}/install/${INSTALL_SUFFIX} CACHE INTERNAL "")
	# Ensure other projects don't set CMAKE_INSTALL_PREFIX
	set(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT FALSE)
endif()

option(PVR_BUILD_FRAMEWORK "Build the PowerVR Framework" ON)
option(PVR_SKIP_EXTERNAL "Skip the external modules used by the PowerVR Framework" OFF)
option(PVR_FIXED_LOCATION "Build the PowerVR Framework with framework and external libraries built to and imported from a fixed location (see cmake/PlatformSdkFolders for more information)" OFF)
option(PVR_PREBUILT_DEPENDENCIES "Indicates that the PowerVR Framework and its dependencies have been prebuilt. Libraries will not be built and will instead be imported. The Examples will still be built" OFF)

set(GLSLANG_VALIDATOR_INSTALL_DIR "GLSLANG_VALIDATOR-NOTFOUND" CACHE PATH "Path to a prebuilt glslangValidator")

if(NOT GLSLANG_VALIDATOR_INSTALL_DIR)
	set(GLSLANG_VALIDATOR_INSTALL_DIR $ENV{GLSLANG_VALIDATOR_INSTALL_DIR})
endif()

if(APPLE AND NOT IOS)
	find_package(MoltenVK)
endif()

if(NOT PVR_SKIP_EXTERNAL)
	add_subdirectory(external EXCLUDE_FROM_ALL)
endif()

if(WIN32)
	set(PVR_VFRAME_LIB_FOLDER "${CMAKE_CURRENT_LIST_DIR}/lib/Windows_x86_${PROJECT_ARCH}" CACHE INTERNAL "")
elseif(APPLE AND NOT IOS)
	set(PVR_VFRAME_LIB_FOLDER "${CMAKE_CURRENT_LIST_DIR}/lib/macOS_x86" CACHE INTERNAL "")
elseif(UNIX)
	set(PVR_VFRAME_LIB_FOLDER "${CMAKE_CURRENT_LIST_DIR}/lib/${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}" CACHE INTERNAL "")
endif()

if(NOT PVR_PREBUILT_DEPENDENCIES)
	if(PVR_BUILD_FRAMEWORK)
		add_subdirectory(framework EXCLUDE_FROM_ALL)
	endif()
endif()

get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT)
	option(PVR_BUILD_EXAMPLES "Build the PowerVR SDK Examples" OFF)
else()
	option(PVR_BUILD_EXAMPLES "Build the PowerVR SDK Examples" ON)
endif()

if(PVR_BUILD_EXAMPLES)
	option(PVR_BUILD_OPENGLES_EXAMPLES "Build the OpenGLES PowerVR SDK Examples - PVR_BUILD_EXAMPLES must also be enabled" OFF)
	option(PVR_BUILD_VULKAN_EXAMPLES "Build the Vulkan PowerVR SDK Examples - PVR_BUILD_EXAMPLES must also be enabled" OFF)
	option(PVR_BUILD_OPENCL_EXAMPLES "Build the OpenCL PowerVR SDK Examples - PVR_BUILD_EXAMPLES must also be enabled" OFF)
	option(PVR_BUILD_OPENGLES2_EXAMPLES "Only build OpenGL ES 2.0 examples - PVR_BUILD_EXAMPLES must also be enabled. Assumes PVR_BUILD_OPENGLES_EXAMPLES to ON and disables Non-OpenGL ES examples as well." OFF)

	if (PVR_BUILD_OPENGLES2_EXAMPLES)
		set(PVR_BUILD_OPENGLES_EXAMPLES ON) 
	endif()

	if(NOT PVR_BUILD_OPENGLES_EXAMPLES AND NOT PVR_BUILD_VULKAN_EXAMPLES AND NOT PVR_BUILD_OPENCL_EXAMPLES)
		if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/examples/OpenGLES/CMakeLists.txt)
			set(PVR_BUILD_OPENGLES_EXAMPLES 1)
		endif()
		if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/examples/Vulkan/CMakeLists.txt)
			set(PVR_BUILD_VULKAN_EXAMPLES 1)
		endif()
		if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/examples/OpenCL/CMakeLists.txt)
			set(PVR_BUILD_OPENCL_EXAMPLES 1)
		endif()
	endif()

	add_subdirectory(examples)
endif()