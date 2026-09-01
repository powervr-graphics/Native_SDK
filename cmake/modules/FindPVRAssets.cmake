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
include("${CMAKE_CURRENT_LIST_DIR}/../utilities/android_utils.cmake")

if(NOT TARGET PVRCore)
	find_dependency(PVRCore REQUIRED MODULE)
endif()

if(NOT TARGET tinygltf)
	find_dependency(tinygltf REQUIRED MODULE)
endif()

# Try to find prebuilt if configured


if(PVR_PREBUILT_DEPENDENCIES AND ANDROID)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
    pvr_find_android_build_path(PVRAssets_PREBUILT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRAssets/build-android" "PVRAssets")
    if(PVRAssets_PREBUILT_DIR)
        list(APPEND CMAKE_PREFIX_PATH "${PVRAssets_PREBUILT_DIR}")
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
if(TARGET PVRAssets)
    set(PVRAssets_FOUND TRUE)
endif()


if(TARGET PVRAssets)
    get_target_property(_loc_debug PVRAssets IMPORTED_LOCATION_DEBUG)
    if(_loc_debug AND NOT EXISTS "${_loc_debug}")
        message(STATUS "Restoring PVRAssets IMPORTED_LOCATION_DEBUG")
        get_filename_component(_fw_dir "${CMAKE_CURRENT_LIST_DIR}/../../framework" ABSOLUTE)
        # Find the .a file!
        file(GLOB_RECURSE _a_file "${_fw_dir}/*/build-android/.cxx/Debug/*/${ANDROID_ABI}/libPVRAssets.a")
        if(_a_file)
            list(GET _a_file 0 _a_file_path)
            set_property(TARGET PVRAssets PROPERTY IMPORTED_LOCATION_DEBUG "${_a_file_path}")
        endif()
    endif()
endif()
