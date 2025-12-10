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
	list(APPEND CMAKE_MODULE_PATH "${PVR_SDK_ROOT}/cmake/modules")
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
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Use the consolidated install function
pvr_install_target(TARGET PVRUtilsVk HEADERS ${PVRUtilsVk_HEADERS})
# Install extra headers
if(PVR_ENABLE_INSTALL)
	install(FILES ${PVRUtils_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PVRUtils OPTIONAL)
endif()
