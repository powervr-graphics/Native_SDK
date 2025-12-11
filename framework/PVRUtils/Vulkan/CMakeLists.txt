cmake_minimum_required(VERSION 3.10)
# Copyright (c) Imagination Technologies Limited.

project(PVRUtilsVk)
include(GNUInstallDirs)
include(CMakeFindDependencyMacro)
include(../../FrameworkCommon.cmake)

set(PVRUtilsVk_PRIVATE_LINK_LIBS glslang SPIRV)
set(PVRUtilsVk_INCLUDE_DIRECTORIES ${PVR_FRAMEWORK_ROOT}/PVRUtils)

# Prevent PVRUtilsVulkan being added multiple times
if(TARGET PVRUtilsVk)
	return()
endif()

set(PVRUtils_HEADERS 
	../ArialBoldFont.h
	../MultiObject.h
	../ImaginationLogo.h
	../PBRUtils.h
	../PVRUtilsTypes.h
	../StructuredMemory.h)

# PVRUtilsVk sources
set(PVRUtilsVk_HEADERS
	../ArialBoldFont.h
	../MultiObject.h
	../ImaginationLogo.h
	../PBRUtils.h
	../PVRUtilsTypes.h
	../PVRUtilsVk.h
	../StructuredMemory.h
	AccelerationStructure.h
	AsynchronousVk.h
	ConvertToPVRVkTypes.h
	HelperVk.h
	MemoryAllocator.h
	PBRUtilsVk.h
	PBRUtilsVertShader.h
	PBRUtilsIrradianceFragShader.h
	PBRUtilsPrefilteredFragShader.h
	ShaderUtilsVk.h
	SpriteVk.h
	UIRendererFragShader.h
	UIRendererVertShader.h
	UIRendererVk.h)
	
# PVRUtilsVk sources
set(PVRUtilsVk_SRC
	AccelerationStructure.cpp
	HelperVk.cpp
	MemoryAllocator.cpp
	PBRUtilsVk.cpp
	ShaderUtilsVk.cpp
	SpriteVk.cpp
	UIRendererVk.cpp)

# Create the library
add_library(PVRUtilsVk STATIC 
	${PVRUtilsVk_SRC}
	${PVRUtils_HEADERS}
	${PVRUtilsVk_HEADERS})

if(PVR_PREBUILT_DEPENDENCIES)
    # CMAKE_MODULE_PATH handled by FrameworkCommon.cmake
	find_dependency(PVRCore MODULE REQUIRED)
	find_dependency(PVRAssets MODULE REQUIRED)
	find_dependency(PVRVk MODULE REQUIRED)
	find_dependency(glslang MODULE REQUIRED)
	find_dependency(SPIRV MODULE REQUIRED)
	find_dependency(VulkanMemoryAllocator MODULE REQUIRED)
endif()

apply_framework_compile_options_to_target(PVRUtilsVk)

target_link_libraries(PVRUtilsVk 
	PUBLIC
		PVRAssets 
		PVRCore
		PVRVk
		VulkanMemoryAllocator
	PRIVATE
		glslang 
		SPIRV)

target_include_directories(PVRUtilsVk 
	PUBLIC 
		"$<BUILD_INTERFACE:${PVRUtilsVk_INCLUDE_DIRECTORIES}>"
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    # Add PVR_FRAMEWORK_ROOT so includes like <PVRUtils/Vulkan/Header.h> might work if they exist, 
    # but based on PVRCore, headers are often relative.
    # The variable PVRUtilsVk_INCLUDE_DIRECTORIES above points to ${PVR_FRAMEWORK_ROOT}/PVRUtils
    # This seems to intend that users include <Vulkan/Header.h> or just <Header.h> if they add that dir?
    # Actually PVRUtilsVk_INCLUDE_DIRECTORIES is set to ${PVR_FRAMEWORK_ROOT}/PVRUtils.
    # So headers in PVRUtils/Vulkan are typically included as #include "PVRUtils/Vulkan/..." ?
    # Let's add the framework root generally to be safe and consistent with PVRCore.
    "$<BUILD_INTERFACE:${PVR_FRAMEWORK_ROOT}>"
)

# Use the consolidated install function
pvr_install_target(TARGET PVRUtilsVk HEADERS ${PVRUtilsVk_HEADERS})
# Install extra headers
if(PVR_ENABLE_INSTALL)
	install(FILES ${PVRUtils_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PVRUtils OPTIONAL)
endif()
