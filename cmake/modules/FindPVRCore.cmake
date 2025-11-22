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

		message(STATUS "FindPVRCore: CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
		message(STATUS "FindPVRCore: PVR_ANDROID_BUILD_TYPE=${PVR_ANDROID_BUILD_TYPE}")
		message(STATUS "FindPVRCore: ANDROID_ABI=${ANDROID_ABI}")
		
		# Use wildcard for build type to handle Debug/debug casing differences and other variants
		# Try lowercase build type first
		set(SEARCH_PATH "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRCore/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/PVRCore")
		message(STATUS "FindPVRCore: SEARCH_PATH_1=${SEARCH_PATH}")
		file(GLOB PVRCore_DIR_GLOB "${SEARCH_PATH}")
		
		# If not found, try original build type (case sensitive on Linux)
		if(NOT PVRCore_DIR_GLOB)
			set(SEARCH_PATH "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRCore/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/PVRCore")
			message(STATUS "FindPVRCore: SEARCH_PATH_2=${SEARCH_PATH}")
			file(GLOB PVRCore_DIR_GLOB "${SEARCH_PATH}")
		endif()
		
		message(STATUS "FindPVRCore: PVRCore_DIR_GLOB=${PVRCore_DIR_GLOB}")

		# The glob will return a list, but there should only be one match.
		list(LENGTH PVRCore_DIR_GLOB LEN)
		if(LEN GREATER 0)
			list(GET PVRCore_DIR_GLOB 0 PVRCore_DIR)
			message(STATUS "FindPVRCore: Found PVRCore_DIR=${PVRCore_DIR}")
		else()
			message(WARNING "PVRCore: No build directory found matching ${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRCore/build-android/.cxx/*/*/${ANDROID_ABI}/PVRCore")
		endif()
	endif()
endif()

if(NOT TARGET PVRCore)
	find_package(PVRCore REQUIRED CONFIG)
endif()