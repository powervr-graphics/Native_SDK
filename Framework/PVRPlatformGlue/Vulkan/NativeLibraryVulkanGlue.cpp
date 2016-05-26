/*!*********************************************************************************************************************
\file         PVRPlatformGlue\EGL\NativeLibraryEgl.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains implementation of functions required for the EGL wrapper library (namespace vkglue::).
***********************************************************************************************************************/
//!\cond NO_DOXYGEN

//TODO: This may not be required.

#include "PVRPlatformGlue/Vulkan/NativeLibraryVulkanGlue.h"
#include "PVRCore/NativeLibrary.h"
#include <algorithm>
#include <memory>
#include <string.h>
#ifdef _WIN32
static const char* vkglueLibraryPath = "vulkan-1.dll";
#elif defined(TARGET_OS_MAC)
static const char* vkglueLibraryPath = "libvulkan.dylib";
#else
static const char* vkglueLibraryPath = "libvulkan.so";
#endif

#define PVR_VULKAN_FUNCTION_POINTER_DEFINITION(function_name) PFN_vk##function_name vkglue::function_name;

std::vector<VkExtensionProperties> vkglue::extensionStore;
std::vector<VkLayerProperties> vkglue::layerStore;

PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroySemaphore);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(ResetFences);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetFenceStatus);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyFence);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(WaitForFences);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateImage);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyImage);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetImageMemoryRequirements);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(AllocateMemory);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(FreeMemory);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(BindImageMemory);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(QueueWaitIdle);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(BeginCommandBuffer);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CmdPipelineBarrier);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(EndCommandBuffer);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(AllocateCommandBuffers);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(FreeCommandBuffers);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(QueueSubmit);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(EnumerateDeviceLayerProperties)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(EnumerateDeviceExtensionProperties)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(EnumerateInstanceExtensionProperties)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(EnumerateInstanceLayerProperties)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceSurfaceCapabilitiesKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceSurfaceFormatsKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceFormatProperties)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateSwapchainKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetSwapchainImagesKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(AcquireNextImageKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(QueuePresentKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroySwapchainKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateInstance)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(EnumeratePhysicalDevices)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceQueueFamilyProperties)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceFeatures)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateDevice)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyImageView)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetInstanceProcAddr)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetDeviceProcAddr)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyDevice)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyInstance)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateImageView)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateSemaphore)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetDeviceQueue);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateFence);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceMemoryProperties);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceSurfacePresentModesKHR);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceSurfaceSupportKHR);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateCommandPool);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroyCommandPool);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceProperties);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(DestroySurfaceKHR);

#ifdef ANDROID
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateAndroidSurfaceKHR)

#elif defined(VK_USE_PLATFORM_WIN32_KHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateWin32SurfaceKHR);
#elif defined(VK_USE_PLATFORM_XLIB_KHR) || defined(VK_USE_PLATFORM_XCB_KHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateXlibSurfaceKHR);
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateXcbSurfaceKHR);
#else
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetPhysicalDeviceDisplayPropertiesKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(GetDisplayModePropertiesKHR)
PVR_VULKAN_FUNCTION_POINTER_DEFINITION(CreateDisplayPlaneSurfaceKHR)
#endif
#undef PVR_VULKAN_FUNCTION_POINTER_DEFINITION

static pvr::native::NativeLibrary& vkglueLib()
{
	static pvr::native::NativeLibrary mylib(vkglueLibraryPath);
	return mylib;
}
#define PVR_STR(x) #x
#define PVR_VULKAN_GET_INSTANCE_POINTER(instance, function_name) function_name = (PFN_vk##function_name)GetInstanceProcAddr(instance, "vk" PVR_STR(function_name));
#define PVR_VULKAN_GET_DEVICE_POINTER(device, function_name) function_name = (PFN_vk##function_name)GetDeviceProcAddr(device, "vk" PVR_STR(function_name));

bool vkglue::initVulkanGlue()
{
	GetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)vkglueLib().getFunction("vkGetInstanceProcAddr");
	EnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)vkglueLib().getFunction("vkEnumerateInstanceExtensionProperties");
	EnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)vkglueLib().getFunction("vkEnumerateInstanceLayerProperties");
	CreateInstance = (PFN_vkCreateInstance)vkglueLib().getFunction("vkCreateInstance");
	DestroyInstance = (PFN_vkDestroyInstance)vkglueLib().getFunction("vkDestroyInstance");

	enumerateExtensions();
	enumerateLayers();
	return true;
}

bool vkglue::initVulkanGlueInstance(VkInstance instance)
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
#ifdef ANDROID
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, CreateAndroidSurfaceKHR);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, CreateWin32SurfaceKHR);
#elif defined(VK_USE_PLATFORM_XLIB_KHR) || defined(VK_USE_PLATFORM_XCB_KHR)
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, CreateXlibSurfaceKHR);
	PVR_VULKAN_GET_INSTANCE_POINTER(instance, CreateXcbSurfaceKHR);
#else
	PVR_VULKAN_GET_INSTANCE_POINTER(NULL, GetPhysicalDeviceDisplayPropertiesKHR);//
	PVR_VULKAN_GET_INSTANCE_POINTER(NULL, GetDisplayModePropertiesKHR);//
	PVR_VULKAN_GET_INSTANCE_POINTER(NULL, CreateDisplayPlaneSurfaceKHR);//
#endif
	return true;
}

bool vkglue::initVulkanGlueDevice(VkDevice device)
{
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroySemaphore);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetFenceStatus);
	PVR_VULKAN_GET_DEVICE_POINTER(device, ResetFences);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyFence);
	PVR_VULKAN_GET_DEVICE_POINTER(device, WaitForFences);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateImage);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyImage);
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetImageMemoryRequirements);
	PVR_VULKAN_GET_DEVICE_POINTER(device, AllocateMemory);
	PVR_VULKAN_GET_DEVICE_POINTER(device, FreeMemory);
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
	PVR_VULKAN_GET_DEVICE_POINTER(device, AcquireNextImageKHR);//
	PVR_VULKAN_GET_DEVICE_POINTER(device, QueuePresentKHR);//
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroySwapchainKHR);//
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyImageView);//
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyDevice);//
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateImageView);//
	PVR_VULKAN_GET_DEVICE_POINTER(device, GetDeviceQueue);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateFence);
	PVR_VULKAN_GET_DEVICE_POINTER(device, CreateCommandPool);
	PVR_VULKAN_GET_DEVICE_POINTER(device, DestroyCommandPool);
	vkglue::CreateSemaphore = (PFN_vkCreateSemaphore)GetDeviceProcAddr(device, "vkCreateSemaphore");
	return true;
}

#undef PVR_STR
#undef PVR_VULKAN_GET_FUNCTION_POINTER


struct ExtensionComparer
{
	const std::string& extension;
	ExtensionComparer(const std::string& extension) : extension(extension) {}
	bool operator()(const VkExtensionProperties& props)
	{
		return !strcmp(extension.c_str(), props.extensionName);
	}
};

bool vkglue::isVulkanExtensionSupported(const std::string& extension)
{
	std::vector<VkExtensionProperties>::iterator find_if = std::find_if(extensionStore.begin(), extensionStore.end(), ExtensionComparer(extension));
	return (find_if != extensionStore.end());
}

bool vkglue::isVulkanGlueExtensionSupported(void* dpy, const std::string& extension)
{
	return isVulkanExtensionSupported(extension);
}

void vkglue::enumerateExtensions()
{
	pvr::uint32 numExtensions;
	vkglue::EnumerateInstanceExtensionProperties(NULL, &numExtensions, NULL);
	extensionStore.resize(numExtensions);
	vkglue::EnumerateInstanceExtensionProperties(NULL, &numExtensions, extensionStore.data());
}

void vkglue::enumerateLayers()
{
	pvr::uint32 propertyCount;
	vkglue::EnumerateInstanceLayerProperties(&propertyCount, NULL);
	layerStore.resize(propertyCount);
	vkglue::EnumerateInstanceLayerProperties(&propertyCount, layerStore.data());
}

namespace pvr {
namespace native {
void* glueGetProcAddress(const char* functionName)
{
	return (void*)vkglue::GetInstanceProcAddr(NULL, functionName);
}
}
}
//!\endcond
