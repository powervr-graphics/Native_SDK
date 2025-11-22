# FindPVRUtilsCL.cmake
#
# Finds the PVRUtilsCL library and its dependencies
#
# This will define the following imported targets
#     PVRCore
#	  PVRUtilsCL

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(CMakeFindDependencyMacro)

if(NOT TARGET PVRCore)
	find_dependency(PVRCore REQUIRED MODULE)
endif()

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		# set(PVRUtilsCL_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRUtils/OpenCL/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/PVRUtilsCL")

		file(GLOB PVRUtilsCL_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRUtils/OpenCL/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/PVRUtilsCL")
		# The glob will return a list, but there should only be one match.
		list(GET PVRUtilsCL_DIR_GLOB 0 PVRUtilsCL_DIR)


	endif()
endif()

if(NOT TARGET PVRUtilsCL)
	find_package(PVRUtilsCL REQUIRED CONFIG)
endif()