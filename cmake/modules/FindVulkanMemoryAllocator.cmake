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

		set(VMA_CONFIG_FILE "VulkanMemoryAllocatorConfig.cmake")
		
		# Build list of possible files
		file(GLOB VMA_CONFIG_GLOB
            "${CMAKE_CURRENT_LIST_DIR}/../../external/VulkanMemoryAllocator/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/build/${VMA_CONFIG_FILE}"
            "${CMAKE_CURRENT_LIST_DIR}/../../external/VulkanMemoryAllocator/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/${VMA_CONFIG_FILE}"
            "${CMAKE_CURRENT_LIST_DIR}/../../external/VulkanMemoryAllocator/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/${VMA_CONFIG_FILE}"
            "${CMAKE_CURRENT_LIST_DIR}/../../external/VulkanMemoryAllocator/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/build/${VMA_CONFIG_FILE}"
            "${CMAKE_CURRENT_LIST_DIR}/../../external/VulkanMemoryAllocator/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/cmake/${VMA_CONFIG_FILE}"
        )

		set(VulkanMemoryAllocator_DIR "")
		foreach(CONFIG_PATH IN LISTS VMA_CONFIG_GLOB)
            get_filename_component(DIR "${CONFIG_PATH}" DIRECTORY)
            if(EXISTS "${DIR}/VulkanMemoryAllocatorTargets.cmake")
                set(VulkanMemoryAllocator_DIR "${DIR}")
                break()
            endif()
		endforeach()

        # If we couldn't find one with Targets.cmake, fallback to any config
        if(NOT VulkanMemoryAllocator_DIR AND VMA_CONFIG_GLOB)
            list(GET VMA_CONFIG_GLOB 0 VMA_CONFIG_PATH)
            get_filename_component(VulkanMemoryAllocator_DIR "${VMA_CONFIG_PATH}" DIRECTORY)
        endif()
	endif()
endif()

if(NOT TARGET VulkanMemoryAllocator)
    if(VulkanMemoryAllocator_DIR)
	    find_package(VulkanMemoryAllocator REQUIRED CONFIG)
    else()
        if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../../external/VulkanMemoryAllocator/CMakeLists.txt")
            set(PVR_PREBUILT_DEPENDENCIES_CACHED ${PVR_PREBUILT_DEPENDENCIES})
            set(PVR_PREBUILT_DEPENDENCIES OFF)
            add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../../external/VulkanMemoryAllocator" "${CMAKE_BINARY_DIR}/external/VulkanMemoryAllocator")
            set(PVR_PREBUILT_DEPENDENCIES ${PVR_PREBUILT_DEPENDENCIES_CACHED})
        endif()
    endif()
endif()

if(TARGET VulkanMemoryAllocator)
    get_filename_component(VMA_INC_DIR "${CMAKE_CURRENT_LIST_DIR}/../../external/VulkanMemoryAllocator/source/src" ABSOLUTE)
    set_property(TARGET VulkanMemoryAllocator APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${VMA_INC_DIR}")
endif()
