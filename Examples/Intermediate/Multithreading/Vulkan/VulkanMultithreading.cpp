/*!*********************************************************************************************************************
\File         VulkanMultithreading.cpp
\Title        Bump mapping
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief      Shows how to perform tangent space bump mapping
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVREngineUtils/PVREngineUtils.h"
#include "PVREngineUtils/Asynchronous.h"

using namespace pvr;
using namespace types;
const float32 RotateY = glm::pi<float32>() / 150;
const glm::vec4 LightDir(.24f, .685f, -.685f, 0.0f);

/*!*********************************************************************************************************************
 shader attributes
 ***********************************************************************************************************************/
// vertex attributes
namespace VertexAttrib {
enum Enum
{
	VertexArray, NormalArray, TexCoordArray, TangentArray, numAttribs
};
}

const utils::VertexBindings VertexAttribBindings[] =
{
	{ "POSITION", 0 },
	{ "NORMAL",   1 },
	{ "UV0",    2 },
	{ "TANGENT",  3 },
};

// shader uniforms
namespace Uniform {
enum Enum { MVPMatrix, LightDir, NumUniforms };
}


/*!*********************************************************************************************************************
 Content file names
 ***********************************************************************************************************************/

// Source and binary shaders
const char FragShaderSrcFile[]    = "FragShader.fsh";
const char VertShaderSrcFile[]    = "VertShader.vsh";

// PVR texture files
const char StatueTexFile[]      = "Marble.pvr";
const char StatueNormalMapFile[]  = "MarbleNormalMap.pvr";

const char ShadowTexFile[]      = "Shadow.pvr";
const char ShadowNormalMapFile[]  = "ShadowNormalMap.pvr";

// POD scene files
const char SceneFile[]        = "scene.pod";



/*!*********************************************************************************************************************
 Class implementing the Shell functions.
 ***********************************************************************************************************************/
class VulkanMultithreading : public Shell
{
	struct UboPerMeshData
	{
		glm::mat4 mvpMtx;
		glm::vec3 lightDirModel;
	};

	// Print3D class used to display text
	ui::UIRenderer  uiRenderer;

	// 3D Model
	assets::ModelHandle scene;

	// Projection and view matrix
	glm::mat4 viewProj;


	struct DescriptorSetUpdateRequiredInfo
	{
		async::AsyncApiTexture diffuseTex;
		async::AsyncApiTexture bumpTex;
		api::Sampler trilinearSampler;
		api::Sampler bilinearSampler;
	};

	struct DeviceResources
	{
		pvr::async::TextureAsyncLoader loader;
		pvr::async::TextureApiAsyncUploader uploader;
		std::vector<api::Buffer> vbo;
		std::vector<api::Buffer> ibo;
		api::DescriptorSetLayout texLayout;
		api::DescriptorSetLayout uboLayoutDynamic;
		api::PipelineLayout pipelayout;
		api::DescriptorSet texDescSet;

		api::GraphicsPipeline pipe;

		pvr::ui::Text loadingText[3];

		std::vector<api::CommandBuffer> mainCommandBuffer;// per swapchain
		std::vector<api::CommandBuffer> loadingCommandBuffer;// per swapchain

		api::FboSet fboOnScreen;// per swapchain
		pvr::utils::StructuredMemoryView ubo;//per swapchain
		std::vector<api::DescriptorSet> uboDescSet;

		DescriptorSetUpdateRequiredInfo asyncUpdateInfo;
	};

	bool loadingDone;

	GraphicsContext context;
	utils::AssetStore assetManager;
	// The translation and Rotate parameter of Model
	float32 angleY;
	std::auto_ptr<DeviceResources> deviceResource;
public:
	VulkanMultithreading(): loadingDone(false) { }
	virtual Result initApplication();
	virtual Result initView();
	virtual Result releaseView();
	virtual Result quitApplication();
	virtual Result renderFrame();

	bool createImageSamplerDescriptorSets();
	bool createUbo();
	bool loadPipeline();
	void drawMesh(api::CommandBuffer& cmdBuffer, int i32NodeIndex);
	void recordMainCommandBuffer();
	void recordLoadingCommandBuffer();
	bool updateTextureDescriptorSet();
};

bool VulkanMultithreading::updateTextureDescriptorSet()
{
	// create the descriptor set
	api::DescriptorSetUpdate descSetCreateInfo;
	descSetCreateInfo
	.setCombinedImageSampler(0, deviceResource->asyncUpdateInfo.diffuseTex->get(), deviceResource->asyncUpdateInfo.bilinearSampler)
	.setCombinedImageSampler(1, deviceResource->asyncUpdateInfo.bumpTex->get(), deviceResource->asyncUpdateInfo.trilinearSampler);
	if (!deviceResource->texDescSet.isValid())
	{
		setExitMessage("ERROR: Failed to create Combined Image Sampler Descriptor set.");
		return false;
	}
	return deviceResource->texDescSet->update(descSetCreateInfo);
}

/*!*********************************************************************************************************************
\return return true if no error occurred
\brief  Loads the textures required for this training course
***********************************************************************************************************************/
bool VulkanMultithreading::createImageSamplerDescriptorSets()
{
	deviceResource->texDescSet = context->createDescriptorSetOnDefaultPool(deviceResource->texLayout);
	// create the bilinear sampler
	assets::SamplerCreateParam samplerInfo;
	samplerInfo.magnificationFilter = SamplerFilter::Linear;
	samplerInfo.minificationFilter = SamplerFilter::Linear;
	samplerInfo.mipMappingFilter = SamplerFilter::Nearest;
	deviceResource->asyncUpdateInfo.bilinearSampler = context->createSampler(samplerInfo);
	samplerInfo.mipMappingFilter = SamplerFilter::Linear;
	deviceResource->asyncUpdateInfo.trilinearSampler = context->createSampler(samplerInfo);

	if (!deviceResource->texDescSet.isValid())
	{
		setExitMessage("ERROR: Failed to create Combined Image Sampler Descriptor set.");
		return false;
	}
	return true;
}

bool VulkanMultithreading::createUbo()
{
	api::DescriptorSetUpdate descUpdate;
	deviceResource->uboDescSet.resize(getPlatformContext().getSwapChainLength());
	deviceResource->ubo.addEntryPacked("MVPMatrix", pvr::types::GpuDatatypes::mat4x4);
	deviceResource->ubo.addEntryPacked("LightDirModel", pvr::types::GpuDatatypes::vec3);
	deviceResource->ubo.finalize(context, 1, pvr::types::BufferBindingUse::UniformBuffer, false, false);
	for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		auto buffer = context->createBuffer(deviceResource->ubo.getAlignedTotalSize(), BufferBindingUse::UniformBuffer, true);
		deviceResource->ubo.createConnectedBuffer(i, context);
		deviceResource->uboDescSet[i] = context->createDescriptorSetOnDefaultPool(deviceResource->uboLayoutDynamic);
		descUpdate.setUbo(0, deviceResource->ubo.getConnectedBuffer(i));
		deviceResource->uboDescSet[i]->update(descUpdate);
	}
	return true;
}

/*!*********************************************************************************************************************
\return  Return true if no error occurred
\brief  Loads and compiles the shaders and create a pipeline
***********************************************************************************************************************/
bool VulkanMultithreading::loadPipeline()
{
	types::BlendingConfig colorAttachemtState;
	api::GraphicsPipelineCreateParam pipeInfo;
	colorAttachemtState.blendEnable = false;

	//--- create the texture-sampler descriptor set layout
	{
		api::DescriptorSetLayoutCreateParam descSetLayoutInfo;
		descSetLayoutInfo
		.setBinding(0, DescriptorType::CombinedImageSampler, 1, ShaderStageFlags::Fragment)/*binding 0*/
		.setBinding(1, DescriptorType::CombinedImageSampler, 1, ShaderStageFlags::Fragment);/*binding 1*/
		deviceResource->texLayout = context->createDescriptorSetLayout(descSetLayoutInfo);
	}

	//--- create the ubo descriptorset layout
	{
		api::DescriptorSetLayoutCreateParam descSetLayoutInfo;
		descSetLayoutInfo.setBinding(0, DescriptorType::UniformBuffer, 1, ShaderStageFlags::Vertex); /*binding 0*/
		deviceResource->uboLayoutDynamic = context->createDescriptorSetLayout(descSetLayoutInfo);
	}

	//--- create the pipeline layout
	{
		api::PipelineLayoutCreateParam pipeLayoutInfo;
		pipeLayoutInfo
		.addDescSetLayout(deviceResource->texLayout)/*set 0*/
		.addDescSetLayout(deviceResource->uboLayoutDynamic);/*set 1*/
		deviceResource->pipelayout = context->createPipelineLayout(pipeLayoutInfo);
	}

	pipeInfo.rasterizer.setCullFace(pvr::types::Face::Back);

	pipeInfo.colorBlend.setAttachmentState(0, colorAttachemtState);

	pvr::assets::ShaderFile fileVersioner;
	fileVersioner.populateValidVersions(VertShaderSrcFile, *this);
	pipeInfo.vertexShader = context->createShader(*fileVersioner.getBestStreamForContext(context),
	                        ShaderType::VertexShader);

	fileVersioner.populateValidVersions(FragShaderSrcFile, *this);
	pipeInfo.fragmentShader = context->createShader(*fileVersioner.getBestStreamForContext(context),
	                          ShaderType::FragmentShader);

	const assets::Mesh& mesh = scene->getMesh(0);
	pipeInfo.inputAssembler.setPrimitiveTopology(mesh.getPrimitiveType());
	pipeInfo.pipelineLayout = deviceResource->pipelayout;
	pipeInfo.renderPass = deviceResource->fboOnScreen[0]->getRenderPass();
	pipeInfo.subPass = 0;
	// Enable z-buffer test. We are using a projection matrix optimized for a floating point depth buffer,
	// so the depth test and clear value need to be inverted (1 becomes near, 0 becomes far).
	pipeInfo.depthStencil.setDepthTestEnable(true);
	pipeInfo.depthStencil.setDepthCompareFunc(ComparisonMode::Less);
	pipeInfo.depthStencil.setDepthWrite(true);
	utils::createInputAssemblyFromMesh(mesh, VertexAttribBindings, sizeof(VertexAttribBindings) /
	                                   sizeof(VertexAttribBindings[0]), pipeInfo);
	deviceResource->pipe = context->createGraphicsPipeline(pipeInfo);
	return (deviceResource->pipe.isValid());
}


/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in initApplication() will be called by Shell once per run, before the rendering context is created.
    Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
    If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
Result VulkanMultithreading::initApplication()
{
	prepareSharedContexts(std::vector<SharedContextCapabilities>(
	{
		{ false, false, true, false, false, false },
	}));

	// Load the scene
	assetManager.init(*this);
	if (!assetManager.loadModel(SceneFile, scene))
	{
		this->setExitMessage("ERROR: Couldn't load the .pod file\n");
		return Result::NotInitialized;
	}
	angleY = 0.0f;


	return Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
    If the rendering context is lost, quitApplication() will not be called.x
***********************************************************************************************************************/
Result VulkanMultithreading::quitApplication() { return Result::Success;}



void DiffuseTextureDoneCallback(pvr::async::AsyncApiTexture tex)
{
	auto fmt = tex->get()->getResource()->getFormat();
	if (tex->isSuccessful())
	{
		std::this_thread::sleep_for(std::chrono::seconds(5));
		Log(Log.Information, "ASYNCUPLOADER: Diffuse texture uploading completed successfully.");
	}
	else
	{
		Log(Log.Information, "ASYNCUPLOADER: ERROR uploading normal texture. You can handle this information in your applications.");
	}
}

void NormalTextureDoneCallback(pvr::async::AsyncApiTexture tex)
{
	if (tex->isSuccessful())
	{
		std::this_thread::sleep_for(std::chrono::seconds(5));
		Log(Log.Information, "ASYNCUPLOADER: Normal texture uploading has been completed.");
	}
	else
	{
		Log(Log.Information, "ASYNCUPLOADER: ERROR uploading normal texture. You can handle this information in your applications.");
	}
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
    Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
Result VulkanMultithreading::initView()
{
	context = getGraphicsContext();
	deviceResource.reset(new DeviceResources());

	deviceResource->uploader.init(context, 0);
	deviceResource->asyncUpdateInfo.diffuseTex =
	  deviceResource->uploader.uploadTextureAsync(
	    deviceResource->loader.loadTextureAsync("Marble.pvr", this, TextureFileFormat::PVR),
	    true, &DiffuseTextureDoneCallback);
	deviceResource->asyncUpdateInfo.bumpTex =
	  deviceResource->uploader.uploadTextureAsync(
	    deviceResource->loader.loadTextureAsync("MarbleNormalMap.pvr", this, TextureFileFormat::PVR),
	    true, &NormalTextureDoneCallback);

	// load the vbo and ibo data
	utils::appendSingleBuffersFromModel(getGraphicsContext(), *scene, deviceResource->vbo, deviceResource->ibo);
	deviceResource->fboOnScreen = context->createOnScreenFboSet();
	// load the pipeline
	if (!loadPipeline()) {  return Result::UnknownError;  }
	if (!createUbo()) { return Result::UnknownError; }

	//  Initialize UIRenderer
	if (uiRenderer.init(deviceResource->fboOnScreen[0]->getRenderPass(), 0) != Result::Success)
	{
		this->setExitMessage("ERROR: Cannot initialize UIRenderer\n");
		return Result::UnknownError;
	}

	uiRenderer.getDefaultTitle()->setText("Multithreading");
	uiRenderer.getDefaultTitle()->commitUpdates();
	glm::vec3 from, to, up;
	float32 fov;
	scene->getCameraProperties(0, fov, from, to, up);

	// Is the screen rotated
	bool bRotate = this->isScreenRotated() && this->isFullScreen();

	//  Calculate the projection and rotate it by 90 degree if the screen is rotated.
	viewProj = (bRotate ?
	            math::perspectiveFov(getApiType(), fov, (float)this->getHeight(), (float)this->getWidth(),
	                                 scene->getCamera(0).getNear(), scene->getCamera(0).getFar(), glm::pi<float32>() * .5f) :
	            math::perspectiveFov(getApiType(), fov, (float)this->getWidth(), (float)this->getHeight(),
	                                 scene->getCamera(0).getNear(), scene->getCamera(0).getFar()));

	viewProj = viewProj * glm::lookAt(from, to, up);
	recordLoadingCommandBuffer();
	return Result::Success;
}

/*!*********************************************************************************************************************
\brief  Code in releaseView() will be called by PVRShell when theapplication quits or before a change in the rendering context.
\return Return Result::Success if no error occurred
***********************************************************************************************************************/
Result VulkanMultithreading::releaseView()
{
	auto items_remaining = deviceResource->loader.getNumQueuedItems();
	if (items_remaining)
	{
		Log(Log.Information, "Asynchronous Texture Loader is not done: %d items pending. Before releasing, will wait until all pending load jobs are done.", items_remaining);
	}
	items_remaining = deviceResource->uploader.getNumQueuedItems();
	if (items_remaining)
	{
		Log(Log.Information, "Asynchronous Texture Uploader is not done: %d items pending. Before releasing, will wait until all pending load jobs are done.", items_remaining);
	}

	deviceResource.reset();
	uiRenderer.release();
	scene.reset();
	assetManager.releaseAll();
	return Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
Result VulkanMultithreading::renderFrame()
{
	if (!loadingDone)
	{
		if (deviceResource->asyncUpdateInfo.bumpTex->isComplete() && deviceResource->asyncUpdateInfo.diffuseTex->isComplete())
		{
			if (!createImageSamplerDescriptorSets()) { return Result::UnknownError; }
			if (!updateTextureDescriptorSet()) { return Result::UnknownError; }
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
		deviceResource->loadingText[getSwapChainIndex()]->setColor(1.0f, 1.0f, 1.0f, f + .01f);
		deviceResource->loadingText[getSwapChainIndex()]->setScale(sin(f) * 3.f, sin(f) * 3.f);
		deviceResource->loadingText[getSwapChainIndex()]->commitUpdates();
		deviceResource->loadingCommandBuffer[getSwapChainIndex()]->submit();
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

		// update the ubo
		{
			UboPerMeshData srcWrite;
			srcWrite.lightDirModel = glm::vec3(LightDir * mModel);
			srcWrite.mvpMtx = viewProj * mModel * scene->getWorldMatrix(scene->getNode(0).getObjectId());
			deviceResource->ubo.map(getSwapChainIndex());
			deviceResource->ubo.setValue(0, srcWrite.mvpMtx);
			deviceResource->ubo.setValue(1, srcWrite.lightDirModel);
			deviceResource->ubo.unmap(getSwapChainIndex());
		}
		deviceResource->mainCommandBuffer[getPlatformContext().getSwapChainIndex()]->submit();
	}

	return Result::Success;
}

/*!*********************************************************************************************************************
\brief  Draws a assets::Mesh after the model view matrix has been set and the material prepared.
\param  nodeIndex Node index of the mesh to draw
***********************************************************************************************************************/
void VulkanMultithreading::drawMesh(api::CommandBuffer& cmdBuffer, int nodeIndex)
{
	uint32 meshId = scene->getNode(nodeIndex).getObjectId();
	const assets::Mesh& mesh = scene->getMesh(meshId);

	// bind the VBO for the mesh
	cmdBuffer->bindVertexBuffer(deviceResource->vbo[meshId], 0, 0);

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
			cmdBuffer->bindIndexBuffer(deviceResource->ibo[meshId], 0, mesh.getFaces().getDataType());
			cmdBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
		}
		else
		{
			// Non-Indexed Triangle list
			cmdBuffer->drawArrays(0, mesh.getNumFaces() * 3, 0, 1);
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
				cmdBuffer->bindIndexBuffer(deviceResource->ibo[meshId], 0,
				                           mesh.getFaces().getDataType());
				cmdBuffer->drawIndexed(0, mesh.getStripLength(i) + 2, offset * 2, 0, 1);
			}
			else
			{
				// Non-Indexed Triangle strips
				cmdBuffer->drawArrays(0, mesh.getStripLength(i) + 2, 0, 1);
			}
			offset += mesh.getStripLength(i) + 2;
		}
	}
}

/*!*********************************************************************************************************************
\brief  Pre record the commands
***********************************************************************************************************************/
void VulkanMultithreading::recordMainCommandBuffer()
{
	deviceResource->mainCommandBuffer.resize(getPlatformContext().getSwapChainLength());
	for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		deviceResource->mainCommandBuffer[i] = context->createCommandBufferOnDefaultPool();
		api::CommandBuffer cmdBuffer = deviceResource->mainCommandBuffer[i];
		cmdBuffer->beginRecording();
		cmdBuffer->beginRenderPass(deviceResource->fboOnScreen[i], Rectanglei(0, 0, getWidth(), getHeight()), true,
		                           glm::vec4(0.00, 0.70, 0.67, 1.f));
		// enqueue the static states which wont be changed through out the frame
		cmdBuffer->bindPipeline(deviceResource->pipe);
		cmdBuffer->bindDescriptorSet(deviceResource->pipelayout, 0, deviceResource->texDescSet, 0);
		cmdBuffer->bindDescriptorSet(deviceResource->pipelayout, 1, deviceResource->uboDescSet[i]);
		drawMesh(cmdBuffer, 0);

		// record the uirenderer commands
		uiRenderer.beginRendering(cmdBuffer);
		uiRenderer.getDefaultTitle()->render();
		uiRenderer.getSdkLogo()->render();
		uiRenderer.endRendering();
		cmdBuffer->endRenderPass();
		cmdBuffer->endRecording();
	}
}

/*!*********************************************************************************************************************
\brief  Pre record the commands
***********************************************************************************************************************/
void VulkanMultithreading::recordLoadingCommandBuffer()
{
	deviceResource->loadingCommandBuffer.resize(getSwapChainLength());

	for (pvr::uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		deviceResource->loadingCommandBuffer[i] = context->createCommandBufferOnDefaultPool();
		api::CommandBuffer& cmdBuffer = deviceResource->loadingCommandBuffer[i];
		cmdBuffer->beginRecording();
		cmdBuffer->beginRenderPass(deviceResource->fboOnScreen[i], true, glm::vec4(0.00, 0.70, 0.67, 1.f));

		deviceResource->loadingText[i] = uiRenderer.createText("Loading...");
		deviceResource->loadingText[i]->commitUpdates();

		// record the uirenderer commands
		uiRenderer.beginRendering(cmdBuffer);
		uiRenderer.getDefaultTitle()->render();
		uiRenderer.getSdkLogo()->render();
		deviceResource->loadingText[i]->render();
		uiRenderer.endRendering();
		cmdBuffer->endRenderPass();
		cmdBuffer->endRecording();
	}
}

/*!*********************************************************************************************************************
\return Return an auto ptr to the demo supplied by the user
\brief  This function must be implemented by the user of the shell. The user should return its
    Shell object defining the behavior of the application.
***********************************************************************************************************************/
std::auto_ptr<Shell> pvr::newDemo() { return std::auto_ptr<Shell>(new VulkanMultithreading()); }
