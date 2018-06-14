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
#include "PVRVk/DisplayVk.h"
#include "PVRVk/DisplayModeVk.h"
namespace pvrvk {
namespace impl {
bool PhysicalDevice_::getSurfaceSupport(uint32_t queueFamilyIndex, Surface surface) const
{
	VkBool32 supportsWsi = false;
	_instance->getVkBindings().vkGetPhysicalDeviceSurfaceSupportKHR(getVkHandle(), queueFamilyIndex, surface->getVkHandle(), &supportsWsi);
	return supportsWsi;
}

const std::vector<Display>& PhysicalDevice_::getDisplays() const
{
	return _displays;
}

Display PhysicalDevice_::getDisplay(uint32_t id)
{
	return _displays[id];
}

const Display& PhysicalDevice_::getDisplay(uint32_t id) const
{
	return _displays[id];
}

FormatProperties PhysicalDevice_::getFormatProperties(Format format)
{
	if (format == Format::e_UNDEFINED)
	{
		return FormatProperties();
	}

	FormatProperties formatProperties;

	// use VK_KHR_get_physical_device_properties2 if the extension is supported
	if (_instance->isInstanceExtensionEnabled(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
	{
		VkFormatProperties2KHR formatProperties2KHR = {};
		formatProperties2KHR.sType = static_cast<VkStructureType>(StructureType::e_FORMAT_PROPERTIES_2_KHR);

		_instance->getVkBindings().vkGetPhysicalDeviceFormatProperties2KHR(getVkHandle(), static_cast<VkFormat>(format), &formatProperties2KHR);
		memcpy(&formatProperties, &formatProperties2KHR.formatProperties, sizeof(formatProperties2KHR.formatProperties));
	}
	else
	{
		_instance->getVkBindings().vkGetPhysicalDeviceFormatProperties(_vkHandle, static_cast<VkFormat>(format), reinterpret_cast<VkFormatProperties*>(&formatProperties));
	}

	return formatProperties;
}

SurfaceCapabilitiesKHR PhysicalDevice_::getSurfaceCapabilities(const Surface& surface) const
{
	SurfaceCapabilitiesKHR surfaceCapabilities;
	if (_instance->getVkBindings().vkGetPhysicalDeviceSurfaceCapabilitiesKHR == nullptr)
	{
		throw ErrorValidationFailedEXT("GetPhysicalDeviceSurfaceCapabilitiesKHR does not exist. Cannot get surface capabilities.");
	}
	_instance->getVkBindings().vkGetPhysicalDeviceSurfaceCapabilitiesKHR(getVkHandle(), surface->getVkHandle(), reinterpret_cast<VkSurfaceCapabilitiesKHR*>(&surfaceCapabilities));
	return surfaceCapabilities;
}

std::vector<ExtensionProperties>& PhysicalDevice_::enumerateDeviceExtensionsProperties()
{
	if (_deviceExtensions.size())
	{
		return _deviceExtensions;
	}
	uint32_t numItems = 0;
	_instance->getVkBindings().vkEnumerateDeviceExtensionProperties(getVkHandle(), nullptr, &numItems, nullptr);

	_deviceExtensions.resize(numItems);
	_instance->getVkBindings().vkEnumerateDeviceExtensionProperties(getVkHandle(), nullptr, &numItems, reinterpret_cast<VkExtensionProperties*>(_deviceExtensions.data()));
	return _deviceExtensions;
}

DisplayMode PhysicalDevice_::createDisplayMode(pvrvk::Display& display, const pvrvk::DisplayModeCreateInfo& displayModeCreateInfo)
{
	return DisplayMode_::createNew(getWeakReference(), display, displayModeCreateInfo);
}

Display PhysicalDevice_::getDisplayPlaneProperties(uint32_t displayPlaneIndex, uint32_t& currentStackIndex)
{
	updateDisplayPlaneProperties();
	currentStackIndex = _displayPlaneProperties[displayPlaneIndex].getCurrentStackIndex();
	VkDisplayKHR displayVk = _displayPlaneProperties[displayPlaneIndex].getCurrentDisplay();

	for (uint32_t i = 0; i < getNumDisplays(); ++i)
	{
		if (_displays[i]->getVkHandle() == displayVk)
		{
			return _displays[i];
		}
	}
	return Display();
}

DisplayPlaneCapabilitiesKHR PhysicalDevice_::getDisplayPlaneCapabilities(DisplayMode& mode, uint32_t planeIndex)
{
	DisplayPlaneCapabilitiesKHR capabilities;
	getInstance()->getVkBindings().vkGetDisplayPlaneCapabilitiesKHR(getVkHandle(), mode->getVkHandle(), planeIndex, reinterpret_cast<VkDisplayPlaneCapabilitiesKHR*>(&capabilities));
	return capabilities;
}

std::vector<Display> PhysicalDevice_::getDisplayPlaneSupportedDisplays(uint32_t planeIndex)
{
	std::vector<VkDisplayKHR> supportedDisplaysVk;

	uint32_t numSupportedDisplays = 0;
	getInstance()->getVkBindings().vkGetDisplayPlaneSupportedDisplaysKHR(getVkHandle(), planeIndex, &numSupportedDisplays, nullptr);
	supportedDisplaysVk.resize(numSupportedDisplays);

	getInstance()->getVkBindings().vkGetDisplayPlaneSupportedDisplaysKHR(getVkHandle(), planeIndex, &numSupportedDisplays, supportedDisplaysVk.data());

	std::vector<Display> supportedDisplays;
	for (uint32_t i = 0; i < numSupportedDisplays; ++i)
	{
		for (uint32_t j = 0; j < getNumDisplays(); ++j)
		{
			if (_displays[j]->getVkHandle() == supportedDisplaysVk[i])
			{
				supportedDisplays.push_back(_displays[j]);
			}
		}
	}
	return supportedDisplays;
}

void PhysicalDevice_::updateDisplayPlaneProperties()
{
	uint32_t numProperties = 0;
	getInstance()->getVkBindings().vkGetPhysicalDeviceDisplayPlanePropertiesKHR(getVkHandle(), &numProperties, NULL);

	_displayPlaneProperties.clear();
	_displayPlaneProperties.resize(numProperties);
	getInstance()->getVkBindings().vkGetPhysicalDeviceDisplayPlanePropertiesKHR(
		getVkHandle(), &numProperties, reinterpret_cast<VkDisplayPlanePropertiesKHR*>(_displayPlaneProperties.data()));
}

const ImageFormatProperties PhysicalDevice_::getImageFormatProperties(Format format, ImageType imageType, ImageTiling tiling, ImageUsageFlags usage, ImageCreateFlags flags)
{
	ImageFormatProperties imageProperties = {};

	VkResult result = VK_SUCCESS;

	// use VK_KHR_get_physical_device_properties2 if the extension is supported
	if (getInstance()->isInstanceExtensionEnabled(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
	{
		VkPhysicalDeviceImageFormatInfo2KHR imageFormatInfo = {};
		imageFormatInfo.sType = static_cast<VkStructureType>(StructureType::e_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2_KHR);
		imageFormatInfo.flags = static_cast<VkImageCreateFlags>(flags);
		imageFormatInfo.format = static_cast<VkFormat>(format);
		imageFormatInfo.tiling = static_cast<VkImageTiling>(tiling);
		imageFormatInfo.type = static_cast<VkImageType>(imageType);
		imageFormatInfo.usage = static_cast<VkImageUsageFlags>(usage);

		VkImageFormatProperties2KHR imageFormatProperties = {};
		imageFormatProperties.sType = static_cast<VkStructureType>(StructureType::e_IMAGE_FORMAT_PROPERTIES_2_KHR);

		result = getInstance()->getVkBindings().vkGetPhysicalDeviceImageFormatProperties2KHR(getVkHandle(), &imageFormatInfo, &imageFormatProperties);
		memcpy(&imageProperties, &imageFormatProperties.imageFormatProperties, sizeof(imageFormatProperties.imageFormatProperties));
	}
	else
	{
		result = getInstance()->getVkBindings().vkGetPhysicalDeviceImageFormatProperties(getVkHandle(), static_cast<VkFormat>(format), static_cast<VkImageType>(imageType),
			static_cast<VkImageTiling>(tiling), static_cast<VkImageUsageFlags>(usage), static_cast<VkImageCreateFlags>(flags),
			reinterpret_cast<VkImageFormatProperties*>(&imageProperties));
	}

	if (result != VK_SUCCESS)
	{
		Log("The combination of paramters used is not supported by the implementation for use in vkCreateImage (Format: %s, ImageType: %s, Tiling: %s, usage: %s, flags: %s, "
			"imageProperties: (MaxExtent: (width, height, depth): (%u, %u, %u), MaxMipLevels: %u, MaxArrayLayers: %u, SampleCounts: %s, MaxResourceSize: %u)",
			pvrvk::to_string(format).c_str(), pvrvk::to_string(imageType).c_str(), pvrvk::to_string(tiling).c_str(), pvrvk::to_string(usage).c_str(), pvrvk::to_string(flags).c_str(),
			imageProperties.getMaxExtent().getWidth(), imageProperties.getMaxExtent().getHeight(), imageProperties.getMaxExtent().getDepth(), imageProperties.getMaxMipLevels(),
			imageProperties.getMaxArrayLayers(), pvrvk::to_string(imageProperties.getSampleCounts()).c_str(), imageProperties.getMaxResourceSize());
		throw ErrorValidationFailedEXT("The combination of paramters used is not supported by the implementation for use in vkCreateImage");
	}

	return imageProperties;
}

const std::vector<SparseImageFormatProperties> PhysicalDevice_::getSparseImageFormatProperties(
	Format format, ImageType imageType, SampleCountFlags sampleCount, ImageUsageFlags usage, ImageTiling tiling)
{
	std::vector<SparseImageFormatProperties> sparseImageFormatProperties = {};
	uint32_t propertiesCount;

	// use VK_KHR_get_physical_device_properties2 if the extension is supported
	if (getInstance()->isInstanceExtensionEnabled(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
	{
		VkPhysicalDeviceSparseImageFormatInfo2KHR sparseImageFormatInfo = {};
		sparseImageFormatInfo.sType = static_cast<VkStructureType>(StructureType::e_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2_KHR);
		sparseImageFormatInfo.format = static_cast<VkFormat>(format);
		sparseImageFormatInfo.type = static_cast<VkImageType>(imageType);
		sparseImageFormatInfo.samples = static_cast<VkSampleCountFlagBits>(sampleCount);
		sparseImageFormatInfo.usage = static_cast<VkImageUsageFlags>(usage);
		sparseImageFormatInfo.tiling = static_cast<VkImageTiling>(tiling);

		getInstance()->getVkBindings().vkGetPhysicalDeviceSparseImageFormatProperties2KHR(getVkHandle(), &sparseImageFormatInfo, &propertiesCount, nullptr);
		sparseImageFormatProperties.resize(propertiesCount);

		std::vector<VkSparseImageFormatProperties2KHR> properties(propertiesCount);
		for (uint32_t i = 0; i < propertiesCount; i++)
		{
			properties[i].sType = static_cast<VkStructureType>(StructureType::e_SPARSE_IMAGE_FORMAT_PROPERTIES_2_KHR);
		}

		getInstance()->getVkBindings().vkGetPhysicalDeviceSparseImageFormatProperties2KHR(getVkHandle(), &sparseImageFormatInfo, &propertiesCount, properties.data());
		for (uint32_t i = 0; i < propertiesCount; i++)
		{
			sparseImageFormatProperties[i] = *reinterpret_cast<pvrvk::SparseImageFormatProperties*>(&properties[i].properties);
		}
	}
	else
	{
		// determine the properties count
		getInstance()->getVkBindings().vkGetPhysicalDeviceSparseImageFormatProperties(getVkHandle(), static_cast<VkFormat>(format), static_cast<VkImageType>(imageType),
			static_cast<VkSampleCountFlagBits>(sampleCount), static_cast<VkImageUsageFlags>(usage), static_cast<VkImageTiling>(tiling), &propertiesCount, nullptr);
		sparseImageFormatProperties.resize(propertiesCount);

		// retrieve the sparse image format properties
		getInstance()->getVkBindings().vkGetPhysicalDeviceSparseImageFormatProperties(getVkHandle(), static_cast<VkFormat>(format), static_cast<VkImageType>(imageType),
			static_cast<VkSampleCountFlagBits>(sampleCount), static_cast<VkImageUsageFlags>(usage), static_cast<VkImageTiling>(tiling), &propertiesCount,
			reinterpret_cast<VkSparseImageFormatProperties*>(sparseImageFormatProperties.data()));
	}

	return sparseImageFormatProperties;
}

PhysicalDevice_::PhysicalDevice_(InstanceWeakPtr instance, const VkPhysicalDevice& vkPhysicalDevice) : InstanceObjectHandle(instance, vkPhysicalDevice)
{
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;

	// use VK_KHR_get_physical_device_properties2 if the extension is supported
	if (instance->isInstanceExtensionEnabled(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
	{
		VkPhysicalDeviceMemoryProperties2KHR memoryProperties = {};
		memoryProperties.sType = static_cast<VkStructureType>(StructureType::e_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2_KHR);

		_instance->getVkBindings().vkGetPhysicalDeviceMemoryProperties2KHR(getVkHandle(), &memoryProperties);
		memcpy(&physicalDeviceMemoryProperties, &memoryProperties.memoryProperties, sizeof(memoryProperties.memoryProperties));
	}
	else
	{
		_instance->getVkBindings().vkGetPhysicalDeviceMemoryProperties(getVkHandle(), &physicalDeviceMemoryProperties);
	}

	memcpy(&_deviceMemoryProperties, &physicalDeviceMemoryProperties, sizeof(physicalDeviceMemoryProperties));

	uint32_t numQueueFamilies = 0;

	// use VK_KHR_get_physical_device_properties2 if the extension is supported
	if (instance->isInstanceExtensionEnabled(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
	{
		_instance->getVkBindings().vkGetPhysicalDeviceQueueFamilyProperties2KHR(getVkHandle(), &numQueueFamilies, nullptr);
		_queueFamilyPropeties.resize(numQueueFamilies);

		std::vector<VkQueueFamilyProperties2KHR> queueFamilyProperties(numQueueFamilies);
		for (uint32_t i = 0; i < numQueueFamilies; i++)
		{
			queueFamilyProperties[i].sType = static_cast<VkStructureType>(StructureType::e_QUEUE_FAMILY_PROPERTIES_2_KHR);
		}

		_instance->getVkBindings().vkGetPhysicalDeviceQueueFamilyProperties2KHR(getVkHandle(), &numQueueFamilies, queueFamilyProperties.data());

		for (uint32_t i = 0; i < numQueueFamilies; ++i)
		{
			_queueFamilyPropeties[i] = *reinterpret_cast<pvrvk::QueueFamilyProperties*>(&queueFamilyProperties[i].queueFamilyProperties);
		}
	}
	else
	{
		_instance->getVkBindings().vkGetPhysicalDeviceQueueFamilyProperties(getVkHandle(), &numQueueFamilies, nullptr);
		_queueFamilyPropeties.resize(numQueueFamilies);
		_instance->getVkBindings().vkGetPhysicalDeviceQueueFamilyProperties(getVkHandle(), &numQueueFamilies, reinterpret_cast<VkQueueFamilyProperties*>(_queueFamilyPropeties.data()));
	}

	// use VK_KHR_get_physical_device_properties2 if the extension is supported
	if (_instance->isInstanceExtensionEnabled(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
	{
		{
			VkPhysicalDeviceFeatures2KHR deviceFeatures = {};
			deviceFeatures.sType = static_cast<VkStructureType>(StructureType::e_PHYSICAL_DEVICE_FEATURES_2_KHR);

			_instance->getVkBindings().vkGetPhysicalDeviceFeatures2KHR(getVkHandle(), &deviceFeatures);
			memcpy(&_deviceFeatures, &deviceFeatures.features, sizeof(deviceFeatures.features));
		}

		{
			VkPhysicalDeviceProperties2KHR deviceProperties = {};
			deviceProperties.sType = static_cast<VkStructureType>(StructureType::e_PHYSICAL_DEVICE_PROPERTIES_2_KHR);

			_instance->getVkBindings().vkGetPhysicalDeviceProperties2KHR(getVkHandle(), &deviceProperties);
			memcpy(&_deviceProperties, &deviceProperties.properties, sizeof(deviceProperties.properties));
		}
	}
	else
	{
		_instance->getVkBindings().vkGetPhysicalDeviceFeatures(getVkHandle(), (VkPhysicalDeviceFeatures*)&_deviceFeatures);
		_instance->getVkBindings().vkGetPhysicalDeviceProperties(getVkHandle(), (VkPhysicalDeviceProperties*)&_deviceProperties);
	}

	if (_instance->isInstanceExtensionEnabled(VK_KHR_DISPLAY_EXTENSION_NAME))
	{
		{
			uint32_t numProperties = 0;
			getInstance()->getVkBindings().vkGetPhysicalDeviceDisplayPropertiesKHR(getVkHandle(), &numProperties, NULL);

			std::vector<VkDisplayPropertiesKHR> _displayProperties;
			_displayProperties.resize(numProperties);
			getInstance()->getVkBindings().vkGetPhysicalDeviceDisplayPropertiesKHR(getVkHandle(), &numProperties, _displayProperties.data());

			for (uint32_t i = 0; i < numProperties; ++i)
			{
				DisplayPropertiesKHR displayProperties = *reinterpret_cast<DisplayPropertiesKHR*>(&_displayProperties[i]);
				_displays.push_back(Display_::createNew(getWeakReference(), displayProperties));
			}
		}

		{
			updateDisplayPlaneProperties();
		}
	}
}

uint32_t PhysicalDevice_::getMemoryTypeIndex(uint32_t allowedMemoryTypeBits, pvrvk::MemoryPropertyFlags requiredMemoryProperties, pvrvk::MemoryPropertyFlags& usedMemoryProperties) const
{
	const uint32_t memoryCount = _deviceMemoryProperties.getMemoryTypeCount();
	for (uint32_t memoryIndex = 0; memoryIndex < memoryCount; ++memoryIndex)
	{
		const uint32_t memoryTypeBits = (1 << memoryIndex);
		const bool isRequiredMemoryType = static_cast<uint32_t>(allowedMemoryTypeBits & memoryTypeBits) != 0;

		usedMemoryProperties = _deviceMemoryProperties.getMemoryTypes()[memoryIndex].getPropertyFlags();
		const bool hasRequiredProperties = static_cast<uint32_t>(usedMemoryProperties & requiredMemoryProperties) == requiredMemoryProperties;

		if (isRequiredMemoryType && hasRequiredProperties)
		{
			return static_cast<uint32_t>(memoryIndex);
		}
	}

	// failed to find memory type
	return static_cast<uint32_t>(-1);
}

Device PhysicalDevice_::createDevice(const DeviceCreateInfo& deviceCreateInfo)
{
	return Device_::createNew(getWeakReference(), deviceCreateInfo);
}
} // namespace impl
} // namespace pvrvk
