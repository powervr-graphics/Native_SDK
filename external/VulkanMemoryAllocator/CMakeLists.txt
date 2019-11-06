cmake_minimum_required(VERSION 3.3)

if(NOT PVR_PREBUILT_DEPENDENCIES)
	include(../../cmake/Functions.cmake)

	# Set current VulkanMemoryAllocator version
	set(external_VulkanMemoryAllocator_VERSION "v2.2.0" CACHE INTERNAL "")

	# Set the VulkanMemoryAllocator source directory
	set(VulkanMemoryAllocator_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src CACHE INTERNAL "")

	# Set VulkanMemoryAllocator download directory - we use a downloads directory to allow us to maintain cleanliness
	set(VulkanMemoryAllocator_DOWNLOAD_DIR ${CMAKE_CURRENT_LIST_DIR}/../downloads/VulkanMemoryAllocator_downloads CACHE INTERNAL "")
	
	# Don't download if it is already present - this handles cases where internet connectivity may be limited but all packages are already available
	if(EXISTS ${VulkanMemoryAllocator_DOWNLOAD_DIR}/${external_VulkanMemoryAllocator_VERSION}.tar.gz AND EXISTS ${VulkanMemoryAllocator_SRC_DIR}/src/vk_mem_alloc.h)
		set(VulkanMemoryAllocator_URL "" CACHE INTERNAL "")
	else()
		set(VulkanMemoryAllocator_URL "https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/archive/${external_VulkanMemoryAllocator_VERSION}.tar.gz")
		# Remove the existing VulkanMemoryAllocator downloads so that only the newest version of VulkanMemoryAllocator will be present
		file(REMOVE_RECURSE ${VulkanMemoryAllocator_DOWNLOAD_DIR})
		message("VulkanMemoryAllocator was not found so will be downloaded: ${VulkanMemoryAllocator_URL}")
	endif()
	
	download_external_project("VulkanMemoryAllocator" "${CMAKE_CURRENT_BINARY_DIR}/download" "${VulkanMemoryAllocator_SRC_DIR}" "${VulkanMemoryAllocator_DOWNLOAD_DIR}" "${VulkanMemoryAllocator_URL}" "${VulkanMemoryAllocator_SRC_DIR}/src/vk_mem_alloc.h")

	add_library(VulkanMemoryAllocator INTERFACE)
		
	target_include_directories(VulkanMemoryAllocator INTERFACE
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/src>)
	
	install(TARGETS VulkanMemoryAllocator EXPORT VulkanMemoryAllocatorTargets)
	install(EXPORT VulkanMemoryAllocatorTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VulkanMemoryAllocator OPTIONAL)

	# Export VulkanMemoryAllocator from the build tree for use by other projects
	export(EXPORT VulkanMemoryAllocatorTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/VulkanMemoryAllocatorTargets.cmake")
	# build tree package config
	file(COPY VulkanMemoryAllocatorConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/" NO_SOURCE_PERMISSIONS)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/VulkanMemoryAllocatorConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/VulkanMemoryAllocatorConfig.cmake" COPYONLY)
endif()