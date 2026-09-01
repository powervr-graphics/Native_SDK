# FindPVRUtilsCL.cmake
#
# Finds the PVRUtilsCL library and its dependencies
#
# This will define the following imported targets
#     PVRCore
#	  PVRUtilsCL

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(CMakeFindDependencyMacro)
include("${CMAKE_CURRENT_LIST_DIR}/../utilities/android_utils.cmake")

if(NOT TARGET PVRCore)
	find_dependency(PVRCore REQUIRED MODULE)
endif()

# Try to find prebuilt if configured


if(PVR_PREBUILT_DEPENDENCIES AND ANDROID)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
    pvr_find_android_build_path(PVRUtilsCL_PREBUILT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRUtils/OpenCL/build-android" "PVRUtilsCL")
    if(PVRUtilsCL_PREBUILT_DIR)
        list(APPEND CMAKE_PREFIX_PATH "${PVRUtilsCL_PREBUILT_DIR}")
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
if(TARGET PVRUtilsCL)
    set(PVRUtilsCL_FOUND TRUE)
endif()


if(TARGET PVRUtilsCL)
    get_target_property(_loc_debug PVRUtilsCL IMPORTED_LOCATION_DEBUG)
    if(_loc_debug AND NOT EXISTS "${_loc_debug}")
        message(STATUS "Restoring PVRUtilsCL IMPORTED_LOCATION_DEBUG")
        get_filename_component(_fw_dir "${CMAKE_CURRENT_LIST_DIR}/../../framework" ABSOLUTE)
        # Find the .a file!
        file(GLOB_RECURSE _a_file "${_fw_dir}/*/build-android/.cxx/Debug/*/${ANDROID_ABI}/libPVRUtilsCL.a")
        if(_a_file)
            list(GET _a_file 0 _a_file_path)
            set_property(TARGET PVRUtilsCL PROPERTY IMPORTED_LOCATION_DEBUG "${_a_file_path}")
        endif()
    endif()
endif()
