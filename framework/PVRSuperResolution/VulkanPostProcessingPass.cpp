/*!
\brief Post processing class using Vulkan API (child classes will use either rasterization or compute as an approach for postprocessing)
\file PVRSuperResolution/VulkanPostProcessingPass.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "Log.h"
#include "FileIO.h"
#include "DynamicMap.h"
#include "../include/vk_bindings.h"
#include "VulkanPostProcessingPass.h"

namespace pvr
{

void VulkanPostProcessingPass::init(const VulkanInitializationData& initializationData)
{
	_device = initializationData.device;
	_physicalDevice = initializationData.physicalDevice;
	_imageExtent = (_postprocessingPassOrder != PostprocessingPassOrder::LastPass) ? initializationData.inputImageExtent : initializationData.outputImageExtent;
	_queue = initializationData.queue;

	VkQueueFlagBits requiredQueueFlagBits{ VK_QUEUE_GRAPHICS_BIT };
	if (_postProcessingMethod == PostProcessingMethod::MentisV2NeuralSuperResolution)
	{
		requiredQueueFlagBits = VK_QUEUE_COMPUTE_BIT;
	}

	assertCondition((initializationData.queueFamilyIndex & requiredQueueFlagBits) == 0, "ERROR: Provided queue in VulkanPostProcessingPass::init does not have compute capabilities.");

	_queueFamilyIndex = initializationData.queueFamilyIndex;
	_numberCommandBuffer = initializationData.numberCommandBuffer;
	_vk = initializationData.vk;
	_vkInstance = initializationData.vkInstance;
	_application = initializationData.application;

	_vectorInputImageLayout = initializationData.inputImageLayout;	

	if (_postProcessingAPI == PostProcessingAPI::PostProcessingGraphicsAPIVulkan)
	{
		if (_postprocessingPassOrder == PostprocessingPassOrder::FirstPass) { _vectorVectorInputImageView = initializationData.vectorInputImageView; }

		if (_postprocessingPassOrder == PostprocessingPassOrder::LastPass)
		{
			_vectorOutputImageInitialLayout = { initializationData.outputImageInitialLayout };
			_vectorOutputImageFinalLayout = { initializationData.outputImageFinalLayout };
			_vectorOutputImageFormat = { initializationData.outputImageFormat };

			// The output of this SuperResolution pass is the one specified in VulkanInitializationData::vectorOutputImageView
			adaptVectorDataToVectorVectorData(initializationData.vectorOutputImageView, _vectorVectorOutputImageView);
		}
	}
	else if (_postProcessingAPI == PostProcessingAPI::PostProcessingComputeAPIVulkan)
	{
		// Current compute pass API is supposed to be the only pass present		
		_vectorVectorInputImageView = initializationData.vectorInputImageView;
		adaptVectorDataToVectorVectorData(initializationData.vectorOutputImageView, _vectorVectorOutputImageView);
	}
	
	_vulkanResourceAllocator = VulkanResourceAllocator(_device, _physicalDevice, _vk, _vkInstance, _application);

	_combinedImageSamplerDescriptorCount = 0;

	if (_vectorVectorInputImageView.size() > 0)
	{
		_combinedImageSamplerDescriptorCount += static_cast<int>(_vectorVectorInputImageView[0].size()) * static_cast<int>(_numberCommandBuffer);
	}
	if (_vectorVectorOutputImageView.size() > 0)
	{
		_combinedImageSamplerDescriptorCount += static_cast<int>(_vectorVectorOutputImageView[0].size()) * static_cast<int>(_numberCommandBuffer);
	}

	_combinedImageSamplerDescriptorCount += static_cast<int>(_vectorBuffer.size()) * static_cast<int>(_numberCommandBuffer);

	// Initialize all Vulkan objects
	buildCommandPool();
	buildSampler();
	buildBuffers();
	buildDescriptorPool();
	buildRenderPass();

	// First and intermediate passes will generate one image and image view as output for each element in _vectorOutputImageFormat,
	// This process is repeated _numberCommandBuffer times
	if ((_postprocessingPassOrder == PostprocessingPassOrder::FirstPass) || (_postprocessingPassOrder == PostprocessingPassOrder::IntermediatePass))
	{
		buildImages();
		buildImageViews();
	}

	buildFramebuffers();
	buildDescriptorSetLayouts();
	allocateDescriptorSets();
	updateDescriptorSets();
	buildPipelineLayouts();
	buildPipelines();
}

void VulkanPostProcessingPass::buildCommandPool()
{
	// Build command pool
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	commandPoolCreateInfo.queueFamilyIndex = _queueFamilyIndex;
	assertFunctionResult(_vk->vkCreateCommandPool(_device, &commandPoolCreateInfo, nullptr, &_commandPool), "ERROR: vkCreateCommandPool.\n");
}

void VulkanPostProcessingPass::shutdown()
{
	destroyVulkanObjectVector<PFN_vkDestroyPipeline, VkPipeline>(_device, _vk->vkDestroyPipeline, _vectorPipeline);
	destroyVulkanObjectVector<PFN_vkDestroyFramebuffer, VkFramebuffer>(_device, _vk->vkDestroyFramebuffer, _vectorFramebuffer);
	destroyVulkanObjectVector<PFN_vkDestroyDescriptorSetLayout, VkDescriptorSetLayout>(_device, _vk->vkDestroyDescriptorSetLayout, _vectorDescriptorSetLayout);	
	destroyVulkanObjectVector<PFN_vkDestroyPipelineLayout, VkPipelineLayout>(_device, _vk->vkDestroyPipelineLayout, _vectorPipelineLayout);	
	destroyVulkanObject<PFN_vkDestroySampler, VkSampler>(_device, _vk->vkDestroySampler, _sampler);
	destroyVulkanObject<PFN_vkDestroyDescriptorPool, VkDescriptorPool>(_device, _vk->vkDestroyDescriptorPool, _descriptorPool);
	destroyVulkanObject<PFN_vkDestroyCommandPool, VkCommandPool>(_device, _vk->vkDestroyCommandPool, _commandPool);

	// Each pass will receive input either from the application or from a previous pass, being it the output of that pass.
	// No need to destroy input images or image views

	destroyVulkanObjectVectorOfVectors<PFN_vkDestroyImage, VkImage>(_device, _vk->vkDestroyImage, _vectorVectorOutputImage);
	destroyVulkanObjectVectorOfVectors<PFN_vkFreeMemory, VkDeviceMemory>(_device, _vk->vkFreeMemory, _vectorVectorOutputImageDeviceMemory);

	// Each pass will output to either an image built by the pass or to an image provided by the application using the
	// library. Destroy image views only if built by the SuperResolution pass.

	if (_postprocessingPassOrder != PostprocessingPassOrder::LastPass)
	{
		destroyVulkanObjectVectorOfVectors<PFN_vkDestroyImageView, VkImageView>(_device, _vk->vkDestroyImageView, _vectorVectorOutputImageView);
	}

	destroyVulkanObjectVector<PFN_vkDestroyBuffer, VkBuffer>(_device, _vk->vkDestroyBuffer, _vectorBuffer);
	destroyVulkanObjectVector<PFN_vkFreeMemory, VkDeviceMemory>(_device, _vk->vkFreeMemory, _vectorBufferDeviceMemory);
}

void VulkanPostProcessingPass::allocateDescriptorSets()
{	
	_vectorDescriptorSet.resize(_numberCommandBuffer);
	_vulkanResourceAllocator.allocateDescriptorSets(_vectorDescriptorSetLayout[0], _descriptorPool, _vectorDescriptorSet);
}

void VulkanPostProcessingPass::updateDescriptorSets()
{
	// Write descriptor sets: Read from as many textures as in _vectorVectorInputImageView[0].size()
	// In case the descriptor set is for a compute pass, write to as many images in _vectorVectorOutputImageView[0].size()
	uint32_t numberDescriptorSets = _numberCommandBuffer * static_cast<uint32_t>(_vectorVectorInputImageView[0].size());

	// Compute passes assume elements in _vectorVectorOutputImageView will be written to
	if (_postProcessingAPI == PostProcessingAPI::PostProcessingComputeAPIVulkan)
	{
		numberDescriptorSets += _numberCommandBuffer * static_cast<uint32_t>(_vectorVectorOutputImageView[0].size());
	}

	std::vector<VkDescriptorImageInfo> vectorDescriptorImageInfo(numberDescriptorSets);
	std::vector<VkDescriptorBufferInfo> vectorDescriptorBufferInfo(_numberCommandBuffer * _vectorBuffer.size());
	std::vector<VkWriteDescriptorSet> vectorWriteDescriptorSet(vectorDescriptorImageInfo.size() + vectorDescriptorBufferInfo.size());

	VkWriteDescriptorSet writeDescriptorSet = {};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.dstArrayElement = 0;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	int writeDescriptorIndex = 0;
	int descriptorImageInfoIndex = 0;
	int descriptorBufferInfoIndex = 0;

	for (uint32_t i = 0; i < _numberCommandBuffer; ++i)
	{
		uint32_t bindingIndex = 0;

		// First all the input images will be set as descriptors for sampling
		for (size_t j = 0; j < _vectorVectorInputImageView[0].size(); ++j)
		{
			vectorDescriptorImageInfo[descriptorImageInfoIndex] = {};
			vectorDescriptorImageInfo[descriptorImageInfoIndex].sampler = _sampler;
			vectorDescriptorImageInfo[descriptorImageInfoIndex].imageView = _vectorVectorInputImageView[i][j];

			if (_postprocessingPassOrder == PostprocessingPassOrder::SinglePass)
			{
				vectorDescriptorImageInfo[descriptorImageInfoIndex].imageLayout = _vectorInputImageLayout[j];
			}
			else { vectorDescriptorImageInfo[descriptorImageInfoIndex].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; }

			writeDescriptorSet.dstSet = _vectorDescriptorSet[i];
			writeDescriptorSet.pImageInfo = &vectorDescriptorImageInfo[descriptorImageInfoIndex];
			writeDescriptorSet.dstBinding = bindingIndex;
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			vectorWriteDescriptorSet[writeDescriptorIndex] = writeDescriptorSet;

			descriptorImageInfoIndex++;
			bindingIndex++;
			writeDescriptorIndex++;
		}

		// Compute passes assume elements in _vectorVectorOutputImageView will be written to
		if (_postProcessingAPI == PostProcessingAPI::PostProcessingComputeAPIVulkan)
		{
			// Then all the output images will be set as descriptors for storage
			for (size_t j = 0; j < _vectorVectorOutputImageView[0].size(); ++j)
			{
				vectorDescriptorImageInfo[descriptorImageInfoIndex] = {};
				vectorDescriptorImageInfo[descriptorImageInfoIndex].sampler = _sampler;
				vectorDescriptorImageInfo[descriptorImageInfoIndex].imageView = _vectorVectorOutputImageView[i][j];
				vectorDescriptorImageInfo[descriptorImageInfoIndex].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

				writeDescriptorSet.dstSet = _vectorDescriptorSet[i];
				writeDescriptorSet.pImageInfo = &vectorDescriptorImageInfo[descriptorImageInfoIndex];
				writeDescriptorSet.dstBinding = bindingIndex;
				writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				vectorWriteDescriptorSet[writeDescriptorIndex] = writeDescriptorSet;

				descriptorImageInfoIndex++;
				bindingIndex++;
				writeDescriptorIndex++;
			}
		}

		// Lastly, all buffers will be added
		for (size_t j = 0; j < _vectorBuffer.size(); ++j)
		{
			vectorDescriptorBufferInfo[descriptorBufferInfoIndex] = {};
			vectorDescriptorBufferInfo[descriptorBufferInfoIndex].buffer = _vectorBuffer[j];
			vectorDescriptorBufferInfo[descriptorBufferInfoIndex].offset = 0;
			vectorDescriptorBufferInfo[descriptorBufferInfoIndex].range = VK_WHOLE_SIZE;

			writeDescriptorSet.dstSet = _vectorDescriptorSet[i];
			writeDescriptorSet.pBufferInfo = &vectorDescriptorBufferInfo[descriptorBufferInfoIndex];
			writeDescriptorSet.dstBinding = bindingIndex;
			writeDescriptorSet.descriptorType = _vectorBufferDescriptorType[j];
			vectorWriteDescriptorSet[writeDescriptorIndex] = writeDescriptorSet;

			descriptorBufferInfoIndex++;
			bindingIndex++;
			writeDescriptorIndex++;
		}

		bindingIndex = 0;
	}

	_vk->vkUpdateDescriptorSets(_device, static_cast<uint32_t>(vectorWriteDescriptorSet.size()), vectorWriteDescriptorSet.data(), 0, nullptr);
}

void VulkanPostProcessingPass::buildPipelineLayouts() { _vectorPipelineLayout.push_back(_vulkanResourceAllocator.buildPipelineLayout(_vectorDescriptorSetLayout)); }

} // namespace pvr
