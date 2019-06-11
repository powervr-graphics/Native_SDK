# Common.cmake Sets up various variables for use elsewhere including:
#	${SDK_ROOT} 				: Root of the SDK
#	${CMAKE_MODULE_PATH} 		: An additional path used by find_package
#	${CMAKE_BUILD_TYPE}			: The build type. Normally Debug, Release, RelWithDebInfo, MinSizeRel. Defaults to Release
# This file includes other configuration files which setup more variables including paths/libraries, add functionality and setup compiler flags.
set(SDK_ROOT ${CMAKE_CURRENT_LIST_DIR}/.. CACHE INTERNAL "The root directory of the SDK")
# CMAKE_MODULE_PATH is used by find_package
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/modules")

# Default to release if the user passes nothing.
if(NOT CMAKE_BUILD_TYPE) 
	message("-DCMAKE_BUILD_TYPE not defined. Assuming Release")
	set(CMAKE_BUILD_TYPE "Release" CACHE INTERNAL "CMAKE_BUILD_TYPE - Specifies the build type on single-configuration generators")
endif()

# Include a set of reusable functions
include(${CMAKE_CURRENT_LIST_DIR}/Functions.cmake)

# Add platform specific definitions (FRAMEWORK_LIB_FOLDER, WS_DEFINES, PROJECT_ARCH etc.)
include(${CMAKE_CURRENT_LIST_DIR}/PlatformSdkFolders.cmake)

# Provides a number of reusable PowerVR specific interface libraries including for windowing systems and platform specific functionality required by the PowerVR SDK
include(${CMAKE_CURRENT_LIST_DIR}/PlatformLibraries.cmake)

# Sets up optimal compiler/linker flags
include(${CMAKE_CURRENT_LIST_DIR}/CompilerFlags.cmake)
