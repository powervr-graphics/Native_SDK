<<<<<<< HEAD
/*!*********************************************************************************************************************
\file         PVRNativeApi\Vulkan\VulkanBindings.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        This file contains Vulkan bindings with function pointers. The PowerVR Framework uses them to allow unified
			  access to Vulkancontext throught the same functions.
			  Function pointer loading is done with the vk:initVk function which is normally called by the PVR Shell.
***********************************************************************************************************************/
=======
/*!
\brief This file contains Vulkan bindings with function pointers. The PowerVR Framework uses them to allow unified
access to Vulkancontext throught the same functions. Function pointer loading is done with the vk:initVk
function which is normally called by the PVR Shell.
\file PVRNativeApi/Vulkan/VulkanBindings.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
>>>>>>> 1776432f... 4.3
#pragma once
#include "PVRNativeApi/Vulkan/HeadersVk.h"

#include <algorithm>
#include <vector>
#include <memory>
#include <string.h>
#include "PVRCore/Base/Types.h"

#define PVR_VULKAN_FUNCTION_POINTER_DECLARATION(function_name) static PFN_vk##function_name function_name;


/// <summary>This class contains function pointers to all Vulkan functions. These function pointers will be
/// populated on the initVk call. Use normally, using the vk class as a namespace. For example
/// vk::CmdBindPipeline(...);</summary>
class vk
{
public:
	//Call this function once before using the vk namespaced functions. Do not call during static initialisation phase as it uses static global components.
	/// <summary>Call once per application run to populate the function pointers. PVRShell calls this on context
	/// creation.</summary>
	static void initVk(VkInstance instance, VkDevice device);


<<<<<<< HEAD
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceFeatures);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceFormatProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceImageFormatProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceQueueFamilyProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceMemoryProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateDevice);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyDevice);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EnumerateInstanceExtensionProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EnumerateDeviceExtensionProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EnumerateInstanceLayerProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EnumerateDeviceLayerProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetDeviceQueue);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(QueueSubmit);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(QueueWaitIdle);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DeviceWaitIdle);
=======




	/// <summary>Initialize the vulkan glue. automatically called just before context initialisation</summary>
	static bool initVulkan();

	/// <summary>Initialize the vullkan instance function pointers. Automatically Called just before context initialisation
	/// </summary>
	static bool initVulkanInstance(VkInstance instance);

	/// <summary>Initialize the vullkan device function pointers. Automatically called just before context initialisation
	/// </summary>
	static bool initVulkanDevice(VkDevice device);

	/// <summary>Check for the presense of a Vulkan extension for the specified display</summary>
	static bool isVulkanExtensionSupported(void* display, const std::string& extension);

	/// <summary>Check for the presense of a Vulkan extension for the current context</summary>
	static bool isVulkanExtensionSupported(const std::string& extension);

	/// <summary>Get number of extensions supported</summary>
	/// <returns>Number of supported extensions</returns>
	static pvr::uint32 getNumExtensions() { return (pvr::uint32)extensionStore.size(); }

	/// <summary>Get number of layers supported</summary>
	/// <returns>Number of supported layers</returns>
	static pvr::uint32 getNumLayers() { return (pvr::uint32)layerStore.size(); }

	/// <summary>Get all supported layers</summary>
	/// <returns>Return all supported layers</returns>
	static VkLayerProperties* getAllLayersName() { return layerStore.data(); }

	/// <summary>Get all supported extensions</summary>
	/// <returns>Return all supported extensions</returns>
	static VkExtensionProperties* getAllExtensionsName() { return extensionStore.data(); }


#undef CreateSemaphore
#undef CreateEvent


	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(AcquireNextImageKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(AllocateCommandBuffers);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(AllocateDescriptorSets);
>>>>>>> 1776432f... 4.3
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(AllocateMemory);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(BeginCommandBuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(BindBufferMemory);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(BindImageMemory);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdBeginQuery);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdBeginRenderPass);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdBindDescriptorSets);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdBindIndexBuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdBindPipeline);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdBindVertexBuffers);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdBlitImage);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdClearAttachments);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdClearColorImage);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdClearDepthStencilImage);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdCopyBuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdCopyBufferToImage);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdCopyImage);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdCopyImageToBuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdCopyQueryPoolResults);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdDispatch);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdDispatchIndirect);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdDraw);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdDrawIndexed);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdDrawIndexedIndirect);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdDrawIndirect);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdEndQuery);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdEndRenderPass);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdExecuteCommands);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdFillBuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdNextSubpass);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdPipelineBarrier);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdPushConstants);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdResetEvent);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdResetQueryPool);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdResolveImage);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetBlendConstants);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetDepthBias);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetDepthBounds);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetEvent);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetLineWidth);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetScissor);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetStencilCompareMask);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetStencilReference);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetStencilWriteMask);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetViewport);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdUpdateBuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdWaitEvents);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdWriteTimestamp);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateBuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateBufferView);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateCommandPool);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateComputePipelines);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateDescriptorPool);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateDescriptorSetLayout);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateDevice);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateEvent);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateFence);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateFramebuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateGraphicsPipelines);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateImage);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateImageView);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreatePipelineCache);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreatePipelineLayout);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateQueryPool);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateRenderPass);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateSampler);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateSemaphore);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateShaderModule);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyBuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyBufferView);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyCommandPool);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyDescriptorPool);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyDescriptorSetLayout);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyDevice);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyEvent);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyFence);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyFramebuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyImage);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyImageView);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyPipeline);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyPipelineCache);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyPipelineLayout);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyQueryPool);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyRenderPass);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroySampler);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroySemaphore);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyShaderModule);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DeviceWaitIdle);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EndCommandBuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EnumerateDeviceExtensionProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EnumerateDeviceLayerProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EnumerateInstanceExtensionProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EnumerateInstanceLayerProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(FlushMappedMemoryRanges);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(FreeCommandBuffers);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(FreeDescriptorSets);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(FreeMemory);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetBufferMemoryRequirements);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetDeviceMemoryCommitment);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetDeviceProcAddr);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetDeviceQueue);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetEventStatus);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetFenceStatus);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetImageMemoryRequirements);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetImageSparseMemoryRequirements);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetImageSubresourceLayout);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetInstanceProcAddr);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceFeatures);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceFormatProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceImageFormatProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceMemoryProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceQueueFamilyProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSparseImageFormatProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPipelineCacheData);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetQueryPoolResults);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetRenderAreaGranularity);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(InvalidateMappedMemoryRanges);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(MapMemory);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(MergePipelineCaches);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(QueueBindSparse);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(QueueSubmit);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(QueueWaitIdle);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(ResetCommandBuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(ResetCommandPool);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(ResetDescriptorPool);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(ResetEvent);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(ResetFences);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(SetEvent);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(UnmapMemory);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(UpdateDescriptorSets);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(WaitForFences);


	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateInstance);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateSwapchainKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyInstance);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroySurfaceKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroySwapchainKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EnumeratePhysicalDevices);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSurfaceCapabilitiesKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSurfaceFormatsKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSurfacePresentModesKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSurfaceSupportKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetSwapchainImagesKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(QueuePresentKHR);

#ifdef DEBUG
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateDebugReportCallbackEXT);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DebugReportMessageEXT);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyDebugReportCallbackEXT);
#endif

#ifdef ANDROID
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateAndroidSurfaceKHR);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateWin32SurfaceKHR);
	// expose both of them so one of them get used.
#elif defined(VK_USE_PLATFORM_XLIB_KHR) || defined(VK_USE_PLATFORM_XCB_KHR)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateXlibSurfaceKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateXcbSurfaceKHR);
#else
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceDisplayPropertiesKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetDisplayModePropertiesKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateDisplayPlaneSurfaceKHR);
#endif
private:
	// Enumerate list of all extensions supported on this device
	static void enumerateExtensions();
	static void enumerateLayers();
	static std::vector<VkExtensionProperties> extensionStore;
	static std::vector<VkLayerProperties> layerStore;
};


#undef PVR_VULKAN_FUNCTION_POINTER_DECLARATION
//!\endcond