// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See vk_bindings_helper_generator.py for modifications

/*
\brief Helper functions for filling vk bindings function pointer tables.
\file vk_bindings_helper.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include <string>
#include <cstring>
#include "pvr_openlib.h"
#include "vk_bindings.h"

namespace vk {
namespace internal {
/** DEFINE THE PLATFORM SPECIFIC LIBRARY NAME **/
#ifdef _WIN32
static const char* libName = "vulkan-1.dll";
#elif defined(TARGET_OS_MAC)
static const char* libName = "libvulkan.dylib";
#else
static const char* libName = "libvulkan.so.1;libvulkan.so";
#endif
}
}

static inline bool initVkBindings(VkBindings *bindings)
{
	pvr::lib::LIBTYPE lib = pvr::lib::openlib(vk::internal::libName);

    memset(bindings, 0, sizeof(*bindings));

    // Load the function pointer dynamically for vkGetInstanceProcAddr
	bindings->vkGetInstanceProcAddr = pvr::lib::getLibFunctionChecked<PFN_vkGetInstanceProcAddr>(lib, "vkGetInstanceProcAddr");

    // Use vkGetInstanceProcAddr with a NULL instance to retrieve the function pointers
	bindings->vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)bindings->vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceExtensionProperties");
	bindings->vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)bindings->vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceLayerProperties");
	bindings->vkCreateInstance = (PFN_vkCreateInstance)bindings->vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    bindings->vkEnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion)bindings->vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceVersion");

    // validate that we have all necessary function pointers to continue
	if (!bindings->vkEnumerateInstanceExtensionProperties || !bindings->vkEnumerateInstanceLayerProperties || !bindings->vkCreateInstance)
	{
		return false;
	}
    return true;
}


static inline void initVkDeviceBindings(VkDevice device, VkDeviceBindings *bindings, PFN_vkGetDeviceProcAddr getDeviceProcAddress) {
    memset(bindings, 0, sizeof(*bindings));
    // Device function pointers
    bindings->vkGetDeviceProcAddr = getDeviceProcAddress;
    bindings->vkDestroyDevice = (PFN_vkDestroyDevice) getDeviceProcAddress(device, "vkDestroyDevice");
    bindings->vkGetDeviceQueue = (PFN_vkGetDeviceQueue) getDeviceProcAddress(device, "vkGetDeviceQueue");
    bindings->vkQueueSubmit = (PFN_vkQueueSubmit) getDeviceProcAddress(device, "vkQueueSubmit");
    bindings->vkQueueWaitIdle = (PFN_vkQueueWaitIdle) getDeviceProcAddress(device, "vkQueueWaitIdle");
    bindings->vkDeviceWaitIdle = (PFN_vkDeviceWaitIdle) getDeviceProcAddress(device, "vkDeviceWaitIdle");
    bindings->vkAllocateMemory = (PFN_vkAllocateMemory) getDeviceProcAddress(device, "vkAllocateMemory");
    bindings->vkFreeMemory = (PFN_vkFreeMemory) getDeviceProcAddress(device, "vkFreeMemory");
    bindings->vkMapMemory = (PFN_vkMapMemory) getDeviceProcAddress(device, "vkMapMemory");
    bindings->vkUnmapMemory = (PFN_vkUnmapMemory) getDeviceProcAddress(device, "vkUnmapMemory");
    bindings->vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges) getDeviceProcAddress(device, "vkFlushMappedMemoryRanges");
    bindings->vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges) getDeviceProcAddress(device, "vkInvalidateMappedMemoryRanges");
    bindings->vkGetDeviceMemoryCommitment = (PFN_vkGetDeviceMemoryCommitment) getDeviceProcAddress(device, "vkGetDeviceMemoryCommitment");
    bindings->vkBindBufferMemory = (PFN_vkBindBufferMemory) getDeviceProcAddress(device, "vkBindBufferMemory");
    bindings->vkBindImageMemory = (PFN_vkBindImageMemory) getDeviceProcAddress(device, "vkBindImageMemory");
    bindings->vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements) getDeviceProcAddress(device, "vkGetBufferMemoryRequirements");
    bindings->vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements) getDeviceProcAddress(device, "vkGetImageMemoryRequirements");
    bindings->vkGetImageSparseMemoryRequirements = (PFN_vkGetImageSparseMemoryRequirements) getDeviceProcAddress(device, "vkGetImageSparseMemoryRequirements");
    bindings->vkQueueBindSparse = (PFN_vkQueueBindSparse) getDeviceProcAddress(device, "vkQueueBindSparse");
    bindings->vkCreateFence = (PFN_vkCreateFence) getDeviceProcAddress(device, "vkCreateFence");
    bindings->vkDestroyFence = (PFN_vkDestroyFence) getDeviceProcAddress(device, "vkDestroyFence");
    bindings->vkResetFences = (PFN_vkResetFences) getDeviceProcAddress(device, "vkResetFences");
    bindings->vkGetFenceStatus = (PFN_vkGetFenceStatus) getDeviceProcAddress(device, "vkGetFenceStatus");
    bindings->vkWaitForFences = (PFN_vkWaitForFences) getDeviceProcAddress(device, "vkWaitForFences");
    bindings->vkCreateSemaphore = (PFN_vkCreateSemaphore) getDeviceProcAddress(device, "vkCreateSemaphore");
    bindings->vkDestroySemaphore = (PFN_vkDestroySemaphore) getDeviceProcAddress(device, "vkDestroySemaphore");
    bindings->vkCreateEvent = (PFN_vkCreateEvent) getDeviceProcAddress(device, "vkCreateEvent");
    bindings->vkDestroyEvent = (PFN_vkDestroyEvent) getDeviceProcAddress(device, "vkDestroyEvent");
    bindings->vkGetEventStatus = (PFN_vkGetEventStatus) getDeviceProcAddress(device, "vkGetEventStatus");
    bindings->vkSetEvent = (PFN_vkSetEvent) getDeviceProcAddress(device, "vkSetEvent");
    bindings->vkResetEvent = (PFN_vkResetEvent) getDeviceProcAddress(device, "vkResetEvent");
    bindings->vkCreateQueryPool = (PFN_vkCreateQueryPool) getDeviceProcAddress(device, "vkCreateQueryPool");
    bindings->vkDestroyQueryPool = (PFN_vkDestroyQueryPool) getDeviceProcAddress(device, "vkDestroyQueryPool");
    bindings->vkGetQueryPoolResults = (PFN_vkGetQueryPoolResults) getDeviceProcAddress(device, "vkGetQueryPoolResults");
    bindings->vkCreateBuffer = (PFN_vkCreateBuffer) getDeviceProcAddress(device, "vkCreateBuffer");
    bindings->vkDestroyBuffer = (PFN_vkDestroyBuffer) getDeviceProcAddress(device, "vkDestroyBuffer");
    bindings->vkCreateBufferView = (PFN_vkCreateBufferView) getDeviceProcAddress(device, "vkCreateBufferView");
    bindings->vkDestroyBufferView = (PFN_vkDestroyBufferView) getDeviceProcAddress(device, "vkDestroyBufferView");
    bindings->vkCreateImage = (PFN_vkCreateImage) getDeviceProcAddress(device, "vkCreateImage");
    bindings->vkDestroyImage = (PFN_vkDestroyImage) getDeviceProcAddress(device, "vkDestroyImage");
    bindings->vkGetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout) getDeviceProcAddress(device, "vkGetImageSubresourceLayout");
    bindings->vkCreateImageView = (PFN_vkCreateImageView) getDeviceProcAddress(device, "vkCreateImageView");
    bindings->vkDestroyImageView = (PFN_vkDestroyImageView) getDeviceProcAddress(device, "vkDestroyImageView");
    bindings->vkCreateShaderModule = (PFN_vkCreateShaderModule) getDeviceProcAddress(device, "vkCreateShaderModule");
    bindings->vkDestroyShaderModule = (PFN_vkDestroyShaderModule) getDeviceProcAddress(device, "vkDestroyShaderModule");
    bindings->vkCreatePipelineCache = (PFN_vkCreatePipelineCache) getDeviceProcAddress(device, "vkCreatePipelineCache");
    bindings->vkDestroyPipelineCache = (PFN_vkDestroyPipelineCache) getDeviceProcAddress(device, "vkDestroyPipelineCache");
    bindings->vkGetPipelineCacheData = (PFN_vkGetPipelineCacheData) getDeviceProcAddress(device, "vkGetPipelineCacheData");
    bindings->vkMergePipelineCaches = (PFN_vkMergePipelineCaches) getDeviceProcAddress(device, "vkMergePipelineCaches");
    bindings->vkCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines) getDeviceProcAddress(device, "vkCreateGraphicsPipelines");
    bindings->vkCreateComputePipelines = (PFN_vkCreateComputePipelines) getDeviceProcAddress(device, "vkCreateComputePipelines");
    bindings->vkDestroyPipeline = (PFN_vkDestroyPipeline) getDeviceProcAddress(device, "vkDestroyPipeline");
    bindings->vkCreatePipelineLayout = (PFN_vkCreatePipelineLayout) getDeviceProcAddress(device, "vkCreatePipelineLayout");
    bindings->vkDestroyPipelineLayout = (PFN_vkDestroyPipelineLayout) getDeviceProcAddress(device, "vkDestroyPipelineLayout");
    bindings->vkCreateSampler = (PFN_vkCreateSampler) getDeviceProcAddress(device, "vkCreateSampler");
    bindings->vkDestroySampler = (PFN_vkDestroySampler) getDeviceProcAddress(device, "vkDestroySampler");
    bindings->vkCreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout) getDeviceProcAddress(device, "vkCreateDescriptorSetLayout");
    bindings->vkDestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout) getDeviceProcAddress(device, "vkDestroyDescriptorSetLayout");
    bindings->vkCreateDescriptorPool = (PFN_vkCreateDescriptorPool) getDeviceProcAddress(device, "vkCreateDescriptorPool");
    bindings->vkDestroyDescriptorPool = (PFN_vkDestroyDescriptorPool) getDeviceProcAddress(device, "vkDestroyDescriptorPool");
    bindings->vkResetDescriptorPool = (PFN_vkResetDescriptorPool) getDeviceProcAddress(device, "vkResetDescriptorPool");
    bindings->vkAllocateDescriptorSets = (PFN_vkAllocateDescriptorSets) getDeviceProcAddress(device, "vkAllocateDescriptorSets");
    bindings->vkFreeDescriptorSets = (PFN_vkFreeDescriptorSets) getDeviceProcAddress(device, "vkFreeDescriptorSets");
    bindings->vkUpdateDescriptorSets = (PFN_vkUpdateDescriptorSets) getDeviceProcAddress(device, "vkUpdateDescriptorSets");
    bindings->vkCreateFramebuffer = (PFN_vkCreateFramebuffer) getDeviceProcAddress(device, "vkCreateFramebuffer");
    bindings->vkDestroyFramebuffer = (PFN_vkDestroyFramebuffer) getDeviceProcAddress(device, "vkDestroyFramebuffer");
    bindings->vkCreateRenderPass = (PFN_vkCreateRenderPass) getDeviceProcAddress(device, "vkCreateRenderPass");
    bindings->vkDestroyRenderPass = (PFN_vkDestroyRenderPass) getDeviceProcAddress(device, "vkDestroyRenderPass");
    bindings->vkGetRenderAreaGranularity = (PFN_vkGetRenderAreaGranularity) getDeviceProcAddress(device, "vkGetRenderAreaGranularity");
    bindings->vkCreateCommandPool = (PFN_vkCreateCommandPool) getDeviceProcAddress(device, "vkCreateCommandPool");
    bindings->vkDestroyCommandPool = (PFN_vkDestroyCommandPool) getDeviceProcAddress(device, "vkDestroyCommandPool");
    bindings->vkResetCommandPool = (PFN_vkResetCommandPool) getDeviceProcAddress(device, "vkResetCommandPool");
    bindings->vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers) getDeviceProcAddress(device, "vkAllocateCommandBuffers");
    bindings->vkFreeCommandBuffers = (PFN_vkFreeCommandBuffers) getDeviceProcAddress(device, "vkFreeCommandBuffers");
    bindings->vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer) getDeviceProcAddress(device, "vkBeginCommandBuffer");
    bindings->vkEndCommandBuffer = (PFN_vkEndCommandBuffer) getDeviceProcAddress(device, "vkEndCommandBuffer");
    bindings->vkResetCommandBuffer = (PFN_vkResetCommandBuffer) getDeviceProcAddress(device, "vkResetCommandBuffer");
    bindings->vkCmdBindPipeline = (PFN_vkCmdBindPipeline) getDeviceProcAddress(device, "vkCmdBindPipeline");
    bindings->vkCmdSetViewport = (PFN_vkCmdSetViewport) getDeviceProcAddress(device, "vkCmdSetViewport");
    bindings->vkCmdSetScissor = (PFN_vkCmdSetScissor) getDeviceProcAddress(device, "vkCmdSetScissor");
    bindings->vkCmdSetLineWidth = (PFN_vkCmdSetLineWidth) getDeviceProcAddress(device, "vkCmdSetLineWidth");
    bindings->vkCmdSetDepthBias = (PFN_vkCmdSetDepthBias) getDeviceProcAddress(device, "vkCmdSetDepthBias");
    bindings->vkCmdSetBlendConstants = (PFN_vkCmdSetBlendConstants) getDeviceProcAddress(device, "vkCmdSetBlendConstants");
    bindings->vkCmdSetDepthBounds = (PFN_vkCmdSetDepthBounds) getDeviceProcAddress(device, "vkCmdSetDepthBounds");
    bindings->vkCmdSetStencilCompareMask = (PFN_vkCmdSetStencilCompareMask) getDeviceProcAddress(device, "vkCmdSetStencilCompareMask");
    bindings->vkCmdSetStencilWriteMask = (PFN_vkCmdSetStencilWriteMask) getDeviceProcAddress(device, "vkCmdSetStencilWriteMask");
    bindings->vkCmdSetStencilReference = (PFN_vkCmdSetStencilReference) getDeviceProcAddress(device, "vkCmdSetStencilReference");
    bindings->vkCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets) getDeviceProcAddress(device, "vkCmdBindDescriptorSets");
    bindings->vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer) getDeviceProcAddress(device, "vkCmdBindIndexBuffer");
    bindings->vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers) getDeviceProcAddress(device, "vkCmdBindVertexBuffers");
    bindings->vkCmdDraw = (PFN_vkCmdDraw) getDeviceProcAddress(device, "vkCmdDraw");
    bindings->vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed) getDeviceProcAddress(device, "vkCmdDrawIndexed");
    bindings->vkCmdDrawIndirect = (PFN_vkCmdDrawIndirect) getDeviceProcAddress(device, "vkCmdDrawIndirect");
    bindings->vkCmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect) getDeviceProcAddress(device, "vkCmdDrawIndexedIndirect");
    bindings->vkCmdDispatch = (PFN_vkCmdDispatch) getDeviceProcAddress(device, "vkCmdDispatch");
    bindings->vkCmdDispatchIndirect = (PFN_vkCmdDispatchIndirect) getDeviceProcAddress(device, "vkCmdDispatchIndirect");
    bindings->vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer) getDeviceProcAddress(device, "vkCmdCopyBuffer");
    bindings->vkCmdCopyImage = (PFN_vkCmdCopyImage) getDeviceProcAddress(device, "vkCmdCopyImage");
    bindings->vkCmdBlitImage = (PFN_vkCmdBlitImage) getDeviceProcAddress(device, "vkCmdBlitImage");
    bindings->vkCmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage) getDeviceProcAddress(device, "vkCmdCopyBufferToImage");
    bindings->vkCmdCopyImageToBuffer = (PFN_vkCmdCopyImageToBuffer) getDeviceProcAddress(device, "vkCmdCopyImageToBuffer");
    bindings->vkCmdUpdateBuffer = (PFN_vkCmdUpdateBuffer) getDeviceProcAddress(device, "vkCmdUpdateBuffer");
    bindings->vkCmdFillBuffer = (PFN_vkCmdFillBuffer) getDeviceProcAddress(device, "vkCmdFillBuffer");
    bindings->vkCmdClearColorImage = (PFN_vkCmdClearColorImage) getDeviceProcAddress(device, "vkCmdClearColorImage");
    bindings->vkCmdClearDepthStencilImage = (PFN_vkCmdClearDepthStencilImage) getDeviceProcAddress(device, "vkCmdClearDepthStencilImage");
    bindings->vkCmdClearAttachments = (PFN_vkCmdClearAttachments) getDeviceProcAddress(device, "vkCmdClearAttachments");
    bindings->vkCmdResolveImage = (PFN_vkCmdResolveImage) getDeviceProcAddress(device, "vkCmdResolveImage");
    bindings->vkCmdSetEvent = (PFN_vkCmdSetEvent) getDeviceProcAddress(device, "vkCmdSetEvent");
    bindings->vkCmdResetEvent = (PFN_vkCmdResetEvent) getDeviceProcAddress(device, "vkCmdResetEvent");
    bindings->vkCmdWaitEvents = (PFN_vkCmdWaitEvents) getDeviceProcAddress(device, "vkCmdWaitEvents");
    bindings->vkCmdPipelineBarrier = (PFN_vkCmdPipelineBarrier) getDeviceProcAddress(device, "vkCmdPipelineBarrier");
    bindings->vkCmdBeginQuery = (PFN_vkCmdBeginQuery) getDeviceProcAddress(device, "vkCmdBeginQuery");
    bindings->vkCmdEndQuery = (PFN_vkCmdEndQuery) getDeviceProcAddress(device, "vkCmdEndQuery");
    bindings->vkCmdResetQueryPool = (PFN_vkCmdResetQueryPool) getDeviceProcAddress(device, "vkCmdResetQueryPool");
    bindings->vkCmdWriteTimestamp = (PFN_vkCmdWriteTimestamp) getDeviceProcAddress(device, "vkCmdWriteTimestamp");
    bindings->vkCmdCopyQueryPoolResults = (PFN_vkCmdCopyQueryPoolResults) getDeviceProcAddress(device, "vkCmdCopyQueryPoolResults");
    bindings->vkCmdPushConstants = (PFN_vkCmdPushConstants) getDeviceProcAddress(device, "vkCmdPushConstants");
    bindings->vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass) getDeviceProcAddress(device, "vkCmdBeginRenderPass");
    bindings->vkCmdNextSubpass = (PFN_vkCmdNextSubpass) getDeviceProcAddress(device, "vkCmdNextSubpass");
    bindings->vkCmdEndRenderPass = (PFN_vkCmdEndRenderPass) getDeviceProcAddress(device, "vkCmdEndRenderPass");
    bindings->vkCmdExecuteCommands = (PFN_vkCmdExecuteCommands) getDeviceProcAddress(device, "vkCmdExecuteCommands");
    bindings->vkBindBufferMemory2 = (PFN_vkBindBufferMemory2) getDeviceProcAddress(device, "vkBindBufferMemory2");
    bindings->vkBindImageMemory2 = (PFN_vkBindImageMemory2) getDeviceProcAddress(device, "vkBindImageMemory2");
    bindings->vkGetDeviceGroupPeerMemoryFeatures = (PFN_vkGetDeviceGroupPeerMemoryFeatures) getDeviceProcAddress(device, "vkGetDeviceGroupPeerMemoryFeatures");
    bindings->vkCmdSetDeviceMask = (PFN_vkCmdSetDeviceMask) getDeviceProcAddress(device, "vkCmdSetDeviceMask");
    bindings->vkCmdDispatchBase = (PFN_vkCmdDispatchBase) getDeviceProcAddress(device, "vkCmdDispatchBase");
    bindings->vkGetImageMemoryRequirements2 = (PFN_vkGetImageMemoryRequirements2) getDeviceProcAddress(device, "vkGetImageMemoryRequirements2");
    bindings->vkGetBufferMemoryRequirements2 = (PFN_vkGetBufferMemoryRequirements2) getDeviceProcAddress(device, "vkGetBufferMemoryRequirements2");
    bindings->vkGetImageSparseMemoryRequirements2 = (PFN_vkGetImageSparseMemoryRequirements2) getDeviceProcAddress(device, "vkGetImageSparseMemoryRequirements2");
    bindings->vkTrimCommandPool = (PFN_vkTrimCommandPool) getDeviceProcAddress(device, "vkTrimCommandPool");
    bindings->vkGetDeviceQueue2 = (PFN_vkGetDeviceQueue2) getDeviceProcAddress(device, "vkGetDeviceQueue2");
    bindings->vkCreateSamplerYcbcrConversion = (PFN_vkCreateSamplerYcbcrConversion) getDeviceProcAddress(device, "vkCreateSamplerYcbcrConversion");
    bindings->vkDestroySamplerYcbcrConversion = (PFN_vkDestroySamplerYcbcrConversion) getDeviceProcAddress(device, "vkDestroySamplerYcbcrConversion");
    bindings->vkCreateDescriptorUpdateTemplate = (PFN_vkCreateDescriptorUpdateTemplate) getDeviceProcAddress(device, "vkCreateDescriptorUpdateTemplate");
    bindings->vkDestroyDescriptorUpdateTemplate = (PFN_vkDestroyDescriptorUpdateTemplate) getDeviceProcAddress(device, "vkDestroyDescriptorUpdateTemplate");
    bindings->vkUpdateDescriptorSetWithTemplate = (PFN_vkUpdateDescriptorSetWithTemplate) getDeviceProcAddress(device, "vkUpdateDescriptorSetWithTemplate");
    bindings->vkGetDescriptorSetLayoutSupport = (PFN_vkGetDescriptorSetLayoutSupport) getDeviceProcAddress(device, "vkGetDescriptorSetLayoutSupport");
    bindings->vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR) getDeviceProcAddress(device, "vkCreateSwapchainKHR");
    bindings->vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR) getDeviceProcAddress(device, "vkDestroySwapchainKHR");
    bindings->vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR) getDeviceProcAddress(device, "vkGetSwapchainImagesKHR");
    bindings->vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR) getDeviceProcAddress(device, "vkAcquireNextImageKHR");
    bindings->vkQueuePresentKHR = (PFN_vkQueuePresentKHR) getDeviceProcAddress(device, "vkQueuePresentKHR");
    bindings->vkGetDeviceGroupPresentCapabilitiesKHR = (PFN_vkGetDeviceGroupPresentCapabilitiesKHR) getDeviceProcAddress(device, "vkGetDeviceGroupPresentCapabilitiesKHR");
    bindings->vkGetDeviceGroupSurfacePresentModesKHR = (PFN_vkGetDeviceGroupSurfacePresentModesKHR) getDeviceProcAddress(device, "vkGetDeviceGroupSurfacePresentModesKHR");
    bindings->vkAcquireNextImage2KHR = (PFN_vkAcquireNextImage2KHR) getDeviceProcAddress(device, "vkAcquireNextImage2KHR");
    bindings->vkCreateSharedSwapchainsKHR = (PFN_vkCreateSharedSwapchainsKHR) getDeviceProcAddress(device, "vkCreateSharedSwapchainsKHR");
    bindings->vkGetDeviceGroupPeerMemoryFeaturesKHR = (PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR) getDeviceProcAddress(device, "vkGetDeviceGroupPeerMemoryFeaturesKHR");
    bindings->vkCmdSetDeviceMaskKHR = (PFN_vkCmdSetDeviceMaskKHR) getDeviceProcAddress(device, "vkCmdSetDeviceMaskKHR");
    bindings->vkCmdDispatchBaseKHR = (PFN_vkCmdDispatchBaseKHR) getDeviceProcAddress(device, "vkCmdDispatchBaseKHR");
    bindings->vkTrimCommandPoolKHR = (PFN_vkTrimCommandPoolKHR) getDeviceProcAddress(device, "vkTrimCommandPoolKHR");
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bindings->vkGetMemoryWin32HandleKHR = (PFN_vkGetMemoryWin32HandleKHR) getDeviceProcAddress(device, "vkGetMemoryWin32HandleKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bindings->vkGetMemoryWin32HandlePropertiesKHR = (PFN_vkGetMemoryWin32HandlePropertiesKHR) getDeviceProcAddress(device, "vkGetMemoryWin32HandlePropertiesKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR
    bindings->vkGetMemoryFdKHR = (PFN_vkGetMemoryFdKHR) getDeviceProcAddress(device, "vkGetMemoryFdKHR");
    bindings->vkGetMemoryFdPropertiesKHR = (PFN_vkGetMemoryFdPropertiesKHR) getDeviceProcAddress(device, "vkGetMemoryFdPropertiesKHR");
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bindings->vkImportSemaphoreWin32HandleKHR = (PFN_vkImportSemaphoreWin32HandleKHR) getDeviceProcAddress(device, "vkImportSemaphoreWin32HandleKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bindings->vkGetSemaphoreWin32HandleKHR = (PFN_vkGetSemaphoreWin32HandleKHR) getDeviceProcAddress(device, "vkGetSemaphoreWin32HandleKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR
    bindings->vkImportSemaphoreFdKHR = (PFN_vkImportSemaphoreFdKHR) getDeviceProcAddress(device, "vkImportSemaphoreFdKHR");
    bindings->vkGetSemaphoreFdKHR = (PFN_vkGetSemaphoreFdKHR) getDeviceProcAddress(device, "vkGetSemaphoreFdKHR");
    bindings->vkCmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR) getDeviceProcAddress(device, "vkCmdPushDescriptorSetKHR");
    bindings->vkCmdPushDescriptorSetWithTemplateKHR = (PFN_vkCmdPushDescriptorSetWithTemplateKHR) getDeviceProcAddress(device, "vkCmdPushDescriptorSetWithTemplateKHR");
    bindings->vkCreateDescriptorUpdateTemplateKHR = (PFN_vkCreateDescriptorUpdateTemplateKHR) getDeviceProcAddress(device, "vkCreateDescriptorUpdateTemplateKHR");
    bindings->vkDestroyDescriptorUpdateTemplateKHR = (PFN_vkDestroyDescriptorUpdateTemplateKHR) getDeviceProcAddress(device, "vkDestroyDescriptorUpdateTemplateKHR");
    bindings->vkUpdateDescriptorSetWithTemplateKHR = (PFN_vkUpdateDescriptorSetWithTemplateKHR) getDeviceProcAddress(device, "vkUpdateDescriptorSetWithTemplateKHR");
    bindings->vkGetSwapchainStatusKHR = (PFN_vkGetSwapchainStatusKHR) getDeviceProcAddress(device, "vkGetSwapchainStatusKHR");
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bindings->vkImportFenceWin32HandleKHR = (PFN_vkImportFenceWin32HandleKHR) getDeviceProcAddress(device, "vkImportFenceWin32HandleKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bindings->vkGetFenceWin32HandleKHR = (PFN_vkGetFenceWin32HandleKHR) getDeviceProcAddress(device, "vkGetFenceWin32HandleKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR
    bindings->vkImportFenceFdKHR = (PFN_vkImportFenceFdKHR) getDeviceProcAddress(device, "vkImportFenceFdKHR");
    bindings->vkGetFenceFdKHR = (PFN_vkGetFenceFdKHR) getDeviceProcAddress(device, "vkGetFenceFdKHR");
    bindings->vkGetImageMemoryRequirements2KHR = (PFN_vkGetImageMemoryRequirements2KHR) getDeviceProcAddress(device, "vkGetImageMemoryRequirements2KHR");
    bindings->vkGetBufferMemoryRequirements2KHR = (PFN_vkGetBufferMemoryRequirements2KHR) getDeviceProcAddress(device, "vkGetBufferMemoryRequirements2KHR");
    bindings->vkGetImageSparseMemoryRequirements2KHR = (PFN_vkGetImageSparseMemoryRequirements2KHR) getDeviceProcAddress(device, "vkGetImageSparseMemoryRequirements2KHR");
    bindings->vkCreateSamplerYcbcrConversionKHR = (PFN_vkCreateSamplerYcbcrConversionKHR) getDeviceProcAddress(device, "vkCreateSamplerYcbcrConversionKHR");
    bindings->vkDestroySamplerYcbcrConversionKHR = (PFN_vkDestroySamplerYcbcrConversionKHR) getDeviceProcAddress(device, "vkDestroySamplerYcbcrConversionKHR");
    bindings->vkBindBufferMemory2KHR = (PFN_vkBindBufferMemory2KHR) getDeviceProcAddress(device, "vkBindBufferMemory2KHR");
    bindings->vkBindImageMemory2KHR = (PFN_vkBindImageMemory2KHR) getDeviceProcAddress(device, "vkBindImageMemory2KHR");
    bindings->vkGetDescriptorSetLayoutSupportKHR = (PFN_vkGetDescriptorSetLayoutSupportKHR) getDeviceProcAddress(device, "vkGetDescriptorSetLayoutSupportKHR");
    bindings->vkDebugMarkerSetObjectTagEXT = (PFN_vkDebugMarkerSetObjectTagEXT) getDeviceProcAddress(device, "vkDebugMarkerSetObjectTagEXT");
    bindings->vkDebugMarkerSetObjectNameEXT = (PFN_vkDebugMarkerSetObjectNameEXT) getDeviceProcAddress(device, "vkDebugMarkerSetObjectNameEXT");
    bindings->vkCmdDebugMarkerBeginEXT = (PFN_vkCmdDebugMarkerBeginEXT) getDeviceProcAddress(device, "vkCmdDebugMarkerBeginEXT");
    bindings->vkCmdDebugMarkerEndEXT = (PFN_vkCmdDebugMarkerEndEXT) getDeviceProcAddress(device, "vkCmdDebugMarkerEndEXT");
    bindings->vkCmdDebugMarkerInsertEXT = (PFN_vkCmdDebugMarkerInsertEXT) getDeviceProcAddress(device, "vkCmdDebugMarkerInsertEXT");
    bindings->vkCmdDrawIndirectCountAMD = (PFN_vkCmdDrawIndirectCountAMD) getDeviceProcAddress(device, "vkCmdDrawIndirectCountAMD");
    bindings->vkCmdDrawIndexedIndirectCountAMD = (PFN_vkCmdDrawIndexedIndirectCountAMD) getDeviceProcAddress(device, "vkCmdDrawIndexedIndirectCountAMD");
    bindings->vkGetShaderInfoAMD = (PFN_vkGetShaderInfoAMD) getDeviceProcAddress(device, "vkGetShaderInfoAMD");
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bindings->vkGetMemoryWin32HandleNV = (PFN_vkGetMemoryWin32HandleNV) getDeviceProcAddress(device, "vkGetMemoryWin32HandleNV");
#endif // VK_USE_PLATFORM_WIN32_KHR
    bindings->vkCmdProcessCommandsNVX = (PFN_vkCmdProcessCommandsNVX) getDeviceProcAddress(device, "vkCmdProcessCommandsNVX");
    bindings->vkCmdReserveSpaceForCommandsNVX = (PFN_vkCmdReserveSpaceForCommandsNVX) getDeviceProcAddress(device, "vkCmdReserveSpaceForCommandsNVX");
    bindings->vkCreateIndirectCommandsLayoutNVX = (PFN_vkCreateIndirectCommandsLayoutNVX) getDeviceProcAddress(device, "vkCreateIndirectCommandsLayoutNVX");
    bindings->vkDestroyIndirectCommandsLayoutNVX = (PFN_vkDestroyIndirectCommandsLayoutNVX) getDeviceProcAddress(device, "vkDestroyIndirectCommandsLayoutNVX");
    bindings->vkCreateObjectTableNVX = (PFN_vkCreateObjectTableNVX) getDeviceProcAddress(device, "vkCreateObjectTableNVX");
    bindings->vkDestroyObjectTableNVX = (PFN_vkDestroyObjectTableNVX) getDeviceProcAddress(device, "vkDestroyObjectTableNVX");
    bindings->vkRegisterObjectsNVX = (PFN_vkRegisterObjectsNVX) getDeviceProcAddress(device, "vkRegisterObjectsNVX");
    bindings->vkUnregisterObjectsNVX = (PFN_vkUnregisterObjectsNVX) getDeviceProcAddress(device, "vkUnregisterObjectsNVX");
    bindings->vkCmdSetViewportWScalingNV = (PFN_vkCmdSetViewportWScalingNV) getDeviceProcAddress(device, "vkCmdSetViewportWScalingNV");
    bindings->vkDisplayPowerControlEXT = (PFN_vkDisplayPowerControlEXT) getDeviceProcAddress(device, "vkDisplayPowerControlEXT");
    bindings->vkRegisterDeviceEventEXT = (PFN_vkRegisterDeviceEventEXT) getDeviceProcAddress(device, "vkRegisterDeviceEventEXT");
    bindings->vkRegisterDisplayEventEXT = (PFN_vkRegisterDisplayEventEXT) getDeviceProcAddress(device, "vkRegisterDisplayEventEXT");
    bindings->vkGetSwapchainCounterEXT = (PFN_vkGetSwapchainCounterEXT) getDeviceProcAddress(device, "vkGetSwapchainCounterEXT");
    bindings->vkGetRefreshCycleDurationGOOGLE = (PFN_vkGetRefreshCycleDurationGOOGLE) getDeviceProcAddress(device, "vkGetRefreshCycleDurationGOOGLE");
    bindings->vkGetPastPresentationTimingGOOGLE = (PFN_vkGetPastPresentationTimingGOOGLE) getDeviceProcAddress(device, "vkGetPastPresentationTimingGOOGLE");
    bindings->vkCmdSetDiscardRectangleEXT = (PFN_vkCmdSetDiscardRectangleEXT) getDeviceProcAddress(device, "vkCmdSetDiscardRectangleEXT");
    bindings->vkSetHdrMetadataEXT = (PFN_vkSetHdrMetadataEXT) getDeviceProcAddress(device, "vkSetHdrMetadataEXT");
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    bindings->vkGetAndroidHardwareBufferPropertiesANDROID = (PFN_vkGetAndroidHardwareBufferPropertiesANDROID) getDeviceProcAddress(device, "vkGetAndroidHardwareBufferPropertiesANDROID");
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    bindings->vkGetMemoryAndroidHardwareBufferANDROID = (PFN_vkGetMemoryAndroidHardwareBufferANDROID) getDeviceProcAddress(device, "vkGetMemoryAndroidHardwareBufferANDROID");
#endif // VK_USE_PLATFORM_ANDROID_KHR
    bindings->vkCmdSetSampleLocationsEXT = (PFN_vkCmdSetSampleLocationsEXT) getDeviceProcAddress(device, "vkCmdSetSampleLocationsEXT");
    bindings->vkCreateValidationCacheEXT = (PFN_vkCreateValidationCacheEXT) getDeviceProcAddress(device, "vkCreateValidationCacheEXT");
    bindings->vkDestroyValidationCacheEXT = (PFN_vkDestroyValidationCacheEXT) getDeviceProcAddress(device, "vkDestroyValidationCacheEXT");
    bindings->vkMergeValidationCachesEXT = (PFN_vkMergeValidationCachesEXT) getDeviceProcAddress(device, "vkMergeValidationCachesEXT");
    bindings->vkGetValidationCacheDataEXT = (PFN_vkGetValidationCacheDataEXT) getDeviceProcAddress(device, "vkGetValidationCacheDataEXT");
    bindings->vkGetMemoryHostPointerPropertiesEXT = (PFN_vkGetMemoryHostPointerPropertiesEXT) getDeviceProcAddress(device, "vkGetMemoryHostPointerPropertiesEXT");
    bindings->vkCmdWriteBufferMarkerAMD = (PFN_vkCmdWriteBufferMarkerAMD) getDeviceProcAddress(device, "vkCmdWriteBufferMarkerAMD");
}


static inline void initVkInstanceBindings(VkInstance instance, VkInstanceBindings *bindings, PFN_vkGetInstanceProcAddr getinstanceProcAddress) {
    memset(bindings, 0, sizeof(*bindings));
    // Instance function pointers
    bindings->vkDestroyInstance = (PFN_vkDestroyInstance) getinstanceProcAddress(instance, "vkDestroyInstance");
    bindings->vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices) getinstanceProcAddress(instance, "vkEnumeratePhysicalDevices");
    bindings->vkGetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures) getinstanceProcAddress(instance, "vkGetPhysicalDeviceFeatures");
    bindings->vkGetPhysicalDeviceFormatProperties = (PFN_vkGetPhysicalDeviceFormatProperties) getinstanceProcAddress(instance, "vkGetPhysicalDeviceFormatProperties");
    bindings->vkGetPhysicalDeviceImageFormatProperties = (PFN_vkGetPhysicalDeviceImageFormatProperties) getinstanceProcAddress(instance, "vkGetPhysicalDeviceImageFormatProperties");
    bindings->vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties) getinstanceProcAddress(instance, "vkGetPhysicalDeviceProperties");
    bindings->vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties) getinstanceProcAddress(instance, "vkGetPhysicalDeviceQueueFamilyProperties");
    bindings->vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties) getinstanceProcAddress(instance, "vkGetPhysicalDeviceMemoryProperties");
    bindings->vkGetInstanceProcAddr = getinstanceProcAddress;
    bindings->vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr) getinstanceProcAddress(instance, "vkGetDeviceProcAddr");
    bindings->vkCreateDevice = (PFN_vkCreateDevice) getinstanceProcAddress(instance, "vkCreateDevice");
    bindings->vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties) getinstanceProcAddress(instance, "vkEnumerateDeviceExtensionProperties");
    bindings->vkEnumerateDeviceLayerProperties = (PFN_vkEnumerateDeviceLayerProperties) getinstanceProcAddress(instance, "vkEnumerateDeviceLayerProperties");
    bindings->vkGetPhysicalDeviceSparseImageFormatProperties = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties) getinstanceProcAddress(instance, "vkGetPhysicalDeviceSparseImageFormatProperties");
    bindings->vkEnumeratePhysicalDeviceGroups = (PFN_vkEnumeratePhysicalDeviceGroups) getinstanceProcAddress(instance, "vkEnumeratePhysicalDeviceGroups");
    bindings->vkGetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2) getinstanceProcAddress(instance, "vkGetPhysicalDeviceFeatures2");
    bindings->vkGetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2) getinstanceProcAddress(instance, "vkGetPhysicalDeviceProperties2");
    bindings->vkGetPhysicalDeviceFormatProperties2 = (PFN_vkGetPhysicalDeviceFormatProperties2) getinstanceProcAddress(instance, "vkGetPhysicalDeviceFormatProperties2");
    bindings->vkGetPhysicalDeviceImageFormatProperties2 = (PFN_vkGetPhysicalDeviceImageFormatProperties2) getinstanceProcAddress(instance, "vkGetPhysicalDeviceImageFormatProperties2");
    bindings->vkGetPhysicalDeviceQueueFamilyProperties2 = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2) getinstanceProcAddress(instance, "vkGetPhysicalDeviceQueueFamilyProperties2");
    bindings->vkGetPhysicalDeviceMemoryProperties2 = (PFN_vkGetPhysicalDeviceMemoryProperties2) getinstanceProcAddress(instance, "vkGetPhysicalDeviceMemoryProperties2");
    bindings->vkGetPhysicalDeviceSparseImageFormatProperties2 = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2) getinstanceProcAddress(instance, "vkGetPhysicalDeviceSparseImageFormatProperties2");
    bindings->vkGetPhysicalDeviceExternalBufferProperties = (PFN_vkGetPhysicalDeviceExternalBufferProperties) getinstanceProcAddress(instance, "vkGetPhysicalDeviceExternalBufferProperties");
    bindings->vkGetPhysicalDeviceExternalFenceProperties = (PFN_vkGetPhysicalDeviceExternalFenceProperties) getinstanceProcAddress(instance, "vkGetPhysicalDeviceExternalFenceProperties");
    bindings->vkGetPhysicalDeviceExternalSemaphoreProperties = (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties) getinstanceProcAddress(instance, "vkGetPhysicalDeviceExternalSemaphoreProperties");
    bindings->vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR) getinstanceProcAddress(instance, "vkDestroySurfaceKHR");
    bindings->vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
    bindings->vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    bindings->vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    bindings->vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    bindings->vkGetPhysicalDevicePresentRectanglesKHR = (PFN_vkGetPhysicalDevicePresentRectanglesKHR) getinstanceProcAddress(instance, "vkGetPhysicalDevicePresentRectanglesKHR");
    bindings->vkGetPhysicalDeviceDisplayPropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceDisplayPropertiesKHR");
    bindings->vkGetPhysicalDeviceDisplayPlanePropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    bindings->vkGetDisplayPlaneSupportedDisplaysKHR = (PFN_vkGetDisplayPlaneSupportedDisplaysKHR) getinstanceProcAddress(instance, "vkGetDisplayPlaneSupportedDisplaysKHR");
    bindings->vkGetDisplayModePropertiesKHR = (PFN_vkGetDisplayModePropertiesKHR) getinstanceProcAddress(instance, "vkGetDisplayModePropertiesKHR");
    bindings->vkCreateDisplayModeKHR = (PFN_vkCreateDisplayModeKHR) getinstanceProcAddress(instance, "vkCreateDisplayModeKHR");
    bindings->vkGetDisplayPlaneCapabilitiesKHR = (PFN_vkGetDisplayPlaneCapabilitiesKHR) getinstanceProcAddress(instance, "vkGetDisplayPlaneCapabilitiesKHR");
    bindings->vkCreateDisplayPlaneSurfaceKHR = (PFN_vkCreateDisplayPlaneSurfaceKHR) getinstanceProcAddress(instance, "vkCreateDisplayPlaneSurfaceKHR");
#ifdef VK_USE_PLATFORM_XLIB_KHR
    bindings->vkCreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR) getinstanceProcAddress(instance, "vkCreateXlibSurfaceKHR");
#endif // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
    bindings->vkGetPhysicalDeviceXlibPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceXlibPresentationSupportKHR");
#endif // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
    bindings->vkCreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR) getinstanceProcAddress(instance, "vkCreateXcbSurfaceKHR");
#endif // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
    bindings->vkGetPhysicalDeviceXcbPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceXcbPresentationSupportKHR");
#endif // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    bindings->vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR) getinstanceProcAddress(instance, "vkCreateWaylandSurfaceKHR");
#endif // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    bindings->vkGetPhysicalDeviceWaylandPresentationSupportKHR = (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
#endif // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_MIR_KHR
    bindings->vkCreateMirSurfaceKHR = (PFN_vkCreateMirSurfaceKHR) getinstanceProcAddress(instance, "vkCreateMirSurfaceKHR");
#endif // VK_USE_PLATFORM_MIR_KHR
#ifdef VK_USE_PLATFORM_MIR_KHR
    bindings->vkGetPhysicalDeviceMirPresentationSupportKHR = (PFN_vkGetPhysicalDeviceMirPresentationSupportKHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceMirPresentationSupportKHR");
#endif // VK_USE_PLATFORM_MIR_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    bindings->vkCreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR) getinstanceProcAddress(instance, "vkCreateAndroidSurfaceKHR");
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bindings->vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR) getinstanceProcAddress(instance, "vkCreateWin32SurfaceKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bindings->vkGetPhysicalDeviceWin32PresentationSupportKHR = (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR
    bindings->vkGetPhysicalDeviceFeatures2KHR = (PFN_vkGetPhysicalDeviceFeatures2KHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceFeatures2KHR");
    bindings->vkGetPhysicalDeviceProperties2KHR = (PFN_vkGetPhysicalDeviceProperties2KHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceProperties2KHR");
    bindings->vkGetPhysicalDeviceFormatProperties2KHR = (PFN_vkGetPhysicalDeviceFormatProperties2KHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceFormatProperties2KHR");
    bindings->vkGetPhysicalDeviceImageFormatProperties2KHR = (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceImageFormatProperties2KHR");
    bindings->vkGetPhysicalDeviceQueueFamilyProperties2KHR = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceQueueFamilyProperties2KHR");
    bindings->vkGetPhysicalDeviceMemoryProperties2KHR = (PFN_vkGetPhysicalDeviceMemoryProperties2KHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceMemoryProperties2KHR");
    bindings->vkGetPhysicalDeviceSparseImageFormatProperties2KHR = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceSparseImageFormatProperties2KHR");
    bindings->vkEnumeratePhysicalDeviceGroupsKHR = (PFN_vkEnumeratePhysicalDeviceGroupsKHR) getinstanceProcAddress(instance, "vkEnumeratePhysicalDeviceGroupsKHR");
    bindings->vkGetPhysicalDeviceExternalBufferPropertiesKHR = (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceExternalBufferPropertiesKHR");
    bindings->vkGetPhysicalDeviceExternalSemaphorePropertiesKHR = (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
    bindings->vkGetPhysicalDeviceExternalFencePropertiesKHR = (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceExternalFencePropertiesKHR");
    bindings->vkGetPhysicalDeviceSurfaceCapabilities2KHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
    bindings->vkGetPhysicalDeviceSurfaceFormats2KHR = (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR) getinstanceProcAddress(instance, "vkGetPhysicalDeviceSurfaceFormats2KHR");
    bindings->vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) getinstanceProcAddress(instance, "vkCreateDebugReportCallbackEXT");
    bindings->vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT) getinstanceProcAddress(instance, "vkDestroyDebugReportCallbackEXT");
    bindings->vkDebugReportMessageEXT = (PFN_vkDebugReportMessageEXT) getinstanceProcAddress(instance, "vkDebugReportMessageEXT");
    bindings->vkGetPhysicalDeviceExternalImageFormatPropertiesNV = (PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV) getinstanceProcAddress(instance, "vkGetPhysicalDeviceExternalImageFormatPropertiesNV");
#ifdef VK_USE_PLATFORM_VI_NN
    bindings->vkCreateViSurfaceNN = (PFN_vkCreateViSurfaceNN) getinstanceProcAddress(instance, "vkCreateViSurfaceNN");
#endif // VK_USE_PLATFORM_VI_NN
    bindings->vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX = (PFN_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX) getinstanceProcAddress(instance, "vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX");
    bindings->vkReleaseDisplayEXT = (PFN_vkReleaseDisplayEXT) getinstanceProcAddress(instance, "vkReleaseDisplayEXT");
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
    bindings->vkAcquireXlibDisplayEXT = (PFN_vkAcquireXlibDisplayEXT) getinstanceProcAddress(instance, "vkAcquireXlibDisplayEXT");
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
    bindings->vkGetRandROutputDisplayEXT = (PFN_vkGetRandROutputDisplayEXT) getinstanceProcAddress(instance, "vkGetRandROutputDisplayEXT");
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT
    bindings->vkGetPhysicalDeviceSurfaceCapabilities2EXT = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT) getinstanceProcAddress(instance, "vkGetPhysicalDeviceSurfaceCapabilities2EXT");
#ifdef VK_USE_PLATFORM_IOS_MVK
    bindings->vkCreateIOSSurfaceMVK = (PFN_vkCreateIOSSurfaceMVK) getinstanceProcAddress(instance, "vkCreateIOSSurfaceMVK");
#endif // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
    bindings->vkCreateMacOSSurfaceMVK = (PFN_vkCreateMacOSSurfaceMVK) getinstanceProcAddress(instance, "vkCreateMacOSSurfaceMVK");
#endif // VK_USE_PLATFORM_MACOS_MVK
    bindings->vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT) getinstanceProcAddress(instance, "vkSetDebugUtilsObjectNameEXT");
    bindings->vkSetDebugUtilsObjectTagEXT = (PFN_vkSetDebugUtilsObjectTagEXT) getinstanceProcAddress(instance, "vkSetDebugUtilsObjectTagEXT");
    bindings->vkQueueBeginDebugUtilsLabelEXT = (PFN_vkQueueBeginDebugUtilsLabelEXT) getinstanceProcAddress(instance, "vkQueueBeginDebugUtilsLabelEXT");
    bindings->vkQueueEndDebugUtilsLabelEXT = (PFN_vkQueueEndDebugUtilsLabelEXT) getinstanceProcAddress(instance, "vkQueueEndDebugUtilsLabelEXT");
    bindings->vkQueueInsertDebugUtilsLabelEXT = (PFN_vkQueueInsertDebugUtilsLabelEXT) getinstanceProcAddress(instance, "vkQueueInsertDebugUtilsLabelEXT");
    bindings->vkCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT) getinstanceProcAddress(instance, "vkCmdBeginDebugUtilsLabelEXT");
    bindings->vkCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT) getinstanceProcAddress(instance, "vkCmdEndDebugUtilsLabelEXT");
    bindings->vkCmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT) getinstanceProcAddress(instance, "vkCmdInsertDebugUtilsLabelEXT");
    bindings->vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) getinstanceProcAddress(instance, "vkCreateDebugUtilsMessengerEXT");
    bindings->vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) getinstanceProcAddress(instance, "vkDestroyDebugUtilsMessengerEXT");
    bindings->vkSubmitDebugUtilsMessageEXT = (PFN_vkSubmitDebugUtilsMessageEXT) getinstanceProcAddress(instance, "vkSubmitDebugUtilsMessageEXT");
    bindings->vkGetPhysicalDeviceMultisamplePropertiesEXT = (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT) getinstanceProcAddress(instance, "vkGetPhysicalDeviceMultisamplePropertiesEXT");
}
