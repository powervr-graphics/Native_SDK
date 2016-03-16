/*!*********************************************************************************************************************
\file         PVRPlatformGlue\Vulkan\ExtensionLoaderVulkanGlue.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Include this file if you want to directly use Vulkan extensions in your code. vkglueExt class is populated automatically.
***********************************************************************************************************************/
#pragma once
#include "PVRPlatformGlue/Vulkan/ApiVulkanGlue.h"
#include "PVRPlatformGlue/Vulkan/NativeLibraryVulkanGlue.h"

/*!*********************************************************************************************************************
\brief      This class contains function pointers for all Vulkan extensions that existed at the time of publishing. Can be updated
           as required. The function pointers get automatically populated on application start during State Machine creation.
		   (function pvr::createNativePlatformContext). Use as a namespace (e.g. vkglueExt::WaitSyncKHR).
***********************************************************************************************************************/
class vkglueExt
{
public:
	static void initVkglueExt();
};

namespace pvr {
/*!*********************************************************************************************************************
\brief         The pvr::native namespace contains low-level classes and functions that are relevant to the underlying Graphics API
***********************************************************************************************************************/
namespace native {
/*!*********************************************************************************************************************
\brief         VkglueExtensions class can be used to facilitate loading and using Vulkan extensions
***********************************************************************************************************************/
class VkglueExtensions
{
public:
	bool supports_VK_Glue_Something_or_other;

	bool isInitialized;

	VkglueExtensions();
	void init(const char* const extensions);
};
}
}
