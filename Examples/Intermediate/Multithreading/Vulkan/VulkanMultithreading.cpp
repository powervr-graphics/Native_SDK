/*!*********************************************************************************************************************
\File         VulkanMultithreading.cpp
\Title        Bump mapping
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief      Shows how to perform tangent space bump mapping
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/Vulkan/AsynchronousVk.h"
#include "PVRUtils/PVRUtilsVk.h"

const float RotateY = glm::pi<float>() / 150;
const glm::vec4 LightDir(.24f, .685f, -.685f, 0.0f);
const pvrvk::ClearValue ClearValue(0.00, 0.70, 0.67, 1.f);
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

const pvr::utils::VertexBindings VertexAttribBindings[] =
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

// POD _scene files
const char SceneFile[]        = "scene.pod";

/*!*********************************************************************************************************************
 Class implementing the Shell functions.
 ***********************************************************************************************************************/
class VulkanMultithreading : public pvr::Shell
{
	struct UboPerMeshData
	{
		glm::mat4 mvpMtx;
		glm::vec3 lightDirModel;
	};

	struct DescriptorSetUpdateRequiredInfo
	{
		pvr::utils::AsyncApiTexture diffuseTex;
		pvr::utils::AsyncApiTexture bumpTex;
		pvrvk::Sampler trilinearSampler;
		pvrvk::Sampler bilinearSampler;
	};

	struct DeviceResources
	{
		pvrvk::Instance instance;
		pvrvk::Surface surface;
		pvrvk::Device device;
		pvrvk::Swapchain swapchain;
		pvrvk::Queue queue;

		pvrvk::DescriptorPool descriptorPool;
		pvrvk::CommandPool commandPool;

		pvr::Multi<pvrvk::CommandBuffer> mainCommandBuffer;// per swapchain
		pvr::Multi<pvrvk::CommandBuffer> loadingTextCommandBuffer;// per swapchain

		pvr::Multi<pvrvk::Framebuffer> framebuffer;
		pvr::Multi<pvrvk::ImageView> depthStencilImages;

		pvrvk::Semaphore semaphoreImageAcquired[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Fence perFrameAcquireFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Semaphore semaphorePresent[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Fence perFrameCommandBufferFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

		pvrvk::GraphicsPipeline pipe;

		pvr::async::TextureAsyncLoader loader;
		pvr::utils::ImageApiAsyncUploader uploader;
		std::vector<pvrvk::Buffer> vbo;
		std::vector<pvrvk::Buffer> ibo;
		pvrvk::DescriptorSetLayout texLayout;
		pvrvk::DescriptorSetLayout uboLayoutDynamic;
		pvrvk::PipelineLayout pipelayout;
		pvrvk::DescriptorSet texDescSet;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;
		pvr::ui::Text loadingText[3];
		pvr::utils::StructuredBufferView structuredMemoryView;
		pvrvk::Buffer ubo;
		pvrvk::DescriptorSet uboDescSet[4];

		DescriptorSetUpdateRequiredInfo asyncUpdateInfo;
	};

	pvr::async::Mutex _hostMutex;

	// 3D Model
	pvr::assets::ModelHandle _scene;

	// Projection and view matrix
	glm::mat4 _viewProj;

	bool _loadingDone;
	// The translation and Rotate parameter of Model
	float _angleY;
	uint32_t _frameId;
	std::unique_ptr<DeviceResources> _deviceResources;

public:
	VulkanMultithreading(): _loadingDone(false) { }
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	bool createImageSamplerDescriptorSets();
	bool createUbo();
	bool loadPipeline();
	void drawMesh(pvrvk::CommandBuffer& commandBuffer, int i32NodeIndex);
	void recordMainCommandBuffer();
	void recordLoadingCommandBuffer();
	bool updateTextureDescriptorSet();
};

bool VulkanMultithreading::updateTextureDescriptorSet()
{
	// create the descriptor set
	pvrvk::WriteDescriptorSet writeDescInfo[2] =
	{
		pvrvk::WriteDescriptorSet(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->texDescSet, 0),
		pvrvk::WriteDescriptorSet(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->texDescSet, 1)
	};

	writeDescInfo[0].setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->asyncUpdateInfo.diffuseTex->get(),
	                              _deviceResources->asyncUpdateInfo.bilinearSampler, VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

	writeDescInfo[1].setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->asyncUpdateInfo.bumpTex->get(),
	                              _deviceResources->asyncUpdateInfo.trilinearSampler, VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

	if (!_deviceResources->texDescSet.isValid())
	{
		setExitMessage("ERROR: Failed to create Combined Image Sampler Descriptor set.");
		return false;
	}
	_deviceResources->device->updateDescriptorSets(writeDescInfo,
	    ARRAY_SIZE(writeDescInfo), nullptr, 0);
	return true;
}

/*!*********************************************************************************************************************
\return return true if no error occurred
\brief  Loads the textures required for this training course
***********************************************************************************************************************/
bool VulkanMultithreading::createImageSamplerDescriptorSets()
{
	_deviceResources->texDescSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->texLayout);
	// create the bilinear sampler
	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.magFilter = VkFilter::e_LINEAR;
	samplerInfo.minFilter = VkFilter::e_LINEAR;
	samplerInfo.mipMapMode = VkSamplerMipmapMode::e_NEAREST;
	_deviceResources->asyncUpdateInfo.bilinearSampler = _deviceResources->device->createSampler(samplerInfo);

	samplerInfo.mipMapMode = VkSamplerMipmapMode::e_NEAREST;
	_deviceResources->asyncUpdateInfo.trilinearSampler = _deviceResources->device->createSampler(samplerInfo);

	if (!_deviceResources->texDescSet.isValid())
	{
		setExitMessage("ERROR: Failed to create Combined Image Sampler Descriptor set.");
		return false;
	}
	return true;
}

bool VulkanMultithreading::createUbo()
{
	const uint32_t swapchainLength = _deviceResources->swapchain->getSwapchainLength();
	pvrvk::WriteDescriptorSet descUpdate[pvrvk::FrameworkCaps::MaxSwapChains];
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("MVPMatrix", pvr::GpuDatatypes::mat4x4);
		desc.addElement("LightDirModel", pvr::GpuDatatypes::vec3);

		_deviceResources->structuredMemoryView.initDynamic(desc, swapchainLength, pvr::BufferUsageFlags::UniformBuffer,
		    static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment));
		_deviceResources->ubo = pvr::utils::createBuffer(_deviceResources->device,_deviceResources->structuredMemoryView.getSize(),
		                        VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);
	}

	for (uint32_t i = 0; i < swapchainLength; ++i)
	{
		_deviceResources->uboDescSet[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->uboLayoutDynamic);
		descUpdate[i]
		.set(VkDescriptorType::e_UNIFORM_BUFFER, _deviceResources->uboDescSet[i])
		.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->ubo, _deviceResources->structuredMemoryView.getDynamicSliceOffset(i),
		               _deviceResources->structuredMemoryView.getDynamicSliceSize()));
	}

	_deviceResources->device->updateDescriptorSets(descUpdate, swapchainLength, nullptr, 0);
	return true;
}

/*!*********************************************************************************************************************
\return  Return true if no error occurred
\brief  Loads and compiles the shaders and create a pipeline
***********************************************************************************************************************/
bool VulkanMultithreading::loadPipeline()
{
	//--- create the texture-sampler descriptor set layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
		descSetLayoutInfo
		.setBinding(0, VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, VkShaderStageFlags::e_FRAGMENT_BIT)/*binding 0*/
		.setBinding(1, VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, VkShaderStageFlags::e_FRAGMENT_BIT);/*binding 1*/
		_deviceResources->texLayout = _deviceResources->device->createDescriptorSetLayout(descSetLayoutInfo);
	}

	//--- create the ubo descriptorset layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
		descSetLayoutInfo.setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER, 1, VkShaderStageFlags::e_VERTEX_BIT); /*binding 0*/
		_deviceResources->uboLayoutDynamic = _deviceResources->device->createDescriptorSetLayout(descSetLayoutInfo);
	}

	//--- create the pipeline layout
	{
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo; pipeLayoutInfo
		.addDescSetLayout(_deviceResources->texLayout)/*set 0*/
		.addDescSetLayout(_deviceResources->uboLayoutDynamic);/*set 1*/
		_deviceResources->pipelayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
	}
	pvrvk::GraphicsPipelineCreateInfo pipeInfo;
	pipeInfo.rasterizer.setCullMode(VkCullModeFlags::e_BACK_BIT);
	pipeInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

	pvr::assets::ShaderFile fileVersioner;
	fileVersioner.populateValidVersions(VertShaderSrcFile, *this);
	pipeInfo.vertexShader = _deviceResources->device->createShader(fileVersioner.getBestStreamForApi(pvr::Api::Vulkan)->readToEnd<uint32_t>());

	fileVersioner.populateValidVersions(FragShaderSrcFile, *this);
	pipeInfo.fragmentShader = _deviceResources->device->createShader(fileVersioner.getBestStreamForApi(pvr::Api::Vulkan)->readToEnd<uint32_t>());

	const pvr::assets::Mesh& mesh = _scene->getMesh(0);
	pipeInfo.inputAssembler.setPrimitiveTopology(pvr::utils::convertToVk(mesh.getPrimitiveType()));
	pipeInfo.pipelineLayout = _deviceResources->pipelayout;
	pipeInfo.renderPass = _deviceResources->framebuffer[0]->getRenderPass();
	pipeInfo.subpass = 0;
	// Enable z-buffer test. We are using a projection matrix optimized for a floating point depth buffer,
	// so the depth test and clear value need to be inverted (1 becomes near, 0 becomes far).
	pipeInfo.depthStencil.enableDepthTest(true);
	pipeInfo.depthStencil.setDepthCompareFunc(VkCompareOp::e_LESS);
	pipeInfo.depthStencil.enableDepthWrite(true);
	pvr::utils::populateInputAssemblyFromMesh(mesh, VertexAttribBindings, sizeof(VertexAttribBindings) /
	    sizeof(VertexAttribBindings[0]), pipeInfo.vertexInput, pipeInfo.inputAssembler);

	pvr::utils::populateViewportStateCreateInfo(_deviceResources->framebuffer[0], pipeInfo.viewport);
	_deviceResources->pipe = _deviceResources->device->createGraphicsPipeline(pipeInfo);
	return (_deviceResources->pipe.isValid());
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in initApplication() will be called by Shell once per run, before the rendering context is created.
    Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
    If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result VulkanMultithreading::initApplication()
{
	// Load the _scene
	if (!pvr::assets::helper::loadModel(*this, SceneFile, _scene))
	{
		this->setExitMessage("ERROR: Couldn't load the .pod file\n");
		return pvr::Result::NotInitialized;
	}
	_angleY = 0.0f;
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
    If the rendering context is lost, quitApplication() will not be called.x
***********************************************************************************************************************/
pvr::Result VulkanMultithreading::quitApplication() { return pvr::Result::Success;}

void DiffuseTextureDoneCallback(pvr::utils::AsyncApiTexture tex)

{
	//We have set the "callbackBeforeSignal" to "true", which means we should NOT call GET before this function returns!
	if (tex->isSuccessful())
	{
		std::this_thread::sleep_for(std::chrono::seconds(2));
		Log(LogLevel::Information, "ASYNCUPLOADER: Diffuse texture uploading completed successfully.");
	}
	else
	{
		Log(LogLevel::Information, "ASYNCUPLOADER: ERROR uploading normal texture. You can handle this information in your applications.");
	}
}

void NormalTextureDoneCallback(pvr::utils::AsyncApiTexture tex)
{
	//We have set the "callbackBeforeSignal" to "true", which means we should NOT call GET before this function returns!
	if (tex->isSuccessful())
	{
		std::this_thread::sleep_for(std::chrono::seconds(2));
		Log(LogLevel::Information, "ASYNCUPLOADER: Normal texture uploading has been completed.");
	}
	else
	{
		Log(LogLevel::Information, "ASYNCUPLOADER: ERROR uploading normal texture. You can handle this "
		    "information in your applications.");
	}
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
    Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result VulkanMultithreading::initView()
{
	_frameId = 0;
	_deviceResources.reset(new DeviceResources());
	// Create the Vulkan instance and surface
	if (!pvr::utils::createInstanceAndSurface(this->getApplicationName(), this->getWindow(), this->getDisplay(), _deviceResources->instance, _deviceResources->surface))
	{
		return pvr::Result::UnknownError;
	}

	// look for 2 queues one support Graphics and present operation and the second one with transfer operation
	pvr::utils::QueuePopulateInfo queuePopulateInfo =
	{
		VkQueueFlags::e_GRAPHICS_BIT, _deviceResources->surface,
	};
	pvr::utils::QueueAccessInfo queueAccessInfo;
	// create the Logical device
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), &queuePopulateInfo, 1, &queueAccessInfo);
	if (_deviceResources->device.isNull())
	{
		return pvr::Result::UnknownError;
	}

	//Get the queues
	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	// Create the commandpool & Descriptorpool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(_deviceResources->queue->getQueueFamilyId(),
	                                VkCommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT);

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(
	                                     pvrvk::DescriptorPoolCreateInfo()
	                                     .addDescriptorInfo(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 16)
	                                     .addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 16)
	                                     .addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER, 16)
	                                     .setMaxDescriptorSets(16));


	// create a new commandpool for image uploading and upload the images in separate thread
	_deviceResources->uploader.init(_deviceResources->device, _deviceResources->queue, &_hostMutex);

	_deviceResources->asyncUpdateInfo.diffuseTex =
	  _deviceResources->uploader.uploadTextureAsync(_deviceResources->loader.loadTextureAsync("Marble.pvr", this,
	      pvr::TextureFileFormat::PVR), true, &DiffuseTextureDoneCallback, true);

	_deviceResources->asyncUpdateInfo.bumpTex = _deviceResources->uploader.uploadTextureAsync(
	      _deviceResources->loader.loadTextureAsync("MarbleNormalMap.pvr", this, pvr::TextureFileFormat::PVR), true,
	      &NormalTextureDoneCallback, true);

	// load the vbo and ibo data
	pvr::utils::appendSingleBuffersFromModel(_deviceResources->device, *_scene, _deviceResources->vbo, _deviceResources->ibo);

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(_deviceResources->surface);

	// validate the supported swapchain image usage
	VkImageUsageFlags swapchainImageUsage = VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, VkImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= VkImageUsageFlags::e_TRANSFER_SRC_BIT;
	}

	// Create the swapchain image and depthstencil image
	if (!pvr::utils::createSwapchainAndDepthStencilImageView(_deviceResources->device, _deviceResources->surface,
	    getDisplayAttributes(), _deviceResources->swapchain, _deviceResources->depthStencilImages, swapchainImageUsage))
	{
		return pvr::Result::UnknownError;
	}

	if (!pvr::utils::createOnscreenFramebufferAndRenderpass(_deviceResources->swapchain, &_deviceResources->depthStencilImages[0], _deviceResources->framebuffer))
	{
		return pvr::Result::UnknownError;
	}
	// load the pipeline
	if (!loadPipeline()) {  return pvr::Result::UnknownError;  }
	if (!createUbo()) { return pvr::Result::UnknownError; }

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->semaphorePresent[i] = _deviceResources->device->createSemaphore();
		_deviceResources->semaphoreImageAcquired[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameCommandBufferFence[i] = _deviceResources->device->createFence(VkFenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->perFrameAcquireFence[i] = _deviceResources->device->createFence(VkFenceCreateFlags::e_SIGNALED_BIT);

		_deviceResources->loadingTextCommandBuffer[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->mainCommandBuffer[i] = _deviceResources->commandPool->allocateCommandBuffer();
	}

	//  Initialize UIRenderer
	if (!_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->framebuffer[0]->getRenderPass(), 0,
	                                       _deviceResources->commandPool, _deviceResources->queue))
	{
		this->setExitMessage("ERROR: Cannot initialize UIRenderer\n");
		return pvr::Result::UnknownError;
	}

	_deviceResources->uiRenderer.getDefaultTitle()->setText("Multithreading");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	glm::vec3 from, to, up;
	float fov;
	_scene->getCameraProperties(0, fov, from, to, up);

	// Is the screen rotated
	bool bRotate = this->isScreenRotated() && this->isFullScreen();

	//  Calculate the projection and rotate it by 90 degree if the screen is rotated.
	_viewProj = (bRotate ?
	             pvr::math::perspectiveFov(pvr::Api::Vulkan, fov, (float)this->getHeight(), (float)this->getWidth(),
	                                       _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar(), glm::pi<float>() * .5f) :
	             pvr::math::perspectiveFov(pvr::Api::Vulkan, fov, (float)this->getWidth(), (float)this->getHeight(),
	                                       _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar()));

	_viewProj = _viewProj * glm::lookAt(from, to, up);
	recordLoadingCommandBuffer();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  Code in releaseView() will be called by PVRShell when theapplication quits or before a change in the rendering context.
\return Return pvr::Result::Success if no error occurred
***********************************************************************************************************************/
pvr::Result VulkanMultithreading::releaseView()
{
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); i++)
	{
		_deviceResources->perFrameAcquireFence[i]->wait();
		_deviceResources->perFrameAcquireFence[i]->reset();

		_deviceResources->perFrameCommandBufferFence[i]->wait();
		_deviceResources->perFrameCommandBufferFence[i]->reset();
	}

	auto items_remaining = _deviceResources->loader.getNumQueuedItems();
	if (items_remaining)
	{
		Log(LogLevel::Information, "Asynchronous Texture Loader is not done: %d items pending. Before releasing,"
		    " will wait until all pending load jobs are done.", items_remaining);
	}
	items_remaining = _deviceResources->uploader.getNumQueuedItems();
	if (items_remaining)
	{
		Log(LogLevel::Information, "Asynchronous Texture Uploader is not done: %d items pending. Before releasing,"
		    " will wait until all pending load jobs are done.", items_remaining);
	}

	_deviceResources->device->waitIdle();

	_deviceResources.reset();
	_scene.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result VulkanMultithreading::renderFrame()
{
	_deviceResources->perFrameAcquireFence[_frameId]->wait();
	_deviceResources->perFrameAcquireFence[_frameId]->reset();
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->semaphoreImageAcquired[_frameId], _deviceResources->perFrameAcquireFence[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameCommandBufferFence[swapchainIndex]->wait();
	_deviceResources->perFrameCommandBufferFence[swapchainIndex]->reset();

	pvrvk::SubmitInfo submitInfo;
	VkPipelineStageFlags waitDestStages = VkPipelineStageFlags::e_ALL_GRAPHICS_BIT;
	submitInfo.waitDestStages = &waitDestStages;
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->semaphoreImageAcquired[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->semaphorePresent[_frameId];
	submitInfo.numSignalSemaphores = 1;

	if (!_loadingDone)
	{
		if (_deviceResources->asyncUpdateInfo.bumpTex->isComplete() && _deviceResources->asyncUpdateInfo.diffuseTex->isComplete())
		{
			if (!createImageSamplerDescriptorSets()) { return pvr::Result::UnknownError; }
			if (!updateTextureDescriptorSet()) { return pvr::Result::UnknownError; }
			recordMainCommandBuffer();
			_loadingDone = true;
		}
	}
	if (!_loadingDone)
	{
		static float f = 0;
		f += getFrameTime() * .0005f;
		if (f > glm::pi<float>() * .5f)
		{
			f  = 0;
		}
		_deviceResources->loadingText[swapchainIndex]->setColor(1.0f, 1.0f, 1.0f, f + .01f);
		_deviceResources->loadingText[swapchainIndex]->setScale(sin(f) * 3.f, sin(f) * 3.f);
		_deviceResources->loadingText[swapchainIndex]->commitUpdates();

		submitInfo.commandBuffers = &_deviceResources->loadingTextCommandBuffer[swapchainIndex];
	}

	if (_loadingDone)
	{
		// Calculate the model matrix
		glm::mat4 mModel = glm::rotate(_angleY, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(1.8f));
		_angleY += -RotateY * 0.05f  * getFrameTime();

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
			srcWrite.mvpMtx = _viewProj * mModel * _scene->getWorldMatrix(_scene->getNode(0).getObjectId());
			void* memory;
			uint32_t mappedDynamicSlice = swapchainIndex * _scene->getNumMeshNodes();
			_deviceResources->ubo->getDeviceMemory()->map(&memory, _deviceResources->structuredMemoryView.getDynamicSliceOffset(mappedDynamicSlice),
			    _deviceResources->structuredMemoryView.getDynamicSliceSize());
			_deviceResources->structuredMemoryView.pointToMappedMemory(memory, mappedDynamicSlice);
			uint32_t dynamicSlice = mappedDynamicSlice;
			_deviceResources->structuredMemoryView.getElement(0, 0, dynamicSlice).setValue(&srcWrite.mvpMtx);
			_deviceResources->structuredMemoryView.getElement(1, 0, dynamicSlice).setValue(&srcWrite.lightDirModel);
			_deviceResources->ubo->getDeviceMemory()->unmap();
		}
		submitInfo.commandBuffers = &_deviceResources->mainCommandBuffer[swapchainIndex];
	}

	_hostMutex.lock();
	//submit
	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameCommandBufferFence[swapchainIndex]);
	_hostMutex.unlock();

	if (this->shouldTakeScreenshot())
	{
		if (_deviceResources->swapchain->supportsUsage(VkImageUsageFlags::e_TRANSFER_SRC_BIT))
		{
			pvr::utils::takeScreenshot(_deviceResources->swapchain, swapchainIndex, _deviceResources->commandPool, _deviceResources->queue, this->getScreenshotFileName());
		}
		else
		{
			Log(LogLevel::Warning, "Could not take screenshot as the swapchain does not support TRANSFER_SRC_BIT");
		}
	}

	//present
	pvrvk::PresentInfo present;
	present.swapchains = &_deviceResources->swapchain;
	present.imageIndices = &swapchainIndex;
	present.numSwapchains = 1;
	present.waitSemaphores = &_deviceResources->semaphorePresent[_frameId];
	present.numWaitSemaphores = 1;
	_deviceResources->queue->present(present);

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  Draws a assets::Mesh after the model view matrix has been set and the material prepared.
\param  nodeIndex Node index of the mesh to draw
***********************************************************************************************************************/
void VulkanMultithreading::drawMesh(pvrvk::CommandBuffer& commandBuffer, int nodeIndex)
{
	uint32_t meshId = _scene->getNode(nodeIndex).getObjectId();
	const pvr::assets::Mesh& mesh = _scene->getMesh(meshId);

	// bind the VBO for the mesh
	commandBuffer->bindVertexBuffer(_deviceResources->vbo[meshId], 0, 0);

	//  The geometry can be exported in 4 ways:
	//  - Indexed Triangle list
	//  - Non-Indexed Triangle list
	//  - Indexed Triangle strips
	//  - Non-Indexed Triangle strips
	if (mesh.getNumStrips() == 0)
	{
		// Indexed Triangle list
		if (_deviceResources->ibo[meshId].isValid())
		{
			commandBuffer->bindIndexBuffer(_deviceResources->ibo[meshId], 0, pvr::utils::convertToVk(mesh.getFaces().getDataType()));
			commandBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
		}
		else
		{
			// Non-Indexed Triangle list
			commandBuffer->draw(0, mesh.getNumFaces() * 3, 0, 1);
		}
	}
	else
	{
		for (int i = 0; i < (int)mesh.getNumStrips(); ++i)
		{
			int offset = 0;
			if (_deviceResources->ibo[meshId].isValid())
			{
				// Indexed Triangle strips
				commandBuffer->bindIndexBuffer(_deviceResources->ibo[meshId], 0, pvr::utils::convertToVk(mesh.getFaces().getDataType()));
				commandBuffer->drawIndexed(0, mesh.getStripLength(i) + 2, offset * 2, 0, 1);
			}
			else
			{
				// Non-Indexed Triangle strips
				commandBuffer->draw(0, mesh.getStripLength(i) + 2, 0, 1);
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
	const pvrvk::ClearValue clearValues[] =
	{
		pvrvk::ClearValue(0.00, 0.70, 0.67, 1.f),
		pvrvk::ClearValue(1.f, 0u)
	};
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		pvrvk::CommandBuffer& commandBuffer = _deviceResources->mainCommandBuffer[i];
		commandBuffer->begin();
		commandBuffer->beginRenderPass(_deviceResources->framebuffer[i], pvrvk::Rect2Di(0, 0, getWidth(), getHeight()), true,
		                               clearValues, ARRAY_SIZE(clearValues));
		// enqueue the static states which wont be changed through out the frame
		commandBuffer->bindPipeline(_deviceResources->pipe);
		commandBuffer->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS, _deviceResources->pipelayout, 0, _deviceResources->texDescSet, 0);
		commandBuffer->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS, _deviceResources->pipelayout, 1, _deviceResources->uboDescSet[i]);
		drawMesh(commandBuffer, 0);

		// record the uirenderer commands
		_deviceResources->uiRenderer.beginRendering(commandBuffer);
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.endRendering();
		commandBuffer->endRenderPass();
		commandBuffer->end();
	}
}

/*!*********************************************************************************************************************
\brief  Pre record the commands
***********************************************************************************************************************/
void VulkanMultithreading::recordLoadingCommandBuffer()
{
	const pvrvk::ClearValue clearColor[2] =
	{
		pvrvk::ClearValue(0.00, 0.70, 0.67, 1.f),
		pvrvk::ClearValue(1.f, 0u)
	};

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		pvrvk::CommandBuffer& commandBuffer = _deviceResources->loadingTextCommandBuffer[i];
		commandBuffer->begin();

		commandBuffer->beginRenderPass(_deviceResources->framebuffer[i], true, clearColor, ARRAY_SIZE(clearColor));


		_deviceResources->loadingText[i] = _deviceResources->uiRenderer.createText("Loading...");
		_deviceResources->loadingText[i]->commitUpdates();

		// record the uirenderer commands
		_deviceResources->uiRenderer.beginRendering(commandBuffer);
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->loadingText[i]->render();
		_deviceResources->uiRenderer.endRendering();

		commandBuffer->endRenderPass();
		commandBuffer->end();
	}
}

/*!*********************************************************************************************************************
\return Return an auto ptr to the demo supplied by the user
\brief  This function must be implemented by the user of the shell. The user should return its
    Shell object defining the behavior of the application.
***********************************************************************************************************************/
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::unique_ptr<pvr::Shell>(new VulkanMultithreading()); }
