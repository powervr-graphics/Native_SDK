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
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../cmake/modules")
	find_dependency(PVRCore MODULE REQUIRED)
endif()

apply_framework_compile_options_to_target(PVRCamera)

# Link PVRCamera to PVRCore and the platform specific private libs
target_link_libraries(PVRCamera
	PUBLIC
		PVRCore
	PRIVATE
		${PVRCamera_PRIVATE_LINK_LIBS})
	
set(PVRCamera_INCLUDE_DIRECTORIES 
	${CMAKE_CURRENT_LIST_DIR}/..
	${CMAKE_CURRENT_LIST_DIR}/../../include)
		
target_include_directories(PVRCamera 
	PUBLIC 
		"$<BUILD_INTERFACE:${PVRCamera_INCLUDE_DIRECTORIES}>"
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

if(PVR_ENABLE_INSTALL)
	install(TARGETS PVRCamera EXPORT PVRCameraTargets
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		OPTIONAL)
	foreach(file ${PVRCamera_HEADERS})
		get_filename_component(dir ${file} DIRECTORY)
		install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PVRCamera/${dir})
	endforeach()
	install(EXPORT PVRCameraTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PVRCamera OPTIONAL)

	export(EXPORT PVRCameraTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/PVRCamera/PVRCameraTargets.cmake")
	# build tree package config
	file(COPY cmake/PVRCameraConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PVRCamera/" NO_SOURCE_PERMISSIONS)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PVRCamera/PVRCameraConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/PVRCamera/PVRCameraConfig.cmake" COPYONLY)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/PVRCamera/PVRCameraTargets.cmake" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PVRCamera OPTIONAL)
endif()
