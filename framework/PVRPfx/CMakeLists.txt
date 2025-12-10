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
	list(APPEND CMAKE_MODULE_PATH "${PVR_SDK_ROOT}/cmake/modules")
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
	${PVR_FRAMEWORK_ROOT}
	${PVR_SDK_ROOT}/include)

# Explicitly add the parent directory to the include path for this directory
# This ensures that #include "PVRPfx/EffectVk.h" works correctly
target_include_directories(PVRPfx PRIVATE "${PVR_FRAMEWORK_ROOT}")

target_include_directories(PVRPfx 
	PUBLIC 
		"$<BUILD_INTERFACE:${PVRPfx_INCLUDE_DIRECTORIES}>"
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Use the consolidated install function
pvr_install_target(TARGET PVRPfx HEADERS ${PVRPfx_HEADERS})
