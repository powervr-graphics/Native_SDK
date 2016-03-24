/*!*********************************************************************************************************************
\file         PVRPlatformGlue/Vulkan/PlatformContextVulkanGlue.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains the implementation of the PlatformContext class for Vulkan. Provides the implementation of the important
			  createNativePlatformContext function that the PVRShell uses to create the graphics context used for the main
			  application window.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
//#if defined(ANDROID)
//#include "EGL/egl.h"
//#endif// Android
#include "PVRPlatformGlue/Vulkan/NativeLibraryVulkanGlue.h"
#include "PVRPlatformGlue/Vulkan/ExtensionLoaderVulkanGlue.h"
#include "PVRPlatformGlue/Vulkan/PlatformHandlesVulkanGlue.h" //CAREFUL OF THE ORDER.
#include "PVRPlatformGlue/PlatformContext.h"
#include "PVRCore/StringFunctions.h"

namespace {
const char* instance_extension_names[] =
{
	"",
	"VK_KHR_surface",
	"VK_KHR_win32_surface",
//#ifdef DEBUG
	"VK_EXT_debug_report",
//#endif
};
const char* device_extension_names[] =
{
	"",
	"VK_KHR_swapchain",
//	"VK_NV_glsl_shader",
#ifdef DEBUG
	"VK_LUNARG_DEBUG_MARKER",
#endif
};
const char* instance_layer_names[] =
{
	"",
#ifdef DEBUG
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
#endif
};


const char* device_layer_names[] =
{
	"",
#ifdef DEBUG
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
#endif
};

inline char const* vkErrorToStr(VkResult errorCode)
{
	switch (errorCode)
	{
	case VK_SUCCESS: return "VK_SUCCESS";
	case VK_NOT_READY: return "VK_NOT_READY";
	case VK_TIMEOUT: return "VK_TIMEOUT";
	case VK_EVENT_SET: return "VK_EVENT_SET";
	case VK_EVENT_RESET: return "VK_EVENT_RESET";
	case VK_INCOMPLETE: return "VK_INCOMPLETE";
	case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
	case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
	case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
	case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
	case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
	case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
	case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
	case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
	case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
	case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
	case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
	// Declared here to Hide the Warnings.
	case VK_RESULT_RANGE_SIZE:
	case VK_RESULT_MAX_ENUM: break;
	}
	return "";

}

inline bool vkIsSuccessful(VkResult result, const char* msg)
{
	if (result != VK_SUCCESS)
	{
		pvr::Log(pvr::Log.Error, "Failed: %s. Vulkan has raised an error: %s", msg, vkErrorToStr(result));
		pvr::assertion(0);
		return false;
	}
	return true;
}


inline bool vkIsSuccessful(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		pvr::Log(pvr::Log.Error, "Failed vulkan command with Vulkan error: %s", vkErrorToStr(result));
		pvr::assertion(0);
		return false;
	}
	return true;
}
inline void vkSuccessOrDie(VkResult result, const char* msg)
{
	if (result != VK_SUCCESS)
	{
		pvr::Log(pvr::Log.Error, "Failed: %s. Vulkan has raised an error: %s", msg, vkErrorToStr(result));
		pvr::assertion(false, pvr::strings::createFormatted("Failed: %s. Vulkan has raised an error: %s", msg, vkErrorToStr(result)).c_str());
	}
}

bool allocateImageDeviceMemory(VkDevice device, VkImage& image, VkDeviceMemory& outMemory, VkMemoryRequirements* outMemRequirements)
{
	VkMemoryAllocateInfo memAllocInfo;
	VkMemoryRequirements memReq;
	VkMemoryRequirements* memReqPtr = &memReq;
	if (outMemRequirements)
	{
		memReqPtr = outMemRequirements;
	}
	vkglue::GetImageMemoryRequirements(device, image, memReqPtr);
	if (memReqPtr->memoryTypeBits == 0) // find the first allowed type
	{
		pvr::Log("Failed to get buffer memory requirements: memory requirements are 0");
		return false;
	}
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.pNext = NULL;
	memAllocInfo.allocationSize = memReqPtr->size;

	pvr::uint32 firstAllowedType = memReqPtr->memoryTypeBits - 1;
	memAllocInfo.memoryTypeIndex = firstAllowedType;

	if (vkglue::AllocateMemory(device, &memAllocInfo, NULL, &outMemory) != VK_SUCCESS)
	{
		pvr::Log("Failed to allocate Image memory");
		return false;
	}
	vkglue::BindImageMemory(device, image, outMemory, 0);
	return true;
}

}

namespace pvr {
using std::string;
namespace system {


inline bool postAcquireTransition(NativePlatformHandles_& handles, uint32 swapIndex, uint32 lastSwapIndex, VkFence fence)
{
	//LAYOUT TRANSITION COLOR ATTACHMENT -> PRESENTATION SRC
	VkSubmitInfo snfo = {};
	snfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	snfo.commandBufferCount = 1;

	snfo.pCommandBuffers = &handles.acquireBarrierCommandBuffers[swapIndex];

	snfo.pWaitSemaphores = &handles.semaphoreImageAcquired[lastSwapIndex];
	snfo.waitSemaphoreCount = (handles.semaphoreImageAcquired[lastSwapIndex] != VK_NULL_HANDLE);
	snfo.pSignalSemaphores = &handles.semaphoreCanBeginRendering[swapIndex];
	snfo.signalSemaphoreCount = (handles.semaphoreCanBeginRendering[swapIndex] != VK_NULL_HANDLE);
	VkPipelineStageFlags flags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	snfo.pWaitDstStageMask = &flags;

	if (!vkIsSuccessful(vkglue::QueueSubmit(handles.graphicsQueue, 1, &snfo, fence),
	                    "PresentBackBuffer: image layout transition PRESENTATION -> ATTACHMENT OPTIMAL failed"))
	{
		assertion(0, "PresentBackBuffer: image layout transition PRESENTATION -> ATTACHMENT OPTIMAL failed.");
		return false;
	};
	return true;
}

inline bool prePresentTransition(NativePlatformHandles_& handles, uint32& swapIndex, VkFence fence)
{
	//LAYOUT TRANSITION COLOR ATTACHMENT -> PRESENTATION SRC
	VkSubmitInfo snfo = {};
	snfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	snfo.commandBufferCount = 1;
	snfo.pCommandBuffers = &handles.presentBarrierCommandBuffers[swapIndex];

	snfo.pWaitSemaphores = &handles.semaphoreFinishedRendering[swapIndex];
	snfo.waitSemaphoreCount = (handles.semaphoreFinishedRendering[swapIndex] != VK_NULL_HANDLE);
	snfo.pSignalSemaphores = &handles.semaphoreCanPresent[swapIndex];
	snfo.signalSemaphoreCount = (handles.semaphoreCanPresent[swapIndex] != VK_NULL_HANDLE);
	VkPipelineStageFlags flags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	snfo.pWaitDstStageMask = &flags;

	if (!vkIsSuccessful(vkglue::QueueSubmit(handles.graphicsQueue, 1, &snfo, fence),
	                    "PresentBackBuffer: image layout transition ATTACHMENT OPTIMAL -> PRESENTATION failed"))
	{
		assertion(0, "PresentBackBuffer: image layout transition ATTACHMENT OPTIMAL -> PRESENTATION failed.");
		return false;
	};
	return true;
}


inline void logVKConfiguration(const DisplayAttributes& attributes)
{
	assertion(false, "Not implemented yet");
	Log(Log.Debug, "Vulkan Configuration");
	Log(Log.Debug, "\tRedBits: %d", attributes.redBits);
	Log(Log.Debug, "\tGreenBits: %d", attributes.greenBits);
	Log(Log.Debug, "\tBlueBits: %d", attributes.blueBits);
	Log(Log.Debug, "\tAlphaBits: %d", attributes.alphaBits);
	Log(Log.Debug, "\taaSamples: %d", attributes.aaSamples);
	Log(Log.Debug, "\tFullScreen: %s", (attributes.fullscreen ? "true" : "false"));
}

inline void editPhysicalDeviceFeatures(VkPhysicalDeviceFeatures& features)
{
	features.robustBufferAccess = false;
}

void PlatformContext::release()
{
	VkDevice dev = m_platformContextHandles->context.device;
	uint32 swapLength = m_displayHandle->swapChainLength;
	if (!m_initialized && !m_preInitialized) { return; }
	// release the display
	for (uint32 i = 0; i < swapLength; ++i)
	{
		vkglue::DestroyImageView(dev, m_displayHandle->fb.colorImageViews[i], NULL);
		vkglue::DestroyImageView(dev, m_displayHandle->fb.depthStencilImageView[i], NULL);
		vkglue::DestroyImage(dev, m_displayHandle->fb.depthStencilImage[i].first, NULL);
		vkglue::FreeMemory(dev, m_displayHandle->fb.depthStencilImage[i].second, NULL);
		vkglue::DestroyFence(dev, m_platformContextHandles->fenceAcquire[i], NULL);
		vkglue::DestroyFence(dev, m_platformContextHandles->fencePresent[i], NULL);
		vkglue::DestroyFence(dev, m_platformContextHandles->fenceRender[i], NULL);
		vkglue::DestroySemaphore(dev, m_platformContextHandles->semaphoreCanBeginRendering[i], NULL);
		vkglue::DestroySemaphore(dev, m_platformContextHandles->semaphoreCanPresent[i], NULL);
		vkglue::DestroySemaphore(dev, m_platformContextHandles->semaphoreFinishedRendering[i], NULL);
		vkglue::DestroySemaphore(dev, m_platformContextHandles->semaphoreImageAcquired[i], NULL);

		m_displayHandle->fb.colorImageViews[i] = VK_NULL_HANDLE;
		m_displayHandle->fb.depthStencilImageView[i] = VK_NULL_HANDLE;
		m_displayHandle->fb.depthStencilImage[i].first = VK_NULL_HANDLE;
		m_displayHandle->fb.depthStencilImage[i].second = VK_NULL_HANDLE;
		m_platformContextHandles->fenceAcquire[i] = VK_NULL_HANDLE;
		m_platformContextHandles->fencePresent[i], VK_NULL_HANDLE;
		m_platformContextHandles->fenceRender[i], VK_NULL_HANDLE;
		m_platformContextHandles->semaphoreCanBeginRendering[i] = VK_NULL_HANDLE;
		m_platformContextHandles->semaphoreCanPresent[i] = VK_NULL_HANDLE;
		m_platformContextHandles->semaphoreFinishedRendering[i] = VK_NULL_HANDLE;
		m_platformContextHandles->semaphoreImageAcquired[i] = VK_NULL_HANDLE;
	}
	vkglue::DestroySemaphore(dev, m_platformContextHandles->semaphoreImageAcquired[swapLength], NULL);
	vkglue::DestroyFence(dev, m_platformContextHandles->fencePresent[swapLength], NULL);
	vkglue::DestroyFence(dev, m_platformContextHandles->fenceAcquire[swapLength], NULL);

	vkglue::FreeCommandBuffers(dev, m_platformContextHandles->commandPool, 3, m_platformContextHandles->acquireBarrierCommandBuffers);
	vkglue::FreeCommandBuffers(dev, m_platformContextHandles->commandPool, 3, m_platformContextHandles->presentBarrierCommandBuffers);

	vkglue::DestroyCommandPool(dev, m_platformContextHandles->commandPool, NULL);
	m_platformContextHandles->commandPool = VK_NULL_HANDLE;
	vkglue::DestroySwapchainKHR(dev, m_displayHandle->swapChain, NULL);
	vkglue::DestroyDevice(dev, NULL);
	vkglue::DestroySurfaceKHR(m_platformContextHandles->context.instance, m_displayHandle->surface, NULL);
	vkglue::DestroyInstance(m_platformContextHandles->context.instance, NULL);
	m_initialized = m_preInitialized = false;
}

static inline std::vector<const char*> filterExtensions(const std::vector<VkExtensionProperties>& vec, const char* const* filters, uint32 numfilters)
{
	std::vector<const char*> retval;
	for (uint32 i = 0; i < vec.size(); ++i)
	{
		for (uint32 j = 0; j < numfilters; ++j)
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

static inline std::vector<const char*> filterLayers(const std::vector<VkLayerProperties>& vec, const char* const* filters, uint32 numfilters)
{
	std::vector<const char*> retval;
	for (uint32 i = 0; i < vec.size(); ++i)
	{
		for (uint32 j = 0; j < numfilters; ++j)
		{
			if (!strcmp(vec[i].layerName, filters[j]))
			{
				retval.push_back(filters[j]);
			}
		}
	}
	return retval;
}

static inline std::vector<const char*> getDeviceExtensions(VkPhysicalDevice device)
{
	uint32 numItems = 0;
	vkglue::EnumerateDeviceExtensionProperties(device, NULL, &numItems, NULL);
	std::vector<VkExtensionProperties> extensions; extensions.resize(numItems);
	vkglue::EnumerateDeviceExtensionProperties(device, NULL, &numItems, extensions.data());
	return filterExtensions(extensions, device_extension_names, sizeof(device_extension_names) / sizeof(device_extension_names[0]));
}

static inline std::vector<const char*> getInstanceExtensions()
{
	uint32 numItems = 0;
	vkglue::EnumerateInstanceExtensionProperties(NULL, &numItems, NULL);
	std::vector<VkExtensionProperties> extensions; extensions.resize(numItems);
	vkglue::EnumerateInstanceExtensionProperties(NULL, &numItems, extensions.data());
	return filterExtensions(extensions, instance_extension_names, sizeof(instance_extension_names) / sizeof(instance_extension_names[0]));
}

static inline std::vector<const char*> getDeviceLayers(VkPhysicalDevice device)
{
	uint32 numItems = 0;
	vkglue::EnumerateDeviceLayerProperties(device, &numItems, NULL);
	std::vector<VkLayerProperties> layers; layers.resize(numItems);
	vkglue::EnumerateDeviceLayerProperties(device, &numItems, layers.data());
	return filterLayers(layers, device_layer_names, sizeof(device_layer_names) / sizeof(device_layer_names[0]));
}

static inline std::vector<const char*> getInstanceLayers()
{
	uint32 numItems = 0;
	vkglue::EnumerateInstanceLayerProperties(&numItems, NULL);
	std::vector<VkLayerProperties> layers; layers.resize(numItems);
	vkglue::EnumerateInstanceLayerProperties(&numItems, layers.data());
	return filterLayers(layers, instance_layer_names, sizeof(instance_layer_names) / sizeof(instance_layer_names[0]));
}

static inline bool initVkInstanceAndPhysicalDevice(NativePlatformHandles_& platformHandle, const NativeDisplayHandle_& displayHandle,
    const OSManager& osManager, bool enableLayers)
{
	VkApplicationInfo appInfo;
	VkInstanceCreateInfo instanceCreateInfo;

	pvr::uint32 gpuCount = 100;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = NULL;
#if defined __linux__
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 3);
#else
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 2);
#endif
	appInfo.applicationVersion = 1;
	appInfo.engineVersion = 0;
	appInfo.pApplicationName = "PowerVR SDK Example";
	appInfo.pEngineName = "PVRApi";
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = NULL;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	std::vector<const char*> instanceExtensions = getInstanceExtensions();
	std::vector<const char*> instanceLayers = getInstanceLayers();

	instanceCreateInfo.enabledExtensionCount = (uint32)instanceExtensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
	instanceCreateInfo.enabledLayerCount = (uint32)instanceLayers.size();
	instanceCreateInfo.ppEnabledLayerNames = instanceLayers.data();

	// create the vk instance
	VkInstance instance;
	vkSuccessOrDie(vkglue::CreateInstance(&instanceCreateInfo, NULL, &instance), "Failed to create instance");

	pvr::native::HContext_& ctx = platformHandle.context;

	vkglue::initVulkanGlueInstance(instance);
	platformHandle.context.instance = instance;
	vkglue::EnumeratePhysicalDevices(instance, &gpuCount, NULL);
	Log(Log.Information, "Number of Vulkan Physical devices: [%d]", gpuCount);
	gpuCount = 1;
	vkglue::EnumeratePhysicalDevices(instance, &gpuCount, &ctx.physicalDevice);
	VkPhysicalDeviceProperties deviceProp;
	vkglue::GetPhysicalDeviceProperties(ctx.physicalDevice, &deviceProp);
	const std::string vendorName(deviceProp.deviceName);
	std::string vendorlower = strings::toLower(vendorName);

	platformHandle.supportPvrtcImage = ((vendorlower.find("powervr") != vendorlower.npos) ? 1 : 0);
	return true;
}


static inline bool initDevice(NativePlatformHandles_ & platformHandle, const NativeDisplayHandle_ & displayHandle,
                              bool enableLayers)
{
	pvr::native::HContext_& ctx = platformHandle.context;
	VkPhysicalDeviceFeatures physicalFeatures;
	vkglue::GetPhysicalDeviceFeatures(ctx.physicalDevice, &physicalFeatures);
	editPhysicalDeviceFeatures(physicalFeatures);
	VkDeviceQueueCreateInfo queueCreateInfo;
	VkDeviceCreateInfo deviceCreateInfo;
	// create the queue
	float priority = 1.f;
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.pNext = NULL;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.queueFamilyIndex = platformHandle.graphicsQueueIndex;
	queueCreateInfo.pQueuePriorities = &priority;
	queueCreateInfo.flags = 0;

	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = NULL;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.pEnabledFeatures = &physicalFeatures;

	std::vector<const char*> deviceExtensions = getDeviceExtensions(platformHandle.context.physicalDevice);
	std::vector<const char*> deviceLayers = getDeviceLayers(platformHandle.context.physicalDevice);

	deviceCreateInfo.enabledExtensionCount = (uint32)deviceExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceCreateInfo.enabledLayerCount = (uint32)deviceLayers.size();
	deviceCreateInfo.ppEnabledLayerNames = deviceLayers.data();

	if (vkIsSuccessful(vkglue::CreateDevice(platformHandle.context.physicalDevice, &deviceCreateInfo, NULL, &platformHandle.context.device), "Vulkan Device Creation"))
	{
		vkglue::initVulkanGlueDevice(platformHandle.context.device);
		// Gather physical device memory properties
		vkglue::GetPhysicalDeviceMemoryProperties(platformHandle.context.physicalDevice, &platformHandle.deviceMemProperties);
		vkglue::GetDeviceQueue(platformHandle.context.device, 0, 0, &platformHandle.graphicsQueue);
		return true;
	}
	return false;
}

// initilise the native display and its surface
static inline bool initSurface(NativePlatformHandles_ & platformHandle, NativeDisplayHandle_ & displayHandle)
{
#if defined(ANDROID)
	// create an android surface
	VkAndroidSurfaceCreateInfoKHR surfaceInfo;
	surfaceInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.pNext = NULL;
	surfaceInfo.flags = 0;
	surfaceInfo.window = displayHandle.nativeDisplay;
	vkSuccessOrDie(vkglue::CreateAndroidSurfaceKHR(platformHandle.context.instance, &surfaceInfo, NULL, &displayHandle.surface), "vkCreateAndroidSurfaceKHR returned an error");
#pragma warning TODO__NON_ANDROID_INIT_SURFACE

#elif defined _WIN32
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = (HINSTANCE)GetModuleHandle(NULL);
	surfaceCreateInfo.hwnd = (HWND)displayHandle.nativeDisplay;
	surfaceCreateInfo.flags = 0;
	vkSuccessOrDie(vkglue::CreateWin32SurfaceKHR(platformHandle.context.instance, &surfaceCreateInfo, NULL, &displayHandle.surface),
	               "failed to create Window surface, returned an error");
#else // Linux?!
	VkDisplayPropertiesKHR properties;
	uint32_t propertiesCount = 1;
	if (vkglue::GetPhysicalDeviceDisplayPropertiesKHR)
	{
		vkglue::GetPhysicalDeviceDisplayPropertiesKHR(platformHandle.context.physicalDevice, &propertiesCount, &properties);
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
	Log("**** Display Properties: ****");
	Log("name: %s", properties.displayName);
	Log("size: %dx%d", properties.physicalDimensions.width, properties.physicalDimensions.height);
	Log("resolution: %dx%d", properties.physicalResolution.width, properties.physicalResolution.height);
	Log("transforms: %s", supportedTransforms.c_str());
	Log("plane reordering?: %s", properties.planeReorderPossible ? "yes" : "no");
	Log("persistent conents?: %s", properties.persistentContent ? "yes" : "no");

	displayHandle.nativeDisplay = properties.display;

	uint32 modeCount = 0;
	vkglue::GetDisplayModePropertiesKHR(platformHandle.context.physicalDevice, displayHandle.nativeDisplay, &modeCount, NULL);
	std::vector<VkDisplayModePropertiesKHR> modeProperties; modeProperties.resize(modeCount);
	vkglue::GetDisplayModePropertiesKHR(platformHandle.context.physicalDevice, displayHandle.nativeDisplay, &modeCount, modeProperties.data());

	Log("Display Modes:");
	for (uint32_t i = 0; i < modeCount; ++i)
	{
		Log("\t[%u] %ux%u @%uHz", i, modeProperties[i].parameters.visibleRegion.width, modeProperties[i].parameters.visibleRegion.height, modeProperties[i].parameters.refreshRate);
	}

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

	if (!vkIsSuccessful(vkglue::CreateDisplayPlaneSurfaceKHR(platformHandle.context.instance, &surfaceCreateInfo, NULL, &displayHandle.surface),
	                    "Could not create DisplayPlane Surface"))
	{
		return false;
	}
#endif

	uint32 numQueues = 1;
	vkglue::GetPhysicalDeviceQueueFamilyProperties(platformHandle.context.physicalDevice, &numQueues, NULL);
	assertion(numQueues >= 1);
	std::vector<VkQueueFamilyProperties> queueProperties;
	queueProperties.resize(numQueues);
	platformHandle.graphicsQueueIndex = 0;
	vkglue::GetPhysicalDeviceQueueFamilyProperties(platformHandle.context.physicalDevice, &numQueues, &queueProperties[0]);

	// find the queue that support both graphics and present
	std::vector<VkBool32> supportsPresent(numQueues);
	for (pvr::uint32 i = 0; i < numQueues; ++i)
	{
		vkglue::GetPhysicalDeviceSurfaceSupportKHR(platformHandle.context.physicalDevice, i, displayHandle.surface, &supportsPresent[i]);
	}
	pvr::uint32 graphicsQueueIndex = pvr::uint32(-1);
	pvr::uint32 presentQueueIndex = pvr::uint32(-1);
	for (uint32 i = 0; i < numQueues; ++i)
	{
		if ((queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			if (graphicsQueueIndex == pvr::uint32(-1))
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
	if (graphicsQueueIndex == pvr::uint32(-1))
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
	if (graphicsQueueIndex == pvr::uint32(-1) || presentQueueIndex == pvr::uint32(-1))
	{
		pvr::Log("Could not find a graphics and a present queue Swapchain Initialization Failed");
	}
	// NOTE: While it is possible for an application to use a separate graphics
	//       and a present queues, the framework program assumes it is only using one
	if (graphicsQueueIndex != presentQueueIndex)
	{
		pvr::Log("Could not find a common graphics and a present queue Swapchain Initialization Failure");
	}

	platformHandle.graphicsQueueIndex = graphicsQueueIndex;
	return true;
}

static inline VkFormat getDepthFormat(OSManager& osManager)
{
	uint32 depthBpp = osManager.getDisplayAttributes().depthBPP;
	uint32 stencilBpp = osManager.getDisplayAttributes().stencilBPP;
	assertion((depthBpp == 16 || depthBpp == 24 || depthBpp == 32) && (stencilBpp == 0 || stencilBpp == 8), "getDepthFormat:: unsupported depth or stencil bits per pixel");
	return
	  stencilBpp ?
	  (depthBpp == 16 ? VK_FORMAT_D16_UNORM_S8_UINT : depthBpp == 24 ? VK_FORMAT_D24_UNORM_S8_UINT : VK_FORMAT_D32_SFLOAT_S8_UINT) :
	  (depthBpp == 16 ? VK_FORMAT_D16_UNORM : depthBpp == 24 ? VK_FORMAT_D32_SFLOAT : VK_FORMAT_D32_SFLOAT);
}

// create the swapchains, displayimages and views
static bool initSwapChain(NativePlatformHandles_ & platformHandle, NativeDisplayHandle_ & displayHandle, VkFormat dsFormat, VkExtent2D& outSize, VkExtent2D& outmaxSize)
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkglue::GetPhysicalDeviceSurfaceCapabilitiesKHR(platformHandle.context.physicalDevice, displayHandle.surface, &surfaceCapabilities);

	Log(Log.Information, "Surface Capabilities:");
	Log(Log.Information, "Image count: %u - %u", surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
	Log(Log.Information, "Array size: %u", surfaceCapabilities.maxImageArrayLayers);
	Log(Log.Information, "Image size (now): %dx%d", surfaceCapabilities.currentExtent.width, surfaceCapabilities.currentExtent.height);
	Log(Log.Information, "Image size (extent): %dx%d - %dx%d", surfaceCapabilities.minImageExtent.width, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.width,
	    surfaceCapabilities.maxImageExtent.height);
	Log(Log.Information, "Usage: %x", surfaceCapabilities.supportedUsageFlags);
	Log(Log.Information, "Current transform: %u\n", surfaceCapabilities.currentTransform);
	//Log("Allowed transforms: %x\n", surfaceCapabilities.supportedTransforms);
	outSize = surfaceCapabilities.currentExtent;
	outmaxSize = surfaceCapabilities.maxImageExtent;

	uint32_t formatCount = 0;
	vkglue::GetPhysicalDeviceSurfaceFormatsKHR(platformHandle.context.physicalDevice, displayHandle.surface, &formatCount, NULL);
	VkSurfaceFormatKHR tmpformats[16]; std::vector<VkSurfaceFormatKHR> tmpFormatsVector;
	VkSurfaceFormatKHR* allFormats = tmpformats;
	if (formatCount > 16)
	{
		tmpFormatsVector.resize(formatCount);
		allFormats = tmpFormatsVector.data();
	}
	vkglue::GetPhysicalDeviceSurfaceFormatsKHR(platformHandle.context.physicalDevice, displayHandle.surface, &formatCount, allFormats);

	VkSurfaceFormatKHR format = allFormats[0];

	VkFormat preferredFormats[] =
	{
		VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_B8G8R8_SNORM,  VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_R5G6B5_UNORM_PACK16
	};
	for (uint32_t f = 0; f < formatCount; ++f)
	{
		if (allFormats[f].format == preferredFormats[0] || allFormats[f].format == preferredFormats[1])
		{
			format = allFormats[f]; break;
		}
	}

	uint32 numPresentMode;
	vkSuccessOrDie(vkglue::GetPhysicalDeviceSurfacePresentModesKHR(platformHandle.context.physicalDevice, displayHandle.surface, &numPresentMode, NULL),
	               "Failed to get the number of present modes count");
	assertion(numPresentMode > 0);
	std::vector<VkPresentModeKHR> presentModes(numPresentMode);
	vkSuccessOrDie(vkglue::GetPhysicalDeviceSurfacePresentModesKHR(platformHandle.context.physicalDevice, displayHandle.surface, &numPresentMode, &presentModes[0]),
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
	swapchainCreate.minImageCount = std::max<uint32>(surfaceCapabilities.minImageCount + 1, std::min<uint32>(surfaceCapabilities.maxImageCount, PVR_MAX_SWAPCHAIN_IMAGES));
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
	
	assertion(swapchainCreate.minImageCount <= PVR_MAX_SWAPCHAIN_IMAGES, "Minimum number of swapchain images is larger than Max set");
	
	if (!vkIsSuccessful(vkglue::CreateSwapchainKHR(platformHandle.context.device, &swapchainCreate,
	                    NULL, &displayHandle.swapChain), "Could not create the swap chain"))
	{
		return false;
	}

	// get the number of swapchains
	if (!vkIsSuccessful(vkglue::GetSwapchainImagesKHR(platformHandle.context.device, displayHandle.swapChain,
	                    &displayHandle.swapChainLength, NULL), "Could not get swapchain length"))
	{
		return false;
	}

	assertion(displayHandle.swapChainLength <= PVR_MAX_SWAPCHAIN_IMAGES, "Number of swapchain images is larger than Max set");

	displayHandle.fb.colorImages.resize(displayHandle.swapChainLength);
	displayHandle.fb.colorImageViews.resize(displayHandle.swapChainLength);
	if (!vkIsSuccessful(vkglue::GetSwapchainImagesKHR(platformHandle.context.device, displayHandle.swapChain,
	                    &displayHandle.swapChainLength, &displayHandle.fb.colorImages[0]), "Could not get swapchain images"))
	{
		return false;
	}

	assertion(displayHandle.swapChainLength <= PVR_MAX_SWAPCHAIN_IMAGES, "Number of swapchain images is larger than Max set");


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

	for (uint32 i = 0; i < displayHandle.swapChainLength; ++i)
	{
		viewCreateInfo.image = displayHandle.fb.colorImages[i];
		if (!vkIsSuccessful(vkglue::CreateImageView(platformHandle.context.device, &viewCreateInfo, NULL,
		                    &displayHandle.fb.colorImageViews[i]), "create display image view"))
		{
			return false;
		}

		// create the depth stencil image
		VkImageCreateInfo dsCreateInfo;
		dsCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		dsCreateInfo.pNext = NULL;
		dsCreateInfo.flags = 0;
		dsCreateInfo.format = dsFormat;
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

		VkResult rslt = vkglue::CreateImage(platformHandle.context.device, &dsCreateInfo, NULL, &displayHandle.fb.depthStencilImage[i].first);
		vkSuccessOrDie(rslt, "Image creation failed");

		if (!allocateImageDeviceMemory(platformHandle.context.device, displayHandle.fb.depthStencilImage[i].first, displayHandle.fb.depthStencilImage[i].second, NULL))
		{
			assertion(false, "Memory allocation failed");
		}
		// create the depth stencil view
		VkImageViewCreateInfo dsViewCreateInfo;
		dsViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		dsViewCreateInfo.pNext = NULL;
		dsViewCreateInfo.image = displayHandle.fb.depthStencilImage[i].first;
		dsViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		dsViewCreateInfo.format = dsFormat;
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

		displayHandle.fb.depthStencilFormat = dsFormat;

		vkSuccessOrDie(vkglue::CreateImageView(platformHandle.context.device, &dsViewCreateInfo, NULL, &displayHandle.fb.depthStencilImageView[i]), "Create Depth stencil image view");
	}
	return true;
}

inline static bool initSynchronizationObjects(NativePlatformHandles_ & platformHandle, uint32 numSwapImages)
{
	VkDevice device = platformHandle.context.device;

	// create the semaphores
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;


	for (uint32 i = 0; i < numSwapImages; ++i)
	{
#define PVR_SEMAPHORES 1
#ifdef PVR_SEMAPHORES
		if (!vkIsSuccessful(vkglue::CreateSemaphore(device, &semaphoreCreateInfo, NULL, &platformHandle.semaphoreFinishedRendering[i]),
		                    "Cannot create the Swapchain Image Acquisition Semaphore"))
		{
			return false;
		}
		if (!vkIsSuccessful(vkglue::CreateSemaphore(device, &semaphoreCreateInfo, NULL, &platformHandle.semaphoreCanBeginRendering[i]),
		                    "Cannot create the Presentation Semaphore"))
		{
			return false;
		}
		if (!vkIsSuccessful(vkglue::CreateSemaphore(device, &semaphoreCreateInfo, NULL, &platformHandle.semaphoreImageAcquired[i]),
		                    "Cannot create the Swapchain Image Acquisition Semaphore"))
		{
			return false;
		}
		if (!vkIsSuccessful(vkglue::CreateSemaphore(device, &semaphoreCreateInfo, NULL, &platformHandle.semaphoreCanPresent[i]),
		                    "Cannot create the Presentation Semaphore"))
		{
			return false;
		}
#else
		platformHandle.semaphoreFinishedRendering[i] = VK_NULL_HANDLE;
		platformHandle.semaphoreCanBeginRendering[i] = VK_NULL_HANDLE;
		platformHandle.semaphoreImageAcquired[i] = VK_NULL_HANDLE;
		platformHandle.semaphoreCanPresent[i] = VK_NULL_HANDLE;
#endif
		if (!vkIsSuccessful(vkglue::CreateFence(device, &fenceCreateInfo, NULL, &platformHandle.fenceAcquire[i]), "Failed to create fence"))
		{
			return false;
		}
		if (!vkIsSuccessful(vkglue::CreateFence(device, &fenceCreateInfo, NULL, &platformHandle.fencePresent[i]), "Failed to create fence"))
		{
			return false;
		}
		if (!vkIsSuccessful(vkglue::CreateFence(device, &fenceCreateInfo, NULL, &platformHandle.fenceRender[i]), "Failed to create fence"))
		{
			return false;
		}
	}

#ifdef PVR_SEMAPHORES
	if (!vkIsSuccessful(vkglue::CreateSemaphore(device, &semaphoreCreateInfo, NULL, &platformHandle.semaphoreImageAcquired[numSwapImages]),
	                    "Cannot create the Swapchain Image Acquisition Semaphore"))
	{
		return false;
	}
	fenceCreateInfo.flags = 0;
	if (!vkIsSuccessful(vkglue::CreateFence(device, &fenceCreateInfo, NULL, &platformHandle.fencePresent[numSwapImages]), "Failed to create fence"))
	{
		return false;
	}
	if (!vkIsSuccessful(vkglue::CreateFence(device, &fenceCreateInfo, NULL, &platformHandle.fenceAcquire[numSwapImages]), "Failed to create fence"))
	{
		return false;
	}
#else
	platformHandle.semaphoreImageAcquired[numSwapImages] = VK_NULL_HANDLE;
#endif
	return true;
}

bool initPresentationCommandBuffers(NativePlatformHandles_& handles, NativeDisplayHandle_& displayHandle)
{
	// create the commandpool and setup commandbuffer
	VkCommandPoolCreateInfo pinfo = {};

	pinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	vkglue::CreateCommandPool(handles.context.device, &pinfo, NULL, &handles.commandPool);
	VkCommandBufferAllocateInfo cinfo = {};
	cinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cinfo.commandPool = handles.commandPool;
	cinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	cinfo.commandBufferCount = displayHandle.swapChainLength;

	vkglue::AllocateCommandBuffers(handles.context.device, &cinfo, handles.acquireBarrierCommandBuffers);
	vkglue::AllocateCommandBuffers(handles.context.device, &cinfo, handles.presentBarrierCommandBuffers);

	cinfo.commandBufferCount = 1;


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

	for (uint32 swapIndex = 0; swapIndex < displayHandle.swapChainLength; ++swapIndex)
	{
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		barrier.image = displayHandle.fb.colorImages[swapIndex];
		vkglue::BeginCommandBuffer(handles.presentBarrierCommandBuffers[swapIndex], &beginnfo);
		vkglue::CmdPipelineBarrier(handles.presentBarrierCommandBuffers[swapIndex],
		                           VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		                           VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0,
		                           NULL, 1, &barrier);
		vkglue::EndCommandBuffer(handles.presentBarrierCommandBuffers[swapIndex]);


		// post present
		barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		barrier.image = displayHandle.fb.colorImages[swapIndex];
		vkglue::BeginCommandBuffer(handles.acquireBarrierCommandBuffers[swapIndex], &beginnfo);
		vkglue::CmdPipelineBarrier(handles.acquireBarrierCommandBuffers[swapIndex],
		                           VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		                           VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0,
		                           NULL, 1, &barrier);
		vkglue::EndCommandBuffer(handles.acquireBarrierCommandBuffers[swapIndex]);
	}
	return true;
}



Api::Enum PlatformContext::getMaxApiVersion()
{
	if (m_maxApiVersion == Api::Unspecified) { populateMaxApiVersion(); }
	return m_maxApiVersion;
}

void PlatformContext::populateMaxApiVersion()
{
	//Originally, only support numExtensionsone Vulkan version
	m_maxApiVersion = Api::Vulkan;
}

bool PlatformContext::isApiSupported(Api::Enum apiLevel) { return apiLevel == Api::Vulkan; }

bool PlatformContext::makeCurrent()
{
	//No global context...
	return true;
}

string PlatformContext::getInfo()
{
	assertion(false, "Not Implemented Yet");
	return "";
	//Ya, well, we have other issues now...
}

uint32 PlatformContext::getSwapChainLength() const
{
	return m_displayHandle->swapChainLength;
}


/*This function assumes that the osManager's getDisplay() and getWindow() types are one and the same with NativePlatformHandles::NativeDisplay and NativePlatformHandles::NativeWindow.*/
Result::Enum PlatformContext::init()
{
	if (m_initialized) { return Result::AlreadyInitialized; }
	m_preInitialized = true;
	populateMaxApiVersion();
	m_platformContextHandles.reset(new NativePlatformHandles_());
	NativePlatformHandles_& handles = *m_platformContextHandles;

	m_displayHandle.reset(new NativeDisplayHandle_());
	m_displayHandle->nativeDisplay = reinterpret_cast<NativeDisplay>(m_OSManager.getWindow());

	if (m_OSManager.getApiTypeRequired() == Api::Unspecified)
	{
		if (m_OSManager.getMinApiTypeRequired() == Api::Unspecified)
		{
			Api::Enum version = getMaxApiVersion();
			m_OSManager.setApiTypeRequired(version);
			Log(Log.Information, "Unspecified target API -- Setting to max API level : %s", Api::getApiName(version));
		}
		else
		{
			Api::Enum version = (std::max)(m_OSManager.getMinApiTypeRequired(), getMaxApiVersion());
			Log(Log.Information, "Requested minimum API level : %s. Will actually create %s since it is supported.",
			    Api::getApiName(m_OSManager.getMinApiTypeRequired()), Api::getApiName(getMaxApiVersion()));
			m_OSManager.setApiTypeRequired(version);
		}
	}
	else
	{
		Log(Log.Information, "Forcing specific API level: %s", Api::getApiName(m_OSManager.getApiTypeRequired()));
	}

	if (m_OSManager.getApiTypeRequired() != Api::Vulkan)
	{
		Log(Log.Error, "API level requested [%s] was not supported. Only Supported API level on this device is [%s]\n",
		    Api::getApiName(m_OSManager.getApiTypeRequired()), Api::Vulkan);
		return Result::UnsupportedRequest;
	}
	VkExtent2D size, maxsize;

	std::vector<const char*> layers;
	std::vector<const char*> extensions;

	if (!initVkInstanceAndPhysicalDevice(*m_platformContextHandles, *m_displayHandle, m_OSManager, true)) { return Result::UnknownError; }
	if (!initSurface(*m_platformContextHandles, *m_displayHandle)) { return Result::UnknownError; }
	if (!initDevice(*m_platformContextHandles, *m_displayHandle, true)) { return Result::UnknownError; }
	if (!initSwapChain(*m_platformContextHandles, *m_displayHandle, getDepthFormat(m_OSManager), size, maxsize)) { return Result::UnknownError; }
	if (!initSynchronizationObjects(*m_platformContextHandles, m_displayHandle->swapChainLength)) { return Result::UnknownError; }
	if (!initPresentationCommandBuffers(*m_platformContextHandles, *m_displayHandle)) { return Result::UnknownError; }

	m_OSManager.getDisplayAttributes().width = size.width;
	m_OSManager.getDisplayAttributes().height = size.height;

	//DO THE INITIAL LAYOUT TRANSITION OF THE BACKBUFFER IMAGES
	VkCommandBufferAllocateInfo cinfo = {};
	cinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cinfo.commandPool = handles.commandPool;
	cinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cinfo.commandBufferCount = 1;
	VkCommandBuffer tmpCmdBuff;
	if (!vkIsSuccessful(vkglue::AllocateCommandBuffers(handles.context.device, &cinfo, &tmpCmdBuff))) { return Result::UnknownError; }

	lastPresentedSwapIndex = getSwapChainLength(); //Using an extra sem. as we can't risk getting the one we should be waiting on.
	if (!vkIsSuccessful(vkglue::AcquireNextImageKHR(handles.context.device,
	                    m_displayHandle->swapChain, pvr::uint64(-1),
	                    handles.semaphoreImageAcquired[lastPresentedSwapIndex],
	                    VK_NULL_HANDLE, &swapIndex), "Failed to acquire initial Swapchain image"))
	{
		return Result::UnknownError;
	}

	vkglue::ResetFences(m_platformContextHandles->context.device, 1, &m_platformContextHandles->fenceAcquire[swapIndex]);
	vkglue::ResetFences(m_platformContextHandles->context.device, 1, &m_platformContextHandles->fenceRender[swapIndex]);

	postAcquireTransition(*m_platformContextHandles, swapIndex, lastPresentedSwapIndex, handles.fenceAcquire[swapIndex]);

	vkglue::QueueWaitIdle(handles.graphicsQueue);
	vkglue::FreeCommandBuffers(handles.context.device, handles.commandPool, 1, &tmpCmdBuff);

	m_initialized = true;
	return Result::Success;
}

bool PlatformContext::presentBackbuffer()
{
	vkglue::WaitForFences(m_platformContextHandles->context.device, 1, &m_platformContextHandles->fencePresent[swapIndex], true, std::numeric_limits<int64>::max());
	vkglue::ResetFences(m_platformContextHandles->context.device, 1, &m_platformContextHandles->fencePresent[swapIndex]);
	///// Ensure that we are over and done with with the image we just acquired!
	//Transition: Rendering done, ready to Present
	if (!prePresentTransition(*m_platformContextHandles, swapIndex, m_platformContextHandles->fencePresent[swapIndex])) { return false; }

	////// PRESENT //////
	VkResult result = VK_SUCCESS;
	static VkPresentInfoKHR pInfo = {};
	pInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	pInfo.swapchainCount = 1;
	pInfo.pSwapchains = &m_displayHandle->swapChain;
	pInfo.pImageIndices = &swapIndex;
	pInfo.pWaitSemaphores = &m_platformContextHandles->semaphoreCanPresent[swapIndex];

#ifndef __linux__
	pInfo.waitSemaphoreCount = (m_platformContextHandles->semaphoreCanPresent[swapIndex] != 0);
#endif
	pInfo.pResults = &result;

	if (!vkIsSuccessful(vkglue::QueuePresentKHR(m_platformContextHandles->graphicsQueue, &pInfo),
	                    "PlatformContext:PresentBackbuffer Present Queue error"))
	{
		assertion("PLatform Context: presentBackBuffer failed.");
		return false;
	}
	if (result != VK_SUCCESS)
	{
		pvr::Log("Present back buffer failed");
		return false;
	}

	/////////////// THE REAL FRAME SEPARATOR : ACQUIRE NEXT IMAGE //////////////////////////

	lastPresentedSwapIndex = swapIndex;

	if (!vkIsSuccessful(vkglue::AcquireNextImageKHR(m_platformContextHandles->context.device, m_displayHandle->swapChain, pvr::uint64(-1),
	                    m_platformContextHandles->semaphoreImageAcquired[lastPresentedSwapIndex],
	                    VK_NULL_HANDLE, &swapIndex),
	                    "PlatformContext:PresentBackbuffer AcquireNextImage error"))
	{
		return false;
	}

	vkglue::WaitForFences(m_platformContextHandles->context.device, 1, &m_platformContextHandles->fenceAcquire[swapIndex], true, std::numeric_limits<int64>::max());
	vkglue::ResetFences(m_platformContextHandles->context.device, 1, &m_platformContextHandles->fenceAcquire[swapIndex]);

	//Transition: READY TO RENDER
	if (!postAcquireTransition(*m_platformContextHandles, swapIndex, lastPresentedSwapIndex, m_platformContextHandles->fenceAcquire[swapIndex])) { return false; }


	vkglue::WaitForFences(m_platformContextHandles->context.device, 1, &m_platformContextHandles->fenceRender[swapIndex], true, std::numeric_limits<int64>::max());
	vkglue::ResetFences(m_platformContextHandles->context.device, 1, &m_platformContextHandles->fenceRender[swapIndex]);
	return true;
}


}// namespace system

}// namespace pvr

namespace pvr {
//Creates an instance of a graphics context
std::auto_ptr<IPlatformContext> createNativePlatformContext(OSManager& mgr)
{
	vkglue::initVulkanGlue();
	//vkglueext::initVulkanGlueExt();
	return std::auto_ptr<IPlatformContext>(new system::PlatformContext(mgr));
}
}// namespace pvr
//!\endcond
