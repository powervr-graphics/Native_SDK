/*!
\brief Post processing pass using Vulkan API and rasterization
\file PVRSuperResolution/VulkanGraphicsPostProcessingPass.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include <vector>
#include <string>
#include "../include/vulkan/vulkan.h"
#include "../include/vk_bindings.h"
#include "VulkanResourceAllocator.h"
#include "VulkanPostProcessingPass.h"

namespace pvr {

/// <summary>SuperResolution base postprocessing pass.</summary>
class VulkanGraphicsPostProcessingPass : public VulkanPostProcessingPass
{
public:
	/// <summary>Constructor.</summary>
	/// <param name="postprocessingPassOrder">Enum to know what order each specific post processing pass ocuppies (first, intermediate, last).</param>
	/// <param name="postProcessingMethod">Technique implemented by this instance across all available.</param>
	VulkanGraphicsPostProcessingPass(PostprocessingPassOrder postprocessingPassOrder, PostProcessingMethod postProcessingMethod)
		: VulkanPostProcessingPass(postprocessingPassOrder, postProcessingMethod, PostProcessingAPI::PostProcessingGraphicsAPIVulkan)
	{}

	/// <summary>Command buffer to record to the SuperResolution commands for later submission.</summary>
	/// <param name="commandBuffer">Command buffer to record to.</param>
	/// <param name="commandBufferIndex">When the SuperResolution algorithm is initialized, a value is provided in 
	/// VulkanInitializationData::numberCommandBuffer with the amount of command buffers that will be recorded for 
	/// the algorithm (to cover all the swapchain images / internal amount of frames an engine could have). This parameter indicates
	/// what command buffer index in the range [0, VulkanInitializationData::numberCommandBuffer - 1] to record to.</param>
	virtual void recordCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferIndex);

	/// <summary>Standard destructor.</summary>
	virtual ~VulkanGraphicsPostProcessingPass() { shutdown(); }

	/// <summary>Build _numberCommandBuffer internal framebuffers in _vectorRGBAToYUVAFramebuffer used by each SuperResolution method.</summary>
	/// <param name="renderPass">Handle to the Vulkan render pass object used in the framebuffers.</param>
	/// <param name="vectorVectorAttachment">Vector where each index has a vector with the image view and sample attachments,
	/// with as many indices as _numberCommandBuffer, used to build _numberCommandBuffer framebuffers.</param>
	void buildFramebuffer(VkRenderPass renderPass, const std::vector<std::vector<VkImageView>>& vectorVectorAttachment, std::vector<VkFramebuffer>& _vectorFramebuffer);

	/// <summary>Build a graphics pipeline to be used for postprocessing passes.</summary>
	/// <returns>Graphics pipeline with the fragment shader provided as parameter.</returns>
	VkPipeline buildPostProcessingPipeline(VkRenderPass renderPass, VkPipelineLayout pipelineLayout, const std::string& fragmentShaderName, int numberColorAttachment);

	/// <summary>Build a render pass which can be used for post processing effects.</summary>
	/// <param name="vectorAttachmentFormat">Vector with the formats of the color attachments in the render pass.</param>
	/// <param name="attachmentInitialLayout">Initial layout of the color attachments.</param>
	/// <param name="attachmentFinalLayout">Final layout of the color attachments.</param>
	/// <param name="attachmentFinalLayout">Final layout of the color attachments.</param>
	/// <returns>Render pass built.</returns>
	VkRenderPass buildPostProcessRenderPass(
		const std::vector<VkFormat>& vectorAttachmentFormat, VkImageLayout attachmentInitialLayout, VkImageLayout attachmentFinalLayout);

	/// <summary>Build a descriptor pool to allocate descriptor sets from.</summary>
	void buildDescriptorPool();

protected:
	/// <summary>Method to build a sampler used by each SuperResolution pass instance.</summary>
	virtual void buildSampler();

	/// <summary>Method to build the render pass used by each SuperResolution pass instance.</summary>
	virtual void buildRenderPass();

	/// <summary>Method to build the framebuffers used by each SuperResolution pass instance.</summary>
	virtual void buildFramebuffers();

	/// <summary>Method to build all the imaged used by each SuperResolution pass instance (generally framebuffer 
	/// attachments to draw to and pass as input for the next SuperResolution pass).</summary>
	virtual void buildImages();

	/// <summary>Method to build image views of any images used by each SuperResolution pass instance.</summary>
	virtual void buildImageViews();

	/// <summary>Method to build descriptor set layouts used by each SuperResolution pass instance.</summary>
	virtual void buildDescriptorSetLayouts();

	/// <summary>Method to build pipelines used by each SuperResolution pass instance.</summary>
	virtual void buildPipelines(){};
};

} // namespace pvr
