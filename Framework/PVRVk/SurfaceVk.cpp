/*!
\brief Function implementations for the Surface class
\file PVRVk/SurfaceVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "PVRVk/SurfaceVk.h"
#include "PVRVk/InstanceVk.h"
#ifdef X11
#include <dlfcn.h>
#endif

namespace pvrvk {
namespace impl {

bool Surface_::init(InstanceWeakPtr instance, const pvrvk::PhysicalDevice& physicalDevice,
                    void* window, void* display)
{
	physicalDevice;// hide warning
	_nativeWindow = reinterpret_cast<NativeWindow>(window);
	_nativeDisplay = reinterpret_cast<NativeDisplay>(display);
	_instance = instance;
#if defined(ANDROID)
	if (instance->isInstanceExtensionEnabled(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME))
	{
		// create an android surface
		VkAndroidSurfaceCreateInfoKHR surfaceInfo;
		surfaceInfo.sType = VkStructureType::e_ANDROID_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.pNext = NULL;
		surfaceInfo.flags = 0;
		surfaceInfo.window = _nativeWindow;
		vkThrowIfFailed(vk::CreateAndroidSurfaceKHR(instance->getNativeObject(),
		                &surfaceInfo, NULL, &_surface),
		                "failed to create Android Window surface, returned an error");
	}
	else
	{
		Log("Android platform not supported");
		debug_assertion(false, "Android platform not supported");
	}
#elif defined _WIN32
	if (instance->isInstanceExtensionEnabled(VK_KHR_WIN32_SURFACE_EXTENSION_NAME))
	{
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VkStructureType::e_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.hinstance = (HINSTANCE)GetModuleHandle(NULL);
		surfaceCreateInfo.hwnd = (HWND)_nativeWindow;
		surfaceCreateInfo.flags = 0;
		vkThrowIfFailed(vk::CreateWin32SurfaceKHR(instance->getNativeObject(), &surfaceCreateInfo, NULL, &_surface),
		                "failed to create Win32 Window surface, returned an error");
	}
	else
	{
		Log("Win32 platform not supported");
		debug_assertion(false, "Win32 platform not supported");
	}
#elif defined(X11)
	if (instance->isInstanceExtensionEnabled(VK_KHR_XLIB_SURFACE_EXTENSION_NAME))
	{
		VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VkStructureType::e_XLIB_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.dpy = (Display*)_nativeDisplay;
		surfaceCreateInfo.window = (Window)_nativeWindow;
		VkInstance vkInstance = instance->getNativeObject();
		if (vk::CreateXlibSurfaceKHR(vkInstance, &surfaceCreateInfo, NULL, &_surface) != VkResult::e_SUCCESS)
		{
			Log("failed to create Xlib Window surface, returned an error");
			return false;
		}
	}
	else if (instance->isInstanceExtensionEnabled(VK_KHR_XCB_SURFACE_EXTENSION_NAME))
	{
		typedef xcb_connection_t* (*PFN_XGetXCBConnection)(Display*);
		void* dlHandle = dlopen("libX11-xcb.so.1;libX11-xcb.so", RTLD_LAZY);
		if (!dlHandle)
		{
			Log("Failed to libX11-xcb");
			return false;
		}
		PFN_XGetXCBConnection fn_XGetXCBConnection = (PFN_XGetXCBConnection)dlsym(dlHandle, "XGetXCBConnection");
		if (fn_XGetXCBConnection == nullptr)
		{
			Log("Failed to retrieve XGetXCBConnection function pointer. Requires libX11-xcb installed on the system");
			dlclose(dlHandle);
			return false;
		}
		// to do load the x11-xcb library.
		VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VkStructureType::e_XCB_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.connection =  fn_XGetXCBConnection((Display*)_nativeDisplay);
		surfaceCreateInfo.window = (Window)_nativeWindow;
		dlclose(dlHandle);
		vkThrowIfFailed(vk::CreateXcbSurfaceKHR(instance->getNativeObject(),
		                                        &surfaceCreateInfo, nullptr, &_surface), "failed to create Xcb Window surface, returned an error");
	}
	else
	{
		Log("X11 platform not supported");
		debug_assertion(false, "X11 platform not supported");
	}
#elif defined(WAYLAND)
	if (instance->isInstanceExtensionEnabled(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME))
	{
		VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VkStructureType::e_WAYLAND_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.display = (wl_display*)_nativeDisplay;
		surfaceCreateInfo.surface = (wl_surface*)_nativeWindow;
		vkThrowIfFailed(vk::CreateWaylandSurfaceKHR(instance->getNativeObject(), &surfaceCreateInfo, NULL, &_surface), "failed to create Wayland Window surface, returned an error");
	}
	else
	{
		Log("Wayland platform not supported");
		debug_assertion(false, "Wayland platform not supported");
	}
#else // NullWS
	VkDisplayPropertiesKHR properties = {};
	uint32_t numProperties = 1;
	if (vk::GetPhysicalDeviceDisplayPropertiesKHR)
	{
		vk::GetPhysicalDeviceDisplayPropertiesKHR(physicalDevice->getNativeObject(), &numProperties, &properties);
	}

	std::string supportedTransforms;
	if ((properties.supportedTransforms & VkSurfaceTransformFlagsKHR::e_IDENTITY_BIT_KHR) != 0) { supportedTransforms.append("none "); }
	if ((properties.supportedTransforms & VkSurfaceTransformFlagsKHR::e_ROTATE_90_BIT_KHR) != 0) { supportedTransforms.append("rot90 "); }
	if ((properties.supportedTransforms & VkSurfaceTransformFlagsKHR::e_ROTATE_180_BIT_KHR) != 0) { supportedTransforms.append("rot180 "); }
	if ((properties.supportedTransforms & VkSurfaceTransformFlagsKHR::e_ROTATE_270_BIT_KHR) != 0) { supportedTransforms.append("rot270 "); }
	if ((properties.supportedTransforms & VkSurfaceTransformFlagsKHR::e_HORIZONTAL_MIRROR_BIT_KHR) != 0) { supportedTransforms.append("h_mirror "); }
	if ((properties.supportedTransforms & VkSurfaceTransformFlagsKHR::e_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR) != 0) { supportedTransforms.append("h_mirror+rot90 "); }
	if ((properties.supportedTransforms & VkSurfaceTransformFlagsKHR::e_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR) != 0) { supportedTransforms.append("hmirror+rot180 "); }
	if ((properties.supportedTransforms & VkSurfaceTransformFlagsKHR::e_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR) != 0) { supportedTransforms.append("hmirror+rot270 "); }
	if ((properties.supportedTransforms & VkSurfaceTransformFlagsKHR::e_INHERIT_BIT_KHR) != 0) { supportedTransforms.append("inherit "); }
	Log(LogLevel::Information, "**** Display Properties: ****");
	Log(LogLevel::Information, "name: %s", properties.displayName);
	Log(LogLevel::Information, "size: %dx%d", properties.physicalDimensions.width, properties.physicalDimensions.height);
	Log(LogLevel::Information, "resolution: %dx%d", properties.physicalResolution.width, properties.physicalResolution.height);
	Log(LogLevel::Information, "transforms: %s", supportedTransforms.c_str());
	Log(LogLevel::Information, "plane reordering?: %s", properties.planeReorderPossible ? "yes" : "no");
	Log(LogLevel::Information, "persistent conents?: %s", properties.persistentContent ? "yes" : "no");

	_nativeDisplay = properties.display;

	uint32_t numModes = 0;
	vk::GetDisplayModePropertiesKHR(physicalDevice->getNativeObject(), _nativeDisplay, &numModes, NULL);
	std::vector<VkDisplayModePropertiesKHR> modeProperties; modeProperties.resize(numModes);
	vk::GetDisplayModePropertiesKHR(physicalDevice->getNativeObject(), _nativeDisplay, &numModes, modeProperties.data());

	Log(LogLevel::Information, "Display Modes:");
	for (uint32_t i = 0; i < numModes; ++i)
	{
		Log(LogLevel::Information, "\t[%u] %ux%u @%uHz", i, modeProperties[i].parameters.visibleRegion.width,
		    modeProperties[i].parameters.visibleRegion.height, modeProperties[i].parameters.refreshRate);
	}

	VkDisplaySurfaceCreateInfoKHR surfaceCreateInfo;

	surfaceCreateInfo.sType = VkStructureType::e_DISPLAY_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.pNext = NULL;

	surfaceCreateInfo.displayMode = modeProperties[0].displayMode;
	surfaceCreateInfo.planeIndex = 0;
	surfaceCreateInfo.planeStackIndex = 0;
	surfaceCreateInfo.transform = VkSurfaceTransformFlagsKHR::e_IDENTITY_BIT_KHR;
	surfaceCreateInfo.globalAlpha = 0.0f;
	surfaceCreateInfo.alphaMode = VkDisplayPlaneAlphaFlagsKHR::e_PER_PIXEL_BIT_KHR;
	surfaceCreateInfo.imageExtent = modeProperties[0].parameters.visibleRegion;

	if (!vkIsSuccessful(vk::CreateDisplayPlaneSurfaceKHR(instance->getNativeObject(), &surfaceCreateInfo,
	                    nullptr, &_surface), "Could not create DisplayPlane Surface"))
	{
		return false;
	}
#endif // NullWS
	return true;
}

Surface_::~Surface_()
{
	if (_surface != VK_NULL_HANDLE)
	{
		vk::DestroySurfaceKHR(_instance->getNativeObject(), _surface, NULL);
		_surface = VK_NULL_HANDLE;
	}
}
} // namespace impl
} // namespace pvrvk
