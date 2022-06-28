cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)
project(PVRUtilsVk)
include(GNUInstallDirs)
include(../../FrameworkCommon.cmake)
# Copyright (c) Imagination Technologies Limited.

# Prevent PVRUtilsVulkan being added multiple times
if(TARGET PVRUtilsVk)
	return()
endif()

message(STATUS "Adding PVRUtilsVulkan")

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

# Add PVRCore and PVRVK
add_subdirectory(../../PVRCore PVRCore)
add_subdirectory(../../PVRVk PVRVk)

# Create the library
add_library(PVRUtilsVk STATIC 
	${PVRUtilsVk_SRC}
	${PVRUtils_HEADERS}
	${PVRUtilsVk_HEADERS})

apply_framework_compile_options_to_target(PVRUtilsVk)

# Ensure that PVRUtils recieves the correct surface extension define 
# because PVRUtilsVk will also handle surface creation
if(PVRVK_USE_PLATFORM)
	target_compile_definitions(PVRUtilsVk PUBLIC "${PVRVK_USE_PLATFORM}")
endif()

target_link_libraries(PVRUtilsVk 
	PUBLIC
		PVRAssets 
		PVRCore
		PVRVk
		VulkanMemoryAllocator
	PRIVATE
		glslang 
		SPIRV)

# Allow PVRUtilsVk to include the rest of the framework VMA and the vulkan bindings
target_include_directories(PVRUtilsVk PUBLIC 
	${CMAKE_CURRENT_LIST_DIR}/../../
	${CMAKE_CURRENT_LIST_DIR}/../../../include
	${SDK_ROOT_INTERNAL_DIR}/external/VulkanMemoryAllocator/source/src)
