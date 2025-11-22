# FindPVRUtilsCL.cmake
#
# Finds the PVRUtilsCL library and its dependencies
#
# This will define the following imported targets
#     PVRCore
#	  PVRUtilsCL

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(CMakeFindDependencyMacro)

if(NOT TARGET PVRCore)
	find_dependency(PVRCore REQUIRED MODULE)
endif()

# Try to find prebuilt if configured
if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		# Allow finding packages in the host file system
		set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
		
		string(TOLOWER "${CMAKE_BUILD_TYPE}" PVR_ANDROID_BUILD_TYPE)

		# Search for the config file directly
		# Try lowercase build type first
		set(SEARCH_PATH "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRUtils/OpenCL/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/PVRUtilsCL/PVRUtilsCLConfig.cmake")
		file(GLOB PVRUtilsCL_CONFIG_GLOB "${SEARCH_PATH}")
		
		# If not found, try original build type
		if(NOT PVRUtilsCL_CONFIG_GLOB)
			set(SEARCH_PATH "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRUtils/OpenCL/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/PVRUtilsCL/PVRUtilsCLConfig.cmake")
			file(GLOB PVRUtilsCL_CONFIG_GLOB "${SEARCH_PATH}")
		endif()
		
		if(PVRUtilsCL_CONFIG_GLOB)
			list(GET PVRUtilsCL_CONFIG_GLOB 0 PVRUtilsCL_CONFIG_FILE)
			get_filename_component(PVRUtilsCL_DIR ${PVRUtilsCL_CONFIG_FILE} DIRECTORY)
			message(STATUS "PVRUtilsCL: Found prebuilt directory ${PVRUtilsCL_DIR}")
		endif()
	endif()
endif()

if(NOT TARGET PVRUtilsCL)
	# Try to find the package configuration
	find_package(PVRUtilsCL CONFIG QUIET)
	
	if(PVRUtilsCL_FOUND)
		message(STATUS "PVRUtilsCL: Package configuration found.")
	else()
		message(STATUS "PVRUtilsCL: Prebuilt package not found. Attempting to build from source...")
		set(PVRUtilsCL_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRUtils/OpenCL")
		if(EXISTS "${PVRUtilsCL_SOURCE_DIR}/CMakeLists.txt")
			add_subdirectory("${PVRUtilsCL_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/framework/PVRUtils/OpenCL")
		else()
			message(FATAL_ERROR "PVRUtilsCL: Could not find prebuilt package AND could not find source at ${PVRUtilsCL_SOURCE_DIR}")
		endif()
	endif()
endif()
