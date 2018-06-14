/*!
\brief Function definitions for LayersVk header file
\file PVRVk/LayersVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN

#include "PVRVk/LayersVk.h"
#include "PVRCore/Log.h"
#include <sstream>

namespace pvrvk {
namespace Layers {
std::vector<std::string> filterLayers(const std::vector<LayerProperties>& layerProperties, const std::string* layersToEnable, uint32_t layersCount)
{
	std::vector<std::string> outLayers;

	for (uint32_t j = 0; j < layersCount; ++j)
	{
		for (uint32_t i = 0; i < layerProperties.size(); ++i)
		{
			if (!strcmp(layersToEnable[j].c_str(), layerProperties[i].getLayerName()))
			{
				outLayers.push_back(layersToEnable[j]);
				break;
			}
		}
	}
	return outLayers;
}

namespace Instance {

void enumerateInstanceLayersString(std::vector<std::string>& outLayers)
{
	std::vector<LayerProperties> layers;
	enumerateInstanceLayers(layers);
	for (uint32_t i = 0; i < layers.size(); i++)
	{
		outLayers.push_back(layers[i].getLayerName());
	}
}

void enumerateInstanceLayers(std::vector<LayerProperties>& outLayers)
{
	VkBindings vkBindings;
	initVkBindings(&vkBindings);
	uint32_t numItems = 0;
	pvrvk::impl::vkThrowIfFailed(vkBindings.vkEnumerateInstanceLayerProperties(&numItems, nullptr), "LayersVk::Failed to enumerate instance layer properties");
	outLayers.resize(numItems);
	pvrvk::impl::vkThrowIfFailed(vkBindings.vkEnumerateInstanceLayerProperties(&numItems, reinterpret_cast<VkLayerProperties*>(outLayers.data())),
		"LayersVk::Failed to enumerate instance layer properties");
}
} // namespace Instance
} // namespace Layers
} // namespace pvrvk
//!\endcond
