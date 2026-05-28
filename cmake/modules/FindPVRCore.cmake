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
include("${CMAKE_CURRENT_LIST_DIR}/../utilities/android_utils.cmake")

if(NOT TARGET glm)
	find_dependency(glm REQUIRED MODULE)
endif()

if(NOT TARGET pugixml)
	find_dependency(pugixml REQUIRED MODULE)
endif()



if(PVR_PREBUILT_DEPENDENCIES AND ANDROID)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
    pvr_find_android_build_path(PVRCore_PREBUILT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRCore/build-android" "PVRCore")
    if(PVRCore_PREBUILT_DIR)
        list(APPEND CMAKE_PREFIX_PATH "${PVRCore_PREBUILT_DIR}")
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
if(TARGET PVRCore)
    set(PVRCore_FOUND TRUE)
endif()
