/*!
\brief The PVRVk Instance class
\file PVRVk/InstanceVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/ExtensionsVk.h"
#include "PVRVk/ObjectHandleVk.h"
#include "PVRVk/DebugReportCallbackVk.h"
#include "PVRVk/DebugMarkerVk.h"
#include "PVRVk/PhysicalDeviceVk.h"
#include "PVRVk/ForwardDecObjectsVk.h"
#include "PVRVk/LayersVk.h"
#include "PVRVk/SurfaceVk.h"

namespace pvrvk {
namespace impl {
class InstanceHelperFactory_;

/// <summary>The Instance is a system-wide vulkan "implementation", similar in concept to the
/// "installation" of Vulkan libraries on a system. Contrast with the "Physical Device" which
/// for example represents a particular driver implementing Vulkan for a specific Device.
/// Conceptually, the Instance "Forwards" to the "Physical Device / Device"</summary>
class Instance_ : public ObjectHandle<VkInstance>, public EmbeddedRefCount<Instance_>
{
public:
	DECLARE_NO_COPY_SEMANTICS(Instance_)

	/// <summary>Get instance create info(const)</summary>
	/// <returns>const InstanceCreateInfo&</returns>
	const InstanceCreateInfo& getInfo() const
	{
		return _createInfo;
	}

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	/// <summary>Create an android surface</summary>
	/// <param name="window">A pointer to an Android Native Window</param>
	/// <param name="flags">A set of AndroidSurfaceCreateFlagsKHR flags to use when creating the android surface</param>
	/// <returns>Valid AndroidSurface object if success.</returns>
	AndroidSurface createAndroidSurface(ANativeWindow* window, AndroidSurfaceCreateFlagsKHR flags = AndroidSurfaceCreateFlagsKHR::e_NONE)
	{
		AndroidSurface androidSurface;
		androidSurface.construct(getWeakReference(), window, flags);
		return androidSurface;
	}
#endif

#if defined(VK_USE_PLATFORM_WIN32_KHR)
	/// <summary>Create a Win32 surface</summary>
	/// <param name="hInstance">A Win32 HINSTANCE</param>
	/// <param name="hwnd">A Win32 HWND</param>
	/// <param name="flags">A set of Win32SurfaceCreateFlagsKHR flags to use when creating the Win32 surface</param>
	/// <returns>Valid Win32Surface object if success.</returns>
	Win32Surface createWin32Surface(HINSTANCE hInstance, HWND hwnd, Win32SurfaceCreateFlagsKHR flags = Win32SurfaceCreateFlagsKHR::e_NONE)
	{
		Win32Surface win32Surface;
		win32Surface.construct(getWeakReference(), hInstance, hwnd, flags);
		return win32Surface;
	}
#endif

#if defined(VK_USE_PLATFORM_XCB_KHR)
	/// <summary>Create an XCB surface</summary>
	/// <param name="connection">An Xcb connection</param>
	/// <param name="window">An Xcb window</param>
	/// <param name="flags">A set of XcbSurfaceCreateFlagsKHR flags to use when creating the xcb surface</param>
	/// <returns>Valid XcbSurface object if success.</returns>
	XcbSurface createXcbSurface(xcb_connection_t* connection, xcb_window_t window, XcbSurfaceCreateFlagsKHR flags = XcbSurfaceCreateFlagsKHR::e_NONE)
	{
		XcbSurface xcbSurface;
		xcbSurface.construct(getWeakReference(), connection, window, flags);
		return xcbSurface;
	}
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
	/// <summary>Create an Xlib surface</summary>
	/// <param name="dpy">An xlib display</param>
	/// <param name="window">An xlib window</param>
	/// <param name="flags">A set of XlibSurfaceCreateFlagsKHR flags to use when creating the xlib surface</param>
	/// <returns>Valid XlibSurface object if success.</returns>
	XlibSurface createXlibSurface(::Display* dpy, Window window, XlibSurfaceCreateFlagsKHR flags = XlibSurfaceCreateFlagsKHR::e_NONE)
	{
		XlibSurface xlibSurface;
		xlibSurface.construct(getWeakReference(), dpy, window, flags);
		return xlibSurface;
	}
#endif

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
	/// <summary>Create a Wayland surface</summary>
	/// <param name="display">The Wayland display</param>
	/// <param name="surface">A Wayland surface</param>
	/// <param name="flags">A set of WaylandSurfaceCreateFlagsKHR flags to use when creating the Wayland surface</param>
	/// <returns>Valid WaylandSurface object if success.</returns>
	WaylandSurface createWaylandSurface(wl_display* display, wl_surface* surface, WaylandSurfaceCreateFlagsKHR flags = WaylandSurfaceCreateFlagsKHR::e_NONE)
	{
		WaylandSurface waylandSurface;
		waylandSurface.construct(getWeakReference(), display, surface, flags);
		return waylandSurface;
	}
#endif

	/// <summary>Create a DisplayPlane surface</summary>
	/// <param name="displayMode">A display mode to use for creating the DisplayPlane Surface</param>
	/// <param name="imageExtent">The image extent to use for creating the DisplayPlane Surface</param>
	/// <param name="flags">A set of DisplaySurfaceCreateFlagsKHR flags to use when creating the DisplayPlane surface</param>
	/// <param name="planeIndex">A plane index</param>
	/// <param name="planeStackIndex">A plane stack index</param>
	/// <param name="transformFlags">A set of SurfaceTransformFlagsKHR flags to use when creating the DisplayPlane surface</param>
	/// <param name="globalAlpha">A global alpha value</param>
	/// <param name="alphaFlags">A set of DisplayPlaneAlphaFlagsKHR flags to use when creating the DisplayPlane surface</param>
	/// <returns>Valid DisplayPlane object if success.</returns>
	DisplayPlaneSurface createDisplayPlaneSurface(const DisplayMode& displayMode, Extent2D imageExtent, const DisplaySurfaceCreateFlagsKHR flags = DisplaySurfaceCreateFlagsKHR::e_NONE,
		uint32_t planeIndex = 0, uint32_t planeStackIndex = 0, SurfaceTransformFlagsKHR transformFlags = SurfaceTransformFlagsKHR::e_IDENTITY_BIT_KHR, float globalAlpha = 0.0f,
		DisplayPlaneAlphaFlagsKHR alphaFlags = DisplayPlaneAlphaFlagsKHR::e_PER_PIXEL_BIT_KHR)
	{
		DisplayPlaneSurface displayPlaneSurface;
		displayPlaneSurface.construct(getWeakReference(), displayMode, imageExtent, flags, planeIndex, planeStackIndex, transformFlags, globalAlpha, alphaFlags);
		return displayPlaneSurface;
	}

	/// <summary>Get all enabled extensions names</summary>
	/// <returns>std::vector<const char*>&</returns>
	const std::vector<std::string>& getEnabledInstanceExtensions()
	{
		return _createInfo.getEnabledExtensionNames();
	}

	/// <summary>Get all enabled layers names</summary>
	/// <returns>std::vector<const char*>&</returns>
	const std::vector<const char*>& getEnabledInstanceLayers()
	{
		return _enabledInstanceLayers;
	}

	/// <summary>Check if extension is enabled</summary>
	/// <param name="extensionName">Extension name</param>
	/// <returns>Return true if it is enabled</returns>
	bool isInstanceExtensionEnabled(const char* extensionName) const
	{
		for (uint32_t i = 0; i < _createInfo.getNumEnabledExtensionNames(); i++)
		{
			if (!strcmp(_createInfo.getEnabledExtensionName(i).c_str(), extensionName))
			{
				return true;
			}
		}

		return false;
	}

	/// <summary>Check if layer is enabled</summary>
	/// <param name="layerName">Layer name</param>
	/// <returns>Return true if enabled</returns>
	bool isInstanceLayerEnabled(const char* layerName)
	{
		for (uint32_t i = 0; i < _enabledInstanceLayers.size(); i++)
		{
			if (!strcmp(_enabledInstanceLayers[i], layerName))
			{
				return true;
			}
		}
		return false;
	}

	/// <summary>Creates a debug report callback object</summary>
	/// <param name="createInfo">DebugReportCallbackCreateInfo structure specifying how the debug report callback function should work.</param>
	/// <returns>Returns the created debug report callback object.</returns>
	inline DebugReportCallback createDebugReportCallback(const DebugReportCallbackCreateInfo& createInfo)
	{
		DebugReportCallback debugCallback;
		debugCallback.construct(getWeakReference(), createInfo);
		return debugCallback;
	}

	/// <summary>Gets the instance dispatch table</summary>
	/// <returns>The instance dispatch table</returns>
	inline const VkInstanceBindings& getVkBindings() const
	{
		return _vkBindings;
	}

	/// <summary>Get the list of physical devices (const)</summary>
	/// <returns>const PhysicalDevice&</returns>
	const std::vector<PhysicalDevice>& getPhysicalDevices() const;

	/// <summary>Get physical device (const)</summary>
	/// <param name="id">Physcialdevice id</param>
	/// <returns>const PhysicalDevice&</returns>
	const PhysicalDevice& getPhysicalDevice(uint32_t id) const;

	/// <summary>Get physical device (const)</summary>
	/// <param name="id">Physcialdevice id</param>
	/// <returns>const PhysicalDevice&</returns>
	PhysicalDevice& getPhysicalDevice(uint32_t id);

	/// <summary>Get number of physcial device available</summary>
	/// <returns>uint32_t</returns>
	uint32_t getNumPhysicalDevices() const
	{
		return static_cast<uint32_t>(_physicalDevices.size());
	}

private:
	friend class InstanceHelperFactory_;
	friend class ::pvrvk::EmbeddedRefCount<Instance_>;

	void destroyObject()
	{
		_physicalDevices.clear();
		if (getVkHandle() != VK_NULL_HANDLE)
		{
			_vkBindings.vkDestroyInstance(getVkHandle(), nullptr);
			_vkHandle = VK_NULL_HANDLE;
		}
	}

	static Instance createNew(const InstanceCreateInfo& instanceCreateInfo)
	{
		return EmbeddedRefCount<Instance_>::createNew(instanceCreateInfo);
	}

	explicit Instance_(const InstanceCreateInfo& instanceCreateInfo);

	std::vector<const char*> _enabledInstanceLayers;
	InstanceCreateInfo _createInfo;
	VkInstanceBindings _vkBindings;
	std::vector<PhysicalDevice> _physicalDevices;
};
} // namespace impl

/// <summary>Create a PVRVk Instance</summary>
/// <param name="createInfo">The Create Info object for created Instance</param>
/// <returns>A newly created instance. In case of failure, null instance</returns>
Instance createInstance(const InstanceCreateInfo& createInfo);
} // namespace pvrvk
