# FindPVRShell.cmake
#
# Finds the PVRShell library and its dependencies
#
# This will define the following imported targets
#	  PVRCore
#	  PVRShell

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
		file(GLOB PVRShell_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRShell/build-android/.cxx/*/*/${ANDROID_ABI}/PVRShell")
		
		# The glob will return a list, but there should only be one match.
		if(PVRShell_DIR_GLOB)
			list(GET PVRShell_DIR_GLOB 0 PVRShell_DIR)
		else()
			message(STATUS "PVRShell: No build directory found matching ${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRShell/build-android/.cxx/*/*/${ANDROID_ABI}/PVRShell")
		endif()
	endif()
endif()

if(NOT TARGET PVRShell)
	find_package(PVRShell REQUIRED CONFIG)
endif()