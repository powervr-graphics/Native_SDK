/*!
\brief Defines a simple interface for a Vulkan object.
\file PVRVk/ObjectHandleVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/TypesVk.h"
#include "PVRVk/ForwardDecObjectsVk.h"

/// <summary>Main PowerVR Framework Namespace</summary>
namespace pvrvk {
/// <summary>Contains internal objects and wrapped versions of the PVRVk module</summary>
namespace impl {
/// <summary>Defines a simple interface for a Vulkan object.</summary>
template<class VkHandle>
class ObjectHandle
{
public:
	DECLARE_NO_COPY_SEMANTICS(ObjectHandle)

	/// <summary>Get vulkan object (const)</summary>
	/// <returns>Returns the templated 'HandleType'</returns>
	const VkHandle& getVkHandle() const
	{
		return _vkHandle;
	}

protected:
	/// <summary>The Vulkan object handle representing the Vulkan object at an API level.</summary>
	VkHandle _vkHandle;

	/// <summary>default Constructor for an object handle</summary>
	ObjectHandle() : _vkHandle(VK_NULL_HANDLE) {}
	/// <summary>Constructor for an object handle initialising the Vulkan object handle</summary>
	/// <param name="handle">The Vulkan object handle given to the Vulkan object.</param>
	explicit ObjectHandle(const VkHandle& handle) : _vkHandle(handle) {}
};

/// <summary>Defines a simple interface for a Vulkan which holds a reference to a Vulkan instance.</summary>
template<class VkHandle>
class InstanceObjectHandle : public ObjectHandle<VkHandle>
{
public:
	DECLARE_NO_COPY_SEMANTICS(InstanceObjectHandle)

	/// <summary>Get instance (const)</summary>
	/// <returns>Instance</returns>
	const InstanceWeakPtr getInstance() const
	{
		return _instance;
	}

	/// <summary>Get instance</summary>
	/// <returns>Instance</returns>
	InstanceWeakPtr getInstance()
	{
		return _instance;
	}

protected:
	/// <summary>The instance which was used to create this InstanceObject</summary>
	InstanceWeakPtr _instance;

	/// <summary>default Constructor for an instance object handle</summary>
	InstanceObjectHandle() : ObjectHandle<VkHandle>() {}
	/// <summary>Constructor for an instance object handle initialising the instance</summary>
	/// <param name="instance">The Vulkan instance used to create the Vulkan object.</param>
	explicit InstanceObjectHandle(const InstanceWeakPtr& instance) : ObjectHandle<VkHandle>(), _instance(instance) {}
	/// <summary>Constructor for an instance object handle initialising the Vulkan object handle</summary>
	/// <param name="handle">The Vulkan object handle given to the Vulkan object.</param>
	explicit InstanceObjectHandle(const VkHandle& handle) : ObjectHandle<VkHandle>(handle) {}
	/// <summary>Constructor for an instance object handle initialising the instance and Vulkan object handle</summary>
	/// <param name="instance">The Vulkan instance used to create the Vulkan object.</param>
	/// <param name="handle">The Vulkan object handle given to the Vulkan object.</param>
	InstanceObjectHandle(const InstanceWeakPtr& instance, const VkHandle& handle) : ObjectHandle<VkHandle>(handle), _instance(instance) {}
};

/// <summary>Defines a simple interface for a Vulkan which holds a reference to a Vulkan Physical Device.</summary>
template<class VkHandle>
class PhysicalDeviceObjectHandle : public ObjectHandle<VkHandle>
{
public:
	DECLARE_NO_COPY_SEMANTICS(PhysicalDeviceObjectHandle)

	/// <summary>Get physical device (const)</summary>
	/// <returns>PhysicalDevice</returns>
	const PhysicalDeviceWeakPtr getPhysicalDevice() const
	{
		return _physicalDevice;
	}

	/// <summary>Get physical device</summary>
	/// <returns>PhysicalDevice</returns>
	PhysicalDeviceWeakPtr getPhysicalDevice()
	{
		return _physicalDevice;
	}

protected:
	/// <summary>The physical device which was used to create this PhysicalDeviceObject</summary>
	PhysicalDeviceWeakPtr _physicalDevice;

	/// <summary>default Constructor for a physical device object handle</summary>
	PhysicalDeviceObjectHandle() : ObjectHandle<VkHandle>() {}
	/// <summary>Constructor for a physical device object handle initialising the physical device</summary>
	/// <param name="physicalDevice">The Vulkan physical device used to create the Vulkan object.</param>
	explicit PhysicalDeviceObjectHandle(const PhysicalDeviceWeakPtr& physicalDevice) : ObjectHandle<VkHandle>(), _physicalDevice(physicalDevice) {}
	/// <summary>Constructor for a physical device object handle initialising the physical device and Vulkan object handle</summary>
	/// <param name="handle">The Vulkan object handle given to the Vulkan object.</param>
	explicit PhysicalDeviceObjectHandle(const VkHandle& handle) : ObjectHandle<VkHandle>(handle) {}
	/// <summary>Constructor for a physical device object handle initialising the physical device and Vulkan object handle</summary>
	/// <param name="physicalDevice">The Vulkan physical device used to create the Vulkan object.</param>
	/// <param name="handle">The Vulkan object handle given to the Vulkan object.</param>
	PhysicalDeviceObjectHandle(const PhysicalDeviceWeakPtr& physicalDevice, const VkHandle& handle) : ObjectHandle<VkHandle>(handle), _physicalDevice(physicalDevice) {}
};

/// <summary>Defines a simple interface for a Vulkan object which holds a reference to a particular device.</summary>
template<class VkHandle>
class DeviceObjectHandle : public ObjectHandle<VkHandle>
{
public:
	DECLARE_NO_COPY_SEMANTICS(DeviceObjectHandle)

	/// <summary>Get Device (const)</summary>
	/// <returns>DeviceWeakPtr</returns>
	const DeviceWeakPtr getDevice() const
	{
		return _device;
	}

	/// <summary>Get Device</summary>
	/// <returns>DeviceWeakPtr</returns>
	DeviceWeakPtr getDevice()
	{
		return _device;
	}

protected:
	/// <summary>The device which was used to create this DeviceObject</summary>
	DeviceWeakPtr _device;

	/// <summary>default Constructor for a device object handle</summary>
	DeviceObjectHandle() : ObjectHandle<VkHandle>() {}
	/// <summary>Constructor for a device object handle initialising the Vulkan device</summary>
	/// <param name="device">The Vulkan device used to create the Vulkan object.</param>
	explicit DeviceObjectHandle(const DeviceWeakPtr& device) : ObjectHandle<VkHandle>(), _device(device) {}
	/// <summary>Constructor for a device object handle initialising the Vulkan object handle</summary>
	/// <param name="handle">The Vulkan object handle given to the Vulkan object.</param>
	explicit DeviceObjectHandle(const VkHandle& handle) : ObjectHandle<VkHandle>(handle) {}
	/// <summary>Constructor for a device object handle initialising the Vulkan device and device object handle</summary>
	/// <param name="device">The Vulkan device used to create the Vulkan device.</param>
	/// <param name="handle">The Vulkan object handle given to the Vulkan object.</param>
	DeviceObjectHandle(const DeviceWeakPtr& device, const VkHandle& handle) : ObjectHandle<VkHandle>(handle), _device(device) {}
};
} // namespace impl
} // namespace pvrvk
