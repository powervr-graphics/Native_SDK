/*!
\brief Implementation of methods of SuperNova interface class
\file PVRSupernova/VulkanPostProcessingPass.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "VulkanPostProcessingPass.h"
#include "Log.h"
#include "FileIO.h"

const char* vertexShaderName = "AttributelessVertexShader.vsh.spv";

namespace pvr
{

void VulkanPostProcessingPass::init(const PostProcessingInitializationData& initializationData)
{
	_device = initializationData.device;
	_physicalDevice = initializationData.physicalDevice;
	_imageExtent = (_postprocessingPassOrder != PostprocessingPassOrder::LastPass) ? initializationData.inputImageExtent : initializationData.outputImageExtent;
	_graphicsQueue = initializationData.graphicsQueue;
	_graphicsQueueFamilyIndex = initializationData.graphicsQueueFamilyIndex;
	_numberCommandBuffer = initializationData.numberCommandBuffer;
	_vk = initializationData.vk;
	_vkInstance = initializationData.vkInstance;
	_application = initializationData.application;

	if (_postprocessingPassOrder == PostprocessingPassOrder::FirstPass)
	{
		// The input of this Supernova pass is the one specified in PostProcessingInitializationData::vectorInputImageView
		adaptVectorDataToVectorVectorData(initializationData.vectorInputImageView, _vectorVectorInputImageView);
	}

	if (_postprocessingPassOrder == PostprocessingPassOrder::LastPass)
	{
		_vectorOutputImageInitialLayout = { initializationData.outputImageInitialLayout };
		_vectorOutputImageFinalLayout = { initializationData.outputImageFinalLayout };
		_vectorOutputImageFormat = { initializationData.outputImageFormat };

		// The output of this Supernova pass is the one specified in PostProcessingInitializationData::vectorOutputImageView
		adaptVectorDataToVectorVectorData(initializationData.vectorOutputImageView, _vectorVectorOutputImageView);
	}

	_combinedImageSamplerDescriptorCount = static_cast<int>(_vectorVectorInputImageView[0].size()) * static_cast<int>(_numberCommandBuffer);

	// Initialize all Vulkan objects
	buildCommandPool();
	buildSampler();
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

void VulkanPostProcessingPass::recordCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferIndex)
{
	std::vector<VkClearValue> vectorClearValue(_vectorOutputImageFormat.size());
	for (size_t i = 0; i < vectorClearValue.size(); ++i)
	{
		vectorClearValue[i].color.float32[0] = 0.0f;
		vectorClearValue[i].color.float32[1] = 0.0f;
		vectorClearValue[i].color.float32[2] = 0.0f;
		vectorClearValue[i].color.float32[3] = 0.0f;
	}

	// Supernova V1 Mode 1X algorithm, reading from _vectorYImageView[commandBufferIndex] and storing
	// into _vectorOutputImageView[commandBufferIndex]
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = _renderPass;
	renderPassBeginInfo.framebuffer = _vectorFramebuffer[commandBufferIndex];
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = _imageExtent.width;
	renderPassBeginInfo.renderArea.extent.height = _imageExtent.height;
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(vectorClearValue.size());
	renderPassBeginInfo.pClearValues = vectorClearValue.data();

	// TODO: Add command buffer and general Vulkan object debug label
	// TODO: Should allow an option to record to a secondary command buffer (use VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR )?

	_vk->vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	_vk->vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _vectorPipeline[0]);
	_vk->vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _vectorPipelineLayout[0], 0, 1, &_vectorDescriptorSet[commandBufferIndex], 0, nullptr);
	_vk->vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	_vk->vkCmdEndRenderPass(commandBuffer);
}

void VulkanPostProcessingPass::buildCommandPool()
{
	// Build command pool
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	commandPoolCreateInfo.queueFamilyIndex = _graphicsQueueFamilyIndex;
	assertFunctionResult(_vk->vkCreateCommandPool(_device, &commandPoolCreateInfo, nullptr, &_commandPool), "ERROR: vkCreateCommandPool.\n");
}

void VulkanPostProcessingPass::buildSampler()
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
	samplerCreateInfo.maxLod = 100.0f;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
	assertFunctionResult(_vk->vkCreateSampler(_device, &samplerCreateInfo, nullptr, &_sampler), "ERROR: vkCreateSampler.\n");
}

void VulkanPostProcessingPass::buildRenderPass()
{
	_renderPass = buildPostProcessRenderPass(_vectorOutputImageFormat, _vectorOutputImageInitialLayout[0], _vectorOutputImageFinalLayout[0]);
}

void VulkanPostProcessingPass::buildDescriptorPool()
{
	// Build descriptor pool
	VkDescriptorPoolSize descriptorPoolSize = {};
	descriptorPoolSize.descriptorCount = _combinedImageSamplerDescriptorCount;
	descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolCreateInfo.maxSets = _combinedImageSamplerDescriptorCount;
	descriptorPoolCreateInfo.poolSizeCount = 1;
	descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;
	assertFunctionResult(_vk->vkCreateDescriptorPool(_device, &descriptorPoolCreateInfo, nullptr, &_descriptorPool), "ERROR: vkCreateDescriptorPool.\n");
}

VkPipelineLayout VulkanPostProcessingPass::buildPipelineLayout(const std::vector<VkDescriptorSetLayout>& vectorDescriptorSetLayout)
{
	// Build pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(vectorDescriptorSetLayout.size());
	pipelineLayoutCreateInfo.pSetLayouts = vectorDescriptorSetLayout.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	assertFunctionResult(_vk->vkCreatePipelineLayout(_device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout), "ERROR: vkCreatePipelineLayout.\n");
	return pipelineLayout;
}


void VulkanPostProcessingPass::shutdown()
{
	destroyVulkanObject<PFN_vkDestroyRenderPass, VkRenderPass>(_device, _vk->vkDestroyRenderPass, _renderPass);
	destroyVulkanObjectVector<PFN_vkDestroyPipeline, VkPipeline>(_device, _vk->vkDestroyPipeline, _vectorPipeline);
	destroyVulkanObjectVector<PFN_vkDestroyFramebuffer, VkFramebuffer>(_device, _vk->vkDestroyFramebuffer, _vectorFramebuffer);
	destroyVulkanObjectVector<PFN_vkDestroyDescriptorSetLayout, VkDescriptorSetLayout>(_device, _vk->vkDestroyDescriptorSetLayout, _vectorDescriptorSetLayout);	
	destroyVulkanObjectVector<PFN_vkDestroyPipelineLayout, VkPipelineLayout>(_device, _vk->vkDestroyPipelineLayout, _vectorPipelineLayout);	
	destroyVulkanObject<PFN_vkDestroySampler, VkSampler>(_device, _vk->vkDestroySampler, _sampler);
	destroyVulkanObject<PFN_vkDestroyDescriptorPool, VkDescriptorPool>(_device, _vk->vkDestroyDescriptorPool, _descriptorPool);
	destroyVulkanObject<PFN_vkDestroyCommandPool, VkCommandPool>(_device, _vk->vkDestroyCommandPool, _commandPool);

	// Each Supernova pass will receive input either from the application or from a previous pass, being it the output of that pass.
	// No need to destroy input images or image views

	destroyVulkanObjectVectorOfVectors<PFN_vkDestroyImage, VkImage>(_device, _vk->vkDestroyImage, _vectorVectorOutputImage);
	destroyVulkanObjectVectorOfVectors<PFN_vkFreeMemory, VkDeviceMemory>(_device, _vk->vkFreeMemory, _vectorVectorOutputImageDeviceMemory);

	// Each Supernova pass will output to either an image built by the pass or to an image provided by the application using the
	// library. Destroy image views only if built by the Supernova pass.

	if (_postprocessingPassOrder != PostprocessingPassOrder::LastPass)
	{
		destroyVulkanObjectVectorOfVectors<PFN_vkDestroyImageView, VkImageView>(_device, _vk->vkDestroyImageView, _vectorVectorOutputImageView);
	}
}

void VulkanPostProcessingPass::buildFramebuffer(VkRenderPass renderPass, const std::vector<std::vector<VkImageView>>& vectorVectorAttachment, std::vector<VkFramebuffer>& _vectorFramebuffer)
{
	VkFramebufferCreateInfo framebufferCreateInfo = {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.flags = 0;
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.width = _imageExtent.width;
	framebufferCreateInfo.height = _imageExtent.height;
	framebufferCreateInfo.layers = 1;

	_vectorFramebuffer.resize(_numberCommandBuffer);

	for (uint32_t i = 0; i < _numberCommandBuffer; ++i)
	{
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(vectorVectorAttachment[i].size());
		framebufferCreateInfo.pAttachments = vectorVectorAttachment[i].data();
		assertFunctionResult(_vk->vkCreateFramebuffer(_device, &framebufferCreateInfo, nullptr, &_vectorFramebuffer[i]), "ERROR: vkCreateFramebuffer.\n");
	}
}

VkPipeline VulkanPostProcessingPass::buildPostProcessingPipeline(VkRenderPass renderPass, VkPipelineLayout pipelineLayout, const std::string& fragmentShaderName, int numberColorAttachment)
{
	// Build shader modules
	std::vector<unsigned char> pointerVertexShader;
	loadFile(_shaderPath + std::string(vertexShaderName), pointerVertexShader, _application);
	assertCondition((pointerVertexShader.size() != 0), "ERROR: Wrong result reading vertex shader AttributelessVertexShader.vsh.spv.");

	std::vector<unsigned char> pointerFragmentShader;
	loadFile(_shaderPath + std::string(fragmentShaderName), pointerFragmentShader, _application);
	assertCondition((pointerFragmentShader.size() != 0), "ERROR: Wrong result reading fragment shader.");

	// First build vertex shader module
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.flags = 0;
	shaderModuleCreateInfo.codeSize = static_cast<uint32_t>(pointerVertexShader.size());
	shaderModuleCreateInfo.pCode = static_cast<uint32_t*>((void*)(pointerVertexShader.data()));
	VkShaderModule vertexShaderModule = VK_NULL_HANDLE;
	assertFunctionResult(_vk->vkCreateShaderModule(_device, &shaderModuleCreateInfo, nullptr, &vertexShaderModule), "ERROR: vkCreateShaderModule.\n");

	// Second build fragment shader module
	shaderModuleCreateInfo.codeSize = static_cast<uint32_t>(pointerFragmentShader.size());
	shaderModuleCreateInfo.pCode = static_cast<uint32_t*>((void*)(pointerFragmentShader.data()));
	VkShaderModule fragmentShaderModule = VK_NULL_HANDLE;
	assertFunctionResult(_vk->vkCreateShaderModule(_device, &shaderModuleCreateInfo, nullptr, &fragmentShaderModule), "ERROR: vkCreateShaderModule.\n");

	// Prepare data for VkGraphicsPipelineCreateInfo::pStages
	std::vector<VkPipelineShaderStageCreateInfo> vectorPipelineShaderStageCreateInfo(2);
	vectorPipelineShaderStageCreateInfo[0] = {};
	vectorPipelineShaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vectorPipelineShaderStageCreateInfo[0].flags = 0;
	vectorPipelineShaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	vectorPipelineShaderStageCreateInfo[0].module = vertexShaderModule;
	vectorPipelineShaderStageCreateInfo[0].pName = "main";
	vectorPipelineShaderStageCreateInfo[0].pSpecializationInfo = nullptr;

	vectorPipelineShaderStageCreateInfo[1] = {};
	vectorPipelineShaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vectorPipelineShaderStageCreateInfo[1].flags = 0;
	vectorPipelineShaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	vectorPipelineShaderStageCreateInfo[1].module = fragmentShaderModule;
	vectorPipelineShaderStageCreateInfo[1].pName = "main";
	vectorPipelineShaderStageCreateInfo[1].pSpecializationInfo = nullptr;

	// Prepare the vertex input struct
	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = {};
	pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	// Prepare input assembly struct
	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = {};
	pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	// Prepare viewport state struct
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(_imageExtent.width);
	viewport.height = static_cast<float>(_imageExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissors = {};
	scissors.offset.x = 0;
	scissors.offset.y = 0;
	scissors.extent.width = _imageExtent.width;
	scissors.extent.height = _imageExtent.height;

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = {};
	pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pipelineViewportStateCreateInfo.viewportCount = 1;
	pipelineViewportStateCreateInfo.pViewports = &viewport;
	pipelineViewportStateCreateInfo.scissorCount = 1;
	pipelineViewportStateCreateInfo.pScissors = &scissors;

	// Prepare rasterization state struct
	VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo = {};
	pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
	pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	pipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
	pipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f;
	pipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
	pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;

	// Prepare multisample state struct
	VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = {};
	pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	pipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
	pipelineMultisampleStateCreateInfo.minSampleShading = 0.0f;
	pipelineMultisampleStateCreateInfo.pSampleMask = nullptr;
	pipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
	pipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

	// Prepare depth stencil state struct
	VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = {};
	pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
	pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
	pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_NEVER;
	pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
	pipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
	pipelineDepthStencilStateCreateInfo.front.failOp = VK_STENCIL_OP_KEEP;
	pipelineDepthStencilStateCreateInfo.front.passOp = VK_STENCIL_OP_KEEP;
	pipelineDepthStencilStateCreateInfo.front.depthFailOp = VK_STENCIL_OP_KEEP;
	pipelineDepthStencilStateCreateInfo.front.compareOp = VK_COMPARE_OP_ALWAYS;
	pipelineDepthStencilStateCreateInfo.front.compareMask = 0;
	pipelineDepthStencilStateCreateInfo.front.writeMask = 0;
	pipelineDepthStencilStateCreateInfo.front.reference = 0;
	pipelineDepthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
	pipelineDepthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
	pipelineDepthStencilStateCreateInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
	pipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
	pipelineDepthStencilStateCreateInfo.back.compareMask = 0;
	pipelineDepthStencilStateCreateInfo.back.writeMask = 0;
	pipelineDepthStencilStateCreateInfo.back.reference = 0;
	pipelineDepthStencilStateCreateInfo.minDepthBounds = 0.0f;
	pipelineDepthStencilStateCreateInfo.maxDepthBounds = 1.0f;

	// Prepare color blend state struct
	std::vector<VkPipelineColorBlendAttachmentState> vectorPipelineColorBlendAttachmentState(numberColorAttachment);

	for (size_t i = 0; i < vectorPipelineColorBlendAttachmentState.size(); ++i)
	{
		vectorPipelineColorBlendAttachmentState[i].blendEnable = VK_FALSE;
		vectorPipelineColorBlendAttachmentState[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		vectorPipelineColorBlendAttachmentState[i].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		vectorPipelineColorBlendAttachmentState[i].colorBlendOp = VK_BLEND_OP_ADD;
		vectorPipelineColorBlendAttachmentState[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		vectorPipelineColorBlendAttachmentState[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		vectorPipelineColorBlendAttachmentState[i].alphaBlendOp = VK_BLEND_OP_ADD;
		vectorPipelineColorBlendAttachmentState[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	}

	VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = {};
	pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
	pipelineColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;
	pipelineColorBlendStateCreateInfo.attachmentCount = static_cast<uint32_t>(vectorPipelineColorBlendAttachmentState.size());
	pipelineColorBlendStateCreateInfo.pAttachments = vectorPipelineColorBlendAttachmentState.data();
	pipelineColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[3] = 0.0f;

	// Finally prepare the graphics pipeline struct
	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.flags = 0;
	graphicsPipelineCreateInfo.stageCount = 2;
	graphicsPipelineCreateInfo.pStages = vectorPipelineShaderStageCreateInfo.data();
	graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo; // NOTE: No specific vertex input description as the postprocessing vertex shader generates a triangle from gl_VertexIndex values in the vertex shader
	graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
	graphicsPipelineCreateInfo.pTessellationState = nullptr; // No tessellation
	graphicsPipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState = nullptr;
	graphicsPipelineCreateInfo.layout = pipelineLayout;
	graphicsPipelineCreateInfo.renderPass = renderPass;
	graphicsPipelineCreateInfo.subpass = 0; // The graphics pipeline will be the only subpass used in the render pass
	graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	graphicsPipelineCreateInfo.basePipelineIndex = 0;

	VkPipeline pipeline = VK_NULL_HANDLE;
	assertFunctionResult(_vk->vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline), "ERROR: vkCreateGraphicsPipelines.\n");

	destroyVulkanObject<PFN_vkDestroyShaderModule, VkShaderModule>(_device, _vk->vkDestroyShaderModule, vertexShaderModule);
	destroyVulkanObject<PFN_vkDestroyShaderModule, VkShaderModule>(_device, _vk->vkDestroyShaderModule, fragmentShaderModule);

	return pipeline;
}

int VulkanPostProcessingPass::getMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlagBits requestedMemoryProperties)
{
	for (uint32_t i = 0; i < _vkPhysicalDeviceMemoryProperties.memoryTypeCount; ++i)
	{
		// Verify first bit in memoryTypeBits by swifting each loop iteration one bit
		if ((memoryTypeBits & 1) > 0)
		{
			// Verify whether this memory type has the memory properties which are requested
			if ((_vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & requestedMemoryProperties) == requestedMemoryProperties) { return i; }
		}
		memoryTypeBits >>= 1;
	}

	assertCondition(false, "ERROR: No memory type found.");
	return -1;
}

VkRenderPass VulkanPostProcessingPass::buildPostProcessRenderPass(
	const std::vector<VkFormat>& vectorAttachmentFormat, VkImageLayout attachmentInitialLayout, VkImageLayout attachmentFinalLayout)
{
	size_t numberColorAttachment = vectorAttachmentFormat.size();
	// Build attachment descriptions
	// Attachment index 0: Where to store the results
	std::vector<VkAttachmentDescription> vectorAttachmentDescription(numberColorAttachment);

	for (size_t i = 0; i < numberColorAttachment; ++i)
	{
		vectorAttachmentDescription[i].flags = 0;
		vectorAttachmentDescription[i].format = vectorAttachmentFormat[i];
		vectorAttachmentDescription[i].samples = VK_SAMPLE_COUNT_1_BIT;
		vectorAttachmentDescription[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		vectorAttachmentDescription[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		vectorAttachmentDescription[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vectorAttachmentDescription[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vectorAttachmentDescription[i].initialLayout = attachmentInitialLayout;
		vectorAttachmentDescription[i].finalLayout = attachmentFinalLayout;
	}

	// Build subpass description
	std::vector<VkAttachmentReference> vectorColorAttachment(numberColorAttachment);

	for (size_t i = 0; i < numberColorAttachment; ++i)
	{
		vectorColorAttachment[i].attachment = static_cast<uint32_t>(i);
		vectorColorAttachment[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	VkSubpassDescription subpassDescription = {};
	subpassDescription.flags = 0;
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.colorAttachmentCount = static_cast<uint32_t>(vectorColorAttachment.size());
	subpassDescription.pColorAttachments = vectorColorAttachment.data();
	subpassDescription.pResolveAttachments = nullptr;
	subpassDescription.pDepthStencilAttachment = nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;

	std::vector<VkSubpassDescription> vectorSubpass = { subpassDescription };

	// Build subpass dependencies
	// Dependency between a previous external render pass and the subpass contained in the render pass
	VkSubpassDependency dependencyExternalToSubpass = {};
	dependencyExternalToSubpass.srcSubpass = VK_SUBPASS_EXTERNAL; // TODO: CHANGE srcStageMask
	dependencyExternalToSubpass.dstSubpass = 0;
	dependencyExternalToSubpass.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; // "Wait for all color attachment operations to be completed in the render pass prior to the first subpass...
	dependencyExternalToSubpass.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // ... before doing any fragment shader operations with the first subpass" (previous stages, where the information from the color attachment is not needed, like the vertex shader, can be executed)
	dependencyExternalToSubpass.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT; // "Wait for all color attachment memory write operations to be done in the render pass prior to the first subpass...
	dependencyExternalToSubpass.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // "... before doing any shader read operations (with the result of the input image provided) with the first subpass"
	dependencyExternalToSubpass.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT; // Each fragment thread will access only the same texel to read it and convert it to YUVA, only a texel needs to be accessed

	// Dependency between the the subpass contained in the render pass abd any possible external render passes
	VkSubpassDependency dependencySubpassToExternal = {};
	dependencySubpassToExternal.srcSubpass = 0;
	dependencySubpassToExternal.dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencySubpassToExternal.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // "Wait for all color attachment operations to be completed by the first subpass...
	dependencySubpassToExternal.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; // ... before doing any fragment shader operations with the second subpass" (previous stages, where the information from the color attachment is not needed, like the vertex shader, can be executed)
	dependencySubpassToExternal.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // "Wait for all color attachment memory write operations to be done before starting with the second subpass...
	dependencySubpassToExternal.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT; // "... before doing any input attachment read operations (with the result of the input image provided) with the first subpass"
	dependencySubpassToExternal.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT; // Each fragment thread will access only the same texel to read it and convert it to YUVA, only a texel needs to be accessed

	std::vector<VkSubpassDependency> vectorSubpassDependency = { dependencyExternalToSubpass, dependencySubpassToExternal };

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.flags = 0;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(vectorAttachmentDescription.size()); // TODO: Avoid constants, use vector size
	renderPassCreateInfo.pAttachments = vectorAttachmentDescription.data();
	renderPassCreateInfo.subpassCount = static_cast<uint32_t>(vectorSubpass.size());
	renderPassCreateInfo.pSubpasses = vectorSubpass.data();
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(vectorSubpassDependency.size());
	renderPassCreateInfo.pDependencies = vectorSubpassDependency.data();

	VkRenderPass renderPass = VK_NULL_HANDLE;
	assertFunctionResult(_vk->vkCreateRenderPass(_device, &renderPassCreateInfo, nullptr, &renderPass), "ERROR: vkCreateRenderPass.\n");
	return renderPass;
}

void VulkanPostProcessingPass::buildDescriptorSetLayouts()
{
	assertCondition((_vectorVectorInputImageView.size() != 0), "ERROR: VulkanPostProcessingPass::_vectorVectorInputImageView has no elements.");

	// Verify each element has the same amount of elements
	size_t numberElements = _vectorVectorInputImageView[0].size();

	for (uint32_t i = 0; i < _vectorVectorInputImageView.size(); ++i)
	{
		assertCondition(
			(_vectorVectorInputImageView[i].size() == numberElements), "ERROR: Not all elements in VulkanPostProcessingPass::_vectorVectorInputImageView have the same amount of elements.");
	}

	// Build descriptor set layout
	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBinding(_vectorVectorInputImageView[0].size());

	for (uint32_t i = 0; i < descriptorSetLayoutBinding.size(); ++i)
	{
		descriptorSetLayoutBinding[i].binding = i;
		descriptorSetLayoutBinding[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorSetLayoutBinding[i].descriptorCount = 1;
		descriptorSetLayoutBinding[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptorSetLayoutBinding[i].pImmutableSamplers = nullptr;
	}

	_vectorDescriptorSetLayout.resize(1);

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.flags = 0;
	descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBinding.size());
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBinding.data();
	assertFunctionResult(_vk->vkCreateDescriptorSetLayout(_device, &descriptorSetLayoutCreateInfo, nullptr, _vectorDescriptorSetLayout.data()), "ERROR: vkCreateDescriptorSetLayout.\n");
}

void VulkanPostProcessingPass::allocateDescriptorSets()
{	
	// Allocate descriptor sets
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfoSupernovaV1Mode1X = {};
	descriptorSetAllocateInfoSupernovaV1Mode1X.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfoSupernovaV1Mode1X.descriptorPool = _descriptorPool;
	descriptorSetAllocateInfoSupernovaV1Mode1X.descriptorSetCount = 1;
	descriptorSetAllocateInfoSupernovaV1Mode1X.pSetLayouts = _vectorDescriptorSetLayout.data();

	_vectorDescriptorSet.resize(_numberCommandBuffer);

	for (uint32_t i = 0; i < _numberCommandBuffer; ++i)
	{
		assertFunctionResult(_vk->vkAllocateDescriptorSets(_device, &descriptorSetAllocateInfoSupernovaV1Mode1X, &_vectorDescriptorSet[i]), "ERROR: vkAllocateDescriptorSets.\n");
	}
}

void VulkanPostProcessingPass::updateDescriptorSets()
{
	// Update Supernova V1 Mode 2X subpass descriptor sets for Y, U, V and A images
	std::vector<VkWriteDescriptorSet> vectorWriteDescriptorSet(_vectorVectorInputImageView[0].size());
	std::vector<VkDescriptorImageInfo> vectorDescriptorImageInfo(_vectorVectorInputImageView[0].size());

	VkWriteDescriptorSet writeDescriptorSet = {};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.dstArrayElement = 0;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	for (uint32_t i = 0; i < _numberCommandBuffer; ++i)
	{
		for (size_t j = 0; j < _vectorVectorInputImageView[0].size(); ++j)
		{
			vectorDescriptorImageInfo[j] = {};
			vectorDescriptorImageInfo[j].sampler = _sampler;
			vectorDescriptorImageInfo[j].imageView = _vectorVectorInputImageView[i][j];
			vectorDescriptorImageInfo[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			writeDescriptorSet.dstSet = _vectorDescriptorSet[i];
			writeDescriptorSet.pImageInfo = &vectorDescriptorImageInfo[j];
			writeDescriptorSet.dstBinding = static_cast<uint32_t>(j);
			vectorWriteDescriptorSet[j] = writeDescriptorSet;
		}

		_vk->vkUpdateDescriptorSets(_device, static_cast<uint32_t>(vectorWriteDescriptorSet.size()), vectorWriteDescriptorSet.data(), 0, nullptr);
	}
}

void VulkanPostProcessingPass::buildFramebuffers()
{
	std::vector<std::vector<VkImageView>> vectorVectorAttachment(_numberCommandBuffer);

	for (uint32_t i = 0; i < _numberCommandBuffer; ++i)
	{
		vectorVectorAttachment[i].insert(vectorVectorAttachment[i].end(), _vectorVectorOutputImageView[i].begin(), _vectorVectorOutputImageView[i].end());
	}

	buildFramebuffer(_renderPass, vectorVectorAttachment, _vectorFramebuffer);
}

void VulkanPostProcessingPass::buildImages()
{
	_vectorVectorOutputImage.resize(_numberCommandBuffer);
	_vectorVectorOutputImageDeviceMemory.resize(_numberCommandBuffer);

	for (size_t i = 0; i < _vectorVectorOutputImage.size(); ++i)
	{
		_vectorVectorOutputImage[i].resize(_vectorOutputImageFormat.size());
		_vectorVectorOutputImageDeviceMemory[i].resize(_vectorOutputImageFormat.size());
	}

	for (uint32_t i = 0; i < _numberCommandBuffer; ++i)
	{
		for (size_t j = 0; j < _vectorOutputImageFormat.size(); ++j)
		{
			buildImage(_vectorOutputImageFormat[j], _imageExtent, _vectorVectorOutputImage[i][j], _vectorVectorOutputImageDeviceMemory[i][j]);
		}
	}
}

void VulkanPostProcessingPass::buildImageViews()
{
	_vectorVectorOutputImageView.resize(_numberCommandBuffer);

	for (size_t i = 0; i < _vectorVectorOutputImage.size(); ++i) { _vectorVectorOutputImageView[i].resize(_vectorOutputImageFormat.size()); }

	for (uint32_t i = 0; i < _numberCommandBuffer; ++i)
	{
		for (size_t j = 0; j < _vectorOutputImageFormat.size(); ++j)
		{
			buildImageView(_vectorOutputImageFormat[j], _vectorVectorOutputImage[i][j], _vectorVectorOutputImageView[i][j]);
		}
	}
}

void VulkanPostProcessingPass::buildPipelineLayouts() {
	_vectorPipelineLayout.push_back(buildPipelineLayout(_vectorDescriptorSetLayout));
}

void VulkanPostProcessingPass::buildImage(VkFormat format, VkExtent2D extent2D, VkImage& image, VkDeviceMemory& deviceMemory)
{
	// First: Build image
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.flags = 0;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.extent = VkExtent3D{ extent2D.width, extent2D.height, 1 };
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT; // TODO: Verify sampled bit is really needed.
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 1;
	imageCreateInfo.pQueueFamilyIndices = &_graphicsQueueFamilyIndex;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	assertFunctionResult(_vk->vkCreateImage(_device, &imageCreateInfo, nullptr, &image), "ERROR: vkCreateImage.\n");

	// Second: Get the memory requirements of the image built and allocate memory
	// Get the memory requirements in size, allignment and memory type bits for the image resource allocated:
	_vkInstance->vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &_vkPhysicalDeviceMemoryProperties);
	VkMemoryRequirements memoryRequirementsYImage = {};
	_vk->vkGetImageMemoryRequirements(_device, image, &memoryRequirementsYImage);
	assertCondition((memoryRequirementsYImage.size != 0), "ERROR: vkGetImageMemoryRequirements::size is 0.");

	// getMemoryType() will return the first index in VkPhysicalDeviceMemoryProperties::memoryTypes[] which supports the memory
	// property "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT" (if any) from all possible indices in VkPhysicalDeviceMemoryProperties::memoryTypes[]
	// which can be used to allocate memory for the image being built. The set of all indices in
	// VkPhysicalDeviceMemoryProperties::memoryTypes[] which allow to allocate memory for the image being built are given by
	// VkMemoryRequirements::memoryTypeBits, where each bit set to one indicates such index.
	int memoryTypeIndexYImage = getMemoryType(memoryRequirementsYImage.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VkMemoryAllocateInfo memoryAllocateInfoYImage = {};
	memoryAllocateInfoYImage.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfoYImage.allocationSize = memoryRequirementsYImage.size;
	memoryAllocateInfoYImage.memoryTypeIndex = memoryTypeIndexYImage;
	assertFunctionResult(_vk->vkAllocateMemory(_device, &memoryAllocateInfoYImage, nullptr, &deviceMemory), "ERROR: vkAllocateMemory.\n");

	// Third: Bind the memory allocated for the image to the image
	assertFunctionResult(_vk->vkBindImageMemory(_device, image, deviceMemory, 0), "ERROR: vkBindImageMemory.\n");
}

void VulkanPostProcessingPass::buildImageView(VkFormat format, VkImage image, VkImageView& imageView)
{
	// Fourth: Build image view (used by the framebuffer)
	VkImageViewCreateInfo imageViewCreateInfo = {};

	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.flags = 0;
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	assertFunctionResult(_vk->vkCreateImageView(_device, &imageViewCreateInfo, nullptr, &imageView), "ERROR: vkCreateImageView.\n");
}

} // namespace pvr
