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

std::vector<std::string> filterExtensions(const std::vector<VkExtensionProperties>& extensionProperties,
    const std::string* extensionsToEnable, uint32_t numExtensions)
{
	std::vector<std::string> outExtensions;
	for (uint32_t i = 0; i < extensionProperties.size(); ++i)
	{
		for (uint32_t j = 0; j < numExtensions; ++j)
		{
			if (!strcmp(extensionsToEnable[j].c_str(), extensionProperties[i].extensionName))
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
		extStr[i] = extensionProps[i].extensionName;
	}
}

void enumerateInstanceExtensions(std::vector<VkExtensionProperties>& outExtensionProps)
{
	vk::initVulkan();
	uint32_t numItems = 0;
	assert(vk::EnumerateInstanceExtensionProperties);
	vk::EnumerateInstanceExtensionProperties(nullptr, &numItems, nullptr);
	outExtensionProps.resize(numItems);
	vk::EnumerateInstanceExtensionProperties(nullptr, &numItems, outExtensionProps.data());
}

bool isInstanceExtensionSupported(const char* extension)
{
	std::vector<ExtensionProperties> extensions;
	enumerateInstanceExtensions(extensions);
	for (uint32_t i = 0; i < extensions.size(); ++i)
	{
		if (!strcmp(extensions[i].extensionName, extension))
		{
			return true;
		}
	}
	return false;
}
}// namespace Extensions
}// namespace pvrvk

//!\endcond
