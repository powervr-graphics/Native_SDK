cmake_minimum_required(VERSION 3.10)
# Copyright (c) Imagination Technologies Limited.

project(external)

include(Common.cmake)

# Includes external modules which are dependencies for the PowerVR SDK.
add_subdirectory_if_exists(glm glm)
add_subdirectory_if_exists(pugixml pugixml)
add_subdirectory_if_exists(tinygltf tinygltf)
add_subdirectory_if_exists(PVRScope PVRScope)

message ("PowerVR SDK uses git submodules for external dependencies since R22.1-v5.9. Please remember to clone with --recurse-submodules parameter")

if(((${MOLTENVK_FOUND}) OR (NOT APPLE)) AND NOT CMAKE_SYSTEM_NAME MATCHES "QNX")
	add_subdirectory_if_exists(glslang glslang)
	add_subdirectory_if_exists(VulkanMemoryAllocator VulkanMemoryAllocator)
endif()
