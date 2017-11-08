/*!
\brief Conditionally and correctly includes the vulkan header files
\file PVRVk/HeadersVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#pragma once
#ifndef VK_PROTOTYPES
#define VK_NO_PROTOTYPES 1
#endif
#ifdef ANDROID
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#elif defined(X11)
#define VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_XCB_KHR
#elif defined(WAYLAND)
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif
#include "vulkan/vulkan_IMG.h"
#if defined (X11)
// undef these macros from the xlib files, they are breaking the framework types.
#undef Success
#undef Enum
#undef None
#undef Always
#undef byte
#undef char8
#undef ShaderStageFlags
#undef capability
#endif
//!\endcond