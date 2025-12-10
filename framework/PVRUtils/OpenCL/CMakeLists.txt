cmake_minimum_required(VERSION 3.10)
# Copyright (c) Imagination Technologies Limited.

project(PVRUtilsCL)
include(GNUInstallDirs)
include(CMakeFindDependencyMacro)
include(../../FrameworkCommon.cmake)

message(STATUS "Adding PVRUtilsOpenCL")

set(PVRUtilsCL_HEADERS
	../../../include/DynamicOCL.h
	OpenCLUtils.h)

add_library(PVRUtilsCL INTERFACE)

if(PVR_PREBUILT_DEPENDENCIES)
	list(APPEND CMAKE_MODULE_PATH "${PVR_SDK_ROOT}/cmake/modules")
	find_dependency(PVRCore MODULE REQUIRED)
endif()

# Fix: Use generator expressions to avoid exporting absolute paths in INTERFACE_INCLUDE_DIRECTORIES
target_include_directories(PVRUtilsCL 
	INTERFACE 
		"$<BUILD_INTERFACE:${PVR_SDK_ROOT}/include>"
		"$<BUILD_INTERFACE:${PVR_FRAMEWORK_ROOT}>"
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

list(APPEND PVRUtilsCL_LINK_LIBS PVRCore)

target_link_libraries(PVRUtilsCL 
	INTERFACE
		${PVRUtilsCL_LINK_LIBS})

# Use the consolidated install function (installing INTERFACE libraries is slightly different, but pvr_install_target works for general targets)
# But since this is INTERFACE library, the standard install command in the function works fine too
pvr_install_target(TARGET PVRUtilsCL HEADERS ${PVRUtilsCL_HEADERS})
