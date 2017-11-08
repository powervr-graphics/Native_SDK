/*!
\brief The Physical Device class
\file PVRVk/PhysicalDeviceVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/ForwardDecObjectsVk.h"
#include "PVRVk/TypesVk.h"
#include "PVRVk/BindingsVk.h"
#include "PVRVk/ErrorsVk.h"

namespace pvrvk {
namespace impl {

/// <summary>The representation of an entire actual, physical GPU device (as opposed to Device,
/// which is a local, logical part of it). A Physical device is "determined", or "found", or
/// "enumerated", (while a logical device is "created"). You can use the physical device to
/// create logical Devices, determine Extensions etc. See Vulkan spec. </summary>
class PhysicalDevice_ : public EmbeddedRefCount<PhysicalDevice_>
{
public:
	DECLARE_NO_COPY_SEMANTICS(PhysicalDevice_)

	/// <summary>Get Vulkan handle (const)</summary>
	/// <returns>Returns VkPhyscialDevice</returns>
	const VkPhysicalDevice& getNativeObject()const
	{
		return _physicalDevice;
	}

	/// <summary>Get Vulkan handle (const)</summary>
	/// <returns>Returns VkPhyscialDevice</returns>
	const PhysicalDeviceProperties& getProperties()const
	{
		return _deviceProperties;
	}


	/// <summary>Get supported memory properties (const)</summary>
	/// <returns>PhysicalDeviceMemoryProperties</returns>
	const PhysicalDeviceMemoryProperties& getMemoryProperties()const
	{
		return _deviceMemProperties;
	}

	/// <summary>Get supported presentation queue families for the given surface</summary>
	/// <param name="surface">The surface for presenation</param>
	/// <param name="outQueueFamily">std::vector<VkBool32> of Supported queue families </param>
	void getPresentationQueueFamily(Surface surface, std::vector<VkBool32>& outQueueFamily)const;

	/// <summary>Get format properties</summary>
	/// <param name="format">Format</param>
	/// <returns>FormatProperties</returns>
	FormatProperties getFormatProperties(const VkFormat& format)
	{
		if (format == VkFormat::e_UNDEFINED) { return VkFormatProperties(); }
		VkFormatProperties vkFmtProp;
		vk::GetPhysicalDeviceFormatProperties(_physicalDevice, format, &vkFmtProp);
		VkFormatProperties fmtProps; memcpy(&fmtProps, &vkFmtProp, sizeof(vkFmtProp));
		return fmtProps;
	}

	/// <summary>Get surface capabilities for a surface created using this physical device</summary>
	/// <param name="surface">The surface to retrieve capabilities for</param>
	/// <returns>SurfaceCapabilities</returns>
	SurfaceCapabilitiesKHR getSurfaceCapabilities(const Surface& surface);

	/// <summary>Get This physical device features</summary>
	/// <returns>PhysicalDeviceFeatures</returns>
	const PhysicalDeviceFeatures& getFeatures()const
	{
		return _deviceFeatures;
	}

	/// <summary>Get instance (const)</summary>
	/// <returns>Instance</returns>
	const InstanceWeakPtr getInstance()const { return _instance; }

	/// <summary>Get instance</summary>
	/// <returns>Instance</returns>
	InstanceWeakPtr getInstance() { return _instance; }

	/// <summary> create gpu device. </summary>
	/// <param name="deviceCreateInfo">Device create info</param>
	/// <returns>Device</returns>
	Device createDevice(const DeviceCreateInfo& deviceCreateInfo);

	/// <summary>Get queue family properties</summary>
	/// <returns>std::vector<QueueFamilyProperties></returns>
	const std::vector<QueueFamilyProperties>& getQueueFamilyProperties()const
	{
		return _queueFamilyPropeties;
	}

	/// <summary>Enumerate device extensions properties</summary>
	/// <returns>std::vector<VkExtensionProperties></returns>
	std::vector<ExtensionProperties>& enumerateDeviceExtensionsProperties()
	{
		if (_deviceExtensions.size())
		{
			return _deviceExtensions;
		}
		uint32_t numItems = 0;
		vk::EnumerateDeviceExtensionProperties(getNativeObject(), nullptr, &numItems, nullptr);

		_deviceExtensions.resize(numItems);
		vk::EnumerateDeviceExtensionProperties(getNativeObject(), nullptr, &numItems, _deviceExtensions.data());
		return _deviceExtensions;
	}

private:
	friend class ::pvrvk::impl::Instance_;
	friend class ::pvrvk::EmbeddedRefCount<PhysicalDevice_>;
	PhysicalDevice_()  : _physicalDevice(VK_NULL_HANDLE)
	{}

	static PhysicalDevice createNew()
	{
		return EmbeddedRefCount<PhysicalDevice_>::createNew();
	}
	bool init(InstanceWeakPtr instance, const VkPhysicalDevice& vkPhysicalDevice);
	void destroyObject()
	{
		_physicalDevice = VK_NULL_HANDLE;
		_instance.reset();
		_supportedFormats.clear();
		_deviceExtensions.clear();
	}
	std::vector<QueueFamilyProperties>      _queueFamilyPropeties;
	PhysicalDeviceProperties _deviceProperties;
	PhysicalDeviceMemoryProperties _deviceMemProperties;
	PhysicalDeviceFeatures _deviceFeatures;
	std::vector<FormatProperties> _supportedFormats;
	std::vector<ExtensionProperties> _deviceExtensions;
	uint32_t                          _graphicsQueueIndex;
	uint32_t                          _universalQueueFamilyId;
	VkPhysicalDevice                  _physicalDevice;
	InstanceWeakPtr                   _instance;
};
}
}// namespace pvrvk
