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
#define VK_USE_PLATFORM_ANDROID_KHR

#define VK_NO_PROTOTYPES 1
#include "vulkan/vulkan.h"
#include <android/log.h>
#include <android_native_app_glue.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "com.imgtec.vk", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "com.imgtec.vk", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_WARN, "com.imgtec.vk", __VA_ARGS__))
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <cassert>
#include <string>
#include <dlfcn.h>
#include <unistd.h>
#include "vshader_vert.h"
#include "fshader_frag.h"

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
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroySurfaceKHR)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSurfaceCapabilitiesKHR)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSurfaceFormatsKHR)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateSwapchainKHR)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetSwapchainImagesKHR)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(QueuePresentKHR)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroySwapchainKHR)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateInstance)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EnumeratePhysicalDevices)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetInstanceProcAddr)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetDeviceProcAddr)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyInstance)
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSurfacePresentModesKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSurfaceSupportKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateAndroidSurfaceKHR);
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
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(AcquireNextImageKHR);
};
#undef PVR_VULKAN_FUNCTION_POINTER_DECLARATION
#define PVR_VULKAN_FUNCTION_POINTER_DEFINITION(function_name) PFN_vk##function_name vk::function_name;
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroySurfaceKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceSurfaceCapabilitiesKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceSurfaceFormatsKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateSwapchainKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetSwapchainImagesKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(QueuePresentKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroySwapchainKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateInstance)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(EnumeratePhysicalDevices)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetInstanceProcAddr)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetDeviceProcAddr)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyInstance)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceSurfacePresentModesKHR);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceSurfaceSupportKHR);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateAndroidSurfaceKHR);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(AcquireNextImageKHR)
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
#undef PVR_VULKAN_FUNCTION_POINTER_DEFINITION
typedef void* LIBTYPE;
LIBTYPE OpenLibrary(const char* pszPath)
{
	LIBTYPE lt = dlopen(pszPath, RTLD_LAZY | RTLD_GLOBAL);
	if (!lt)
	{
		const char* err = dlerror();
		if (err)
		{
			LOGE("dlopen failed with error ");
			LOGE("%s", err);
		}
	}
	return lt;
}

struct NativeLibrary
{
public:
	/*!*********************************************************************************************************************
	\brief   Check if the library was loaded properly.
	\return   True if the library did NOT load properly.
	***********************************************************************************************************************/
	bool LoadFailed();
	bool m_disableErrorPrint; //!< Set to true to avoid printing errors

	/*!*********************************************************************************************************************
	\brief   Load a library with the specified filename.
	\param   libraryPath The path to find the library (name or Path+name).
	***********************************************************************************************************************/
	NativeLibrary(const std::string& LibPath)
	{
		size_t start = 0;
		std::string tmp;

		while (!m_hHostLib)
		{
			size_t end = LibPath.find_first_of(';', start);

			if (end == std::string::npos)
			{
				tmp = LibPath.substr(start, LibPath.length() - start);
			}
			else
			{
				tmp = LibPath.substr(start, end - start);
			}
			if (!tmp.empty())
			{
				m_hHostLib = OpenLibrary(tmp.c_str());

				if (!m_hHostLib)
				{
					// Remove the last character in case a new line character snuck in
					tmp = tmp.substr(0, tmp.size() - 1);
					m_hHostLib = OpenLibrary(tmp.c_str());
				}
			}
			if (end == std::string::npos) { break; }
			start = end + 1;
		}
		if (!m_hHostLib)
		{
			LOGE("Could not load host library '%s'", LibPath.c_str());
			this->m_bError = true;
		}
		LOGI("Host library '%s' loaded", LibPath.c_str());
	}
	~NativeLibrary() {	CloseLib(); }

	/*!*********************************************************************************************************************
	\brief   Get a function pointer from the library.
	\param   functionName  The name of the function to retrieve the pointer to.
	\return  The function pointer as a void pointer. Null if failed. Cast to the proper type.
	***********************************************************************************************************************/
	void* getFunction(const char* functionName)
	{
		void* pFn = dlsym((LIBTYPE)m_hHostLib, functionName);
		if (pFn == NULL && m_disableErrorPrint == false)
		{
			m_bError |= (pFn == NULL);
			LOGE("Could not get function %s", functionName);
		}
		return pFn;
	}

	/*!*********************************************************************************************************************
	\brief   Get a function pointer from the library.
	\tparam  PtrType_ The type of the function pointer
	\param   functionName  The name of the function to retrieve the pointer to
	\return  The function pointer. Null if failed.
	***********************************************************************************************************************/
	template<typename PtrType_> PtrType_ getFunction(const char* functionName) {	return reinterpret_cast<PtrType_>(getFunction(functionName)); }

	/*!*********************************************************************************************************************
	\brief   Release this library.
	***********************************************************************************************************************/
	void CloseLib()
	{
		if (m_hHostLib)
		{
			dlclose((LIBTYPE)m_hHostLib);
			m_hHostLib = 0;
		}
	}
protected:
	void*		m_hHostLib;
	bool		m_bError;
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
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, DestroySurfaceKHR)
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, CreateAndroidSurfaceKHR);
	return true;
}

bool vk::initVulkanDevice(VkDevice device)
{
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateRenderPass);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroySemaphore);
	PVR_VULKAN_GET_DEVICE_POINTER(device, ResetFences);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyFence);
	PVR_VULKAN_GET_DEVICE_POINTER(device, WaitForFences);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateImage);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetImageMemoryRequirements);
	PVR_VULKAN_GET_DEVICE_POINTER(device, AllocateMemory);
	PVR_VULKAN_GET_DEVICE_POINTER(device, BindImageMemory);
	PVR_VULKAN_GET_DEVICE_POINTER(device, QueueWaitIdle);
	PVR_VULKAN_GET_DEVICE_POINTER(device, BeginCommandBuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CmdPipelineBarrier);
	PVR_VULKAN_GET_DEVICE_POINTER(device, EndCommandBuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, AllocateCommandBuffers);
	PVR_VULKAN_GET_DEVICE_POINTER(device, FreeCommandBuffers);
	PVR_VULKAN_GET_DEVICE_POINTER(device, QueueSubmit);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateSwapchainKHR);//
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetSwapchainImagesKHR);//
	PVR_VULKAN_GET_DEVICE_POINTER(device, QueuePresentKHR);//
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroySwapchainKHR);//
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyImageView);//
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyDevice);//
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateImageView);//
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateCommandPool);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyCommandPool);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateFramebuffer);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreatePipelineLayout);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetDeviceQueue);
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
	return true;
}

inline void vkSuccessOrDie(VkResult result, const char* msg)
{
	if (result != VK_SUCCESS)
	{
		LOGE("Failed: %s", msg);
		exit(0);
	}
}

struct App;
struct Context
{
	VkDevice device;
	VkInstance instance;
	VkPhysicalDevice gpu;
	VkQueue graphicsQueue;
	uint32_t graphicsQueueIndex;
	App* app;
	VkCommandPool cmdPool;
	VkCommandBuffer postPresentCmdBuffer;
	struct NativeDisplayHandle
	{
		ANativeWindow* nativeDisplay;
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
		} fb;
	};

	NativeDisplayHandle displayHandle;
	VkPhysicalDeviceMemoryProperties deviceMemProps;
	void initVkInstanceAndPhysicalDevice(bool enableLayers);
	VkDeviceMemory allocateImageDeviceMemory(VkImage image, VkMemoryRequirements* memoryRequirementsOut = 0);
	VkDeviceMemory allocateBufferDeviceMemory(VkBuffer buffer, VkMemoryRequirements* memoryRequirementsOut = 0);
	void initSwapChain();
	void initColorBlendAttachmentState(VkPipelineColorBlendAttachmentState& state);
	bool initDevice(bool enableLayers);
	void initPipelineCreateInfo(
	  VkGraphicsPipelineCreateInfo& createInfo,
	  VkPipelineInputAssemblyStateCreateInfo& ia,
	  VkPipelineRasterizationStateCreateInfo& rs,
	  VkPipelineMultisampleStateCreateInfo& ms,
	  VkPipelineViewportStateCreateInfo& vp,
	  VkPipelineColorBlendStateCreateInfo& cb,
	  VkPipelineDepthStencilStateCreateInfo& ds,
	  VkPipelineVertexInputStateCreateInfo& vertexInput,
	  VkPipelineShaderStageCreateInfo& vertex,
	  VkPipelineShaderStageCreateInfo& fragment);
	bool getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlagBits properties, uint32_t& outTypeIndex);

	void initGlobalState();
	void deinitGlobalState();
	void submitPostPresentBarrier(uint32_t swapchain);
	void prePresentBarrier(uint32_t swapchain, VkCommandBuffer& cmdBuffer);
	void initSurface();
	void deinitDisplay();
	VkCommandBuffer createCommandBuffer();
	char* displayPlatform;
	bool ready = false;
	bool initialised = false;
};
struct DepthBuffer
{
	VkImage image;
	VkDeviceMemory memory;
	VkImageView view;
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

		shaderStages[GraphicsPipelineCreate::Vertex].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[GraphicsPipelineCreate::Vertex].stage = VK_SHADER_STAGE_VERTEX_BIT;

		shaderStages[GraphicsPipelineCreate::Fragment].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[GraphicsPipelineCreate::Fragment].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

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

struct App
{
	VkFence fence;
	VkSemaphore semaphore;

	VkRenderPass renderPass;

	DepthBuffer depthBuffers[8];

	VkCommandBuffer cmdBuffer[8];
	VkFramebuffer framebuffer[8];

	VkPipelineLayout emptyPipelayout;

	VkPipeline opaquePipeline;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

};
struct Vertex
{
	float x, y, z, w;
};
static void writeVertexBuffer(Context& context)
{
	Vertex* ptr = 0;

	vk::MapMemory(context.device, context.app->vertexBufferMemory, 0, 4096, 0, (void**)&ptr);

	// triangle
	ptr->x = -.4; ptr->y = .4; ptr->z = 0; ptr->w = 1;
	ptr++;

	ptr->x = .4; ptr->y = .4; ptr->z = 0; ptr->w = 1;
	ptr++;

	ptr->x = 0; ptr->y = -.4; ptr->z = 0; ptr->w = 1;
	ptr++;
	vk::UnmapMemory(context.device, context.app->vertexBufferMemory);
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

static void createPipeline(Context& context)
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
	vkSuccessOrDie(vk::CreatePipelineLayout(context.device, &sPipelineLayoutCreateInfo, NULL, &context.app->emptyPipelayout),
	               "Failed to create pipeline layout");

	static VkSampleMask sampleMask = 0xffffffff;
	pipeCreate.ms.pSampleMask = &sampleMask;
	context.initColorBlendAttachmentState(attachments[0]);
	SetupVertexAttribs(bindings, attributes, pipeCreate.vi);

	VkRect2D scissors[1];
	VkViewport viewports[1];

	scissors[0].offset.x = 0;
	scissors[0].offset.y = 0;
	scissors[0].extent = context.displayHandle.displayExtent;

	pipeCreate.vp.pScissors = scissors;

	viewports[0].minDepth = 0.0f;
	viewports[0].maxDepth = 1.0f;
	viewports[0].x = 0;
	viewports[0].y = 0;
	viewports[0].width = context.displayHandle.displayExtent.width;
	viewports[0].height = context.displayHandle.displayExtent.height;

	pipeCreate.vp.pViewports = viewports;
	pipeCreate.vp.viewportCount = 1;
	pipeCreate.vp.scissorCount = 1;
	//These are only required to create the graphics pipeline, so we create and destroy them locally
	VkShaderModule vertexShaderModule;
	VkShaderModule fragmentShaderModule;

	if (!createVertShaderModule(context.device, vertexShaderModule))
	{
		LOGE("Failed to create the vertex shader");
		exit(0);
	}

	if (!createFragShaderModule(context.device, fragmentShaderModule))
	{
		LOGE("Failed to create the fragment shader");
		exit(0);
	}

	pipeCreate.ds.depthTestEnable = VK_FALSE;
	pipeCreate.vkPipeInfo.layout = context.app->emptyPipelayout;
	pipeCreate.vkPipeInfo.renderPass = context.app->renderPass;
	pipeCreate.vkPipeInfo.subpass = 0;

	pipeCreate.shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	pipeCreate.shaderStages[0].module = vertexShaderModule;
	pipeCreate.shaderStages[0].pName = "main";
	pipeCreate.shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	pipeCreate.shaderStages[1].module = fragmentShaderModule;
	pipeCreate.shaderStages[1].pName = "main";
	attachments[0].blendEnable = VK_FALSE;
	vkSuccessOrDie(vk::CreateGraphicsPipelines(context.device, NULL, 1, &pipeCreate.vkPipeInfo, NULL, &context.app->opaquePipeline),
	               "Failed to create the graphicsPipeline");
	vk::DestroyShaderModule(context.device, vertexShaderModule, NULL);
	vk::DestroyShaderModule(context.device, fragmentShaderModule, NULL);
}


static void createBuffers(Context& context)
{
	VkBufferCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.pNext = NULL;
	createInfo.size = 4096;
	createInfo.flags = 0;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vk::CreateBuffer(context.device, &createInfo, NULL, &context.app->vertexBuffer);
	context.app->vertexBufferMemory = context.allocateBufferDeviceMemory(context.app->vertexBuffer);

}

static void recordCommandBuffer(Context& context, uint32_t bufferIndex)
{
	uint32_t dynamicOffset = 0;
	context.app->cmdBuffer[bufferIndex] = context.createCommandBuffer();
	VkCommandBuffer cmd = context.app->cmdBuffer[bufferIndex];

	VkCommandBufferBeginInfo cmdBufferBeginInfo;

	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.pNext = NULL;
	cmdBufferBeginInfo.flags = 0;
	cmdBufferBeginInfo.pInheritanceInfo = NULL;

	vk::BeginCommandBuffer(cmd, &cmdBufferBeginInfo);

	VkRenderPassBeginInfo renderPassBeginInfo;
	VkClearValue clearVals[2];
	clearVals[0].color.float32[0] = 0.6f;
	clearVals[0].color.float32[1] = 0.8f;
	clearVals[0].color.float32[2] = 1.f;
	clearVals[0].color.float32[3] = 1.0f;
	clearVals[1].depthStencil.depth = 1.0f;
	clearVals[1].depthStencil.stencil = 0xFF;

	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = NULL;
	renderPassBeginInfo.renderPass = context.app->renderPass;
	renderPassBeginInfo.framebuffer = context.app->framebuffer[bufferIndex];
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent = context.displayHandle.displayExtent;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = &clearVals[0];

	vk::CmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vk::CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, context.app->opaquePipeline);
	VkDeviceSize vertexOffset = 0;
	vk::CmdBindVertexBuffers(cmd, 0, 1, &context.app->vertexBuffer, &vertexOffset);
	vk::CmdDraw(cmd, 3, 1, 0, 0);
	vk::CmdEndRenderPass(cmd);
	context.prePresentBarrier(bufferIndex, cmd);
	vk::EndCommandBuffer(cmd);
}

static void recordCommandBuffer(Context& context)
{
	for (uint32_t i = 0; i < context.displayHandle.swapChainLength; i++)
	{
		recordCommandBuffer(context, i);
	}
}

static void initOnScreenFbo(Context& context)
{
	VkRenderPassCreateInfo renderPassCreateInfo;
	VkAttachmentDescription attachmentDescriptions[2];
	VkSubpassDescription subpassDescription = {};

	attachmentDescriptions[0].flags = 0;
	attachmentDescriptions[0].format = context.displayHandle.fb.colorFormat;
	attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	attachmentDescriptions[1].flags = 0;
	attachmentDescriptions[1].format = context.displayHandle.fb.depthStencilFormat;
	attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
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

	vkSuccessOrDie(vk::CreateRenderPass(context.device, &renderPassCreateInfo, NULL, &context.app->renderPass),
	               "Failed to create the renderpass");

	for (uint32_t i = 0; i < context.displayHandle.swapChainLength; i++)
	{
		VkFramebufferCreateInfo fbCreateInfo;
		VkImageView attachments[2];

		attachments[0] = context.displayHandle.fb.colorImageViews[i];
		attachments[1] = context.displayHandle.fb.depthStencilImageView[i];

		fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbCreateInfo.pNext = NULL;
		fbCreateInfo.flags = 0;
		fbCreateInfo.renderPass = context.app->renderPass;
		fbCreateInfo.layers = 1;
		fbCreateInfo.attachmentCount = 2;
		fbCreateInfo.pAttachments = &attachments[0];

		fbCreateInfo.width = context.displayHandle.displayExtent.width;
		fbCreateInfo.height = context.displayHandle.displayExtent.height;

		vkSuccessOrDie(vk::CreateFramebuffer(context.device, &fbCreateInfo, NULL, &context.app->framebuffer[i]),
		               "Failed to create the framebuffer");
	}
}

void prepare(Context& context)
{
	context.app = new App();
	context.initGlobalState();
	initOnScreenFbo(context);

	createPipeline(context);

	createBuffers(context);

	writeVertexBuffer(context);

	recordCommandBuffer(context);

	VkFenceCreateInfo fenceCreateInfo;
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.pNext = NULL;
	fenceCreateInfo.flags = 0;
	vk::CreateFence(context.device, &fenceCreateInfo, NULL, &context.app->fence);

	VkSemaphoreCreateInfo semaphoreCreateInfo;
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = NULL;
	semaphoreCreateInfo.flags = 0;
	vk::CreateSemaphore(context.device, &semaphoreCreateInfo, NULL, &context.app->semaphore);
}

void deinit(Context& context)
{
	vk::DestroySemaphore(context.device, context.app->semaphore, NULL);
	vk::DestroyFence(context.device, context.app->fence, NULL);
	vk::DestroyRenderPass(context.device, context.app->renderPass, NULL);
	vk::DestroyPipelineLayout(context.device, context.app->emptyPipelayout, NULL);
	vk::DestroyPipeline(context.device, context.app->opaquePipeline, NULL);
	vk::DestroyBuffer(context.device, context.app->vertexBuffer, NULL);
	vk::FreeMemory(context.device, context.app->vertexBufferMemory, NULL);
	context.deinitDisplay();
	context.deinitGlobalState();
	delete context.app;
}

void drawFrame(Context& context)
{
	VkResult presentResult;

	uint32_t nextIndex = 0;

	vk::AcquireNextImageKHR(context.device, context.displayHandle.swapChain, uint64_t(-1),
	                        context.app->semaphore, VK_NULL_HANDLE, &nextIndex);

	VkSubmitInfo sSubmitInfo = {};

	sSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	sSubmitInfo.waitSemaphoreCount = 1;
	sSubmitInfo.pWaitSemaphores = &context.app->semaphore;
	sSubmitInfo.commandBufferCount = 1;
	sSubmitInfo.pCommandBuffers = &context.app->cmdBuffer[nextIndex];

	vk::QueueSubmit(context.graphicsQueue, 1, &sSubmitInfo, VK_NULL_HANDLE);

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pSwapchains = &context.displayHandle.swapChain;
	presentInfo.pImageIndices = &nextIndex;
	presentInfo.swapchainCount = 1;
	presentInfo.pResults = &presentResult;

	vkSuccessOrDie(vk::QueuePresentKHR(context.graphicsQueue, &presentInfo), "Failed to present");

	context.submitPostPresentBarrier(nextIndex);
	vk::QueueWaitIdle(context.graphicsQueue);
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


void Context::initVkInstanceAndPhysicalDevice(bool enableLayers)
{
	vk::initVulkan();
	VkApplicationInfo appInfo = {};
	VkInstanceCreateInfo instanceCreateInfo = {};

	uint32_t gpuCount = 100;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = NULL;
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 2);

	appInfo.applicationVersion = 1;
	appInfo.engineVersion = 0;
	appInfo.pApplicationName = "MyApp";
	appInfo.pEngineName = "PVRApi";
#pragma warning TODO_APPLICATION_NAME
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = NULL;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	const char* instanceValidationLayers[] =
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

	instanceCreateInfo.enabledExtensionCount = 0;

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
		instanceCreateInfo.enabledLayerCount = enabledLayers.size();
	}
	// create the vk instance
	vkSuccessOrDie(vk::CreateInstance(&instanceCreateInfo, NULL, &instance), "Failed to create instance");
	vk::initVulkanInstance(instance);
	vkSuccessOrDie(vk::EnumeratePhysicalDevices(instance, &gpuCount, NULL), "");
	LOGI("Number of Vulkan Physical devices: [%d]", gpuCount);
	gpuCount = 1;
	vkSuccessOrDie(vk::EnumeratePhysicalDevices(instance, &gpuCount, &gpu), "");
}

inline void editPhysicalDeviceFeatures(VkPhysicalDeviceFeatures& features)
{
	features.robustBufferAccess = false;
}

inline  bool Context::initDevice(bool enableLayers)
{
	VkPhysicalDeviceFeatures physicalFeatures = {};
	vk::GetPhysicalDeviceFeatures(gpu, &physicalFeatures);
	editPhysicalDeviceFeatures(physicalFeatures);
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	VkDeviceCreateInfo deviceCreateInfo = {};
	// create the queue
	float priority = 1.f;
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.pNext = NULL;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.queueFamilyIndex = graphicsQueueIndex;
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
		deviceCreateInfo.enabledLayerCount = enabledLayers.size();
	}


	vkSuccessOrDie(vk::CreateDevice(gpu, &deviceCreateInfo, NULL, &device), "Vulkan Device Creation");
	vk::initVulkanDevice(device);
	// Gather physical device memory properties
	vk::GetPhysicalDeviceMemoryProperties(gpu, &deviceMemProps);
	vk::GetDeviceQueue(device, 0, 0, &graphicsQueue);
	return true;
}

void Context::initSwapChain()
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkSuccessOrDie(vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, displayHandle.surface, &surfaceCapabilities),
	               "Failed to get the surface capabilities");

	LOGI("Surface Capabilities:\n");
	LOGI("Image count: %u - %u\n", surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
	LOGI("Array size: %u\n", surfaceCapabilities.maxImageArrayLayers);
	LOGI("Image size (now): %dx%d\n", surfaceCapabilities.currentExtent.width, surfaceCapabilities.currentExtent.height);
	LOGI("Image size (extent): %dx%d - %dx%d\n", surfaceCapabilities.minImageExtent.width, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.width,
	     surfaceCapabilities.maxImageExtent.height);
	LOGI("Usage: %x\n", surfaceCapabilities.supportedUsageFlags);
	LOGI("Current transform: %u\n", surfaceCapabilities.currentTransform);
	//LOGI("Allowed transforms: %x\n", surfaceCapabilities.supportedTransforms);

	uint32_t formatCount = 0;
	vk::GetPhysicalDeviceSurfaceFormatsKHR(gpu, displayHandle.surface, &formatCount, NULL);
	VkSurfaceFormatKHR tmpformats[16]; std::vector<VkSurfaceFormatKHR> tmpFormatsVector;
	VkSurfaceFormatKHR* allFormats = tmpformats;
	if (formatCount > 16)
	{
		tmpFormatsVector.resize(formatCount);
		allFormats = tmpFormatsVector.data();
	}
	vk::GetPhysicalDeviceSurfaceFormatsKHR(gpu, displayHandle.surface, &formatCount, allFormats);

	VkSurfaceFormatKHR format = allFormats[0];

	VkFormat preferredFormats[] =
	{
		VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_B8G8R8_SNORM, VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_R5G6B5_UNORM_PACK16
	};

	for (uint32_t f = 0; f < formatCount; ++f)
	{
		if (allFormats[f].format == preferredFormats[0] || allFormats[f].format == preferredFormats[1])
		{
			format = allFormats[f]; break;
		}
	}

	uint32_t numPresentMode;
	vkSuccessOrDie(vk::GetPhysicalDeviceSurfacePresentModesKHR(gpu, displayHandle.surface, &numPresentMode, NULL),
	               "Failed to get the number of present modes count");

	assert(numPresentMode > 0);
	std::vector<VkPresentModeKHR> presentModes(numPresentMode);
	vkSuccessOrDie(vk::GetPhysicalDeviceSurfacePresentModesKHR(gpu, displayHandle.surface, &numPresentMode, &presentModes[0]),
	               "failed to get the present modes");

	// Try to use mailbox mode, Low latency and non-tearing
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (size_t i = 0; i < numPresentMode; i++)
	{
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
		{
			swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	displayHandle.fb.colorFormat = format.format;
	displayHandle.displayExtent = surfaceCapabilities.currentExtent;

	//--- create the swap chain
	VkSwapchainCreateInfoKHR swapchainCreate;
	swapchainCreate.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreate.pNext = NULL;
	swapchainCreate.flags = 0;
	swapchainCreate.clipped = VK_TRUE;
	swapchainCreate.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreate.surface = displayHandle.surface;
	swapchainCreate.minImageCount = std::max(surfaceCapabilities.minImageCount + 1, std::min(surfaceCapabilities.maxImageCount, 3u));
	swapchainCreate.imageFormat = displayHandle.fb.colorFormat;
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
	vkSuccessOrDie(vk::CreateSwapchainKHR(device, &swapchainCreate,
	                                      NULL, &displayHandle.swapChain), "Could not create the swap chain");
	// get the number of swapchains
	vkSuccessOrDie(vk::GetSwapchainImagesKHR(device, displayHandle.swapChain,
	               &displayHandle.swapChainLength, NULL), "Could not get swapchain length");

	displayHandle.fb.colorImages.resize(displayHandle.swapChainLength);
	displayHandle.fb.colorImageViews.resize(displayHandle.swapChainLength);
	vkSuccessOrDie(vk::GetSwapchainImagesKHR(device, displayHandle.swapChain,
	               &displayHandle.swapChainLength, &displayHandle.fb.colorImages[0]), "Could not get swapchain images");

	//--- create the swapchain view
	VkImageViewCreateInfo viewCreateInfo;
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.pNext = NULL;
	viewCreateInfo.flags = 0;
	viewCreateInfo.image = VK_NULL_HANDLE;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = displayHandle.fb.colorFormat;
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewCreateInfo.subresourceRange.baseMipLevel = 0;
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;
	viewCreateInfo.subresourceRange.layerCount = 1;
	displayHandle.fb.depthStencilImage.resize(displayHandle.swapChainLength);
	displayHandle.fb.depthStencilImageView.resize(displayHandle.swapChainLength);
	displayHandle.fb.depthStencilFormat = VK_FORMAT_D32_SFLOAT;

	for (uint32_t i = 0; i < displayHandle.swapChainLength; ++i)
	{
		viewCreateInfo.image = displayHandle.fb.colorImages[i];
		vkSuccessOrDie(vk::CreateImageView(device, &viewCreateInfo, NULL,
		                                   &displayHandle.fb.colorImageViews[i]), "create display image view");

		// create the depth stencil image
		VkImageCreateInfo dsCreateInfo = {};
		dsCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		dsCreateInfo.pNext = NULL;
		dsCreateInfo.flags = 0;
		dsCreateInfo.format = displayHandle.fb.depthStencilFormat;
		dsCreateInfo.extent.width = displayHandle.displayExtent.width;
		dsCreateInfo.extent.height = displayHandle.displayExtent.height;
		dsCreateInfo.extent.depth = 1;
		dsCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		dsCreateInfo.arrayLayers = 1;
		dsCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		dsCreateInfo.mipLevels = 1;
		dsCreateInfo.flags = 0;
		dsCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		dsCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		dsCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		dsCreateInfo.pQueueFamilyIndices = 0;
		dsCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		dsCreateInfo.queueFamilyIndexCount = 0;
		dsCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VkDeviceMemory depthMemory;
		VkResult rslt = vk::CreateImage(device, &dsCreateInfo, NULL, &displayHandle.fb.depthStencilImage[i].first);
		vkSuccessOrDie(rslt, "Image creation failed");

		displayHandle.fb.depthStencilImage[i].second = allocateImageDeviceMemory(displayHandle.fb.depthStencilImage[i].first, NULL);
		if (displayHandle.fb.depthStencilImage[i].second == VK_NULL_HANDLE)
		{
			LOGE("Memory allocation failed");
			exit(0);
		}
		// create the depth stencil view
		VkImageViewCreateInfo dsViewCreateInfo;
		dsViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		dsViewCreateInfo.pNext = NULL;
		dsViewCreateInfo.image = displayHandle.fb.depthStencilImage[i].first;
		dsViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		dsViewCreateInfo.format = displayHandle.fb.depthStencilFormat;
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
		vkSuccessOrDie(vk::CreateImageView(device, &dsViewCreateInfo, NULL, &displayHandle.fb.depthStencilImageView[i]),
		               "Create Depth stencil image view");
	}
}

void Context::initGlobalState()
{
	initVkInstanceAndPhysicalDevice(true);
	initSurface();
	initDevice(true);
	initSwapChain();
	VkCommandPoolCreateInfo cmdPoolCreateInfo;
	cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolCreateInfo.pNext = NULL;
	cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cmdPoolCreateInfo.queueFamilyIndex = 0;
	vk::CreateCommandPool(device, &cmdPoolCreateInfo, NULL, &cmdPool);
	postPresentCmdBuffer = createCommandBuffer();
}

bool Context::getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlagBits properties,
                                 uint32_t& outTypeIndex)
{
	for (uint32_t i = 0; i < 32; ++i)
	{
		if ((typeBits & 1) == 1)
		{
			if ((deviceMemProps.memoryTypes[i].propertyFlags & properties) == properties)
			{
				outTypeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	return false;
}

void Context::deinitGlobalState()
{
	vk::FreeCommandBuffers(device, cmdPool, 1, &postPresentCmdBuffer);
	vk::DestroyCommandPool(device, cmdPool, NULL);
	vk::DestroyDevice(device, NULL);
	vk::DestroyInstance(instance, NULL);
}

VkDeviceMemory Context::allocateImageDeviceMemory(VkImage image, VkMemoryRequirements* memoryRequirementsOut)
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

	vk::GetImageMemoryRequirements(device, image, pMemoryRequirements);

	sMemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	sMemoryAllocInfo.pNext = NULL;
	sMemoryAllocInfo.allocationSize = pMemoryRequirements->size;

	//Find the first allowed type:
	if (pMemoryRequirements->memoryTypeBits == 0)
	{
		LOGE("unsupported memory type bits");
		exit(0);
	}

	getMemoryTypeIndex(pMemoryRequirements->memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sMemoryAllocInfo.memoryTypeIndex);
	vk::AllocateMemory(device, &sMemoryAllocInfo, NULL, &memory);
	vk::BindImageMemory(device, image, memory, 0);

	return memory;
}

VkDeviceMemory Context::allocateBufferDeviceMemory(VkBuffer buffer, VkMemoryRequirements* memoryRequirementsOut)
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

	vk::GetBufferMemoryRequirements(device, buffer, pMemoryRequirements);

	sMemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	sMemoryAllocInfo.pNext = NULL;
	sMemoryAllocInfo.allocationSize = pMemoryRequirements->size;

	//Find the first allowed type:
	if (pMemoryRequirements->memoryTypeBits == 0)
	{
		LOGE("Invalid memory type bits");
		exit(0);
	}
	getMemoryTypeIndex(pMemoryRequirements->memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sMemoryAllocInfo.memoryTypeIndex);
	vk::AllocateMemory(device, &sMemoryAllocInfo, NULL, &memory);
	vk::BindBufferMemory(device, buffer, memory, 0);

	return memory;
}

void Context::initColorBlendAttachmentState(VkPipelineColorBlendAttachmentState& state)
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

void Context::initSurface()
{
	PFN_vkCreateAndroidSurfaceKHR fn_vkCreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR)vk::GetInstanceProcAddr(instance, "vkCreateAndroidSurfaceKHR");
	VkAndroidSurfaceCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.flags = 0;
	createInfo.window = displayHandle.nativeDisplay;

	if (!fn_vkCreateAndroidSurfaceKHR)
	{
		LOGI("Could not get address of 'vkCreateAndroidSurfaceKHR'!");
	}
	else
	{
		LOGI("COULD FIND IT");
	}
	fn_vkCreateAndroidSurfaceKHR(instance, &createInfo, NULL, &displayHandle.surface);
}


void Context::deinitDisplay()
{
	PFN_vkDestroySwapchainKHR fn_vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR)vk::GetDeviceProcAddr(device, "vkDestroySwapchainKHR");
	// release the display
	for (uint32_t i = 0; i < displayHandle.swapChainLength; ++i)
	{
		vk::DestroyImageView(device, displayHandle.fb.colorImageViews[i], NULL);
		vk::DestroyImageView(device, displayHandle.fb.depthStencilImageView[i], NULL);
		vk::DestroyImage(device, displayHandle.fb.depthStencilImage[i].first, NULL);
		vk::FreeMemory(device, displayHandle.fb.depthStencilImage[i].second, NULL);

		vk::DestroyFramebuffer(device, app->framebuffer[i], NULL);
	}
	fn_vkDestroySwapchainKHR(device, displayHandle.swapChain, NULL);
	vk::DestroySurfaceKHR(instance, displayHandle.surface, NULL);
}

void Context::submitPostPresentBarrier(uint32_t swapchain)
{
	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkResult vkRes = vk::BeginCommandBuffer(postPresentCmdBuffer, &cmdBufInfo);
	assert(!vkRes);

	VkImageMemoryBarrier postPresentBarrier = {};
	postPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	postPresentBarrier.srcAccessMask = 0;
	postPresentBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	postPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	postPresentBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	postPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	postPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	
	postPresentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	postPresentBarrier.subresourceRange.baseArrayLayer = 0;
	postPresentBarrier.subresourceRange.baseMipLevel = 0;
	postPresentBarrier.subresourceRange.layerCount = 1;
	postPresentBarrier.subresourceRange.levelCount = 1;

	postPresentBarrier.image = displayHandle.fb.colorImages[swapchain];

	vk::CmdPipelineBarrier(
	  postPresentCmdBuffer,
	  VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
	  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
	  0,
	  0, NULL, // No memory barriers,
	  0, NULL, // No buffer barriers,
	  1, &postPresentBarrier);

	vkRes = vk::EndCommandBuffer(postPresentCmdBuffer);
	assert(!vkRes);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &postPresentCmdBuffer;

	vkRes = vk::QueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	assert(!vkRes);

	vkRes = vk::QueueWaitIdle(graphicsQueue);
	assert(!vkRes);
}

VkCommandBuffer Context::createCommandBuffer()
{
	VkCommandBufferAllocateInfo cmdInfo = {};
	cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdInfo.commandPool = cmdPool;
	cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdInfo.commandBufferCount = 1;
	VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
	vk::AllocateCommandBuffers(device, &cmdInfo, &cmdBuffer);
	return cmdBuffer;
}

void Context::prePresentBarrier(uint32_t swapchain, VkCommandBuffer& cmdBuffer)
{
	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = NULL;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = 0;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	imageMemoryBarrier.image = displayHandle.fb.colorImages[swapchain];
	vk::CmdPipelineBarrier(cmdBuffer,	VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
	                       0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);
}

extern "C" {

	static void initAndroidDisplay(struct android_app* app, Context& context)
	{
		context.displayHandle.nativeDisplay = app->window;
	}

	static int32_t processInput(struct android_app* app, AInputEvent* event)
	{
		return 0;
	}
	static void processCommand(struct android_app* app, int32_t cmd)
	{
		Context* context = (Context*)app->userData;
		switch (cmd)
		{
		case APP_CMD_INIT_WINDOW:
		{
			// The window is being shown, get it ready.
			if (app->window != NULL)
			{
				usleep(100000);
				initAndroidDisplay(app, *context);
				prepare(*context);
				context->ready = true;
				context->initialised = true;
			}
			break;
		}
		case APP_CMD_TERM_WINDOW:
		{
			// The window is being hidden or closed, clean it up.
			break;
		}
		case APP_CMD_GAINED_FOCUS:
		{
			LOGI("Waking up");
			context->ready = true;
			break;
		}
		case APP_CMD_LOST_FOCUS:
		{
			LOGI("Going to sleepy times");
			context->ready = false;
			break;
		}
		}
	}
	static void processTerminate()
	{
	}

	void android_main(struct android_app* state)
	{
		Context context;

		// Make sure glue isn't stripped.
		app_dummy();

		state->userData = &context;
		state->onAppCmd = processCommand;
		state->onInputEvent = processInput;
		LOGI("Hello android");
		while (1)
		{
			// Read all pending events.
			int ident;
			int events;
			struct android_poll_source* source;


			while ((ident = ALooper_pollAll(context.ready ? 0 : -1, NULL, &events,
			                                (void**)&source)) >= 0)
			{

				// Process this event.
				if (source != NULL)
				{
					source->process(state, source);
				}

				// Check if we are exiting.
				if (state->destroyRequested != 0)
				{
					processTerminate();
					return;
				}
			}

			if (context.ready && context.initialised)
			{
				//Draw stuff
				drawFrame(context);
			}
		}
	}
}
