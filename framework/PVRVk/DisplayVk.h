/*!
\brief The Display class
\file PVRVk/DisplayVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/ForwardDecObjectsVk.h"
#include "PVRVk/PhysicalDeviceVk.h"
#include "PVRVk/ObjectHandleVk.h"
#include "PVRVk/DebugMarkerVk.h"
#include "PVRVk/DisplayModeVk.h"

namespace pvrvk {
namespace impl {
/// <summary>A display device can in some environments be used directly for Vulkan rendering without using intermediate windowing systems.</summary>
class Display_ : public PhysicalDeviceObjectHandle<VkDisplayKHR>, public EmbeddedRefCount<Display_>
{
public:
	/// <summary>Get the number of supported display modes</summary>
	/// <returns>Returns the number of supported display modes</returns>
	size_t getNumDisplayModes() const
	{
		return _displayModes.size();
	}

	/// <summary>Get the supported display mode at displayModeIndex</summary>
	/// <param name="displayModeIndex">The index of the display mode to retrieve</param>
	/// <returns>Returns one of supported display modes</returns>
	DisplayMode& getDisplayMode(uint32_t displayModeIndex)
	{
		return _displayModes[displayModeIndex];
	}

	/// <summary>Get the supported display mode at displayModeIndex (const)</summary>
	/// <param name="displayModeIndex">The index of the display mode to retrieve</param>
	/// <returns>Returns one of supported display modes (const)</returns>
	const DisplayMode& getDisplayMode(uint32_t displayModeIndex) const
	{
		return _displayModes[displayModeIndex];
	}

	/// <summary>Gets the name of the display</summary>
	/// <returns>The name of the display</returns>
	inline const char* getDisplayName() const
	{
		return _properties.getDisplayName();
	}

	/// <summary>Gets the physical dimensions of the display</summary>
	/// <returns>The physical dimensions of the display</returns>
	inline const Extent2D& getPhysicalDimensions() const
	{
		return _properties.getPhysicalDimensions();
	}

	/// <summary>Gets the physical resolutions of the display</summary>
	/// <returns>The physical resolutions of the display</returns>
	inline const Extent2D& getPhysicalResolution() const
	{
		return _properties.getPhysicalResolution();
	}

	/// <summary>Gets the set of supported surface transform flags for the display</summary>
	/// <returns>A SurfaceTransformFlagsKHR structure specifying the supported surface transform flags.</returns>
	inline SurfaceTransformFlagsKHR getSupportedTransforms() const
	{
		return _properties.getSupportedTransforms();
	}

	/// <summary>Indicates whether the planes on this display can have their z order changed.</summary>
	/// <returns>If True then the application can re-arrange the planes on this display in any order relative to each other.</returns>
	inline bool getPlaneReorderPossible() const
	{
		return _properties.getPlaneReorderPossible();
	}

	/// <summary>Indicates whether the display supports self-refresh/internal buffering</summary>
	/// <returns>True if the application can submit persistent present operations on swapchains created against this display</returns>
	inline bool getPersistentContent() const
	{
		return _properties.getPersistentContent();
	}

private:
	friend class pvrvk::impl::PhysicalDevice_;
	friend class pvrvk::impl::DisplayMode_;
	friend class pvrvk::EmbeddedRefCount<Display_>;

	Display_(PhysicalDeviceWeakPtr physicalDevice, const DisplayPropertiesKHR& displayProperties);

	static Display createNew(PhysicalDeviceWeakPtr physicalDevice, const DisplayPropertiesKHR& displayProperties)
	{
		return EmbeddedRefCount<Display_>::createNew(physicalDevice, displayProperties);
	}

	void destroyObject()
	{
		_displayModes.clear();
	}

	std::vector<DisplayMode> _displayModes;
	DisplayPropertiesKHR _properties;
};
} // namespace impl
} // namespace pvrvk
