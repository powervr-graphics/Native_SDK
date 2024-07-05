/*!
\brief Shows how to make use of the Vulkan subgroup extensions to perform optimized compute tasks and fine grained control over the execution of the pipeline.
The application uses a compute shader to ray-march a MandleBulb, which is the 3D equivalent of the MandleBrot set.
\file VulkanSubgroups.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRCore/PVRCore.h"
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"

#include "vulkan/vulkan.h"
#include "vk_bindings_helper.h"

// There are a series of compute pipelines using the subgroup functionality. Each feature has a flag for if it is enabled or not
// There is then one extra compute pipeline for the basic compute only fall back
// The flags combine to the following list of functionalities:
// 0 : Subgroup Basic Only
// 1 : Subgroup Basic & Vote
// 2 : Subgroup Basic & Ballot
// 3 : Subgroup Basic & Vote & Ballot
// 4 : Subgroup Basic & Arithmetic
// 5 : Subgroup Basic & Vote & Arithmetic
// 6 : Subgroup Basic & Ballot & Arithmetic
// 7 : Subgroup Basic & Vote & Ballot & Arithmetic
// 8 : Compute Only Fallback
namespace SubgroupFunctionalityFlags {
const uint8_t SubgroupBasic = 0;
const uint8_t SubgroupVote = 1;
const uint8_t SubgroupBallot = 1 << 1;
const uint8_t SubgroupArithmetic = 1 << 2;
const uint8_t Count = 1 << 3;
} // namespace SubgroupFunctionalityFlags

// A list of file paths to the SPIR-V shaders used in this demo
namespace ShaderFilePaths {
const std::string VertexShader = "VertShader.vsh.spv";
const std::string FragmentShader = "FragShader.fsh.spv";
const std::string ComputeShader = "CompShader.csh.spv";
} // namespace ShaderFilePaths

// Settings that can either be explicitly controlled by command line arguments, or change as a result of the other settings
namespace DemoSettings {
// Parts the user can change - without input these settings will be tried to optimized based on physical device features:
// How many pixels wide the off screen texture is
uint32_t computeTextureWidth;

// How many pixels tall the off screen texture is
uint32_t computeTextureHeight;

// How large each of the workgroups are in the X direction
uint32_t workGroupWidth;

// How large each of the workgroups are in the Y direction
uint32_t workGroupHeight;

// Calculated from the above variables, can't be directly changed by user :
// How many compute workgroups are dispatched in the X direction
uint32_t dispatchWidth;

// How many compute workgroups are dispatched in the Y direction
uint32_t dispatchHeight;

// One extra compute pipeline for the basic compute only fallback with no subgroup features
const uint8_t ComputePipelineCount = SubgroupFunctionalityFlags::Count + 1;

// File name of the texture used as the font
const char* fontFilePath = "Inconsolata.pvr";
} // namespace DemoSettings

// A struct that contains all of the Vulkan resources for easier releasing
struct DeviceResources
{
	// Vulkan instance
	pvrvk::Instance instance;

	// Debug messenger to report validation warnings
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;

	// Vulkan logical device, used to interface with the specific GPU
	pvrvk::Device device;

	// The Vulkan swapchain handle which presents rendering results to the surface
	pvrvk::Swapchain swapchain;

	// Two Vulkan Queues so that work can be submitted in parallel if multi queue is supported
	pvrvk::Queue queues[2];

	// Reference to Vulkan memory allocator, to group our device allocations efficiently
	pvr::utils::vma::Allocator vmaAllocator;

	// Command pool which command buffers can be allocated from
	pvrvk::CommandPool commandPool;

	// Descriptor pool which descriptor sets can be allocated from
	pvrvk::DescriptorPool descriptorPool;

	// Per swapchain image sync object for when an image is ready to be rendered to
	std::vector<pvrvk::Semaphore> imageAcquiredSemaphores;

	// Per swapchain image sync object for when an image is finished rendering and ready to be presented
	std::vector<pvrvk::Semaphore> presentationSemaphores;

	// Per swapchain image sync object between CPU and GPU
	std::vector<pvrvk::Fence> perFrameResourcesFences;

	// The primary command buffers that contains all the commands submitted to the GPU
	std::vector<pvrvk::CommandBuffer> primaryCmdBuffers;

	// The secondary command buffers that contain the commands for rendering the UI
	std::vector<pvrvk::SecondaryCommandBuffer> uiSecondaryCmdBuffers;

	// The secondary command buffers that contain the commands for drawing to the backbuffer
	std::vector<pvrvk::SecondaryCommandBuffer> graphicsSecondaryCmdBuffers;

	// The secondary command buffers that contain the commands for dispatching the compute tasks
	std::vector<pvrvk::SecondaryCommandBuffer> computeSecondaryCmdBuffers;

	// Graphics pipeline which is responsible for copying the compute texture to the backbuffer
	pvrvk::GraphicsPipeline graphicsPipeline;

	// Graphics pipeline layout telling the pipeline the layout of required descriptor sets
	pvrvk::PipelineLayout graphicsPipelineLayout;

	// Vector of compute pipelines which are responsible running their associated compute shaders
	std::vector<pvrvk::ComputePipeline> computePiplines;

	// Compute Pipeline layout shared between all compute pipelines
	pvrvk::PipelineLayout computePipelineLayout;

	// Ui renderer handle provided by the SDK for efficient font rendering
	pvr::ui::UIRenderer uiRenderer;

	// Image view for the custom font used by this demo for a mono spaced font
	pvrvk::ImageView fontImageView;

	// Image view of the output from the compute pipeline, which allows the pipelines to access the image
	// one for each image in the swapchain
	std::vector<pvrvk::ImageView> computeOutputImageViews;

	// Image of the output from the compute pipeline, one for each image in the swapchain
	std::vector<pvrvk::Image> computeOutputImages;

	// Descriptor set layout for the output of the compute shader, describes elements in the descriptor sets
	pvrvk::DescriptorSetLayout computeOutputImageDescSetLayout;

	// Per swapchain image descriptor set for the compute image output so we can have multiple in flight
	std::vector<pvrvk::DescriptorSet> computeOutputImageDescSets;

	// Structured buffer view provided by the SDK which allows easier write access from the CPU
	// for each of the slices of the buffer
	pvr::utils::StructuredBufferView matrixBufferView;

	// Underlying buffer handle which holds the matrix transform data of the camera
	// It is one large buffer containing a multiple slices for each swapchain image
	pvrvk::Buffer matrixBuffer;

	// Descriptor set layout telling the pipelines to expect one matrix
	pvrvk::DescriptorSetLayout matrixDescSetLayout;

	// Since the buffer is dynamic, we only need one descriptor, and we can change the index per frame
	// instead of having a unique descriptor set per swapchain image
	pvrvk::DescriptorSet matrixDescSet;

	// Descriptor set layout for copying the compute output to screen
	pvrvk::DescriptorSetLayout graphicsDescSetLayout;

	// A list of descriptors for copying the compute image, one for each image in the swapchain
	std::vector<pvrvk::DescriptorSet> graphicsDescSet;

	// A list of onscreen framebuffers produced by SDK initialization, one for each image in the swapchain
	std::vector<pvrvk::Framebuffer> onScreenFramebuffer;

	~DeviceResources()
	{
		if (device)
		{
			device->waitIdle();
			uint32_t l = swapchain->getSwapchainLength();
			for (uint32_t i = 0; i < l; ++i)
			{
				if (perFrameResourcesFences[i]) perFrameResourcesFences[i]->wait();
			}
		}
	}
};

/// <summary> A class which implements the methods of pvr::Shell </summary>
class VulkanSubgroups : public pvr::Shell
{
	// Private member variables

	// All of the Vulkan resources bundled together
	std::unique_ptr<DeviceResources> _deviceResources;

	// Store the length of the swapchain since it is frequently referenced
	uint32_t _swapLength = 0;

	// Index into image ready sync objects to identify which swapchain image is being waited on
	uint32_t _frameId = 0;

	// Which queue we are using in device resources to submit the command buffers
	uint32_t _queueIndex = 0;

	// Boolean to define if we're allowing for multi queue support
	bool _useMultiQueue = false;

	// Which compute pipeline we're using
	uint8_t _computePipelineIndex = 0;

	// Track the view matrix so it can be rotated each frame
	glm::vec3 _cameraPos = glm::vec3(0.0, 0.0, -1.5);

	// Ui : Should we show the advanced controls
	bool _showSubgroupControls = false;

	// Ui : Which pipeline feature are we currently placing the cursor on
	uint8_t _subgroupControlSelected = 0;

	// Platform agnostic command line argument parser
	pvr::CommandLine _cmdLine{};

public:
	// Public functions that must be overridden to use pvr::Shell
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result renderFrame();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();

	// Handle user input events launched by pvr::Shell
	void eventMappedInput(pvr::SimplifiedInput key);

	// Parse in the user command line args to calculate properties like the workgroup sizes etc
	bool calculateDemoSetting();

	// Helper functions that are used to show details to the user
	std::string getUIRendererControlsText(bool showSubgroupSelection, uint8_t controlSelected, uint8_t pipelineIndex);
	std::string getPipelineNameText(uint8_t pipelineIndex);

	// Create the textures outputted from the compute pipeline
	void createComputeOutputTextures();

	// Create the descriptor sets and layouts
	void createDescriptorSetsAndLayouts();
	void createMatrixDescSets();
	void createcomputeOutputImageDescSets();
	void createGraphicsDescSet();

	// Creating the pipelines
	void createPipelines();
	void createGraphicsPipeline();
	void createComputePipeline();

	// Record the command buffers
	void recordPrimaryCommandBuffers();
	void recordSecondaryCommandBuffers();
	void recordComputeCommandBuffer(uint32_t i);
	void recordGraphicsCommandBuffer(uint32_t i);
	void recordUICommandBuffer(uint32_t i);
};

// Define the functions that must be implemented by users of pvr::Shell to drive the application

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it(e.g.external modules, loading meshes, etc.).If the rendering
/// context is lost, initApplication() will not be called again.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanSubgroups::initApplication()
{
	_cmdLine = this->getCommandLine();
	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change  in the rendering context. Used to initialize variables that are dependent on the
/// rendering context(e.g.textures, vertex buffers, etc.).</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanSubgroups::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();

	// Create instance and retrieve compatible physical devices while targeting Vulkan 1.1. This is because subgroup functionality was added to core in 1.1
	pvr::utils::VulkanVersion vulkanVersion(1, 1);
	_deviceResources->instance = pvr::utils::createInstance(this->getApplicationName(), vulkanVersion, pvr::utils::InstanceExtensions(vulkanVersion));

	if (_deviceResources->instance->getNumPhysicalDevices() == 0)
	{
		setExitMessage("Unable not find a compatible Vulkan physical device.");
		return pvr::Result::UnknownError;
	}

	// Create the surface
	pvrvk::Surface surface =
		pvr::utils::createSurface(_deviceResources->instance, _deviceResources->instance->getPhysicalDevice(0), this->getWindow(), this->getDisplay(), this->getConnection());

	// Create a default set of debug utils messengers or debug callbacks using either VK_EXT_debug_utils or VK_EXT_debug_report respectively
	_deviceResources->debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(_deviceResources->instance);

	// Retrieve the device and queues capable of supporting the graphics and compute queues
	pvr::utils::QueuePopulateInfo queueCreateInfos[] = {
		{ pvrvk::QueueFlags::e_GRAPHICS_BIT | pvrvk::QueueFlags::e_COMPUTE_BIT, surface }, // Queue 0
		{ pvrvk::QueueFlags::e_GRAPHICS_BIT | pvrvk::QueueFlags::e_COMPUTE_BIT, surface } // Queue 1
	};
	pvr::utils::QueueAccessInfo queueAccessInfos[2];
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), queueCreateInfos, 2, queueAccessInfos);

	_deviceResources->queues[0] = _deviceResources->device->getQueue(queueAccessInfos[0].familyId, queueAccessInfos[0].queueId);
	_deviceResources->queues[0]->setObjectName("Queue0");

	// In the future we may want to improve our flexibility with regards to making use of multiple queues but for now to support multi queue the queue must support
	// Graphics + Compute + WSI support.
	// Other multi queue approaches may be possible i.e. making use of additional queues which do not support graphics/WSI
	_useMultiQueue = false;

	if (queueAccessInfos[1].familyId != static_cast<uint32_t>(-1) && queueAccessInfos[1].queueId != static_cast<uint32_t>(-1))
	{
		_deviceResources->queues[1] = _deviceResources->device->getQueue(queueAccessInfos[1].familyId, queueAccessInfos[1].queueId);
		_deviceResources->queues[1]->setObjectName("Queue1");

		if (_deviceResources->queues[0]->getFamilyIndex() == _deviceResources->queues[1]->getFamilyIndex())
		{
			_useMultiQueue = true;
			Log(LogLevel::Information, "Multiple queues support e_GRAPHICS_BIT + e_COMPUTE_BIT + WSI. These queues will be used to ping-pong work each frame");
		}
		else
		{
			Log(LogLevel::Information, "Queues are from a different Family. We cannot ping-pong work each frame");
		}
	}
	else
	{
		Log(LogLevel::Information, "Only a single queue supports e_GRAPHICS_BIT + e_COMPUTE_BIT + WSI. We cannot ping-pong work each frame");
	}

	// Create the memory allocated
	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	// Validate that the swapchain supports screenshots and the colour formats we require.
	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(surface);
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT;
	}

	// Create the swapchain
	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage).enableDepthBuffer(false));

	_deviceResources->swapchain = swapChainCreateOutput.swapchain;
	_deviceResources->onScreenFramebuffer = swapChainCreateOutput.framebuffer;
	_swapLength = _deviceResources->swapchain->getSwapchainLength();

	_deviceResources->imageAcquiredSemaphores.resize(_swapLength);
	_deviceResources->presentationSemaphores.resize(_swapLength);
	_deviceResources->perFrameResourcesFences.resize(_swapLength);
	_deviceResources->primaryCmdBuffers.resize(_swapLength);
	_deviceResources->uiSecondaryCmdBuffers.resize(_swapLength);
	_deviceResources->graphicsSecondaryCmdBuffers.resize(_swapLength);
	_deviceResources->computeSecondaryCmdBuffers.resize(_swapLength);
	_deviceResources->computeOutputImageViews.resize(_swapLength);
	_deviceResources->computeOutputImages.resize(_swapLength);
	_deviceResources->computeOutputImageDescSets.resize(_swapLength);
	_deviceResources->graphicsDescSet.resize(_swapLength);

	// Create the resource pools
	_deviceResources->commandPool =
		_deviceResources->device->createCommandPool(pvrvk::CommandPoolCreateInfo(_deviceResources->queues[0]->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_NONE));

	_deviceResources->descriptorPool =
		_deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo(10).addDescriptorInfo(pvrvk::DescriptorType::e_STORAGE_IMAGE, static_cast<uint16_t>(8 * _swapLength)));

	_deviceResources->descriptorPool->setObjectName("DescriptorPool");

	// Create the per frame resources
	for (uint32_t i = 0; i < _swapLength; i++)
	{
		// Sync objects
		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->presentationSemaphores[i]->setObjectName("PresentationSemaphoreSwapchain" + std::to_string(i));
		_deviceResources->imageAcquiredSemaphores[i]->setObjectName("ImageAcquiredSemaphoreSwapchain" + std::to_string(i));

		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->perFrameResourcesFences[i]->setObjectName("FenceSwapchain" + std::to_string(i));

		// Command Buffers
		_deviceResources->primaryCmdBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->computeSecondaryCmdBuffers[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->graphicsSecondaryCmdBuffers[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->uiSecondaryCmdBuffers[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();

		_deviceResources->primaryCmdBuffers[i]->setObjectName("MainCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->computeSecondaryCmdBuffers[i]->setObjectName("ComputeSecondaryCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->graphicsSecondaryCmdBuffers[i]->setObjectName("GraphicsSecondaryCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->uiSecondaryCmdBuffers[i]->setObjectName("UISecondaryCommandBufferSwapchain" + std::to_string(i));
	}

	// Before creating any resources specific to this demo, fill the demo settings namespace
	if (!calculateDemoSetting())
	{
		return pvr::Result::UnknownError;
	}

	// Upload the font texture
	// We need to allocate an extra command buffer for a one time submit since we're not allowing command buffers to be reset
	pvrvk::CommandBuffer cmd = _deviceResources->commandPool->allocateCommandBuffer();
	cmd->begin(pvrvk::CommandBufferUsageFlags::e_ONE_TIME_SUBMIT_BIT);

	pvr::Texture fontTexture;
	_deviceResources->fontImageView = pvr::utils::loadAndUploadImageAndView(_deviceResources->device, DemoSettings::fontFilePath, true, cmd, *this,
		pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, &fontTexture, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);

	// submit the one time command buffer
	cmd->end();
	pvrvk::SubmitInfo submit;
	submit.commandBuffers = &cmd;
	submit.numCommandBuffers = 1;
	_deviceResources->queues[0]->submit(submit);

	// Create a sampler and font object for the texture atlas
	pvrvk::Sampler sampler = _deviceResources->device->createSampler(pvrvk::SamplerCreateInfo(pvrvk::Filter::e_LINEAR, pvrvk::Filter::e_LINEAR));

	// Initialize the UI renderer while setting the uploaded font to the default
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queues[0], _deviceResources->fontImageView, fontTexture, sampler);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("VulkanSubgroups");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();

	_deviceResources->uiRenderer.getDefaultControls()->setText(getUIRendererControlsText(false, _subgroupControlSelected, _computePipelineIndex));
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	_deviceResources->uiRenderer.getDefaultDescription()->setText("Using " + getPipelineNameText(_computePipelineIndex) + " Pipeline");
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();

	// Create the resources used for outputting information
	createComputeOutputTextures();
	createDescriptorSetsAndLayouts();

	// Create the pipelines
	createPipelines();

	// Record all the command buffers.
	recordSecondaryCommandBuffers();
	recordPrimaryCommandBuffers();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanSubgroups::renderFrame()
{
	// Wait for the image to be ready for rendering to
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();
	_deviceResources->perFrameResourcesFences[swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[swapchainIndex]->reset();

	// Define the stages the semaphore will wait for
	// pvrvk::PipelineStageFlags pipeWaitStageFlags = pvrvk::PipelineStageFlags::e_ALL_GRAPHICS_BIT | pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT;
	pvrvk::PipelineStageFlags pipeWaitStageFlags = pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT;

	// Rotate the camera position
	float theta = 0.0001f * static_cast<float>(this->getFrameTime());
	_cameraPos = (glm::vec4(_cameraPos, 1.0) * glm::rotate(theta, glm::vec3(0, 1, 0)));

	// Calculate the new view matrix
	glm::mat4 viewMatrix = glm::lookAt(_cameraPos, glm::vec3(0, 0, 0), glm::vec3(0.0, 1.0, 0.0));

	// Write value to correct slice of the matrix buffer
	_deviceResources->matrixBufferView.getElementByName("mInvViewMatrix", 0, swapchainIndex).setValue(glm::inverse(viewMatrix));

	// Create a submit info
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->primaryCmdBuffers[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->imageAcquiredSemaphores[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.waitDstStageMask = &pipeWaitStageFlags;

	// Submit
	_deviceResources->queues[_queueIndex]->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[swapchainIndex]);

	// Does shell want to take a screenshot?
	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->queues[_queueIndex], _deviceResources->commandPool, _deviceResources->swapchain, swapchainIndex, this->getScreenshotFileName(),
			_deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	// Present
	pvrvk::PresentInfo presentInfo;
	presentInfo.imageIndices = &swapchainIndex;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numWaitSemaphores = 1;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	presentInfo.numSwapchains = 1;
	// As above we must present using the same VkQueue as submitted to previously
	_deviceResources->queues[_queueIndex]->present(presentInfo);

	// Advance the frame and queue indexes as appropriate
	_frameId = (_frameId + 1) % _swapLength;
	if (_useMultiQueue) { _queueIndex = (_queueIndex + 1) % 2; }

	return pvr::Result::Success;
}

/// <summary>Function called by pvr:shell whenever the graphics context changes and needs to be released</summary>
/// <returns>Result::Success if no errors occurred</returns>
pvr::Result VulkanSubgroups::releaseView() { return pvr::Result::Success; }

/// <summary>Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.</summary>
/// <returns>Result::Success if no error occurred</returns>.
pvr::Result VulkanSubgroups::quitApplication() { return pvr::Result::Success; }

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behavior of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanSubgroups>(); }

/// <summary>Handles the user input once per frame </summary>
/// <param name="key">The input which has been abstracted and simplified</param>
void VulkanSubgroups::eventMappedInput(pvr::SimplifiedInput key)
{
	// Store the current pipeline index
	uint32_t prevPipelineIndex = _computePipelineIndex;

	// Process each of the keys
	switch (key)
	{
	case pvr::SimplifiedInput::ActionClose: exitShell(); break;
	case pvr::SimplifiedInput::Left:

		// Switching the selection off
		if (_showSubgroupControls)
		{
			// If the selection is 0 then we're moving to the compute only fallback
			if (_subgroupControlSelected == 0)
				_computePipelineIndex = SubgroupFunctionalityFlags::Count;
			else
				_computePipelineIndex &= ~_subgroupControlSelected;
		}
		break;
	case pvr::SimplifiedInput::Right:
		// Switching the selection on
		if (_showSubgroupControls)
		{
			// We are turning on a subgroup feature, so turn off the computeOnlyFlag
			_computePipelineIndex &= ~SubgroupFunctionalityFlags::Count;
			_computePipelineIndex |= _subgroupControlSelected;
		}
		break;
	case pvr::SimplifiedInput::Up:

		// If the user has pressed up then move the selector cursor up one
		if (_showSubgroupControls)
		{
			if (_subgroupControlSelected == 0)
				_subgroupControlSelected = SubgroupFunctionalityFlags::SubgroupArithmetic;
			else
				_subgroupControlSelected = (_subgroupControlSelected >> 1);
		}
		break;
	case pvr::SimplifiedInput::Down:

		// If the user has pressed down then move the selector cursor down one
		if (_showSubgroupControls)
		{
			if (_subgroupControlSelected == 0)
				_subgroupControlSelected++;
			else
				_subgroupControlSelected = (_subgroupControlSelected << 1) % SubgroupFunctionalityFlags::Count;
		}
		break;
	case pvr::SimplifiedInput::Action1:
		// Change the controls to show the more detailed version
		_showSubgroupControls = !_showSubgroupControls;
		break;
	default: break;
	}

	// Has the pipeline index changed? if so we need to update the UI and rerecord the command buffers
	if (_computePipelineIndex != prevPipelineIndex)
	{
		// Reset the command pool to erase all of the command buffers in one go
		_deviceResources->device->waitIdle();
		_deviceResources->commandPool->reset(pvrvk::CommandPoolResetFlags::e_RELEASE_RESOURCES_BIT);

		// Rerecord the command buffers
		recordSecondaryCommandBuffers();
		recordPrimaryCommandBuffers();

		// Tell the user which pipeline is being used
		Log(LogLevel::Information, "Using compute pipeline %s", getPipelineNameText(_computePipelineIndex).c_str());
	}

	// Always update the UI
	_deviceResources->uiRenderer.getDefaultControls()->setText(getUIRendererControlsText(_showSubgroupControls, _subgroupControlSelected, _computePipelineIndex));
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	_deviceResources->uiRenderer.getDefaultDescription()->setText("Using " + getPipelineNameText(_computePipelineIndex) + " Pipeline");
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
}

// Enter into the functions which are specific to this demo.

/// <summary>Attempts to parse and sanitize the command line arguments received, after that properties related to the execution of the demo will be calculated
/// if the user doesn't set a specific argument, then a best guess is calculated from physical device properties</summary>
bool VulkanSubgroups::calculateDemoSetting()
{
	// Set the off screen texture size
	{
		// Attempt to parse the users choice for width and height of the off screen texture
		bool computeTextureSizeSet = false;
		if (_cmdLine.hasOption("-texWidth") && _cmdLine.hasOption("-texHeight"))
		{
			// Assume the user has parsed correct info until proven otherwise
			computeTextureSizeSet = true;

			int32_t width = 0, height = 0;
			_cmdLine.getIntOption("-texWidth", width);
			_cmdLine.getIntOption("-texHeight", height);

			// If either of these were less than 0 then the user has entered in incorrect data
			if (width <= 0 || height <= 0)
			{
				Log(LogLevel::Warning, "Width and height values must both be larger than 0. You parsed (%i, %i)", width, height);
				computeTextureSizeSet = false;
			}
			else
			{
				DemoSettings::computeTextureWidth = static_cast<uint32_t>(width);
				DemoSettings::computeTextureHeight = static_cast<uint32_t>(height);
			}
		}
		else if (_cmdLine.hasOption("-texWidth") || _cmdLine.hasOption("-texHeight"))
		{
			Log(LogLevel::Warning, "You must pass both a width and height");
		}

		// If the user didn't successfully parse the width and height for an off screen texture size
		// use the onscreen framebuffer multiplied by a scale
		if (!computeTextureSizeSet)
		{
			// Did the user set a scale?
			bool scaleSet = false;
			float scaleFactor = 0.0f;
			if (_cmdLine.hasOption("-scale"))
			{
				_cmdLine.getFloatOption("-scale", scaleFactor);
				if (scaleFactor <= 0.0f) { Log(LogLevel::Warning, "Off screen texture scale must be larger than 0"); }
				else
				{
					// Scale command line argument matches requirements
					scaleSet = true;
				}
			}

			// Default the scale factor to 0.5
			if (!scaleSet) scaleFactor = 0.5f;

			DemoSettings::computeTextureWidth = static_cast<uint32_t>(this->getWidth() * scaleFactor);
			DemoSettings::computeTextureHeight = static_cast<uint32_t>(this->getHeight() * scaleFactor);
		}
	}

	// Calculate the workgroup width and height
	{
		// We need the physical device in order to check against the physical device limits
		pvrvk::PhysicalDevice device = _deviceResources->device->getPhysicalDevice();

		// Get the subgroup properties and device limits
		VkPhysicalDeviceSubgroupProperties subgroupProperties{};
		device->populateExtensionPropertiesVk(pvrvk::StructureType::e_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES, &subgroupProperties);

		if (subgroupProperties.subgroupSize == 0)
		{
			setExitMessage("subgroupSize must be at least 1.");
			return false;
		}

		pvrvk::PhysicalDeviceLimits limits = device->getProperties().getLimits();

		// Log the device limits supported by the hardware.
		Log(LogLevel::Information, "Compute Device limits : \n * Subgroup size: %i\n * Workgroup invocation count: %i", subgroupProperties.subgroupSize,
			limits.getMaxComputeWorkGroupInvocations());

		bool workgroupSizeSet = false;
		if (_cmdLine.hasOption("-wgWidth") && _cmdLine.hasOption("-wgHeight"))
		{
			// Assume the user has passed correct information until proven otherwise
			workgroupSizeSet = true;

			// Retrieve the values set by the user
			uint32_t width = 0, height = 0;
			_cmdLine.getUintOption("-wgWidth", width);
			_cmdLine.getUintOption("-wgHeight", height);

			// Are the values too large?
			if (width > limits.getMaxComputeWorkGroupSize()[0] || height > limits.getMaxComputeWorkGroupSize()[1])
			{
				Log(LogLevel::Warning, "Workgroup dimensions are too large, (%i, %i) must be smaller than the max (%i, %i)", width, height, limits.getMaxComputeWorkGroupSize()[0],
					limits.getMaxComputeWorkGroupSize()[1]);
				workgroupSizeSet = false;
			}
			else if (width * height > limits.getMaxComputeWorkGroupInvocations())
			{
				Log(LogLevel::Warning, "Workgroup size is too large (%i x %i) = %i must be less than %i", width, height, width * height, limits.getMaxComputeWorkGroupInvocations());
				workgroupSizeSet = false;
			}
			// Values just right, pass them on
			else
			{
				DemoSettings::workGroupWidth = static_cast<uint32_t>(width);
				DemoSettings::workGroupHeight = static_cast<uint32_t>(height);
			}
		}
		else if (_cmdLine.hasOption("-wgWidth") || _cmdLine.hasOption("-wgHeight"))
		{
			Log(LogLevel::Warning, "You must pass both -wgWidth and -wgHeight to set the workgroup sizes");
		}

		// If the user hasn't set the workgroup size correctly, then we attempt to construct the optimal dimensions
		if (!workgroupSizeSet)
		{
			// Start by making the workgroup size as large as possible, while retaining a square shape
			uint32_t width = static_cast<uint32_t>(sqrt(limits.getMaxComputeWorkGroupInvocations()));
			uint32_t height = limits.getMaxComputeWorkGroupInvocations() / width;

			// We know that the workgroup size is within the max invocation count, now just check its within width and height bounds
			if (width > limits.getMaxComputeWorkGroupSize()[0]) width = limits.getMaxComputeWorkGroupSize()[0];
			if (height > limits.getMaxComputeWorkGroupSize()[1]) height = limits.getMaxComputeWorkGroupSize()[1];

			// In order to get full utilization of the hardware, we need to ensure that the workgroup size is divisible by the subgroup size
			// First ensure that the workgroup size is divisible by subgroup count, if now then we need to make one of the dimensions divisble
			// by the subgroup count, this will ensure that the product is divisible by the subgroup count
			if ((width > subgroupProperties.subgroupSize) && (width * height) % subgroupProperties.subgroupSize != 0)
			{
				// Knock the remainder off the width
				width -= width % subgroupProperties.subgroupSize;
			}

			// Pass these onto the demo settings
			DemoSettings::workGroupWidth = width;
			DemoSettings::workGroupHeight = height;
		}
	}

	// Calculate the number of workgroups that need to be dispatched, which will be the off screen texture size divided by workgroup size
	// Round up to ensure that the entire texture is filled, even if the workgroup size doesn't nicely devise the texture size
	DemoSettings::dispatchWidth = (DemoSettings::computeTextureWidth + DemoSettings::workGroupWidth - 1) / DemoSettings::workGroupWidth;
	DemoSettings::dispatchHeight = (DemoSettings::computeTextureHeight + DemoSettings::workGroupHeight - 1) / DemoSettings::workGroupHeight;

	// Log Demo settings
	Log(LogLevel::Information,
		"Demo settings are as following -"
		"\n * Texture Width : %i\n * Texture Height : %i"
		"\n * Workgroup Width : %i\n * Workgroup Height : %i"
		"\n * Dispatch Width : %i\n * Dispatch Height : %i",
		DemoSettings::computeTextureWidth, DemoSettings::computeTextureHeight, DemoSettings::workGroupWidth, DemoSettings::workGroupHeight, DemoSettings::dispatchWidth,
		DemoSettings::dispatchHeight);

	// Get which features the user wants to enable on start up
	{
		if (_cmdLine.hasOption("-Subgroup_Basic") || _cmdLine.hasOption("-Subgroup_Vote") || _cmdLine.hasOption("-Subgroup_Ballot") || _cmdLine.hasOption("-Subgroup_Arithmetic"))
		{
			// Get each option the user wants and set the corresponding flags
			if (_cmdLine.hasOption("-Subgroup_Basic")) _computePipelineIndex += SubgroupFunctionalityFlags::SubgroupBasic;
			if (_cmdLine.hasOption("-Subgroup_Vote")) _computePipelineIndex += SubgroupFunctionalityFlags::SubgroupVote;
			if (_cmdLine.hasOption("-Subgroup_Ballot")) _computePipelineIndex += SubgroupFunctionalityFlags::SubgroupBallot;
			if (_cmdLine.hasOption("-Subgroup_Arithmetic")) _computePipelineIndex += SubgroupFunctionalityFlags::SubgroupArithmetic;
		}
		else
		{
			// User hasn't specified any features, so set the pipeline flag to the compute only fallback
			_computePipelineIndex = SubgroupFunctionalityFlags::Count;
		}
	}

	return true;
}

/// <summary>Gets the text to display at the bottom of the screen based on the current demo parameters </summary>
/// <param name="showSubgroupSelection">Tell the user to Swipe up for controls if false or pipeline selection if true</param>
/// <param name="controlSelected">Which index in the advanced controls are we showing</param>
/// <param name="pipelineIndex">The currently selected pipeline's index</param>
/// <returns>A string that UI renderer will display at the bottom left</returns>
std::string VulkanSubgroups::getUIRendererControlsText(bool showSubgroupSelection, uint8_t controlSelected, uint8_t pipelineIndex)
{
	// Are we displaying the subgroup selection screen
	if (!showSubgroupSelection) { return "Controls\nAction 1 : Show Subgroup Functionality Selection"; }

	// Construct the string
	std::string controls = "Controls\n"
						   "Action 1 : Hide Subgroup Functionality Selection\n";

	// Is the user using the compute only fallback
	bool computeFallback = pipelineIndex >= SubgroupFunctionalityFlags::Count;

	// Is the user selecting the basic subgroup feature
	controls += (controlSelected == SubgroupFunctionalityFlags::SubgroupBasic ? ">" : " ");
	controls += " Subgroup Basic Enabled      : ";
	controls += !computeFallback ? "True" : "False";

	// Is the user selecting the vote feature
	controls += (controlSelected == SubgroupFunctionalityFlags::SubgroupVote ? "\n>" : "\n ");
	controls += " Subgroup Vote Enabled       : ";
	controls += !computeFallback && pipelineIndex & SubgroupFunctionalityFlags::SubgroupVote ? "True" : "False";

	// Is the user selecting the ballot feature
	controls += (controlSelected == SubgroupFunctionalityFlags::SubgroupBallot ? "\n>" : "\n ");
	controls += " Subgroup Ballot Enabled     : ";
	controls += !computeFallback && pipelineIndex & SubgroupFunctionalityFlags::SubgroupBallot ? "True" : "False";

	// Is the user selecting the arithmetic feature
	controls += (controlSelected == SubgroupFunctionalityFlags::SubgroupArithmetic ? "\n>" : "\n ");
	controls += " Subgroup Arithmetic Enabled : ";
	controls += !computeFallback && pipelineIndex & SubgroupFunctionalityFlags::SubgroupArithmetic ? "True" : "False";

	return controls;
}

/// <summary>Turns the pipeline index into a string representing the name of the selected pipeline</summary>
/// <param name="pipelineIndex">Pipeline index</param>
/// <returns>A string for the name of the pipeline</returns>
std::string VulkanSubgroups::getPipelineNameText(uint8_t pipelineIndex)
{
	// Are we using the computeOnlyFallback?
	bool computeOnly = pipelineIndex >= SubgroupFunctionalityFlags::Count;

	if (computeOnly) { return "Compute Only Fallback"; }
	else
	{
		std::string builder = "Subgroup";
		if (pipelineIndex & SubgroupFunctionalityFlags::SubgroupVote) builder += ", Vote";
		if (pipelineIndex & SubgroupFunctionalityFlags::SubgroupBallot) builder += ", Ballot";
		if (pipelineIndex & SubgroupFunctionalityFlags::SubgroupArithmetic) builder += ", Arithmetic";
		return builder;
	}
}

/// <summary>Allocates texture memory on the GPU, for the compute shader to write to. One texture for each image in the swapchain
/// This is so we can have as many frames in flight as possible</summary>
void VulkanSubgroups::createComputeOutputTextures()
{
	// Allocate a one time use command buffer
	pvrvk::CommandBuffer cmd = _deviceResources->commandPool->allocateCommandBuffer();
	cmd->begin();

	// Create an output texture for each swapchain index
	for (uint32_t i = 0; i < _swapLength; i++)
	{
		// Create an rgba image the size of the swapchain image
		_deviceResources->computeOutputImages[i] = pvr::utils::createImage(_deviceResources->device,
			pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, pvrvk::Format::e_R8G8B8A8_UNORM, pvrvk::Extent3D(DemoSettings::computeTextureWidth, DemoSettings::computeTextureHeight, 1),
				pvrvk::ImageUsageFlags::e_STORAGE_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _deviceResources->vmaAllocator,
			pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);

		// Transform the image layout to general
		pvr::utils::setImageLayout(_deviceResources->computeOutputImages[i], pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_GENERAL, cmd);

		// Extract image view from the image object
		_deviceResources->computeOutputImageViews[i] = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(_deviceResources->computeOutputImages[i]));
	}

	// End the recording of the command buffer and submit
	cmd->end();

	pvrvk::SubmitInfo submit;
	submit.commandBuffers = &cmd;
	submit.numCommandBuffers = 1;
	_deviceResources->queues[0]->submit(&submit, 1);
	_deviceResources->queues[0]->waitIdle();
}

/// <summary>A helper function that calls all of the descriptor set and layout creations</summary>
void VulkanSubgroups::createDescriptorSetsAndLayouts()
{
	createcomputeOutputImageDescSets();
	createMatrixDescSets();
	createGraphicsDescSet();
}

/// <summary>Allocates a descriptor set for each compute output texture so it can be written to</summary>
void VulkanSubgroups::createcomputeOutputImageDescSets()
{
	// Create the descriptor set 0's layout for the compute shader
	// location 0 - storage image
	pvrvk::DescriptorSetLayoutCreateInfo layout;
	layout.setBinding(0, pvrvk::DescriptorType::e_STORAGE_IMAGE, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	_deviceResources->computeOutputImageDescSetLayout = _deviceResources->device->createDescriptorSetLayout(layout);

	// Create a list of each descriptor to write
	std::vector<pvrvk::WriteDescriptorSet> descriptorWriter;
	for (uint32_t i = 0; i < _swapLength; i++)
	{
		// For each frame in the swapchain allocate a descriptor set from the pool
		_deviceResources->computeOutputImageDescSets[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->computeOutputImageDescSetLayout);
		_deviceResources->computeOutputImageDescSets[i]->setObjectName("ComputeOutputImageSwapchain" + std::to_string(i) + "DescriptorSet");

		// For each frame in add a descriptor to be written
		descriptorWriter.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_IMAGE, _deviceResources->computeOutputImageDescSets[i], 0)
									   .setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->computeOutputImageViews[i], pvrvk::ImageLayout::e_GENERAL)));
	}

	// Write the descriptors
	_deviceResources->device->updateDescriptorSets(descriptorWriter.data(), static_cast<uint32_t>(descriptorWriter.size()), nullptr, 0);
}

/// <summary>Creates a dynamic buffer which has one slice for each image in the swapchain. The only one unique descriptor has
/// to be allocated for that buffer, then when binding to the set in the command buffer, we can specify an offset, and so the
/// pipeline will read the slice of the buffer corresponding to the matrix data required for that frame</summary>
void VulkanSubgroups::createMatrixDescSets()
{
	// Create a dynamic buffer view with _swapLength number of dynamic slices which contains the camera matrices
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement("mInvProjMatrix", pvr::GpuDatatypes::mat4x4);
	desc.addElement("mInvViewMatrix", pvr::GpuDatatypes::mat4x4);
	_deviceResources->matrixBufferView.initDynamic(desc, _swapLength, pvr::BufferUsageFlags::UniformTexelBuffer,
		static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

	// Create a dynamic buffer using the buffer view as a template
	_deviceResources->matrixBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->matrixBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->matrixBuffer->setObjectName("MatrixUBO");
	_deviceResources->matrixBufferView.pointToMappedMemory(_deviceResources->matrixBuffer->getDeviceMemory()->getMappedData());

	// Create some basic values for the camera matrices
	glm::mat4 proj = pvr::math::perspective(pvr::Api::Vulkan, 90, static_cast<float>(this->getWidth()) / static_cast<float>(this->getHeight()), 0.01, 1000.0);
	glm::mat4 view = glm::lookAt(_cameraPos, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

	// For each slice in the buffer, write the default value to the buffer
	for (uint32_t i = 0; i < _swapLength; i++)
	{
		_deviceResources->matrixBufferView.getElementByName("mInvProjMatrix", 0, i).setValue(glm::inverse(proj));
		_deviceResources->matrixBufferView.getElementByName("mInvViewMatrix", 0, i).setValue(glm::inverse(view));
	}

	// After writing to the buffer, we might need to flush the buffer if it doesn't contain e_HOST_COHERENT_BIT
	if (static_cast<uint32_t>(_deviceResources->matrixBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->matrixBuffer->getDeviceMemory()->flushRange(0, VK_WHOLE_SIZE);
	}

	// Create the descriptor set 1's layout for compute shader
	// location 0 - Camera Ubo
	pvrvk::DescriptorSetLayoutCreateInfo layout;
	layout.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	_deviceResources->matrixDescSetLayout = _deviceResources->device->createDescriptorSetLayout(layout);

	_deviceResources->matrixDescSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->matrixDescSetLayout);
	_deviceResources->matrixDescSet->setObjectName("MatrixDescriptorSet");

	// Since this is a dynamic descriptor set we only need to write one descriptor for the per frame resources
	pvrvk::WriteDescriptorSet descriptorWriter =
		pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->matrixDescSet)
			.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->matrixBuffer, 0, _deviceResources->matrixBufferView.getDynamicSliceSize()));
	_deviceResources->device->updateDescriptorSets(&descriptorWriter, 1, nullptr, 0);
}

/// <summary>Creates the descriptor sets required for the graphics pipeline to read the output of the compute pipeline, once
/// again this is done on once for each image in the swapchain</summary>
void VulkanSubgroups::createGraphicsDescSet()
{
	// Create the descriptor set 0's layout for the graphics pipeline
	// location 0 - read image
	pvrvk::DescriptorSetLayoutCreateInfo layout;
	layout.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	_deviceResources->graphicsDescSetLayout = _deviceResources->device->createDescriptorSetLayout(layout);

	// Create a sampler to copy the image to the screen
	pvrvk::SamplerCreateInfo sampleInfo;
	sampleInfo.minFilter = pvrvk::Filter::e_LINEAR;
	sampleInfo.magFilter = pvrvk::Filter::e_LINEAR;
	sampleInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_LINEAR;
	pvrvk::Sampler triNearest = _deviceResources->device->createSampler(sampleInfo);

	// Create a list of each descriptor to write
	std::vector<pvrvk::WriteDescriptorSet> descriptorWriter;
	for (uint32_t i = 0; i < _swapLength; i++)
	{
		// For each frame in the swapchain, allocate a descriptor set
		_deviceResources->graphicsDescSet[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->graphicsDescSetLayout);
		_deviceResources->graphicsDescSet[i]->setObjectName("GraphicsSwapchain" + std::to_string(i) + "DescriptorSet");

		// For each frame, add a descriptor to be written
		descriptorWriter.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->graphicsDescSet[i], 0)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->computeOutputImageViews[i], triNearest, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
	}

	// Write the descriptors
	_deviceResources->device->updateDescriptorSets(descriptorWriter.data(), static_cast<uint32_t>(descriptorWriter.size()), nullptr, 0);
}

/// <summary> Helper function that creates all the pipelines in this demo</summary>
void VulkanSubgroups::createPipelines()
{
	createGraphicsPipeline();
	createComputePipeline();
}

/// <summary> Creates the one graphics pipeline used in this demo, it copies the output from the compute shader to the swapchain </summary>
void VulkanSubgroups::createGraphicsPipeline()
{
	// For now the demo only needs a pipeline that will copy the output from a compute image
	pvrvk::GraphicsPipelineCreateInfo pipeDesc;

	// Pipeline layout
	pvrvk::PipelineLayoutCreateInfo pipeLayout;
	pipeLayout.addDescSetLayout(_deviceResources->graphicsDescSetLayout);
	_deviceResources->graphicsPipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayout);
	pipeDesc.pipelineLayout = _deviceResources->graphicsPipelineLayout;

	// Add the shaders to the pipelines
	std::unique_ptr<pvr::Stream> vertSource = getAssetStream(ShaderFilePaths::VertexShader);
	std::unique_ptr<pvr::Stream> fragSource = getAssetStream(ShaderFilePaths::FragmentShader);
	pipeDesc.vertexShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(vertSource->readToEnd<uint32_t>())));
	pipeDesc.fragmentShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(fragSource->readToEnd<uint32_t>())));

	// This graphics pipeline has hard coded vertices in the shader
	pipeDesc.vertexInput.clear();
	pipeDesc.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);

	// Graphics rasterization
	pipeDesc.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
	pipeDesc.rasterizer.setCullMode(pvrvk::CullModeFlags::e_NONE);
	pvr::utils::populateViewportStateCreateInfo(_deviceResources->onScreenFramebuffer[0], pipeDesc.viewport);

	// Renderpass from the onscreen framebuffer
	pipeDesc.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
	pipeDesc.subpass = 0;

	// Create pipeline
	_deviceResources->graphicsPipeline = _deviceResources->device->createGraphicsPipeline(pipeDesc);
	_deviceResources->graphicsPipeline->setObjectName("GraphicsPipeline");
}

/// <summary>Creates all of the compute pipelines for this demo, they all have the same layouts and description, however the shader
/// constants known as specialization constants change per pipeline. These give values to the const booleans in the SPIR-V and so
/// any branching will be optimized out by the pipeline compilation. Meaning the end result is a different shader path for each
/// subgroup functionality, without any dynamic branching</summary>
void VulkanSubgroups::createComputePipeline()
{
	// Create compute pipeline layout. They should all use the same layout
	// set 0 being the per swapchain image layout
	pvrvk::PipelineLayoutCreateInfo pipeLayout;
	pipeLayout.addDescSetLayout(_deviceResources->computeOutputImageDescSetLayout);
	pipeLayout.addDescSetLayout(_deviceResources->matrixDescSetLayout);
	_deviceResources->computePipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayout);

	// Attach the layout to the compute pipeline create info
	pvrvk::ComputePipelineCreateInfo pipeDesc;
	pipeDesc.pipelineLayout = _deviceResources->computePipelineLayout;

	// Pass specialization constants into the spirv, so that we can set the compute workgroup sizes at runtime
	pipeDesc.computeShader.setShaderConstant(0, pvrvk::ShaderConstantInfo(0, &DemoSettings::workGroupWidth, sizeof(uint32_t)));
	pipeDesc.computeShader.setShaderConstant(1, pvrvk::ShaderConstantInfo(1, &DemoSettings::workGroupHeight, sizeof(uint32_t)));

	// Read In the compute shader
	std::unique_ptr<pvr::Stream> compSource = getAssetStream(ShaderFilePaths::ComputeShader);
	pipeDesc.computeShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(compSource->readToEnd<uint32_t>())));

	// Now that we have the basic compute fallback shader, Fill the deviceResources vector of compute pipelines with the fallback
	_deviceResources->computePiplines.clear();
	_deviceResources->computePiplines.resize(SubgroupFunctionalityFlags::Count + 1);

	// Now loop through each of the subgroup shaders and add them to the list

	for (uint8_t i = 0; i < SubgroupFunctionalityFlags::Count + 1; i++)
	{
		Log(LogLevel::Information, "Compiling compute shader : %s", getPipelineNameText(i).c_str());

		// In order to create an easier to read demo, we want to use one compute shader, so we need to pass which subgroup features are being used
		// as a series of bools to the compute shader. However, preproccessors can't be used because the shader code has already been compiled into SPIRV.
		// Using specialization constants will allow us to pass constant bools into the SPIRV, these branches will then not be dynamic and so won't shut
		// off any shader invocations. There's even a chance that the SPIRV -> hardware compiler will spot these branches on constants and optimize them out
		bool subgroupBasic = false, subgroupVote = false, subgroupBallot = false, subgroupArithmetic = false;
		if (i < SubgroupFunctionalityFlags::Count)
		{
			subgroupBasic = true;
			subgroupVote = i & SubgroupFunctionalityFlags::SubgroupVote;
			subgroupBallot = i & SubgroupFunctionalityFlags::SubgroupBallot;
			subgroupArithmetic = i & SubgroupFunctionalityFlags::SubgroupArithmetic;
		}

		pipeDesc.computeShader.setShaderConstant(2, pvrvk::ShaderConstantInfo(2, &subgroupBasic, sizeof(VkBool32)));
		pipeDesc.computeShader.setShaderConstant(3, pvrvk::ShaderConstantInfo(3, &subgroupVote, sizeof(VkBool32)));
		pipeDesc.computeShader.setShaderConstant(4, pvrvk::ShaderConstantInfo(4, &subgroupBallot, sizeof(VkBool32)));
		pipeDesc.computeShader.setShaderConstant(5, pvrvk::ShaderConstantInfo(5, &subgroupArithmetic, sizeof(VkBool32)));

		// Create and add the pipeline
		pvrvk::ComputePipeline pipeline = _deviceResources->device->createComputePipeline(pipeDesc);
		pipeline->setObjectName("SubgroupFunctionality" + std::to_string(i) + "ComputePipeline");
		_deviceResources->computePiplines[i] = pipeline;
	}
}

/// <summary> Used to record the primary command buffer, assuming the secondary command buffers have already been recorded </summary>
void VulkanSubgroups::recordPrimaryCommandBuffers()
{
	const pvrvk::ClearValue clearValue[] = { pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 1.0f) };

	for (uint32_t i = 0; i < _swapLength; i++)
	{
		// Begin the primary command buffer
		_deviceResources->primaryCmdBuffers[i]->begin();

		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->primaryCmdBuffers[i], pvrvk::DebugUtilsLabel("MainRenderPassSwapchain" + std::to_string(i)));

		// Run the compute stage
		_deviceResources->primaryCmdBuffers[i]->executeCommands(_deviceResources->computeSecondaryCmdBuffers[i]);

		// Start the onscreen renderpass
		_deviceResources->primaryCmdBuffers[i]->beginRenderPass(_deviceResources->onScreenFramebuffer[i], false, clearValue, 1);
		_deviceResources->primaryCmdBuffers[i]->executeCommands(_deviceResources->graphicsSecondaryCmdBuffers[i]);
		_deviceResources->primaryCmdBuffers[i]->executeCommands(_deviceResources->uiSecondaryCmdBuffers[i]);
		_deviceResources->primaryCmdBuffers[i]->endRenderPass();

		pvr::utils::endCommandBufferDebugLabel(_deviceResources->primaryCmdBuffers[i]);

		// End the command buffer
		_deviceResources->primaryCmdBuffers[i]->end();
	}
}

/// <summary> Records all of the secondary command buffers </summary>
void VulkanSubgroups::recordSecondaryCommandBuffers()
{
	for (uint32_t i = 0; i < _swapLength; i++)
	{
		recordComputeCommandBuffer(i);
		recordGraphicsCommandBuffer(i);
		recordUICommandBuffer(i);
	}
}

/// <summary>Records the command buffer for the compute pass for the current pipeline index</summary>
/// <param name="i">The command buffer index that we're recording for</param>
void VulkanSubgroups::recordComputeCommandBuffer(uint32_t i)
{
	// Begin the compute command buffer, ensuring to label the command buffer with the compute pipeline being executed
	_deviceResources->computeSecondaryCmdBuffers[i]->begin();
	pvr::utils::beginCommandBufferDebugLabel(_deviceResources->computeSecondaryCmdBuffers[i], pvrvk::DebugUtilsLabel("Compute Work : " + getPipelineNameText(_computePipelineIndex)));

	// Add an image memory barrier to ensure the compute output image is in the layout we need it to be
	pvrvk::MemoryBarrierSet barriers;

	// Set a barrier that transitions the image back to writable once the graphics pipeline is done with it, so the compute can write to it the next time
	barriers.clearAllBarriers();
	barriers.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::AccessFlags::e_SHADER_WRITE_BIT, _deviceResources->computeOutputImages[i],
		pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_GENERAL,
		_deviceResources->queues[0]->getFamilyIndex(), _deviceResources->queues[0]->getFamilyIndex()));
	_deviceResources->computeSecondaryCmdBuffers[i]->pipelineBarrier(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, barriers);

	// Bind to the pipeline and the descriptor sets for this frame
	_deviceResources->computeSecondaryCmdBuffers[i]->bindPipeline(_deviceResources->computePiplines.at(_computePipelineIndex));
	_deviceResources->computeSecondaryCmdBuffers[i]->bindDescriptorSet(
		pvrvk::PipelineBindPoint::e_COMPUTE, _deviceResources->computePipelineLayout, 0, _deviceResources->computeOutputImageDescSets[i]);

	// Bind to the correct offset inside the dynamic buffer
	uint32_t descriptorOffset[1] = { _deviceResources->matrixBufferView.getDynamicSliceOffset(i) };
	_deviceResources->computeSecondaryCmdBuffers[i]->bindDescriptorSet(
		pvrvk::PipelineBindPoint::e_COMPUTE, _deviceResources->computePipelineLayout, 1, _deviceResources->matrixDescSet, descriptorOffset, 1);

	// Run the compute shader
	_deviceResources->computeSecondaryCmdBuffers[i]->dispatch(DemoSettings::dispatchWidth, DemoSettings::dispatchHeight, 1);

	// Set a barrier that transitions the image to read only, so the graphics pipeline can use it
	barriers.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, _deviceResources->computeOutputImages[i],
		pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_GENERAL, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL,
		_deviceResources->queues[0]->getFamilyIndex(), _deviceResources->queues[0]->getFamilyIndex()));
	_deviceResources->computeSecondaryCmdBuffers[i]->pipelineBarrier(pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, barriers);

	pvr::utils::endCommandBufferDebugLabel(_deviceResources->computeSecondaryCmdBuffers[i]);

	// end the compute shader
	_deviceResources->computeSecondaryCmdBuffers[i]->end();
}

/// <summary> Records the command buffer for the graphics pipeline that copies the compute image over to the swapchain </summary>
/// <param name="i">Index into the command buffer array that is being recorded</param>
void VulkanSubgroups::recordGraphicsCommandBuffer(uint32_t i)
{
	// begin the command buffer with a debug label
	_deviceResources->graphicsSecondaryCmdBuffers[i]->begin(_deviceResources->onScreenFramebuffer[i]);
	pvr::utils::beginCommandBufferDebugLabel(_deviceResources->graphicsSecondaryCmdBuffers[i], pvrvk::DebugUtilsLabel("Copy compute output to the swapchain"));

	// bind to the pipeline and descriptor sets for this frame
	_deviceResources->graphicsSecondaryCmdBuffers[i]->bindPipeline(_deviceResources->graphicsPipeline);
	_deviceResources->graphicsSecondaryCmdBuffers[i]->bindDescriptorSet(
		pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->graphicsPipelineLayout, 0, _deviceResources->graphicsDescSet[i]);

	_deviceResources->graphicsSecondaryCmdBuffers[i]->draw(0, 3);

	pvr::utils::endCommandBufferDebugLabel(_deviceResources->graphicsSecondaryCmdBuffers[i]);

	// end this secondary command buffer
	_deviceResources->graphicsSecondaryCmdBuffers[i]->end();
}

/// <summary>Records the command buffer for the UI renderer </summary>
/// <param name="i">Index into the command buffer array that is being recorded</param>
void VulkanSubgroups::recordUICommandBuffer(uint32_t i)
{
	_deviceResources->uiSecondaryCmdBuffers[i]->begin(_deviceResources->onScreenFramebuffer[i]);
	_deviceResources->uiRenderer.beginRendering(_deviceResources->uiSecondaryCmdBuffers[i]);
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.endRendering();
	_deviceResources->uiSecondaryCmdBuffers[i]->end();
}
