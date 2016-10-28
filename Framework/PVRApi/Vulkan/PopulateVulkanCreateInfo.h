/*!*********************************************************************************************************************
\file         PVRApi\Vulkan\PopulateVulkanCreateInfo.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contain helper for populating vulkan pipeline create info
***********************************************************************************************************************/
//!\cond NO_DOXYGEN

#pragma once
#include "PVRApi/ApiObjects/GraphicsPipeline.h"
#include "PVRApi/ApiObjects/ComputePipeline.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRApi/Vulkan/PipelineLayoutVk.h"
#include "PVRApi/Vulkan/RenderPassVk.h"
#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"
#include "PVRApi/Vulkan/ShaderVk.h"

namespace pvr {
namespace api {
namespace vulkan {
/*!
\brief	Contains everything needed to define a VkGraphicsPipelineCreateInfo, with provision for all memory required
*/
struct GraphicsPipelineCreateInfoVulkan
{
	VkGraphicsPipelineCreateInfo createInfo; //<! After construction, will contain the ready-to-use create info
	VkPipelineInputAssemblyStateCreateInfo _ia;
	VkPipelineRasterizationStateCreateInfo _rs;
	VkPipelineMultisampleStateCreateInfo _ms;
	VkPipelineViewportStateCreateInfo _vp;
	VkPipelineColorBlendStateCreateInfo _cb;
	VkPipelineDepthStencilStateCreateInfo _ds;
	VkPipelineVertexInputStateCreateInfo _vertexInput;
	VkPipelineShaderStageCreateInfo _shaders[(uint32)types::ShaderType::Count];

	VkGraphicsPipelineCreateInfo& operator*() { return createInfo; }

	VkVertexInputBindingDescription _vkVertexBindings[8]; //Memory for the bindings
	VkVertexInputAttributeDescription _vkVertexAttributes[8]; //Memory for the attributes
	VkPipelineColorBlendAttachmentState _vkBlendAttachments[8]; //Memory for the attachments
	VkPipelineDynamicStateCreateInfo _vkDynamicState;
	std::vector<VkRect2D> scissors;
	std::vector<VkViewport> viewport;
	VkDynamicState dynamicStates[(uint32)types::DynamicState::Count];
	VkSpecializationInfo specializationInfos[(uint32)types::ShaderType::Count];
	std::vector<byte> specializationInfoData[(uint32)types::ShaderType::Count];
	std::vector<VkSpecializationMapEntry> specilizationEntries;

	/*!****************************************************************************************************************
	\brief ctor
	\param gpcp A framework graphics pipeline create param to be generated from
	\param context The Context the pipeline to be created from
	\param parent A Parent pipeline to be inherited from
	******************************************************************************************************************/
	GraphicsPipelineCreateInfoVulkan(const GraphicsPipelineCreateParam& gpcp, const GraphicsContext& context,
									 impl::ParentableGraphicsPipeline_* parent = NULL)
	{
		const pvr::platform::DisplayAttributes& displayAttr = context->getDisplayAttributes();
		{
			// renderpass validation
			if (!(gpcp.renderPass.isValid() || (parent != NULL && parent->getCreateParam().renderPass.isValid())))
			{
				assertion(false, "Invalid RenderPass: A Pipeline must have a valid render pass");
				Log("Invalid RenderPass: A Pipeline must have a valid render pass");
			}

			// assert that the vertex & fragment shader stage must be valid else it should be inhertied from the parent
			if (!gpcp.vertexShader.isActive() && parent == NULL)
			{
				assertion(false, "Graphics Pipeline should either have a valid vertex shader or inherited from its parent");
				Log("Graphics Pipeline should either have a valid vertex shader or inherited from its parent");

			}
			if (!gpcp.fragmentShader.isActive() && parent == NULL)
			{
				assertion(false, "Graphics Pipeline should either have a valid fragment shader or inherited from its parent");
				Log("Graphics Pipeline should either have a valid fragment shader or inherited from its parent");
			}

			createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			createInfo.pNext = NULL;
			createInfo.flags = (parent != NULL) * VK_PIPELINE_CREATE_DERIVATIVE_BIT;
			//Set up the pipeline state
			createInfo.pNext = NULL;
			createInfo.pInputAssemblyState = &_ia;
			createInfo.pRasterizationState = &_rs;
			createInfo.pMultisampleState = &_ms;
			createInfo.pViewportState = &_vp;
			createInfo.pColorBlendState = &_cb;
			createInfo.pDepthStencilState = (gpcp.depthStencil.isStateEnable() ? &_ds : NULL);
			createInfo.pTessellationState = NULL;
			createInfo.pVertexInputState = &_vertexInput;
			createInfo.pDynamicState = NULL;

            createInfo.basePipelineHandle = parent ? native_cast(*parent).handle : VK_NULL_HANDLE;
			createInfo.basePipelineIndex = -1;
			createInfo.layout = (gpcp.pipelineLayout.isValid() ?  native_cast(*gpcp.pipelineLayout).handle : VK_NULL_HANDLE);
			createInfo.renderPass = (gpcp.renderPass.isValid()  ?  native_cast(*gpcp.renderPass).handle : VK_NULL_HANDLE);
			assertion(createInfo.layout != VK_NULL_HANDLE, "Pipelinelayout must be Valid");
			assertion(createInfo.renderPass != VK_NULL_HANDLE, "Renderpass mus be valid");

			createInfo.subpass = gpcp.subPass;
			createInfo.flags = 0;

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
			_ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			_ia.pNext = NULL;
			_ia.flags = 0;
			_ia.topology = ConvertToVk::primitiveTopology(val.topology);
			_ia.primitiveRestartEnable = val.primitiveRestartEnable;
		}
		{
			auto val = gpcp.vertexInput;
			//vertex input
			memset(&_vertexInput, 0, sizeof(_vertexInput));
			_vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			_vertexInput.pNext = NULL;
			_vertexInput.flags = 0;
			assertion(val.getAttributes().size() <= 8);
			//_vkVertexAttributes.resize(gpcp.vertexInput.getAttributes().size());
			int tmp = 0;
			for (auto it = val.getAttributes().begin(); it != val.getAttributes().end(); ++it)
			{
				convert(_vkVertexAttributes[tmp++], *it);
			}
			assertion(val.getInputBindings().size() <= 8);
			tmp = 0;
			for (auto it = val.getInputBindings().begin(); it != val.getInputBindings().end(); ++it)
			{
				convert(_vkVertexBindings[tmp++], *it);
			}
			_vertexInput.vertexBindingDescriptionCount = (uint32)val.getInputBindings().size();
			_vertexInput.pVertexBindingDescriptions = (uint32)val.getInputBindings().size() == 0u ? NULL : &_vkVertexBindings[0];
			_vertexInput.vertexAttributeDescriptionCount = (uint32)val.getAttributes().size();
			_vertexInput.pVertexAttributeDescriptions = (val.getAttributes().size() ? &_vkVertexAttributes[0] : NULL);
		}
		{
			uint32 shaderIndex = 0;
			if (gpcp.vertexShader.isActive())
			{
				populateShaderInfo(native_cast(*gpcp.vertexShader.getShader()),
				                   specializationInfos[(uint32)types::ShaderType::VertexShader],
				                   specializationInfoData[(uint32)types::ShaderType::VertexShader],
								   VK_SHADER_STAGE_VERTEX_BIT, gpcp.vertexShader.getAllShaderConstants(),
								   gpcp.vertexShader.getNumShaderConsts(),
								   _shaders[shaderIndex]);
				++shaderIndex;
			}
			if (gpcp.fragmentShader.isActive())
			{
				populateShaderInfo(native_cast(*gpcp.fragmentShader.getShader()),
				                   specializationInfos[(uint32)types::ShaderType::FragmentShader],
				                   specializationInfoData[(uint32)types::ShaderType::FragmentShader],
								   VK_SHADER_STAGE_FRAGMENT_BIT, gpcp.vertexShader.getAllShaderConstants(),
								   gpcp.vertexShader.getNumShaderConsts()
								   , _shaders[shaderIndex]);
				++shaderIndex;
			}
			if (gpcp.geometryShader.isActive())
			{
				populateShaderInfo(native_cast(*gpcp.geometryShader.getShader()),
				                   specializationInfos[(uint32)types::ShaderType::GeometryShader],
				                   specializationInfoData[(uint32)types::ShaderType::GeometryShader],
								   VK_SHADER_STAGE_GEOMETRY_BIT, gpcp.geometryShader.getAllShaderConstants(),
								   gpcp.geometryShader.getNumShaderConsts(),
								   _shaders[shaderIndex]);
				++shaderIndex;
			}
			if (gpcp.tesselationStates.isControlShaderActive())
			{
				populateShaderInfo(native_cast(*gpcp.tesselationStates.getControlShader()),
				                   specializationInfos[(uint32)types::ShaderType::TessControlShader],
				                   specializationInfoData[(uint32)types::ShaderType::TessControlShader],
								   VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
								   gpcp.tesselationStates.getAllControlShaderConstants(),
								   gpcp.tesselationStates.getNumControlShaderConstants(), _shaders[shaderIndex]);
				++shaderIndex;
			}
			if (gpcp.tesselationStates.isEvaluationShaderActive())
			{
				populateShaderInfo(native_cast(*gpcp.tesselationStates.getEvaluationShader()),
				                   specializationInfos[(uint32)types::ShaderType::TessEvaluationShader],
				                   specializationInfoData[(uint32)types::ShaderType::TessEvaluationShader],
								   VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
								   gpcp.tesselationStates.getAllEvaluationShaderConstants(),
								   gpcp.tesselationStates.getNumEvaluatinonShaderConstants(), _shaders[shaderIndex]);
				++shaderIndex;
			}
		}
		{
			auto val = gpcp.colorBlend;
			assertion(val.getAttachmentStates().size() <= 8);
			//color blend
			_cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			_cb.pNext = NULL;
			_cb.flags = 0;
			_cb.logicOp = ConvertToVk::logicOp(val.logicOp);
			_cb.logicOpEnable = val.logicOpEnable;
			{
				_cb.blendConstants[0] = val.getColorBlendConst().x;
				_cb.blendConstants[1] = val.getColorBlendConst().y;
				_cb.blendConstants[2] = val.getColorBlendConst().z;
				_cb.blendConstants[3] = val.getColorBlendConst().w;
			}
			int tmp = 0;
			for (auto it = val.attachmentStates.begin(); it != val.attachmentStates.end(); ++it)
			{
				convert(_vkBlendAttachments[tmp], *it);
				++tmp;
			}
			_cb.pAttachments = &_vkBlendAttachments[0];
			_cb.attachmentCount = (uint32)val.attachmentStates.size();
		}
		if (createInfo.pDepthStencilState)
		{
			auto val = gpcp.depthStencil;
			//depth-stencil
			_ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			_ds.pNext = NULL;
			_ds.flags = 0;
			_ds.depthTestEnable = val.isDepthTestEnable();
			_ds.depthWriteEnable = val.isDepthWriteEnable();
			_ds.depthCompareOp = ConvertToVk::compareMode(val.getDepthComapreOp());
			_ds.depthBoundsTestEnable = val.isDepthBoundTestEnable();
			_ds.stencilTestEnable = val.isStencilTestEnable();
			_ds.minDepthBounds = val.getMinDepth();
			_ds.maxDepthBounds = val.getMaxDepth();

			convert(val.getStencilFront(), _ds.front);
			convert(val.getStencilBack(), _ds.back);
		}
		{
			// create a default viewport if one is not provided
			if (gpcp.viewport.getNumViewportScissor() == 0)
			{
				viewport.push_back(VkViewport
				{
					0.0f,
					0.0f,
					(float32)displayAttr.width,
					(float32)displayAttr.height,
					0.0f,
					1.0f
				});
				scissors.push_back(VkRect2D
				{
					VkOffset2D{0, 0},
					VkExtent2D{ (uint32)displayAttr.width, (uint32)displayAttr.height }
				});
			}
			else
			{
				viewport.resize(gpcp.viewport.getNumViewportScissor());
				scissors.resize(gpcp.viewport.getNumViewportScissor());
				for (pvr::uint32 i = 0; i < gpcp.viewport.getNumViewportScissor(); ++i)
				{
					convert(viewport[i], gpcp.viewport.getViewport(i));
					convert(scissors[i], gpcp.viewport.getScissor(i));
				}
			}
			//viewport-scissor
			_vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			_vp.pNext = NULL;
			_vp.flags = 0;
			_vp.viewportCount = (uint32)viewport.size();
			_vp.pViewports = viewport.data();
			_vp.scissorCount = (uint32)scissors.size();
			_vp.pScissors = scissors.data();
		}
		{
			auto val = gpcp.rasterizer;
			//rasterization
			_rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			_rs.pNext = NULL;
			_rs.flags = 0;
			_rs.depthClampEnable = !val.enableDepthClip;
			_rs.rasterizerDiscardEnable = val.enableRasterizerDiscard;
			_rs.polygonMode = ConvertToVk::polygonMode(val.fillMode);
			_rs.cullMode = ConvertToVk::cullMode(val.cullFace);
			_rs.frontFace = ConvertToVk::frontFaceWinding(val.frontFaceWinding);
			_rs.depthBiasEnable = val.isDepthBiasEnable();
			_rs.depthBiasClamp = val.isDepthBiasClampEnable();
			_rs.depthBiasConstantFactor = 0.;
			_rs.depthBiasSlopeFactor = 0.;
			_rs.lineWidth = val.getLineWidth();
		}
		{
			auto val = gpcp.multiSample;
			static VkSampleMask sampleMask = val.getSampleMask();
			//multisampling
			_ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			_ms.pNext = NULL;
			_ms.flags = 0;
			_ms.rasterizationSamples = ConvertToVk::aaSamples((uint8)displayAttr.aaSamples);
			_ms.sampleShadingEnable = val.isSampleShadingEnabled();
			_ms.minSampleShading = val.getMinSampleShading();
			_ms.pSampleMask = &sampleMask;
			_ms.alphaToCoverageEnable = val.isAlphaToCoverageEnabled();
			_ms.alphaToOneEnable = val.isAlphaToOneEnabled();
		}
		{

			pvr::uint32 count = 0;
			for (uint32 i = 0; i < (uint32)types::DynamicState::Count; ++i)
			{
				if (gpcp.dynamicStates.isDynamicStateEnabled((types::DynamicState)i))
				{
					dynamicStates[count] = (VkDynamicState)i;
					++count;
				}
			}
			_vkDynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			_vkDynamicState.flags = 0;
			_vkDynamicState.pNext = NULL;
			_vkDynamicState.pDynamicStates = dynamicStates;
			_vkDynamicState.dynamicStateCount = count;
			createInfo.pDynamicState = (count != 0 ? &_vkDynamicState : NULL);
		}
	}

	/*!****************************************************************************************************************
	\brief Populate vulkan shader create info
	\param shader A ShaderModule
	\param shaderStage Pipeline shader stage (VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT etc)
	\param outShader A Parent pipeline to be inherited from
	******************************************************************************************************************/
	void populateShaderInfo(const VkShaderModule& shader, VkSpecializationInfo& specializationInfo,
							std::vector<byte>& specializationInfoData, VkShaderStageFlagBits vkShaderStage,
							const pipelineCreation::ShaderConstantInfo* shaderConsts,
							uint32 numShaderConsts, VkPipelineShaderStageCreateInfo& outShader)
	{
		// caculate the number of size in bytes required.
		uint32 specicalizedataSize = 0;
		for (pvr::uint32 i = 0; i < numShaderConsts; ++i)
		{
			specicalizedataSize += shaderConsts[i].sizeInBytes;
		}

		if (specicalizedataSize)
		{
			specializationInfoData.resize(specicalizedataSize);
			uint32 mapEntriesOffset = (uint32)specilizationEntries.size();
			uint32 dataOffset = 0;
			for (pvr::uint32 i = 0; i < numShaderConsts; ++i)
			{
				memcpy(&specializationInfoData[dataOffset], shaderConsts[i].data, shaderConsts[i].sizeInBytes);
				specilizationEntries.push_back(VkSpecializationMapEntry{shaderConsts[i].constantId, dataOffset,
											   shaderConsts[i].sizeInBytes});
				dataOffset += shaderConsts[i].sizeInBytes;
			}
			specializationInfo.dataSize = specicalizedataSize;
			specializationInfo.mapEntryCount = numShaderConsts;
			specializationInfo.pData = specializationInfoData.data();
			specializationInfo.pMapEntries = &specilizationEntries[mapEntriesOffset];
		}

		outShader.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		outShader.pNext = NULL;
		outShader.flags = 0;
		outShader.pSpecializationInfo = (specicalizedataSize ? &specializationInfo : NULL);
		outShader.stage = vkShaderStage;
		outShader.module = shader;
		outShader.pName = "main";
	}

	/*!****************************************************************************************************************
	\brief Populate vulkan input attribute description
	\param[out] vkva Return populated VkVertexInputAttributeDescription
	\param pvrva A vertex attribute info to convert from
	******************************************************************************************************************/
	void convert(VkVertexInputAttributeDescription& vkva, const VertexAttributeInfoWithBinding& pvrva)
	{
		vkva.binding = pvrva.binding;
		vkva.format = ConvertToVk::dataFormat(pvrva.format, pvrva.width);
		vkva.location = pvrva.index;
		vkva.offset = pvrva.offsetInBytes;
	}

	/*!****************************************************************************************************************
	\brief Populate vulkan input binding description
	\param[out] vkvb Return populated VkVertexInputBindingDescription
	\param pvrvb A vertex input binding info to convert from
	******************************************************************************************************************/
	void convert(VkVertexInputBindingDescription& vkvb, const VertexInputBindingInfo& pvrvb)
	{
		vkvb.binding = pvrvb.bindingId;
		vkvb.inputRate = ConvertToVk::stepRate(pvrvb.stepRate);
		vkvb.stride = pvrvb.strideInBytes;
	}

	/*!****************************************************************************************************************
	\brief Populate vulkan pipeline color blend attachment state
	\param[out] vkcb Return populated VkPipelineColorBlendAttachmentState
	\param pvrcb A color blend attachment state to convert from
	******************************************************************************************************************/
	void convert(VkPipelineColorBlendAttachmentState& vkcb, const types::BlendingConfig& pvrcb)
	{
		vkcb.alphaBlendOp = ConvertToVk::blendOp(pvrcb.blendOpAlpha);
		vkcb.blendEnable = pvrcb.blendEnable;
		vkcb.colorBlendOp = ConvertToVk::blendOp(pvrcb.blendOpColor);
		vkcb.colorWriteMask = ConvertToVk::colorChannel(pvrcb.channelWriteMask);
		vkcb.dstAlphaBlendFactor = ConvertToVk::blendFactor(pvrcb.destBlendAlpha);
		vkcb.dstColorBlendFactor = ConvertToVk::blendFactor(pvrcb.destBlendColor);
		vkcb.srcAlphaBlendFactor = ConvertToVk::blendFactor(pvrcb.srcBlendAlpha);
		vkcb.srcColorBlendFactor = ConvertToVk::blendFactor(pvrcb.srcBlendColor);
	}

	/*!****************************************************************************************************************
	\brief Populate vulkan stencil state
	\param stencilState A stencil state state to convert from
	\param vkStencilState Return populated VkStencilOpState
	******************************************************************************************************************/
	void convert(const pipelineCreation::DepthStencilStateCreateParam::StencilState& stencilState,
				 VkStencilOpState& vkStencilState)
	{
		vkStencilState.failOp = ConvertToVk::stencilOp(stencilState.opStencilFail);
		vkStencilState.passOp = ConvertToVk::stencilOp(stencilState.opDepthPass);
		vkStencilState.depthFailOp = ConvertToVk::stencilOp(stencilState.opDepthFail);
		vkStencilState.compareOp = ConvertToVk::compareMode(stencilState.compareOp);
		vkStencilState.compareMask = stencilState.compareMask;
		vkStencilState.writeMask = stencilState.writeMask;
		vkStencilState.reference = stencilState.reference;
	}

	/*!****************************************************************************************************************
	\brief Populate vulkan scissor box
	\param scissor A scissor box to convert from
	\param vkScissor Return populated scissor box
	******************************************************************************************************************/
	void convert(VkRect2D& vkScissor, const Rectanglei& scissor)
	{
		vkScissor.extent.width = scissor.width, vkScissor.extent.height = scissor.height;
		vkScissor.offset.x = scissor.x, vkScissor.offset.y = scissor.y;
	}

	/*!****************************************************************************************************************
	\brief Populate vulkan viewport
	\param vp A viewport to convert from
	\param vkvp Return populated Viewport
	******************************************************************************************************************/
	void convert(VkViewport& vkvp, const Viewport& vp)
	{
		vkvp.x = vp.x;
		vkvp.y = vp.y;
		vkvp.width = vp.width;
		vkvp.height = vp.height;
		vkvp.minDepth = vp.minDepth;
		vkvp.maxDepth = vp.maxDepth;
	}
};

/*!
\brief	Contains everything needed to define a VkComputePipelineCreateInfo, with provision for all memory required
*/
struct ComputePipelineCreateInfoVulkan
{
	VkComputePipelineCreateInfo createInfo; //<! After construction, will contain the ready-to-use create info

	VkComputePipelineCreateInfo& operator*() { return createInfo; }
	/*!****************************************************************************************************************
	\brief ctor
	\param gpcp A framework compute pipeline create param to be generated from
	\param context The Context the pipeline to be created from
	******************************************************************************************************************/
	ComputePipelineCreateInfoVulkan(const ComputePipelineCreateParam& cpcp, const GraphicsContext& /*context*/)
	{
		{
			createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			createInfo.pNext = NULL;
			createInfo.flags = 0;
			//Set up the pipeline state
			createInfo.pNext = NULL;
			//createInfo.basePipelineHandle = parent ? native_cast(*parent) : VK_NULL_HANDLE;
			createInfo.basePipelineHandle = VK_NULL_HANDLE;
			createInfo.layout = native_cast(*cpcp.pipelineLayout);
			//	createInfo.basePipelineIndex = -1;
		}

		const pipelineCreation::ComputeShaderStageCreateParam& val = cpcp.computeShader;
		//vertex shader
		createInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		createInfo.stage.pNext = NULL;
		createInfo.stage.flags = 0;
		createInfo.stage.pSpecializationInfo = NULL;
		createInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		createInfo.stage.module = native_cast(*val.getShader());
		createInfo.stage.pName = val.getEntryPoint();

	}
};
}//vulkan
}//api
}//pvr

//!\endcond