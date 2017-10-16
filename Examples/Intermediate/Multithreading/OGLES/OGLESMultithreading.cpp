/*!*********************************************************************************************************************
\File         OGLESMultithreading.cpp
\Title        Bump mapping
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief      Shows how to perform tangent space bump mapping
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVREngineUtils/PVREngineUtils.h"
#include "PVREngineUtils/Asynchronous.h"
#include "PVRNativeApi/OGLES/ApiErrorsGles.h"


using namespace pvr;
using namespace pvr::types;
const pvr::float32 RotateY = glm::pi<pvr::float32>() / 150;
const glm::vec4 LightDir(.24f, .685f, -.685f, 0.0f);

/*!*********************************************************************************************************************
 shader attributes
 ***********************************************************************************************************************/
const utils::VertexBindings_Name VertexAttribBindings[] =
{
	{ "POSITION", "inVertex" },
	{ "NORMAL", "inNormal" },
	{ "UV0",  "inTexCoord" },
	{ "TANGENT",  "inTangent" },
};

// shader uniforms
namespace Uniform {
enum Enum { MVPMatrix, LightDir, NumUniforms };
}

const char* UniformNames[] = {  "MVPMatrix", "LightDirModel" };

/*!*********************************************************************************************************************
 Content file names
 ***********************************************************************************************************************/

// Source and binary shaders
const char FragShaderSrcFile[]    = "FragShader.fsh";
const char VertShaderSrcFile[]    = "VertShader.vsh";

// PVR texture files
const char StatueTexFile[]      = "Marble.pvr";
const char StatueNormalMapFile[]  = "MarbleNormalMap.pvr";

// POD scene files
const char SceneFile[]        = "scene.pod";

/*!*********************************************************************************************************************
 Class implementing the Shell functions.
 ***********************************************************************************************************************/
class OGLESMultithreading : public Shell
{
	// Print3D class used to display text
	ui::UIRenderer  uiRenderer;

	// 3D Model
	assets::ModelHandle scene;

	// Projection and view matrix
	glm::mat4 viewProj;

	struct DrawPass
	{
		glm::mat4 mvp;
		glm::vec3 lightDir;
	};

	struct DescriptorSetUpdateRequiredInfo
	{
		async::AsyncApiTexture diffuseTex;
		async::AsyncApiTexture bumpTex;
	};

	struct DeviceResources
	{
		async::TextureAsyncLoader loader;
		async::TextureApiAsyncUploader uploader;
		std::vector<api::Buffer> vbo;
		std::vector<api::Buffer> ibo;
		api::DescriptorSetLayout descSetLayout;
		api::DescriptorSet imageSamplerDescSet;
		api::GraphicsPipeline pipe;
		api::CommandBuffer mainCommandBuffer;
		api::CommandBuffer loadingCommandBuffer;
		ui::Text loadingText;
		api::Fbo fboOnScreen;

		DescriptorSetUpdateRequiredInfo asyncUpdateInfo;
	};

	bool loadingDone;

	uint32 pipeUniformLoc[Uniform::NumUniforms];
	GraphicsContext context;
	utils::AssetStore assetManager;
	// The translation and Rotate parameter of Model
	float32 angleY;
	DrawPass drawPass;
	std::auto_ptr<DeviceResources> deviceResource;
public:
	OGLESMultithreading() {}
	virtual Result initApplication();
	virtual Result initView();
	virtual Result releaseView();
	virtual Result quitApplication();
	virtual Result renderFrame();

	bool createImageSamplerDescriptor();
	bool loadPipeline();
	void drawMesh(int i32NodeIndex);
	void recordMainCommandBuffer();
	void recordLoadingCommandBuffer();
};

/*!*********************************************************************************************************************
\return return true if no error occurred
\brief  Loads the textures required for this training course
***********************************************************************************************************************/
bool OGLESMultithreading::createImageSamplerDescriptor()
{
	pvr::api::TextureView texBase;
	pvr::api::TextureView texNormalMap;

	// create the bilinear sampler
	pvr::assets::SamplerCreateParam samplerInfo;
	samplerInfo.magnificationFilter = SamplerFilter::Linear;
	samplerInfo.minificationFilter = SamplerFilter::Linear;
	samplerInfo.mipMappingFilter = SamplerFilter::Linear;
	debugLogApiError("createImageSamplerDescriptor 1");
	auto trilinearSampler = context->createSampler(samplerInfo);

	debugLogApiError("createImageSamplerDescriptor 2");
	deviceResource->imageSamplerDescSet = context->createDescriptorSetOnDefaultPool(deviceResource->descSetLayout);
	debugLogApiError("createImageSamplerDescriptor 3");

	if (!deviceResource->imageSamplerDescSet.isValid())
	{
		setExitMessage("ERROR: Failed to create Combined Image Sampler Descriptor set.");
		return false;
	}


	// create the descriptor set
	api::DescriptorSetUpdate descSetCreateInfo;
	descSetCreateInfo
	.setCombinedImageSampler(0, deviceResource->asyncUpdateInfo.diffuseTex->get(), trilinearSampler)
	.setCombinedImageSampler(1, deviceResource->asyncUpdateInfo.bumpTex->get(), trilinearSampler);
	if (!deviceResource->imageSamplerDescSet.isValid())
	{
		setExitMessage("ERROR: Failed to create Combined Image Sampler Descriptor set.");
		return false;
	}
	deviceResource->imageSamplerDescSet->update(descSetCreateInfo);
	return true;
}

/*!*********************************************************************************************************************
\return  Return true if no error occurred
\brief  Loads and compiles the shaders and create a pipeline
***********************************************************************************************************************/
bool OGLESMultithreading::loadPipeline()
{
	pvr::types::BlendingConfig colorAttachemtState;
	pvr::api::GraphicsPipelineCreateParam pipeInfo;
	colorAttachemtState.blendEnable = false;

	//--- create the descriptor set layout
	pvr::api::DescriptorSetLayoutCreateParam descSetLayoutInfo;
	descSetLayoutInfo.setBinding(0, DescriptorType::CombinedImageSampler, 1, ShaderStageFlags::Fragment).
	setBinding(1, DescriptorType::CombinedImageSampler, 1, ShaderStageFlags::Fragment);
	deviceResource->descSetLayout = context->createDescriptorSetLayout(descSetLayoutInfo);

	//--- create the pipeline layout
	pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
	pipeLayoutInfo.addDescSetLayout(deviceResource->descSetLayout);

	pipeInfo.colorBlend.setAttachmentState(0, colorAttachemtState);

	pvr::assets::ShaderFile fileVersioning;
	fileVersioning.populateValidVersions(VertShaderSrcFile, *this);
	pipeInfo.vertexShader = context->createShader(*fileVersioning.getBestStreamForApi(context->getApiType()),
	                        ShaderType::VertexShader);

	fileVersioning.populateValidVersions(FragShaderSrcFile, *this);
	pipeInfo.fragmentShader = context->createShader(*fileVersioning.getBestStreamForApi(context->getApiType()),
	                          ShaderType::FragmentShader);

	const pvr::assets::Mesh& mesh = scene->getMesh(0);
	pipeInfo.inputAssembler.setPrimitiveTopology(mesh.getPrimitiveType());
	pipeInfo.pipelineLayout = context->createPipelineLayout(pipeLayoutInfo);
	pipeInfo.rasterizer.setCullFace(pvr::types::Face::Back);
	// Enable z-buffer test. We are using a projection matrix optimized for a floating point depth buffer,
	// so the depth test and clear value need to be inverted (1 becomes near, 0 becomes far).
	pipeInfo.depthStencil.setDepthTestEnable(true).setDepthCompareFunc(ComparisonMode::Less).setDepthWrite(true);
	pvr::utils::createInputAssemblyFromMesh(mesh, VertexAttribBindings,
	                                        sizeof(VertexAttribBindings) / sizeof(VertexAttribBindings[0]), pipeInfo);

	deviceResource->pipe = context->createGraphicsPipeline(pipeInfo);
	// Store the location of uniforms for later use
	for (int i = 0; i < Uniform::NumUniforms; ++i) {  pipeUniformLoc[i] = deviceResource->pipe->getUniformLocation(UniformNames[i]); }

	deviceResource->mainCommandBuffer->beginRecording();
	deviceResource->mainCommandBuffer->bindPipeline(deviceResource->pipe);
	deviceResource->mainCommandBuffer->setUniform(deviceResource->pipe->getUniformLocation("sBaseTex"), 0);
	deviceResource->mainCommandBuffer->setUniform(deviceResource->pipe->getUniformLocation("sNormalMap"), 1);
	deviceResource->mainCommandBuffer->endRecording();
	deviceResource->mainCommandBuffer->submit();
	return true;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in initApplication() will be called by Shell once per run, before the rendering context is created.
    Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
    If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result OGLESMultithreading::initApplication()
{
	loadingDone = false;
	setMinApiType(pvr::Api::OpenGLES3);
	// Load the scene
	assetManager.init(*this);
	if (!assetManager.loadModel(SceneFile, scene))
	{
		this->setExitMessage("ERROR: Couldn't load the .pod file\n");
		return pvr::Result::NotInitialized;
	}
	angleY = 0.0f;
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
    If the rendering context is lost, quitApplication() will not be called.x
***********************************************************************************************************************/
pvr::Result OGLESMultithreading::quitApplication() {  return pvr::Result::Success;}



void DiffuseTextureDoneCallback(pvr::async::AsyncApiTexture tex)
{
	if (tex->isSuccessful())
	{
		//Simulate a wait to show the loading screen...
		std::this_thread::sleep_for(std::chrono::seconds(5));
		Log(Log.Information, "ASYNCUPLOADER: Diffuse texture uploading completed successfully.");
	}
	else
	{
		Log(Log.Error, "ASYNCUPLOADER: There was an error uploading the Diffuse texture!");
	}
}

void NormalTextureDoneCallback(pvr::async::AsyncApiTexture tex)
{
	//Caution - to avoid deadlocks the callback is called AFTER signalling the semaphore.
	if (tex->isSuccessful())
	{
		Log(Log.Information, "ASYNCUPLOADER: Normal texture uploading has been completed.");
	}
	else
	{
		Log(Log.Error, "ASYNCUPLOADER: There was an error uploading the Normal texture!");
	}
}


/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
    Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result OGLESMultithreading::initView()
{
	context = getGraphicsContext();
	deviceResource.reset(new DeviceResources());
	deviceResource->uploader.init(getGraphicsContext(), 1);
	auto diff = deviceResource->loader.loadTextureAsync(StatueTexFile, this, TextureFileFormat::PVR);
	auto bump = deviceResource->loader.loadTextureAsync(StatueNormalMapFile, this, TextureFileFormat::PVR);

	deviceResource->asyncUpdateInfo.diffuseTex =
	  deviceResource->uploader.uploadTextureAsync(deviceResource->loader.loadTextureAsync(StatueTexFile, this, TextureFileFormat::PVR),
	      true, &DiffuseTextureDoneCallback);

	deviceResource->asyncUpdateInfo.bumpTex =
	  deviceResource->uploader.uploadTextureAsync(deviceResource->loader.loadTextureAsync(StatueNormalMapFile, this, TextureFileFormat::PVR),
	      true, &NormalTextureDoneCallback);

	deviceResource->mainCommandBuffer = context->createCommandBufferOnDefaultPool();
	deviceResource->loadingCommandBuffer = context->createCommandBufferOnDefaultPool();
	// load the vbo and ibo data
	pvr::utils::appendSingleBuffersFromModel(getGraphicsContext(), *scene, deviceResource->vbo, deviceResource->ibo);

	// load the pipeline
	if (!loadPipeline()) {  return pvr::Result::UnknownError; }

	// create OnScreen FBO
	deviceResource->fboOnScreen = context->createOnScreenFbo(0);

	//  Initialize UIRenderer
	if (uiRenderer.init(deviceResource->fboOnScreen->getRenderPass(), 0) != pvr::Result::Success)
	{
		this->setExitMessage("ERROR: Cannot initialize UIRenderer\n");
		return pvr::Result::UnknownError;
	}

	uiRenderer.getDefaultTitle()->setText("Multithreading");
	uiRenderer.getDefaultTitle()->commitUpdates();
	glm::vec3 from, to, up;
	pvr::float32 fov;
	scene->getCameraProperties(0, fov, from, to, up);

	// Is the screen rotated
	bool bRotate = this->isScreenRotated() && this->isFullScreen();

	//  Calculate the projection and rotate it by 90 degree if the screen is rotated.
	viewProj = (bRotate ?
	            pvr::math::perspectiveFov(getApiType(), fov, (float)this->getHeight(), (float)this->getWidth(), scene->getCamera(0).getNear(), scene->getCamera(0).getFar(), glm::pi<pvr::float32>() * .5f) :
	            glm::perspectiveFov<pvr::float32>(fov, (float)this->getWidth(), (float)this->getHeight(), scene->getCamera(0).getNear(), scene->getCamera(0).getFar()));

	viewProj = viewProj * glm::lookAt(from, to, up);

	recordLoadingCommandBuffer();
	return pvr::Result::Success;
}


/*!*********************************************************************************************************************
\brief  Code in releaseView() will be called by PVRShell when theapplication quits or before a change in the rendering context.
\return Return Result::Success if no error occurred
***********************************************************************************************************************/
pvr::Result OGLESMultithreading::releaseView()
{
	deviceResource.reset();
	uiRenderer.release();
	scene.reset();
	assetManager.releaseAll();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result OGLESMultithreading::renderFrame()
{
	if (!loadingDone)
	{
		if (deviceResource->asyncUpdateInfo.diffuseTex->isComplete() &&
		    deviceResource->asyncUpdateInfo.bumpTex->isComplete())
		{
			if (!createImageSamplerDescriptor()) { return Result::UnknownError; }
			recordMainCommandBuffer();
			loadingDone = true;
		}
	}
	if (!loadingDone)
	{
		static float f = 0;
		f += getFrameTime() * .0005f;
		if (f > glm::pi<float32>() * .5f)
		{
			f  = 0;
		}
		
		deviceResource->loadingText->setColor(1.0f, 1.0f, 1.0f, f + .01f);
		deviceResource->loadingText->setScale(sin(f) * 3.f, sin(f) * 3.f);
		deviceResource->loadingText->commitUpdates();
		deviceResource->loadingCommandBuffer->submit();
	}
	if (loadingDone)
	{
		// Calculate the model matrix
		glm::mat4 mModel = glm::rotate(angleY, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(1.8f));
		angleY += -RotateY * 0.05f  * getFrameTime();

		// Set light Direction in model space
		//  The inverse of a rotation matrix is the transposed matrix
		//  Because of v * M = transpose(M) * v, this means:
		//  v * R == inverse(R) * v
		//  So we don't have to actually invert or transpose the matrix
		//  to transform back from world space to model space

		// update the uniforms
		drawPass.lightDir = glm::vec3(LightDir * mModel);
		drawPass.mvp = viewProj * mModel * scene->getWorldMatrix(scene->getNode(0).getObjectId());
		deviceResource->mainCommandBuffer->submit();
	}
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  Draws a pvr::assets::Mesh after the model view matrix has been set and  the material prepared.
\param  nodeIndex Node index of the mesh to draw
***********************************************************************************************************************/
void OGLESMultithreading::drawMesh(int nodeIndex)
{
	pvr::uint32 meshId = scene->getNode(nodeIndex).getObjectId();
	const pvr::assets::Mesh& mesh = scene->getMesh(meshId);

	// bind the VBO for the mesh
	deviceResource->mainCommandBuffer->bindVertexBuffer(deviceResource->vbo[meshId], 0, 0);
	deviceResource->mainCommandBuffer->bindIndexBuffer(deviceResource->ibo[meshId], 0, mesh.getFaces().getDataType());

	//  The geometry can be exported in 4 ways:
	//  - Indexed Triangle list
	//  - Non-Indexed Triangle list
	//  - Indexed Triangle strips
	//  - Non-Indexed Triangle strips
	if (mesh.getNumStrips() == 0)
	{
		// Indexed Triangle list
		if (deviceResource->ibo[meshId].isValid())
		{
			deviceResource->mainCommandBuffer->bindIndexBuffer(deviceResource->ibo[meshId], 0, mesh.getFaces().getDataType());
			deviceResource->mainCommandBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
		}
		else
		{
			// Non-Indexed Triangle list
			deviceResource->mainCommandBuffer->drawArrays(0, mesh.getNumFaces() * 3, 0, 1);
		}
	}
	else
	{
		for (int i = 0; i < (int)mesh.getNumStrips(); ++i)
		{
			int offset = 0;
			if (deviceResource->ibo[meshId].isValid())
			{
				// Indexed Triangle strips
				deviceResource->mainCommandBuffer->bindIndexBuffer(deviceResource->ibo[meshId], 0,
				    mesh.getFaces().getDataType());
				deviceResource->mainCommandBuffer->drawIndexed(0, mesh.getStripLength(i) + 2, offset * 2, 0, 1);
			}
			else
			{
				// Non-Indexed Triangle strips
				deviceResource->mainCommandBuffer->drawArrays(0, mesh.getStripLength(i) + 2, 0, 1);
			}
			offset += mesh.getStripLength(i) + 2;
		}
	}
}

void OGLESMultithreading::recordLoadingCommandBuffer()
{
	deviceResource->loadingCommandBuffer = context->createCommandBufferOnDefaultPool();
	api::CommandBuffer& cmdBuffer = deviceResource->loadingCommandBuffer;
	cmdBuffer->beginRecording();
	cmdBuffer->beginRenderPass(deviceResource->fboOnScreen, Rectanglei(0, 0, getWidth(), getHeight()), true,
	                           glm::vec4(0.00, 0.70, 0.67, 1.f));

	deviceResource->loadingText = uiRenderer.createText("Loading...");
	deviceResource->loadingText->commitUpdates();

	// record the uirenderer commands
	uiRenderer.beginRendering(cmdBuffer);
	uiRenderer.getDefaultTitle()->render();
	uiRenderer.getSdkLogo()->render();
	deviceResource->loadingText->render();
	uiRenderer.endRendering();
	cmdBuffer->endRenderPass();
	cmdBuffer->endRecording();
}

/*!*********************************************************************************************************************
\brief  Pre record the commands
***********************************************************************************************************************/
void OGLESMultithreading::recordMainCommandBuffer()
{
	deviceResource->mainCommandBuffer->beginRecording();
	deviceResource->mainCommandBuffer->beginRenderPass(deviceResource->fboOnScreen, pvr::Rectanglei(0, 0, getWidth(), getHeight()), true, glm::vec4(0.00, 0.70, 0.67, 1.f));

	// enqueue the static states which wont be changed through out the frame
	deviceResource->mainCommandBuffer->bindPipeline(deviceResource->pipe);
	deviceResource->mainCommandBuffer->setUniformPtr(pipeUniformLoc[Uniform::LightDir], 1, &drawPass.lightDir);

	deviceResource->mainCommandBuffer->bindDescriptorSet(deviceResource->pipe->getPipelineLayout(), 0, deviceResource->imageSamplerDescSet, 0);
	deviceResource->mainCommandBuffer->setUniformPtr(pipeUniformLoc[Uniform::MVPMatrix], 1, &drawPass.mvp);
	drawMesh(0);

	pvr::api::SecondaryCommandBuffer uiCmdBuffer = context->createSecondaryCommandBufferOnDefaultPool();
	uiRenderer.beginRendering(uiCmdBuffer);
	uiRenderer.getDefaultTitle()->render();
	uiRenderer.getSdkLogo()->render();
	uiRenderer.endRendering();
	deviceResource->mainCommandBuffer->enqueueSecondaryCmds(uiCmdBuffer);
	deviceResource->mainCommandBuffer->endRenderPass();
	deviceResource->mainCommandBuffer->endRecording();
}

/*!*********************************************************************************************************************
\return Return an auto ptr to the demo supplied by the user
\brief  This function must be implemented by the user of the shell. The user should return its
    Shell object defining the behavior of the application.
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() {  return std::auto_ptr<pvr::Shell>(new OGLESMultithreading()); }
