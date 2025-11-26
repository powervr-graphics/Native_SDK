# FindPVRCamera.cmake
#
# Finds the PVRCamera library and its dependencies
#
# This will define the following imported targets
#     PVRCore
#     PVRCamera

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(CMakeFindDependencyMacro)

if(NOT TARGET PVRCore)
	find_dependency(PVRCore REQUIRED MODULE)
endif()

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		# Allow finding packages in the host file system
		set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
		
		# Use wildcard for build type to handle Debug/debug casing differences
		file(GLOB PVRCamera_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRCamera/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/PVRCamera")
		
		# The glob will return a list, but there should only be one match.
		if(PVRCamera_DIR_GLOB)
			list(GET PVRCamera_DIR_GLOB 0 PVRCamera_DIR)
		else()
			message(STATUS "PVRCamera: No build directory found matching ${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRCamera/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/PVRCamera")
		endif()
	endif()
endif()

if(NOT TARGET PVRCamera)
	find_package(PVRCamera REQUIRED CONFIG)
endif()