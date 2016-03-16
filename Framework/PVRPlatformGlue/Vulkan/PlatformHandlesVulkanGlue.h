/*!*********************************************************************************************************************
\file         PVRPlatformGlue/Vulkan/PlatformHandlesVulkanGlue.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains platform objects required for vulkan initalization and surface creation
***********************************************************************************************************************/
#pragma once

#include "PVRCore/CoreIncludes.h"
#define VK_NO_PROTOTYPES 1
#include <vulkan/vulkan.h>
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#ifdef ANDROID
#include "android_native_app_glue.h"
#endif

#define PVR_MAX_SWAPCHAIN_IMAGES 4

namespace pvr {
namespace system {
/*!*********************************************************************************************************************
\brief     Vulkan display type.
***********************************************************************************************************************/
#if defined(ANDROID)
typedef ANativeWindow* NativeWindow;
typedef NativeWindow NativeDisplay;
#elif defined(WIN32)
typedef void* NativeDisplay;
#else
typedef VkDisplayKHR NativeDisplay;
#endif
typedef VkSurfaceKHR NativeSurface;

/*!*********************************************************************************************************************
\brief     Forward-declare and smart pointer friendly handle to all the objects that vulkan needs to identify a rendering context.
***********************************************************************************************************************/
struct NativePlatformHandles_
{
	native::HContext_ context;
	VkQueue graphicsQueue;
	VkQueue computeQueue;// TODO SUPPORT IT LATER
	VkPhysicalDeviceMemoryProperties deviceMemProperties;
	VkCommandPool	commandPool;
	VkFence fenceAcquire[PVR_MAX_SWAPCHAIN_IMAGES + 1];
	VkFence fencePresent[PVR_MAX_SWAPCHAIN_IMAGES + 1];
	VkFence fenceRender[PVR_MAX_SWAPCHAIN_IMAGES];
	VkCommandBuffer	acquireBarrierCommandBuffers[PVR_MAX_SWAPCHAIN_IMAGES];
	VkCommandBuffer	presentBarrierCommandBuffers[PVR_MAX_SWAPCHAIN_IMAGES];
	VkSemaphore semaphoreFinishedRendering[PVR_MAX_SWAPCHAIN_IMAGES];
	VkSemaphore semaphoreCanPresent[PVR_MAX_SWAPCHAIN_IMAGES];
	VkSemaphore semaphoreImageAcquired[PVR_MAX_SWAPCHAIN_IMAGES + 1];
	VkSemaphore semaphoreCanBeginRendering[PVR_MAX_SWAPCHAIN_IMAGES];
	bool	supportPvrtcImage;
	pvr::uint32 graphicsQueueIndex;
	NativePlatformHandles_() {}
};

/*!*********************************************************************************************************************
\brief     Forward-declare and smart pointer friendly handle to an vulkan display
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
		VkFormat colorFormat;
		VkFormat depthStencilFormat;
	} fb;

	operator NativeDisplay& () { return nativeDisplay; }
	operator const NativeDisplay& () const { return nativeDisplay; }
};

///*!*********************************************************************************************************************
//\brief     Forward-declare and smart pointer friendly handle to an EGL window
//***********************************************************************************************************************/
//struct NativeWindowHandle_
//{
//	NativeWindow nativeWindow;
//	operator NativeWindow& () { return nativeWindow; }
//	operator const NativeWindow& () const { return nativeWindow; }
//};

}
}
