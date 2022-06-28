cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)
project(tinygltf)

# Check that tinygltf hasn't been added before
if(TARGET tinygltf)
	return()
endif()

# Tiny gltf is a header only library, in CMake, interface libraries shouldn't have any sources
add_library(tinygltf INTERFACE)
target_include_directories(tinygltf INTERFACE ${CMAKE_CURRENT_LIST_DIR}/source)

# Force turn off the tinygltf examples
#set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE BOOL "" FORCE)
#set(TINYGLTF_BUILD_GL_EXAMPLES OFF CACHE BOOL "" FORCE)
#set(TINYGLTF_BUILD_VALIDATOR_EXAMPLE OFF CACHE BOOL "" FORCE)

# Finally add tinygltf to the build system
#add_subdirectory(source "${CMAKE_CURRENT_BINARY_DIR}/build")