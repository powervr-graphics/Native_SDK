/*!
\brief Function definitions for the Physical Device class
\file PVRVk/PhysicalDeviceVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "PVRVk/PhysicalDeviceVk.h"
#include "PVRVk/SurfaceVk.h"
#include "PVRVk/InstanceVk.h"
#include "PVRVk/DeviceVk.h"
namespace pvrvk {
namespace impl {
void PhysicalDevice_::getPresentationQueueFamily(
  Surface surface, std::vector<VkBool32>& queueFamily) const
{
	queueFamily.resize(_queueFamilyPropeties.size());
	for (uint32_t i = 0; i < _queueFamilyPropeties.size(); ++i)
	{
		vk::GetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, surface->getNativeObject(), &queueFamily[i]);
	}
}

SurfaceCapabilitiesKHR PhysicalDevice_::getSurfaceCapabilities(const Surface& surface)
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	assertion(vk::GetPhysicalDeviceSurfaceCapabilitiesKHR != nullptr);
	vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, surface->getNativeObject(), &surfaceCapabilities);
	return surfaceCapabilities;
}

bool PhysicalDevice_::init(InstanceWeakPtr instance, const VkPhysicalDevice& vkPhysicalDevice)
{
	_instance = instance;
	_physicalDevice = vkPhysicalDevice;
	VkPhysicalDeviceMemoryProperties vkDeviceMemProps;
	vk::GetPhysicalDeviceMemoryProperties(_physicalDevice, &vkDeviceMemProps);

	// take a copy of the device memory properties
	_deviceMemProperties.memoryTypeCount = vkDeviceMemProps.memoryTypeCount;
	for (uint32_t i = 0; i < _deviceMemProperties.memoryTypeCount; i++)
	{
		_deviceMemProperties.memoryTypes[i].heapIndex = vkDeviceMemProps.memoryTypes[i].heapIndex;
		_deviceMemProperties.memoryTypes[i].propertyFlags = vkDeviceMemProps.memoryTypes[i].propertyFlags;
	}
	_deviceMemProperties.memoryHeapCount = vkDeviceMemProps.memoryHeapCount;
	for (uint32_t i = 0; i < _deviceMemProperties.memoryHeapCount; i++)
	{
		_deviceMemProperties.memoryHeaps[i].size = vkDeviceMemProps.memoryHeaps[i].size;
		_deviceMemProperties.memoryHeaps[i].flags = vkDeviceMemProps.memoryHeaps[i].flags;
	}

	uint32_t numQueueFamilies = 0;
	vk::GetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &numQueueFamilies, nullptr);
	_queueFamilyPropeties.resize(numQueueFamilies);

	vk::GetPhysicalDeviceQueueFamilyProperties(_physicalDevice,
	    &numQueueFamilies, (VkQueueFamilyProperties*)&_queueFamilyPropeties[0]);

	// use vkGetPhysicalDeviceProperties2KHR if the extension is supported
	if (instance->isInstanceExtensionEnabled("VK_KHR_get_physical_device_properties2") &&
	    vk::GetPhysicalDeviceProperties2KHR != nullptr)
	{
		VkPhysicalDeviceProperties2KHR deviceProperties = {};
		deviceProperties.sType = VkStructureType::e_PHYSICAL_DEVICE_PROPERTIES_2_KHR;

		vk::GetPhysicalDeviceProperties2KHR(_physicalDevice, &deviceProperties);
		memcpy(&_deviceProperties, &deviceProperties.properties, sizeof(deviceProperties.properties));

	}
	else
	{
		vk::GetPhysicalDeviceProperties(_physicalDevice, (VkPhysicalDeviceProperties*)&_deviceProperties);
	}

	vk::GetPhysicalDeviceFeatures(_physicalDevice, (VkPhysicalDeviceFeatures*)&_deviceFeatures);
	return true;
}

Device PhysicalDevice_::createDevice(const DeviceCreateInfo& deviceCreateInfo)
{
	Device device = Device_::createNew(getWeakReference());
	if (!device->init(deviceCreateInfo))
	{
		Log("failed to create GpuDevice");
		device.reset();
	}
	return device;
}
}
}// namespace pvrvk
