# FindPVRVk.cmake
#
# Finds the PVRVk library
#
# This will define the following imported targets
#     PVRVk

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		# Allow finding packages in the host file system
		set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

		string(TOLOWER "${CMAKE_BUILD_TYPE}" PVR_ANDROID_BUILD_TYPE)

		# Use wildcard for build type to handle Debug/debug casing differences
        file(GLOB PVRVk_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRVk/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/PVRVk")
        
        # If not found, try original build type
		if(NOT PVRVk_DIR_GLOB)
			file(GLOB PVRVk_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRVk/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/PVRVk")
		endif()

        # The glob will return a list, but there should only be one match.
		if(PVRVk_DIR_GLOB)
			list(GET PVRVk_DIR_GLOB 0 PVRVk_DIR)
		else()
			message(STATUS "PVRVk: No build directory found matching ${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRVk/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/PVRVk")
		endif()
	endif()
endif()

if(NOT TARGET PVRVk)
	find_package(PVRVk REQUIRED CONFIG)
    if(PVRVk_FOUND)
        # Ensure the framework source directory is in the include path.
        get_filename_component(PVR_FRAMEWORK_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework" ABSOLUTE)
        get_filename_component(PVR_SDK_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../include" ABSOLUTE)
        
        set_property(TARGET PVRVk APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${PVR_FRAMEWORK_DIR}" "${PVR_SDK_INCLUDE_DIR}")
    endif()
endif()