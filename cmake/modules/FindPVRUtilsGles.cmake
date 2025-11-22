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

# Helper function to check if the target is valid
function(check_pvrutilsgles_target)
    if(TARGET PVRUtilsGles)
        get_target_property(type PVRUtilsGles TYPE)
        if(NOT "${type}" STREQUAL "INTERFACE_LIBRARY")
            # For static/shared libs, check if we have a location
            get_target_property(loc PVRUtilsGles IMPORTED_LOCATION)
            if(NOT loc)
                get_target_property(loc_rel PVRUtilsGles IMPORTED_LOCATION_RELEASE)
                get_target_property(loc_deb PVRUtilsGles IMPORTED_LOCATION_DEBUG)
                if(NOT loc_rel AND NOT loc_deb)
                    message(STATUS "PVRUtilsGles: Target exists but has no location. Considering invalid.")
                    return() # Invalid
                endif()
            endif()
        endif()
        set(PVRUTILSGLES_TARGET_VALID TRUE PARENT_SCOPE)
    endif()
endfunction()

set(PVRUTILSGLES_TARGET_VALID FALSE)

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
		string(TOLOWER "${CMAKE_BUILD_TYPE}" PVR_ANDROID_BUILD_TYPE)

        # Try finding prebuilt
		file(GLOB PVRUtilsGles_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRUtils/OpenGLES/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/PVRUtilsGles")
		if(NOT PVRUtilsGles_DIR_GLOB)
			file(GLOB PVRUtilsGles_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRUtils/OpenGLES/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/PVRUtilsGles")
		endif()

		if(PVRUtilsGles_DIR_GLOB)
			list(GET PVRUtilsGles_DIR_GLOB 0 PVRUtilsGles_DIR)
            
            find_package(PVRUtilsGles CONFIG QUIET)
            
            check_pvrutilsgles_target()
            
            if(PVRUTILSGLES_TARGET_VALID)
                message(STATUS "PVRUtilsGles: Found valid prebuilt target.")
                # Fix include paths
                get_filename_component(PVR_FRAMEWORK_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework" ABSOLUTE)
                get_filename_component(PVR_SDK_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../include" ABSOLUTE)
                set_property(TARGET PVRUtilsGles APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${PVR_FRAMEWORK_DIR}" "${PVR_SDK_INCLUDE_DIR}")
            endif()
		endif()
	endif()
endif()

if(NOT PVRUTILSGLES_TARGET_VALID)
    # If we are here, either we aren't using prebuilts, or we couldn't find/validate them.
    # We must ensure we don't have a broken IMPORTED target preventing add_subdirectory.
    if(TARGET PVRUtilsGles)
        message(STATUS "PVRUtilsGles: Existing target found but deemed invalid/broken. Cannot simply remove it. Attempting to force source build anyway.")
        # Note: We can't remove the target. If it exists and is broken, we are in a bad state.
        # However, usually if find_package failed to find a valid config, it wouldn't define the target.
    endif()

	message(STATUS "PVRUtilsGles: Attempting to build from source...")
    
    # Save state and force PVR_PREBUILT_DEPENDENCIES OFF for the subdirectory build
    set(PVR_PREBUILT_DEPENDENCIES_CACHED ${PVR_PREBUILT_DEPENDENCIES})
    set(PVR_PREBUILT_DEPENDENCIES OFF)
    
	set(PVRUtilsGles_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRUtils/OpenGLES")
	if(EXISTS "${PVRUtilsGles_SOURCE_DIR}/CMakeLists.txt")
		add_subdirectory("${PVRUtilsGles_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/framework/PVRUtils/OpenGLES")
        
        # After add_subdirectory, the target MUST exist.
        if(TARGET PVRUtilsGles)
             message(STATUS "PVRUtilsGles: Source build successful.")
        else()
             message(FATAL_ERROR "PVRUtilsGles: add_subdirectory called but target PVRUtilsGles still not defined!")
        endif()
	else()
		message(FATAL_ERROR "PVRUtilsGles: Could not find source at ${PVRUtilsGles_SOURCE_DIR}")
	endif()
    
    set(PVR_PREBUILT_DEPENDENCIES ${PVR_PREBUILT_DEPENDENCIES_CACHED})
endif()