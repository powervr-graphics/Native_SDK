/*!*********************************************************************************************************************
\file         PVRPlatformGlue/Vulkan/NativeLibraryVulkanGlue.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains the Vulkan Glue Function pointers
***********************************************************************************************************************/
#pragma once
#ifdef ANDROID
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(X11)
#define VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_XCB_KHR
#endif

#define VK_NO_PROTOTYPES 1
#include "vulkan/vulkan.h"
#if defined (VK_USE_PLATFORM_XLIB_KHR)
// undef these macros from the xlib files, they are breaking the framework types.
#undef Success
#undef Enum
#undef None
#undef Always
#undef byte
#undef char8
#undef ShaderStageFlags
#endif
#include "PVRCore/Types.h"
#include "PVRPlatformGlue/Vulkan/ApiVulkanGlue.h"
#include <map>

#define PVR_VULKAN_FUNCTION_POINTER_DECLARATION(function_name) static PFN_vk##function_name function_name;

/*!*********************************************************************************************************************
\brief   This class's static members are function pointers to the EGL functions. The Shell kicks off their initialisation on
        before context creation. Use the class name like a namespace: egl::ChooseConfig.
***********************************************************************************************************************/
class vkglue
{
public:
	/*!*********************************************************************************************************************
	\brief   Initialize the vulkan glue. automatically called just before context initialisation
	***********************************************************************************************************************/
	static bool initVulkanGlue();

	/*!*********************************************************************************************************************
	\brief   Initialize the vullkan instance function pointers. Automatically Called just before context initialisation
	***********************************************************************************************************************/
	static bool initVulkanGlueInstance(VkInstance instance);

	/*!*********************************************************************************************************************
	\brief   Initialize the vullkan  device function pointers. Automatically called just before context initialisation
	***********************************************************************************************************************/
	static bool initVulkanGlueDevice(VkDevice device);

	/*!*********************************************************************************************************************
	\brief   Check for the presense of an VulkanGlue extension for the specified display
	***********************************************************************************************************************/
	static bool isVulkanGlueExtensionSupported(void* display, const std::string& extension);

	/*!*********************************************************************************************************************
	\brief   Check for the presense of a Vulkan extension for the current context
	***********************************************************************************************************************/
	static bool isVulkanExtensionSupported(const std::string& extension);

	/*!*********************************************************************************************************************
	\brief  Get number of extensions supported
	\return Number of supported extensions
	***********************************************************************************************************************/
	static pvr::uint32 getNumExtensions() { return (pvr::uint32)extensionStore.size(); }

	/*!*********************************************************************************************************************
	\brief  Get number of layers supported
	\return Number of supported layers
	***********************************************************************************************************************/
	static pvr::uint32 getNumLayers() { return (pvr::uint32)layerStore.size(); }

	/*!*********************************************************************************************************************
	\brief   Get all supported layers
	\return Return all supported layers
	***********************************************************************************************************************/
	static VkLayerProperties* getAllLayersName() { return layerStore.data(); }

	/*!*********************************************************************************************************************
	\brief   Get all supported extensions
	\return Return all supported extensions
	***********************************************************************************************************************/
	static VkExtensionProperties* getAllExtensionsName() { return extensionStore.data(); }

	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroySemaphore);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(ResetFences);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetFenceStatus);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(WaitForFences);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyFence);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateImage);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyImage);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetImageMemoryRequirements);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(AllocateMemory);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(FreeMemory);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(BindImageMemory);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(QueueWaitIdle);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(BeginCommandBuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CmdPipelineBarrier);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EndCommandBuffer);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(AllocateCommandBuffers);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(FreeCommandBuffers);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateCommandPool);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(QueueSubmit);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSurfaceSupportKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSurfacePresentModesKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EnumerateDeviceLayerProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EnumerateDeviceExtensionProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EnumerateInstanceExtensionProperties); //static PFN_vkEnumerate... Enumerate... etc.
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EnumerateInstanceLayerProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSurfaceCapabilitiesKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceSurfaceFormatsKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceFormatProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateSwapchainKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetSwapchainImagesKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(AcquireNextImageKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(QueuePresentKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroySwapchainKHR);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateInstance);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(EnumeratePhysicalDevices);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceQueueFamilyProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceFeatures);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateDevice);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyImageView);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetInstanceProcAddr);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetDeviceProcAddr);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyDevice);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyInstance);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroyCommandPool);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateImageView);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateSemaphore);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetDeviceQueue);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(CreateFence);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceMemoryProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(GetPhysicalDeviceProperties);
	PVR_VULKAN_FUNCTION_POINTER_DECLARATION(DestroySurfaceKHR);

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
