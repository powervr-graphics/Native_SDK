/*!
\brief Function implementations for the DebugReportCallback class.
\file PVRVk/DebugReportCallbackVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/DebugReportCallbackVk.h"
#include "PVRVk/InstanceVk.h"

namespace pvrvk {
namespace impl {
DebugReportCallback_::DebugReportCallback_(InstanceWeakPtr instance, const DebugReportCallbackCreateInfo& createInfo) : InstanceObjectHandle(instance)
{
	// Setup callback creation information
	VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
	callbackCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT);
	callbackCreateInfo.pNext = nullptr;
	callbackCreateInfo.flags = static_cast<VkDebugReportFlagsEXT>(createInfo.getFlags());
	callbackCreateInfo.pfnCallback = createInfo.getCallback();
	callbackCreateInfo.pUserData = createInfo.getPUserData();

	if (_instance->isInstanceExtensionEnabled(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
	{
		// Register the DebugReportCallback
		vkThrowIfFailed(
			_instance->getVkBindings().vkCreateDebugReportCallbackEXT(_instance->getVkHandle(), &callbackCreateInfo, nullptr, &_vkHandle), "Failed to create DebugReportCallback");
		_enabled = true;
	}
	else
	{
		_enabled = false;
	}
}

DebugReportCallback_::~DebugReportCallback_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (_instance.isValid())
		{
			if (isEnabled())
			{
				_instance->getVkBindings().vkDestroyDebugReportCallbackEXT(_instance->getVkHandle(), getVkHandle(), nullptr);
			}
			_instance.reset();
		}
		else
		{
			Log(LogLevel::Warning, "Attempted to destroy object of type [%s] after its corresponding instance", "DebugReportCallback");
		}
	}
}
} // namespace impl
} // namespace pvrvk
