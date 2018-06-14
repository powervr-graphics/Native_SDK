/*!
\brief The DisplayMode class
\file PVRVk/DisplayModeVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/ForwardDecObjectsVk.h"
#include "PVRVk/PhysicalDeviceVk.h"
#include "PVRVk/ObjectHandleVk.h"
#include "PVRVk/DebugMarkerVk.h"

namespace pvrvk {
/// <summary>Display Mode creation descriptor.</summary>
struct DisplayModeCreateInfo
{
public:
	/// <summary>Constructor (zero initialization)</summary>
	DisplayModeCreateInfo() : _flags(DisplayModeCreateFlagsKHR::e_NONE), _parameters(DisplayModeParametersKHR()) {}

	/// <summary>Constructor</summary>
	/// <param name="parameters">Display mode parameters used to initialise the display mode creation structure</param>
	DisplayModeCreateInfo(DisplayModeParametersKHR parameters) : _flags(DisplayModeCreateFlagsKHR::e_NONE), _parameters(parameters) {}

	/// <summary>Getter for the display mode creation flags</summary>
	/// <returns>A DisplayModeCreateFlagsKHR structure</returns>
	DisplayModeCreateFlagsKHR getFlags() const
	{
		return _flags;
	}

	/// <summary>Setter for the display mode creation flags</summary>
	/// <param name="flags">A set of DisplayModeCreateFlagsKHR</param>
	/// <returns>This object</returns>
	DisplayModeCreateInfo& setFlags(DisplayModeCreateFlagsKHR flags)
	{
		this->_flags = flags;
		return *this;
	}

	/// <summary>Getter for the display mode parameters</summary>
	/// <returns>A DisplayModeParametersKHR structure</returns>
	DisplayModeParametersKHR getParameters() const
	{
		return _parameters;
	}

	/// <summary>Setter for the display mode parameters</summary>
	/// <param name="parameters">A set of DisplayModeParametersKHR</param>
	/// <returns>This object</returns>
	DisplayModeCreateInfo& setParameters(DisplayModeParametersKHR parameters)
	{
		this->_parameters = parameters;
		return *this;
	}

private:
	/// <summary>The set of DisplayModeCreateFlagsKHR used when creating the display mode</summary>
	DisplayModeCreateFlagsKHR _flags;
	/// <summary>The set of DisplayModeParametersKHR used when creating the display mode</summary>
	DisplayModeParametersKHR _parameters;
};

namespace impl {
/// <summary>Each display has one or more supported modes associated with it by default. These are called the display modes.</summary>
class DisplayMode_ : public PhysicalDeviceObjectHandle<VkDisplayModeKHR>, public EmbeddedRefCount<DisplayMode_>
{
public:
	/// <summary>Returns the display mode parameters</summary>
	/// <returns>A DisplayModeParametersKHR structure specifying the display mode parameters for the display mode</returns>
	DisplayModeParametersKHR getParameters() const
	{
		return _parameters;
	}

private:
	friend class pvrvk::impl::PhysicalDevice_;
	friend class pvrvk::impl::Display_;
	friend class pvrvk::EmbeddedRefCount<DisplayMode_>;

	DisplayMode_(PhysicalDeviceWeakPtr physicalDevice, const DisplayModePropertiesKHR& displayModeProperties);
	DisplayMode_(PhysicalDeviceWeakPtr physicalDevice, pvrvk::Display& display, const pvrvk::DisplayModeCreateInfo& displayModeCreateInfo);

	static DisplayMode createNew(PhysicalDeviceWeakPtr physicalDevice, const DisplayModePropertiesKHR& displayModeProperties)
	{
		return EmbeddedRefCount<DisplayMode_>::createNew(physicalDevice, displayModeProperties);
	}

	static DisplayMode createNew(PhysicalDeviceWeakPtr physicalDevice, pvrvk::Display& display, const DisplayModeCreateInfo& displayModeCreateInfo)
	{
		return EmbeddedRefCount<DisplayMode_>::createNew(physicalDevice, display, displayModeCreateInfo);
	}

	void destroyObject() {}

	DisplayModeParametersKHR _parameters;
};
} // namespace impl
} // namespace pvrvk
