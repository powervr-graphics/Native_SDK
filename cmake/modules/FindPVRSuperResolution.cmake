# FindPVRSuperResolution.cmake
#
# Finds the PVRSuperResolution library and its dependencies
#
# This will define the following imported targets
#     PVRSuperResolution

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
		set(SEARCH_PATH "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRSuperResolution/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/PVRSuperResolution/PVRSuperResolutionConfig.cmake")
		file(GLOB PVRSuperResolution_CONFIG_GLOB "${SEARCH_PATH}")
		
		# If not found, try original build type
		if(NOT PVRSuperResolution_CONFIG_GLOB)
			set(SEARCH_PATH "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRSuperResolution/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/PVRSuperResolution/PVRSuperResolutionConfig.cmake")
			file(GLOB PVRSuperResolution_CONFIG_GLOB "${SEARCH_PATH}")
		endif()
		
		if(PVRSuperResolution_CONFIG_GLOB)
			list(GET PVRSuperResolution_CONFIG_GLOB 0 PVRSuperResolution_CONFIG_FILE)
			get_filename_component(PVRSuperResolution_DIR ${PVRSuperResolution_CONFIG_FILE} DIRECTORY)
			message(STATUS "PVRSuperResolution: Found prebuilt directory ${PVRSuperResolution_DIR}")
		endif()
	endif()
endif()

if(NOT TARGET PVRSuperResolution)
	# Try to find the package configuration
	find_package(PVRSuperResolution CONFIG QUIET)
	
	if(PVRSuperResolution_FOUND)
		message(STATUS "PVRSuperResolution: Package configuration found.")
		# Ensure the framework source directory is in the include path.
		get_filename_component(PVR_FRAMEWORK_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework" ABSOLUTE)
		get_filename_component(PVR_SDK_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../include" ABSOLUTE)
		
		message(STATUS "PVRSuperResolution: Appending include directories: ${PVR_FRAMEWORK_DIR} and ${PVR_SDK_INCLUDE_DIR}")
		set_property(TARGET PVRSuperResolution APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${PVR_FRAMEWORK_DIR}" "${PVR_SDK_INCLUDE_DIR}")
	else()
		message(STATUS "PVRSuperResolution: Prebuilt package not found. Attempting to build from source...")
		set(PVRSuperResolution_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRSuperResolution")
		if(EXISTS "${PVRSuperResolution_SOURCE_DIR}/CMakeLists.txt")
			add_subdirectory("${PVRSuperResolution_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/framework/PVRSuperResolution")
		else()
			message(FATAL_ERROR "PVRSuperResolution: Could not find prebuilt package AND could not find source at ${PVRSuperResolution_SOURCE_DIR}")
		endif()
	endif()
endif()