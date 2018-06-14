/*!
\brief The PVRVk Surface class
\file PVRVk/SurfaceVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/ObjectHandleVk.h"
#include "PVRVk/DebugMarkerVk.h"
#include "PVRVk/ForwardDecObjectsVk.h"
#include "PVRVk/TypesVk.h"

#if defined(VK_USE_PLATFORM_XCB_KHR)
#include <xcb/xcb.h>
#endif

namespace pvrvk {
namespace impl {
/// <summary>A surface represents a renderable part of the "screen", e.g. the inside part of the window.</summary>
class Surface_ : public InstanceObjectHandle<VkSurfaceKHR>
{
public:
	DECLARE_NO_COPY_SEMANTICS(Surface_)

protected:
	/// <summary>Constructor for a surface object.</summary>
	/// <param name="instance">The instance which will be used to create the surface.</param>
	Surface_(InstanceWeakPtr instance);

	/// <summary>Destructor for a surface object.</summary>
	~Surface_();

private:
	friend class pvrvk::impl::Instance_;
	template<typename>
	friend struct ::pvrvk::RefCountEntryIntrusive;
};

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
/// <summary>An Android Surface.</summary>
class AndroidSurface_ : public Surface_
{
public:
	/// <summary>Get window handle</summary>
	/// <returns>ANativeWindow&</returns>
	const ANativeWindow* getANativeWindow() const
	{
		return _aNativeWindow;
	}

	/// <summary>Get AndroidSurfaceCreateFlagsKHR flags</summary>
	/// <returns>AndroidSurfaceCreateFlagsKHR&</returns>
	const AndroidSurfaceCreateFlagsKHR& getFlags() const
	{
		return _flags;
	}

private:
	template<typename>
	friend struct ::pvrvk::RefCountEntryIntrusive;
	AndroidSurface_(InstanceWeakPtr instance, ANativeWindow* aNativewindow, AndroidSurfaceCreateFlagsKHR flags = AndroidSurfaceCreateFlagsKHR::e_NONE);

	ANativeWindow* _aNativeWindow;
	AndroidSurfaceCreateFlagsKHR _flags;
};
#endif

#if defined(VK_USE_PLATFORM_WIN32_KHR)
/// <summary>An Win32 Surface.</summary>
class Win32Surface_ : public Surface_
{
public:
	/// <summary>Get hinstance</summary>
	/// <returns>HINSTANCE&</returns>
	const HINSTANCE& getHInstance() const
	{
		return _hinstance;
	}

	/// <summary>Get hwnd</summary>
	/// <returns>HWND&</returns>
	const HWND& getHwnd() const
	{
		return _hwnd;
	}

	/// <summary>Get Win32SurfaceCreateFlagsKHR flags</summary>
	/// <returns>Win32SurfaceCreateFlagsKHR&</returns>
	const Win32SurfaceCreateFlagsKHR& getFlags() const
	{
		return _flags;
	}

private:
	template<typename>
	friend struct ::pvrvk::RefCountEntryIntrusive;
	Win32Surface_(InstanceWeakPtr instance, HINSTANCE hinstance, HWND hwnd, Win32SurfaceCreateFlagsKHR flags = Win32SurfaceCreateFlagsKHR::e_NONE);

	HINSTANCE _hinstance;
	HWND _hwnd;
	Win32SurfaceCreateFlagsKHR _flags;
};
#endif

#if defined(VK_USE_PLATFORM_XCB_KHR)
/// <summary>An Xcb Surface.</summary>
class XcbSurface_ : public Surface_
{
public:
	/// <summary>Get hinstance</summary>
	/// <returns>HINSTANCE&</returns>
	const xcb_connection_t& getConnection() const
	{
		return *_connection;
	}

	/// <summary>Get hwnd</summary>
	/// <returns>HWND&</returns>
	xcb_window_t getWindow() const
	{
		return _window;
	}

	/// <summary>Get XcbSurfaceCreateFlagsKHR flags</summary>
	/// <returns>XcbSurfaceCreateFlagsKHR&</returns>
	const XcbSurfaceCreateFlagsKHR& getFlags() const
	{
		return _flags;
	}

private:
	template<typename>
	friend struct ::pvrvk::RefCountEntryIntrusive;
	XcbSurface_(InstanceWeakPtr instance, xcb_connection_t* connection, xcb_window_t window, XcbSurfaceCreateFlagsKHR flags = XcbSurfaceCreateFlagsKHR::e_NONE);

	xcb_connection_t* _connection;
	xcb_window_t _window;
	XcbSurfaceCreateFlagsKHR _flags;
};
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
/// <summary>An Xlib Surface.</summary>
class XlibSurface_ : public Surface_
{
public:
	/// <summary>Get display</summary>
	/// <returns>Display&</returns>
	const ::Display& getDpy() const
	{
		return *_dpy;
	}

	/// <summary>Get Window</summary>
	/// <returns>Window&</returns>
	const Window& getWindow() const
	{
		return _window;
	}

	/// <summary>Get XlibSurfaceCreateFlagsKHR flags</summary>
	/// <returns>XlibSurfaceCreateFlagsKHR&</returns>
	const XlibSurfaceCreateFlagsKHR& getFlags() const
	{
		return _flags;
	}

private:
	template<typename>
	friend struct ::pvrvk::RefCountEntryIntrusive;
	XlibSurface_(InstanceWeakPtr instance, ::Display* dpy, Window window, XlibSurfaceCreateFlagsKHR flags = XlibSurfaceCreateFlagsKHR::e_NONE);

	::Display* _dpy;
	Window _window;
	XlibSurfaceCreateFlagsKHR _flags;
};
#endif

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
/// <summary>A Wayland Surface.</summary>
class WaylandSurface_ : public Surface_
{
public:
	/// <summary>Get display</summary>
	/// <returns>wl_display&</returns>
	const wl_display* getDisplay() const
	{
		return _display;
	}

	/// <summary>Get Surface</summary>
	/// <returns>wl_surface&</returns>
	const wl_surface* getSurface() const
	{
		return _surface;
	}

	/// <summary>Get WaylandSurfaceCreateFlagsKHR flags</summary>
	/// <returns>WaylandSurfaceCreateFlagsKHR&</returns>
	const WaylandSurfaceCreateFlagsKHR& getFlags() const
	{
		return _flags;
	}

private:
	template<typename>
	friend struct ::pvrvk::RefCountEntryIntrusive;
	WaylandSurface_(InstanceWeakPtr instance, wl_display* display, wl_surface* surface, WaylandSurfaceCreateFlagsKHR flags = WaylandSurfaceCreateFlagsKHR::e_NONE);

	wl_display* _display;
	wl_surface* _surface;
	WaylandSurfaceCreateFlagsKHR _flags;
};
#endif

/// <summary>A DisplayPlane Surface.</summary>
class DisplayPlaneSurface_ : public Surface_
{
public:
	/// <summary>Get Display Mode Properties</summary>
	/// <returns>DisplayModePropertiesKHR&</returns>
	const DisplayModeWeakPtr& getDisplayMode() const
	{
		return _displayMode;
	}

	/// <summary>Get DisplaySurfaceCreateFlagsKHR flags</summary>
	/// <returns>DisplaySurfaceCreateFlagsKHR&</returns>
	const DisplaySurfaceCreateFlagsKHR& getFlags() const
	{
		return _flags;
	}

	/// <summary>Get display plane index</summary>
	/// <returns>Plane index</returns>
	uint32_t getPlaneIndex() const
	{
		return _planeIndex;
	}

	/// <summary>Get display plane stack index</summary>
	/// <returns>Plane stack index</returns>
	uint32_t getPlaneStackIndex() const
	{
		return _planeStackIndex;
	}

	/// <summary>Get SurfaceTransformFlagsKHR flags</summary>
	/// <returns>SurfaceTransformFlagsKHR&</returns>
	const SurfaceTransformFlagsKHR& getTransformFlags() const
	{
		return _transformFlags;
	}

	/// <summary>Get display plane global alpha</summary>
	/// <returns>Plane global alpha</returns>
	float getGlobalAlpha() const
	{
		return _globalAlpha;
	}

	/// <summary>Get DisplayPlaneAlphaFlagsKHR flags</summary>
	/// <returns>DisplayPlaneAlphaFlagsKHR&</returns>
	const DisplayPlaneAlphaFlagsKHR& getAlphaFlags() const
	{
		return _alphaFlags;
	}

	/// <summary>Get Extent2D</summary>
	/// <returns>Extent2D&</returns>
	const Extent2D& getImageExtent() const
	{
		return _imageExtent;
	}

private:
	template<typename>
	friend struct ::pvrvk::RefCountEntryIntrusive;
	DisplayPlaneSurface_(InstanceWeakPtr instance, const DisplayMode& displayMode, Extent2D imageExtent,
		const DisplaySurfaceCreateFlagsKHR flags = DisplaySurfaceCreateFlagsKHR::e_NONE, uint32_t planeIndex = 0, uint32_t planeStackIndex = 0,
		SurfaceTransformFlagsKHR transformFlags = SurfaceTransformFlagsKHR::e_IDENTITY_BIT_KHR, float globalAlpha = 0.0f,
		DisplayPlaneAlphaFlagsKHR alphaFlags = DisplayPlaneAlphaFlagsKHR::e_PER_PIXEL_BIT_KHR);

	DisplayModeWeakPtr _displayMode;
	DisplaySurfaceCreateFlagsKHR _flags;
	uint32_t _planeIndex;
	uint32_t _planeStackIndex;
	SurfaceTransformFlagsKHR _transformFlags;
	float _globalAlpha;
	DisplayPlaneAlphaFlagsKHR _alphaFlags;
	Extent2D _imageExtent;
};
} // namespace impl
} // namespace pvrvk
