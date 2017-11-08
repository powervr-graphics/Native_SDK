/*!*********************************************************************************************************************
\File         VulkanHelloAPI.cpp
\Title        Vulkan HelloAPI
\Author       PowerVR by Imagination, Developer Technology Team.
\Copyright    Copyright(c) Imagination Technologies Limited.
\brief        Build an introductory Vulkan application to show the process of getting started with Vulkan.
***********************************************************************************************************************/
#include "VulkanHelloAPI.h"
#include "vertShader.h"
#include "fragShader.h"

void VulkanHelloAPI::initLayers()
{
	//Due to the (intentionally) limited overhead in Vulkan, error checking is virtually non existent.
	//We initialize Validation Layers to help with that issue. Validation Layers help tracking API objects
	//and calls and make sure there are no validity errors in the code. They are initialized by the Vulkan loader
	//when vk::CreateInstance is called.

#ifdef PVR_DEBUG
	//We create a vector to hold our Layer properties.
	std::vector<VkLayerProperties> outLayers;
	uint32_t numItems = 0;
	//We enumerate on all the layer properties to find the total number of items to add to our vector.
	debugAssertFunctionResult(vk::EnumerateInstanceLayerProperties(&numItems, nullptr), "Fetching Layer count");
	//We resize our vector to hold the result from vk::EnumerateInstanceLayerProperties
	outLayers.resize(numItems);
	//We enumerate once more this time we pass our vector and fetch the Layer properties themselves and store them in our vector.
	debugAssertFunctionResult(vk::EnumerateInstanceLayerProperties(&numItems, outLayers.data()), "Fetching Layer Data");

	//We log the supported layers on this system.
	Log(false, "---------- LAYERS SUPPORTED ----------");
	for (auto && layer : outLayers)
	{
		Log(false, ">> %s", layer.layerName);
	}
	Log(false, "--------------------------------------");

	//We check if the "VK_LAYER_LUNARG_standard_validation" is supported on our system.
	auto found = std::find_if(outLayers.begin(), outLayers.end(), [](VkLayerProperties & prop) { return strcmp(prop.layerName, "VK_LAYER_LUNARG_standard_validation") == 0; });

	if (found != outLayers.end())
	{
		Log(false, ">> Enabling VK_LAYER_LUNARG_standard_validation");

		//instanceLayerNames is a vector that holds the layers we want to activate. We'll use it later when creating the instance.
		appManager.instanceLayerNames.push_back("VK_LAYER_LUNARG_standard_validation");
	}
#endif
}

void VulkanHelloAPI::initExtensions()
{
	//Extensions extend the API�s functionality; they may add additional features or commands. They can be used to a variety of purpose,
	//like providing compatibility for specific Hardware. Instance level extensions are extensions with global-functionality; they affect
	//both the instance-level and device-level commands. Device level extensions affect specifically the device they are bound to.

	//Surface and Swapchain are both extensions as Vulkan doesn't make assumptions on the type of application (it could very well be a compute one not a graphic one).
	//for this reason they are both considered extensions that add functionality to the core API. The surface extension is an instance extension and is added to our
	//instanceExtensionNames vector, while the swapchain is a device one and is added to deviceExtensionNames.
	appManager.instanceExtensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	appManager.deviceExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	//an additional surface extension needs to be loaded, this extension is platform specific and we need to pick it based on the
	//platform we're going to deploy the example to.
#ifdef VK_USE_PLATFORM_WIN32_KHR
	appManager.instanceExtensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
	appManager.instanceExtensionNames.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
	appManager.instanceExtensionNames.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
	appManager.instanceExtensionNames.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif
#ifdef USE_PLATFORM_NULLWS
	appManager.instanceExtensionNames.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#endif

}

void VulkanHelloAPI::initApplicationAndInstance()
{
	//Here we create our instance. Vulkan doesn't have a global state like OpenGL, so a handle to
	//access its functions is required; the instance is the primary access to the API. It will be
	//used to define and create all other Vulkan objects in the rest of the example.

	//Here we create and populate our Application info.
	VkApplicationInfo applicationInfo = {};
	applicationInfo.pNext = nullptr;
	applicationInfo.pApplicationName = "Vulkan Hello API Sample";
	applicationInfo.applicationVersion = 1;
	applicationInfo.engineVersion = 1;
	applicationInfo.pEngineName = "Vulkan Hello API Sample";
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.apiVersion = VK_API_VERSION_1_0;

	//Here we create our Instance Info and assign our application info to it
	//along with our instance layers and extensions.
	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.pNext = nullptr;
	instanceInfo.flags = 0;
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &applicationInfo;
	instanceInfo.enabledLayerCount = static_cast<uint32_t>(appManager.instanceLayerNames.size());
	instanceInfo.ppEnabledLayerNames = appManager.instanceLayerNames.data();
	instanceInfo.enabledExtensionCount = static_cast<uint32_t>(appManager.instanceExtensionNames.size());
	instanceInfo.ppEnabledExtensionNames = appManager.instanceExtensionNames.data();

	//Create our Vulkan Application Instance.
	debugAssertFunctionResult(vk::CreateInstance(&instanceInfo, nullptr, &appManager.instance), "Create Instance");

	//We need to initialize the function pointers depending on the our Vulkan instance. We are using GetInstanceProcAddr to find the correct function
	//pointer associated with this is not necessary but it's a best practice. It provides us with a way to bypass the Vulkan loader and grants us a
	//small performance boost.
	if (!vk::initVulkanInstance(appManager.instance))
	{
		Log(true, "Could not initialize the instance function pointers.");
	}
}

void VulkanHelloAPI::initPhysicalDevice()
{
	//We need to pick which device we want to use for this example. Querying all physical devices and then we decide which one to use
	//base on its compatibility with our needs. Physical device it basically represents a GPU we want to use for our operations.

	//This will hold the number of GPUs available.
	uint32_t gpuCount;

	//We query for the number of GPUs available.
	debugAssertFunctionResult(vk::EnumeratePhysicalDevices(appManager.instance, &gpuCount, nullptr), "GPUS Enumeration - Get Count");

	//We resize the GPUs vector.
	appManager.gpus.resize(gpuCount);

	//We populate the vector with a list of gpus we have available on our platform
	debugAssertFunctionResult(vk::EnumeratePhysicalDevices(appManager.instance, &gpuCount, appManager.gpus.data()), "GPUS Enumeration - Allocate Data");

	//We log some data about our available physical devices
	Log(false, "%s", "------------Devices Info--------------");
	for (const auto& device : appManager.gpus)
	{
		//General device properties like vendor and driver version.
		VkPhysicalDeviceProperties deviceProperties;
		vk::GetPhysicalDeviceProperties(device, &deviceProperties);

		Log(false, "Device Name: %s", deviceProperties.deviceName);
		Log(false, "Device ID: %d", deviceProperties.deviceID);
		Log(false, "Device Driver Version: %d", deviceProperties.driverVersion);
		Log(false, "%s", "--------------------------------------");

		//Features are more in-depth information that we don't need right now so we won't output them
		VkPhysicalDeviceFeatures deviceFeatures;
		vk::GetPhysicalDeviceFeatures(device, &deviceFeatures);
	}

	//We get the device compatible with our needs and query for it's memory properties.
	appManager.physicalDevice = getCompatibleDevice();
	vk::GetPhysicalDeviceMemoryProperties(appManager.physicalDevice, &appManager.deviceMemoryProperties);

}

void VulkanHelloAPI::initQueuesFamilies()
{
	//Queue families are in their simplest form a collection of queues that share properties.
	//Queues are needed by Vulkan to execute commands on. Queue families make sure that the collection
	//of queues we're using is compatible with the operations we want to execute. Here we query the device
	//for the supported queue families and initialize the handle to  the one we need.

	//This will hold the number of queue families available.
	uint32_t queueFamiliesCount;

	//Get the count of queue Families the physical device supports.
	vk::GetPhysicalDeviceQueueFamilyProperties(appManager.physicalDevice, &queueFamiliesCount, nullptr);

	//Resize the vector to fit the number of queue families
	appManager.queueFamilyProperties.resize(queueFamiliesCount);

	//Load the queue families data from the physical device to the list.
	vk::GetPhysicalDeviceQueueFamilyProperties(appManager.physicalDevice, &queueFamiliesCount, &appManager.queueFamilyProperties[0]);

	//Get the index of a compatible queue family.
	getCompatibleQueueFamilies(appManager.graphicsQueueFamilyIndex, appManager.presentQueueFamilyIndex);

}

void VulkanHelloAPI::initLogicDevice()
{
	//We need to create a logical device to start using the API. A logical device is an
	//application view of the physical device that we will be using. The logical device is
	//used to load the device extensions, we and create the rest of the Vulkan API objects.

	//This is a priority for queue (it ranges from 0 - 1) in this case we only have one so it doesn't matter.
	float queuePriorities[1] = { 0.0f };

	//We set up the device queue information.
	VkDeviceQueueCreateInfo deviceQueueInfo = {};
	deviceQueueInfo.pNext = nullptr;
	deviceQueueInfo.flags = 0;
	deviceQueueInfo.queueFamilyIndex = appManager.graphicsQueueFamilyIndex;
	deviceQueueInfo.pQueuePriorities = queuePriorities;
	deviceQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueInfo.queueCount = 1;

	//Our logical device information. Here we add the device extensions we looked up earlier.
	VkDeviceCreateInfo deviceInfo;
	deviceInfo.flags = 0;
	deviceInfo.pNext = nullptr;
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.enabledLayerCount = 0;
	deviceInfo.ppEnabledLayerNames = nullptr;
	deviceInfo.enabledExtensionCount = static_cast<uint32_t>(appManager.deviceExtensionNames.size());
	deviceInfo.ppEnabledExtensionNames = appManager.deviceExtensionNames.data();
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &deviceQueueInfo;
	VkPhysicalDeviceFeatures features;
	vk::GetPhysicalDeviceFeatures(appManager.physicalDevice, &features);
	features.robustBufferAccess = false;
	deviceInfo.pEnabledFeatures = &features;

	//Create the logical device.
	debugAssertFunctionResult(vk::CreateDevice(appManager.physicalDevice, &deviceInfo, nullptr, &appManager.device), "Logic Device Creation");

	//Initialize the Function pointers that require the Device address (same as the instance one)
	if (!vk::initVulkanDevice(appManager.device))
	{
		Log(true, "Could not initialize the device function pointers.");
	}
}

void VulkanHelloAPI::initQueues()
{
	//We need to get the Queues that we will be using for executing the commands. We need two queues one for rendering
	//the other will be used to present the rendering on the surface. Some devices support both operations on the same
	//queue family.

	//We get the Queues from our logical device and save it for later.
	vk::GetDeviceQueue(appManager.device, appManager.graphicsQueueFamilyIndex, 0, &appManager.graphicQueue);

	//If the queue families indices are the same we the same queue to do both operations.
	//If not we get another queue for presenting.
	if (appManager.graphicsQueueFamilyIndex == appManager.presentQueueFamilyIndex)
	{
		appManager.presentQueue = appManager.graphicQueue;
	}
	else
	{
		vk::GetDeviceQueue(appManager.device, appManager.presentQueueFamilyIndex, 0, &appManager.presentQueue);
	}
}

void VulkanHelloAPI::initSurface()
{
	//We initialize the surface we'll need to present our rendered example. Surfaces are based
	//on the platform (OS) we are deploying to. We are using preprocessors here to select the
	//correct function call and info struct datatype for creating a surface.

#ifdef VK_USE_PLATFORM_WIN32_KHR

	//We create the surface info and pass the Win32 window instance and window handles.
	VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.flags = 0;
	surfaceInfo.pNext = nullptr;
	surfaceInfo.hinstance = surfaceData.connection;
	surfaceInfo.hwnd = surfaceData.window;
	surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	//We create the surface we'll be rendering on.
	debugAssertFunctionResult(vk::CreateWin32SurfaceKHR(appManager.instance, &surfaceInfo, nullptr, &appManager.surface), "Windows Surface Creation");
#endif


#ifdef VK_USE_PLATFORM_XLIB_KHR
	//We call the struct method that will create the actual window. We pass the Xlib display and window handles.
	VkXlibSurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.flags = 0;
	surfaceInfo.pNext = nullptr;
	surfaceInfo.dpy = surfaceData.display;
	surfaceInfo.window = surfaceData.window;
	//We create the xlib surface we'll be presenting on.
	debugAssertFunctionResult(vk::CreateXlibSurfaceKHR(appManager.instance, &surfaceInfo, nullptr, &appManager.surface), "XLIB Surface Creation");
#endif


#ifdef VK_USE_PLATFORM_ANDROID_KHR

	//We create the Android surface info. We pass the android window handle.
	VkAndroidSurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.flags = 0;
	surfaceInfo.pNext = 0;
	surfaceInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.window = surfaceData.window;
	//We create the android surface we'll be presenting on.
	debugAssertFunctionResult(vk::CreateAndroidSurfaceKHR(appManager.instance, &surfaceInfo, nullptr, &appManager.surface), "Android Surface Creation");

#endif


#ifdef VK_USE_PLATFORM_WAYLAND_KHR

	//We create the wayland surface info. We pass the android display and surface handles.
	VkWaylandSurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.display = surfaceData.display;
	surfaceInfo.surface = surfaceData.surface;

	//We create the wayland surface we'll be presenting on.
	debugAssertFunctionResult(vk::CreateWaylandSurfaceKHR(appManager.instance, &surfaceInfo, NULL, &appManager.surface), "Wayland Surface Creation");

#endif

#ifdef USE_PLATFORM_NULLWS

	VkDisplayPropertiesKHR properties;
	uint32_t propertiesCount = 1;
	if (vk::GetPhysicalDeviceDisplayPropertiesKHR)
	{
		lastRes = vk::GetPhysicalDeviceDisplayPropertiesKHR(appManager.physicalDevice, &propertiesCount, &properties);
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

	VkDisplayKHR nativeDisplay = properties.display;

	uint32_t modeCount = 0;
	vk::GetDisplayModePropertiesKHR(appManager.physicalDevice, nativeDisplay, &modeCount, NULL);
	std::vector<VkDisplayModePropertiesKHR> modeProperties; modeProperties.resize(modeCount);
	vk::GetDisplayModePropertiesKHR(appManager.physicalDevice, nativeDisplay, &modeCount, modeProperties.data());

	VkDisplaySurfaceCreateInfoKHR surfaceInfo = {};

	surfaceInfo.sType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.pNext = NULL;

	surfaceInfo.displayMode = modeProperties[0].displayMode;
	surfaceInfo.planeIndex = 0;
	surfaceInfo.planeStackIndex = 0;
	surfaceInfo.transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	surfaceInfo.globalAlpha = 0.0f;
	surfaceInfo.alphaMode = VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR;
	surfaceInfo.imageExtent = modeProperties[0].parameters.visibleRegion;

	debugAssertFunctionResult(vk::CreateDisplayPlaneSurfaceKHR(appManager.instance, &surfaceInfo, nullptr, &appManager.surface), "Surface Creation");
#endif
}

void VulkanHelloAPI::initSwapChain()
{
	//If we decide to develop an application that displays something we need to create a swapchain
	//and define its properties. A swapchain is a series of images that are used to render and then
	//present to the surface. On changing the screen size or other changes, the Swap Chain needs to be destroyed
	//and recreated at runtime.

	//variables we need to fetch the surface formats of the Physical Device.
	uint32_t formatsCount;
	std::vector<VkSurfaceFormatKHR> formats;

	//We get the formats count.
	debugAssertFunctionResult(vk::GetPhysicalDeviceSurfaceFormatsKHR(appManager.physicalDevice, appManager.surface, &formatsCount, nullptr), "Swap Chain Format - Get Count");

	//We resize our vector to the size of formats count.
	formats.resize(formatsCount);

	//We populate the vector list with the surface formats.
	debugAssertFunctionResult(vk::GetPhysicalDeviceSurfaceFormatsKHR(appManager.physicalDevice, appManager.surface, &formatsCount, formats.data()), "Swap Chain Format - Allocate Data");

	//If the first format is undefined we pick a default one, else we go with the first one.
	if (formatsCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED) { appManager.surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM; }
	else { appManager.surfaceFormat = formats[0]; }

	//We get the surface capabilities from the surface and physical device.
	VkSurfaceCapabilitiesKHR surface_capabilities;
	debugAssertFunctionResult(vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(appManager.physicalDevice, appManager.surface, &surface_capabilities), "Fetch Surface Capabilities");

	//Present modes are the methods with which images are presented to the surface,
	//we need to figure out which ones are supported by our surface.

	//Variables we need to fetch the present mode formats of the Physical Device.
	uint32_t presentModesCount;
	std::vector<VkPresentModeKHR> presentModes;

	//Get the present mode count
	debugAssertFunctionResult(vk::GetPhysicalDeviceSurfacePresentModesKHR(appManager.physicalDevice, appManager.surface, &presentModesCount, nullptr), "Surface Present Modes - Get Count");

	//Resize and allocate the data for the present mode.
	presentModes.resize(presentModesCount);
	debugAssertFunctionResult(vk::GetPhysicalDeviceSurfacePresentModesKHR(appManager.physicalDevice, appManager.surface, &presentModesCount, presentModes.data()), "Surface Present Modes - Allocate Data");

	//Check if the present mode we want is compatible with the device.
	appManager.presentMode = getCompatiblePresentMode(VK_PRESENT_MODE_IMMEDIATE_KHR, presentModes);

	//Get the correct extent (dimensions) of the surface
	appManager.swapchainExtent = getCorrectExtent(surface_capabilities);

	//We get the minimum number of images supported on this surface.
	uint32_t surfaceImageCount = std::max<uint32_t>(2, surface_capabilities.minImageCount);

	//We create the swapchain info to create our swapchain
	VkSwapchainCreateInfoKHR swapchainInfo = {};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.flags = 0;
	swapchainInfo.pNext = NULL;
	swapchainInfo.surface = appManager.surface;
	swapchainInfo.imageFormat = appManager.surfaceFormat.format;
	swapchainInfo.preTransform = surface_capabilities.currentTransform;
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainInfo.presentMode = appManager.presentMode;
	swapchainInfo.minImageCount = surfaceImageCount;
	swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
	swapchainInfo.clipped = VK_TRUE;
	swapchainInfo.imageExtent.width = appManager.swapchainExtent.width;
	swapchainInfo.imageExtent.height = appManager.swapchainExtent.height;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageColorSpace = appManager.surfaceFormat.colorSpace;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	//Fixing the Height and Width of the Surface (in case it's not defined).
	if (surfaceData.width == 0 || surfaceData.height == 0)
	{
		surfaceData.width = (float)swapchainInfo.imageExtent.width;
		surfaceData.height = (float)swapchainInfo.imageExtent.height;
	}

	//Here we check if the present queue and the graphic queue are the same
	//if they are we don't need to share images between multiple queues so we go exclusive mode.
	//If not we go for a sharing mode concurrent.
	if (appManager.graphicsQueueFamilyIndex == appManager.presentQueueFamilyIndex)
	{
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainInfo.queueFamilyIndexCount = 0;
		swapchainInfo.pQueueFamilyIndices = NULL;
	}
	else
	{
		uint32_t queueFamilyIndices[] = { appManager.graphicsQueueFamilyIndex,  appManager.presentQueueFamilyIndex };

		swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainInfo.queueFamilyIndexCount = 2;
		swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
	}

	//We finally create the swapchain.
	debugAssertFunctionResult(vk::CreateSwapchainKHR(appManager.device, &swapchainInfo, NULL, &appManager.swapchain), "SwapChain Creation");
}

void VulkanHelloAPI::initImagesAndViews()
{
	//Images in Vulkan are the object representation of data it can take many forms (e.g. attachments, textures etc) in our case we
	//use them to hold the swapchain (to screen render) image. In case of the swapchain the images are automatically created for us.
	//On the other hand views are a snapshot of the images parameters. It describes how to access the image and which parts to access.
	//it helps to distinguish the type of image we're working with.

	uint32_t swapchainImageCount;
	std::vector<VkImage> images;

	//Get the count of the images we need (we set this in InitSwapchain function, it's the minimum number of images supported.)
	debugAssertFunctionResult(vk::GetSwapchainImagesKHR(appManager.device, appManager.swapchain, &swapchainImageCount, nullptr), "SwapChain Images - Get Count");

	//We resize and get the data of those images.
	images.resize(swapchainImageCount);

	//We resize our swapchain vector to be able to hold the number of images we need.
	appManager.swapChainImages.resize(swapchainImageCount);

	//We get the images for our swapchain we save them in a temporary vector.
	debugAssertFunctionResult(vk::GetSwapchainImagesKHR(appManager.device, appManager.swapchain, &swapchainImageCount, images.data()), "SwapChain Images - Allocate Data");

	for (uint32_t i = 0; i < swapchainImageCount; ++i)
	{
		//We copy over the images to the vector in our struct.
		appManager.swapChainImages[i].image = images[i];

		//We create our image view info and associate it with our image, we got from the swap chain
		VkImageViewCreateInfo image_view_info = {};
		image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_info.pNext = NULL;
		image_view_info.flags = 0;
		image_view_info.image = appManager.swapChainImages[i].image;
		image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_info.format = appManager.surfaceFormat.format;

		image_view_info.components.r = VK_COMPONENT_SWIZZLE_R;
		image_view_info.components.g = VK_COMPONENT_SWIZZLE_G;
		image_view_info.components.b = VK_COMPONENT_SWIZZLE_B;
		image_view_info.components.a = VK_COMPONENT_SWIZZLE_A;

		image_view_info.subresourceRange.layerCount = 1;
		image_view_info.subresourceRange.levelCount = 1;
		image_view_info.subresourceRange.baseArrayLayer = 0;
		image_view_info.subresourceRange.baseMipLevel = 0;
		image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		//We create an image view to hold all info about the image.
		debugAssertFunctionResult(vk::CreateImageView(appManager.device, &image_view_info, nullptr, &appManager.swapChainImages[i].view), "SwapChain Images View Creation");
	}
}

void VulkanHelloAPI::initShaders()
{
	//In Vulkan shaders are in SPIR-V format which is a bytecode format rather than a human readable one.
	//SPIR-V can be used for both graphical and compute operations. We load the compiled code (see vertshader.h & fragshader.h)
	//and create shader stages that are going to be used by our pipeline later on.

	createShader(spvVertShader, sizeof(spvVertShader), 0, VK_SHADER_STAGE_VERTEX_BIT);

	createShader(spvFragShader, sizeof(spvFragShader), 1, VK_SHADER_STAGE_FRAGMENT_BIT);

}

void VulkanHelloAPI::initRenderPass()
{
	//In Vulkan, a render pass is a collection of data that describes a set of frame buffer
	//attachments that are needed for rendering.A render pass is composed of sub passes that
	//order the data.Render pass collects all the color, depth and stencil attachments and
	//makes sure to explicitly define them so that driver doesn't have to deduce them itself.


	//We create a description of our color attachment that will be added to the render pass.
	//This will tell the render pass what to do with the image (frame buffer) before, during and after rendering.
	VkAttachmentDescription colorAttachmentDescription = {};
	colorAttachmentDescription.format = appManager.surfaceFormat.format;
	colorAttachmentDescription.flags = 0;
	colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

	//We create a color attachment reference.
	VkAttachmentReference colorAttachmentReference = {};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//We create a description of our sub pass
	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.flags = 0;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentReference;
	subpassDescription.pDepthStencilAttachment = nullptr;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;
	subpassDescription.pResolveAttachments = nullptr;

	//We create our info struct for the render pass.
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.flags = 0;
	renderPassInfo.pNext = nullptr;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = &colorAttachmentDescription;
	renderPassInfo.pSubpasses = &subpassDescription; // the sub pass we just created.
	renderPassInfo.pDependencies = nullptr;
	renderPassInfo.dependencyCount = 0;

	//We don't need to create any depth or stencil buffer so we are good to go and can create our render pass.

	//We create our render pass here.
	debugAssertFunctionResult(vk::CreateRenderPass(appManager.device, &renderPassInfo, nullptr, &appManager.renderPass), "Render pass Creation");
}

void VulkanHelloAPI::initVertexBuffers()
{
	//We'll create a simple triangle to pass to our vertex shader to be rendered
	//on screen.

	//Calculate the size of the vertex buffer to be passed to the vertex shader
	appManager.vertexBuffer.size = sizeof(Vertex) * 3;

	//We set the values for our triangle's vertices
	Vertex triangle[3];
	triangle[0] = { -.5f, -.5f, 0.0f, 1.0f,
	                0.0f, 0.0f, -1.0f,
	                0.0f, 0.0f
	              };
	triangle[1] = { .5f, -.5f, 0.0f, 1.0f,
	                0.0f, 0.0f, -1.0f,
	                1.0f, 0.0f
	              };
	triangle[2] = { 0.0f,  .5f, 0.0f, 1.0f,
	                0.0f, 0.0f, -1.0f,
	                0.5f, 1.0f
	              };

	//We create the buffer that will hold the data and be passed to the shaders.
	createBuffer(appManager.vertexBuffer, (uint8_t*)triangle, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
}

void VulkanHelloAPI::initUniformBuffers()
{
	//We'll create a uniform buffer to hold the MVP and pass that to the vertex shader.

	//We query the physical device properties to find out the minimum alignment offset.
	//We'll need this information to align the uniform buffer correctly.
	VkPhysicalDeviceProperties deviceProperties;
	vk::GetPhysicalDeviceProperties(appManager.physicalDevice, &deviceProperties);
	size_t uboAlignment = (size_t)deviceProperties.limits.minUniformBufferOffsetAlignment;
	appManager.uniformBuffer.size = ((sizeof(float) * 16 * 3) / uboAlignment) * uboAlignment + (((sizeof(float) * 16 * 3) % uboAlignment) > 0 ? uboAlignment : 0);;

	//We allocate the necessary memory to hold the MVP
	float* MVP = (float*)malloc(appManager.uniformBuffer.size);

	//Our matrices are identity matrices, in our case we are only rendering a simple triangle so
	//we don't need to provide a MVP, but this illustrates how to pass a uniform buffer to the vertex shader.
	float projectionMatrix[16] =
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	float viewMatrix[16] =
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	float modelMatrix[16] =
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	//We memcpy the three matrices and we'll calculate the MVP in the shader itself.
	memcpy(MVP, &projectionMatrix, sizeof(projectionMatrix));
	memcpy(((float*)MVP + 16), &viewMatrix, sizeof(viewMatrix));
	memcpy(((float*)MVP + 32), &modelMatrix, sizeof(modelMatrix));


	//We create the actual uniform buffer.
	createBuffer(appManager.uniformBuffer, (uint8_t*)MVP, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

	//We clean up our allocation.
	free(MVP);
}

void VulkanHelloAPI::initTexture()
{
	// In Vulkan, uploading an image requires multiple steps, including:

	// 1) Creating the texture
	//    a) Creating the texture definition ("VkImage" object)
	//    b) Determining its memory requirements, creating the backing memory object ("VkDeviceMemory" object)
	//    c) Binding the memory to the image

	// 2) Uploading the data into the texture.
	//    a) Creating a staging buffer
	//    b) Determining its memory requirements, creating the backing memory object ("VkDeviceMemory" object)
	//    c) Mapping the staging buffer and copying the image data into it
	//    d) Performing a vkCmdCopyBufferToImage operation to transfer the data. This requires a command buffer and relevant objects.

	// A texture (Sampled Image) is stored in the GPU in an implementation-defined way, which may be completely different
	// to the layout of the texture on disk/cpu side.
	// For that reason, it is not possible to map its memory and write directly the data for that image.
	// This is the reason for the second (Uploading) step: The vkCmdCopyBufferToImage command guarantees the correct
	// translation/swizzling of the texture data.

	//Our texture data, Size, height, width.
	appManager.texture.height = 255;
	appManager.texture.width = 255;
	appManager.texture.data.resize(appManager.texture.width * appManager.texture.height * (sizeof(uint8_t) * 4));

	//This function generates our texture pattern on-the-fly into a block of cpu side memory (appManager.texture.data)
	generateTexture();

	//We use our custom buffer data struct to hold the necessary data for our staging buffer.
	BufferData stagingBufferData;
	stagingBufferData.size = appManager.texture.data.size();

	//We use our buffer creation function to generate a staging buffer. We pass the VK_BUFFER_USAGE_TRANSFER_SRC_BIT flag to specify its use.
	createBuffer(stagingBufferData, (uint8_t*)appManager.texture.data.data(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

	//We create the image info struct. We set the parameters for our texture (layout, format, usage etc..)
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.flags = 0;
	imageInfo.pNext = nullptr;
	imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.extent = { appManager.texture.width, appManager.texture.height, 1 };
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;

	//We create the texture image handle.
	debugAssertFunctionResult(vk::CreateImage(appManager.device, &imageInfo, nullptr, &appManager.texture.image), "Texture Image Creation");

	//We need to allocate actual memory for the Image we just created.

	//We get the memory allocation requirements for our Image
	VkMemoryRequirements memoryRequirments;
	vk::GetImageMemoryRequirements(appManager.device, appManager.texture.image, &memoryRequirments);

	//We create a memory allocation info to hold the memory requirements size for our image
	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.pNext = nullptr;
	allocateInfo.memoryTypeIndex = 0;
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirments.size;

	//Here we check if the memory we're going to use is compatible with our operation. If it isn't we find the compatible one.
	getMemoryTypeFromProperties(appManager.deviceMemoryProperties, memoryRequirments.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &(allocateInfo.memoryTypeIndex));

	//We finally allocate the memory for our Image and bind the memory to texture buffer.
	debugAssertFunctionResult(vk::AllocateMemory(appManager.device, &allocateInfo, nullptr, &appManager.texture.memory), "Texture Image Memory Allocation");
	debugAssertFunctionResult(vk::BindImageMemory(appManager.device, appManager.texture.image, appManager.texture.memory, 0), "Texture Image Memory Binding");

	//We specify the region we want to copy from our Texture. In our case it's the entire Image so we pass
	//the Texture width and height as extents.
	VkBufferImageCopy copyRegion = {};
	copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.imageSubresource.mipLevel = 0;
	copyRegion.imageSubresource.baseArrayLayer = 0;
	copyRegion.imageSubresource.layerCount = 1;
	copyRegion.imageExtent.width = static_cast<uint32_t>(appManager.texture.width);
	copyRegion.imageExtent.height = static_cast<uint32_t>(appManager.texture.height);
	copyRegion.imageExtent.depth = 1;
	copyRegion.bufferOffset = 0;

	//We create command buffer to execute the copy operation from our command pool.
	VkCommandBuffer commandBuffer;
	VkCommandBufferAllocateInfo commandAllocateInfo = {};
	commandAllocateInfo.commandPool = appManager.commandPool;
	commandAllocateInfo.pNext = nullptr;
	commandAllocateInfo.commandBufferCount = 1;
	commandAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

	//We Allocate the command buffer from the command pool's memory.
	debugAssertFunctionResult(vk::AllocateCommandBuffers(appManager.device, &commandAllocateInfo, &commandBuffer), "Allocate Command Buffers");

	//We start recording our command buffer operation
	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = 0;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	debugAssertFunctionResult(vk::BeginCommandBuffer(commandBuffer, &commandBufferBeginInfo), "Begin Image Copy to Staging Buffer Command Buffer Recording");

	//We specify the sub resource range of our Image. In the case our Image the parameters are default as our image is very simple.
	VkImageSubresourceRange subResourceRange = {};
	subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subResourceRange.baseMipLevel = 0;
	subResourceRange.levelCount = 1;
	subResourceRange.layerCount = 1;

	//We need to create a memory barrier to make sure that the image layout is set up for a copy operation.
	VkImageMemoryBarrier copyMemoryBarrier = {};
	copyMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	copyMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	copyMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	copyMemoryBarrier.image = appManager.texture.image;
	copyMemoryBarrier.subresourceRange = subResourceRange;
	copyMemoryBarrier.srcAccessMask = 0;
	copyMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	//We use a pipeline barrier to change the image layout to accommodate the transfer operation
	vk::CmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &copyMemoryBarrier);

	//We copy the staging buffer data to memory bound to the image we just created.
	vk::CmdCopyBufferToImage(commandBuffer, stagingBufferData.buffer, appManager.texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

	//We create a barrier to make sure that the Image layout is Shader read only.
	VkImageMemoryBarrier layoutMemoryBarrier = {};
	layoutMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	layoutMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	layoutMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	layoutMemoryBarrier.image = appManager.texture.image;
	layoutMemoryBarrier.subresourceRange = subResourceRange;
	layoutMemoryBarrier.srcAccessMask = 0;
	layoutMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	//We use a pipeline barrier to change the image layout to be optimized to be read by the shader.
	vk::CmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &layoutMemoryBarrier);

	//We end the recording of our command buffer.
	debugAssertFunctionResult(vk::EndCommandBuffer(commandBuffer), "End Image Copy to Staging Buffer Command Buffer Recording");

	//We create a fence to make sure that the command buffer is synchronized correctly.
	VkFence copyFence;
	VkFenceCreateInfo copyFenceInfo  = {};
	copyFenceInfo.flags = 0;
	copyFenceInfo.pNext = 0;
	copyFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	//We create the fence proper.
	debugAssertFunctionResult(vk::CreateFence(appManager.device, &copyFenceInfo, nullptr, &copyFence), "Image Copy to Staging Buffer Fence Creation");

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	debugAssertFunctionResult(vk::QueueSubmit(appManager.graphicQueue, 1, &submitInfo, copyFence), "Submit Image Copy to Staging Buffer Command Buffer");
	// Wait for the fence to be signaled. We need to make sure that command buffer has finished executing
	debugAssertFunctionResult(vk::WaitForFences(appManager.device, 1, &copyFence, VK_TRUE, FENCE_TIMEOUT), "Image Copy to Staging Buffer Fence Signal");

	//After the Image is complete, and we copied all the texture data, we need to create an Image View to make sure
	//that API can understand what the Image is. We can provide information on the format for example.

	//We create an Image view info.
	VkImageViewCreateInfo imageViewInfo = {};
	imageViewInfo.flags = 0;
	imageViewInfo.pNext = nullptr;
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageViewInfo.image = appManager.texture.image;
	imageViewInfo.subresourceRange.layerCount = 1;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;

	//We create the image view proper.
	debugAssertFunctionResult(vk::CreateImageView(appManager.device, &imageViewInfo, nullptr, &appManager.texture.view), "Texture Image View Creation");

	//We create a sampler info struct. We'll need the sampler to pass
	//data to the fragment shader during the execution of the rendering phase.
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.flags = 0;
	samplerInfo.pNext = nullptr;
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 5.0f;

	//We create the sampler object.
	debugAssertFunctionResult(vk::CreateSampler(appManager.device, &samplerInfo, nullptr, &appManager.texture.sampler), "Texture Sampler Creation");

	//We clean up all the temporary data we created for this operation.
	vk::DestroyFence(appManager.device, copyFence, nullptr);
	vk::FreeCommandBuffers(appManager.device, appManager.commandPool, 1, &commandBuffer);
	vk::FreeMemory(appManager.device, stagingBufferData.memory, nullptr);
	vk::DestroyBuffer(appManager.device, stagingBufferData.buffer, nullptr);
}

void VulkanHelloAPI::initDynamicUniformBuffers()
{
	//We create a dynamic uniform buffer that will hold the rotation data for our triangle.
	//Using a dynamic uniform buffer we can avoid multiple uniform buffers and instead place
	//use offsets. We have one longer buffer and a number of offsets (equal to number of frames)
	//that point to the start of the next data set in the buffer. This gives a performance boost
	//to the application.

	//We create a vector for rotation, the actual size of the uniform buffer
	//will be based on the minimum alignment offset we extrapolated earlier from
	//the device properties.
	appManager.angle = 45.0f;
	float rot_angles[4] = { 0.0f, 0.0f, appManager.angle, 0.0f };

	//We create a dynamic uniform buffer to hold the rotation data for all the frames
	createDynamicUniformBuffer(appManager.dynamicUniformBufferData.uniformBuffer);

	uint8_t* pData;
	//We create a (offsetted) uniform buffer for each swapchain image we have.
	for (size_t i = 0; i < appManager.swapChainImages.size(); ++i)
	{
		VkDeviceSize offset = (uint8_t)(appManager.offset * i);
		debugAssertFunctionResult(vk::MapMemory(appManager.device, appManager.dynamicUniformBufferData.uniformBuffer.memory, offset, appManager.dynamicUniformBufferData.uniformBuffer.bufferInfo.range, 0, (void**)&pData), "Dynamic Buffer Memory Mapping");
		//Copy the data into the pointer mapped to the memory.
		memcpy(pData, (uint8_t*)rot_angles, appManager.offset);
		//remove the mapping. Unmap it quickly as the mechanism has limited
		//memory that is visible to both CPU and GPU
		vk::UnmapMemory(appManager.device, appManager.dynamicUniformBufferData.uniformBuffer.memory);
	}

	//We bind the memory to the buffer.
	debugAssertFunctionResult(vk::BindBufferMemory(appManager.device, appManager.dynamicUniformBufferData.uniformBuffer.buffer, appManager.dynamicUniformBufferData.uniformBuffer.memory, 0), "Dynamic Buffer Bind Memory");

	//The DescriptorPool size we specify is of type VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
	//This tells Vulkan that the Descriptor pool we're going to create needs to be configured
	//to accommodate the creation of Dynamic uniform buffers.

	VkDescriptorPoolSize descriptorPoolSize;
	descriptorPoolSize.descriptorCount = 1;
	descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

	VkDescriptorPoolCreateInfo descriptroPoolInfo = {};
	descriptroPoolInfo.flags = 0;
	descriptroPoolInfo.pNext = nullptr;
	descriptroPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptroPoolInfo.poolSizeCount = 1;
	descriptroPoolInfo.pPoolSizes = &descriptorPoolSize;
	descriptroPoolInfo.maxSets = 1;

	//We create the descriptor pool proper, that we'll use for allocating the dynamic uniform buffer descriptor set.
	debugAssertFunctionResult(vk::CreateDescriptorPool(appManager.device, &descriptroPoolInfo, NULL, &appManager.dynamicUniformBufferData.descriptorPool), "Frame Descriptor Pool Creation");

	//We create our descriptor layout binding (we define the type of data we'll be passing the shader and the binding location)
	VkDescriptorSetLayoutBinding descriptorLayoutBinding;
	descriptorLayoutBinding.descriptorCount = 1;
	descriptorLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	descriptorLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	descriptorLayoutBinding.binding = 1;
	descriptorLayoutBinding.pImmutableSamplers = nullptr;

	//The info struct for our Descriptor set layout we pass the number of bindings we created.
	VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
	descriptorLayoutInfo.flags = 0;
	descriptorLayoutInfo.pNext = nullptr;
	descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayoutInfo.bindingCount = 1;
	descriptorLayoutInfo.pBindings = &descriptorLayoutBinding;

	//We create our descriptor set layout.
	debugAssertFunctionResult(vk::CreateDescriptorSetLayout(appManager.device, &descriptorLayoutInfo, NULL, &appManager.dynamicUniformBufferData.descriptorSetLayout), "Frame Descriptor Set Layout Creation");

	//We create an descriptor allocation info to allocate the descriptors from the descriptor pool.
	VkDescriptorSetAllocateInfo descriptorAllocateInfo = {};
	descriptorAllocateInfo.descriptorPool = appManager.dynamicUniformBufferData.descriptorPool;
	descriptorAllocateInfo.descriptorSetCount = 1;
	descriptorAllocateInfo.pNext = nullptr;
	descriptorAllocateInfo.pSetLayouts = &appManager.dynamicUniformBufferData.descriptorSetLayout;
	descriptorAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

	//We allocate our descriptor set.
	debugAssertFunctionResult(vk::AllocateDescriptorSets(appManager.device, &descriptorAllocateInfo, &appManager.dynamicUniformBufferData.descriptorSet), "Frame Descriptor Set Creation");

	//We use this info struct to define the info we'll be using to write the actual data to the descriptor set we created (we take info from uniform buffer).
	VkWriteDescriptorSet descriptorSetWrite = {};
	descriptorSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorSetWrite.pNext = nullptr;
	descriptorSetWrite.dstSet = appManager.dynamicUniformBufferData.descriptorSet;
	descriptorSetWrite.descriptorCount = 1;
	descriptorSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	descriptorSetWrite.pBufferInfo = &appManager.dynamicUniformBufferData.uniformBuffer.bufferInfo; //pass Uniform buffer to this function
	descriptorSetWrite.dstArrayElement = 0;
	descriptorSetWrite.dstBinding = 1;

	//We update the descriptor sets.
	vk::UpdateDescriptorSets(appManager.device, 1, &descriptorSetWrite, 0, nullptr);
}

void VulkanHelloAPI::initDescriptorPoolAndSet()
{
	//In Vulkan to pass data to shaders we define descriptor sets. Descriptors as the name implies are used to describe the data we want to pass. They hold information
	//that helps with binding data to shaders and additionally describes any information Vulkan requires to know before executing the shader. Descriptors are not passed
	//individually (and are opaque to the application) but instead bundled in sets, known as Descriptor Sets.

	//The process of creating a descriptor set is a three step process. We start with creating a descriptor pool that is used to allocate descriptor sets.
	//We then create a descriptor layout that defines how the descriptor set is laid out; information on the binding points and the type of data passed to the shader.
	//The descriptor sets themselves hold (in form of a pointer) the data that we need to pass to the shader (textures, uniform buffers etc..).

	//The size of the descriptor Pool (we establish how many descriptors we need)
	VkDescriptorPoolSize descriptorPoolSize[2];

	descriptorPoolSize[0].descriptorCount = 1;
	descriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	descriptorPoolSize[1].descriptorCount = 1;
	descriptorPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	//The info struct for our descriptor Pool
	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.flags = 0;
	descriptorPoolInfo.pNext = nullptr;
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = 2;
	descriptorPoolInfo.pPoolSizes = descriptorPoolSize;
	descriptorPoolInfo.maxSets = 1;

	//We create our descriptor Pool
	debugAssertFunctionResult(vk::CreateDescriptorPool(appManager.device, &descriptorPoolInfo, NULL, &appManager.descriptorPool), "Descriptor Pool Creation");

	//We create our descriptor layout binding (we define the type of data we'll be passing the shader and the binding location)
	VkDescriptorSetLayoutBinding descriptorLayoutBinding[2];
	descriptorLayoutBinding[0].descriptorCount = 1;
	descriptorLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorLayoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	descriptorLayoutBinding[0].binding = 0;
	descriptorLayoutBinding[0].pImmutableSamplers = nullptr;

	descriptorLayoutBinding[1].descriptorCount = 1;
	descriptorLayoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorLayoutBinding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptorLayoutBinding[1].binding = 1;
	descriptorLayoutBinding[1].pImmutableSamplers = nullptr;

	//The info struct for our Descriptor set layout. We pass the number of bindings we created.
	VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
	descriptorLayoutInfo.flags = 0;
	descriptorLayoutInfo.pNext = nullptr;
	descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayoutInfo.bindingCount = 2;
	descriptorLayoutInfo.pBindings = descriptorLayoutBinding;

	//We create our descriptor set layout.
	debugAssertFunctionResult(vk::CreateDescriptorSetLayout(appManager.device, &descriptorLayoutInfo, NULL, &appManager.descriptorSetLayout), "Descriptor Set Layout Creation");

	//We create an descriptor allocation info to allocate the descriptors from the descriptor pool.
	VkDescriptorSetAllocateInfo descriptorAllocateInfo = {};
	descriptorAllocateInfo.descriptorPool = appManager.descriptorPool;
	descriptorAllocateInfo.descriptorSetCount = 1;
	descriptorAllocateInfo.pNext = nullptr;
	descriptorAllocateInfo.pSetLayouts = &appManager.descriptorSetLayout;
	descriptorAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

	//We allocate our descriptor set.
	debugAssertFunctionResult(vk::AllocateDescriptorSets(appManager.device, &descriptorAllocateInfo, &appManager.descriptorSet), "Descriptor Set Creation");

	//We use this info struct to define the info we'll be using to write the actual data to the descriptor set we created (we take info from uniform buffer).
	VkWriteDescriptorSet descriptorSetWrite = {};
	descriptorSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorSetWrite.pNext = nullptr;
	descriptorSetWrite.dstSet = appManager.descriptorSet;
	descriptorSetWrite.descriptorCount = 1;
	descriptorSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSetWrite.pBufferInfo = &appManager.uniformBuffer.bufferInfo; //pass Uniform buffer to this function
	descriptorSetWrite.dstArrayElement = 0;
	descriptorSetWrite.dstBinding = 0;

	vk::UpdateDescriptorSets(appManager.device, 1, &descriptorSetWrite, 0, nullptr);

	//This info is referencing the texture sampler we'll be passing to the shaders by way of the descriptors.
	VkDescriptorImageInfo descriptorImageInfo = {};
	descriptorImageInfo.sampler = appManager.texture.sampler;
	descriptorImageInfo.imageView = appManager.texture.view;
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

	VkWriteDescriptorSet descriptorSetWriteImage = {};
	descriptorSetWriteImage.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorSetWriteImage.pNext = nullptr;
	descriptorSetWriteImage.dstSet = appManager.descriptorSet;
	descriptorSetWriteImage.descriptorCount = 1;
	descriptorSetWriteImage.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorSetWriteImage.pImageInfo = &descriptorImageInfo;
	descriptorSetWriteImage.dstArrayElement = 0;
	descriptorSetWriteImage.dstBinding = 1;
	descriptorSetWriteImage.pBufferInfo = nullptr;
	descriptorSetWriteImage.pTexelBufferView = nullptr;
	descriptorSetWriteImage.pImageInfo = &descriptorImageInfo;

	vk::UpdateDescriptorSets(appManager.device, 1, &descriptorSetWriteImage, 0, NULL);
}

void VulkanHelloAPI::initPipeline()
{
	//A pipeline can best be described as a collection of stages in the rendering or compute process.
	//Each stage processes data and passes it on to the next stage.In Vulkan, there are two types of pipelines a graphics and compute one.
	//The graphics in used for rendering operations, while the compute allows the application to perform computational work(e.g.Physics calculations).
	//In Vulkan, The pipeline is stored in one object that is immutable; therefore each object we want to render will possibly use a different pipeline.
	//The pipeline in Vulkan needs to be prepared before the its use, this helps with increasing the performance of the application.

	//The descriptor of the binding between the vertex buffer data and the vertex shader in the pipeline.
	VkVertexInputBindingDescription vertexInputBindingDescription = {};
	vertexInputBindingDescription.binding = 0;
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vertexInputBindingDescription.stride = sizeof(Vertex);

	//The descriptor of the attributes for the vertex input.
	VkVertexInputAttributeDescription vertexInputAttributeDescription[3];
	vertexInputAttributeDescription[0].binding = 0;
	vertexInputAttributeDescription[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vertexInputAttributeDescription[0].location = 0;
	vertexInputAttributeDescription[0].offset = 0;

	vertexInputAttributeDescription[1].binding = 0;
	vertexInputAttributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributeDescription[1].location = 1;
	vertexInputAttributeDescription[1].offset = 4 * sizeof(float);

	vertexInputAttributeDescription[2].binding = 0;
	vertexInputAttributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
	vertexInputAttributeDescription[2].location = 2;
	vertexInputAttributeDescription[2].offset = (4 + 3) * sizeof(float);

	//We create the Vertex input info to be added to the Pipeline.
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = 3;
	vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributeDescription;

	//We create the input assembly info to be added to the pipeline.
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.flags = 0;
	inputAssemblyInfo.pNext = nullptr;
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	//We define the rasterizer info that the pipeline will be using.
	VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
	rasterizationInfo.pNext = nullptr;
	rasterizationInfo.flags = 0;
	rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizationInfo.lineWidth = 1.0f;
	rasterizationInfo.depthBiasClamp = 0.0f;
	rasterizationInfo.depthBiasConstantFactor = 0.0f;
	rasterizationInfo.depthBiasEnable = VK_FALSE;
	rasterizationInfo.depthBiasSlopeFactor = 0.0f;

	//This color blend attachment state will be used by the color blend info.
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = 0xf;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

	//The color blend info required by the pipeline
	VkPipelineColorBlendStateCreateInfo colorBlendInfo = {};
	colorBlendInfo.flags = 0;
	colorBlendInfo.pNext = nullptr;
	colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
	colorBlendInfo.logicOpEnable = VK_FALSE;
	colorBlendInfo.attachmentCount = 1;
	colorBlendInfo.pAttachments = &colorBlendAttachment;
	colorBlendInfo.blendConstants[0] = 0.0f;
	colorBlendInfo.blendConstants[1] = 0.0f;
	colorBlendInfo.blendConstants[2] = 0.0f;
	colorBlendInfo.blendConstants[3] = 0.0f;

	//We create our multi-sampling info, we don't need it so we set everything to the default value.
	VkPipelineMultisampleStateCreateInfo multisamplingInfo = {};
	multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingInfo.pNext = nullptr;
	multisamplingInfo.flags = 0;
	multisamplingInfo.pSampleMask = nullptr;
	multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisamplingInfo.sampleShadingEnable = VK_TRUE;
	multisamplingInfo.alphaToCoverageEnable = VK_FALSE;
	multisamplingInfo.alphaToOneEnable = VK_FALSE;
	multisamplingInfo.minSampleShading = 0.0f;

	//We create a list of dynamic states we will be using.
	VkDynamicState dynamicState[VK_DYNAMIC_STATE_RANGE_SIZE];
	memset(dynamicState, 0, sizeof(dynamicState));

	//We create the dynamic state info struct.
	VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
	dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateInfo.dynamicStateCount = 0;
	dynamicStateInfo.pNext = nullptr;
	dynamicStateInfo.pDynamicStates = dynamicState;

	//We create the view port and we add the view port and scissor as dynamic states in our dynamic state info struct.
	VkPipelineViewportStateCreateInfo viewportInfo = {};
	viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportInfo.pNext = nullptr;
	viewportInfo.flags = 0;

	viewportInfo.viewportCount = 1;
	dynamicState[dynamicStateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
	viewportInfo.pViewports = &appManager.viewport;

	viewportInfo.scissorCount = 1;
	dynamicState[dynamicStateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
	viewportInfo.pScissors = &appManager.scissor;

	//We create a list of the descriptor layouts we are going to add to the pipeline.
	VkDescriptorSetLayout descriptorSetLayout[] = { appManager.descriptorSetLayout, appManager.dynamicUniformBufferData.descriptorSetLayout };

	//Create Pipeline Layout info for generating our pipeline.
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 2; //We know the count of our descriptors already.
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayout; //We add them to the pipeline layout info struct.
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = 0;

	//We create our pipeline layout.
	debugAssertFunctionResult(vk::CreatePipelineLayout(appManager.device, &pipelineLayoutInfo, nullptr, &appManager.pipelineLayout), "Pipeline Layout Creation");

	//We create our pipeline info and add all the info structs we created in this init function
	VkGraphicsPipelineCreateInfo pipelineInfo;
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = nullptr;
	pipelineInfo.layout = appManager.pipelineLayout;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = 0;
	pipelineInfo.flags = 0;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineInfo.pRasterizationState = &rasterizationInfo;
	pipelineInfo.pColorBlendState = &colorBlendInfo;
	pipelineInfo.pTessellationState = nullptr;
	pipelineInfo.pMultisampleState = &multisamplingInfo;
	pipelineInfo.pDynamicState = &dynamicStateInfo;
	pipelineInfo.pViewportState = &viewportInfo;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pStages = appManager.shaderStages;
	pipelineInfo.stageCount = 2;
	pipelineInfo.renderPass = appManager.renderPass;
	pipelineInfo.subpass = 0;

	//We create our pipeline we'll use for rendering.
	debugAssertFunctionResult(vk::CreateGraphicsPipelines(appManager.device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &appManager.pipeline), "Pipeline Creation");
}

void VulkanHelloAPI::initFrameBuffers()
{
	//In Vulkan, all the attachments used by the render pass are defined in frame buffers.Each frame in a frame buffer defines
	//the attachments related to it.This can encompass the textures (including the color and depth / stencil attachments) and
	//the input attachment. This way of separating descriptions in render passes and definitions in frame buffers gives us the option
	//to use different render passes with different frame buffers. The degree of flexibility with which we can do this is based on the
	//compatibility of the two however.

	//Placeholder handle for the attachment to be stored in the VkFramebufferCreateInfo
	VkImageView attachment = VK_NULL_HANDLE;

	//We create the frame buffer info that needed for the frame buffers, we add the attachment (Null variable for now).
	VkFramebufferCreateInfo frameBufferInfo = {};
	frameBufferInfo.flags = 0;
	frameBufferInfo.pNext = nullptr;
	frameBufferInfo.attachmentCount = 1;
	frameBufferInfo.height = appManager.swapchainExtent.height;
	frameBufferInfo.width = appManager.swapchainExtent.width;
	frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferInfo.renderPass = appManager.renderPass;
	frameBufferInfo.pAttachments = &attachment;
	frameBufferInfo.layers = 1;

	//Resize frame buffer based on the number of images in the swap chain.
	appManager.frameBuffers.resize(appManager.swapChainImages.size());

	//Iterate the size of our swapchain list and create a frame buffer for each.

	for (size_t i = 0; i < appManager.swapChainImages.size(); ++i)
	{
		//Assign the buffer's view to our attachment variable.
		attachment = appManager.swapChainImages[i].view;
		//Create the frame buffer (with the current attachment value).
		debugAssertFunctionResult(vk::CreateFramebuffer(appManager.device, &frameBufferInfo, nullptr, &appManager.frameBuffers[i]), "Swapchain Frame buffer creation");
	}
}

void VulkanHelloAPI::initCommandPoolAndBuffer()
{
	//We create a CommandPool, which is used to reserve memory for command buffers that we'll have to create as execution
	//After the CommandPool is created we allocate command buffers from it. We need a number of command buffers equal to
	//the number of Images in the swapchain (assuming the command buffers are used for rendering)

	//We create a command pool info to (based on the queue family we'll be using)
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.pNext = nullptr;
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.queueFamilyIndex = appManager.graphicsQueueFamilyIndex;

	//Create the actual command pool.
	debugAssertFunctionResult(vk::CreateCommandPool(appManager.device, &commandPoolInfo, nullptr, &appManager.commandPool), "Command Pool Creation");

	//Resize our vector to have a number of elements equal to the number of swapchain images.
	appManager.commandBuffers.resize(appManager.swapChainImages.size());

	//create a command buffer info, reference our command pool to point to where we'll get the memory for our command buffer.
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = appManager.commandPool;
	commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(appManager.commandBuffers.size());
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	//Allocate our command buffer from our command pool.
	debugAssertFunctionResult(vk::AllocateCommandBuffers(appManager.device, &commandBufferAllocateInfo, appManager.commandBuffers.data()), "Command Buffer Creation");
}

void VulkanHelloAPI::initViewportAndScissor()
{
	//We set up the view port (the dimensions of the view of our rendering area) and we also set up the
	//Scissor which is a sub section of the view port in our case the extents of the scissor is the same
	//as the view port as we want to view all of it.

	//Set the view port dimensions, depth and starting coordinates.
	appManager.viewport.width = surfaceData.width;
	appManager.viewport.height = surfaceData.height;
	appManager.viewport.minDepth = 0.0f;
	appManager.viewport.maxDepth = 1.0f;
	appManager.viewport.x = 0;
	appManager.viewport.y = 0;

	//Set the extent to the surface dimensions and the offset to 0
	appManager.scissor.extent.width = static_cast<uint32_t>(surfaceData.width);
	appManager.scissor.extent.height = static_cast<uint32_t>(surfaceData.height);
	appManager.scissor.offset.x = 0;
	appManager.scissor.offset.y = 0;
}

void VulkanHelloAPI::initSemaphoreAndFence()
{
	//Fences and Semaphores are used to synchronize work on the CPU and GPU that share the same resources.

	//Fences are GPU to CPU syncs, they get signaled by the GPU and can only be waited on by the CPU. They need to be reset manually

	//Semaphores are GPU to GPU syncs, specifically used to sync queue submissions (on the same or different queue) it get signaled by
	//the GPU. It's waited on by the GPU and it's reset after it's waited on.

	for (uint32_t i = 0; i < appManager.swapChainImages.size(); ++i)
	{
		//We create 2 sets of semaphores to sync between rendering and acquiring operations between different swapchain images.
		VkSemaphore acquireSemaphore;
		VkSemaphore renderSemaphore;

		//We create a fence per frame that will sync between CPU and GPU.
		VkFence frameFence;

		//We create our acquire semaphore info struct.
		VkSemaphoreCreateInfo acquireSemaphoreInfo = {};
		acquireSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		acquireSemaphoreInfo.pNext = nullptr;
		acquireSemaphoreInfo.flags = 0;

		//We Create the actual semaphore for the acquired image
		debugAssertFunctionResult(vk::CreateSemaphore(appManager.device, &acquireSemaphoreInfo, nullptr, &acquireSemaphore), "Acquire Semaphore creation");

		//Push the acquire semaphore to our vector.
		appManager.acquireSemaphore.push_back(acquireSemaphore);

		//We create our render semaphore info struct.
		VkSemaphoreCreateInfo renderSemaphoreInfo = {};
		renderSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		renderSemaphoreInfo.pNext = nullptr;
		renderSemaphoreInfo.flags = 0;

		//We Create the actual semaphore for rendering the finished image
		debugAssertFunctionResult(vk::CreateSemaphore(appManager.device, &renderSemaphoreInfo, nullptr, &renderSemaphore), "Render Semaphore creation");

		//Push the render semaphore to our vector.
		appManager.presentSemaphores.push_back(renderSemaphore);

		//We create our acquire fence info struct.
		VkFenceCreateInfo FenceInfo;
		FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		FenceInfo.pNext = nullptr;
		FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; //Start the fence as signaled.

		//We create the fence required to sync the command operations between frames.
		debugAssertFunctionResult(vk::CreateFence(appManager.device, &FenceInfo, nullptr, &frameFence), "Fence Creation");

		//Push the render fence to our vector.
		appManager.frameFences.push_back(frameFence);
	}

}

void VulkanHelloAPI::createBuffer(BufferData& inBuffer, const uint8_t* inData, const VkBufferUsageFlags& inUsage)
{
	//We use this generic function to create buffers. The usage flag that determines the type of buffer that is going to be used.
	//is passed when called. The function is responsible for creating the buffer allocating the memory, mapping the memory and
	//coping the data into the buffer.

	//We create our Buffer Creation info, this will tell the API what the buffer is for and to use it.
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.flags = 0;
	bufferInfo.pNext = nullptr;
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = inBuffer.size;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.usage = inUsage;
	bufferInfo.pQueueFamilyIndices = nullptr;
	bufferInfo.queueFamilyIndexCount = 0;

	//We create our buffer.
	debugAssertFunctionResult(vk::CreateBuffer(appManager.device, &bufferInfo, NULL, &inBuffer.buffer), "Buffer Creation");

	//The memory requirements for our buffer
	VkMemoryRequirements memoryRequirments;

	//We extract the memory requirements for our buffer.
	vk::GetBufferMemoryRequirements(appManager.device, inBuffer.buffer, &memoryRequirments);

	//We create our allocation info struct and pass the memory requirement size.
	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.pNext = nullptr;
	allocateInfo.memoryTypeIndex = 0;
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirments.size;

	//Here we check if the memory we're going to use is compatible with our operation.
	//If it isn't we find the compatible one.
	bool pass = getMemoryTypeFromProperties(appManager.deviceMemoryProperties, memoryRequirments.memoryTypeBits,
	                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &(allocateInfo.memoryTypeIndex));
	if (pass)
	{
		//We'll use this pointer to pass the data into the buffer
		uint8_t* pData;

		//We allocate the memory necessary for our data.
		debugAssertFunctionResult(vk::AllocateMemory(appManager.device, &allocateInfo, nullptr, &(inBuffer.memory)), "Allocate Buffer Memory");

		//We save data in our buffer struct
		inBuffer.bufferInfo.range = memoryRequirments.size;
		inBuffer.bufferInfo.offset = 0;
		inBuffer.bufferInfo.buffer = inBuffer.buffer;

		//Mapping Data to the memory, the inBuffer.memory is the device memory handle.
		//memoryRequirments.size. The size of the memory required for the mapping.
		//the &pData the actual data to map to that memory.
		debugAssertFunctionResult(vk::MapMemory(appManager.device, inBuffer.memory, 0, inBuffer.size, 0, (void**)&pData), "Map Buffer Memory");

		//Copy the data into the pointer mapped to the memory.
		memcpy(pData, inData, (size_t)inBuffer.size);

		//Remove the mapping. Unmap it quickly as the mechanism has limited
		//Memory that is visible to both CPU and GPU
		vk::UnmapMemory(appManager.device, inBuffer.memory);

		debugAssertFunctionResult(vk::BindBufferMemory(appManager.device, inBuffer.buffer, inBuffer.memory, 0), "Bind Buffer Memory");
	}

}

void VulkanHelloAPI::createDynamicUniformBuffer(BufferData& inBuffer)
{
	//We use this function to create a dynamic uniform buffer. Dynamic uniform buffers are buffers that contain the
	//data for multiple single uniform buffer (usually each associated with a frame) and use offsets to access said data.
	//this minimizes the amount of descriptor sets required and may help optimize write operations.

	//We query the physical device properties.
	VkPhysicalDeviceProperties deviceProperties;
	vk::GetPhysicalDeviceProperties(appManager.physicalDevice, &deviceProperties);

	//We check the limit of the dynamic buffers the physical device supports.
	if (deviceProperties.limits.maxDescriptorSetUniformBuffersDynamic > 1)
	{
		//We get the alignment of the uniform buffer
		size_t uboAlignment = (size_t)deviceProperties.limits.minUniformBufferOffsetAlignment;
		//We calculate the size of each offset so that it aligns correctly with our device property alignment
		appManager.offset = static_cast<uint32_t>(((sizeof(float) * 4) / uboAlignment) * uboAlignment + (((sizeof(float) * 4) % uboAlignment) > 0 ? uboAlignment : 0));

		//Calculate the full size of the buffer.
		inBuffer.size = appManager.swapChainImages.size() * appManager.offset;

		//We create our Buffer Creation info, this will tell the API what the buffer is for and to use it.
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.flags = 0;
		bufferInfo.pNext = nullptr;
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = inBuffer.size;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferInfo.pQueueFamilyIndices = nullptr;
		bufferInfo.queueFamilyIndexCount = 0;

		//We create our buffer.
		debugAssertFunctionResult(vk::CreateBuffer(appManager.device, &bufferInfo, NULL, &inBuffer.buffer), "Buffer Creation");

		//The memory requirements for our buffer
		VkMemoryRequirements memoryRequirments;

		//We extract the memory requirements for our buffer.
		vk::GetBufferMemoryRequirements(appManager.device, inBuffer.buffer, &memoryRequirments);

		//We create our allocation info struct and pass the memory requirements size.
		VkMemoryAllocateInfo allocateInfo = {};
		allocateInfo.pNext = nullptr;
		allocateInfo.memoryTypeIndex = 0;
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memoryRequirments.size;

		//Here we check if the memory we're going to use is compatible with the our operation.
		//If it isn't we find the compatible one.
		bool pass = getMemoryTypeFromProperties(appManager.deviceMemoryProperties, memoryRequirments.memoryTypeBits,
		                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &(allocateInfo.memoryTypeIndex));

		if (pass)
		{
			//We allocate the memory for the buffer.
			debugAssertFunctionResult(vk::AllocateMemory(appManager.device, &allocateInfo, nullptr, &(inBuffer.memory)), "Dynamic Buffer Memory Allocation");

			//We save the data to our buffer struct
			inBuffer.bufferInfo.range = memoryRequirments.size / appManager.swapChainImages.size();
			inBuffer.bufferInfo.offset = 0;
			inBuffer.bufferInfo.buffer = inBuffer.buffer;
		}

	}

}

void VulkanHelloAPI::createShader(uint32_t* spvShader, size_t spvShaderSize, int indx, VkShaderStageFlagBits shaderStage)
{
	//This function will create a shader module and update our shader stage array. The shader stages will be used later on
	//by the pipeline to determine the stages that the rendering process will go through. The shader module will hold
	//the data from the pre-compiled shader.

	//We create our shader module create info to specify the code and size of our shader.
	VkShaderModuleCreateInfo shaderModuleInfo = {};
	shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleInfo.flags = 0;
	shaderModuleInfo.pCode = spvShader;
	shaderModuleInfo.codeSize = spvShaderSize;
	shaderModuleInfo.pNext = nullptr;

	//We create a shader stage, it defines what stage the shader is. It will also be used later on in the pipeline.
	appManager.shaderStages[indx].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	appManager.shaderStages[indx].flags = 0;
	appManager.shaderStages[indx].pName = "main";
	appManager.shaderStages[indx].pNext = nullptr;
	appManager.shaderStages[indx].stage = shaderStage;
	appManager.shaderStages[indx].pSpecializationInfo = nullptr;

	//We create our shader module and added it to the shader stage corresponding to the VkShaderStageFlagBits stage
	debugAssertFunctionResult(vk::CreateShaderModule(appManager.device, &shaderModuleInfo, nullptr, &(appManager.shaderStages[indx].module)), "Shader Module Creation");

}

void VulkanHelloAPI::recordCommandBuffer()
{
	//Command buffers are containers that contain GPU commands. They are passed to the queues to be executed on the device.
	//Each command when executed performs a different task. The command buffer, for instance, required to render an object is
	//recorded before the rendering. When we reach the rendering stage of our application we submit the command buffer to execute its tasks.

	//We state our clear values for rendering.
	VkClearValue clearColor = { 0.00f, 0.70f, 0.67f, 1.0f };
	//Constant offset we need for the vertex buffer binding.
	const VkDeviceSize vertexOffsets[1] = { 0 };

	//We record each command buffer we created.
	for (size_t i = 0; i < appManager.commandBuffers.size(); ++i)
	{
		debugAssertFunctionResult(vk::ResetCommandBuffer(appManager.commandBuffers[i], 0), "Command Buffer Reset");

		//We create our command buffer info struct
		VkCommandBufferBeginInfo cmd_begin_info = {};
		cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmd_begin_info.pNext = nullptr;
		cmd_begin_info.flags = 0;
		cmd_begin_info.pInheritanceInfo = nullptr;

		//We Begin recording for the selected command buffer.
		debugAssertFunctionResult(vk::BeginCommandBuffer(appManager.commandBuffers[i], &cmd_begin_info), "Command Buffer Recording Started.");

		//We set the view port
		vk::CmdSetViewport(appManager.commandBuffers[i], 0, 1, &appManager.viewport);
		//We set the scissor
		vk::CmdSetScissor(appManager.commandBuffers[i], 0, 1, &appManager.scissor);

		//We set up our Render pass info and attach a frame buffer to it
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.pNext = nullptr;
		renderPassInfo.renderPass = appManager.renderPass;
		renderPassInfo.framebuffer = appManager.frameBuffers[i];
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;
		renderPassInfo.renderArea.extent = appManager.swapchainExtent;
		renderPassInfo.renderArea.offset.x = 0;
		renderPassInfo.renderArea.offset.y = 0;

		//We begin recording our render pass operations
		vk::CmdBeginRenderPass(appManager.commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		//We bind our previously created pipeline to the command buffer.
		vk::CmdBindPipeline(appManager.commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, appManager.pipeline);

		//We bind the descriptors, we use the same for each frame.
		VkDescriptorSet descriptorSet[] = { appManager.descriptorSet, appManager.dynamicUniformBufferData.descriptorSet };
		//We calculate the offset per frame.
		uint32_t offset = static_cast<uint32_t>(appManager.offset * i);
		//We bind the descriptor with the offset to each command buffer.
		vk::CmdBindDescriptorSets(appManager.commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, appManager.pipelineLayout, 0, NUM_DESCRIPTOR_SETS, descriptorSet, 1, &offset);

		//We bind the vertex buffer.
		vk::CmdBindVertexBuffers(appManager.commandBuffers[i], 0, 1, &appManager.vertexBuffer.buffer, vertexOffsets);

		//We record a draw command.
		vk::CmdDraw(appManager.commandBuffers[i], 3, 1, 0, 0);

		//We end the rendering pass operations
		vk::CmdEndRenderPass(appManager.commandBuffers[i]);

		//End the command recording process.
		debugAssertFunctionResult(vk::EndCommandBuffer(appManager.commandBuffers[i]), "Command Buffer Recording Ended.");

	}
}

void VulkanHelloAPI::drawFrame()
{
	//Here we execute our recorded command buffers. The recorded operations will end up rendering
	//and present our frame to the surface.

	//We wait for the fence to be signaled before we start rendering the current frame.
	debugAssertFunctionResult(vk::WaitForFences(appManager.device, 1, &appManager.frameFences[frameId], true, FENCE_TIMEOUT), "Fence - Signaled");

	//We go ahead and reset the fence we won't need it for the rest of the frame.
	vk::ResetFences(appManager.device, 1, &appManager.frameFences[frameId]);

	//CurrentBuffer will be used to point to the correct frame/command buffer/uniform buffer data
	//it's going to be the general index of what data we're working on.
	uint32_t currentBuffer = 0;
	VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	//Get the index of the next available swapchain image.
	debugAssertFunctionResult(vk::AcquireNextImageKHR(appManager.device, appManager.swapchain, std::numeric_limits<uint64_t>::max(), appManager.acquireSemaphore[frameId], VK_NULL_HANDLE, &currentBuffer), "Draw - Acquire Image");

	//Since the uniform buffer is dynamic we specify the current frame index to calculate the offset.
	applyRotation(currentBuffer);

	//We create the submit info to submit our command buffer to the GPU
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.pWaitDstStageMask = &pipe_stage_flags;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &appManager.acquireSemaphore[frameId];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &appManager.presentSemaphores[frameId];
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &appManager.commandBuffers[currentBuffer];

	//We submit to the graphic queue to start the render.
	debugAssertFunctionResult(vk::QueueSubmit(appManager.graphicQueue, 1, &submitInfo, appManager.frameFences[frameId]), "Draw - Submit to Graphic Queue");

	//We create our present info to get ready to present our rendered image to the surface.
	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &appManager.swapchain;
	presentInfo.pImageIndices = &currentBuffer;
	presentInfo.pWaitSemaphores = &appManager.presentSemaphores[frameId];
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pResults = nullptr;

	//We present our rendered image to the surface.
	debugAssertFunctionResult(vk::QueuePresentKHR(appManager.presentQueue, &presentInfo), "Draw - Submit to Present Queue");

	//Update the frameId to get the next suitable one.
	frameId = (frameId + 1) % appManager.swapChainImages.size();
}

void VulkanHelloAPI::getCompatibleQueueFamilies(uint32_t& graphicsfamilyindex, uint32_t& presentfamilyindex)
{
	//We go through all the QueueFamilies available to us on the selected device and we pick a graphics
	//and present queue (by selecting the index) we make sure that the present queue supports presenting.
	//The indices may end up being the same, we check for that later on.

	int i = 0;
	VkBool32 compatible = VK_FALSE;

	for (const auto& queuefamily : appManager.queueFamilyProperties)
	{
		//Check if the family has queues and that are Graphical and not Computational queues.
		if (queuefamily.queueCount > 0 && queuefamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphicsfamilyindex = i;
			break;
		}
		i++;
	}

	i = 0;

	for (const auto& queuefamily : appManager.queueFamilyProperties)
	{
		//Check if the family has queues and that are Graphical and not Computational queues.
		if (queuefamily.queueCount > 0 && queuefamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			//We check if the queue family supports presenting.
			debugAssertFunctionResult(vk::GetPhysicalDeviceSurfaceSupportKHR(appManager.physicalDevice, i, appManager.surface, &compatible), "Querying Physical Device Surface Support");

			if (compatible)
			{
				presentfamilyindex = i;
				break;
			}
		}
		i++;
	}
}

VkPhysicalDevice VulkanHelloAPI::getCompatibleDevice()
{
	//We iterate through the available physical devices and figure out which one is compatible with what we require.

	for (const auto& device : appManager.gpus)
	{
		//We make sure the device we are using is of type VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vk::GetPhysicalDeviceProperties(device, &deviceProperties);
		vk::GetPhysicalDeviceFeatures(device, &deviceFeatures);

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader)
		{
			//We return the device compatible with our needs.
			Log(false, "Active Device is -- %s", deviceProperties.deviceName);
			return device;
		}
	}

	if (appManager.gpus.size() == 1)
	{
		//If there's only one device we return that one.
		return appManager.gpus[0];
	}

	//We return null if we find nothing.
	return nullptr;
}

VkPresentModeKHR VulkanHelloAPI::getCompatiblePresentMode(const VkPresentModeKHR& inReqMode, const std::vector<VkPresentModeKHR>& inModes)
{
	//Checks for the present mode compatibility. VK_PRESENT_MODE_FIFO_KHR is the most supported format.
	//We check if the other format we selected is valid if not we just default to VK_PRESENT_MODE_FIFO_KHR

	//Check if the modes supported are compatible with the one requested
	for (const auto& mode : inModes)
		if (mode == inReqMode)
		{
			return mode;
		}

	Log(false, "Defaulting to VK_PRESENT_MODE_FIFO_KHR");

	//If not we return the default one.
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanHelloAPI::getCorrectExtent(const VkSurfaceCapabilitiesKHR& inSurfCap)
{
	//This function makes sure we have the correct extents for our surface based on the surface
	//capabilities. It also checks whether the extents are valid and the same as the one we picked in
	//initSurface()

	//The width and height of the Swapchain are either both 0xFFFFFFFF (Max value for uint_32t)
	//or they are both NOT 0xFFFFFFFF
	if (inSurfCap.currentExtent.width == std::numeric_limits<uint32_t>::max())
	{

		//Passing the Width and Height from the surface.
		appManager.swapchainExtent.width = appManager.swapchainExtent.height = static_cast<uint32_t>(surfaceData.width);
		VkExtent2D currentExtent = appManager.swapchainExtent;

		//The swapchain extent width and height cannot be less then the minimum surface capability
		//Also The swapchain extent width and height cannot be greater then the maximum surface capability
		if (appManager.swapchainExtent.width < inSurfCap.minImageExtent.width) { currentExtent.width = inSurfCap.minImageExtent.width; }
		else if (appManager.swapchainExtent.width > inSurfCap.maxImageExtent.width) { currentExtent.width = inSurfCap.maxImageExtent.width; }

		if (appManager.swapchainExtent.height < inSurfCap.minImageExtent.height) { currentExtent.height = inSurfCap.minImageExtent.height; }
		else if (appManager.swapchainExtent.height > inSurfCap.maxImageExtent.height) { currentExtent.height = inSurfCap.maxImageExtent.height; }

		//Make sure the extents are not 0 if they are use values we picked form our surface data.
		if (currentExtent.width == 0 && currentExtent.height == 0)
		{
			currentExtent.width = (uint32_t)surfaceData.width;
			currentExtent.height = (uint32_t)surfaceData.height;
		}

		return currentExtent;
	}
	else
	{
		//Make sure the extents are not 0 if they are use values we picked form our surface data.
		if (inSurfCap.currentExtent.width == 0 && inSurfCap.currentExtent.height == 0)
		{
			VkExtent2D currentExtent;
			currentExtent.width = (uint32_t)surfaceData.width;
			currentExtent.height = (uint32_t)surfaceData.height;
			return currentExtent;
		}

		return inSurfCap.currentExtent;
	}

}

void VulkanHelloAPI::generateTexture()
{
	//This function will generate a checkered texture on the fly to be used on the triangle we are going
	//to render and rotate on screen.

	//Generates checkered texture
	for (uint8_t x = 0; x < appManager.texture.width; ++x)
	{
		for (uint8_t y = 0; y < appManager.texture.height; ++y)
		{
			float g = 0.3;
			if (x % 128 < 64 && y % 128 < 64) { g = 1; }
			if (x % 128 >= 64 && y % 128 >= 64) { g = 1; }

			uint8_t* pixel = ((uint8_t*)appManager.texture.data.data()) + (x * appManager.texture.height * 4) + (y * 4);
			pixel[0] = static_cast<uint8_t>(100 * g);
			pixel[1] = static_cast<uint8_t>(80 * g);
			pixel[2] = static_cast<uint8_t>(70 * g);
			pixel[3] = 255;
		}
	}
}

void VulkanHelloAPI::applyRotation(int idx)
{
	//We call this every frame to update the dynamic uniform buffer with the new rotation
	//value. An offset is used to point to the correct subset of data and it's mapped then
	//updated.

	//Pointer to where the data will be copied.
	void* pData;

	//Calculate the offset, and map the memory. The offset and range specify the part of the memory we need.
	VkDeviceSize offset = (appManager.offset * idx);
	debugAssertFunctionResult(vk::MapMemory(appManager.device, appManager.dynamicUniformBufferData.uniformBuffer.memory, offset, appManager.dynamicUniformBufferData.uniformBuffer.bufferInfo.range, 0, (void**)&pData), "Dynamic Buffer Map Memory");

	//Update our angle of rotation
	appManager.angle += 0.2;
	float rot_angles[4] = { 0.0f, 0.0f, appManager.angle, 0.0f };

	//Copy the data to our mapped memory (using our offset)
	memcpy(pData, (uint8_t*)rot_angles, appManager.offset);

	//Unmap the memory.
	vk::UnmapMemory(appManager.device, appManager.dynamicUniformBufferData.uniformBuffer.memory);

}

void VulkanHelloAPI::initialize()
{
	//We initialize all our Vulkan objects here. The vk::initVulkan() function is used to load
	//the Vulkan library and definitions.

	//frameId is the index we'll be using for synchronization. It's going to be used mostly by
	//fences and semaphores to keep track of which one we're current free to work on.
	frameId = 0;

	//Required to initialize the Function pointers.
	vk::initVulkan();

	initLayers();
	initExtensions();

	initApplicationAndInstance();
	initPhysicalDevice();

	initSurface();

	initQueuesFamilies();
	initLogicDevice();
	initQueues();
	initSwapChain();
	initImagesAndViews();
	initCommandPoolAndBuffer();

	initShaders();
	initVertexBuffers();
	initUniformBuffers();
	initRenderPass();
	initTexture();
	initDescriptorPoolAndSet();

	initDynamicUniformBuffers();
	initFrameBuffers();
	initPipeline();

	initViewportAndScissor();
	initSemaphoreAndFence();
}

void VulkanHelloAPI::deinitialize()
{
	//Making sure that all the objects we created are clean up correctly and nothing
	//is left "open" when we close our application.

	//Wait for the device to have finished all operations before we start the clean up.
	debugAssertFunctionResult(vk::DeviceWaitIdle(appManager.device), "Device Wait for Idle");

	//Here we destroy our fence
	for (auto& fence : appManager.frameFences)
	{
		vk::DestroyFence(appManager.device, fence, nullptr);
	}

	//We destroy  the semaphores used for image acquisition and rendering
	for (auto& semaphore : appManager.acquireSemaphore)
	{
		vk::DestroySemaphore(appManager.device, semaphore, nullptr);
	}

	for (auto& semaphore : appManager.presentSemaphores)
	{
		vk::DestroySemaphore(appManager.device, semaphore, nullptr);
	}

	//We destroy  both the descriptor Layout and descriptor Pool.
	vk::DestroyDescriptorSetLayout(appManager.device, appManager.descriptorSetLayout, nullptr);
	vk::DestroyDescriptorPool(appManager.device, appManager.descriptorPool, nullptr);

	//Destroy all descriptor related data from our dynamic uniform buffer.
	vk::DestroyDescriptorSetLayout(appManager.device, appManager.dynamicUniformBufferData.descriptorSetLayout, nullptr);
	vk::DestroyDescriptorPool(appManager.device, appManager.dynamicUniformBufferData.descriptorPool, nullptr);

	//Destroy the buffer and free the memory.
	vk::DestroyBuffer(appManager.device, appManager.dynamicUniformBufferData.uniformBuffer.buffer, nullptr);
	vk::FreeMemory(appManager.device, appManager.dynamicUniformBufferData.uniformBuffer.memory, nullptr);

	//Destroy the pipeline followed by the pipeline layout
	vk::DestroyPipeline(appManager.device, appManager.pipeline, nullptr);
	vk::DestroyPipelineLayout(appManager.device, appManager.pipelineLayout, nullptr);

	//Destroy our texture image
	vk::DestroyImage(appManager.device, appManager.texture.image, nullptr);
	//We destroy the image view
	vk::DestroyImageView(appManager.device, appManager.texture.view, nullptr);
	//We free the memory we allocated for the texture.
	vk::FreeMemory(appManager.device, appManager.texture.memory, nullptr);
	//We destroy the sampler
	vk::DestroySampler(appManager.device, appManager.texture.sampler, nullptr);

	//We destroy then free the memory for the vertex buffer.
	vk::DestroyBuffer(appManager.device, appManager.vertexBuffer.buffer, nullptr);
	vk::FreeMemory(appManager.device, appManager.vertexBuffer.memory, nullptr);

	//We destroy then free the memory for the uniform buffer.
	vk::DestroyBuffer(appManager.device, appManager.uniformBuffer.buffer, nullptr);
	vk::FreeMemory(appManager.device, appManager.uniformBuffer.memory, nullptr);

	//Iterate through the Frame buffers and destroy them.
	for (uint32_t i = 0; i < appManager.frameBuffers.size(); i++)
	{
		vk::DestroyFramebuffer(appManager.device, appManager.frameBuffers[i], nullptr);
	}

	//Destroy our two shader stages (vertex and fragment)
	vk::DestroyShaderModule(appManager.device, appManager.shaderStages[0].module, nullptr);
	vk::DestroyShaderModule(appManager.device, appManager.shaderStages[1].module, nullptr);

	//Destroy the render pass.
	vk::DestroyRenderPass(appManager.device, appManager.renderPass, nullptr);

	for (auto& imagebuffers : appManager.swapChainImages)
	{
		//Clean up Swapchain image views
		vk::DestroyImageView(appManager.device, imagebuffers.view, nullptr);
	}
	//Clean up the swapchain.
	vk::DestroySwapchainKHR(appManager.device, appManager.swapchain, nullptr);
	//Clean up Surface.
	vk::DestroySurfaceKHR(appManager.instance, appManager.surface, nullptr);

	//Free the allocated memory in the command Buffers;
	vk::FreeCommandBuffers(appManager.device, appManager.commandPool, static_cast<uint32_t>(appManager.commandBuffers.size()), appManager.commandBuffers.data());

	//Destroy the command pool
	vk::DestroyCommandPool(appManager.device, appManager.commandPool, nullptr);

	//Destroy the logical device.
	vk::DestroyDevice(appManager.device, nullptr);

	//Clean up our instance.
	vk::DestroyInstance(appManager.instance, nullptr);
}

