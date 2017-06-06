/*
Copyright (c) 2015 Imagination Technologies Ltd.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and/or associated documentation files (the
"Materials"), to deal in the Materials without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Materials, and to
permit persons to whom the Materials are furnished to do so, subject to
the following conditions:

The above copyright notice(s) and this permission notice shall be included
in all copies or substantial portions of the Materials.


The Materials are Confidential Information as defined by the
Khronos Membership Agreement until designated non-confidential by Khronos,
at which point this condition clause shall be removed.


THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/

#define PVR_MAX_SWAPCHAIN_IMAGES 3
#define VK_NO_PROTOTYPES
#include "vulkan/vulkan.h"
#include <cstdio>
#define LOGI(...) ((void)printf(__VA_ARGS__))
#define LOGW(...) ((void)fprintf(stderr, __VA_ARGS__))
#define LOGE(...) ((void)fprintf(stderr, __VA_ARGS__))

#include <algorithm>
#include <unistd.h>
#include <dlfcn.h>
#include <cstdlib>
#include <vector>
#include <cassert>
#include <string>
#include <cstring>
#include "vshader_vert.h"
#include "fshader_frag.h"

#undef CreateSemaphore
#undef CreateEvent

#define PVR_VULKAN_FUNCTION_POINTER_DECLARATION(function_name) static PFN_vk##function_name function_name;
/*!*********************************************************************************************************************
\brief   This class's static members are function pointers to the Vulkan functions. The Shell kicks off their initialisation on
before context creation. Use the class name like a namespace: vk::CreateInstance.
***********************************************************************************************************************/
class vk
{
public:
	static bool initVulkan(); //!<Automatically called just before context initialisation.
	static bool initVulkanInstance(VkInstance instance); //!<Automatically called just before context initialisation.
	static bool initVulkanDevice(VkDevice device); //!<Automatically called just before context initialisation.

	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceDisplayPropertiesKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetDisplayModePropertiesKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateDisplayPlaneSurfaceKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSurfaceSupportKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSurfacePresentModesKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSurfaceCapabilitiesKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSurfaceFormatsKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateSwapchainKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetSwapchainImagesKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(AcquireNextImageKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(QueuePresentKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroySwapchainKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateInstance);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EnumeratePhysicalDevices);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyInstance);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroySurfaceKHR);

	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetInstanceProcAddr);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetDeviceProcAddr);

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
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(AllocateMemory);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(FreeMemory);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(MapMemory);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(UnmapMemory);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(FlushMappedMemoryRanges);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(InvalidateMappedMemoryRanges);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetDeviceMemoryCommitment);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(BindBufferMemory);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(BindImageMemory);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetBufferMemoryRequirements);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetImageMemoryRequirements);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetImageSparseMemoryRequirements);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSparseImageFormatProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(QueueBindSparse);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateFence);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyFence);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(ResetFences);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetFenceStatus);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(WaitForFences);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateSemaphore);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroySemaphore);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateEvent);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyEvent);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetEventStatus);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(SetEvent);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(ResetEvent);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateQueryPool);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyQueryPool);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetQueryPoolResults);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateBuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyBuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateBufferView);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyBufferView);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateImage);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyImage);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetImageSubresourceLayout);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateImageView);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyImageView);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateShaderModule);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyShaderModule);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreatePipelineCache);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyPipelineCache);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPipelineCacheData);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(MergePipelineCaches);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateGraphicsPipelines);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateComputePipelines);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyPipeline);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreatePipelineLayout);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyPipelineLayout);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateSampler);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroySampler);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateDescriptorSetLayout);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyDescriptorSetLayout);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateDescriptorPool);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyDescriptorPool);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(ResetDescriptorPool);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(AllocateDescriptorSets);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(FreeDescriptorSets);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(UpdateDescriptorSets);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateFramebuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyFramebuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateRenderPass);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyRenderPass);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetRenderAreaGranularity);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateCommandPool);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyCommandPool);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(ResetCommandPool);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(AllocateCommandBuffers);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(FreeCommandBuffers);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(BeginCommandBuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EndCommandBuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(ResetCommandBuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdBindPipeline);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetViewport);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetScissor);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetLineWidth);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetDepthBias);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetBlendConstants);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetDepthBounds);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetStencilCompareMask);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetStencilWriteMask);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetStencilReference);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdBindDescriptorSets);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdBindIndexBuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdBindVertexBuffers);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdDraw);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdDrawIndexed);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdDrawIndirect);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdDrawIndexedIndirect);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdDispatch);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdDispatchIndirect);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdCopyBuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdCopyImage);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdBlitImage);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdCopyBufferToImage);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdCopyImageToBuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdUpdateBuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdFillBuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdClearColorImage);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdClearDepthStencilImage);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdClearAttachments);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdResolveImage);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdSetEvent);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdResetEvent);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdWaitEvents);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdPipelineBarrier);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdBeginQuery);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdEndQuery);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdResetQueryPool);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdWriteTimestamp);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdCopyQueryPoolResults);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdPushConstants);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdBeginRenderPass);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdNextSubpass);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdEndRenderPass);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdExecuteCommands);

	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateDebugReportCallbackEXT);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DebugReportMessageEXT);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyDebugReportCallbackEXT);
};

#undef PVR_VULKAN_FUNCTION_POINTER_DECLARATION
#define PVR_VULKAN_FUNCTION_POINTER_DEFINITION(function_name) PFN_vk##function_name vk::function_name;

PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceDisplayPropertiesKHR);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetDisplayModePropertiesKHR);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateDisplayPlaneSurfaceKHR);

PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceSurfaceCapabilitiesKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceSurfaceFormatsKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateSwapchainKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetSwapchainImagesKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(AcquireNextImageKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(QueuePresentKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroySwapchainKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateInstance)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(EnumeratePhysicalDevices)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyInstance)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceSurfacePresentModesKHR);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceSurfaceSupportKHR);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroySurfaceKHR);

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
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateSemaphore);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroySemaphore);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateEvent);
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

PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateDebugReportCallbackEXT);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DebugReportMessageEXT);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyDebugReportCallbackEXT);

#undef PVR_VULKAN_FUNCTION_POINTER_DEFINITION

struct NativeLibrary
{
	typedef void* LIBTYPE;

public:
	/*!*********************************************************************************************************************
	\brief   Load a library with the specified filename.
	\param   libraryPath The path to find the library (name or Path+name).
	***********************************************************************************************************************/
	NativeLibrary(const std::string& LibPath)
	{
		_hostLib = dlopen(LibPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);

		if (!_hostLib)
		{
			LOGE("Could not load host library '%s'\n", LibPath.c_str());

			const char* err = dlerror();

			if (err)
			{
				LOGE("dlopen failed with error: %s => %s\n", err, LibPath.c_str());
			}

			char pathMod[256];
			strcpy(pathMod, "./");
			strcat(pathMod, LibPath.c_str());

			_hostLib = dlopen(pathMod, RTLD_LAZY | RTLD_GLOBAL);

			if (!_hostLib)
			{
				const char* err = dlerror();

				if (err)
				{
					LOGE("dlopen failed with error: %s => %s\n", err, pathMod);
				}
			}
			else
			{
				LOGE("dlopen loaded (MOD PATH) %s\n", pathMod);
			}
		}
		LOGI("Host library '%s' loaded\n", LibPath.c_str());
	}
	~NativeLibrary() { CloseLib(); }

	/*!*********************************************************************************************************************
	\brief   Get a function pointer from the library.
	\param   functionName  The name of the function to retrieve the pointer to.
	\return  The function pointer as a void pointer. Null if failed. Cast to the proper type.
	***********************************************************************************************************************/
	void* getFunction(const char* functionName)
	{
		if (_hostLib)
		{
			void* pFn = dlsym(_hostLib, functionName);
			if (pFn == NULL)
			{
				LOGE("Could not get function %s\n", functionName);
			}

			return pFn;
		}

		return NULL;
	}

	/*!*********************************************************************************************************************
	\brief   Get a function pointer from the library.
	\tparam  PtrType_ The type of the function pointer
	\param   functionName  The name of the function to retrieve the pointer to
	\return  The function pointer. Null if failed.
	***********************************************************************************************************************/
	template<typename PtrType_> PtrType_ getFunction(const char* functionName) { return reinterpret_cast<PtrType_>(getFunction(functionName)); }

	/*!*********************************************************************************************************************
	\brief   Release this library.
	***********************************************************************************************************************/
	void CloseLib()
	{
		if (_hostLib)
		{
			dlclose(_hostLib);
			_hostLib = 0;
		}
	}
protected:
	LIBTYPE		_hostLib;
};
static NativeLibrary& vkglueLib()
{
	static NativeLibrary mylib("libvulkan.so");
	return mylib;
}

#define PVR_STR(x) #x
#define PVR_VULKAN_GET_INSTANCE_POINTER(instance, function_name) function_name = (PFN_vk##function_name)GetInstanceProcAddr(instance, "vk" PVR_STR(function_name));
#define PVR_VULKAN_GET_DEVICE_POINTER(device, function_name) function_name = (PFN_vk##function_name)GetDeviceProcAddr(device, "vk" PVR_STR(function_name));

bool vk::initVulkan()
{
	GetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)vkglueLib().getFunction("vkGetInstanceProcAddr");
	EnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)vkglueLib().getFunction("vkEnumerateInstanceExtensionProperties");
	EnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)vkglueLib().getFunction("vkEnumerateInstanceLayerProperties");
	CreateInstance = (PFN_vkCreateInstance)vkglueLib().getFunction("vkCreateInstance");
	DestroyInstance = (PFN_vkDestroyInstance)vkglueLib().getFunction("vkDestroyInstance");

	if (!GetInstanceProcAddr || !EnumerateInstanceExtensionProperties || !EnumerateInstanceLayerProperties || !CreateInstance || !DestroyInstance)
	{
		return false;
	}

	return true;
}

bool vk::initVulkanInstance(VkInstance instance)
{
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetPhysicalDeviceDisplayPropertiesKHR);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetDisplayModePropertiesKHR);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, CreateDisplayPlaneSurfaceKHR);

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
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, GetPhysicalDeviceSparseImageFormatProperties);

	PVR_VULKAN_GET_INSTANCE_POINTER(instance, CreateDebugReportCallbackEXT);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, DebugReportMessageEXT);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, DestroyDebugReportCallbackEXT);

	return true;
}

bool vk::initVulkanDevice(VkDevice device)
{
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateSwapchainKHR);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetSwapchainImagesKHR);
	PVR_VULKAN_GET_DEVICE_POINTER(device, AcquireNextImageKHR);
	PVR_VULKAN_GET_DEVICE_POINTER(device, QueuePresentKHR);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroySwapchainKHR);

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

	return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL CustomDebugReportCallback(
  VkDebugReportFlagsEXT       flags,
  VkDebugReportObjectTypeEXT  objectType,
  uint64_t                    object,
  size_t                      location,
  int32_t                     messageCode,
  const char*                 pLayerPrefix,
  const char*                 pMessage,
  void*                       pUserData)
{
	LOGE("LAYER_VALIDATION: %s\n", pMessage);

	return VK_FALSE;
}

// Windows class name to register
#define	WINDOW_CLASS_NAME _T("PVRShellClass")

// Name of the application
#define APPLICATION_NAME _T("HelloAPI")

// Title to display for errors.
#define ERROR_TITLE _T("Error")

// Width and height of the window
const unsigned int WindowWidth = 800;
const unsigned int WindowHeight = 600;

// Index to bind the attributes to vertex shaders
const unsigned int VertexArray = 0;

// Variable set by the message handler to finish the demo
bool HasUserQuit = false;

inline void vkSuccessOrDie(VkResult result, const char* msg)
{
	if (result != VK_SUCCESS)
	{
		LOGE("Failed: %s\n", msg);
		exit(0);
	}
}

struct NativePlatformHandles;
struct NativeDisplayHandle;
struct HelloAPI;

/*
Top level structure
Handles the management of platforms and displays
*/
struct App
{
	NativePlatformHandles* platformHandles;
	NativeDisplayHandle* displayHandle;

	HelloAPI* application;

	void initVkInstanceAndPhysicalDevice(bool enableLayers, bool enableExtensions);
	VkDeviceMemory allocateImageDeviceMemory(VkImage image, VkMemoryRequirements* memoryRequirementsOut = 0);
	VkDeviceMemory allocateBufferDeviceMemory(VkBuffer buffer, VkMemoryRequirements* memoryRequirementsOut = 0);
	void initSwapChain();
	void setInitialSwapchainLayouts();
	void initSynchronizationObjects();
	void initColorBlendAttachmentState(VkPipelineColorBlendAttachmentState& state);
	bool initDevice(bool enableLayers);
	bool getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlagBits properties,
	                        uint32_t& outTypeIndex);
	void initGlobalState();
	void initPostPresentBarrierCommandBuffer();
	void deinitGlobalState();
	void submitPostPresentBarrier(uint32_t swapchain);
	void initSurface();
	void deinitDisplayAndApplication();
	VkCommandBuffer createCommandBuffer();
};

struct DepthBuffer
{
	VkImage image;
	VkDeviceMemory memory;
	VkImageView view;
};

/*
Holds application specific rendering objects
*/
struct HelloAPI
{
	VkRenderPass renderPass;
	DepthBuffer depthBuffers[8];
	VkCommandBuffer cmdBuffer[8];
	VkFramebuffer framebuffer[8];

	VkPipelineLayout emptyPipelayout;
	VkPipeline opaquePipeline;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
};

struct NativeDisplayHandle
{
	VkDisplayKHR nativeDisplay;
	VkSurfaceKHR surface;
	VkExtent2D displayExtent;
	VkSwapchainKHR swapChain;
	uint32_t swapChainLength;//< Number of SwapChains
	struct FrameBuffer
	{
		std::vector<VkImage> colorImages;
		std::vector<VkImageView>colorImageViews;
		std::vector<std::pair<VkImage, VkDeviceMemory>/**/> depthStencilImage;
		std::vector<VkImageView>depthStencilImageView;
		VkFormat colorFormat;
		VkFormat depthStencilFormat;
		bool depthStencilHasStencil;
	} onscreenFbo;
};

struct Context
{
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkInstance instance;
};

struct PlatformInfo
{
	std::string deviceName;
	std::string platformName;
	uint32_t numberOfPhysicalDevices;

	const char* enabledExtensions[16];
	const char* enabledLayers[16];
};

struct NativePlatformHandles
{
	Context context;
	VkQueue graphicsQueue;
	VkPhysicalDeviceMemoryProperties deviceMemProperties;
	uint32_t graphicsQueueIndex;
	VkCommandPool commandPool;

	VkFence fenceAcquire[PVR_MAX_SWAPCHAIN_IMAGES + 1];
	VkFence fencePrePresent[PVR_MAX_SWAPCHAIN_IMAGES + 1];
	VkFence fenceRender[PVR_MAX_SWAPCHAIN_IMAGES];

	VkSemaphore semaphoreFinishedRendering[PVR_MAX_SWAPCHAIN_IMAGES];
	VkSemaphore semaphoreCanPresent[PVR_MAX_SWAPCHAIN_IMAGES];
	VkSemaphore semaphoreImageAcquired[PVR_MAX_SWAPCHAIN_IMAGES + 1];
	VkSemaphore semaphoreCanBeginRendering[PVR_MAX_SWAPCHAIN_IMAGES];

	VkCommandBuffer postPresentCmdBuffer[PVR_MAX_SWAPCHAIN_IMAGES];

	VkDebugReportCallbackEXT debugReportCallback;
	bool supportsDebugReport;

	PlatformInfo platformInfo;

	uint32_t currentImageAcqSem;

	uint32_t swapIndex;
	uint32_t lastPresentedSwapIndex;

	NativePlatformHandles() : supportsDebugReport(false), currentImageAcqSem(0) {}
};

struct GraphicsPipelineCreate
{
	enum ShaderStage
	{
		Vertex, Fragment
	};
	VkGraphicsPipelineCreateInfo vkPipeInfo;
	VkPipelineShaderStageCreateInfo shaderStages[2];
	VkPipelineColorBlendStateCreateInfo cb;
	VkPipelineInputAssemblyStateCreateInfo ia;
	VkPipelineDepthStencilStateCreateInfo ds;
	VkPipelineVertexInputStateCreateInfo vi;
	VkPipelineViewportStateCreateInfo vp;
	VkPipelineMultisampleStateCreateInfo ms;
	VkPipelineRasterizationStateCreateInfo rs;

	void reset()
	{
		memset(&vkPipeInfo, 0, sizeof(vkPipeInfo));
		memset(&shaderStages, 0, sizeof(shaderStages));
		memset(&cb, 0, sizeof(cb));
		memset(&ia, 0, sizeof(ia));
		memset(&ds, 0, sizeof(ds));
		memset(&vi, 0, sizeof(vi));
		memset(&vp, 0, sizeof(vp));
		vkPipeInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

		shaderStages[ShaderStage::Vertex].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[ShaderStage::Vertex].stage = VK_SHADER_STAGE_VERTEX_BIT;

		shaderStages[ShaderStage::Fragment].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[ShaderStage::Fragment].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

		ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		ia.primitiveRestartEnable = VK_FALSE;

		vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vi.pNext = NULL;
		vi.flags = 0;
		vi.vertexBindingDescriptionCount = 0;
		vi.vertexAttributeDescriptionCount = 0;

		cb.attachmentCount = 1;
		cb.pNext = NULL;
		cb.flags = 0;
		cb.logicOp = VK_LOGIC_OP_COPY;
		cb.logicOpEnable = VK_FALSE;

		vkPipeInfo.pColorBlendState = &cb;
		vkPipeInfo.pDepthStencilState = &ds;
		vkPipeInfo.pInputAssemblyState = &ia;
		vkPipeInfo.pMultisampleState = &ms;
		vkPipeInfo.pRasterizationState = &rs;
		vkPipeInfo.pTessellationState = NULL;
		vkPipeInfo.pVertexInputState = &vi;
		vkPipeInfo.pViewportState = &vp;
		vkPipeInfo.pDynamicState = NULL;
		vkPipeInfo.pStages = shaderStages;
		vkPipeInfo.stageCount = 2;
		resetDepthStencil().resetRasterizer().resetMultisample();

	}
	GraphicsPipelineCreate() { reset(); }

	GraphicsPipelineCreate& resetRasterizer()
	{
		memset(&rs, 0, sizeof(rs));
		rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rs.cullMode = VK_CULL_MODE_BACK_BIT;
		rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rs.polygonMode = VK_POLYGON_MODE_FILL;
		rs.lineWidth = 1.0;
		return *this;
	}

	GraphicsPipelineCreate& resetMultisample()
	{
		memset(&ms, 0, sizeof(ms));
		ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		ms.minSampleShading = 0.0f;
		return *this;
	}

	GraphicsPipelineCreate& resetDepthStencil()
	{
		memset(&ds, 0, sizeof(ds));
		ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		ds.depthTestEnable = VK_TRUE;
		ds.maxDepthBounds = 1.0f;
		ds.depthWriteEnable = VK_TRUE;
		ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		ds.front.compareMask = 0xff;
		ds.front.compareOp = VK_COMPARE_OP_ALWAYS;
		ds.front.depthFailOp = ds.front.passOp = ds.front.failOp = VK_STENCIL_OP_KEEP;
		ds.back = ds.front;
		return *this;
	}
};

struct Vertex
{
	float x, y, z, w;
};
static void writeVertexBuffer(App& app)
{
	Vertex* ptr = 0;

	vk::MapMemory(app.platformHandles->context.device, app.application->vertexBufferMemory, 0, 4096, 0, (void**)&ptr);

	// triangle
	ptr->x = -.4f; ptr->y = .4f; ptr->z = 0.0f; ptr->w = 1.0f;
	ptr++;

	ptr->x = .4f; ptr->y = .4f; ptr->z = 0.0f; ptr->w = 1.0f;
	ptr++;

	ptr->x = 0.0f; ptr->y = -.4f; ptr->z = 0.0f; ptr->w = 1.0f;
	ptr++;
	vk::UnmapMemory(app.platformHandles->context.device, app.application->vertexBufferMemory);
}

static void SetupVertexAttribs(VkVertexInputBindingDescription* bindings, VkVertexInputAttributeDescription* attributes, VkPipelineVertexInputStateCreateInfo& createInfo)
{
	VkFormat sAttributeFormat;

	sAttributeFormat = VK_FORMAT_R32G32B32A32_SFLOAT;

	bindings[0].binding = 0;
	bindings[0].stride = sizeof(Vertex);
	bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	attributes[0].location = 0;
	attributes[0].binding = 0;
	attributes[0].offset = 0;
	attributes[0].format = sAttributeFormat;
	createInfo.vertexBindingDescriptionCount = 1;
	createInfo.vertexAttributeDescriptionCount = 1;
}

bool createVertShaderModule(VkDevice device, VkShaderModule& outModule)
{
	return (vk::CreateShaderModule(device, &shaderModuleCreateInfo_vshader_vert, NULL, &outModule) == VK_SUCCESS);
}

bool createFragShaderModule(VkDevice device, VkShaderModule& outModule)
{
	return (vk::CreateShaderModule(device, &shaderModuleCreateInfo_fshader_frag, NULL, &outModule) == VK_SUCCESS);
}

static void createPipeline(App& app)
{
	//The various CreateInfos needed for a graphics pipeline
	GraphicsPipelineCreate pipeCreate;

	//These arrays is pointed to by the vertexInput create struct:
	VkVertexInputAttributeDescription attributes[16];
	VkVertexInputBindingDescription bindings[16];

	pipeCreate.vi.pVertexAttributeDescriptions = attributes;
	pipeCreate.vi.pVertexBindingDescriptions = bindings;

	//This array is pointed to by the cb create struct
	VkPipelineColorBlendAttachmentState attachments[1];

	pipeCreate.cb.pAttachments = attachments;

	//Set up the pipeline state
	pipeCreate.vkPipeInfo.pNext = NULL;

	//CreateInfos for the SetLayouts and PipelineLayouts
	VkPipelineLayoutCreateInfo sPipelineLayoutCreateInfo;
	sPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	sPipelineLayoutCreateInfo.pNext = NULL;
	sPipelineLayoutCreateInfo.flags = 0;
	sPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	sPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
	sPipelineLayoutCreateInfo.setLayoutCount = 0;
	sPipelineLayoutCreateInfo.pSetLayouts = NULL;
	vkSuccessOrDie(vk::CreatePipelineLayout(app.platformHandles->context.device,
	                                        &sPipelineLayoutCreateInfo, NULL, &app.application->emptyPipelayout),
	               "Failed to create pipeline layout");

	static VkSampleMask sampleMask = 0xffffffff;
	pipeCreate.ms.pSampleMask = &sampleMask;
	app.initColorBlendAttachmentState(attachments[0]);
	SetupVertexAttribs(bindings, attributes, pipeCreate.vi);

	VkRect2D scissors[1];
	VkViewport viewports[1];

	scissors[0].offset.x = 0;
	scissors[0].offset.y = 0;
	scissors[0].extent = app.displayHandle->displayExtent;

	pipeCreate.vp.pScissors = scissors;

	viewports[0].minDepth = 0.0f;
	viewports[0].maxDepth = 1.0f;
	viewports[0].x = 0;
	viewports[0].y = 0;
	viewports[0].width = static_cast<float>(app.displayHandle->displayExtent.width);
	viewports[0].height = static_cast<float>(app.displayHandle->displayExtent.height);

	pipeCreate.vp.pViewports = viewports;
	pipeCreate.vp.viewportCount = 1;
	pipeCreate.vp.scissorCount = 1;
	//These are only required to create the graphics pipeline, so we create and destroy them locally
	VkShaderModule vertexShaderModule;
	VkShaderModule fragmentShaderModule;

	if (!createVertShaderModule(app.platformHandles->context.device, vertexShaderModule))
	{
		LOGE("Failed to create the vertex shader\n");
		exit(0);
	}

	if (!createFragShaderModule(app.platformHandles->context.device, fragmentShaderModule))
	{
		LOGE("Failed to create the fragment shader\n");
		exit(0);
	}

	pipeCreate.ds.depthTestEnable = VK_FALSE;
	pipeCreate.vkPipeInfo.layout = app.application->emptyPipelayout;
	pipeCreate.vkPipeInfo.renderPass = app.application->renderPass;
	pipeCreate.vkPipeInfo.subpass = 0;

	pipeCreate.shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	pipeCreate.shaderStages[0].module = vertexShaderModule;
	pipeCreate.shaderStages[0].pName = "main";
	pipeCreate.shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	pipeCreate.shaderStages[1].module = fragmentShaderModule;
	pipeCreate.shaderStages[1].pName = "main";
	attachments[0].blendEnable = VK_FALSE;
	vkSuccessOrDie(vk::CreateGraphicsPipelines(app.platformHandles->context.device, VK_NULL_HANDLE, 1,
	               &pipeCreate.vkPipeInfo, NULL, &app.application->opaquePipeline),
	               "Failed to create the graphicsPipeline");
	vk::DestroyShaderModule(app.platformHandles->context.device, vertexShaderModule, NULL);
	vk::DestroyShaderModule(app.platformHandles->context.device, fragmentShaderModule, NULL);
}


static void createBuffers(App& app)
{
	VkBufferCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.pNext = NULL;
	createInfo.size = 4096;
	createInfo.flags = 0;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vk::CreateBuffer(app.platformHandles->context.device, &createInfo, NULL, &app.application->vertexBuffer);
	app.application->vertexBufferMemory = app.allocateBufferDeviceMemory(app.application->vertexBuffer);
}

static void recordCommandBuffer(App& app, uint32_t bufferIndex)
{
	app.application->cmdBuffer[bufferIndex] = app.createCommandBuffer();
	VkCommandBuffer cmd = app.application->cmdBuffer[bufferIndex];

	VkCommandBufferBeginInfo cmdBufferBeginInfo;

	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.pNext = NULL;
	cmdBufferBeginInfo.flags = 0;
	cmdBufferBeginInfo.pInheritanceInfo = NULL;

	vk::BeginCommandBuffer(cmd, &cmdBufferBeginInfo);

	VkRenderPassBeginInfo renderPassBeginInfo;
	VkClearValue clearVals[2];
	clearVals[0].color.float32[0] = 0.00f;
	clearVals[0].color.float32[1] = 0.70f;
	clearVals[0].color.float32[2] = .67f;
	clearVals[0].color.float32[3] = 1.0f;
	clearVals[1].depthStencil.depth = 1.0f;
	clearVals[1].depthStencil.stencil = 0xFF;

	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = NULL;
	renderPassBeginInfo.renderPass = app.application->renderPass;
	renderPassBeginInfo.framebuffer = app.application->framebuffer[bufferIndex];
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent = app.displayHandle->displayExtent;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = &clearVals[0];

	vk::CmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vk::CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, app.application->opaquePipeline);
	VkDeviceSize vertexOffset = 0;
	vk::CmdBindVertexBuffers(cmd, 0, 1, &app.application->vertexBuffer, &vertexOffset);
	vk::CmdDraw(cmd, 3, 1, 0, 0);
	vk::CmdEndRenderPass(cmd);
	vk::EndCommandBuffer(cmd);
}

static void recordCommandBuffer(App& app)
{
	for (uint32_t i = 0; i < app.displayHandle->swapChainLength; i++)
	{
		recordCommandBuffer(app, i);
	}
}

static void initOnScreenFbo(App& app)
{
	VkRenderPassCreateInfo renderPassCreateInfo;
	VkAttachmentDescription attachmentDescriptions[2];
	VkSubpassDescription subpassDescription = {};

	attachmentDescriptions[0].flags = 0;
	attachmentDescriptions[0].format = app.displayHandle->onscreenFbo.colorFormat;
	attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	attachmentDescriptions[1].flags = 0;
	attachmentDescriptions[1].format = app.displayHandle->onscreenFbo.depthStencilFormat;
	attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colourReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

	VkAttachmentReference dsReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colourReference;
	subpassDescription.pDepthStencilAttachment = &dsReference;

	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.pNext = NULL;
	renderPassCreateInfo.flags = 0;
	renderPassCreateInfo.attachmentCount = 2;
	renderPassCreateInfo.pAttachments = &attachmentDescriptions[0];
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.dependencyCount = 0;
	renderPassCreateInfo.pDependencies = NULL;

	vkSuccessOrDie(vk::CreateRenderPass(app.platformHandles->context.device, &renderPassCreateInfo,
	                                    NULL, &app.application->renderPass),
	               "Failed to create the renderpass");

	for (uint32_t i = 0; i < app.displayHandle->swapChainLength; i++)
	{
		VkFramebufferCreateInfo fbCreateInfo;
		VkImageView attachments[2];

		attachments[0] = app.displayHandle->onscreenFbo.colorImageViews[i];
		attachments[1] = app.displayHandle->onscreenFbo.depthStencilImageView[i];

		fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbCreateInfo.pNext = NULL;
		fbCreateInfo.flags = 0;
		fbCreateInfo.renderPass = app.application->renderPass;
		fbCreateInfo.layers = 1;
		fbCreateInfo.attachmentCount = 2;
		fbCreateInfo.pAttachments = &attachments[0];

		fbCreateInfo.width = app.displayHandle->displayExtent.width;
		fbCreateInfo.height = app.displayHandle->displayExtent.height;

		vkSuccessOrDie(vk::CreateFramebuffer(app.platformHandles->context.device, &fbCreateInfo, NULL,
		                                     &app.application->framebuffer[i]),
		               "Failed to create the framebuffer");
	}
}

void prepare(App& app)
{
	app.initGlobalState();

	initOnScreenFbo(app);
	createPipeline(app);
	createBuffers(app);
	writeVertexBuffer(app);
	recordCommandBuffer(app);
}

void deinit(App& app)
{
	vk::QueueWaitIdle(app.platformHandles->graphicsQueue);
	for (uint32_t i = 0; i < app.displayHandle->swapChainLength; ++i)
	{
		vk::DestroyFence(app.platformHandles->context.device, app.platformHandles->fenceAcquire[i], NULL);
		vk::DestroyFence(app.platformHandles->context.device, app.platformHandles->fencePrePresent[i], NULL);
		vk::DestroyFence(app.platformHandles->context.device, app.platformHandles->fenceRender[i], NULL);
		vk::DestroySemaphore(app.platformHandles->context.device, app.platformHandles->semaphoreCanBeginRendering[i], NULL);
		vk::DestroySemaphore(app.platformHandles->context.device, app.platformHandles->semaphoreCanPresent[i], NULL);
		vk::DestroySemaphore(app.platformHandles->context.device, app.platformHandles->semaphoreFinishedRendering[i], NULL);
		vk::DestroySemaphore(app.platformHandles->context.device, app.platformHandles->semaphoreImageAcquired[i], NULL);

		app.platformHandles->fenceAcquire[i] = VK_NULL_HANDLE;
		app.platformHandles->fencePrePresent[i] = VK_NULL_HANDLE;
		app.platformHandles->fenceRender[i] = VK_NULL_HANDLE;
		app.platformHandles->semaphoreCanBeginRendering[i] = VK_NULL_HANDLE;
		app.platformHandles->semaphoreCanPresent[i] = VK_NULL_HANDLE;
		app.platformHandles->semaphoreFinishedRendering[i] = VK_NULL_HANDLE;
		app.platformHandles->semaphoreImageAcquired[i] = VK_NULL_HANDLE;
	}
	vk::DestroySemaphore(app.platformHandles->context.device, app.platformHandles->semaphoreImageAcquired[app.displayHandle->swapChainLength], NULL);
	app.platformHandles->semaphoreImageAcquired[app.displayHandle->swapChainLength] = VK_NULL_HANDLE;

	vk::DestroyFence(app.platformHandles->context.device, app.platformHandles->fencePrePresent[app.displayHandle->swapChainLength], NULL);
	app.platformHandles->fencePrePresent[app.displayHandle->swapChainLength] = VK_NULL_HANDLE;

	vk::DestroyFence(app.platformHandles->context.device, app.platformHandles->fenceAcquire[app.displayHandle->swapChainLength], NULL);
	app.platformHandles->fenceAcquire[app.displayHandle->swapChainLength] = VK_NULL_HANDLE;

	vk::FreeCommandBuffers(app.platformHandles->context.device, app.platformHandles->commandPool, app.displayHandle->swapChainLength,
	                       app.platformHandles->postPresentCmdBuffer);

	vk::DestroyRenderPass(app.platformHandles->context.device, app.application->renderPass, NULL);
	vk::DestroyPipelineLayout(app.platformHandles->context.device, app.application->emptyPipelayout, NULL);
	vk::DestroyPipeline(app.platformHandles->context.device, app.application->opaquePipeline, NULL);
	vk::DestroyBuffer(app.platformHandles->context.device, app.application->vertexBuffer, NULL);
	vk::FreeMemory(app.platformHandles->context.device, app.application->vertexBufferMemory, NULL);
	app.deinitDisplayAndApplication();
	app.deinitGlobalState();
}

void drawFrame(App& app)
{
	VkResult presentResult;

	// queue present
	VkPipelineStageFlags pipeStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

	VkSubmitInfo sSubmitInfo = {};
	sSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	sSubmitInfo.waitSemaphoreCount = app.platformHandles->semaphoreCanBeginRendering[app.platformHandles->swapIndex] != 0;
	sSubmitInfo.pWaitSemaphores = &app.platformHandles->semaphoreCanBeginRendering[app.platformHandles->swapIndex];
	sSubmitInfo.pWaitDstStageMask = &pipeStageFlags;
	sSubmitInfo.pSignalSemaphores = &app.platformHandles->semaphoreFinishedRendering[app.platformHandles->swapIndex];
	sSubmitInfo.signalSemaphoreCount = app.platformHandles->semaphoreFinishedRendering[app.platformHandles->swapIndex] != 0;
	sSubmitInfo.commandBufferCount = 1;
	sSubmitInfo.pCommandBuffers = &app.application->cmdBuffer[app.platformHandles->swapIndex];

	vk::QueueSubmit(app.platformHandles->graphicsQueue, 1, &sSubmitInfo,
	                app.platformHandles->fenceRender[app.platformHandles->swapIndex]);

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pSwapchains = &app.displayHandle->swapChain;
	presentInfo.pImageIndices = &app.platformHandles->swapIndex;
	presentInfo.swapchainCount = 1;
	presentInfo.pWaitSemaphores = &app.platformHandles->semaphoreFinishedRendering[app.platformHandles->swapIndex];
	presentInfo.waitSemaphoreCount = app.platformHandles->semaphoreFinishedRendering[app.platformHandles->swapIndex] != 0;
	presentInfo.pResults = &presentResult;

	vkSuccessOrDie(vk::QueuePresentKHR(app.platformHandles->graphicsQueue, &presentInfo), "Failed to present");

	// acquire image
	app.platformHandles->lastPresentedSwapIndex = app.platformHandles->swapIndex;
	// we are reusing the same semaphore "semaphoreImageAcquired" to be signaled because we know that from previous
	// frame we have wait for this semaphore in postAcquireTransition so it is guranteed of reuse.
	app.platformHandles->currentImageAcqSem = (app.platformHandles->currentImageAcqSem + 1) % (app.displayHandle->swapChainLength + 1);
	vkSuccessOrDie(vk::AcquireNextImageKHR(app.platformHandles->context.device,
	                                       app.displayHandle->swapChain, uint64_t(-1),
	                                       app.platformHandles->semaphoreImageAcquired[app.platformHandles->currentImageAcqSem],
	                                       VK_NULL_HANDLE, &app.platformHandles->swapIndex), "AcquireNextImage error");

	// transition to color attachment
	app.submitPostPresentBarrier(app.platformHandles->swapIndex);

	// make sure the fenceRender is avaiable to be used by the commandbuffers of the application
	vk::WaitForFences(app.platformHandles->context.device, 1,
	                  &app.platformHandles->fenceRender[app.platformHandles->swapIndex], true, uint64_t(-1));
	vk::ResetFences(app.platformHandles->context.device, 1,
	                &app.platformHandles->fenceRender[app.platformHandles->swapIndex]);
}

static inline std::vector<const char*> filterExtensions(const std::vector<VkExtensionProperties>& vec,
    const char* const* filters, uint32_t numfilters)
{
	std::vector<const char*> retval;
	for (uint32_t i = 0; i < vec.size(); ++i)
	{
		for (uint32_t j = 0; j < numfilters; ++j)
		{
			if (!strcmp(vec[i].extensionName, filters[j]))
			{
				retval.push_back(filters[j]);
				break;
			}
		}
	}
	return retval;
}

static inline std::vector<const char*> filterLayers(const std::vector<VkLayerProperties>& vec,
    const char* const* filters, uint32_t numfilters)
{
	std::vector<const char*> retval;
	for (uint32_t i = 0; i < vec.size(); ++i)
	{
		for (uint32_t j = 0; j < numfilters; ++j)
		{
			if (!strcmp(vec[i].layerName, filters[j]))
			{
				retval.push_back(filters[j]);
			}
		}
	}
	return retval;
}

void App::initVkInstanceAndPhysicalDevice(bool enableLayers, bool enableExtensions)
{
	vk::initVulkan();

	VkApplicationInfo appInfo = {};
	VkInstanceCreateInfo instanceCreateInfo = {};

	uint32_t gpuCount = 100;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = NULL;
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 3);

	appInfo.applicationVersion = 1;
	appInfo.engineVersion = 0;
	appInfo.pApplicationName = "HelloAPI";
	appInfo.pEngineName = "HelloAPI";
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = NULL;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	const char* instanceValidationLayers[] =
	{
		"VK_LAYER_LUNARG_standard_validation",
	};
	const char* instanceExtNames[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_DISPLAY_EXTENSION_NAME,
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME
	};

	std::vector<const char*> enabledLayers;
	if (enableLayers)
	{
		uint32_t enabledLayerCount = 0;
		vkSuccessOrDie(vk::EnumerateInstanceLayerProperties(&enabledLayerCount, NULL),
		               "Failed to enumerate instance layer properties");

		std::vector<VkLayerProperties> layers; layers.resize(enabledLayerCount);
		vkSuccessOrDie(vk::EnumerateInstanceLayerProperties(&enabledLayerCount, layers.data()),
		               "Failed to enumerate instance layer properties");

		enabledLayers = filterLayers(layers, instanceValidationLayers, sizeof(instanceValidationLayers)
		                             / sizeof(instanceValidationLayers[0]));

		instanceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
		instanceCreateInfo.enabledLayerCount = (uint32_t)enabledLayers.size();
	}

	std::vector<const char*> enabledExtensions;
	if (enableExtensions)
	{
		uint32_t enabledExtensionCount = 0;
		vkSuccessOrDie(vk::EnumerateInstanceExtensionProperties(NULL, &enabledExtensionCount, NULL),
		               "Failed to enumerate instance extension properties");

		std::vector<VkExtensionProperties> extensions; extensions.resize(enabledExtensionCount);
		vkSuccessOrDie(vk::EnumerateInstanceExtensionProperties(NULL, &enabledExtensionCount, extensions.data()),
		               "Failed to enumerate instance extension properties");

		enabledExtensions = filterExtensions(extensions, instanceExtNames, sizeof(instanceExtNames)
		                                     / sizeof(instanceExtNames[0]));

		instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
		instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
	}

	// create the vk instance
	vkSuccessOrDie(vk::CreateInstance(&instanceCreateInfo, NULL, &this->platformHandles->context.instance),
	               "Failed to create instance");
	vk::initVulkanInstance(this->platformHandles->context.instance);

	vkSuccessOrDie(vk::EnumeratePhysicalDevices(this->platformHandles->context.instance, &gpuCount, NULL), "");
	LOGI("Number of Vulkan Physical devices: [%d]\n", gpuCount);

	vkSuccessOrDie(vk::EnumeratePhysicalDevices(this->platformHandles->context.instance, &gpuCount,
	               &this->platformHandles->context.physicalDevice), "");

	if (vk::CreateDebugReportCallbackEXT &&
	    vk::DebugReportMessageEXT &&
	    vk::DestroyDebugReportCallbackEXT)
	{
		// Setup callback creation information
		VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
		callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		callbackCreateInfo.pNext = nullptr;
		callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
		                           VK_DEBUG_REPORT_WARNING_BIT_EXT |
		                           VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		                           VK_DEBUG_REPORT_DEBUG_BIT_EXT;
		callbackCreateInfo.pfnCallback = &CustomDebugReportCallback;
		callbackCreateInfo.pUserData = nullptr;

		// Register the callback
		VkResult result = vk::CreateDebugReportCallbackEXT(this->platformHandles->context.instance, &callbackCreateInfo,
		                  nullptr, &this->platformHandles->debugReportCallback);

		LOGE("debug callback result: %i\n", result);

		if (result == VK_SUCCESS)
		{
			this->platformHandles->supportsDebugReport = true;
		}
		else
		{
			this->platformHandles->supportsDebugReport = false;
		}
	}
}

inline void editPhysicalDeviceFeatures(VkPhysicalDeviceFeatures& features)
{
	features.robustBufferAccess = false;
}

inline bool App::initDevice(bool enableLayers)
{
	VkPhysicalDeviceFeatures physicalFeatures = {};
	vk::GetPhysicalDeviceFeatures(this->platformHandles->context.physicalDevice, &physicalFeatures);
	editPhysicalDeviceFeatures(physicalFeatures);
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	VkDeviceCreateInfo deviceCreateInfo = {};
	// create the queue
	float priority = 1.f;
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.pNext = NULL;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.queueFamilyIndex = this->platformHandles->graphicsQueueIndex;
	queueCreateInfo.pQueuePriorities = &priority;
	queueCreateInfo.flags = 0;

	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = NULL;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.pEnabledFeatures = &physicalFeatures;

	const char* deviceValidationlayers[] =
	{
		"VK_LAYER_LUNARG_threading",
		"VK_LAYER_LUNARG_mem_tracker",
		"VK_LAYER_LUNARG_object_tracker",
		"VK_LAYER_LUNARG_draw_state",
		"VK_LAYER_LUNARG_param_checker",
		"VK_LAYER_LUNARG_swapchain",
		"VK_LAYER_LUNARG_device_limits",
		"VK_LAYER_LUNARG_image",
		"VK_LAYER_GOOGLE_unique_objects",
		"VK_LAYER_LUNARG_api_dump",
		"VK_LAYER_LUNARG_standard_validation",
	};
	const char* deviceExtNames[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	deviceCreateInfo.enabledExtensionCount = 1;
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtNames;
	std::vector<const char*> enabledLayers;

	if (enableLayers)
	{
		uint32_t enabledLayerCount = 0;
		vkSuccessOrDie(vk::EnumerateInstanceLayerProperties(&enabledLayerCount, NULL),
		               "Failed to enumerate instance layer properties");

		std::vector<VkLayerProperties> layers; layers.resize(enabledLayerCount);
		vkSuccessOrDie(vk::EnumerateInstanceLayerProperties(&enabledLayerCount, layers.data()),
		               "Failed to enumerate instance layer properties");

		enabledLayers = filterLayers(layers, deviceValidationlayers, sizeof(deviceValidationlayers)
		                             / sizeof(deviceValidationlayers[0]));

		deviceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
	}

	vkSuccessOrDie(vk::CreateDevice(this->platformHandles->context.physicalDevice, &deviceCreateInfo, NULL,
	                                &this->platformHandles->context.device), "Vulkan Device Creation");
	vk::initVulkanDevice(this->platformHandles->context.device);

	// Gather physical device memory properties
	vk::GetPhysicalDeviceMemoryProperties(this->platformHandles->context.physicalDevice,
	                                      &this->platformHandles->deviceMemProperties);
	vk::GetDeviceQueue(this->platformHandles->context.device, 0, 0, &this->platformHandles->graphicsQueue);
	return true;
}

void App::initSwapChain()
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkSuccessOrDie(vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(this->platformHandles->context.physicalDevice, this->displayHandle->surface, &surfaceCapabilities),
	               "Failed to get the surface capabilities");

	LOGI("Surface Capabilities:\n");
	LOGI("Image count: %u - %u\n", surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
	LOGI("Array size: %u\n", surfaceCapabilities.maxImageArrayLayers);
	LOGI("Image size (now): %dx%d\n", surfaceCapabilities.currentExtent.width, surfaceCapabilities.currentExtent.height);
	LOGI("Image size (extent): %dx%d - %dx%d\n", surfaceCapabilities.minImageExtent.width, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.width,
	     surfaceCapabilities.maxImageExtent.height);
	LOGI("Usage: %x\n", surfaceCapabilities.supportedUsageFlags);
	LOGI("Current transform: %u\n", surfaceCapabilities.currentTransform);

	uint32_t formatCount = 0;
	vk::GetPhysicalDeviceSurfaceFormatsKHR(this->platformHandles->context.physicalDevice, this->displayHandle->surface,
	                                       &formatCount, NULL);
	VkSurfaceFormatKHR tmpformats[16]; std::vector<VkSurfaceFormatKHR> tmpFormatsVector;
	VkSurfaceFormatKHR* allFormats = tmpformats;
	if (formatCount > 16)
	{
		tmpFormatsVector.resize(formatCount);
		allFormats = tmpFormatsVector.data();
	}
	vk::GetPhysicalDeviceSurfaceFormatsKHR(this->platformHandles->context.physicalDevice, this->displayHandle->surface,
	                                       &formatCount, allFormats);

	VkSurfaceFormatKHR format = allFormats[0];

	VkFormat preferredColorFormats[] =
	{
		VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_R8G8B8A8_SNORM,
		VK_FORMAT_B8G8R8_SNORM, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_R5G6B5_UNORM_PACK16
	};

	bool foundFormat = false;
	for (unsigned int i = 0; i < (sizeof(preferredColorFormats) / sizeof(preferredColorFormats[0])) && !foundFormat; ++i)
	{
		for (uint32_t f = 0; f < formatCount; ++f)
		{
			if (allFormats[f].format == preferredColorFormats[i])
			{
				format = allFormats[f]; foundFormat = true; break;
			}
		}
	}

	this->displayHandle->onscreenFbo.depthStencilHasStencil = false;
	VkFormat dsFormatRequested = VK_FORMAT_D32_SFLOAT;
	this->displayHandle->onscreenFbo.depthStencilFormat = VK_FORMAT_UNDEFINED;
	VkFormat preferredDsFormat[] =
	{
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D16_UNORM,
		VK_FORMAT_X8_D24_UNORM_PACK32
	};

	VkFormat currentDsFormat = dsFormatRequested;
	for (uint32_t f = 0; f < sizeof(preferredDsFormat) / sizeof(preferredDsFormat[0]); ++f)
	{
		VkFormatProperties prop;
		vk::GetPhysicalDeviceFormatProperties(this->platformHandles->context.physicalDevice, currentDsFormat, &prop);
		if (prop.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			this->displayHandle->onscreenFbo.depthStencilFormat = currentDsFormat;
			break;
		}
		currentDsFormat = preferredDsFormat[f];
	}

	switch (this->displayHandle->onscreenFbo.depthStencilFormat)
	{
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
	case VK_FORMAT_D24_UNORM_S8_UINT:
	case VK_FORMAT_D16_UNORM_S8_UINT:
		this->displayHandle->onscreenFbo.depthStencilHasStencil = true;
		break;
	default:
		this->displayHandle->onscreenFbo.depthStencilHasStencil = false;
	}

	// Use FIFO mode: no tearing, good battery use
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	this->displayHandle->onscreenFbo.colorFormat = format.format;
	this->displayHandle->displayExtent = surfaceCapabilities.currentExtent;

	//--- create the swap chain
	VkSwapchainCreateInfoKHR swapchainCreate;
	swapchainCreate.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreate.pNext = NULL;
	swapchainCreate.flags = 0;
	swapchainCreate.clipped = VK_TRUE;
	swapchainCreate.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreate.surface = this->displayHandle->surface;
	swapchainCreate.minImageCount = std::max(surfaceCapabilities.minImageCount, std::min(surfaceCapabilities.maxImageCount, 2u));
	swapchainCreate.imageFormat = this->displayHandle->onscreenFbo.colorFormat;
	swapchainCreate.imageArrayLayers = 1;
	swapchainCreate.imageColorSpace = format.colorSpace;
	swapchainCreate.imageExtent = surfaceCapabilities.currentExtent;
	swapchainCreate.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreate.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainCreate.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreate.presentMode = swapchainPresentMode;
	swapchainCreate.oldSwapchain = VK_NULL_HANDLE;
	swapchainCreate.queueFamilyIndexCount = 0;
	swapchainCreate.pQueueFamilyIndices = NULL;
	vkSuccessOrDie(vk::CreateSwapchainKHR(this->platformHandles->context.device, &swapchainCreate,
	                                      NULL, &this->displayHandle->swapChain), "Could not create the swap chain");
	// get the number of swapchains
	vkSuccessOrDie(vk::GetSwapchainImagesKHR(this->platformHandles->context.device, this->displayHandle->swapChain,
	               &this->displayHandle->swapChainLength, NULL), "Could not get swapchain length");

	this->displayHandle->onscreenFbo.colorImages.resize(this->displayHandle->swapChainLength);
	this->displayHandle->onscreenFbo.colorImageViews.resize(this->displayHandle->swapChainLength);
	vkSuccessOrDie(vk::GetSwapchainImagesKHR(this->platformHandles->context.device, this->displayHandle->swapChain,
	               &this->displayHandle->swapChainLength, &this->displayHandle->onscreenFbo.colorImages[0]), "Could not get swapchain images");

	//--- create the swapchain view
	VkImageViewCreateInfo viewCreateInfo;
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.pNext = NULL;
	viewCreateInfo.flags = 0;
	viewCreateInfo.image = VK_NULL_HANDLE;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = this->displayHandle->onscreenFbo.colorFormat;
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewCreateInfo.subresourceRange.baseMipLevel = 0;
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;
	viewCreateInfo.subresourceRange.layerCount = 1;
	this->displayHandle->onscreenFbo.depthStencilImage.resize(this->displayHandle->swapChainLength);
	this->displayHandle->onscreenFbo.depthStencilImageView.resize(this->displayHandle->swapChainLength);

	for (uint32_t i = 0; i < this->displayHandle->swapChainLength; ++i)
	{
		viewCreateInfo.image = this->displayHandle->onscreenFbo.colorImages[i];
		vkSuccessOrDie(vk::CreateImageView(this->platformHandles->context.device, &viewCreateInfo, NULL,
		                                   &this->displayHandle->onscreenFbo.colorImageViews[i]), "create display image view");

		// create the depth stencil image
		VkImageCreateInfo dsCreateInfo = {};
		dsCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		dsCreateInfo.pNext = NULL;
		dsCreateInfo.flags = 0;
		dsCreateInfo.format = this->displayHandle->onscreenFbo.depthStencilFormat;
		dsCreateInfo.extent.width = this->displayHandle->displayExtent.width;
		dsCreateInfo.extent.height = this->displayHandle->displayExtent.height;
		dsCreateInfo.extent.depth = 1;
		dsCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		dsCreateInfo.arrayLayers = 1;
		dsCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		dsCreateInfo.mipLevels = 1;
		dsCreateInfo.flags = 0;
		dsCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		dsCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
		                     (this->displayHandle->onscreenFbo.depthStencilHasStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
		dsCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		dsCreateInfo.pQueueFamilyIndices = 0;
		dsCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		dsCreateInfo.queueFamilyIndexCount = 0;
		dsCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult rslt = vk::CreateImage(this->platformHandles->context.device, &dsCreateInfo, NULL, &this->displayHandle->onscreenFbo.depthStencilImage[i].first);
		vkSuccessOrDie(rslt, "Image creation failed");

		this->displayHandle->onscreenFbo.depthStencilImage[i].second = allocateImageDeviceMemory(this->displayHandle->onscreenFbo.depthStencilImage[i].first, NULL);
		if (this->displayHandle->onscreenFbo.depthStencilImage[i].second == VK_NULL_HANDLE)
		{
			LOGE("Memory allocation failed\n");
			exit(0);
		}
		// create the depth stencil view
		VkImageViewCreateInfo dsViewCreateInfo;
		dsViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		dsViewCreateInfo.pNext = NULL;
		dsViewCreateInfo.image = this->displayHandle->onscreenFbo.depthStencilImage[i].first;
		dsViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		dsViewCreateInfo.format = this->displayHandle->onscreenFbo.depthStencilFormat;
		dsViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		dsViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		dsViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		dsViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		dsViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		dsViewCreateInfo.subresourceRange.baseMipLevel = 0;
		dsViewCreateInfo.subresourceRange.levelCount = 1;
		dsViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		dsViewCreateInfo.subresourceRange.layerCount = 1;
		dsViewCreateInfo.flags = 0;
		vkSuccessOrDie(vk::CreateImageView(this->platformHandles->context.device, &dsViewCreateInfo,
		                                   NULL, &this->displayHandle->onscreenFbo.depthStencilImageView[i]),
		               "Create Depth stencil image view");
	}
}

void App::initSynchronizationObjects()
{
	// create the semaphores
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (uint32_t i = 0; i < this->displayHandle->swapChainLength; ++i)
	{
		vkSuccessOrDie(vk::CreateSemaphore(this->platformHandles->context.device, &semaphoreCreateInfo,
		                                   NULL, &this->platformHandles->semaphoreFinishedRendering[i]),
		               "Cannot create the Semaphore used to signal rendering finished");
		vkSuccessOrDie(vk::CreateSemaphore(this->platformHandles->context.device, &semaphoreCreateInfo,
		                                   NULL, &this->platformHandles->semaphoreCanBeginRendering[i]),
		               "Cannot create the Presentation Semaphore");
		vkSuccessOrDie(vk::CreateSemaphore(this->platformHandles->context.device, &semaphoreCreateInfo,
		                                   NULL, &this->platformHandles->semaphoreCanPresent[i]),
		               "Cannot create the Presentation Semaphore");
		vkSuccessOrDie(vk::CreateSemaphore(this->platformHandles->context.device, &semaphoreCreateInfo,
		                                   NULL, &this->platformHandles->semaphoreImageAcquired[i]),
		               "Cannot create the Swapchain Image Acquisition Semaphore");
		vkSuccessOrDie(vk::CreateFence(this->platformHandles->context.device, &fenceCreateInfo,
		                               NULL, &this->platformHandles->fencePrePresent[i]), "Failed to create fence");
		vkSuccessOrDie(vk::CreateFence(this->platformHandles->context.device, &fenceCreateInfo,
		                               NULL, &this->platformHandles->fenceRender[i]), "Failed to create fence");
		vkSuccessOrDie(vk::CreateFence(this->platformHandles->context.device, &fenceCreateInfo,
		                               NULL, &this->platformHandles->fenceAcquire[i]), "Failed to create fence");
	}

	vkSuccessOrDie(vk::CreateFence(this->platformHandles->context.device, &fenceCreateInfo,
	                               NULL, &this->platformHandles->fencePrePresent[this->displayHandle->swapChainLength]),
	               "Failed to create fence");
	vkSuccessOrDie(vk::CreateFence(this->platformHandles->context.device, &fenceCreateInfo,
	                               NULL, &this->platformHandles->fenceAcquire[this->displayHandle->swapChainLength]),
	               "Failed to create fence");
	vkSuccessOrDie(vk::CreateSemaphore(this->platformHandles->context.device, &semaphoreCreateInfo,
	                                   NULL, &this->platformHandles->semaphoreImageAcquired[this->displayHandle->swapChainLength]),
	               "Cannot create the Swapchain Image Acquisition Semaphore");
}

static void inline setImageLayout(VkCommandBuffer& cmd, VkImageLayout oldLayout, VkImageLayout newLayout,
                                  VkImageAspectFlags aspectMask, VkAccessFlags srcAccessMask, VkImage image)
{
	VkImageMemoryBarrier imageMemBarrier = {};
	imageMemBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemBarrier.pNext = NULL;
	imageMemBarrier.srcAccessMask = srcAccessMask;
	imageMemBarrier.dstAccessMask = 0;
	imageMemBarrier.oldLayout = oldLayout;
	imageMemBarrier.newLayout = newLayout;
	imageMemBarrier.image = image;
	imageMemBarrier.subresourceRange = { aspectMask, 0, 1, 0, 1 };

	if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		/* Make sure anything that was copying from this image has completed */
		imageMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}

	if (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		imageMemBarrier.dstAccessMask =
		  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		imageMemBarrier.dstAccessMask =
		  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}

	if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		/* Make sure any Copy or CPU writes to image are flushed */
		imageMemBarrier.dstAccessMask =
		  VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	}

	VkImageMemoryBarrier* memBarries = &imageMemBarrier;
	vk::CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, NULL, 0,
	                       NULL, 1, memBarries);
}

void App::setInitialSwapchainLayouts()
{
	VkCommandBuffer cmdImgLayoutTrans = createCommandBuffer();

	VkCommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	vkSuccessOrDie(vk::BeginCommandBuffer(cmdImgLayoutTrans, &cmdBeginInfo), "Failed to begin commandbuffer");

	for (uint32_t i = 0; i < this->displayHandle->swapChainLength; ++i)
	{
		// prepare the current swapchain image for writing
		if (i == this->platformHandles->swapIndex)
		{
			setImageLayout(cmdImgLayoutTrans, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			               VK_IMAGE_ASPECT_COLOR_BIT, 0, this->displayHandle->onscreenFbo.colorImages[i]);
		}
		else// set all other swapchains to present so they will be transformed properly later.
		{
			setImageLayout(cmdImgLayoutTrans, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			               VK_IMAGE_ASPECT_COLOR_BIT, 0, this->displayHandle->onscreenFbo.colorImages[i]);
		}
		setImageLayout(cmdImgLayoutTrans, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		               VK_IMAGE_ASPECT_DEPTH_BIT | (this->displayHandle->onscreenFbo.depthStencilHasStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0),
		               0, this->displayHandle->onscreenFbo.depthStencilImage[i].first);
	}

	vk::EndCommandBuffer(cmdImgLayoutTrans);
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pCommandBuffers = &cmdImgLayoutTrans;
	submitInfo.commandBufferCount = 1;
	submitInfo.pSignalSemaphores = &this->platformHandles->semaphoreCanBeginRendering[this->platformHandles->swapIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &this->platformHandles->semaphoreImageAcquired[this->platformHandles->currentImageAcqSem];
	submitInfo.waitSemaphoreCount = 1;

	VkPipelineStageFlags pipeStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	submitInfo.pWaitDstStageMask = &pipeStageFlags;

	VkFence fence;
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vk::CreateFence(this->platformHandles->context.device, &fenceInfo, NULL, &fence);
	vk::QueueSubmit(this->platformHandles->graphicsQueue, 1, &submitInfo, fence);
	vk::WaitForFences(this->platformHandles->context.device, 1, &fence, true, uint64_t(-1));
	vk::DestroyFence(this->platformHandles->context.device, fence, NULL);
	vk::FreeCommandBuffers(this->platformHandles->context.device, this->platformHandles->commandPool, 1, &cmdImgLayoutTrans);
}

void App::initPostPresentBarrierCommandBuffer()
{
	VkCommandBufferAllocateInfo cinfo = {};
	cinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cinfo.commandPool = this->platformHandles->commandPool;
	cinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	cinfo.commandBufferCount = this->displayHandle->swapChainLength;

	vk::AllocateCommandBuffers(this->platformHandles->context.device, &cinfo, this->platformHandles->postPresentCmdBuffer);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = NULL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	VkCommandBufferBeginInfo beginnfo = {};
	beginnfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	for (uint32_t swapIndex = 0; swapIndex < this->displayHandle->swapChainLength; ++swapIndex)
	{
		// post present
		barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		barrier.image = this->displayHandle->onscreenFbo.colorImages[swapIndex];
		vk::BeginCommandBuffer(this->platformHandles->postPresentCmdBuffer[swapIndex], &beginnfo);
		vk::CmdPipelineBarrier(this->platformHandles->postPresentCmdBuffer[swapIndex],
		                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, NULL, 0,
		                       NULL, 1, &barrier);
		vk::EndCommandBuffer(this->platformHandles->postPresentCmdBuffer[swapIndex]);
	}
}

void App::initGlobalState()
{
	initVkInstanceAndPhysicalDevice(true, true);
	initSurface();
	initDevice(true);
	initSwapChain();
	VkCommandPoolCreateInfo cmdPoolCreateInfo;
	cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolCreateInfo.pNext = NULL;
	cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cmdPoolCreateInfo.queueFamilyIndex = 0;
	vk::CreateCommandPool(this->platformHandles->context.device, &cmdPoolCreateInfo, NULL, &this->platformHandles->commandPool);

	initSynchronizationObjects();

	vkSuccessOrDie(vk::AcquireNextImageKHR(this->platformHandles->context.device, this->displayHandle->swapChain, uint64_t(-1),
	                                       this->platformHandles->semaphoreImageAcquired[this->platformHandles->currentImageAcqSem],
	                                       VK_NULL_HANDLE, &this->platformHandles->swapIndex),
	               "Failed to acquire initial Swapchain image");

	setInitialSwapchainLayouts();

	initPostPresentBarrierCommandBuffer();

	vk::ResetFences(this->platformHandles->context.device, 1,
	                &this->platformHandles->fenceRender[this->platformHandles->swapIndex]);
}

bool App::getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlagBits properties,
                             uint32_t& outTypeIndex)
{
	for (uint32_t i = 0; i < 32; ++i)
	{
		if ((typeBits & 1) == 1)
		{
			if ((this->platformHandles->deviceMemProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				outTypeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	return false;
}

void App::deinitGlobalState()
{
	if (this->platformHandles->debugReportCallback && this->platformHandles->supportsDebugReport)
	{
		vk::DestroyDebugReportCallbackEXT(this->platformHandles->context.instance,
		                                  this->platformHandles->debugReportCallback, NULL);
	}

	vk::DestroyDevice(this->platformHandles->context.device, NULL);
	this->platformHandles->commandPool = VK_NULL_HANDLE;

	vk::DestroyInstance(this->platformHandles->context.instance, NULL);
}

VkDeviceMemory App::allocateImageDeviceMemory(VkImage image, VkMemoryRequirements* memoryRequirementsOut)
{
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkMemoryAllocateInfo sMemoryAllocInfo;
	VkMemoryRequirements* pMemoryRequirements;
	VkMemoryRequirements memoryRequirements;
	if (memoryRequirementsOut)
	{
		pMemoryRequirements = memoryRequirementsOut;
	}
	else
	{
		pMemoryRequirements = &memoryRequirements;
	}

	vk::GetImageMemoryRequirements(this->platformHandles->context.device, image, pMemoryRequirements);

	sMemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	sMemoryAllocInfo.pNext = NULL;
	sMemoryAllocInfo.allocationSize = pMemoryRequirements->size;

	//Find the first allowed type:
	if (pMemoryRequirements->memoryTypeBits == 0)
	{
		LOGE("unsupported memory type bits\n");
		exit(0);
	}

	getMemoryTypeIndex(pMemoryRequirements->memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	                   sMemoryAllocInfo.memoryTypeIndex);
	vk::AllocateMemory(this->platformHandles->context.device, &sMemoryAllocInfo, NULL, &memory);
	vk::BindImageMemory(this->platformHandles->context.device, image, memory, 0);

	return memory;
}

VkDeviceMemory App::allocateBufferDeviceMemory(VkBuffer buffer, VkMemoryRequirements* memoryRequirementsOut)
{
	VkDeviceMemory memory;
	VkMemoryAllocateInfo sMemoryAllocInfo;
	VkMemoryRequirements* pMemoryRequirements;
	VkMemoryRequirements memoryRequirements;
	if (memoryRequirementsOut)
	{
		pMemoryRequirements = memoryRequirementsOut;
	}
	else
	{
		pMemoryRequirements = &memoryRequirements;
	}

	vk::GetBufferMemoryRequirements(this->platformHandles->context.device, buffer, pMemoryRequirements);

	sMemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	sMemoryAllocInfo.pNext = NULL;
	sMemoryAllocInfo.allocationSize = pMemoryRequirements->size;

	//Find the first allowed type:
	if (pMemoryRequirements->memoryTypeBits == 0)
	{
		LOGE("Invalid memory type bits\n");
		exit(0);
	}
	getMemoryTypeIndex(pMemoryRequirements->memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
	                   sMemoryAllocInfo.memoryTypeIndex);
	vk::AllocateMemory(this->platformHandles->context.device, &sMemoryAllocInfo, NULL, &memory);
	vk::BindBufferMemory(this->platformHandles->context.device, buffer, memory, 0);

	return memory;
}

void App::initColorBlendAttachmentState(VkPipelineColorBlendAttachmentState& state)
{
	state.blendEnable = VK_TRUE;
	state.colorWriteMask = 0xf;

	state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	state.colorBlendOp = VK_BLEND_OP_ADD;

	state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	state.alphaBlendOp = VK_BLEND_OP_ADD;
}

void App::initSurface()
{
	VkDisplayPropertiesKHR properties;
	uint32_t propertiesCount = 1;
	if (vk::GetPhysicalDeviceDisplayPropertiesKHR)
	{
		vkSuccessOrDie(vk::GetPhysicalDeviceDisplayPropertiesKHR(this->platformHandles->context.physicalDevice, &propertiesCount, &properties),
		               "Failed to get the physical device display properties");
	}

	std::string supportedTransforms;
	if (properties.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) { supportedTransforms.append("none "); }
	if (properties.supportedTransforms & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR) { supportedTransforms.append("rot90 "); }
	if (properties.supportedTransforms & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR) { supportedTransforms.append("rot180 "); }
	if (properties.supportedTransforms & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR) { supportedTransforms.append("rot270 "); }
	if (properties.supportedTransforms & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR) { supportedTransforms.append("h_mirror "); }
	if (properties.supportedTransforms & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR) { supportedTransforms.append("h_mirror+rot90 "); }
	if (properties.supportedTransforms & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR) { supportedTransforms.append("hmirror+rot180 "); }
	if (properties.supportedTransforms & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR) { supportedTransforms.append("hmirror+rot270 "); }
	if (properties.supportedTransforms & VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR) { supportedTransforms.append("inherit "); }

	displayHandle->nativeDisplay = properties.display;

	uint32_t modeCount = 0;
	vkSuccessOrDie(vk::GetDisplayModePropertiesKHR(this->platformHandles->context.physicalDevice, this->displayHandle->nativeDisplay, &modeCount, NULL),
	               "Failed to get the display mode propertes");
	std::vector<VkDisplayModePropertiesKHR> modeProperties; modeProperties.resize(modeCount);
	vkSuccessOrDie(vk::GetDisplayModePropertiesKHR(this->platformHandles->context.physicalDevice, this->displayHandle->nativeDisplay,
	               &modeCount, modeProperties.data()),
	               "Failed to get the display mode propertes");

	VkDisplaySurfaceCreateInfoKHR surfaceCreateInfo;

	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.pNext = NULL;

	surfaceCreateInfo.displayMode = modeProperties[0].displayMode;
	surfaceCreateInfo.planeIndex = 0;
	surfaceCreateInfo.planeStackIndex = 0;
	surfaceCreateInfo.transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	surfaceCreateInfo.globalAlpha = 0.0f;
	surfaceCreateInfo.alphaMode = VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR;
	surfaceCreateInfo.imageExtent = modeProperties[0].parameters.visibleRegion;

	vkSuccessOrDie(vk::CreateDisplayPlaneSurfaceKHR(this->platformHandles->context.instance, &surfaceCreateInfo, NULL, &this->displayHandle->surface),
	               "Could not create DisplayPlane Surface");

	uint32_t numQueues = 1;
	vk::GetPhysicalDeviceQueueFamilyProperties(this->platformHandles->context.physicalDevice, &numQueues, NULL);
	assert(numQueues >= 1);
	std::vector<VkQueueFamilyProperties> queueProperties;
	queueProperties.resize(numQueues);
	this->platformHandles->graphicsQueueIndex = 0;
	vk::GetPhysicalDeviceQueueFamilyProperties(this->platformHandles->context.physicalDevice, &numQueues, &queueProperties[0]);

	// find the queue that support both graphics and present
	std::vector<VkBool32> supportsPresent(numQueues);
	for (uint32_t i = 0; i < numQueues; ++i)
	{
		vkSuccessOrDie(vk::GetPhysicalDeviceSurfaceSupportKHR(this->platformHandles->context.physicalDevice, i, this->displayHandle->surface, &supportsPresent[i]),
		               "Failed to get physical device surface support");
	}
	uint32_t graphicsQueueIndex = uint32_t(-1);
	uint32_t presentQueueIndex = uint32_t(-1);
	for (uint32_t i = 0; i < numQueues; ++i)
	{
		if ((queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			if (graphicsQueueIndex == uint32_t(-1))
			{
				graphicsQueueIndex = i;
			}

			if (supportsPresent[i] == VK_TRUE)
			{
				graphicsQueueIndex = i;
				presentQueueIndex = i;
				break;
			}
		}
	}
	if (graphicsQueueIndex == uint32_t(-1))
	{
		// If didn't find a queue that supports both graphics and present, then
		// find a separate present queue.
		for (uint32_t i = 0; i < numQueues; ++i)
		{
			if (supportsPresent[i] == VK_TRUE)
			{
				presentQueueIndex = i;
				break;
			}
		}
	}
	if (graphicsQueueIndex == uint32_t(-1) || presentQueueIndex == uint32_t(-1))
	{
		LOGI("Could not find a graphics and a present queue Swapchain Initialization Failed\n");
	}
	// NOTE: While it is possible for an application to use a separate graphics
	//       and a present queues, the framework program assumes it is only using one
	if (graphicsQueueIndex != presentQueueIndex)
	{
		LOGI("Could not find a common graphics and a present queue Swapchain Initialization Failure\n");
	}
	graphicsQueueIndex = graphicsQueueIndex;
}


void App::deinitDisplayAndApplication()
{
	// release the display
	for (uint32_t i = 0; i < this->displayHandle->swapChainLength; ++i)
	{
		vk::DestroyImageView(this->platformHandles->context.device, this->displayHandle->onscreenFbo.colorImageViews[i], NULL);
		vk::DestroyImageView(this->platformHandles->context.device, this->displayHandle->onscreenFbo.depthStencilImageView[i], NULL);
		vk::DestroyImage(this->platformHandles->context.device, this->displayHandle->onscreenFbo.depthStencilImage[i].first, NULL);
		vk::FreeMemory(this->platformHandles->context.device, this->displayHandle->onscreenFbo.depthStencilImage[i].second, NULL);

		vk::DestroyFramebuffer(this->platformHandles->context.device, this->application->framebuffer[i], NULL);
	}
	vk::DestroySwapchainKHR(this->platformHandles->context.device, this->displayHandle->swapChain, NULL);
	vk::DestroySurfaceKHR(this->platformHandles->context.instance, this->displayHandle->surface, NULL);

	vk::FreeCommandBuffers(this->platformHandles->context.device, this->platformHandles->commandPool,
	                       this->displayHandle->swapChainLength,
	                       this->application->cmdBuffer);

	vk::DestroyCommandPool(this->platformHandles->context.device, this->platformHandles->commandPool, NULL);
}

void App::submitPostPresentBarrier(uint32_t swapchain)
{
	vk::WaitForFences(this->platformHandles->context.device, 1, &this->platformHandles->fenceAcquire[swapchain], true, uint64_t(-1));
	vk::ResetFences(this->platformHandles->context.device, 1, &this->platformHandles->fenceAcquire[swapchain]);

	//LAYOUT TRANSITION COLOR ATTACHMENT -> PRESENTATION SRC
	VkSubmitInfo snfo = {};
	snfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	snfo.commandBufferCount = 1;
	snfo.pCommandBuffers = &this->platformHandles->postPresentCmdBuffer[this->platformHandles->swapIndex];
	snfo.pWaitSemaphores = &this->platformHandles->semaphoreImageAcquired[this->platformHandles->currentImageAcqSem];
	snfo.waitSemaphoreCount = 1;
	snfo.pSignalSemaphores = &this->platformHandles->semaphoreCanBeginRendering[this->platformHandles->swapIndex];
	snfo.signalSemaphoreCount = (this->platformHandles->semaphoreCanBeginRendering[this->platformHandles->swapIndex]
	                             != VK_NULL_HANDLE);
	VkPipelineStageFlags flags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	snfo.pWaitDstStageMask = &flags;

	vkSuccessOrDie(vk::QueueSubmit(this->platformHandles->graphicsQueue, 1, &snfo,
	                               this->platformHandles->fenceAcquire[swapchain]), "Post Present Image transition error");
}

VkCommandBuffer App::createCommandBuffer()
{
	VkCommandBufferAllocateInfo cmdInfo = {};
	cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdInfo.commandPool = this->platformHandles->commandPool;
	cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdInfo.commandBufferCount = 1;
	VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
	vk::AllocateCommandBuffers(this->platformHandles->context.device, &cmdInfo, &cmdBuffer);
	return cmdBuffer;
}

bool initializeWindow()
{
	return true;
}

int main(int /*argc*/, char** /*argv*/)
{
	initializeWindow();
	App app;
	app.displayHandle = new NativeDisplayHandle();
	app.platformHandles = new NativePlatformHandles();
	app.application = new HelloAPI();
	prepare(app);
	for (int i = 0; i < 600; ++i) { drawFrame(app); }
	deinit(app);

	delete app.displayHandle;
	delete app.platformHandles;
	delete app.application;
}