# FindPVRPfx.cmake
#
# Finds the PVRPfx library and its dependencies
#
# This will define the following imported targets
#     PVRUtilsVk
#     PVRPfx

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(CMakeFindDependencyMacro)

# Include helper for Android paths
include("${CMAKE_CURRENT_LIST_DIR}/../utilities/android_utils.cmake")

if(NOT TARGET PVRUtilsVk)
	find_dependency(PVRUtilsVk REQUIRED MODULE)
endif()



if(PVR_PREBUILT_DEPENDENCIES AND ANDROID)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
    pvr_find_android_build_path(PVRPfx_PREBUILT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRPfx/build-android" "PVRPfx")
    if(PVRPfx_PREBUILT_DIR)
        list(APPEND CMAKE_PREFIX_PATH "${PVRPfx_PREBUILT_DIR}")
    endif()
endif()

if(NOT TARGET PVRPfx)

	# Try to find the package configuration
	find_package(PVRPfx CONFIG QUIET)

    if(PVRPfx_FOUND)
        message(STATUS "PVRPfx: Package configuration found.")
    else()
        message(STATUS "PVRPfx: Prebuilt package not found. Attempting to build from source...")
        set(PVRPfx_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRPfx")
        if(EXISTS "${PVRPfx_SOURCE_DIR}/CMakeLists.txt")
            add_subdirectory("${PVRPfx_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/framework/PVRPfx")
        else()
            message(FATAL_ERROR "PVRPfx: Could not find prebuilt package AND could not find source at ${PVRPfx_SOURCE_DIR}")
        endif()
    endif()
endif()
if(TARGET PVRPfx)
    set(PVRPfx_FOUND TRUE)
endif()


if(TARGET PVRPfx)
    get_target_property(_loc_debug PVRPfx IMPORTED_LOCATION_DEBUG)
    if(_loc_debug AND NOT EXISTS "${_loc_debug}")
        message(STATUS "Restoring PVRPfx IMPORTED_LOCATION_DEBUG")
        get_filename_component(_fw_dir "${CMAKE_CURRENT_LIST_DIR}/../../framework" ABSOLUTE)
        # Find the .a file!
        file(GLOB_RECURSE _a_file "${_fw_dir}/*/build-android/.cxx/Debug/*/${ANDROID_ABI}/libPVRPfx.a")
        if(_a_file)
            list(GET _a_file 0 _a_file_path)
            set_property(TARGET PVRPfx PROPERTY IMPORTED_LOCATION_DEBUG "${_a_file_path}")
        endif()
    endif()
endif()
