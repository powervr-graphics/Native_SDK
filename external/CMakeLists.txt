cmake_minimum_required(VERSION 3.3)
project(external)

# Includes external modules which are dependencies for the PowerVR SDK.
add_subdirectory_if_not_already_included(glm glm)
add_subdirectory_if_not_already_included(pugixml pugixml)
add_subdirectory_if_not_already_included(tinygltf tinygltf)
add_subdirectory_if_not_already_included(PVRScope PVRScope)

if(((${MOLTENVK_FOUND}) OR (NOT APPLE)) AND NOT CMAKE_SYSTEM_NAME MATCHES "QNX")
	add_subdirectory_if_not_already_included(glslang glslang)
	add_subdirectory_if_not_already_included(VulkanMemoryAllocator VulkanMemoryAllocator)
endif()