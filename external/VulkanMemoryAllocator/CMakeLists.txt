cmake_minimum_required(VERSION 3.10)
project(VulkanMemoryAllocator)

# Ensure The Vulkan Memory Allocator isn't added multiple times
if(TARGET VulkanMemoryAllocator)
	return()
endif() 

if(NOT PVR_PREBUILT_DEPENDENCIES)
	include(../Common.cmake)

	# Set the VulkanMemoryAllocator source directory
	set(VulkanMemoryAllocator_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/source CACHE INTERNAL "")

	add_library(VulkanMemoryAllocator INTERFACE)

	target_include_directories(VulkanMemoryAllocator INTERFACE
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
		$<BUILD_INTERFACE:${VulkanMemoryAllocator_SRC_DIR}/src>)

	if(PVR_ENABLE_INSTALL)
		install(TARGETS VulkanMemoryAllocator EXPORT VulkanMemoryAllocatorTargets)
		install(EXPORT VulkanMemoryAllocatorTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VulkanMemoryAllocator OPTIONAL)

		# Export VulkanMemoryAllocator from the build tree for use by other projects
		export(EXPORT VulkanMemoryAllocatorTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/VulkanMemoryAllocatorTargets.cmake")
		# build tree package config
		file(COPY VulkanMemoryAllocatorConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/" NO_SOURCE_PERMISSIONS)
		configure_file("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/VulkanMemoryAllocatorConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/VulkanMemoryAllocatorConfig.cmake" COPYONLY)
	endif()
endif()