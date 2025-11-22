# FindPVRAssets.cmake
#
# Finds the PVRAssets library and its dependencies
#
# This will define the following imported targets
#     PVRCore
#     tinygltf
#     PVRAssets

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(CMakeFindDependencyMacro)

if(NOT TARGET PVRCore)
	find_dependency(PVRCore REQUIRED MODULE)
endif()

if(NOT TARGET tinygltf)
	find_dependency(tinygltf REQUIRED MODULE)
endif()

# Try to find prebuilt if configured
if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		# Allow finding packages in the host file system
		set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

		string(TOLOWER "${CMAKE_BUILD_TYPE}" PVR_ANDROID_BUILD_TYPE)

		# Search for the config file directly
		# Try lowercase build type first
		set(SEARCH_PATH "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRAssets/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/PVRAssets/PVRAssetsConfig.cmake")
		file(GLOB PVRAssets_CONFIG_GLOB "${SEARCH_PATH}")
		
		# If not found, try original build type
		if(NOT PVRAssets_CONFIG_GLOB)
			set(SEARCH_PATH "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRAssets/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/PVRAssets/PVRAssetsConfig.cmake")
			file(GLOB PVRAssets_CONFIG_GLOB "${SEARCH_PATH}")
		endif()
		
		if(PVRAssets_CONFIG_GLOB)
			list(GET PVRAssets_CONFIG_GLOB 0 PVRAssets_CONFIG_FILE)
			get_filename_component(PVRAssets_DIR ${PVRAssets_CONFIG_FILE} DIRECTORY)
			message(STATUS "PVRAssets: Found prebuilt directory ${PVRAssets_DIR}")
		endif()
	endif()
endif()

if(NOT TARGET PVRAssets)
	# Try to find the package configuration
	find_package(PVRAssets CONFIG QUIET)
	
	if(PVRAssets_FOUND)
		message(STATUS "PVRAssets: Package configuration found.")
        # Ensure the framework source directory is in the include path.
        get_filename_component(PVR_FRAMEWORK_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework" ABSOLUTE)
        get_filename_component(PVR_SDK_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../include" ABSOLUTE)
        
        set_property(TARGET PVRAssets APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${PVR_FRAMEWORK_DIR}" "${PVR_SDK_INCLUDE_DIR}")
	else()
		message(STATUS "PVRAssets: Prebuilt package not found. Attempting to build from source...")
		set(PVRAssets_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRAssets")
		if(EXISTS "${PVRAssets_SOURCE_DIR}/CMakeLists.txt")
			add_subdirectory("${PVRAssets_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/framework/PVRAssets")
		else()
			message(FATAL_ERROR "PVRAssets: Could not find prebuilt package AND could not find source at ${PVRAssets_SOURCE_DIR}")
		endif()
	endif()
endif()
