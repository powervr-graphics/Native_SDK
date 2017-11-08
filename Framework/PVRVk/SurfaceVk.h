/*!
\brief The PVRVk Surface class
\file PVRVk/SurfaceVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/ForwardDecObjectsVk.h"
#include "PVRVk/TypesVk.h"

namespace pvrvk {
namespace impl {
/// <summary>A surface represents a renderable part of the "screen", e.g. the inside part of the window.</summary>
class Surface_
{
public:
	DECLARE_NO_COPY_SEMANTICS(Surface_)

	/// <summary>Get vulkan object (const)</summary>
	/// <returns>VkSurfaceKHR</returns>
	VkSurfaceKHR getNativeObject() const { return _surface; }

	/// <summary>Get vulkan object (const)</summary>
	/// <returns>VkSurfaceKHR</returns>
	VkSurfaceKHR getNativeObject() { return _surface; }

	/// <summary>Get window handle</summary>
	/// <returns>NativeWindow&</returns>
	const NativeWindow& getNativeWindow()const
	{
		return _nativeWindow;
	}

	/// <summary>Get display handle</summary>
	/// <returns>NativeDisplay&</returns>
	const NativeDisplay& getNativeDisplay()const
	{
		return _nativeDisplay;
	}

private:
	bool init(InstanceWeakPtr instance, const PhysicalDevice& physicalDevice,
	          void* window, void* display);

	friend class pvrvk::impl::Instance_;
	template<typename> friend struct ::pvrvk::RefCountEntryIntrusive;
	Surface_() : _surface(VK_NULL_HANDLE) {}
	~Surface_();

	InstanceWeakPtr _instance;
	NativeDisplay _nativeDisplay;
	VkSurfaceKHR  _surface;
	NativeWindow  _nativeWindow;
};
}
}
