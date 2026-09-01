/*!
\brief Post processing pass implementing Mentis v2 (Neural Super Resolution) through a compute pass
\file PVRSuperResolution/MentisV2NeuralSuperResolution.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include <chrono>
#include <regex>
#include <fstream>

#include "MentisV2NeuralSuperResolution.h"
#include "Log.h"
#include "FileIO.h"
#include "DynamicMap.h"
#include "MathUtil.h"
#include "NRSWeights.h"

const char* computeShaderName = "MentisV2NeuralSuperResolution.csh.spv";

namespace pvr
{

void MentisV2NeuralSuperResolution::buildDescriptorPool()
{
	std::vector<VkDescriptorType> vectorDescriptorType = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER };

	std::vector<int> vectorDescriptorCount = { _combinedImageSamplerDescriptorCount * 2, _combinedImageSamplerDescriptorCount * 2, _combinedImageSamplerDescriptorCount * 2,
		_combinedImageSamplerDescriptorCount * 2, _combinedImageSamplerDescriptorCount * 2 };

	// Build descriptor pool
	std::vector<VkDescriptorPoolSize> vectorDescriptorPoolSize(vectorDescriptorType.size());

	for (size_t i = 0; i < vectorDescriptorPoolSize.size(); ++i)
	{
		vectorDescriptorPoolSize[i] = {};
		vectorDescriptorPoolSize[i].type = vectorDescriptorType[i];
		vectorDescriptorPoolSize[i].descriptorCount = vectorDescriptorCount[i];
	}

	_descriptorPool = _vulkanResourceAllocator.buildDescriptorPool(vectorDescriptorType, vectorDescriptorCount);
}

void MentisV2NeuralSuperResolution::recordCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferIndex)
{
	// TODO: Build uniform buffer with per-frame information (one per swapchain image) and bind the corresponding one here

	const int M = _imageExtent.width * _imageExtent.height;
	const int iM = 16;

	_vk->vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _vectorPipeline[0]);
	_vk->vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _vectorPipelineLayout[0], 0, 1, &_vectorDescriptorSet[commandBufferIndex], 0, nullptr);
	_vk->vkCmdDispatch(commandBuffer, 1, M / iM, 1);
}

void MentisV2NeuralSuperResolution::frameUpdate(int swapchainIndex)
{
	std::vector<float> vectorData = 
	{
		_dynamicMap->getValue("jitterX", -999.0f),
		_dynamicMap->getValue("jitterY", -999.0f),
		_dynamicMap->getValue("frameCounter", -999.0f)
	};

	_vulkanResourceAllocator.copyInformationToHostVisibleMemory(_vectorBufferDeviceMemory[0], vectorData.data(), sizeof(float) * vectorData.size());
}

void MentisV2NeuralSuperResolution::buildSampler()
{
	// Build linear sampler
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.flags = 0;
	samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.anisotropyEnable = VK_FALSE;
	samplerCreateInfo.maxAnisotropy = 1.0f;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_TRUE;
	assertFunctionResult(_vk->vkCreateSampler(_device, &samplerCreateInfo, nullptr, &_sampler), "ERROR: vkCreateSampler.\n");
}

/// <summary>Method to build any buffers required.</summary>
void MentisV2NeuralSuperResolution::buildBuffers()
{
	// 1. Build a buffer for the per-frame jitter in _vectorBuffer[0]

	// 2. Build a buffer to contain the weights of the residual layers ("matrix_b" in the MentisV2NeuralSuperResolution compute shader) in _vectorBuffer[1]
	// The values are currently hardcoded in NRSWeights.h and is material that CANNOT BE RELELASED PUBLICLY because of
	// the training dataset used (I guess it can be handled under request?)

	// 3. Build a buffer to contain the weights of the residual layers ("matrix_b_l1" in the MentisV2NeuralSuperResolution compute shader) in _vectorBuffer[2]
	// The values are currently hardcoded in NRSWeights.h and is material that CANNOT BE RELELASED PUBLICLY because of
	// the training dataset used (I guess it can be handled under request?)

	// 4. Debug buffer
	_vectorBuffer.resize(4);
	_vectorBufferDeviceMemory.resize(4);

	_vectorBufferDescriptorType.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	_vectorBufferDescriptorType.push_back(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	_vectorBufferDescriptorType.push_back(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	_vectorBufferDescriptorType.push_back(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

	std::vector<VkBufferUsageFlags> vectorBufferUsageFlags = {
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
	};

	std::vector<VkMemoryPropertyFlagBits> vectorMemoryPropertyFlags = {
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	};

	std::vector<uint8_t> vectorFP16Converted;
	pvr::convertByteArrayFloat32ToFloat16(vector_matrix_b, vectorFP16Converted);
	VkDeviceSize bufferByteSize = static_cast<int>(vectorFP16Converted.size());

	std::vector<uint8_t> vectorFP16Converted2;
	pvr::convertByteArrayFloat32ToFloat16(vector_matrix_b_l1, vectorFP16Converted2);
	VkDeviceSize bufferByteSize2 = static_cast<int>(vectorFP16Converted2.size());
	
	// TODO: Remove once the sample is considered as completed
	std::vector<VkDeviceSize> vectorBufferSize = {
		3 * sizeof(float),
		bufferByteSize,
		bufferByteSize2,
		2097152
	};

	std::vector<void*> vectorBufferData = { 
		nullptr, 
		vectorFP16Converted.data(), 
		vectorFP16Converted2.data(), 
		nullptr
	};

	for (size_t i = 0; i < _vectorBuffer.size(); ++i)
	{
		_vulkanResourceAllocator.buildBuffer(vectorBufferSize[i], vectorBufferUsageFlags[i], 1, &_queueFamilyIndex, vectorMemoryPropertyFlags[i],
			vectorBufferData[i], _vectorBuffer[i], _vectorBufferDeviceMemory[i]);
	}	
}

void MentisV2NeuralSuperResolution::buildPipelines()
{
	_vectorPipeline.push_back(_vulkanResourceAllocator.buildComputePipeline(_vectorPipelineLayout[0], _shaderPath + std::string(computeShaderName)));
}

} // namespace pvr
