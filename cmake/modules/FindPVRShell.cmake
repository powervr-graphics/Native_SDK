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

		string(TOLOWER "${CMAKE_BUILD_TYPE}" PVR_ANDROID_BUILD_TYPE)

		# Search for the config file directly
		# Try lowercase build type first
		set(SEARCH_PATH "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRShell/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/PVRShell/PVRShellConfig.cmake")
		file(GLOB PVRShell_CONFIG_GLOB "${SEARCH_PATH}")
		
		# If not found, try original build type
		if(NOT PVRShell_CONFIG_GLOB)
			set(SEARCH_PATH "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRShell/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/PVRShell/PVRShellConfig.cmake")
			file(GLOB PVRShell_CONFIG_GLOB "${SEARCH_PATH}")
		endif()
		
		if(PVRShell_CONFIG_GLOB)
			list(GET PVRShell_CONFIG_GLOB 0 PVRShell_CONFIG_FILE)
			get_filename_component(PVRShell_DIR ${PVRShell_CONFIG_FILE} DIRECTORY)
			message(STATUS "PVRShell: Found prebuilt directory ${PVRShell_DIR}")
		endif()
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