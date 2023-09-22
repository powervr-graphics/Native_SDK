cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)
project(PowerVR_SDK)

# Ensure we are not building for apple targets 
if(APPLE OR IOS)
	message(FATAL_ERROR "Sorry, the PowerVR SDK is no longer supported on Apple devices")
endif()

# Avoids the SDK target being added multiple times
if(TARGET PowerVR_SDK)
	return()
endif()

# Provides install directory variables to GNU standards
include(GNUInstallDirs)

# Store the root directory of the SDK framework, so configurable files can be found
set(SDK_ROOT_INTERNAL_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "Path to the root of the SDK")

# Make a top-level project file to build everything
add_library(PowerVR_SDK INTERFACE)

# Set default vakues for the options used by the SDK
# These options configure which parts of the framework are built
option(PVR_BUILD_FRAMEWORK_VULKAN "Build the Vulkan part of the framework" ON)
option(PVR_BUILD_FRAMEWORK_GLES "Build the GLES part of the framework" ON)
option(PVR_BUILD_FRAMEWORK_GLSC "Build the GLSC part of the framework" ON)
option(PVR_BUILD_FRAMEWORK_OPENCL "Build the OpenCL part of the framework" ON)
option(PVR_BUILD_VULKAN_EXAMPLES "Build the Vulkan examples" ON)
option(PVR_BUILD_OPENGLES_EXAMPLES "Build the GLES examples" ON)
option(PVR_BUILD_OPENGLSC_EXAMPLES "Build the GLSC examples" ON)
option(PVR_BUILD_OPENCL_EXAMPLES "Build the OpenCL examples" ON)

# If this cmake file isn't the top level one, then the SDK is being used in another project
# And we can assume the user doesn't want to build the examples 
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT)
	option(PVR_BUILD_EXAMPLES "Build the PowerVR SDK Examples" OFF)
else()
	option(PVR_BUILD_EXAMPLES "Build the PowerVR SDK Examples" ON)
endif()

# Ensure that if the user wants to build the examples for a specific API
# Then the corrosponding part of the framework would also be built
if(PVR_BUILD_EXAMPLES)
	if(PVR_BUILD_VULKAN_EXAMPLES)
		option(PVR_BUILD_FRAMEWORK_VULKAN "Build the Vulkan part of the framework" ON)
	endif()

	if(PVR_BUILD_OPENGLES_EXAMPLES)
		option(PVR_BUILD_FRAMEWORK_GLES "Build the GLES part of the framework" ON)
	endif()

	if(PVR_BUILD_OPENGLSC_EXAMPLES)
		option(PVR_BUILD_FRAMEWORK_GLSC "Build the GLSC part of the framework" ON)
	endif()
endif()

# Tell CMake where to find the SDK's cmake modules, ie when cmake calls find_package(*)
# cmake needs to know where the SDK's custom scripts for this process are located
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# Now we need to add in the CMake utilities, they're cmake files that provide helper functions
include(cmake/utilities/assets.cmake)			# Provides add_assets_to_target
include(cmake/utilities/executable.cmake)		# Provides add_platform_specific_executable
include(cmake/utilities/spirv.cmake)			# Provides add_spirv_shaders_to_target

# Add the framework source tree
add_subdirectory(framework)

# If the user has the examples option, then also add the examples source tree
if(PVR_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

# Add in the PVR Scope Developer external target.
add_subdirectory(external/PVRScope)