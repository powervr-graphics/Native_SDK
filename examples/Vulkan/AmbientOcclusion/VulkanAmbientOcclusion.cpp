/*!
\brief Implements SSAO ambient occlusion demo optimized for IMG hardware
\file VulkanAmbientOcclusion.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

/*!
Ambient Occlusion description:
	The ambient occlusion demo uses a deferred rendering pipeline, the Gbuffer has a Albedo, Normal and depth attachment. The Normal and depth attachments are passed forward to the ambient occlusion
	renderpass, they are used to represent the geometry and thus can be used to calculate the amount of occlusion. The ambient occlusion pass is heavily reliant on textures, so to reduce texturing
	overhead, the AO texture is at half resolution. The Ambient occlusion renderpass uses samples to generate the occlusion per fragment, these samples are randomly rotated in a 3x3 array, thus there
	is a 3x3 interference pattern. Perform a 3x3 Gaussian blur to negate this. This is done in 2 separable passes. The first blur render pass is a horizontal blur still at half resolution. Then the
	next blur pass is a subpass of a larger presentation renderpass, this upscales the Ambient occlusion texture and finalizes the blur. Now the fully formed and blurred ambient occlusion texture can
	be passed to the composite pass via local pixel storage. The composite pass allows the user to turn on and off the ambient occlusion, so that they can see the difference.
*/

// include the framework
#include "PVRShell/PVRShell.h"
#include "PVRVk/PVRVk.h"
#include "PVRUtils/PVRUtilsVk.h"
// Used for generating samples
#include <random>

// Specify the input shape for the vertex shaders using a pvr format
namespace VertexBindings {
const pvr::utils::VertexBindings_Name sceneVertexInput[] = { { "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoord" } };
const uint32_t sceneVertexInputLength = 3;
} // namespace VertexBindings

// Indexes the RenderPasses
namespace RenderPasses {
enum Enum
{
	GBuffer,
	AmbientOcclusion,
	HorizontalBlur,
	Presentation,
};
} // namespace RenderPasses

// Indexes the subpasses
namespace Subpasses {
enum Enum
{
	GBuffer,
	AmbientOcclusion,
	HorizontalBlur,
	VerticalBlur,
	Composite
};
} // namespace Subpasses

// Indexes the UBOs
namespace UBOs {
enum Enum
{
	AOParamaters,
	CompositeParams
};
} // namespace UBOs

struct DeviceResources
{
	// Communicate with the device.
	pvrvk::Instance instance;
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
	pvrvk::Device device;
	pvrvk::Queue queue;
	pvrvk::Swapchain swapchain;
	pvr::utils::vma::Allocator vmaAllocator;

	// Command and descriptor pool to allocate from
	pvrvk::CommandPool commandPool;
	pvrvk::DescriptorPool descriptorPool;

	// command buffers
	pvr::Multi<pvrvk::CommandBuffer> cmdBuffers;

	// synchronization objects
	// semaphores for when the image is ready to be drawn to and when it is ready for presenting
	// Create a resource for each of the framebuffers in the swapchain, take the maximum number of buffers this windowing system supports
	pvrvk::Semaphore imageAcquiredSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Semaphore presentationSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Fence perFrameResourcesFences[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	// vertex buffer and index buffer objects for the scene
	std::vector<pvrvk::Buffer> sceneVbos;
	std::vector<pvrvk::Buffer> sceneIbos;

	// Pipeline Cache
	pvrvk::PipelineCache pipelineCache;

	// For each Subpass there is exactly one graphics pipeline
	pvr::Multi<pvrvk::GraphicsPipeline, 5> pipelines;
	pvr::Multi<pvrvk::PipelineLayout, 5> pipelineLayouts;

	// For each render pass there is N framebuffers, where N is the length of the swapchain
	// There is one image view per frameBuffer attachment, and one image view per material
	pvr::Multi<pvrvk::RenderPass> renderPasses;
	pvr::Multi<pvr::Multi<pvrvk::Framebuffer>> framebuffers;
	pvr::Multi<pvrvk::ImageView> modelTextureViews;
	pvr::Multi<pvrvk::ImageView> albedoAttachment;
	pvr::Multi<pvrvk::ImageView> normalsAttachment;
	pvr::Multi<pvrvk::ImageView> depthAttachment;
	pvr::Multi<pvrvk::ImageView> ambientOcclusionAttachment;
	pvr::Multi<pvrvk::ImageView> horizontalBlurredAttachment;
	pvr::Multi<pvrvk::ImageView> verticalBlurredAttachment;
	pvr::Multi<pvrvk::ImageView> compositeAttachment;

	// For each subpass there is one input descriptor set layout, that layout is then used for multiple sets.
	// on Gpass the input sets are per material, the other subpasses input sets are per frame buffer.
	pvr::Multi<pvrvk::DescriptorSetLayout, 5> inputDescSetLayouts;
	pvr::Multi<pvr::Multi<pvrvk::DescriptorSet>, 5> inputDescSets;

	// Use a dynamic buffer to store the per model uniform buffer objects
	pvrvk::Buffer modelBuffer;
	pvr::utils::StructuredBufferView modelBufferView;

	// For each uniform buffer there is a Buffer, structured buffer view, descriptor set layout, and a descriptor set
	pvr::Multi<pvrvk::Buffer, 3> uniformBuffers;
	pvr::Multi<pvr::utils::StructuredBufferView, 3> uniformBufferViews;
	pvr::Multi<pvrvk::DescriptorSetLayout, 3> uniformDescSetLayouts;
	pvr::Multi<pvrvk::DescriptorSet, 3> uniformDescSets;

	// UI renderer to display text
	pvr::ui::UIRenderer uiRenderer;

	~DeviceResources()
	{
		if (device) { device->waitIdle(); }
		// clear the swapchain resources
		uint32_t swapchainLength = swapchain->getSwapchainLength();
		for (uint32_t i = 0; i < swapchainLength; i++)
		{
			if (perFrameResourcesFences[i]) { perFrameResourcesFences[i]->wait(); }
		}
	}
};

class VulkanAmbientOcclusion : public pvr::Shell
{
	// all the api resources are bundled together for easier releasing.
	std::unique_ptr<DeviceResources> _resources;

	// store the swapchain length as it is frequently accessed
	uint32_t _swapLength = 0;

	// Identify the index of the current frame, modulo number of buffers the windowing system supports
	// This will index the currently active framebuffer
	uint32_t _frameID = 0;

	// Handle to load the scene data
	const char* _sceneFilePath = "Saloon.pod";
	pvr::assets::ModelHandle _sceneHandle;

	// Identify the animation index in the scene
	float _animationID = 0.0;
	bool _animate = true;

	// Set the size of the Ambient occlusion samples and the random rotations
	uint32_t _aoSampleSize = 32;
	uint32_t _aoRotationSize = 9;

	// Hold the parameters for how the colors should be mixed in the presentation pass
	float _compositeParams[3][2] = { { 1.0, 1.0 }, { 1.0, 0.0 }, { 0.0, 1.0 } };
	std::string _uiLabels[3] = { "Albedo and Ambient Occlusion", "Albedo", "Ambient Occlusion" };
	uint32_t _compositeParamsCount = 3;
	uint32_t _compositeParamsID = 0;
	bool _updateAoParams = true;

public:
	// Overridden from PVRShell
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();
	virtual void eventMappedInput(pvr::SimplifiedInput key);

private:
	// Create the Framebuffer and attachments, render passes and the associated framebuffer objects
	void createFramebufferAtachments();
	void createRenderpasses();
	void createFramebufferObjects();

	// Create all the UBO buffers for the different render passes and then
	// upload any data that remains constant across the runtime of the project
	void createBuffers();
	void updateBuffers();
	void uploadStaticData(pvrvk::CommandBuffer cmdBuffer);

	// Once all the buffers have been created, create all the descriptor sets
	void createUBODescriptorSets();
	void createInputDescriptorSets();

	// Once the descriptor sets have been created create the pipelines for each subpass
	void createPipelines();

	// Record the prebaked command buffers that will remain unchanged for the runtime of the program
	void recordCommandBuffers();
};

/// <summary>Will be called by pvr::Shell every time the rendering context changes or is lost. Will be used to set up variables dependent on the rendering context</summary>
/// <returns>Success if no errors occurred</returns>
pvr::Result VulkanAmbientOcclusion::initView()
{
	_resources = std::make_unique<DeviceResources>();

	// create an instance and query for any Vulkan compatible devices
	_resources->instance = pvr::utils::createInstance(this->getApplicationName());
	if (_resources->instance->getNumPhysicalDevices() == 0)
	{
		setExitMessage("Unable to find a compatible Vulkan physical device.");
		return pvr::Result::UnknownError;
	}

	// Since we can assume we have a Vulkan device, create a debug callback messenger
	_resources->debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(_resources->instance);
	pvrvk::PhysicalDevice physicalDevice = _resources->instance->getPhysicalDevice(0);

	// Establish the connection between Vulkan and the windowing system with a surface
	pvrvk::Surface surface = pvr::utils::createSurface(_resources->instance, physicalDevice, this->getWindow(), this->getDisplay(), this->getConnection());

	// Use the surface and the physical device to to create a logical device and a queue
	const pvr::utils::QueuePopulateInfo queuePopulateInfo = { pvrvk::QueueFlags::e_GRAPHICS_BIT, surface };
	pvr::utils::QueueAccessInfo queueAccessInfo;
	_resources->device = pvr::utils::createDeviceAndQueues(physicalDevice, &queuePopulateInfo, 1, &queueAccessInfo);
	_resources->queue = _resources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	// An addition for SDK examples is to validate that the swapchain supports screen shots
	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice->getSurfaceCapabilities(surface);
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT;
	}

	// create the Vulkan memory allocator
	_resources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_resources->device));

	// create the swapchain and on screen framebuffer using the device, vmallocator and swapchain usage defined for screen shots
	auto swapchainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_resources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_resources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage));
	_resources->swapchain = swapchainCreateOutput.swapchain;
	_resources->framebuffers[RenderPasses::Presentation] = swapchainCreateOutput.framebuffer;

	// Store the swapchain length for repeated use
	_swapLength = _resources->swapchain->getSwapchainLength();

	// create the command pool and descriptor pool
	_resources->commandPool = _resources->device->createCommandPool(pvrvk::CommandPoolCreateInfo(queueAccessInfo.familyId, pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));
	if (!_resources->commandPool) { return pvr::Result::UnknownError; }

	// Allocate enough descriptor pool memory for the application
	_resources->descriptorPool =
		_resources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
													 .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, 2)
													 .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 2)
													 .addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 2 + static_cast<uint16_t>(6 * _swapLength))
													 .addDescriptorInfo(pvrvk::DescriptorType::e_INPUT_ATTACHMENT, static_cast<uint16_t>(1 * _swapLength))
													 .setMaxDescriptorSets(4 + static_cast<uint16_t>(4 * _swapLength)));
	if (!_resources->descriptorPool) { return pvr::Result::UnknownError; }

	// create the synchronization objects and command buffers
	for (uint32_t i = 0; i < _resources->swapchain->getSwapchainLength(); i++)
	{
		_resources->presentationSemaphores[i] = _resources->device->createSemaphore();
		_resources->imageAcquiredSemaphores[i] = _resources->device->createSemaphore();
		_resources->perFrameResourcesFences[i] = _resources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_resources->cmdBuffers[i] = _resources->commandPool->allocateCommandBuffer();
	}

	// Framebuffers and render passes
	createFramebufferAtachments();
	createRenderpasses();
	createFramebufferObjects();

	// Buffers and static data such as textures and meshes
	createBuffers();
	uploadStaticData(_resources->cmdBuffers[0]);

	// Descriptor sets
	createUBODescriptorSets();
	createInputDescriptorSets();

	// Graphics pipelines and record command buffers
	createPipelines();
	recordCommandBuffers();

	return pvr::Result::Success;
}

/// <summary>Will be called by pvr::Shell when the application quits or before a rendering context change</summary>
/// <returns>Success if no errors occurred</returns>
pvr::Result VulkanAmbientOcclusion::releaseView()
{
	_resources.reset();
	return pvr::Result::Success;
}

/// <summary>Will be called by pvr::Shell once every frame and is the main rendering loop of the program</summary>
/// <returns>Success if no errors occurred</returns>
pvr::Result VulkanAmbientOcclusion::renderFrame()
{
	updateBuffers();

	// Acquire the next frame in the queue
	_resources->swapchain->acquireNextImage(uint64_t(-1), _resources->imageAcquiredSemaphores[_frameID]);
	const uint32_t swapchainIndex = _resources->swapchain->getSwapchainIndex();
	_resources->perFrameResourcesFences[swapchainIndex]->wait();
	_resources->perFrameResourcesFences[swapchainIndex]->reset();

	// Create submit information that has the correct sync objects
	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags pipeWaitStageFlags = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;

	submitInfo.commandBuffers = &_resources->cmdBuffers[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_resources->imageAcquiredSemaphores[_frameID];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_resources->presentationSemaphores[_frameID];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.waitDstStageMask = &pipeWaitStageFlags;

	// Submit the command queue
	_resources->queue->submit(&submitInfo, 1, _resources->perFrameResourcesFences[swapchainIndex]);

	// Take a screen shot?
	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(
			_resources->queue, _resources->commandPool, _resources->swapchain, swapchainIndex, this->getScreenshotFileName(), _resources->vmaAllocator, _resources->vmaAllocator);
	}

	// Create the present information so that the rendered frame can be presented
	pvrvk::PresentInfo presentInfo;
	presentInfo.swapchains = &_resources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.waitSemaphores = &_resources->presentationSemaphores[_frameID];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.imageIndices = &swapchainIndex;
	_resources->queue->present(presentInfo);

	// update the frameID so that the next frame can be grabbed on the next call
	_frameID = (_frameID + 1) % _swapLength;
	return pvr::Result::Success;
}

/// <summary>Will be called by pvr::Shell once per run, before the graphics context is initialized. If the graphics context
/// is lost this will not be ran again, as a result, limit initialization to variables independent of the graphics context</summary>
/// <returns>Success if no errors occurred</returns>
pvr::Result VulkanAmbientOcclusion::initApplication()
{
	_sceneHandle = pvr::assets::loadModel(*this, _sceneFilePath);
	return pvr::Result::Success;
}

/// <summary>Will be called by pvr::Shell once per run, just before the application is closed, only once.</summary>
/// <returns>Success if no errors occurred</returns>
pvr::Result VulkanAmbientOcclusion::quitApplication()
{
	_sceneHandle.reset();
	_resources.reset();
	return pvr::Result::Success;
}

/// <summary>Will be called by pvr::Shell whenever there is an input event</summary>
/// <param name="key">The platform agnostic input event that was fired</param>
void VulkanAmbientOcclusion::eventMappedInput(pvr::SimplifiedInput key)
{
	// If the user presses left or right, update the method used to composite the different attachments
	// If the user does a simple press or click, turn on or off the animation
	// Or quit the application
	switch (key)
	{
	case pvr::SimplifiedInput::Left:
		// Modulo undefined for negative numbers
		if (_compositeParamsID == 0) { _compositeParamsID = _compositeParamsCount - 1; }
		else
		{
			_compositeParamsID = (_compositeParamsID - 1) % _compositeParamsCount;
		}
		_updateAoParams = true;
		break;
	case pvr::SimplifiedInput::Right:
		_compositeParamsID = (_compositeParamsID + 1) % _compositeParamsCount;
		_updateAoParams = true;
		break;
	case pvr::SimplifiedInput::Action1: _animate = !_animate; break;
	case pvr::SimplifiedInput::ActionClose: exitShell(); break;
	default: break;
	}
}

/// <summary>This function must be implemented, the user should return the pvr::Shell object defining the behavior of the application</summary>
/// <returns>Returns a unique pointer to the demo supplied by the user</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanAmbientOcclusion>(); }

/// <summary>Creates the buffers and their buffer views, this includes the dynamic per model buffer and the UBOs</summary>
void VulkanAmbientOcclusion::createBuffers()
{
	// Dynamic per model dynamic buffer
	pvr::utils::StructuredMemoryDescription modelBufferDesc;
	modelBufferDesc.addElement("MVPMatrix", pvr::GpuDatatypes::mat4x4);
	modelBufferDesc.addElement("NormalMatrix", pvr::GpuDatatypes::mat4x4);

	// AO parameters, stays the same for the runtime of the program
	pvr::utils::StructuredMemoryDescription aoBufferDesc;
	aoBufferDesc.addElement("SamplePositions", pvr::GpuDatatypes::vec3, _aoSampleSize);
	aoBufferDesc.addElement("SampleRotations", pvr::GpuDatatypes::vec3, _aoRotationSize);
	aoBufferDesc.addElement("Projection", pvr::GpuDatatypes::mat4x4);
	aoBufferDesc.addElement("ProjectionInv", pvr::GpuDatatypes::mat4x4);
	_resources->uniformBufferViews[UBOs::AOParamaters].init(aoBufferDesc);

	// Composite parameters
	pvr::utils::StructuredMemoryDescription compositeBufferDesc;
	compositeBufferDesc.addElement("AlbedoStrength", pvr::GpuDatatypes::Float);
	compositeBufferDesc.addElement("AOStrength", pvr::GpuDatatypes::Float);
	_resources->uniformBufferViews[UBOs::CompositeParams].init(compositeBufferDesc);

	// Initialize a dynamic buffer for per model UBO
	_resources->modelBufferView.initDynamic(modelBufferDesc, _sceneHandle->getNumMeshNodes(), pvr::BufferUsageFlags::UniformBuffer,
		_resources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment());

	// Create the buffers
	_resources->modelBuffer = pvr::utils::createBuffer(_resources->device,
		pvrvk::BufferCreateInfo(_resources->modelBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _resources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

	_resources->uniformBuffers[UBOs::AOParamaters] = pvr::utils::createBuffer(_resources->device,
		pvrvk::BufferCreateInfo(_resources->uniformBufferViews[UBOs::AOParamaters].getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
		pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _resources->vmaAllocator,
		pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

	_resources->uniformBuffers[UBOs::CompositeParams] = pvr::utils::createBuffer(_resources->device,
		pvrvk::BufferCreateInfo(_resources->uniformBufferViews[UBOs::CompositeParams].getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
		pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _resources->vmaAllocator,
		pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

	// Associate the buffers to their buffer view
	_resources->modelBufferView.pointToMappedMemory(_resources->modelBuffer->getDeviceMemory()->getMappedData());
	_resources->uniformBufferViews[UBOs::AOParamaters].pointToMappedMemory(_resources->uniformBuffers[UBOs::AOParamaters]->getDeviceMemory()->getMappedData());
	_resources->uniformBufferViews[UBOs::CompositeParams].pointToMappedMemory(_resources->uniformBuffers[UBOs::CompositeParams]->getDeviceMemory()->getMappedData());
}

/// <summary>Updates the contents of the Ubo Buffers that change once per frame, this includes the MVPMatrix, the Composite parameters and the UI Renderer</summary>
void VulkanAmbientOcclusion::updateBuffers()
{
	if (_animate)
	{
		// Get the SDKs method of animation handling and modulo the animation index so that it repeats seamlessly
		_animationID += static_cast<float>(getFrameTime());
		pvr::assets::AnimationInstance animation = _sceneHandle->getAnimationInstance(0);
		_animationID = fmod(_animationID, animation.getTotalTimeInMs());
		animation.updateAnimation(_animationID);

		// Gather the projection view matrix from the scene handle
		float fov;
		glm::vec3 cameraPos(0, 0, 0), cameraTarget(0, 0, 0), cameraUp(0, 0, 0);
		_sceneHandle->getCameraProperties(0, fov, cameraPos, cameraTarget, cameraUp);
		glm::mat4x4 view = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0, 1, 0));
		glm::mat4x4 projection = pvr::math::perspective(pvr::Api::Vulkan, fov, static_cast<float>(this->getWidth()) / static_cast<float>(this->getHeight()),
			_sceneHandle->getCamera(0).getNear(), _sceneHandle->getCamera(0).getFar());

		// Update the per model normal matrix
		for (uint32_t i = 0; i < _sceneHandle->getNumMeshNodes(); i++)
		{
			glm::mat4 modelToWorld = _sceneHandle->getWorldMatrix(i);
			glm::mat3x3 NormalMat = glm::mat3x3(glm::inverseTranspose(view * modelToWorld));
			_resources->modelBufferView.getElementByName("NormalMatrix", 0, i).setValue(NormalMat);
			_resources->modelBufferView.getElementByName("MVPMatrix", 0, i).setValue(projection * view * modelToWorld);
		}

		// The memory must be flushed if the devices memory's flags does not contain the HOST_COHERENT_BIT
		// In this case it is known that the entire dynamic buffer has been updated
		if (static_cast<uint32_t>(_resources->uniformBuffers[UBOs::CompositeParams]->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_resources->modelBuffer->getDeviceMemory()->flushRange(0, _resources->modelBufferView.getSize());
		}
	}

	// Update the composite params
	if (_updateAoParams)
	{
		_resources->uniformBufferViews[UBOs::CompositeParams].getElementByName("AlbedoStrength").setValue(_compositeParams[_compositeParamsID][0]);
		_resources->uniformBufferViews[UBOs::CompositeParams].getElementByName("AOStrength").setValue(_compositeParams[_compositeParamsID][1]);

		// Flush the device memory if required
		if (static_cast<uint32_t>(_resources->uniformBuffers[UBOs::CompositeParams]->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_resources->uniformBuffers[UBOs::CompositeParams]->getDeviceMemory()->flushRange(0, _resources->uniformBufferViews[UBOs::CompositeParams].getSize());
		}

		if (_compositeParamsID == _compositeParamsCount - 1)
		{
			// Set the UIRenderText to be black because the AO pass is white
			_resources->uiRenderer.getDefaultTitle()->setColor(glm::vec4(0.0, 0.0, 0.0, 1.0));
			_resources->uiRenderer.getDefaultDescription()->setColor(glm::vec4(0.0, 0.0, 0.0, 1.0));
			_resources->uiRenderer.getDefaultControls()->setColor(glm::vec4(0.0, 0.0, 0.0, 1.0));
		}
		else
		{
			_resources->uiRenderer.getDefaultTitle()->setColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
			_resources->uiRenderer.getDefaultDescription()->setColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
			_resources->uiRenderer.getDefaultControls()->setColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
		}
		_resources->uiRenderer.getDefaultDescription()->setText(_uiLabels[_compositeParamsID]);
		_resources->uiRenderer.getDefaultDescription()->commitUpdates();
		_resources->uiRenderer.getDefaultControls()->commitUpdates();
		_resources->uiRenderer.getDefaultTitle()->commitUpdates();
		_updateAoParams = false;
	}
}

/// <summary>Produces an array of randomly distributed samples in tangent space</summary>
/// <param name="size">How many samples to produces</param>
/// <returns>Array of randomly distributed tangent space samples</returns>
std::vector<glm::vec3> createAOSamples(uint32_t size)
{
	std::vector<glm::vec3> samplePositions;
	std::uniform_real_distribution<float> randomFloats(0.1, 0.9);
	std::default_random_engine generator;

	for (uint32_t i = 0; i < size; i++)
	{
		// x,y range from (-1,1) and z range (0,1), ensuring we get a hemisphere in tangent space
		glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
		sample = glm::normalize(sample);

		// Cluster the samples in the center
		float scale = randomFloats(generator);
		scale = (0.1f + static_cast<float>(scale * scale * (0.9)));
		sample *= scale;

		samplePositions.push_back(sample);
	}

	return samplePositions;
}

/// <summary>Produces a randomly distributed series of rotation vectors with z component 0 to rotate the tangent space samples around z axis</summary>
/// <param name="size">How many random rotation vectors</param>
/// <returns>randomly distributed series of rotation vectors</returns>
std::vector<glm::vec3> createRandomRotations(uint32_t size)
{
	std::vector<glm::vec3> sampleRotations;
	// The random rotations will be about the z axis in tangent space, to do this ensure the z component is 0
	for (size_t i = 0; i < size; i++)
	{
		float yaw = 2 * glm::pi<float>() * static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		glm::vec4 random(0, 0, 0, 0);
		random.x = cos(yaw);
		random.y = sin(yaw);
		random.z = 0;
		sampleRotations.push_back(random);
	}
	return sampleRotations;
}

/// <summary>Uploads data to the GPU that doesn't change over time, for this demo this includes the meshes, material data and the sampling buffers</summary>
void VulkanAmbientOcclusion::uploadStaticData(pvrvk::CommandBuffer cmdBuffer)
{
	// Upload the mesh vertices
	bool requiresSubmission = false;
	cmdBuffer->begin();

	pvr::utils::appendSingleBuffersFromModel(
		_resources->device, *_sceneHandle, _resources->sceneVbos, _resources->sceneIbos, _resources->cmdBuffers[0], requiresSubmission, _resources->vmaAllocator);

	cmdBuffer->end();

	// submit the upload command buffer
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_resources->cmdBuffers[0];
	submitInfo.numCommandBuffers = 1;
	_resources->queue->submit(&submitInfo, 1);
	_resources->queue->waitIdle();

	// for each distinct material, upload the associated texture
	for (uint32_t i = 0; i < _sceneHandle->getNumMaterials(); i++)
	{
		// get the albedo texture index from the material
		pvr::assets::Material material = _sceneHandle->getMaterial(i);
		uint32_t textureID = material.defaultSemantics().getDiffuseTextureIndex();
		// Get the file path of the texture
		const char* filePath = _sceneHandle->getTexture(textureID).getName().c_str();

		// use the asset loader to get the texture
		std::unique_ptr<pvr::Stream> textureStream = getAssetStream(filePath);
		pvr::Texture tex = pvr::textureLoad(*textureStream, pvr::TextureFileFormat::PVR);

		// Upload this texture to the GPU
		pvrvk::ImageView view = pvr::utils::uploadImageAndViewSubmit(_resources->device, tex, true, _resources->commandPool, _resources->queue, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, _resources->vmaAllocator, _resources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);

		_resources->modelTextureViews[i] = view;
	}

	// Create and upload the ambient occlusion samples and rotations
	_resources->uniformBufferViews[UBOs::AOParamaters].getElementByName("SamplePositions").setValue(createAOSamples(_aoSampleSize).data());
	_resources->uniformBufferViews[UBOs::AOParamaters].getElementByName("SampleRotations").setValue(createRandomRotations(_aoRotationSize).data());

	// Set the projection and inverse projection matrix in the Ambient occlusion pass, these remain constant as the window cannot be resized
	float fov = _sceneHandle->getCamera(0).getFOV();
	glm::mat4x4 projection = pvr::math::perspective(pvr::Api::Vulkan, fov, static_cast<float>(this->getWidth()) / static_cast<float>(this->getHeight()),
		_sceneHandle->getCamera(0).getNear(), _sceneHandle->getCamera(0).getFar());
	_resources->uniformBufferViews[UBOs::AOParamaters].getElementByName("Projection").setValue(projection);
	_resources->uniformBufferViews[UBOs::AOParamaters].getElementByName("ProjectionInv").setValue(glm::inverse(projection));

	// Flush the device memory if required
	if (static_cast<uint32_t>(_resources->uniformBuffers[UBOs::AOParamaters]->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_resources->uniformBuffers[UBOs::AOParamaters]->getDeviceMemory()->flushRange(0, _resources->uniformBufferViews[UBOs::AOParamaters].getSize());
	}
}

/// <summary> Create the image views for each framebuffer attachment </summary>
void VulkanAmbientOcclusion::createFramebufferAtachments()
{
	// The render targets have two different sizes they are either full screen sized or half sized
	// There is an extra dimension for the transient image views, as they use a different creation method to set the image properties
	const pvrvk::Extent2D fullscreenDimension = _resources->swapchain->getDimension();
	const pvrvk::Extent2D halfSizedDimension = pvrvk::Extent2D(fullscreenDimension.getWidth() / 2, fullscreenDimension.getHeight() / 2);
	const pvrvk::Extent3D transcientDimension = pvrvk::Extent3D(fullscreenDimension.getWidth(), fullscreenDimension.getHeight(), 1);

	// Use pvr::Utils to create the framebuffer attachments
	// Albedo : 16 bit RGBA
	pvr::utils::createAttachmentImages(_resources->albedoAttachment, _resources->device, _swapLength, pvrvk::Format::e_R16G16B16A16_SFLOAT, fullscreenDimension,
		pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::SampleCountFlags::e_1_BIT, _resources->vmaAllocator,
		pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT, "AlbedoAttachment");

	// Normal : 16 bit RGBA
	pvr::utils::createAttachmentImages(_resources->normalsAttachment, _resources->device, _swapLength, pvrvk::Format::e_R16G16B16A16_SFLOAT, fullscreenDimension,
		pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::SampleCountFlags::e_1_BIT, _resources->vmaAllocator,
		pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT, "BufferPassNormalAttachment");

	// Depth : Supported Depth and stencil attachment
	pvr::utils::createAttachmentImages(_resources->depthAttachment, _resources->device, _swapLength,
		pvr::utils::getSupportedDepthStencilFormat(_resources->device, getDisplayAttributes()), fullscreenDimension,
		pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::SampleCountFlags::e_1_BIT, _resources->vmaAllocator,
		pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT, "DepthStencilImageBuffers");

	// Ambient Occlusion : 32 bit signed float - Half sized texture
	pvr::utils::createAttachmentImages(_resources->ambientOcclusionAttachment, _resources->device, _swapLength, pvrvk::Format::e_R32_SFLOAT, halfSizedDimension,
		pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::SampleCountFlags::e_1_BIT, _resources->vmaAllocator,
		pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);

	// Horizontal blur : 32 bit signed float - Half sized texture
	pvr::utils::createAttachmentImages(_resources->horizontalBlurredAttachment, _resources->device, _swapLength, pvrvk::Format::e_R32_SFLOAT, halfSizedDimension,
		pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::SampleCountFlags::e_1_BIT, _resources->vmaAllocator,
		pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);

	// The vertical blur needs to be a transient image attachment so it can be passed to the presentation pass via PLS
	// The presentation pass, take the image view already created by the SDK
	for (uint32_t i = 0; i < _swapLength; i++)
	{
		// Create a transient image attachment for the vertical Blur
		pvrvk::Image blurAttachment = pvr::utils::createImage(_resources->device,
			pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, pvrvk::Format::e_R32_SFLOAT, transcientDimension,
				pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_INPUT_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT,
			_resources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);

		// Store the image of thee transient images
		_resources->verticalBlurredAttachment[i] = _resources->device->createImageView(pvrvk::ImageViewCreateInfo(blurAttachment));

		// Take the on screen framebuffer color attachment from the SDK for the composition image, the one that will be presented
		_resources->compositeAttachment[i] = _resources->swapchain->getImageView(i);
	}
}

/// <summary> Create the render passes, this is dependent on the framebuffer attachments being created </summary>
void VulkanAmbientOcclusion::createRenderpasses()
{
	pvrvk::RenderPassCreateInfo renderPassCreateInfo[RenderPasses::Presentation + 1];

	// For each renderpass firstly create a description for each of the render targets, this will control the image layout transition that occur after a renderpass ends

	// Gbuffer render pass :
	//	0 : Albedo
	//	1 : Normals
	//	2 : Depth Stencil
	renderPassCreateInfo[RenderPasses::GBuffer].setAttachmentDescription(0,
		pvrvk::AttachmentDescription::createColorDescription(_resources->albedoAttachment[0]->getImage()->getFormat(), pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT));

	renderPassCreateInfo[RenderPasses::GBuffer].setAttachmentDescription(1,
		pvrvk::AttachmentDescription::createColorDescription(_resources->normalsAttachment[0]->getImage()->getFormat(), pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT));

	renderPassCreateInfo[RenderPasses::GBuffer].setAttachmentDescription(2,
		pvrvk::AttachmentDescription::createDepthStencilDescription(_resources->depthAttachment[0]->getImage()->getFormat(), pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::AttachmentLoadOp::e_CLEAR,
			pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT));

	// Ambient Occlusion render pass :
	//	0 : Ambient Occlusion color
	// Horizontal blur renderpass has exactly the same settings as the AO render pass, so the create info can be reused.
	renderPassCreateInfo[RenderPasses::AmbientOcclusion].setAttachmentDescription(0,
		pvrvk::AttachmentDescription::createColorDescription(_resources->ambientOcclusionAttachment[0]->getImage()->getFormat(), pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT));

	// Presentation render pass
	//	0: composite
	//	1: Vertical blur - can be lazily allocated as it is transient
	renderPassCreateInfo[RenderPasses::Presentation].setAttachmentDescription(0,
		pvrvk::AttachmentDescription::createColorDescription(_resources->compositeAttachment[0]->getImage()->getFormat(), pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_PRESENT_SRC_KHR, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT));

	renderPassCreateInfo[RenderPasses::Presentation].setAttachmentDescription(1,
		pvrvk::AttachmentDescription::createColorDescription(_resources->verticalBlurredAttachment[0]->getImage()->getFormat(), pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE, pvrvk::SampleCountFlags::e_1_BIT));

	// Create a subpass description for each subpass, sets which attachments are outputs for each subpass
	// Once again, the horizontal blur description is identical to the ambient occlusion subpass
	pvrvk::SubpassDescription subpassDesc[Subpasses::Composite + 1];

	// G Buffer Render pass, just one subpass
	subpassDesc[Subpasses::GBuffer].setColorAttachmentReference(0, pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	subpassDesc[Subpasses::GBuffer].setColorAttachmentReference(1, pvrvk::AttachmentReference(1, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	subpassDesc[Subpasses::GBuffer].setDepthStencilAttachmentReference(pvrvk::AttachmentReference(2, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
	renderPassCreateInfo[RenderPasses::GBuffer].setSubpass(0, subpassDesc[Subpasses::GBuffer]);

	// Ambient occlusion (And horizontal blur) just one subpass
	subpassDesc[Subpasses::AmbientOcclusion].setColorAttachmentReference(0, pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	renderPassCreateInfo[RenderPasses::AmbientOcclusion].setSubpass(0, subpassDesc[Subpasses::AmbientOcclusion]);

	// Presentation pass, 2 subpasses and one transient image
	subpassDesc[Subpasses::VerticalBlur].setColorAttachmentReference(0, pvrvk::AttachmentReference(1, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	subpassDesc[Subpasses::VerticalBlur].setPreserveAttachmentReference(0, 0);
	subpassDesc[Subpasses::Composite].setInputAttachmentReference(0, pvrvk::AttachmentReference(1, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
	subpassDesc[Subpasses::Composite].setColorAttachmentReference(0, pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	renderPassCreateInfo[RenderPasses::Presentation].setSubpass(0, subpassDesc[Subpasses::VerticalBlur]);
	renderPassCreateInfo[RenderPasses::Presentation].setSubpass(1, subpassDesc[Subpasses::Composite]);

	// Subpasses will need to wait for previous dependent subpasses to finish before they can execute, instead of using barriers, this can be done
	// with subpass dependencies, there are two kinds used here. External dependencies between render passes, and internal for between subpasses
	pvrvk::SubpassDependency internalDependency;
	internalDependency.setSrcSubpass(0);
	internalDependency.setDstSubpass(1);
	internalDependency.setSrcStageMask(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT);
	internalDependency.setSrcAccessMask(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT);
	internalDependency.setDstStageMask(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT);
	internalDependency.setDstAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);

	pvrvk::SubpassDependency externalDependency;
	externalDependency.setSrcSubpass(pvrvk::SubpassExternal);
	externalDependency.setDstSubpass(0);
	externalDependency.setSrcStageMask(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT);
	externalDependency.setSrcAccessMask(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT);
	externalDependency.setDstStageMask(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT);
	externalDependency.setDstAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);

	// Give all render passes that depend on a previous renderpass an external dependency
	renderPassCreateInfo[RenderPasses::AmbientOcclusion].addSubpassDependency(externalDependency);
	renderPassCreateInfo[RenderPasses::Presentation].addSubpassDependency(externalDependency);

	// Give the presentation pass a internal dependency to wait for it's subpass
	renderPassCreateInfo[RenderPasses::Presentation].addSubpassDependency(internalDependency);

	// Clone the AO renderpass create info into the horizontal blur.
	renderPassCreateInfo[RenderPasses::HorizontalBlur] = renderPassCreateInfo[RenderPasses::AmbientOcclusion];

	// Create all of the render passes
	for (uint32_t i = 0; i < RenderPasses::Presentation + 1; i++) { _resources->renderPasses[i] = _resources->device->createRenderPass(renderPassCreateInfo[i]); }
}

/// <summary> Create the framebuffer objects, this is dependent on the render passes being created first </summary>
void VulkanAmbientOcclusion::createFramebufferObjects()
{
	pvrvk::Extent2D fullScreenDimension = _resources->swapchain->getDimension();
	pvrvk::Extent2D halfSizeDimension = pvrvk::Extent2D(fullScreenDimension.getWidth() / 2, fullScreenDimension.getHeight() / 2);
	// For each element in the swapchain create a framebuffer object for each renderpass
	for (uint32_t i = 0; i < _swapLength; i++)
	{
		pvrvk::FramebufferCreateInfo fboCreateInfo[RenderPasses::Presentation + 1];

		// GBuffer FBO
		fboCreateInfo[RenderPasses::GBuffer].setAttachment(0, _resources->albedoAttachment[i]);
		fboCreateInfo[RenderPasses::GBuffer].setAttachment(1, _resources->normalsAttachment[i]);
		fboCreateInfo[RenderPasses::GBuffer].setAttachment(2, _resources->depthAttachment[i]);
		fboCreateInfo[RenderPasses::GBuffer].setDimensions(fullScreenDimension);
		fboCreateInfo[RenderPasses::GBuffer].setRenderPass(_resources->renderPasses[RenderPasses::GBuffer]);

		// AO FBO
		fboCreateInfo[RenderPasses::AmbientOcclusion].setAttachment(0, _resources->ambientOcclusionAttachment[i]);
		fboCreateInfo[RenderPasses::AmbientOcclusion].setDimensions(halfSizeDimension);
		fboCreateInfo[RenderPasses::AmbientOcclusion].setRenderPass(_resources->renderPasses[RenderPasses::AmbientOcclusion]);

		// Horizontal blur FBO
		fboCreateInfo[RenderPasses::HorizontalBlur].setAttachment(0, _resources->horizontalBlurredAttachment[i]);
		fboCreateInfo[RenderPasses::HorizontalBlur].setDimensions(halfSizeDimension);
		fboCreateInfo[RenderPasses::HorizontalBlur].setRenderPass(_resources->renderPasses[RenderPasses::HorizontalBlur]);

		// Presentation FBO
		fboCreateInfo[RenderPasses::Presentation].setAttachment(0, _resources->compositeAttachment[i]);
		fboCreateInfo[RenderPasses::Presentation].setAttachment(1, _resources->verticalBlurredAttachment[i]);
		fboCreateInfo[RenderPasses::Presentation].setDimensions(fullScreenDimension);
		fboCreateInfo[RenderPasses::Presentation].setRenderPass(_resources->renderPasses[RenderPasses::Presentation]);

		// Create the fbos
		for (uint32_t j = 0; j < RenderPasses::Presentation + 1; j++) { _resources->framebuffers[j][i] = _resources->device->createFramebuffer(fboCreateInfo[j]); }
	}

	// Setup the UI renderer
	_resources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _resources->renderPasses[RenderPasses::Presentation], 1,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _resources->commandPool, _resources->queue);
	_resources->uiRenderer.getDefaultTitle()->setText("Ambient Occlusion");
	_resources->uiRenderer.getDefaultTitle()->commitUpdates();

	_resources->uiRenderer.getDefaultDescription()->setText(_uiLabels[0]);
	_resources->uiRenderer.getDefaultDescription()->commitUpdates();

	_resources->uiRenderer.getDefaultControls()->setText("Action 1 : Pause or Play Animation\nLeft / Right : Change How Occlusion is Composited");
	_resources->uiRenderer.getDefaultControls()->commitUpdates();
}

/// <summary> Create all the descriptor sets for the UBOs, that is the AO samples and the composite parameters</summary>
void VulkanAmbientOcclusion::createUBODescriptorSets()
{
	pvrvk::DescriptorSetLayoutCreateInfo layoutCreateInfo[UBOs::CompositeParams + 1];

	// create the descriptor set layouts for each UBO
	// AO samples
	layoutCreateInfo[UBOs::AOParamaters].setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	_resources->uniformDescSetLayouts[UBOs::AOParamaters] = _resources->device->createDescriptorSetLayout(layoutCreateInfo[UBOs::AOParamaters]);
	// Composite parameters
	layoutCreateInfo[UBOs::CompositeParams].setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	_resources->uniformDescSetLayouts[UBOs::CompositeParams] = _resources->device->createDescriptorSetLayout(layoutCreateInfo[UBOs::CompositeParams]);

	// Allocate the descriptor sets from their layouts
	_resources->uniformDescSets[UBOs::AOParamaters] = _resources->descriptorPool->allocateDescriptorSet(_resources->uniformDescSetLayouts[UBOs::AOParamaters]);
	_resources->uniformDescSets[UBOs::CompositeParams] = _resources->descriptorPool->allocateDescriptorSet(_resources->uniformDescSetLayouts[UBOs::CompositeParams]);

	// Use a vector to store the information about the UBO descriptors, so that they can be updated in one go
	std::vector<pvrvk::WriteDescriptorSet> descriptorSetWriter;

	// Ambient Occlusion samples
	descriptorSetWriter.push_back(
		pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _resources->uniformDescSets[UBOs::AOParamaters], 0)
			.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_resources->uniformBuffers[UBOs::AOParamaters], 0, _resources->uniformBufferViews[UBOs::AOParamaters].getDynamicSliceSize())));

	// Composite parameters
	descriptorSetWriter.push_back(
		pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _resources->uniformDescSets[UBOs::CompositeParams], 0)
			.setBufferInfo(
				0, pvrvk::DescriptorBufferInfo(_resources->uniformBuffers[UBOs::CompositeParams], 0, _resources->uniformBufferViews[UBOs::CompositeParams].getDynamicSliceSize())));

	// Update the descriptors
	_resources->device->updateDescriptorSets(descriptorSetWriter.data(), static_cast<uint32_t>(descriptorSetWriter.size()), nullptr, 0);
}

/// <summary> Creates the descriptor sets that will be used as input for the render passes,
/// this includes the samplers for all the color attachments of the previous passes,
/// and the model textures as input to the Gbuffer pass</summary>
void VulkanAmbientOcclusion::createInputDescriptorSets()
{
	pvrvk::DescriptorSetLayoutCreateInfo layoutCreateInfo[Subpasses::Composite + 1];

	// create the per model descriptor set layout
	//	binding 0: Dynamic uniform buffer
	//	binding 1: Input texture
	layoutCreateInfo[Subpasses::GBuffer].setBinding(
		0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	layoutCreateInfo[Subpasses::GBuffer].setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	// Create the descriptor set layouts for the the color attachments
	// Ambient occlusion
	//	binding 0: Normal
	//	binding 1: Depth buffer
	layoutCreateInfo[Subpasses::AmbientOcclusion].setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	layoutCreateInfo[Subpasses::AmbientOcclusion].setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	// Horizontal blur
	//	binding 0: Ambient Occlusion texture
	layoutCreateInfo[Subpasses::HorizontalBlur].setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER);

	// Vertical blur
	//	binding 0: Horizontally blurred texture
	layoutCreateInfo[Subpasses::VerticalBlur].setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER);

	// Presentation
	//	binding 0: Albedo
	//	binding 1: Blurred AO via LPS
	layoutCreateInfo[Subpasses::Composite].setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	layoutCreateInfo[Subpasses::Composite].setBinding(1, pvrvk::DescriptorType::e_INPUT_ATTACHMENT, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	// Create the descriptor set layouts
	for (uint32_t i = 0; i < Subpasses::Composite + 1; i++) { _resources->inputDescSetLayouts[i] = _resources->device->createDescriptorSetLayout(layoutCreateInfo[i]); }

	// To write the descriptor sets for the inputs, a sampler is needed to create the combined image samplers
	pvrvk::SamplerCreateInfo samplerCreateInfo;
	samplerCreateInfo.minFilter = pvrvk::Filter::e_LINEAR;
	samplerCreateInfo.magFilter = pvrvk::Filter::e_LINEAR;
	samplerCreateInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_LINEAR;
	pvrvk::Sampler trilinear = _resources->device->createSampler(samplerCreateInfo);

	// Use a vector to store the information about the UBO descriptors, so that they can be updated in one go
	std::vector<pvrvk::WriteDescriptorSet> descriptorSetWriter;

	// For each unique material allocate a descriptor set for the model inputs.
	for (uint32_t i = 0; i < _sceneHandle->getNumMaterials(); i++)
	{
		_resources->inputDescSets[Subpasses::GBuffer][i] = _resources->descriptorPool->allocateDescriptorSet(_resources->inputDescSetLayouts[Subpasses::GBuffer]);

		// Add the dynamic buffer descriptor
		descriptorSetWriter.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _resources->inputDescSets[Subpasses::GBuffer][i], 0)
										  .setBufferInfo(0, pvrvk::DescriptorBufferInfo(_resources->modelBuffer, 0, _resources->modelBufferView.getDynamicSliceSize())));

		// Add the combined image sampler descriptor
		descriptorSetWriter.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _resources->inputDescSets[Subpasses::GBuffer][i], 1)
										  .setImageInfo(0, pvrvk::DescriptorImageInfo(_resources->modelTextureViews[i], trilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
	}

	// And for each element in the swapchain, allocate a descriptor set for the input attachments.
	for (uint32_t i = 0; i < _swapLength; i++)
	{
		for (uint32_t j = 1; j < Subpasses::Composite + 1; j++)
		{
			// Allocate all the descriptor sets for the attachments
			_resources->inputDescSets[j][i] = _resources->descriptorPool->allocateDescriptorSet(_resources->inputDescSetLayouts[j]);
		}
		// Ambient Occlusion Pass : Normal attachment
		descriptorSetWriter.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _resources->inputDescSets[Subpasses::AmbientOcclusion][i], 0)
										  .setImageInfo(0, pvrvk::DescriptorImageInfo(_resources->normalsAttachment[i], trilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
		// Ambient Occlusion Pass : depth attachment
		descriptorSetWriter.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _resources->inputDescSets[Subpasses::AmbientOcclusion][i], 1)
										  .setImageInfo(0, pvrvk::DescriptorImageInfo(_resources->depthAttachment[i], trilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		// Horizontal Blur Pass : Unblurred AO attachment
		descriptorSetWriter.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _resources->inputDescSets[Subpasses::HorizontalBlur][i], 0)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(_resources->ambientOcclusionAttachment[i], trilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		// Vertical Blur Subpass : Horizontal blur attachment
		descriptorSetWriter.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _resources->inputDescSets[Subpasses::VerticalBlur][i], 0)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(_resources->horizontalBlurredAttachment[i], trilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		// Composite Pass: Albedo attachment
		descriptorSetWriter.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _resources->inputDescSets[Subpasses::Composite][i], 0)
										  .setImageInfo(0, pvrvk::DescriptorImageInfo(_resources->albedoAttachment[i], trilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
		// Composite Pass: Fully blurred transient attachment
		descriptorSetWriter.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_INPUT_ATTACHMENT, _resources->inputDescSets[Subpasses::Composite][i], 1)
										  .setImageInfo(0, pvrvk::DescriptorImageInfo(_resources->verticalBlurredAttachment[i], pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
	}

	// Update all the descriptor sets that have been placed into the vector
	_resources->device->updateDescriptorSets(descriptorSetWriter.data(), static_cast<uint32_t>(descriptorSetWriter.size()), nullptr, 0);
}

/// <summary>Creates the graphics pipeline for this demo, there is one pipeline for each subpass</summary>
void VulkanAmbientOcclusion::createPipelines()
{
	_resources->pipelineCache = _resources->device->createPipelineCache();

	// Create the pipeline layouts, used to set the indexes of the descriptor sets in the shader
	pvrvk::PipelineLayoutCreateInfo layoutCreateInfo[Subpasses::Composite + 1];

	// Set the UBOs to set 1 in the subpasses that have them
	layoutCreateInfo[Subpasses::AmbientOcclusion].setDescSetLayout(1, _resources->uniformDescSetLayouts[UBOs::AOParamaters]);
	layoutCreateInfo[Subpasses::Composite].setDescSetLayout(1, _resources->uniformDescSetLayouts[UBOs::CompositeParams]);
	for (uint32_t i = 0; i < Subpasses::Composite + 1; i++)
	{
		// Set all the input descriptor sets to be set 0 then create the layouts
		layoutCreateInfo[i].setDescSetLayout(0, _resources->inputDescSetLayouts[i]);
		_resources->pipelineLayouts[i] = _resources->device->createPipelineLayout(layoutCreateInfo[i]);
	}

	// Use one pipeline create info, as the vast majority of settings are mainly reusable from each pipeline.
	// Start with the GBufferPass
	pvrvk::GraphicsPipelineCreateInfo pipelineCreateInfo;
	pipelineCreateInfo.pipelineLayout = _resources->pipelineLayouts[Subpasses::GBuffer];

	// Set information about the pipeline such as the attachments and cull mode
	pipelineCreateInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
	pipelineCreateInfo.colorBlend.setAttachmentState(1, pvrvk::PipelineColorBlendAttachmentState());
	pipelineCreateInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);
	pvr::utils::populateViewportStateCreateInfo(_resources->framebuffers[RenderPasses::GBuffer][0], pipelineCreateInfo.viewport);

	// Read in and set the shader source code for the pipeline
	std::unique_ptr<pvr::Stream> vertSource = getAssetStream("GBuffer.vsh.spv");
	std::unique_ptr<pvr::Stream> fragSource = getAssetStream("GBuffer.fsh.spv");
	pipelineCreateInfo.vertexShader.setShader(_resources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(vertSource->readToEnd<uint32_t>())));
	pipelineCreateInfo.fragmentShader.setShader(_resources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(fragSource->readToEnd<uint32_t>())));

	// Load the model into the input assembly for the pipeline
	const pvr::assets::Mesh& bunnyMesh = _sceneHandle->getMesh(0);
	pipelineCreateInfo.vertexInput.clear();
	pipelineCreateInfo.inputAssembler.setPrimitiveTopology(pvr::utils::convertToPVRVk(bunnyMesh.getPrimitiveType()));
	pvr::utils::populateInputAssemblyFromMesh(
		bunnyMesh, VertexBindings::sceneVertexInput, VertexBindings::sceneVertexInputLength, pipelineCreateInfo.vertexInput, pipelineCreateInfo.inputAssembler);

	// pipeline descriptions details about the render pass
	pipelineCreateInfo.renderPass = _resources->renderPasses[RenderPasses::GBuffer];
	pipelineCreateInfo.depthStencil.enableDepthTest(true);
	pipelineCreateInfo.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_LESS);
	pipelineCreateInfo.depthStencil.enableDepthWrite(true);
	pipelineCreateInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);
	pipelineCreateInfo.subpass = 0;

	if (getAASamples() > 1)
	{
		pipelineCreateInfo.multiSample.setSampleShading(true);
		pipelineCreateInfo.multiSample.setNumRasterizationSamples(pvr::utils::convertToPVRVkNumSamples((uint8_t)getAASamples()));
	}

	// Create the GBuffer pipeline
	_resources->pipelines[Subpasses::GBuffer] = _resources->device->createGraphicsPipeline(pipelineCreateInfo, _resources->pipelineCache);
	_resources->pipelines[Subpasses::GBuffer]->setObjectName("GBufferPipeline");

	// Change the graphics pipeline create info to match the rest of the subpasses, these are all screen space effects with the depth test disabled
	pipelineCreateInfo.depthStencil.enableDepthTest(false);
	pipelineCreateInfo.depthStencil.enableDepthWrite(false);
	// Remove the input assembler for the vertex shader since the screen space effects have hard coded values for the vertex shader
	pipelineCreateInfo.vertexInput.clear();

	// Ambient Occlusion pass
	// Only one color attachment, so need to remove the attachment states
	pipelineCreateInfo.colorBlend.clearAttachments();
	pipelineCreateInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

	// Set the renderpass and the pipeline layout
	pipelineCreateInfo.renderPass = _resources->renderPasses[RenderPasses::AmbientOcclusion];
	pipelineCreateInfo.pipelineLayout = _resources->pipelineLayouts[Subpasses::AmbientOcclusion];

	// Update the vertex shader to be a screen space effect and update the fragment shader to be the ambient occlusion shader
	vertSource = getAssetStream("ScreenSpaceEffect.vsh.spv");
	fragSource = getAssetStream("AmbientOcclusion.fsh.spv");
	pipelineCreateInfo.vertexShader.setShader(_resources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(vertSource->readToEnd<uint32_t>())));
	pipelineCreateInfo.fragmentShader.setShader(_resources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(fragSource->readToEnd<uint32_t>())));

	// AO Pass hasd a downscaled render target, so set the view port to match
	pipelineCreateInfo.viewport.clear();
	pvr::utils::populateViewportStateCreateInfo(_resources->framebuffers[RenderPasses::AmbientOcclusion][0], pipelineCreateInfo.viewport);

	// Create the AO Pipeline
	_resources->pipelines[Subpasses::AmbientOcclusion] = _resources->device->createGraphicsPipeline(pipelineCreateInfo, _resources->pipelineCache);

	// Horizontal Blur pass
	pipelineCreateInfo.renderPass = _resources->renderPasses[RenderPasses::HorizontalBlur];
	pipelineCreateInfo.pipelineLayout = _resources->pipelineLayouts[Subpasses::HorizontalBlur];
	fragSource = getAssetStream("BlurHorizontal.fsh.spv");
	pipelineCreateInfo.fragmentShader.setShader(_resources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(fragSource->readToEnd<uint32_t>())));
	_resources->pipelines[Subpasses::HorizontalBlur] = _resources->device->createGraphicsPipeline(pipelineCreateInfo, _resources->pipelineCache);

	// Presentation pass : Vertical Blur subpass
	// Reset the view port back to full sized
	pipelineCreateInfo.viewport.clear();
	pvr::utils::populateViewportStateCreateInfo(_resources->framebuffers[RenderPasses::Presentation][0], pipelineCreateInfo.viewport);
	pipelineCreateInfo.renderPass = _resources->renderPasses[RenderPasses::Presentation];
	pipelineCreateInfo.pipelineLayout = _resources->pipelineLayouts[Subpasses::VerticalBlur];
	fragSource = getAssetStream("BlurVertical.fsh.spv");
	pipelineCreateInfo.fragmentShader.setShader(_resources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(fragSource->readToEnd<uint32_t>())));
	_resources->pipelines[Subpasses::VerticalBlur] = _resources->device->createGraphicsPipeline(pipelineCreateInfo, _resources->pipelineCache);

	// Presentation pass : Composition subpass
	pipelineCreateInfo.subpass = 1;
	pipelineCreateInfo.pipelineLayout = _resources->pipelineLayouts[Subpasses::Composite];
	fragSource = getAssetStream("Composite.fsh.spv");
	pipelineCreateInfo.fragmentShader.setShader(_resources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(fragSource->readToEnd<uint32_t>())));
	_resources->pipelines[Subpasses::Composite] = _resources->device->createGraphicsPipeline(pipelineCreateInfo, _resources->pipelineCache);
}

/// <summary>Prerecords the prebaked command buffers, one for each framebuffer in the swapchain</summary>
void VulkanAmbientOcclusion::recordCommandBuffers()
{
	// Create the clear values for the different framebuffer objects. Clear the Albedo attachment to blue.
	glm::vec3 clearColorLinearSpace(0.0f, 0.45f, 0.41f);
	pvrvk::ClearValue gClearValues[3] = { pvrvk::ClearValue(clearColorLinearSpace.x, clearColorLinearSpace.y, clearColorLinearSpace.z, 1.0f), pvrvk::ClearValue(0.f, 0.f, 0.f, 0.f),
		pvrvk::ClearValue(1.f, 0u) };
	// Clear values for the Textures used AO creation texture and horizontal blurred texture
	pvrvk::ClearValue downscaledClearValues[1] = { pvrvk::ClearValue(0.f, 0u) };
	// Clear values for the presentation framebuffer with two attachments
	pvrvk::ClearValue onScreenClearValues[2] = { pvrvk::ClearValue(0.f, 0u), pvrvk::ClearValue(0.f, 0u) };

	for (uint32_t i = 0; i < _resources->swapchain->getSwapchainLength(); i++)
	{
		_resources->cmdBuffers[i]->begin();

		// Gbuffer renderpass
		_resources->cmdBuffers[i]->beginRenderPass(
			_resources->framebuffers[RenderPasses::GBuffer][i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, gClearValues, ARRAY_SIZE(gClearValues));
		_resources->cmdBuffers[i]->bindPipeline(_resources->pipelines[Subpasses::GBuffer]);

		// Go through each mesh node and draw them to the Gbuffer attachment
		for (uint32_t j = 0; j < _sceneHandle->getNumMeshNodes(); j++)
		{
			const pvr::assets::Node node = _sceneHandle->getNode(j);
			const pvr::assets::Mesh mesh = _sceneHandle->getMesh(node.getObjectId());
			// Use the material index to bind to the correct descriptor set for this mesh
			uint32_t descriptorIndex = node.getMaterialIndex();
			// Get the starting position of this mesh node's ubo in the dynamic buffer
			uint32_t bufferOffset = _resources->modelBufferView.getDynamicSliceOffset(j);

			_resources->cmdBuffers[i]->bindVertexBuffer(_resources->sceneVbos.at(node.getObjectId()), 0, 0);
			_resources->cmdBuffers[i]->bindIndexBuffer(_resources->sceneIbos.at(node.getObjectId()), 0, pvr::utils::convertToPVRVk(mesh.getFaces().getDataType()));

			_resources->cmdBuffers[i]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _resources->pipelineLayouts[Subpasses::GBuffer], 0u,
				_resources->inputDescSets[Subpasses::GBuffer][descriptorIndex], &bufferOffset, 1);
			_resources->cmdBuffers[i]->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
		}
		_resources->cmdBuffers[i]->endRenderPass();

		// AO generation renderpass
		_resources->cmdBuffers[i]->beginRenderPass(_resources->framebuffers[RenderPasses::AmbientOcclusion][i], pvrvk::Rect2D(0, 0, getWidth() / 2, getHeight() / 2), true,
			downscaledClearValues, ARRAY_SIZE(downscaledClearValues));
		_resources->cmdBuffers[i]->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _resources->pipelineLayouts[Subpasses::AmbientOcclusion], 0, _resources->inputDescSets[Subpasses::AmbientOcclusion][i]);
		_resources->cmdBuffers[i]->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _resources->pipelineLayouts[Subpasses::AmbientOcclusion], 1, _resources->uniformDescSets[UBOs::AOParamaters]);
		_resources->cmdBuffers[i]->bindPipeline(_resources->pipelines[Subpasses::AmbientOcclusion]);
		_resources->cmdBuffers[i]->draw(0, 3, 0, 1);
		_resources->cmdBuffers[i]->endRenderPass();

		// Horizontal blur renderpass
		_resources->cmdBuffers[i]->beginRenderPass(_resources->framebuffers[RenderPasses::HorizontalBlur][i], pvrvk::Rect2D(0, 0, getWidth() / 2, getHeight() / 2), true,
			downscaledClearValues, ARRAY_SIZE(downscaledClearValues));
		_resources->cmdBuffers[i]->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _resources->pipelineLayouts[Subpasses::HorizontalBlur], 0, _resources->inputDescSets[Subpasses::HorizontalBlur][i]);
		_resources->cmdBuffers[i]->bindPipeline(_resources->pipelines[Subpasses::HorizontalBlur]);
		_resources->cmdBuffers[i]->draw(0, 3, 0, 1);
		_resources->cmdBuffers[i]->endRenderPass();

		// Presentation pass - Vertical blur subpass
		_resources->cmdBuffers[i]->beginRenderPass(
			_resources->framebuffers[RenderPasses::Presentation][i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, onScreenClearValues, ARRAY_SIZE(onScreenClearValues));
		_resources->cmdBuffers[i]->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _resources->pipelineLayouts[Subpasses::VerticalBlur], 0, _resources->inputDescSets[Subpasses::VerticalBlur][i]);
		_resources->cmdBuffers[i]->bindPipeline(_resources->pipelines[Subpasses::VerticalBlur]);
		_resources->cmdBuffers[i]->draw(0, 3, 0, 1);

		// Presentation pass - composite subpass
		_resources->cmdBuffers[i]->nextSubpass(pvrvk::SubpassContents::e_INLINE);
		_resources->cmdBuffers[i]->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _resources->pipelineLayouts[Subpasses::Composite], 0, _resources->inputDescSets[Subpasses::Composite][i]);
		_resources->cmdBuffers[i]->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _resources->pipelineLayouts[Subpasses::Composite], 1, _resources->uniformDescSets[UBOs::CompositeParams]);
		_resources->cmdBuffers[i]->bindPipeline(_resources->pipelines[Subpasses::Composite]);
		_resources->cmdBuffers[i]->draw(0, 3, 0, 1);

		// UI Pass
		_resources->uiRenderer.beginRendering(_resources->cmdBuffers[i]);
		_resources->uiRenderer.getDefaultTitle()->render();
		_resources->uiRenderer.getDefaultDescription()->render();
		_resources->uiRenderer.getDefaultControls()->render();
		_resources->uiRenderer.getSdkLogo()->render();
		_resources->uiRenderer.endRendering();

		// Finished rendering
		_resources->cmdBuffers[i]->endRenderPass();
		_resources->cmdBuffers[i]->end();
	}
}
