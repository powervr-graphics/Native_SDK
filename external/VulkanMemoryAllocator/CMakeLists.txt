cmake_minimum_required(VERSION 3.3)

include(../../cmake/Common.cmake)

# Set current VulkanMemoryAllocator version
set(external_VulkanMemoryAllocator_VERSION "v2.2.0" CACHE INTERNAL "")

# Set VulkanMemoryAllocator prefix location
set(VulkanMemoryAllocator_PREFIX ${EXTERNAL_CMAKE_FILES_FOLDER}/external_VulkanMemoryAllocator CACHE INTERNAL "")

# Set the VulkanMemoryAllocator source directory
set(VulkanMemoryAllocator_SRC_DIR ${SDK_ROOT}/external/VulkanMemoryAllocator/src CACHE INTERNAL "")

# Set VulkanMemoryAllocator download directory - we use a downloads directory to allow us to maintain cleanliness
set(VulkanMemoryAllocator_DOWNLOAD_DIR ${SDK_ROOT}/external/downloads/VulkanMemoryAllocator_downloads CACHE INTERNAL "")

get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(NOT HAS_PARENT OR NOT USE_PREBUILT_DEPENDENCIES)
	# Don't download if it is already present - this handles cases where internet connectivity may be limited but all packages are already available
	if(EXISTS ${VulkanMemoryAllocator_DOWNLOAD_DIR}/${external_VulkanMemoryAllocator_VERSION}.tar.gz AND EXISTS ${VulkanMemoryAllocator_SRC_DIR}/src/vk_mem_alloc.h)
		set(VulkanMemoryAllocator_URL "" CACHE INTERNAL "")
		message("VulkanMemoryAllocator was found so will not be downloaded: ${VulkanMemoryAllocator_SRC_DIR}")
	else()
		set(VulkanMemoryAllocator_URL "https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/archive/${external_VulkanMemoryAllocator_VERSION}.tar.gz")
		# Remove the existing VulkanMemoryAllocator downloads so that only the newest version of VulkanMemoryAllocator will be present
		file(REMOVE_RECURSE ${VulkanMemoryAllocator_DOWNLOAD_DIR})
		message("VulkanMemoryAllocator was not found so will be downloaded: ${VulkanMemoryAllocator_URL}")
	endif()
	
	download_external_project("VulkanMemoryAllocator" "${VulkanMemoryAllocator_PREFIX}" "${VulkanMemoryAllocator_SRC_DIR}" "${VulkanMemoryAllocator_DOWNLOAD_DIR}" "${VulkanMemoryAllocator_URL}" "")
endif()

add_library(VulkanMemoryAllocator INTERFACE IMPORTED GLOBAL)
	
set_target_properties(VulkanMemoryAllocator PROPERTIES 
	INTERFACE_INCLUDE_DIRECTORIES ${VulkanMemoryAllocator_SRC_DIR}/src)
