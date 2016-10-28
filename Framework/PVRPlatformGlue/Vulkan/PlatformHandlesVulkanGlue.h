/*!*********************************************************************************************************************
\file         PVRPlatformGlue/Vulkan/PlatformHandlesVulkanGlue.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains platform objects required for vulkan initalization and surface creation
***********************************************************************************************************************/
#pragma once

#include "PVRCore/CoreIncludes.h"
#include "PVRCore/ForwardDecApiObjects.h"
#define VK_NO_PROTOTYPES 1
#include <vulkan/vulkan.h>
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#ifdef ANDROID
#include "android_native_app_glue.h"
#endif

namespace pvr {
namespace platform {
/*!*********************************************************************************************************************
\brief     Vulkan display type.
***********************************************************************************************************************/
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
	uint32      numberOfPhysicalDevices;

	const char* enabledExtensions[16];
	const char* enabledLayers[16];
	bool        supportPvrtcImage;
};

/*!*********************************************************************************************************************
\brief     Forward-declare and smart pointer friendly handle to all the objects that vulkan needs to identify a rendering context.
***********************************************************************************************************************/
struct NativePlatformHandles_
{
	native::HContext_                   context;
	VkQueue                             graphicsQueue;
	VkPhysicalDeviceMemoryProperties    deviceMemProperties;
	VkCommandPool                       commandPool;
	VkFence                             fenceAcquire[(uint32)FrameworkCaps::MaxSwapChains + 1];
	VkFence                             fencePrePresent[(uint32)FrameworkCaps::MaxSwapChains + 1];
	VkFence                             fenceRender[(uint32)FrameworkCaps::MaxSwapChains];
	VkCommandBuffer                     acquireBarrierCommandBuffers[(uint32)FrameworkCaps::MaxSwapChains];
	VkCommandBuffer                     presentBarrierCommandBuffers[(uint32)FrameworkCaps::MaxSwapChains];
	VkSemaphore                         semaphoreFinishedRendering[(uint32)FrameworkCaps::MaxSwapChains];
	VkSemaphore                         semaphoreCanPresent[(uint32)FrameworkCaps::MaxSwapChains];
	VkSemaphore                         semaphoreImageAcquired[(uint32)FrameworkCaps::MaxSwapChains + 1];
	VkSemaphore                         semaphoreCanBeginRendering[(uint32)FrameworkCaps::MaxSwapChains];
	VkDebugReportCallbackEXT			debugReportCallback;
	bool								supportsDebugReport;
	pvr::uint32                         graphicsQueueIndex;
	PlatformInfo                        platformInfo;
	uint32                              currentImageAcqSem;

    NativePlatformHandles_()  : supportsDebugReport(false), currentImageAcqSem(0) {}
};

/*!*********************************************************************************************************************
\brief     Forward-declare and smart pointer friendly handle to a vulkan display
***********************************************************************************************************************/
struct NativeDisplayHandle_
{
	NativeDisplay nativeDisplay;
	NativeSurface surface;
	VkExtent2D displayExtent;
	VkSwapchainKHR swapChain;
	uint32 swapChainLength;//< Number of SwapChains
	struct FrameBuffer
	{
		std::vector<VkImage> colorImages;
		std::vector<VkImageView>colorImageViews;
		std::vector<std::pair<VkImage, VkDeviceMemory>/**/> depthStencilImage;
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
