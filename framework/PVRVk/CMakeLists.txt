cmake_minimum_required(VERSION 3.3)
project (PVRVk)

set(PVRVk_SRC
	   ${CMAKE_CURRENT_LIST_DIR}/ApiObjectsVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/BufferVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/BufferVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/CommandBufferVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/CommandBufferVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/CommandPoolVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/CommandPoolVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/ComputePipelineVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/DebugReportCallbackVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/DebugReportCallbackVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/DebugUtilsMessengerVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/DebugUtilsMessengerVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/DebugUtilsVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/DebugUtilsVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/DescriptorSetVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/DescriptorSetVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/DeviceMemoryVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/DeviceVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/DeviceVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/DisplayModeVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/DisplayModeVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/DisplayVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/DisplayVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/EventVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/EventVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/ExtensionsVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/ExtensionsVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/FenceVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/FenceVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/ForwardDecObjectsVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/FramebufferVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/FramebufferVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/GraphicsPipelineVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/HeadersVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/ImageVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/ImageVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/InstanceVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/InstanceVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/LayersVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/LayersVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/MemoryBarrierVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/PhysicalDeviceVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/PhysicalDeviceVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/PipelineCacheVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/PipelineConfigVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/PipelineLayoutVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/PipelineLayoutVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/PipelineVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/PopulateCreateInfoVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/PVRVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/pvrvk_vulkan_wrapper.h
	   ${CMAKE_CURRENT_LIST_DIR}/PVRVkObjectBaseVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/QueryPoolVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/QueryPoolVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/QueueVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/QueueVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/RenderPassVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/RenderPassVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/SamplerVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/SamplerVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/SemaphoreVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/SemaphoreVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/ShaderModuleVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/ShaderModuleVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/SurfaceVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/SurfaceVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/SwapchainVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/SwapchainVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/TypesVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/../../include/vk_bindings.h
	   ${CMAKE_CURRENT_LIST_DIR}/../../include/vk_bindings_helper.h)

set(PVRVk_INCLUDE_DIRECTORIES 
	${CMAKE_CURRENT_LIST_DIR}/../../include
	${CMAKE_CURRENT_LIST_DIR}/../../framework)
	
set(PVRVk_LINK_LIBS VulkanPlatformInterface PowerVR_SDK)
	   
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT AND USE_PREBUILT_DEPENDENCIES)
add_library(PVRVk STATIC IMPORTED GLOBAL)

# add the subdirectory after the add_library so that the target exists
add_subdirectory(../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk EXCLUDE_FROM_ALL)

set_target_properties(PVRVk PROPERTIES
	IMPORTED_LOCATION_DEBUG ${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRVk${CMAKE_STATIC_LIBRARY_SUFFIX}
	IMPORTED_LOCATION_RELEASE ${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRVk${CMAKE_STATIC_LIBRARY_SUFFIX}
	INTERFACE_INCLUDE_DIRECTORIES "${PVRVk_INCLUDE_DIRECTORIES}"
	INTERFACE_LINK_LIBRARIES "${PVRVk_LINK_LIBS}")
else()
add_library(PVRVk STATIC EXCLUDE_FROM_ALL ${PVRVk_SRC})

# add the subdirectory after the add_library so that the target exists
add_subdirectory(../../ ${CMAKE_CURRENT_BINARY_DIR}/sdk EXCLUDE_FROM_ALL)

target_include_directories(PVRVk PUBLIC ${PVRVk_INCLUDE_DIRECTORIES})
target_link_libraries(PVRVk ${PVRVk_LINK_LIBS})
set_target_properties(PVRVk PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${FRAMEWORK_LIB_FOLDER}
	LIBRARY_OUTPUT_DIRECTORY ${FRAMEWORK_LIB_FOLDER})
endif()