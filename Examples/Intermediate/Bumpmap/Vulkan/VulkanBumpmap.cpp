/*!*********************************************************************************************************************
\File         VulkanBumpmap.cpp
\Title        Bump mapping
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief      Shows how to perform tangent space bump mapping
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"

const float RotateY = glm::pi<float>() / 150;
const glm::vec4 LightDir(.24f, .685f, -.685f, 0.0f);

/*!*********************************************************************************************************************
 shader attributes
 ***********************************************************************************************************************/
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
const char FragShaderSrcFile[] = "FragShader.fsh";
const char VertShaderSrcFile[] = "VertShader.vsh";

// PVR texture files
const char StatueTexFile[] = "Marble.pvr";
const char StatueNormalMapFile[] = "MarbleNormalMap.pvr";

const char ShadowTexFile[] = "Shadow.pvr";
const char ShadowNormalMapFile[] = "ShadowNormalMap.pvr";

// POD _scene files
const char SceneFile[] = "scene.pod";

/*!*********************************************************************************************************************
 Class implementing the Shell functions.
 ***********************************************************************************************************************/
class VulkanBumpmap : public pvr::Shell
{
	struct DeviceResources
	{
		pvrvk::Instance instance;
		pvrvk::Device device;
		pvrvk::Swapchain swapchain;
		pvrvk::CommandPool commandPool;
		pvrvk::DescriptorPool descriptorPool;
		pvrvk::Queue queue;
		pvrvk::Semaphore semaphoreImageAcquired[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Fence perFrameAcquireFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Semaphore semaphorePresent[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Fence perFrameCommandBufferFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		std::vector<pvrvk::Buffer> vbo;
		std::vector<pvrvk::Buffer> ibo;
		pvrvk::DescriptorSetLayout texLayout;
		pvrvk::DescriptorSetLayout uboLayoutDynamic;
		pvrvk::PipelineLayout pipelayout;
		pvrvk::DescriptorSet texDescSet;
		pvrvk::GraphicsPipeline pipe;
		pvr::Multi<pvrvk::CommandBuffer> commandBuffer;// per swapchain
		pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;// per swapchain
		pvr::Multi<pvrvk::ImageView> depthStencilImages;
		pvr::Multi<pvrvk::DescriptorSet> uboDescSet;
		pvr::utils::StructuredBufferView structuredBufferView;
		pvrvk::Buffer ubo;
		std::vector<pvr::utils::ImageUploadResults> imageUploads;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;
	};

	struct UboPerMeshData
	{
		glm::mat4 mvpMtx;
		glm::vec3 lightDirModel;
	};

	// 3D Model
	pvr::assets::ModelHandle _scene;

	// Projection and view matrix
	glm::mat4 _viewProj;

	uint32_t _frameId;
	// The translation and Rotate parameter of Model
	float _angleY;
	std::unique_ptr<DeviceResources> _deviceResources;

public:
	VulkanBumpmap() {}
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	bool createImageSamplerDescriptor(pvrvk::CommandBuffer& imageUploadCmd);
	bool createUbo();
	bool createPipeline();
	void drawMesh(pvrvk::CommandBuffer& commandBuffer, int i32NodeIndex);
	void recordCommandBuffer();
	bool loadAndUploadImage(const char* fileName, pvrvk::CommandBuffer& uploadCmdBuffer, pvrvk::ImageView& outImage)
	{
		_deviceResources->imageUploads.push_back(pvr::utils::loadAndUploadImage(_deviceResources->device, fileName, true,
		    uploadCmdBuffer, *this));
		if (_deviceResources->imageUploads.back().getImageView().isNull())
		{
			return false;
		}
		outImage = _deviceResources->imageUploads.back().getImageView();
		return true;
	}
};

/*!*********************************************************************************************************************
\return return true if no error occurred
\brief  Loads the textures required for this training course
***********************************************************************************************************************/
bool VulkanBumpmap::createImageSamplerDescriptor(pvrvk::CommandBuffer& imageUploadCmd)
{
	pvrvk::Device& device = _deviceResources->device;
	pvrvk::ImageView texBase;
	pvrvk::ImageView texNormalMap;

	// create the bilinear sampler
	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.magFilter = VkFilter::e_LINEAR;
	samplerInfo.minFilter = VkFilter::e_LINEAR;
	samplerInfo.mipMapMode = VkSamplerMipmapMode::e_NEAREST;
	pvrvk::Sampler samplerMipBilinear = device->createSampler(samplerInfo);

	samplerInfo.mipMapMode = VkSamplerMipmapMode::e_LINEAR;
	pvrvk::Sampler samplerTrilinear = device->createSampler(samplerInfo);

	if (!loadAndUploadImage(StatueTexFile, imageUploadCmd, texBase) ||
	    !loadAndUploadImage(StatueNormalMapFile, imageUploadCmd, texNormalMap))
	{
		setExitMessage("ERROR: Failed to load texture.");
		return false;
	}
	// create the descriptor set
	_deviceResources->texDescSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->texLayout);

	pvrvk::WriteDescriptorSet writeDescSets[2] =
	{
		pvrvk::WriteDescriptorSet(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER,
		                          _deviceResources->texDescSet, 0),
		pvrvk::WriteDescriptorSet(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER,
		                          _deviceResources->texDescSet, 1)
	};
	writeDescSets[0].setImageInfo(0, pvrvk::DescriptorImageInfo(texBase, samplerMipBilinear));
	writeDescSets[1].setImageInfo(0, pvrvk::DescriptorImageInfo(texNormalMap, samplerTrilinear));


	if (!_deviceResources->texDescSet.isValid())
	{
		setExitMessage("ERROR: Failed to create Combined Image Sampler Descriptor set.");
		return false;
	}
	_deviceResources->device->updateDescriptorSets(writeDescSets, ARRAY_SIZE(writeDescSets), nullptr, 0);
	return true;
}

bool VulkanBumpmap::createUbo()
{
	pvrvk::WriteDescriptorSet descUpdate[pvrvk::FrameworkCaps::MaxSwapChains];
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("MVPMatrix", pvr::GpuDatatypes::mat4x4);
		desc.addElement("LightDirModel", pvr::GpuDatatypes::vec3);

		_deviceResources->structuredBufferView.initDynamic(desc, _scene->getNumMeshNodes() * _deviceResources->swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
		    static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment));
		_deviceResources->ubo = pvr::utils::createBuffer(_deviceResources->device,_deviceResources->structuredBufferView.getSize(),
		                        VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);
	}
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->uboDescSet.add(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->uboLayoutDynamic));
		descUpdate[i].set(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->uboDescSet[i])
		.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->ubo,
		               0, _deviceResources->structuredBufferView.getDynamicSliceSize()));
	}
	_deviceResources->device->updateDescriptorSets(descUpdate, _deviceResources->swapchain->getSwapchainLength(), nullptr, 0);
	return true;
}

/*!*********************************************************************************************************************
\return  Return true if no error occurred
\brief  Loads and compiles the shaders and create a pipeline
***********************************************************************************************************************/
bool VulkanBumpmap::createPipeline()
{
	pvrvk::PipelineColorBlendAttachmentState colorAttachemtState;
	pvrvk::GraphicsPipelineCreateInfo pipeInfo;
	colorAttachemtState.blendEnable = false;

	//--- create the texture-sampler descriptor set layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
		descSetLayoutInfo.setBinding(0, VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 1,
		                             VkShaderStageFlags::e_FRAGMENT_BIT);/*binding 0*/
		descSetLayoutInfo.setBinding(1, VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 1,
		                             VkShaderStageFlags::e_FRAGMENT_BIT);/*binding 1*/
		_deviceResources->texLayout = _deviceResources->device->createDescriptorSetLayout(descSetLayoutInfo);
	}

	//--- create the ubo descriptorset layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
		descSetLayoutInfo.setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1,
		                             VkShaderStageFlags::e_VERTEX_BIT); /*binding 0*/
		_deviceResources->uboLayoutDynamic = _deviceResources->device->createDescriptorSetLayout(descSetLayoutInfo);
	}

	//--- create the pipeline layout
	{
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo; pipeLayoutInfo
		.addDescSetLayout(_deviceResources->texLayout)/*set 0*/
		.addDescSetLayout(_deviceResources->uboLayoutDynamic);/*set 1*/
		_deviceResources->pipelayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
	}

	const pvrvk::Rect2Di rect(0, 0, _deviceResources->swapchain->getDimension().width, _deviceResources->swapchain->getDimension().height);
	pipeInfo.viewport.setViewportAndScissor(0, pvrvk::Viewport(rect), rect);
	pipeInfo.rasterizer.setCullMode(VkCullModeFlags::e_BACK_BIT);
	pipeInfo.colorBlend.setAttachmentState(0, colorAttachemtState);

	pvr::assets::ShaderFile fileVersioner;
	fileVersioner.populateValidVersions(VertShaderSrcFile, *this);
	pipeInfo.vertexShader = _deviceResources->device->createShader(fileVersioner.getBestStreamForApi(pvr::Api::Vulkan)
	                        ->readToEnd<uint32_t>());

	fileVersioner.populateValidVersions(FragShaderSrcFile, *this);
	pipeInfo.fragmentShader = _deviceResources->device->createShader(fileVersioner.getBestStreamForApi(pvr::Api::Vulkan)
	                          ->readToEnd<uint32_t>());

	const pvr::assets::Mesh& mesh = _scene->getMesh(0);
	pipeInfo.inputAssembler.setPrimitiveTopology(pvr::utils::convertToVk(mesh.getPrimitiveType()));
	pipeInfo.pipelineLayout = _deviceResources->pipelayout;
	pipeInfo.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
	pipeInfo.subpass = 0;
	// Enable z-buffer test. We are using a projection matrix optimized for a floating point depth buffer,
	// so the depth test and clear value need to be inverted (1 becomes near, 0 becomes far).
	pipeInfo.depthStencil.enableDepthTest(true);
	pipeInfo.depthStencil.setDepthCompareFunc(VkCompareOp::e_LESS);
	pipeInfo.depthStencil.enableDepthWrite(true);
	pvr::utils::populateInputAssemblyFromMesh(mesh, VertexAttribBindings, sizeof(VertexAttribBindings) /
	    sizeof(VertexAttribBindings[0]), pipeInfo.vertexInput, pipeInfo.inputAssembler);
	_deviceResources->pipe = _deviceResources->device->createGraphicsPipeline(pipeInfo);
	return _deviceResources->pipe.isValid();
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in initApplication() will be called by Shell once per run, before the rendering context is created.
    Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
    If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result VulkanBumpmap::initApplication()
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
\return Return Result::Success if no error occurred
\brief  Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
    If the rendering context is lost, quitApplication() will not be called.x
***********************************************************************************************************************/
pvr::Result VulkanBumpmap::quitApplication() { return pvr::Result::Success;}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
    Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result VulkanBumpmap::initView()
{
	_frameId = 0;
	_deviceResources.reset(new DeviceResources());
	pvrvk::Surface surface ;
	if (!pvr::utils::createInstanceAndSurface(this->getApplicationName(), this->getWindow(), this->getDisplay(), _deviceResources->instance, surface))
	{
		return pvr::Result::UnknownError;
	}

	const pvr::utils::QueuePopulateInfo queuePopulateInfo =
	{ VkQueueFlags::e_GRAPHICS_BIT,  surface };

	pvr::utils::QueueAccessInfo queueAccessInfo;
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), &queuePopulateInfo, 1, &queueAccessInfo);
	if (_deviceResources->device.isNull())
	{
		return pvr::Result::UnknownError;
	}
	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(_deviceResources->instance->getSurface());

	// validate the supported swapchain image usage
	VkImageUsageFlags swapchainImageUsage = VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, VkImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= VkImageUsageFlags::e_TRANSFER_SRC_BIT;
	}

	//---------------
	// Create the swapchain
	if (!pvr::utils::createSwapchainAndDepthStencilImageView(_deviceResources->device, surface, getDisplayAttributes(),
	    _deviceResources->swapchain, _deviceResources->depthStencilImages, swapchainImageUsage))
	{
		return pvr::Result::UnknownError;
	}

	//---------------
	// Create the commandpool and descriptorset pool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(_deviceResources->queue->getQueueFamilyId(),
	                                VkCommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT);

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
	                                   .addDescriptorInfo(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 16)
	                                   .addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 16)
	                                   .addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER, 16)
	                                   .setMaxDescriptorSets(16));

	// load the vbo and ibo data
	pvr::utils::appendSingleBuffersFromModel(_deviceResources->device, *_scene, _deviceResources->vbo, _deviceResources->ibo);

	// create an onscreen framebuffer per swap chain
	if (!pvr::utils::createOnscreenFramebufferAndRenderpass(_deviceResources->swapchain, &_deviceResources->depthStencilImages[0], _deviceResources->onScreenFramebuffer))
	{
		return pvr::Result::UnknownError;
	}

	//---------------
	// load the pipeline
	if (!createPipeline())
	{
		return pvr::Result::UnknownError;
	}

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		// create the per swapchain command buffers
		_deviceResources->commandBuffer[i] = _deviceResources->commandPool->allocateCommandBuffer();
		if (i == 0)
		{
			_deviceResources->commandBuffer[0]->begin();
		}

		_deviceResources->semaphorePresent[i] = _deviceResources->device->createSemaphore();
		_deviceResources->semaphoreImageAcquired[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameCommandBufferFence[i] = _deviceResources->device->createFence(VkFenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->perFrameAcquireFence[i] = _deviceResources->device->createFence(VkFenceCreateFlags::e_SIGNALED_BIT);
	}
	// create the image samplers
	if (!createImageSamplerDescriptor(_deviceResources->commandBuffer[0])) { return pvr::Result::UnknownError; }
	_deviceResources->commandBuffer[0]->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->commandBuffer[0];
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle();
	_deviceResources->imageUploads.clear();// clear up the image upload resuls

	//  Initialize UIRenderer
	if (!_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
	                                       _deviceResources->commandPool, _deviceResources->queue))
	{
		this->setExitMessage("ERROR: Cannot initialize UIRenderer\n");
		return pvr::Result::UnknownError;
	}
	_deviceResources->uiRenderer.getDefaultTitle()->setText("BumpMap");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();

	// create the uniform buffers
	if (!createUbo()) { return pvr::Result::UnknownError; }

	glm::vec3 from, to, up;
	float fov;
	_scene->getCameraProperties(0, fov, from, to, up);

	// Is the screen rotated
	const bool bRotate = this->isScreenRotated() && this->isFullScreen();

	//  Calculate the projection and rotate it by 90 degree if the screen is rotated.
	_viewProj = (bRotate ? pvr::math::perspectiveFov(pvr::Api::Vulkan, fov, (float)this->getHeight(), (float)this->getWidth(),
	             _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar(), glm::pi<float>() * .5f) :
	             pvr::math::perspectiveFov(pvr::Api::Vulkan, fov, (float)this->getWidth(), (float)this->getHeight(),
	                                       _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar()));

	_viewProj = _viewProj * glm::lookAt(from, to, up);

	// record the command buffers
	recordCommandBuffer();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  Code in releaseView() will be called by PVRShell when theapplication quits or before a change in the rendering context.
\return Return Result::Success if no error occurred
***********************************************************************************************************************/
pvr::Result VulkanBumpmap::releaseView()
{
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); i++)
	{
		_deviceResources->perFrameAcquireFence[i]->wait();
		_deviceResources->perFrameAcquireFence[i]->reset();

		_deviceResources->perFrameCommandBufferFence[i]->wait();
		_deviceResources->perFrameCommandBufferFence[i]->reset();
	}

	_deviceResources->device->waitIdle();
	_scene.reset();
	_deviceResources.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result VulkanBumpmap::renderFrame()
{
	_deviceResources->perFrameAcquireFence[_frameId]->wait();
	_deviceResources->perFrameAcquireFence[_frameId]->reset();
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->semaphoreImageAcquired[_frameId], _deviceResources->perFrameAcquireFence[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameCommandBufferFence[swapchainIndex]->wait();
	_deviceResources->perFrameCommandBufferFence[swapchainIndex]->reset();

	// Calculate the model matrix
	const glm::mat4 mModel = glm::rotate(_angleY, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(1.8f));
	_angleY += -RotateY * 0.05f  * getFrameTime();

	// Set light Direction in model space
	//  The inverse of a rotation matrix is the transposed matrix
	//  Because of v * M = transpose(M) * v, this means:
	//  v * R == inverse(R) * v
	//  So we don't have to actually invert or transpose the matrix
	//  to transform back from world space to model space

	//---------------
	// update the ubo
	UboPerMeshData srcWrite;
	srcWrite.lightDirModel = glm::vec3(LightDir * mModel);
	srcWrite.mvpMtx = _viewProj * mModel * _scene->getWorldMatrix(_scene->getNode(0).getObjectId());
	void* memory;
	_deviceResources->ubo->getDeviceMemory()->map(&memory, _deviceResources->structuredBufferView.getDynamicSliceOffset(swapchainIndex),
	    _deviceResources->structuredBufferView.getDynamicSliceSize());
	_deviceResources->structuredBufferView.pointToMappedMemory(memory, swapchainIndex);
	_deviceResources->structuredBufferView.getElementByName("MVPMatrix", 0, swapchainIndex).setValue(&srcWrite.mvpMtx);
	_deviceResources->structuredBufferView.getElementByName("LightDirModel", 0, swapchainIndex).setValue(&srcWrite.lightDirModel);
	_deviceResources->ubo->getDeviceMemory()->unmap();

	//---------------
	// SUBMIT
	pvrvk::SubmitInfo submitInfo;
	VkPipelineStageFlags pipeWaitStageFlags = VkPipelineStageFlags::e_ALL_GRAPHICS_BIT;
	submitInfo.commandBuffers = &_deviceResources->commandBuffer[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->semaphoreImageAcquired[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->semaphorePresent[_frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.waitDestStages = &pipeWaitStageFlags;
	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameCommandBufferFence[swapchainIndex]);

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

	//---------------
	// PRESENT
	pvrvk::PresentInfo presentInfo;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.waitSemaphores = &_deviceResources->semaphorePresent[_frameId];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.imageIndices = &swapchainIndex;
	_deviceResources->queue->present(presentInfo);

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  Draws a assets::Mesh after the model view matrix has been set and the material prepared.
\param  nodeIndex Node index of the mesh to draw
***********************************************************************************************************************/
void VulkanBumpmap::drawMesh(pvrvk::CommandBuffer& commandBuffer, int nodeIndex)
{
	const uint32_t meshId = _scene->getNode(nodeIndex).getObjectId();
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
		for (uint32_t i = 0; i < mesh.getNumStrips(); ++i)
		{
			uint32_t offset = 0;
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
void VulkanBumpmap::recordCommandBuffer()
{
	const uint32_t numSwapchains = _deviceResources->swapchain->getSwapchainLength();
	pvrvk::ClearValue clearValues[2] =
	{
		pvrvk::ClearValue(0.00, 0.70, 0.67, 1.f),
		pvrvk::ClearValue(1.f, 0u)
	};
	for (uint32_t i = 0; i < numSwapchains; ++i)
	{
		// begin recording commands for the current swap chain command buffer
		_deviceResources->commandBuffer[i]->begin();

		// begin the render pass
		_deviceResources->commandBuffer[i]->beginRenderPass(_deviceResources->onScreenFramebuffer[i], pvrvk::Rect2Di(0, 0,
		    getWidth(), getHeight()), true, clearValues, ARRAY_SIZE(clearValues));

		// calculate the dynamic offset to use
		const uint32_t dynamicOffset = _deviceResources->structuredBufferView.getDynamicSliceOffset(i);
		// enqueue the static states which wont be changed through out the frame
		_deviceResources->commandBuffer[i]->bindPipeline(_deviceResources->pipe);
		_deviceResources->commandBuffer[i]->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
		    _deviceResources->pipelayout, 0, _deviceResources->texDescSet, 0);

		_deviceResources->commandBuffer[i]->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
		    _deviceResources->pipelayout, 1, _deviceResources->uboDescSet[i], &dynamicOffset, 1);

		drawMesh(_deviceResources->commandBuffer[i], 0);

		// record the ui renderer commands
		_deviceResources->uiRenderer.beginRendering(_deviceResources->commandBuffer[i]);
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.endRendering();

		// end the renderpass
		_deviceResources->commandBuffer[i]->endRenderPass();

		// end recording commands for the current command buffer
		_deviceResources->commandBuffer[i]->end();
	}
}

/*!*********************************************************************************************************************
\return Return an auto ptr to the demo supplied by the user
\brief  This function must be implemented by the user of the shell. The user should return its
    Shell object defining the behavior of the application.
***********************************************************************************************************************/
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::unique_ptr<pvr::Shell>(new VulkanBumpmap()); }
