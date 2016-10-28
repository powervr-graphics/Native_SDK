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
#if defined(X11)
#undef None
#undef Success
#undef capability
#endif
namespace {
const char* instance_extension_names[] =
{
	"",
	"VK_KHR_surface",
	"VK_KHR_display",
	"VK_KHR_win32_surface",
	"VK_KHR_android_surface",
	"VK_KHR_xlib_surface",
	"VK_KHR_xcb_surface",
#ifdef DEBUG
	"VK_EXT_debug_report",
#endif
};
const char* device_extension_names[] =
{
	"",
	"VK_KHR_swapchain",
	"VK_NV_glsl_shader",
	"VK_IMG_format_pvrtc",
#ifdef DEBUG
	"VK_LUNARG_DEBUG_MARKER",
#endif
};
const char* instance_layer_names[] =
{
	"",
#ifdef DEBUG
	"VK_LAYER_LUNARG_standard_validation",
	"VK_LAYER_LUNARG_api_dump",
#endif
};


const char* device_layer_names[] =
{
	"",
#ifdef DEBUG
	"VK_LAYER_LUNARG_standard_validation",
	"VK_LAYER_LUNARG_api_dump",
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
	case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
	case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
	// Declared here to Hide the Warnings.
	default: return "";
	}
}

#ifdef DEBUG

pvr::Logger::Severity mapValidationTypeToLogType(VkDebugReportFlagsEXT flags)
{
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
	{
		return pvr::Logger::Information;
	}
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		return pvr::Logger::Warning;
	}
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		return pvr::Logger::Information;
	}
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		return pvr::Logger::Error;
	}
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
	{
		return pvr::Logger::Debug;
	}

	return pvr::Logger::Information;
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
	pvr::Log(mapValidationTypeToLogType(flags), "LAYER_VALIDATION: %s",
	         pMessage);

	return VK_FALSE;
}
#endif

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
		pvr::assertion(false, pvr::strings::createFormatted("Failed: %s. Vulkan has raised an error: %s", msg,
		               vkErrorToStr(result)).c_str());
	}
}

bool getMemoryTypeIndex(VkPhysicalDeviceMemoryProperties& deviceMemProps, uint32_t typeBits,
                        VkMemoryPropertyFlagBits properties, uint32_t& outTypeIndex)
{
	for (uint32_t i = 0; i < 32; ++i)
	{
		if ((typeBits & 1) == 1)
		{
			if ((deviceMemProps.memoryTypes[i].propertyFlags & properties) == static_cast<pvr::uint32>(properties))
			{
				outTypeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	return false;
}

bool allocateImageDeviceMemory(pvr::platform::NativePlatformHandles_& platformHandle, VkImage& image,
                               VkDeviceMemory& outMemory, VkMemoryRequirements* outMemRequirements)
{
	VkMemoryAllocateInfo memAllocInfo;
	VkMemoryRequirements memReq;
	VkMemoryRequirements* memReqPtr = &memReq;
	if (outMemRequirements)
	{
		memReqPtr = outMemRequirements;
	}
	vkglue::GetImageMemoryRequirements(platformHandle.context.device, image, memReqPtr);
	if (memReqPtr->memoryTypeBits == 0) // find the first allowed type
	{
		pvr::Log("Failed to get image memory requirements: memory requirements are 0");
		return false;
	}
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.pNext = NULL;
	memAllocInfo.allocationSize = memReqPtr->size;

	//Find the first allowed type:
	getMemoryTypeIndex(platformHandle.deviceMemProperties, memReqPtr->memoryTypeBits,
	                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memAllocInfo.memoryTypeIndex);

	if (vkglue::AllocateMemory(platformHandle.context.device, &memAllocInfo, NULL, &outMemory) != VK_SUCCESS)
	{
		pvr::Log("Failed to allocate Image memory");
		return false;
	}
	vkglue::BindImageMemory(platformHandle.context.device, image, outMemory, 0);
	return true;
}

void setImageLayout(VkCommandBuffer& cmd, VkImageLayout oldLayout, VkImageLayout newLayout,
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
	vkglue::CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, NULL, 0,
	                           NULL, 1, memBarries);
}

VkCommandBuffer allocPrimaryCmdBuffer(pvr::platform::NativePlatformHandles_& platformHandle)
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandBufferCount = 1;
	allocInfo.commandPool = platformHandle.commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	VkCommandBuffer cmd;
	vkSuccessOrDie(vkglue::AllocateCommandBuffers(platformHandle.context.device, &allocInfo, &cmd), "Failed to allocate command buffer");
	return cmd;
}
}

namespace pvr {
using std::string;

bool use_old_pvrtc_vulkan_enums;

namespace platform {


inline bool postAcquireTransition(NativePlatformHandles_& handles, uint32 swapIndex, VkFence signalFence)
{
	// make sure acquireBarrierCommandBuffers is ready to be used
	vkglue::WaitForFences(handles.context.device, 1, &signalFence, true, uint64(-1));
	vkglue::ResetFences(handles.context.device, 1, &signalFence);

	//LAYOUT TRANSITION COLOR ATTACHMENT -> PRESENTATION SRC
	VkSubmitInfo snfo = {};
	snfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	snfo.commandBufferCount = 1;
	snfo.pCommandBuffers = &handles.acquireBarrierCommandBuffers[swapIndex];
	snfo.pWaitSemaphores = &handles.semaphoreImageAcquired[handles.currentImageAcqSem];
	snfo.waitSemaphoreCount = 1;
	snfo.pSignalSemaphores = &handles.semaphoreCanBeginRendering[swapIndex];
	snfo.signalSemaphoreCount = (handles.semaphoreCanBeginRendering[swapIndex] != VK_NULL_HANDLE);
	VkPipelineStageFlags flags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	snfo.pWaitDstStageMask = &flags;
	if (!vkIsSuccessful(vkglue::QueueSubmit(handles.graphicsQueue, 1, &snfo, signalFence),
	                    "PresentBackBuffer: image layout transition PRESENTATION -> ATTACHMENT OPTIMAL failed"))
	{
		assertion(0, "PresentBackBuffer: image layout transition PRESENTATION -> ATTACHMENT OPTIMAL failed.");
		return false;
	};
	return true;
}

inline bool prePresentTransition(NativePlatformHandles_& handles, uint32& swapIndex, VkFence signalFence)
{
	// make sure the presentBarrierCommandBuffers is ready to be used
	vkglue::WaitForFences(handles.context.device, 1, &signalFence, true, uint64(-1));
	vkglue::ResetFences(handles.context.device, 1, &signalFence);

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

	if (!vkIsSuccessful(vkglue::QueueSubmit(handles.graphicsQueue, 1, &snfo, signalFence),
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
		vkglue::DestroyImageView(dev, m_displayHandle->onscreenFbo.colorImageViews[i], NULL);
		if (m_displayHandle->onscreenFbo.hasDepthStencil)
		{
			vkglue::DestroyImageView(dev, m_displayHandle->onscreenFbo.depthStencilImageView[i], NULL);
			vkglue::DestroyImage(dev, m_displayHandle->onscreenFbo.depthStencilImage[i].first, NULL);
			vkglue::FreeMemory(dev, m_displayHandle->onscreenFbo.depthStencilImage[i].second, NULL);
		}
		vkglue::DestroyFence(dev, m_platformContextHandles->fenceAcquire[i], NULL);
		vkglue::DestroyFence(dev, m_platformContextHandles->fencePrePresent[i], NULL);
		vkglue::DestroyFence(dev, m_platformContextHandles->fenceRender[i], NULL);
		vkglue::DestroySemaphore(dev, m_platformContextHandles->semaphoreCanBeginRendering[i], NULL);
		vkglue::DestroySemaphore(dev, m_platformContextHandles->semaphoreCanPresent[i], NULL);
		vkglue::DestroySemaphore(dev, m_platformContextHandles->semaphoreFinishedRendering[i], NULL);
		vkglue::DestroySemaphore(dev, m_platformContextHandles->semaphoreImageAcquired[i], NULL);

		m_displayHandle->onscreenFbo.colorImageViews[i] = VK_NULL_HANDLE;
		if (m_displayHandle->onscreenFbo.hasDepthStencil)
		{
			m_displayHandle->onscreenFbo.depthStencilImageView[i] = VK_NULL_HANDLE;
			m_displayHandle->onscreenFbo.depthStencilImage[i].first = VK_NULL_HANDLE;
			m_displayHandle->onscreenFbo.depthStencilImage[i].second = VK_NULL_HANDLE;
		}
		m_platformContextHandles->fenceAcquire[i] = VK_NULL_HANDLE;
		m_platformContextHandles->fencePrePresent[i] = VK_NULL_HANDLE;
		m_platformContextHandles->fenceRender[i] = VK_NULL_HANDLE;
		m_platformContextHandles->semaphoreCanBeginRendering[i] = VK_NULL_HANDLE;
		m_platformContextHandles->semaphoreCanPresent[i] = VK_NULL_HANDLE;
		m_platformContextHandles->semaphoreFinishedRendering[i] = VK_NULL_HANDLE;
		m_platformContextHandles->semaphoreImageAcquired[i] = VK_NULL_HANDLE;
	}
	vkglue::DestroySemaphore(dev, m_platformContextHandles->semaphoreImageAcquired[swapLength], NULL);
	m_platformContextHandles->semaphoreImageAcquired[swapLength] = VK_NULL_HANDLE;

	vkglue::DestroyFence(dev, m_platformContextHandles->fencePrePresent[swapLength], NULL);
	m_platformContextHandles->fencePrePresent[swapLength] = VK_NULL_HANDLE;

	vkglue::DestroyFence(dev, m_platformContextHandles->fenceAcquire[swapLength], NULL);
	m_platformContextHandles->fenceAcquire[swapLength] = VK_NULL_HANDLE;

	vkglue::FreeCommandBuffers(dev, m_platformContextHandles->commandPool, swapLength,
	                           m_platformContextHandles->acquireBarrierCommandBuffers);

	vkglue::FreeCommandBuffers(dev, m_platformContextHandles->commandPool, swapLength,
	                           m_platformContextHandles->presentBarrierCommandBuffers);

#ifdef DEBUG
	if (m_platformContextHandles->debugReportCallback && m_platformContextHandles->supportsDebugReport)
	{
		vkglue::DestroyDebugReportCallbackEXT(m_platformContextHandles->context.instance,
		                                      m_platformContextHandles->debugReportCallback, NULL);
	}
#endif

	vkglue::DestroyCommandPool(dev, m_platformContextHandles->commandPool, NULL);
	m_platformContextHandles->commandPool = VK_NULL_HANDLE;
	vkglue::DestroySwapchainKHR(dev, m_displayHandle->swapChain, NULL);
	vkglue::DestroyDevice(dev, NULL);
	vkglue::DestroySurfaceKHR(m_platformContextHandles->context.instance, m_displayHandle->surface, NULL);
	vkglue::DestroyInstance(m_platformContextHandles->context.instance, NULL);

	m_initialized = m_preInitialized = false;
}

static inline std::vector<const char*> filterExtensions(const std::vector<VkExtensionProperties>& vec,
    const char* const* filters, uint32 numfilters)
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

static inline std::vector<const char*> filterLayers(const std::vector<VkLayerProperties>& vec,
    const char* const* filters, uint32 numfilters)
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
	return filterExtensions(extensions, device_extension_names,
	                        sizeof(device_extension_names) / sizeof(device_extension_names[0]));
}

static inline std::vector<const char*> getInstanceExtensions()
{
	uint32 numItems = 0;
	vkglue::EnumerateInstanceExtensionProperties(NULL, &numItems, NULL);
	std::vector<VkExtensionProperties> extensions; extensions.resize(numItems);
	vkglue::EnumerateInstanceExtensionProperties(NULL, &numItems, extensions.data());
	return filterExtensions(extensions, instance_extension_names,
	                        sizeof(instance_extension_names) / sizeof(instance_extension_names[0]));
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

static inline bool initVkInstanceAndPhysicalDevice(NativePlatformHandles_& platformHandle)
{
	VkApplicationInfo appInfo;
	VkInstanceCreateInfo instanceCreateInfo;

	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = NULL;
#if defined __linux__
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 3);
#else
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 5);
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
	const char* platformNames[] =
	{
		"VK_KHR_win32_surface",
		"VK_KHR_xlib_surface",
		"VK_KHR_xcb_surface",
	};

	for (uint32 i = 0; i < sizeof(platformNames) / sizeof(platformNames[0]); ++i)
	{
		for (uint32 j = 0; j < instanceExtensions.size(); ++j)
		{
			if (strcmp(instanceExtensions[j], platformNames[i]) == 0)
			{
				platformHandle.platformInfo.platformName = platformNames[i];
				i = sizeof(platformNames) / sizeof(platformNames[0]);// break the outer loop.
				break;
			}
		}
	}

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

	pvr::uint32 gpuCount = 100;
	vkglue::EnumeratePhysicalDevices(platformHandle.context.instance, &gpuCount, NULL);
	Log(Log.Information, "Number of Vulkan Physical devices: [%d]", gpuCount);
	vkglue::EnumeratePhysicalDevices(platformHandle.context.instance, &gpuCount, &ctx.physicalDevice);

#ifdef DEBUG
	if (vkglue::CreateDebugReportCallbackEXT &&
	    vkglue::DebugReportMessageEXT &&
	    vkglue::DestroyDebugReportCallbackEXT)
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
		VkResult result = vkglue::CreateDebugReportCallbackEXT(instance, &callbackCreateInfo,
		                  nullptr, &platformHandle.debugReportCallback);

		Log(Log.Information, "debug callback result: %i", result);

		if (result == VK_SUCCESS)
		{
			platformHandle.supportsDebugReport = true;
		}
		else
		{
			platformHandle.supportsDebugReport = false;
		}
	}
#endif

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
	uint32 count = 0;
	vkglue::GetPhysicalDeviceQueueFamilyProperties(platformHandle.context.physicalDevice, &count, NULL);
	std::vector<VkQueueFamilyProperties> queueProp(count);
	vkglue::GetPhysicalDeviceQueueFamilyProperties(platformHandle.context.physicalDevice, &count, &queueProp[0]);


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

	bool pvrtcExtensionString = false;

	for (pvr::uint32 i = 0; i < deviceExtensions.size(); i++)
	{
		if (!strcmp(deviceExtensions[i], "VK_IMG_format_pvrtc"))
		{
			pvrtcExtensionString = true;
		}
	}

	VkPhysicalDeviceProperties deviceProp;
	vkglue::GetPhysicalDeviceProperties(ctx.physicalDevice, &deviceProp);
	const std::string vendorName(deviceProp.deviceName);
	std::string vendorlower = strings::toLower(vendorName);

#ifndef VK_FORMAT_RGB_PVRTC1_4BPP_BLOCK_IMG_BETA
#define VK_FORMAT_RGB_PVRTC1_4BPP_BLOCK_IMG_BETA -0x40000001
#define VK_FORMAT_RGBA_PVRTC1_4BPP_BLOCK_IMG_BETA -0x40000002
#endif

	bool pvrtcVendorString = ((vendorlower.find("powervr") != vendorlower.npos) ? 1 : 0);;

	if (pvrtcExtensionString)
	{
		platformHandle.platformInfo.supportPvrtcImage = true;
		use_old_pvrtc_vulkan_enums = false;
	}
	else
	{
		if (pvrtcVendorString)
		{
			platformHandle.platformInfo.supportPvrtcImage = true;
			use_old_pvrtc_vulkan_enums = true;
		}
		else
		{
			platformHandle.platformInfo.supportPvrtcImage = false;
			use_old_pvrtc_vulkan_enums = false;
		}
	}

	std::vector<const char*> deviceLayers = getDeviceLayers(platformHandle.context.physicalDevice);

	deviceCreateInfo.enabledExtensionCount = (uint32)deviceExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceCreateInfo.enabledLayerCount = (uint32)deviceLayers.size();
	deviceCreateInfo.ppEnabledLayerNames = deviceLayers.data();

	if (vkIsSuccessful(vkglue::CreateDevice(platformHandle.context.physicalDevice, &deviceCreateInfo, NULL,
	                                        &platformHandle.context.device), "Vulkan Device Creation"))
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
	surfaceInfo.window = displayHandle.nativeWindow;
	vkSuccessOrDie(vkglue::CreateAndroidSurfaceKHR(platformHandle.context.instance, &surfaceInfo, NULL,
	               &displayHandle.surface), "failed to create Android Window surface, returned an error");

#elif defined _WIN32
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = (HINSTANCE)GetModuleHandle(NULL);
	surfaceCreateInfo.hwnd = (HWND)displayHandle.nativeWindow;
	surfaceCreateInfo.flags = 0;
	vkSuccessOrDie(vkglue::CreateWin32SurfaceKHR(platformHandle.context.instance, &surfaceCreateInfo, NULL, &displayHandle.surface),
	               "failed to create Win32 Window surface, returned an error");
#elif defined(X11)
	if (strcmp(platformHandle.platformInfo.platformName.c_str(), VK_KHR_XLIB_SURFACE_EXTENSION_NAME) == 0)
	{

		VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.dpy = (Display*)displayHandle.nativeDisplay;
		surfaceCreateInfo.window = (Window)displayHandle.nativeWindow;
		vkSuccessOrDie(vkglue::CreateXlibSurfaceKHR(platformHandle.context.instance, &surfaceCreateInfo, NULL, &displayHandle.surface),
		               "failed to create Xlib Window surface, returned an error");
	}
	else
	{
		assertion(false, "X11 platform not supported");
	}
#else // NullWS
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
	Log(Logger::Information, "**** Display Properties: ****");
	Log(Logger::Information, "name: %s", properties.displayName);
	Log(Logger::Information, "size: %dx%d", properties.physicalDimensions.width, properties.physicalDimensions.height);
	Log(Logger::Information, "resolution: %dx%d", properties.physicalResolution.width, properties.physicalResolution.height);
	Log(Logger::Information, "transforms: %s", supportedTransforms.c_str());
	Log(Logger::Information, "plane reordering?: %s", properties.planeReorderPossible ? "yes" : "no");
	Log(Logger::Information, "persistent conents?: %s", properties.persistentContent ? "yes" : "no");

	displayHandle.nativeDisplay = properties.display;

	uint32 modeCount = 0;
	vkglue::GetDisplayModePropertiesKHR(platformHandle.context.physicalDevice, displayHandle.nativeDisplay, &modeCount, NULL);
	std::vector<VkDisplayModePropertiesKHR> modeProperties; modeProperties.resize(modeCount);
	vkglue::GetDisplayModePropertiesKHR(platformHandle.context.physicalDevice, displayHandle.nativeDisplay, &modeCount, modeProperties.data());

	Log(Logger::Information, "Display Modes:");
	for (uint32_t i = 0; i < modeCount; ++i)
	{
		Log(Logger::Information, "\t[%u] %ux%u @%uHz", i, modeProperties[i].parameters.visibleRegion.width,
		    modeProperties[i].parameters.visibleRegion.height, modeProperties[i].parameters.refreshRate);
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

	if (!vkIsSuccessful(vkglue::CreateDisplayPlaneSurfaceKHR(platformHandle.context.instance, &surfaceCreateInfo,
	                    NULL, &displayHandle.surface), "Could not create DisplayPlane Surface"))
	{
		return false;
	}
#endif // NullWS

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
		// An implementation must support compute as well if a queue supports graphics
		if ((queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 &&
		    (queueProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0)
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

static inline VkFormat getDepthStencilFormat(DisplayAttributes& displayAttribs)
{
	uint32 depthBpp = displayAttribs.depthBPP;
	uint32 stencilBpp = displayAttribs.stencilBPP;

	VkFormat dsFormat = VK_FORMAT_UNDEFINED;

	if (stencilBpp)
	{
		switch (depthBpp)
		{
		case 0: dsFormat = VK_FORMAT_S8_UINT; break;
		case 16: dsFormat = VK_FORMAT_D16_UNORM_S8_UINT; break;
		case 24: dsFormat = VK_FORMAT_D24_UNORM_S8_UINT; break;
		case 32: dsFormat = VK_FORMAT_D32_SFLOAT_S8_UINT; break;
		default: assertion("Unsupported Depth Stencil Format");
		}
	}
	else
	{
		switch (depthBpp)
		{
		case 16: dsFormat = VK_FORMAT_D16_UNORM; break;
		case 24: dsFormat = VK_FORMAT_X8_D24_UNORM_PACK32; break;
		case 32: dsFormat = VK_FORMAT_D32_SFLOAT; break;
		default: assertion("Unsupported Depth Stencil Format");
		}
	}

	return dsFormat;
}

const std::string depthStencilFormatToStr(VkFormat format)
{
	const std::string preferredDsFormat[] =
	{
		"VK_FORMAT_D16_UNORM",
		"VK_FORMAT_X8_D24_UNORM_PACK32",
		"VK_FORMAT_D32_SFLOAT",
		"VK_FORMAT_S8_UINT",
		"VK_FORMAT_D16_UNORM_S8_UINT",
		"VK_FORMAT_D24_UNORM_S8_UINT",
		"VK_FORMAT_D32_SFLOAT_S8_UINT",
	};
	return preferredDsFormat[format - VK_FORMAT_D16_UNORM];
}

static inline void getColorBits(VkFormat format, uint32& redBits, uint32& greenBits, uint32& blueBits, uint32& alphaBits)
{
	switch (format)
	{
	case VK_FORMAT_R8G8B8A8_SRGB:
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_R8G8B8A8_SNORM:
	case VK_FORMAT_B8G8R8A8_UNORM:
	case VK_FORMAT_B8G8R8A8_SRGB: redBits = 8; greenBits = 8; blueBits = 8; alphaBits = 8;  break;
	case VK_FORMAT_B8G8R8_SRGB:
	case VK_FORMAT_B8G8R8_UNORM:
	case VK_FORMAT_B8G8R8_SNORM:
	case VK_FORMAT_R8G8B8_SRGB:
	case VK_FORMAT_R8G8B8_UNORM:
	case VK_FORMAT_R8G8B8_SNORM: redBits = 8; greenBits = 8; blueBits = 8; alphaBits = 0;  break;
	case VK_FORMAT_R5G6B5_UNORM_PACK16: redBits = 5; greenBits = 6; blueBits = 5; alphaBits = 0;  break;
	default: assertion(0, "UnSupported Format");
	}
}
static inline void getDepthStencilBits(VkFormat format, uint32& depthBits, uint32& stencilBits)
{
	switch (format)
	{
	case VK_FORMAT_D16_UNORM: depthBits = 16; stencilBits = 0; break;
	case VK_FORMAT_D16_UNORM_S8_UINT: depthBits = 16; stencilBits = 8; break;
	case VK_FORMAT_D24_UNORM_S8_UINT: depthBits = 24; stencilBits = 8; break;
	case VK_FORMAT_D32_SFLOAT: depthBits = 32; stencilBits = 0; break;
	case VK_FORMAT_D32_SFLOAT_S8_UINT: depthBits = 32; stencilBits = 8; break;
	case VK_FORMAT_X8_D24_UNORM_PACK32: depthBits = 24; stencilBits = 0; break;
	case VK_FORMAT_S8_UINT: depthBits = 0; stencilBits = 8; break;
	default: assertion(0, "UnSupported Format");
	}
}
// create the swapchains, displayimages and views
static bool initSwapChain(NativePlatformHandles_ & platformHandle, NativeDisplayHandle_ & displayHandle,
                          bool hasDepth, bool hasStencil,
                          DisplayAttributes& displayAttribs)
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkglue::GetPhysicalDeviceSurfaceCapabilitiesKHR(platformHandle.context.physicalDevice,
	    displayHandle.surface, &surfaceCapabilities);

	Log(Log.Information, "Queried Surface Capabilities:");
	Log(Log.Information, "Min-max swap image count: %u - %u", surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
	Log(Log.Information, "Array size: %u", surfaceCapabilities.maxImageArrayLayers);
	Log(Log.Information, "Image size (now): %dx%d", surfaceCapabilities.currentExtent.width,
	    surfaceCapabilities.currentExtent.height);
	Log(Log.Information, "Image size (extent): %dx%d - %dx%d", surfaceCapabilities.minImageExtent.width,
	    surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.width,
	    surfaceCapabilities.maxImageExtent.height);
	Log(Log.Information, "Usage: %x", surfaceCapabilities.supportedUsageFlags);
	Log(Log.Information, "Current transform: %u", surfaceCapabilities.currentTransform);

#if !defined(ANDROID)
	surfaceCapabilities.currentExtent.width = std::max(surfaceCapabilities.minImageExtent.width,
	    std::min(displayAttribs.width, surfaceCapabilities.maxImageExtent.width));
	surfaceCapabilities.currentExtent.height = std::max(surfaceCapabilities.minImageExtent.height,
	    std::min(displayAttribs.height, surfaceCapabilities.maxImageExtent.height));
#endif
	Log(Log.Information, "Surface Properties after Shell:");

	displayAttribs.width = surfaceCapabilities.currentExtent.width;
	displayAttribs.height = surfaceCapabilities.currentExtent.height;

	Log(Log.Information, "Image size (now): %dx%d", displayAttribs.width,
	    displayAttribs.height);

	uint32_t formatCount = 0;
	vkglue::GetPhysicalDeviceSurfaceFormatsKHR(platformHandle.context.physicalDevice, displayHandle.surface,
	    &formatCount, NULL);
	VkSurfaceFormatKHR tmpformats[16]; std::vector<VkSurfaceFormatKHR> tmpFormatsVector;
	VkSurfaceFormatKHR* allFormats = tmpformats;
	if (formatCount > 16)
	{
		tmpFormatsVector.resize(formatCount);
		allFormats = tmpFormatsVector.data();
	}
	vkglue::GetPhysicalDeviceSurfaceFormatsKHR(platformHandle.context.physicalDevice, displayHandle.surface,
	    &formatCount, allFormats);

	VkSurfaceFormatKHR format = allFormats[0];

	VkFormat preferredColorFormats[] =
	{
		VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_R8G8B8A8_SNORM,
		VK_FORMAT_B8G8R8_SNORM, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_R5G6B5_UNORM_PACK16
	};
	uint32 requestedRedBpp = displayAttribs.redBits;
	uint32 requestedGreenBpp = displayAttribs.greenBits;
	uint32 requestedBlueBpp = displayAttribs.blueBits;
	uint32 requestedAlphaBpp = displayAttribs.alphaBits;
	bool foundFormat = false;
	for (unsigned int i = 0; i < (sizeof(preferredColorFormats) / sizeof(preferredColorFormats[0])) && !foundFormat; ++i)
	{
		for (uint32_t f = 0; f < formatCount; ++f)
		{
			if (allFormats[f].format == preferredColorFormats[i])
			{
				if (displayAttribs.forceColorBPP)
				{
					uint32 currentRedBpp, currentGreenBpp, currentBlueBpp, currentAlphaBpp = 0;
					getColorBits(allFormats[f].format, currentRedBpp, currentGreenBpp, currentBlueBpp, currentAlphaBpp);
					if (currentRedBpp == requestedRedBpp &&
					    requestedGreenBpp == currentGreenBpp &&
					    requestedBlueBpp == currentBlueBpp &&
					    requestedAlphaBpp == currentAlphaBpp)
					{
						format = allFormats[f]; foundFormat = true; break;
					}
				}
				else
				{
					format = allFormats[f]; foundFormat = true; break;
				}
			}
		}
	}
	if (!foundFormat)
	{
		pvr::Log(Logger::Warning, "Unable to find supported preferred color format. Using color format: %s", format);
	}
	bool useDepthStencil = hasDepth || hasStencil;
	VkFormat dsFormatRequested = getDepthStencilFormat(displayAttribs);
	VkFormat supportedDsFormat = VK_FORMAT_UNDEFINED;
	if (useDepthStencil)
	{
		VkFormat preferredDsFormat[] =
		{
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D16_UNORM,
			VK_FORMAT_X8_D24_UNORM_PACK32
		};

		// start by checking for the requested depth stencil format
		VkFormat currentDsFormat = dsFormatRequested;
		for (uint32 f = 0; f < sizeof(preferredDsFormat) / sizeof(preferredDsFormat[0]); ++f)
		{
			VkFormatProperties prop;
			vkglue::GetPhysicalDeviceFormatProperties(platformHandle.context.physicalDevice, currentDsFormat, &prop);
			if (prop.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				supportedDsFormat = currentDsFormat;
				break;
			}
			currentDsFormat = preferredDsFormat[f];
		}

		if (dsFormatRequested != supportedDsFormat)
		{
			pvr::Log(Logger::Severity::Information, "Requested DepthStencil Format %s is not supported. Falling back to %s",
			         depthStencilFormatToStr(dsFormatRequested).c_str(), depthStencilFormatToStr(supportedDsFormat).c_str());
		}
		getDepthStencilBits(supportedDsFormat, displayAttribs.depthBPP, displayAttribs.stencilBPP);
		pvr::Log(Logger::Information, "Surface DepthStencil Format: %s", depthStencilFormatToStr(supportedDsFormat).c_str());
	}

	uint32 numPresentMode;
	vkSuccessOrDie(vkglue::GetPhysicalDeviceSurfacePresentModesKHR(platformHandle.context.physicalDevice,
	               displayHandle.surface, &numPresentMode, NULL), "Failed to get the number of present modes count");
	assertion(numPresentMode > 0);
	std::vector<VkPresentModeKHR> presentModes(numPresentMode);
	vkSuccessOrDie(vkglue::GetPhysicalDeviceSurfacePresentModesKHR(platformHandle.context.physicalDevice,
	               displayHandle.surface, &numPresentMode, &presentModes[0]), "failed to get the present modes");

	// Default is FIFO - Which is typical Vsync.
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	VkPresentModeKHR desiredSwapMode = VK_PRESENT_MODE_FIFO_KHR;
	switch (displayAttribs.vsyncMode)
	{
	case VsyncMode::Off:
		desiredSwapMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		break;
	case VsyncMode::Mailbox:
		desiredSwapMode = VK_PRESENT_MODE_MAILBOX_KHR;
		break;
	case VsyncMode::Relaxed:
		desiredSwapMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
		break;
	}
	for (size_t i = 0; i < numPresentMode; i++)
	{
		if (presentModes[i] == desiredSwapMode)
		{
			//Precise match - Break!
			swapchainPresentMode = desiredSwapMode;
			break;
		}
		//Secondary matches : Immediate and Mailbox are better fits for each other than Fifo, so set them as secondaries
		// If the user asked for Mailbox, and we found Immediate, set it (in case Mailbox is not found) and keep looking
		if ((desiredSwapMode == VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
		{
			swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
		// ... And vice versa: If the user asked for Immediate, and we found Mailbox, set it (in case Immediate is not found) and keep looking
		if ((desiredSwapMode == VK_PRESENT_MODE_IMMEDIATE_KHR) && (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR))
		{
			swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		}
	}
#if defined(X11)
	// use Present mode FIFO which is the only one seems to be supported at the time of implementation.
	pvr::Log(Logger::Warning, "Forcing to VK_PRESENT_MODE_FIFO_KHR for X11");
	swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
#endif
	switch (swapchainPresentMode)
	{
	case VK_PRESENT_MODE_IMMEDIATE_KHR: Log(Logger::Information, "Presentation mode: Immediate (Vsync OFF)"); break;
	case VK_PRESENT_MODE_MAILBOX_KHR: Log(Logger::Information, "Presentation mode: Mailbox (Vsync 'OFF' with anti-tearing)"); break;
	case VK_PRESENT_MODE_FIFO_KHR: Log(Logger::Information, "Presentation mode: FIFO (Vsync ON)"); break;
	case VK_PRESENT_MODE_FIFO_RELAXED_KHR: Log(Logger::Information, "Presentation mode: Relaxed FIFO (Improved Vsync)"); break;
	default: assertion(false, "Unrecognised presentation mode"); break;
	}

	displayHandle.onscreenFbo.colorFormat = format.format;
	displayHandle.displayExtent = surfaceCapabilities.currentExtent;

	//--- create the swap chain
	VkSwapchainCreateInfoKHR swapchainCreate = {};
	swapchainCreate.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreate.clipped = VK_TRUE;
	swapchainCreate.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreate.surface = displayHandle.surface;

	displayAttribs.swapLength = std::max<uint32>(displayAttribs.swapLength, surfaceCapabilities.minImageCount);
	displayAttribs.swapLength = std::min<uint32>(displayAttribs.swapLength, surfaceCapabilities.maxImageCount);
	displayAttribs.swapLength = std::min<uint32>(displayAttribs.swapLength, (uint32)FrameworkCaps::MaxSwapChains);
	swapchainCreate.minImageCount = displayAttribs.swapLength;
	swapchainCreate.imageFormat = displayHandle.onscreenFbo.colorFormat;
	swapchainCreate.imageArrayLayers = 1;
	swapchainCreate.imageColorSpace = format.colorSpace;
	swapchainCreate.imageExtent = surfaceCapabilities.currentExtent;
	swapchainCreate.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapchainCreate.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainCreate.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreate.presentMode = swapchainPresentMode;
	swapchainCreate.queueFamilyIndexCount = 1;
	uint32_t queueFamily = 0;
	swapchainCreate.pQueueFamilyIndices = &queueFamily;

	assertion(swapchainCreate.minImageCount <= (uint32)FrameworkCaps::MaxSwapChains,
	          "Minimum number of swapchain images is larger than Max set");

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

	Log(Log.Information, "Actual swap image count: %u ", displayHandle.swapChainLength);

	assertion(displayHandle.swapChainLength <= (uint32)FrameworkCaps::MaxSwapChains, "Number of swapchain images is larger than Max set");

	displayHandle.onscreenFbo.colorImages.resize(displayHandle.swapChainLength);
	displayHandle.onscreenFbo.colorImageViews.resize(displayHandle.swapChainLength);
	if (!vkIsSuccessful(vkglue::GetSwapchainImagesKHR(platformHandle.context.device, displayHandle.swapChain,
	                    &displayHandle.swapChainLength, &displayHandle.onscreenFbo.colorImages[0]), "Could not get swapchain images"))
	{
		return false;
	}

	assertion(displayHandle.swapChainLength <= (uint32)FrameworkCaps::MaxSwapChains, "Number of swapchain images is larger than Max set");


	//--- create the swapchain view
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = displayHandle.onscreenFbo.colorFormat;
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.layerCount = 1;
	if (useDepthStencil)
	{
		displayHandle.onscreenFbo.depthStencilImage.resize(displayHandle.swapChainLength);
		displayHandle.onscreenFbo.depthStencilImageView.resize(displayHandle.swapChainLength);
	}

	for (uint32 i = 0; i < displayHandle.swapChainLength; ++i)
	{
		viewCreateInfo.image = displayHandle.onscreenFbo.colorImages[i];
		if (!vkIsSuccessful(vkglue::CreateImageView(platformHandle.context.device, &viewCreateInfo, NULL,
		                    &displayHandle.onscreenFbo.colorImageViews[i]), "create display image view"))
		{
			return false;
		}

		if (useDepthStencil)
		{
			// create the depth stencil image
			VkImageCreateInfo dsCreateInfo = {};
			dsCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			dsCreateInfo.format = supportedDsFormat;
			dsCreateInfo.extent.width = displayHandle.displayExtent.width;
			dsCreateInfo.extent.height = displayHandle.displayExtent.height;
			dsCreateInfo.extent.depth = 1;
			dsCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			dsCreateInfo.arrayLayers = 1;
			dsCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			dsCreateInfo.mipLevels = 1;
			dsCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			dsCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			dsCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			dsCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VkResult rslt = vkglue::CreateImage(platformHandle.context.device, &dsCreateInfo, NULL,
			                                    &displayHandle.onscreenFbo.depthStencilImage[i].first);
			vkSuccessOrDie(rslt, "Image creation failed");

			if (!allocateImageDeviceMemory(platformHandle, displayHandle.onscreenFbo.depthStencilImage[i].first,
			                               displayHandle.onscreenFbo.depthStencilImage[i].second, NULL))
			{
				assertion(false, "Memory allocation failed");
			}
			// create the depth stencil view
			VkImageViewCreateInfo dsViewCreateInfo = {};
			dsViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			dsViewCreateInfo.image = displayHandle.onscreenFbo.depthStencilImage[i].first;
			dsViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			dsViewCreateInfo.format = supportedDsFormat;
			dsViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
			dsViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
			dsViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
			dsViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
			dsViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | (hasStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
			dsViewCreateInfo.subresourceRange.levelCount = 1;
			dsViewCreateInfo.subresourceRange.layerCount = 1;

			displayHandle.onscreenFbo.depthStencilFormat = supportedDsFormat;
			vkSuccessOrDie(vkglue::CreateImageView(platformHandle.context.device, &dsViewCreateInfo, NULL,
			                                       &displayHandle.onscreenFbo.depthStencilImageView[i]), "Create Depth stencil image view");
		}
	}
	return true;
}

inline static void setInitialSwapchainLayouts(NativePlatformHandles_ & platformHandle,
    NativeDisplayHandle_ & displayHandle, bool hasDepth, bool hasStencil, pvr::uint32 swapChain)
{
	VkCommandBuffer cmdImgLayoutTrans = allocPrimaryCmdBuffer(platformHandle);
	VkCommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	vkSuccessOrDie(vkglue::BeginCommandBuffer(cmdImgLayoutTrans, &cmdBeginInfo), "Failed to begin commandbuffer");

	bool useDepthStencil = hasDepth || hasStencil;
	for (uint32 i = 0; i < displayHandle.swapChainLength; ++i)
	{
		// prepare the current swapchain image for writing
		if (i == swapChain)
		{
			setImageLayout(cmdImgLayoutTrans, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			               VK_IMAGE_ASPECT_COLOR_BIT, 0, displayHandle.onscreenFbo.colorImages[i]);
		}
		else// set all other swapchains to present so they will be transformed properly later.
		{
			setImageLayout(cmdImgLayoutTrans, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			               VK_IMAGE_ASPECT_COLOR_BIT, 0, displayHandle.onscreenFbo.colorImages[i]);
		}
		if (useDepthStencil)
		{
			setImageLayout(cmdImgLayoutTrans, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			               VK_IMAGE_ASPECT_DEPTH_BIT | (hasStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0),
			               0, displayHandle.onscreenFbo.depthStencilImage[i].first);
		}
	}
	vkglue::EndCommandBuffer(cmdImgLayoutTrans);
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pCommandBuffers = &cmdImgLayoutTrans;
	submitInfo.commandBufferCount = 1;
	submitInfo.pSignalSemaphores = &platformHandle.semaphoreCanBeginRendering[swapChain];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &platformHandle.semaphoreImageAcquired[platformHandle.currentImageAcqSem];
	submitInfo.waitSemaphoreCount = 1;

	VkPipelineStageFlags pipeStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	submitInfo.pWaitDstStageMask = &pipeStageFlags;

	VkFence fence;
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkglue::CreateFence(platformHandle.context.device, &fenceInfo, NULL, &fence);
	vkglue::QueueSubmit(platformHandle.graphicsQueue, 1, &submitInfo, fence);
	vkglue::WaitForFences(platformHandle.context.device, 1, &fence, true, uint64(-1));
	vkglue::DestroyFence(platformHandle.context.device, fence, NULL);
	vkglue::FreeCommandBuffers(platformHandle.context.device, platformHandle.commandPool, 1, &cmdImgLayoutTrans);
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
		if (!vkIsSuccessful(vkglue::CreateSemaphore(device, &semaphoreCreateInfo, NULL, &platformHandle.semaphoreFinishedRendering[i]),
		                    "Cannot create the Semaphore used to signal rendering finished"))
		{
			return false;
		}
		if (!vkIsSuccessful(vkglue::CreateSemaphore(device, &semaphoreCreateInfo, NULL, &platformHandle.semaphoreCanBeginRendering[i]),
		                    "Cannot create the Presentation Semaphore"))
		{
			return false;
		}
		if (!vkIsSuccessful(vkglue::CreateSemaphore(device, &semaphoreCreateInfo, NULL, &platformHandle.semaphoreCanPresent[i]),
		                    "Cannot create the Presentation Semaphore"))
		{
			return false;
		}
		if (!vkIsSuccessful(vkglue::CreateSemaphore(device, &semaphoreCreateInfo, NULL, &platformHandle.semaphoreImageAcquired[i]),
		                    "Cannot create the Swapchain Image Acquisition Semaphore"))
		{
			return false;
		}
		if (!vkIsSuccessful(vkglue::CreateFence(device, &fenceCreateInfo, NULL, &platformHandle.fencePrePresent[i]), "Failed to create fence"))
		{
			return false;
		}
		if (!vkIsSuccessful(vkglue::CreateFence(device, &fenceCreateInfo, NULL, &platformHandle.fenceRender[i]), "Failed to create fence"))
		{
			return false;
		}
		if (!vkIsSuccessful(vkglue::CreateFence(device, &fenceCreateInfo, NULL, &platformHandle.fenceAcquire[i]), "Failed to create fence"))
		{
			return false;
		}
	}

	if (!vkIsSuccessful(vkglue::CreateFence(device, &fenceCreateInfo, NULL, &platformHandle.fencePrePresent[numSwapImages]), "Failed to create fence"))
	{
		return false;
	}
	if (!vkIsSuccessful(vkglue::CreateFence(device, &fenceCreateInfo, NULL, &platformHandle.fenceAcquire[numSwapImages]), "Failed to create fence"))
	{
		return false;
	}
	if (!vkIsSuccessful(vkglue::CreateSemaphore(device, &semaphoreCreateInfo, NULL, &platformHandle.semaphoreImageAcquired[numSwapImages]),
	                    "Cannot create the Swapchain Image Acquisition Semaphore"))
	{
		return false;
	}

	return true;
}

bool initPresentationCommandBuffers(NativePlatformHandles_ & handles, NativeDisplayHandle_ & displayHandle)
{
	// create the commandpool and setup commandbuffer

	VkCommandBufferAllocateInfo cinfo = {};
	cinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cinfo.commandPool = handles.commandPool;
	cinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	cinfo.commandBufferCount = displayHandle.swapChainLength;

	vkglue::AllocateCommandBuffers(handles.context.device, &cinfo, handles.acquireBarrierCommandBuffers);
	vkglue::AllocateCommandBuffers(handles.context.device, &cinfo, handles.presentBarrierCommandBuffers);

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
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		barrier.image = displayHandle.onscreenFbo.colorImages[swapIndex];
		vkglue::BeginCommandBuffer(handles.presentBarrierCommandBuffers[swapIndex], &beginnfo);
		vkglue::CmdPipelineBarrier(handles.presentBarrierCommandBuffers[swapIndex],
		                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, NULL, 0,
		                           NULL, 1, &barrier);
		vkglue::EndCommandBuffer(handles.presentBarrierCommandBuffers[swapIndex]);


		// post present
		barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		barrier.image = displayHandle.onscreenFbo.colorImages[swapIndex];
		vkglue::BeginCommandBuffer(handles.acquireBarrierCommandBuffers[swapIndex], &beginnfo);
		vkglue::CmdPipelineBarrier(handles.acquireBarrierCommandBuffers[swapIndex],
		                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, NULL, 0,
		                           NULL, 1, &barrier);
		vkglue::EndCommandBuffer(handles.acquireBarrierCommandBuffers[swapIndex]);
	}
	return true;
}


Api PlatformContext::getMaxApiVersion()
{
	if (m_maxApiVersion == Api::Unspecified) { populateMaxApiVersion(); }
	return m_maxApiVersion;
}

void PlatformContext::populateMaxApiVersion()
{
	//Originally, only support numExtensionsone Vulkan version
	m_maxApiVersion = Api::Vulkan;
}

bool PlatformContext::isApiSupported(Api apiLevel) { return apiLevel == Api::Vulkan; }

bool PlatformContext::makeCurrent()
{
	//No global context...
	return true;
}

string PlatformContext::getInfo()
{
	return getNativePlatformHandles().platformInfo.deviceName;
}

uint32 PlatformContext::getSwapChainLength() const
{
	return m_displayHandle->swapChainLength;
}


/*This function assumes that the osManager's getDisplay() and getWindow() types are one and the same with NativePlatformHandles::NativeDisplay and NativePlatformHandles::NativeWindow.*/
Result PlatformContext::init()
{
	if (m_initialized) { return Result::AlreadyInitialized; }
	m_preInitialized = true;
	populateMaxApiVersion();
	m_platformContextHandles.reset(new NativePlatformHandles_());
	NativePlatformHandles_& handles = *m_platformContextHandles;

	m_displayHandle.reset(new NativeDisplayHandle_());
	m_displayHandle->nativeDisplay = reinterpret_cast<NativeDisplay>(m_OSManager.getDisplay());
	m_displayHandle->nativeWindow = reinterpret_cast<NativeWindow>(m_OSManager.getWindow());
	if (m_OSManager.getApiTypeRequired() == Api::Unspecified)
	{
		if (m_OSManager.getMinApiTypeRequired() == Api::Unspecified)
		{
			Api version = getMaxApiVersion();
			m_OSManager.setApiTypeRequired(version);
			Log(Log.Information, "Unspecified target API -- Setting to max API level : %s", apiName(version));
		}
		else
		{
			Api version = (std::max)(m_OSManager.getMinApiTypeRequired(), getMaxApiVersion());
			Log(Log.Information, "Requested minimum API level : %s. Will actually create %s since it is supported.",
			    apiName(m_OSManager.getMinApiTypeRequired()), apiName(getMaxApiVersion()));
			m_OSManager.setApiTypeRequired(version);
		}
	}
	else
	{
		Log(Log.Information, "Forcing specific API level: %s", apiName(m_OSManager.getApiTypeRequired()));
	}

	if (m_OSManager.getApiTypeRequired() != Api::Vulkan)
	{
		Log(Log.Error, "API level requested [%s] was not supported. Only Supported API level on this device is [%s]",
		    apiName(m_OSManager.getApiTypeRequired()), Api::Vulkan);
		return Result::UnsupportedRequest;
	}

	bool hasDepth = m_OSManager.getDisplayAttributes().depthBPP > 0;
	bool hasStencil = m_OSManager.getDisplayAttributes().stencilBPP > 0;
	m_displayHandle->onscreenFbo.hasDepthStencil = hasDepth || hasStencil;

	if (!initVkInstanceAndPhysicalDevice(*m_platformContextHandles)) { return Result::UnknownError; }
	if (!initSurface(*m_platformContextHandles, *m_displayHandle)) { return Result::UnknownError; }
	if (!initDevice(*m_platformContextHandles, *m_displayHandle, true)) { return Result::UnknownError; }
	{
		VkCommandPoolCreateInfo pinfo = {};
		pinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		pinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pinfo.queueFamilyIndex = 0;
		vkglue::CreateCommandPool(handles.context.device, &pinfo, NULL, &handles.commandPool);
	}
	if (!initSwapChain(*m_platformContextHandles, *m_displayHandle, hasDepth, hasStencil,
	                   m_OSManager.getDisplayAttributes())) { return Result::UnknownError; }
	if (!initSynchronizationObjects(*m_platformContextHandles, m_displayHandle->swapChainLength)) { return Result::UnknownError; }
	if (!initPresentationCommandBuffers(*m_platformContextHandles, *m_displayHandle)) { return Result::UnknownError; }

	if (!vkIsSuccessful(vkglue::AcquireNextImageKHR(handles.context.device,
	                    m_displayHandle->swapChain, pvr::uint64(-1), handles.semaphoreImageAcquired[handles.currentImageAcqSem],
	                    VK_NULL_HANDLE, &swapIndex), "Failed to acquire initial Swapchain image"))
	{
		return Result::UnknownError;
	}

	lastPresentedSwapIndex = swapIndex;

	setInitialSwapchainLayouts(*m_platformContextHandles, *m_displayHandle, hasDepth, hasStencil, swapIndex);
	vkglue::ResetFences(m_platformContextHandles->context.device, 1, &m_platformContextHandles->fenceRender[swapIndex]);

	m_initialized = true;
	return Result::Success;
}

bool PlatformContext::presentBackbuffer()
{
	///// Ensure that we are over and done with with the image we just acquired!
	//Transition: Rendering done, ready to Present
	if (!prePresentTransition(*m_platformContextHandles, swapIndex, m_platformContextHandles->fencePrePresent[swapIndex]))
	{
		return false;
	}

	////// PRESENT //////
	VkResult result = VK_SUCCESS;
	static VkPresentInfoKHR pInfo = {};
	pInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	pInfo.swapchainCount = 1;
	pInfo.pSwapchains = &m_displayHandle->swapChain;
	pInfo.pImageIndices = &swapIndex;
	pInfo.pWaitSemaphores = &m_platformContextHandles->semaphoreCanPresent[swapIndex];
	pInfo.waitSemaphoreCount = (m_platformContextHandles->semaphoreCanPresent[swapIndex] != 0);
	pInfo.pResults = &result;

	if (!vkIsSuccessful(vkglue::QueuePresentKHR(m_platformContextHandles->graphicsQueue, &pInfo),
	                    "PlatformContext:PresentBackbuffer Present Queue error"))
	{
		assertion("Platform Context: presentBackBuffer failed.");
		return false;
	}
	if (result != VK_SUCCESS)
	{
		pvr::Log("Present back buffer failed");
		return false;
	}

	/////////////// THE REAL FRAME SEPARATOR : ACQUIRE NEXT IMAGE //////////////////////////

	lastPresentedSwapIndex = swapIndex;

	m_platformContextHandles->currentImageAcqSem = (m_platformContextHandles->currentImageAcqSem + 1) %
	    (getSwapChainLength() + 1);

	if (!vkIsSuccessful(vkglue::AcquireNextImageKHR(m_platformContextHandles->context.device,
	                    m_displayHandle->swapChain, pvr::uint64(-1),
	                    m_platformContextHandles->semaphoreImageAcquired[m_platformContextHandles->currentImageAcqSem],
	                    VK_NULL_HANDLE, &swapIndex), "PlatformContext:PresentBackbuffer AcquireNextImage error"))
	{
		return false;
	}

	//Transition: READY TO RENDER
	if (!postAcquireTransition(*m_platformContextHandles, swapIndex,
	                           m_platformContextHandles->fenceAcquire[swapIndex])) { return false; }

	// make sure the fenceRender is avaiable to be used by the commandbuffers of the application
	vkglue::WaitForFences(m_platformContextHandles->context.device, 1,
	                      &m_platformContextHandles->fenceRender[swapIndex], true, uint64(-1));
	vkglue::ResetFences(m_platformContextHandles->context.device, 1,
	                    &m_platformContextHandles->fenceRender[swapIndex]);
	return true;
}


}// namespace platform

}// namespace pvr

namespace pvr {
//Creates an instance of a graphics context
std::auto_ptr<IPlatformContext> createNativePlatformContext(OSManager& mgr)
{
	vkglue::initVulkanGlue();
	//vkglueext::initVulkanGlueExt();
	return std::auto_ptr<IPlatformContext>(new platform::PlatformContext(mgr));
}
}// namespace pvr
//!\endcond
