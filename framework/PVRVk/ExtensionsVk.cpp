/*!
\brief Function definitions for Extensions functionality
\file PVRVk/ExtensionsVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

//!\cond NO_DOXYGEN
#include "PVRVk/ExtensionsVk.h"
#include "PVRCore/Log.h"
namespace pvrvk {
namespace Extensions {

std::vector<std::string> filterExtensions(const std::vector<ExtensionProperties>& extensionProperties, const std::string* extensionsToEnable, uint32_t numExtensions)
{
	std::vector<std::string> outExtensions;
	for (uint32_t i = 0; i < extensionProperties.size(); ++i)
	{
		for (uint32_t j = 0; j < numExtensions; ++j)
		{
			if (!strcmp(extensionsToEnable[j].c_str(), extensionProperties[i].getExtensionName()))
			{
				outExtensions.push_back(extensionsToEnable[j]);
				break;
			}
		}
	}
	return outExtensions;
}

std::vector<std::string> filterInstanceExtensions(const std::string* extensionsToEnable, uint32_t numExtensions)
{
	std::vector<ExtensionProperties> extprops;
	enumerateInstanceExtensions(extprops);
	return Extensions::filterExtensions(extprops, extensionsToEnable, numExtensions);
}

void enumerateInstanceExtensionsString(std::vector<std::string>& extStr)
{
	std::vector<ExtensionProperties> extensionProps;
	enumerateInstanceExtensions(extensionProps);
	extStr.resize(extensionProps.size());
	for (uint32_t i = 0; i < extensionProps.size(); i++)
	{
		extStr[i] = extensionProps[i].getExtensionName();
	}
}

void enumerateInstanceExtensions(std::vector<ExtensionProperties>& outExtensionProps)
{
	VkBindings vkBindings;
	initVkBindings(&vkBindings);
	uint32_t numItems = 0;
	pvrvk::impl::vkThrowIfFailed(vkBindings.vkEnumerateInstanceExtensionProperties(nullptr, &numItems, nullptr), "ExtensionsVk::Failed to enumerate instance extension properties");
	outExtensionProps.resize(numItems);
	pvrvk::impl::vkThrowIfFailed(vkBindings.vkEnumerateInstanceExtensionProperties(nullptr, &numItems, (VkExtensionProperties*)outExtensionProps.data()),
		"ExtensionsVk::Failed to enumerate instance extension properties");
}

bool isInstanceExtensionSupported(const char* extension)
{
	std::vector<ExtensionProperties> extensions;
	enumerateInstanceExtensions(extensions);
	for (uint32_t i = 0; i < extensions.size(); ++i)
	{
		if (!strcmp(extensions[i].getExtensionName(), extension))
		{
			return true;
		}
	}
	return false;
}
} // namespace Extensions
} // namespace pvrvk

//!\endcond
