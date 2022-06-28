# This file provides all of the information that is required for independent parts of the framework to be able to build 
# through cmake, this primarily focuses on ensuring that all modules know where the root of the SDK is and also 
# how to find different submodules 

# Add a cmake function which streamlines adding submodues to the SDK
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/utilities/submodules.cmake)		

# Add a cmake function which adds the default framework compile definitions
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/utilities/compile_options.cmake) 

# Cache the SDK default cmake values to build the framework
# This includes which parts of the framework to build 
# These variables are cached but not forced, so they won't 
# override the values passed from the directory above
set(SDK_ROOT_INTERNAL_DIR "${CMAKE_CURRENT_LIST_DIR}/.." CACHE INTERNAL "Path to the root of the SDK")
option(PVR_BUILD_FRAMEWORK_VULKAN "Build the Vulkan part of the framework" ON)
option(PVR_BUILD_FRAMEWORK_GLES "Build the GLES part of the framework" ON)
option(PVR_BUILD_FRAMEWORK_OPENCL "Build the OpenCL part of the framework" ON)