cmake_minimum_required(VERSION 3.3)
project(PVRUtilsGlsc)
include(GNUInstallDirs)
include(../../FrameworkCommon.cmake)
# Copyright (c) Imagination Technologies Limited.

# Prevent PVRUtilsGlsc being added multiple times
if(TARGET PVRUtilsGlsc)
	return()
endif()

message(STATUS "Adding PVRUtilsGlsc")

if(PVR_BUILD_OPENGLSC_EXAMPLES AND NOT WIN32)
	add_definitions(-DSC_ENABLED=1)
else()
	add_definitions(-DSC_ENABLED=0)
endif()

set(PVRUtils_HEADERS 
	../ArialBoldFont.h
	../MultiObject.h
	../ImaginationLogo.h
	../PVRUtilsGlsc.h
	../PVRUtilsTypes.h
	../StructuredMemory.h)

# PVRUtilsGlsc include files
set(PVRUtilsGlsc_HEADERS 
	../../../include/DynamicEgl.h
	../../../include/DynamicGlsc.h
	BindingsGlsc.h
	../OpenGLES/ConvertToGlesTypes.h
	../OpenGLES/ErrorsGles.h
	../OpenGLES/HelperGles.h
	../OpenGLES/ModelGles.h
	../OpenGLES/PBRUtilsGles.h
	../OpenGLES/ShaderUtilsGles.h
	../OpenGLES/SpriteGles.h
	../OpenGLES/TextureUtilsGles.h
	../OpenGLES/UIRendererGles.h
	../OpenGLES/UIRendererShaders_ES.h)

# PVRUtilsGlsc sources
set(PVRUtilsGlsc_SRC 
	../OpenGLES/ConvertToGlesTypes.cpp
	../OpenGLES/ErrorsGles.cpp
	../OpenGLES/SpriteGles.cpp
	../OpenGLES/TextureUtilsGles.cpp
	../OpenGLES/UIRendererGles.cpp)

# Append EGL sources to PVRUtils
list(APPEND PVRUtilsGlsc_SRC
	../EGL/EglPlatformContext.cpp)
list(APPEND PVRUtilsGlsc_EGL_HEADERS
	../EGL/EglPlatformContext.h
	../EGL/EglPlatformHandles.h)

set(PVRUtilsGlsc_PRIVATE_LINK_LIBS "")
set(PVRUtilsGlsc_INCLUDE_DIRECTORIES 
	${CMAKE_CURRENT_LIST_DIR}/../../../include
	${CMAKE_CURRENT_LIST_DIR}/../../)
set(PVRUtilsGlsc_PRIVATE_INCLUDE_DIRECTORIES "")

# Handle the UNIX platform specific api specific libraries
if(UNIX)
	if(PVR_WINDOW_SYSTEM AND ("${PVR_WINDOW_SYSTEM}" STREQUAL Wayland))
		list(APPEND PVRUtilsGlsc_PRIVATE_LINK_LIBS ${WAYLAND_EGL_LIBRARIES})
		list(APPEND PVRUtilsGlsc_PRIVATE_INCLUDE_DIRECTORIES ${WAYLAND_EGL_INCLUDE_DIR})
	endif()
endif()

# Create the library
add_library(PVRUtilsGlsc STATIC 
	${PVRUtilsGlsc_SRC} 
	${PVRUtilsGlsc_HEADERS}
	${PVRUtils_HEADERS} 
	${PVRUtilsGlsc_EAGL_HEADERS}
	${PVRUtilsGlsc_EGL_HEADERS})

# Apply the common framework compile definitions to PVRUtilsGlsc
# This sets certain warnings on/off and adds appropriate
# debug and release flags in the compiler and linker  
apply_framework_compile_options_to_target(PVRUtilsGlsc)

# Link PVRUtilsGlsc with PVRAssets and PVRCore, along with the platform dependant libraries
target_link_libraries(PVRUtilsGlsc 
	PUBLIC
		PVRAssets 
		PVRCore
	PRIVATE
		${PVRUtilsGlsc_PRIVATE_LINK_LIBS})

# Handle the PVR_WINDOW_SYSTEM variable on Unix
if(PVR_WINDOW_SYSTEM)
	set(PVR_WINDOW_SYSTEM_DEFINE "" CACHE INTERNAL "")
	# Validate the use of -DPVR_WINDOW_SYSTEM
	if(${PVR_WINDOW_SYSTEM} STREQUAL X11 OR ${PVR_WINDOW_SYSTEM} STREQUAL XCB OR ${PVR_WINDOW_SYSTEM} STREQUAL Wayland OR ${PVR_WINDOW_SYSTEM} STREQUAL NullWS OR ${PVR_WINDOW_SYSTEM} STREQUAL Screen)
		set(PVR_WINDOW_SYSTEM_DEFINE "${PVR_WINDOW_SYSTEM}" CACHE INTERNAL "")
	else()
		message(FATAL_ERROR "Unrecognised PVR_WINDOW_SYSTEM: Valid values are NullWS, X11, XCB, Wayland.")
	endif()
	target_compile_definitions(PVRUtilsGlsc PUBLIC ${PVR_WINDOW_SYSTEM_DEFINE})
endif()

target_include_directories(PVRUtilsGlsc 
	PUBLIC 
		"$<BUILD_INTERFACE:${PVRUtilsGlsc_INCLUDE_DIRECTORIES}>"
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

if(PVR_ENABLE_INSTALL)
	install(TARGETS PVRUtilsGlsc EXPORT PVRUtilsGlscTargets
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		OPTIONAL)
	install(FILES ${PVRUtils_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PVRUtils OPTIONAL)
	install(FILES ${PVRUtilsGlsc_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PVRUtils/OpenGLES OPTIONAL)
	install(FILES ${PVRUtilsGlsc_EAGL_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PVRUtils/OpenGLES OPTIONAL)
	install(FILES ${PVRUtilsGlsc_EGL_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PVRUtils/OpenGLES OPTIONAL)
	install(EXPORT PVRUtilsGlscTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PVRUtilsGlsc OPTIONAL)

	export(EXPORT PVRUtilsGlscTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/PVRUtilsGlsc/PVRUtilsGlscTargets.cmake")
	# build tree package config
	file(COPY cmake/PVRUtilsGlscConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PVRUtilsGlsc/" NO_SOURCE_PERMISSIONS)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PVRUtilsGlsc/PVRUtilsGlscConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/PVRUtilsGlsc/PVRUtilsGlscConfig.cmake" COPYONLY)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/PVRUtilsGlsc/PVRUtilsGlscTargets.cmake" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PVRUtilsGles OPTIONAL)
endif()
