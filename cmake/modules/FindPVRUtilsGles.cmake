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

set(PVRUtilsGles_PREBUILT_FOUND FALSE)

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
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
            
            # Check if the library file actually exists before we trust the config
            # Check inside the dir and in the parent dir (common layout variations)
            if(EXISTS "${PVRUtilsGles_DIR}/libPVRUtilsGles.a" OR EXISTS "${PVRUtilsGles_DIR}/../libPVRUtilsGles.a")
                set(PVRUtilsGles_PREBUILT_FOUND TRUE)
            else()
                message(STATUS "PVRUtilsGles: Config dir found at ${PVRUtilsGles_DIR} but library file (libPVRUtilsGles.a) missing. Skipping prebuilt.")
            endif()
		endif()
	endif()
endif()

if(PVRUtilsGles_PREBUILT_FOUND)
    find_package(PVRUtilsGles REQUIRED CONFIG)
    
    if(TARGET PVRUtilsGles)
        message(STATUS "PVRUtilsGles: Prebuilt target successfully imported.")
        # Ensure the framework source directory is in the include path.
        get_filename_component(PVR_FRAMEWORK_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework" ABSOLUTE)
        get_filename_component(PVR_SDK_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../include" ABSOLUTE)
        
        set_property(TARGET PVRUtilsGles APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${PVR_FRAMEWORK_DIR}" "${PVR_SDK_INCLUDE_DIR}")
    else()
        message(FATAL_ERROR "PVRUtilsGles: find_package succeeded but target 'PVRUtilsGles' is missing.")
    endif()
else()
    # Fallback to building from source
    # We must ensure PVR_PREBUILT_DEPENDENCIES is OFF for the add_subdirectory call
    # to avoid recursion or incorrect logic inside the child CMakeLists.txt
    message(STATUS "PVRUtilsGles: Building from source...")
    
    set(PVR_PREBUILT_DEPENDENCIES_CACHED ${PVR_PREBUILT_DEPENDENCIES})
    set(PVR_PREBUILT_DEPENDENCIES OFF)
    
	set(PVRUtilsGles_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRUtils/OpenGLES")
	if(EXISTS "${PVRUtilsGles_SOURCE_DIR}/CMakeLists.txt")
        # Check if target already exists (shouldn't happen if we skipped find_package)
        if(NOT TARGET PVRUtilsGles)
		    add_subdirectory("${PVRUtilsGles_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/framework/PVRUtils/OpenGLES")
        endif()
	else()
		message(FATAL_ERROR "PVRUtilsGles: Could not find source at ${PVRUtilsGles_SOURCE_DIR}")
	endif()
    
    set(PVR_PREBUILT_DEPENDENCIES ${PVR_PREBUILT_DEPENDENCIES_CACHED})
endif()