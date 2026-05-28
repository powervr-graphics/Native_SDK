# FindPVRCamera.cmake
#
# Finds the PVRCamera library and its dependencies
#
# This will define the following imported targets
#     PVRCore
#     PVRCamera

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(CMakeFindDependencyMacro)

# Include helper for Android paths
include("${CMAKE_CURRENT_LIST_DIR}/../utilities/android_utils.cmake")

if(NOT TARGET PVRCore)
	find_dependency(PVRCore REQUIRED MODULE)
endif()



if(PVR_PREBUILT_DEPENDENCIES AND ANDROID)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
    pvr_find_android_build_path(PVRCamera_PREBUILT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRCamera/build-android" "PVRCamera")
    if(PVRCamera_PREBUILT_DIR)
        list(APPEND CMAKE_PREFIX_PATH "${PVRCamera_PREBUILT_DIR}")
    endif()
endif()

if(NOT TARGET PVRCamera)

	find_package(PVRCamera CONFIG QUIET)

    if(PVRCamera_FOUND)
        message(STATUS "PVRCamera: Package configuration found.")
    else()
        message(STATUS "PVRCamera: Prebuilt package not found. Attempting to build from source...")
        set(PVRCamera_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRCamera")
        if(EXISTS "${PVRCamera_SOURCE_DIR}/CMakeLists.txt")
            add_subdirectory("${PVRCamera_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/framework/PVRCamera")
        else()
            message(FATAL_ERROR "PVRCamera: Could not find prebuilt package AND could not find source at ${PVRCamera_SOURCE_DIR}")
        endif()
    endif()
endif()
if(TARGET PVRCamera)
    set(PVRCamera_FOUND TRUE)
endif()
