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
std::vector<std::string> filterLayers(const std::vector<VkLayerProperties>& layerProperties, const std::string* layersToEnable,
                                      uint32_t layersCount)
{
	std::vector<std::string> outLayers;
	for (uint32_t i = 0; i < layerProperties.size(); ++i)
	{
		for (uint32_t j = 0; j < layersCount; ++j)
		{
			if (!strcmp(layersToEnable[j].c_str(), layerProperties[i].layerName))
			{
				outLayers.push_back(layersToEnable[j]);
			}
		}
	}
	return outLayers;
}

namespace Instance {

void enumerateInstanceLayersString(std::vector<std::string>& outLayers)
{
	std::vector<VkLayerProperties> layers;
	enumerateInstanceLayers(layers);
	for (uint32_t i = 0; i < layers.size(); i++)
	{
		outLayers.push_back(layers[i].layerName);
	}
}

void enumerateInstanceLayers(std::vector<LayerProperties>& outLayers)
{
	vk::initVulkan();
	uint32_t numItems = 0;
	vk::EnumerateInstanceLayerProperties(&numItems, nullptr);
	outLayers.resize(numItems);
	vk::EnumerateInstanceLayerProperties(&numItems, (VkLayerProperties*)outLayers.data());
}

}// namespace Instance
}// namespace Layers
}// namespace pvrvk

//!\endcond