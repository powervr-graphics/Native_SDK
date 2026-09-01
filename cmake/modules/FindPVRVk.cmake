# FindPVRVk.cmake
#
# Finds the PVRVk library
#
# This will define the following imported targets
#     PVRVk



if(PVR_PREBUILT_DEPENDENCIES AND ANDROID)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
    pvr_find_android_build_path(PVRVk_PREBUILT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRVk/build-android" "PVRVk")
    if(PVRVk_PREBUILT_DIR)
        list(APPEND CMAKE_PREFIX_PATH "${PVRVk_PREBUILT_DIR}")
    endif()
endif()

if(NOT TARGET PVRVk)

	# Try to find the package configuration
	find_package(PVRVk CONFIG QUIET)
	
    if(PVRVk_FOUND)
        message(STATUS "PVRVk: Package configuration found.")
        # Ensure the framework source directory is in the include path.
        get_filename_component(PVR_FRAMEWORK_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework" ABSOLUTE)
        get_filename_component(PVR_SDK_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../include" ABSOLUTE)
        
        set_property(TARGET PVRVk APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${PVR_FRAMEWORK_DIR}" "${PVR_SDK_INCLUDE_DIR}")
    else()
        message(STATUS "PVRVk: Prebuilt package not found. Attempting to build from source...")
        set(PVRVk_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRVk")
        if(EXISTS "${PVRVk_SOURCE_DIR}/CMakeLists.txt")
            add_subdirectory("${PVRVk_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/framework/PVRVk")
        else()
            message(FATAL_ERROR "PVRVk: Could not find prebuilt package AND could not find source at ${PVRVk_SOURCE_DIR}")
        endif()
    endif()
endif()
if(TARGET PVRVk)
    set(PVRVk_FOUND TRUE)
endif()


if(TARGET PVRVk)
    get_target_property(_loc_debug PVRVk IMPORTED_LOCATION_DEBUG)
    if(_loc_debug AND NOT EXISTS "${_loc_debug}")
        message(STATUS "Restoring PVRVk IMPORTED_LOCATION_DEBUG")
        get_filename_component(_fw_dir "${CMAKE_CURRENT_LIST_DIR}/../../framework" ABSOLUTE)
        # Find the .a file!
        file(GLOB_RECURSE _a_file "${_fw_dir}/*/build-android/.cxx/Debug/*/${ANDROID_ABI}/libPVRVk.a")
        if(_a_file)
            list(GET _a_file 0 _a_file_path)
            set_property(TARGET PVRVk PROPERTY IMPORTED_LOCATION_DEBUG "${_a_file_path}")
        endif()
    endif()
endif()
