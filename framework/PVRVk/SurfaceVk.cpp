/*!
\brief Function implementations for the Surface class
\file PVRVk/SurfaceVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "PVRVk/SurfaceVk.h"
#include "PVRVk/InstanceVk.h"
#include "PVRVk/DisplayModeVk.h"
#ifdef VK_USE_PLATFORM_XCB_KHR
#include <dlfcn.h>
#endif

namespace pvrvk {
namespace impl {
Surface_::Surface_(InstanceWeakPtr instance) : InstanceObjectHandle(instance) {}

Surface_::~Surface_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (_instance.isValid())
		{
			getInstance()->getVkBindings().vkDestroySurfaceKHR(_instance->getVkHandle(), getVkHandle(), NULL);
			_vkHandle = VK_NULL_HANDLE;
		}
		else
		{
			Log(LogLevel::Warning, "Attempted to destroy object of type [Surface] after its corresponding VkInstance");
		}
		_vkHandle = VK_NULL_HANDLE;
	}
}

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
AndroidSurface_::AndroidSurface_(InstanceWeakPtr instance, ANativeWindow* aNativewindow, AndroidSurfaceCreateFlagsKHR flags) : Surface_(instance)
{
	if (getInstance()->isInstanceExtensionEnabled(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME))
	{
		_aNativeWindow = aNativewindow;
		_flags = flags;

		// Create an Android Surface
		VkAndroidSurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = static_cast<VkStructureType>(StructureType::e_ANDROID_SURFACE_CREATE_INFO_KHR);
		surfaceInfo.pNext = NULL;
		surfaceInfo.flags = static_cast<VkAndroidSurfaceCreateFlagsKHR>(_flags);
		surfaceInfo.window = _aNativeWindow;
		vkThrowIfFailed(getInstance()->getVkBindings().vkCreateAndroidSurfaceKHR(getInstance()->getVkHandle(), &surfaceInfo, NULL, &_vkHandle), "Failed to create Android Surface");
	}
	else
	{
		throw ErrorUnknown("Android Platform Surface extensions have not been enabled when creating the VkInstance.");
	}
}
#endif

#if defined(VK_USE_PLATFORM_WIN32_KHR)
Win32Surface_::Win32Surface_(InstanceWeakPtr instance, HINSTANCE hinstance, HWND hwnd, Win32SurfaceCreateFlagsKHR flags) : Surface_(instance)
{
	if (getInstance()->isInstanceExtensionEnabled(VK_KHR_WIN32_SURFACE_EXTENSION_NAME))
	{
		_hinstance = hinstance;
		_hwnd = hwnd;
		_flags = flags;

		// Create a Win32 Surface
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_WIN32_SURFACE_CREATE_INFO_KHR);
		surfaceCreateInfo.hinstance = _hinstance;
		surfaceCreateInfo.hwnd = _hwnd;
		surfaceCreateInfo.flags = static_cast<VkWin32SurfaceCreateFlagsKHR>(_flags);
		vkThrowIfFailed(
			getInstance()->getVkBindings().vkCreateWin32SurfaceKHR(getInstance()->getVkHandle(), &surfaceCreateInfo, NULL, &_vkHandle), "failed to create Win32 Window surface");
	}
	else
	{
		throw ErrorUnknown("Win32 Platform Surface extensions have not been enabled when creating the VkInstance.");
	}
}
#endif

#if defined(VK_USE_PLATFORM_XCB_KHR)
XcbSurface_::XcbSurface_(InstanceWeakPtr instance, xcb_connection_t* connection, xcb_window_t window, XcbSurfaceCreateFlagsKHR flags) : Surface_(instance)
{
	if (instance->isInstanceExtensionEnabled(VK_KHR_XCB_SURFACE_EXTENSION_NAME))
	{
		_connection = connection;
		_window = window;
		_flags = flags;

		VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_XCB_SURFACE_CREATE_INFO_KHR);
		surfaceCreateInfo.connection = connection;
		surfaceCreateInfo.window = window;
		vkThrowIfFailed(instance->getVkBindings().vkCreateXcbSurfaceKHR(getInstance()->getVkHandle(), &surfaceCreateInfo, nullptr, &_vkHandle), "Failed to create Xcb Window surface");
	}
	else
	{
		throw ErrorUnknown("Xcb Platform Surface extensions have not been enabled when creating the VkInstance.");
	}
}
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
XlibSurface_::XlibSurface_(InstanceWeakPtr instance, ::Display* dpy, Window window, XlibSurfaceCreateFlagsKHR flags) : Surface_(instance)
{
	if (getInstance()->isInstanceExtensionEnabled(VK_KHR_XLIB_SURFACE_EXTENSION_NAME))
	{
		_dpy = dpy;
		_window = window;
		_flags = flags;

		// Create an Xlib Surface
		VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_XLIB_SURFACE_CREATE_INFO_KHR);
		surfaceCreateInfo.dpy = _dpy;
		surfaceCreateInfo.window = _window;
		surfaceCreateInfo.flags = static_cast<VkXlibSurfaceCreateFlagsKHR>(_flags);
		VkInstance vkInstance = getInstance()->getVkHandle();
		vkThrowIfFailed(getInstance()->getVkBindings().vkCreateXlibSurfaceKHR(vkInstance, &surfaceCreateInfo, NULL, &_vkHandle), "Failed to create Xlib Window surface");
	}
	else
	{
		throw ErrorUnknown("Xlib Platform Surface extensions have not been enabled when creating the VkInstance.");
	}
}
#endif

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
WaylandSurface_::WaylandSurface_(InstanceWeakPtr instance, wl_display* display, wl_surface* surface, WaylandSurfaceCreateFlagsKHR flags) : Surface_(instance)
{
	if (getInstance()->isInstanceExtensionEnabled(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME))
	{
		_display = display;
		_surface = surface;
		_flags = flags;

		// Create a Wayland Surface
		VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_WAYLAND_SURFACE_CREATE_INFO_KHR);
		surfaceCreateInfo.display = _display;
		surfaceCreateInfo.surface = _surface;
		vkThrowIfFailed(getInstance()->getVkBindings().vkCreateWaylandSurfaceKHR(getInstance()->getVkHandle(), &surfaceCreateInfo, NULL, &_vkHandle),
			"Failed to create Wayland Window surface");
	}
	else
	{
		throw ErrorUnknown("Wayland Platform Surface extensions have not been enabled when creating the VkInstance.");
	}
}
#endif

DisplayPlaneSurface_::DisplayPlaneSurface_(InstanceWeakPtr instance, const DisplayMode& displayMode, Extent2D imageExtent, const DisplaySurfaceCreateFlagsKHR flags,
	uint32_t planeIndex, uint32_t planeStackIndex, SurfaceTransformFlagsKHR transformFlags, float globalAlpha, DisplayPlaneAlphaFlagsKHR alphaFlags)
	: Surface_(instance)
{
	if (getInstance()->isInstanceExtensionEnabled(VK_KHR_DISPLAY_EXTENSION_NAME))
	{
		_displayMode = displayMode;
		_flags = flags;
		_planeIndex = planeIndex;
		_planeStackIndex = planeStackIndex;
		_transformFlags = transformFlags;
		_globalAlpha = globalAlpha;
		_alphaFlags = alphaFlags;
		_imageExtent = imageExtent;

		// Create a DisplayPlane Surface
		VkDisplaySurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_DISPLAY_SURFACE_CREATE_INFO_KHR);
		surfaceCreateInfo.pNext = NULL;
		surfaceCreateInfo.displayMode = _displayMode->getVkHandle();
		surfaceCreateInfo.planeIndex = _planeIndex;
		surfaceCreateInfo.planeStackIndex = _planeStackIndex;
		surfaceCreateInfo.transform = static_cast<VkSurfaceTransformFlagBitsKHR>(_transformFlags);
		surfaceCreateInfo.globalAlpha = _globalAlpha;
		surfaceCreateInfo.alphaMode = static_cast<VkDisplayPlaneAlphaFlagBitsKHR>(_alphaFlags);
		surfaceCreateInfo.imageExtent = *reinterpret_cast<const VkExtent2D*>(&_imageExtent);

		vkThrowIfFailed(getInstance()->getVkBindings().vkCreateDisplayPlaneSurfaceKHR(getInstance()->getVkHandle(), &surfaceCreateInfo, nullptr, &_vkHandle),
			"Could not create DisplayPlane Surface");
	}
	else
	{
		throw ErrorUnknown("Display Plane Platform Surface extensions have not been enabled when creating the VkInstance.");
	}
}
} // namespace impl
} // namespace pvrvk
