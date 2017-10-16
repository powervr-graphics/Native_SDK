/*!
\brief Contain helper for populating vulkan pipeline create info
\file PVRApi/Vulkan/PopulateVulkanCreateInfo.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRApi/ApiObjects/GraphicsPipeline.h"
#include "PVRApi/ApiObjects/ComputePipeline.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRApi/Vulkan/PipelineLayoutVk.h"
#include "PVRApi/Vulkan/DescriptorSetVk.h"
#include "PVRApi/Vulkan/RenderPassVk.h"
#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"
#include "PVRApi/Vulkan/ShaderVk.h"
#include "PVRApi/ApiUtils.h"
namespace pvr {
namespace api {
namespace vulkan {

/// <summary>Populate vulkan input attribute description</summary>
/// <param name="vkva">Return populated VkVertexInputAttributeDescription</param>
/// <param name="pvrva">A vertex attribute info to convert from</param>
inline void convert(VkVertexInputAttributeDescription& vkva, const VertexAttributeInfoWithBinding& pvrva)
{
	vkva.binding = pvrva.binding;
	vkva.format = nativeVk::ConvertToVk::dataFormat(pvrva.format, pvrva.width);
	vkva.location = pvrva.index;
	vkva.offset = pvrva.offsetInBytes;
}

/// <summary>Populate vulkan input binding description</summary>
/// <param name="vkvb">Return populated VkVertexInputBindingDescription</param>
/// <param name="pvrvb">A vertex input binding info to convert from</param>
inline void convert(VkVertexInputBindingDescription& vkvb, const VertexInputBindingInfo& pvrvb)
{
	vkvb.binding = pvrvb.bindingId;
	vkvb.inputRate = nativeVk::ConvertToVk::stepRate(pvrvb.stepRate);
	vkvb.stride = pvrvb.strideInBytes;
}

/// <summary>Populate vulkan pipeline color blend attachment state</summary>
/// <param name="vkcb">Return populated VkPipelineColorBlendAttachmentState</param>
/// <param name="pvrcb">A color blend attachment state to convert from</param>
inline void convert(VkPipelineColorBlendAttachmentState& vkcb, const types::BlendingConfig& pvrcb)
{
	vkcb.alphaBlendOp = nativeVk::ConvertToVk::blendOp(pvrcb.blendOpAlpha);
	vkcb.blendEnable = pvrcb.blendEnable;
	vkcb.colorBlendOp = nativeVk::ConvertToVk::blendOp(pvrcb.blendOpColor);
	vkcb.colorWriteMask = nativeVk::ConvertToVk::colorChannel(pvrcb.channelWriteMask);
	vkcb.dstAlphaBlendFactor = nativeVk::ConvertToVk::blendFactor(pvrcb.destBlendAlpha);
	vkcb.dstColorBlendFactor = nativeVk::ConvertToVk::blendFactor(pvrcb.destBlendColor);
	vkcb.srcAlphaBlendFactor = nativeVk::ConvertToVk::blendFactor(pvrcb.srcBlendAlpha);
	vkcb.srcColorBlendFactor = nativeVk::ConvertToVk::blendFactor(pvrcb.srcBlendColor);
}

/// <summary>Populate vulkan stencil state</summary>
/// <param name="stencilState">A stencil state state to convert from</param>
/// <param name="vkStencilState">Return populated VkStencilOpState</param>
inline void convert(const pipelineCreation::DepthStencilStateCreateParam::StencilState& stencilState,
                    VkStencilOpState& vkStencilState)
{
	vkStencilState.failOp = nativeVk::ConvertToVk::stencilOp(stencilState.opStencilFail);
	vkStencilState.passOp = nativeVk::ConvertToVk::stencilOp(stencilState.opDepthPass);
	vkStencilState.depthFailOp = nativeVk::ConvertToVk::stencilOp(stencilState.opDepthFail);
	vkStencilState.compareOp = nativeVk::ConvertToVk::compareMode(stencilState.compareOp);
	vkStencilState.compareMask = stencilState.compareMask;
	vkStencilState.writeMask = stencilState.writeMask;
	vkStencilState.reference = stencilState.reference;
}

/// <summary>Populate vulkan scissor box</summary>
/// <param name="scissor">A scissor box to convert from</param>
/// <param name="vkScissor">Return populated scissor box</param>
inline void convert(VkRect2D& vkScissor, const Rectanglei& scissor, const glm::ivec2& renderSurfaceDimensions)
{
	const Rectanglei& rect = utils::framebufferRectangleToVk(scissor, renderSurfaceDimensions);
	vkScissor.offset.x = rect.x, vkScissor.offset.y = rect.y;
	vkScissor.extent.width = rect.width, vkScissor.extent.height = rect.height;
}

/// <summary>Populate vulkan viewport</summary>
/// <param name="vp">A viewport to convert from</param>
/// <param name="vkvp">Return populated Viewport</param>
inline void convert(VkViewport& vkvp, const Viewport& vp, const glm::ivec2& renderSurfaceDimensions)
{
	const Rectanglef& rect = utils::framebufferRectangleToVk(vp.getRegion(), renderSurfaceDimensions);
	vkvp.x = rect.x;
	vkvp.y = rect.y;
	vkvp.width = rect.width;
	vkvp.height = rect.height;
	vkvp.minDepth = vp.minDepth;
	vkvp.maxDepth = vp.maxDepth;
}

/// <summary>Populate vulkan shader create info</summary>
/// <param name="shader">A ShaderModule</param>
/// <param name="shaderStage">Pipeline shader stage (VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT etc)
/// </param>
/// <param name="outShader">A Parent pipeline to be inherited from</param>
inline void populateShaderInfo(const VkShaderModule& shader, VkShaderStageFlagBits vkShaderStage,
                               VkSpecializationInfo& specializationInfo, byte* specializationInfoData, const pvr::uint32& specializationInfoDataCount,
                               const pipelineCreation::ShaderConstantInfo* shaderConsts, uint32 numShaderConsts, VkSpecializationMapEntry* mapEntries,
                               VkPipelineShaderStageCreateInfo& outShader, const char* entryPoint)
{
	// caculate the number of size in bytes required.
	uint32 specicalizedataSize = 0;
	for (pvr::uint32 i = 0; i < numShaderConsts; ++i)
	{
		specicalizedataSize += shaderConsts[i].sizeInBytes;
	}

	if (specicalizedataSize)
	{
		debug_assertion(specicalizedataSize < pvr::types::PipelineDefaults::SpecialisationStates::MaxSpecialisationInfoDataSize, "Specialised Data out of range.");
		uint32 dataOffset = 0;
		for (pvr::uint32 i = 0; i < numShaderConsts; ++i)
		{
			memcpy(&specializationInfoData[dataOffset], shaderConsts[i].data, shaderConsts[i].sizeInBytes);
			mapEntries[i] = VkSpecializationMapEntry{ shaderConsts[i].constantId, dataOffset,
			                                          shaderConsts[i].sizeInBytes };
			dataOffset += shaderConsts[i].sizeInBytes;
		}
		specializationInfo.mapEntryCount = numShaderConsts;
		specializationInfo.pMapEntries = mapEntries;
		specializationInfo.dataSize = specicalizedataSize;
		specializationInfo.pData = specializationInfoData;
	}

	outShader.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	outShader.pNext = NULL;
	outShader.flags = 0;
	outShader.pSpecializationInfo = (specicalizedataSize ? &specializationInfo : NULL);
	outShader.stage = vkShaderStage;
	outShader.module = shader;
	outShader.pName = entryPoint;
}

/// <summary>Contains everything needed to define a VkGraphicsPipelineCreateInfo, with provision for all memory
/// required</summary>
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

	VkVertexInputBindingDescription _vkVertexBindings[types::PipelineDefaults::VertexInput::MaxVertexBindings]; //Memory for the bindings
	VkVertexInputAttributeDescription _vkVertexAttributes[types::PipelineDefaults::VertexAttributeInfo::MaxVertexAttributes]; //Memory for the attributes
	VkPipelineColorBlendAttachmentState _vkBlendAttachments[types::PipelineDefaults::ColorBlend::MaxBlendAttachments]; //Memory for the attachments
	VkPipelineDynamicStateCreateInfo _vkDynamicState;
	VkRect2D _scissors[types::PipelineDefaults::ViewportScissor::MaxScissorRegions];
	VkViewport _viewports[types::PipelineDefaults::ViewportScissor::MaxViewportRegions];
	VkDynamicState dynamicStates[types::PipelineDefaults::DynamicStates::MaxDynamicStates];
	VkSpecializationInfo specializationInfos[types::PipelineDefaults::SpecialisationStates::MaxSpecialisationInfos];
	byte specializationInfoData[types::PipelineDefaults::SpecialisationStates::MaxSpecialisationInfos][types::PipelineDefaults::SpecialisationStates::MaxSpecialisationInfoDataSize];
	uint32 specializationInfoDataCount[types::PipelineDefaults::SpecialisationStates::MaxSpecialisationInfos];
	VkSpecializationMapEntry specilizationEntries[types::PipelineDefaults::SpecialisationStates::MaxSpecialisationInfos][types::PipelineDefaults::SpecialisationStates::MaxSpecialisationMapEntries];

	/// <summary>ctor</summary>
	/// <param name="gpcp">A framework graphics pipeline create param to be generated from</param>
	/// <param name="context">The Context the pipeline to be created from</param>
	/// <param name="parent">A Parent pipeline to be inherited from</param>
	GraphicsPipelineCreateInfoVulkan(const GraphicsPipelineCreateParam& gpcp, const GraphicsContext& context,
	                                 const ParentableGraphicsPipeline& parent)
	{
		const pvr::platform::DisplayAttributes& displayAttr = context->getDisplayAttributes();
		{
			// renderpass validation
			if (!(gpcp.renderPass.isValid() || (!parent.isNull() && parent->getCreateParam().renderPass.isValid())))
			{
				assertion(false, "Invalid RenderPass: A Pipeline must have a valid render pass");
				Log("Invalid RenderPass: A Pipeline must have a valid render pass");
			}

			// assert that the vertex & fragment shader stage must be valid else it should be inhertied from the parent
			if (!gpcp.vertexShader.isActive() && parent.isNull())
			{
				assertion(false, "Graphics Pipeline should either have a valid vertex shader or inherited from its parent");
				Log("Graphics Pipeline should either have a valid vertex shader or inherited from its parent");

			}
			if (!gpcp.fragmentShader.isActive() && parent.isNull())
			{
				assertion(false, "Graphics Pipeline should either have a valid fragment shader or inherited from its parent");
				Log("Graphics Pipeline should either have a valid fragment shader or inherited from its parent");
			}

			createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			createInfo.pNext = NULL;
			createInfo.flags = (!parent.isNull()) * VK_PIPELINE_CREATE_DERIVATIVE_BIT;
			//Set up the pipeline state
			createInfo.pNext = NULL;
			createInfo.pInputAssemblyState = &_ia;
			createInfo.pRasterizationState = &_rs;
			createInfo.pMultisampleState = NULL;
			createInfo.pViewportState = &_vp;
			createInfo.pColorBlendState = &_cb;
			createInfo.pDepthStencilState = (gpcp.depthStencil.isStateEnable() ? &_ds : NULL);
			createInfo.pTessellationState = NULL;
			createInfo.pVertexInputState = &_vertexInput;
			createInfo.pDynamicState = NULL;

			createInfo.basePipelineHandle = parent.isValid() ? native_cast(parent).handle : VK_NULL_HANDLE;
			createInfo.basePipelineIndex = -1;
			createInfo.layout = (gpcp.pipelineLayout.isValid() ? pvr::api::native_cast(*gpcp.pipelineLayout).handle : VK_NULL_HANDLE);
			createInfo.renderPass = (gpcp.renderPass.isValid() ? pvr::api::native_cast(*gpcp.renderPass).handle : VK_NULL_HANDLE);
			assertion(createInfo.layout != VK_NULL_HANDLE, "Pipelinelayout must be Valid");
			assertion(createInfo.renderPass != VK_NULL_HANDLE, "Renderpass mus be valid");

			createInfo.subpass = gpcp.subPass;

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
			_ia.topology = nativeVk::ConvertToVk::primitiveTopology(val.topology);
			_ia.primitiveRestartEnable = val.isPrimitiveRestartEnabled();
		}
		{
			auto val = gpcp.vertexInput;
			//vertex input
			memset(&_vertexInput, 0, sizeof(_vertexInput));
			_vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			_vertexInput.pNext = NULL;
			_vertexInput.flags = 0;
			assertion(val.getAttributes().size() <= (uint32)pvr::types::PipelineDefaults::VertexAttributeInfo::MaxVertexAttributes);
			for (pvr::uint32 i = 0; i < val.getAttributes().size(); i++)
			{
				convert(_vkVertexAttributes[i], val.getAttributes()[i]);
			}
			assertion(val.getInputBindings().size() <= (uint32)pvr::types::PipelineDefaults::VertexInput::MaxVertexBindings);
			for (pvr::uint32 i = 0; i < val.getInputBindings().size(); i++)
			{
				convert(_vkVertexBindings[i], *val.getInputBinding(i));
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
				populateShaderInfo(pvr::api::native_cast(*gpcp.vertexShader.getShader()), VK_SHADER_STAGE_VERTEX_BIT,
				                   specializationInfos[(uint32)types::ShaderType::VertexShader],
				                   specializationInfoData[(uint32)types::ShaderType::VertexShader],
				                   specializationInfoDataCount[(uint32)types::ShaderType::VertexShader],
				                   gpcp.vertexShader.getAllShaderConstants(), gpcp.vertexShader.getNumShaderConsts(),
				                   specilizationEntries[(uint32)types::ShaderType::VertexShader], _shaders[shaderIndex], gpcp.vertexShader.getEntryPoint());
				++shaderIndex;
			}
			if (gpcp.fragmentShader.isActive())
			{
				populateShaderInfo(pvr::api::native_cast(*gpcp.fragmentShader.getShader()), VK_SHADER_STAGE_FRAGMENT_BIT,
				                   specializationInfos[(uint32)types::ShaderType::FragmentShader],
				                   specializationInfoData[(uint32)types::ShaderType::FragmentShader],
				                   specializationInfoDataCount[(uint32)types::ShaderType::FragmentShader],
				                   gpcp.fragmentShader.getAllShaderConstants(), gpcp.fragmentShader.getNumShaderConsts(),
				                   specilizationEntries[(uint32)types::ShaderType::FragmentShader], _shaders[shaderIndex], gpcp.fragmentShader.getEntryPoint());
				++shaderIndex;
			}
			if (gpcp.geometryShader.isActive())
			{
				populateShaderInfo(pvr::api::native_cast(*gpcp.geometryShader.getShader()), VK_SHADER_STAGE_GEOMETRY_BIT,
				                   specializationInfos[(uint32)types::ShaderType::GeometryShader],
				                   specializationInfoData[(uint32)types::ShaderType::GeometryShader],
				                   specializationInfoDataCount[(uint32)types::ShaderType::GeometryShader],
				                   gpcp.geometryShader.getAllShaderConstants(), gpcp.geometryShader.getNumShaderConsts(),
				                   specilizationEntries[(uint32)types::ShaderType::GeometryShader], _shaders[shaderIndex], gpcp.geometryShader.getEntryPoint());
				++shaderIndex;
			}
			if (gpcp.tesselationStates.isControlShaderActive())
			{
				populateShaderInfo(pvr::api::native_cast(*gpcp.tesselationStates.getControlShader()), VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
				                   specializationInfos[(uint32)types::ShaderType::TessControlShader],
				                   specializationInfoData[(uint32)types::ShaderType::TessControlShader],
				                   specializationInfoDataCount[(uint32)types::ShaderType::TessControlShader],
				                   gpcp.tesselationStates.getAllControlShaderConstants(), gpcp.tesselationStates.getNumControlShaderConstants(),
				                   specilizationEntries[(uint32)types::ShaderType::TessControlShader], _shaders[shaderIndex], gpcp.tesselationStates.getControlShaderEntryPoint());
				++shaderIndex;
			}
			if (gpcp.tesselationStates.isEvaluationShaderActive())
			{
				populateShaderInfo(pvr::api::native_cast(*gpcp.tesselationStates.getEvaluationShader()), VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
				                   specializationInfos[(uint32)types::ShaderType::TessControlShader],
				                   specializationInfoData[(uint32)types::ShaderType::TessControlShader],
				                   specializationInfoDataCount[(uint32)types::ShaderType::TessControlShader],
				                   gpcp.tesselationStates.getAllEvaluationShaderConstants(), gpcp.tesselationStates.getNumEvaluatinonShaderConstants(),
				                   specilizationEntries[(uint32)types::ShaderType::TessControlShader], _shaders[shaderIndex], gpcp.tesselationStates.getEvaluationShaderEntryPoint());
				++shaderIndex;
			}
		}
		// ColorBlend
		{
			auto val = gpcp.colorBlend;
			assertion(val.getAttachmentStatesCount() <= pvr::types::PipelineDefaults::ColorBlend::MaxBlendAttachments);
			//color blend
			_cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			_cb.pNext = NULL;
			_cb.flags = 0;
			_cb.logicOp = nativeVk::ConvertToVk::logicOp(val.getLogicOp());
			_cb.logicOpEnable = val.isLogicOpEnabled();
			{
				_cb.blendConstants[0] = val.getColorBlendConst().x;
				_cb.blendConstants[1] = val.getColorBlendConst().y;
				_cb.blendConstants[2] = val.getColorBlendConst().z;
				_cb.blendConstants[3] = val.getColorBlendConst().w;
			}
			for (pvr::uint32 i = 0; i < val.getAttachmentStatesCount(); i++)
			{
				convert(_vkBlendAttachments[i], val.getAttachmentState(i));
			}
			_cb.pAttachments = &_vkBlendAttachments[0];
			_cb.attachmentCount = (uint32)val.getAttachmentStatesCount();
		}
		// DepthStencil
		if (createInfo.pDepthStencilState)
		{
			auto val = gpcp.depthStencil;
			//depth-stencil
			_ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			_ds.pNext = NULL;
			_ds.flags = 0;
			_ds.depthTestEnable = val.isDepthTestEnable();
			_ds.depthWriteEnable = val.isDepthWriteEnable();
			_ds.depthCompareOp = nativeVk::ConvertToVk::compareMode(val.getDepthComapreOp());
			_ds.depthBoundsTestEnable = val.isDepthBoundTestEnable();
			_ds.stencilTestEnable = val.isStencilTestEnable();
			_ds.minDepthBounds = val.getMinDepth();
			_ds.maxDepthBounds = val.getMaxDepth();

			convert(val.getStencilFront(), _ds.front);
			convert(val.getStencilBack(), _ds.back);
		}
		// Viewport
		{
			uint32 viewportScissorCount = gpcp.viewport.getNumViewportScissor();
			glm::ivec2 renderSurfaceDimensions = gpcp.viewport.getRenderSurfaceDimensions();

			if (renderSurfaceDimensions == pvr::types::PipelineDefaults::ViewportScissor::SurfaceDimensions)
			{
				renderSurfaceDimensions = glm::ivec2(displayAttr.width, displayAttr.height);
			}

			// create a default viewport if one is not provided
			if (viewportScissorCount == 0)
			{
				_viewports[0] = (VkViewport
				{
					0.0f,
					0.0f,
					(float32)renderSurfaceDimensions.x,
					(float32)renderSurfaceDimensions.y,
					0.0f,
					1.0f
				});
				_scissors[0] = (VkRect2D
				{
					VkOffset2D{0, 0},
					VkExtent2D{ (uint32)renderSurfaceDimensions.x, (uint32)renderSurfaceDimensions.y }
				});

				viewportScissorCount = 1;
			}
			else
			{
				for (pvr::uint32 i = 0; i < gpcp.viewport.getNumViewportScissor(); ++i)
				{
					convert(_viewports[i], gpcp.viewport.getViewport(i), renderSurfaceDimensions);
					convert(_scissors[i], gpcp.viewport.getScissor(i), renderSurfaceDimensions);
				}
			}
			//viewport-scissor
			_vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			_vp.pNext = NULL;
			_vp.flags = 0;
			_vp.viewportCount = viewportScissorCount;
			_vp.pViewports = _viewports;
			_vp.scissorCount = viewportScissorCount;
			_vp.pScissors = _scissors;
		}
		// Rasterizer
		{
			auto val = gpcp.rasterizer;
			//rasterization
			_rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			_rs.pNext = NULL;
			_rs.flags = 0;
			_rs.depthClampEnable = !val.isDepthClipEnabled();
			_rs.rasterizerDiscardEnable = val.isRasterizerDiscardEnabled();
			_rs.polygonMode = nativeVk::ConvertToVk::polygonMode(val.getFillMode());
			_rs.cullMode = nativeVk::ConvertToVk::cullMode(val.getCullFace());
			_rs.frontFace = nativeVk::ConvertToVk::frontFaceWinding(val.getFrontFaceWinding());
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
				_ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
				_ms.pNext = NULL;
				_ms.flags = 0;
				_ms.rasterizationSamples = nativeVk::ConvertToVk::aaSamples((uint8)gpcp.multiSample.getNumRasterizationSamples());
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
				_ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
				_ms.pNext = NULL;
				_ms.flags = 0;
				_ms.rasterizationSamples = nativeVk::ConvertToVk::aaSamples((uint8)gpcp.multiSample.getNumRasterizationSamples());
				_ms.sampleShadingEnable = val.isSampleShadingEnabled();
				_ms.minSampleShading = val.getMinSampleShading();
				_ms.pSampleMask = &sampleMask;
				_ms.alphaToCoverageEnable = val.isAlphaToCoverageEnabled();
				_ms.alphaToOneEnable = val.isAlphaToOneEnabled();
				createInfo.pMultisampleState = &_ms;
			}
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
};

/// <summary>Contains everything needed to define a VkComputePipelineCreateInfo, with provision for all memory required
/// </summary>
struct ComputePipelineCreateInfoVulkan
{
	VkComputePipelineCreateInfo createInfo; //<! After construction, will contain the ready-to-use create info

	VkComputePipelineCreateInfo& operator*() { return createInfo; }
	/// <summary>ctor</summary>
	/// <param name="gpcp">A framework compute pipeline create param to be generated from</param>
	/// <param name="context">The Context the pipeline to be created from</param>
	ComputePipelineCreateInfoVulkan(const ComputePipelineCreateParam& cpcp, const GraphicsContext& /*context*/)
	{
		{
			createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			createInfo.pNext = NULL;
			createInfo.flags = 0;
			//Set up the pipeline state
			createInfo.pNext = NULL;
			//createInfo.basePipelineHandle = parent ? pvr::api::native_cast(*parent) : VK_NULL_HANDLE;
			createInfo.basePipelineHandle = VK_NULL_HANDLE;
			createInfo.layout = pvr::api::native_cast(*cpcp.pipelineLayout);
			//  createInfo.basePipelineIndex = -1;
		}

		const pipelineCreation::ComputeShaderStageCreateParam& val = cpcp.computeShader;
		//vertex shader
		createInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		createInfo.stage.pNext = NULL;
		createInfo.stage.flags = 0;
		createInfo.stage.pSpecializationInfo = NULL;
		createInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		createInfo.stage.module = pvr::api::native_cast(*val.getShader());
		createInfo.stage.pName = val.getEntryPoint();

	}
};

}//vulkan
}//api
}//pvr
