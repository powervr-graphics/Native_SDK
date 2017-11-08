/*!
\brief Helper functionality for populating the Pipeline Create Infos
\file PVRVk/PopulateCreateInfoVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/DeviceVk.h"
#include "PVRVk/PipelineLayoutVk.h"
#include "PVRVk/DescriptorSetVk.h"
#include "PVRVk/RenderPassVk.h"
#include "PVRVk/ShaderVk.h"
#include "PVRVk/GraphicsPipelineVk.h"
#include "PVRVk/ComputePipelineVk.h"
namespace pvrvk {
namespace impl {

/// <summary>Populate vulkan input attribute description</summary>
/// <param name="vkva">Return populated VkVertexInputAttributeDescription</param>
/// <param name="pvrva">A vertex attribute info to convert from</param>
inline void convert(VkVertexInputAttributeDescription& vkva, const VertexInputAttributeDescription& pvrva)
{
	vkva.binding = pvrva.binding;
	vkva.format = pvrva.format;
	vkva.location = pvrva.location;
	vkva.offset = pvrva.offset;
}

/// <summary>Populate vulkan input binding description</summary>
/// <param name="vkvb">Return populated VkVertexInputBindingDescription</param>
/// <param name="pvrvb">A vertex input binding info to convert from</param>
inline void convert(VkVertexInputBindingDescription& vkvb, const VertexInputBindingDescription& pvrvb)
{
	vkvb.binding = pvrvb.binding;
	vkvb.inputRate = pvrvb.inputRate;
	vkvb.stride = pvrvb.stride;
}

/// <summary>Populate vulkan pipeline color blend attachment state</summary>
/// <param name="vkcb">Return populated VkPipelineColorBlendAttachmentState</param>
/// <param name="pvrcb">A color blend attachment state to convert from</param>
inline void convert(VkPipelineColorBlendAttachmentState& vkcb, const PipelineColorBlendAttachmentState& pvrcb)
{
	vkcb.alphaBlendOp = pvrcb.alphaBlendOp;
	vkcb.blendEnable = pvrcb.blendEnable;
	vkcb.colorBlendOp = pvrcb.colorBlendOp;
	vkcb.colorWriteMask = pvrcb.colorWriteMask;
	vkcb.dstAlphaBlendFactor = pvrcb.dstAlphaBlendFactor;
	vkcb.dstColorBlendFactor = pvrcb.dstColorBlendFactor;
	vkcb.srcAlphaBlendFactor = pvrcb.srcAlphaBlendFactor;
	vkcb.srcColorBlendFactor = pvrcb.srcColorBlendFactor;
}

/// <summary>Populate vulkan stencil state</summary>
/// <param name="stencilState">A stencil state state to convert from</param>
/// <param name="vkStencilState">Return populated VkStencilOpState</param>
inline void convert(VkStencilOpState& vkStencilState, const StencilOpState& stencilState)
{
	vkStencilState.failOp = stencilState.failOp;
	vkStencilState.passOp = stencilState.passOp;
	vkStencilState.depthFailOp = stencilState.depthFailOp;
	vkStencilState.compareOp = stencilState.compareOp;
	vkStencilState.compareMask = stencilState.compareMask;
	vkStencilState.writeMask = stencilState.writeMask;
	vkStencilState.reference = stencilState.reference;
}

/// <summary>Populate vulkan viewport</summary>
/// <param name="vp">A viewport to convert from</param>
/// <param name="vkvp">Return populated Viewport</param>
inline void convert(VkViewport& vkvp, const Viewport& vp)
{
	vkvp.x = vp.x;
	vkvp.y = vp.y;
	vkvp.width = vp.width;
	vkvp.height = vp.height;
	vkvp.minDepth = vp.minDepth;
	vkvp.maxDepth = vp.maxDepth;
}

//!\cond NO_DOXYGEN
inline void populateShaderInfo(
  const VkShaderModule& shader, VkShaderStageFlags vkShaderStage,
  VkSpecializationInfo& specializationInfo, char* specializationInfoData,
  const ShaderConstantInfo* shaderConsts, uint32_t shaderConstCount,
  VkSpecializationMapEntry* mapEntries,
  VkPipelineShaderStageCreateInfo& outShader, const char* entryPoint)
{
	// caculate the number of size in bytes required.
	uint32_t specicalizedataSize = 0;
	for (uint32_t i = 0; i < shaderConstCount; ++i)
	{
		specicalizedataSize += shaderConsts[i].sizeInBytes;
	}

	if (specicalizedataSize)
	{
		debug_assertion(specicalizedataSize < FrameworkCaps::MaxSpecialisationInfoDataSize,
		                "Specialised Data out of range.");
		uint32_t dataOffset = 0;
		for (uint32_t i = 0; i < shaderConstCount; ++i)
		{
			memcpy(&specializationInfoData[dataOffset], shaderConsts[i].data, shaderConsts[i].sizeInBytes);
			mapEntries[i] = VkSpecializationMapEntry{ shaderConsts[i].constantId, dataOffset, shaderConsts[i].sizeInBytes };
			dataOffset += shaderConsts[i].sizeInBytes;
		}
		specializationInfo.mapEntryCount = shaderConstCount;
		specializationInfo.pMapEntries = mapEntries;
		specializationInfo.dataSize = specicalizedataSize;
		specializationInfo.pData = specializationInfoData;
	}

	outShader.sType = VkStructureType::e_PIPELINE_SHADER_STAGE_CREATE_INFO;
	outShader.pNext = nullptr;
	outShader.flags = 0;
	outShader.pSpecializationInfo = (specicalizedataSize ? &specializationInfo : nullptr);
	outShader.stage = vkShaderStage;
	outShader.module = shader;
	outShader.pName = entryPoint;
}
//!\endcond

/// <summary>Contains everything needed to define a VkGraphicsPipelineCreateInfo, with provision for all memory
/// required</summary>
struct GraphicsPipelinePopulate
{
private:
	VkGraphicsPipelineCreateInfo createInfo; //< After construction, will contain the ready-to-use create info
	VkPipelineInputAssemblyStateCreateInfo _ia;//< Input assembler create info
	VkPipelineRasterizationStateCreateInfo _rs;//< rasterization create info
	VkPipelineMultisampleStateCreateInfo _ms;//< Multisample create info
	VkPipelineViewportStateCreateInfo _vp;//< Viewport createinfo
	VkPipelineColorBlendStateCreateInfo _cb;//< Color blend create info
	VkPipelineDepthStencilStateCreateInfo _ds;//< Depth-stencil create info
	VkPipelineVertexInputStateCreateInfo _vertexInput;//< Vertex input create info
	VkPipelineShaderStageCreateInfo _shaders[static_cast<uint32_t>(10)];

	VkVertexInputBindingDescription _vkVertexBindings[FrameworkCaps::MaxVertexBindings]; //Memory for the bindings
	VkVertexInputAttributeDescription _vkVertexAttributes[FrameworkCaps::MaxVertexAttributes]; //Memory for the attributes
	VkPipelineColorBlendAttachmentState _vkBlendAttachments[FrameworkCaps::MaxColorAttachments]; //Memory for the attachments
	VkPipelineDynamicStateCreateInfo _vkDynamicState;
	VkRect2D _scissors[FrameworkCaps::MaxScissorRegions];
	VkViewport _viewports[FrameworkCaps::MaxViewportRegions];
	VkDynamicState dynamicStates[FrameworkCaps::MaxDynamicStates];
	VkSpecializationInfo specializationInfos[FrameworkCaps::MaxSpecialisationInfos];
	char specializationInfoData[FrameworkCaps::MaxSpecialisationInfos][FrameworkCaps::MaxSpecialisationInfoDataSize];
	VkSpecializationMapEntry specilizationEntries[FrameworkCaps::MaxSpecialisationInfos][FrameworkCaps::MaxSpecialisationMapEntries];

public:
	/// <summary>Dereference operator - returns the underlying vulkan object</summary>
	/// <returns>The vulkan object</returns>
	VkGraphicsPipelineCreateInfo& operator*() { return createInfo; }

	/// <summary>Returns the underlying vulkan object</summary>
	/// <returns>The vulkan object</returns>
	const VkGraphicsPipelineCreateInfo& getVkCreateInfo()const { return createInfo; }

	/// <summary>Initialize this graphics pipeline</summary>
	/// <param name="gpcp">A framework graphics pipeline create param to be generated from</param>
	/// <returns>Returns return if success</returns>
	bool init(const GraphicsPipelineCreateInfo& gpcp)
	{
		if (!gpcp.pipelineLayout.isValid())
		{
			Log("Invalid Pipeline Layout");
			assertion(false, "Invalid Pipeline Layout");
			return false;
		}
		if (!gpcp.renderPass.isValid())
		{
			Log("Invalid Renderpass");
			assertion(false, "Invalid Renderpass");
			return false;
		}
		{
			// renderpass validation
			if (!gpcp.renderPass.isValid())
			{
				assertion(false, "Invalid RenderPass: A Pipeline must have a valid render pass");
				Log("Invalid RenderPass: A Pipeline must have a valid render pass");
				return false;
			}

			// assert that the vertex & fragment shader stage must be valid else it should be inhertied from the parent
			if (!gpcp.vertexShader.isActive())
			{
				assertion(false, "Graphics Pipeline should either have a valid vertex shader or inherited from its parent");
				Log("Graphics Pipeline should either have a valid vertex shader or inherited from its parent");
				return false;
			}
			if (!gpcp.fragmentShader.isActive())
			{
				assertion(false, "Graphics Pipeline should either have a valid fragment shader or inherited from its parent");
				Log("Graphics Pipeline should either have a valid fragment shader or inherited from its parent");
				return false;
			}

			createInfo.sType = VkStructureType::e_GRAPHICS_PIPELINE_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = gpcp.flags;
			//Set up the pipeline state
			createInfo.pNext = nullptr;
			createInfo.pInputAssemblyState = &_ia;
			createInfo.pRasterizationState = &_rs;
			createInfo.pMultisampleState = nullptr;
			createInfo.pViewportState = &_vp;
			createInfo.pColorBlendState = &_cb;
			createInfo.pDepthStencilState = (gpcp.depthStencil.isAllStatesEnabled() ? &_ds : nullptr);
			createInfo.pTessellationState = nullptr;
			createInfo.pVertexInputState = &_vertexInput;
			createInfo.pDynamicState = nullptr;
			createInfo.layout = gpcp.pipelineLayout->getNativeObject();
			createInfo.renderPass = gpcp.renderPass->getNativeObject();

			createInfo.subpass = gpcp.subpass;

			createInfo.stageCount = (gpcp.vertexShader.isActive() ? 1 : 0) +
			                        (gpcp.fragmentShader.isActive() ? 1 : 0) +
			                        (gpcp.tesselationStates.isControlShaderActive() ? 1 : 0) +
			                        (gpcp.tesselationStates.isEvaluationShaderActive() ? 1 : 0) +
			                        (gpcp.geometryShader.isActive() ? 1 : 0);
			createInfo.pStages = &_shaders[0];
		}
		{
			auto val = gpcp.inputAssembler;
			//input assembly
			_ia.sType = VkStructureType::e_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			_ia.pNext = nullptr;
			_ia.flags = 0;
			_ia.topology = val.getPrimitiveTopology();
			_ia.primitiveRestartEnable = val.isPrimitiveRestartEnabled();
		}
		{
			auto val = gpcp.vertexInput;
			//vertex input
			memset(&_vertexInput, 0, sizeof(_vertexInput));
			_vertexInput.sType = VkStructureType::e_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			_vertexInput.pNext = nullptr;
			_vertexInput.flags = 0;
			assertion(val.getAttributes().size() <= FrameworkCaps::MaxVertexAttributes);
			for (uint32_t i = 0; i < val.getAttributes().size(); i++)
			{
				convert(_vkVertexAttributes[i], val.getAttributes()[i]);
			}
			assertion(val.getInputBindings().size() <= FrameworkCaps::MaxVertexBindings);
			for (uint32_t i = 0; i < val.getInputBindings().size(); i++)
			{
				convert(_vkVertexBindings[i], *val.getInputBinding(i));
			}
			_vertexInput.vertexBindingDescriptionCount = static_cast<uint32_t>(val.getInputBindings().size());
			_vertexInput.pVertexBindingDescriptions = static_cast<uint32_t>(val.getInputBindings().size()) == 0u ?
			    nullptr : &_vkVertexBindings[0];
			_vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(val.getAttributes().size());
			_vertexInput.pVertexAttributeDescriptions = (val.getAttributes().size() ? &_vkVertexAttributes[0] : nullptr);
		}
		{
			uint32_t shaderIndex = 0;
			if (gpcp.vertexShader.isActive())
			{
				populateShaderInfo(gpcp.vertexShader.getShader()->getNativeObject(), VkShaderStageFlags::e_VERTEX_BIT,
				                   specializationInfos[0],
				                   specializationInfoData[0],
				                   gpcp.vertexShader.getAllShaderConstants(), gpcp.vertexShader.getNumShaderConsts(),
				                   specilizationEntries[0], _shaders[shaderIndex],
				                   gpcp.vertexShader.getEntryPoint());
				++shaderIndex;
			}
			if (gpcp.fragmentShader.isActive())
			{
				populateShaderInfo(gpcp.fragmentShader.getShader()->getNativeObject(),
				                   VkShaderStageFlags::e_FRAGMENT_BIT, specializationInfos[1],
				                   specializationInfoData[1],
				                   gpcp.fragmentShader.getAllShaderConstants(), gpcp.fragmentShader.getNumShaderConsts(),
				                   specilizationEntries[1], _shaders[shaderIndex],
				                   gpcp.fragmentShader.getEntryPoint());
				++shaderIndex;
			}
			if (gpcp.geometryShader.isActive())
			{
				populateShaderInfo(gpcp.geometryShader.getShader()->getNativeObject(),
				                   VkShaderStageFlags::e_GEOMETRY_BIT, specializationInfos[2],
				                   specializationInfoData[2],
				                   gpcp.geometryShader.getAllShaderConstants(), gpcp.geometryShader.getNumShaderConsts(),
				                   specilizationEntries[2],
				                   _shaders[shaderIndex], gpcp.geometryShader.getEntryPoint());
				++shaderIndex;
			}
			if (gpcp.tesselationStates.isControlShaderActive())
			{
				populateShaderInfo(gpcp.tesselationStates.getControlShader()->getNativeObject(),
				                   VkShaderStageFlags::e_TESSELLATION_CONTROL_BIT,
				                   specializationInfos[3],
				                   specializationInfoData[3],
				                   gpcp.tesselationStates.getAllControlShaderConstants(),
				                   gpcp.tesselationStates.getNumControlShaderConstants(),
				                   specilizationEntries[3],
				                   _shaders[shaderIndex], gpcp.tesselationStates.getControlShaderEntryPoint());
				++shaderIndex;
			}
			if (gpcp.tesselationStates.isEvaluationShaderActive())
			{
				populateShaderInfo(gpcp.tesselationStates.getEvaluationShader()->getNativeObject(),
				                   VkShaderStageFlags::e_TESSELLATION_EVALUATION_BIT,
				                   specializationInfos[4],
				                   specializationInfoData[4],
				                   gpcp.tesselationStates.getAllEvaluationShaderConstants(),
				                   gpcp.tesselationStates.getNumEvaluatinonShaderConstants(),
				                   specilizationEntries[4],
				                   _shaders[shaderIndex], gpcp.tesselationStates.getEvaluationShaderEntryPoint());
				++shaderIndex;
			}
		}
		// ColorBlend
		{
			auto val = gpcp.colorBlend;
			assertion(val.getNumAttachmentStates() <= FrameworkCaps::MaxColorAttachments);
			//color blend
			_cb.sType = VkStructureType::e_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			_cb.pNext = nullptr;
			_cb.flags = 0;
			_cb.logicOp = val.getLogicOp();
			_cb.logicOpEnable = val.isLogicOpEnabled();
			{
				_cb.blendConstants[0] = val.getColorBlendConst().r();
				_cb.blendConstants[1] = val.getColorBlendConst().g();
				_cb.blendConstants[2] = val.getColorBlendConst().b();
				_cb.blendConstants[3] = val.getColorBlendConst().a();
			}
			for (uint32_t i = 0; i < val.getNumAttachmentStates(); i++)
			{
				convert(_vkBlendAttachments[i], val.getAttachmentState(i));
			}
			_cb.pAttachments = &_vkBlendAttachments[0];
			_cb.attachmentCount = static_cast<uint32_t>(val.getNumAttachmentStates());
		}
		// DepthStencil
		if (createInfo.pDepthStencilState)
		{
			auto val = gpcp.depthStencil;
			//depth-stencil
			_ds.sType = VkStructureType::e_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			_ds.pNext = nullptr;
			_ds.flags = 0;
			_ds.depthTestEnable = val.isDepthTestEnable();
			_ds.depthWriteEnable = val.isDepthWriteEnable();
			_ds.depthCompareOp = val.getDepthComapreOp();
			_ds.depthBoundsTestEnable = val.isDepthBoundTestEnable();
			_ds.stencilTestEnable = val.isStencilTestEnable();
			_ds.minDepthBounds = val.getMinDepth();
			_ds.maxDepthBounds = val.getMaxDepth();

			convert(_ds.front, val.getStencilFront());
			convert(_ds.back, val.getStencilBack());
		}
		// Viewport
		{
			debug_assertion(gpcp.viewport.getNumViewportScissors() > 0, "Pipeline must have atleast one viewport and scissor");

			for (uint32_t i = 0; i < gpcp.viewport.getNumViewportScissors(); ++i)
			{
				convert(_viewports[i], gpcp.viewport.getViewport(i));
				_scissors[i] = gpcp.viewport.getScissor(i);
			}

			//viewport-scissor
			_vp.sType = VkStructureType::e_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			_vp.pNext = nullptr;
			_vp.flags = 0;
			_vp.viewportCount = gpcp.viewport.getNumViewportScissors();
			_vp.pViewports = _viewports;
			_vp.scissorCount = gpcp.viewport.getNumViewportScissors();
			_vp.pScissors = _scissors;
		}
		// Rasterizer
		{
			auto val = gpcp.rasterizer;
			//rasterization
			_rs.sType = VkStructureType::e_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			_rs.pNext = nullptr;
			_rs.flags = 0;
			_rs.depthClampEnable = !val.isDepthClipEnabled();
			_rs.rasterizerDiscardEnable = val.isRasterizerDiscardEnabled();
			_rs.polygonMode = val.getPolygonMode();
			_rs.cullMode = val.getCullFace();
			_rs.frontFace = val.getFrontFaceWinding();
			_rs.depthBiasEnable = val.isDepthBiasEnabled();
			_rs.depthBiasClamp = val.getDepthBiasClamp();
			_rs.depthBiasConstantFactor = val.getDepthBiasConstantFactor();
			_rs.depthBiasSlopeFactor = val.getDepthBiasSlopeFactor();
			_rs.lineWidth = val.getLineWidth();
		}
		// Multisample
		if (!_rs.rasterizerDiscardEnable)
		{
			if (gpcp.multiSample.isStateEnabled())
			{
				auto val = gpcp.multiSample;
				static VkSampleMask sampleMask = val.getSampleMask();
				//multisampling
				_ms.sType = VkStructureType::e_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
				_ms.pNext = nullptr;
				_ms.flags = 0;
				_ms.rasterizationSamples = gpcp.multiSample.getRasterizationSamples();
				_ms.sampleShadingEnable = val.isSampleShadingEnabled();
				_ms.minSampleShading = val.getMinSampleShading();
				_ms.pSampleMask = &sampleMask;
				_ms.alphaToCoverageEnable = val.isAlphaToCoverageEnabled();
				_ms.alphaToOneEnable = val.isAlphaToOneEnabled();
				createInfo.pMultisampleState = &_ms;
			}
			else
			{
				auto val = gpcp.multiSample;
				static VkSampleMask sampleMask = val.getSampleMask();
				//multisampling
				_ms.sType = VkStructureType::e_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
				_ms.pNext = nullptr;
				_ms.flags = 0;
				_ms.rasterizationSamples = gpcp.multiSample.getRasterizationSamples();
				_ms.sampleShadingEnable = val.isSampleShadingEnabled();
				_ms.minSampleShading = val.getMinSampleShading();
				_ms.pSampleMask = &sampleMask;
				_ms.alphaToCoverageEnable = val.isAlphaToCoverageEnabled();
				_ms.alphaToOneEnable = val.isAlphaToOneEnabled();
				createInfo.pMultisampleState = &_ms;
			}
		}

		{
			uint32_t count = 0;
			for (uint32_t i = 0; i < static_cast<uint32_t>(VkDynamicState::e_RANGE_SIZE); ++i)
			{
				if (gpcp.dynamicStates.isDynamicStateEnabled((VkDynamicState)i))
				{
					dynamicStates[count] = (VkDynamicState)i;
					++count;
				}
			}
			_vkDynamicState.sType = VkStructureType::e_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			_vkDynamicState.flags = 0;
			_vkDynamicState.pNext = nullptr;
			_vkDynamicState.pDynamicStates = dynamicStates;
			_vkDynamicState.dynamicStateCount = count;
			createInfo.pDynamicState = (count != 0 ? &_vkDynamicState : nullptr);
		}
		createInfo.basePipelineHandle = VK_NULL_HANDLE;
		if (gpcp.basePipeline.isValid())
		{
			createInfo.basePipelineHandle = gpcp.basePipeline->getNativeObject();
		}
		createInfo.basePipelineIndex = gpcp.basePipelineIndex;
		return true;
	}
};

/// <summary>Contains everything needed to define a VkComputePipelineCreateInfo, with provision for all memory required
/// </summary>
struct ComputePipelinePopulate
{
	VkComputePipelineCreateInfo createInfo; //!< After construction, will contain the ready-to-use create info

	/// <summary>Dereference operator - returns the underlying vulkan object</summary>
	/// <returns>The vulkan object</returns>
	VkComputePipelineCreateInfo& operator*() { return createInfo; }
	/// <summary>Initialize this compute pipeline</summary>
	/// <param name="cpcp">A framework compute pipeline create param to be generated from</param>
	/// <returns>Returns return if success</returns>
	bool init(const ComputePipelineCreateInfo& cpcp)
	{
		if (cpcp.pipelineLayout.isNull())
		{
			Log("PipelineLayout must be valid");
			return false;
		}
		createInfo.sType = VkStructureType::e_COMPUTE_PIPELINE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = cpcp.flags;
		//Set up the pipeline state
		createInfo.pNext = nullptr;
		createInfo.basePipelineHandle = (cpcp.basePipeline.isValid() ? cpcp.basePipeline->getNativeObject() : VK_NULL_HANDLE);
		createInfo.basePipelineIndex = cpcp.basePipelineIndex;
		createInfo.layout = cpcp.pipelineLayout->getNativeObject();

		const ShaderStageCreateInfo& val = cpcp.computeShader;
		//vertex shader
		createInfo.stage.sType = VkStructureType::e_PIPELINE_SHADER_STAGE_CREATE_INFO;
		createInfo.stage.pNext = nullptr;
		createInfo.stage.flags = 0;
		createInfo.stage.pSpecializationInfo = nullptr;
		createInfo.stage.stage = VkShaderStageFlags::e_COMPUTE_BIT;
		createInfo.stage.module = val.getShader()->getNativeObject();
		createInfo.stage.pName = val.getEntryPoint();
		return true;
	}
};
}//impl
}// namespace pvrvk
