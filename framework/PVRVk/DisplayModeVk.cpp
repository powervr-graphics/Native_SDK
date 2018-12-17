/*!
\brief Function implementations of the Vulkan Device class.
\file PVRVk/DeviceVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN

#include "PVRVk/DisplayModeVk.h"
#include "PVRVk/DisplayVk.h"

namespace pvrvk {
namespace impl {
DisplayMode_::DisplayMode_(PhysicalDeviceWeakPtr physicalDevice, const DisplayModePropertiesKHR& displayModeProperties)
	: PhysicalDeviceObjectHandle(physicalDevice, displayModeProperties.getDisplayMode()), _parameters(displayModeProperties.getParameters())
{}

DisplayMode_::DisplayMode_(PhysicalDeviceWeakPtr physicalDevice, pvrvk::Display& display, const pvrvk::DisplayModeCreateInfo& displayModeCreateInfo)
	: PhysicalDeviceObjectHandle(physicalDevice), _parameters(displayModeCreateInfo.getParameters())
{
	VkDisplayModeCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DISPLAY_MODE_CREATE_INFO_KHR;
	createInfo.flags = static_cast<VkDisplayModeCreateFlagsKHR>(displayModeCreateInfo.getFlags());
	createInfo.parameters = _parameters.get();
	physicalDevice->getInstance()->getVkBindings().vkCreateDisplayModeKHR(physicalDevice->getVkHandle(), display->getVkHandle(), &createInfo, nullptr, &_vkHandle);
}
} // namespace impl
} // namespace pvrvk

//!\endcond
