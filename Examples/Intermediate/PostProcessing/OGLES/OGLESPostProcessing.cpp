/*!********************************************************************************************
\File         OGLESBloom.cpp
\Title        Bloom
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Shows how to do a bloom effect
***********************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVRUIRenderer/PVRUIRenderer.h"

using namespace pvr::api;
using namespace pvr::types;
pvr::utils::VertexBindings_Name VertexBindings[] =
{
	{ "POSITION", "inVertex" },
	{ "NORMAL", "inNormal" },
	{ "UV0", "inTexCoord" },
};

namespace FboPass {
enum Enum {OnScreen, RenderScene, BlurFbo0, BlurFbo1, Count, NumBlurFbo = 2};
}

namespace QuadAttribute {
enum  Enum
{
	Position,
	TexCoord
};
}

/**********************************************************************************************
Consts
**********************************************************************************************/
const glm::vec4 LightPos(-1.5f, 0.0f, 10.0f, 0.0);
const pvr::uint32 TexSize = 256;    // Blur render target size (power-of-two)

/**********************************************************************************************
Content file names
***********************************************************************************************/
const char FragShaderSrcFile[]			= "FragShader.fsh";
const char VertShaderSrcFile[]			= "VertShader.vsh";
const char PreBloomFragShaderSrcFile[]	= "PreBloomFragShader.fsh";
const char PreBloomVertShaderSrcFile[]	= "PreBloomVertShader.vsh";
const char PostBloomFragShaderSrcFile[]	= "PostBloomFragShader.fsh";
const char PostBloomVertShaderSrcFile[]	= "PostBloomVertShader.vsh";
const char BlurFragSrcFile[]			= "BlurFragShader.fsh";
const char BlurVertSrcFile[]			= "BlurVertShader.vsh";

// PVR texture files
const char BaseTexFile[]				= "Marble.pvr";
// POD scene files
const char SceneFile[]					= "scene.pod";

/*!********************************************************************************************
Class implementing the pvr::Shell functions.
***********************************************************************************************/
class OGLESBloom : public pvr::Shell
{
	struct FrameBuffer
	{
		pvr::api::Fbo fbo;
		pvr::api::TextureView renderTex;
		pvr::api::TextureView depthTex;
		pvr::Rectanglei renderArea;
	};

	struct DeviceResources
	{
		// OpenGL handles for shaders, textures and VBOs
		pvr::api::GraphicsPipeline basePipe;
		pvr::api::GraphicsPipeline preBloomPipe;
		pvr::api::GraphicsPipeline postBloomPipe;
		pvr::api::GraphicsPipeline blurPipe;

		std::vector<pvr::api::Buffer> vbos;
		std::vector<pvr::api::Buffer> ibos;

		FrameBuffer fbo[FboPass::Count];

		pvr::api::TextureView baseTex;
		pvr::api::TextureView bloomMapTex;
		pvr::api::Sampler   samplerRepeat;
		pvr::api::Sampler   samplerClamp;

		pvr::api::Buffer		quadVbo;
		pvr::api::Buffer		quadIbo;
		pvr::api::DescriptorSet descSetRenderPass;
		pvr::api::DescriptorSet descSetFilterPass;
		pvr::api::DescriptorSet descSetBlurPass[2];
		pvr::api::DescriptorSet descSetPostBloom;

		pvr::api::CommandBuffer cmdBuffer;
		pvr::api::SecondaryCommandBuffer cmdBufferUIRenderer;
		pvr::api::DescriptorSetLayout texSamplerPipeLayout;
	};

	std::auto_ptr<DeviceResources> deviceResource;


	// Print3D class used to display text
	pvr::ui::UIRenderer	uiRenderer;

	// 3D Model
	pvr::assets::ModelHandle scene;

	pvr::float32 bloomIntensity;
	bool applyBloom;
	bool drawObject;
	bool animating;

	pvr::float32 rotation;
	pvr::api::AssetStore assetManager;
	// Group shader programs and their uniform locations together
	struct
	{
		pvr::uint32 mvpLoc;
		pvr::uint32 mvInvLoc;
		pvr::uint32 lightDirLoc;
		pvr::uint32 shininess;
	}
	basicProgUniform;

	struct
	{
		pvr::uint32 texOffsetX;
		pvr::uint32 texOffsetY;
		pvr::int32 mvpMtx;
	}
	blurProgUnifom;

	struct
	{
		pvr::int32 mvpMtx;
		pvr::uint32 texFactor;
		pvr::uint32 blurTexFactor;
	}
	postBloomProgUniform;

	struct
	{
		pvr::uint32 bloomIntensity;
		pvr::uint32 mvpLoc;
	}
	preBloomProgUniform;

	struct DrawPass
	{
		glm::vec3 lightPos;
		glm::mat4 mvp;
		glm::mat4 mvInv;
		pvr::float32 texelOffset;

	};
	DrawPass passDrawMesh;
	DrawPass passBloom;

	pvr::GraphicsContext context;
	glm::mat4 world, view, proj;
public:
	OGLESBloom() : bloomIntensity(1.f) {}

	virtual pvr::Result::Enum initApplication();
	virtual pvr::Result::Enum initView();
	virtual pvr::Result::Enum releaseView();
	virtual pvr::Result::Enum quitApplication();
	virtual pvr::Result::Enum renderFrame();

	bool createTextureDescriptor();
	bool createPipeline();
	bool loadVbos();
	bool createBlurFbo();
	bool createOnScreenFbo()
	{
		deviceResource->fbo[FboPass::OnScreen].renderArea = pvr::Rectanglei(0, 0, getWidth(), getHeight());
		deviceResource->fbo[FboPass::OnScreen].fbo = context->createOnScreenFbo(0, LoadOp::Clear);
		return true;
	}

	// create fbo for rendering the screen
	bool createRenderFbo();

	void updateSubtitleText();
	void drawMesh(int i32NodeIndex, pvr::api::CommandBuffer& cmdBuffer);

	void drawAxisAlignedQuad(pvr::float32 scaleX, pvr::float32 scaleY, const pvr::int32& scaleMtxUniformLoc,
	                         pvr::api::CommandBuffer& cmdBuffer);

	void eventMappedInput(pvr::SimplifiedInput::Enum e);

	void updateAnimation();
	void recordCommandBuffer();
};

/*!********************************************************************************************
\return	Return true if no error occurred
\brief	Loads the textures required for this training course
***********************************************************************************************/
bool OGLESBloom::createTextureDescriptor()
{
	// Load Textures
	if (!assetManager.getTextureWithCaching(getGraphicsContext(), BaseTexFile, &deviceResource->baseTex, NULL))
	{
		setExitMessage("FAILED to load texture %s.", BaseTexFile);
		return false;
	}

	// sampler repeat
	pvr::assets::SamplerCreateParam samplerDesc;
	samplerDesc.minificationFilter = SamplerFilter::Linear;
	samplerDesc.mipMappingFilter = SamplerFilter::Nearest;
	samplerDesc.magnificationFilter = SamplerFilter::Linear;
	samplerDesc.wrapModeU = SamplerWrap::Repeat;
	samplerDesc.wrapModeV = SamplerWrap::Repeat;
	deviceResource->samplerRepeat = context->createSampler(samplerDesc);

	// sampler clamp
	samplerDesc.wrapModeU = SamplerWrap::Clamp;
	samplerDesc.wrapModeV = SamplerWrap::Clamp;
	deviceResource->samplerClamp = context->createSampler(samplerDesc);

	// render pass descriptor set (albedo texture)
	pvr::api::DescriptorSetUpdate descCreateParam;
	descCreateParam.setCombinedImageSampler(0, deviceResource->baseTex, deviceResource->samplerRepeat);
	deviceResource->descSetRenderPass = context->createDescriptorSetOnDefaultPool(deviceResource->texSamplerPipeLayout);
	deviceResource->descSetRenderPass->update(descCreateParam);

	// pre-bloom pass descriptor set (render texture)
	descCreateParam.setCombinedImageSampler(0, deviceResource->fbo[FboPass::RenderScene].renderTex, deviceResource->samplerClamp);

	deviceResource->descSetFilterPass = context->createDescriptorSetOnDefaultPool(deviceResource->texSamplerPipeLayout);
	deviceResource->descSetFilterPass->update(descCreateParam);

	// blur pass0 descriptor set (blur pass1 texture)
	descCreateParam.setCombinedImageSampler(0, deviceResource->fbo[FboPass::BlurFbo1].renderTex, deviceResource->samplerClamp);

	deviceResource->descSetBlurPass[0] = context->createDescriptorSetOnDefaultPool(deviceResource->texSamplerPipeLayout);
	deviceResource->descSetBlurPass[0]->update(descCreateParam);

	// blur pass1 descriptor set (blur pass0 texture)
	descCreateParam.setCombinedImageSampler(0, deviceResource->fbo[FboPass::BlurFbo0].renderTex, deviceResource->samplerClamp);

	deviceResource->descSetBlurPass[1] = context->createDescriptorSetOnDefaultPool(deviceResource->texSamplerPipeLayout);
	deviceResource->descSetBlurPass[1]->update(descCreateParam);

	// post bloom
	descCreateParam.setCombinedImageSampler(0, deviceResource->fbo[FboPass::RenderScene].renderTex, deviceResource->samplerClamp);

	descCreateParam.setCombinedImageSampler(1, deviceResource->fbo[FboPass::BlurFbo0].renderTex, deviceResource->samplerClamp);

	deviceResource->descSetPostBloom = context->createDescriptorSetOnDefaultPool(deviceResource->texSamplerPipeLayout);
	deviceResource->descSetPostBloom->update(descCreateParam);

	return true;
}

/*!********************************************************************************************
\brief	Loads and compiles the shaders and links the shader programs
\return	Return true if no error occurred required for this training course
***********************************************************************************************/
bool OGLESBloom::createPipeline()
{
	pvr::api::DescriptorSetLayoutCreateParam layoutDesc;
	layoutDesc.setBinding(0, DescriptorType::CombinedImageSampler, 1, ShaderStageFlags::Fragment);
	deviceResource->texSamplerPipeLayout = context->createDescriptorSetLayout(layoutDesc);

	pvr::api::GraphicsPipelineCreateParam basePipe;
	basePipe.colorBlend.addAttachmentState(pvr::api::pipelineCreation::ColorBlendAttachmentState(false));
	basePipe.depthStencil.setDepthTestEnable(true);
	basePipe.depthStencil.setDepthWrite(true);
	pvr::api::VertexAttributeInfo quadAttributes[2] =
	{
		pvr::api::VertexAttributeInfo(QuadAttribute::Position, DataType::Float32, 2, 0, "inVertex"),
		pvr::api::VertexAttributeInfo(QuadAttribute::TexCoord, DataType::Float32, 2, sizeof(pvr::float32) * 8, "inTexCoord")
	};

	pvr::assets::ShaderFile shaderVersioning;
	const pvr::assets::Mesh& mesh = scene->getMesh(0);

	// create render scene pipeline
	{
		pvr::api::GraphicsPipelineCreateParam basicPipeDesc = basePipe;
		shaderVersioning.populateValidVersions(VertShaderSrcFile, *this);
		basicPipeDesc.vertexShader.setShader(context->createShader(*shaderVersioning.getBestStreamForApi(context->getApiType()), ShaderType::VertexShader));

		shaderVersioning.populateValidVersions(FragShaderSrcFile, *this);
		basicPipeDesc.fragmentShader.setShader(context->createShader(*shaderVersioning.getBestStreamForApi(context->getApiType()), ShaderType::FragmentShader));

		pvr::utils::createInputAssemblyFromMesh(mesh, VertexBindings, 3, basicPipeDesc);

		pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(deviceResource->texSamplerPipeLayout);
		basicPipeDesc.pipelineLayout = context->createPipelineLayout(pipeLayoutInfo);

		deviceResource->basePipe = context->createGraphicsPipeline(basicPipeDesc);

		if (deviceResource->basePipe.isValid() == false)
		{
			this->setExitMessage("Failed To Create Basic Pipeline");
			return false;
		}

		// Store the location of uniforms for later use
		basicProgUniform.mvpLoc = deviceResource->basePipe->getUniformLocation("MVPMatrix");
		basicProgUniform.mvInvLoc = deviceResource->basePipe->getUniformLocation("MVInv");
		basicProgUniform.lightDirLoc = deviceResource->basePipe->getUniformLocation("LightDirection");
		basicProgUniform.shininess = deviceResource->basePipe->getUniformLocation("Shininess");
	}

	// create prebloom pipeline
	{
		pvr::api::GraphicsPipelineCreateParam prebloomPipeDesc = basePipe;

		shaderVersioning.populateValidVersions(PreBloomVertShaderSrcFile, *this);
		prebloomPipeDesc.vertexShader.setShader(context->createShader(*shaderVersioning.getBestStreamForApi(context->getApiType()), ShaderType::VertexShader));

		shaderVersioning.populateValidVersions(PreBloomFragShaderSrcFile, *this);
		prebloomPipeDesc.fragmentShader.setShader(context->createShader(*shaderVersioning.getBestStreamForApi(context->getApiType()), ShaderType::FragmentShader));
		prebloomPipeDesc.vertexInput.setInputBinding(0, 0).addVertexAttribute(0, quadAttributes[0]).addVertexAttribute(0, quadAttributes[1]);

		pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(deviceResource->texSamplerPipeLayout);
		prebloomPipeDesc.pipelineLayout = context->createPipelineLayout(pipeLayoutInfo);
		deviceResource->preBloomPipe = context->createGraphicsPipeline(prebloomPipeDesc);

		if (deviceResource->preBloomPipe.isValid() == false)
		{
			this->setExitMessage("Failed to Create preBloom pipeline");
			return false;
		}

		// Store the location of uniforms for later use
		preBloomProgUniform.bloomIntensity = deviceResource->preBloomPipe->getUniformLocation("BloomIntensity");
		preBloomProgUniform.mvpLoc = deviceResource->preBloomPipe->getUniformLocation("MVPMatrix");
	}

	//   Blur Pipeline
	{
		GraphicsPipelineCreateParam blurPipeDesc;
		blurPipeDesc.colorBlend.addAttachmentState(pvr::api::pipelineCreation::ColorBlendAttachmentState(false));
		blurPipeDesc.depthStencil.setDepthTestEnable(false).setDepthWrite(false);

		shaderVersioning.populateValidVersions(BlurVertSrcFile, *this);
		blurPipeDesc.vertexShader.setShader(context->createShader(*shaderVersioning.getBestStreamForApi(context->getApiType()), ShaderType::VertexShader));

		shaderVersioning.populateValidVersions(BlurFragSrcFile, *this);
		blurPipeDesc.fragmentShader.setShader(context->createShader(*shaderVersioning.getBestStreamForApi(context->getApiType()), ShaderType::FragmentShader));
		blurPipeDesc.vertexInput.setInputBinding(0, 0).addVertexAttribute(0, quadAttributes[0]).addVertexAttribute(0, quadAttributes[1]);

		pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(deviceResource->texSamplerPipeLayout);
		blurPipeDesc.pipelineLayout = context->createPipelineLayout(pipeLayoutInfo);
		deviceResource->blurPipe = context->createGraphicsPipeline(blurPipeDesc);

		if (deviceResource->blurPipe.isValid() == false)
		{
			this->setExitMessage("Failed to Create Blur pipeline");
			return false;
		}

		blurProgUnifom.texOffsetX = deviceResource->blurPipe->getUniformLocation("TexelOffsetX");
		blurProgUnifom.texOffsetY = deviceResource->blurPipe->getUniformLocation("TexelOffsetY");
		blurProgUnifom.mvpMtx = deviceResource->blurPipe->getUniformLocation("MVPMatrix");
	}

	// create Post-Bloom Pipeline
	{
		GraphicsPipelineCreateParam postbloomPipeDesc;
		pipelineCreation::ColorBlendAttachmentState attachmentState(false, BlendFactor::One, BlendFactor::One, BlendOp::Add);
		postbloomPipeDesc.colorBlend.addAttachmentState(attachmentState);
		postbloomPipeDesc.rasterizer.setCullFace(Face::Back);
		postbloomPipeDesc.depthStencil.setDepthTestEnable(false).setDepthWrite(false);

		shaderVersioning.populateValidVersions(PostBloomVertShaderSrcFile, *this);
		postbloomPipeDesc.vertexShader.setShader(context->createShader(*shaderVersioning.getBestStreamForApi(context->getApiType()), ShaderType::VertexShader));

		shaderVersioning.populateValidVersions(PostBloomFragShaderSrcFile, *this);
		postbloomPipeDesc.fragmentShader.setShader(context->createShader(*shaderVersioning.getBestStreamForApi(context->getApiType()), ShaderType::FragmentShader));

		postbloomPipeDesc.vertexInput.setInputBinding(0, 0, StepRate::Vertex).addVertexAttribute(0, quadAttributes[0]).addVertexAttribute(0, quadAttributes[1]);

		pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(deviceResource->texSamplerPipeLayout);
		postbloomPipeDesc.pipelineLayout = context->createPipelineLayout(pipeLayoutInfo);
		deviceResource->postBloomPipe = context->createGraphicsPipeline(postbloomPipeDesc);
		postBloomProgUniform.mvpMtx = deviceResource->postBloomPipe->getUniformLocation("MVPMatrix");
		postBloomProgUniform.texFactor = deviceResource->postBloomPipe->getUniformLocation("sTexFactor");
		postBloomProgUniform.blurTexFactor = deviceResource->postBloomPipe->getUniformLocation("sBlurTexFactor");
		if (deviceResource->postBloomPipe.isValid() == false)
		{
			this->setExitMessage("Failed to Create postBloom pipeline");
			return false;
		}
	}

	deviceResource->cmdBuffer->beginRecording();

	// Set the sampler2D variable to the first texture unit
	deviceResource->cmdBuffer->bindPipeline(deviceResource->preBloomPipe);
	deviceResource->cmdBuffer->setUniform<pvr::int32>(deviceResource->preBloomPipe->getUniformLocation("sTexture"), 0);

	// Set the sampler2D variable to the first texture unit
	deviceResource->cmdBuffer->bindPipeline(deviceResource->blurPipe);
	deviceResource->cmdBuffer->setUniform<pvr::int32>(deviceResource->blurPipe->getUniformLocation("sTexture"), 0);

	// Set the sampler2D variable to the first texture unit
	deviceResource->cmdBuffer->bindPipeline(deviceResource->basePipe);
	deviceResource->cmdBuffer->setUniform<pvr::int32>(deviceResource->basePipe->getUniformLocation("sTexture"), 0);

	// Set the sampler2D variable to the first texture unit
	deviceResource->cmdBuffer->bindPipeline(deviceResource->postBloomPipe);
	deviceResource->cmdBuffer->setUniform<pvr::int32>(deviceResource->postBloomPipe->getUniformLocation("sTexture"), 0);
	deviceResource->cmdBuffer->setUniform<pvr::int32>(deviceResource->postBloomPipe->getUniformLocation("sBlurTexture"), 1);

	deviceResource->cmdBuffer->endRecording();
	deviceResource->cmdBuffer->submit();
	return true;
}

/*!****************************************************************************
\brief	Loads the mesh data required for this training course into vertex buffer objects
******************************************************************************/
bool OGLESBloom::loadVbos()
{

	// Load vertex data of all meshes in the scene into VBOs
	// The meshes have been exported with the "Interleave Vectors" option,
	// so all data is interleaved in the buffer at pMesh->pInterleaved.
	// Interleaving data improves the memory access pattern and cache efficiency,
	// thus it can be read faster by the hardware.

	pvr::utils::appendSingleBuffersFromModel(getGraphicsContext(), *scene, deviceResource->vbos, deviceResource->ibos);

	const pvr::float32 halfDim = 1.f;
	// create quad vertices..
	const pvr::float32 afVertexData[] =
	{
		-halfDim, halfDim, // top left
		-halfDim, -halfDim,// bottom left
		halfDim, -halfDim,//  bottom right
		halfDim, halfDim,// top right

		// texCoords
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
	};

	pvr::uint16 indices[] = { 1, 2, 0, 0, 2, 3 };
	auto i  = sizeof(afVertexData);
	deviceResource->quadVbo = context->createBuffer(sizeof(afVertexData), BufferBindingUse::VertexBuffer);
	deviceResource->quadVbo->update(afVertexData, 0, sizeof(afVertexData));

	deviceResource->quadIbo = context->createBuffer(sizeof(indices), BufferBindingUse::IndexBuffer);

	deviceResource->quadIbo->update(indices, 0, sizeof(indices));
	std::string apiError;
	if (pvr::api::checkApiError(&apiError))
	{
		this->setExitMessage("Failed to create the VBOs");
		return false;
	}
	return true;
}

/*!********************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in initApplication() will be called by pvr::Shell once per run, before the rendering
		context is created.
		Used to initialize variables that are not dependent on it (e.g. external modules,
		loading meshes, etc.)
		If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************/
pvr::Result::Enum OGLESBloom::initApplication()
{
	// Apply bloom per default
	applyBloom = true;
	drawObject = true;
	animating = true;
	// Initial number of blur passes, can be changed during runtime
	rotation = 0.0f;

	// Texel offset for blur filter kernel
	passBloom.texelOffset = 1.0f / (pvr::float32)TexSize;
	// Altered weights for the faster filter kernel
	pvr::float32 w1 = 0.0555555f;
	pvr::float32 w2 = 0.2777777f;
	pvr::float32 intraTexelOffset = (w1 / (w1 + w2)) * passBloom.texelOffset;
	passBloom.texelOffset += intraTexelOffset;
	// Intensity multiplier for the bloom effect
	// Load the scene
	assetManager.init(*this);

	if (!assetManager.loadModel(SceneFile, scene))
	{
		this->setExitMessage("Error: Couldn't load the %s file\n", SceneFile);
		return pvr::Result::NotFound;
	}
	return pvr::Result::Success;
}

/*!********************************************************************************************
\return	Return  pvr::Result::Success if no error occured
\brief	Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.
quitApplication() will not be called every time the rendering context is lost, only before application exit.
***********************************************************************************************/
pvr::Result::Enum OGLESBloom::quitApplication()
{
	//Instructs the Asset Manager to free all resources
	assetManager.releaseAll();
	return pvr::Result::Success;
}

/*!********************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in initView() will be called by pvr::Shell upon initialization or after a change
		in the rendering context. Used to initialize variables that are dependent on the rendering
		context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************/
pvr::Result::Enum OGLESBloom::initView()
{
	context = getGraphicsContext();
	deviceResource.reset(new DeviceResources());
	deviceResource->cmdBuffer = context->createCommandBufferOnDefaultPool();
	deviceResource->cmdBufferUIRenderer = context->createSecondaryCommandBufferOnDefaultPool();

	//	Initialize VBO data
	if (!loadVbos()) {  return pvr::Result::NotInitialized;  }

	//	Load and compile the shaders & link programs
	if (!createPipeline()) {   return pvr::Result::NotInitialized;   }

	if (!createOnScreenFbo() || !createRenderFbo() || !createBlurFbo())
	{
		return pvr::Result::NotInitialized;
	}

	//	Load textures
	if (!createTextureDescriptor()) {  return pvr::Result::NotInitialized; }

	if (uiRenderer.init(getGraphicsContext(), deviceResource->fbo->fbo->getRenderPass(), 0) != pvr::Result::Success)
	{
		setExitMessage("Error: Failed to initialize the UIRenderer\n");
		return pvr::Result::NotInitialized;
	}

	uiRenderer.getDefaultTitle()->setText("Bloom");
	uiRenderer.getDefaultTitle()->commitUpdates();
	uiRenderer.getDefaultControls()->setText(
	    "Left / right: Rendering mode\n"
	    "Up / down: Bloom intensity\n"
	    "Action:     Pause\n"
	);
	uiRenderer.getDefaultControls()->commitUpdates();
	updateSubtitleText();
	pvr::float32 fov;
	glm::vec3 from, to, up;
	scene->getCameraProperties(0, fov, from, to, up);
	view = glm::lookAt(from, to, up);

	bool bRotate = isFullScreen() && isScreenRotated();
	if (bRotate)
	{
		proj = pvr::math::perspectiveFov(getApiType(), fov, (float)getHeight(), (float)getWidth(), scene->getCamera(0).getNear(),
		                                 scene->getCamera(0).getFar(), glm::pi<pvr::float32>() * .5f);
	}
	else
	{
		proj = glm::perspectiveFov<glm::float32>(fov, (float)getWidth(), (float)getHeight(),
		        scene->getCamera(0).getNear(), scene->getCamera(0).getFar());
	}
	updateSubtitleText();
	return pvr::Result::Success;
}

/*!********************************************************************************************
\brief Create render fbo for rendering the scene
\return	Return true if success
***********************************************************************************************/
bool OGLESBloom::createRenderFbo()
{
	pvr::api::ImageStorageFormat depthTexFormat(pvr::PixelFormat::Depth16, 1, ColorSpace::lRGB, pvr::VariableType::Float);
	pvr::api::ImageStorageFormat colorTexFormat(pvr::PixelFormat::RGBA_8888, 1, ColorSpace::lRGB, pvr::VariableType::UnsignedByteNorm);
	pvr::api::TextureStore depthTexture, colorTexture;
	pvr::api::TextureView  depthTexView, colorTexView;

	// create depth and color texture
	depthTexture = context->createTexture();
	depthTexture->allocate2D(depthTexFormat, getWidth(), getHeight());
	depthTexView = context->createTextureView(depthTexture);

	colorTexture = context->createTexture();
	colorTexture->allocate2D(colorTexFormat, getWidth(), getHeight());
	colorTexView = context->createTextureView(colorTexture);


	// create the render pass.
	pvr::api::RenderPassCreateParam renderPassInfo;
	pvr::api::RenderPassColorInfo colorInfo(colorTexFormat, LoadOp::Clear);
	pvr::api::RenderPassDepthStencilInfo dsInfo(depthTexFormat, LoadOp::Clear, StoreOp::Store);

	pvr::api::SubPass subPass;
	subPass.setColorAttachment(0);// use the first color attachment
	renderPassInfo.addSubPass(0, subPass);
	renderPassInfo.setDepthStencilInfo(dsInfo);
	renderPassInfo.addColorInfo(0, colorInfo);

	pvr::api::FboCreateParam fboInfo;
	fboInfo.setRenderPass(context->createRenderPass(renderPassInfo));

	fboInfo.addColor(0, colorTexView);
	fboInfo.setDepthStencil(depthTexView);
	deviceResource->fbo[FboPass::RenderScene].fbo = context->createFbo(fboInfo);
	deviceResource->fbo[FboPass::RenderScene].renderTex = colorTexView;
	deviceResource->fbo[FboPass::RenderScene].depthTex = depthTexView;
	deviceResource->fbo[FboPass::RenderScene].renderArea = pvr::Rectanglei(0, 0, getWidth(), getHeight());
	if (!deviceResource->fbo[FboPass::RenderScene].fbo.isValid())
	{
		pvr::Log("Failed to create rendering fbo");
		return false;
	}
	return true;
}

/*!********************************************************************************************
\brief	Create the blur fbo
\return	Return  true on success
***********************************************************************************************/
bool OGLESBloom::createBlurFbo()
{
	pvr::api::ImageStorageFormat colorTexFormat(pvr::PixelFormat::RGB_888, 1, ColorSpace::lRGB, pvr::VariableType::UnsignedByteNorm);

	// create the render passes.
	pvr::api::RenderPassCreateParam blurRenderPassDesc;
	pvr::api::RenderPassColorInfo colorInfo(colorTexFormat, LoadOp::Clear);
	pvr::api::SubPass subPass;
	subPass.setColorAttachment(0);// use the first color attachment
	blurRenderPassDesc.addColorInfo(0, colorInfo);
	blurRenderPassDesc.addSubPass(0, subPass);
	pvr::api::RenderPass blurRenderPass = context->createRenderPass(blurRenderPassDesc);

	for (pvr::uint32 i = 0; i < FboPass::NumBlurFbo; i++)
	{
		pvr::api::TextureStore tex = context->createTexture();
		tex->allocate2D(colorTexFormat, TexSize, TexSize);
		deviceResource->fbo[FboPass::BlurFbo0 + i].renderTex = context->createTextureView(tex);

		pvr::api::FboCreateParam blurFboDesc;
		blurFboDesc.setRenderPass(blurRenderPass);
		blurFboDesc.addColor(0, deviceResource->fbo[FboPass::BlurFbo0 + i].renderTex);
		// The first render target needs a depth buffer, as we have to draw "blooming" 3d objects into it
		deviceResource->fbo[FboPass::BlurFbo0 + i].fbo = context->createFbo(blurFboDesc);
		if (!deviceResource->fbo[FboPass::BlurFbo0 + i].fbo.isValid())
		{
			pvr::Log("Failed to create blur fbo % d", i);
			return false;
		}
		deviceResource->fbo[FboPass::BlurFbo0 + i].renderArea = pvr::Rectanglei(0, 0, TexSize, TexSize);
	}
	return true;
}

/*!********************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in releaseView() will be called by pvr::Shell when the application quits or before
a change in the rendering context.
***********************************************************************************************/
pvr::Result::Enum OGLESBloom::releaseView()
{
	uiRenderer.release();
	scene.release();
	assetManager.releaseAll();
	deviceResource.release();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief Update the animation
***********************************************************************************************************************/
void OGLESBloom::updateAnimation()
{
	// Calculate the mask and light rotation based on the passed time
	pvr::float32 const twoPi = glm::pi<pvr::float32>() * 2.f;

	if (animating)
	{
		rotation += glm::pi<pvr::float32>() * getFrameTime() * 0.0002f;
		// wrap it
		if (rotation > twoPi) { rotation -= twoPi; }
	}

	// Calculate the model, view and projection matrix
	world = glm::rotate((-rotation), glm::vec3(0.f, 1.f, 0.f)) * glm::scale(glm::vec3(1.65f));

	pvr::float32 fov;
	fov = scene->getCamera(0).getFOV(0);

	glm::mat4x4 viewProj = proj * view;
	// Simple rotating directional light in model-space);
	passDrawMesh.lightPos = glm::vec3(glm::normalize(glm::inverse(world) * LightPos));
	passDrawMesh.mvInv = glm::inverse(view * world * scene->getWorldMatrix(scene->getNode(0).getObjectId()));
	passDrawMesh.mvp = viewProj * world * scene->getWorldMatrix(scene->getNode(0).getObjectId());
}

/*!********************************************************************************************
\return	Return Result::Suceess if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************/
pvr::Result::Enum OGLESBloom::renderFrame()
{
	updateAnimation();
	deviceResource->cmdBuffer->submit();
	return pvr::Result::Success;
}

/*!********************************************************************************************
\brief	update the subtitle sprite
***********************************************************************************************/
void OGLESBloom::updateSubtitleText()
{
	if (applyBloom)
	{
		if (drawObject)
		{
			uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("Object with bloom effect, intensity % 2.1f", bloomIntensity));
		}
		else
		{
			uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("Bloom effect textures, intensity % 2.1f", bloomIntensity));
		}
	}
	else
	{
		if (drawObject)
		{
			uiRenderer.getDefaultDescription()->setText("Object without bloom");
		}
		else
		{
			uiRenderer.getDefaultDescription()->setText("Use up - down to draw object and / or bloom textures");
		}
	}
	uiRenderer.getDefaultDescription()->commitUpdates();
	recordCommandBuffer();// re-record the command buffer
}

/*!********************************************************************************************
\brief	Handles user input and updates live variables accordingly.
***********************************************************************************************/
void OGLESBloom::eventMappedInput(pvr::SimplifiedInput::Enum e)
{
	static int mode = 0;
	//Object+Bloom, object, bloom
	switch (e)
	{
	case pvr::SimplifiedInput::Left:
		if (--mode < 0) { mode = 2; }
		applyBloom = (mode != 1); drawObject = (mode != 2);
		updateSubtitleText();
		recordCommandBuffer();
		break;
	case pvr::SimplifiedInput::Right:
		++mode %= 3;
		applyBloom = (mode != 1); drawObject = (mode != 2);
		updateSubtitleText();
		recordCommandBuffer();
		break;
	case pvr::SimplifiedInput::Up:
		bloomIntensity = int(.5f + 10.f * std::min(bloomIntensity + .2f, 5.f)) * .1f;
		updateSubtitleText();
		recordCommandBuffer();
		break;
	case pvr::SimplifiedInput::Down:
		bloomIntensity = int(.5f + 10.f * std::max(bloomIntensity - .2f, 0.f)) * .1f;
		updateSubtitleText();
		recordCommandBuffer();
		break;
	case pvr::SimplifiedInput::ActionClose:
		this->exitShell();
		break;
	case pvr::SimplifiedInput::Action1:
	case pvr::SimplifiedInput::Action2:
	case pvr::SimplifiedInput::Action3:
		animating = !animating;
		break;
	default:
		break;
	}

}

/*!********************************************************************************************
\param	nodeIndex	Node index of the mesh to draw
\brief	Draws a pvr::Model::Mesh after the model view matrix has been set and the material prepared.
***********************************************************************************************/
void OGLESBloom::drawMesh(int nodeIndex, pvr::api::CommandBuffer& cmdBuffer)
{
	int meshIndex = scene->getNode(nodeIndex).getObjectId();
	const pvr::assets::Model::Mesh& mesh = scene->getMesh(meshIndex);
	// bind the VBO for the mesh
	cmdBuffer->bindVertexBuffer(deviceResource->vbos[meshIndex], 0, 0);
	// bind the index buffer, won't hurt if the handle is 0
	cmdBuffer->bindIndexBuffer(deviceResource->ibos[meshIndex], 0, mesh.getFaces().getDataType());

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
void OGLESBloom::drawAxisAlignedQuad(pvr::float32 scaleX, pvr::float32 scaleY, const pvr::int32& matrixUniformLoc,
                                     pvr::api::CommandBuffer& cmdBuffer)
{
	// construct the scale matrix
	glm::mat4 scaleMtx = glm::scale(glm::vec3(scaleX, scaleY, 1.0f));
	cmdBuffer->bindVertexBuffer(deviceResource->quadVbo, 0, 0);
	cmdBuffer->bindIndexBuffer(deviceResource->quadIbo, 0, IndexType::IndexType16Bit);
	cmdBuffer->setUniform<glm::mat4>(matrixUniformLoc, scaleMtx);
	cmdBuffer->drawIndexed(0, 6);
}

/*!********************************************************************************************
\brief	Record the command buffer
***********************************************************************************************/
void OGLESBloom::recordCommandBuffer()
{
	// draw the scene
	{
		deviceResource->cmdBuffer->beginRecording();
		// Simple rotating directional light in model-space
		deviceResource->cmdBuffer->beginRenderPass(deviceResource->fbo[FboPass::RenderScene].fbo,
		        deviceResource->fbo[FboPass::RenderScene].renderArea, true, glm::vec4(0.00, 0.70, 0.67, 0.f));

		// Use simple shader program to render the mask
		deviceResource->cmdBuffer->bindPipeline(deviceResource->basePipe);
		// bind the albedo texture
		deviceResource->cmdBuffer->bindDescriptorSet(
		    deviceResource->basePipe->getPipelineLayout(), 0, deviceResource->descSetRenderPass, 0);

		deviceResource->cmdBuffer->setUniform<pvr::float32>(basicProgUniform.shininess, .6f);
		deviceResource->cmdBuffer->setUniformPtr<glm::vec3>(basicProgUniform.lightDirLoc, 1, &passDrawMesh.lightPos);
		// Draw the mesh
		deviceResource->cmdBuffer->setUniformPtr<glm::mat4>(basicProgUniform.mvpLoc, 1, &passDrawMesh.mvp);
		deviceResource->cmdBuffer->setUniformPtr<glm::mat4>(basicProgUniform.mvInvLoc, 1, &passDrawMesh.mvInv);
		drawMesh(0, deviceResource->cmdBuffer);
		deviceResource->cmdBuffer->endRenderPass();
	}
	// not applying bloom, render the scene without bloom and return
	if (!applyBloom)
	{
		// Draw scene with bloom
		deviceResource->cmdBuffer->beginRenderPass(deviceResource->fbo[FboPass::OnScreen].fbo,
		        deviceResource->fbo[FboPass::OnScreen].renderArea, true, glm::vec4(0.0f));

		// bind the blurred texture
		deviceResource->cmdBuffer->bindDescriptorSet(
		    deviceResource->postBloomPipe->getPipelineLayout(), 0, deviceResource->descSetFilterPass, 0);

		// The following section will draw a quad on the screen where the post processing pixel
		// shader shall be executed.Try to minimize the area by only drawing where the actual
		// post processing should happen, as this is a very costly operation.
		deviceResource->cmdBuffer->bindPipeline(deviceResource->postBloomPipe);
		deviceResource->cmdBuffer->setUniform<pvr::float32>(postBloomProgUniform.texFactor, 1.f);
		deviceResource->cmdBuffer->setUniform<pvr::float32>(postBloomProgUniform.blurTexFactor, 0.f);
		drawAxisAlignedQuad(1, 1, postBloomProgUniform.mvpMtx, deviceResource->cmdBuffer);
	}
	else
	{
		// bloom
		{
			// filter the bright portion of the image
			deviceResource->cmdBuffer->beginRenderPass(deviceResource->fbo[FboPass::BlurFbo0].fbo,
			        deviceResource->fbo[FboPass::BlurFbo0].renderArea, true, glm::vec4(0.0f));
			deviceResource->cmdBuffer->bindPipeline(deviceResource->preBloomPipe);

			// bind the render texture
			deviceResource->cmdBuffer->bindDescriptorSet(
			    deviceResource->preBloomPipe->getPipelineLayout(), 0, deviceResource->descSetFilterPass, 0);

			deviceResource->cmdBuffer->setUniformPtr<pvr::float32>(preBloomProgUniform.bloomIntensity, 1, &bloomIntensity);
			drawAxisAlignedQuad(1, 1, preBloomProgUniform.mvpLoc, deviceResource->cmdBuffer);

			deviceResource->cmdBuffer->endRenderPass();

			// BLUR render pass
			{
				// Horizontal blur
				deviceResource->cmdBuffer->beginRenderPass(deviceResource->fbo[FboPass::BlurFbo1].fbo, deviceResource->fbo[FboPass::BlurFbo1].renderArea,
				        true, glm::vec4(0.0f));

				deviceResource->cmdBuffer->bindPipeline(deviceResource->blurPipe);
				deviceResource->cmdBuffer->bindDescriptorSet(deviceResource->blurPipe->getPipelineLayout(), 0, deviceResource->descSetBlurPass[1], 0);

				deviceResource->cmdBuffer->setUniformPtr<pvr::float32>(blurProgUnifom.texOffsetX, 1, &passBloom.texelOffset);
				deviceResource->cmdBuffer->setUniform<pvr::float32>(blurProgUnifom.texOffsetY, 0.0f);
				drawAxisAlignedQuad(1, 1, blurProgUnifom.mvpMtx, deviceResource->cmdBuffer);
				deviceResource->cmdBuffer->endRenderPass();

				// Vertical Blur
				deviceResource->cmdBuffer->beginRenderPass(deviceResource->fbo[FboPass::BlurFbo0].fbo, deviceResource->fbo[FboPass::BlurFbo0].renderArea, true, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));

				// bind the texture that we rendered in the horizontal pass
				deviceResource->cmdBuffer->bindDescriptorSet(deviceResource->blurPipe->getPipelineLayout(), 0, deviceResource->descSetBlurPass[0], 0);

				deviceResource->cmdBuffer->setUniform<pvr::float32>(blurProgUnifom.texOffsetX, 0.0f);
				deviceResource->cmdBuffer->setUniformPtr<pvr::float32>(blurProgUnifom.texOffsetY, 1, &passBloom.texelOffset);
				drawAxisAlignedQuad(1, 1, blurProgUnifom.mvpMtx, deviceResource->cmdBuffer);
				deviceResource->cmdBuffer->endRenderPass();
			}

			// Draw scene with bloom
			deviceResource->cmdBuffer->beginRenderPass(deviceResource->fbo[FboPass::OnScreen].fbo,
			        deviceResource->fbo[FboPass::OnScreen].renderArea, true, glm::vec4(0.f, 0.0f, 0.0f, 0.0f));

			// bind the blurred texture
			deviceResource->cmdBuffer->bindDescriptorSet(deviceResource->postBloomPipe->getPipelineLayout(), 0, deviceResource->descSetPostBloom, 0);

			// The following section will draw a quad on the screen where the post processing pixel
			// shader shall be executed.Try to minimize the area by only drawing where the actual
			// post processing should happen, as this is a very costly operation.
			deviceResource->cmdBuffer->bindPipeline(deviceResource->postBloomPipe);
			deviceResource->cmdBuffer->setUniform<pvr::float32>(postBloomProgUniform.blurTexFactor, 1.f);

			if (drawObject)
			{
				deviceResource->cmdBuffer->setUniform<pvr::float32>(postBloomProgUniform.texFactor, 1.f);
			}
			else  // Hide the object to show the bloom textures...
			{
				deviceResource->cmdBuffer->setUniform<pvr::float32>(postBloomProgUniform.texFactor, 0.f);
			}

			drawAxisAlignedQuad(1, 1, postBloomProgUniform.mvpMtx, deviceResource->cmdBuffer);

		}

	}

	//UIRENDERER
	{
		// record the commands
		deviceResource->cmdBufferUIRenderer->beginRecording(deviceResource->fbo[FboPass::OnScreen].fbo, 0);
		uiRenderer.beginRendering(deviceResource->cmdBufferUIRenderer);
		uiRenderer.getSdkLogo()->render();
		uiRenderer.getDefaultTitle()->render();
		uiRenderer.getDefaultControls()->render();
		uiRenderer.getDefaultDescription()->render();
		uiRenderer.endRendering();
		deviceResource->cmdBufferUIRenderer->endRecording();
		deviceResource->cmdBuffer->enqueueSecondaryCmds(deviceResource->cmdBufferUIRenderer);

		deviceResource->cmdBuffer->endRenderPass();
		deviceResource->cmdBuffer->endRecording();
	}
}

/*!********************************************************************************************
\return	Return auto ptr to the demo supplied by the user
\brief	This function must be implemented by the user of the shell.
The user should return its pvr::Shell object defining the behaviour of the application.
***********************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() { return std::auto_ptr<pvr::Shell>(new OGLESBloom()); }
