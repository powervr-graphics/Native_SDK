cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)
project(PVRVk)
include(GNUInstallDirs)
include(../FrameworkCommon.cmake)
# Copyright (c) Imagination Technologies Limited.

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
	TypesVk.h)
	  
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
	SwapchainVk.cpp)

set(PVRVk_INCLUDE_DIRECTORIES 
	${CMAKE_CURRENT_LIST_DIR}/../../include
	${CMAKE_CURRENT_LIST_DIR}/../
	${MVK_INCLUDE_DIR})

# Create the library
add_library(PVRVk STATIC
	${PVRVk_SRC}
	${PVRVk_HEADERS}
	${PVRVk_INCLUDES}
	)


# Add in the submodules that are requirements in Vulkan
add_submodule_to_sdk(${SDK_ROOT_INTERNAL_DIR}/external/VulkanMemoryAllocator)
add_submodule_to_sdk(${SDK_ROOT_INTERNAL_DIR}/external/glslang)

# The users surface choice is stored in PVRVK_USE_PLATFORM
# We have to select the surface being targetted because Vulkan 
# provides a different function to target a different surface type.
# This is then passed to the frame work as a compile definition 
# VK_USE_PLATFORM_*_KHR
set(PVRVK_USE_PLATFORM "" CACHE INTERNAL "")

if (WIN32)
	list(APPEND PVRVK_USE_PLATFORM "VK_USE_PLATFORM_WIN32_KHR")
elseif(ANDROID)
	list(APPEND PVRVK_USE_PLATFORM "VK_USE_PLATFORM_ANDROID_KHR")
elseif(APPLE)
	if (${MOLTENVK_FOUND})
		list(APPEND PVRVK_USE_PLATFORM "VK_USE_PLATFORM_MACOS_MVK")
	endif()	
elseif(UNIX)
	if(PVR_WINDOW_SYSTEM)
		if (${PVR_WINDOW_SYSTEM} STREQUAL X11)
			list(APPEND PVRVK_USE_PLATFORM "VK_USE_PLATFORM_XLIB_KHR")
		elseif (${PVR_WINDOW_SYSTEM} STREQUAL XCB)
			list(APPEND PVRVK_USE_PLATFORM "VK_USE_PLATFORM_XCB_KHR")
		elseif (${PVR_WINDOW_SYSTEM} STREQUAL Wayland)
			list(APPEND PVRVK_USE_PLATFORM "VK_USE_PLATFORM_WAYLAND_KHR")
		endif()
	endif()
endif()

# Ensure that the PVRVK_USE_PLATFORM variable is cached
set(PVRVK_USE_PLATFORM "${PVRVK_USE_PLATFORM}" CACHE STRING "A list of Vulkan platforms of the form VK_USE_PLATFORM_* used as compile-time definitions in PVRVk")

# Set the include directories for PVRVk
target_include_directories(PVRVk PUBLIC ${PVRVk_INCLUDE_DIRECTORIES})

# Link to the libraries that contain dlopen and dlclose, 
# this allows PVRVk to dynamically open shared libraries, such as libvulkan.so
target_link_libraries(PVRVk PRIVATE ${CMAKE_DL_LIBS})

# Set the compile definitions, so that the correct part of the Vulkan headers is included
# If we need to include the Vulkan Surface Extension and the beta extensions
if(PVRVK_USE_PLATFORM)
	# Setup compile definitions
	target_compile_definitions(PVRVk 
		PUBLIC 
			"${PVRVK_USE_PLATFORM}"
			"${PVRVK_BETA_EXT}")
endif()

# Apply the common framework compile definitions to PVRVk
# This sets certain warnings on/off and adds appropriate
# debug and release flags in the compiler and linker  
apply_framework_compile_options_to_target(PVRVk)