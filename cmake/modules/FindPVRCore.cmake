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
        set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

        # Use wildcard for build type to handle Debug/debug casing differences
        file(GLOB PVRCore_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRCamera/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/PVRCore")

        # The glob will return a list, but there should only be one match.
        if(PVRCore_DIR_GLOB)
            list(GET PVRCore_DIR_GLOB 0 PVRCore_DIR)
        else()
            message(STATUS "PVRCore: No build directory found matching ${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRCamera/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/PVRCore")
        endif()
	endif()
endif()

if(NOT TARGET PVRCore)
	find_package(PVRCore REQUIRED CONFIG)
endif()
