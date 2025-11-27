# FindVulkanMemoryAllocator.cmake
#
# Finds the VulkanMemoryAllocator target
#
# This will define the following imported targets
#     VulkanMemoryAllocator

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		
		message(STATUS "Debug FindVulkanMemoryAllocator: CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE=${PVR_ANDROID_BUILD_TYPE}")
		message(STATUS "Debug FindVulkanMemoryAllocator: ANDROID_ABI=${ANDROID_ABI}")

		# Try to find the prebuilt dependency in the .cxx folder
		# We check multiple patterns to account for AGP variations and case sensitivity
		
		# 1. Lowercase build type, with /build suffix (common for some AGP versions/configs)
		set(GLOB_PATTERN "${CMAKE_CURRENT_LIST_DIR}/../../external/VulkanMemoryAllocator/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/build")
		file(GLOB VulkanMemoryAllocator_DIR_GLOB "${GLOB_PATTERN}")
		
		# 2. Lowercase build type, without /build suffix
		if(NOT VulkanMemoryAllocator_DIR_GLOB)
		    set(GLOB_PATTERN "${CMAKE_CURRENT_LIST_DIR}/../../external/VulkanMemoryAllocator/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}")
		    file(GLOB VulkanMemoryAllocator_DIR_GLOB "${GLOB_PATTERN}")
		endif()

		# 3. Original case build type (fallback)
		if(NOT VulkanMemoryAllocator_DIR_GLOB)
		    set(GLOB_PATTERN "${CMAKE_CURRENT_LIST_DIR}/../../external/VulkanMemoryAllocator/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}")
		    file(GLOB VulkanMemoryAllocator_DIR_GLOB "${GLOB_PATTERN}")
		endif()
		
		# 4. Original case build type with /build suffix
		if(NOT VulkanMemoryAllocator_DIR_GLOB)
		    set(GLOB_PATTERN "${CMAKE_CURRENT_LIST_DIR}/../../external/VulkanMemoryAllocator/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/build")
		    file(GLOB VulkanMemoryAllocator_DIR_GLOB "${GLOB_PATTERN}")
		endif()

        # 5. Lowercase build type with /cmake suffix
        if(NOT VulkanMemoryAllocator_DIR_GLOB)
             set(GLOB_PATTERN "${CMAKE_CURRENT_LIST_DIR}/../../external/VulkanMemoryAllocator/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/cmake")
             file(GLOB VulkanMemoryAllocator_DIR_GLOB "${GLOB_PATTERN}")
        endif()

		message(STATUS "Debug FindVulkanMemoryAllocator: Result ${VulkanMemoryAllocator_DIR_GLOB}")

		if(VulkanMemoryAllocator_DIR_GLOB)
		    list(GET VulkanMemoryAllocator_DIR_GLOB 0 VulkanMemoryAllocator_DIR)
		endif()
	endif()
endif()

if(NOT TARGET VulkanMemoryAllocator)
    if(VulkanMemoryAllocator_DIR)
	    find_package(VulkanMemoryAllocator REQUIRED CONFIG)
    else()
        # Fallback to building from source if prebuilt not found
        message(STATUS "VulkanMemoryAllocator: Prebuilt package not found. Attempting to build from source...")
        if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../../external/VulkanMemoryAllocator/CMakeLists.txt")
            set(PVR_PREBUILT_DEPENDENCIES_CACHED ${PVR_PREBUILT_DEPENDENCIES})
            set(PVR_PREBUILT_DEPENDENCIES OFF)
            add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../../external/VulkanMemoryAllocator" "${CMAKE_BINARY_DIR}/external/VulkanMemoryAllocator")
            set(PVR_PREBUILT_DEPENDENCIES ${PVR_PREBUILT_DEPENDENCIES_CACHED})
        endif()
    endif()
endif()

if(NOT TARGET VulkanMemoryAllocator)
    # If still not found (and build from source failed or didn't happen), try standard find_package as last resort
    # This might fail if required, which is expected
	find_package(VulkanMemoryAllocator REQUIRED CONFIG)
endif()
