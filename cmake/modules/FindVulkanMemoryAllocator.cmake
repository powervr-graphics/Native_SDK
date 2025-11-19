# FindVulkanMemoryAllocator.cmake
#
# Finds the VulkanMemoryAllocator target
#
# This will define the following imported targets
#     VulkanMemoryAllocator

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		#set(VulkanMemoryAllocator_DIR "${CMAKE_CURRENT_LIST_DIR}/../../external/VulkanMemoryAllocator/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}")

		file(GLOB VulkanMemoryAllocator_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../external/VulkanMemoryAllocator/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}")
		# The glob will return a list, but there should only be one match.
		list(GET VulkanMemoryAllocator_DIR_GLOB 0 VulkanMemoryAllocator_DIR)
	endif()
endif()

if(NOT TARGET VulkanMemoryAllocator)
	find_package(VulkanMemoryAllocator REQUIRED CONFIG)
endif()