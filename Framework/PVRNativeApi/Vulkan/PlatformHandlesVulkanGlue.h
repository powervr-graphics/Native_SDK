/*!
\brief Contains platform objects required for vulkan initalization and surface creation
\file PVRNativeApi/Vulkan/PlatformHandlesVulkanGlue.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRNativeApi/Vulkan/HeadersVk.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include <string>
#include <vector>
#ifdef ANDROID
#include "android_native_app_glue.h"
#endif

namespace pvr {
namespace platform {
/// <summary>Vulkan display type.</summary>
#if defined(ANDROID)
typedef ANativeWindow* NativeWindow;
typedef NativeWindow NativeDisplay;
#elif defined(WIN32)
typedef void* NativeWindow;
typedef void* NativeDisplay;
#elif defined(X11)
typedef void* NativeWindow;
typedef void* NativeDisplay;
#else
typedef void* NativeWindow;
typedef VkDisplayKHR NativeDisplay;
#endif
typedef VkSurfaceKHR NativeSurface;

struct PlatformInfo
{
	std::string deviceName;
	std::string platformName;

	const char* enabledExtensions[16];
	const char* enabledLayers[16];
	bool        supportPvrtcImage;
	bool        supportsRayTracing;
	PlatformInfo() : supportPvrtcImage(false), supportsRayTracing(false) {}
};


struct QueueId
{
	uint32 family;
	uint32 idx;

};


/// <summary>Forward-declare and smart pointer friendly handle to all the objects that vulkan needs to identify a rendering
/// context.</summary>
struct NativePlatformHandles_
{
	native::HContext_                   context;
	pvr::uint32                         universalQueueFamily;
	VkQueue                             universalQueues[2];
	pvr::uint32                         universalQueueIndex;
	pvr::uint32                         universalQueueCount;

	VkPhysicalDeviceMemoryProperties    deviceMemProperties;
	VkCommandPool                       universalCommandPool;
	VkFence                             fenceAcquire[(uint32)FrameworkCaps::MaxSwapChains + 1];
	VkFence                             fencePrePresent[(uint32)FrameworkCaps::MaxSwapChains + 1];
	VkFence                             fenceRender[(uint32)FrameworkCaps::MaxSwapChains];
	VkCommandBuffer                     acquireBarrierCommandBuffersRenderQueue[(uint32)FrameworkCaps::MaxSwapChains];
	VkCommandBuffer                     presentBarrierCommandBuffersRenderQueue[(uint32)FrameworkCaps::MaxSwapChains];
	VkSemaphore                         semaphoreFinishedRendering[(uint32)FrameworkCaps::MaxSwapChains];
	VkSemaphore                         semaphoreCanPresent[(uint32)FrameworkCaps::MaxSwapChains];
	VkSemaphore                         semaphoreImageAcquired[(uint32)FrameworkCaps::MaxSwapChains + 1];
	VkSemaphore                         semaphoreCanBeginRendering[(uint32)FrameworkCaps::MaxSwapChains];
	VkDebugReportCallbackEXT            debugReportCallback;
	bool                                supportsDebugReport;
	std::vector<QueueId>                queuesRequested;

	PlatformInfo                        platformInfo;
	uint32                              currentImageAcqSem;

	VkQueue                             mainQueue() const { return universalQueues[universalQueueIndex]; }

	NativePlatformHandles_()  : supportsDebugReport(false), universalQueueFamily(uint32(-1)),
	    universalQueueIndex(0), universalQueueCount(0), universalCommandPool(VK_NULL_HANDLE), currentImageAcqSem(0)
	{
		universalQueues[0] = universalQueues[1] = VK_NULL_HANDLE;
		memset(fenceAcquire, VK_NULL_HANDLE, sizeof(fenceAcquire));
		memset(fencePrePresent, VK_NULL_HANDLE, sizeof(fencePrePresent));
		memset(fenceRender, VK_NULL_HANDLE, sizeof(fenceRender));
		memset(acquireBarrierCommandBuffersRenderQueue, VK_NULL_HANDLE, sizeof(acquireBarrierCommandBuffersRenderQueue));
		memset(semaphoreFinishedRendering, VK_NULL_HANDLE, sizeof(semaphoreFinishedRendering));
		memset(semaphoreCanPresent, VK_NULL_HANDLE, sizeof(semaphoreCanPresent));
		memset(semaphoreImageAcquired, VK_NULL_HANDLE, sizeof(semaphoreImageAcquired));
		memset(semaphoreCanBeginRendering, VK_NULL_HANDLE, sizeof(semaphoreCanBeginRendering));
	}
};


/// <todo_add_comment></todo_add_comment>
struct NativeSharedPlatformHandles_
{
	pvr::uint32                         queueFamily;
	VkQueue                             queue;
	VkCommandPool                       pool;

	NativeSharedPlatformHandles_() : queueFamily((uint32) - 1), queue(VK_NULL_HANDLE), pool(VK_NULL_HANDLE) {}
};

/// <summary>Forward-declare and smart pointer friendly handle to a vulkan display</summary>
struct NativeDisplayHandle_
{
	NativeDisplay nativeDisplay;
	NativeSurface surface;
	VkExtent2D displayExtent;
	VkSwapchainKHR swapChain;
	struct FrameBuffer
	{
		std::vector<VkImage> colorImages;
		std::vector<VkImageView>colorImageViews;
		std::vector<pvr::native::HTexture_> depthStencilImage;
		std::vector<VkImageView>depthStencilImageView;
		bool hasDepthStencil;
		VkFormat colorFormat;
		VkFormat depthStencilFormat;
	} onscreenFbo;
	NativeWindow nativeWindow;

	operator NativeDisplay& () { return nativeDisplay; }
	operator const NativeDisplay& () const { return nativeDisplay; }
};

}
}