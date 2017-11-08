/*!*********************************************************************************************************************
\File         GaussianBlur.cpp
\Title        Gaussian Blur
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright(c) Imagination Technologies Limited.
\brief        Shows how to perform a single pass Gaussian Blur using Compute shader.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRVk/ApiObjectsVk.h"
#include "PVRUtils/PVRUtilsVk.h"

// Source and binary shaders
const char FragShaderSrcFile[] = "FragShader_vk.fsh.spv";
const char VertShaderSrcFile[] = "VertShader_vk.vsh.spv";
const char CompShaderSrcFile[] = "CompShader_vk.csh.spv";

// PVR texture files
const char StatueTexFile[] = "Lenna.pvr";

/*!*********************************************************************************************************************
Class implementing the Shell functions.
***********************************************************************************************************************/
class VulkanGaussianBlur : public pvr::Shell
{
	struct DeviceResources
	{
		pvrvk::Instance instance;
		pvrvk::Surface surface;
		pvrvk::Device device;
		pvrvk::Queue queue;
		pvrvk::Swapchain swapchain;

		pvrvk::DescriptorPool descriptorPool;
		pvrvk::CommandPool commandPool;

		pvr::utils::StructuredBufferView structuredBufferView;
		pvrvk::Buffer buffer;

		pvrvk::Semaphore semaphoreImageAcquired[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Fence perFrameAcquireFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Semaphore semaphorePresent[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Fence perFrameCommandBufferFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

		pvr::Multi<pvrvk::Framebuffer> framebuffer;
		pvr::Multi<pvrvk::ImageView> depthStencilImages;
		pvr::Multi<pvrvk::CommandBuffer> renderCmdBuffers;
		pvr::Multi<pvrvk::SecondaryCommandBuffer> uiRendererCommandBuffers;
		pvr::Multi<pvrvk::SecondaryCommandBuffer> graphicsCommandBuffers;
		pvr::Multi<pvrvk::SecondaryCommandBuffer> computeCommandBuffers;
		pvr::Multi<pvrvk::ImageView> ImageViewOutputs;
		pvr::Multi<pvrvk::DescriptorSet> descriptorSet;

		pvrvk::ImageView textureInputView;

		pvrvk::GraphicsPipeline graphicPipeline;
		pvrvk::ComputePipeline computePipeline;
		pvrvk::PipelineLayout pipelinelayout;

		pvrvk::MemoryBarrierSet barrierSet;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;
	};

private:
	std::unique_ptr<DeviceResources> _deviceResources;
	uint32_t _numSwapchain;
	uint32_t _frameId;

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	bool loadTextures(pvrvk::CommandBuffer& commandBuffer,
	                  std::vector<pvr::utils::ImageUploadResults>& outUploadResults);
	bool createPipelines();
	void recordCommandBuffer();
};

/*!*********************************************************************************************************************
\return return true if no error occurred
\brief  Loads the textures required for this training course
***********************************************************************************************************************/
bool VulkanGaussianBlur::loadTextures(pvrvk::CommandBuffer& commandBuffer,
                                      std::vector<pvr::utils::ImageUploadResults>& outUploadResults)
{
	pvr::Texture texture;

	//Load the Texture PVR file from the disk
	auto texStream = getAssetStream(StatueTexFile);
	if (pvr::assets::textureLoad(texStream, pvr::TextureFileFormat::PVR, texture))
	{
		pvr::TextureArea texturearea;
		pvr::ImageDataFormat imageformat;

		imageformat.colorSpace = texture.getColorSpace();
		imageformat.format = texture.getPixelFormat();

		texturearea.width = texture.getWidth();
		texturearea.height = texture.getHeight();
		texturearea.arraySize = 1;
		texturearea.setCompressedSize(texture.getDataSize(0, true, true));

		//Create and Allocate Textures.
		pvr::utils::ImageUploadResults result =  pvr::utils::uploadImage(
		      _deviceResources->device, texture, true, commandBuffer,  VkImageUsageFlags::e_SAMPLED_BIT |
		      VkImageUsageFlags::e_STORAGE_BIT);
		if (result.getImageView().isNull())
		{
			return false;
		}
		outUploadResults.push_back(result);
		_deviceResources->textureInputView = result.getImageView();

		// Trasnform the image layout from undefined to general
		pvr::utils::setImageLayout(_deviceResources->textureInputView->getImage(), VkImageLayout::e_UNDEFINED,
		                           VkImageLayout::e_GENERAL, commandBuffer);

		//Create 1 texture per frame.
		for (uint32_t i = 0; i < _numSwapchain; i++)
		{
			pvrvk::Image outputTextureStore = pvr::utils::createImage(_deviceResources->device, VkImageType::e_2D,
			                                  pvr::utils::convertToVkPixelFormat(texture.getPixelFormat(), texture.getColorSpace(),
			                                      texture.getChannelType()), pvrvk::Extent3D(texture.getWidth(), texture.getHeight(), 1u),
			                                  VkImageUsageFlags::e_STORAGE_BIT | VkImageUsageFlags::e_TRANSFER_DST_BIT |
			                                  VkImageUsageFlags::e_SAMPLED_BIT | VkImageUsageFlags::e_TRANSFER_SRC_BIT);

			// trasnfer the layout from UNDEFINED to GENERAL
			pvr::utils::setImageLayout(outputTextureStore, VkImageLayout::e_UNDEFINED,
			                           VkImageLayout::e_GENERAL, commandBuffer);
			_deviceResources->ImageViewOutputs[i] = _deviceResources->device->createImageView(outputTextureStore);
		}
	}
	else
	{
		setExitMessage("ERROR: Failed to load texture.");
		return false;
	}
	return true;
}

/*!*********************************************************************************************************************
\return  Return true if no error occurred
\brief  Loads and compiles the shaders, create the pipelines and descriptor sets.
***********************************************************************************************************************/
bool VulkanGaussianBlur:: createPipelines()
{
	// Load the shaders from their files
	pvrvk::Shader compShader = _deviceResources->device->createShader(getAssetStream(CompShaderSrcFile)->readToEnd<uint32_t>());
	pvrvk::Shader vert = _deviceResources->device->createShader(getAssetStream(VertShaderSrcFile)->readToEnd<uint32_t>());
	pvrvk::Shader frag = _deviceResources->device->createShader(getAssetStream(FragShaderSrcFile)->readToEnd<uint32_t>());
	pvrvk::DescriptorSetLayout descriptorSetLayout;

	{
		//Create the descriptor set layouts
		pvrvk::DescriptorSetLayoutCreateInfo descriptorSetLayoutParams;
		descriptorSetLayoutParams.setBinding(0, VkDescriptorType::e_STORAGE_IMAGE, 1, VkShaderStageFlags::e_COMPUTE_BIT);
		descriptorSetLayoutParams.setBinding(1, VkDescriptorType::e_STORAGE_IMAGE, 1, VkShaderStageFlags::e_COMPUTE_BIT);
		descriptorSetLayoutParams.setBinding(2, VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, VkShaderStageFlags::e_FRAGMENT_BIT);
		descriptorSetLayoutParams.setBinding(3, VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, VkShaderStageFlags::e_FRAGMENT_BIT);
		descriptorSetLayoutParams.setBinding(4, VkDescriptorType::e_UNIFORM_BUFFER, 1, VkShaderStageFlags::e_FRAGMENT_BIT);

		descriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(descriptorSetLayoutParams);
	}

	//Create the pipeline layout
	{
		pvrvk::PipelineLayoutCreateInfo compPipelineLayoutParams;
		compPipelineLayoutParams.addDescSetLayout(descriptorSetLayout);
		_deviceResources->pipelinelayout = _deviceResources->device->createPipelineLayout(compPipelineLayoutParams);
	}

	//Create the compute pipeline
	{
		pvrvk::ComputePipelineCreateInfo compPipelineParams;
		compPipelineParams.computeShader.setShader(compShader);

		compPipelineParams.pipelineLayout = _deviceResources->pipelinelayout;
		_deviceResources->computePipeline = _deviceResources->device->createComputePipeline(compPipelineParams);
	}

	//Create the graphics pipeline
	{
		pvrvk::GraphicsPipelineCreateInfo graphicPipeParameters;

		const pvrvk::Rect2Di rect(0, 0, _deviceResources->swapchain->getDimension().width, _deviceResources->swapchain->getDimension().height);
		graphicPipeParameters.viewport.setViewportAndScissor(0, pvrvk::Viewport(rect), rect);

		pvrvk::PipelineColorBlendAttachmentState colorAttachemtState;
		colorAttachemtState.blendEnable = false;
		graphicPipeParameters.vertexShader.setShader(vert);
		graphicPipeParameters.fragmentShader.setShader(frag);

		// enable back face culling
		graphicPipeParameters.rasterizer.setCullMode(VkCullModeFlags::e_FRONT_BIT);

		// set counter clockwise winding order for front faces
		graphicPipeParameters.rasterizer.setFrontFaceWinding(VkFrontFace::e_COUNTER_CLOCKWISE);

		// setup vertex inputs
		graphicPipeParameters.vertexInput.clear();
		graphicPipeParameters.inputAssembler.setPrimitiveTopology(VkPrimitiveTopology::e_TRIANGLE_STRIP);

		graphicPipeParameters.colorBlend.setAttachmentState(0, colorAttachemtState);
		graphicPipeParameters.pipelineLayout = _deviceResources->pipelinelayout;
		graphicPipeParameters.renderPass = _deviceResources->framebuffer[0]->getRenderPass();
		graphicPipeParameters.subpass = 0;
		_deviceResources->graphicPipeline = _deviceResources->device->createGraphicsPipeline(graphicPipeParameters);
	}

	//Create the sampler
	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.magFilter = VkFilter::e_LINEAR;
	samplerInfo.minFilter = VkFilter::e_LINEAR;
	samplerInfo.mipMapMode = VkSamplerMipmapMode::e_NEAREST;
	pvrvk::Sampler samplerMipBilinear = _deviceResources->device->createSampler(samplerInfo);

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(
	                                     pvrvk::DescriptorPoolCreateInfo().configureBasic().addDescriptorInfo(VkDescriptorType::e_STORAGE_IMAGE, 16));

	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement("WindowWidth", pvr::GpuDatatypes::Float);

	_deviceResources->structuredBufferView.init(desc);
	_deviceResources->buffer = pvr::utils::createBuffer(_deviceResources->device,_deviceResources->structuredBufferView.getSize(),
	                           VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);

	// update buffer with the window width
	void* memory;
	_deviceResources->buffer->getDeviceMemory()->map(&memory);
	const float windowWidth = getWidth() * 1.2f;
	_deviceResources->structuredBufferView.pointToMappedMemory(memory);
	_deviceResources->structuredBufferView.getElementByName("WindowWidth").setValue(&windowWidth);
	_deviceResources->buffer->getDeviceMemory()->unmap();

	//Populate the descriptor set
	pvrvk::WriteDescriptorSet writeDescSets[pvrvk::FrameworkCaps::MaxSwapChains * 5];
	for (uint32_t i = 0; i < _numSwapchain; ++i)
	{

		_deviceResources->descriptorSet[i] = _deviceResources->descriptorPool->allocateDescriptorSet(descriptorSetLayout);
		if (!_deviceResources->descriptorSet[i].isValid())
		{
			setExitMessage("ERROR: Failed to Create Descriptor set.");
			return false;
		}

		writeDescSets[i * 5].set(VkDescriptorType::e_STORAGE_IMAGE, _deviceResources->descriptorSet[i], 0)
		.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->textureInputView, VkImageLayout::e_GENERAL));

		writeDescSets[i * 5 + 1].set(VkDescriptorType::e_STORAGE_IMAGE, _deviceResources->descriptorSet[i], 1)
		.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->ImageViewOutputs[i], VkImageLayout::e_GENERAL));

		writeDescSets[i * 5 + 2].set(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->descriptorSet[i], 2)
		.setImageInfo(0,  pvrvk::DescriptorImageInfo(_deviceResources->ImageViewOutputs[i], samplerMipBilinear, VkImageLayout::e_GENERAL));

		writeDescSets[i * 5 + 3].set(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->descriptorSet[i], 3)
		.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->textureInputView, samplerMipBilinear, VkImageLayout::e_GENERAL));

		writeDescSets[i * 5 + 4].set(VkDescriptorType::e_UNIFORM_BUFFER, _deviceResources->descriptorSet[i], 4)
		.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->buffer, 0, _deviceResources->structuredBufferView.getSize()));
	}
	_deviceResources->device->updateDescriptorSets(writeDescSets, _numSwapchain * 5, nullptr, 0);
	return (_deviceResources->graphicPipeline.isValid() && _deviceResources->computePipeline.isValid());
}

/*!*********************************************************************************************************************
\brief  Pre record the commands
***********************************************************************************************************************/
void VulkanGaussianBlur::recordCommandBuffer()
{
	const pvrvk::ClearValue clearValue[] =
	{
		pvrvk::ClearValue(123.0f / 255.0f, 172.0f / 255.0, 189.0f / 255.0, 1.0f),
		pvrvk::ClearValue::createDefaultDepthStencilClearValue(),
	};
	for (uint32_t i = 0; i < _numSwapchain; ++i)
	{
		_deviceResources->uiRendererCommandBuffers[i]->begin(_deviceResources->framebuffer[i], 0, VkCommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

		_deviceResources->uiRenderer.beginRendering(_deviceResources->uiRendererCommandBuffers[i]);

		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getDefaultControls()->render();
		_deviceResources->uiRenderer.getDefaultDescription()->render();
		_deviceResources->uiRenderer.endRendering();
		_deviceResources->uiRendererCommandBuffers[i]->end();

		_deviceResources->computeCommandBuffers[i]->begin(VkCommandBufferUsageFlags(0));

		//Bind the compute pipeline & the descriptor set.
		_deviceResources->computeCommandBuffers[i]->bindPipeline(_deviceResources->computePipeline);
		_deviceResources->computeCommandBuffers[i]->bindDescriptorSet(VkPipelineBindPoint::e_COMPUTE, _deviceResources->pipelinelayout, 0, _deviceResources->descriptorSet[i]);

		// dispatch x = image.height / 32
		// dispatch y = 1
		// dispatch z = 1
		_deviceResources->computeCommandBuffers[i]->dispatch(getHeight() / 32, 1, 1);

		//Set up a barrier to pass the image from our computeshader to fragment shader.
		_deviceResources->barrierSet.addBarrier(pvrvk::ImageMemoryBarrier(VkAccessFlags::e_SHADER_WRITE_BIT, VkAccessFlags::e_SHADER_READ_BIT,
		                                        _deviceResources->ImageViewOutputs[i]->getImage(), pvrvk::ImageSubresourceRange(VkImageAspectFlags::e_COLOR_BIT),
		                                        VkImageLayout::e_GENERAL, VkImageLayout::e_GENERAL,
		                                        _deviceResources->queue->getQueueFamilyId(), _deviceResources->queue->getQueueFamilyId()));
		_deviceResources->computeCommandBuffers[i]->pipelineBarrier(VkPipelineStageFlags::e_COMPUTE_SHADER_BIT, VkPipelineStageFlags::e_FRAGMENT_SHADER_BIT, _deviceResources->barrierSet);

		_deviceResources->computeCommandBuffers[i]->end();

		// Create a command buffer for each frame buffer in swap chain
		pvrvk::CommandBuffer& cb = _deviceResources->renderCmdBuffers[i];

		// Begin recording to the command buffer
		cb->begin();

		cb->executeCommands(_deviceResources->computeCommandBuffers[i]);

		//begin the render patch.
		cb->beginRenderPass(_deviceResources->framebuffer[i], pvrvk::Rect2Di(0, 0, getWidth(), getHeight()), false, clearValue, ARRAY_SIZE(clearValue));

		_deviceResources->graphicsCommandBuffers[i]->begin(_deviceResources->framebuffer[i], 0, VkCommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
		//bind the Graphic pipeline and descriptro set.
		_deviceResources->graphicsCommandBuffers[i]->bindPipeline(_deviceResources->graphicPipeline);
		_deviceResources->graphicsCommandBuffers[i]->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS, _deviceResources->pipelinelayout, 0, _deviceResources->descriptorSet[i]);
		// Draw our Quad.
		_deviceResources->graphicsCommandBuffers[i]->draw(0, 3);
		_deviceResources->graphicsCommandBuffers[i]->end();

		cb->executeCommands(_deviceResources->graphicsCommandBuffers[i]);

		// enqueue the command buffer containing ui renderer commands
		cb->executeCommands(_deviceResources->uiRendererCommandBuffers[i]);

		//End Renderpass and recording.
		cb->endRenderPass();
		cb->end();
	}
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in initApplication() will be called by Shell once per run, before the rendering context is created.
Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result VulkanGaussianBlur::initApplication()
{
	//Create a new Device.
	this->_deviceResources.reset(new DeviceResources());

	_frameId = 0;

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result VulkanGaussianBlur::initView()
{
	if (!pvr::utils::createInstanceAndSurface(this->getApplicationName(), this->getWindow(), this->getDisplay(), _deviceResources->instance, _deviceResources->surface))
	{
		return pvr::Result::UnknownError;
	}
	const pvr::utils::QueuePopulateInfo queueInfo = { VkQueueFlags::e_GRAPHICS_BIT, _deviceResources->surface};
	pvr::utils::QueueAccessInfo queueAccessInfo;
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0),
	                           &queueInfo, 1, &queueAccessInfo);

	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(_deviceResources->surface);

	// validate the supported swapchain image usage
	VkImageUsageFlags swapchainImageUsage = VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, VkImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= VkImageUsageFlags::e_TRANSFER_SRC_BIT;
	}

	// Create the swapchain
	if (!pvr::utils::createSwapchainAndDepthStencilImageView(_deviceResources->device, _deviceResources->surface,  getDisplayAttributes(),
	    _deviceResources->swapchain, _deviceResources->depthStencilImages, swapchainImageUsage))
	{
		return pvr::Result::UnknownError;
	}

	//create the framebuffers.
	if (!pvr::utils::createOnscreenFramebufferAndRenderpass(_deviceResources->swapchain, &_deviceResources->depthStencilImages[0],
	    _deviceResources->framebuffer))
	{
		return pvr::Result::UnknownError;
	}

	_deviceResources->commandPool = _deviceResources->device->createCommandPool(_deviceResources->queue->getQueueFamilyId(),
	                                VkCommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT);

	_numSwapchain = _deviceResources->swapchain->getSwapchainLength();
	// Create per frame resource
	for (uint32_t i = 0; i < _numSwapchain; ++i)
	{
		_deviceResources->renderCmdBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->uiRendererCommandBuffers[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->graphicsCommandBuffers[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->computeCommandBuffers[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();

		_deviceResources->semaphorePresent[i] = _deviceResources->device->createSemaphore();
		_deviceResources->semaphoreImageAcquired[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameCommandBufferFence[i] = _deviceResources->device->createFence(VkFenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->perFrameAcquireFence[i] = _deviceResources->device->createFence(VkFenceCreateFlags::e_SIGNALED_BIT);
	}

	_deviceResources->renderCmdBuffers[0]->begin();
	std::vector<pvr::utils::ImageUploadResults> uploadResult;
	if (!loadTextures(_deviceResources->renderCmdBuffers[0], uploadResult))
	{
		return pvr::Result::UnknownError;
	}
	_deviceResources->renderCmdBuffers[0]->end();
	// submit the image upload commmands
	pvrvk::SubmitInfo submit;
	submit.commandBuffers = &_deviceResources->renderCmdBuffers[0];
	submit.numCommandBuffers = 1;
	_deviceResources->queue->submit(&submit, 1);
	_deviceResources->queue->waitIdle();
	_deviceResources->renderCmdBuffers[0]->reset(VkCommandBufferResetFlags(0));
	uploadResult.clear();

	if (!createPipelines()) { return pvr::Result::UnknownError; }

	if (!_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->framebuffer[0]->getRenderPass(), 0, _deviceResources->commandPool,
	                                       _deviceResources->queue))
	{
		setExitMessage("Error: Failed to initialize the UIRenderer\n");
		return pvr::Result::NotInitialized;
	}

	_deviceResources->uiRenderer.getDefaultTitle()->setText("GaussianBlur");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	_deviceResources->uiRenderer.getDefaultDescription()->setText("Left hand side samples from the original texture.\nRight hand side samples from the Gaussian Blurred texture.");
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();

	this->recordCommandBuffer();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  Code in releaseView() will be called by PVRShell when theapplication quits or before a change in the rendering context.
\return Return Result::Success if no error occurred
***********************************************************************************************************************/
pvr::Result VulkanGaussianBlur::releaseView()
{
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); i++)
	{
		_deviceResources->perFrameAcquireFence[i]->wait();
		_deviceResources->perFrameAcquireFence[i]->reset();

		_deviceResources->perFrameCommandBufferFence[i]->wait();
		_deviceResources->perFrameCommandBufferFence[i]->reset();
	}

	//clean up our resources.
	_deviceResources->device->waitIdle();
	_deviceResources.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
If the rendering context is lost, quitApplication() will not be called.x
***********************************************************************************************************************/
pvr::Result VulkanGaussianBlur::quitApplication()
{
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result VulkanGaussianBlur::renderFrame()
{
	//submit the recorded command buffer.
	_deviceResources->perFrameAcquireFence[_frameId]->wait();
	_deviceResources->perFrameAcquireFence[_frameId]->reset();
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->semaphoreImageAcquired[_frameId], _deviceResources->perFrameAcquireFence[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameCommandBufferFence[swapchainIndex]->wait();
	_deviceResources->perFrameCommandBufferFence[swapchainIndex]->reset();

	// SUBMIT
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->renderCmdBuffers[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->semaphoreImageAcquired[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->semaphorePresent[_frameId];
	submitInfo.numSignalSemaphores = 1;
	// wait just before the renderpass clear op.
	VkPipelineStageFlags waitStage = VkPipelineStageFlags::e_TRANSFER_BIT;
	submitInfo.waitDestStages = &waitStage;
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

	//PRESENT
	pvrvk::PresentInfo presentInfo;
	presentInfo.imageIndices = &swapchainIndex;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numWaitSemaphores = 1;
	presentInfo.waitSemaphores = &_deviceResources->semaphorePresent[_frameId];
	presentInfo.numSwapchains = 1;
	_deviceResources->queue->present(presentInfo);

	_frameId = (_frameId + 1) % _numSwapchain;
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return an auto ptr to the demo supplied by the user
\brief  This function must be implemented by the user of the shell. The user should return its
Shell object defining the behavior of the application.
***********************************************************************************************************************/
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::unique_ptr<pvr::Shell>(new VulkanGaussianBlur()); }
