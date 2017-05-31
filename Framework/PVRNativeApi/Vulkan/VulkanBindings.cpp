<<<<<<< HEAD
/*!*********************************************************************************************************************
\file         PVRNativeApi\Vulkan\VulkanBindings.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains function definitions for the Vulkan library bindings (vk:: namespace)
***********************************************************************************************************************/
=======
/*!
\brief Contains function definitions for the Vulkan library bindings (vk:: namespace)
\file PVRNativeApi/Vulkan/VulkanBindings.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
>>>>>>> 1776432f... 4.3
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRCore/Base/NativeLibrary.h"

#ifdef _WIN32
static const char* libName = "vulkan-1.dll";
#elif defined(TARGET_OS_MAC)
static const char* libName = "libvulkan.dylib";
#else
static const char* libName = "libvulkan.so.1;libvulkan.so";
#endif
#ifndef TARGET_OS_IPHONE
static pvr::native::NativeLibrary& vkLibrary()
{
	static pvr::native::NativeLibrary mylib(libName);
	return mylib;
}
#endif


std::vector<VkExtensionProperties> vk::extensionStore;
std::vector<VkLayerProperties> vk::layerStore;


#define PVR_STR(x) #x
#define PVR_VULKAN_GET_INSTANCE_POINTER(instance, function_name) function_name = (PFN_vk##function_name)GetInstanceProcAddr(instance, "vk" PVR_STR(function_name)); PVR_ASSERTION(function_name);
#define PVR_VULKAN_GET_INSTANCE_POINTER_NO_CHECK(instance, function_name) function_name = (PFN_vk##function_name)GetInstanceProcAddr(instance, "vk" PVR_STR(function_name));
#define PVR_VULKAN_GET_DEVICE_POINTER(device, function_name) function_name = (PFN_vk##function_name)GetDeviceProcAddr(device, "vk" PVR_STR(function_name)); PVR_ASSERTION(function_name);
#define PVR_VULKAN_FUNCTION_POINTER_DEFINITION(function_name) PFN_vk##function_name vk::function_name;

#define PVR_VULKAN_RAY_TRACING_FUNCTION_POINTER_DEFINITION(function_name) PFN_vk##function_name vkRT::function_name;

PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetInstanceProcAddr);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetDeviceProcAddr);

PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceFeatures);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceFormatProperties);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceImageFormatProperties);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceProperties);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceQueueFamilyProperties);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceMemoryProperties);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateDevice);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyDevice);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(EnumerateInstanceExtensionProperties);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(EnumerateDeviceExtensionProperties);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(EnumerateInstanceLayerProperties);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(EnumerateDeviceLayerProperties);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetDeviceQueue);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(QueueSubmit);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(QueueWaitIdle);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DeviceWaitIdle);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(AllocateMemory);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(FreeMemory);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(MapMemory);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(UnmapMemory);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(FlushMappedMemoryRanges);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(InvalidateMappedMemoryRanges);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetDeviceMemoryCommitment);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(BindBufferMemory);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(BindImageMemory);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetBufferMemoryRequirements);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetImageMemoryRequirements);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetImageSparseMemoryRequirements);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceSparseImageFormatProperties);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(QueueBindSparse);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateFence);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyFence);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(ResetFences);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetFenceStatus);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(WaitForFences);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroySemaphore);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyEvent);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetEventStatus);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(SetEvent);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(ResetEvent);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateQueryPool);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyQueryPool);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetQueryPoolResults);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateBuffer);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyBuffer);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateBufferView);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyBufferView);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateImage);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyImage);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetImageSubresourceLayout);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateImageView);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyImageView);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateShaderModule);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyShaderModule);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreatePipelineCache);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyPipelineCache);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPipelineCacheData);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(MergePipelineCaches);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateGraphicsPipelines);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateComputePipelines);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyPipeline);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreatePipelineLayout);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyPipelineLayout);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateSampler);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroySampler);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateDescriptorSetLayout);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyDescriptorSetLayout);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateDescriptorPool);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyDescriptorPool);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(ResetDescriptorPool);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(AllocateDescriptorSets);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(FreeDescriptorSets);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(UpdateDescriptorSets);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateFramebuffer);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyFramebuffer);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateRenderPass);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyRenderPass);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetRenderAreaGranularity);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateCommandPool);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyCommandPool);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(ResetCommandPool);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(AllocateCommandBuffers);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(FreeCommandBuffers);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(BeginCommandBuffer);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(EndCommandBuffer);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(ResetCommandBuffer);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdBindPipeline);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdSetViewport);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdSetScissor);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdSetLineWidth);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdSetDepthBias);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdSetBlendConstants);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdSetDepthBounds);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdSetStencilCompareMask);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdSetStencilWriteMask);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdSetStencilReference);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdBindDescriptorSets);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdBindIndexBuffer);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdBindVertexBuffers);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdDraw);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdDrawIndexed);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdDrawIndirect);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdDrawIndexedIndirect);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdDispatch);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdDispatchIndirect);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdCopyBuffer);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdCopyImage);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdBlitImage);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdCopyBufferToImage);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdCopyImageToBuffer);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdUpdateBuffer);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdFillBuffer);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdClearColorImage);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdClearDepthStencilImage);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdClearAttachments);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdResolveImage);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdSetEvent);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdResetEvent);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdWaitEvents);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdPipelineBarrier);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdBeginQuery);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdEndQuery);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdResetQueryPool);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdWriteTimestamp);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdCopyQueryPoolResults);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdPushConstants);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdBeginRenderPass);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdNextSubpass);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdEndRenderPass);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdExecuteCommands);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(AcquireNextImageKHR);



PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateInstance);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateSwapchainKHR);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyInstance);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroySurfaceKHR);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroySwapchainKHR);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(EnumeratePhysicalDevices);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceSurfaceCapabilitiesKHR);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceSurfaceFormatsKHR);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceSurfacePresentModesKHR);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceSurfaceSupportKHR);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetSwapchainImagesKHR);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(QueuePresentKHR);

#ifdef DEBUG
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateDebugReportCallbackEXT);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DebugReportMessageEXT);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyDebugReportCallbackEXT);
#endif

#ifdef ANDROID
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateAndroidSurfaceKHR);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateWin32SurfaceKHR);
// expose both of them so one of them get used.
#elif defined(VK_USE_PLATFORM_XLIB_KHR) || defined(VK_USE_PLATFORM_XCB_KHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateXlibSurfaceKHR);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateXcbSurfaceKHR);
#else
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceDisplayPropertiesKHR);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetDisplayModePropertiesKHR);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateDisplayPlaneSurfaceKHR);
#endif

PFN_vkCreateSemaphore(vk::CreateSemaphore);
PFN_vkCreateEvent(vk::CreateEvent);


void vk::initVk(VkInstance instance, VkDevice device)
{
	GetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)vkLibrary().getFunction("vkGetInstanceProcAddr");
	EnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)vkLibrary().getFunction("vkEnumerateInstanceLayerProperties");
	EnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)vkLibrary().getFunction("vkEnumerateInstanceExtensionProperties");
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetDeviceProcAddr);

	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetPhysicalDeviceFeatures);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetPhysicalDeviceFormatProperties);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetPhysicalDeviceImageFormatProperties);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetPhysicalDeviceProperties);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetPhysicalDeviceQueueFamilyProperties);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetPhysicalDeviceMemoryProperties);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, CreateDevice);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, EnumerateDeviceExtensionProperties);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, EnumerateDeviceLayerProperties);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetPhysicalDeviceSparseImageFormatProperties);

	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyDevice);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetDeviceQueue);
	PVR_VULKAN_GET_DEVICE_POINTER(device, QueueSubmit);
	PVR_VULKAN_GET_DEVICE_POINTER(device, QueueWaitIdle);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DeviceWaitIdle);
	PVR_VULKAN_GET_DEVICE_POINTER(device, AllocateMemory);
	PVR_VULKAN_GET_DEVICE_POINTER(device, FreeMemory);
	PVR_VULKAN_GET_DEVICE_POINTER(device, MapMemory);
	PVR_VULKAN_GET_DEVICE_POINTER(device, UnmapMemory);
	PVR_VULKAN_GET_DEVICE_POINTER(device, FlushMappedMemoryRanges);
	PVR_VULKAN_GET_DEVICE_POINTER(device, InvalidateMappedMemoryRanges);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetDeviceMemoryCommitment);
	PVR_VULKAN_GET_DEVICE_POINTER(device, BindBufferMemory);
	PVR_VULKAN_GET_DEVICE_POINTER(device, BindImageMemory);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetBufferMemoryRequirements);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetImageMemoryRequirements);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetImageSparseMemoryRequirements);
	PVR_VULKAN_GET_DEVICE_POINTER(device, QueueBindSparse);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateFence);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyFence);
	PVR_VULKAN_GET_DEVICE_POINTER(device, ResetFences);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetFenceStatus);
	PVR_VULKAN_GET_DEVICE_POINTER(device, WaitForFences);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateSemaphore);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroySemaphore);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateEvent);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyEvent);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetEventStatus);
	PVR_VULKAN_GET_DEVICE_POINTER(device, SetEvent);
	PVR_VULKAN_GET_DEVICE_POINTER(device, ResetEvent);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateQueryPool);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyQueryPool);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetQueryPoolResults);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateBuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyBuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateBufferView);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyBufferView);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateImage);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyImage);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetImageSubresourceLayout);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateImageView);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyImageView);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateShaderModule);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyShaderModule);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreatePipelineCache);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyPipelineCache);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetPipelineCacheData);
	PVR_VULKAN_GET_DEVICE_POINTER(device, MergePipelineCaches);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateGraphicsPipelines);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateComputePipelines);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyPipeline);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreatePipelineLayout);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyPipelineLayout);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateSampler);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroySampler);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateDescriptorSetLayout);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyDescriptorSetLayout);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateDescriptorPool);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyDescriptorPool);
	PVR_VULKAN_GET_DEVICE_POINTER(device, ResetDescriptorPool);
	PVR_VULKAN_GET_DEVICE_POINTER(device, AllocateDescriptorSets);
	PVR_VULKAN_GET_DEVICE_POINTER(device, FreeDescriptorSets);
	PVR_VULKAN_GET_DEVICE_POINTER(device, UpdateDescriptorSets);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateFramebuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyFramebuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateRenderPass);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyRenderPass);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetRenderAreaGranularity);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateCommandPool);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyCommandPool);
	PVR_VULKAN_GET_DEVICE_POINTER(device, ResetCommandPool);
	PVR_VULKAN_GET_DEVICE_POINTER(device, AllocateCommandBuffers);
	PVR_VULKAN_GET_DEVICE_POINTER(device, FreeCommandBuffers);
	PVR_VULKAN_GET_DEVICE_POINTER(device, BeginCommandBuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, EndCommandBuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, ResetCommandBuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdBindPipeline);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdSetViewport);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdSetScissor);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdSetLineWidth);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdSetDepthBias);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdSetBlendConstants);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdSetDepthBounds);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdSetStencilCompareMask);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdSetStencilWriteMask);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdSetStencilReference);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdBindDescriptorSets);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdBindIndexBuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdBindVertexBuffers);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdDraw);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdDrawIndexed);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdDrawIndirect);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdDrawIndexedIndirect);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdDispatch);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdDispatchIndirect);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdCopyBuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdCopyImage);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdBlitImage);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdCopyBufferToImage);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdCopyImageToBuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdUpdateBuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdFillBuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdClearColorImage);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdClearDepthStencilImage);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdClearAttachments);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdResolveImage);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdSetEvent);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdResetEvent);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdWaitEvents);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdPipelineBarrier);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdBeginQuery);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdEndQuery);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdResetQueryPool);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdWriteTimestamp);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdCopyQueryPoolResults);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdPushConstants);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdBeginRenderPass);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdNextSubpass);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdEndRenderPass);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdExecuteCommands);
	PVR_VULKAN_GET_DEVICE_POINTER(device, AcquireNextImageKHR);
}








bool vk::initVulkan()
{
	GetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)vkLibrary().getFunction("vkGetInstanceProcAddr");
	EnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)vkLibrary().getFunction("vkEnumerateInstanceExtensionProperties");
	EnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)vkLibrary().getFunction("vkEnumerateInstanceLayerProperties");
	CreateInstance = (PFN_vkCreateInstance)vkLibrary().getFunction("vkCreateInstance");
	DestroyInstance = (PFN_vkDestroyInstance)vkLibrary().getFunction("vkDestroyInstance");

	if (!GetInstanceProcAddr || !EnumerateInstanceExtensionProperties || !EnumerateInstanceLayerProperties || !CreateInstance || !DestroyInstance)
	{
		return false;
	}

	enumerateExtensions();
	enumerateLayers();
	return true;
}

bool vk::initVulkanInstance(VkInstance instance)
{
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, EnumerateDeviceLayerProperties);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, EnumerateDeviceExtensionProperties);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetPhysicalDeviceSurfaceFormatsKHR);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, EnumeratePhysicalDevices);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetPhysicalDeviceQueueFamilyProperties);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetPhysicalDeviceFeatures);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, CreateDevice);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetDeviceProcAddr);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetPhysicalDeviceMemoryProperties);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetPhysicalDeviceSurfacePresentModesKHR);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetPhysicalDeviceSurfaceSupportKHR);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetPhysicalDeviceFormatProperties);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetPhysicalDeviceProperties);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, DestroySurfaceKHR);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetPhysicalDeviceImageFormatProperties);

#ifdef DEBUG
	PVR_VULKAN_GET_INSTANCE_POINTER_NO_CHECK(instance, CreateDebugReportCallbackEXT);
	PVR_VULKAN_GET_INSTANCE_POINTER_NO_CHECK(instance, DebugReportMessageEXT);
	PVR_VULKAN_GET_INSTANCE_POINTER_NO_CHECK(instance, DestroyDebugReportCallbackEXT);
#endif

#ifdef ANDROID
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, CreateAndroidSurfaceKHR);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, CreateWin32SurfaceKHR);
#elif defined(VK_USE_PLATFORM_XLIB_KHR) || defined(VK_USE_PLATFORM_XCB_KHR)
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, CreateXlibSurfaceKHR);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, CreateXcbSurfaceKHR);
#else
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetPhysicalDeviceDisplayPropertiesKHR);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetDisplayModePropertiesKHR);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, CreateDisplayPlaneSurfaceKHR);
#endif
	return true;
}

bool vk::initVulkanDevice(VkDevice device)
{
	PVR_VULKAN_GET_DEVICE_POINTER(device, AcquireNextImageKHR);
	PVR_VULKAN_GET_DEVICE_POINTER(device, AllocateCommandBuffers);
	PVR_VULKAN_GET_DEVICE_POINTER(device, AllocateDescriptorSets);
	PVR_VULKAN_GET_DEVICE_POINTER(device, AllocateMemory);
	PVR_VULKAN_GET_DEVICE_POINTER(device, BeginCommandBuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, BindBufferMemory);
	PVR_VULKAN_GET_DEVICE_POINTER(device, BindImageMemory);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdBeginQuery);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdBeginRenderPass);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdBindDescriptorSets);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdBindIndexBuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdBindPipeline);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdBindVertexBuffers);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdBlitImage);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdClearAttachments);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdClearColorImage);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdClearDepthStencilImage);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdCopyBuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdCopyBufferToImage);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdCopyImage);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdCopyImageToBuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdCopyQueryPoolResults);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdDispatch);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdDispatchIndirect);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdDraw);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdDrawIndexed);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdDrawIndexedIndirect);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdDrawIndirect);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdEndQuery);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdEndRenderPass);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdExecuteCommands);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdFillBuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdNextSubpass);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdPipelineBarrier);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdPushConstants);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdResetEvent);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdResetQueryPool);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdResolveImage);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdSetBlendConstants);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdSetDepthBias);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdSetDepthBounds);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdSetEvent);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdSetLineWidth);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdSetScissor);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdSetStencilCompareMask);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdSetStencilReference);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdSetStencilWriteMask);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdSetViewport);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdUpdateBuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdWaitEvents);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdWriteTimestamp);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateBuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateBufferView);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateCommandPool);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateComputePipelines);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateDescriptorPool);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateDescriptorSetLayout);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateEvent);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateFence);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateFramebuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateGraphicsPipelines);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateImage);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateImageView);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreatePipelineCache);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreatePipelineLayout);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateQueryPool);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateRenderPass);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateSampler);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateSemaphore);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateShaderModule);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyBuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyBufferView);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyCommandPool);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyDescriptorPool);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyDescriptorSetLayout);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyDevice);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyEvent);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyFence);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyFramebuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyImage);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyImageView);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyPipeline);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyPipelineCache);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyPipelineLayout);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyQueryPool);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyRenderPass);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroySampler);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroySemaphore);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyShaderModule);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DeviceWaitIdle);
	PVR_VULKAN_GET_DEVICE_POINTER(device, EndCommandBuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, FlushMappedMemoryRanges);
	PVR_VULKAN_GET_DEVICE_POINTER(device, FreeCommandBuffers);
	PVR_VULKAN_GET_DEVICE_POINTER(device, FreeDescriptorSets);
	PVR_VULKAN_GET_DEVICE_POINTER(device, FreeMemory);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetBufferMemoryRequirements);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetDeviceMemoryCommitment);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetDeviceQueue);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetEventStatus);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetFenceStatus);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetImageMemoryRequirements);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetImageSparseMemoryRequirements);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetImageSubresourceLayout);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetPipelineCacheData);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetQueryPoolResults);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetRenderAreaGranularity);
	PVR_VULKAN_GET_DEVICE_POINTER(device, InvalidateMappedMemoryRanges);
	PVR_VULKAN_GET_DEVICE_POINTER(device, MapMemory);
	PVR_VULKAN_GET_DEVICE_POINTER(device, MergePipelineCaches);
	PVR_VULKAN_GET_DEVICE_POINTER(device, QueueBindSparse);
	PVR_VULKAN_GET_DEVICE_POINTER(device, QueueSubmit);
	PVR_VULKAN_GET_DEVICE_POINTER(device, QueueWaitIdle);
	PVR_VULKAN_GET_DEVICE_POINTER(device, ResetCommandBuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, ResetCommandPool);
	PVR_VULKAN_GET_DEVICE_POINTER(device, ResetDescriptorPool);
	PVR_VULKAN_GET_DEVICE_POINTER(device, ResetEvent);
	PVR_VULKAN_GET_DEVICE_POINTER(device, ResetFences);
	PVR_VULKAN_GET_DEVICE_POINTER(device, SetEvent);
	PVR_VULKAN_GET_DEVICE_POINTER(device, UnmapMemory);
	PVR_VULKAN_GET_DEVICE_POINTER(device, UpdateDescriptorSets);
	PVR_VULKAN_GET_DEVICE_POINTER(device, WaitForFences);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateSwapchainKHR);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroySwapchainKHR);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetSwapchainImagesKHR);
	PVR_VULKAN_GET_DEVICE_POINTER(device, QueuePresentKHR);

	vk::CreateSemaphore = (PFN_vkCreateSemaphore)GetDeviceProcAddr(device, "vkCreateSemaphore");
	return true;
}

struct ExtensionComparer
{
	const std::string& extension;
	ExtensionComparer(const std::string& extension) : extension(extension) {}
	bool operator()(const VkExtensionProperties& props)
	{
		return !strcmp(extension.c_str(), props.extensionName);
	}
};

bool vk::isVulkanExtensionSupported(const std::string& extension)
{
	std::vector<VkExtensionProperties>::iterator find_if = std::find_if(extensionStore.begin(), extensionStore.end(), ExtensionComparer(extension));
	return (find_if != extensionStore.end());
}

bool vk::isVulkanExtensionSupported(void* dpy, const std::string& extension)
{
	return isVulkanExtensionSupported(extension);
}

void vk::enumerateExtensions()
{
	pvr::uint32 numExtensions;
	vk::EnumerateInstanceExtensionProperties(NULL, &numExtensions, NULL);
	extensionStore.resize(numExtensions);
	vk::EnumerateInstanceExtensionProperties(NULL, &numExtensions, extensionStore.data());
	for (size_t i = 0; i < extensionStore.size(); ++i)
	{
		pvr::Log(pvr::Logger::Debug, "Extension found: %s", extensionStore[i].extensionName);
	}
}

void vk::enumerateLayers()
{
	pvr::uint32 propertyCount;
	vk::EnumerateInstanceLayerProperties(&propertyCount, NULL);
	layerStore.resize(propertyCount);
	vk::EnumerateInstanceLayerProperties(&propertyCount, layerStore.data());
}


#undef PVR_STR
#undef PVR_VULKAN_GET_INSTANCE_POINTER
#undef PVR_VULKAN_GET_DEVICE_POINTER
#undef PVR_VULKAN_FUNCTION_POINTER_DEFINITION

#undef PVR_VULKAN_RAY_TRACING_GET_INSTANCE_POINTER
#undef PVR_VULKAN_GET_DEVICE_POINTER
#undef PVR_VULKAN_RAY_TRACING_FUNCTION_POINTER_DEFINITION
//!\endcond
