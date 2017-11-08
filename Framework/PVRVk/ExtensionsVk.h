/*!
\brief Functionality that helps management of Vulkan extensions, such as 
enumerating, enabling/disabling
\file PVRVk/ExtensionsVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once

#include "PVRVk/TypesVk.h"

namespace pvrvk {
namespace Extensions {

/// <summary>Filter the extensions</summary>
/// <param name="extensionProperties">Extension properties</param>
/// <param name="extensionsToEnable">Extensions to enable</param>
/// <param name="numExtensions">Number of extensions to enable</param>
/// <returns>std::vector<std::string></returns>
extern std::vector<std::string> filterExtensions(const std::vector<VkExtensionProperties>& extensionProperties,
    const std::string* extensionsToEnable, uint32_t numExtensions);

/// <summary>Filter instance extensions</summary>
/// <param name="extensionsToEnable">Pointer to extensions to filter</param>
/// <param name="extensionsCount">Number of extensions to filter</param>
/// <returns>Filtered extensions</returns>
std::vector<std::string> filterInstanceExtensions(const std::string* extensionsToEnable, uint32_t extensionsCount);

/// <summary>Get list of all supported instance extension properties</summary>
/// <param name="outExtensions">Returned extensions</param>
void enumerateInstanceExtensions(std::vector<ExtensionProperties>& outExtensions);

/// <summary>Get list of all supported instance extension strings</summary>
/// <param name="extStr">Returned extensions</param>
void enumerateInstanceExtensionsString(std::vector<std::string>& extStr);

/// <summary>Query if an Instance Extension is supported</summary>
/// <param name="extension">The extension string</param>
/// <returns>True if the instance supports the extension, otherwise false</param>
bool isInstanceExtensionSupported(const char* extension);

}// namespace Extensions
}// namespace pvrvk
