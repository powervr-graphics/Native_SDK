/*!*********************************************************************************************************************
\file         PVRApi\Vulkan\VulkanBindings.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        This file contains Vulkan bindings with function pointers. The PowerVR Framework uses them to allow unified
			  access to Vulkancontext throught the same functions.
			  Function pointer loading is done with the vk:initVk function which is normally called by the PVR Shell.
***********************************************************************************************************************/
#pragma once
#include "PVRNativeApi/Vulkan/HeadersVk.h"
#include "PVRCore/NativeLibrary.h"

#define PVR_VULKAN_FUNCTION_POINTER_DECLARATION(function_name) static PFN_vk##function_name function_name;


/*!*********************************************************************************************************************
\brief This class contains function pointers to all Vulkan functions. These function pointers will be populated
on the initVk call. Use normally, using the vk class as a namespace. For example vk::CmdBindPipeline(...);
***********************************************************************************************************************/
class vk
{
public:
	//Call this function once before using the vk namespaced functions. Do not call during static initialisation phase as it uses static global components.
	/*!*******************************************************************************************************************
	\brief Call once per application run to populate the function pointers. PVRShell calls this on context creation.
	*********************************************************************************************************************/
	static void initVk(VkInstance instance, VkDevice device);

	/*!*******************************************************************************************************************
	\brief Call once per application run to release the OpenGL library. PVRShell calls this on exit.
	*********************************************************************************************************************/
	static void releaseVk() {}
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetInstanceProcAddr);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetDeviceProcAddr);

	//PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateInstance);
	//PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyInstance);
	//PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EnumeratePhysicalDevices);
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