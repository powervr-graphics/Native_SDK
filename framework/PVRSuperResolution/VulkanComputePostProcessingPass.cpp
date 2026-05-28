/*!
\brief Post processing pass using Vulkan API and compute
\file PVRSuperResolution/VulkanComputePostProcessingPass.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "VulkanComputePostProcessingPass.h"

namespace pvr
{

void VulkanComputePostProcessingPass::buildDescriptorSetLayouts()
{
	std::vector<VkDescriptorType> vectorDescriptorType;
	std::vector<VkShaderStageFlagBits> vectorDescriptorStage;

	size_t numberInput = _vectorVectorInputImageView[0].size();
	size_t numberOutput = _vectorVectorOutputImageView[0].size();
	size_t numberBuffer = _vectorBufferDescriptorType.size();

	// Build descriptor set layouts following this order
	// + As many descriptor set layout bindings of type VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER as elements in _vectorVectorInputImageView[0] (assuming other indices have the same number of image views)
	// + As many descriptor set layout bindings of type VK_DESCRIPTOR_TYPE_STORAGE_IMAGE as elements in _vectorVectorOutputImageView[0] (assuming other indices have the same number of image views),
	//   since this is a compute pass, any oututs will be written through image Store operations
	// + As many descriptor set layout bindings of type VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER or VK_DESCRIPTOR_TYPE_STORAGE_BUFFER as elements in _vectorBufferDescriptorType for any possible uniform and storage buffers
	//   being read / written to
	// The descriptor sets of this descriptor set layout will be arranged in incremental indices

	for (uint32_t i = 0; i < numberInput; ++i)
	{
		vectorDescriptorType.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		vectorDescriptorStage.push_back(VK_SHADER_STAGE_COMPUTE_BIT);
	}

	for (uint32_t i = 0; i < numberOutput; ++i)
	{
		vectorDescriptorType.push_back(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		vectorDescriptorStage.push_back(VK_SHADER_STAGE_COMPUTE_BIT);
	}

	for (uint32_t i = 0; i < numberBuffer; ++i)
	{
		vectorDescriptorType.push_back(_vectorBufferDescriptorType[i]);
		vectorDescriptorStage.push_back(VK_SHADER_STAGE_COMPUTE_BIT);
	}

	_vectorDescriptorSetLayout.push_back(_vulkanResourceAllocator.buildDescriptorSetLayouts(vectorDescriptorType, vectorDescriptorStage));
}

} // namespace pvr
