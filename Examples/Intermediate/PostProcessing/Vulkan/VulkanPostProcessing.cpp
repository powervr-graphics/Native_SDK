/*!********************************************************************************************
\File         VulkanPostProcessing.cpp
\Title        Bloom
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Shows how to do a bloom effect
***********************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVREngineUtils/PVREngineUtils.h"

pvr::utils::VertexBindings_Name VertexBindings[] =
{
	{ "POSITION", "inVertex" },
	{ "NORMAL", "inNormal" },
	{ "UV0", "inTexCoord" },
};

enum class Config
{
	MaxSwapChain = 4
};

/**********************************************************************************************
Consts
**********************************************************************************************/
const glm::vec4 LightPosition(-1.5f, 0.0f, 10.0f, 0.0);

/**********************************************************************************************
Content file names
***********************************************************************************************/
const char FragShaderSrcFile[] = "FragShader_vk.fsh.spv";
const char VertShaderSrcFile[] = "VertShader_vk.vsh.spv";
const char PreBloomFragShaderSrcFile[] = "PreBloomFragShader_vk.fsh.spv";
const char PreBloomVertShaderSrcFile[] = "PreBloomVertShader_vk.vsh.spv";
const char PostBloomFragShaderSrcFile[]	= "PostBloomFragShader_vk.fsh.spv";
const char PostBloomVertShaderSrcFile[]	= "PostBloomVertShader_vk.vsh.spv";
const char BlurFragSrcFile[] = "BlurFragShader_vk.fsh.spv";
const char BlurVertSrcFile[] = "BlurVertShader_vk.vsh.spv";

// PVR texture files
const char BaseTexFile[] = "Marble.pvr";
// POD scene files
const char SceneFile[] = "scene.pod";

struct Ubo
{
	pvr::utils::StructuredMemoryView buffer;
	pvr::api::DescriptorSet sets[static_cast<pvr::uint32>(Config::MaxSwapChain)];
};

struct BlurPass
{
	std::pair<pvr::utils::StructuredMemoryView, pvr::api::DescriptorSet> uboPerVert;
	pvr::api::GraphicsPipeline pipeline;
	pvr::api::DescriptorSet texDescSet[static_cast<pvr::uint32>(Config::MaxSwapChain)]; // per swapchain
	pvr::api::Fbo fbo[static_cast<pvr::uint32>(Config::MaxSwapChain)];
};

typedef std::pair<pvr::StringHash, pvr::types::GpuDatatypes::Enum> BufferViewMapping;

struct RenderScenePass
{
	Ubo uboDynamic;
	Ubo uboStatic;

	pvr::api::GraphicsPipeline pipeline;
	pvr::Rectanglei renderArea;
	pvr::api::DescriptorSet texDescriptor;
	static const BufferViewMapping UboDynamicMapping[];
	enum class UboDynamicElements
	{
		MVInv, MVPMatrix, LightDirection
	};

	static const BufferViewMapping UboStaticMapping[];
	enum class UboStaticElements
	{
		Shininess
	};
};

const BufferViewMapping RenderScenePass::UboDynamicMapping[] =
{
	BufferViewMapping("MVInv", pvr::types::GpuDatatypes::mat4x4),
	BufferViewMapping("MVPMatrix", pvr::types::GpuDatatypes::mat4x4),
	BufferViewMapping("LightDirection", pvr::types::GpuDatatypes::vec3),
};

struct PreBloomPass
{
	pvr::api::Fbo fbo[static_cast<pvr::uint32>(Config::MaxSwapChain)];
	pvr::api::GraphicsPipeline pipeline;
	pvr::api::DescriptorSet descTex;
	std::pair<pvr::utils::StructuredMemoryView, pvr::api::DescriptorSet> descIntensity;

	Ubo uboDynamic;
	Ubo uboStatic;
};

struct PostBloomPass
{
	pvr::api::GraphicsPipeline pipeline;
	std::pair<pvr::utils::StructuredMemoryView, pvr::Multi<pvr::api::DescriptorSet>> uboBloomConfig;
	pvr::api::DescriptorSet texDescSet[static_cast<pvr::uint32>(Config::MaxSwapChain)];//per swapchain
};

/*!********************************************************************************************
Class implementing the Shell functions.
***********************************************************************************************/
class VulkanPostProcessing : public pvr::Shell
{
	struct ApiObjects
	{
		// Renderpasses
		PreBloomPass preBloomPass;
		RenderScenePass renderScenePass;
		PostBloomPass postBloomPass;
		BlurPass horizontalBlurPass;
		BlurPass verticalBlurPass;

		pvr::api::FboSet onScreenFbo;

		// Textures
		pvr::api::TextureView baseTex;
		pvr::api::TextureView bloomMapTex;

		// Samplers
		pvr::api::Sampler sceneSamplerClamp;

		// Vbos and Ibos
		std::vector<pvr::api::Buffer> vbos;
		std::vector<pvr::api::Buffer> ibos;

		// Command Buffers
		pvr::api::CommandBuffer mainCmdBloom[static_cast<pvr::uint32>(Config::MaxSwapChain)];
		pvr::api::CommandBuffer mainCmdNoBloom[static_cast<pvr::uint32>(Config::MaxSwapChain)];

		pvr::api::SecondaryCommandBuffer preBloomCommandBuffer[static_cast<pvr::uint32>(Config::MaxSwapChain)];
		pvr::api::SecondaryCommandBuffer noBloomCommandBuffer[static_cast<pvr::uint32>(Config::MaxSwapChain)];
		pvr::api::SecondaryCommandBuffer noBloomUiRendererCommandBuffer[static_cast<pvr::uint32>(Config::MaxSwapChain)];
		pvr::api::SecondaryCommandBuffer bloomUiRendererCommandBuffer[static_cast<pvr::uint32>(Config::MaxSwapChain)];

		pvr::api::SecondaryCommandBuffer horizontalBlurCommandBuffer[static_cast<pvr::uint32>(Config::MaxSwapChain)];
		pvr::api::SecondaryCommandBuffer verticalBlurCommandBuffer[static_cast<pvr::uint32>(Config::MaxSwapChain)];

		pvr::api::SecondaryCommandBuffer postBloomCommandBuffer[static_cast<pvr::uint32>(Config::MaxSwapChain)];

		// descriptor layouts
		pvr::api::DescriptorSetLayout texSamplerLayoutFrag;
		pvr::api::DescriptorSetLayout postBloomTexLayoutFrag;
		pvr::api::DescriptorSetLayout uboLayoutVert;
		pvr::api::DescriptorSetLayout uboLayoutFrag;
		pvr::api::DescriptorSetLayout uboLayoutDynamicVert;

		// 3D Model
		pvr::assets::ModelHandle scene;
		// context
		pvr::GraphicsContext context;
		// UI Renderer
		pvr::ui::UIRenderer uiRenderer;
	};

	std::auto_ptr<ApiObjects> _deviceResources;

	pvr::float32 _bloomIntensity;
	bool _applyBloom;
	bool _drawObject;
	bool _animating;

	pvr::float32 _rotation;
	pvr::utils::AssetStore _assetManager;
	glm::mat4 _worldMatrix;
	glm::mat4 _viewMatrix;
	glm::mat4 _projectionMatrix;

	glm::float32 _blurTexelOffset;
	pvr::uint32 _blurDimension;

public:
	VulkanPostProcessing() : _bloomIntensity(1.f) {}

	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void calculateBlurTexelOffsets();
	bool createDescriptors();
	bool createPipelines();
	void createBuffers();
	void createDescriptorSetLayouts();
	bool createBlurFbo();
	bool createPreBloomFbo();
	void recordBloomCommands(pvr::uint32 swapchain);
	void recordCommandsPreBloom(pvr::uint32 swapchain);
	void recordNoBloomCommands(pvr::uint32 swapchain);
	void recordCommandsBlur(pvr::api::SecondaryCommandBuffer& cmdBuffer, BlurPass& pass, pvr::uint32 swapchain);
	void recordCommandsPostBloom(pvr::uint32 swapchain);
	void recordCommandUIRenderer(pvr::uint32 swapchain);
	void recordCommandsNoBloom(pvr::uint32 swapchain);
	void updateSubtitleText();
	void updatePostBloomConfig(pvr::uint32 swapchain);
	void drawMesh(int i32NodeIndex, pvr::api::SecondaryCommandBuffer& cmdBuffer);
	void eventMappedInput(pvr::SimplifiedInput e);
	void updateAnimation();
	void updateBloomIntensity(pvr::float32 _bloomIntensity);
	void recordCommandBuffers();
	void createCommandBuffers(pvr::uint32 swapchain);
};

void VulkanPostProcessing::calculateBlurTexelOffsets()
{
	// Texel offset for blur filter kernel
	_blurTexelOffset = 1.0f / (pvr::float32)_blurDimension;
	// Altered weights for the faster filter kernel
	pvr::float32 w1 = 0.0555555f;
	pvr::float32 w2 = 0.2777777f;
	pvr::float32 intraTexelOffset = (w1 / (w1 + w2)) * _blurTexelOffset;
	_blurTexelOffset += intraTexelOffset;
}

/*!********************************************************************************************
\return	Return true if no error occurred
\brief	Loads the textures required for this training course
***********************************************************************************************/
bool VulkanPostProcessing::createDescriptors()
{
	// Load Textures
	if (!_assetManager.getTextureWithCaching(getGraphicsContext(), BaseTexFile, &_deviceResources->baseTex, NULL))
	{
		setExitMessage("FAILED to load texture %s.", BaseTexFile);
		return false;
	}

	// sampler clamp
	pvr::assets::SamplerCreateParam samplerDesc;
	samplerDesc.minificationFilter = pvr::types::SamplerFilter::Linear;
	samplerDesc.mipMappingFilter = pvr::types::SamplerFilter::Nearest;
	samplerDesc.magnificationFilter = pvr::types::SamplerFilter::Linear;
	samplerDesc.wrapModeU = pvr::types::SamplerWrap::Clamp;
	samplerDesc.wrapModeV = pvr::types::SamplerWrap::Clamp;
	_deviceResources->sceneSamplerClamp = _deviceResources->context->createSampler(samplerDesc);

	for (pvr::uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		// render pass descriptor set dynamic ubo
		{
			_deviceResources->renderScenePass.uboDynamic.sets[i] = _deviceResources->context->createDescriptorSetOnDefaultPool(_deviceResources->uboLayoutDynamicVert);

			pvr::api::DescriptorSetUpdate descUpdate;
			descUpdate.setDynamicUbo(0, _deviceResources->renderScenePass.uboDynamic.buffer.getConnectedBuffer(i));

			_deviceResources->renderScenePass.uboDynamic.sets[i]->update(descUpdate);
		}

		// pre-bloom pass descriptor set
		{
			_deviceResources->preBloomPass.uboDynamic = _deviceResources->renderScenePass.uboDynamic;
		}

		// horizontal blur descriptor set
		{
			_deviceResources->horizontalBlurPass.texDescSet[i] = _deviceResources->context->createDescriptorSetOnDefaultPool(_deviceResources->texSamplerLayoutFrag);

			pvr::api::DescriptorSetUpdate descUpdate;
			descUpdate.setCombinedImageSampler(0, _deviceResources->preBloomPass.fbo[i]->getColorAttachment(1), _deviceResources->sceneSamplerClamp);

			_deviceResources->horizontalBlurPass.texDescSet[i]->update(descUpdate);
		}

		// vertical blur pass descriptor set
		{
			_deviceResources->verticalBlurPass.texDescSet[i] = _deviceResources->context->createDescriptorSetOnDefaultPool(_deviceResources->texSamplerLayoutFrag);

			pvr::api::DescriptorSetUpdate descUpdate;
			descUpdate.setCombinedImageSampler(0, _deviceResources->horizontalBlurPass.fbo[i]->getColorAttachment(0), _deviceResources->sceneSamplerClamp);
			_deviceResources->verticalBlurPass.texDescSet[i]->update(descUpdate);
		}

		// post bloom descriptor set
		{
			_deviceResources->postBloomPass.texDescSet[i] = _deviceResources->context->createDescriptorSetOnDefaultPool(_deviceResources->postBloomTexLayoutFrag);

			pvr::api::DescriptorSetUpdate descSetUpdate;
			descSetUpdate.setCombinedImageSampler(0, _deviceResources->preBloomPass.fbo[i]->getColorAttachment(0), _deviceResources->sceneSamplerClamp);
			descSetUpdate.setCombinedImageSampler(1, _deviceResources->verticalBlurPass.fbo[i]->getColorAttachment(0), _deviceResources->sceneSamplerClamp);

			//descSetUpdate.setCombinedImageSampler(1, _deviceResources->horizontalBlurPass.fbo[i]->getColorAttachment(0), _deviceResources->sceneSamplerClamp);
			//descSetUpdate.setCombinedImageSampler(1, _deviceResources->preBloomPass.fbo[i]->getColorAttachment(1), _deviceResources->sceneSamplerClamp);

			_deviceResources->postBloomPass.texDescSet[i]->update(descSetUpdate);
		}

		// pre bloom pass
		{
			// create the intensity descriptor
			_deviceResources->preBloomPass.descIntensity.second = _deviceResources->context->createDescriptorSetOnDefaultPool(_deviceResources->uboLayoutFrag);

			pvr::api::DescriptorSetUpdate descSetUpdate;
			descSetUpdate.setUbo(0, _deviceResources->preBloomPass.descIntensity.first.getConnectedBuffer(0));

			_deviceResources->preBloomPass.descIntensity.second->update(descSetUpdate);
		}

		// bloom config
		{
			_deviceResources->postBloomPass.uboBloomConfig.second.add(_deviceResources->context->createDescriptorSetOnDefaultPool(_deviceResources->uboLayoutFrag));

			pvr::api::DescriptorSetUpdate descUpdate;
			descUpdate.setUbo(0, _deviceResources->postBloomPass.uboBloomConfig.first.getConnectedBuffer(i));

			_deviceResources->postBloomPass.uboBloomConfig.second[i]->update(descUpdate);
		}
	}

	// set up the render scene pass static descriptors
	{
		{
			_deviceResources->renderScenePass.uboStatic.sets[0] = _deviceResources->context->createDescriptorSetOnDefaultPool(_deviceResources->uboLayoutVert);

			pvr::api::DescriptorSetUpdate descSetUpdate;
			descSetUpdate.setUbo(0, _deviceResources->renderScenePass.uboStatic.buffer.getConnectedBuffer(0));
			_deviceResources->renderScenePass.uboStatic.sets[0]->update(descSetUpdate);
		}

		{
			_deviceResources->renderScenePass.texDescriptor = _deviceResources->context->createDescriptorSetOnDefaultPool(_deviceResources->texSamplerLayoutFrag);

			pvr::api::DescriptorSetUpdate descSetUpdate;
			descSetUpdate.setCombinedImageSampler(0, _deviceResources->baseTex, _deviceResources->sceneSamplerClamp);

			_deviceResources->renderScenePass.texDescriptor->update(descSetUpdate);
		}

		// copy the texture descriptor from the render scene pass
		_deviceResources->preBloomPass.uboStatic = _deviceResources->renderScenePass.uboStatic;
		_deviceResources->preBloomPass.descTex = _deviceResources->renderScenePass.texDescriptor;
	}

	// blur pass (horizontal)
	{
		_deviceResources->horizontalBlurPass.uboPerVert.second = _deviceResources->context->createDescriptorSetOnDefaultPool(_deviceResources->uboLayoutVert);

		pvr::api::DescriptorSetUpdate descUpdate;
		descUpdate.setUbo(0, _deviceResources->horizontalBlurPass.uboPerVert.first.getConnectedBuffer(0));

		_deviceResources->horizontalBlurPass.uboPerVert.second->update(descUpdate);
	}

	// blur pass1 (vertical)
	{
		_deviceResources->verticalBlurPass.uboPerVert.second = _deviceResources->context->createDescriptorSetOnDefaultPool(_deviceResources->uboLayoutVert);

		pvr::api::DescriptorSetUpdate descUpdate;
		descUpdate.setUbo(0, _deviceResources->verticalBlurPass.uboPerVert.first.getConnectedBuffer(0));

		_deviceResources->verticalBlurPass.uboPerVert.second->update(descUpdate);
	}

	return true;
}

void VulkanPostProcessing::createBuffers()
{
	// dynamic ubos
	{
		_deviceResources->renderScenePass.uboDynamic.buffer.addEntriesPacked(RenderScenePass::UboDynamicMapping,
		    sizeof(RenderScenePass::UboDynamicMapping) / sizeof(RenderScenePass::UboDynamicMapping[0]));

		_deviceResources->renderScenePass.uboDynamic.buffer.finalize(_deviceResources->context,
		    _deviceResources->scene->getNumMeshNodes(), pvr::types::BufferBindingUse::UniformBuffer, true, false);

		_deviceResources->renderScenePass.uboDynamic.buffer.createConnectedBuffers(getSwapChainLength(), _deviceResources->context);
	}

	// static ubos
	{
		_deviceResources->renderScenePass.uboStatic.buffer.addEntryPacked("Shininess", pvr::types::GpuDatatypes::float32);
		_deviceResources->renderScenePass.uboStatic.buffer.finalize(_deviceResources->context, 1, pvr::types::BufferBindingUse::UniformBuffer, false, false);
		_deviceResources->renderScenePass.uboStatic.buffer.createConnectedBuffer(0, _deviceResources->context);

		// update the buffer once
		_deviceResources->renderScenePass.uboStatic.buffer.map(0, pvr::types::MapBufferFlags::Write);
		_deviceResources->renderScenePass.uboStatic.buffer.setValue("Shininess", .6f);
		_deviceResources->renderScenePass.uboStatic.buffer.unmap(0);
	}

	// bloom intensity buffer
	{
		_deviceResources->preBloomPass.descIntensity.first.addEntryPacked("BloomIntensity", pvr::types::GpuDatatypes::float32);
		_deviceResources->preBloomPass.descIntensity.first.finalize(_deviceResources->context, 1, pvr::types::BufferBindingUse::UniformBuffer, false, false);
		_deviceResources->preBloomPass.descIntensity.first.createConnectedBuffer(0, _deviceResources->context);

		// update the initial bloom intensity
		_deviceResources->preBloomPass.descIntensity.first.map(0, pvr::types::MapBufferFlags::Write);
		_deviceResources->preBloomPass.descIntensity.first.setValue("BloomIntensity", 1.f);
		_deviceResources->preBloomPass.descIntensity.first.unmap(0);
	}

	// blur pass (horizontal)
	{
		_deviceResources->horizontalBlurPass.uboPerVert.first.addEntryPacked("TexelOffsetX", pvr::types::GpuDatatypes::float32);
		_deviceResources->horizontalBlurPass.uboPerVert.first.addEntryPacked("TexelOffsetY", pvr::types::GpuDatatypes::float32);
		_deviceResources->horizontalBlurPass.uboPerVert.first.finalize(_deviceResources->context, 1, pvr::types::BufferBindingUse::UniformBuffer, false, false);
		_deviceResources->horizontalBlurPass.uboPerVert.first.createConnectedBuffer(0, _deviceResources->context);

		// set the const values
		_deviceResources->horizontalBlurPass.uboPerVert.first.map(0, pvr::types::MapBufferFlags::Write);
		_deviceResources->horizontalBlurPass.uboPerVert.first.setValue("TexelOffsetX", _blurTexelOffset);
		_deviceResources->horizontalBlurPass.uboPerVert.first.setValue("TexelOffsetY", 0.f);
		_deviceResources->horizontalBlurPass.uboPerVert.first.unmap(0);
	}

	// blur pass (vertical)
	{
		_deviceResources->verticalBlurPass.uboPerVert.first.addEntryPacked("TexelOffsetX", pvr::types::GpuDatatypes::float32);
		_deviceResources->verticalBlurPass.uboPerVert.first.addEntryPacked("TexelOffsetY", pvr::types::GpuDatatypes::float32);
		_deviceResources->verticalBlurPass.uboPerVert.first.finalize(_deviceResources->context, 1, pvr::types::BufferBindingUse::UniformBuffer, false, false);
		_deviceResources->verticalBlurPass.uboPerVert.first.createConnectedBuffer(0, _deviceResources->context);

		// set the const values
		_deviceResources->verticalBlurPass.uboPerVert.first.map(0, pvr::types::MapBufferFlags::Write);
		_deviceResources->verticalBlurPass.uboPerVert.first.setValue("TexelOffsetX", 0);
		_deviceResources->verticalBlurPass.uboPerVert.first.setValue("TexelOffsetY", _blurTexelOffset);
		_deviceResources->verticalBlurPass.uboPerVert.first.unmap(0);
	}

	// post bloom config
	{
		_deviceResources->postBloomPass.uboBloomConfig.first.addEntryPacked("sTexFactor", pvr::types::GpuDatatypes::float32);
		_deviceResources->postBloomPass.uboBloomConfig.first.addEntryPacked("sBlurTexFactor", pvr::types::GpuDatatypes::float32);
		_deviceResources->postBloomPass.uboBloomConfig.first.finalize(_deviceResources->context, 1, pvr::types::BufferBindingUse::UniformBuffer, false, false);
		_deviceResources->postBloomPass.uboBloomConfig.first.createConnectedBuffers(getSwapChainLength(), _deviceResources->context);

		// set the const values
		for (pvr::uint32 i = 0; i < getSwapChainLength(); ++i)
		{
			_deviceResources->postBloomPass.uboBloomConfig.first.map(i, pvr::types::MapBufferFlags::Write);
			_deviceResources->postBloomPass.uboBloomConfig.first.setValue("sTexFactor", 1.0f);
			_deviceResources->postBloomPass.uboBloomConfig.first.setValue("sBlurTexFactor", 1.0f);
			_deviceResources->postBloomPass.uboBloomConfig.first.unmap(i);
		}
	}
}

void VulkanPostProcessing::createDescriptorSetLayouts()
{
	{
		pvr::api::DescriptorSetLayoutCreateParam layoutDesc;
		layoutDesc.setBinding(0, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
		_deviceResources->texSamplerLayoutFrag = _deviceResources->context->createDescriptorSetLayout(layoutDesc);
	}

	{
		pvr::api::DescriptorSetLayoutCreateParam layoutDesc;
		layoutDesc.setBinding(0, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
		layoutDesc.setBinding(1, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
		_deviceResources->postBloomTexLayoutFrag = _deviceResources->context->createDescriptorSetLayout(layoutDesc);
	}

	{
		pvr::api::DescriptorSetLayoutCreateParam layoutDesc;
		layoutDesc.setBinding(0, pvr::types::DescriptorType::UniformBuffer, 1, pvr::types::ShaderStageFlags::Vertex);
		_deviceResources->uboLayoutVert = _deviceResources->context->createDescriptorSetLayout(layoutDesc);
	}

	{
		pvr::api::DescriptorSetLayoutCreateParam layoutDesc;
		layoutDesc.setBinding(0, pvr::types::DescriptorType::UniformBuffer, 1, pvr::types::ShaderStageFlags::Fragment);
		_deviceResources->uboLayoutFrag = _deviceResources->context->createDescriptorSetLayout(layoutDesc);
	}

	{
		pvr::api::DescriptorSetLayoutCreateParam layoutDesc;
		layoutDesc.setBinding(0, pvr::types::DescriptorType::UniformBufferDynamic, 1, pvr::types::ShaderStageFlags::Vertex);
		_deviceResources->uboLayoutDynamicVert = _deviceResources->context->createDescriptorSetLayout(layoutDesc);
	}
}

/*!********************************************************************************************
\brief	Loads and compiles the shaders and links the shader programs
\return	Return true if no error occurred required for this training course
***********************************************************************************************/
bool VulkanPostProcessing::createPipelines()
{
	const pvr::assets::Mesh& mesh = _deviceResources->scene->getMesh(0);

	// create render scene pass pipeline
	{
		pvr::api::GraphicsPipelineCreateParam basicPipeDesc;

		// enable backface culling
		basicPipeDesc.rasterizer.setCullFace(pvr::types::Face::Back);
		// disable blending
		basicPipeDesc.colorBlend.setAttachmentState(0, pvr::types::BlendingConfig());

		// enable depth testing
		basicPipeDesc.depthStencil.setDepthCompareFunc(pvr::types::ComparisonMode::Less);
		basicPipeDesc.depthStencil.setDepthTestEnable(true);
		basicPipeDesc.depthStencil.setDepthWrite(true);

		basicPipeDesc.vertexShader.setShader(_deviceResources->context->createShader(*getAssetStream(VertShaderSrcFile), pvr::types::ShaderType::VertexShader));
		basicPipeDesc.fragmentShader.setShader(_deviceResources->context->createShader(*getAssetStream(FragShaderSrcFile), pvr::types::ShaderType::FragmentShader));

		pvr::utils::createInputAssemblyFromMesh(mesh, VertexBindings, 3, basicPipeDesc);

		// create pipeline layout
		pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(_deviceResources->texSamplerLayoutFrag);
		pipeLayoutInfo.addDescSetLayout(_deviceResources->uboLayoutDynamicVert);
		pipeLayoutInfo.addDescSetLayout(_deviceResources->uboLayoutVert);
		basicPipeDesc.pipelineLayout = _deviceResources->context->createPipelineLayout(pipeLayoutInfo);

		basicPipeDesc.renderPass = _deviceResources->onScreenFbo[0]->getRenderPass();
		basicPipeDesc.subPass = 0;
		_deviceResources->renderScenePass.pipeline = _deviceResources->context->createGraphicsPipeline(basicPipeDesc);

		if (_deviceResources->renderScenePass.pipeline.isValid() == false)
		{
			this->setExitMessage("Failed To Create the RenderScenePass Pipeline");
			return false;
		}
	}

	// create prebloom pass pipeline
	{
		pvr::api::GraphicsPipelineCreateParam prebloomPipeDesc;

		// enable backface culling
		prebloomPipeDesc.rasterizer.setCullFace(pvr::types::Face::Back);

		// enable depth testing
		prebloomPipeDesc.depthStencil.setDepthCompareFunc(pvr::types::ComparisonMode::Less);
		prebloomPipeDesc.depthStencil.setDepthTestEnable(true);
		prebloomPipeDesc.depthStencil.setDepthWrite(true);

		prebloomPipeDesc.vertexShader.setShader(_deviceResources->context->createShader(*getAssetStream(PreBloomVertShaderSrcFile), pvr::types::ShaderType::VertexShader));
		prebloomPipeDesc.fragmentShader.setShader(_deviceResources->context->createShader(*getAssetStream(PreBloomFragShaderSrcFile), pvr::types::ShaderType::FragmentShader));

		pvr::utils::createInputAssemblyFromMesh(mesh, VertexBindings, 3, prebloomPipeDesc);

		// set blending states - disable blending
		prebloomPipeDesc.colorBlend.setAttachmentState(0, pvr::types::BlendingConfig());
		prebloomPipeDesc.colorBlend.setAttachmentState(1, pvr::types::BlendingConfig());

		// create pipeline layout
		pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(_deviceResources->texSamplerLayoutFrag);
		pipeLayoutInfo.addDescSetLayout(_deviceResources->uboLayoutFrag);
		pipeLayoutInfo.addDescSetLayout(_deviceResources->uboLayoutDynamicVert);
		pipeLayoutInfo.addDescSetLayout(_deviceResources->uboLayoutVert);

		prebloomPipeDesc.pipelineLayout = _deviceResources->context->createPipelineLayout(pipeLayoutInfo);

		prebloomPipeDesc.renderPass = _deviceResources->preBloomPass.fbo[0]->getRenderPass();
		prebloomPipeDesc.subPass = 0;

		_deviceResources->preBloomPass.pipeline = _deviceResources->context->createGraphicsPipeline(prebloomPipeDesc);
		if (_deviceResources->preBloomPass.pipeline.isValid() == false)
		{
			this->setExitMessage("Failed to Create preBloom pipeline");
			return false;
		}
	}

	// create Post-Bloom Pipeline
	{
		pvr::api::GraphicsPipelineCreateParam postbloomPipeDesc;

		// enable back face culling
		postbloomPipeDesc.rasterizer.setCullFace(pvr::types::Face::Back);

		// set counter clockwise winding order for front faces
		postbloomPipeDesc.rasterizer.setFrontFaceWinding(pvr::types::PolygonWindingOrder::FrontFaceCCW);
		postbloomPipeDesc.colorBlend.setAttachmentState(0, pvr::types::BlendingConfig());

		postbloomPipeDesc.depthStencil.setDepthTestEnable(false);
		postbloomPipeDesc.depthStencil.setDepthWrite(false);

		postbloomPipeDesc.depthStencil.setStencilTest(false);

		postbloomPipeDesc.vertexShader.setShader(_deviceResources->context->createShader(*getAssetStream(PostBloomVertShaderSrcFile), pvr::types::ShaderType::VertexShader));
		postbloomPipeDesc.fragmentShader.setShader(_deviceResources->context->createShader(*getAssetStream(PostBloomFragShaderSrcFile), pvr::types::ShaderType::FragmentShader));

		postbloomPipeDesc.renderPass = _deviceResources->onScreenFbo[0]->getRenderPass();
		postbloomPipeDesc.subPass = 0;

		// setup vertex inputs
		postbloomPipeDesc.vertexInput.clear();
		postbloomPipeDesc.inputAssembler.setPrimitiveTopology(pvr::types::PrimitiveTopology::TriangleStrip);

		// create pipeline layout
		pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
		pipeLayoutInfo.setDescSetLayout(0, _deviceResources->postBloomTexLayoutFrag);
		pipeLayoutInfo.setDescSetLayout(1, _deviceResources->uboLayoutFrag);

		postbloomPipeDesc.pipelineLayout = _deviceResources->context->createPipelineLayout(pipeLayoutInfo);

		_deviceResources->postBloomPass.pipeline = _deviceResources->context->createGraphicsPipeline(postbloomPipeDesc);

		if (_deviceResources->postBloomPass.pipeline.isValid() == false)
		{
			this->setExitMessage("Failed to Create postBloom pipeline");
			return false;
		}
	}

	//   Blur Pipeline
	{
		pvr::api::GraphicsPipelineCreateParam blurPipeDesc;

		// enable back face culling
		blurPipeDesc.rasterizer.setCullFace(pvr::types::Face::Back);

		// set counter clockwise winding order for front faces
		blurPipeDesc.rasterizer.setFrontFaceWinding(pvr::types::PolygonWindingOrder::FrontFaceCCW);

		// set blending states - disable blending
		blurPipeDesc.colorBlend.setAttachmentState(0, pvr::types::BlendingConfig());

		blurPipeDesc.depthStencil.setDepthTestEnable(false);
		blurPipeDesc.depthStencil.setDepthWrite(false);
		blurPipeDesc.depthStencil.setStencilTest(false);

		blurPipeDesc.vertexShader.setShader(_deviceResources->context->createShader(*getAssetStream(BlurVertSrcFile), pvr::types::ShaderType::VertexShader));
		blurPipeDesc.fragmentShader.setShader(_deviceResources->context->createShader(*getAssetStream(BlurFragSrcFile), pvr::types::ShaderType::FragmentShader));

		// setup vertex inputs
		blurPipeDesc.vertexInput.clear();
		blurPipeDesc.inputAssembler.setPrimitiveTopology(pvr::types::PrimitiveTopology::TriangleStrip);

		pvr::Rectanglei region(0, 0, _deviceResources->horizontalBlurPass.fbo[0]->getDimensions().x,
		                       _deviceResources->horizontalBlurPass.fbo[0]->getDimensions().y);
		blurPipeDesc.viewport.setViewportAndScissor(0, pvr::api::Viewport(region), region, _deviceResources->horizontalBlurPass.fbo[0]->getDimensions());

		// create pipeline layout
		pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(_deviceResources->texSamplerLayoutFrag);
		pipeLayoutInfo.addDescSetLayout(_deviceResources->uboLayoutVert);
		blurPipeDesc.pipelineLayout = _deviceResources->context->createPipelineLayout(pipeLayoutInfo);

		blurPipeDesc.renderPass = _deviceResources->horizontalBlurPass.fbo[0]->getRenderPass();
		blurPipeDesc.subPass = 0;

		_deviceResources->horizontalBlurPass.pipeline = _deviceResources->verticalBlurPass.pipeline =
		      _deviceResources->context->createGraphicsPipeline(blurPipeDesc);

		if (_deviceResources->horizontalBlurPass.pipeline.isValid() == false)
		{
			this->setExitMessage("Failed to Create Blur pipeline");
			return false;
		}
	}
	return true;
}

/*!********************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in initApplication() will be called by Shell once per run, before the rendering
		context is created.
		Used to initialize variables that are not dependent on it (e.g. external modules,
		loading meshes, etc.)
		If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************/
pvr::Result VulkanPostProcessing::initApplication()
{
	this->setStencilBitsPerPixel(0);

	// Apply bloom per default
	_applyBloom = true;
	_drawObject = true;
	_animating = true;

	_rotation = 0.0f;

	_assetManager.init(*this);
	_deviceResources.reset(new ApiObjects());

	// Load the scene
	if (!_assetManager.loadModel(SceneFile, _deviceResources->scene))
	{
		this->setExitMessage("Error: Couldn't load the %s file\n", SceneFile);
		return pvr::Result::NotFound;
	}

	// calculate initial view matrix
	pvr::float32 fov;
	glm::vec3 from, to, up;
	_deviceResources->scene->getCameraProperties(0, fov, from, to, up);
	_viewMatrix = glm::lookAt(from, to, up);
	return pvr::Result::Success;
}

/*!********************************************************************************************
\return	Return  Result::Success if no error occured
\brief	Code in quitApplication() will be called by Shell once per run, just before exiting the program.
quitApplication() will not be called every time the rendering context is lost, only before application exit.
***********************************************************************************************/
pvr::Result VulkanPostProcessing::quitApplication()
{
	//Instructs the Asset Manager to free all resources
	_assetManager.releaseAll();
	return pvr::Result::Success;
}

/*!********************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in initView() will be called by Shell upon initialization or after a change
		in the rendering context. Used to initialize variables that are dependent on the rendering
		context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************/
pvr::Result VulkanPostProcessing::initView()
{
	_deviceResources->context = getGraphicsContext();

	// Calculates the projection matrix
	pvr::float32 fov = _deviceResources->scene->getCamera(0).getFOV();
	bool bRotate = isFullScreen() && isScreenRotated();
	if (bRotate)
	{
		_projectionMatrix = pvr::math::perspectiveFov(getApiType(), fov, (float)getHeight(), (float)getWidth(),
		                    _deviceResources->scene->getCamera(0).getNear(), _deviceResources->scene->getCamera(0).getFar(),
		                    glm::pi<pvr::float32>() * .5f);
	}
	else
	{
		_projectionMatrix = pvr::math::perspectiveFov(getApiType(), fov, (float)getWidth(), (float)getHeight(),
		                    _deviceResources->scene->getCamera(0).getNear(), _deviceResources->scene->getCamera(0).getFar());
	}

	_blurDimension = 256;

	//	Initialize VBO data
	// Load vertex data of all meshes in the scene into VBOs
	// The meshes have been exported with the "Interleave Vectors" option,
	// so all data is interleaved in the buffer at pMesh->pInterleaved.
	// Interleaving data improves the memory access pattern and cache efficiency,
	// thus it can be read faster by the hardware.
	pvr::utils::appendSingleBuffersFromModel(getGraphicsContext(), *_deviceResources->scene, _deviceResources->vbos, _deviceResources->ibos);

	// Create on screen Fbos
	_deviceResources->onScreenFbo = _deviceResources->context->createOnScreenFboSet();

	// Create fbo used for blur pass
	if (!createBlurFbo())
	{
		return pvr::Result::NotInitialized;
	}

	// create Fbo used for the pre bloom pass
	if (!createPreBloomFbo())
	{
		return pvr::Result::NotInitialized;
	}

	// calculate the texel offsets used in the blurring passes
	calculateBlurTexelOffsets();

	// create demo buffers
	createBuffers();

	// create the descriptor set layouts and pipeline layouts
	createDescriptorSetLayouts();

	//	Load textures
	if (!createDescriptors())
	{
		return pvr::Result::NotInitialized;
	}

	// create the graphics pipelines used throughout the demo
	if (!createPipelines())
	{
		return pvr::Result::NotInitialized;
	}

	if (_deviceResources->uiRenderer.init(_deviceResources->onScreenFbo[0]->getRenderPass(), 0) != pvr::Result::Success)
	{
		setExitMessage("Error: Failed to initialize the UIRenderer\n");
		return pvr::Result::NotInitialized;
	}

	_deviceResources->uiRenderer.getDefaultTitle()->setText("PostProcessing");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText(
	  "Left / right: Rendering mode\n"
	  "Up / down: Bloom intensity\n"
	  "Action:     Pause\n"
	);
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();
	updateSubtitleText();
	recordCommandBuffers();
	return pvr::Result::Success;
}

void VulkanPostProcessing::createCommandBuffers(pvr::uint32 swapchain)
{
	if (!_deviceResources->mainCmdNoBloom[swapchain].isValid())
	{
		_deviceResources->mainCmdNoBloom[swapchain] = _deviceResources->context->createCommandBufferOnDefaultPool();
	}
	if (!_deviceResources->mainCmdBloom[swapchain].isValid())
	{
		_deviceResources->mainCmdBloom[swapchain] = _deviceResources->context->createCommandBufferOnDefaultPool();
	}
	if (!_deviceResources->preBloomCommandBuffer[swapchain].isValid())
	{
		_deviceResources->preBloomCommandBuffer[swapchain] = _deviceResources->context->createSecondaryCommandBufferOnDefaultPool();
	}
	if (!_deviceResources->noBloomCommandBuffer[swapchain].isValid())
	{
		_deviceResources->noBloomCommandBuffer[swapchain] = _deviceResources->context->createSecondaryCommandBufferOnDefaultPool();
	}
	if (!_deviceResources->noBloomUiRendererCommandBuffer[swapchain].isValid())
	{
		_deviceResources->noBloomUiRendererCommandBuffer[swapchain] = _deviceResources->context->createSecondaryCommandBufferOnDefaultPool();
	}
	if (!_deviceResources->bloomUiRendererCommandBuffer[swapchain].isValid())
	{
		_deviceResources->bloomUiRendererCommandBuffer[swapchain] = _deviceResources->context->createSecondaryCommandBufferOnDefaultPool();
	}
	if (!_deviceResources->horizontalBlurCommandBuffer[swapchain].isValid())
	{
		_deviceResources->horizontalBlurCommandBuffer[swapchain] = _deviceResources->context->createSecondaryCommandBufferOnDefaultPool();
	}
	if (!_deviceResources->verticalBlurCommandBuffer[swapchain].isValid())
	{
		_deviceResources->verticalBlurCommandBuffer[swapchain] = _deviceResources->context->createSecondaryCommandBufferOnDefaultPool();
	}
	if (!_deviceResources->postBloomCommandBuffer[swapchain].isValid())
	{
		_deviceResources->postBloomCommandBuffer[swapchain] = _deviceResources->context->createSecondaryCommandBufferOnDefaultPool();
	}
}

void VulkanPostProcessing::recordCommandBuffers()
{
	for (pvr::uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		createCommandBuffers(i);

		recordCommandUIRenderer(i);

		// record no bloom command buffer
		recordNoBloomCommands(i);

		// record bloom command buffer
		recordBloomCommands(i);
	}
}

/*!********************************************************************************************
\brief	Create the blur fbo
\return	Return  true on success
***********************************************************************************************/
bool VulkanPostProcessing::createBlurFbo()
{
	pvr::ImageStorageFormat colorFormat(pvr::PixelFormat::RGBA_8888, 1, pvr::types::ColorSpace::lRGB, pvr::VariableType::UnsignedByteNorm);

	// create the render passes.
	pvr::api::RenderPassCreateParam blurRenderPassDesc;

	pvr::api::SubPass subPass;
	// use the first color attachment
	subPass.setColorAttachment(0, 0);
	subPass.enableDepthStencilAttachment(false);

	// setup subpasses
	blurRenderPassDesc.setColorInfo(0, pvr::api::RenderPassColorInfo(colorFormat, pvr::types::LoadOp::Clear, pvr::types::StoreOp::Store,
	                                pvr::types::ImageLayout::ColorAttachmentOptimal, pvr::types::ImageLayout::ShaderReadOnlyOptimal));
	blurRenderPassDesc.setSubPass(0, subPass);

	// create renderpass
	pvr::api::RenderPass blurRenderPass = _deviceResources->context->createRenderPass(blurRenderPassDesc);

	pvr::api::FboCreateParam blurFboDesc;
	blurFboDesc.setRenderPass(blurRenderPass);

	// blur at a much lower resolution
	blurFboDesc.setDimensions(_blurDimension, _blurDimension);

	// for each swapchain
	for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		// blur pass0
		{
			pvr::api::TextureStore colorTex = _deviceResources->context->createTexture();
			colorTex->allocate2D(colorFormat, _blurDimension, _blurDimension, pvr::types::ImageUsageFlags::ColorAttachment |
			                     pvr::types::ImageUsageFlags::Sampled, pvr::types::ImageLayout::ColorAttachmentOptimal);

			// set fbo color attachments
			blurFboDesc.setColor(0, _deviceResources->context->createTextureView(colorTex));

			// create the blur pass fbo
			_deviceResources->horizontalBlurPass.fbo[i] = _deviceResources->context->createFbo(blurFboDesc);

			if (!_deviceResources->horizontalBlurPass.fbo[i].isValid())
			{
				pvr::Log("Failed to create blur fbo");
				return false;
			}
		}
		// blur pass1
		{
			pvr::api::TextureStore colorTex = _deviceResources->context->createTexture();
			colorTex->allocate2D(colorFormat, _blurDimension, _blurDimension, pvr::types::ImageUsageFlags::ColorAttachment |
			                     pvr::types::ImageUsageFlags::Sampled, pvr::types::ImageLayout::ColorAttachmentOptimal);

			// set fbo color attachments
			blurFboDesc.setColor(0, _deviceResources->context->createTextureView(colorTex));

			// create the blur pass fbo
			_deviceResources->verticalBlurPass.fbo[i] = _deviceResources->context->createFbo(blurFboDesc);

			if (!_deviceResources->verticalBlurPass.fbo[i].isValid())
			{
				pvr::Log("Failed to create blur fbo");
				return false;
			}
		}
	}

	return true;
}

bool VulkanPostProcessing::createPreBloomFbo()
{
	// color and depth image formats
	pvr::ImageStorageFormat depthTexFormat(pvr::PixelFormat::Depth16, 1, pvr::types::ColorSpace::lRGB, pvr::VariableType::Float);
	pvr::ImageStorageFormat colorTexFormat(pvr::PixelFormat::RGBA_8888, 1, pvr::types::ColorSpace::lRGB, pvr::VariableType::UnsignedByteNorm);

	// depth texture storage
	pvr::api::TextureStore depthTexture[static_cast<pvr::uint32>(Config::MaxSwapChain)];

	// color texture storage
	pvr::api::TextureStore colorTexture[static_cast<pvr::uint32>(Config::MaxSwapChain)];
	pvr::api::TextureStore filterTexture[static_cast<pvr::uint32>(Config::MaxSwapChain)];

	// create the render pass
	pvr::api::RenderPassCreateParam renderPassInfo;
	pvr::api::RenderPassDepthStencilInfo dsInfo(depthTexFormat, pvr::types::LoadOp::Clear, pvr::types::StoreOp::Ignore);

	// configure the subpass
	pvr::api::SubPass subPass;
	subPass.setColorAttachment(0, 0);
	subPass.setColorAttachment(1, 1);
	subPass.enableDepthStencilAttachment(true);
	subPass.setDepthStencilAttachment(0);
	renderPassInfo.setSubPass(0, subPass);

	renderPassInfo.setColorInfo(0, pvr::api::RenderPassColorInfo(colorTexFormat, pvr::types::LoadOp::Clear, pvr::types::StoreOp::Store, 1,
	                            pvr::types::ImageLayout::ColorAttachmentOptimal, pvr::types::ImageLayout::ShaderReadOnlyOptimal));
	renderPassInfo.setColorInfo(1, pvr::api::RenderPassColorInfo(colorTexFormat, pvr::types::LoadOp::Clear, pvr::types::StoreOp::Store, 1,
	                            pvr::types::ImageLayout::ColorAttachmentOptimal, pvr::types::ImageLayout::ShaderReadOnlyOptimal));
	renderPassInfo.setDepthStencilInfo(0, dsInfo);

	// create the renderpass
	pvr::api::RenderPass renderPass = _deviceResources->context->createRenderPass(renderPassInfo);

	// pre bloom render area uses the full screen dimensions
	pvr::Rectanglei renderArea(0, 0, getWidth(), getHeight());

	// create the fbo
	pvr::api::FboCreateParam fboInfo;
	fboInfo.setRenderPass(renderPass);
	fboInfo.setDimensions(renderArea.width, renderArea.height);

	for (pvr::uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		// create depth texture as transient
		depthTexture[i] = _deviceResources->context->createTexture();
		depthTexture[i]->allocate2D(depthTexFormat, renderArea.width, renderArea.height, pvr::types::ImageUsageFlags::DepthStencilAttachment |
		                            pvr::types::ImageUsageFlags::TransientAttachment);

		// color and filter textures will be sampled
		colorTexture[i] = _deviceResources->context->createTexture();
		colorTexture[i]->allocate2D(colorTexFormat, renderArea.width, renderArea.height, pvr::types::ImageUsageFlags::ColorAttachment |
		                            pvr::types::ImageUsageFlags::Sampled, pvr::types::ImageLayout::ColorAttachmentOptimal);

		filterTexture[i] = _deviceResources->context->createTexture();
		filterTexture[i]->allocate2D(colorTexFormat, renderArea.width, renderArea.height, pvr::types::ImageUsageFlags::ColorAttachment |
		                             pvr::types::ImageUsageFlags::Sampled, pvr::types::ImageLayout::ColorAttachmentOptimal);

		// set color attachments
		fboInfo.setColor(0, _deviceResources->context->createTextureView(colorTexture[i]));
		fboInfo.setColor(1, _deviceResources->context->createTextureView(filterTexture[i]));

		// set depth stencil attachment
		fboInfo.setDepthStencil(0, _deviceResources->context->createTextureView(depthTexture[i]));

		// create the fbo
		_deviceResources->preBloomPass.fbo[i] = _deviceResources->context->createFbo(fboInfo);

		if (!_deviceResources->preBloomPass.fbo[i].isValid())
		{
			pvr::Log("Failed to create the rendering fbo");
			return false;
		}
	}
	return true;
}

/*!********************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in releaseView() will be called by Shell when the application quits or before
a change in the rendering context.
***********************************************************************************************/
pvr::Result VulkanPostProcessing::releaseView()
{
	_assetManager.releaseAll();
	_deviceResources.reset();
	return pvr::Result::Success;
}

void VulkanPostProcessing::updatePostBloomConfig(pvr::uint32 swapchain)
{
	if (_applyBloom)
	{
		pvr::float32 config[] = {(_drawObject ? 1.f : 0.0f), 1.f};
		_deviceResources->postBloomPass.uboBloomConfig.first.map(swapchain);
		_deviceResources->postBloomPass.uboBloomConfig.first.setValue("sTexFactor", config[0]);
		_deviceResources->postBloomPass.uboBloomConfig.first.setValue("sBlurTexFactor", config[1]);
		_deviceResources->postBloomPass.uboBloomConfig.first.unmap(swapchain);
	}
}


/*!*********************************************************************************************************************
\brief Update the animation
***********************************************************************************************************************/
void VulkanPostProcessing::updateAnimation()
{
	// Calculate the mask and light _rotation based on the passed time
	pvr::float32 const twoPi = glm::pi<pvr::float32>() * 2.f;

	if (_animating)
	{
		_rotation += glm::pi<pvr::float32>() * getFrameTime() * 0.0002f;
		// wrap it
		if (_rotation > twoPi) { _rotation -= twoPi; }
	}

	// Calculate the model, viewMatrix and projection matrix
	_worldMatrix = glm::rotate((-_rotation), glm::vec3(0.f, 1.f, 0.f)) * glm::scale(glm::vec3(1.65f));

	pvr::float32 fov;
	fov = _deviceResources->scene->getCamera(0).getFOV(0);

	glm::mat4x4 viewProj = _projectionMatrix * _viewMatrix;
	// Simple rotating directional light in model-space)
	const glm::mat4& mvInv = glm::inverse(_viewMatrix * _worldMatrix * _deviceResources->scene->getWorldMatrix(
	                                        _deviceResources->scene->getNode(0).getObjectId()));
	const glm::mat4& mvp = viewProj * _worldMatrix * _deviceResources->scene->getWorldMatrix(_deviceResources->scene->getNode(0).getObjectId());

	_deviceResources->renderScenePass.uboDynamic.buffer.mapMultipleArrayElements(getSwapChainIndex(), 0, _deviceResources->scene->getNumMeshNodes(),
	    pvr::types::MapBufferFlags::Write);
	for (pvr::uint32 i = 0; i < _deviceResources->scene->getNumMeshNodes(); ++i)
	{
		_deviceResources->renderScenePass.uboDynamic.buffer
		.setArrayValue(static_cast<pvr::uint32>(RenderScenePass::UboDynamicElements::MVInv), i, mvInv);

		_deviceResources->renderScenePass.uboDynamic.buffer.setArrayValue(
		  static_cast<pvr::uint32>(RenderScenePass::UboDynamicElements::MVPMatrix), i, mvp);
		_deviceResources->renderScenePass.uboDynamic.buffer.setArrayValue(
		  static_cast<pvr::uint32>(RenderScenePass::UboDynamicElements::LightDirection), i,
		  (glm::normalize(glm::vec3(glm::inverse(_worldMatrix) * LightPosition))));
	}
	_deviceResources->renderScenePass.uboDynamic.buffer.unmap(getSwapChainIndex());
}

void VulkanPostProcessing::updateBloomIntensity(pvr::float32 bloomIntensity)
{
	this->_bloomIntensity = bloomIntensity;
	_deviceResources->preBloomPass.descIntensity.first.map(0);
	_deviceResources->preBloomPass.descIntensity.first.setValue("BloomIntensity", _bloomIntensity);
	_deviceResources->preBloomPass.descIntensity.first.unmap(0);
}

/*!********************************************************************************************
\return	Return Result::Suceess if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************/
pvr::Result VulkanPostProcessing::renderFrame()
{
	updateAnimation();
	if (_applyBloom)
	{
		_deviceResources->mainCmdBloom[getSwapChainIndex()]->submit();
	}
	else
	{
		_deviceResources->mainCmdNoBloom[getSwapChainIndex()]->submit();
	}
	return pvr::Result::Success;
}

/*!********************************************************************************************
\brief	update the subtitle sprite
***********************************************************************************************/
void VulkanPostProcessing::updateSubtitleText()
{
	if (_applyBloom)
	{
		if (_drawObject)
		{
			_deviceResources->uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("Object with bloom"
			    " effect, intensity % 2.1f", _bloomIntensity));
		}
		else
		{
			_deviceResources->uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("Bloom effect"
			    " textures, intensity % 2.1f", _bloomIntensity));
		}
	}
	else
	{
		if (_drawObject)
		{
			_deviceResources->uiRenderer.getDefaultDescription()->setText("Object without bloom");
		}
		else
		{
			_deviceResources->uiRenderer.getDefaultDescription()->setText("Use up - down to draw object and / or bloom textures");
		}
	}
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
}

/*!********************************************************************************************
\brief	Handles user input and updates live variables accordingly.
***********************************************************************************************/
void VulkanPostProcessing::eventMappedInput(pvr::SimplifiedInput e)
{
	static int mode = 0;
	//Object+Bloom, object, bloom
	switch (e)
	{
	case pvr::SimplifiedInput::Left:
		if (--mode < 0) { mode = 2; }
		_applyBloom = (mode != 1); _drawObject = (mode != 2);
		updateSubtitleText();
		_deviceResources->context->waitIdle();
		for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
		{
			updatePostBloomConfig(i);
		}
		recordCommandBuffers();
		break;
	case pvr::SimplifiedInput::Right:
		++mode %= 3;
		_applyBloom = (mode != 1); _drawObject = (mode != 2);
		updateSubtitleText();
		_deviceResources->context->waitIdle();
		for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
		{
			updatePostBloomConfig(i);
		}
		recordCommandBuffers();
		break;
	case pvr::SimplifiedInput::Up:
		updateSubtitleText();
		updateBloomIntensity(std::min(_bloomIntensity + 0.1f, 5.f));
		_deviceResources->context->waitIdle();
		recordCommandBuffers();
		break;
	case pvr::SimplifiedInput::Down:
		updateBloomIntensity(std::max(_bloomIntensity - 0.1f, 0.f));
		updateSubtitleText();
		_deviceResources->context->waitIdle();
		recordCommandBuffers();
		break;
	case pvr::SimplifiedInput::ActionClose:
		this->exitShell();
		break;
	case pvr::SimplifiedInput::Action1:
	case pvr::SimplifiedInput::Action2:
	case pvr::SimplifiedInput::Action3:
		_animating = !_animating;
		break;
	default:
		break;
	}
}

/*!********************************************************************************************
\param	nodeIndex	Node index of the mesh to draw
\brief	Draws a Model::Mesh after the model viewMatrix matrix has been set and the material prepared.
***********************************************************************************************/
void VulkanPostProcessing::drawMesh(int nodeIndex, pvr::api::SecondaryCommandBuffer& cmdBuffer)
{
	int meshIndex = _deviceResources->scene->getNode(nodeIndex).getObjectId();
	const pvr::assets::Model::Mesh& mesh = _deviceResources->scene->getMesh(meshIndex);
	// bind the VBO for the mesh
	cmdBuffer->bindVertexBuffer(_deviceResources->vbos[meshIndex], 0, 0);
	// bind the index buffer, won't hurt if the handle is 0
	cmdBuffer->bindIndexBuffer(_deviceResources->ibos[meshIndex], 0, mesh.getFaces().getDataType());

	if (mesh.getMeshInfo().isIndexed)
	{
		// Indexed Triangle list
		cmdBuffer->drawIndexed(0, mesh.getNumFaces() * 3);
	}
	else
	{
		// Non-Indexed Triangle list
		cmdBuffer->drawArrays(0, mesh.getNumFaces() * 3);
	}
}

void VulkanPostProcessing::recordCommandUIRenderer(pvr::uint32 swapchain)
{
	_deviceResources->noBloomUiRendererCommandBuffer[swapchain]->beginRecording(_deviceResources->onScreenFbo[swapchain], 0);
	_deviceResources->uiRenderer.beginRendering(_deviceResources->noBloomUiRendererCommandBuffer[swapchain]);
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.endRendering();
	_deviceResources->noBloomUiRendererCommandBuffer[swapchain]->endRecording();

	_deviceResources->bloomUiRendererCommandBuffer[swapchain]->beginRecording(_deviceResources->onScreenFbo[swapchain], 0);
	_deviceResources->uiRenderer.beginRendering(_deviceResources->bloomUiRendererCommandBuffer[swapchain]);
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.endRendering();
	_deviceResources->bloomUiRendererCommandBuffer[swapchain]->endRecording();
}

void VulkanPostProcessing::recordCommandsNoBloom(pvr::uint32 swapchain)
{
	_deviceResources->noBloomCommandBuffer[swapchain]->beginRecording(_deviceResources->onScreenFbo[swapchain], 0);
	// Simple rotating directional light in model-space
	// Use simple shader program to render the mask
	_deviceResources->noBloomCommandBuffer[swapchain]->bindPipeline(_deviceResources->renderScenePass.pipeline);

	// Bind descriptor Sets
	// bind the albedo texture
	_deviceResources->noBloomCommandBuffer[swapchain]->bindDescriptorSet(_deviceResources->renderScenePass.pipeline->getPipelineLayout(), 0,
	    _deviceResources->renderScenePass.texDescriptor);

	pvr::uint32 uboOffset = _deviceResources->renderScenePass.uboDynamic.buffer.getAlignedElementArrayOffset(0);

	_deviceResources->noBloomCommandBuffer[swapchain]->bindDescriptorSet(_deviceResources->renderScenePass.pipeline->getPipelineLayout(), 1,
	    _deviceResources->renderScenePass.uboDynamic.sets[swapchain], &uboOffset, 1);

	_deviceResources->noBloomCommandBuffer[swapchain]->bindDescriptorSet(_deviceResources->renderScenePass.pipeline->getPipelineLayout(), 2,
	    _deviceResources->renderScenePass.uboStatic.sets[0]);
	// Draw the mesh
	drawMesh(0, _deviceResources->noBloomCommandBuffer[swapchain]);
	_deviceResources->noBloomCommandBuffer[swapchain]->endRecording();
}

void VulkanPostProcessing::recordNoBloomCommands(pvr::uint32 swapchain)
{
	recordCommandsNoBloom(swapchain);

	_deviceResources->mainCmdNoBloom[swapchain]->beginRecording();
	_deviceResources->mainCmdNoBloom[swapchain]->beginRenderPass(_deviceResources->onScreenFbo[swapchain], _deviceResources->onScreenFbo[swapchain]->getRenderPass(),
	    pvr::Rectanglei(0, 0, getWidth(), getHeight()), false, glm::vec4(0.00, 0.70, 0.67, 1.f));
	_deviceResources->mainCmdNoBloom[swapchain]->enqueueSecondaryCmds(_deviceResources->noBloomCommandBuffer[swapchain]);
	_deviceResources->mainCmdNoBloom[swapchain]->enqueueSecondaryCmds(_deviceResources->noBloomUiRendererCommandBuffer[swapchain]);
	_deviceResources->mainCmdNoBloom[swapchain]->endRenderPass();
	_deviceResources->mainCmdNoBloom[swapchain]->endRecording();
}

void VulkanPostProcessing::recordCommandsPreBloom(pvr::uint32 swapchain)
{
	_deviceResources->preBloomCommandBuffer[swapchain]->beginRecording(_deviceResources->preBloomPass.fbo[swapchain], 0);

	// filter the bright portion of the image
	_deviceResources->preBloomCommandBuffer[swapchain]->bindPipeline(_deviceResources->preBloomPass.pipeline);

	pvr::uint32 uboOffset = _deviceResources->renderScenePass.uboDynamic.buffer.getAlignedElementArrayOffset(0);

	// bind the pre bloom descriptor sets
	_deviceResources->preBloomCommandBuffer[swapchain]->bindDescriptorSet(_deviceResources->preBloomPass.pipeline->getPipelineLayout(), 0, _deviceResources->preBloomPass.descTex);
	_deviceResources->preBloomCommandBuffer[swapchain]->bindDescriptorSet(_deviceResources->preBloomPass.pipeline->getPipelineLayout(), 1, _deviceResources->preBloomPass.descIntensity.second);
	_deviceResources->preBloomCommandBuffer[swapchain]->bindDescriptorSet(_deviceResources->preBloomPass.pipeline->getPipelineLayout(), 2, _deviceResources->preBloomPass.uboDynamic.sets[swapchain], &uboOffset, 1);
	_deviceResources->preBloomCommandBuffer[swapchain]->bindDescriptorSet(_deviceResources->preBloomPass.pipeline->getPipelineLayout(), 3, _deviceResources->preBloomPass.uboStatic.sets[0]);
	drawMesh(0, _deviceResources->preBloomCommandBuffer[swapchain]);
	_deviceResources->preBloomCommandBuffer[swapchain]->endRecording();
}

void VulkanPostProcessing::recordCommandsBlur(pvr::api::SecondaryCommandBuffer& cmdBuffer, BlurPass& pass, pvr::uint32 swapchain)
{
	cmdBuffer->beginRecording(pass.fbo[swapchain], 0);
	cmdBuffer->bindPipeline(pass.pipeline);
	cmdBuffer->bindDescriptorSet(pass.pipeline->getPipelineLayout(), 0, pass.texDescSet[swapchain]);
	cmdBuffer->bindDescriptorSet(pass.pipeline->getPipelineLayout(), 1, pass.uboPerVert.second);
	cmdBuffer->drawArrays(0, 4);
	cmdBuffer->endRecording();
}

void VulkanPostProcessing::recordCommandsPostBloom(pvr::uint32 swapchain)
{
	_deviceResources->postBloomCommandBuffer[swapchain]->beginRecording(_deviceResources->onScreenFbo[swapchain], 0);
	_deviceResources->postBloomCommandBuffer[swapchain]->bindPipeline(_deviceResources->postBloomPass.pipeline);
	_deviceResources->postBloomCommandBuffer[swapchain]->bindDescriptorSet(_deviceResources->postBloomPass.pipeline->getPipelineLayout(), 0,
	    _deviceResources->postBloomPass.texDescSet[swapchain]);
	_deviceResources->postBloomCommandBuffer[swapchain]->bindDescriptorSet(_deviceResources->postBloomPass.pipeline->getPipelineLayout(), 1,
	    _deviceResources->postBloomPass.uboBloomConfig.second[swapchain]);
	_deviceResources->postBloomCommandBuffer[swapchain]->drawArrays(0, 4);
	_deviceResources->postBloomCommandBuffer[swapchain]->endRecording();
}

void VulkanPostProcessing::recordBloomCommands(pvr::uint32 swapchain)
{
	recordCommandsPreBloom(swapchain);
	recordCommandsBlur(_deviceResources->horizontalBlurCommandBuffer[swapchain], _deviceResources->horizontalBlurPass, swapchain);
	recordCommandsBlur(_deviceResources->verticalBlurCommandBuffer[swapchain], _deviceResources->verticalBlurPass, swapchain);
	recordCommandsPostBloom(swapchain);

	_deviceResources->mainCmdBloom[swapchain]->beginRecording();

	// pre bloom
	{
		glm::vec4 preBloomClearColors[] = { glm::vec4(0.0, 0.70, 0.67, 1.0f), glm::vec4(0.0, 0.0, 0.0, 1.f) };
		_deviceResources->mainCmdBloom[swapchain]->beginRenderPass(_deviceResources->preBloomPass.fbo[swapchain],
		    pvr::Rectanglei(0, 0, getWidth(), getHeight()), false, preBloomClearColors, 2);
		_deviceResources->mainCmdBloom[swapchain]->enqueueSecondaryCmds(_deviceResources->preBloomCommandBuffer[swapchain]);
		_deviceResources->mainCmdBloom[swapchain]->endRenderPass();
	}

	// horizontal blur
	{
		_deviceResources->mainCmdBloom[swapchain]->beginRenderPass(_deviceResources->horizontalBlurPass.fbo[swapchain],
		    pvr::Rectanglei(0, 0, _deviceResources->horizontalBlurPass.fbo[swapchain]->getDimensions().x,
		                    _deviceResources->horizontalBlurPass.fbo[swapchain]->getDimensions().y), false, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		_deviceResources->mainCmdBloom[swapchain]->enqueueSecondaryCmds(_deviceResources->horizontalBlurCommandBuffer[swapchain]);
		_deviceResources->mainCmdBloom[swapchain]->endRenderPass();
	}

	// vertical blur
	{
		_deviceResources->mainCmdBloom[swapchain]->beginRenderPass(_deviceResources->verticalBlurPass.fbo[swapchain],
		    pvr::Rectanglei(0, 0, _deviceResources->verticalBlurPass.fbo[swapchain]->getDimensions().x,
		                    _deviceResources->verticalBlurPass.fbo[swapchain]->getDimensions().y), false, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		_deviceResources->mainCmdBloom[swapchain]->enqueueSecondaryCmds(_deviceResources->verticalBlurCommandBuffer[swapchain]);
		_deviceResources->mainCmdBloom[swapchain]->endRenderPass();
	}

	// post bloom
	{
		_deviceResources->mainCmdBloom[swapchain]->beginRenderPass(_deviceResources->onScreenFbo[swapchain],
		    pvr::Rectanglei(0, 0, getWidth(), getHeight()), false, glm::vec4(0.0, 0.0, 0.0, 1.0f));
		_deviceResources->mainCmdBloom[swapchain]->enqueueSecondaryCmds(_deviceResources->postBloomCommandBuffer[swapchain]);
		_deviceResources->mainCmdBloom[swapchain]->enqueueSecondaryCmds(_deviceResources->bloomUiRendererCommandBuffer[swapchain]);
		_deviceResources->mainCmdBloom[swapchain]->endRenderPass();
	}

	// Transition image layouts
	pvr::api::MemoryBarrierSet barriers;

	// transform back to color-attachment write from shader read
	barriers.addBarrier(pvr::api::ImageAreaBarrier(pvr::types::AccessFlags::ShaderRead, pvr::types::AccessFlags::ColorAttachmentWrite,
	                    _deviceResources->horizontalBlurPass.fbo[swapchain]->getColorAttachment(0)->getResource(),
	                    pvr::types::ImageSubresourceRange(), pvr::types::ImageLayout::ShaderReadOnlyOptimal,
	                    pvr::types::ImageLayout::ColorAttachmentOptimal));
	barriers.addBarrier(pvr::api::ImageAreaBarrier(pvr::types::AccessFlags::ShaderRead, pvr::types::AccessFlags::ColorAttachmentWrite,
	                    _deviceResources->verticalBlurPass.fbo[swapchain]->getColorAttachment(0)->getResource(),
	                    pvr::types::ImageSubresourceRange(), pvr::types::ImageLayout::ShaderReadOnlyOptimal,
	                    pvr::types::ImageLayout::ColorAttachmentOptimal));

	// transform back to color-attachment write from shader read
	barriers.addBarrier(pvr::api::ImageAreaBarrier(pvr::types::AccessFlags::ShaderRead, pvr::types::AccessFlags::ColorAttachmentWrite,
	                    _deviceResources->preBloomPass.fbo[swapchain]->getColorAttachment(0)->getResource(),
	                    pvr::types::ImageSubresourceRange(), pvr::types::ImageLayout::ShaderReadOnlyOptimal,
	                    pvr::types::ImageLayout::ColorAttachmentOptimal));
	barriers.addBarrier(pvr::api::ImageAreaBarrier(pvr::types::AccessFlags::ShaderRead, pvr::types::AccessFlags::ColorAttachmentWrite,
	                    _deviceResources->preBloomPass.fbo[swapchain]->getColorAttachment(1)->getResource(),
	                    pvr::types::ImageSubresourceRange(), pvr::types::ImageLayout::ShaderReadOnlyOptimal,
	                    pvr::types::ImageLayout::ColorAttachmentOptimal));

	_deviceResources->mainCmdBloom[swapchain]->pipelineBarrier(pvr::types::PipelineStageFlags::FragmentShader, pvr::types::PipelineStageFlags::FragmentShader, barriers);

	_deviceResources->mainCmdBloom[swapchain]->endRecording();
}

/*!********************************************************************************************
\return	Return auto ptr to the demo supplied by the user
\brief	This function must be implemented by the user of the shell.
The user should return its Shell object defining the behaviour of the application.
***********************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() { return std::auto_ptr<pvr::Shell>(new VulkanPostProcessing()); }
