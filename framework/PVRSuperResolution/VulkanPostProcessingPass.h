/*!
\brief Post processing class using Vulkan API (child classes will use either rasterization or compute as an approach for postprocessing)
\file PVRSuperResolution/VulkanPostProcessingPass.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include <vector>
#include "../include/vulkan/vulkan.h"
#include "VulkanResourceAllocator.h"
#include "PostProcessingPass.h"

namespace pvr {

/// <summary>SuperResolution base postprocessing pass.</summary>
class VulkanPostProcessingPass : public PostProcessingPass
{
public:
	/// <summary>Constructor.</summary>
	/// <param name="postprocessingPassOrder">Enum to know what order each specific post processing pass ocuppies (first, intermediate, last).</param>
	/// <param name="postProcessingMethod">Technique implemented by this instance across all available.</param>
	/// <param name="postProcessingAPI">API and Compute / Graphics approach used by this instance.</param>
	VulkanPostProcessingPass(PostprocessingPassOrder postprocessingPassOrder, PostProcessingMethod postProcessingMethod, PostProcessingAPI postProcessingAPI)
		: PostProcessingPass(postprocessingPassOrder, postProcessingMethod, postProcessingAPI)
	{}

	/// <summary>Generate resources needed by the chosen method.</summary>
	/// <param name="initializationData">Struct with all the information required by the method.</param>
	virtual void init(const VulkanInitializationData& initializationData);

	/// <summary>Command buffer to record to the SuperResolution commands for later submission.</summary>
	/// <param name="VkCommandBuffer">Command buffer to record to.</param>
	/// <param name="uint32_t">When the SuperResolution algorithm is initialized, a value is provided in
	/// VulkanComputeInitializationData::numberCommandBuffer with the amount of command buffers that will be recorded for
	/// the algorithm (to cover all the swapchain images / internal amount of frames an engine could have). This parameter indicates
	/// what command buffer index in the range [0, VulkanComputeInitializationData::numberCommandBuffer - 1] to record to.</param>
	virtual void recordCommands(VkCommandBuffer, uint32_t) {}

	/// <summary>Method called from the client to allow the library to run any per-frame updates like updating
	/// Uniform Buffer Object contents before submitting recorded commands from a specific swapchain index.</summary>
	/// <param name="int">Index of the swapchain used byt he current frame update.</param>
	virtual void frameUpdate(int) {}

	/// <summary>Destroy any possible resources allocated by the chosen SuperResolution method.
	/// Important note: This method has to be called once all GPU tasks have finished, otherwise resources
	/// still in use might be destroy.</summary>
	virtual void shutdown();

	/// <summary>Standard destructor.</summary>
	virtual ~VulkanPostProcessingPass() { shutdown(); }

	/// <summary>Build a descriptor pool to allocate descriptor sets from.</summary>
	virtual void buildDescriptorPool() {}

	/// <summary>Reference to _vectorVectorInputImageView.</summary>
	std::vector<std::vector<VkImageView>>& refVectorVectorInputImageView() { return _vectorVectorInputImageView; }

	/// <summary>Reference to _vectorVectorInputImage.</summary>
	std::vector<std::vector<VkImage>>& refVectorVectorInputImage() { return _vectorVectorInputImage; }

	/// <summary>Reference to _vectorVectorOutputImageView.</summary>
	std::vector<std::vector<VkImageView>>& refVectorVectorOutputImageView() { return _vectorVectorOutputImageView; }

	/// <summary>Reference to _vectorVectorOutputImage.</summary>
	std::vector<std::vector<VkImage>>& refVectorVectorOutputImage() { return _vectorVectorOutputImage; }

	/// <summary>Setter of _vectorVectorInputImageView.</summary>
	/// <param name="vectorData">Value to set.</param>
	void setVectorVectorInputImageView(const std::vector<std::vector<VkImageView>>& vectorData) { _vectorVectorInputImageView = vectorData; }

	/// <summary>Setter of _vectorVectorInputImage.</summary>
	/// <param name="vectorData">Value to set.</param>
	void setVectorVectorInputImage(const std::vector<std::vector<VkImage>>& vectorData) { _vectorVectorInputImage = vectorData; }

	/// <summary>Setter of _vectorVectorOutputImageView.</summary>
	/// <param name="vectorData">Value to set.</param>
	void setVectorVectorOutputImageView(const std::vector<std::vector<VkImageView>>& vectorData) { _vectorVectorOutputImageView = vectorData; }

	/// <summary>Setter of _vectorVectorOutputImage.</summary>
	/// <param name="vectorData">Value to set.</param>
	void setVectorVectorOutputImage(const std::vector<std::vector<VkImage>>& vectorData) { _vectorVectorOutputImage = vectorData; }

protected:
	/// <summary>Method to build a command pool used by each SuperResolution pass instance.</summary>
	virtual void buildCommandPool();

	/// <summary>Method to build a sampler used by each SuperResolution pass instance.</summary>
	virtual void buildSampler() {}

	/// <summary>Method to build the render pass used by each SuperResolution pass instance.</summary>
	virtual void buildRenderPass() {}

	/// <summary>Method to build the framebuffers used by each SuperResolution pass instance.</summary>
	virtual void buildFramebuffers() {}

	/// <summary>Method to build all the imaged used by each SuperResolution pass instance (generally framebuffer
	/// attachments to draw to and pass as input for the next SuperResolution pass).</summary>
	virtual void buildImages() {}

	/// <summary>Method to build image views of any images used by each SuperResolution pass instance.</summary>
	virtual void buildImageViews() {}

	/// <summary>Method to build any buffers required.</summary>
	virtual void buildBuffers() {}

	/// <summary>Method to build descriptor set layouts used by each SuperResolution pass instance.</summary>
	virtual void buildDescriptorSetLayouts() {}

	/// <summary>Method to allocate descriptor sets used by each SuperResolution pass instance.</summary>
	virtual void allocateDescriptorSets();

	/// <summary>Method updated those descriptor set allocated and used by each SuperResolution pass instance.</summary>
	virtual void updateDescriptorSets();

	/// <summary>Method to build pipeline layouts used by each SuperResolution pass instance.</summary>
	virtual void buildPipelineLayouts();

	/// <summary>Method to build pipelines used by each SuperResolution pass instance.</summary>
	virtual void buildPipelines() {}

	/// <summary>Vector with handles to the framebuffers used by the SuperResolution algorithms.</summary>
	std::vector<VkFramebuffer> _vectorFramebuffer;

	/// <summary>Render pass used by the SuperResolution algorithms.</summary>
	VkRenderPass _renderPass{ VK_NULL_HANDLE };

	/// <summary>Vector with the initial image layout of the output images for this pass.</summary>
	std::vector<VkFormat> _vectorInputFormat;

	/// <summary>Vector with the images provided as input for this pass.</summary>
	std::vector<std::vector<VkImage>> _vectorVectorInputImage;

	/// <summary>Vector with the image views provided as input for this pass.</summary>
	std::vector<std::vector<VkImageView>> _vectorVectorInputImageView;

	/// <summary></summary>
	std::vector<VkImageLayout> _vectorInputImageLayout;

	/// <summary>Vector with the images this pass will build and draw to.</summary>
	std::vector<std::vector<VkImage>> _vectorVectorOutputImage;

	/// <summary>Vector with the memory allocations of the images built by this pass.</summary>
	std::vector<std::vector<VkDeviceMemory>> _vectorVectorOutputImageDeviceMemory;

	/// <summary>Vector with the image views used to draw to for this pass (this image views might be
	/// generated by this pass if _postprocessingPassOrder is PostprocessingPassOrder::FirstPass or
	/// PostprocessingPassOrder::IntermediatePass, or provided by the application using the library
	/// if _postprocessingPassOrder is PostprocessingPassOrder::LastPass.</summary>
	std::vector<std::vector<VkImageView>> _vectorVectorOutputImageView;

	/// <summary>Extent of the 2D image used as output.</summary>
	VkExtent2D _imageExtent = {};

	/// <summary>Vulkan logical device.</summary>
	VkDevice _device{ VK_NULL_HANDLE };

	/// <summary>Vulkan physical device.</summary>
	VkPhysicalDevice _physicalDevice{ VK_NULL_HANDLE };

	std::vector<VkDescriptorSetLayout> _vectorDescriptorSetLayout;

	/// <summary>Descriptor sets used.</summary>
	std::vector<VkDescriptorSet> _vectorDescriptorSet;

	/// <summary>Number of command buffers that will be used to record to the commands for the chosen SuperResolution
	/// algorithm. Each time a call to recordCommands() is done, an internal counter increases and if it surpasses the
	/// value of numberCommandBuffer, an assert will be triggered.</summary>
	uint32_t _numberCommandBuffer{ 0 };

	/// <summary>Number of times the call to record command buffer for the chosen SuperResolution
	/// algorithm, calling recordCommands(), has been done. If this value surpasses _numberCommandBuffer an
	/// assert will be triggered.</summary>
	uint32_t _recordedCommandBuffer{ 0 };

	/// <summary>Physical device memory properties, for any memory allocations needed.</summary>
	VkPhysicalDeviceMemoryProperties _vkPhysicalDeviceMemoryProperties = {};

	/// <summary>Vulkan device bindings for all the Vulkan API calls that will be done.</summary>
	const VkDeviceBindings* _vk{ nullptr };

	/// <summary>Vulkan instance bindings for all the instance Vulkan API calls that will be done.</summary>
	const VkInstanceBindings* _vkInstance{ nullptr };

	/// <summary>Pointer to the application running using this library.</summary>
	void* _application{ nullptr };

	/// <summary>Graphics capabilities queue to submit internal initialization commands to.</summary>
	VkQueue _queue{ VK_NULL_HANDLE };

	/// <summary>Graphics capabilities queue index to submit internal initialization commands to.</summary>
	uint32_t _queueFamilyIndex{ static_cast<uint32_t>(-1) };

	/// <summary>Command pool used to allocate commands internally used by the SuperResolution algorithms.</summary>
	VkCommandPool _commandPool{ VK_NULL_HANDLE };

	/// <summary>Descriptor set pool used to allocate descriptor sets internally used by the SuperResolution algorithms.</summary>
	VkDescriptorPool _descriptorPool{ VK_NULL_HANDLE };

	/// <summary>Sampler used by all SuperResolution passes.</summary>
	VkSampler _sampler { VK_NULL_HANDLE};

	/// <summary>Vector with the pipelines used by this pass.</summary>
	std::vector<VkPipeline> _vectorPipeline;

	/// <summary>Vector with the pipeline layouts used by this pass.</summary>
	std::vector<VkPipelineLayout> _vectorPipelineLayout;

	/// <summary>Vector with the initial image layout of the output images for this pass.</summary>
	std::vector<VkImageLayout> _vectorOutputImageInitialLayout;

	/// <summary>Vector with the final image layout of the output images for this pass.</summary>
	std::vector<VkImageLayout> _vectorOutputImageFinalLayout;

	/// <summary>Vector with the final image layout of the output images for this pass.</summary>
	std::vector<VkFormat> _vectorOutputImageFormat;

	/// <summary>Number of total image sampler descriptors allocated for this pass.</summary>
	int _combinedImageSamplerDescriptorCount{ 0 };

	/// <summary>Vector with the buffers used.</summary>
	std::vector<VkBuffer> _vectorBuffer;

	/// <summary>Vector with the type of descriptor used for each element in _vectorBuffer (should be 
	/// either VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER or VK_DESCRIPTOR_TYPE_STORAGE_BUFFER).</summary>
	std::vector<VkDescriptorType> _vectorBufferDescriptorType;

	/// <summary>Vector with the memory used for each element in _vectorBuffer.</summary>
	std::vector<VkDeviceMemory> _vectorBufferDeviceMemory;

	/// <summary>Helper to allocate Vulkan resources.</summary>
	VulkanResourceAllocator _vulkanResourceAllocator;
};

} // namespace pvr
