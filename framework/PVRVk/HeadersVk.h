/*!
\brief Conditionally and correctly includes the vulkan header files
\file PVRVk/HeadersVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#pragma once
#ifdef ANDROID
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#endif

#include "vk_bindings.h"
#include "vk_bindings_helper.h"
#include "pvrvk_vulkan_wrapper.h"
//!\endcond
