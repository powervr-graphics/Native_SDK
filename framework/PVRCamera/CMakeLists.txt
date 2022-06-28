cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)
project(PVRCamera)
include(GNUInstallDirs)
include(../FrameworkCommon.cmake)
# Copyright (c) Imagination Technologies Limited.

# Prevent PVRCamera being added multiple times
if(TARGET PVRCamera)
	return()
endif()

# Add PVRCore
add_subdirectory(../PVRCore PVRCore)

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

# Link PVRCamera to PVRCore and the platform specific private libs
target_link_libraries(PVRCamera
	PUBLIC
		PVRCore
	PRIVATE
		${PVRCamera_PRIVATE_LINK_LIBS})
	
# Allow PVRCamera to see the rest of the framework	
target_include_directories(PVRCamera PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/..
	${CMAKE_CURRENT_LIST_DIR}/../../include)

# Apply the common framework compile definitions to PVRCamera
# This sets certain warnings on/off and adds appropriate
# debug and release flags in the compiler and linker  
apply_framework_compile_options_to_target(PVRCamera)