/*!*********************************************************************************************************************
\File         VulkanIntroducingPVRUtils.cpp
\Title        Introducing the PowerVR Framework
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief      Shows how to use the PVRApi library together with loading models from POD files and rendering them with effects from PFX files.
***********************************************************************************************************************/
#include "PVRCore/PVRCore.h"
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"

pvr::utils::VertexBindings Attributes[] =
{
	{"POSITION", 0},
	{"NORMAL", 1},
	{"UV0", 2}
};

/*!*********************************************************************************************************************
 Content file names
***********************************************************************************************************************/
const char VertShaderFileName[] = "VertShader_vk.spv";
const char FragShaderFileName[] = "FragShader_vk.spv";
const char SceneFileName[] = "GnomeToy.pod"; // POD scene files

/*!*********************************************************************************************************************
 Class implementing the pvr::Shell functions.
***********************************************************************************************************************/
class VulkanIntroducingPVRUtils : public pvr::Shell
{
	typedef std::pair<int32_t, pvrvk::DescriptorSet> MaterialDescSet;
	struct DeviceResources
	{
		pvrvk::Instance instance;
		pvrvk::Surface surface;
		pvrvk::Device device;
		pvrvk::Swapchain swapchain;
		pvr::Multi<pvrvk::ImageView> depthStencilImages;
		pvrvk::Queue queue;

		pvrvk::CommandPool commandPool;
		pvrvk::DescriptorPool descriptorPool;

		pvr::Multi<pvrvk::Semaphore> semaphoreAcquire;
		pvr::Multi<pvrvk::Semaphore> semaphoreSubmit;
		pvr::Multi<pvrvk::Fence> perFrameFence;
		// The Vertex buffer object handle array.
		std::vector<pvrvk::Buffer> vbos;
		std::vector<pvrvk::Buffer> ibos;

		// the framebuffer used in the demo
		pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;

		// main command buffer used to store rendering commands
		pvr::Multi<pvrvk::CommandBuffer> commandBuffers;

		// descriptor sets
		std::vector<MaterialDescSet> texDescSets;
		pvr::Multi<pvrvk::DescriptorSet> matrixUboDescSets;
		pvr::Multi<pvrvk::DescriptorSet> lightUboDescSets;

		// structured memory views
		pvr::utils::StructuredBufferView matrixMemoryView;
		pvrvk::Buffer matrixBuffer;
		pvr::utils::StructuredBufferView lightMemoryView;
		pvrvk::Buffer lightBuffer;

		// samplers
		pvrvk::Sampler samplerTrilinear;

		// descriptor set layouts
		pvrvk::DescriptorSetLayout texDescSetLayout;
		pvrvk::DescriptorSetLayout uboDescSetLayoutDynamic, uboDescSetLayoutStatic;

		// pipeline layout
		pvrvk::PipelineLayout pipelineLayout;

		// graphics pipeline
		pvrvk::GraphicsPipeline pipeline;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;
		~DeviceResources()
		{
			if (device.isValid())
			{
				device->waitIdle();
			}
		}
	};
	std::unique_ptr<DeviceResources> _deviceResources;

	// 3D Model
	pvr::assets::ModelHandle _scene;

	// Projection and Model View matrices
	glm::mat4 _projMtx;
	glm::mat4 _viewMtx;

	// Variables to handle the animation in a time-based manner
	float _frame;

	uint32_t _frameId;

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void createBuffers();
	bool createDescriptorSets(pvrvk::CommandBuffer& uploadCmd,
	                          std::vector<pvr::utils::ImageUploadResults>& uploadResuls);
	void recordCommandBuffers();
	void createPipeline();
	void createDescriptorSetLayouts();
};

struct DescripotSetComp
{
	int32_t id;
	DescripotSetComp(int32_t id) : id(id) {}
	bool operator()(std::pair<int32_t, pvrvk::DescriptorSet> const& pair)  { return pair.first == id;  }
};


/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Code in initApplication() will be called by Shell once per run, before the rendering context is created.
    Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.). If the rendering
    context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRUtils::initApplication()
{
	_deviceResources.reset(new DeviceResources());

	// Load the _scene
	if ((_scene = pvr::assets::Model::createWithReader(pvr::assets::PODReader(getAssetStream(SceneFileName)))).isNull())
	{
		this->setExitMessage("ERROR: Couldn't load the %s file\n", SceneFileName);
		return pvr::Result::NotInitialized;
	}

	// The cameras are stored in the file. We check it contains at least one.
	if (_scene->getNumCameras() == 0)
	{
		this->setExitMessage("ERROR: The _scene does not contain a camera\n");
		return pvr::Result::UnknownError;
	}

	// Ensure that all meshes use an indexed triangle list
	for (uint32_t i = 0; i < _scene->getNumMeshes(); ++i)
	{
		if (_scene->getMesh(i).getPrimitiveType() != pvr::PrimitiveTopology::TriangleList ||
		    _scene->getMesh(i).getFaces().getDataSize() == 0)
		{
			this->setExitMessage("ERROR: The meshes in the _scene should use an indexed triangle list\n");
			return pvr::Result::UnknownError;
		}
	}

	// Initialize variables used for the animation
	_frame = 0;
	_frameId = 0;

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.
        If the rendering context is lost, quitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRUtils::quitApplication()
{
	_scene.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Code in initView() will be called by Shell upon initialization or after a change  in the rendering context.
        Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRUtils::initView()
{
	//---------------
	// Create instance
	if (!pvr::utils::createInstanceAndSurface(this->getApplicationName(), this->getWindow(), this->getDisplay(), _deviceResources->instance, _deviceResources->surface))
	{
		return pvr::Result::UnknownError;
	}

	pvr::utils::QueueAccessInfo queueAccessInfo;
	const pvr::utils::QueuePopulateInfo queuePopulateInfo =
	{
		VkQueueFlags::e_GRAPHICS_BIT, _deviceResources->surface
	};

	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0),
	                           &queuePopulateInfo, 1, &queueAccessInfo);
	if (_deviceResources->device.isNull())
	{
		return pvr::Result::UnknownError;
	}

	// Get the queue
	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(_deviceResources->surface);

	// validate the supported swapchain image usage
	VkImageUsageFlags swapchainImageUsage = VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, VkImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= VkImageUsageFlags::e_TRANSFER_SRC_BIT;
	}

	// Create the swapchain
	if (!pvr::utils::createSwapchainAndDepthStencilImageView(_deviceResources->device,
	    _deviceResources->surface, getDisplayAttributes(), _deviceResources->swapchain, _deviceResources->depthStencilImages, swapchainImageUsage))
	{
		return pvr::Result::UnknownError;
	}

	if (!pvr::utils::createOnscreenFramebufferAndRenderpass(_deviceResources->swapchain, &_deviceResources->depthStencilImages[0],
	    _deviceResources->onScreenFramebuffer))
	{
		return pvr::Result::UnknownError;
	}

	//---------------
	// Create the Comandpool & Descriptorpool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(queueAccessInfo.familyId,
	                                VkCommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT);
	if (!_deviceResources->commandPool.isValid())
	{
		return pvr::Result::UnknownError;
	}

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
	                                   .addDescriptorInfo(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 16)
	                                   .addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 16)
	                                   .addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER, 16)
	                                   .setMaxDescriptorSets(16));
	if (!_deviceResources->descriptorPool.isValid())
	{
		return pvr::Result::UnknownError;
	}

	pvr::utils::appendSingleBuffersFromModel(_deviceResources->device, *_scene, _deviceResources->vbos, _deviceResources->ibos);

	// We check the _scene contains at least one light
	if (_scene->getNumLights() == 0)
	{
		("The _scene does not contain a light\n");
		return pvr::Result::UnknownError;
	}

	// create demo buffers
	createBuffers();

	// Create per swapchain resource
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->semaphoreAcquire[i] = _deviceResources->device->createSemaphore();
		_deviceResources->semaphoreSubmit[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameFence[i] = _deviceResources->device->createFence(VkFenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->commandBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
	}

	// create the descriptor set layouts and pipeline layouts
	createDescriptorSetLayouts();

	// create the descriptor sets
	_deviceResources->commandBuffers[0]->begin();
	std::vector<pvr::utils::ImageUploadResults> imageUploads;
	createDescriptorSets(_deviceResources->commandBuffers[0], imageUploads);
	_deviceResources->commandBuffers[0]->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->commandBuffers[0];
	submitInfo.numCommandBuffers = 1;

	// submit the queue and wait for it to become idle
	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle();

	if (!_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
	                                       _deviceResources->commandPool, _deviceResources->queue))
	{
		setExitMessage("Failed top initialize the UIRenderer");
		return pvr::Result::NotInitialized;
	}
	_deviceResources->uiRenderer.getDefaultTitle()->setText("VulkanIntroducingPVRUtils").commitUpdates();
	imageUploads.clear();
	_deviceResources->commandBuffers[0]->reset(VkCommandBufferResetFlags::e_RELEASE_RESOURCES_BIT);

	// create demo graphics pipeline
	createPipeline();

	// record the rendering commands
	recordCommandBuffers();

	// Calculates the projection matrix
	bool isRotated = this->isScreenRotated() && this->isFullScreen();
	if (isRotated)
	{
		_projMtx = pvr::math::perspective(pvr::Api::Vulkan, _scene->getCamera(0).getFOV(),
		                                  (float)this->getHeight() / (float)this->getWidth(),
		                                  _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar(), glm::pi<float>() * .5f);
	}
	else
	{
		_projMtx = pvr::math::perspective(pvr::Api::Vulkan, _scene->getCamera(0).getFOV(), (float)this->getWidth() / (float)this->getHeight(),
		                                  _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar());
	}

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRUtils::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every _frame.
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRUtils::renderFrame()
{
	_deviceResources->perFrameFence[_frameId]->wait();
	_deviceResources->perFrameFence[_frameId]->reset();

	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->semaphoreAcquire[_frameId]);
	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	//  Calculates the _frame number to animate in a time-based manner.
	//  get the time in milliseconds.
	_frame += (float)getFrameTime() / 30.f; // design-time target fps for animation

	if (_frame >= _scene->getNumFrames() - 1) { _frame = 0; }

	// Sets the _scene animation to this _frame
	_scene->setCurrentFrame(_frame);

	//  We can build the world view matrix from the camera position, target and an up vector.
	//  A _scene is composed of nodes. There are 3 types of nodes:
	//  - MeshNodes :
	//    references a mesh in the getMesh().
	//    These nodes are at the beginning of of the Nodes array.
	//    And there are nNumMeshNode number of them.
	//    This way the .pod format can instantiate several times the same mesh
	//    with different attributes.
	//  - lights
	//  - cameras
	//  To draw a _scene, you must go through all the MeshNodes and draw the referenced meshes.
	float fov;
	glm::vec3 cameraPos, cameraTarget, cameraUp;
	_scene->getCameraProperties(0, fov, cameraPos, cameraTarget, cameraUp);
	_viewMtx = glm::lookAt(cameraPos, cameraTarget, cameraUp);

	// update the matrix uniform buffer
	{
		// only update the current swapchain ubo
		void* memory;
		uint32_t mappedDynamicSlice = swapchainIndex * _scene->getNumMeshNodes();
		_deviceResources->matrixBuffer->getDeviceMemory()->map(&memory, _deviceResources->matrixMemoryView.getDynamicSliceOffset(mappedDynamicSlice),
		    _deviceResources->matrixMemoryView.getDynamicSliceSize() * _scene->getNumMeshNodes());

		_deviceResources->matrixMemoryView.pointToMappedMemory(memory, mappedDynamicSlice);

		glm::mat4 tempMtx;
		for (uint32_t i = 0; i < _scene->getNumMeshNodes(); ++i)
		{
			uint32_t dynamicSlice = i + mappedDynamicSlice;
			tempMtx = _viewMtx * _scene->getWorldMatrix(i);
			_deviceResources->matrixMemoryView.getElementByName("MVP", 0, dynamicSlice).setValue(_projMtx * tempMtx);
			_deviceResources->matrixMemoryView.getElementByName("WorldViewItMtx", 0, dynamicSlice).setValue(glm::inverseTranspose(glm::mat3x3(tempMtx)));
		}
		_deviceResources->matrixBuffer->getDeviceMemory()->unmap();
	}

	{
		// update the light direction ubo
		glm::vec3 lightDir3;
		_scene->getLightDirection(0, lightDir3);
		lightDir3 = glm::normalize(glm::mat3(_viewMtx) * lightDir3);

		void* memory;

		uint32_t mappedDynamicSlice = swapchainIndex;

		_deviceResources->lightBuffer->getDeviceMemory()->map(&memory, _deviceResources->lightMemoryView.getDynamicSliceOffset(mappedDynamicSlice),
		    _deviceResources->lightMemoryView.getDynamicSliceSize());
		_deviceResources->lightMemoryView.pointToMappedMemory(memory, mappedDynamicSlice);
		_deviceResources->lightMemoryView.getElementByName("LightDirection", 0, swapchainIndex).setValue(glm::vec4(lightDir3, 1.f));
		_deviceResources->lightBuffer->getDeviceMemory()->unmap();
	}

	// submit
	pvrvk::SubmitInfo submitInfo;
	VkPipelineStageFlags waitStage = VkPipelineStageFlags::e_ALL_GRAPHICS_BIT;
	submitInfo.commandBuffers = &_deviceResources->commandBuffers[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitDestStages  = &waitStage;
	submitInfo.waitSemaphores = &_deviceResources->semaphoreAcquire[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->semaphoreSubmit[_frameId];
	submitInfo.numSignalSemaphores = 1;

	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameFence[_frameId]);

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

	// present
	pvrvk::PresentInfo presentInfo;
	presentInfo.waitSemaphores = &_deviceResources->semaphoreSubmit[_frameId];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.numSwapchains = 1;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.imageIndices = &swapchainIndex;
	_deviceResources->queue->present(presentInfo);

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  Pre-record the rendering commands
***********************************************************************************************************************/
void VulkanIntroducingPVRUtils::recordCommandBuffers()
{
	pvrvk::ClearValue clearValues[2] =
	{
		pvrvk::ClearValue(0.00, 0.70, 0.67, 1.0f),
		pvrvk::ClearValue(1.f, 0)
	};
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		// begin recording commands
		_deviceResources->commandBuffers[i]->begin();

		// begin the renderpass
		_deviceResources->commandBuffers[i]->beginRenderPass(_deviceResources->onScreenFramebuffer[i],
		    pvrvk::Rect2Di(0, 0, getWidth(), getHeight()), true, clearValues, ARRAY_SIZE(clearValues));

		// bind the graphics pipeline
		_deviceResources->commandBuffers[i]->bindPipeline(_deviceResources->pipeline);

		// A scene is composed of nodes. There are 3 types of nodes:
		// - MeshNodes :
		// references a mesh in the getMesh().
		// These nodes are at the beginning of of the Nodes array.
		// And there are nNumMeshNode number of them.
		// This way the .pod format can instantiate several times the same mesh
		// with different attributes.
		// - lights
		// - cameras
		// To draw a scene, you must go through all the MeshNodes and draw the referenced meshes.
		uint32_t offsets[2];
		offsets[0] = 0;
		offsets[1] = 0;

		pvrvk::DescriptorSet descriptorSets[3];
		descriptorSets[1] = _deviceResources->matrixUboDescSets[i];
		descriptorSets[2] = _deviceResources->lightUboDescSets[i];
		for (uint32_t j = 0; j < _scene->getNumMeshNodes(); ++j)
		{
			// get the current mesh node
			const pvr::assets::Model::Node* pNode = &_scene->getMeshNode(j);

			// Gets pMesh referenced by the pNode
			const pvr::assets::Mesh* pMesh = &_scene->getMesh(pNode->getObjectId());

			// get the material id
			int32_t matId = pNode->getMaterialIndex();

			// find the texture descriptor set which matches the current material
			auto found = std::find_if(_deviceResources->texDescSets.begin(), _deviceResources->texDescSets.end(), DescripotSetComp(matId));
			descriptorSets[0] = found->second;

			// get the matrix buffer array offset
			offsets[0] =  _deviceResources->matrixMemoryView.getDynamicSliceOffset(j + i * _scene->getNumMeshNodes());
			offsets[1] = _deviceResources->lightMemoryView.getDynamicSliceOffset(i);

			// bind the descriptor sets
			_deviceResources->commandBuffers[i]->bindDescriptorSets(VkPipelineBindPoint::e_GRAPHICS,
			    _deviceResources->pipelineLayout, 0, descriptorSets, 3, offsets, 2);

			// bind the vbo and ibos for the current mesh node
			_deviceResources->commandBuffers[i]->bindVertexBuffer(_deviceResources->vbos[pNode->getObjectId()], 0, 0);
			_deviceResources->commandBuffers[i]->bindIndexBuffer(_deviceResources->ibos[pNode->getObjectId()], 0,
			    pvr::utils::convertToVk(pMesh->getFaces().getDataType()));

			// draw
			_deviceResources->commandBuffers[i]->drawIndexed(0, pMesh->getNumFaces() * 3, 0, 0, 1);
		}

		// add ui effects using ui renderer
		_deviceResources->uiRenderer.beginRendering(_deviceResources->commandBuffers[i]);
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.endRendering();
		_deviceResources->commandBuffers[i]->endRenderPass();
		_deviceResources->commandBuffers[i]->end();
	}
}

/*!*********************************************************************************************************************
\brief  Creates the descriptor set layouts used throughout the demo.
***********************************************************************************************************************/
void VulkanIntroducingPVRUtils::createDescriptorSetLayouts()
{
	// create the texture descriptor set layout and pipeline layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, VkShaderStageFlags::e_FRAGMENT_BIT);
		_deviceResources->texDescSetLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);
	}

	// create the ubo descriptor set layouts
	{
		// dynamic ubo
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, VkShaderStageFlags::e_VERTEX_BIT); /*binding 0*/
		_deviceResources->uboDescSetLayoutDynamic = _deviceResources->device->createDescriptorSetLayout(descSetInfo);
	}
	{
		//static ubo
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, VkShaderStageFlags::e_VERTEX_BIT);/*binding 0*/
		_deviceResources->uboDescSetLayoutStatic = _deviceResources->device->createDescriptorSetLayout(descSetInfo);
	}

	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
	pipeLayoutInfo.addDescSetLayout(_deviceResources->texDescSetLayout);/* set 0 */
	pipeLayoutInfo.addDescSetLayout(_deviceResources->uboDescSetLayoutDynamic);/* set 1 */
	pipeLayoutInfo.addDescSetLayout(_deviceResources->uboDescSetLayoutStatic);/* set 2 */
	_deviceResources->pipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
}

/*!*********************************************************************************************************************
\brief  Creates the graphics pipeline used in the demo.
***********************************************************************************************************************/
void VulkanIntroducingPVRUtils::createPipeline()
{
	pvrvk::GraphicsPipelineCreateInfo pipeDesc;
	pipeDesc.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
	pipeDesc.rasterizer.setCullMode(VkCullModeFlags::e_BACK_BIT);
	pvr::utils::populateViewportStateCreateInfo(_deviceResources->onScreenFramebuffer[0], pipeDesc.viewport);
	pvr::utils::populateInputAssemblyFromMesh(_scene->getMesh(0), Attributes, 3,
	    pipeDesc.vertexInput, pipeDesc.inputAssembler);

	pvr::Stream::ptr_type vertSource =  getAssetStream(VertShaderFileName);
	pvr::Stream::ptr_type fragSource =  getAssetStream(FragShaderFileName);

	pipeDesc.vertexShader.setShader(_deviceResources->device->createShader(vertSource->readToEnd<uint32_t>()));
	pipeDesc.fragmentShader.setShader(_deviceResources->device->createShader(fragSource->readToEnd<uint32_t>()));

	pipeDesc.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
	pipeDesc.depthStencil.enableDepthTest(true);
	pipeDesc.depthStencil.setDepthCompareFunc(VkCompareOp::e_LESS);
	pipeDesc.depthStencil.enableDepthWrite(true);
	pipeDesc.rasterizer.setCullMode(VkCullModeFlags::e_BACK_BIT);
	pipeDesc.subpass = 0;

	pipeDesc.pipelineLayout = _deviceResources->pipelineLayout;

	_deviceResources->pipeline = _deviceResources->device->createGraphicsPipeline(pipeDesc);
}

/*!*********************************************************************************************************************
\brief  Creates the buffers used throughout the demo.
***********************************************************************************************************************/
void VulkanIntroducingPVRUtils::createBuffers()
{
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("MVP", pvr::GpuDatatypes::mat4x4);
		desc.addElement("WorldViewItMtx", pvr::GpuDatatypes::mat3x3);

		_deviceResources->matrixMemoryView.initDynamic(desc, _scene->getNumMeshNodes() * _deviceResources->swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
		    static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment));
		_deviceResources->matrixBuffer = pvr::utils::createBuffer(_deviceResources->device,_deviceResources->matrixMemoryView.getSize(),
		                                 VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);
	}

	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("LightDirection", pvr::GpuDatatypes::vec4);

		_deviceResources->lightMemoryView.initDynamic(desc, _deviceResources->swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
		    static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment));
		_deviceResources->lightBuffer = pvr::utils::createBuffer(_deviceResources->device,_deviceResources->lightMemoryView.getSize(),
		                                VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);
	}
}

/*!*********************************************************************************************************************
\brief  Create combined texture and sampler descriptor set for the materials in the _scene
\return Return true on success
***********************************************************************************************************************/
bool VulkanIntroducingPVRUtils::createDescriptorSets(pvrvk::CommandBuffer& uploadCmd,
    std::vector<pvr::utils::ImageUploadResults>& uploadResuls)
{
	// create the sampler object
	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.minFilter = samplerInfo.magFilter =  VkFilter::e_LINEAR;
	samplerInfo.mipMapMode = VkSamplerMipmapMode::e_LINEAR;
	samplerInfo.wrapModeU = samplerInfo.wrapModeV = VkSamplerAddressMode::e_REPEAT;
	_deviceResources->samplerTrilinear = _deviceResources->device->createSampler(samplerInfo);

	if (!_deviceResources->samplerTrilinear.isValid())
	{
		("Failed to create Sampler Object");
		return false;
	}
	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;
	uint32_t i = 0;
	for (uint32_t i = 0; i < _scene->getNumMaterials(); ++i)
	{
		if (_scene->getMaterial(i).defaultSemantics().getDiffuseTextureIndex() == -1) { continue; }

		MaterialDescSet matDescSet = std::make_pair(i, _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->texDescSetLayout));
		_deviceResources->texDescSets.push_back(matDescSet);

		writeDescSets.push_back(pvrvk::WriteDescriptorSet());
		pvrvk::WriteDescriptorSet& writeDescSet = writeDescSets.back();
		writeDescSet.set(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, matDescSet.second, 0);
		const pvr::assets::Model::Material& material = _scene->getMaterial(i);

		// Load the diffuse texture map
		const char* fileName = _scene->getTexture(material.defaultSemantics().getDiffuseTextureIndex()).getName().c_str();

		uploadResuls.push_back(pvr::utils::loadAndUploadImage(_deviceResources->device, fileName, true, uploadCmd,
		                       *this));
		pvrvk::ImageView diffuseMap = uploadResuls.back().getImageView();
		if (diffuseMap.isNull())
		{
			setExitMessage("ERROR: Failed to load texture %s", fileName);
			return false;
		}

		writeDescSet.setImageInfo(0, pvrvk::DescriptorImageInfo(diffuseMap, _deviceResources->samplerTrilinear,
		                          VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
	}

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->lightUboDescSets.add(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->uboDescSetLayoutStatic));
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->lightUboDescSets[i], 0)
		                        .setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->lightBuffer,
		                                       0, _deviceResources->lightMemoryView.getDynamicSliceSize())));


		_deviceResources->matrixUboDescSets.add(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->uboDescSetLayoutDynamic));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->matrixUboDescSets[i], 0));
		pvrvk::WriteDescriptorSet& writeDescSet = writeDescSets.back();
		writeDescSet.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->matrixBuffer, 0,
		                           _deviceResources->matrixMemoryView.getDynamicSliceSize()));
	}

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), writeDescSets.size(), nullptr, 0);
	return true;
}

/*!*********************************************************************************************************************
\brief  This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.
\return Return an auto ptr to the demo supplied by the user
***********************************************************************************************************************/
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::unique_ptr<pvr::Shell>(new VulkanIntroducingPVRUtils()); }
