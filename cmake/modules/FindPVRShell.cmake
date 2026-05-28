# FindPVRShell.cmake
#
# Finds the PVRShell library and its dependencies
#
# This will define the following imported targets
#	  PVRCore
#	  PVRShell

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(CMakeFindDependencyMacro)
include("${CMAKE_CURRENT_LIST_DIR}/../utilities/android_utils.cmake")

if(NOT TARGET PVRCore)
	find_dependency(PVRCore REQUIRED MODULE)
endif()



if(PVR_PREBUILT_DEPENDENCIES AND ANDROID)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
    pvr_find_android_build_path(PVRShell_PREBUILT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRShell/build-android" "PVRShell")
    if(PVRShell_PREBUILT_DIR)
        list(APPEND CMAKE_PREFIX_PATH "${PVRShell_PREBUILT_DIR}")
    endif()
endif()

if(NOT TARGET PVRShell)

	# Try to find the package configuration
	find_package(PVRShell CONFIG QUIET)
	
	if(PVRShell_FOUND)
		message(STATUS "PVRShell: Package configuration found.")
		# Ensure the framework source directory is in the include path.
		get_filename_component(PVR_FRAMEWORK_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework" ABSOLUTE)
		get_filename_component(PVR_SDK_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../include" ABSOLUTE)
		
		message(STATUS "PVRShell: Appending include directories: ${PVR_FRAMEWORK_DIR} and ${PVR_SDK_INCLUDE_DIR}")
		set_property(TARGET PVRShell APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${PVR_FRAMEWORK_DIR}" "${PVR_SDK_INCLUDE_DIR}")
	else()
		message(STATUS "PVRShell: Prebuilt package not found. Attempting to build from source...")
		set(PVRShell_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRShell")
		if(EXISTS "${PVRShell_SOURCE_DIR}/CMakeLists.txt")
			add_subdirectory("${PVRShell_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/framework/PVRShell")
		else()
			message(FATAL_ERROR "PVRShell: Could not find prebuilt package AND could not find source at ${PVRShell_SOURCE_DIR}")
		endif()
	endif()
endif()
if(TARGET PVRShell)
    set(PVRShell_FOUND TRUE)
endif()
