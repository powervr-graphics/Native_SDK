/*!*********************************************************************************************************************
\File         VulkanIntroducingPVRApi.cpp
\Title        Introducing the PowerVR Framework
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Shows how to use the PVRApi library together with loading models from POD files and rendering them with effects from PFX files.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRVk/ApiObjectsVk.h"
#include "PVRUtils/PVRUtilsVk.h"

const VkSampleCountFlags NumSamples = VkSampleCountFlags::e_4_BIT;
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
class VulkanMultiSampling : public pvr::Shell
{
	typedef std::pair<int32_t, pvrvk::DescriptorSet> MaterialDescSet;
	struct DeviceResources
	{
		pvrvk::Instance instance;
		pvrvk::Surface surface;
		pvrvk::Device device;
		pvrvk::Swapchain swapchain;
		pvrvk::CommandPool commandPool;
		pvrvk::DescriptorPool descriptorPool;
		pvrvk::Queue queue;

		pvrvk::Semaphore semaphoreImageAcquired[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Fence perFrameAcquireFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Semaphore semaphorePresent[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Fence perFrameCommandBufferFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

		pvr::Multi<pvrvk::ImageView> depthStencilImages;
		// main command buffer used to store rendering commands
		pvr::Multi<pvrvk::CommandBuffer> commandBuffers;

		// The Vertex buffer object handle array.
		std::vector<pvrvk::Buffer> vbos;
		std::vector<pvrvk::Buffer> ibos;

		// the framebuffer used in the demo
		pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;

		// descriptor sets
		std::vector<MaterialDescSet> texDescSets;
		pvr::Multi<pvrvk::DescriptorSet> matrixUboDescSets;
		pvrvk::DescriptorSet lightUboDescSet;

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
		pvrvk::GraphicsPipeline uiPipeline;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;
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

	bool createMultiSampleFramebufferAndRenderPass();
	void createBuffers();
	bool createDescriptorSets(pvrvk::CommandBuffer& commandBuffer, std::vector<pvr::utils::ImageUploadResults>& imageUploads);
	void recordCommandBuffers();
	void createPipeline();
	void createDescriptorSetLayouts();
};

struct DescripotSetComp
{
	int32_t id;
	DescripotSetComp(int32_t id) : id(id) {}
	bool operator()(const std::pair<int32_t, pvrvk::DescriptorSet>& pair)
	{
		return pair.first == id;
	}
};

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief	Code in initApplication() will be called by Shell once per run, before the rendering context is created.
		Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.). If the rendering
		context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result VulkanMultiSampling::initApplication()
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
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.
				If the rendering context is lost, quitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result VulkanMultiSampling::quitApplication() { return pvr::Result::Success; }

bool VulkanMultiSampling::createMultiSampleFramebufferAndRenderPass()
{
	// Create the Framebuffer with the folowing configurations
	// Attachment 0: MultiSample Color
	// Attachment 1: MultiSample DepthStencil
	// Attachment 2: Swapchain Color (Resolve)
	// Attachment 3: DepthStencil  (Resolve)
	// Subpass 0: Renders in to the Multisample attachments(0,1) and then reolve in to the final image(2,3)

	VkFormat msColorDsFmt[] =
	{
		_deviceResources->swapchain->getImageFormat(), // color
		VkFormat::e_D32_SFLOAT // depth stencil
	};

	// set up the renderpass.
	pvrvk::SubPassDescription subpass;

	// We need two subpass dependency here.
	// First dependency does the image memory barrier before a render pass and its only subpass.
	// It Tranisition the image from memory access(from presentation engine) to color read and write operation.
	//
	// The second dependency is defined for operations occurring inside a subpass and after the render pass.
	// it transition the barrier from color read/ write operation to memory read so the presentation engine can read
	// them.
	pvrvk::SubPassDependency dependencies[2] =
	{
		pvrvk::SubPassDependency(pvrvk::SubpassExternal, 0, VkPipelineStageFlags::e_BOTTOM_OF_PIPE_BIT,
		                         VkPipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, VkAccessFlags::e_MEMORY_READ_BIT,
		                         VkAccessFlags::e_COLOR_ATTACHMENT_READ_BIT | VkAccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT,
		                         VkDependencyFlags::e_BY_REGION_BIT),

		pvrvk::SubPassDependency(0, pvrvk::SubpassExternal,
		                         VkPipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT,
		                         VkPipelineStageFlags::e_BOTTOM_OF_PIPE_BIT,
		                         VkAccessFlags::e_COLOR_ATTACHMENT_READ_BIT | VkAccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT,
		                         VkAccessFlags::e_MEMORY_READ_BIT,
		                         VkDependencyFlags::e_BY_REGION_BIT)
	};

	// multi sample color attachment
	subpass.setColorAttachmentReference(0, pvrvk::AttachmentReference(0, VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	// multi sample depth stencil attachment
	subpass.setDepthStencilAttachmentReference(pvrvk::AttachmentReference(1, VkImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));

	// resolve color attachment == presentation image
	subpass.setResolveAttachmentReference(0, pvrvk::AttachmentReference(2, VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));

	// resolve depth stencil attachment
	subpass.setResolveAttachmentReference(1, pvrvk::AttachmentReference(3, VkImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));

	pvrvk::RenderPassCreateInfo rpInfo;
	// The image will get  resolved in to the final swapchain image, so don't care about the store.
	rpInfo.setAttachmentDescription(0, pvrvk::AttachmentDescription::createColorDescription(msColorDsFmt[0],
	                                VkImageLayout::e_UNDEFINED, VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL,
	                                VkAttachmentLoadOp::e_CLEAR, VkAttachmentStoreOp::e_DONT_CARE, NumSamples));

	rpInfo.setAttachmentDescription(1, pvrvk::AttachmentDescription::createDepthStencilDescription(msColorDsFmt[1],
	                                VkImageLayout::e_UNDEFINED, VkImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	                                VkAttachmentLoadOp::e_CLEAR, VkAttachmentStoreOp::e_DONT_CARE, VkAttachmentLoadOp::e_CLEAR,
	                                VkAttachmentStoreOp::e_DONT_CARE, NumSamples));

	// We dont care about the load op since they will get overriden during resolving.
	rpInfo.setAttachmentDescription(2, pvrvk::AttachmentDescription::createColorDescription(
	                                  msColorDsFmt[0], VkImageLayout::e_UNDEFINED, VkImageLayout::e_PRESENT_SRC_KHR,
	                                  VkAttachmentLoadOp::e_DONT_CARE, VkAttachmentStoreOp::e_STORE));

	// resolving depth-stencil image we render.
	rpInfo.setAttachmentDescription(3, pvrvk::AttachmentDescription::createDepthStencilDescription(msColorDsFmt[1],
	                                VkImageLayout::e_UNDEFINED, VkImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	                                VkAttachmentLoadOp::e_DONT_CARE, VkAttachmentStoreOp::e_DONT_CARE, VkAttachmentLoadOp::e_DONT_CARE,
	                                VkAttachmentStoreOp::e_DONT_CARE));

	rpInfo.setSubPass(0, subpass);
	rpInfo.addSubPassDependencies(dependencies, 2);

	// create the renderpass
	pvrvk::RenderPass renderPass = _deviceResources->device->createRenderPass(rpInfo);
	if (!renderPass.isValid())
	{
		setExitMessage("Failed to create Multisample On screen render pass");
		return false;
	}

	// create the framebuffer
	pvr::Multi<pvrvk::FramebufferCreateInfo> framebufferInfo;
	const pvrvk::Extent3D& dimension = pvrvk::Extent3D(_deviceResources->swapchain->getDimension(), 1u);
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		pvrvk::FramebufferCreateInfo& info = framebufferInfo[i];
		// allocate the musltisample color and depth stencil attachment
		// color attachment. The attachment will transient
		pvrvk::ImageView msColor = _deviceResources->device->createImageView(
		                             pvr::utils::createImage(_deviceResources->device, VkImageType::e_2D, msColorDsFmt[0],
		                                 dimension, VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT | VkImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT,
		                                 VkImageCreateFlags(0), pvrvk::ImageLayersSize(), NumSamples,
		                                 VkMemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT));

		// depth stencil attachment. The attachment will be transient
		pvrvk::ImageView msDs = _deviceResources->device->createImageView(
		                          pvr::utils::createImage(_deviceResources->device, VkImageType::e_2D, msColorDsFmt[1],
		                              dimension, VkImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT |
		                              VkImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT, VkImageCreateFlags(0),
		                              pvrvk::ImageLayersSize(), NumSamples, VkMemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT));

		pvrvk::ImageView ds = _deviceResources->device->createImageView(
		                        pvr::utils::createImage(_deviceResources->device, VkImageType::e_2D, msColorDsFmt[1],
		                            dimension, VkImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT |
		                            VkImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT, VkImageCreateFlags(0),
		                            pvrvk::ImageLayersSize(), VkSampleCountFlags::e_1_BIT, VkMemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT));

		info.setAttachment(0, msColor);
		info.setAttachment(1, msDs);
		info.setAttachment(2, _deviceResources->swapchain->getImageView(i));
		info.setAttachment(3, ds);
		info.setRenderPass(renderPass);
		info.setDimensions(_deviceResources->swapchain->getDimension());
		_deviceResources->onScreenFramebuffer[i] = _deviceResources->device->createFramebuffer(info);

		// validate
		if (_deviceResources->onScreenFramebuffer[i].isNull())
		{
			return false;
		}
	}

	// validate framebuffer creation
	return true;
}

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in initView() will be called by Shell upon initialization or after a change  in the rendering context.
				Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result VulkanMultiSampling::initView()
{
	_frameId = 0;
	// Create vulkan instance
	if (!pvr::utils::createInstanceAndSurface(this->getApplicationName(), this->getWindow(), this->getDisplay(), _deviceResources->instance, _deviceResources->surface))
	{
		return pvr::Result::UnknownError;
	}

	// create the logical device and get the queue
	pvr::utils::QueuePopulateInfo queuePopulateInfo =
	{
		VkQueueFlags::e_GRAPHICS_BIT, _deviceResources->surface
	};
	pvr::utils::QueueAccessInfo queueAccessInfo;

	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0),
	                           &queuePopulateInfo, 1, &queueAccessInfo);
	if (_deviceResources->device.isNull())
	{
		return pvr::Result::UnknownError;
	}
	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(_deviceResources->surface);

	// validate the supported swapchain image usage
	VkImageUsageFlags swapchainImageUsage = VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, VkImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= VkImageUsageFlags::e_TRANSFER_SRC_BIT;
	}

	//--------------------
	// Create the depthstencil image and the swapchain
	if (!pvr::utils::createSwapchainAndDepthStencilImageView(_deviceResources->device, _deviceResources->surface,
	    getDisplayAttributes(), _deviceResources->swapchain, _deviceResources->depthStencilImages, swapchainImageUsage))
	{
		return pvr::Result::UnknownError;
	}

	// create the descriptor pool & commandpool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(queueAccessInfo.familyId,
	                                VkCommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT);
	if (_deviceResources->commandPool.isNull())
	{
		return pvr::Result::UnknownError;
	}

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(
	                                     pvrvk::DescriptorPoolCreateInfo()
	                                     .addDescriptorInfo(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 16)
	                                     .addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 16)
	                                     .addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER, 16)
	                                     .setMaxDescriptorSets(16));

	pvr::utils::appendSingleBuffersFromModel(_deviceResources->device, *_scene, _deviceResources->vbos, _deviceResources->ibos);

	// We check the scene contains at least one light
	if (_scene->getNumLights() == 0)
	{
		("The _scene does not contain a light\n");
		return pvr::Result::UnknownError;
	}

	if (!createMultiSampleFramebufferAndRenderPass())
	{
		setExitMessage("Fauled to create Multisample onscreen framebuffer");
		return pvr::Result::NotInitialized;
	}

	if (!_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
	                                       _deviceResources->commandPool, _deviceResources->queue))
	{
		setExitMessage("Failed top initialize the UIRenderer");
		return pvr::Result::NotInitialized;
	}

	// Create Multisample Pipeline for UIRenderer
	pvrvk::GraphicsPipelineCreateInfo uiPipeInfo = _deviceResources->uiRenderer.getPipeline()->getCreateInfo();
	uiPipeInfo.multiSample.enableAllStates(true).setNumRasterizationSamples(NumSamples);
	uiPipeInfo.basePipeline = _deviceResources->uiRenderer.getPipeline();
	uiPipeInfo.flags = VkPipelineCreateFlags::e_DERIVATIVE_BIT;
	_deviceResources->uiPipeline = _deviceResources->device->createGraphicsPipeline(uiPipeInfo);
	if (_deviceResources->uiPipeline.isNull())
	{
		return pvr::Result::UnknownError;
	}

	_deviceResources->uiRenderer.getDefaultTitle()->setText("VulkanMultiSampling").commitUpdates();
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	// create demo buffers
	createBuffers();

	// create the descriptor set layouts and pipeline layouts
	createDescriptorSetLayouts();

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->semaphorePresent[i] = _deviceResources->device->createSemaphore();
		_deviceResources->semaphoreImageAcquired[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameCommandBufferFence[i] = _deviceResources->device->createFence(VkFenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->perFrameAcquireFence[i] = _deviceResources->device->createFence(VkFenceCreateFlags::e_SIGNALED_BIT);

		_deviceResources->commandBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		if (i == 0)
		{
			_deviceResources->commandBuffers[i]->begin();
		}
	}

	// create the descriptor sets
	std::vector<pvr::utils::ImageUploadResults> imageUploads;
	createDescriptorSets(_deviceResources->commandBuffers[0], imageUploads);
	_deviceResources->commandBuffers[0]->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->commandBuffers[0];
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle();

	// create demo graphics pipeline
	createPipeline();

	// record the rendering commands
	recordCommandBuffers();

	// Calculates the projection matrix
	bool isRotated = this->isScreenRotated() && this->isFullScreen();
	if (isRotated)
	{
		_projMtx = pvr::math::perspective(pvr::Api::Vulkan, _scene->getCamera(0).getFOV(),
		                                  (float)this->getHeight() / (float)this->getWidth(), _scene->getCamera(0).getNear(),
		                                  _scene->getCamera(0).getFar(), glm::pi<float>() * .5f);
	}
	else
	{
		_projMtx = pvr::math::perspective(pvr::Api::Vulkan, _scene->getCamera(0).getFOV(),
		                                  (float)this->getWidth() / (float)this->getHeight(),
		                                  _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar());
	}

	// update the light direction ubo only once.
	glm::vec3 lightDir3;
	_scene->getLightDirection(0, lightDir3);
	lightDir3 = glm::normalize(lightDir3);

	void* memory;
	_deviceResources->lightBuffer->getDeviceMemory()->map(&memory);
	_deviceResources->lightMemoryView.pointToMappedMemory(memory);
	_deviceResources->lightMemoryView.getElementByName("LightPos").setValue(glm::vec4(lightDir3, 1.f));
	_deviceResources->lightBuffer->getDeviceMemory()->unmap();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result VulkanMultiSampling::releaseView()
{
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); i++)
	{
		_deviceResources->perFrameAcquireFence[i]->wait();
		_deviceResources->perFrameAcquireFence[i]->reset();

		_deviceResources->perFrameCommandBufferFence[i]->wait();
		_deviceResources->perFrameCommandBufferFence[i]->reset();
	}

	_deviceResources->device->waitIdle();

	_deviceResources.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every _frame.
***********************************************************************************************************************/
pvr::Result VulkanMultiSampling::renderFrame()
{
	_deviceResources->perFrameAcquireFence[_frameId]->wait();
	_deviceResources->perFrameAcquireFence[_frameId]->reset();
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->semaphoreImageAcquired[_frameId], _deviceResources->perFrameAcquireFence[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameCommandBufferFence[swapchainIndex]->wait();
	_deviceResources->perFrameCommandBufferFence[swapchainIndex]->reset();

	//	Calculates the _frame number to animate in a time-based manner.
	//	get the time in milliseconds.
	_frame += (float)getFrameTime() / 30.f; // design-time target fps for animation

	if (_frame >= _scene->getNumFrames() - 1)	{	_frame = 0;	}

	// Sets the _scene animation to this _frame
	_scene->setCurrentFrame(_frame);

	//	We can build the world view matrix from the camera position, target and an up vector.
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
			_deviceResources->matrixMemoryView.getElementByName("MVP", 0, dynamicSlice).setValue(glm::value_ptr(_projMtx * tempMtx));
			_deviceResources->matrixMemoryView.getElementByName("WorldViewItMtx", 0, dynamicSlice).setValue(glm::inverseTranspose(glm::mat3x3(tempMtx)));
		}
		_deviceResources->matrixBuffer->getDeviceMemory()->unmap();
	}
	// Submit
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->commandBuffers[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->semaphoreImageAcquired[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->semaphorePresent[_frameId];
	submitInfo.numSignalSemaphores = 1;
	VkPipelineStageFlags waitDstStages = VkPipelineStageFlags::e_ALL_GRAPHICS_BIT;
	submitInfo.waitDestStages = &waitDstStages;
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

	//Present
	pvrvk::PresentInfo presentInfo;
	presentInfo.imageIndices = &swapchainIndex;
	presentInfo.numSwapchains = 1;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.waitSemaphores = &_deviceResources->semaphorePresent[_frameId];
	presentInfo.numWaitSemaphores = 1;
	_deviceResources->queue->present(presentInfo);

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief	Pre-record the rendering commands
***********************************************************************************************************************/
void VulkanMultiSampling::recordCommandBuffers()
{
	pvrvk::ClearValue clearValues[] =
	{
		pvrvk::ClearValue(0.00, 0.70, 0.67, 1.0f),
		pvrvk::ClearValue(1.f, 0u)
	};
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		// begin recording commands
		_deviceResources->commandBuffers[i]->begin();

		// begin the renderpass
		_deviceResources->commandBuffers[i]->beginRenderPass(_deviceResources->onScreenFramebuffer[i], pvrvk::Rect2Di(0, 0, getWidth(),
		    getHeight()), true, clearValues, ARRAY_SIZE(clearValues));

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
		uint32_t offset = 0;
		pvrvk::DescriptorSet descriptorSets[3];
		descriptorSets[1] = _deviceResources->matrixUboDescSets[i];
		descriptorSets[2] = _deviceResources->lightUboDescSet;
		for (uint32_t j = 0; j < static_cast<uint32_t>(_scene->getNumMeshNodes()); ++j)
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
			offset = static_cast<uint32_t>(_deviceResources->matrixMemoryView.getDynamicSliceOffset(j + i * _scene->getNumMeshNodes()));

			// bind the descriptor sets
			_deviceResources->commandBuffers[i]->bindDescriptorSets(VkPipelineBindPoint::e_GRAPHICS,
			    _deviceResources->pipelineLayout, 0, descriptorSets, 3, &offset, 1);

			// bind the vbo and ibos for the current mesh node
			_deviceResources->commandBuffers[i]->bindVertexBuffer(_deviceResources->vbos[pNode->getObjectId()], 0, 0);
			_deviceResources->commandBuffers[i]->bindIndexBuffer(_deviceResources->ibos[pNode->getObjectId()], 0,
			    pvr::utils::convertToVk(pMesh->getFaces().getDataType()));

			// draw
			_deviceResources->commandBuffers[i]->drawIndexed(0, pMesh->getNumFaces() * 3, 0, 0, 1);
		}

		// add ui effects using ui renderer
		_deviceResources->uiRenderer.beginRendering(_deviceResources->commandBuffers[i], _deviceResources->uiPipeline);
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.endRendering();
		_deviceResources->commandBuffers[i]->endRenderPass();
		_deviceResources->commandBuffers[i]->end();
	}
}

/*!*********************************************************************************************************************
\brief	Creates the descriptor set layouts used throughout the demo.
***********************************************************************************************************************/
void VulkanMultiSampling::createDescriptorSetLayouts()
{
	// create the texture descriptor set layout and pipeline layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, VkDescriptorType::e_COMBINED_IMAGE_SAMPLER,
		                       1, VkShaderStageFlags::e_FRAGMENT_BIT);
		_deviceResources->texDescSetLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);
	}

	// create the ubo descriptor set layouts
	{
		// dynamic ubo
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1,
		                       VkShaderStageFlags::e_VERTEX_BIT); /*binding 0*/
		_deviceResources->uboDescSetLayoutDynamic = _deviceResources->device->createDescriptorSetLayout(descSetInfo);
	}
	{
		//static ubo
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER, 1,
		                       VkShaderStageFlags::e_VERTEX_BIT);/*binding 0*/
		_deviceResources->uboDescSetLayoutStatic = _deviceResources->device->createDescriptorSetLayout(descSetInfo);
	}

	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
	pipeLayoutInfo.addDescSetLayout(_deviceResources->texDescSetLayout);/* set 0 */
	pipeLayoutInfo.addDescSetLayout(_deviceResources->uboDescSetLayoutDynamic);/* set 1 */
	pipeLayoutInfo.addDescSetLayout(_deviceResources->uboDescSetLayoutStatic);/* set 2 */
	_deviceResources->pipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
}

void VulkanMultiSampling::createPipeline()
{
	pvrvk::GraphicsPipelineCreateInfo pipeDesc;

	pipeDesc.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
	pipeDesc.rasterizer.setCullMode(VkCullModeFlags::e_BACK_BIT);
	pipeDesc.rasterizer.setFrontFaceWinding(VkFrontFace::e_COUNTER_CLOCKWISE);
	pvr::utils::populateInputAssemblyFromMesh(_scene->getMesh(0), Attributes, 3,
	    pipeDesc.vertexInput, pipeDesc.inputAssembler);

	pvr::utils::populateViewportStateCreateInfo(_deviceResources->onScreenFramebuffer[0], pipeDesc.viewport);

	const pvr::Stream::ptr_type vertSource = getAssetStream(VertShaderFileName);
	const pvr::Stream::ptr_type fragSource = getAssetStream(FragShaderFileName);

	pipeDesc.vertexShader.setShader(_deviceResources->device->createShader(vertSource->readToEnd<uint32_t>()));
	pipeDesc.fragmentShader.setShader(_deviceResources->device->createShader(fragSource->readToEnd<uint32_t>()));

	pipeDesc.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
	pipeDesc.depthStencil.enableDepthTest(true);
	pipeDesc.depthStencil.setDepthCompareFunc(VkCompareOp::e_LESS);
	pipeDesc.depthStencil.enableDepthWrite(true);
	pipeDesc.rasterizer.setCullMode(VkCullModeFlags::e_BACK_BIT);
	pipeDesc.subpass = 0;
	pipeDesc.multiSample.enableAllStates(true);
	pipeDesc.multiSample.setNumRasterizationSamples(NumSamples);

	pipeDesc.pipelineLayout = _deviceResources->pipelineLayout;

	_deviceResources->pipeline = _deviceResources->device->createGraphicsPipeline(pipeDesc);
}

/*!*********************************************************************************************************************
\brief	Creates the buffers used throughout the demo.
***********************************************************************************************************************/
void VulkanMultiSampling::createBuffers()
{
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("MVP", pvr::GpuDatatypes::mat4x4);
		desc.addElement("WorldViewItMtx", pvr::GpuDatatypes::mat3x3);

		_deviceResources->matrixMemoryView.initDynamic(desc, _scene->getNumMeshNodes() * _deviceResources->swapchain->getSwapchainLength(),
		    pvr::BufferUsageFlags::UniformBuffer, static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment));
		_deviceResources->matrixBuffer = pvr::utils::createBuffer(_deviceResources->device, _deviceResources->matrixMemoryView.getSize(),
		                                 VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);
	}

	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("LightPos", pvr::GpuDatatypes::vec4);

		_deviceResources->lightMemoryView.init(desc);
		_deviceResources->lightBuffer = pvr::utils::createBuffer(_deviceResources->device, _deviceResources->lightMemoryView.getSize(),
		                                VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);
	}
}


/*!*********************************************************************************************************************
\brief	Create combined texture and sampler descriptor set for the materials in the _scene
\return	Return true on success
***********************************************************************************************************************/
bool VulkanMultiSampling::createDescriptorSets(pvrvk::CommandBuffer& commandBuffer,
    std::vector<pvr::utils::ImageUploadResults>& imageUploads)
{
	// create the sampler object
	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.minFilter = samplerInfo.magFilter = VkFilter::e_LINEAR;
	samplerInfo.mipMapMode = VkSamplerMipmapMode::e_LINEAR;
	samplerInfo.wrapModeU = samplerInfo.wrapModeV = VkSamplerAddressMode::e_REPEAT;
	_deviceResources->samplerTrilinear = _deviceResources->device->createSampler(samplerInfo);

	if (!_deviceResources->samplerTrilinear.isValid())
	{
		("Failed to create Sampler Object");
		return false;
	}

	uint32_t i = 0;
	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;
	while (i < _scene->getNumMaterials() && _scene->getMaterial(i).defaultSemantics().getDiffuseTextureIndex() != -1)
	{
		MaterialDescSet matDescSet = std::make_pair(i, _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->texDescSetLayout));
		pvrvk::ImageView diffuseMap;
		const pvr::assets::Model::Material& material = _scene->getMaterial(i);

		// Load the diffuse texture map
		const char* fileName = _scene->getTexture(material.defaultSemantics().getDiffuseTextureIndex()).getName().c_str();
		imageUploads.push_back(pvr::utils::loadAndUploadImage(_deviceResources->device, fileName, true, commandBuffer, *this));
		diffuseMap = imageUploads.back().getImageView();
		if (diffuseMap.isNull())
		{
			setExitMessage("ERROR: Failed to load texture %s", fileName);
			return false;
		}
		writeDescSets.push_back(
		  pvrvk::WriteDescriptorSet(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, matDescSet.second)
		  .setImageInfo(0, pvrvk::DescriptorImageInfo(diffuseMap, _deviceResources->samplerTrilinear,
		                VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
		_deviceResources->texDescSets.push_back(matDescSet);
		++i;
	}

	_deviceResources->lightUboDescSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->uboDescSetLayoutStatic);
	writeDescSets.push_back(
	  pvrvk::WriteDescriptorSet(VkDescriptorType::e_UNIFORM_BUFFER, _deviceResources->lightUboDescSet)
	  .setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->lightBuffer,
	                 0, _deviceResources->lightMemoryView.getSize())));

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->matrixUboDescSets.add(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->uboDescSetLayoutDynamic));
		writeDescSets.push_back(
		  pvrvk::WriteDescriptorSet(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->matrixUboDescSets[i])
		  .setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->matrixBuffer, 0, _deviceResources->matrixMemoryView.getDynamicSliceSize())));
	}
	_deviceResources->device->updateDescriptorSets(writeDescSets.data(),
	    writeDescSets.size(), nullptr, 0);
	return true;
}

/*!*********************************************************************************************************************
\brief	This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.
\return Return an auto ptr to the demo supplied by the user
***********************************************************************************************************************/
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::unique_ptr<pvr::Shell>(new VulkanMultiSampling()); }
