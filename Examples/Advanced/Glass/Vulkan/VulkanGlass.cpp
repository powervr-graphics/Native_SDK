/*!*********************************************************************************************************************
\file         VulkanGlass.cpp
\Title        Glass
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Demonstrates dynamic reflection and refraction by rendering two halves of the scene to a single rectangular texture.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVREngineUtils/PVREngineUtils.h"
#include "PVRAssets/Shader.h"
#include <limits.h>

// vertex bindings
const pvr::utils::VertexBindings_Name VertexBindings[] =
{
	{ "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoords" }
};

// Shader uniforms
namespace ShaderUniforms {
enum Enum {	MVPMatrix, MVMatrix, MMatrix, InvVPMatrix, LightDir, EyePos, NumUniforms };
const char* names[] = {"MVPMatrix", "MVMatrix", "MMatrix", "InvVPMatrix", "LightDir", "EyePos"};
}

enum
{
	MaxSwapChain = 4
};

// paraboloid texture size
const pvr::uint32 ParaboloidTexSize = 1024;

// camera constants
const pvr::float32 CamNear = 1.0f;
const pvr::float32 CamFar = 5000.0f;
const pvr::float32 CamFov = glm::pi<pvr::float32>() * 0.41f;

// textures
const char* BalloonTexFile[2] = { "BalloonTex.pvr", "BalloonTex2.pvr" };
const char CubeTexFile[] = "SkyboxTex.pvr";

// model files
const char StatueFile[] = "scene.pod";
const char BalloonFile[] = "Balloon.pod";

// shaders
namespace Shaders {
const std::pair<const char*, pvr::types::ShaderType> Names[] =
{
	{"DefaultVertShader_vk.vsh.spv", pvr::types::ShaderType::VertexShader},
	{"DefaultFragShader_vk.fsh.spv", pvr::types::ShaderType::FragmentShader},
	{"ParaboloidVertShader_vk.vsh.spv", pvr::types::ShaderType::VertexShader},
	{"SkyboxVertShader_vk.vsh.spv", pvr::types::ShaderType::VertexShader},
	{"SkyboxFragShader_vk.fsh.spv", pvr::types::ShaderType::FragmentShader},
	{"EffectReflectVertShader_vk.vsh.spv", pvr::types::ShaderType::VertexShader},
	{"EffectReflectFragShader_vk.fsh.spv", pvr::types::ShaderType::FragmentShader},
	{"EffectRefractVertShader_vk.vsh.spv", pvr::types::ShaderType::VertexShader},
	{"EffectRefractFragShader_vk.fsh.spv", pvr::types::ShaderType::FragmentShader},
	{"EffectChromaticDispersion_vk.vsh.spv", pvr::types::ShaderType::VertexShader},
	{"EffectChromaticDispersion_vk.fsh.spv", pvr::types::ShaderType::FragmentShader},
	{"EffectReflectionRefraction_vk.vsh.spv", pvr::types::ShaderType::VertexShader},
	{"EffectReflectionRefraction_vk.fsh.spv", pvr::types::ShaderType::FragmentShader},
	{"EffectReflectChromDispersion_vk.vsh.spv", pvr::types::ShaderType::VertexShader},
	{"EffectReflectChromDispersion_vk.fsh.spv", pvr::types::ShaderType::FragmentShader},
};

enum Enum
{
	DefaultVS,
	DefaultFS,
	ParaboloidVS,
	SkyboxVS,
	SkyboxFS,
	EffectReflectVS,
	EffectReflectFS,
	EffectRefractionVS,
	EffectRefractionFS,
	EffectChromaticDispersionVS,
	EffectChromaticDispersionFS,
	EffectReflectionRefractionVS,
	EffectReflectionRefractionFS,
	EffectReflectChromDispersionVS,
	EffectReflectChromDispersionFS,
	NumShaders
};
}

// effect mappings
namespace Effects {
enum Enum { ReflectChromDispersion, ReflectRefraction, Reflection, ChromaticDispersion, Refraction, NumEffects };
const char* Names[Effects::NumEffects] =
{
	"Reflection + Chromatic Dispersion", "Reflection + Refraction", "Reflection", "Chromatic Dispersion", "Refraction"
};
}

// clear color for the sky
const glm::vec4 ClearSkyColor(glm::vec4(.6f, 0.8f, 1.0f, 0.0f));

struct Model
{
	pvr::assets::ModelHandle handle;
	std::vector<pvr::api::Buffer> vbos;
	std::vector<pvr::api::Buffer> ibos;
};

static inline pvr::api::Sampler createTrilinearImageSampler(pvr::GraphicsContext& context)
{
	pvr::assets::SamplerCreateParam samplerInfo;
	samplerInfo.wrapModeU = pvr::types::SamplerWrap::Clamp;
	samplerInfo.wrapModeV = pvr::types::SamplerWrap::Clamp;

	samplerInfo.minificationFilter = pvr::types::SamplerFilter::Linear;
	samplerInfo.magnificationFilter = pvr::types::SamplerFilter::Linear;

	samplerInfo.mipMappingFilter = pvr::types::SamplerFilter::Linear;

	return context->createSampler(samplerInfo);
}

// an abstract base for a rendering pass - handles the drawing of different pvr::types of meshes
struct IModelPass
{
private:
	void drawMesh(pvr::api::CommandBufferBase& cmd, const Model& model, pvr::uint32 nodeIndex)
	{
		pvr::int32 meshId = model.handle->getNode(nodeIndex).getObjectId();
		const pvr::assets::Mesh& mesh = model.handle->getMesh(meshId);

		// bind the VBO for the mesh
		cmd->bindVertexBuffer(model.vbos[meshId], 0, 0);
		if (mesh.getFaces().getDataSize() != 0)
		{
			// Indexed Triangle list
			cmd->bindIndexBuffer(model.ibos[meshId], 0, mesh.getFaces().getDataType());
			cmd->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
		}
		else
		{
			// Non-Indexed Triangle list
			cmd->drawArrays(0, mesh.getNumFaces() * 3, 0, 1);
		}
	}

protected:

	void drawMesh(pvr::api::SecondaryCommandBuffer& cmd, const Model& model, pvr::uint32 nodeIndex)
	{
		drawMesh((pvr::api::CommandBufferBase&)cmd, model, nodeIndex);
	}

	void drawMesh(pvr::api::CommandBuffer& cmd, const Model& model, pvr::uint32 nodeIndex)
	{
		drawMesh((pvr::api::CommandBufferBase&)cmd, model, nodeIndex);
	}
};

// skybox pass
struct PassSkyBox
{
	pvr::utils::StructuredMemoryView _bufferMemoryView;
	pvr::api::GraphicsPipeline _pipeline;
	pvr::api::Buffer _vbo;
	pvr::api::DescriptorSetLayout _descriptorSetLayout;
	pvr::Multi<pvr::api::DescriptorSet> _descriptorSets;
	pvr::api::TextureView _skyboxTex;
	pvr::api::Sampler _trilinearSampler;
	pvr::Multi<pvr::api::SecondaryCommandBuffer> secondaryCommandBuffers;

	enum {UboInvViewProj, UboEyePos, UboElementCount};

	void update(pvr::uint32 swapChain, const glm::mat4& invViewProj, const glm::vec3& eyePos)
	{
		_bufferMemoryView.map(swapChain);
		_bufferMemoryView.setValue(UboInvViewProj, invViewProj);
		_bufferMemoryView.setValue(UboEyePos, glm::vec4(eyePos, 0.0f));
		_bufferMemoryView.unmap(swapChain);
	}

	pvr::api::TextureView getSkyBox()
	{
		return _skyboxTex;
	}

	pvr::api::GraphicsPipeline getPipeline() { return _pipeline; }

	bool initDescriptorSetLayout(pvr::GraphicsContext& context)
	{
		// create skybox descriptor set layout
		pvr::api::DescriptorSetLayoutCreateParam descSetLayout;

		// combined image sampler descriptor
		descSetLayout.setBinding(0, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
		// uniform buffer
		descSetLayout.setBinding(1, pvr::types::DescriptorType::UniformBuffer, 1, pvr::types::ShaderStageFlags::Vertex);

		_descriptorSetLayout = context->createDescriptorSetLayout(descSetLayout);

		return true;
	}

	bool initPipeline(pvr::Shell& shell, pvr::GraphicsContext& context, const pvr::api::RenderPass& renderpass)
	{
		pvr::api::GraphicsPipelineCreateParam pipeInfo;

		// on screen renderpass
		pipeInfo.renderPass = renderpass;

		// load, create and set the shaders for rendering the skybox
		auto& vertexShader = Shaders::Names[Shaders::SkyboxVS];
		auto& fragmentShader = Shaders::Names[Shaders::SkyboxFS];
		pvr::Stream::ptr_type vertexShaderSource = shell.getAssetStream(vertexShader.first);
		pvr::Stream::ptr_type fragmentShaderSource = shell.getAssetStream(fragmentShader.first);

		pipeInfo.vertexShader.setShader(context->createShader(*vertexShaderSource, vertexShader.second));
		pipeInfo.fragmentShader.setShader(context->createShader(*fragmentShaderSource, fragmentShader.second));

		// create the pipeline layout
		pvr::api::PipelineLayoutCreateParam pipelineLayout;
		pipelineLayout.setDescSetLayout(0, _descriptorSetLayout);

		pipeInfo.pipelineLayout = context->createPipelineLayout(pipelineLayout);

		// depth stencil state
		pipeInfo.depthStencil.setDepthWrite(false);
		pipeInfo.depthStencil.setDepthTestEnable(false);

		// rasterizer state
		pipeInfo.rasterizer.setCullFace(pvr::types::Face::Front);

		// blend state
		pipeInfo.colorBlend.setAttachmentState(0, pvr::types::BlendingConfig());

		// input assembler
		pipeInfo.inputAssembler.setPrimitiveTopology(pvr::types::PrimitiveTopology::TriangleList);

		// vertex attributes and bindings
		pipeInfo.vertexInput.clear();
		pipeInfo.vertexInput.setInputBinding(0, sizeof(pvr::float32) * 3);
		pipeInfo.vertexInput.addVertexAttribute(0, 0, pvr::assets::VertexAttributeLayout(pvr::types::DataType::Float32, 3, 0), VertexBindings[0].variableName.c_str());

		_pipeline = context->createGraphicsPipeline(pipeInfo);
		if (!_pipeline.isValid())
		{
			pvr::Log("Failed to create the skybox pipeline");
			return false;
		}
		return true;
	}

	void createBuffers(pvr::GraphicsContext& context)
	{
		{
			// create the sky box vbo
			static pvr::float32 quadVertices[] =
			{
				-1,  1, 0.9999f,// upper left
				-1, -1, 0.9999f,// lower left
				1,  1, 0.9999f,// upper right
				1,  1, 0.9999f,// upper right
				-1, -1, 0.9999f,// lower left
				1, -1, 0.9999f// lower right
			};

			_vbo = context->createBuffer(sizeof(quadVertices), pvr::types::BufferBindingUse::VertexBuffer, true);
			_vbo->update(quadVertices, 0, sizeof(quadVertices));
		}

		{
			const std::pair<pvr::StringHash, pvr::types::GpuDatatypes::Enum> uboEntriesStr[PassSkyBox::UboElementCount] =
			{
				{ "InvVPMatrix", pvr::types::GpuDatatypes::mat4x4 },
				{ "EyePos", pvr::types::GpuDatatypes::vec4 }
			};

			// create the structured memory view
			_bufferMemoryView.addEntriesPacked(uboEntriesStr, UboElementCount);
			_bufferMemoryView.finalize(context, 1, pvr::types::BufferBindingUse::UniformBuffer, false, false);
			_bufferMemoryView.createConnectedBuffers(context->getSwapChainLength(), context);
		}
	}

	bool createDescriptorSets(pvr::GraphicsContext& context, pvr::api::Sampler& sampler)
	{
		// create a descriptor set per swapchain
		for (pvr::uint32 i = 0; i < context->getSwapChainLength(); ++i)
		{
			_descriptorSets.add(context->createDescriptorSetOnDefaultPool(_descriptorSetLayout));

			pvr::api::DescriptorSetUpdate descSetUpdate;
			descSetUpdate.setCombinedImageSampler(0, _skyboxTex, sampler);
			descSetUpdate.setUbo(1, _bufferMemoryView.getConnectedBuffer(i));

			if (!_descriptorSets[i]->update(descSetUpdate))
			{
				pvr::Log("Failed to update the skybox descritpor set");
				return false;
			}
		}

		return true;
	}

	bool init(pvr::Shell& shell, pvr::utils::AssetStore& assetsLoader, pvr::GraphicsContext& context, pvr::Multi<pvr::api::Fbo>& fbos, const pvr::api::RenderPass& renderpass)
	{
		_trilinearSampler = createTrilinearImageSampler(context);
		initDescriptorSetLayout(context);
		createBuffers(context);

		// load the  skybox texture
		if (!assetsLoader.getTextureWithCaching(context, CubeTexFile, &_skyboxTex, NULL))
		{
			pvr::Log("Failed to load Skybox texture");
			return false;
		}

		if (!createDescriptorSets(context, _trilinearSampler))
		{
			return false;
		}
		if (!initPipeline(shell, context, renderpass))
		{
			return false;
		}

		recordCommands(context, fbos);

		return true;
	}

	pvr::api::SecondaryCommandBuffer& getSecondaryCommandBuffer(pvr::uint32 swapchain)
	{
		return secondaryCommandBuffers[swapchain];
	}

	void recordCommands(pvr::GraphicsContext& context, pvr::Multi<pvr::api::Fbo>& fbos)
	{
		for (pvr::uint32 i = 0; i < context->getSwapChainLength(); ++i)
		{
			secondaryCommandBuffers[i] = context->createSecondaryCommandBufferOnDefaultPool();

			secondaryCommandBuffers[i]->beginRecording(fbos[i], 0);

			secondaryCommandBuffers[i]->bindPipeline(_pipeline);
			secondaryCommandBuffers[i]->bindVertexBuffer(_vbo, 0, 0);
			secondaryCommandBuffers[i]->bindDescriptorSet(_pipeline->getPipelineLayout(), 0, _descriptorSets[i]);
			secondaryCommandBuffers[i]->drawArrays(0, 6, 0, 1);

			secondaryCommandBuffers[i]->endRecording();
		}
	}
};

// balloon pass
struct PassBalloon : public IModelPass
{
	// variable number of balloons
	enum {NumBalloon = 2};

	// structured memory view with entries for each balloon
	pvr::utils::StructuredMemoryView _bufferMemoryView;

	// descriptor set layout and per swap chain descriptor set
	pvr::api::DescriptorSetLayout _matrixBufferDescriptorSetLayout;
	pvr::Multi<pvr::api::DescriptorSet> _matrixDescriptorSets;

	pvr::api::DescriptorSetLayout _textureBufferDescriptorSetLayout;
	pvr::api::DescriptorSet _textureDescriptorSets[NumBalloon];

	// texture for each balloon
	pvr::api::TextureView _balloonTexures[NumBalloon];
	enum UboElement { UboElementModelViewProj, UboElementLightDir, UboElementEyePos, UboElementCount };
	enum UboBalloonIdElement { UboBalloonId };

	// graphics pipeline used for rendering the balloons
	pvr::api::GraphicsPipeline _pipeline;

	// container for the balloon model
	Model _balloonModel;

	pvr::api::Sampler _trilinearSampler;

	const glm::vec3 EyePos;
	const glm::vec3 LightDir;

	pvr::Multi<pvr::api::SecondaryCommandBuffer> _secondaryCommandBuffers;

	PassBalloon() : EyePos(0.0f, 0.0f, 0.0f), LightDir(19.0f, 22.0f, -50.0f) {}

	bool initDescriptorSetLayout(pvr::GraphicsContext& context)
	{
		{
			pvr::api::DescriptorSetLayoutCreateParam descSetLayout;
			// uniform buffer
			descSetLayout.setBinding(0, pvr::types::DescriptorType::UniformBufferDynamic, 1, pvr::types::ShaderStageFlags::Vertex);
			_matrixBufferDescriptorSetLayout = context->createDescriptorSetLayout(descSetLayout);
		}

		{
			pvr::api::DescriptorSetLayoutCreateParam descSetLayout;
			// combined image sampler descriptor
			descSetLayout.setBinding(0, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
			_textureBufferDescriptorSetLayout = context->createDescriptorSetLayout(descSetLayout);
		}

		return true;
	}

	void createBuffers(pvr::GraphicsContext& context)
	{
		pvr::utils::appendSingleBuffersFromModel(context, *_balloonModel.handle, _balloonModel.vbos, _balloonModel.ibos);

		// ubo entries
		const std::pair<pvr::StringHash, pvr::types::GpuDatatypes::Enum> UboMapping[UboElementCount] =
		{
			{ "UboElementModelViewProj", pvr::types::GpuDatatypes::mat4x4 },
			{ "UboElementLightDir", pvr::types::GpuDatatypes::vec4 },
			{ "UboElementEyePos", pvr::types::GpuDatatypes::vec4 },
		};

		// create the structured memory view
		_bufferMemoryView.addEntriesPacked(UboMapping, UboElementCount);
		_bufferMemoryView.finalize(context, NumBalloon, pvr::types::BufferBindingUse::UniformBuffer, true, false);
		_bufferMemoryView.createConnectedBuffers(context->getSwapChainLength(), context);
	}

	bool createDescriptorSets(pvr::GraphicsContext& context, pvr::api::Sampler& sampler)
	{
		// create a descriptor set per swapchain
		for (pvr::uint32 i = 0; i < context->getSwapChainLength(); ++i)
		{
			_matrixDescriptorSets.add(context->createDescriptorSetOnDefaultPool(_matrixBufferDescriptorSetLayout));

			pvr::api::DescriptorSetUpdate descSetUpdate;
			// set both of the balloon textures at their array indices
			descSetUpdate.setDynamicUbo(0, _bufferMemoryView.getConnectedBuffer(i));

			if (!_matrixDescriptorSets[i]->update(descSetUpdate))
			{
				pvr::Log("Failed to update the matrix descritpor set");
				return false;
			}
		}

		for (pvr::uint32 i = 0; i < NumBalloon; ++i)
		{
			_textureDescriptorSets[i] = context->createDescriptorSetOnDefaultPool(_textureBufferDescriptorSetLayout);

			pvr::api::DescriptorSetUpdate descSetUpdate;
			descSetUpdate.setCombinedImageSampler(0, _balloonTexures[i], sampler);

			if (!_textureDescriptorSets[i]->update(descSetUpdate))
			{
				pvr::Log("Failed to update the texture descritpor set");
				return false;
			}
		}

		return true;
	}

	void setPipeline(pvr::api::GraphicsPipeline& pipeline)
	{
		_pipeline = pipeline;
	}

	bool initPipeline(pvr::Shell& shell, pvr::GraphicsContext& context, const pvr::api::RenderPass& renderpass)
	{
		pvr::api::GraphicsPipelineCreateParam pipeInfo;

		// on screen renderpass
		pipeInfo.renderPass = renderpass;

		// load, create and set the shaders for rendering the skybox
		auto& vertexShader = Shaders::Names[Shaders::DefaultVS];
		auto& fragmentShader = Shaders::Names[Shaders::DefaultFS];
		pvr::Stream::ptr_type vertexShaderSource = shell.getAssetStream(vertexShader.first);
		pvr::Stream::ptr_type fragmentShaderSource = shell.getAssetStream(fragmentShader.first);

		pipeInfo.vertexShader.setShader(context->createShader(*vertexShaderSource, vertexShader.second));
		pipeInfo.fragmentShader.setShader(context->createShader(*fragmentShaderSource, fragmentShader.second));

		// create the pipeline layout
		pvr::api::PipelineLayoutCreateParam pipelineLayout;
		pipelineLayout.setDescSetLayout(0, _matrixBufferDescriptorSetLayout);
		pipelineLayout.setDescSetLayout(1, _textureBufferDescriptorSetLayout);

		pipeInfo.pipelineLayout = context->createPipelineLayout(pipelineLayout);

		// depth stencil state
		pipeInfo.depthStencil.setDepthWrite(true);
		pipeInfo.depthStencil.setDepthTestEnable(true);

		// rasterizer state
		pipeInfo.rasterizer.setCullFace(pvr::types::Face::Back);

		// blend state
		pipeInfo.colorBlend.setAttachmentState(0, pvr::types::BlendingConfig());

		// input assembler
		pipeInfo.inputAssembler.setPrimitiveTopology(pvr::types::PrimitiveTopology::TriangleList);
		pvr::utils::createInputAssemblyFromMesh(_balloonModel.handle->getMesh(0), VertexBindings,
		                                        sizeof(VertexBindings) / sizeof(VertexBindings[0]), pipeInfo);

		_pipeline = context->createGraphicsPipeline(pipeInfo);
		if (!_pipeline.isValid())
		{
			pvr::Log("Failed to create the balloon pipeline");
			return false;
		}
		return true;
	}

	bool init(pvr::Shell& shell, pvr::utils::AssetStore& assetManager, pvr::GraphicsContext& context, const Model& modelBalloon, pvr::Multi<pvr::api::Fbo>& fbos, const pvr::api::RenderPass& renderpass)
	{
		_balloonModel = modelBalloon;

		_trilinearSampler = createTrilinearImageSampler(context);
		initDescriptorSetLayout(context);
		createBuffers(context);

		for (pvr::uint32 i = 0; i < NumBalloon; ++i)
		{
			if (!assetManager.getTextureWithCaching(context, BalloonTexFile[i], &_balloonTexures[i], NULL))
			{
				return false;
			}
		}

		if (!createDescriptorSets(context, _trilinearSampler))
		{
			return false;
		}

		// create the pipeline
		if (!initPipeline(shell, context, renderpass)) { return false; }

		recordCommands(context, fbos);

		return true;
	}

	void recordCommands(pvr::GraphicsContext& context, pvr::Multi<pvr::api::Fbo>& fbos)
	{
		for (pvr::uint32 i = 0; i < context->getSwapChainLength(); ++i)
		{
			_secondaryCommandBuffers[i] = context->createSecondaryCommandBufferOnDefaultPool();

			_secondaryCommandBuffers[i]->beginRecording(fbos[i], 0);
			recordCommandsIntoSecondary(_secondaryCommandBuffers[i], _bufferMemoryView, _matrixDescriptorSets[i], 0);
			_secondaryCommandBuffers[i]->endRecording();
		}
	}

	void recordCommandsIntoSecondary(pvr::api::SecondaryCommandBuffer& cmd, pvr::utils::StructuredBufferView& bufferView, pvr::api::DescriptorSet& matrixDescriptorSet, pvr::uint32 baseOffset)
	{
		cmd->bindPipeline(_pipeline);
		for (pvr::uint32 i = 0; i < NumBalloon; ++i)
		{
			pvr::uint32 offset = bufferView.getAlignedElementArrayOffset(i) + baseOffset;

			cmd->bindDescriptorSet(_pipeline->getPipelineLayout(), 0, matrixDescriptorSet, &offset, 1);
			cmd->bindDescriptorSet(_pipeline->getPipelineLayout(), 1, _textureDescriptorSets[i]);
			drawMesh(cmd, _balloonModel, 0);
		}
	}

	pvr::api::SecondaryCommandBuffer& getSecondaryCommandBuffer(pvr::uint32 swapChain)
	{
		return _secondaryCommandBuffers[swapChain];
	}

	void update(pvr::uint32 swapChain, const glm::mat4 model[NumBalloon], const glm::mat4& view, const glm::mat4& proj)
	{
		_bufferMemoryView.mapMultipleArrayElements(swapChain, 0, NumBalloon);

		for (pvr::uint32 i = 0; i < NumBalloon; ++i)
		{
			const glm::mat4 modelView = view * model[i];
			_bufferMemoryView.setArrayValue(UboElementModelViewProj, i, proj * modelView);
			// Calculate and set the model space light direction
			_bufferMemoryView.setArrayValue(UboElementLightDir, i, glm::normalize(glm::inverse(model[i]) * glm::vec4(LightDir, 1.0f)));
			// Calculate and set the model space eye position
			_bufferMemoryView.setArrayValue(UboElementEyePos, i, glm::inverse(modelView) * glm::vec4(EyePos, 0.0f));
		}

		_bufferMemoryView.unmap(swapChain);
	}
};

// paraboloid pass
struct PassParabloid
{
	enum { ParabolidLeft, ParabolidRight, NumParabloid = 2};
private:
	enum { UboMV, UboLightDir, UboEyePos, UboNear, UboFar, UboCount };
	enum UboBalloonIdElement { UboBalloonId };
	const static std::pair<pvr::StringHash, pvr::types::GpuDatatypes::Enum> UboElementMap[UboCount];

	PassBalloon _passes[NumParabloid];
	pvr::api::GraphicsPipeline _pipelines[2];
	pvr::Multi<pvr::api::Fbo> _fbo;
	pvr::Multi<pvr::api::TextureView> _paraboloidTextures;
	pvr::api::RenderPass _renderPass;
	pvr::api::Sampler _trilinearSampler;
	pvr::api::DescriptorSetLayout _descriptorSetLayout;
	pvr::utils::StructuredMemoryView _bufferMemoryView;
	pvr::Multi<pvr::api::DescriptorSet> _matrixDescriptorSets;
	pvr::api::DescriptorSet _textureDescriptorSets[PassBalloon::NumBalloon];

	pvr::Multi<pvr::api::SecondaryCommandBuffer> _secondaryCommandBuffers;

	bool initPipeline(pvr::Shell& shell, pvr::GraphicsContext& context, const Model& modelBalloon)
	{
		pvr::Rectanglei parabolidViewport[] =
		{
			pvr::Rectanglei(0, 0, ParaboloidTexSize, ParaboloidTexSize),				// first parabolid (Viewport left)
			pvr::Rectanglei(ParaboloidTexSize, 0, ParaboloidTexSize, ParaboloidTexSize) // second paraboloid (Viewport right)
		};

		//create the first pipeline for the left viewport
		pvr::api::GraphicsPipelineCreateParam pipeInfo;

		pipeInfo.renderPass = _renderPass;

		pipeInfo.vertexShader.setShader(context->createShader(*shell.getAssetStream(Shaders::Names[Shaders::ParaboloidVS].first), Shaders::Names[Shaders::ParaboloidVS].second));
		pipeInfo.fragmentShader.setShader(context->createShader(*shell.getAssetStream(Shaders::Names[Shaders::DefaultFS].first), Shaders::Names[Shaders::DefaultFS].second));

		// create the pipeline layout
		pvr::api::PipelineLayoutCreateParam pipelineLayout;
		pipelineLayout.setDescSetLayout(0, _descriptorSetLayout);
		pipelineLayout.setDescSetLayout(1, _passes[0]._textureBufferDescriptorSetLayout);

		pipeInfo.pipelineLayout = context->createPipelineLayout(pipelineLayout);

		// blend state
		pipeInfo.colorBlend.setAttachmentState(0, pvr::types::BlendingConfig());

		// input assembler
		pipeInfo.inputAssembler.setPrimitiveTopology(pvr::types::PrimitiveTopology::TriangleList);

		pvr::utils::createInputAssemblyFromMesh(modelBalloon.handle->getMesh(0), VertexBindings,
		                                        sizeof(VertexBindings) / sizeof(VertexBindings[0]), pipeInfo);

		// depth stencil state
		pipeInfo.depthStencil.setDepthWrite(true);
		pipeInfo.depthStencil.setDepthTestEnable(true);

		// rasterizer state
		pipeInfo.rasterizer.setCullFace(pvr::types::Face::Front);

		// set the viewport to render to the left paraboloid
		pipeInfo.viewport.setViewportAndScissor(0, pvr::api::Viewport(parabolidViewport[0]), parabolidViewport[0], glm::ivec2(ParaboloidTexSize * 2, ParaboloidTexSize));

		// create the left paraboloid graphics pipeline
		_pipelines[0] = context->createGraphicsPipeline(pipeInfo);

		// clear viewport/scissors before resetting them
		pipeInfo.viewport.clear();

		// create the second pipeline for the right viewport
		pipeInfo.viewport.setViewportAndScissor(0, pvr::api::Viewport(parabolidViewport[1]), parabolidViewport[1], glm::ivec2(ParaboloidTexSize * 2, ParaboloidTexSize));
		pipeInfo.rasterizer.setCullFace(pvr::types::Face::Back);

		// create the right paraboloid graphics pipeline
		_pipelines[1] = context->createGraphicsPipeline(pipeInfo);

		// validate paraboloid pipeline creation
		if (!_pipelines[0].isValid() || !_pipelines[1].isValid())
		{
			pvr::Log("Faild to create paraboild pipelines");
			return false;
		}
		return true;
	}

	bool initFbo(pvr::GraphicsContext& context)
	{
		// create the paraboloid subpass
		pvr::api::SubPass subPass(pvr::types::PipelineBindPoint::Graphics);
		// uses a single color attachment
		subPass.setColorAttachment(0, 0);
		// subpass uses depth stencil attachment
		subPass.enableDepthStencilAttachment(true);
		subPass.setDepthStencilAttachment(0);

		pvr::ImageStorageFormat depthStencilFormat(pvr::PixelFormat::Depth16, 1, pvr::types::ColorSpace::lRGB, pvr::VariableType::Float);
		pvr::ImageStorageFormat colorFormat = pvr::ImageStorageFormat(pvr::PixelFormat::RGBA_8888, 1, pvr::types::ColorSpace::lRGB, pvr::VariableType::UnsignedByteNorm);

		//create the renderpass
		// set the final layout to ShaderReadOnlyOptimal so that the image can be bound as a texture in following passes.
		pvr::api::RenderPassCreateParam renderPassInfo;
		// clear the image at the beginning of the renderpass and store it at the end
		// the images initial layout will be color attachment optimal and the final layout will be shader read only optimal
		renderPassInfo.setColorInfo(0, pvr::api::RenderPassColorInfo(colorFormat, pvr::types::LoadOp::Clear, pvr::types::StoreOp::Store, 1,
		                            pvr::types::ImageLayout::ColorAttachmentOptimal, pvr::types::ImageLayout::ShaderReadOnlyOptimal));

		// clear the depth stencil image at the beginning of the renderpass and ignore at the end
		renderPassInfo.setDepthStencilInfo(pvr::api::RenderPassDepthStencilInfo(depthStencilFormat, pvr::types::LoadOp::Clear,
		                                   pvr::types::StoreOp::Ignore, pvr::types::LoadOp::Ignore, pvr::types::StoreOp::Ignore));
		renderPassInfo.setSubPass(0, subPass);

		// create the renderpass to use when rendering into the paraboloid
		_renderPass = context->createRenderPass(renderPassInfo);

		// the paraboloid will be split up into left and right sections when rendering
		const pvr::uint32 fboWidth = ParaboloidTexSize * 2;
		const pvr::uint32 fboHeight = ParaboloidTexSize;

		_fbo.resize(context->getSwapChainLength());

		for (pvr::uint32 i = 0; i < context->getSwapChainLength(); ++i)
		{
			// create the render-target color texture
			pvr::api::TextureStore colorTexture = context->createTexture();
			// allocate the color atatchment and transform to shader read layout so that the layout transformtion
			// works properly durring the command buffer recording.
			colorTexture->allocate2D(colorFormat, fboWidth, fboHeight, pvr::types::ImageUsageFlags::ColorAttachment | pvr::types::ImageUsageFlags::Sampled,
			                         pvr::types::ImageLayout::ShaderReadOnlyOptimal);

			_paraboloidTextures[i] = context->createTextureView(colorTexture);

			// create the render-target depth-stencil texture
			pvr::api::TextureStore depthTexture = context->createTexture();
			// make depth stencil attachment transient as it is only used within this renderpass
			depthTexture->allocateTransient(depthStencilFormat, fboWidth, fboHeight,
			                                pvr::types::ImageUsageFlags::DepthStencilAttachment | pvr::types::ImageUsageFlags::TransientAttachment,
			                                pvr::types::ImageLayout::DepthStencilAttachmentOptimal);

			// create the fbo
			pvr::api::FboCreateParam fboInfo;
			fboInfo.setRenderPass(_renderPass);
			fboInfo.setColor(0, _paraboloidTextures[i]);
			fboInfo.setDepthStencil(context->createTextureView(depthTexture));
			fboInfo.setDimensions(fboWidth, fboHeight);

			_fbo[i] = context->createFbo(fboInfo);
			if (!_fbo[i].isValid())
			{
				pvr::Log("failed to create the paraboloid fbo");
				return false;
			}
		}
		return true;
	}

	void createBuffers(pvr::GraphicsContext& context)
	{
		// create the structured memory view
		_bufferMemoryView.addEntriesPacked(UboElementMap, UboCount);
		_bufferMemoryView.finalize(context, PassBalloon::NumBalloon * NumParabloid, pvr::types::BufferBindingUse::UniformBuffer, true, false);
		_bufferMemoryView.createConnectedBuffers(context->getSwapChainLength(), context);
	}

	bool initDescriptorSetLayout(pvr::GraphicsContext& context)
	{
		// create skybox descriptor set layout
		pvr::api::DescriptorSetLayoutCreateParam descSetLayout;

		// uniform buffer
		descSetLayout.setBinding(0, pvr::types::DescriptorType::UniformBufferDynamic, 1, pvr::types::ShaderStageFlags::Vertex);

		_descriptorSetLayout = context->createDescriptorSetLayout(descSetLayout);

		return true;
	}

	bool createDescriptorSets(pvr::GraphicsContext& context, pvr::api::Sampler& sampler)
	{
		// create a descriptor set per swapchain
		for (pvr::uint32 i = 0; i < context->getSwapChainLength(); ++i)
		{
			_matrixDescriptorSets.add(context->createDescriptorSetOnDefaultPool(_descriptorSetLayout));

			pvr::api::DescriptorSetUpdate descSetUpdate;
			descSetUpdate.setDynamicUbo(0, _bufferMemoryView.getConnectedBuffer(i));
			if (!_matrixDescriptorSets[i]->update(descSetUpdate))
			{
				pvr::Log("Failed to update the paraboloid descritpor set");
				return false;
			}
		}

		return true;
	}

public:
	pvr::api::Fbo& getFbo(pvr::uint32 swapchainIndex)
	{
		return _fbo[swapchainIndex];
	}

	const pvr::api::TextureView& getParaboloid(pvr::uint32 swapchainIndex)
	{
		return _paraboloidTextures[swapchainIndex];
	}

	bool init(pvr::Shell& shell, pvr::utils::AssetStore& assetManager, pvr::GraphicsContext& context, const Model& modelBalloon)
	{
		if (!initFbo(context)) { return false; }

		for (pvr::uint32 i = 0; i < NumParabloid; i++)
		{
			_passes[i].init(shell, assetManager, context, modelBalloon, _fbo, _renderPass);
		}

		_trilinearSampler = createTrilinearImageSampler(context);
		initDescriptorSetLayout(context);
		createBuffers(context);
		if (!createDescriptorSets(context, _trilinearSampler))
		{
			return false;
		}

		// create the pipeline
		if (!initPipeline(shell, context, modelBalloon)) { return false; }

		for (pvr::uint32 i = 0; i < NumParabloid; i++)
		{
			_passes[i].setPipeline(_pipelines[i]);
		}

		recordCommands(context);

		return true;
	}

	void update(pvr::uint32 swapChain, const glm::mat4 balloonModelMatrices[PassBalloon::NumBalloon], const glm::vec3& position)
	{
		//--- Create the first view matrix and make it flip the X coordinate
		glm::mat4 mViewLeft = glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
		mViewLeft = glm::scale(glm::vec3(-1.0f, 1.0f, 1.0f)) * mViewLeft;

		glm::mat4 mViewRight = glm::lookAt(position, position - glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
		glm::mat4 modelView;

		// map the whole of the current swap chain buffer
		_bufferMemoryView.mapMultipleArrayElements(swapChain, 0, PassBalloon::NumBalloon * NumParabloid);

		// [LeftParaboloid_balloon0, LeftParaboloid_balloon1, RightParaboloid_balloon0, RightParaboloid_balloon1]

		for (pvr::uint32 i = 0; i < PassBalloon::NumBalloon; ++i)
		{
			// left paraboloid
			{
				modelView = mViewLeft * balloonModelMatrices[i];
				_bufferMemoryView.setArrayValue(UboMV, i, modelView);
				_bufferMemoryView.setArrayValue(UboLightDir, i, glm::normalize(glm::inverse(balloonModelMatrices[i]) * glm::vec4(_passes[i].LightDir, 1.0f)));

				// Calculate and set the model space eye position
				_bufferMemoryView.setArrayValue(UboEyePos, i, glm::inverse(modelView) * glm::vec4(_passes[i].EyePos, 0.0f));
				_bufferMemoryView.setArrayValue(UboNear, i, CamNear);
				_bufferMemoryView.setArrayValue(UboFar, i, CamFar);
			}
			// right paraboloid
			{
				modelView = mViewRight * balloonModelMatrices[i];
				_bufferMemoryView.setArrayValue(UboMV, NumParabloid + i, modelView);
				_bufferMemoryView.setArrayValue(UboLightDir, NumParabloid + i, glm::normalize(glm::inverse(balloonModelMatrices[i]) * glm::vec4(_passes[i].LightDir, 1.0f)));

				// Calculate and set the model space eye position
				_bufferMemoryView.setArrayValue(UboEyePos, NumParabloid + i, glm::inverse(modelView) * glm::vec4(_passes[i].EyePos, 0.0f));
				_bufferMemoryView.setArrayValue(UboNear, NumParabloid + i, CamNear);
				_bufferMemoryView.setArrayValue(UboFar, NumParabloid + i, CamFar);
			}
		}

		_bufferMemoryView.unmap(swapChain);
	}

	pvr::api::SecondaryCommandBuffer& getSecondaryCommandBuffer(pvr::uint32 swapChain)
	{
		return _secondaryCommandBuffers[swapChain];
	}

	void recordCommands(pvr::GraphicsContext& context)
	{
		for (pvr::uint32 i = 0; i < context->getSwapChainLength(); ++i)
		{
			_secondaryCommandBuffers[i] = context->createSecondaryCommandBufferOnDefaultPool();

			_secondaryCommandBuffers[i]->beginRecording(_fbo[i], 0);

			// left paraboloid
			_passes[ParabolidLeft].recordCommandsIntoSecondary(_secondaryCommandBuffers[i], _bufferMemoryView, _matrixDescriptorSets[i], 0);
			// right paraboloid
			pvr::uint32 baseOffset = _bufferMemoryView.getAlignedElementArrayOffset(2);
			_passes[ParabolidRight].recordCommandsIntoSecondary(_secondaryCommandBuffers[i], _bufferMemoryView, _matrixDescriptorSets[i], baseOffset);

			_secondaryCommandBuffers[i]->endRecording();
		}
	}
};

const std::pair<pvr::StringHash, pvr::types::GpuDatatypes::Enum> PassParabloid::UboElementMap[PassParabloid::UboCount] =
{
	{ "MVMatrix", pvr::types::GpuDatatypes::mat4x4 },
	{ "LightDir", pvr::types::GpuDatatypes::vec4 },
	{ "EyePos", pvr::types::GpuDatatypes::vec4 },
	{ "Near", pvr::types::GpuDatatypes::float32 },
	{ "Far", pvr::types::GpuDatatypes::float32 },
};

struct PassStatue : public IModelPass
{
	pvr::api::GraphicsPipeline _effectPipelines[Effects::NumEffects];

	pvr::utils::StructuredMemoryView _bufferMemoryView;
	pvr::api::DescriptorSetLayout _descriptorSetLayout;
	pvr::Multi<pvr::api::DescriptorSet> _descriptorSets;
	pvr::api::Sampler _trilinearSampler;

	struct Model _modelStatue;

	pvr::Multi<pvr::api::SecondaryCommandBuffer> _secondaryCommandBuffers;

	enum { DescSetUbo, DescSetParabolid, DescSetSkybox,};
	enum UboElements { MVP, Model, EyePos, Count };
	static  const std::pair<pvr::StringHash, pvr::types::GpuDatatypes::Enum> UboElementsNames[UboElements::Count];

	bool initDescriptorSetLayout(pvr::GraphicsContext& context)
	{
		// create skybox descriptor set layout
		pvr::api::DescriptorSetLayoutCreateParam descSetLayout;

		// combined image sampler descriptors
		descSetLayout.setBinding(1, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
		descSetLayout.setBinding(2, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
		// uniform buffer
		descSetLayout.setBinding(0, pvr::types::DescriptorType::UniformBufferDynamic, 1, pvr::types::ShaderStageFlags::Vertex);

		_descriptorSetLayout = context->createDescriptorSetLayout(descSetLayout);

		return true;
	}

	void createBuffers(pvr::GraphicsContext& context)
	{
		// create the vbo & ibos
		pvr::utils::appendSingleBuffersFromModel(context, *this->_modelStatue.handle, this->_modelStatue.vbos, this->_modelStatue.ibos);

		{
			// create the structured memory view
			_bufferMemoryView.addEntriesPacked(UboElementsNames, UboElements::Count);
			_bufferMemoryView.finalize(context, _modelStatue.handle->getNumMeshNodes(), pvr::types::BufferBindingUse::UniformBuffer, true, false);
			_bufferMemoryView.createConnectedBuffers(context->getSwapChainLength(), context);
		}
	}

	bool createDescriptorSets(pvr::GraphicsContext& context, PassParabloid& passParabloid, PassSkyBox& passSkybox, pvr::api::Sampler& sampler)
	{
		// create a descriptor set per swapchain
		for (pvr::uint32 i = 0; i < context->getSwapChainLength(); ++i)
		{
			_descriptorSets.add(context->createDescriptorSetOnDefaultPool(_descriptorSetLayout));

			pvr::api::DescriptorSetUpdate descSetUpdate;
			descSetUpdate.setCombinedImageSampler(1, passParabloid.getParaboloid(i), sampler);
			descSetUpdate.setCombinedImageSampler(2, passSkybox.getSkyBox(), sampler);
			descSetUpdate.setDynamicUbo(0, _bufferMemoryView.getConnectedBuffer(i));

			if (!_descriptorSets[i]->update(descSetUpdate))
			{
				pvr::Log("Failed to update the skybox descritpor set");
				return false;
			}
		}

		return true;
	}

	bool initEffectPipelines(pvr::Shell& shell, pvr::GraphicsContext& context, const pvr::api::RenderPass& renderpass)
	{
		pvr::api::GraphicsPipelineCreateParam pipeInfo;

		// on screen renderpass
		pipeInfo.renderPass = renderpass;

		// create the pipeline layout
		pvr::api::PipelineLayoutCreateParam pipelineLayout;
		pipelineLayout.setDescSetLayout(0, _descriptorSetLayout);

		pipeInfo.pipelineLayout = context->createPipelineLayout(pipelineLayout);

		// depth stencil state
		pipeInfo.depthStencil.setDepthWrite(true);
		pipeInfo.depthStencil.setDepthTestEnable(true);

		// rasterizer state
		pipeInfo.rasterizer.setCullFace(pvr::types::Face::Back);

		// blend state
		pipeInfo.colorBlend.setAttachmentState(0, pvr::types::BlendingConfig());

		// input assembler
		pipeInfo.inputAssembler.setPrimitiveTopology(pvr::types::PrimitiveTopology::TriangleList);

		pvr::utils::createInputAssemblyFromMesh(_modelStatue.handle->getMesh(0), VertexBindings, 2, pipeInfo);

		// load, create and set the shaders for rendering the skybox
		auto& vertexShader = Shaders::Names[Shaders::SkyboxVS];
		auto& fragmentShader = Shaders::Names[Shaders::SkyboxFS];
		pvr::Stream::ptr_type vertexShaderSource = shell.getAssetStream(vertexShader.first);
		pvr::Stream::ptr_type fragmentShaderSource = shell.getAssetStream(fragmentShader.first);

		pipeInfo.vertexShader.setShader(context->createShader(*vertexShaderSource, vertexShader.second));
		pipeInfo.fragmentShader.setShader(context->createShader(*fragmentShaderSource, fragmentShader.second));

		pvr::api::Shader shaders[Shaders::NumShaders];
		for (pvr::uint32 i = 0; i < Shaders::NumShaders; ++i)
		{
			shaders[i] = context->createShader(*shell.getAssetStream(Shaders::Names[i].first), Shaders::Names[i].second);
			if (!shaders[i].isValid())
			{
				pvr::Log("Failed to create the demo effect shaders");
				return false;
			}
		}

		// Effects Vertex and fragment shader
		std::pair<Shaders::Enum, Shaders::Enum> effectShaders[Effects::NumEffects] =
		{
			{ Shaders::EffectReflectChromDispersionVS, Shaders::EffectReflectChromDispersionFS }, // ReflectChromDispersion
			{ Shaders::EffectReflectionRefractionVS, Shaders::EffectReflectionRefractionFS },//ReflectRefraction
			{ Shaders::EffectReflectVS, Shaders::EffectReflectFS },// Reflection
			{ Shaders::EffectChromaticDispersionVS, Shaders::EffectChromaticDispersionFS },// ChromaticDispersion
			{ Shaders::EffectRefractionVS, Shaders::EffectRefractionFS }// Refraction
		};

		for (pvr::uint32 i = 0; i < Effects::NumEffects; ++i)
		{
			pipeInfo.vertexShader.setShader(shaders[effectShaders[i].first]);
			pipeInfo.fragmentShader.setShader(shaders[effectShaders[i].second]);
			_effectPipelines[i] = context->createGraphicsPipeline(pipeInfo);
			if (!_effectPipelines[i].isValid())
			{
				pvr::Log("Failed to create the effects pipelines");
				return false;
			}
		}

		return true;
	}

	bool init(pvr::Shell& shell, pvr::GraphicsContext& context, const struct Model& modelStatue, PassParabloid& passParabloid, PassSkyBox& passSkybox, const pvr::api::RenderPass& renderpass)
	{
		_modelStatue = modelStatue;

		_trilinearSampler = createTrilinearImageSampler(context);
		initDescriptorSetLayout(context);
		createBuffers(context);
		if (!createDescriptorSets(context, passParabloid, passSkybox, _trilinearSampler))
		{
			return false;
		}
		if (!initEffectPipelines(shell, context, renderpass))
		{
			return false;
		}

		return true;
	}

	void recordCommands(pvr::GraphicsContext& context, pvr::uint32 pipeEffect, pvr::api::Fbo& fbo, pvr::uint32 swapChain)
	{
		// create the command buffer if it does not already exist
		if (!_secondaryCommandBuffers[swapChain].isValid())
		{
			_secondaryCommandBuffers[swapChain] = context->createSecondaryCommandBufferOnDefaultPool();
		}

		_secondaryCommandBuffers[swapChain]->beginRecording(fbo, 0);

		_secondaryCommandBuffers[swapChain]->bindPipeline(_effectPipelines[pipeEffect]);
		// bind the texture and samplers and the ubos

		for (pvr::uint32 i = 0; i < _modelStatue.handle->getNumMeshNodes(); i++)
		{
			pvr::uint32 offsets = _bufferMemoryView.getAlignedElementArrayOffset(i);
			_secondaryCommandBuffers[swapChain]->bindDescriptorSet(_effectPipelines[pipeEffect]->getPipelineLayout(), 0, _descriptorSets[swapChain], &offsets, 1);
			drawMesh(_secondaryCommandBuffers[swapChain], _modelStatue, 0);
		}

		_secondaryCommandBuffers[swapChain]->endRecording();
	}

	pvr::api::SecondaryCommandBuffer& getSecondaryCommandBuffer(pvr::uint32 swapChain)
	{
		return _secondaryCommandBuffers[swapChain];
	}

	void update(pvr::uint32 swapChain, const glm::mat4& view, const glm::mat4& proj)
	{
		// The final statue transform brings him with 0.0.0 coordinates at his feet.
		// For this model we want 0.0.0 to be the around the center of the statue, and the statue to be smaller.
		// So, we apply a transformation, AFTER all transforms that have brought him to the center,
		// that will shrink him and move him downwards.
		_bufferMemoryView.mapMultipleArrayElements(swapChain, 0, _modelStatue.handle->getNumMeshNodes());
		static const glm::vec3 scale = glm::vec3(0.25f, 0.25f, 0.25f);
		static const glm::vec3 offset = glm::vec3(0.f, -2.f, 0.f);
		static const glm::mat4 local_transform = glm::translate(offset) * glm::scale(scale);

		for (pvr::uint32 i = 0; i < _modelStatue.handle->getNumMeshNodes(); ++i)
		{
			const glm::mat4& modelMat = local_transform * _modelStatue.handle->getWorldMatrix(i);
			const glm::mat4& modelView = view * modelMat;
			_bufferMemoryView.setArrayValue(UboElements::MVP, i, proj * modelView);
			_bufferMemoryView.setArrayValue(UboElements::Model, i, glm::mat3(modelMat));
			_bufferMemoryView.setArrayValue(UboElements::EyePos, i, glm::inverse(modelView) * glm::vec4(0, 0, 0, 1));
		}
		_bufferMemoryView.unmap(swapChain);
	}
};

const std::pair<pvr::StringHash, pvr::types::GpuDatatypes::Enum> PassStatue::UboElementsNames[PassStatue::UboElements::Count]
{
	{ "MVPMatrix", pvr::types::GpuDatatypes::mat4x4 },
	{ "MMatrix", pvr::types::GpuDatatypes::mat3x3 },
	{ "EyePos", pvr::types::GpuDatatypes::vec4 },
};

/*!*********************************************************************************************************************
 Class implementing the Shell functions.
***********************************************************************************************************************/
class VulkanGlass : public pvr::Shell
{
	struct ApiObjects
	{
		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;

		Model balloon;
		Model statue;

		pvr::Multi<pvr::api::Fbo> fboOnScreen;

		// related sets of drawing commands are grouped into "passes"
		PassSkyBox passSkyBox;
		PassParabloid passParaboloid;
		PassStatue  passStatue;
		PassBalloon passBalloon;

		pvr::Multi<pvr::api::CommandBuffer> sceneCommandBuffers;
		pvr::Multi<pvr::api::SecondaryCommandBuffer> uiSecondaryCommandBuffers;

		pvr::api::Sampler samplerTrilinear;
		pvr::GraphicsContext device;
	};

	std::auto_ptr<ApiObjects> _apiObj;
	pvr::utils::AssetStore _assetManager;

	// Projection, view and model matrices
	glm::mat4 _projectionMatrix;
	glm::mat4 _viewMatrix;

	// Rotation angle for the model
	pvr::float32 _cameraAngle;
	pvr::float32 _balloonAngle[PassBalloon::NumBalloon];
	pvr::int32 _currentEffect;
	pvr::float32 _tilt;
	pvr::float32 _currentTilt;

public:
	VulkanGlass() : _tilt(0), _currentTilt(0) {}
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();
private:
	void eventMappedInput(pvr::SimplifiedInput action);
	void UpdateScene();
	void recordCommands();
};

void VulkanGlass::eventMappedInput(pvr::SimplifiedInput action)
{
	switch (action)
	{
	case pvr::SimplifiedInput::Left:
		_currentEffect -= 1;
		_currentEffect = (_currentEffect + Effects::NumEffects) % Effects::NumEffects;
		_apiObj->uiRenderer.getDefaultDescription()->setText(Effects::Names[_currentEffect]);
		_apiObj->uiRenderer.getDefaultDescription()->commitUpdates();
		_apiObj->device->waitIdle();// make sure the command buffer is finished before re-recording
		recordCommands();
		break;
	case pvr::SimplifiedInput::Up:
		_tilt += 5.f;
		break;
	case pvr::SimplifiedInput::Down:
		_tilt -= 5.f;
		break;
	case pvr::SimplifiedInput::Right:
		_currentEffect += 1;
		_currentEffect = (_currentEffect + Effects::NumEffects) % Effects::NumEffects;
		_apiObj->uiRenderer.getDefaultDescription()->setText(Effects::Names[_currentEffect]);
		_apiObj->uiRenderer.getDefaultDescription()->commitUpdates();
		_apiObj->device->waitIdle();// make sure the command buffer is finished before re-recording
		recordCommands();
		break;
	case pvr::SimplifiedInput::ActionClose: exitShell(); break;
	}
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in initApplication() will be called by PVRShell once perrun, before the rendering context is created.
        Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
        If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result VulkanGlass::initApplication()
{
	_apiObj.reset(new ApiObjects());
	_assetManager.init(*this);

	_cameraAngle =  glm::pi<pvr::float32>() - .6f;

	for (int i = 0; i < PassBalloon::NumBalloon; ++i) { _balloonAngle[i] = glm::pi<pvr::float32>() * i / 5.f;	}

	_currentEffect = 0;

	// load the balloon
	if (!_assetManager.loadModel(BalloonFile, _apiObj->balloon.handle))
	{
		setExitMessage("ERROR: Couldn't load the %s file\n", BalloonFile);
		return pvr::Result::UnknownError;
	}

	// load the statue
	if (!_assetManager.loadModel(StatueFile, _apiObj->statue.handle))
	{
		setExitMessage("ERROR: Couldn't load the %s file", StatueFile);
		return pvr::Result::UnknownError;
	}


	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.If the rendering context
        is lost, quitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result VulkanGlass::quitApplication() { return pvr::Result::Success; }

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in lPVREgl() will be called by PVRShell upon initialization or after a change in the rendering context.
        Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result VulkanGlass::initView()
{
	// get the graphics context
	_apiObj->device = getGraphicsContext();

	// create the onscreen fbo set (per swap chain)
	_apiObj->fboOnScreen = _apiObj->device->createOnScreenFboSet();

	// set up the passes
	if (!_apiObj->passSkyBox.init(*this, _assetManager, _apiObj->device, _apiObj->fboOnScreen, _apiObj->fboOnScreen[0]->getRenderPass()))
	{
		setExitMessage("Failed to initialize the Skybox pass"); return pvr::Result::UnknownError;
	}

	if (!_apiObj->passBalloon.init(*this, _assetManager, _apiObj->device, _apiObj->balloon, _apiObj->fboOnScreen, _apiObj->fboOnScreen[0]->getRenderPass()))
	{
		setExitMessage("Failed to initialize Balloon pass"); return pvr::Result::UnknownError;
	}

	if (!_apiObj->passParaboloid.init(*this, _assetManager, _apiObj->device, _apiObj->balloon))
	{
		setExitMessage("Failed to initialize Paraboloid pass"); return pvr::Result::UnknownError;
	}

	if (!_apiObj->passStatue.init(*this, _apiObj->device, _apiObj->statue, _apiObj->passParaboloid, _apiObj->passSkyBox, _apiObj->fboOnScreen[0]->getRenderPass()))
	{
		setExitMessage("Failed to initialize Statue pass"); return pvr::Result::UnknownError;
	}

	// Initialize UIRenderer
	if (_apiObj->uiRenderer.init(_apiObj->fboOnScreen[0]->getRenderPass(), 0) != pvr::Result::Success)
	{
		this->setExitMessage("ERROR: Cannot initialize UIRenderer\n");
		return pvr::Result::UnknownError;
	}

	_apiObj->uiRenderer.getDefaultTitle()->setText("Glass");
	_apiObj->uiRenderer.getDefaultTitle()->commitUpdates();
	_apiObj->uiRenderer.getDefaultDescription()->setText(Effects::Names[_currentEffect]);
	_apiObj->uiRenderer.getDefaultDescription()->commitUpdates();
	_apiObj->uiRenderer.getDefaultControls()->setText("Left / Right : Change the effect\nUp / Down  : Tilt camera");
	_apiObj->uiRenderer.getDefaultControls()->commitUpdates();
	//Calculate the projection and view matrices
	_projectionMatrix = pvr::math::perspectiveFov(getApiType(), CamFov, (float)this->getWidth(), (float)this->getHeight(),
	                    CamNear, CamFar, (isScreenRotated() ? glm::pi<pvr::float32>() * .5f : 0.0f));
	recordCommands();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result VulkanGlass::releaseView()
{
	_apiObj.reset();
	_assetManager.releaseAll();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result VulkanGlass::renderFrame()
{
	UpdateScene();
	_apiObj->sceneCommandBuffers[getSwapChainIndex()]->submit();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief	Update the scene
***********************************************************************************************************************/
void VulkanGlass::UpdateScene()
{
	// Fetch current time and make sure the previous time isn't greater
	pvr::uint64 timeDifference = getFrameTime();
	// Store the current time for the next frame
	_cameraAngle += timeDifference * 0.00005f;
	for (pvr::int32 i = 0; i < PassBalloon::NumBalloon; ++i)
	{
		_balloonAngle[i] += timeDifference * 0.0002f * (pvr::float32(i) * .5f + 1.f);
	}

	static const glm::vec3 rotateAxis(0.0f, 1.0f, 0.0f);
	pvr::float32 diff = fabs(_tilt - _currentTilt);
	pvr::float32 diff2 = timeDifference / 20.f;
	_currentTilt += glm::sign(_tilt - _currentTilt) * (std::min)(diff, diff2);

	// Rotate the camera
	_viewMatrix = glm::lookAt(glm::vec3(0, -4, -10), glm::vec3(0, _currentTilt - 3, 0), glm::vec3(0, 1, 0))
	              * glm::rotate(_cameraAngle, rotateAxis);

	static glm::mat4 balloonModelMatrices[PassBalloon::NumBalloon];
	for (pvr::int32 i = 0; i < PassBalloon::NumBalloon; ++i)
	{
		// Rotate the balloon model matrices
		balloonModelMatrices[i] = glm::rotate(_balloonAngle[i], rotateAxis) * glm::translate(glm::vec3(120.f + i * 40.f,
		                          sin(_balloonAngle[i] * 3.0f) * 20.0f, 0.0f)) * glm::scale(glm::vec3(3.0f, 3.0f, 3.0f));
	}
	_apiObj->passParaboloid.update(getSwapChainIndex(), balloonModelMatrices, glm::vec3(0, 0, 0));
	_apiObj->passStatue.update(getSwapChainIndex(), _viewMatrix, _projectionMatrix);
	_apiObj->passBalloon.update(getSwapChainIndex(), balloonModelMatrices, _viewMatrix, _projectionMatrix);
	_apiObj->passSkyBox.update(getSwapChainIndex(), glm::inverse(_projectionMatrix * _viewMatrix), glm::vec3(glm::inverse(_viewMatrix)
	                           * glm::vec4(0, 0, 0, 1)));
}

/*!*********************************************************************************************************************
\brief	record all the secondary command buffers
***********************************************************************************************************************/
void VulkanGlass::recordCommands()
{
	for (pvr::uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		if (!_apiObj->sceneCommandBuffers[i].isValid())
		{
			_apiObj->sceneCommandBuffers[i] = _apiObj->device->createCommandBufferOnDefaultPool();
		}

		if (!_apiObj->uiSecondaryCommandBuffers[i].isValid())
		{
			_apiObj->uiSecondaryCommandBuffers[i] = _apiObj->device->createSecondaryCommandBufferOnDefaultPool();
		}

		_apiObj->uiRenderer.beginRendering(_apiObj->uiSecondaryCommandBuffers[i], _apiObj->fboOnScreen[i]);
		_apiObj->uiRenderer.getSdkLogo()->render();
		_apiObj->uiRenderer.getDefaultTitle()->render();
		_apiObj->uiRenderer.getDefaultDescription()->render();
		_apiObj->uiRenderer.getDefaultControls()->render();
		_apiObj->uiRenderer.endRendering();

		// rerecord the statue pass with the current effect
		_apiObj->passStatue.recordCommands(_apiObj->device, _currentEffect, _apiObj->fboOnScreen[i], i);

		_apiObj->sceneCommandBuffers[i]->beginRecording();

		pvr::api::MemoryBarrierSet membarriers;
		// prepare the fbo color attachment for rendering
		// image transition: ShaderReadOnly -> ColorAttachment
		membarriers.addBarrier(pvr::api::ImageAreaBarrier(pvr::types::AccessFlags::ShaderRead, pvr::types::AccessFlags::ColorAttachmentWrite,
		                       _apiObj->passParaboloid.getParaboloid(i)->getResource(), pvr::types::ImageSubresourceRange(), pvr::types::ImageLayout::ShaderReadOnlyOptimal,
		                       pvr::types::ImageLayout::ColorAttachmentOptimal));

		_apiObj->sceneCommandBuffers[i]->pipelineBarrier(pvr::types::PipelineStageFlags::FragmentShader, pvr::types::PipelineStageFlags::FragmentShader, membarriers);

		// Render into the paraboloid
		_apiObj->sceneCommandBuffers[i]->beginRenderPass(_apiObj->passParaboloid.getFbo(i), pvr::Rectanglei(0, 0, 2 * ParaboloidTexSize, ParaboloidTexSize), false, ClearSkyColor);
		_apiObj->sceneCommandBuffers[i]->enqueueSecondaryCmds(_apiObj->passParaboloid.getSecondaryCommandBuffer(i));
		_apiObj->sceneCommandBuffers[i]->endRenderPass();

		// make use of the paraboloid and render the other elements of the scene
		_apiObj->sceneCommandBuffers[i]->beginRenderPass(_apiObj->fboOnScreen[i], pvr::Rectanglei(0, 0, getWidth(), getHeight()), false, ClearSkyColor);
		_apiObj->sceneCommandBuffers[i]->enqueueSecondaryCmds(_apiObj->passSkyBox.getSecondaryCommandBuffer(i));
		_apiObj->sceneCommandBuffers[i]->enqueueSecondaryCmds(_apiObj->passBalloon.getSecondaryCommandBuffer(i));
		_apiObj->sceneCommandBuffers[i]->enqueueSecondaryCmds(_apiObj->passStatue.getSecondaryCommandBuffer(i));
		_apiObj->sceneCommandBuffers[i]->enqueueSecondaryCmds(_apiObj->uiSecondaryCommandBuffers[i]);
		_apiObj->sceneCommandBuffers[i]->endRenderPass();

		_apiObj->sceneCommandBuffers[i]->endRecording();
	}
}

/*!*********************************************************************************************************************
\return	auto ptr of the demo supplied by the user
\brief	This function must be implemented by the user of the shell. The user should return its PVRShell object defining the
        behaviour of the application.
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() {	return std::auto_ptr<pvr::Shell>(new VulkanGlass()); }
