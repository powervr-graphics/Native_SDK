/*!*********************************************************************************************************************
\File         VulkanSkinning.cpp
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief      Shows how to perform skinning combined with Dot3 (normal-mapped) lighting
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRPfx/RenderManagerVk.h"
#include "PVRCore/pfx/PFXParser.h"

namespace Configuration {
const char EffectFile[] = "Skinning.pfx";

// POD scene files
const char SceneFile[] = "Robot.pod";
} // namespace Configuration

// Put all API managed objects in a struct so that we can one-line free them...
struct DeviceResources
{
	pvrvk::Instance instance;
	pvrvk::DebugReportCallback debugCallbacks[2];
	pvrvk::Device device;

	// Rendering manager, putting together Effects with Models to render things
	pvr::utils::RenderManager mgr;
	// Asset loader
	pvr::Multi<pvrvk::CommandBuffer> commandBuffers;
	pvrvk::Swapchain swapchain;
	pvrvk::CommandPool commandPool;
	pvrvk::DescriptorPool descriptorPool;
	pvrvk::Queue queue;

	pvr::utils::vma::Allocator vmaAllocator;

	pvrvk::Surface surface;

	pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;
	pvr::Multi<pvrvk::ImageView> depthStencilImages;
	pvrvk::Semaphore semaphoreImageAcquired[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Fence perFrameAcquireFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Semaphore semaphorePresent[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Fence perFrameCommandBufferFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	// UIRenderer used to display text
	pvr::ui::UIRenderer uiRenderer;
	~DeviceResources()
	{
		if (device.isValid())
		{
			device->waitIdle();
			uint32_t l = swapchain->getSwapchainLength();
			for (uint32_t i = 0; i < l; ++i)
			{
				if (perFrameAcquireFence[i].isValid())
					perFrameAcquireFence[i]->wait();
				if (perFrameCommandBufferFence[i].isValid())
					perFrameCommandBufferFence[i]->wait();
			}
		}
	}
};

/*!*********************************************************************************************************************
Class implementing the Shell functions.
***********************************************************************************************************************/
class VulkanSkinning : public pvr::Shell
{
	std::unique_ptr<DeviceResources> _deviceResources;

	uint32_t _frameId;
	// 3D Model
	pvr::assets::ModelHandle _scene;

	bool _isPaused;

	// Variables to handle the animation in a time-based manner
	float _currentFrame;

public:
	VulkanSkinning() : _isPaused(false), _currentFrame(0) {}

	pvr::Result initApplication();
	pvr::Result initView();
	pvr::Result releaseView();
	pvr::Result quitApplication();
	pvr::Result renderFrame();

	void recordCommandBuffer();
	void eventMappedInput(pvr::SimplifiedInput action);
};

/*!*********************************************************************************************************************
\brief  handle the input event
\param  action input actions to handle
***********************************************************************************************************************/
void VulkanSkinning::eventMappedInput(pvr::SimplifiedInput action)
{
	switch (action)
	{
	case pvr::SimplifiedInput::Action1:
	case pvr::SimplifiedInput::Action2:
	case pvr::SimplifiedInput::Action3:
		_isPaused = !_isPaused;
		break;
	case pvr::SimplifiedInput::ActionClose:
		exitShell();
		break;
	default:
		break;
	}
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in initApplication() will be called by Shell once per run, before the rendering context is created.
Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result VulkanSkinning::initApplication()
{
	this->setStencilBitsPerPixel(0);
	pvr::assets::PODReader podReader(getAssetStream(Configuration::SceneFile));
	if ((_scene = pvr::assets::Model::createWithReader(podReader)).isNull())
	{
		setExitMessage("Error: Could not create the _scene file %s.", Configuration::SceneFile);
		return pvr::Result::InitializationError;
	}
	_frameId = 0;
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in quitApplication() will be called by Shell once per run, just before exiting the program.
If the rendering context is lost, quitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result VulkanSkinning::quitApplication()
{
	_scene.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result VulkanSkinning::initView()
{
	_deviceResources = std::unique_ptr<DeviceResources>(new DeviceResources());

	// Create instance and retrieve compatible physical devices
	_deviceResources->instance = pvr::utils::createInstance(this->getApplicationName());

	if (_deviceResources->instance->getNumPhysicalDevices() == 0)
	{
		setExitMessage("Unable not find a compatible Vulkan physical device.");
		return pvr::Result::UnknownError;
	}

	// Create the surface
	_deviceResources->surface = pvr::utils::createSurface(_deviceResources->instance, _deviceResources->instance->getPhysicalDevice(0), this->getWindow(), this->getDisplay());

	// Add Debug Report Callbacks
	// Add a Debug Report Callback for logging messages for events of all supported types.
	_deviceResources->debugCallbacks[0] = pvr::utils::createDebugReportCallback(_deviceResources->instance);
	// Add a second Debug Report Callback for throwing exceptions for Error events.
	_deviceResources->debugCallbacks[1] =
		pvr::utils::createDebugReportCallback(_deviceResources->instance, pvrvk::DebugReportFlagsEXT::e_ERROR_BIT_EXT, pvr::utils::throwOnErrorDebugReportCallback);

	// look for graphics queue with presentation support for the given surface
	const pvr::utils::QueuePopulateInfo queueCreateInfo = {
		pvrvk::QueueFlags::e_GRAPHICS_BIT | pvrvk::QueueFlags::e_COMPUTE_BIT,
		_deviceResources->surface,
	};

	pvr::utils::QueueAccessInfo queueAccessInfo;

	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), &queueCreateInfo, 1, &queueAccessInfo);

	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	if (!_deviceResources->device.isValid())
	{
		setExitMessage("failed to create Logical device");
		return pvr::Result::UnknownError;
	}

	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(_deviceResources->surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT;
	}

	// create the swapchain
	pvr::utils::createSwapchainAndDepthStencilImageAndViews(_deviceResources->device, _deviceResources->surface, getDisplayAttributes(), _deviceResources->swapchain,
		_deviceResources->depthStencilImages, swapchainImageUsage, pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT,
		&_deviceResources->vmaAllocator);

	_currentFrame = 0.;

	// Setup the effect
	pvr::pfx::PfxParser rd(Configuration::EffectFile, this);

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 16)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 128)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, 128)
																						  .setMaxDescriptorSets(256));

	_deviceResources->commandPool = _deviceResources->device->createCommandPool(
		pvrvk::CommandPoolCreateInfo(_deviceResources->queue->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	// create the commandbuffers, semaphores & the fence
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->commandBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->semaphorePresent[i] = _deviceResources->device->createSemaphore();
		_deviceResources->semaphoreImageAcquired[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameCommandBufferFence[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->perFrameAcquireFence[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
	}

	_deviceResources->mgr.init(*this, _deviceResources->swapchain, _deviceResources->descriptorPool);
	_deviceResources->commandBuffers[0]->begin();
	_deviceResources->mgr.addEffect(*rd.getAssetHandle(), _deviceResources->commandBuffers[0]);
	_deviceResources->mgr.addModelForAllPasses(_scene);
	_deviceResources->mgr.buildRenderObjects(_deviceResources->commandBuffers[0]);
	_scene->releaseVertexData();
	_deviceResources->mgr.createAutomaticSemantics();

	// set the initial layout of the framebuffer from undefined to Present
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		pvrvk::Framebuffer framebuffer = _deviceResources->mgr.toPass(0, 0).getFramebuffer(i);
		if (framebuffer->getAttachment(0).isValid())
		{
			pvr::utils::setImageLayout(
				framebuffer->getAttachment(0)->getImage(), pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_PRESENT_SRC_KHR, _deviceResources->commandBuffers[0]);
		}
		if (framebuffer->getAttachment(1).isValid())
		{
			pvr::utils::setImageLayout(framebuffer->getAttachment(1)->getImage(), pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				_deviceResources->commandBuffers[0]);
		}
	}

	/***************************************************************
	**** Under the hood, the above line will do the following: *****

	for (auto& pipe : devObj->mgr.toSubpass(0, 0, 0).pipelines) // actually, for all subpasses, but we only have 1
	{
	  pipe.createAutomaticModelSemantics();
	}

	for (auto& node : devObj->mgr.renderables())
	{
	  node.createAutomaticSemantics();
	}
	***************************************************************/

	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->mgr.toPass(0, 0).getFramebuffer(0)->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queue);

	_deviceResources->commandBuffers[0]->end();
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->commandBuffers[0];
	submitInfo.numCommandBuffers = 1;
	_deviceResources->perFrameAcquireFence[0]->reset();
	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameAcquireFence[0]);
	_deviceResources->perFrameAcquireFence[0]->wait();

	_deviceResources->uiRenderer.getDefaultTitle()->setText("Skinning");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultDescription()->setText("Skinning with Normal Mapped Per Pixel Lighting");

	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText("Any Action Key : Pause");
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();
	_deviceResources->uiRenderer.getSdkLogo()->setColor(1.0f, 1.0f, 1.0f, 1.f);
	_deviceResources->uiRenderer.getSdkLogo()->commitUpdates();
	recordCommandBuffer();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result VulkanSkinning::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result VulkanSkinning::renderFrame()
{
	_deviceResources->perFrameAcquireFence[_frameId]->wait();
	_deviceResources->perFrameAcquireFence[_frameId]->reset();
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->semaphoreImageAcquired[_frameId], _deviceResources->perFrameAcquireFence[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameCommandBufferFence[swapchainIndex]->wait();
	_deviceResources->perFrameCommandBufferFence[swapchainIndex]->reset();

	const float fDelta = static_cast<float>(getFrameTime());
	auto& animation = _scene->getAnimationInstance(0);

	if (fDelta > 0.0001f)
	{
		if (!_isPaused)
		{
			if (_currentFrame > animation.getTotalTimeInMs())
			{
				_currentFrame = 0;
			}
			else
			{
				_currentFrame += getFrameTime();
			}
		}
	}
	_scene->getAnimationInstance(0).updateAnimation(_currentFrame);

	// Set the _scene animation to the current frame
	_deviceResources->mgr.updateAutomaticSemantics(swapchainIndex);

	/***************************************************************
	**** Under the hood, the above line will do the following: *****

	//Get a new worldview camera and light position
	auto& pipeline = devObj->mgr.toPipeline(0, 0, 0, 0);
	pipeline.updateAutomaticModelSemantics(getSwapChainIndex());

	// Update all node-specific matrices (Worldview, bone array etc).
	uint32_t swapChainIndex = getGraphicsContext()->getPlatformContext().getSwapChainIndex();

	// Should be called before updating anything to optimise map/unmap. Suggest call once per frame.
	devObj->mgr.toEffect(0).beginBufferUpdates(swapChainIndex);

	for (auto& rendernode : devObj->mgr.renderables()) { rendernode.updateAutomaticSemantics(swapChainIndex); }

	// Must be called if beginBufferUpdates were previously called (performs the buffer->unmap calls)
	devObj->mgr.toEffect(0).endBufferUpdates(swapChainIndex);

	***************************************************************/

	// Update all the bones matrices
	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags pipeWaitStageFlags = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.commandBuffers = &_deviceResources->commandBuffers[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->semaphoreImageAcquired[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->semaphorePresent[_frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.waitDestStages = &pipeWaitStageFlags;
	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameCommandBufferFence[swapchainIndex]);

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->swapchain, swapchainIndex, _deviceResources->commandPool, _deviceResources->queue, this->getScreenshotFileName(),
			&_deviceResources->vmaAllocator, &_deviceResources->vmaAllocator);
	}

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

inline std::vector<pvr::StringHash> generateBonesList(const char* base, uint32_t numBones)
{
	std::vector<pvr::StringHash> boneSemantics;
	char buffer[255];
	assertion(strlen(base) < 240);
	strcpy(buffer, base);
	char* ptr = buffer + strlen(base);
	int32_t decades = numBones / 10;
	int32_t units = numBones % 10;
	for (int32_t decade = 0; decade < decades; ++decade)
	{
		for (int32_t unit = 0; unit < 10; ++unit)
		{
			*ptr = '0' + unit;
			ptr[1] = '\0';
			boneSemantics.push_back(pvr::StringHash(buffer));
		}
		if (decade == 0)
		{
			ptr++;
		}
		*(ptr - 1) = '0' + decade + 1;
	}

	// 0, 1, 2, 3, 4, 5, 6, 7
	for (int32_t unit = 0; unit < units; ++unit)
	{
		*ptr = '0' + unit;
		ptr[1] = '\0';
		boneSemantics.push_back(pvr::StringHash(buffer));
	}

	return boneSemantics;
}

/*!*********************************************************************************************************************
\brief  pre-record the rendering commands
***********************************************************************************************************************/
inline void VulkanSkinning::recordCommandBuffer()
{
	const pvrvk::ClearValue clearValues[2] = {
		pvrvk::ClearValue(0.0f, 0.45f, 0.41f, 1.0f),
		pvrvk::ClearValue(1.0f, 0u),
	};
	for (uint32_t swapidx = 0; swapidx < _deviceResources->swapchain->getSwapchainLength(); ++swapidx)
	{
		_deviceResources->commandBuffers[swapidx]->begin();
		// Clear the color and depth buffer automatically.

		pvr::utils::setImageLayout(_deviceResources->mgr.toPass(0, 0).getFramebuffer(swapidx)->getAttachment(0)->getImage(), pvrvk::ImageLayout::e_PRESENT_SRC_KHR,
			pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, _deviceResources->commandBuffers[swapidx]);

		_deviceResources->commandBuffers[swapidx]->beginRenderPass(_deviceResources->mgr.toPass(0, 0).getFramebuffer(swapidx), true, clearValues, ARRAY_SIZE(clearValues));
		_deviceResources->mgr.toPass(0, 0).recordRenderingCommands(_deviceResources->commandBuffers[swapidx], swapidx, false);

		//// PART 3 :  UIRenderer
		_deviceResources->uiRenderer.beginRendering(_deviceResources->commandBuffers[swapidx]);
		_deviceResources->uiRenderer.getDefaultDescription()->render();
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.getDefaultControls()->render();
		_deviceResources->uiRenderer.endRendering();

		///// PART 4 : End the RenderePass
		_deviceResources->commandBuffers[swapidx]->endRenderPass();
		// prepare image for presentation
		pvr::utils::setImageLayout(_deviceResources->mgr.toPass(0, 0).getFramebuffer(swapidx)->getAttachment(0)->getImage(), pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL,
			pvrvk::ImageLayout::e_PRESENT_SRC_KHR, _deviceResources->commandBuffers[swapidx]);
		_deviceResources->commandBuffers[swapidx]->end();
	}
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo()
{
	return std::unique_ptr<pvr::Shell>(new VulkanSkinning());
}
