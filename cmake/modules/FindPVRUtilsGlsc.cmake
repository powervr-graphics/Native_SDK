# FindPVRUtilsGles.cmake
#
# Finds the PVRUtilsGles library and its dependencies
#
# This will define the following imported targets
#     PVRCore
#     PVRAssets
#     PVRUtilsGles

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(CMakeFindDependencyMacro)

if(NOT TARGET PVRCore)
	find_dependency(PVRCore REQUIRED MODULE)
endif()

if(NOT TARGET PVRAssets)
	find_dependency(PVRAssets REQUIRED MODULE)
endif()

if(NOT TARGET PVRUtilsGlsc)
	find_package(PVRUtilsGlsc REQUIRED CONFIG)
endif()
if(TARGET PVRUtilsGlsc)
    set(PVRUtilsGlsc_FOUND TRUE)
endif()


if(TARGET PVRUtilsGlsc)
    get_target_property(_loc_debug PVRUtilsGlsc IMPORTED_LOCATION_DEBUG)
    if(_loc_debug AND NOT EXISTS "${_loc_debug}")
        message(STATUS "Restoring PVRUtilsGlsc IMPORTED_LOCATION_DEBUG")
        get_filename_component(_fw_dir "${CMAKE_CURRENT_LIST_DIR}/../../framework" ABSOLUTE)
        # Find the .a file!
        file(GLOB_RECURSE _a_file "${_fw_dir}/*/build-android/.cxx/Debug/*/${ANDROID_ABI}/libPVRUtilsGlsc.a")
        if(_a_file)
            list(GET _a_file 0 _a_file_path)
            set_property(TARGET PVRUtilsGlsc PROPERTY IMPORTED_LOCATION_DEBUG "${_a_file_path}")
        endif()
    endif()
endif()
