/*!
\brief Function definitions for the Instance class
\file PVRVk/InstanceVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

//!\cond NO_DOXYGEN
#include "InstanceVk.h"

namespace pvrvk {
namespace impl {

LogLevel mapValidationTypeToLogType(VkDebugReportFlagsEXT flags)
{
	if ((flags & VkDebugReportFlagsEXT::e_INFORMATION_BIT_EXT) != 0)
	{
		return LogLevel::Information;
	}
	if ((flags & VkDebugReportFlagsEXT::e_WARNING_BIT_EXT) != 0)
	{
		return LogLevel::Warning;
	}
	if ((flags & VkDebugReportFlagsEXT::e_PERFORMANCE_WARNING_BIT_EXT) != 0)
	{
		return LogLevel::Information;
	}
	if ((flags & VkDebugReportFlagsEXT::e_ERROR_BIT_EXT) != 0)
	{
		return LogLevel::Error;
	}
	if ((flags & VkDebugReportFlagsEXT::e_DEBUG_BIT_EXT) != 0)
	{
		return LogLevel::Debug;
	}

	return LogLevel::Information;
}

std::string mapDebugReportObjectTypeToString(VkDebugReportObjectTypeEXT objectType)
{
	switch (objectType)
	{
	case VkDebugReportObjectTypeEXT::e_INSTANCE_EXT: return "INSTANCE_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_PHYSICAL_DEVICE_EXT: return "PHYSICAL_DEVICE_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_DEVICE_EXT: return "DEVICE_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_QUEUE_EXT: return "QUEUE_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_SEMAPHORE_EXT: return "SEMAPHORE_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_COMMAND_BUFFER_EXT: return "COMMAND_BUFFER_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_FENCE_EXT: return "FENCE_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_DEVICE_MEMORY_EXT: return "DEVICE_MEMORY_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_BUFFER_EXT: return "BUFFER_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_IMAGE_EXT: return "IMAGE_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_EVENT_EXT: return "EVENT_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_QUERY_POOL_EXT: return "QUERY_POOL_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_BUFFER_VIEW_EXT: return "BUFFER_VIEW_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_IMAGE_VIEW_EXT: return "IMAGE_VIEW_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_SHADER_MODULE_EXT: return "SHADER_MODULE_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_PIPELINE_CACHE_EXT: return "PIPELINE_CACHE_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_PIPELINE_LAYOUT_EXT: return "PIPELINE_LAYOUT_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_RENDER_PASS_EXT: return "RENDER_PASS_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_PIPELINE_EXT: return "PIPELINE_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_DESCRIPTOR_SET_LAYOUT_EXT: return "DESCRIPTOR_SET_LAYOUT_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_SAMPLER_EXT: return "SAMPLER_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_DESCRIPTOR_POOL_EXT: return "DESCRIPTOR_POOL_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_DESCRIPTOR_SET_EXT: return "DESCRIPTOR_SET_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_FRAMEBUFFER_EXT: return "FRAMEBUFFER_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_COMMAND_POOL_EXT: return "COMMAND_POOL_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_SURFACE_KHR_EXT: return "SURFACE_KHR_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_SWAPCHAIN_KHR_EXT: return "SWAPCHAIN_KHR_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_DEBUG_REPORT_CALLBACK_EXT_EXT: return "DEBUG_REPORT_CALLBACK_EXT_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_DISPLAY_KHR_EXT: return "DISPLAY_KHR_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_DISPLAY_MODE_KHR_EXT: return "DISPLAY_MODE_KHR_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_OBJECT_TABLE_NVX_EXT: return "OBJECT_TABLE_NVX_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_INDIRECT_COMMANDS_LAYOUT_NVX_EXT: return "INDIRECT_COMMANDS_LAYOUT_NVX_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_DESCRIPTOR_UPDATE_TEMPLATE_KHR_EXT: return "DESCRIPTOR_UPDATE_TEMPLATE_KHR_EXT"; break;
	case VkDebugReportObjectTypeEXT::e_UNKNOWN_EXT:
	default:
		return "UNKNOWN_EXT"; break;
	}
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
	(void)object;
	(void)location;
	(void)messageCode;
	(void)pLayerPrefix;
	(void)pUserData;
	Log(mapValidationTypeToLogType(flags), std::string(mapDebugReportObjectTypeToString(objectType) +
	    std::string(". VULKAN_LAYER_VALIDATION: %s")).c_str(), pMessage);

	return VK_FALSE;
}

bool Instance_::initDebugCallbacks()
{
	if (pvrvk::Extensions::isInstanceExtensionSupported("VK_EXT_debug_report"))
	{
		if (vk::CreateDebugReportCallbackEXT && vk::DebugReportMessageEXT && vk::DestroyDebugReportCallbackEXT)
		{
			// Setup callback creation information
			VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
			callbackCreateInfo.sType = VkStructureType::e_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
			callbackCreateInfo.pNext = nullptr;
			callbackCreateInfo.flags = VkDebugReportFlagsEXT::e_ERROR_BIT_EXT | VkDebugReportFlagsEXT::e_WARNING_BIT_EXT |
			                           VkDebugReportFlagsEXT::e_PERFORMANCE_WARNING_BIT_EXT | VkDebugReportFlagsEXT::e_DEBUG_BIT_EXT;
			callbackCreateInfo.pfnCallback = &CustomDebugReportCallback;
			callbackCreateInfo.pUserData = nullptr;

			// Register the callback
			VkResult result = vk::CreateDebugReportCallbackEXT(getNativeObject(), &callbackCreateInfo,
			                  nullptr, &_debugReportCallback);

			if (result == VkResult::e_SUCCESS)
			{
				Log(LogLevel::Information, "Debug report callback successfully enabled");
				_supportsDebugReport = true;
			}
			else
			{
				Log(LogLevel::Information, "Could not enable debug report callback");
				_supportsDebugReport = false;
			}
		}
	}

	return true;
}

bool Instance_::init(const InstanceCreateInfo& instanceCreateInfo)
{
	vk::initVulkan();
	_createInfo = instanceCreateInfo;

	VkApplicationInfo appInfo = {};
	if (_createInfo.applicationInfo)
	{
		appInfo.sType = VkStructureType::e_APPLICATION_INFO;
		appInfo.apiVersion = _createInfo.applicationInfo->apiVersion;
		appInfo.applicationVersion = _createInfo.applicationInfo->applicationVersion;
		appInfo.engineVersion = _createInfo.applicationInfo->engineVersion;
		appInfo.pApplicationName = _createInfo.applicationInfo->applicationName;
		appInfo.pEngineName = _createInfo.applicationInfo->engineName;
	}

	std::vector<const char*> enabledExtensions;
	std::vector<const char*> enableLayers;

	VkInstanceCreateInfo instanceCreateInfoVk = {};
	instanceCreateInfoVk.sType = VkStructureType::e_INSTANCE_CREATE_INFO;
	instanceCreateInfoVk.pApplicationInfo = &appInfo;

	if (instanceCreateInfo.enabledExtensionNames.size())
	{
		_createInfo.enabledExtensionNames = pvrvk::Extensions::filterInstanceExtensions(instanceCreateInfo.enabledExtensionNames.data(),
		                                    static_cast<uint32_t>(instanceCreateInfo.enabledExtensionNames.size()));

		enabledExtensions.resize(_createInfo.enabledExtensionNames.size());
		for (uint32_t i = 0; i < _createInfo.enabledExtensionNames.size(); ++i)
		{
			enabledExtensions[i] = _createInfo.enabledExtensionNames[i].c_str();
		}

		instanceCreateInfoVk.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
		instanceCreateInfoVk.ppEnabledExtensionNames = enabledExtensions.data();

		Log(LogLevel::Information, "Enabled Instance Extensions:");
		for (uint32_t i = 0; i < enabledExtensions.size(); ++i)
		{
			Log(LogLevel::Information, "\t%s", enabledExtensions[i]);
		}
	}

	if (instanceCreateInfo.enabledLayerNames.size())
	{
		std::vector<LayerProperties> layerProp;
		pvrvk::Layers::Instance::enumerateInstanceLayers(layerProp);
		_createInfo.enabledLayerNames = pvrvk::Layers::filterLayers(layerProp, instanceCreateInfo.enabledLayerNames.data(),
		                                static_cast<uint32_t>(instanceCreateInfo.enabledLayerNames.size()));

		enableLayers.resize(_createInfo.enabledLayerNames.size());
		for (uint32_t i = 0; i < _createInfo.enabledLayerNames.size(); ++i)
		{
			enableLayers[i] = _createInfo.enabledLayerNames[i].c_str();
		}

		instanceCreateInfoVk.enabledLayerCount = static_cast<uint32_t>(enableLayers.size());
		instanceCreateInfoVk.ppEnabledLayerNames = enableLayers.data();

		Log(LogLevel::Information, "Enabled Instance Layers:");
		for (uint32_t i = 0; i < enableLayers.size(); ++i)
		{
			Log(LogLevel::Information, "\t%s", enableLayers[i]);
		}
	}

	if (vk::CreateInstance(&instanceCreateInfoVk, nullptr, &_instance) != VkResult::e_SUCCESS)
	{
		Log("Failed to create Vulkan instance");
		return false;
	}

	vk::initVulkanInstance(_instance);
#ifdef DEBUG
	// If supported enable the use of VK_EXT_debug_report
	initDebugCallbacks();
#endif
	uint32_t numPhysicalDevices = 0;
	vk::EnumeratePhysicalDevices(getNativeObject(), &numPhysicalDevices, nullptr);
	Log(LogLevel::Information, "Number of Vulkan Physical devices: [%d]", numPhysicalDevices);
	std::vector<VkPhysicalDevice> vkPhyscialDevice(numPhysicalDevices);
	_physicalDevice.resize(numPhysicalDevices);
	if (vk::EnumeratePhysicalDevices(getNativeObject(),
	                                 &numPhysicalDevices, vkPhyscialDevice.data()) != VkResult::e_SUCCESS)
	{
		return false;
	}

	for (uint32_t i = 0; i < numPhysicalDevices; ++i)
	{
		_physicalDevice[i] = PhysicalDevice_::createNew();
		if (!_physicalDevice[i]->init(getWeakReference(), vkPhyscialDevice[i]))
		{
			_physicalDevice[i].reset();
			return false;
		}
	}
	return true;
}
class InstanceHelperFactory_
{
public:
	static Instance createVkInstance(const InstanceCreateInfo& createInfo);
};

pvrvk::Instance InstanceHelperFactory_::createVkInstance(const InstanceCreateInfo& createInfo)
{
	Instance instance =  impl::Instance_::createNew();
	if (!instance->init(createInfo))
	{
		instance.reset();
	}
	return instance;
}
}//namespace impl

Instance createInstance(const InstanceCreateInfo& createInfo)
{
	return impl::InstanceHelperFactory_::createVkInstance(createInfo);
}
}// namespace pvrvk

//!\endcond
