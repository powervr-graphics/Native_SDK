/*!
\brief Functionality for working with and managing the Vulkan Layers,
such as enumerating, enabling/disabling lists of them and similar functionality
\file PVRVk/LayersVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once

#include "PVRVk/TypesVk.h"

namespace pvrvk {
namespace Layers {
/// <summary>Filter layers</summary>
/// <param name="layerProperties"> Supportted layers</param>
/// <param name="layersToEnable">layers to filter</param>
/// <param name="layersCount">Number of layers to filter</param>
/// <returns>Filtered layers</returns>
std::vector<std::string> filterLayers(const std::vector<VkLayerProperties>& layerProperties,
                                      const std::string* layersToEnable, uint32_t layersCount);

namespace Instance {

/// <summary>Enumerate instance layers strings</summary>
/// <param name="outLayers">Out strings</param>
void enumerateInstanceLayersString(std::vector<std::string>& outLayers);

/// <summary>Enumerate instance layers</summary>
/// <param name="outLayers">Out layers</param>
void enumerateInstanceLayers(std::vector<LayerProperties>& outLayers);
}// namespace Instance
}// namespace Layers
}// namespace pvrvk
