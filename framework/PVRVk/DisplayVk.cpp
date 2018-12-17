/*!
\brief Function implementations of the Vulkan Device class.
\file PVRVk/DeviceVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN

#include "PVRVk/DisplayVk.h"
#include "PVRVk/DisplayModeVk.h"

namespace pvrvk {
namespace impl {
Display_::Display_(PhysicalDeviceWeakPtr physicalDevice, const DisplayPropertiesKHR& displayProperties)
	: PhysicalDeviceObjectHandle(physicalDevice, displayProperties.getDisplay()), _properties(displayProperties)
{
	const InstanceWeakPtr& instance = getPhysicalDevice()->getInstance();
	if (instance->isInstanceExtensionEnabled(VK_KHR_DISPLAY_EXTENSION_NAME))
	{
		uint32_t numModes = 0;
		instance->getVkBindings().vkGetDisplayModePropertiesKHR(getPhysicalDevice()->getVkHandle(), getVkHandle(), &numModes, NULL);
		std::vector<VkDisplayModePropertiesKHR> displayModePropertiesVk;
		displayModePropertiesVk.resize(numModes);
		instance->getVkBindings().vkGetDisplayModePropertiesKHR(getPhysicalDevice()->getVkHandle(), getVkHandle(), &numModes, displayModePropertiesVk.data());

		for (uint32_t i = 0; i < numModes; ++i)
		{
			DisplayModePropertiesKHR displayModeProperties = displayModePropertiesVk[i];
			_displayModes.push_back(DisplayMode_::createNew(getPhysicalDevice(), displayModeProperties));
		}
	}
	else
	{
		throw ErrorUnknown("Display Extension must be enabled when creating the VkInstance.");
	}
}
} // namespace impl
} // namespace pvrvk

//!\endcond
