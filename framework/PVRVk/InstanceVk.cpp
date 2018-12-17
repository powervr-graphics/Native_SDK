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
class InstanceHelperFactory_
{
public:
	static Instance createVkInstance(const InstanceCreateInfo& createInfo);
};

const std::vector<PhysicalDevice>& Instance_::getPhysicalDevices() const
{
	return _physicalDevices;
}

PhysicalDevice& Instance_::getPhysicalDevice(uint32_t id)
{
	return _physicalDevices[id];
}

const PhysicalDevice& Instance_::getPhysicalDevice(uint32_t id) const
{
	return _physicalDevices[id];
}

pvrvk::impl::Instance_::Instance_(const InstanceCreateInfo& instanceCreateInfo) : ObjectHandle()
{
	VkBindings vkBindings;
	if (!initVkBindings(&vkBindings))
	{
		throw pvrvk::ErrorInitializationFailed("We were unable to retrieve Vulkan bindings");
	}

	_createInfo = instanceCreateInfo;

	VkApplicationInfo appInfo = {};
	if (_createInfo.getApplicationInfo())
	{
		appInfo.sType = static_cast<VkStructureType>(StructureType::e_APPLICATION_INFO);
		appInfo.apiVersion = _createInfo.getApplicationInfo()->getApiVersion();
		appInfo.pApplicationName = _createInfo.getApplicationInfo()->getApplicationName().c_str();
		appInfo.applicationVersion = _createInfo.getApplicationInfo()->getApplicationVersion();
		appInfo.pEngineName = _createInfo.getApplicationInfo()->getEngineName().c_str();
		appInfo.engineVersion = _createInfo.getApplicationInfo()->getEngineVersion();
	}

	std::vector<const char*> enabledExtensions;
	std::vector<const char*> enableLayers;

	VkInstanceCreateInfo instanceCreateInfoVk = {};
	instanceCreateInfoVk.sType = static_cast<VkStructureType>(StructureType::e_INSTANCE_CREATE_INFO);
	instanceCreateInfoVk.pApplicationInfo = &appInfo;

	if (instanceCreateInfo.getNumEnabledExtensionNames())
	{
		enabledExtensions.resize(_createInfo.getNumEnabledExtensionNames());
		for (uint32_t i = 0; i < _createInfo.getNumEnabledExtensionNames(); ++i)
		{
			enabledExtensions[i] = _createInfo.getEnabledExtensionName(i).c_str();
		}

		instanceCreateInfoVk.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
		instanceCreateInfoVk.ppEnabledExtensionNames = enabledExtensions.data();
	}

	if (instanceCreateInfo.getNumEnabledLayerNames())
	{
		enableLayers.resize(_createInfo.getNumEnabledLayerNames());
		for (uint32_t i = 0; i < _createInfo.getNumEnabledLayerNames(); ++i)
		{
			enableLayers[i] = _createInfo.getEnabledLayerName(i).c_str();
		}

		instanceCreateInfoVk.enabledLayerCount = static_cast<uint32_t>(enableLayers.size());
		instanceCreateInfoVk.ppEnabledLayerNames = enableLayers.data();
	}

	vkThrowIfFailed(vkBindings.vkCreateInstance(&instanceCreateInfoVk, nullptr, &_vkHandle), "Instance Constructor");

	// Retrieve the function pointers for the functions taking a VkInstance as their dispatchable handles.
	initVkInstanceBindings(_vkHandle, &_vkBindings, vkBindings.vkGetInstanceProcAddr);

	// Enumerate the list of installed physical devices
	uint32_t numPhysicalDevices = 0;
	getVkBindings().vkEnumeratePhysicalDevices(getVkHandle(), &numPhysicalDevices, nullptr);

	// Retreive the numPhysicalDevices installed on the system
	std::vector<VkPhysicalDevice> vkPhyscialDevice(numPhysicalDevices);
	vkThrowIfFailed(getVkBindings().vkEnumeratePhysicalDevices(getVkHandle(), &numPhysicalDevices, vkPhyscialDevice.data()));

	for (uint32_t i = 0; i < numPhysicalDevices; ++i)
	{
		_physicalDevices.push_back(PhysicalDevice_::createNew(getWeakReference(), vkPhyscialDevice[i]));
	}
}

pvrvk::Instance InstanceHelperFactory_::createVkInstance(const InstanceCreateInfo& createInfo)
{
	return impl::Instance_::createNew(createInfo);
}
} // namespace impl

Instance createInstance(const InstanceCreateInfo& createInfo)
{
	return impl::InstanceHelperFactory_::createVkInstance(createInfo);
}
} // namespace pvrvk
  //!\endcond
