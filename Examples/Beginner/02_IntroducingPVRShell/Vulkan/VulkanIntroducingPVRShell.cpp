/*!*********************************************************************************************************************
\File         VulkanIntroducingPVRUtils.cpp
\Title        Introducing the PowerVR Framework
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief        Introduction to the PVRShell library. Shows how to get started using the PVRShell library.
***********************************************************************************************************************/

#include "vk_getProcAddrs.h"
#include "PVRCore/PVRCore.h"
#include "PVRShell/PVRShell.h"

#ifdef Success
#undef Success
#endif

/*!*********************************************************************************************************************
Content file names
***********************************************************************************************************************/
const char* VertShaderName = "VertShader_vk.spv";
const char* FragShaderName = "FragShader_vk.spv";


void vulkanSuccessOrDie(VkResult result, const char* msg)
{
	if (result != VK_SUCCESS)
	{
		("%s Vulkan Raised an error", msg);
		exit(0);
	}
}

const uint32_t MaxSwapchains = 4;

struct Buffer
{
	VkBuffer buffer;
	VkDeviceMemory memory;
	Buffer() : buffer(VK_NULL_HANDLE), memory(VK_NULL_HANDLE) {}
};

/*!*********************************************************************************************************************
Class implementing the pvr::Shell functions.
***********************************************************************************************************************/
class VulkanIntroducingPVRShell : public pvr::Shell
{
	struct Vertex
	{
		float x, y, z, w;
	};

	VkInstance _instance;
	VkPhysicalDevice _physicalDevice;
	VkSurfaceKHR _surface;
	VkDevice _device;
	Buffer _vbo;

	struct Swapchain
	{
		VkSwapchainKHR swapchainVk;
		VkExtent2D dimension;
		VkImageView swapchainImages[MaxSwapchains];
		uint32_t numSwapchains;
		VkFormat colorFormat;
		uint32_t swapchainIndex;

		Swapchain() : swapchainVk(VK_NULL_HANDLE)
		{
			memset(swapchainImages, VK_NULL_HANDLE, sizeof(swapchainImages));
		}
	} _swapchain;

	struct Framebuffer
	{
		VkFramebuffer framebufferVk;
		VkImageView depthStencilImages;
		Framebuffer() : framebufferVk(VK_NULL_HANDLE), depthStencilImages(VK_NULL_HANDLE) {}
	};

	VkRenderPass _renderPass;
	Framebuffer _framebuffers[MaxSwapchains];
	VkSemaphore _semaphoresImageAcquire[MaxSwapchains];
	VkSemaphore _semaphoresPresent[MaxSwapchains];
	VkFence _fencesPerFrameAcquire[MaxSwapchains];
	VkFence _fencesCommandBuffer[MaxSwapchains];
	uint32_t _currentFrameIndex;
	VkQueue _queue;
	VkCommandBuffer _commandBuffers[MaxSwapchains];
	VkPipelineLayout _emptyPipelayout;
	VkPipeline _opaquePipeline;
	uint32_t _graphicsQueueFamilyIndex;
	VkCommandPool _commandPool;

public:
	pvr::Result initApplication();
	pvr::Result initView();
	pvr::Result releaseView();
	pvr::Result quitApplication();
	pvr::Result renderFrame();

	void initColorBlendAttachmentState(VkPipelineColorBlendAttachmentState& state);
	bool createFramebufferAndRenderpass();
	bool createSynchronisationPrimitives();
	VkDevice& getDevice() { return _device; }
	bool loadShader(pvr::Stream::ptr_type stream, VkShaderModule& outShader);
	void recordCommandBuffer();
	bool createPipeline(uint32_t width, uint32_t height);
	bool initVbo();
	void setupVertexAttribs(VkVertexInputBindingDescription* bindings, VkVertexInputAttributeDescription* attributes,
	                        VkPipelineVertexInputStateCreateInfo& createInfo);
	bool createPhysicalDevice();
	bool createInstance();
	bool createSurface(void* display, void* window);
	bool createDevice(VkSurfaceKHR _surface);
	bool createSwapchain(VkSurfaceKHR _surface, uint32_t width, uint32_t height);
	bool createBufferAndMemory(VkDeviceSize size, VkBufferUsageFlags usageFlags,
	                           VkMemoryPropertyFlags memFlags, Buffer& outBuffer);
	int32_t getCompatibleQueueFamilies(VkSurfaceKHR _surface);
};

namespace Extensions {
const char* InstanceExtensions[] =
{
	"VK_KHR_surface",
	"VK_KHR_display",
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	"VK_KHR_win32_surface",
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	"VK_KHR_android_surface",
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	"VK_KHR_xlib_surface",
#elif  defined(VK_USE_PLATFORM_WAYLAND_KHR)
	"VK_KHR_wayland_surface",
#endif
	"VK_KHR_get_physical_device_properties2"
};

const char* DeviceExtensions[] =
{
	"VK_KHR_swapchain",
};
}

namespace Layers {
const char* InstanceLayers[] =
{
#ifdef DEBUG
	"VK_LAYER_LUNARG_standard_validation"
#else
	""
#endif
};
}

/*!*********************************************************************************************************************
\brief  Filters the extensions to be used in this demo
***********************************************************************************************************************/
std::vector<std::string> filterExtensions(const std::vector<VkExtensionProperties>& extensionProperties,
    const char** extensionsToEnable, uint32_t numExtensions)
{
	std::vector<std::string> outExtensions;
	for (uint32_t i = 0; i < extensionProperties.size(); ++i)
	{
		for (uint32_t j = 0; j < numExtensions; ++j)
		{
			if (!strcmp(extensionsToEnable[j], extensionProperties[i].extensionName))
			{
				outExtensions.push_back(extensionsToEnable[j]);
				break;
			}
		}
	}
	return outExtensions;
}

/*!*********************************************************************************************************************
\brief  Filters the layers to be used in this demo
***********************************************************************************************************************/
std::vector<std::string> filterLayers(const std::vector<VkLayerProperties>& layerProperties, const char** layersToEnable,
                                      uint32_t layersCount)
{
	std::vector<std::string> outLayers;
	for (uint32_t i = 0; i < layerProperties.size(); ++i)
	{
		for (uint32_t j = 0; j < layersCount; ++j)
		{
			if (!strcmp(layersToEnable[j], layerProperties[i].layerName))
			{
				outLayers.push_back(layersToEnable[j]);
			}
		}
	}
	return outLayers;
}

/*!*********************************************************************************************************************
\brief  Create our Vulkan application instance
***********************************************************************************************************************/
bool VulkanIntroducingPVRShell::createInstance()
{
	vk::initVulkan();
	VkApplicationInfo appInfo = {};
	appInfo.pApplicationName = "VulkanIntroducingPVRShell";
	appInfo.applicationVersion = 1;
	appInfo.engineVersion = 1;
	appInfo.pEngineName = "VulkanIntroducingPVRShell";
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	// Here we create our Instance Info and assign our app info to it
	// along with our _instance layers and extensions.
	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;

	uint32_t numExtensions = 0;
	vk::EnumerateInstanceExtensionProperties(nullptr, &numExtensions, nullptr);
	std::vector<VkExtensionProperties> extensiosProps(numExtensions);
	vk::EnumerateInstanceExtensionProperties(nullptr, &numExtensions, extensiosProps.data());

	std::vector<std::string> enabledExtensionNames = filterExtensions(extensiosProps, Extensions::InstanceExtensions, ARRAY_SIZE(Extensions::InstanceExtensions));

	uint32_t numLayers = 0;
	vk::EnumerateInstanceLayerProperties(&numLayers, nullptr);
	std::vector<VkLayerProperties> layerProps(numLayers);
	vk::EnumerateInstanceLayerProperties(&numLayers, layerProps.data());

	std::vector<std::string> enabledLayerNames = filterLayers(layerProps, Layers::InstanceLayers, ARRAY_SIZE(Layers::InstanceLayers));

	std::vector<const char*> enabledExtensions;
	std::vector<const char*> enableLayers;

	enabledExtensions.resize(enabledExtensionNames.size());
	for (uint32_t i = 0; i < enabledExtensionNames.size(); ++i)
	{
		enabledExtensions[i] = enabledExtensionNames[i].c_str();
	}

	instanceInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
	instanceInfo.ppEnabledExtensionNames = enabledExtensions.data();

	enableLayers.resize(enabledLayerNames.size());
	for (uint32_t i = 0; i < enabledLayerNames.size(); ++i)
	{
		enableLayers[i] = enabledLayerNames[i].c_str();
	}

	instanceInfo.enabledLayerCount = static_cast<uint32_t>(enableLayers.size());
	instanceInfo.ppEnabledLayerNames = enableLayers.data();

	// Create our Vulkan Application Instance.
	if (vk::CreateInstance(&instanceInfo, nullptr, &_instance) != VK_SUCCESS)
	{
		return false;
	}
	// Initialize the Function pointers that require the Instance address
	return vk::initVulkanInstance(_instance);
}

/*!*********************************************************************************************************************
\brief  Find a physical device and create a Vulkan handle for it.
***********************************************************************************************************************/
bool VulkanIntroducingPVRShell::createPhysicalDevice()
{
	uint32_t gpuCount;

	// we query for the number of GPUs available.
	VkResult result = vk::EnumeratePhysicalDevices(_instance, &gpuCount, nullptr);
	if (gpuCount == 0)
	{
		return false;
	}
	vk::EnumeratePhysicalDevices(_instance, &gpuCount, &_physicalDevice);
	return true;
}

/*!*********************************************************************************************************************
\brief  Create the surface we'll present on. Differs based on platform.
***********************************************************************************************************************/
bool VulkanIntroducingPVRShell::createSurface(void* display, void* window)
{
	VkResult result = VK_SUCCESS;
#ifdef VK_USE_PLATFORM_WIN32_KHR
	VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.hinstance = (HINSTANCE)GetModuleHandle(NULL);
	surfaceInfo.hwnd = (HWND)window;
	surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	// we create the _surface we'll be rendering on.
	result = vk::CreateWin32SurfaceKHR(_instance, &surfaceInfo, nullptr, &_surface);
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
	VkXlibSurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.dpy = (Display*)display;
	surfaceInfo.window = (Window)window;
	// we create the _surface we'll be rendering on.
	result = vk::CreateXlibSurfaceKHR(_instance, &surfaceInfo, nullptr, &_surface);
#endif

#ifdef VK_USE_PLATFORM_ANDROID_KHR
	VkAndroidSurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.window = (ANativeWindow*)window;
	result = vk::CreateAndroidSurfaceKHR(_instance, &surfaceInfo, nullptr, &_surface);
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
	VkWaylandSurfaceCreateInfoKHR surface_info = {};
	surface_info.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
	surface_info.display = (wl_display*)display;
	surface_info.surface = (wl_surface*)window;
	result = vk::CreateWaylandSurfaceKHR(_instance, &surface_info, NULL, &_surface);
#endif

#ifdef VK_USE_PLATFORM_NULLWS
	VkDisplayPropertiesKHR properties;
	uint32_t propertiesCount = 1;
	if (vk::GetPhysicalDeviceDisplayPropertiesKHR)
	{
		vk::GetPhysicalDeviceDisplayPropertiesKHR(_physicalDevice, &propertiesCount, &properties);
	}

	VkDisplayKHR nativeDisplay = properties.display;

	uint32_t modeCount = 0;
	vk::GetDisplayModePropertiesKHR(_physicalDevice, nativeDisplay, &modeCount, NULL);
	std::vector<VkDisplayModePropertiesKHR> modeProperties;
	modeProperties.resize(modeCount);
	vk::GetDisplayModePropertiesKHR(_physicalDevice, nativeDisplay, &modeCount, modeProperties.data());

	VkDisplaySurfaceCreateInfoKHR surface_info = {};

	surface_info.sType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR;
	surface_info.pNext = NULL;

	surface_info.displayMode = modeProperties[0].displayMode;
	surface_info.planeIndex = 0;
	surface_info.planeStackIndex = 0;
	surface_info.transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	surface_info.globalAlpha = 0.0f;
	surface_info.alphaMode = VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR;
	surface_info.imageExtent = modeProperties[0].parameters.visibleRegion;

	result = vk::CreateDisplayPlaneSurfaceKHR(_instance, &surface_info, NULL, &_surface);
#endif

	return result == VK_SUCCESS;
}

/*!*********************************************************************************************************************
\brief  get the compatible queue families from the device we selected.
***********************************************************************************************************************/
int32_t VulkanIntroducingPVRShell::getCompatibleQueueFamilies(VkSurfaceKHR _surface)
{
	// Check which _queue Family supports both graphics and presentation for the given _surface
	uint32_t queueFamilyCount;
	// Get the count of _queue Families the physical device supports.
	vk::GetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyProps(queueFamilyCount);
	// Load the _queue families data from the phydevice to the list.
	vk::GetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, queueFamilyProps.data());

	VkBool32 supportPresent;
	int32_t i = 0;
	for (; i < (int32_t)queueFamilyCount; ++i)
	{
		vk::GetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, _surface, &supportPresent);
		if (queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && supportPresent)
		{
			break;
		}
	}
	if (i == queueFamilyCount)
	{
		return -1;
	}
	return i;
}

/*!*********************************************************************************************************************
\brief  Create the logical device.
***********************************************************************************************************************/
bool VulkanIntroducingPVRShell::createDevice(VkSurfaceKHR _surface)
{
	// Check which _queue family supports both Graphics and presentation
	const int32_t queueFamiltIndex = getCompatibleQueueFamilies(_surface);
	if (queueFamiltIndex == -1) { return false; }
	_graphicsQueueFamilyIndex = queueFamiltIndex;
	// This is a priority for _queue (it ranges from 0 - 1) in this case we only have one so it doesn't matter.
	float queue_priorities[1] = { 1.0f };

	// Lets set up the device _queue information.
	VkDeviceQueueCreateInfo queueInfo = {};
	queueInfo.queueFamilyIndex = _graphicsQueueFamilyIndex;
	queueInfo.pQueuePriorities = queue_priorities;
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.queueCount = 1;

	// Our actual logical device information.
	VkDeviceCreateInfo deviceInfo = {};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.enabledLayerCount = 0;
	deviceInfo.ppEnabledLayerNames = nullptr;
	deviceInfo.enabledExtensionCount = ARRAY_SIZE(Extensions::DeviceExtensions);
	deviceInfo.ppEnabledExtensionNames = Extensions::DeviceExtensions;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueInfo;

	VkPhysicalDeviceFeatures deviceFeatures;
	vk::GetPhysicalDeviceFeatures(_physicalDevice, &deviceFeatures);
	deviceFeatures.robustBufferAccess = false;
	deviceInfo.pEnabledFeatures = &deviceFeatures;
	// Create the logical device.
	if (vk::CreateDevice(_physicalDevice, &deviceInfo, nullptr, &_device) != VK_SUCCESS)
	{
		return false;
	}
	// Initialize the Function pointers that require the Device address
	vk::initVulkanDevice(_device);

	// Get our queue
	vk::GetDeviceQueue(_device, queueInfo.queueFamilyIndex, 0, &_queue);
	return true;
}

/*!*********************************************************************************************************************
\brief  Get the correct screen extents.
***********************************************************************************************************************/
VkExtent2D getCorrectExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, uint32_t width, uint32_t height)
{
	// The width and height of the Swapchain are either both 0xFFFFFFFF (Max value for uint_32t)
	// or they are both NOT 0xFFFFFFFF
	if (surfaceCapabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
	{
		VkExtent2D currentExtent = {width , height};
		// The _swapchain extent width and height cannot be less then the minimum _surface capability
		// Also The _swapchain extent width and height cannot be greater then the maximum _surface capability
		if (width < surfaceCapabilities.minImageExtent.width)
		{
			currentExtent.width = surfaceCapabilities.minImageExtent.width;
		}
		else if (width > surfaceCapabilities.maxImageExtent.width)
		{
			currentExtent.width = surfaceCapabilities.maxImageExtent.width;
		}

		if (height < surfaceCapabilities.minImageExtent.height)
		{
			currentExtent.height = surfaceCapabilities.minImageExtent.height;
		}
		else if (height > surfaceCapabilities.maxImageExtent.height)
		{
			currentExtent.height = surfaceCapabilities.maxImageExtent.height;
		}
		return currentExtent;
	}
	else
	{
		return surfaceCapabilities.currentExtent;
	}
}

/*!*********************************************************************************************************************
\brief  Select the present mode to be used with the swapchain and surface.
***********************************************************************************************************************/
VkPresentModeKHR selectPresentMode(std::vector<VkPresentModeKHR>& modes)
{
	const VkPresentModeKHR preferredPresentMode[] =
	{
		VK_PRESENT_MODE_FIFO_KHR,
		VK_PRESENT_MODE_MAILBOX_KHR,
		VK_PRESENT_MODE_IMMEDIATE_KHR,
	};

	for (uint32_t i = 0; i < ARRAY_SIZE(preferredPresentMode); ++i)
	{
		for (uint32_t j = 0; j < modes.size(); ++j)
		{
			if (preferredPresentMode[i] == modes[j]) { return preferredPresentMode[i];}
		}
	}
	return VK_PRESENT_MODE_MAX_ENUM_KHR;
}

/*!*********************************************************************************************************************
\brief  Creates swapchain to present images on the surface.
***********************************************************************************************************************/
bool VulkanIntroducingPVRShell::createSwapchain(VkSurfaceKHR _surface, uint32_t width, uint32_t height)
{
	uint32_t formatCount;

	// we get the count of the formats.
	VkResult result = vk::GetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount, nullptr);
	// We get the format count.
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	result = vk::GetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount, formats.data());

	VkSurfaceFormatKHR surfaceFormat;

	// if the first format is undefined we pick a default one, else we go with the first one.
	if (formatCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
	{
		surfaceFormat = formats[0];
	}

	_swapchain.colorFormat = surfaceFormat.format;// This Format is always supported.
	// We get the _surface capabilities from the _surface and physical device.
	VkSurfaceCapabilitiesKHR surfaceCaps;
	if (vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &surfaceCaps) != VK_SUCCESS)
	{
		return false;
	}

	uint32_t numPresentMode;

	// Get the present mode
	result = vk::GetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &numPresentMode, nullptr);
	if (result != VK_SUCCESS || numPresentMode == 0)
	{
		return false;
	}
	std::vector<VkPresentModeKHR> presentModes(numPresentMode);
	result = vk::GetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &numPresentMode, presentModes.data());
	if (result != VK_SUCCESS)
	{
		return false;
	}

	VkPresentModeKHR presentMode = selectPresentMode(presentModes);
	if (presentMode == VK_PRESENT_MODE_MAX_ENUM_KHR)
	{
		return false;
	}

	// Get the correct extent (size) of the _surface
	_swapchain.dimension = getCorrectExtent(surfaceCaps, width, height);

	uint32_t surfaceImageCount = 2;

	// we create the _swapchain info to create our _swapchain
	VkSwapchainCreateInfoKHR swapChainInfo = {};
	swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainInfo.surface = _surface;
	swapChainInfo.imageFormat = _swapchain.colorFormat;
	swapChainInfo.preTransform = surfaceCaps.currentTransform;
	swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainInfo.presentMode = presentMode;
	swapChainInfo.minImageCount = surfaceImageCount;
	swapChainInfo.clipped = VK_TRUE;
	swapChainInfo.imageExtent.width = _swapchain.dimension.width;
	swapChainInfo.imageExtent.height = _swapchain.dimension.height;
	swapChainInfo.imageArrayLayers = 1;
	swapChainInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapChainInfo.queueFamilyIndexCount = 1;
	swapChainInfo.pQueueFamilyIndices = &_graphicsQueueFamilyIndex;

	// here we check if the present _queue and the graphic _queue are the same
	// if not we go for a sharing mode.

	// we create the _swapchain proper here.
	if (vk::CreateSwapchainKHR(_device, &swapChainInfo, nullptr, &_swapchain.swapchainVk) != VK_SUCCESS)
	{
		return false;
	}

	// Create the image view
	VkImage swapchainImage[4] = {};
	vk::GetSwapchainImagesKHR(_device, _swapchain.swapchainVk, &_swapchain.numSwapchains, nullptr);

	if (_swapchain.numSwapchains == 0 || _swapchain.numSwapchains > 4)
	{
		return false;
	}

	vk::GetSwapchainImagesKHR(_device, _swapchain.swapchainVk, &_swapchain.numSwapchains, swapchainImage);

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.format = _swapchain.colorFormat;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.components = VkComponentMapping
	{
		VK_COMPONENT_SWIZZLE_R,
		VK_COMPONENT_SWIZZLE_G,
		VK_COMPONENT_SWIZZLE_B,
		VK_COMPONENT_SWIZZLE_A,
	};

	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	for (uint32_t i = 0; i < _swapchain.numSwapchains; ++i)
	{
		viewInfo.image = swapchainImage[i];
		if (vk::CreateImageView(_device, &viewInfo, nullptr, &_swapchain.swapchainImages[i]) != VK_SUCCESS)
		{
			return false;
		}
	}
	return true;
}

/*!*********************************************************************************************************************
\brief  Extrapolate the memory type from the device properties.
***********************************************************************************************************************/
bool memory_type_from_properties(const VkPhysicalDeviceMemoryProperties& memProperties,
                                 uint32_t typeBits, VkMemoryPropertyFlags requirements_mask, uint32_t* typeIndex)
{
	// Search memtypes to find first index with those properties
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			// Type is available, does it match user properties?
			if ((memProperties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask)
			{
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	// No memory types matched, return failure
	return false;
}

/*!*********************************************************************************************************************
\brief  Creates Buffer and Memory for the vertex buffer used in this demo.
***********************************************************************************************************************/
bool VulkanIntroducingPVRShell::createBufferAndMemory(VkDeviceSize size, VkBufferUsageFlags usageFlags,
    VkMemoryPropertyFlags memFlags, Buffer& outBuffer)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pQueueFamilyIndices = &_graphicsQueueFamilyIndex;
	bufferInfo.queueFamilyIndexCount = 1;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.size = size;
	bufferInfo.usage = usageFlags;
	Buffer buffer;
	if (vk::CreateBuffer(_device, &bufferInfo, nullptr, &buffer.buffer) != VK_SUCCESS)
	{
		return false;
	}

	// Allocate memory
	VkMemoryRequirements memReq;
	VkMemoryAllocateInfo memAllocateInfo = {};
	vk::GetBufferMemoryRequirements(_device, buffer.buffer, &memReq);
	VkPhysicalDeviceMemoryProperties memProps;
	vk::GetPhysicalDeviceMemoryProperties(_physicalDevice, &memProps);
	memory_type_from_properties(memProps, memReq.memoryTypeBits, memFlags, &memAllocateInfo.memoryTypeIndex);
	memAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocateInfo.allocationSize = memReq.size;

	if (vk::AllocateMemory(_device, &memAllocateInfo, nullptr, &buffer.memory) != VK_SUCCESS)
	{
		return false;
	}

	if (vk::BindBufferMemory(_device, buffer.buffer, buffer.memory, 0) != VK_SUCCESS)
	{
		return false;
	}

	outBuffer = buffer;
	return true;
}


/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Code in initApplication() will be called by Shell once per run, before the rendering context is created.
Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.). If the rendering
context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRShell::initApplication()
{
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.
If the rendering context is lost, quitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRShell::quitApplication()
{
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Code in initView() will be called by Shell upon initialization or after a change  in the rendering context.
Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRShell::initView()
{
	_currentFrameIndex = 0;
	_instance = VK_NULL_HANDLE;
	_physicalDevice = VK_NULL_HANDLE;
	_surface = VK_NULL_HANDLE;
	_device = VK_NULL_HANDLE;
	_renderPass  = VK_NULL_HANDLE;
	_queue = VK_NULL_HANDLE;
	memset(_commandBuffers, 0, sizeof(_commandBuffers));
	_emptyPipelayout = VK_NULL_HANDLE;
	_opaquePipeline = VK_NULL_HANDLE;
	_commandPool = VK_NULL_HANDLE;

	if (!createInstance())
	{
		return pvr::Result::UnknownError;
	}

	if (!createPhysicalDevice())
	{
		return pvr::Result::UnknownError;
	}

	if (!createSurface(getDisplay(), getWindow()))
	{
		return pvr::Result::UnknownError;
	}

	if (!createDevice(_surface))
	{
		return pvr::Result::UnknownError;
	}

	if (!createSwapchain(_surface, getWidth(), getHeight()))
	{
		return pvr::Result::UnknownError;
	}

	if (!createFramebufferAndRenderpass())
	{
		return pvr::Result::UnknownError;
	}

	if (!createSynchronisationPrimitives())
	{
		return pvr::Result::UnknownError;
	}

	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCreateInfo.queueFamilyIndex = _graphicsQueueFamilyIndex;
	vk::CreateCommandPool(_device, &commandPoolCreateInfo, nullptr, &_commandPool);

	if (!createPipeline(_swapchain.dimension.width, _swapchain.dimension.height))
	{
		return pvr::Result::UnknownError;
	}

	if (!initVbo())
	{
		return pvr::Result::UnknownError;
	}
	recordCommandBuffer();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRShell::releaseView()
{
	// wait for the device to finsish with the resources
	vk::DeviceWaitIdle(_device);

	// Release the resources
	vk::DestroyCommandPool(_device, _commandPool, nullptr);

	for (uint32_t i = 0; i < _swapchain.numSwapchains; ++i)
	{
		vk::DestroySemaphore(_device, _semaphoresImageAcquire[i], nullptr);
		vk::DestroySemaphore(_device, _semaphoresPresent[i], nullptr);

		vk::WaitForFences(_device, 1, &_fencesPerFrameAcquire[i], true, uint64_t(-1));
		vk::ResetFences(_device, 1, &_fencesPerFrameAcquire[i]);
		vk::WaitForFences(_device, 1, &_fencesCommandBuffer[i], true, uint64_t(-1));
		vk::ResetFences(_device, 1, &_fencesCommandBuffer[i]);

		vk::DestroyFence(_device, _fencesPerFrameAcquire[i], nullptr);
		vk::DestroyFence(_device, _fencesCommandBuffer[i], nullptr);

		_semaphoresImageAcquire[i] = VK_NULL_HANDLE;
		_semaphoresPresent[i] = VK_NULL_HANDLE;
		_fencesPerFrameAcquire[i] = VK_NULL_HANDLE;
		_fencesCommandBuffer[i] = VK_NULL_HANDLE;

		vk::DestroyImageView(_device, _swapchain.swapchainImages[i], nullptr);

		vk::DestroyFramebuffer(_device, _framebuffers[i].framebufferVk, nullptr);
		vk::DestroyImageView(_device, _framebuffers[i].depthStencilImages, nullptr);
	}
	vk::DestroyRenderPass(getDevice(), _renderPass, nullptr);
	vk::DestroyPipelineLayout(getDevice(), _emptyPipelayout, nullptr);
	vk::DestroyPipeline(getDevice(), _opaquePipeline, nullptr);
	vk::DestroyBuffer(getDevice(), _vbo.buffer, nullptr);
	vk::FreeMemory(getDevice(), _vbo.memory, nullptr);
	vk::DestroySwapchainKHR(_device, _swapchain.swapchainVk, nullptr);
	vk::DestroySurfaceKHR(_instance, _surface, nullptr);
	vk::DestroyDevice(_device, nullptr);
	vk::DestroyInstance(_instance, nullptr);
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every _frame.
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRShell::renderFrame()
{
	// This example uses 3 virtual frames. The virtual frameid will be incremented on each frame and then wrapped.
	// At each call, we use different semaphores and  fence to synchronise the _queue submission between GPU-GPU and
	// GPU-CPU.

	// Wait for the virtual frame fence to make sure that commandbuffers and the swapchain are free from previous use.
	// This waits for the previous frame submission(2 frame earlier hence we use 3 virtual frames.)
	vk::WaitForFences(_device, 1, &_fencesPerFrameAcquire[_currentFrameIndex], true, uint64_t(-1));
	vk::ResetFences(_device, 1, &_fencesPerFrameAcquire[_currentFrameIndex]);

	// The semaphore which will get signaled by acquire swapchain and the queue submit will wait on
	VkSemaphore* semAcquire = &_semaphoresImageAcquire[_currentFrameIndex];

	VkFence* fenceImageAcquired = &_fencesPerFrameAcquire[_currentFrameIndex];

	// The semaphore which will get signaled by queue submit and the presentation will wait on.
	VkSemaphore* semPresent = &_semaphoresPresent[_currentFrameIndex];

	// Get the index of the next available _swapchain image:
	// NOTE: The Presentation Engine might still be using the image so the following queue submit
	// MUST wait on the semaphore which gets signaled when the presentation engine done with it.
	vk::AcquireNextImageKHR(_device, _swapchain.swapchainVk, uint64_t(-1), *semAcquire, *fenceImageAcquired, &_swapchain.swapchainIndex);

	// wait for the command buffer from swapChainLength frames ago to be completed
	vk::WaitForFences(_device, 1, &_fencesCommandBuffer[_swapchain.swapchainIndex], true, uint64_t(-1));
	vk::ResetFences(_device, 1, &_fencesCommandBuffer[_swapchain.swapchainIndex]);

	// SUBMIT
	VkSubmitInfo submitInfo = {};
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pCommandBuffers = &_commandBuffers[_swapchain.swapchainIndex];
	submitInfo.pWaitSemaphores = semAcquire;// wait for the image acuire to finish
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = semPresent;// signal submit
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1;
	vk::QueueSubmit(_queue, 1, &submitInfo, _fencesCommandBuffer[_swapchain.swapchainIndex]);

	// PRESENT
	VkPresentInfoKHR present = {};
	VkResult result;
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present.pImageIndices = &_swapchain.swapchainIndex;
	present.swapchainCount = 1;
	present.pSwapchains = &_swapchain.swapchainVk;
	present.pWaitSemaphores = semPresent;// wait for the queue submit to finish
	present.waitSemaphoreCount = 1;
	present.pResults = &result;
	vk::QueuePresentKHR(_queue, &present);
	if (result != VK_SUCCESS)
	{
		return pvr::Result::UnknownError;
	}
	_currentFrameIndex = (_currentFrameIndex + 1) % _swapchain.numSwapchains;
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  Load the shader files and create shader modules.
***********************************************************************************************************************/
bool VulkanIntroducingPVRShell::loadShader(pvr::Stream::ptr_type stream, VkShaderModule& outShader)
{
	assertion(stream.get() != nullptr && "Invalid Shader source");
	VkShaderModuleCreateInfo shaderInfo = {};
	shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	std::vector<uint32_t> readData(stream->getSize());
	size_t read;
	stream->read(stream->getSize(), 1, readData.data(), read);
	shaderInfo.codeSize = stream->getSize();
	shaderInfo.pCode = readData.data();
	vulkanSuccessOrDie(vk::CreateShaderModule(getDevice(), &shaderInfo, nullptr, &outShader),
	                   "Failed to create the shader");
	return true;
}

/*!*********************************************************************************************************************
\brief  Pre-record the rendering commands.
***********************************************************************************************************************/
void VulkanIntroducingPVRShell::recordCommandBuffer()
{
	VkCommandBufferAllocateInfo sAllocateInfo = {};
	sAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	sAllocateInfo.commandPool = _commandPool;
	sAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	sAllocateInfo.commandBufferCount = 1;

	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkClearValue clearVals[2] = { 0 };
	clearVals[0].color.float32[0] = 0.00f;
	clearVals[0].color.float32[1] = 0.70f;
	clearVals[0].color.float32[2] = .67f;
	clearVals[0].color.float32[3] = 1.0f;
	clearVals[1].depthStencil.depth = 1.0f;
	clearVals[1].depthStencil.stencil = 0xFF;

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = _renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent = _swapchain.dimension;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = &clearVals[0];
	for (uint32_t i = 0; i < _swapchain.numSwapchains; ++i)
	{
		vk::AllocateCommandBuffers(getDevice(), &sAllocateInfo, &_commandBuffers[i]);
		VkCommandBuffer command = _commandBuffers[i];
		vk::BeginCommandBuffer(command, &commandBufferBeginInfo);

		renderPassBeginInfo.framebuffer = _framebuffers[i].framebufferVk;
		vk::CmdBeginRenderPass(command, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vk::CmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, _opaquePipeline);
		VkDeviceSize vertexOffset = 0;
		vk::CmdBindVertexBuffers(command, 0, 1, &_vbo.buffer, &vertexOffset);
		vk::CmdDraw(command, 3, 1, 0, 0);
		vk::CmdEndRenderPass(command);
		vk::EndCommandBuffer(command);
	}
}

/*!*********************************************************************************************************************
\brief  Creates the graphics pipeline used in the demo.
***********************************************************************************************************************/
bool VulkanIntroducingPVRShell::createPipeline(uint32_t width, uint32_t height)
{
	// This is simple pipeline which takes one vertex attribute(position) and writes in to one color attachment
	// and depth stencil attachment. Multisampling and tesselation are disabled.

	VkGraphicsPipelineCreateInfo vkPipeInfo = {};
	VkPipelineShaderStageCreateInfo shaderStages[2] = {};
	VkPipelineColorBlendStateCreateInfo cb = {};
	VkPipelineInputAssemblyStateCreateInfo ia = {};
	VkPipelineDepthStencilStateCreateInfo ds = {};
	VkPipelineVertexInputStateCreateInfo vi = {};
	VkPipelineViewportStateCreateInfo vp = {};
	VkPipelineMultisampleStateCreateInfo ms = {};
	VkPipelineRasterizationStateCreateInfo rs = {};

	// reset:
	{
		cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;

		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

		ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		ia.primitiveRestartEnable = VK_FALSE;

		vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vi.vertexBindingDescriptionCount = 0;
		vi.vertexAttributeDescriptionCount = 0;

		cb.attachmentCount = 1;

		rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rs.cullMode = VK_CULL_MODE_BACK_BIT;
		rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rs.polygonMode = VK_POLYGON_MODE_FILL;
		rs.lineWidth = 1.0;

		ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		ms.minSampleShading = 0.0f;

		// DISABLE DEPTH STATE
		ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		ds.depthTestEnable = VK_FALSE;
		ds.depthWriteEnable = VK_FALSE;

		vkPipeInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		vkPipeInfo.pColorBlendState = &cb;
		vkPipeInfo.pDepthStencilState = &ds;
		vkPipeInfo.pInputAssemblyState = &ia;
		vkPipeInfo.pMultisampleState = &ms;
		vkPipeInfo.pRasterizationState = &rs;
		vkPipeInfo.pVertexInputState = &vi;
		vkPipeInfo.pViewportState = &vp;
		vkPipeInfo.pStages = shaderStages;
		vkPipeInfo.stageCount = 2;
	}
	// These arrays is pointed to by the vertexInput create struct:
	VkVertexInputAttributeDescription attributes[16];
	VkVertexInputBindingDescription bindings[16];

	vi.pVertexAttributeDescriptions = attributes;
	vi.pVertexBindingDescriptions = bindings;

	// This array is pointed to by the cb create struct
	VkPipelineColorBlendAttachmentState attachments[1];

	cb.pAttachments = attachments;

	// CreateInfos for the SetLayouts and PipelineLayouts
	VkPipelineLayoutCreateInfo sPipelineLayoutCreateInfo = {};
	sPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	vk::CreatePipelineLayout(getDevice(), &sPipelineLayoutCreateInfo, nullptr, &_emptyPipelayout);

	const VkSampleMask sampleMask = 0xffffffff;
	ms.pSampleMask = &sampleMask;
	initColorBlendAttachmentState(attachments[0]);
	setupVertexAttribs(bindings, attributes, vi);

	VkRect2D scissors;

	scissors.offset.x = 0;
	scissors.offset.y = 0;
	scissors.extent = { width, height};

	vp.pScissors = &scissors;

	VkViewport viewports;
	viewports.minDepth = 0.0f;
	viewports.maxDepth = 1.0f;
	viewports.x = 0;
	viewports.y = 0;
	viewports.width = static_cast<float>(width);
	viewports.height = static_cast<float>(height);

	vp.pViewports = &viewports;
	vp.viewportCount = 1;
	vp.scissorCount = 1;
	// These are only required to create the graphics pipeline, so we create and destroy them locally
	VkShaderModule vertexShaderModule; loadShader(getAssetStream(VertShaderName), vertexShaderModule);
	VkShaderModule fragmentShaderModule; loadShader(getAssetStream(FragShaderName), fragmentShaderModule);

	vkPipeInfo.layout = _emptyPipelayout;
	vkPipeInfo.renderPass = _renderPass;
	vkPipeInfo.subpass = 0;
	shaderStages[0].module = vertexShaderModule;
	shaderStages[0].pName = "main";
	shaderStages[1].module = fragmentShaderModule;
	shaderStages[1].pName = "main";
	attachments[0].blendEnable = VK_FALSE;
	VkResult result = vk::CreateGraphicsPipelines(getDevice(), VK_NULL_HANDLE, 1,
	                  &vkPipeInfo, nullptr, &_opaquePipeline);

	// Destroy the shader modules, don't need them.
	vk::DestroyShaderModule(getDevice(), vertexShaderModule, nullptr);
	vk::DestroyShaderModule(getDevice(), fragmentShaderModule, nullptr);
	return result == VK_SUCCESS;
}

/*!*********************************************************************************************************************
\brief  Initializes the vertex buffer objects used in the demo.
***********************************************************************************************************************/
bool VulkanIntroducingPVRShell::initVbo()
{
	const uint32_t vboSize  = sizeof(float) * 4 * 3;
	// Create a vertex buffer with memory backing from host visible pool so that
	// we can map and unmap.
	if (!createBufferAndMemory(vboSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, _vbo))
	{
		return false;
	}

	Vertex* ptr = 0;

	vk::MapMemory(_device, _vbo.memory, 0, vboSize, 0, (void**)&ptr);

	// triangle
	ptr->x = -.4; ptr->y = .4; ptr->z = 0; ptr->w = 1;// bottom left
	ptr++;

	ptr->x = .4; ptr->y = .4; ptr->z = 0; ptr->w = 1; // bottom right
	ptr++;

	ptr->x = 0; ptr->y = -.4; ptr->z = 0; ptr->w = 1;// center top
	ptr++;
	vk::UnmapMemory(_device, _vbo.memory);
	return true;
}

/*!*********************************************************************************************************************
\brief  Set up the vertex attributes to be added used by the pipeline.
***********************************************************************************************************************/
void VulkanIntroducingPVRShell::setupVertexAttribs(VkVertexInputBindingDescription* bindings,
    VkVertexInputAttributeDescription* attributes, VkPipelineVertexInputStateCreateInfo& createInfo)
{
	VkFormat sAttributeFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
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

/*!*********************************************************************************************************************
\brief  Creates the Frame buffer objects and Renderpass used in this demo.
***********************************************************************************************************************/
bool VulkanIntroducingPVRShell::createFramebufferAndRenderpass()
{
	// This is an simple framebuffer with 1 color attachment (_swapchain image) with no depth stencil.
	VkRenderPassCreateInfo renderPassInfo = {};
	VkAttachmentDescription attachmentDesc = {};
	VkSubpassDescription subpass = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &attachmentDesc;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.subpassCount = 1;

	attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDesc.format = _swapchain.colorFormat;
	attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	VkAttachmentReference attachmentRef[2] =
	{
		{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
		{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }
	};

	// setup subpass descriptio
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = attachmentRef;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	// WE NEED TWO DEPENDECY HERE
	// 1: Dependecy between the external and the first subpass
	// 2: Dependency between the subpass and the external
	VkSubpassDependency dependencies[] =
	{
		{
			VK_SUBPASS_EXTERNAL,
			0,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		},
		{
			0,
			VK_SUBPASS_EXTERNAL,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		},
	};
	renderPassInfo.pDependencies = dependencies;
	renderPassInfo.dependencyCount = 2;
	if (vk::CreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS)
	{
		return false;
	}

	// Create the framebuffer
	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.width = _swapchain.dimension.width;
	framebufferInfo.height = _swapchain.dimension.height;
	framebufferInfo.layers = 1;
	framebufferInfo.renderPass = _renderPass;
	framebufferInfo.attachmentCount = 1;
	for (uint32_t i = 0; i < _swapchain.numSwapchains; ++i)
	{
		// create the depthstencil images
		VkImageView imageViews[] =
		{
			_swapchain.swapchainImages[i],
			_framebuffers[i].depthStencilImages
		};
		framebufferInfo.pAttachments = imageViews;

		if (vk::CreateFramebuffer(getDevice(), &framebufferInfo, nullptr, &_framebuffers[i].framebufferVk) != VK_SUCCESS)
		{
			return false;
		}
	}
	return true;
}

/*!*********************************************************************************************************************
\brief  Creates Fences and Semaphores to set up synchronisation for this demo.
***********************************************************************************************************************/
bool VulkanIntroducingPVRShell::createSynchronisationPrimitives()
{
	VkSemaphoreCreateInfo sci = {};
	sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fci = {};
	fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (uint32_t i = 0; i < _swapchain.numSwapchains; ++i)
	{
		if (vk::CreateFence(getDevice(), &fci, nullptr, &_fencesPerFrameAcquire[i]) != VK_SUCCESS)
		{
			return false;
		}

		if (vk::CreateFence(getDevice(), &fci, nullptr, &_fencesCommandBuffer[i]) != VK_SUCCESS)
		{
			return false;
		}

		if (vk::CreateSemaphore(getDevice(), &sci, nullptr, &_semaphoresImageAcquire[i]) != VK_SUCCESS)
		{
			return false;
		}

		if (vk::CreateSemaphore(getDevice(), &sci, nullptr, &_semaphoresPresent[i]) != VK_SUCCESS)
		{
			return false;
		}
	}
	return true;
}

/*!*********************************************************************************************************************
\brief  Initialize Color blend attachments used in this demo.
***********************************************************************************************************************/
void VulkanIntroducingPVRShell::initColorBlendAttachmentState(VkPipelineColorBlendAttachmentState& state)
{
	state.blendEnable = VK_FALSE;
	state.colorWriteMask = 0xf;

	state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	state.colorBlendOp = VK_BLEND_OP_ADD;

	state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	state.alphaBlendOp = VK_BLEND_OP_ADD;
}

/*!********************************************************************************************************************
\brief  This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.
\return Return an auto ptr to the demo supplied by the user
***********************************************************************************************************************/
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::unique_ptr<pvr::Shell>(new VulkanIntroducingPVRShell()); }
