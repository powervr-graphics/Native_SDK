# FindPVRCore.cmake
#
# Finds the PVRCore library and its dependencies
#
# This will define the following imported targets
#     glm
#     pugixml
#	  PVRCore

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(CMakeFindDependencyMacro)

if(NOT TARGET glm)
	find_dependency(glm REQUIRED MODULE)
endif()

if(NOT TARGET pugixml)
	find_dependency(pugixml REQUIRED MODULE)
endif()

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		# Allow finding packages in the host file system
		set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

		string(TOLOWER "${CMAKE_BUILD_TYPE}" PVR_ANDROID_BUILD_TYPE)

		# Search for the config file directly
		# Try lowercase build type first
		set(SEARCH_PATH "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRCore/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/PVRCore/PVRCoreConfig.cmake")
		file(GLOB PVRCore_CONFIG_GLOB "${SEARCH_PATH}")
		
		# If not found, try original build type
		if(NOT PVRCore_CONFIG_GLOB)
			set(SEARCH_PATH "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRCore/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/PVRCore/PVRCoreConfig.cmake")
			file(GLOB PVRCore_CONFIG_GLOB "${SEARCH_PATH}")
		endif()
		
		if(PVRCore_CONFIG_GLOB)
			list(GET PVRCore_CONFIG_GLOB 0 PVRCore_CONFIG_FILE)
			get_filename_component(PVRCore_DIR ${PVRCore_CONFIG_FILE} DIRECTORY)
			message(STATUS "PVRCore: Found prebuilt directory ${PVRCore_DIR}")
		endif()
	endif()
endif()

if(NOT TARGET PVRCore)
	# Try to find the package configuration
	find_package(PVRCore CONFIG QUIET)
	
	if(PVRCore_FOUND)
		message(STATUS "PVRCore: Package configuration found.")
        # Ensure the framework source directory is in the include path.
		get_filename_component(PVR_FRAMEWORK_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework" ABSOLUTE)
		get_filename_component(PVR_SDK_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../include" ABSOLUTE)
		
		message(STATUS "PVRCore: Appending include directories: ${PVR_FRAMEWORK_DIR} and ${PVR_SDK_INCLUDE_DIR}")
		set_property(TARGET PVRCore APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${PVR_FRAMEWORK_DIR}" "${PVR_SDK_INCLUDE_DIR}")
	else()
		message(STATUS "PVRCore: Prebuilt package not found. Attempting to build from source...")
		set(PVRCore_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRCore")
		if(EXISTS "${PVRCore_SOURCE_DIR}/CMakeLists.txt")
			add_subdirectory("${PVRCore_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/framework/PVRCore")
		else()
			message(FATAL_ERROR "PVRCore: Could not find prebuilt package AND could not find source at ${PVRCore_SOURCE_DIR}")
		endif()
	endif()
endif()