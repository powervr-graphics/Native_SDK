cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)
project(PVRPfx)
include(GNUInstallDirs)
include(../FrameworkCommon.cmake)
# Copyright (c) Imagination Technologies Limited.

# PVRPfx depends on PVRUtilsVk
add_subdirectory(../PVRUtils/Vulkan PVRUtilsVk)

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

# Link PVRPfx to PVRUtilsVK and pugixml
target_link_libraries(PVRPfx 
	PUBLIC	PVRUtilsVk VulkanMemoryAllocator 
	PRIVATE pugixml)

# Allow PVRPfx to include the rest of the framework, vulkan bindings and Vulkan Memory Allocator
target_include_directories(PVRPfx PUBLIC 
	${CMAKE_CURRENT_LIST_DIR}/../
	${CMAKE_CURRENT_LIST_DIR}/../../include)

# Apply the common framework compile definitions to PVRPfx
# This sets certain warnings on/off and adds appropriate
# debug and release flags in the compiler and linker  
apply_framework_compile_options_to_target(PVRPfx)