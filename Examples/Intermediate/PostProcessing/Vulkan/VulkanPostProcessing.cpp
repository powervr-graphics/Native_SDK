/*!********************************************************************************************
\File         VulkanPostProcessing.cpp
\Title        Bloom
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Shows how to do a bloom effect
***********************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVRUIRenderer/PVRUIRenderer.h"

using namespace pvr;
using namespace types;
utils::VertexBindings_Name VertexBindings[] =
{
	{ "POSITION", "inVertex" },
	{ "NORMAL", "inNormal" },
	{ "UV0", "inTexCoord" },
};

enum class QuadAttribute
{
	Position,
	TexCoord
};

enum class FboPass
{
	OnScreen,
	RenderScene,
	BlurFbo0,
	BlurFbo1,
	Count,
	NumBlurFbo = 2
};

enum class Config
{
	MaxSwapChain = 4
};

/**********************************************************************************************
Consts
**********************************************************************************************/
const glm::vec4 LightPos(-1.5f, 0.0f, 10.0f, 0.0);
const uint32 TexSize = 256;    // Blur render target size (power-of-two)

/**********************************************************************************************
Content file names
***********************************************************************************************/
const char FragShaderSrcFile[]			= "FragShader_vk.fsh.spv";
const char VertShaderSrcFile[]			= "VertShader_vk.vsh.spv";
const char PreBloomFragShaderSrcFile[]	= "PreBloomFragShader_vk.fsh.spv";
const char PreBloomVertShaderSrcFile[]	= "PreBloomVertShader_vk.vsh.spv";
const char PostBloomFragShaderSrcFile[]	= "PostBloomFragShader_vk.fsh.spv";
const char PostBloomVertShaderSrcFile[]	= "PostBloomVertShader_vk.vsh.spv";
const char BlurFragSrcFile[]			= "BlurFragShader_vk.fsh.spv";
const char BlurVertSrcFile[]			= "BlurVertShader_vk.vsh.spv";

// PVR texture files
const char BaseTexFile[]				= "Marble.pvr";
// POD scene files
const char SceneFile[]					= "scene.pod";

struct Ubo
{
	pvr::utils::StructuredMemoryView buffer;
	api::DescriptorSet sets[static_cast<pvr::uint32>(Config::MaxSwapChain)];
};

struct OffScreenFbo
{
	api::Fbo			fbo;		//	per swapchain
	Rectanglei			renderArea;
};

struct BlurPass
{
	std::pair<utils::StructuredMemoryView, api::DescriptorSet> uboPerVert;
	api::GraphicsPipeline			pipeline;
	api::DescriptorSet texDescSet[static_cast<pvr::uint32>(Config::MaxSwapChain)]; // per swapchain

	Rectanglei						renderArea;
	api::Fbo fbo[static_cast<pvr::uint32>(Config::MaxSwapChain)];
};

typedef std::pair<pvr::StringHash, pvr::types::GpuDatatypes::Enum> BufferViewMapping;

struct RenderScenePass
{
	Ubo uboDynamic;
	Ubo uboStatic;

	api::GraphicsPipeline pipeline;
	Rectanglei renderArea;
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

const BufferViewMapping RenderScenePass::UboStaticMapping[] =
{
	BufferViewMapping("Shininess", pvr::types::GpuDatatypes::float32),
};

struct PreBloomPass
{
	api::Fbo fbo[static_cast<pvr::uint32>(Config::MaxSwapChain)];
	api::GraphicsPipeline			pipeline;
	api::DescriptorSet				descTex;
	std::pair<utils::StructuredMemoryView, api::DescriptorSet> descIntensity;
	api::SecondaryCommandBuffer cmdBuffer[static_cast<pvr::uint32>(Config::MaxSwapChain)];

	Ubo uboDynamic;
	Ubo uboStatic;
};

struct PostBloomPass
{
	api::GraphicsPipeline pipeline;
	std::pair<api::BufferView, api::DescriptorSet> uboBloomConfig;
	std::pair<api::BufferView, api::DescriptorSet> uboMVP;
	api::DescriptorSet texDescSet[static_cast<pvr::uint32>(Config::MaxSwapChain)];//per swapchain
};

/*!********************************************************************************************
Class implementing the Shell functions.
***********************************************************************************************/
class VulkanPostProcessing : public Shell
{
	struct DeviceResources
	{
		// Renderpasses
		PreBloomPass preBloomPass;
		RenderScenePass renderScenePass;
		PostBloomPass postBloomPass;
		BlurPass blurPass0;
		BlurPass blurPass1;

		api::FboSet onScreenFbo;

		// Textures
		api::TextureView baseTex;
		api::TextureView bloomMapTex;

		// Samplers
		api::Sampler   samplerRepeat;
		api::Sampler   samplerClamp;

		// Vbos and Ibos
		std::vector<api::Buffer> vbos;
		std::vector<api::Buffer> ibos;

		api::Buffer		quadVbo;
		api::Buffer		quadIbo;

		// Command Buffers
		api::CommandBuffer cmdBloom[static_cast<pvr::uint32>(Config::MaxSwapChain)];
		api::CommandBuffer cmdNoBloom[static_cast<pvr::uint32>(Config::MaxSwapChain)];

		// descriptor layouts
		api::DescriptorSetLayout texSamplerLayoutFrag;
		api::DescriptorSetLayout postBloomTexLayoutFrag;
		api::DescriptorSetLayout uboLayoutVert;
		api::DescriptorSetLayout uboLayoutFrag;
		api::DescriptorSetLayout uboLayoutDynamicVert;

		// 3D Model
		assets::ModelHandle scene;
		// context
		pvr::GraphicsContext context;
		// UI Renderer
		ui::UIRenderer	uiRenderer;
	};

	std::auto_ptr<DeviceResources> apiObj;

	float32 bloomIntensity;
	bool applyBloom;
	bool drawObject;
	bool animating;

	float32 rotation;
	api::AssetStore assetManager;
	glm::mat4 world;
	glm::mat4 view;
	glm::mat4 proj;
public:
	VulkanPostProcessing() : bloomIntensity(1.f) {}

	virtual Result initApplication();
	virtual Result initView();
	virtual Result releaseView();
	virtual Result quitApplication();
	virtual Result renderFrame();

	bool createDescriptors();
	bool createPipelines();
	bool loadVbos();
	bool createBlurFbo();
	bool createOnScreenFbo();
	bool createPreBloomFbo();
	void recordBloomCommands(api::CommandBuffer& cmd, pvr::uint32 swapchain);
	void recordNoBloomCommands(api::CommandBuffer& cmd, pvr::uint32 swapchain);
	void recordCommandUIRenderer(api::CommandBuffer& cmdBuffer, pvr::uint32 swapchain);
	void updateSubtitleText();
	void updatePostBloomConfig();
	void drawMesh(int i32NodeIndex, api::CommandBuffer& cmdBuffer);
	void drawAxisAlignedQuad(api::CommandBuffer& cmdBuffer);
	void eventMappedInput(SimplifiedInput e);
	void updateAnimation();
	void updateBloomIntensity(float32 bloomIntensity);
	void preTransitionFbo(api::CommandBuffer& cmd, api::Fbo& fbo);
	void postTransitionFbo(api::CommandBuffer& cmd, api::Fbo& fbo);
	void recordCommandBuffers();
};

void VulkanPostProcessing::recordCommandUIRenderer(api::CommandBuffer& cmdBuffer, pvr::uint32 swapchain)
{
	apiObj->uiRenderer.beginRendering(cmdBuffer);
	apiObj->uiRenderer.getSdkLogo()->render();
	apiObj->uiRenderer.getDefaultTitle()->render();
	apiObj->uiRenderer.getDefaultControls()->render();
	apiObj->uiRenderer.getDefaultDescription()->render();
	apiObj->uiRenderer.endRendering();
}

void VulkanPostProcessing::recordNoBloomCommands(api::CommandBuffer& cmd, pvr::uint32 swapchain)
{
	//--- draw scene
	cmd->beginRenderPass(apiObj->onScreenFbo[swapchain], pvr::Rectanglei(0, 0, getWidth(), getHeight()),
	                     true, glm::vec4(0.00, 0.70, 0.67, 0.f));

	// Simple rotating directional light in model-space
	// Use simple shader program to render the mask
	cmd->bindPipeline(apiObj->renderScenePass.pipeline);

	// Bind descriptor Sets
	// bind the albedo texture
	cmd->bindDescriptorSet(apiObj->renderScenePass.pipeline->getPipelineLayout(), 0,
	                       apiObj->renderScenePass.texDescriptor);

	pvr::uint32 uboOffset = apiObj->renderScenePass.uboDynamic.buffer.getAlignedElementArrayOffset(0);

	cmd->bindDescriptorSet(apiObj->renderScenePass.pipeline->getPipelineLayout(), 1,
	                       apiObj->renderScenePass.uboDynamic.sets[swapchain], &uboOffset, 1);

	cmd->bindDescriptorSet(apiObj->renderScenePass.pipeline->getPipelineLayout(), 2,
	                       apiObj->renderScenePass.uboStatic.sets[0]);
	// Draw the mesh
	drawMesh(0, cmd);

	recordCommandUIRenderer(cmd, swapchain);

	cmd->endRenderPass();
}


/*!********************************************************************************************
\return	Return true if no error occurred
\brief	Loads the textures required for this training course
***********************************************************************************************/
bool VulkanPostProcessing::createDescriptors()
{
	// Load Textures
	if (!assetManager.getTextureWithCaching(getGraphicsContext(), BaseTexFile, &apiObj->baseTex, NULL))
	{
		setExitMessage("FAILED to load texture %s.", BaseTexFile);
		return false;
	}

	// sampler repeat
	assets::SamplerCreateParam samplerDesc;
	samplerDesc.minificationFilter = SamplerFilter::Linear;
	samplerDesc.mipMappingFilter = SamplerFilter::Nearest;
	samplerDesc.magnificationFilter = SamplerFilter::Linear;
	samplerDesc.wrapModeU = SamplerWrap::Repeat;
	samplerDesc.wrapModeV = SamplerWrap::Repeat;
	apiObj->samplerRepeat = apiObj->context->createSampler(samplerDesc);

	// sampler clamp
	samplerDesc.wrapModeU = SamplerWrap::Clamp;
	samplerDesc.wrapModeV = SamplerWrap::Clamp;
	apiObj->samplerClamp = apiObj->context->createSampler(samplerDesc);

	// set up the per swapchains descriptors
	Ubo& uboDynamic = apiObj->renderScenePass.uboDynamic;
	uboDynamic.buffer.setupDynamic(apiObj->context, apiObj->scene->getNumMeshNodes(),
	                               BufferViewTypes::UniformBufferDynamic);

	uboDynamic.buffer.addEntriesPacked(RenderScenePass::UboDynamicMapping,
	                                   sizeof(RenderScenePass::UboDynamicMapping) /
	                                   sizeof(RenderScenePass::UboDynamicMapping[0]));

	for (pvr::uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		// render pass descriptor set (albedo texture)
		{
			// dynamic ubo
			pvr::api::Buffer  uboDynamicBuffer = apiObj->context->createBuffer(uboDynamic.buffer.getAlignedTotalSize(),
			                                     types::BufferBindingUse::UniformBuffer, true);

			uboDynamic.buffer.connectWithBuffer(i, apiObj->context->createBufferView(uboDynamicBuffer, 0,
			                                    uboDynamic.buffer.getAlignedElementSize()),
			                                    BufferViewTypes::UniformBufferDynamic);

			uboDynamic.sets[i] = apiObj->context->createDescriptorSetOnDefaultPool(apiObj->uboLayoutDynamicVert);
			uboDynamic.sets[i]->update(api::DescriptorSetUpdate().setDynamicUbo(0, uboDynamic.buffer.getConnectedBuffer(i)));
		}

		// pre-bloom pass descriptor set
		{
			apiObj->preBloomPass.uboDynamic = apiObj->renderScenePass.uboDynamic;
		}

		// blur pass0 descriptor set (blur pass1 texture for the shader read)
		{
			apiObj->blurPass0.texDescSet[i] = apiObj->context->createDescriptorSetOnDefaultPool(apiObj->texSamplerLayoutFrag);
			apiObj->blurPass0.texDescSet[i]->update(pvr::api::DescriptorSetUpdate().setCombinedImageSampler(0,
			                                        apiObj->preBloomPass.fbo[i]->getColorAttachment(1), apiObj->samplerClamp));
		}

		// blur pass1 descriptor set (blur pass1 texture for the shader read)
		{
			apiObj->blurPass1.texDescSet[i] = apiObj->context->createDescriptorSetOnDefaultPool(apiObj->texSamplerLayoutFrag);
			apiObj->blurPass1.texDescSet[i]->update(pvr::api::DescriptorSetUpdate().setCombinedImageSampler(0,
			                                        apiObj->blurPass0.fbo[i]->getColorAttachment(0), apiObj->samplerClamp));
		}

		// post bloom
		{
			// tex-sampler descriptor
			pvr::api::DescriptorSetUpdate descSetUpdate;
			descSetUpdate.setCombinedImageSampler(0,
			                                      apiObj->preBloomPass.fbo[i]->getColorAttachment(0),
			                                      apiObj->samplerClamp);
			descSetUpdate.setCombinedImageSampler(1,
			                                      apiObj->blurPass1.fbo[i]->getColorAttachment(0),
			                                      apiObj->samplerClamp);
			apiObj->postBloomPass.texDescSet[i] =
			  apiObj->context->createDescriptorSetOnDefaultPool(apiObj->postBloomTexLayoutFrag);

			apiObj->postBloomPass.texDescSet[i]->update(descSetUpdate);
		}
	}// next swapchain
	// set up the render scene pass static descriptors
	{
		apiObj->renderScenePass.uboStatic.buffer.setupArray(apiObj->context, 1, BufferViewTypes::UniformBuffer);
		apiObj->renderScenePass.uboStatic.buffer.addEntriesPacked(RenderScenePass::UboStaticMapping,
		    sizeof(RenderScenePass::UboStaticMapping) / sizeof(RenderScenePass::UboStaticMapping[0]));
		apiObj->renderScenePass.uboStatic.buffer.connectWithBuffer(0, apiObj->context->createBufferAndView(sizeof(pvr::float32),
		    types::BufferBindingUse::UniformBuffer, true), BufferViewTypes::UniformBuffer);
		apiObj->renderScenePass.uboStatic.sets[0] = apiObj->context->createDescriptorSetOnDefaultPool(apiObj->uboLayoutVert);

		apiObj->renderScenePass.uboStatic.sets[0]->update(api::DescriptorSetUpdate().setUbo(0,
		    apiObj->renderScenePass.uboStatic.buffer.getConnectedBuffer(0)));

		// update the buffer once
		apiObj->renderScenePass.uboStatic.buffer.map(0, MapBufferFlags::Write);
		apiObj->renderScenePass.uboStatic.buffer.setValue(0, .6f);
		apiObj->renderScenePass.uboStatic.buffer.unmap(0);

		api::DescriptorSetUpdate descSetUpdate;
		descSetUpdate.setCombinedImageSampler(0, apiObj->baseTex, apiObj->samplerClamp);
		apiObj->renderScenePass.texDescriptor = apiObj->context->createDescriptorSetOnDefaultPool(apiObj->texSamplerLayoutFrag);
		apiObj->renderScenePass.texDescriptor->update(descSetUpdate);
	}

	api::DescriptorSetUpdate descSetUpdate;
	// pre bloom pass
	{
		// create the intensity descriptor
		apiObj->preBloomPass.descIntensity.first.setupArray(apiObj->context, 1, BufferViewTypes::UniformBuffer);
		apiObj->preBloomPass.descIntensity.first.addEntryPacked("BloomIntensity", GpuDatatypes::float32);
		apiObj->preBloomPass.descIntensity.first.connectWithBuffer(0, apiObj->context->createBufferAndView(sizeof(pvr::float32),
		    types::BufferBindingUse::UniformBuffer, true), BufferViewTypes::UniformBuffer);
		apiObj->preBloomPass.descIntensity.second = apiObj->context->createDescriptorSetOnDefaultPool(apiObj->uboLayoutFrag);
		descSetUpdate.setUbo(0, apiObj->preBloomPass.descIntensity.first.getConnectedBuffer(0));
		apiObj->preBloomPass.descIntensity.second->update(descSetUpdate);

		// update the initial bloom intensity
		apiObj->preBloomPass.descIntensity.first.map(0, types::MapBufferFlags::Write);
		apiObj->preBloomPass.descIntensity.first.setValue(0, 1.f);
		apiObj->preBloomPass.descIntensity.first.unmap(0);

		// copy the texture descriptor from the render scene pass
		apiObj->preBloomPass.uboStatic = apiObj->renderScenePass.uboStatic;
		apiObj->preBloomPass.descTex = apiObj->renderScenePass.texDescriptor;
	}

	// Choosing Gaussian kernel weights row index 8 (1, 8, 28, 56, 70, 56, 28, 8, 1) (total 256)
	// ignore first and second weighting - reduce total weight by 18 = 238 to avoid darkening image through repeated blurring
	// Original Weightings
	//		(28, 56, 70, 56, 28) - (total 238)
	//		28 / 238 = 0.11764705882352941176470588235294
	//		56 / 238 = 0.23529411764705882352941176470588
	//		70 / 238 = 0.29411764705882352941176470588235
	//		56 / 238 = 0.23529411764705882352941176470588
	//		28 / 238 = 0.11764705882352941176470588235294
	// Original Offsets:
	//		2, 1, 0, 1, 2
	// Formulas used:
	//		Weight(t1,t2) = weight(t1) + weight(t2)
	//		offset(t1,t2) = (offset(t1) * weight(t1) + offset(t2) * weight(t2)) / weight(t1,t2)
	// Calculation:
	//		Weight(0) = 0.11764705882352941176470588235294 + 0.23529411764705882352941176470588 =
	//			0.35294117647058823529411764705882
	//		Weight(1) = 0.23529411764705882352941176470588 + 0.29411764705882352941176470588235 =
	//			0.52941176470588235294117647058823
	//
	//		offset(0) = ((0.11764705882352941176470588235294 * 2) + (0.23529411764705882352941176470588)) /
	//												0.35294117647058823529411764705882 =
	//		offset(0) = 1.3333333333333333333333333333333
	//
	//		offset(1) = 0.0

	// Texel offset for blur filter kernel
	float32 texelOffset = 1.0f / (pvr::float32)TexSize;

	// Altered weights for the faster filter kernel
	pvr::float32 intraTexelOffset = 1.333333f * texelOffset;
	texelOffset += intraTexelOffset;

	// blur pass0 (horizontal)
	{
		apiObj->blurPass0.uboPerVert.first.setupArray(apiObj->context, 1, BufferViewTypes::UniformBuffer);
		apiObj->blurPass0.uboPerVert.first.addEntryPacked("TexelOffsetX", GpuDatatypes::float32);
		apiObj->blurPass0.uboPerVert.first.addEntryPacked("TexelOffsetY", GpuDatatypes::float32);
		apiObj->blurPass0.uboPerVert.first.connectWithBuffer(0, apiObj->context->createBufferAndView(
		      apiObj->blurPass0.uboPerVert.first.getAlignedTotalSize(), types::BufferBindingUse::UniformBuffer, true),
		    BufferViewTypes::UniformBuffer);

		apiObj->blurPass0.uboPerVert.second = apiObj->context->createDescriptorSetOnDefaultPool(apiObj->uboLayoutVert);
		apiObj->blurPass0.uboPerVert.second->update(api::DescriptorSetUpdate().setUbo(0,
		    apiObj->blurPass0.uboPerVert.first.getConnectedBuffer(0)));

		// set the const values
		apiObj->blurPass0.uboPerVert.first.map(0, types::MapBufferFlags::Write);
		apiObj->blurPass0.uboPerVert.first.setValue(0, texelOffset);
		apiObj->blurPass0.uboPerVert.first.setValue(1, 0.f);
		apiObj->blurPass0.uboPerVert.first.unmap(0);
	}

	// blur pass1 (vertical)
	{
		apiObj->blurPass1.uboPerVert.first.setupArray(apiObj->context, 1, BufferViewTypes::UniformBuffer);
		apiObj->blurPass1.uboPerVert.first.addEntryPacked("TexelOffsetX", GpuDatatypes::float32);
		apiObj->blurPass1.uboPerVert.first.addEntryPacked("TexelOffsetY", GpuDatatypes::float32);
		apiObj->blurPass1.uboPerVert.first.connectWithBuffer(0, apiObj->context->createBufferAndView(
		      apiObj->blurPass1.uboPerVert.first.getAlignedTotalSize(), types::BufferBindingUse::UniformBuffer, true),
		    BufferViewTypes::UniformBuffer);

		apiObj->blurPass1.uboPerVert.second = apiObj->context->createDescriptorSetOnDefaultPool(apiObj->uboLayoutVert);
		apiObj->blurPass1.uboPerVert.second->update(api::DescriptorSetUpdate().setUbo(0,
		    apiObj->blurPass1.uboPerVert.first.getConnectedBuffer(0)));

		// set the const values
		apiObj->blurPass1.uboPerVert.first.map(0, types::MapBufferFlags::Write);
		apiObj->blurPass1.uboPerVert.first.setValue(0, 0);
		apiObj->blurPass1.uboPerVert.first.setValue(1, texelOffset);
		apiObj->blurPass1.uboPerVert.first.unmap(0);
	}

	// post bloom pass
	{
		apiObj->postBloomPass.uboBloomConfig.first = apiObj->context->createBufferAndView(sizeof(pvr::float32) * 2,
		    types::BufferBindingUse::UniformBuffer, true);
		apiObj->postBloomPass.uboBloomConfig.second = apiObj->context->createDescriptorSetOnDefaultPool(apiObj->uboLayoutFrag);

		descSetUpdate.setUbo(0, apiObj->postBloomPass.uboBloomConfig.first);

		apiObj->postBloomPass.uboBloomConfig.second->update(descSetUpdate);

		pvr::float32 data[] = {1.0f, 1.0f};
		apiObj->postBloomPass.uboBloomConfig.first->update(data, 0, apiObj->postBloomPass.uboBloomConfig.first->getRange());

		// ubo mvp
		apiObj->postBloomPass.uboMVP.first = apiObj->context->createBufferAndView(sizeof(glm::mat4),
		                                     types::BufferBindingUse::UniformBuffer, true);

		apiObj->postBloomPass.uboMVP.second = apiObj->context->createDescriptorSetOnDefaultPool(apiObj->uboLayoutVert);

		descSetUpdate.clear().setUbo(0, apiObj->postBloomPass.uboMVP.first);
		apiObj->postBloomPass.uboMVP.second->update(descSetUpdate);

		glm::mat4 mvp(1.f);
		apiObj->postBloomPass.uboMVP.first->update(glm::value_ptr(mvp), 0, sizeof(mvp));
	}
	return true;
}

/*!********************************************************************************************
\brief	Loads and compiles the shaders and links the shader programs
\return	Return true if no error occurred required for this training course
***********************************************************************************************/
bool VulkanPostProcessing::createPipelines()
{
	{
		api::DescriptorSetLayoutCreateParam layoutDesc;
		layoutDesc.setBinding(0, DescriptorType::CombinedImageSampler, 1, ShaderStageFlags::Fragment);
		apiObj->texSamplerLayoutFrag = apiObj->context->createDescriptorSetLayout(layoutDesc);
	}

	{
		api::DescriptorSetLayoutCreateParam layoutDesc;
		layoutDesc.setBinding(0, DescriptorType::CombinedImageSampler, 1, ShaderStageFlags::Fragment);
		layoutDesc.setBinding(1, DescriptorType::CombinedImageSampler, 1, ShaderStageFlags::Fragment);
		apiObj->postBloomTexLayoutFrag = apiObj->context->createDescriptorSetLayout(layoutDesc);
	}

	{
		api::DescriptorSetLayoutCreateParam layoutDesc;
		layoutDesc.setBinding(0, DescriptorType::UniformBuffer, 1, ShaderStageFlags::Vertex);
		apiObj->uboLayoutVert = apiObj->context->createDescriptorSetLayout(layoutDesc);
	}

	{
		api::DescriptorSetLayoutCreateParam layoutDesc;
		layoutDesc.setBinding(0, DescriptorType::UniformBuffer, 1, ShaderStageFlags::Fragment);
		apiObj->uboLayoutFrag = apiObj->context->createDescriptorSetLayout(layoutDesc);
	}

	{
		api::DescriptorSetLayoutCreateParam layoutDesc;
		layoutDesc.setBinding(0, DescriptorType::UniformBufferDynamic, 1, ShaderStageFlags::Vertex);
		apiObj->uboLayoutDynamicVert = apiObj->context->createDescriptorSetLayout(layoutDesc);
	}

	api::GraphicsPipelineCreateParam basePipe;

	// enable backface culling
	basePipe.rasterizer.setCullFace(pvr::types::Face::Back);

	// set blending states
	types::BlendingConfig colorAttachemtState;
	// disable blending
	colorAttachemtState.blendEnable = false;
	basePipe.colorBlend.setAttachmentState(0, colorAttachemtState);

	// enable depth testing
	basePipe.depthStencil.setDepthCompareFunc(ComparisonMode::Less);
	basePipe.depthStencil.setDepthTestEnable(true);
	basePipe.depthStencil.setDepthWrite(true);

	api::VertexAttributeInfo quadAttributes[2] =
	{
		api::VertexAttributeInfo(static_cast<pvr::uint32>(QuadAttribute::Position),
		DataType::Float32, 2, 0, "inVertex"),
		api::VertexAttributeInfo(static_cast<pvr::uint32>(QuadAttribute::TexCoord),
		DataType::Float32, 2, sizeof(float32) * 2, "inTexCoord")
	};

	const assets::Mesh& mesh = apiObj->scene->getMesh(0);

	// create render scene pass pipeline
	{
		api::GraphicsPipelineCreateParam basicPipeDesc = basePipe;
		basicPipeDesc.vertexShader.setShader(apiObj->context->createShader(*getAssetStream(VertShaderSrcFile),
		                                     ShaderType::VertexShader));

		basicPipeDesc.fragmentShader.setShader(apiObj->context->createShader(*getAssetStream(FragShaderSrcFile),
		                                       ShaderType::FragmentShader));

		utils::createInputAssemblyFromMesh(mesh, VertexBindings, 3, basicPipeDesc);

		// create pipeline layout
		api::PipelineLayoutCreateParam pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(apiObj->texSamplerLayoutFrag);
		pipeLayoutInfo.addDescSetLayout(apiObj->uboLayoutDynamicVert);
		pipeLayoutInfo.addDescSetLayout(apiObj->uboLayoutVert);
		basicPipeDesc.pipelineLayout = apiObj->context->createPipelineLayout(pipeLayoutInfo);

		basicPipeDesc.renderPass = apiObj->onScreenFbo[0]->getRenderPass();
		basicPipeDesc.subPass = 0;
		apiObj->renderScenePass.pipeline = apiObj->context->createGraphicsPipeline(basicPipeDesc);

		if (apiObj->renderScenePass.pipeline.isValid() == false)
		{
			this->setExitMessage("Failed To Create the RenderScenePass Pipeline");
			return false;
		}
	}

	// create prebloom pass pipeline
	{
		api::GraphicsPipelineCreateParam prebloomPipeDesc = basePipe;

		prebloomPipeDesc.vertexShader.setShader(apiObj->context->createShader(*getAssetStream(PreBloomVertShaderSrcFile),
		                                        ShaderType::VertexShader));

		prebloomPipeDesc.fragmentShader.setShader(apiObj->context->createShader(*getAssetStream(PreBloomFragShaderSrcFile),
		    ShaderType::FragmentShader));

		utils::createInputAssemblyFromMesh(mesh, VertexBindings, 3, prebloomPipeDesc);

		// set blending states
		types::BlendingConfig colorAttachemtState;
		// disable blending
		colorAttachemtState.blendEnable = false;
		prebloomPipeDesc.colorBlend.setAttachmentState(0, colorAttachemtState);
		prebloomPipeDesc.colorBlend.setAttachmentState(1, colorAttachemtState);

		// create pipeline layout
		api::PipelineLayoutCreateParam pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(apiObj->texSamplerLayoutFrag);
		pipeLayoutInfo.addDescSetLayout(apiObj->uboLayoutFrag);
		pipeLayoutInfo.addDescSetLayout(apiObj->uboLayoutDynamicVert);
		pipeLayoutInfo.addDescSetLayout(apiObj->uboLayoutVert);

		prebloomPipeDesc.pipelineLayout = apiObj->context->createPipelineLayout(pipeLayoutInfo);

		prebloomPipeDesc.renderPass = apiObj->preBloomPass.fbo[0]->getRenderPass();
		prebloomPipeDesc.subPass = 0;

		apiObj->preBloomPass.pipeline = apiObj->context->createGraphicsPipeline(prebloomPipeDesc);
		if (apiObj->preBloomPass.pipeline.isValid() == false)
		{
			this->setExitMessage("Failed to Create preBloom pipeline");
			return false;
		}
	}

	// create Post-Bloom Pipeline
	{
		api::GraphicsPipelineCreateParam postbloomPipeDesc;
		types::BlendingConfig attachmentState(false, BlendFactor::One, BlendFactor::One, BlendOp::Add);
		postbloomPipeDesc.colorBlend.setAttachmentState(0, attachmentState);
		postbloomPipeDesc.depthStencil.setDepthTestEnable(false);
		postbloomPipeDesc.depthStencil.setDepthWrite(false);

		postbloomPipeDesc.vertexShader.setShader(apiObj->context->createShader(*getAssetStream(PostBloomVertShaderSrcFile),
		    ShaderType::VertexShader));

		postbloomPipeDesc.fragmentShader.setShader(apiObj->context->createShader(*getAssetStream(PostBloomFragShaderSrcFile),
		    ShaderType::FragmentShader));

		postbloomPipeDesc.vertexInput.clear();
		postbloomPipeDesc.inputAssembler.setPrimitiveTopology(types::PrimitiveTopology::TriangleStrip);

		postbloomPipeDesc.renderPass = apiObj->onScreenFbo[0]->getRenderPass();
		postbloomPipeDesc.subPass = 0;

		postbloomPipeDesc.vertexInput.setInputBinding(0, sizeof(float32) * 4, StepRate::Vertex);
		postbloomPipeDesc.vertexInput.addVertexAttributes(0, quadAttributes, sizeof(quadAttributes) / sizeof(quadAttributes[0]));
		postbloomPipeDesc.inputAssembler.setPrimitiveTopology(types::PrimitiveTopology::TriangleList);

		// create pipeline layout
		api::PipelineLayoutCreateParam pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(apiObj->postBloomTexLayoutFrag);
		pipeLayoutInfo.addDescSetLayout(apiObj->uboLayoutFrag);
		pipeLayoutInfo.addDescSetLayout(apiObj->uboLayoutVert);

		postbloomPipeDesc.pipelineLayout = apiObj->context->createPipelineLayout(pipeLayoutInfo);

		apiObj->postBloomPass.pipeline = apiObj->context->createGraphicsPipeline(postbloomPipeDesc);

		if (apiObj->postBloomPass.pipeline.isValid() == false)
		{
			this->setExitMessage("Failed to Create postBloom pipeline");
			return false;
		}
	}

	//   Blur Pipeline
	{
		api::GraphicsPipelineCreateParam blurPipeDesc;

		// set blending states
		types::BlendingConfig colorAttachemtState;
		// disable blending
		colorAttachemtState.blendEnable = false;

		blurPipeDesc.colorBlend.setAttachmentState(0, colorAttachemtState);
		blurPipeDesc.depthStencil.setDepthTestEnable(false);
		blurPipeDesc.depthStencil.setDepthWrite(false);

		blurPipeDesc.vertexShader.setShader(apiObj->context->createShader(*getAssetStream(BlurVertSrcFile),
		                                    ShaderType::VertexShader));
		blurPipeDesc.fragmentShader.setShader(apiObj->context->createShader(*getAssetStream(BlurFragSrcFile),
		                                      ShaderType::FragmentShader));

		blurPipeDesc.vertexInput.clear();

		blurPipeDesc.vertexInput.setInputBinding(0, sizeof(float32) * 4, StepRate::Vertex);
		blurPipeDesc.vertexInput.addVertexAttributes(0, quadAttributes, sizeof(quadAttributes) / sizeof(quadAttributes[0]));
		blurPipeDesc.inputAssembler.setPrimitiveTopology(types::PrimitiveTopology::TriangleList);

		// create pipeline layout
		api::PipelineLayoutCreateParam pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(apiObj->texSamplerLayoutFrag);
		pipeLayoutInfo.addDescSetLayout(apiObj->uboLayoutVert);
		blurPipeDesc.pipelineLayout = apiObj->context->createPipelineLayout(pipeLayoutInfo);

		blurPipeDesc.renderPass = apiObj->blurPass0.fbo[0]->getRenderPass();

		pvr::Rectanglei region(0, 0, apiObj->blurPass0.fbo[0]->getDimensions().x,
		                       apiObj->blurPass0.fbo[0]->getDimensions().y);

		blurPipeDesc.viewport.setViewportAndScissor(0,  api::Viewport(region), region);

		apiObj->blurPass0.pipeline = apiObj->blurPass1.pipeline =
		                               apiObj->context->createGraphicsPipeline(blurPipeDesc);

		if (apiObj->blurPass0.pipeline.isValid() == false)
		{
			this->setExitMessage("Failed to Create Blur pipeline");
			return false;
		}
	}
	return true;
}

/*!****************************************************************************
\brief	Loads the mesh data required for this training course into vertex buffer objects
******************************************************************************/
bool VulkanPostProcessing::loadVbos()
{
	// Load vertex data of all meshes in the scene into VBOs
	// The meshes have been exported with the "Interleave Vectors" option,
	// so all data is interleaved in the buffer at pMesh->pInterleaved.
	// Interleaving data improves the memory access pattern and cache efficiency,
	// thus it can be read faster by the hardware.
	utils::appendSingleBuffersFromModel(getGraphicsContext(), *apiObj->scene, apiObj->vbos, apiObj->ibos);

	const float32 halfDim = 1.f;
	// create quad vertices..
	// in vulkan the -y is up and the texture origin is top left
	const float32 afVertexData[] =
	{
		-halfDim, -halfDim,	// top left
		0.0f, 0.0f,			// uv
		-halfDim, halfDim,	// bottom left
		0.0f, 1.0f,			// uv
		halfDim, halfDim,	//  bottom right
		1.0f, 1.0f,			// uv
		halfDim, -halfDim,	// top right
		1.0f, 0.0f,			// uv
	};

	uint16 indices[] = { 0, 1, 2, 0, 2, 3 };
	apiObj->quadVbo = apiObj->context->createBuffer(sizeof(afVertexData), BufferBindingUse::VertexBuffer, true);
	void* mapData = apiObj->quadVbo->map(types::MapBufferFlags::Write, 0, sizeof(afVertexData));
	memcpy(mapData, afVertexData, sizeof(afVertexData));
	apiObj->quadVbo->unmap();

	apiObj->quadIbo = apiObj->context->createBuffer(sizeof(indices), BufferBindingUse::IndexBuffer, true);
	mapData = apiObj->quadIbo->map(types::MapBufferFlags::Write, 0, sizeof(indices));
	memcpy(mapData, indices, sizeof(afVertexData));
	apiObj->quadIbo->unmap();

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
Result VulkanPostProcessing::initApplication()
{
	// Apply bloom per default
	applyBloom = true;
	drawObject = true;
	animating = true;
	// Initial number of blur passes, can be changed during runtime
	rotation = 0.0f;

	// Texel offset for blur filter kernel
	// Altered weights for the faster filter kernel
	float32 w1 = 0.0555555f;
	float32 w2 = 0.2777777f;

	// Intensity multiplier for the bloom effect
	// Load the scene
	assetManager.init(*this);
	apiObj.reset(new DeviceResources());
	if (!assetManager.loadModel(SceneFile, apiObj->scene))
	{
		this->setExitMessage("Error: Couldn't load the %s file\n", SceneFile);
		return Result::NotFound;
	}
	float32 fov;
	glm::vec3 from, to, up;
	apiObj->scene->getCameraProperties(0, fov, from, to, up);
	view = glm::lookAt(from, to, up);
	return Result::Success;
}

/*!********************************************************************************************
\return	Return  Result::Success if no error occured
\brief	Code in quitApplication() will be called by Shell once per run, just before exiting the program.
quitApplication() will not be called every time the rendering context is lost, only before application exit.
***********************************************************************************************/
Result VulkanPostProcessing::quitApplication()
{
	//Instructs the Asset Manager to free all resources
	assetManager.releaseAll();
	return Result::Success;
}

/*!********************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in initView() will be called by Shell upon initialization or after a change
		in the rendering context. Used to initialize variables that are dependent on the rendering
		context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************/
Result VulkanPostProcessing::initView()
{
	apiObj->context = getGraphicsContext();
	float32 fov = apiObj->scene->getCamera(0).getFOV();
	bool bRotate = isFullScreen() && isScreenRotated();
	if (bRotate)
	{
		proj = math::perspectiveFov(getApiType(), fov, (float)getHeight(), (float)getWidth(),
		                            apiObj->scene->getCamera(0).getNear(), apiObj->scene->getCamera(0).getFar(),
		                            glm::pi<float32>() * .5f);
	}
	else
	{
		proj = math::perspectiveFov(getApiType(), fov, (float)getWidth(), (float)getHeight(),
		                            apiObj->scene->getCamera(0).getNear(), apiObj->scene->getCamera(0).getFar());
	}

	//	Initialize VBO data
	if (!loadVbos()) {  return Result::NotInitialized;  }

	// Create on screen Fbos
	if (!createOnScreenFbo())
	{
		return Result::NotInitialized;
	}

	// Create fbo used for blur pass
	if (!createBlurFbo())
	{
		return Result::NotInitialized;
	}

	// create Fbo used for the pre bloom pass
	if (!createPreBloomFbo())
	{
		return Result::NotInitialized;
	}

	//	Load and compile the shaders & link programs
	if (!createPipelines())
	{
		return Result::NotInitialized;
	}

	//	Load textures
	if (!createDescriptors())
	{
		return Result::NotInitialized;
	}

	if (apiObj->uiRenderer.init(apiObj->onScreenFbo[0]->getRenderPass(), 0) != Result::Success)
	{
		setExitMessage("Error: Failed to initialize the UIRenderer\n");
		return Result::NotInitialized;
	}

	apiObj->uiRenderer.getDefaultTitle()->setText("PostProcessing");
	apiObj->uiRenderer.getDefaultTitle()->commitUpdates();
	apiObj->uiRenderer.getDefaultControls()->setText(
	  "Left / right: Rendering mode\n"
	  "Up / down: Bloom intensity\n"
	  "Action:     Pause\n"
	);
	apiObj->uiRenderer.getDefaultControls()->commitUpdates();
	updateSubtitleText();
	recordCommandBuffers();
	return Result::Success;
}

void VulkanPostProcessing::recordCommandBuffers()
{
	for (pvr::uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		if (!apiObj->cmdNoBloom[i].isValid())
		{
			apiObj->cmdNoBloom[i] = apiObj->context->createCommandBufferOnDefaultPool();
		}
		if (!apiObj->cmdBloom[i].isValid())
		{
			apiObj->cmdBloom[i] = apiObj->context->createCommandBufferOnDefaultPool();
		}

		// record no bloom command buffer
		apiObj->cmdNoBloom[i]->beginRecording();
		recordNoBloomCommands(apiObj->cmdNoBloom[i], i);
		apiObj->cmdNoBloom[i]->endRecording();

		// record bloom command buffer
		apiObj->cmdBloom[i]->beginRecording();
		recordBloomCommands(apiObj->cmdBloom[i], i);
		apiObj->cmdBloom[i]->endRecording();
	}
}

/*!********************************************************************************************
\brief Create render fbo for rendering the scene
\return	Return true if success
***********************************************************************************************/
bool VulkanPostProcessing::createOnScreenFbo()
{
	{
		apiObj->onScreenFbo = apiObj->context->createOnScreenFboSet();
	}

	return true;
}

/*!********************************************************************************************
\brief	Create the blur fbo
\return	Return  true on success
***********************************************************************************************/
bool VulkanPostProcessing::createBlurFbo()
{
	api::ImageStorageFormat colorTexFormat(PixelFormat::RGBA_8888, 1, ColorSpace::lRGB, VariableType::UnsignedByteNorm);

	// create the render passes.
	api::RenderPassCreateParam blurRenderPassDesc;
	api::RenderPassColorInfo colorInfo(colorTexFormat, LoadOp::Clear);
	api::SubPass subPass;

	// use the first color attachment
	subPass.setColorAttachment(0, 0);
	subPass.setDepthStencilAttachment(false);

	// setup subpasses
	blurRenderPassDesc.setColorInfo(0, colorInfo);
	blurRenderPassDesc.setSubPass(0, subPass);

	// create renderpass
	api::RenderPass blurRenderPass = apiObj->context->createRenderPass(blurRenderPassDesc);
	api::FboCreateParam blurFboDesc;
	blurFboDesc.setRenderPass(blurRenderPass);
	blurFboDesc.setDimension(TexSize, TexSize);

	// for each swapchain
	for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		// blur pass0
		{
			api::TextureStore colorTex = apiObj->context->createTexture();
			colorTex->allocate2D(colorTexFormat, TexSize, TexSize,
			                     types::ImageUsageFlags::ColorAttachment | types::ImageUsageFlags::Sampled,
			                     types::ImageLayout::ColorAttachmentOptimal);

			// set fbo color attachments
			blurFboDesc.setColor(0, apiObj->context->createTextureView(colorTex));

			// create the blur pass fbo
			apiObj->blurPass0.fbo[i] = apiObj->context->createFbo(blurFboDesc);

			if (!apiObj->blurPass0.fbo[i].isValid())
			{
				Log("Failed to create blur fbo");
				return false;
			}
		}
		// blur pass1
		{
			api::TextureStore colorTex = apiObj->context->createTexture();
			colorTex->allocate2D(colorTexFormat, TexSize, TexSize,
			                     types::ImageUsageFlags::ColorAttachment | types::ImageUsageFlags::Sampled,
			                     types::ImageLayout::ColorAttachmentOptimal);

			// set fbo color attachments
			blurFboDesc.setColor(0, apiObj->context->createTextureView(colorTex));

			// create the blur pass fbo
			apiObj->blurPass1.fbo[i] = apiObj->context->createFbo(blurFboDesc);

			if (!apiObj->blurPass1.fbo[i].isValid())
			{
				Log("Failed to create blur fbo");
				return false;
			}
		}
	}
	apiObj->blurPass1.renderArea = apiObj->blurPass0.renderArea = Rectanglei(0, 0, TexSize, TexSize);
	return true;
}

bool VulkanPostProcessing::createPreBloomFbo()
{
	api::ImageStorageFormat depthTexFormat(PixelFormat::Depth16, 1, ColorSpace::lRGB, VariableType::Float);
	api::ImageStorageFormat colorTexFormat(PixelFormat::RGBA_8888, 1, ColorSpace::lRGB, VariableType::UnsignedByteNorm);
	api::TextureStore depthTexture[static_cast<pvr::uint32>(Config::MaxSwapChain)];

	api::TextureStore colorTexture0[static_cast<pvr::uint32>(Config::MaxSwapChain)];
	api::TextureStore colorTexture1[static_cast<pvr::uint32>(Config::MaxSwapChain)];

	api::TextureView depthTexView[static_cast<pvr::uint32>(Config::MaxSwapChain)];
	api::TextureView colorTexView[static_cast<pvr::uint32>(Config::MaxSwapChain)];

	// create the render pass.
	api::RenderPassCreateParam renderPassInfo;
	api::RenderPassColorInfo colorInfo(colorTexFormat, LoadOp::Clear);
	api::RenderPassDepthStencilInfo dsInfo(depthTexFormat, LoadOp::Clear);

	api::SubPass subPass; subPass.setColorAttachment(0, 0).setColorAttachment(1, 1); // use the first and second colorAttachment

	renderPassInfo.setSubPass(0, subPass);
	renderPassInfo.setDepthStencilInfo(dsInfo);
	renderPassInfo.setColorInfo(0, colorInfo).setColorInfo(1, colorInfo);
	api::RenderPass renderPass = apiObj->context->createRenderPass(renderPassInfo);
	api::FboCreateParam fboInfo;
	fboInfo.setRenderPass(renderPass);
	Rectanglei renderArea(0, 0, getWidth(), getHeight());
	fboInfo.setDimension(renderArea.width, renderArea.height);
	for (pvr::uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		// create depth and color texture & view for color and sampler usage
		depthTexture[i] = apiObj->context->createTexture();
		depthTexture[i]->allocate2D(depthTexFormat, renderArea.width, renderArea.height,
		                            types::ImageUsageFlags::DepthStencilAttachment);

		colorTexture0[i] = apiObj->context->createTexture();
		colorTexture0[i]->allocate2D(colorTexFormat, renderArea.width, renderArea.height,
		                             types::ImageUsageFlags::ColorAttachment | types::ImageUsageFlags::Sampled,
		                             types::ImageLayout::ColorAttachmentOptimal);

		colorTexture1[i] = apiObj->context->createTexture();
		colorTexture1[i]->allocate2D(colorTexFormat, renderArea.width, renderArea.height,
		                             types::ImageUsageFlags::ColorAttachment | types::ImageUsageFlags::Sampled,
		                             types::ImageLayout::ColorAttachmentOptimal);

		fboInfo
		.setColor(0, apiObj->context->createTextureView(colorTexture0[i]))
		.setColor(1, apiObj->context->createTextureView(colorTexture1[i]));

		fboInfo.setDepthStencil(apiObj->context->createTextureView(depthTexture[i]));
		apiObj->preBloomPass.fbo[i] = apiObj->context->createFbo(fboInfo);

		if (!apiObj->preBloomPass.fbo[i].isValid())
		{
			Log("Failed to create the rendering fbo");
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
Result VulkanPostProcessing::releaseView()
{
	assetManager.releaseAll();
	apiObj.reset();
	return Result::Success;
}

void VulkanPostProcessing::updatePostBloomConfig()
{
	if (applyBloom)
	{
		pvr::float32 config[] = {(drawObject ? 1.f : 0.0f), 1.f};
		apiObj->postBloomPass.uboBloomConfig.first->update(config, 0, sizeof(config));
	}
}


/*!*********************************************************************************************************************
\brief Update the animation
***********************************************************************************************************************/
void VulkanPostProcessing::updateAnimation()
{
	// Calculate the mask and light rotation based on the passed time
	float32 const twoPi = glm::pi<float32>() * 2.f;

	if (animating)
	{
		rotation += glm::pi<float32>() * getFrameTime() * 0.0002f;
		// wrap it
		if (rotation > twoPi) { rotation -= twoPi; }
	}

	// Calculate the model, view and projection matrix
	world = glm::rotate((-rotation), glm::vec3(0.f, 1.f, 0.f)) * glm::scale(glm::vec3(1.65f));

	float32 fov;
	fov = apiObj->scene->getCamera(0).getFOV(0);

	glm::mat4x4 viewProj = proj * view;
	// Simple rotating directional light in model-space)
	const glm::mat4& mvInv = glm::inverse(view * world * apiObj->scene->getWorldMatrix(
	                                        apiObj->scene->getNode(0).getObjectId()));
	const glm::mat4& mvp = viewProj * world * apiObj->scene->getWorldMatrix(apiObj->scene->getNode(0).getObjectId());

	apiObj->renderScenePass.uboDynamic.buffer.mapMultipleArrayElements(getSwapChainIndex(), 0, apiObj->scene->getNumMeshNodes(),
	    types::MapBufferFlags::Write);
	for (pvr::uint32 i = 0; i < apiObj->scene->getNumMeshNodes(); ++i)
	{
		apiObj->renderScenePass.uboDynamic.buffer
		.setArrayValue(static_cast<pvr::uint32>(RenderScenePass::UboDynamicElements::MVInv), i, mvInv);

		apiObj->renderScenePass.uboDynamic.buffer.setArrayValue(
		  static_cast<pvr::uint32>(RenderScenePass::UboDynamicElements::MVPMatrix), i, mvp);
		apiObj->renderScenePass.uboDynamic.buffer.setArrayValue(
		  static_cast<pvr::uint32>(RenderScenePass::UboDynamicElements::LightDirection), i,
		  (glm::normalize(glm::vec3(glm::inverse(world) * LightPos))));
	}
	apiObj->renderScenePass.uboDynamic.buffer.unmap(getSwapChainIndex());
}

void VulkanPostProcessing::updateBloomIntensity(float32 bloomIntensity)
{
	this->bloomIntensity = bloomIntensity;
	apiObj->preBloomPass.descIntensity.first.map(0);
	apiObj->preBloomPass.descIntensity.first.setValue(0, bloomIntensity);
	apiObj->preBloomPass.descIntensity.first.unmap(0);
}

/*!********************************************************************************************
\return	Return Result::Suceess if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************/
Result VulkanPostProcessing::renderFrame()
{
	updateAnimation();
	if (applyBloom)
	{
		apiObj->cmdBloom[getSwapChainIndex()]->submit();
	}
	else
	{
		apiObj->cmdNoBloom[getSwapChainIndex()]->submit();
	}
	return Result::Success;
}

/*!********************************************************************************************
\brief	update the subtitle sprite
***********************************************************************************************/
void VulkanPostProcessing::updateSubtitleText()
{
	if (applyBloom)
	{
		if (drawObject)
		{
			apiObj->uiRenderer.getDefaultDescription()->setText(strings::createFormatted("Object with bloom"
			    " effect, intensity % 2.1f", bloomIntensity));
		}
		else
		{
			apiObj->uiRenderer.getDefaultDescription()->setText(strings::createFormatted("Bloom effect"
			    " textures, intensity % 2.1f", bloomIntensity));
		}
	}
	else
	{
		if (drawObject)
		{
			apiObj->uiRenderer.getDefaultDescription()->setText("Object without bloom");
		}
		else
		{
			apiObj->uiRenderer.getDefaultDescription()->setText("Use up - down to draw object and / or bloom textures");
		}
	}
	apiObj->uiRenderer.getDefaultDescription()->commitUpdates();
}

/*!********************************************************************************************
\brief	Handles user input and updates live variables accordingly.
***********************************************************************************************/
void VulkanPostProcessing::eventMappedInput(SimplifiedInput e)
{
	static int mode = 0;
	//Object+Bloom, object, bloom
	switch (e)
	{
	case SimplifiedInput::Left:
		if (--mode < 0) { mode = 2; }
		applyBloom = (mode != 1); drawObject = (mode != 2);
		updateSubtitleText();
		updatePostBloomConfig();
		apiObj->context->waitIdle();
		recordCommandBuffers();
		break;
	case SimplifiedInput::Right:
		++mode %= 3;
		applyBloom = (mode != 1); drawObject = (mode != 2);
		updateSubtitleText();
		updatePostBloomConfig();
		apiObj->context->waitIdle();
		recordCommandBuffers();
		break;
	case SimplifiedInput::Up:
		updateSubtitleText();
		updateBloomIntensity(int(.5f + 10.f * std::min(bloomIntensity + .2f, 5.f)) * .1f);
		apiObj->context->waitIdle();
		recordCommandBuffers();
		break;
	case SimplifiedInput::Down:
		updateBloomIntensity(int(.5f + 10.f * std::max(bloomIntensity - .2f, 0.f)) * .1f);
		updateSubtitleText();
		apiObj->context->waitIdle();
		recordCommandBuffers();
		break;
	case SimplifiedInput::ActionClose:
		this->exitShell();
		break;
	case SimplifiedInput::Action1:
	case SimplifiedInput::Action2:
	case SimplifiedInput::Action3:
		animating = !animating;
		break;
	default:
		break;
	}
}

/*!********************************************************************************************
\param	nodeIndex	Node index of the mesh to draw
\brief	Draws a Model::Mesh after the model view matrix has been set and the material prepared.
***********************************************************************************************/
void VulkanPostProcessing::drawMesh(int nodeIndex, api::CommandBuffer& cmdBuffer)
{
	int meshIndex = apiObj->scene->getNode(nodeIndex).getObjectId();
	const assets::Model::Mesh& mesh = apiObj->scene->getMesh(meshIndex);
	// bind the VBO for the mesh
	cmdBuffer->bindVertexBuffer(apiObj->vbos[meshIndex], 0, 0);
	// bind the index buffer, won't hurt if the handle is 0
	cmdBuffer->bindIndexBuffer(apiObj->ibos[meshIndex], 0, mesh.getFaces().getDataType());

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

/*!********************************************************************************************
\brief	Add the draw commands for a full screen quad to a commandbuffer
***********************************************************************************************/
void VulkanPostProcessing::drawAxisAlignedQuad(api::CommandBuffer& cmdBuffer)
{
	// construct the scale matrix
	cmdBuffer->bindVertexBuffer(apiObj->quadVbo, 0, 0);
	cmdBuffer->bindIndexBuffer(apiObj->quadIbo, 0, IndexType::IndexType16Bit);
	cmdBuffer->drawIndexed(0, 6);
}

void VulkanPostProcessing::postTransitionFbo(api::CommandBuffer& cmd, api::Fbo& fbo)
{
	api::MemoryBarrierSet barriers;
	for (pvr::uint32 i = 0; i < fbo->getNumColorAttachments(); ++i)
	{
		const api::TextureView& colorTex = fbo->getColorAttachment(i);
		barriers.addBarrier(api::ImageAreaBarrier(types::AccessFlags::ColorAttachmentWrite,
		                    types::AccessFlags::TransferRead, colorTex->getResource(), types::ImageSubresourceRange(),
		                    types::ImageLayout::ColorAttachmentOptimal, types::ImageLayout::PresentSrc));
	}
	cmd->pipelineBarrier(types::PipelineStageFlags::AllCommands, types::PipelineStageFlags::AllCommands, barriers);
}

void VulkanPostProcessing::preTransitionFbo(api::CommandBuffer& cmd, api::Fbo& fbo)
{
	api::MemoryBarrierSet barriers;
	for (pvr::uint32 i = 0; i < fbo->getNumColorAttachments(); ++i)
	{
		const api::TextureView& colorTex = fbo->getColorAttachment(i);
		barriers.addBarrier(api::ImageAreaBarrier(types::AccessFlags::TransferRead,
		                    types::AccessFlags::ColorAttachmentWrite, colorTex->getResource(), types::ImageSubresourceRange(),
		                    types::ImageLayout::PresentSrc, types::ImageLayout::ColorAttachmentOptimal));
	}
	cmd->pipelineBarrier(types::PipelineStageFlags::AllCommands, types::PipelineStageFlags::AllCommands, barriers);
}

void VulkanPostProcessing::recordBloomCommands(api::CommandBuffer& cmd, pvr::uint32 swapchain)
{
	// prebloom
	{
//		put a barrier so bloom intensity updates from host can be synchronized properly
		api::MemoryBarrierSet barriers;
		barriers.addBarrier(pvr::api::BufferRangeBarrier(types::AccessFlags::HostWrite,
		                    types::AccessFlags::ShaderRead,
		                    apiObj->preBloomPass.descIntensity.first.getConnectedBuffer(0)->getResource(),
		                    0, apiObj->preBloomPass.descIntensity.first.getAlignedTotalSize()));
		cmd->pipelineBarrier(types::PipelineStageFlags::TopOfPipeline, types::PipelineStageFlags::TopOfPipeline, barriers);

		glm::vec4 clearColors[] =
		{
			glm::vec4(.00, 0.70, 0.67, 0.f) ,
			glm::vec4(.0, 0.0, 0.0, 1.f)
		};

		cmd->beginRenderPass(apiObj->preBloomPass.fbo[swapchain], pvr::Rectanglei(0, 0, getWidth(), getHeight()),
		                     true, clearColors, sizeof(clearColors) / sizeof(clearColors[0]));

		// filter the bright portion of the image
		cmd->bindPipeline(apiObj->preBloomPass.pipeline);

		// bind the pre bloom descriptor sets
		cmd->bindDescriptorSet(apiObj->preBloomPass.pipeline->getPipelineLayout(), 0,
		                       apiObj->preBloomPass.descTex, 0);

		cmd->bindDescriptorSet(apiObj->preBloomPass.pipeline->getPipelineLayout(), 1,
		                       apiObj->preBloomPass.descIntensity.second, 0);

		pvr::uint32 uboOffset = apiObj->renderScenePass.uboDynamic.buffer.getAlignedElementArrayOffset(0);

		cmd->bindDescriptorSet(apiObj->preBloomPass.pipeline->getPipelineLayout(), 2,
		                       apiObj->preBloomPass.uboDynamic.sets[swapchain], &uboOffset, 1);

		cmd->bindDescriptorSet(apiObj->preBloomPass.pipeline->getPipelineLayout(), 3,
		                       apiObj->preBloomPass.uboStatic.sets[0], 0);
		drawMesh(0, cmd);

		cmd->endRenderPass();
	}

	// Horizontal blur
	{
		BlurPass& pass = apiObj->blurPass0;
		cmd->beginRenderPass(pass.fbo[swapchain], pvr::Rectanglei(0, 0, pass.fbo[swapchain]->getDimensions().x,
		                     pass.fbo[swapchain]->getDimensions().y), true, glm::vec4(0.0f));

		// transform from color-attachment write to shader read
		api::MemoryBarrierSet barriers;
		barriers.addBarrier(api::ImageAreaBarrier(types::AccessFlags::ColorAttachmentWrite, types::AccessFlags::ShaderRead,
		                    apiObj->preBloomPass.fbo[swapchain]->getColorAttachment(1)->getResource(),
		                    types::ImageSubresourceRange(), types::ImageLayout::ColorAttachmentOptimal,
		                    types::ImageLayout::ShaderReadOnlyOptimal));

		cmd->pipelineBarrier(types::PipelineStageFlags::TopOfPipeline, types::PipelineStageFlags::TopOfPipeline, barriers);

		cmd->bindPipeline(apiObj->blurPass0.pipeline);
		cmd->bindDescriptorSet(apiObj->blurPass0.pipeline->getPipelineLayout(), 0, apiObj->blurPass0.texDescSet[swapchain], 0);
		cmd->bindDescriptorSet(apiObj->blurPass0.pipeline->getPipelineLayout(), 1, apiObj->blurPass0.uboPerVert.second, 0);
		drawAxisAlignedQuad(cmd);

		barriers.clearAllBarriers();

		// transform back to color-attachment write from shader read
		barriers.addBarrier(api::ImageAreaBarrier(types::AccessFlags::ShaderRead, types::AccessFlags::ColorAttachmentWrite,
		                    apiObj->preBloomPass.fbo[swapchain]->getColorAttachment(1)->getResource(),
		                    types::ImageSubresourceRange(), types::ImageLayout::ShaderReadOnlyOptimal,
		                    types::ImageLayout::ColorAttachmentOptimal));

		cmd->pipelineBarrier(types::PipelineStageFlags::TopOfPipeline, types::PipelineStageFlags::TopOfPipeline, barriers);

		cmd->endRenderPass();
	}

	// Vertical blur
	{
		BlurPass& pass = apiObj->blurPass1;
		cmd->beginRenderPass(pass.fbo[swapchain], pvr::Rectanglei(0, 0, pass.fbo[swapchain]->getDimensions().x,
		                     pass.fbo[swapchain]->getDimensions().y), true, glm::vec4(0.0f));

		// transform from color-attachment write to shader read
		api::MemoryBarrierSet barriers;
		barriers.addBarrier(api::ImageAreaBarrier(types::AccessFlags::ColorAttachmentWrite, types::AccessFlags::ShaderRead,
		                    apiObj->blurPass0.fbo[swapchain]->getColorAttachment(0)->getResource(),
		                    types::ImageSubresourceRange(), types::ImageLayout::ColorAttachmentOptimal,
		                    types::ImageLayout::ShaderReadOnlyOptimal));

		cmd->pipelineBarrier(types::PipelineStageFlags::TopOfPipeline, types::PipelineStageFlags::TopOfPipeline, barriers);

		cmd->bindPipeline(apiObj->blurPass1.pipeline);
		cmd->bindDescriptorSet(apiObj->blurPass1.pipeline->getPipelineLayout(), 0, apiObj->blurPass1.texDescSet[swapchain], 0);
		cmd->bindDescriptorSet(apiObj->blurPass1.pipeline->getPipelineLayout(), 1, apiObj->blurPass1.uboPerVert.second, 0);
		drawAxisAlignedQuad(cmd);

		barriers.clearAllBarriers();

		// transform back to color-attachment write from shader read
		barriers.addBarrier(api::ImageAreaBarrier(types::AccessFlags::ShaderRead, types::AccessFlags::ColorAttachmentWrite,
		                    apiObj->blurPass0.fbo[swapchain]->getColorAttachment(0)->getResource(),
		                    types::ImageSubresourceRange(), types::ImageLayout::ShaderReadOnlyOptimal,
		                    types::ImageLayout::ColorAttachmentOptimal));

		cmd->pipelineBarrier(types::PipelineStageFlags::TopOfPipeline, types::PipelineStageFlags::TopOfPipeline, barriers);

		cmd->endRenderPass();
	}

	//Draw scene with bloom
	{
		// set a barrier so that the bloom configs updates are properly synchronized.
		api::MemoryBarrierSet barriers;
		barriers.addBarrier(api::BufferRangeBarrier(types::AccessFlags::HostWrite, types::AccessFlags::ShaderRead,
		                    apiObj->postBloomPass.uboBloomConfig.first->getResource(), 0,
		                    apiObj->postBloomPass.uboBloomConfig.first->getRange()));

		cmd->pipelineBarrier(types::PipelineStageFlags::TopOfPipeline, types::PipelineStageFlags::TopOfPipeline, barriers);


		cmd->beginRenderPass(apiObj->onScreenFbo[swapchain], pvr::Rectanglei(0, 0, getWidth(), getHeight()), true,
		                     glm::vec4(.00, 0.70, 0.67, 0.f));

		barriers.clearAllBarriers();

		barriers.addBarrier(api::ImageAreaBarrier(types::AccessFlags::ColorAttachmentWrite, types::AccessFlags::ShaderRead,
		                    apiObj->preBloomPass.fbo[swapchain]->getColorAttachment(0)->getResource(),
		                    types::ImageSubresourceRange(), types::ImageLayout::ColorAttachmentOptimal,
		                    types::ImageLayout::ShaderReadOnlyOptimal));

		barriers.addBarrier(api::ImageAreaBarrier(types::AccessFlags::ColorAttachmentWrite, types::AccessFlags::ShaderRead,
		                    apiObj->blurPass0.fbo[swapchain]->getColorAttachment(0)->getResource(),
		                    types::ImageSubresourceRange(), types::ImageLayout::ColorAttachmentOptimal,
		                    types::ImageLayout::ShaderReadOnlyOptimal));

		cmd->pipelineBarrier(types::PipelineStageFlags::TopOfPipeline, types::PipelineStageFlags::TopOfPipeline, barriers);

		cmd->bindPipeline(apiObj->postBloomPass.pipeline);
		cmd->bindDescriptorSet(apiObj->postBloomPass.pipeline->getPipelineLayout(), 0,
		                       apiObj->postBloomPass.texDescSet[swapchain]);
		cmd->bindDescriptorSet(apiObj->postBloomPass.pipeline->getPipelineLayout(), 1,
		                       apiObj->postBloomPass.uboBloomConfig.second);
		cmd->bindDescriptorSet(apiObj->postBloomPass.pipeline->getPipelineLayout(), 2,
		                       apiObj->postBloomPass.uboMVP.second);
		drawAxisAlignedQuad(cmd);

		barriers.clearAllBarriers();

		barriers.addBarrier(api::ImageAreaBarrier(types::AccessFlags::ShaderRead, types::AccessFlags::ColorAttachmentWrite,
		                    apiObj->preBloomPass.fbo[swapchain]->getColorAttachment(0)->getResource(),
		                    types::ImageSubresourceRange(), types::ImageLayout::ShaderReadOnlyOptimal,
		                    types::ImageLayout::ColorAttachmentOptimal));

		barriers.addBarrier(api::ImageAreaBarrier(types::AccessFlags::ShaderRead, types::AccessFlags::ColorAttachmentWrite,
		                    apiObj->blurPass0.fbo[swapchain]->getColorAttachment(0)->getResource(),
		                    types::ImageSubresourceRange(), types::ImageLayout::ShaderReadOnlyOptimal,
		                    types::ImageLayout::ColorAttachmentOptimal));

		cmd->pipelineBarrier(types::PipelineStageFlags::TopOfPipeline, types::PipelineStageFlags::TopOfPipeline, barriers);
	}

	recordCommandUIRenderer(cmd, swapchain);

	cmd->endRenderPass();
}

/*!********************************************************************************************
\return	Return auto ptr to the demo supplied by the user
\brief	This function must be implemented by the user of the shell.
The user should return its Shell object defining the behaviour of the application.
***********************************************************************************************/
std::auto_ptr<Shell> pvr::newDemo() { return std::auto_ptr<Shell>(new VulkanPostProcessing()); }
