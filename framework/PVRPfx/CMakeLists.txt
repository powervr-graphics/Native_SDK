cmake_minimum_required(VERSION 3.10)
# Copyright (c) Imagination Technologies Limited.

project(PVRPfx)

include(GNUInstallDirs)
include(CMakeFindDependencyMacro)
include(../FrameworkCommon.cmake)

message(STATUS "Adding PVRPfx")

# PVRPfx include files
set(PVRPfx_HEADERS
	EffectVk.h
	RenderManagerVk.h)

# PVRPfx sources
set(PVRPfx_SRC
	EffectVk.cpp
	RenderManagerVk.cpp)

# Create the library
add_library(PVRPfx STATIC ${PVRPfx_SRC} ${PVRPfx_HEADERS})

if(PVR_PREBUILT_DEPENDENCIES)
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../cmake/modules")
	find_dependency(PVRUtilsVk MODULE REQUIRED)
	find_dependency(pugixml MODULE REQUIRED)
	find_dependency(VulkanMemoryAllocator MODULE REQUIRED)
endif()

apply_framework_compile_options_to_target(PVRPfx)

target_link_libraries(PVRPfx
	PUBLIC
		PVRUtilsVk
		VulkanMemoryAllocator
	PRIVATE
		pugixml)

set(PVRPfx_INCLUDE_DIRECTORIES 
	${CMAKE_CURRENT_LIST_DIR}/..
	${CMAKE_CURRENT_LIST_DIR}/../../include)

target_include_directories(PVRPfx 
	PUBLIC 
		"$<BUILD_INTERFACE:${PVRPfx_INCLUDE_DIRECTORIES}>"
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

if(PVR_ENABLE_INSTALL)
	install(TARGETS PVRPfx EXPORT PVRPfxTargets
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
		OPTIONAL)
	foreach(file ${PVRPfx_HEADERS})
		get_filename_component(dir ${file} DIRECTORY)
		install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PVRPfx/${dir})
	endforeach()

	export(EXPORT PVRPfxTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/PVRPfx/PVRPfxTargets.cmake")
	# build tree package config
	file(COPY cmake/PVRPfxConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PVRPfx/" NO_SOURCE_PERMISSIONS)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PVRPfx/PVRPfxConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/PVRPfx/PVRPfxConfig.cmake" COPYONLY)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/PVRPfx/PVRPfxTargets.cmake" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PVRPfx OPTIONAL)
endif()
