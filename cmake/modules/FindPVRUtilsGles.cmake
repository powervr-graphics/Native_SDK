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

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		# Allow finding packages in the host file system
		set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
		string(TOLOWER "${CMAKE_BUILD_TYPE}" PVR_ANDROID_BUILD_TYPE)

		# Use wildcard for build type to handle Debug/debug casing differences
		file(GLOB PVRUtilsGles_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRUtils/OpenGLES/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/PVRUtilsGles")
		
        # If not found, try original build type
		if(NOT PVRUtilsGles_DIR_GLOB)
			file(GLOB PVRUtilsGles_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRUtils/OpenGLES/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/PVRUtilsGles")
		endif()

		# The glob will return a list, but there should only be one match.
		if(PVRUtilsGles_DIR_GLOB)
			list(GET PVRUtilsGles_DIR_GLOB 0 PVRUtilsGles_DIR)
		else()
			message(STATUS "PVRUtilsGles: No build directory found matching ${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRUtils/OpenGLES/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/PVRUtilsGles")
		endif()
	endif()
endif()

if(NOT TARGET PVRUtilsGles)
	find_package(PVRUtilsGles REQUIRED CONFIG)
    if(PVRUtilsGles_FOUND)
        # Ensure the framework source directory is in the include path.
        get_filename_component(PVR_FRAMEWORK_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework" ABSOLUTE)
        get_filename_component(PVR_SDK_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../include" ABSOLUTE)
        
        set_property(TARGET PVRUtilsGles APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${PVR_FRAMEWORK_DIR}" "${PVR_SDK_INCLUDE_DIR}")
    endif()
endif()