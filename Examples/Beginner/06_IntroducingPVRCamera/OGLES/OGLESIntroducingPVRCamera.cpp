/*!*********************************************************************************************************************
\File         OGLESIntroducingPVRCamera.cpp
\Title        Texture Streaming
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief  Demonstrates texture streaming using platform-specific functionality
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVRUIRenderer/PVRUIRenderer.h"
#include "PVRCamera/PVRCamera.h"

#if defined(__ANDROID__)
const char* ShaderDefines[] = { "ANDROID=1" };
int NumShaderDefines = 1;
#elif defined(TARGET_OS_IPHONE)
const char* ShaderDefines[] = { "IOS=1" };
int NumShaderDefines = 1;
#else
const char** ShaderDefines = NULL;
int NumShaderDefines = 0;
#endif

using namespace pvr;

/*!*********************************************************************************************************************
Class implementing the pvr::Shell functions.
***********************************************************************************************************************/
class OGLESIntroducingPVRCamera : public Shell
{
	api::Fbo onScreenFbo;
	api::Buffer vbo;
	api::Buffer ibo;
	api::GraphicsPipeline renderingPipeline;
	api::DescriptorSet descriptorSet;
	api::CommandBuffer commandBuffer;
	pvr::api::DescriptorSetLayout descriptorLayout;
	pvr::api::Sampler sampler;
	int32   uvTransformLocation;

	// Print3D class used to display text
	ui::UIRenderer uiRenderer;

	// Camera interface
	CameraInterface m_Camera;
public:
	virtual Result initApplication();
	virtual Result initView();
	virtual Result releaseView();
	virtual Result quitApplication();
	virtual Result renderFrame();

	void createBuffers();
	bool createPipelineAndDescriptors();
	void recordCommandBuffers();
};

void OGLESIntroducingPVRCamera::createBuffers()
{
	glm::vec2 VBOmem[] =
	{
		//POSITION,
		{ -1., -1. }, //0:BL
		{ 1., -1. },  //1:BR
		{ 1., 1. },   //2:TR
		{ -1., 1. },  //3:TL
	};
	int16 IBOmem[] =
	{
		0, 1, 2,
		0, 2, 3
	};

	vbo = getGraphicsContext()->createBuffer(sizeof(VBOmem), pvr::types::BufferBindingUse::VertexBuffer, true);
	ibo = getGraphicsContext()->createBuffer(sizeof(IBOmem), pvr::types::BufferBindingUse::IndexBuffer, true);

	vbo->update(VBOmem, 0, sizeof(VBOmem));
	ibo->update(IBOmem, 0, sizeof(IBOmem));
}

/*!*********************************************************************************************************************
\brief  Create pipeline and combined-image-sampler DescriptorSet
\return Return true if no error occurred
***********************************************************************************************************************/
bool OGLESIntroducingPVRCamera::createPipelineAndDescriptors()
{
	if (!m_Camera.initializeSession(HWCamera::Front, getWidth(), getHeight())) { return false; }
	pvr::api::DescriptorSetLayoutCreateParam descriptorLayoutDesc;

	pvr::assets::SamplerCreateParam desc;
	desc.magnificationFilter = pvr::types::SamplerFilter::Nearest;
	desc.minificationFilter = pvr::types::SamplerFilter::Nearest;
	desc.mipMappingFilter = pvr::types::SamplerFilter::None;
	sampler = getGraphicsContext()->createSampler(desc);
	if (m_Camera.hasRgbTexture())
	{
		descriptorLayoutDesc.setBinding(0, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
		descriptorLayout = getGraphicsContext()->createDescriptorSetLayout(descriptorLayoutDesc);
		pvr::api::DescriptorSetUpdate descSetCreateParam;
		descSetCreateParam.setCombinedImageSampler(0, getTextureFromPVRCameraHandle(getGraphicsContext(), m_Camera.getRgbTexture()), sampler);
		descriptorSet = getGraphicsContext()->createDescriptorSetOnDefaultPool(descriptorLayout);
		descriptorSet->update(descSetCreateParam);
	}
	else if (m_Camera.hasLumaChromaTextures()) // use the chrominance and luminance
	{
		descriptorLayoutDesc.setBinding(0, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment)
		.setBinding(1, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
		descriptorLayout = getGraphicsContext()->createDescriptorSetLayout(descriptorLayoutDesc);
		pvr::api::DescriptorSetUpdate descSetCreateParam;

		descSetCreateParam.setCombinedImageSampler(0, getTextureFromPVRCameraHandle(getGraphicsContext(), m_Camera.getChrominanceTexture()), sampler);
		descSetCreateParam.setCombinedImageSampler(1, getTextureFromPVRCameraHandle(getGraphicsContext(), m_Camera.getLuminanceTexture()),  sampler);

		descriptorSet = getGraphicsContext()->createDescriptorSetOnDefaultPool(descriptorLayout);
		descriptorSet->update(descSetCreateParam);
	}

	api::Shader vertexShader, fragmentShader;
	Stream::ptr_type vertexShaderStream, fragmentShaderStream;

	if ((vertexShaderStream = getAssetStream("VertShader.vsh")).get() == NULL)
	{
		setExitMessage("Unable to load vertex shader file.");
		return false;
	}
	if ((fragmentShaderStream = getAssetStream("FragShader.fsh")).get() == NULL)
	{
		setExitMessage("Unable to load fragment shader file.");
		return false;
	}

	vertexShader = getGraphicsContext()->createShader(*vertexShaderStream, types::ShaderType::VertexShader, ShaderDefines, NumShaderDefines);
	fragmentShader = getGraphicsContext()->createShader(*fragmentShaderStream, types::ShaderType::FragmentShader, ShaderDefines,
	                 NumShaderDefines);

	api::GraphicsPipelineCreateParam pipeParams;
	pipeParams.vertexShader.setShader(vertexShader);
	pipeParams.fragmentShader.setShader(fragmentShader);
	pipeParams.depthStencil.setDepthWrite(false).setDepthTestEnable(false);

	//Positions are 2D for a full-screen quad
	pipeParams.vertexInput.addVertexAttribute(0, api::VertexAttributeInfo(0, types::DataType::Float32, 2, 0, "inVertex"));
	pipeParams.vertexInput.setInputBinding(0, 0);

	pipeParams.pipelineLayout = getGraphicsContext()->createPipelineLayout(api::PipelineLayoutCreateParam().addDescSetLayout(
	                              descriptorLayout));

	pipeParams.colorBlend.setAttachmentState(0, pvr::types::BlendingConfig());
	renderingPipeline = getGraphicsContext()->createGraphicsPipeline(pipeParams);
	uvTransformLocation = renderingPipeline->getUniformLocation("uvTransform");

	//Create a temporary command buffer to do one-shot initialization
	{
		pvr::api::CommandBuffer oneShotCommandBuffer = getGraphicsContext()->createCommandBufferOnDefaultPool();
		oneShotCommandBuffer->beginRecording();

		oneShotCommandBuffer->bindPipeline(renderingPipeline);
		if (m_Camera.hasLumaChromaTextures())
		{
			oneShotCommandBuffer->setUniform<pvr::int32>(renderingPipeline->getUniformLocation("SamplerY"), 0);
			oneShotCommandBuffer->setUniform<pvr::int32>(renderingPipeline->getUniformLocation("SamplerUV"), 1);
		}
		else if (m_Camera.hasRgbTexture())
		{
			oneShotCommandBuffer->setUniform<pvr::int32>(renderingPipeline->getUniformLocation("Sampler"), 0);
		}

		oneShotCommandBuffer->endRecording();
		oneShotCommandBuffer->submit();
	}
	onScreenFbo = getGraphicsContext()->createOnScreenFbo(0);
	return true;
}

/*!*********************************************************************************************************************
\brief  Pre-record the rendering commands
***********************************************************************************************************************/
void OGLESIntroducingPVRCamera::recordCommandBuffers()
{
	commandBuffer = getGraphicsContext()->createCommandBufferOnDefaultPool();
	commandBuffer->beginRecording();
	commandBuffer->bindVertexBuffer(vbo, 0, 0);
	commandBuffer->bindIndexBuffer(ibo, 0, types::IndexType::IndexType16Bit);

	commandBuffer->bindDescriptorSet(renderingPipeline->getPipelineLayout(), 0, descriptorSet, 0);
	commandBuffer->bindPipeline(renderingPipeline);
	commandBuffer->setUniformPtr<glm::mat4>(uvTransformLocation, 1, &m_Camera.getProjectionMatrix());
	commandBuffer->beginRenderPass(onScreenFbo, Rectanglei(0, 0, getWidth(), getHeight()), true, glm::vec4(.2, .2, .2, 1.));
	commandBuffer->drawIndexed(0, 6);

	pvr::api::SecondaryCommandBuffer uicmd = getGraphicsContext()->createSecondaryCommandBufferOnDefaultPool();
	uiRenderer.beginRendering(uicmd);
	uiRenderer.getDefaultTitle()->render();
	uiRenderer.getDefaultDescription()->render();
	uiRenderer.getSdkLogo()->render();
	uiRenderer.endRendering();
	commandBuffer->enqueueSecondaryCmds(uicmd);
	commandBuffer->endRenderPass();
	commandBuffer->endRecording();
}

/*!*********************************************************************************************************************
\return Return ::pvr::Result::Success if no error occured
\brief  Code in initApplication() will be called by Shell once per run, before the rendering context is created.
        Used to configure the application window and rendering context (API version, vsync, window size etc.), and to load modules
        and objects not dependent on the context.
***********************************************************************************************************************/
Result OGLESIntroducingPVRCamera::initApplication()
{
	return Result::Success;
}

/*!*********************************************************************************************************************
\return Return ::pvr::Result::Success if no error occurred
\brief  Will be called by Shell once per run, just before exiting the program. Nothing to do in this demo.
***********************************************************************************************************************/
Result OGLESIntroducingPVRCamera::quitApplication() { return Result::Success; }

/*!*********************************************************************************************************************
\return Return ::pvr::Result::Success if no error occured
\brief  Code in initView() will be called by PVRShell upon initialization, and after any change to the rendering context.Used to
        initialize variables that are dependent on the rendering context (i.e. API objects)
***********************************************************************************************************************/
Result OGLESIntroducingPVRCamera::initView()
{
	createBuffers();
	//  Load and compile the shaders & link programs
	if (!createPipelineAndDescriptors()) {  return Result::UnknownError;  }
	if (uiRenderer.init(onScreenFbo->getRenderPass(), 0) != Result::Success) { return Result::UnknownError; }
	uiRenderer.getDefaultDescription()->setText("Streaming of hardware Camera video preview");
	uiRenderer.getDefaultDescription()->commitUpdates();
	uiRenderer.getDefaultTitle()->setText("IntroducingPVRCamera");
	uiRenderer.getDefaultTitle()->commitUpdates();
	recordCommandBuffers();
	return Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
Result OGLESIntroducingPVRCamera::releaseView()
{
	// Clean up AV capture
	m_Camera.destroySession();

	// Release Print3D Textures
	uiRenderer.release();
	renderingPipeline.reset();
	onScreenFbo.reset();
	descriptorLayout.reset();
	descriptorSet.reset();
	sampler.reset();
	vbo.reset();
	ibo.reset();
	onScreenFbo.reset();
	commandBuffer.reset();
	return Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
Result OGLESIntroducingPVRCamera::renderFrame()
{
	m_Camera.updateImage();
#if defined(TARGET_OS_IPHONE)
	static bool firstFrame = true;
	static pvr::api::TextureView tex0 = getTextureFromPVRCameraHandle(getGraphicsContext(), m_Camera.getLuminanceTexture());
	static pvr::api::TextureView tex1 = getTextureFromPVRCameraHandle(getGraphicsContext(), m_Camera.getChrominanceTexture());
	if (firstFrame)
	{
		pvr::api::DescriptorSetUpdate descSetInfo;
		descSetInfo.setCombinedImageSampler(0, tex0, sampler)
		.setCombinedImageSampler(1, tex1, sampler);
		descriptorSet->update(descSetInfo);
		firstFrame = false;
	}
#endif
	commandBuffer->submit();
	return Result::Success;
}

/*!*********************************************************************************************************************
\return Return an auto ptr to the demo supplied by the user
\brief  This function must be implemented by the user of the shell. The user should return its PVRShell object defining the behaviour of the application.
***********************************************************************************************************************/
std::auto_ptr<Shell> pvr::newDemo() { return std::auto_ptr<Shell>(new OGLESIntroducingPVRCamera()); }
