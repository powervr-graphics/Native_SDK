cmake_minimum_required(VERSION 3.3)
project (external)

# Includes external modules which are dependencies for the PowerVR SDK.

# Setup the external_pugixml target
add_subdirectory_if_not_already_included(pugixml pugixml ${EXTERNAL_CMAKE_FILES_FOLDER}/external_pugixml)

if(NOT APPLE AND NOT CMAKE_SYSTEM_NAME MATCHES "QNX")
	# Setup the external_glslang target
	add_subdirectory_if_not_already_included(glslang glslang ${EXTERNAL_CMAKE_FILES_FOLDER}/external_glslang)

	# Setup the external_VulkanMemoryAllocator target
	add_subdirectory_if_not_already_included(VulkanMemoryAllocator VulkanMemoryAllocator ${EXTERNAL_CMAKE_FILES_FOLDER}/external_VulkanMemoryAllocator)
endif()