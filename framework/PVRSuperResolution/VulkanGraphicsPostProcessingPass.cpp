/*!
\brief Post processing pass using Vulkan API and rasterization
\file PVRSuperResolution/VulkanGraphicsPostProcessingPass.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "VulkanGraphicsPostProcessingPass.h"
#include "Log.h"
#include "FileIO.h"

const char* vertexShaderName = "AttributelessVertexShader.vsh.spv";

namespace pvr
{

void VulkanGraphicsPostProcessingPass::recordCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferIndex)
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

	_vk->vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	_vk->vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _vectorPipeline[0]);
	_vk->vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _vectorPipelineLayout[0], 0, 1, &_vectorDescriptorSet[commandBufferIndex], 0, nullptr);
	_vk->vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	_vk->vkCmdEndRenderPass(commandBuffer);
}

void VulkanGraphicsPostProcessingPass::buildSampler()
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

void VulkanGraphicsPostProcessingPass::buildRenderPass()
{
	_renderPass = buildPostProcessRenderPass(_vectorOutputImageFormat, _vectorOutputImageInitialLayout[0], _vectorOutputImageFinalLayout[0]);
}

void VulkanGraphicsPostProcessingPass::buildDescriptorPool()
{
	std::vector<VkDescriptorType> vectorDescriptorType = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
	std::vector<int> vectorDescriptorCount = { _combinedImageSamplerDescriptorCount };

	_descriptorPool = _vulkanResourceAllocator.buildDescriptorPool(vectorDescriptorType, vectorDescriptorCount);
}

void VulkanGraphicsPostProcessingPass::buildFramebuffer(
	VkRenderPass renderPass, const std::vector<std::vector<VkImageView>>& vectorVectorAttachment, std::vector<VkFramebuffer>& _vectorFramebuffer)
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

VkPipeline VulkanGraphicsPostProcessingPass::buildPostProcessingPipeline(
	VkRenderPass renderPass, VkPipelineLayout pipelineLayout, const std::string& fragmentShaderName, int numberColorAttachment)
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

VkRenderPass VulkanGraphicsPostProcessingPass::buildPostProcessRenderPass(
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

void VulkanGraphicsPostProcessingPass::buildDescriptorSetLayouts()
{
	assertCondition((_vectorVectorInputImageView.size() != 0), "ERROR: VulkanGraphicsPostProcessingPass::_vectorVectorInputImageView has no elements.");

	// Verify each element has the same amount of elements
	size_t numberElements = _vectorVectorInputImageView[0].size();

	for (uint32_t i = 0; i < _vectorVectorInputImageView.size(); ++i)
	{
		assertCondition((_vectorVectorInputImageView[i].size() == numberElements),
			"ERROR: Not all elements in VulkanGraphicsPostProcessingPass::_vectorVectorInputImageView have the same amount of elements.");
	}

	std::vector<VkDescriptorType> vectorDescriptorType;
	std::vector<VkShaderStageFlagBits> vectorDescriptorStage;

	size_t numberInput = _vectorVectorInputImageView[0].size();

	// Build descriptor set layouts following this order
	// + As many descriptor set layout bindings of type VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER as elements in _vectorVectorInputImageView[0] (assuming other indices have the same number of image views)
	// The descriptor sets of this descriptor set layout will be arranged in incremental indices

	for (size_t i = 0; i < numberInput; ++i)
	{
		vectorDescriptorType.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		vectorDescriptorStage.push_back(VK_SHADER_STAGE_FRAGMENT_BIT);
	}

	_vectorDescriptorSetLayout.push_back(_vulkanResourceAllocator.buildDescriptorSetLayouts(vectorDescriptorType, vectorDescriptorStage));
}

void VulkanGraphicsPostProcessingPass::buildFramebuffers()
{
	std::vector<std::vector<VkImageView>> vectorVectorAttachment(_numberCommandBuffer);

	for (uint32_t i = 0; i < _numberCommandBuffer; ++i)
	{
		vectorVectorAttachment[i].insert(vectorVectorAttachment[i].end(), _vectorVectorOutputImageView[i].begin(), _vectorVectorOutputImageView[i].end());
	}

	buildFramebuffer(_renderPass, vectorVectorAttachment, _vectorFramebuffer);
}

void VulkanGraphicsPostProcessingPass::buildImages()
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
			_vulkanResourceAllocator.buildImage(
				_vectorOutputImageFormat[j], _imageExtent, _queueFamilyIndex, _vectorVectorOutputImage[i][j], _vectorVectorOutputImageDeviceMemory[i][j]);
		}
	}
}

void VulkanGraphicsPostProcessingPass::buildImageViews()
{
	_vectorVectorOutputImageView.resize(_numberCommandBuffer);

	for (size_t i = 0; i < _vectorVectorOutputImage.size(); ++i) { _vectorVectorOutputImageView[i].resize(_vectorOutputImageFormat.size()); }

	for (uint32_t i = 0; i < _numberCommandBuffer; ++i)
	{
		for (size_t j = 0; j < _vectorOutputImageFormat.size(); ++j)
		{
			_vectorVectorOutputImageView[i][j] = _vulkanResourceAllocator.buildImageView(_vectorOutputImageFormat[j], _vectorVectorOutputImage[i][j]);
		}
	}
}

} // namespace pvr
