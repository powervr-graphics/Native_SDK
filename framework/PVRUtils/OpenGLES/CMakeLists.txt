cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)
project(PVRUtilsGles)
include(GNUInstallDirs)
include(../../FrameworkCommon.cmake)
# Copyright (c) Imagination Technologies Limited.

# Prevent PVRUtilsGles being added multiple times
if(TARGET PVRUtilsGles)
	return()
endif()

# Add PVRCore
add_subdirectory(../../PVRCore PVRCore)

message(STATUS "Adding PVRUtilsGles")

set(PVRUtils_HEADERS 
	../ArialBoldFont.h
	../MultiObject.h
	../ImaginationLogo.h
	../PBRUtils.h
	../PVRUtilsGles.h
	../PVRUtilsTypes.h
	../StructuredMemory.h)

# PVRUtilsGles include files
set(PVRUtilsGles_HEADERS 
	../../../include/DynamicEgl.h
	../../../include/DynamicGles.h
	BindingsGles.h
	ConvertToGlesTypes.h
	ErrorsGles.h
	HelperGles.h
	ModelGles.h
	PBRUtilsGles.h
	ShaderUtilsGles.h
	SpriteGles.h
	TextureUtilsGles.h
	UIRendererGles.h
	UIRendererShaders_ES.h)

# PVRUtilsGles sources
set(PVRUtilsGles_SRC 
	ConvertToGlesTypes.cpp
	ErrorsGles.cpp
	ModelGles.cpp
	PBRUtilsGles.cpp
	ShaderUtilsGles.cpp
	SpriteGles.cpp
	TextureUtilsGles.cpp
	UIRendererGles.cpp)

# Append EGL sources to PVRUtils
list(APPEND PVRUtilsGles_SRC
	../EGL/EglPlatformContext.cpp)
list(APPEND PVRUtilsGles_EGL_HEADERS
	../EGL/EglPlatformContext.h
	../EGL/EglPlatformHandles.h)

set(PVRUtilsGles_PRIVATE_LINK_LIBS "")
set(PVRUtilsGles_INCLUDE_DIRECTORIES 
	${CMAKE_CURRENT_LIST_DIR}/../../../include
	${CMAKE_CURRENT_LIST_DIR}/../../)
set(PVRUtilsGles_PRIVATE_INCLUDE_DIRECTORIES "")

# Handle the UNIX platform specific api specific libraries
if(UNIX)
	if(PVR_WINDOW_SYSTEM AND ("${PVR_WINDOW_SYSTEM}" STREQUAL Wayland))
		list(APPEND PVRUtilsGles_PRIVATE_LINK_LIBS ${WAYLAND_EGL_LIBRARIES})
		list(APPEND PVRUtilsGles_PRIVATE_INCLUDE_DIRECTORIES ${WAYLAND_EGL_INCLUDE_DIR})
	endif()
endif()

# Create the library
add_library(PVRUtilsGles STATIC 
	${PVRUtilsGles_SRC} 
	${PVRUtilsGles_HEADERS}
	${PVRUtils_HEADERS} 
	${PVRUtilsGles_EAGL_HEADERS}
	${PVRUtilsGles_EGL_HEADERS})

# Link PVRUtilsGLES with PVRAssets and PVRCore, along with the platform dependant libraries
target_link_libraries(PVRUtilsGles 
	PUBLIC
		PVRAssets 
		PVRCore
	PRIVATE
		${PVRUtilsGles_PRIVATE_LINK_LIBS})

# Handle the PVR_WINDOW_SYSTEM variable on Unix
if(PVR_WINDOW_SYSTEM)
	set(PVR_WINDOW_SYSTEM_DEFINE "" CACHE INTERNAL "")
	# Validate the use of -DPVR_WINDOW_SYSTEM
	if(${PVR_WINDOW_SYSTEM} STREQUAL X11 OR ${PVR_WINDOW_SYSTEM} STREQUAL XCB OR ${PVR_WINDOW_SYSTEM} STREQUAL Wayland OR ${PVR_WINDOW_SYSTEM} STREQUAL NullWS OR ${PVR_WINDOW_SYSTEM} STREQUAL Screen)
		set(PVR_WINDOW_SYSTEM_DEFINE "${PVR_WINDOW_SYSTEM}" CACHE INTERNAL "")
	else()
		message(FATAL_ERROR "Unrecognised PVR_WINDOW_SYSTEM: Valid values are NullWS, X11, XCB, Wayland.")
	endif()
	target_compile_definitions(PVRUtilsGles PUBLIC ${PVR_WINDOW_SYSTEM_DEFINE})
endif()

# Allow PVRUtilsGles to see the rest of the framework and it's platform specific headers
target_include_directories(PVRUtilsGles PUBLIC ${PVRUtilsGles_INCLUDE_DIRECTORIES})

# Apply the common framework compile definitions to PVRUtilsGles
# This sets certain warnings on/off and adds appropriate
# debug and release flags in the compiler and linker  
apply_framework_compile_options_to_target(PVRUtilsGles)