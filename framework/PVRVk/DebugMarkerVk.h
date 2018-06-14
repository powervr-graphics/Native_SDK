/*!
\brief A simple Debug Marker wrapper providing support for object naming and tagging. PVRVk objects which support the
 extension "VK_EXT_debug_maker" must inherit from DeviceObjectDebugMarker.
\file PVRVk/DebugMarkerVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/ForwardDecObjectsVk.h"

/// <summary>Main PowerVR Framework Namespace</summary>
namespace pvrvk {
/// <summary>Contains internal objects and wrapped versions of the PVRVk module</summary>
namespace impl {
/// <summary>Implementation for the Debug Marker wrapper for PVRVk objects. Handles the actual naming and tagging calls for the extension "VK_EXT_debug_maker".</summary>
class DebugMarkerImpl
{
public:
	template<typename>
	friend class DeviceObjectDebugMarker;

	/// <summary>Destructor.</summary>
	~DebugMarkerImpl() {}

	/// <summary>Makes use of the extension VK_EXT_debug_marker to provide a name for a specified object.</summary>
	/// <param name="device">The device from which the object being named was created</param>
	/// <param name="vkHandle">The Vulkan object handle of the object being named</param>
	/// <param name="objectName">The name to use for the object</param>
	/// <returns>VkResult specifying the result of the call to DebugMarkerSetObjectNameEXT</returns>
	void setObjectName(const Device_& device, uint64_t vkHandle, const std::string& objectName);

	/// <summary>Makes use of the extension VK_EXT_debug_marker to provide a tag for a specified object.</summary>
	/// <param name="device">The device from which the object getting tagged was created</param>
	/// <param name="vkHandle">The Vulkan object handle of the object getting a tag was created.</param>
	/// <param name="tagName">A numerical identifier of the tag for the object</param>
	/// <param name="tagSize">The number of bytes of data to attach to the object.</param>
	/// <param name="tag">An array of tagSize bytes containing the data to be associated with the object.</param>
	void setObjectTag(const Device_& device, uint64_t vkHandle, uint64_t tagName, size_t tagSize, const void* tag);

	/// <summary>Resets the name of a specified object using the extension VK_EXT_debug_marker.</summary>
	/// <param name="device">The device from which the object having its name reset was created</param>
	/// <param name="vkHandle">The Vulkan object handle of the object having its name reset.</param>
	void resetObjectName(const Device_& device, uint64_t vkHandle)
	{
		setObjectName(device, vkHandle, "");
	}

	/// <summary>Returns whether the specified object has already been provided with a name.</summary>
	/// <returns>True if the object has a name, otherwise false.</returns>
	bool hasName() const
	{
		return _objectName.length() > 0;
	}

	/// <summary>Returns the specified object's name.</summary>
	/// <returns>The object name</returns>
	const std::string& getName() const
	{
		return _objectName;
	}

	/// <summary>Returns the specified object's type.</summary>
	/// <returns>The object type</returns>
	DebugReportObjectTypeEXT getType() const
	{
		return _objectType;
	}

private:
	/// <summary>Default Constructor for a DebugMarkerImpl.</summary>
	DebugMarkerImpl() : _objectName(""), _objectType(DebugReportObjectTypeEXT(0)) {}

	/// <summary>Constructor for a DebugMarkerImpl which specifies the DebugReportObjectTypeEXT of the object to wrap.</summary>
	/// <param name="objectType">The VkDebugReportObjectTypeEXT of the object to wrap</param>
	explicit DebugMarkerImpl(const DebugReportObjectTypeEXT& objectType) : _objectName(""), _objectType(DebugReportObjectTypeEXT(objectType))
	{
		assertion(_objectType != DebugReportObjectTypeEXT::e_UNKNOWN_EXT);
	}

	std::string _objectName;
	DebugReportObjectTypeEXT _objectType;
};

/// <summary>A Debug Marker wrapper for PVRVk Device allocated objects. Handles naming and tagging calls for the extension "VK_EXT_debug_maker".</summary>
template<class PVRVkObject>
class DeviceObjectDebugMarker
{
public:
	/// <summary>Constructor for a DeviceObjectDebugMarker which specifies the device to use and the DebugReportObjectTypeEXT of the object to wrap.</summary>
	/// <param name="objectType">The DebugReportObjectTypeEXT of the object to wrap</param>
	explicit DeviceObjectDebugMarker(const DebugReportObjectTypeEXT& objectType) : _debugMarker(objectType) {}

	/// <summary>Makes use of the extension VK_EXT_debug_marker to provide a name for a specified object.</summary>
	/// <param name="objectName">The name to use for the object</param>
	void setObjectName(const std::string& objectName)
	{
		_debugMarker.setObjectName(*static_cast<PVRVkObject&>(*this).getDevice().get(),
			*static_cast<const uint64_t*>(static_cast<const void*>(&static_cast<PVRVkObject&>(*this).getVkHandle())), objectName);
	}

	/// <summary>Resets the name of a specified object using the extension VK_EXT_debug_marker.</summary>
	void resetObjectName()
	{
		_debugMarker.setObjectName(
			*static_cast<PVRVkObject&>(*this).getDevice().get(), *static_cast<const uint64_t*>(static_cast<const void*>(&static_cast<PVRVkObject&>(*this).getVkHandle())), "");
	}

	/// <summary>Makes use of the extension VK_EXT_debug_marker to provide a tag for a specified object.</summary>
	/// <param name="tagName">A numerical identifier of the tag for the object</param>
	/// <param name="tagSize">The number of bytes of data to attach to the object.</param>
	/// <param name="tag">An array of tagSize bytes containing the data to be associated with the object.</param>
	void setObjectTag(uint64_t tagName, size_t tagSize, const void* tag)
	{
		_debugMarker.setObjectTag(*static_cast<PVRVkObject&>(*this).getDevice().get(),
			*static_cast<const uint64_t*>(static_cast<const void*>(&static_cast<PVRVkObject&>(*this).getVkHandle())), tagName, tagSize, tag);
	}

private:
	DebugMarkerImpl _debugMarker;
};
} // namespace impl
} // namespace pvrvk
