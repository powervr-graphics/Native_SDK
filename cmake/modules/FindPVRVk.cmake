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

		# Use wildcard for build type to handle Debug/debug casing differences
        file(GLOB PVRVk_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRVk/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/PVRVk")

        # The glob will return a list, but there should only be one match.
		if(PVRVk_DIR_GLOB)
			list(GET PVRVk_DIR_GLOB 0 PVRVk_DIR)
		else()
			message(STATUS "PVRVk: No build directory found matching ${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRVk/build-android/.cxx/*/*/${ANDROID_ABI}/PVRVk")
		endif()
	endif()
endif()

if(NOT TARGET PVRVk)
	find_package(PVRVk REQUIRED CONFIG)
endif()