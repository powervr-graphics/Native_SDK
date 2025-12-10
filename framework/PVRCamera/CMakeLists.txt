cmake_minimum_required(VERSION 3.10)
# Copyright (c) Imagination Technologies Limited.

project(PVRCamera)
include(GNUInstallDirs)
include(CMakeFindDependencyMacro)
include(../FrameworkCommon.cmake)

# Prevent PVRCamera being added multiple times
if(TARGET PVRCamera)
	return()
endif()

message(STATUS "Adding PVRCamera")

# PVRCamera include files
set(PVRCamera_HEADERS
	PVRCamera.h
	CameraInterface.h
	../../include/DynamicEgl.h
	../../include/DynamicGles.h)

# Add platform specific PVRCamera sources
if(ANDROID)
	list(APPEND PVRCamera_SRC CameraInterface_Android.cpp)
else()
	list(APPEND PVRCamera_SRC CameraInterface_Dummy.cpp)
endif()

# Create the library
add_library(PVRCamera STATIC ${PVRCamera_SRC} ${PVRCamera_HEADERS})

if(PVR_PREBUILT_DEPENDENCIES)
	list(APPEND CMAKE_MODULE_PATH "${PVR_SDK_ROOT}/cmake/modules")
	find_dependency(PVRCore MODULE REQUIRED)
endif()

apply_framework_compile_options_to_target(PVRCamera)

# Link PVRCamera to PVRCore and the platform specific private libs
target_link_libraries(PVRCamera
	PUBLIC
		PVRCore
	PRIVATE
		${PVRCamera_PRIVATE_LINK_LIBS})

# Use generator expressions to ensure export compatibility
target_include_directories(PVRCamera 
	PUBLIC
		$<BUILD_INTERFACE:${PVR_FRAMEWORK_ROOT}>
		$<BUILD_INTERFACE:${PVR_SDK_ROOT}/include>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Use the consolidated install function
pvr_install_target(TARGET PVRCamera HEADERS ${PVRCamera_HEADERS})
