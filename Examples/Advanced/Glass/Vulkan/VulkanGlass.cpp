/*!*********************************************************************************************************************
\file         VulkanGlass.cpp
\Title        Glass
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Demonstrates dynamic reflection and refraction by rendering two halves of the scene to a single rectangular texture.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRUIRenderer/PVRUIRenderer.h"
#include "PVRAssets/Shader.h"
#include <limits.h>
using namespace pvr;
// Vertex attributes
namespace VertexAttrib {
enum Enum {	Position, Normal, TEXCOORD_ARRAY, eNumAttribs	};
const char* names[] = {	"inVertex", "inNormal", "inTexCoords"};
}

// Shader uniforms
namespace ShaderUniforms {
enum Enum {	MVPMatrix, MVMatrix, MMatrix, InvVPMatrix, LightDir, EyePos, NumUniforms };
const char* names[] = {"MVPMatrix", "MVMatrix", "MMatrix", "InvVPMatrix", "LightDir", "EyePos"};
}

enum
{
	NumShaderDefines = 3, MaxSwapChain = 4
};

const float32 CamNear = 1.0f;
const uint32 ParaboloidTexSize = 1024;
const float32 CamFar = 5000.0f;
const float32 CamFov = glm::pi<float32>() * 0.41f;

const char* BalloonTexFile[2]					= { "BalloonTex.pvr", "BalloonTex2.pvr" };

const char CubeTexFile[]						= "SkyboxTex.pvr";

const char StatueFile[]							= "scene.pod";
const char BalloonFile[]						= "Balloon.pod";

namespace Shaders {
const std::pair<const char*, types::ShaderType> Names[] =
{
	{"DefaultVertShader_vk.vsh.spv"             , types::ShaderType::VertexShader},
	{"DefaultFragShader_vk.fsh.spv"             , types::ShaderType::FragmentShader},
	{"ParaboloidVertShader_vk.vsh.spv"          , types::ShaderType::VertexShader},
	{"SkyboxVertShader_vk.vsh.spv"              , types::ShaderType::VertexShader},
	{"SkyboxFragShader_vk.fsh.spv"              , types::ShaderType::FragmentShader},
	{"EffectReflectVertShader_vk.vsh.spv"       , types::ShaderType::VertexShader},
	{"EffectReflectFragShader_vk.fsh.spv"       , types::ShaderType::FragmentShader},
	{"EffectRefractVertShader_vk.vsh.spv"       , types::ShaderType::VertexShader},
	{"EffectRefractFragShader_vk.fsh.spv"       , types::ShaderType::FragmentShader},
	{"EffectChromaticDispersion_vk.vsh.spv"     , types::ShaderType::VertexShader},
	{"EffectChromaticDispersion_vk.fsh.spv"     , types::ShaderType::FragmentShader},
	{"EffectReflectionRefraction_vk.vsh.spv"    , types::ShaderType::VertexShader},
	{"EffectReflectionRefraction_vk.fsh.spv"    , types::ShaderType::FragmentShader},
	{"EffectReflectChromDispersion_vk.vsh.spv"  , types::ShaderType::VertexShader},
	{"EffectReflectChromDispersion_vk.fsh.spv"  , types::ShaderType::FragmentShader},
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

namespace Effects {
enum Enum { ReflectChromDispersion, ReflectRefraction, Reflection, ChromaticDispersion, Refraction, NumEffects };
const char* Names[Effects::NumEffects] =
{
	"Reflection + Chromatic Dispersion", "Reflection + Refraction", "Reflection", "Chromatic Dispersion", "Refraction"
};
}

const glm::vec4 ClearSkyColor(glm::vec4(.6f, 0.8f, 1.0f, 0.0f));

const utils::VertexBindings_Name VertexBindings[] =
{
	{ "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoords" }
};

struct Ubo
{
	utils::StructuredMemoryView buffer;
	api::DescriptorSet sets[MaxSwapChain];
};


struct Model
{
	assets::ModelHandle      handle;
	std::vector<api::Buffer> vbos;
	std::vector<api::Buffer> ibos;
};

struct IPass
{

private:
	void drawMesh(api::CommandBufferBase& cmd, const Model& model, uint32 nodeIndex)
	{
		int32 meshId = model.handle->getNode(nodeIndex).getObjectId();
		const assets::Mesh& mesh = model.handle->getMesh(meshId);

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

	void drawMesh(api::SecondaryCommandBuffer& cmd, const Model& model, uint32 nodeIndex)
	{
		drawMesh((api::CommandBufferBase&)cmd, model, nodeIndex);
	}

	void drawMesh(api::CommandBuffer& cmd, const Model& model, uint32 nodeIndex)
	{
		drawMesh((api::CommandBufferBase&)cmd, model, nodeIndex);
	}

};

struct PassSkyBox
{
	Ubo ubo;
	api::GraphicsPipeline pipeline;
	api::Buffer vbo;
	api::DescriptorSet descSetImageSampler;
	api::TextureView skyboxTex;
	enum { DescSetUbo, DescSetImageSampler };
	enum {UboInvViewProj, UboEyePos, UboElementCount};

	static const std::pair<StringHash, types::GpuDatatypes::Enum> uboEntriesStr[UboElementCount];

	void update(uint32 swapChain, const glm::mat4& invViewProj, const glm::vec3& eyePos)
	{
		ubo.buffer.map(swapChain);
		ubo.buffer.setValue(UboInvViewProj, invViewProj);
		ubo.buffer.setValue(UboEyePos, glm::vec4(eyePos, 0.0f));
		ubo.buffer.unmap(swapChain);
	}

	api::GraphicsPipeline getPipeline() { return pipeline; }

	bool init(api::AssetStore& assetsLoader, GraphicsContext& context, const api::GraphicsPipeline& pipeline,
	          api::Sampler& sampler, uint32 swapChainLength)
	{
		this->pipeline = pipeline;

		// create the sky box vbo

		static float32 quadVertices[] =
		{
			-1,  1, 0.9999f,// upper left
			-1, -1, 0.9999f,// lower left
			1,  1, 0.9999f,// upper right
			1,  1, 0.9999f,// upper right
			-1, -1, 0.9999f,// lower left
			1, -1, 0.9999f// lower right
		};
		vbo =  context->createBuffer(sizeof(quadVertices), types::BufferBindingUse::VertexBuffer, true);
		vbo->update(quadVertices, 0, sizeof(quadVertices));

		// load the  skybox texture

		if (!assetsLoader.getTextureWithCaching(context, CubeTexFile, &skyboxTex, NULL))
		{
			Log("Failed to load Skybox texture");
			return false;
		}

		ubo.buffer.setupArray(context, 1, types::BufferViewTypes::UniformBuffer);
		ubo.buffer.addEntriesPacked(uboEntriesStr, UboElementCount);
		// create the ubo
		for (uint32 i = 0; i < swapChainLength; ++i)
		{
			ubo.buffer.connectWithBuffer(i, context->createBufferAndView(ubo.buffer.getAlignedElementSize(),
			                             types::BufferBindingUse::UniformBuffer, true), types::BufferViewTypes::UniformBuffer);

			ubo.sets[i] = context->createDescriptorSetOnDefaultPool(pipeline->getPipelineLayout()->getDescriptorSetLayout(DescSetUbo));
			if (!ubo.sets[i]->update(api::DescriptorSetUpdate().setUbo(0, ubo.buffer.getConnectedBuffer(i))))
			{
				Log("Failed to update the skybox uniform descritpor"); return false;
			}
		}

		// create textrue sampler descriptor
		descSetImageSampler = context->createDescriptorSetOnDefaultPool(pipeline->getPipelineLayout()->getDescriptorSetLayout(DescSetImageSampler));
		return descSetImageSampler->update(api::DescriptorSetUpdate().setCombinedImageSampler(0, skyboxTex, sampler));
	}

	api::TextureView& getSkyboxTexture() { return skyboxTex; }

	api::DescriptorSet& getSkyboxDescriptor() { return descSetImageSampler; }


	void recordCommands(api::CommandBuffer& cmd, uint32 swapChain)
	{
		cmd->bindPipeline(pipeline);
		cmd->bindVertexBuffer(vbo, 0, 0);
		cmd->bindDescriptorSet(pipeline->getPipelineLayout(), DescSetUbo, ubo.sets[swapChain]);
		cmd->bindDescriptorSet(pipeline->getPipelineLayout(), DescSetImageSampler, descSetImageSampler);
		cmd->drawArrays(0, 6, 0, 1);
	}
};


const std::pair<StringHash, types::GpuDatatypes::Enum> PassSkyBox::uboEntriesStr[PassSkyBox::UboElementCount] =
{
	{"InvVPMatrix", types::GpuDatatypes::mat4x4},
	{"EyePos", types::GpuDatatypes::vec3}
};


struct PassBalloon : public IPass
{
	enum {NumBalloon = 2};
	Ubo ubo[NumBalloon];
	api::DescriptorSet descSetImageSampler[NumBalloon];
	api::TextureView balloonTex[NumBalloon];
	enum UboElement { /*UboElementModelView, */UboElementModelViewProj, UboElementLightDir, UboElementEyePos, UboElementCount };
	const static std::pair<StringHash, types::GpuDatatypes::Enum> UboMapping[UboElementCount];
	enum {DescSetUboId, DescSetTexSamplerId};
	api::GraphicsPipeline pipeline;
	Model balloonModel;

	static const glm::vec4 LightDir;
	static const glm::vec4 EyePos;


	bool init(api::AssetStore& assetManager, GraphicsContext& context, const api::GraphicsPipeline& pipeline,
	          const Model& modelBalloon, api::Sampler sampler, uint32 swapChainLength)
	{
		this->pipeline = pipeline;
		api::DescriptorSetUpdate descUpdate;
		balloonModel = modelBalloon;
		utils::appendSingleBuffersFromModel(context, *balloonModel.handle, balloonModel.vbos, balloonModel.ibos);
		for (uint32 i = 0; i < NumBalloon; ++i)
		{
			if (!assetManager.getTextureWithCaching(context, BalloonTexFile[i], &balloonTex[i], NULL))
			{
				return false;
			}
			descSetImageSampler[i] = context->createDescriptorSetOnDefaultPool(pipeline->getPipelineLayout()->getDescriptorSetLayout(DescSetTexSamplerId));
			descUpdate.setCombinedImageSampler(0, balloonTex[i], sampler);
			descSetImageSampler[i]->update(descUpdate);

			if (!setupUbo(context, pipeline, ubo[i])) { return false; }
		}
		return true;
	}

	void recordCommands(api::CommandBuffer& cmd, api::GraphicsPipeline& pipeline, uint32 swapChain)
	{
		cmd->bindPipeline(pipeline);
		for (uint32 i = 0; i < NumBalloon; ++i)
		{
			cmd->bindDescriptorSet(pipeline->getPipelineLayout(), DescSetUboId, ubo[i].sets[swapChain]);
			cmd->bindDescriptorSet(pipeline->getPipelineLayout(), DescSetTexSamplerId, descSetImageSampler[i]);
			// Now that the uniforms are set, call another function to actually draw the mesh.
			drawMesh(cmd, balloonModel, 0);
		}
	}

	void recordCommands(api::CommandBuffer& cmd, uint32 swapChain) {	recordCommands(cmd, pipeline, swapChain); }

	void update(uint32 swapChain, const glm::mat4 model[NumBalloon], const glm::mat4& view, const glm::mat4& proj)
	{
		for (uint32 i = 0; i < NumBalloon; ++i)
		{
			ubo[i].buffer.map(swapChain);
			const glm::mat4 modelView = view * model[i];
			ubo[i].buffer.setValue(UboElementModelViewProj, proj * modelView);
			// Calculate and set the model space light direction
			ubo[i].buffer.setValue(UboElementLightDir, glm::normalize(glm::inverse(model[i]) * LightDir));
			// Calculate and set the model space eye position
			ubo[i].buffer.setValue(UboElementEyePos, glm::inverse(modelView) * EyePos);
			ubo[i].buffer.unmap(swapChain);
		}
	}
private:
	bool setupUbo(GraphicsContext context, const api::GraphicsPipeline& pipeline, Ubo& ubo)const
	{
		ubo.buffer.setupArray(context, 1, types::BufferViewTypes::UniformBuffer);
		ubo.buffer.addEntriesPacked(UboMapping, UboElementCount);
		for (uint32 i = 0; i < context->getSwapChainLength(); ++i)
		{
			ubo.buffer.connectWithBuffer(i, context->createBufferAndView(ubo.buffer.getAlignedElementSize(),
			                             types::BufferBindingUse::UniformBuffer, true), types::BufferViewTypes::UniformBuffer);

			ubo.sets[i] = context->createDescriptorSetOnDefaultPool(pipeline->getPipelineLayout()->getDescriptorSetLayout(DescSetUboId));
			if (!ubo.sets[i]->update(api::DescriptorSetUpdate().setUbo(0, ubo.buffer.getConnectedBuffer(i))))
			{
				Log("Failed to update pass balloon uniform"); return false;
			}
		}
		return true;
	}
};

const glm::vec4 PassBalloon::EyePos(0.f, 0.0f, 0.0f, 1.0f);
const glm::vec4 PassBalloon::LightDir(19, 22, -50, 0);

const std::pair<StringHash, types::GpuDatatypes::Enum> PassBalloon::UboMapping[UboElementCount] =
{
	{"UboElementModelViewProj", types::GpuDatatypes::mat4x4 },
	{"UboElementLightDir", types::GpuDatatypes::vec3},
	{"UboElementEyePos", types::GpuDatatypes::vec3},
};


struct PassParabloid
{
	enum { ParabolidLeft, ParabolidRight, NumParabloid = 2};
private:
	enum { UboMV, UboLightDir, UboEyePos, UboFar, UboCount };
	const static std::pair<StringHash, types::GpuDatatypes::Enum> UboElementMap[UboCount];

	PassBalloon pass[NumParabloid];
	api::GraphicsPipeline pipeline[2];
	std::vector<api::Fbo> fbo;
	bool initPipeline(IAssetProvider& assetProvider, GraphicsContext& context,
	                  const api::ParentableGraphicsPipeline& parentPipeline)
	{
		Rectanglei parabolidViewport[] =
		{
			Rectanglei(0, 0, ParaboloidTexSize, ParaboloidTexSize),// first parabolid (Viewport left)
			Rectanglei(ParaboloidTexSize, 0, ParaboloidTexSize, ParaboloidTexSize)// second paraboloid (Viewport right)
		};

		//create the first pipeline for the left viewport
		api::GraphicsPipelineCreateParam pipelineInfo = parentPipeline->getCreateParam();
		pipelineInfo.renderPass = fbo[0]->getRenderPass();
		pipelineInfo.vertexShader.setShader(
		  context->createShader(*assetProvider.getAssetStream(Shaders::Names[Shaders::ParaboloidVS].first),
		                        Shaders::Names[Shaders::ParaboloidVS].second));

		pipelineInfo.viewport.setViewportAndScissor(0, api::Viewport(parabolidViewport[0]), parabolidViewport[0]);
		pipelineInfo.rasterizer.setCullFace(types::Face::Front);
		pipeline[0] = context->createGraphicsPipeline(pipelineInfo);

		// create the second pipeline for the right viewport
		pipelineInfo.viewport.setViewportAndScissor(0, api::Viewport(parabolidViewport[1]), parabolidViewport[1]);
		pipelineInfo.rasterizer.setCullFace(types::Face::Back);
		// null out shader because we are going to use the parent pipeline.
		pipeline[1] = context->createGraphicsPipeline(pipelineInfo);

		// validate pipeline creation
		if (!pipeline[0].isValid() || !pipeline[1].isValid())
		{
			Log("Faild to create paraboild pipeline"); return false;
		}
		return true;
	}

	bool initFbo(GraphicsContext& context, uint32 numSwapChains)
	{
		api::SubPass subPass(types::PipelineBindPoint::Graphics);
		subPass.setColorAttachment(0, 0).setDepthStencilAttachment(true); // use the first color attachment
		//--- create paraboloid fbo
		api::ImageStorageFormat rtDsFmt(PixelFormat::Depth16, 1, types::ColorSpace::lRGB, VariableType::UnsignedShort);

		api::ImageStorageFormat rtColorFmt = api::ImageStorageFormat(PixelFormat::RGBA_8888, 1,
		                                     types::ColorSpace::lRGB, VariableType::UnsignedByteNorm);

		//create the renderpass
		// set the final layout to ShaderReadOnlyOptimal so that the image can be bound as a texture in following passes.
		api::RenderPassCreateParam renderPassInfo; renderPassInfo
		.setColorInfo(0, api::RenderPassColorInfo(rtColorFmt, types::LoadOp::Clear,
		              types::StoreOp::Store, 1, types::ImageLayout::ColorAttachmentOptimal,
		              types::ImageLayout::ShaderReadOnlyOptimal))
		.setDepthStencilInfo(api::RenderPassDepthStencilInfo(rtDsFmt, types::LoadOp::Clear))
		.setSubPass(0, subPass);

		const uint32 fboWidth = ParaboloidTexSize * 2;
		const uint32 fboHeight = ParaboloidTexSize;
		fbo.resize(numSwapChains);
		api::RenderPass renderPass = context->createRenderPass(renderPassInfo);
		for (uint32 i = 0; i < numSwapChains; ++i)
		{
			// create the render-target color texture
			api::TextureStore rtColorTex = context->createTexture();
			// allocate the color atatchment and transform to shader read layout so that the layout transformtion
			// works properly durring the command buffer recording.
			rtColorTex->allocate2D(rtColorFmt, fboWidth, fboHeight, types::ImageUsageFlags::ColorAttachment |
			                       types::ImageUsageFlags::Sampled, types::ImageLayout::ShaderReadOnlyOptimal);

			// create the render-target depth-stencil texture
			api::TextureStore rtDsTex = context->createTexture();
			rtDsTex->allocate2D(rtDsFmt, fboWidth, fboHeight, types::ImageUsageFlags::DepthStencilAttachment);

			// create the fbo
			api::FboCreateParam fboInfo;
			fboInfo
			.setRenderPass(renderPass)
			.setColor(0, context->createTextureView(rtColorTex))
			.setDepthStencil(context->createTextureView(rtDsTex))
			.setDimension(fboWidth, fboHeight);

			fbo[i] = context->createFbo(fboInfo);
			if (!fbo[i].isValid())
			{
				Log("failed to create the paraboloid fbo");
				return false;
			}
		}
		return true;
	}

	bool setUpUbo(GraphicsContext& context, PassBalloon& balloon)
	{
		// per each balloon in the pass
		for (uint32 j = 0; j < PassBalloon::NumBalloon; ++j)
		{
			Ubo& ubo = balloon.ubo[j];
			ubo.buffer = utils::StructuredBufferView();
			ubo.buffer.setupArray(context, 1, types::BufferViewTypes::UniformBuffer);
			ubo.buffer.addEntriesPacked(UboElementMap, UboCount);
			for (uint32 i = 0; i < context->getSwapChainLength(); ++i)
			{
				// create the buffer
				ubo.buffer.connectWithBuffer(i, context->createBufferAndView(balloon.ubo[j].buffer.getAlignedElementSize(),
				                             types::BufferBindingUse::UniformBuffer, true), types::BufferViewTypes::UniformBuffer);

				// create the descriptor set
				ubo.sets[i] = context->createDescriptorSetOnDefaultPool(
				                balloon.pipeline->getPipelineLayout()->getDescriptorSetLayout(0));

				if (!ubo.sets[i]->update(api::DescriptorSetUpdate().setUbo(0, ubo.buffer.getConnectedBuffer(i))))
				{
					Log("Failed to update the paraboiloid ubo"); return false;
				}
			}// next swapchain
		}// next baloon
		return true;
	}

public:
	bool init(IAssetProvider& assetProvider, GraphicsContext& context, api::ParentableGraphicsPipeline& pipeline,
	          const PassBalloon& passBallon, uint32 numSwapChains)
	{
		if (!initFbo(context, numSwapChains)) { return false; }

		// create the pipeline
		if (!initPipeline(assetProvider, context, pipeline)) { return false; }

		// set up the passes
		for (uint32 i = 0; i < NumParabloid; ++i)
		{
			pass[i] = passBallon;
			pass[i].pipeline = this->pipeline[i];
			setUpUbo(context, pass[i]);
		}
		return true;
	}

	api::Fbo& getFbo(uint32 swapChain) { return fbo[swapChain]; }

	void update(uint32 swapChain, const glm::mat4 balloonModel[PassBalloon::NumBalloon], const glm::vec3& position)
	{
		//--- Create the first view matrix and make it flip the X coordinate
		glm::mat4 mViewLeft = glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
		mViewLeft = glm::scale(glm::vec3(-1.0f, 1.0f, 1.0f)) * mViewLeft;

		glm::mat4 mViewRight = glm::lookAt(position, position - glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
		glm::mat4 modelView;
		for (uint32 i = 0; i < PassBalloon::NumBalloon; ++i)
		{
			{
				modelView = mViewLeft * balloonModel[i];
				pass[0].ubo[i].buffer.map(swapChain);
				pass[0].ubo[i].buffer.setValue(UboMV, modelView);
				pass[0].ubo[i].buffer.setValue(UboLightDir, glm::normalize(
				                                 glm::inverse(balloonModel[i]) * PassBalloon::LightDir));

				// Calculate and set the model space eye position
				pass[0].ubo[i].buffer.setValue(UboEyePos, glm::inverse(modelView) * PassBalloon::EyePos);
				pass[0].ubo[i].buffer.setValue(UboFar, CamFar);

				pass[0].ubo[i].buffer.unmap(swapChain);
			}
			{
				modelView = mViewRight * balloonModel[i];
				pass[1].ubo[i].buffer.map(swapChain);
				pass[1].ubo[i].buffer.setValue(UboMV, modelView);
				pass[1].ubo[i].buffer.setValue(UboLightDir, glm::normalize(
				                                 glm::inverse(balloonModel[i]) * PassBalloon::LightDir));
				// Calculate and set the model space eye position
				pass[1].ubo[i].buffer.setValue(UboEyePos, glm::inverse(modelView) * PassBalloon::EyePos);
				pass[1].ubo[i].buffer.setValue(UboFar, CamFar);
				pass[1].ubo[i].buffer.unmap(swapChain);
			}
		}
	}

	void recordCommands(api::CommandBuffer& cmd, uint32 swapChain, const Rectanglei& renderArea)
	{
		api::MemoryBarrierSet membarriers;
		// prepare the fbo color attachment for rendering
		membarriers.addBarrier(api::ImageAreaBarrier(types::AccessFlags::ShaderRead,
		                       types::AccessFlags::ColorAttachmentWrite, fbo[swapChain]->getColorAttachment(0)->getResource(),
		                       types::ImageSubresourceRange(), types::ImageLayout::ShaderReadOnlyOptimal,
		                       types::ImageLayout::ColorAttachmentOptimal));

		cmd->pipelineBarrier(types::PipelineStageFlags::AllCommands, types::PipelineStageFlags::AllCommands,
		                     membarriers);

		cmd->beginRenderPass(fbo[swapChain], renderArea, true, ClearSkyColor);
		{
			// left paraboloid
			pass[ParabolidLeft].recordCommands(cmd, swapChain);
			// right paraboloid
			pass[ParabolidRight].recordCommands(cmd, swapChain);
		}
		cmd->endRenderPass();
	}
};

const std::pair<StringHash, types::GpuDatatypes::Enum> PassParabloid::UboElementMap[PassParabloid::UboCount] =
{
	{ "MVMatrix", types::GpuDatatypes::mat4x4},
	{ "LightDir", types::GpuDatatypes::vec3},
	{ "EyePos", types::GpuDatatypes::vec3 },
	{ "Far", types::GpuDatatypes::float32 }
};



struct PassStatue : public IPass
{
	api::GraphicsPipeline pipeline[Effects::NumEffects];
	api::DescriptorSet descParabolid[MaxSwapChain];
	api::DescriptorSet descSkybox;
	Ubo uboBuffer;
	struct Model modelStatue;
	enum { DescSetUbo, DescSetParabolid, DescSetSkybox,};
	enum UboElements { MVP, Model, EyePos, Count };
	static  const std::pair<StringHash, types::GpuDatatypes::Enum> UboElementsNames[UboElements::Count];


	bool init(GraphicsContext& context, api::GraphicsPipeline pipeline[Effects::NumEffects],
	          const struct Model& modelStatue, PassParabloid& passParabloid, api::DescriptorSet& skybox,
	          api::Sampler& sampler, uint32 swapChainLength)
	{
		this->modelStatue = modelStatue;
		// create the vbo & ibos
		utils::appendSingleBuffersFromModel(context, *this->modelStatue.handle, this->modelStatue.vbos, this->modelStatue.ibos);
		descSkybox = skybox;
		const api::PipelineLayout& pipeLayout = pipeline[Effects::Reflection]->getPipelineLayout();
		uboBuffer.buffer.addEntriesPacked(UboElementsNames, UboElements::Count);
		for (uint32 i = 0; i < swapChainLength; ++i)
		{
			// create the ubo descriptor
			uboBuffer.buffer.connectWithBuffer(i, context->createBufferAndView(uboBuffer.buffer.getAlignedElementSize(),
			                                   types::BufferBindingUse::UniformBuffer, true), types::BufferViewTypes::UniformBuffer);

			uboBuffer.sets[i] = context->createDescriptorSetOnDefaultPool(pipeLayout->getDescriptorSetLayout(DescSetUbo));
			uboBuffer.sets[i]->update(api::DescriptorSetUpdate().setUbo(0, uboBuffer.buffer.getConnectedBuffer(i)));

			// create the parabolid decscriptor from the the parabolid fbo
			api::DescriptorSetUpdate descUpdate;
			descParabolid[i] = context->createDescriptorSetOnDefaultPool(pipeLayout->getDescriptorSetLayout(DescSetSkybox));
			descUpdate.setCombinedImageSampler(0, passParabloid.getFbo(i)->getColorAttachment(0), sampler);
			descParabolid[i]->update(descUpdate);
		}
		std::copy(pipeline, &pipeline[Effects::NumEffects], this->pipeline);
		return true;
	}

	void recordCommands(api::CommandBuffer& cmd, uint32 pipeEffect, uint32 swapChain)
	{
		// Use shader program
		cmd->bindPipeline(pipeline[pipeEffect]);

		// bind the texture and samplers
		cmd->bindDescriptorSet(pipeline[pipeEffect]->getPipelineLayout(), DescSetUbo, uboBuffer.sets[swapChain]);
		cmd->bindDescriptorSet(pipeline[pipeEffect]->getPipelineLayout(), DescSetParabolid, descParabolid[swapChain], 0);
		cmd->bindDescriptorSet(pipeline[pipeEffect]->getPipelineLayout(), DescSetSkybox, descSkybox, 0);
		// Now that the uniforms are set, call another function to actually draw the mesh
		drawMesh(cmd, modelStatue, 0);
	}

	void update(uint32 swapChain, const glm::mat4& view, const glm::mat4& proj)
	{
		// The final statue transform brings him with 0.0.0 coordinates at his feet.
		// For this model we want 0.0.0 to be the around the center of the statue, and the statue to be smaller.
		// So, we apply a transformation, AFTER all transforms that have brought him to the center,
		// that will shrink him and move him downwards.
		uboBuffer.buffer.map(swapChain);
		static const glm::vec3 scale = glm::vec3(0.25f, 0.25f, 0.25f);
		static const glm::vec3 offset = glm::vec3(0.f, -2.f, 0.f);
		static const glm::mat4 local_transform = glm::translate(offset) * glm::scale(scale);

		for (uint32 i = 0; i < modelStatue.handle->getNumMeshNodes(); ++i)
		{
			const glm::mat4& modelMat = local_transform * modelStatue.handle->getWorldMatrix(i);
			const glm::mat4& modelView = view * modelMat;
			uboBuffer.buffer.setValue(UboElements::MVP, proj * modelView);
			uboBuffer.buffer.setValue(UboElements::Model, glm::mat4(modelMat));
			uboBuffer.buffer.setValue(UboElements::EyePos, glm::inverse(modelView) * glm::vec4(0, 0, 0, 1));
		}
		uboBuffer.buffer.unmap(swapChain);
	}
};

const std::pair<StringHash, types::GpuDatatypes::Enum> PassStatue::UboElementsNames[PassStatue::UboElements::Count]
{
	{"MVPMatrix", types::GpuDatatypes::mat4x4},
	{"MMatrix", types::GpuDatatypes::mat3x3},
	{"EyePos", types::GpuDatatypes::vec3},
};



/*!*********************************************************************************************************************
 Class implementing the Shell functions.
***********************************************************************************************************************/
class VulkanGlass : public Shell
{
	struct ApiObjects
	{
		// UIRenderer class used to display text
		ui::UIRenderer uiRenderer;
		// 3D Models

		api::Semaphore semaphores[MaxSwapChain];
		api::TextureView texCube;
		api::TextureView texBalloon[2];
		api::Buffer vboSquare;
		Model balloon, statue;
		Multi<api::Fbo> fboOnScreen;
		PassSkyBox passSkyBox;
		PassParabloid passParaboloid;
		PassStatue  passStatue;
		PassBalloon passBalloon;
		// Group shader programs and their uniform locations together

		api::ParentableGraphicsPipeline pipeDefault;
		api::GraphicsPipeline pipeSkyBox, pipeparaboloid[PassParabloid::NumParabloid], pipeEffects[Effects::NumEffects];
		api::CommandBuffer staticCmd[MaxSwapChain];
		api::CommandBuffer dynamicCmd[MaxSwapChain];
		api::Sampler samplerTrilinear;
		api::RenderPass renderPassDynScene;
		GraphicsContext device;
	};

	std::auto_ptr<ApiObjects> apiObj;
	api::AssetStore assetManager;

	// Projection, view and model matrices
	glm::mat4 projMtx, viewMtx;

	// Rotation angle for the model
	float32 cameraAngle, balloonAngle[PassBalloon::NumBalloon];
	int32 currentEffect;
	float32 tilt, currentTilt;

public:
	VulkanGlass() : tilt(0), currentTilt(0) {}
	virtual Result initApplication();
	virtual Result initView();
	virtual Result releaseView();
	virtual Result quitApplication();
	virtual Result renderFrame();
private:
	bool createImageSampler();
	bool createPipelines();
	bool createOnscreenFbo();
	void eventMappedInput(SimplifiedInput action);
	void UpdateScene();
	void recordStaticCommands();
	void recordDynamicCommands();
};

void VulkanGlass::eventMappedInput(SimplifiedInput action)
{
	switch (action)
	{
	case SimplifiedInput::Left:
		currentEffect -= 1;
		currentEffect = (currentEffect + Effects::NumEffects) % Effects::NumEffects;
		apiObj->uiRenderer.getDefaultDescription()->setText(Effects::Names[currentEffect]);
		apiObj->uiRenderer.getDefaultDescription()->commitUpdates();
		apiObj->device->waitIdle();// make sure the command buffer is finished before re-recording
		recordDynamicCommands();
		break;
	case SimplifiedInput::Up:
		tilt += 5.f;
		break;
	case SimplifiedInput::Down:
		tilt -= 5.f;
		break;
	case SimplifiedInput::Right:
		currentEffect += 1;
		currentEffect = (currentEffect + Effects::NumEffects) % Effects::NumEffects;
		apiObj->uiRenderer.getDefaultDescription()->setText(Effects::Names[currentEffect]);
		apiObj->uiRenderer.getDefaultDescription()->commitUpdates();
		apiObj->device->waitIdle();// make sure the command buffer is finished before re-recording
		recordDynamicCommands();
		break;
	case SimplifiedInput::ActionClose: exitShell(); break;
	}
}

/*!*********************************************************************************************************************
\return	Return true if no error occurred
\brief	Loads the textures and samplers required for this training course
***********************************************************************************************************************/
bool VulkanGlass::createImageSampler()
{

	assets::SamplerCreateParam samplerInfo;

	samplerInfo.wrapModeU = types::SamplerWrap::Clamp;
	samplerInfo.wrapModeV = types::SamplerWrap::Clamp;

	// create sampler cube
	samplerInfo.minificationFilter = types::SamplerFilter::Linear;
	samplerInfo.magnificationFilter = types::SamplerFilter::Linear;

	samplerInfo.mipMappingFilter = types::SamplerFilter::Linear;

	// create sampler trilinear
	apiObj->samplerTrilinear = apiObj->device->createSampler(samplerInfo);
	// DrawBalloon Pass
	// draw paraboild pass
	return true;
}

/*!*********************************************************************************************************************
\return	Return true if no error occurred
\brief	Loads and compiles the shaders and links the shader programs required for this training course
***********************************************************************************************************************/
bool VulkanGlass::createPipelines()
{
	api::GraphicsPipelineCreateParam basePipeInfo;
	basePipeInfo.depthStencil.setDepthTestEnable(true).setDepthWrite(true);

	basePipeInfo.rasterizer.setCullFace(pvr::types::Face::Back);

	api::Shader shaders[Shaders::NumShaders];

	// load all the shaders
	for (uint32 i = 0; i < Shaders::NumShaders; ++i)
	{
		shaders[i] = apiObj->device->createShader(*getAssetStream(Shaders::Names[i].first), Shaders::Names[i].second);
		if (!shaders[i].isValid()) { return false; }
	}

	types::BlendingConfig colorBlend;
	// create the single image sampler pipeline layout pipelines

	api::DescriptorSetLayoutCreateParam descsetLayoutInfo;

	// combined image sampler descriptor
	descsetLayoutInfo.setBinding(0, types::DescriptorType::CombinedImageSampler,
	                             1, types::ShaderStageFlags::Fragment);
	api::DescriptorSetLayout descImageSampler = apiObj->device->createDescriptorSetLayout(descsetLayoutInfo);

	api::DescriptorSetLayoutCreateParam uboDescSetLayoutInfo;
	uboDescSetLayoutInfo.setBinding(0, types::DescriptorType::UniformBuffer, 1, types::ShaderStageFlags::Vertex);
	api::DescriptorSetLayout descUbo = apiObj->device->createDescriptorSetLayout(uboDescSetLayoutInfo);

	//---------------------------------
	//load the default pipeline
	{
		api::GraphicsPipelineCreateParam pipeInfo;
		pipeInfo.renderPass = apiObj->fboOnScreen[0]->getRenderPass();
		pipeInfo.vertexShader.setShader(shaders[Shaders::DefaultVS]);
		pipeInfo.fragmentShader.setShader(shaders[Shaders::DefaultFS]);

		pipeInfo.pipelineLayout = apiObj->device->createPipelineLayout(api::PipelineLayoutCreateParam()
		                          .setDescSetLayout(0, descUbo)
		                          .setDescSetLayout(1, descImageSampler));

		pipeInfo.depthStencil.setDepthWrite(true).setDepthTestEnable(true);
		pipeInfo.rasterizer.setCullFace(pvr::types::Face::Back);
		pipeInfo.colorBlend.setAttachmentState(0, colorBlend);
		pipeInfo.inputAssembler.setPrimitiveTopology(types::PrimitiveTopology::TriangleList);
		utils::createInputAssemblyFromMesh(apiObj->balloon.handle->getMesh(0), VertexBindings,
		                                   sizeof(VertexBindings) / sizeof(VertexBindings[0]), pipeInfo);

		apiObj->pipeDefault = apiObj->device->createParentableGraphicsPipeline(pipeInfo);
		if (!apiObj->pipeDefault.isValid())
		{
			setExitMessage("failed to load deafult pipeline"); return false;
		}
	}
	//--------------------------------
	//load the skybox pipeline
	{
		api::GraphicsPipelineCreateParam pipeInfo = apiObj->pipeDefault->getCreateParam();
		pipeInfo.pipelineLayout = apiObj->pipeDefault->getPipelineLayout();
		pipeInfo.inputAssembler.setPrimitiveTopology(types::PrimitiveTopology::TriangleList);
		apiObj->balloon.handle->getMesh(0);
		utils::createInputAssemblyFromMesh(apiObj->balloon.handle->getMesh(0), VertexBindings,
		                                   sizeof(VertexBindings) / sizeof(VertexBindings[0]), pipeInfo);

		pipeInfo.vertexShader.setShader(shaders[Shaders::SkyboxVS]);
		pipeInfo.fragmentShader.setShader(shaders[Shaders::SkyboxFS]);

		pipeInfo.rasterizer.setCullFace(types::Face::Front);
		pipeInfo.renderPass = apiObj->fboOnScreen[0]->getRenderPass();
		pipeInfo.depthStencil.setDepthTestEnable(false).setDepthWrite(false);
		pipeInfo.vertexInput.clear();
		pipeInfo.vertexInput.setInputBinding(0, sizeof(float32) * 3);
		pipeInfo.vertexInput.addVertexAttribute(0, 0, assets::VertexAttributeLayout(types::DataType::Float32, 3, 0),
		                                        VertexBindings[0].variableName.c_str());

		apiObj->pipeSkyBox = apiObj->device->createGraphicsPipeline(pipeInfo, apiObj->pipeDefault);
		if (!apiObj->pipeSkyBox.isValid())
		{
			setExitMessage("failed to load skybox pipeline"); return false;
		}
	}

	// load the effect pipeline, has two image and sampler
	api::PipelineLayoutCreateParam effectPipeLayoutInfo;
	effectPipeLayoutInfo
	.setDescSetLayout(0, descUbo)
	.setDescSetLayout(1, descImageSampler)
	.setDescSetLayout(2, descImageSampler);
	api::GraphicsPipelineCreateParam pipeInfo = apiObj->pipeDefault->getCreateParam();
	pipeInfo.depthStencil = api::pipelineCreation::DepthStencilStateCreateParam().setDepthTestEnable(true);
	pipeInfo.pipelineLayout = apiObj->device->createPipelineLayout(effectPipeLayoutInfo);
	pipeInfo.colorBlend.setAttachmentState(0, types::BlendingConfig());
	utils::createInputAssemblyFromMesh(apiObj->statue.handle->getMesh(0), VertexBindings, 2, pipeInfo);
	pipeInfo.renderPass = apiObj->fboOnScreen[0]->getRenderPass();
	pipeInfo.rasterizer.setCullFace(pvr::types::Face::Back);

	// Effects Vertex and fragment shader
	std::pair<Shaders::Enum, Shaders::Enum> effectShaders[Effects::NumEffects] =
	{
		{Shaders::EffectReflectChromDispersionVS, Shaders::EffectReflectChromDispersionFS}, // ReflectChromDispersion
		{Shaders::EffectReflectionRefractionVS, Shaders::EffectReflectionRefractionFS},//ReflectRefraction
		{Shaders::EffectReflectVS, Shaders::EffectReflectFS},// Reflection
		{Shaders::EffectChromaticDispersionVS, Shaders::EffectChromaticDispersionFS},// ChromaticDispersion
		{Shaders::EffectRefractionVS, Shaders::EffectRefractionFS}// Refraction
	};
	for (uint32 i = 0; i < Effects::NumEffects; ++i)
	{
		pipeInfo.vertexShader.setShader(shaders[effectShaders[i].first]);
		pipeInfo.fragmentShader.setShader(shaders[effectShaders[i].second]);
		apiObj->pipeEffects[i] = apiObj->device->createGraphicsPipeline(pipeInfo);
		if (!apiObj->pipeEffects[i].isValid()) { setExitMessage("Failed to create Relfection pipeline"); return false; }
	}
	return true;
}

/*!*********************************************************************************************************************
\return	Return true if no error occurred
\brief	Creates the required frame buffers and textures to render into.
***********************************************************************************************************************/
bool VulkanGlass::createOnscreenFbo()
{
	api::RenderPassColorInfo colorInfo; colorInfo.loadOpColor = types::LoadOp::Clear;
	api::RenderPassDepthStencilInfo dsInfo;
	dsInfo.loadOpDepth = dsInfo.loadOpStencil = types::LoadOp::Clear;
	colorInfo.format = apiObj->device->getPresentationImageFormat();
	dsInfo.format = apiObj->device->getDepthStencilImageFormat();
	pvr::api::RenderPassCreateParam renderPassDesc;
	renderPassDesc
	.setColorInfo(0, colorInfo)
	.setDepthStencilInfo(dsInfo)
	.setSubPass(0, api::SubPass().setColorAttachment(0, 0));
	apiObj->fboOnScreen = apiObj->device->createOnScreenFboSetWithRenderPass(apiObj->device->createRenderPass(renderPassDesc));

	// create the dynamic scene renderPass
	dsInfo.loadOpDepth = dsInfo.loadOpStencil = types::LoadOp::Load;
	colorInfo.loadOpColor = types::LoadOp::Load;
	renderPassDesc.setColorInfo(0, colorInfo).setDepthStencilInfo(dsInfo);
	apiObj->renderPassDynScene = apiObj->device->createRenderPass(renderPassDesc);
	return true;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in initApplication() will be called by PVRShell once perrun, before the rendering context is created.
        Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
        If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
Result VulkanGlass::initApplication()
{
	assetManager.init(*this);
	cameraAngle =  glm::pi<glm::float32>() - .6f;
	for (int i = 0; i < PassBalloon::NumBalloon; ++i) {	balloonAngle[i] = glm::pi<glm::float32>() * i / 5.f;	}
	currentEffect = 0;
	apiObj.reset(new ApiObjects());
	if (!assetManager.loadModel(BalloonFile, apiObj->balloon.handle))
	{
		setExitMessage("ERROR: Couldn't load the %s file\n", BalloonFile);
		return Result::UnknownError;
	}

	if (!assetManager.loadModel(StatueFile, apiObj->statue.handle))
	{
		setExitMessage("ERROR: Couldn't load the %s file", StatueFile);
		return Result::UnknownError;
	}


	return Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.If the rendering context
        is lost, quitApplication() will not be called.
***********************************************************************************************************************/
Result VulkanGlass::quitApplication() { return Result::Success; }

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in lPVREgl() will be called by PVRShell upon initialization or after a change in the rendering context.
        Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
Result VulkanGlass::initView()
{
	// Load the mask
	if (!assetManager.loadModel(StatueFile, apiObj->statue.handle))
	{
		this->setExitMessage("ERROR: Couldn't load the .pod file\n");
		return Result::NotInitialized;
	}
	apiObj->device = getGraphicsContext();

	if (!createOnscreenFbo()) { return Result::UnknownError; }
	if (!createPipelines()) { return Result::UnknownError; }
	if (!createImageSampler())	{ return Result::UnknownError;	}

	// set up the passes
	if (!apiObj->passSkyBox.init(assetManager, apiObj->device, apiObj->pipeSkyBox, apiObj->samplerTrilinear,
	                             getSwapChainLength()))
	{
		setExitMessage("Failed to init Sky pass"); return Result::UnknownError;
	}

	if (!apiObj->passBalloon.init(assetManager, apiObj->device, apiObj->pipeDefault, apiObj->balloon,
	                              apiObj->samplerTrilinear, getSwapChainLength()))
	{
		setExitMessage("Failed to init Balloon pass"); return Result::UnknownError;
	}

	if (!apiObj->passParaboloid.init(*this, apiObj->device, apiObj->pipeDefault,
	                                 apiObj->passBalloon, getSwapChainLength()))
	{
		setExitMessage("Failed to init Paraboloid pass"); return Result::UnknownError;
	}

	if (!apiObj->passStatue.init(apiObj->device, apiObj->pipeEffects, apiObj->statue,
	                             apiObj->passParaboloid, apiObj->passSkyBox.getSkyboxDescriptor(),
	                             apiObj->samplerTrilinear, getSwapChainLength()))
	{
		setExitMessage("Failed to init Statue pass"); return Result::UnknownError;
	}

	// Initialize UIRenderer
	if (apiObj->uiRenderer.init(apiObj->renderPassDynScene, 0) != Result::Success)
	{
		this->setExitMessage("ERROR: Cannot initialize UIRenderer\n");
		return Result::UnknownError;
	}

	apiObj->uiRenderer.getDefaultTitle()->setText("Glass");
	apiObj->uiRenderer.getDefaultTitle()->commitUpdates();
	apiObj->uiRenderer.getDefaultDescription()->setText(Effects::Names[currentEffect]);
	apiObj->uiRenderer.getDefaultDescription()->commitUpdates();
	apiObj->uiRenderer.getDefaultControls()->setText("Left / Right : Change the effect\nUp / Down  : Tilt camera");
	apiObj->uiRenderer.getDefaultControls()->commitUpdates();
	//Calculate the projection and view matrices
	projMtx = math::perspectiveFov(getApiType(), CamFov, (float)this->getWidth(), (float)this->getHeight(),
	                               CamNear, CamFar, (isScreenRotated() ? glm::pi<float32>() * .5f : 0.0f));
	recordStaticCommands();
	recordDynamicCommands();
	return Result::Success;
}

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
Result VulkanGlass::releaseView()
{
	apiObj.reset();
	assetManager.releaseAll();
	return Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
Result VulkanGlass::renderFrame()
{
	UpdateScene();
	apiObj->staticCmd[getSwapChainIndex()]->submitStartOfFrame(apiObj->semaphores[getSwapChainIndex()]);
	apiObj->dynamicCmd[getSwapChainIndex()]->submitEndOfFrame(apiObj->semaphores[getSwapChainIndex()]);

	return Result::Success;
}

/*!*********************************************************************************************************************
\brief	Update the scene
***********************************************************************************************************************/
void VulkanGlass::UpdateScene()
{
	// Fetch current time and make sure the previous time isn't greater
	uint64 timeDifference = getFrameTime();
	// Store the current time for the next frame
	cameraAngle += timeDifference * 0.00005f;
	for (int32 i = 0; i < PassBalloon::NumBalloon; ++i)
	{
		balloonAngle[i] += timeDifference * 0.0002f * (float32(i) * .5f + 1.f);
	}

	static const glm::vec3 rotateAxis(0.0f, 1.0f, 0.0f);
	float32 diff = fabs(tilt - currentTilt);
	float32 diff2 = timeDifference / 20.f;
	currentTilt += glm::sign(tilt - currentTilt) * (std::min)(diff, diff2);

	// Rotate the camera
	viewMtx = glm::lookAt(glm::vec3(0, -4, -10), glm::vec3(0, currentTilt - 3, 0), glm::vec3(0, 1, 0))
	          * glm::rotate(cameraAngle, rotateAxis);

	static glm::mat4 balloonModel[PassBalloon::NumBalloon];
	for (int32 i = 0; i < PassBalloon::NumBalloon; ++i)
	{
		// Rotate the balloon model matrices
		balloonModel[i] = glm::rotate(balloonAngle[i], rotateAxis) * glm::translate(glm::vec3(120.f + i * 40.f ,
		                  sin(balloonAngle[i] * 3.0f) * 20.0f, 0.0f)) * glm::scale(glm::vec3(3.0f, 3.0f, 3.0f));
	}
	apiObj->passParaboloid.update(getSwapChainIndex(), balloonModel, glm::vec3(0, 0, 0));
	apiObj->passStatue.update(getSwapChainIndex(), viewMtx, projMtx);
	apiObj->passBalloon.update(getSwapChainIndex(), balloonModel, viewMtx, projMtx);
	apiObj->passSkyBox.update(getSwapChainIndex(), glm::inverse(projMtx * viewMtx), glm::vec3(glm::inverse(viewMtx)
	                          * glm::vec4(0, 0, 0, 1)));
}

/*!*********************************************************************************************************************
\brief	record all the secondary command buffers
***********************************************************************************************************************/
void VulkanGlass::recordDynamicCommands()
{
	for (uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		if (!apiObj->dynamicCmd[i] .isValid())
		{
			apiObj->dynamicCmd[i] = apiObj->device->createCommandBufferOnDefaultPool();
		}

		// record the uiCommands
		apiObj->dynamicCmd[i]->beginRecording();
		apiObj->dynamicCmd[i]->beginRenderPass(apiObj->fboOnScreen[i], apiObj->renderPassDynScene, true);
		apiObj->passStatue.recordCommands(apiObj->dynamicCmd[i], currentEffect, i);
		apiObj->uiRenderer.beginRendering(apiObj->dynamicCmd[i]);
		apiObj->uiRenderer.getSdkLogo()->render();
		apiObj->uiRenderer.getDefaultTitle()->render();
		apiObj->uiRenderer.getDefaultDescription()->render();
		apiObj->uiRenderer.getDefaultControls()->render();
		apiObj->uiRenderer.endRendering();
		apiObj->dynamicCmd[i]->endRenderPass();
		apiObj->dynamicCmd[i]->endRecording();
	}
}

/*!*********************************************************************************************************************
\brief	Record all the rendering commands for each frame
***********************************************************************************************************************/
void VulkanGlass::recordStaticCommands()
{
	Rectanglei renderArea(0, 0, 2 * ParaboloidTexSize, ParaboloidTexSize);
	for (uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		// create the semaphores
		apiObj->semaphores[i] = apiObj->device->createSemaphore();

		//--- Record the static commands
		apiObj->staticCmd[i] = apiObj->device->createCommandBufferOnDefaultPool();
		apiObj->staticCmd[i]->beginRecording();

		// draw in to paraboloids
		apiObj->passParaboloid.recordCommands(apiObj->staticCmd[i], i, renderArea);

		// Bind back the original frame buffer and reset the viewport
		apiObj->staticCmd[i]->beginRenderPass(apiObj->fboOnScreen[i], Rectanglei(0, 0, getWidth(), getHeight()),
		                                      true);
		apiObj->staticCmd[i]->clearColorAttachment(apiObj->fboOnScreen[i], ClearSkyColor);
		apiObj->passSkyBox.recordCommands(apiObj->staticCmd[i], i);
		apiObj->passBalloon.recordCommands(apiObj->staticCmd[i], i);
		apiObj->staticCmd[i]->endRenderPass();
		apiObj->staticCmd[i]->endRecording();
	}
}

/*!*********************************************************************************************************************
\return	auto ptr of the demo supplied by the user
\brief	This function must be implemented by the user of the shell. The user should return its PVRShell object defining the
        behaviour of the application.
***********************************************************************************************************************/
std::auto_ptr<Shell> pvr::newDemo() {	return std::auto_ptr<Shell>(new VulkanGlass()); }
