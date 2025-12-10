cmake_minimum_required(VERSION 3.10)
# Copyright (c) Imagination Technologies Limited.
project(PVRVk)
include(GNUInstallDirs)
include(../FrameworkCommon.cmake)

set(PVRVK_USE_PLATFORM "${PVRVK_USE_PLATFORM}" CACHE STRING "A list of Vulkan platforms of the form VK_USE_PLATFORM_* used as compile-time definitions in PVRVk")
option(PVRVK_SKIP_INSTALL "Skip installation of the PVRVk" ${PVRVK_SKIP_INSTALL})
if(NOT ${PVRVK_SKIP_INSTALL})
	set(PVRVK_ENABLE_INSTALL ON)
endif()

# Prevent PVRVk being added multiple times
if(TARGET PVRVk)
	return()
endif()

message(STATUS "Adding PVRVk")

# PVRVk include files
set(PVRVk_HEADERS
	AccelerationStructureVk.h
	ApiObjectsVk.h
	BufferVk.h
	CommandBufferVk.h
	CommandPoolVk.h
	ComputePipelineVk.h
	CommonHelpers.h
	DebugReportCallbackVk.h
	DebugUtilsMessengerVk.h
	DebugUtilsVk.h
	DescriptorSetVk.h
	DeviceMemoryVk.h
	DeviceVk.h
	DisplayModeVk.h
	DisplayVk.h
	EventVk.h
	ExtensionsVk.h
	FenceVk.h
	ForwardDecObjectsVk.h
	FramebufferVk.h
	GraphicsPipelineVk.h
	HeadersVk.h
	ImageVk.cpp
	ImageVk.h
	InstanceVk.h
	LayersVk.h
	MemoryBarrierVk.h
	PhysicalDeviceVk.h
	PipelineCacheVk.h
	PipelineConfigVk.h
	PipelineLayoutVk.h
	PipelineVk.h
	PopulateCreateInfoVk.h
	PVRVk.h
	pvrvk_vulkan_sc_minimal_wrapper.h
	pvrvk_vulkan_wrapper.h
	PVRVkObjectBaseVk.h
	QueryPoolVk.h
	QueueVk.h
	RaytracingPipelineVk.h
	RenderPassVk.h
	SamplerVk.h
	SemaphoreVk.h
	ShaderModuleVk.h
	SurfaceVk.h
	SwapchainVk.h
	TimelineSemaphoreVk.h
	TypesVk.h
	VulkanBase.h)
	  
set(PVRVk_INCLUDES 
	../../include/pvr_openlib.h
	../../include/vk_bindings.h
	../../include/vk_bindings_helper.h)

# PVRVk sources
set(PVRVk_SRC
	AccelerationStructureVk.cpp
	BufferVk.cpp
	CommandBufferVk.cpp
	CommandPoolVk.cpp
	DebugReportCallbackVk.cpp
	DebugUtilsMessengerVk.cpp
	DebugUtilsVk.cpp
	DescriptorSetVk.cpp
	DeviceVk.cpp
	DisplayModeVk.cpp
	DisplayVk.cpp
	EventVk.cpp
	ExtensionsVk.cpp
	FenceVk.cpp
	FramebufferVk.cpp
	ImageVk.cpp
	InstanceVk.cpp
	LayersVk.cpp
	PhysicalDeviceVk.cpp
	PipelineLayoutVk.cpp
	QueryPoolVk.cpp
	QueueVk.cpp
	RenderPassVk.cpp
	SamplerVk.cpp
	SemaphoreVk.cpp
	ShaderModuleVk.cpp
	SurfaceVk.cpp
	SwapchainVk.cpp
	TimelineSemaphoreVk.cpp
	VulkanBase.cpp)

set(PVRVk_INCLUDE_DIRECTORIES 
	${PVR_SDK_ROOT}/include
	${PVR_FRAMEWORK_ROOT}
	${MVK_INCLUDE_DIR})

# Create the library
add_library(PVRVk STATIC
	${PVRVk_SRC}
	${PVRVk_HEADERS}
	${PVRVk_INCLUDES}
	)

# Set the include directories for PVRVk
target_include_directories(PVRVk 
	PUBLIC 
		"$<BUILD_INTERFACE:${PVRVk_INCLUDE_DIRECTORIES}>"
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Determine which platform PVRVk should be built to support
if(PVRVK_USE_PLATFORM)
	# Setup compile definitions
	target_compile_definitions(PVRVk 
		PUBLIC 
			"${PVRVK_USE_PLATFORM}")
endif()

apply_framework_compile_options_to_target(PVRVk)

if(PVRVK_ENABLE_INSTALL)
    # Install standard target
    pvr_install_target(TARGET PVRVk HEADERS ${PVRVk_HEADERS})

    # Extra manual install steps from original file
	install(FILES ${PVRVk_INCLUDES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/include OPTIONAL)
	install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../../include/vulkan DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/include OPTIONAL)
endif()
