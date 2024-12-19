/*!
\brief Implements a fully raytraced scene with hard shadows using the Vulkan Khronos raytracing extensions
\file VulkanRayTracedHardShadows.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRVk/PVRVk.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRUtils/Vulkan/AccelerationStructure.h"
#include "vulkan/vulkan_beta.h"

/// <summary> Maximum length of the swapchain, reserve space for this many copies of the per frame resources, declared as a constant at compile time</summary>
constexpr uint32_t MAX_NUMBER_OF_SWAP_IMAGES = 4;

/// <summary>All the file paths for any resources loaded at runtime, including the scene file and the shaders</summary>
namespace Files {
/// <summary> The POD file is the file path to the scene </summary>
const char* const SceneFile = "HardShadows.POD";

/// <summary> File path to the compiled raygen shader, the start of the raytracing pipeline</summary>
const char* const RayGenShader = "RayGen.rgen.spv";

/// <summary> File path to the primary miss shader, it basically sets the clear color for this demo</summary>
const char* const RayMissShader = "RayMiss.rmiss.spv";

/// <summary> File path to the primary hit shader, traces from the camera to scene geometry</summary>
const char* const RayHitShader = "RayHit.rchit.spv";

/// <summary> File path for the shadow miss shader, if this executes the scene is in lighting</summary>
const char* const ShadowMissShader = "ShadowMiss.rmiss.spv";

/// <summary> File path for the shadow hit shader, traces from scene to light, this detects the hard shadows</summary>
const char* const ShadowHitShader = "ShadowHit.rchit.spv";

/// <summary>The fragment shader that copies the finished raytraced image to the swapchain </summary>
const char* const DeferredShadingFragmentShader = "OnScreenFragmentShader.fsh.spv";

/// <summary> Draws a hardcoded triangle that coveres the entire swapchain imaage, so that the entire image is copied</summary>
const char* const FullscreenTriangleVertexShader = "FullscreenTriangleVertexShader.vsh.spv";
} // namespace Files

/// <summary>buffer entry names used for the structured memory views used throughout the demo. These entry names must match the variable
/// names used in the demo shaders.</summary>
namespace BufferEntryNames {

/// <summary> The Uniform buffer object that represents all the details required of the camera </summary>
namespace CameraUbo {
/// <summary> The name of the variable for the inverted view matrix for the primary camera in the shaders </summary>
const char* const InverseViewMatrix = "mInvViewMatrix";

/// <summary> The name if the variable for the inverted projection matrix of the primary camera in the shaders</summary>
const char* const InverseProjectionMatrix = "mInvProjectionMatrix";
} // namespace CameraUbo

/// <summary>The Uniform buffer object that represents the required details of the point light </summary>
namespace PointLightData {
/// <summary> The name for the variable in the shader that stores the color of the light</summary>
const char* const LightColor = "vLightColor";

/// <summary> The name for the variable in the shader that stores the xyz position of the light</summary>
const char* const LightPosition = "vLightPosition";

/// <summary> The name for the variable in the shader that stores how strong the light is</summary>
const char* const LightIntensity = "fLightIntensity";
} // namespace PointLightData

} // namespace BufferEntryNames

/// <summary> Store all the Vulkan resources in one struct so that they are easier to keep track of and release </summary>
struct DeviceResources
{
	/// <summary>Encapsulation of a Vulkan instance.</summary>
	pvrvk::Instance instance;

	/// <summary>Encapsulation of a Vulkan logical device.</summary>
	pvrvk::Device device;

	/// <summary>Callbacks and messengers for debug messages from the validation layers.</summary>
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;

	/// <summary>Queue where to submit commands.</summary>
	pvrvk::Queue queue;

	/// <summary>Encapsulation of a Vulkan swapchain</summary>
	pvrvk::Swapchain swapchain;

	/// <summary>Allocator to manage memory resources on the device and facilitate memory pools and defragmentation</summary>
	pvr::utils::vma::Allocator vmaAllocator;

	/// <summary>A Vulkan command pool to allocate command buffers from</summary>
	pvrvk::CommandPool commandPool;

	/// <summary>A Vulkan descriptor pool to allocate descriptor sets from </summary>
	pvrvk::DescriptorPool descriptorPool;

	/// <summary>Frame buffers created that old the image presented to the screen, one per swapchain element</summary>
	std::vector<pvrvk::Framebuffer> onScreenFramebuffer;

	/// <summary>Image view for the raytraced image, raygen shader writes to this image</summary>
	pvrvk::ImageView raytracedImage;

	/// <summary>An array of primary command buffers that are submitted to the device, one per swapchain image</summary>
	pvrvk::CommandBuffer primaryCmdBuffers[MAX_NUMBER_OF_SWAP_IMAGES];

	/// <summary>An array of secondary command buffers for writing the raytraced image commands, one per swapchain image</summary>
	pvrvk::SecondaryCommandBuffer raytracedCmdBuffers[MAX_NUMBER_OF_SWAP_IMAGES];

	/// <summary>An array of secondart command buffers for writing the copy to onscreen commands, on per swapchain image</summary>
	pvrvk::SecondaryCommandBuffer onScreenCmdBuffers[MAX_NUMBER_OF_SWAP_IMAGES];

	/// <summary>Descriptor set layout for the resources that change once a frame, such as the camera and light position</summary>
	pvrvk::DescriptorSetLayout perFrameDescriptorSetLayout;

	/// <summary>Descriptor set layout for writing the results of raytracing to an image </summary>
	pvrvk::DescriptorSetLayout raytracedImageStoreDescriptorSetLayout;

	/// <summary>Descriptor set layout for reading the finished raytraced image in the final fragment shader</summary>
	pvrvk::DescriptorSetLayout raytracedImageSamplerDescriptorSetLayout;

	/// <summary>Descriptor set layout for the resources that would usually change per model, but in a raytracing scene are bindless, such
	/// as the vertex and index buffers, materials, instance transforms, along with the top level acceleration structure so that that rays
	/// can traverse these resources</summary>
	pvrvk::DescriptorSetLayout bindlessResourcesDescriptorSetLayout;

	/// <summary>Descriptor set for the per frame resources templated from the perFrameDescriptorSetLayout</summary>
	pvrvk::DescriptorSet perFrameDescriptorSet;

	/// <summary>Descriptor set for storing the raytraced image, templated from raytracedImageStoreDescriptorSetLayout</summary>
	pvrvk::DescriptorSet raytracedImageStoreDescriptorSet;

	/// <summary>Descriptor set for reading from the raytraced image, templated from raytracedImageSamplerDescriptorSetLayout</summary>
	pvrvk::DescriptorSet raytracedImageSamplerDescriptorSet;

	/// <summary>Descriptor set for the bindless resources, templated from bindlessResourcesDescriptorSetLayout</summary>
	pvrvk::DescriptorSet bindlessResourcesDescriptorSet;

	/// <summary>Pipline layout for the graphics pipeline that copies the raytraced image to the swapchain </summary>
	pvrvk::PipelineLayout onScreenPipelineLayout;

	/// <summary>Graphics pipeline that copies the raytraced image to the swapchain </summary>
	pvrvk::GraphicsPipeline onScreenPipeline;

	/// <summary>Cache for the graphics pipeline </summary>
	pvrvk::PipelineCache pipelineCache;

	/// <summary> Piplein layout for the RT pipeline, associates the descriptor sets to a descriptor set index</summary>
	pvrvk::PipelineLayout raytracePipelineLayout;

	/// <summary> Raytracing pipeline, used in the offscreen raytracing </summary>
	pvrvk::RaytracingPipeline raytracePipeline;

	/// <summary>GPU buffer where to store the shader binding table.</summary>
	pvrvk::Buffer raytraceShaderBindingTable;

	/// <summary> Pvrvk wrapper for the acceleration structure, both the top level and bottom level acceleration structures</summary>
	pvr::utils::AccelerationStructureWrapper accelerationStructure;

	/// <summary>This buffer will contain the vertex data for the geometry to be ray traced </summary>
	std::vector<pvrvk::Buffer> vertexBuffers;

	/// <summary>This buffer will contain the indices of the geometry to be ray traced</summary>
	std::vector<pvrvk::Buffer> indexBuffers;

	/// <summary>This buffer will contain all the materials information. In this sample, it only has the base colour</summary>
	pvrvk::Buffer materialBuffer;

	/// <summary>This buffer stores the transforms from model space to world space </summary>
	pvrvk::Buffer instanceTransformBuffer;

	/// <summary>  The size of each of the vertex buffers in bindless resources for building the acceleration structures</summary>
	std::vector<int> verticesSize;

	/// <summary> The size of each of the index buffers in bindless resources for building the acceleration structures</summary>
	std::vector<int> indicesSize;

	/// <summary> Buffer view so that the camera buffer can be written to</summary>
	pvr::utils::StructuredBufferView CameraBufferView;

	/// <summary>Buffer to store the camera uniform buffer </summary>
	pvrvk::Buffer CameraBuffer;

	/// <summary>Buffer view for the buffer so that it can be written to </summary>
	pvr::utils::StructuredBufferView lightDataBufferView;

	/// <summary>Buffer for the light data uniform buffer </summary>
	pvrvk::Buffer lightDataBuffer;

	/// <summary>Semaphores for when acquiring the next image from the swap chain, one per swapchain image.</summary>
	std::vector<pvrvk::Semaphore> imageAcquiredSemaphores;

	/// <summary>Semaphores for when submitting the command buffer for the current swapchain image.</summary>
	std::vector<pvrvk::Semaphore> presentationSemaphores;

	/// <summary>Fences for each of the per-frame command buffers, one per swapchain image.</summary>
	std::vector<pvrvk::Fence> perFrameResourcesFences;

	/// <summary>The pvrvk wrapper for the UI renderer to display the text </summary>
	pvr::ui::UIRenderer uiRenderer;

	/// <summary> Default destructor for device resources</summary>
	~DeviceResources()
	{
		if (device)
		{
			device->waitIdle();
			for (auto fence : perFrameResourcesFences)
			{
				if (fence) fence->wait();
			}
		}
	}
};

/// <summary>Class implementing the Shell functions.</summary>
class VulkanRayTracedHardShadows : public pvr::Shell
{
	/// <summary>Put all the API resources into one pointer for easier releasing </summary>
	std::unique_ptr<DeviceResources> _deviceResources;

	/// <summary>The index in the swapchain, so the right per frame resources is used </summary>
	uint32_t _swapchainIndex = 0;

	/// <summary> Number of images in the swapchain so the swap index can be moduloed </summary>
	uint32_t _numSwapImages = 0;

	/// <summary>Ray Tracing properties struct holding important information like the size of a sahder group for the Shader Binding Table.</summary>
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR _rtProperties{};

	/// <summary> How many shader groups are we using</summary>
	uint32_t _shaderGroupCount = 0;

	/// <summary>The memory alignment of shader groups within the shader binding table, calculated from the rtProperties </summary>
	uint32_t _shaderGroupHandleSizeAligned = 0;

	/// <summary>Which index in the swapchain is the current frame on </summary>
	uint32_t _frameId = 0;

	/// <summary> How many milliseconds into the POD animation is the current frame on </summary>
	float _frameNumber = 0;

	/// <summary>Is the scene currently animated, should the models have their transfors updated </summary>
	bool _animateScene = true;

	/// <summary> Is the camera path currently animated, should the camera ubo be updated</summary>
	bool _animateCamera = false;

	/// <summary> The view matrix for the camera current position</summary>
	glm::mat4 _viewMatrix = glm::mat4(0);

	/// <summary> The projection matrix for the camera</summary>
	glm::mat4 _projectionMatrix = glm::mat4(0);

	/// <summary> The inverted view matrix for the current camera position </summary>
	glm::mat4 _inverseViewMatrix = glm::mat4(0);

	/// <summary> The inverted projection matrix for the camera</summary>
	glm::mat4 _viewProjectionMatrix = glm::mat4(0);

	/// <summary> The current camera position</summary>
	glm::vec3 _cameraPosition = glm::vec3(0);

	/// <summary> A vector of matrices which transforms a mesh node from model space to world space, indexed by mesh node ID</summary>
	std::vector<glm::mat4x4> _instanceTransforms;

	/// <summary> Width of the window, including the frame</summary>
	uint32_t _windowWidth = 0;

	/// <summary> Height of the window includingg the window frame</summary>
	uint32_t _windowHeight = 0;

	/// <summary> Width of the on screen framebuffer</summary>
	uint32_t _framebufferWidth = 0;

	/// <summary> Height of the on screen framebuffer</summary>
	uint32_t _framebufferHeight = 0;

	/// <summary> View port transforms</summary>
	int32_t _viewportOffsets[2];

	/// <summary> The pvr assets handle for the scene</summary>
	pvr::assets::ModelHandle _scene;

	/// <summary>Filter performance warning UNASSIGNED-BestPractices-vkAllocateMemory-small-allocation Best Practices which
	/// has ID -602362517 for TLAS buffer build and update. This warning recommends buffer allocations to be of size at least
	/// 256KB which collides with each BLAS node built for each scene element and the size of the TLAS buffer, details of the warning:
	/// https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/master/layers/best_practices_validation.h</summary>
	std::vector<int> vectorValidationIDFilter;
	
	/// <summary>Number of ray generation shaders used</summary>
	uint32_t _numberRayGenShaders;

	/// <summary>Number of ray miss shaders used</summary>
	uint32_t _numberRayMissShaders;

	/// <summary>Number of ray hit shaders used</summary>
	uint32_t _numberRayHitShaders;

	/// <summary>Queried value of the member of VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment</summary>
	uint32_t _shaderGroupBaseAlignment;

	/// <summary>Queried value of the member of VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleAlignment</summary>
	uint32_t _shaderGroupHandleAlignment;

	/// <summary>Size in bytes of the ray generation shader group in the shader binding table buffer</summary>
	uint32_t _sizeRayGenGroup;

	/// <summary>Size in bytes of the ray miss shader group in the shader binding table buffer</summary>
	uint32_t _sizeRayMissGroup;

	/// <summary>Size in bytes of the ray hit shader group in the shader binding table buffer</summary>
	uint32_t _sizeRayHitGroup;

public:
	/// <summary> Code in initApplication() will be called by pvr::Shell once per run, before the rendering context is created. Used to
	/// initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
	/// If the rendering context is lost, initApplication() will not be called again.</summary>
	/// <returns> Return true if no error occurred. </returns>
	virtual pvr::Result initApplication();

	/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context. Used to
	/// initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.).</summary>
	/// <returns>Return Result::Success if no error occurred.</returns>
	virtual pvr::Result initView();

	/// <summary>Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.</summary>
	/// <returns>Return Result::Success if no error occurred.</returns>
	virtual pvr::Result releaseView();

	/// <summary>Code in quitApplication() will be called by PVRShell once per run, just before exiting the program. If the rendering
	/// context is lost, quitApplication() will not be called.</summary>
	/// <returns>Return Result::Success if no error occurred.</returns>
	virtual pvr::Result quitApplication();

	/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
	/// <returns>Return Result::Success if no error occurred</returns>
	virtual pvr::Result renderFrame();

	/// <summary> Creates a device and queues with the required raytracing extensions enabled </summary>
	/// <returns> A pair with the left element being the selected physical device, the right element being the surface created from it</returns>
	std::pair<pvrvk::PhysicalDevice, pvrvk::Surface> createRaytracingEnabledDevice();

	/// <summary>Tests if the physical device has all the requested physical device extensions</summary>
	/// <param name="physicalDevice">The physical device we're validating the device extension support for</param>
	/// <param name="requestedExtensions">The physical device extensions that we are going to request</param>
	/// <returns>True if the device supports all of the requested extensions, false otherwise</returns>
	bool validatePhysicalDeviceFeatures(pvrvk::PhysicalDevice physicalDevice, const std::vector<std::string>& requestedExtensionNames);

	/// <summary>Loads the mesh data required for this example into vertex and index buffer objects and populates material data.</summary>
	/// <param name="uploadCmd">Command Buffer used to record the buffer upload commands.</param>
	void createModelBuffers(pvrvk::CommandBuffer& uploadCmd);

	/// <summary>Creates the scene wide buffer used throughout the demo.</summary>
	void createCameraBuffer();

	/// <summary>Creates the Light data buffer, which remains static throughout the demo</summary>
	void createLightBuffer();

	/// <summary>Create the raytraced image and view </summary>
	void createRayTracedImage();

	/// <summary>Updates the camera position using a rotation matrix and updates the camera ubo</summary>
	void updateCameraAnimation();

	/// <summary>Updates the instance transforms for the scene elements and then rebuilds the top level acceleration structure </summary>
	void updateSceneAnimation();

	/// <summary>Creates descriptor set layouts.</summary>
	void createDescriptorSetLayouts();

	/// <summary>Creates descriptor sets.</summary>
	void createDescriptorSets();

	/// <summary>Create the pipelines for this example.</summary>
	void createPipelines();

	/// <summary>Creates the pipeline for the fully ray traced scene, including the primary rays and shadow testing rays</summary>
	void createRayTracingPipelines();

	/// <summary>Creates the pipeline for copying the raytraced image to the onscreen framebuffer</summary>
	void createOnScreenPipeline();

	/// <summary>Computes a common multiple of a and b parameters.</summary>
	/// <param name="a">One of the two values to make a common multiple.</param>
	/// <param name="b">One of the two values to make a common multiple.</param>
	/// <returns>Value which is a common multiple of a and b parameters.</returns>
	uint32_t makeMultipleOf(uint32_t a, uint32_t b);

	/// <summary>Creates the shader binding table for the Ray-Traced shadows pass. This is used to know which shader to call depending on
	/// which event happens to the ray as it traces the acceleration structure. The sbt also associates an offset to each shader group so
	///  that the traceRaysExt call in the shaders can call different hit and miss groups, ie tracing primary rays from the raygen and then
	///  tracing the shadow rays from the primary hit shader  </summary>
	void createShaderBindingTable();

	/// <summary>Records main command buffer.</summary>
	void recordMainCommandBuffer();

	/// <summary>Record all the secondary command buffers.</summary>
	void recordSecondaryCommandBuffers();

	/// <summary>Record deferred shading commands.</summary>
	/// <param name="cmdBuffers">SecondaryCommandbuffer to record.</param>
	/// <param name="swapchainIndex">Index of the current swapchain image.</param>
	void recordCommandBufferDeferredShading(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex);

	/// <summary>Record ray-tracing commands.</summary>
	/// <param name="cmdBuffers">SecondaryCommandbuffer to record.</param>
	/// <param name="swapchainIndex">Index of the current swapchain image.</param>
	void recordCommandBufferRaytraces(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex);

	/// <summary>Record UIRenderer commands.</summary>
	/// <param name="commandBuff">Commandbuffer to record.</param>
	void recordCommandUIRenderer(pvrvk::SecondaryCommandBuffer& cmdBuffers);

	/// <summary> Handle basic user input via pvr shell</summary>
	/// <param name="key">Which key was pressed</param>
	void eventMappedInput(pvr::SimplifiedInput key)
	{
		switch (key)
		{
		// Handle input
		case pvr::SimplifiedInput::ActionClose: exitShell(); break;
		case pvr::SimplifiedInput::Action1: _animateCamera = !_animateCamera; break;
		case pvr::SimplifiedInput::Action2: _animateScene = !_animateScene; break;
		default: break;
		}
	}
};

/// <summary>This function must be implemented by the user of the shell. The user should return its Shell object defining the
/// behaviour of the application.</summary>
/// <returns>Return an unique_ptr to a new Demo class,supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanRayTracedHardShadows>(); }

pvr::Result VulkanRayTracedHardShadows::initApplication()
{
	_frameNumber = 0.0f;
	_frameId = 0;

	//  Load the scene
	_scene = pvr::assets::loadModel(*this, Files::SceneFile);

	return pvr::Result::Success;
}

pvr::Result VulkanRayTracedHardShadows::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();

	// Create instance and targetting Vulkan version 1.1 and retrieve compatible physical devices
	pvr::utils::VulkanVersion vulkanVersion(1, 1, 0);
	_deviceResources->instance = pvr::utils::createInstance(this->getApplicationName(), vulkanVersion, pvr::utils::InstanceExtensions(vulkanVersion));

	if (_deviceResources->instance->getNumPhysicalDevices() == 0)
	{
		setExitMessage("Unable not find a compatible Vulkan physical device.");
		return pvr::Result::UnknownError;
	}

	// Filter UNASSIGNED-BestPractices-vkAllocateMemory-small-allocation Best Practices performance warning which has ID -602362517 for TLAS buffer build and
	// update (VkBufferDeviceAddressInfo requires VkBuffer handle so in general it's not possible to make a single buffer to put all information
	// and use offsets inside it
	vectorValidationIDFilter.push_back(-602362517);

	// Create a default set of debug utils messengers or debug callbacks using either VK_EXT_debug_utils or VK_EXT_debug_report respectively
	_deviceResources->debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(_deviceResources->instance, (void*)&vectorValidationIDFilter);

	// Create a Vulkan enabled device with the right queues and extensions to be raytracing enabled.
	std::pair<pvrvk::PhysicalDevice, pvrvk::Surface> deviceSurfacePair = createRaytracingEnabledDevice();
	pvrvk::PhysicalDevice physicalDevice = deviceSurfacePair.first;
	pvrvk::Surface surface = deviceSurfacePair.second;

	// Create vulkan memory allocator
	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	// Validate the supported swapchain image usage. Try to add in screenshot support
	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice->getSurfaceCapabilities(surface);
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT;
	}

	// We do not support automatic MSAA for this demo.
	if (getDisplayAttributes().aaSamples > 1)
	{
		Log(LogLevel::Warning, "Full Screen Multisample Antialiasing requested, but not supported for this demo's configuration.");
		getDisplayAttributes().aaSamples = 1;
	}

	// Create the Swapchain
	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters(true).setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage));

	_deviceResources->swapchain = swapChainCreateOutput.swapchain;
	_deviceResources->onScreenFramebuffer = swapChainCreateOutput.framebuffer;

	// Get the number of swap images
	_numSwapImages = _deviceResources->swapchain->getSwapchainLength();

	_deviceResources->imageAcquiredSemaphores.resize(_numSwapImages);
	_deviceResources->presentationSemaphores.resize(_numSwapImages);
	_deviceResources->perFrameResourcesFences.resize(_numSwapImages);

	// Get current swap index
	_swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	// Calculate the frame buffer width and heights
	_framebufferWidth = _windowWidth = this->getWidth();
	_framebufferHeight = _windowHeight = this->getHeight();

	// Allow the user to change the framebuffer size via a command line
	const pvr::CommandLine& commandOptions = getCommandLine();
	int32_t intFramebufferWidth = -1;
	int32_t intFramebufferHeight = -1;
	commandOptions.getIntOption("-fbowidth", intFramebufferWidth);
	_framebufferWidth = static_cast<uint32_t>(intFramebufferWidth);
	_framebufferWidth = static_cast<uint32_t>(std::min(_framebufferWidth, _windowWidth));
	commandOptions.getIntOption("-fboheight", intFramebufferHeight);
	_framebufferHeight = static_cast<uint32_t>(intFramebufferHeight);
	_framebufferHeight = static_cast<uint32_t>(std::min(_framebufferHeight, _windowHeight));

	_viewportOffsets[0] = (_windowWidth - _framebufferWidth) / 2;
	_viewportOffsets[1] = (_windowHeight - _framebufferHeight) / 2;

	Log(LogLevel::Information, "Framebuffer dimensions: %d x %d\n", _framebufferWidth, _framebufferHeight);
	Log(LogLevel::Information, "On-screen Framebuffer dimensions: %d x %d\n", _windowWidth, _windowHeight);

	// Create a descriptor pool with enough space for this demo
	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, static_cast<uint16_t>(16 * _numSwapImages))
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, static_cast<uint16_t>(16 * _numSwapImages))
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, static_cast<uint16_t>(16 * _numSwapImages))
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_INPUT_ATTACHMENT, static_cast<uint16_t>(16 * _numSwapImages))
																						  .setMaxDescriptorSets(static_cast<uint16_t>(16 * _numSwapImages)));

	_deviceResources->descriptorPool->setObjectName("DescriptorPool");

	// Allocate the command buffers out of the command pool
	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		_deviceResources->primaryCmdBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->onScreenCmdBuffers[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->raytracedCmdBuffers[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();

		_deviceResources->primaryCmdBuffers[i]->setObjectName("MainCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->onScreenCmdBuffers[i]->setObjectName("OnScreenSecondaryCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->raytracedCmdBuffers[i]->setObjectName("RaytracedSecondaryCommandBufferSwapchain" + std::to_string(i));

		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->presentationSemaphores[i]->setObjectName("PresentationSemaphoreSwapchain" + std::to_string(i));
		_deviceResources->imageAcquiredSemaphores[i]->setObjectName("ImageAcquiredSemaphoreSwapchain" + std::to_string(i));

		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->perFrameResourcesFences[i]->setObjectName("FenceSwapchain" + std::to_string(i));
	}

	// Handle device rotation
	bool isRotated = this->isScreenRotated();
	if (isRotated)
	{
		_projectionMatrix = pvr::math::perspective(pvr::Api::Vulkan, _scene->getCamera(0).getFOV(), static_cast<float>(this->getHeight()) / static_cast<float>(this->getWidth()),
			_scene->getCamera(0).getNear(), _scene->getCamera(0).getFar(), glm::pi<float>() * .5f);
	}
	else
	{
		_projectionMatrix = pvr::math::perspective(pvr::Api::Vulkan, _scene->getCamera(0).getFOV(), static_cast<float>(this->getWidth()) / static_cast<float>(this->getHeight()),
			_scene->getCamera(0).getNear(), _scene->getCamera(0).getFar());
	}

	// Initialize UIRenderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queue);
	_deviceResources->uiRenderer.getDefaultTitle()->setText("VulkanRayTracedHardShadows");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText("Action 1: Toggle Camera \nAction 2: Toggle Animation");
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	// Upload the mesh data to the GPU
	_deviceResources->primaryCmdBuffers[0]->begin();
	createModelBuffers(_deviceResources->primaryCmdBuffers[0]);
	_deviceResources->primaryCmdBuffers[0]->end();

	// Submit the upload command buffer
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->primaryCmdBuffers[0];
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle(); // wait

	// Create and build one top level acceleration structure representing the scene, with one bottom level acceleration structure per mesh
	_deviceResources->accelerationStructure.buildASModelDescription(
		_deviceResources->vertexBuffers, _deviceResources->indexBuffers, _deviceResources->verticesSize, _deviceResources->indicesSize, _instanceTransforms);
	_deviceResources->accelerationStructure.buildAS(_deviceResources->device, _deviceResources->queue, _deviceResources->primaryCmdBuffers[0],
		pvrvk::BuildAccelerationStructureFlagsKHR::e_PREFER_FAST_TRACE_BIT_KHR | pvrvk::BuildAccelerationStructureFlagsKHR::e_ALLOW_UPDATE_BIT_KHR);

	createRayTracedImage();
	createLightBuffer();
	createCameraBuffer();
	createDescriptorSetLayouts();
	createPipelines();
	createShaderBindingTable();
	createDescriptorSets();
	recordSecondaryCommandBuffers();
	recordMainCommandBuffer();

	return pvr::Result::Success;
}

pvr::Result VulkanRayTracedHardShadows::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

pvr::Result VulkanRayTracedHardShadows::quitApplication()
{
	_scene.reset();
	return pvr::Result::Success;
}

pvr::Result VulkanRayTracedHardShadows::renderFrame()
{
	// Aquire the next frame
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);
	_swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();
	_deviceResources->perFrameResourcesFences[_swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[_swapchainIndex]->reset();

	// Update the animation and the camera
	updateSceneAnimation();
	updateCameraAnimation();

	// submit the main command buffer
	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags pipeWaitStage = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;

	submitInfo.commandBuffers = &_deviceResources->primaryCmdBuffers[_swapchainIndex];

	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->imageAcquiredSemaphores[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.waitDstStageMask = &pipeWaitStage;
	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[_swapchainIndex]);

	// Take a screenshot using pvr::shell
	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->queue, _deviceResources->commandPool, _deviceResources->swapchain, _swapchainIndex, this->getScreenshotFileName(),
			_deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	// Present frame
	pvrvk::PresentInfo presentInfo;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.imageIndices = &_swapchainIndex;
	_deviceResources->queue->present(presentInfo);

	// Move the frame index forward to the next image in the swapchain
	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();

	return pvr::Result::Success;
}

std::pair<pvrvk::PhysicalDevice, pvrvk::Surface> VulkanRayTracedHardShadows::createRaytracingEnabledDevice()
{
	// The list of required raytracing extension names, select the first device with these extensions supported
	// Note that the extensions needed for this Ray Tracing sample are:
	//
	// VK_KHR_RAY_TRACING_EXTENSION_NAME:              Allows the use of all the Vulkan API calls from the Ray Tracing extension.
	//
	// VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME:    Allows to obtain the address of a GPU buffer (device) through the call to vkGetBufferDeviceAddress,
	//                                                 needed for many of the operations to setup bottom and top level acceleration structures.
	//
	// VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME:      Modifies the alignment rules for uniform buffers, storage buffers and push constants, allowing non-scalar
	//                                                 types to be aligned solely based on the size of their components, without additional requirements.
	//
	// VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME:      Allows to bind all textures at once as an unsized array, and later in the shader to index into any
	//                                                 of those textures. This is due to the fact that, when a ray hits a triangle, we don't knw oin advance what
	//                                                 textures will be assigned to the material assigned to tha triangle, meaning any ray could access any
	//                                                 texture in a single ray trace pass.
	//
	// VK_KHR_MAINTENANCE3_EXTENSION_NAME:             Adds detail to the limits of some functionalities, like the maximum number of descriptors supported in a single
	//                                                 descritpr set layout (some implementations only have a limit for the total size of descriptors). Also adds a
	//                                                 limit to the maximum size of a memory allocation, being this sometimes limited by the kernel in some platforms.
	//
	// VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME:         Allows a special pipeline that defines shaders / shader groups that can be linked into other pipelines
	//                                                 (a "pipeline library" is a special pipeline that cannot be bound, instead it defines a set of shaders and
	//                                                 shader groups which can be linked into other pipelines.)
	//
	// VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME: Defines the infrastructure and usage patterns for deferrable commands, but does not specify
	//                                                 any commands as deferrable. This is left to additional dependant extensions (more information in
	//                                                 https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#deferred-host-operations-requesting)
	std::vector<std::string> raytracingExtensionNames{ VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME, VK_KHR_SPIRV_1_4_EXTENSION_NAME, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
		VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME, VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME };

	// Find the first device with raytracing extensions supported
	bool foundCompatableDevice = false;
	pvrvk::PhysicalDevice physicalDevice;
	for (uint32_t i = 0; i < _deviceResources->instance->getNumPhysicalDevices(); i++)
	{
		physicalDevice = _deviceResources->instance->getPhysicalDevice(i);
		if (validatePhysicalDeviceFeatures(physicalDevice, raytracingExtensionNames))
		{
			foundCompatableDevice = true;
			break;
		}
	}
	if (!foundCompatableDevice) { throw pvrvk::ErrorInitializationFailed("Could not find a physical device with the extensions required for raytracing!"); }

	// Found a compatable device, add the raytracing extensions to the default list of requested extensions
	pvr::utils::DeviceExtensions deviceExtensions = pvr::utils::DeviceExtensions();
	for (auto raytracingExtension : raytracingExtensionNames) { deviceExtensions.addExtension(raytracingExtension); }

	// Get the physical device features for all of the raytracing extensions through a continual pNext chain
	VkPhysicalDeviceFeatures2 deviceFeatures{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_FEATURES_2) };

	// Raytracing Pipeline Features
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracingPipelineFeatures{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR) };
	deviceFeatures.pNext = &raytracingPipelineFeatures;

	// Acceleration Structure Features
	VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{ static_cast<VkStructureType>(
		pvrvk::StructureType::e_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR) };
	raytracingPipelineFeatures.pNext = &accelerationStructureFeatures;

	// Device Address Features
	VkPhysicalDeviceBufferDeviceAddressFeatures deviceBufferAddressFeatures{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES) };
	accelerationStructureFeatures.pNext = &deviceBufferAddressFeatures;

	// Scalar Block Layout Features
	VkPhysicalDeviceScalarBlockLayoutFeaturesEXT scalarFeatures{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES) };
	deviceBufferAddressFeatures.pNext = &scalarFeatures;

	// Descriptor Indexing Features
	VkPhysicalDeviceDescriptorIndexingFeatures indexFeatures{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES) };
	scalarFeatures.pNext = &indexFeatures;

	// Fill in all of these device features with one call
	physicalDevice->getInstance()->getVkBindings().vkGetPhysicalDeviceFeatures2(physicalDevice->getVkHandle(), &deviceFeatures);

	// Add these device features to the physical device, since they're all connected by a pNext chain, we only need to explicitly attach the top feature
	deviceExtensions.addExtensionFeatureVk<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(&raytracingPipelineFeatures);

	// Create the physical device, queues and surface using the required extensions and extension features
	// Create the surface
	pvrvk::Surface surface = pvr::utils::createSurface(_deviceResources->instance, physicalDevice, this->getWindow(), this->getDisplay(), this->getConnection());

	// Create device and queues
	const pvr::utils::QueuePopulateInfo queuePopulateInfo = { pvrvk::QueueFlags::e_GRAPHICS_BIT, surface };
	pvr::utils::QueueAccessInfo queueAccessInfo;
	_deviceResources->device = pvr::utils::createDeviceAndQueues(physicalDevice, &queuePopulateInfo, 1, &queueAccessInfo, deviceExtensions);

	// Get queue
	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);
	_deviceResources->queue->setObjectName("GraphicsQueue");

	// Create the command pool
	_deviceResources->commandPool =
		_deviceResources->device->createCommandPool(pvrvk::CommandPoolCreateInfo(queueAccessInfo.familyId, pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	// Get ray tracing device properties
	_rtProperties.sType = static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR);
	_rtProperties.pNext = nullptr;
	VkPhysicalDeviceProperties2 properties{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_PROPERTIES_2) };
	properties.pNext = &_rtProperties;
	_deviceResources->instance->getVkBindings().vkGetPhysicalDeviceProperties2(physicalDevice->getVkHandle(), &properties);
	_shaderGroupBaseAlignment = _rtProperties.shaderGroupBaseAlignment;
	_shaderGroupHandleAlignment = _rtProperties.shaderGroupHandleAlignment;

	Log(LogLevel::Information, "Physical device selected was : %s", physicalDevice->getProperties().getDeviceName());

	// Return the device and surface to the init view function
	return std::pair<pvrvk::PhysicalDevice, pvrvk::Surface>(physicalDevice, surface);
}

bool VulkanRayTracedHardShadows::validatePhysicalDeviceFeatures(pvrvk::PhysicalDevice physicalDevice, const std::vector<std::string>& requestedExtensionNames)
{
	// Get the full list of extensions supported by the current physical device
	std::vector<pvrvk::ExtensionProperties> supportedExtensions = physicalDevice->getDeviceExtensionsProperties();

	// For each of the requested extensions, check that its name is contained within the list of extensions supported by the device
	for (std::string requested : requestedExtensionNames)
	{
		bool found =
			std::any_of(supportedExtensions.begin(), supportedExtensions.end(), [&requested = requested](auto& supported) { return requested == supported.getExtensionName(); });

		// At this point the requested extension name has been compared against all of the supported extensions, if it hasn't been found
		// the device doesn't support all the extensions we need for raytracing, log and exit
		if (!found)
		{
			Log("Physical Device : %s Failed to find the extension : %s ", physicalDevice->getProperties().getDeviceName(), requested.c_str());
			return false;
		}
	}
	// Got through all of the extensions without exiting, therefore all of the requested extensions have been found
	return true;
}

void VulkanRayTracedHardShadows::createDescriptorSetLayouts()
{
	// Per Frame Descriptor Set Layout
	pvrvk::DescriptorSetLayoutCreateInfo perFrameDescSetInfo;
	// Camera buffer
	perFrameDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR);
	// Point light buffer
	perFrameDescSetInfo.setBinding(1, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR);
	// Create the layout
	_deviceResources->perFrameDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(perFrameDescSetInfo);

	// Storing the result of the raytracing to an image layout
	pvrvk::DescriptorSetLayoutCreateInfo imageDescSetInfo;
	// Raytraced image store
	imageDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_STORAGE_IMAGE, 1u, pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR);
	// Create the layout
	_deviceResources->raytracedImageStoreDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(imageDescSetInfo);

	// Sampling the raytraced image layout
	pvrvk::DescriptorSetLayoutCreateInfo defferedShadingDescSetInfo;
	defferedShadingDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	// Create the layout
	_deviceResources->raytracedImageSamplerDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(defferedShadingDescSetInfo);

	// Bindless Resources Descriptor Set Layout
	pvrvk::DescriptorSetLayoutCreateInfo bindlessDescSetInfo;
	// Top level Accelleration structure
	bindlessDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_ACCELERATION_STRUCTURE_KHR, 1u, pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR);
	// Vertex Buffer
	bindlessDescSetInfo.setBinding(
		1, pvrvk::DescriptorType::e_STORAGE_BUFFER, static_cast<uint16_t>(_deviceResources->vertexBuffers.size()), pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR);
	// Index Buffer
	bindlessDescSetInfo.setBinding(
		2, pvrvk::DescriptorType::e_STORAGE_BUFFER, static_cast<uint16_t>(_deviceResources->indexBuffers.size()), pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR);
	// Instance Transform Buffer
	bindlessDescSetInfo.setBinding(3, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR);
	// Material Buffer
	bindlessDescSetInfo.setBinding(4, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR);
	// Create the layout
	_deviceResources->bindlessResourcesDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(bindlessDescSetInfo);
}

void VulkanRayTracedHardShadows::createDescriptorSets()
{
	// Scene Samplers
	pvrvk::SamplerCreateInfo samplerDesc;
	samplerDesc.wrapModeU = samplerDesc.wrapModeV = samplerDesc.wrapModeW = pvrvk::SamplerAddressMode::e_REPEAT;

	samplerDesc.minFilter = pvrvk::Filter::e_LINEAR;
	samplerDesc.magFilter = pvrvk::Filter::e_LINEAR;
	samplerDesc.mipMapMode = pvrvk::SamplerMipmapMode::e_LINEAR;
	pvrvk::Sampler samplerTrilinear = _deviceResources->device->createSampler(samplerDesc);

	// Allocate Descriptor Sets
	_deviceResources->perFrameDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->perFrameDescriptorSetLayout);
	_deviceResources->raytracedImageStoreDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->raytracedImageStoreDescriptorSetLayout);
	_deviceResources->raytracedImageSamplerDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->raytracedImageSamplerDescriptorSetLayout);
	_deviceResources->bindlessResourcesDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->bindlessResourcesDescriptorSetLayout);

	_deviceResources->perFrameDescriptorSet->setObjectName("PerFrameDescriptorSet");
	_deviceResources->raytracedImageStoreDescriptorSet->setObjectName("RaytracedImageStoreDescriptorSet");
	_deviceResources->raytracedImageSamplerDescriptorSet->setObjectName("RaytracedImageSamplerDescriptorSet");
	_deviceResources->bindlessResourcesDescriptorSet->setObjectName("BindlessResourcesDescriptorSet");

	// A vector to update all the descriptor sets in one go
	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

	// Write Per Frame Descriptor Set
	// Camera UBO
	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->perFrameDescriptorSet, 0)
								.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->CameraBuffer, 0, _deviceResources->CameraBufferView.getDynamicSliceSize())));
	// Light UBO
	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->perFrameDescriptorSet, 1)
								.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->lightDataBuffer, 0, _deviceResources->lightDataBufferView.getDynamicSliceSize())));

	// Write RT Image Store Descriptor Set
	// Image store
	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_IMAGE, _deviceResources->raytracedImageStoreDescriptorSet, 0)
								.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->raytracedImage, pvrvk::ImageLayout::e_GENERAL)));

	// Write RT Image Sample Descriptor Set
	// Image sampler of the RT store image
	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->raytracedImageSamplerDescriptorSet, 0)
								.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->raytracedImage, samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

	// Write the bindless resources descriptor set
	// TLAS
	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_ACCELERATION_STRUCTURE_KHR, _deviceResources->bindlessResourcesDescriptorSet, 0)
								.setAccelerationStructureInfo(0, _deviceResources->accelerationStructure.getTopLevelAccelerationStructure()));
	// Instance Transform Buffers
	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->bindlessResourcesDescriptorSet, 3)
								.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->instanceTransformBuffer, 0, VK_WHOLE_SIZE)));
	// Material Buffers
	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->bindlessResourcesDescriptorSet, 4)
								.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->materialBuffer, 0, VK_WHOLE_SIZE)));
	// Vertex and Index buffers per mesh
	pvrvk::WriteDescriptorSet vertexWriter = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->bindlessResourcesDescriptorSet, 1);
	pvrvk::WriteDescriptorSet indexWriter = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->bindlessResourcesDescriptorSet, 2);
	for (uint32_t i = 0; i < _deviceResources->vertexBuffers.size(); i++)
	{
		vertexWriter.setBufferInfo(i, pvrvk::DescriptorBufferInfo(_deviceResources->vertexBuffers[i], 0, _deviceResources->vertexBuffers[i]->getSize()));
		indexWriter.setBufferInfo(i, pvrvk::DescriptorBufferInfo(_deviceResources->indexBuffers[i], 0, _deviceResources->indexBuffers[i]->getSize()));
	}
	writeDescSets.push_back(vertexWriter);
	writeDescSets.push_back(indexWriter);

	// Write the descriptor sets
	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

void VulkanRayTracedHardShadows::createRayTracingPipelines()
{
	// pipeline layout
	pvrvk::PipelineLayoutCreateInfo pipeLayout;
	pipeLayout.addDescSetLayout(_deviceResources->raytracedImageStoreDescriptorSetLayout);
	pipeLayout.addDescSetLayout(_deviceResources->perFrameDescriptorSetLayout);
	pipeLayout.addDescSetLayout(_deviceResources->bindlessResourcesDescriptorSetLayout);

	_deviceResources->raytracePipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayout);

	// Create all the shader modules for this raytracing pipeline
	pvrvk::ShaderModule raygenSM = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::RayGenShader)->readToEnd<uint32_t>()));
	pvrvk::ShaderModule missSM = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::RayMissShader)->readToEnd<uint32_t>()));
	pvrvk::ShaderModule chitSM = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::RayHitShader)->readToEnd<uint32_t>()));
	pvrvk::ShaderModule shadowMissSM = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::ShadowMissShader)->readToEnd<uint32_t>()));
	pvrvk::ShaderModule shadowCHitSM = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::ShadowHitShader)->readToEnd<uint32_t>()));

	pvrvk::RaytracingPipelineCreateInfo raytracingPipeline;

	// Ray Generation
	pvrvk::PipelineShaderStageCreateInfo generateCreateInfo;
	generateCreateInfo.setShader(raygenSM);
	generateCreateInfo.setShaderStage(pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR);
	raytracingPipeline.stages.push_back(generateCreateInfo);

	// Miss - Primary Rays
	pvrvk::PipelineShaderStageCreateInfo missCreateInfo;
	missCreateInfo.setShader(missSM);
	missCreateInfo.setShaderStage(pvrvk::ShaderStageFlags::e_MISS_BIT_KHR);
	raytracingPipeline.stages.push_back(missCreateInfo);

	// Miss - Shadow Rays
	pvrvk::PipelineShaderStageCreateInfo shadowMissCreateInfo;
	shadowMissCreateInfo.setShader(shadowMissSM);
	shadowMissCreateInfo.setShaderStage(pvrvk::ShaderStageFlags::e_MISS_BIT_KHR);
	raytracingPipeline.stages.push_back(shadowMissCreateInfo);

	// Closest Hit - Primary Rays
	pvrvk::PipelineShaderStageCreateInfo hitCreateInfo;
	hitCreateInfo.setShader(chitSM);
	hitCreateInfo.setShaderStage(pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR);
	raytracingPipeline.stages.push_back(hitCreateInfo);

	// Closest Hit - Shadow Rays
	pvrvk::PipelineShaderStageCreateInfo shadowHitCreateInfo;
	shadowHitCreateInfo.setShader(shadowCHitSM);
	shadowHitCreateInfo.setShaderStage(pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR);
	raytracingPipeline.stages.push_back(shadowHitCreateInfo);

	// Create 1 shader group for each shader used as there are no optional shaders in this pipeline
	pvrvk::RayTracingShaderGroupCreateInfo rg = pvrvk::RayTracingShaderGroupCreateInfo(pvrvk::RayTracingShaderGroupTypeKHR::e_GENERAL_KHR);
	pvrvk::RayTracingShaderGroupCreateInfo mg = pvrvk::RayTracingShaderGroupCreateInfo(pvrvk::RayTracingShaderGroupTypeKHR::e_GENERAL_KHR);
	pvrvk::RayTracingShaderGroupCreateInfo hg = pvrvk::RayTracingShaderGroupCreateInfo(pvrvk::RayTracingShaderGroupTypeKHR::e_TRIANGLES_HIT_GROUP_KHR);
	pvrvk::RayTracingShaderGroupCreateInfo smg = pvrvk::RayTracingShaderGroupCreateInfo(pvrvk::RayTracingShaderGroupTypeKHR::e_GENERAL_KHR);
	pvrvk::RayTracingShaderGroupCreateInfo shg = pvrvk::RayTracingShaderGroupCreateInfo(pvrvk::RayTracingShaderGroupTypeKHR::e_TRIANGLES_HIT_GROUP_KHR);

	// Ray Gen group at offset 0
	rg.setGeneralShader(static_cast<uint32_t>(0));
	// Miss group index 0 - Primary rays
	mg.setGeneralShader(static_cast<uint32_t>(1));
	// Miss group index 1 - Shadow rays
	smg.setGeneralShader(static_cast<uint32_t>(2));
	// Hit group offset at 0 - Primary rays
	hg.setClosestHitShader(static_cast<uint32_t>(3));
	// Hit group offset at 1 - Shadow rays
	shg.setClosestHitShader(static_cast<uint32_t>(4));

	// Attach the shader groups to the raytracing pipeline in the order specified above
	raytracingPipeline.shaderGroups = { rg, mg, smg, hg, shg };
	_shaderGroupCount = static_cast<uint32_t>(raytracingPipeline.shaderGroups.size());

	// Set the variables below with data needed for the shader binding table
	_numberRayGenShaders = 1;
	_numberRayMissShaders = 2;
	_numberRayHitShaders = 2;	

	// Allow primary hit group to fire another ray
	raytracingPipeline.maxRecursionDepth = 2;

	// Create the raytracing pipeline
	raytracingPipeline.pipelineLayout = _deviceResources->raytracePipelineLayout;
	_deviceResources->raytracePipeline = _deviceResources->device->createRaytracingPipeline(raytracingPipeline, nullptr);
}

void VulkanRayTracedHardShadows::createOnScreenPipeline()
{
	// Create the pipeline layout with one descriptor set, sampling the raytraced image
	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
	pipeLayoutInfo.setDescSetLayout(0, _deviceResources->raytracedImageSamplerDescriptorSetLayout);
	_deviceResources->onScreenPipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
	pvrvk::GraphicsPipelineCreateInfo pipelineCreateInfo;
	pipelineCreateInfo.pipelineLayout = _deviceResources->onScreenPipelineLayout;

	// Set the viewport from the swapchain
	pipelineCreateInfo.viewport.setViewportAndScissor(0,
		pvrvk::Viewport(
			0.0f, 0.0f, static_cast<float>(_deviceResources->swapchain->getDimension().getWidth()), static_cast<float>(_deviceResources->swapchain->getDimension().getHeight())),
		pvrvk::Rect2D(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight()));

	// set counter clockwise winding order for front faces
	pipelineCreateInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

	// blend state
	pvrvk::PipelineColorBlendAttachmentState colorAttachmentState;
	colorAttachmentState.setBlendEnable(false);
	pipelineCreateInfo.colorBlend.setAttachmentState(0, colorAttachmentState);

	// Vertex input is clear because it is hardcoded inside the vertex shader
	pipelineCreateInfo.vertexInput.clear();
	pipelineCreateInfo.inputAssembler = pvrvk::PipelineInputAssemblerStateCreateInfo();

	// renderpass/subpass
	pipelineCreateInfo.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();

	// Load and create the shaders required for the copying of the raytraced images
	pipelineCreateInfo.vertexShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::FullscreenTriangleVertexShader)->readToEnd<uint32_t>())));
	pipelineCreateInfo.fragmentShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::DeferredShadingFragmentShader)->readToEnd<uint32_t>())));

	// Create the pipeline
	_deviceResources->onScreenPipeline = _deviceResources->device->createGraphicsPipeline(pipelineCreateInfo, _deviceResources->pipelineCache);
	_deviceResources->onScreenPipeline->setObjectName("OnScreenGraphicsPipeline");
}

uint32_t VulkanRayTracedHardShadows::makeMultipleOf(uint32_t a, uint32_t b) {
	return (a + (b - 1)) & ~uint32_t(b - 1);
}

void VulkanRayTracedHardShadows::createShaderBindingTable()
{
	// All shader groups in the shader binding table (i.e., all ray gen shaders, all ray miss shaders, all closest hit shaders) have to be aligned in memory,
	// having a size multiple of shaderGroupBaseAlignment
	// Inside each shader group, each shader handle in the shader binding table has to be aligned in memory as well, with a size multiple of shaderGroupHandleAlignment
	// An example for the ray generation shader group and any set of ray generation shaders is shown below:
	// |------------------------------------------------------------Ray gen shader group-----------------------------------------------------|
	// |------------------------------------------------------Multiple of shaderGroupBaseAlignment-------------------------------------------|
	// ||-------------RayGenShader0-----------||--------------RayGenShader1-----------|...|--------------RayGenShaderN-----------|-----------|
	// |Multiple of shaderGroupHandleAlignment||Multiple of shaderGroupHandleAlignment|...|Multiple of shaderGroupHandleAlignment|-----------|
	
	// This appplies for all the shader groups used, in this case, ray gen, ray miss and ray hit
	// |-----------Ray gen shader group------------||------------Ray miss shader group-----------||-----------Ray hit shader group-------------|
	// |---Multiple of shaderGroupBaseAlignment----||----Multiple of shaderGroupBaseAlignment----||----Multiple of shaderGroupBaseAlignment----|

	uint32_t shaderGroupHandleSize = _rtProperties.shaderGroupHandleSize;

	// Use the Vulkan bindings to get the handles for the shader groups which are attached to the raytracing pipeline
	uint32_t shaderGroupHandlesSize = _shaderGroupCount * shaderGroupHandleSize;
	std::vector<uint8_t> shaderHandleStorage(shaderGroupHandlesSize);

	_deviceResources->device->getVkBindings().vkGetRayTracingShaderGroupHandlesKHR(
		_deviceResources->device->getVkHandle(), _deviceResources->raytracePipeline->getVkHandle(), 0, _shaderGroupCount, shaderGroupHandlesSize, shaderHandleStorage.data());

	// We know the amount of ray generation, miss and hit shaders built at createRayTracingPipelines, and also that the order in which they are setup
	// in the pipeline is ray generation shaders, ray miss shaders and then ray hit shaders (this order has to be reproduced as well in the shader binding table).
	// So basically, for each group, count how many shaders are there, compute its size with shaderGroupHandleAlignment and round it up to a multiple
	// of _shaderGroupBaseAlignment

	_shaderGroupHandleSizeAligned = makeMultipleOf(shaderGroupHandleSize, _shaderGroupHandleAlignment);

	_sizeRayGenGroup = makeMultipleOf(_shaderGroupHandleSizeAligned * _numberRayGenShaders, _shaderGroupBaseAlignment);
	_sizeRayMissGroup = makeMultipleOf(_shaderGroupHandleSizeAligned * _numberRayMissShaders, _shaderGroupBaseAlignment);
	_sizeRayHitGroup = makeMultipleOf(_shaderGroupHandleSizeAligned * _numberRayHitShaders, _shaderGroupBaseAlignment);

	// Use pvr::utils to create a buffer to store the shader binding table in which is of sixe sbtSize
	uint32_t shaderBindingTableSize = _sizeRayGenGroup + _sizeRayMissGroup + _sizeRayHitGroup;
	_deviceResources->raytraceShaderBindingTable = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(shaderBindingTableSize,
			pvrvk::BufferUsageFlags::e_TRANSFER_SRC_BIT | pvrvk::BufferUsageFlags::e_SHADER_BINDING_TABLE_BIT_KHR | pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		pvrvk::MemoryPropertyFlags::e_NONE, nullptr, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT, pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT);
	_deviceResources->raytraceShaderBindingTable->setObjectName("RaytraceShaderBindingTableBuffer");

	// Map the memory from this new buffer so it can be written to.
	void* mapped = _deviceResources->raytraceShaderBindingTable->getDeviceMemory()->map(0, VK_WHOLE_SIZE);
	uint8_t* pData = reinterpret_cast<uint8_t*>(mapped);
	uint32_t shaderGroupCounter = 0;

	// Take into account that the information in shaderHandleStorage follows the shader group setup done when building the
	// ray tracing pipeline, in this case, (ray gen shader, ray miss shader, ray miss shader, ray hit shader, ray hit shader)

	// Copy ray generation shader handle information present in shaderHandleStorage into the shader binding table
	for (uint32_t i = 0; i < _numberRayGenShaders; i++)
	{
		memcpy(pData, shaderHandleStorage.data() + shaderGroupCounter * static_cast<size_t>(shaderGroupHandleSize), shaderGroupHandleSize);
		shaderGroupCounter++;
		pData += _shaderGroupHandleSizeAligned;
	}

	// Copy ray miss shader handle information present in shaderHandleStorage into the shader binding table
	pData = reinterpret_cast<uint8_t*>(mapped);
	pData += _sizeRayGenGroup;
	for (uint32_t i = 0; i < _numberRayMissShaders; i++)
	{
		memcpy(pData, shaderHandleStorage.data() + shaderGroupCounter * static_cast<size_t>(shaderGroupHandleSize), shaderGroupHandleSize);
		shaderGroupCounter++;
		pData += _shaderGroupHandleSizeAligned;
	}

	// Copy ray hit shader handle information present in shaderHandleStorage into the shader binding table
	pData = reinterpret_cast<uint8_t*>(mapped);
	pData += _sizeRayGenGroup + _sizeRayMissGroup;
	for (uint32_t i = 0; i < _numberRayHitShaders; i++)
	{
		memcpy(pData, shaderHandleStorage.data() + shaderGroupCounter * static_cast<size_t>(shaderGroupHandleSize), shaderGroupHandleSize);
		shaderGroupCounter++;
		pData += _shaderGroupHandleSizeAligned;
	}

	_deviceResources->raytraceShaderBindingTable->getDeviceMemory()->unmap();
}

void VulkanRayTracedHardShadows::createPipelines()
{
	createRayTracingPipelines();
	createOnScreenPipeline();
}

void VulkanRayTracedHardShadows::createRayTracedImage()
{
	// Make the raytracing image the same size as the swapchain
	const pvrvk::Extent3D& dimension = pvrvk::Extent3D(_deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight(), 1u);

	// Create image
	pvrvk::Image raytracedImage = pvr::utils::createImage(_deviceResources->device,
		pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, pvrvk::Format::e_R8G8B8A8_UNORM, dimension, pvrvk::ImageUsageFlags::e_STORAGE_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _deviceResources->vmaAllocator,
		pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);

	// Create image view
	_deviceResources->raytracedImage = _deviceResources->device->createImageView(
		pvrvk::ImageViewCreateInfo(raytracedImage, pvrvk::ImageViewType::e_2D, raytracedImage->getFormat(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT)));
}

void VulkanRayTracedHardShadows::createModelBuffers(pvrvk::CommandBuffer& uploadCmd)
{
	// An acceleration structure is a construction which segments the scene to allow the rays to traverse it quicker.
	// A Top Level Acceleration Structure (TLAS) represents the scene and has many Bottom Level Acceleration Structures associated to it.
	// Each BLAS represents a mesh node with its associated vertex and index buffer in model space.

	// Then each instance of a mesh node is tracked via the TLAS and its transformation to world space is stored in the instance buffer.
	// In this example each mesh node is only used by one instance, so reserve that many BLASs and instance transforms too.
	uint32_t numMeshes = _scene->getNumMeshes();
	_deviceResources->vertexBuffers.reserve(numMeshes);
	_deviceResources->indexBuffers.reserve(numMeshes);
	_deviceResources->verticesSize.reserve(numMeshes);
	_deviceResources->indicesSize.reserve(numMeshes);
	_instanceTransforms.reserve(numMeshes);

	// For this demo the materials only use a diffuse color and remains static
	std::vector<glm::vec4> diffuseColors;

	// The format for vertices in an acceleration structure is currently fixed inside the SDK, load each mesh as the required format
	for (uint32_t meshID = 0; meshID < numMeshes; meshID++)
	{
		// Populate the SDK mesh information from the scene handle
		pvr::assets::Mesh mesh = _scene->getMesh(meshID);
		const pvr::assets::Model::Node& node = _scene->getNode(meshID);

		// Reserve space for the index buffer
		uint32_t numIndices = mesh.getNumIndices();
		std::vector<uint32_t> indices(numIndices);

		// Get the indices from the mesh, which depends on the format used by the pod file
		auto indicesWrapper = mesh.getFaces();
		if (indicesWrapper.getDataType() == pvr::IndexType::IndexType16Bit)
		{
			uint16_t* indicesPointer = (uint16_t*)indicesWrapper.getData();
			indices.insert(indices.begin(), indicesPointer, indicesPointer + numIndices);
		}
		else
		{
			uint32_t* indicesPointer = (uint32_t*)indicesWrapper.getData();
			indices.insert(indices.begin(), indicesPointer, indicesPointer + numIndices);
		}

		// Get the vertices information from the pvr::utils mesh wrapper
		pvr::StridedBuffer verticesWrapper = mesh.getVertexData(0);
		uint32_t vertexStrideBytes = static_cast<uint32_t>(verticesWrapper.stride);
		uint32_t vertexStrideFloats = vertexStrideBytes / sizeof(float);
		uint32_t numVertices = static_cast<uint32_t>(verticesWrapper.size()) / vertexStrideBytes;

		// Reserve space for the vertex buffer
		std::vector<pvr::utils::ASVertexFormat> vertices(numVertices);

		// Vertices are stored as a flat array of floats with an implied order, shift those floats into the currently fixed acceleration
		// structure vertex format
		auto verticesStart = reinterpret_cast<float*>(verticesWrapper.data());
		auto verticesEnd = verticesStart + static_cast<size_t>(numVertices) * vertexStrideFloats;
		uint32_t vertexIndex = 0;
		for (auto v = verticesStart; v < verticesEnd; v += vertexStrideFloats)
		{
			vertices.insert(vertices.begin() + vertexIndex,
				{
					glm::vec3(v[0], v[1], v[2]), // position
					glm::vec3(v[3], v[4], v[5]), // normals
					glm::vec2(v[6], v[7]), // texture coordinates
					glm::vec3(1.0) // tangent
				});
			vertexIndex++;
		}

		// Store the world transform for this mesh instance
		_instanceTransforms.push_back(_scene->getWorldMatrix(node.getObjectId()));

		// Get the diffuse color for the material from pvr::utils mesh
		glm::vec3 diffuse = _scene->getMaterial(node.getObjectId()).defaultSemantics().getDiffuse();
		// convert from linear to sRGB
		diffuse = glm::vec4(glm::pow(glm::vec3(diffuse.x, diffuse.y, diffuse.z), glm::vec3(2.2f)), 0.0f);
		diffuseColors.push_back(glm::vec4(diffuse, 1.0));

		// Upload the Buffers to the GPU
		// create vertex buffer
		pvrvk::BufferCreateInfo vertexBufferInfo;
		vertexBufferInfo.setSize(sizeof(pvr::utils::ASVertexFormat) * vertices.size());
		vertexBufferInfo.setUsageFlags(pvrvk::BufferUsageFlags::e_VERTEX_BUFFER_BIT | pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT |
			pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT | pvrvk::BufferUsageFlags::e_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);
		_deviceResources->vertexBuffers.push_back(pvr::utils::createBuffer(_deviceResources->device, vertexBufferInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			pvrvk::MemoryPropertyFlags::e_NONE, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE, pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT));
		_deviceResources->vertexBuffers.back()->setObjectName("VBO");
		pvr::utils::updateBufferUsingStagingBuffer(
			_deviceResources->device, _deviceResources->vertexBuffers[meshID], uploadCmd, vertices.data(), 0, sizeof(pvr::utils::ASVertexFormat) * vertices.size());

		// create index buffer
		pvrvk::BufferCreateInfo indexBufferInfo;
		indexBufferInfo.setSize(sizeof(uint32_t) * indices.size());
		indexBufferInfo.setUsageFlags(pvrvk::BufferUsageFlags::e_INDEX_BUFFER_BIT | pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT |
			pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT | pvrvk::BufferUsageFlags::e_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);
		_deviceResources->indexBuffers.push_back(pvr::utils::createBuffer(_deviceResources->device, indexBufferInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			pvrvk::MemoryPropertyFlags::e_NONE, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE, pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT));
		_deviceResources->indexBuffers.back()->setObjectName("IBO");
		pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, _deviceResources->indexBuffers[meshID], uploadCmd, indices.data(), 0, sizeof(uint32_t) * indices.size());

		// Need to track the number of elements in each buffer for when the acceleration structure is built
		_deviceResources->verticesSize.push_back(static_cast<int32_t>(vertices.size()));
		_deviceResources->indicesSize.push_back(static_cast<int32_t>(indices.size()));
	}

	// Create and upload the transforms buffer
	pvrvk::BufferCreateInfo transformBufferInfo;
	transformBufferInfo.setSize(sizeof(glm::mat4x4) * _instanceTransforms.size());
	transformBufferInfo.setUsageFlags(pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT);
	_deviceResources->instanceTransformBuffer = pvr::utils::createBuffer(_deviceResources->device, transformBufferInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT);
	pvr::utils::updateBufferUsingStagingBuffer(
		_deviceResources->device, _deviceResources->instanceTransformBuffer, uploadCmd, _instanceTransforms.data(), 0, sizeof(glm::mat4x4) * _instanceTransforms.size());
	_deviceResources->instanceTransformBuffer->setObjectName("instanceTransformSBO");

	// Create and upload the material data buffer
	pvrvk::BufferCreateInfo materialColorBufferInfo;
	materialColorBufferInfo.setSize(sizeof(glm::vec4) * diffuseColors.size());
	materialColorBufferInfo.setUsageFlags(pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT);
	_deviceResources->materialBuffer = pvr::utils::createBuffer(_deviceResources->device, materialColorBufferInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT);
	_deviceResources->materialBuffer->setObjectName("MaterialSBO");
	pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, _deviceResources->materialBuffer, uploadCmd, diffuseColors.data(), 0, sizeof(glm::vec4) * diffuseColors.size());
}

void VulkanRayTracedHardShadows::createCameraBuffer()
{
	// Only need the inverse matrix to map the screenspace coordinates to the ray origin in the ray gen shader
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(BufferEntryNames::CameraUbo::InverseViewMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::CameraUbo::InverseProjectionMatrix, pvr::GpuDatatypes::mat4x4);

	// Dynamic buffer, with an offset per swapchain image
	_deviceResources->CameraBufferView.initDynamic(desc, _deviceResources->swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
		static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

	// Create the buffer and map the memory to a structured buffer view
	_deviceResources->CameraBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->CameraBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->CameraBuffer->setObjectName("CameraUBO");

	_deviceResources->CameraBufferView.pointToMappedMemory(_deviceResources->CameraBuffer->getDeviceMemory()->getMappedData());
}

void VulkanRayTracedHardShadows::createLightBuffer()
{
	// Get the data about the light from the scene handle
	glm::vec4 lightPosition;
	_scene->getLightPosition(0, lightPosition);
	pvr::assets::Light light = _scene->getLight(0);

	// Using a pointlight with a position, color and intensity
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(BufferEntryNames::PointLightData::LightPosition, pvr::GpuDatatypes::vec4);
	desc.addElement(BufferEntryNames::PointLightData::LightColor, pvr::GpuDatatypes::vec4);
	desc.addElement(BufferEntryNames::PointLightData::LightIntensity, pvr::GpuDatatypes::Float);

	// Dynamic buffer, with an offset per swapchain image
	_deviceResources->lightDataBufferView.init(desc);

	// Create the buffer and map the memory to a structured buffer view
	_deviceResources->lightDataBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->lightDataBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->lightDataBuffer->setObjectName("LightDataUBO");

	_deviceResources->lightDataBufferView.pointToMappedMemory(_deviceResources->lightDataBuffer->getDeviceMemory()->getMappedData());

	// Update the light UBO
	_deviceResources->lightDataBufferView.getElementByName(BufferEntryNames::PointLightData::LightPosition).setValue(lightPosition);
	_deviceResources->lightDataBufferView.getElementByName(BufferEntryNames::PointLightData::LightColor).setValue(glm::vec4(light.getColor(), 1.0f));
	_deviceResources->lightDataBufferView.getElementByName(BufferEntryNames::PointLightData::LightIntensity).setValue(1.5f);

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->lightDataBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->lightDataBuffer->getDeviceMemory()->flushRange(0, _deviceResources->lightDataBufferView.getDynamicSliceSize());
	}
}

void VulkanRayTracedHardShadows::updateCameraAnimation()
{
	// Update the properties for the camera
	glm::vec3 vFrom, vTo, vUp;
	float fov;
	_scene->getCameraProperties(static_cast<uint32_t>(0), fov, vFrom, vTo, vUp);

	// Create a rotation matrix
	static float angle = 0.0f;
	if (_animateCamera) { angle += getFrameTime() * 0.01f; }
	vFrom = glm::mat4_cast(glm::angleAxis(glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f))) * glm::vec4(vFrom, 1.0f);

	// Update camera matrices
	_cameraPosition = vFrom;
	_viewMatrix = glm::lookAt(_cameraPosition, vTo, glm::vec3(0.0f, 1.0f, 0.0f));
	_viewProjectionMatrix = _projectionMatrix * _viewMatrix;
	_inverseViewMatrix = glm::inverse(_viewMatrix);

	// Update the camera UBO at dynamic offset for this swapchain image
	uint32_t cameraDynamicSliceIdx = _deviceResources->swapchain->getSwapchainIndex();
	_deviceResources->CameraBufferView.getElementByName(BufferEntryNames::CameraUbo::InverseViewMatrix, 0, cameraDynamicSliceIdx).setValue(glm::inverse(_viewMatrix));
	_deviceResources->CameraBufferView.getElementByName(BufferEntryNames::CameraUbo::InverseProjectionMatrix, 0, cameraDynamicSliceIdx).setValue(glm::inverse(_projectionMatrix));

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->CameraBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->CameraBuffer->getDeviceMemory()->flushRange(
			_deviceResources->CameraBufferView.getDynamicSliceOffset(cameraDynamicSliceIdx), _deviceResources->CameraBufferView.getDynamicSliceSize());
	}
}

void VulkanRayTracedHardShadows::updateSceneAnimation()
{
	if (_animateScene)
	{
		// Get the SDKs method of animation handling and modulo the animation index so that it repeats seamlessly
		_frameNumber += static_cast<float>(getFrameTime());
		pvr::assets::AnimationInstance animation = _scene->getAnimationInstance(0);
		_frameNumber = fmod(_frameNumber, animation.getTotalTimeInMs());
		animation.updateAnimation(_frameNumber);

		// For each mesh node update the transform
		for (uint32_t i = 0; i < _scene->getNumMeshNodes(); i++) { _instanceTransforms.at(i) = _scene->getWorldMatrix(_scene->getNodeIdForMeshNodeId(i)); }

		//glm::mat4x4 move = glm::translate(glm::vec3(0, 1, 0));
		// for (uint32_t i = 0; i < _instanceTransforms.size(); i++) { _instanceTransforms.at(i) *= move; }

		// Update the acceleration structure
		_deviceResources->accelerationStructure.updateInstanceTransformData(_instanceTransforms);

		// command buffer at the current frame index has already been waited on so we know there won't be a race condition
		pvrvk::CommandBuffer commandBuffer = _deviceResources->commandPool->allocateCommandBuffer();

		_deviceResources->accelerationStructure.buildTopLevelASAndInstances(_deviceResources->device, commandBuffer, _deviceResources->queue,
			pvrvk::BuildAccelerationStructureFlagsKHR::e_PREFER_FAST_TRACE_BIT_KHR | pvrvk::BuildAccelerationStructureFlagsKHR::e_ALLOW_UPDATE_BIT_KHR, true);

		// Update the instance transforms buffer that is used inside of the closest hit shader
		commandBuffer->begin();
		pvr::utils::updateBufferUsingStagingBuffer(
			_deviceResources->device, _deviceResources->instanceTransformBuffer, commandBuffer, _instanceTransforms.data(), 0, sizeof(glm::mat4) * _instanceTransforms.size());
		commandBuffer->end();

		// Submit the update to the instance transforms buffer
		pvrvk::SubmitInfo submit;
		submit.commandBuffers = &commandBuffer;
		submit.numCommandBuffers = 1;
		_deviceResources->queue->submit(&submit, 1);
		_deviceResources->queue->waitIdle(); // wait
	}
}

void VulkanRayTracedHardShadows::recordMainCommandBuffer()
{
	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		{
			_deviceResources->primaryCmdBuffers[i]->begin();

			pvr::utils::beginCommandBufferDebugLabel(_deviceResources->primaryCmdBuffers[i], pvrvk::DebugUtilsLabel("MainRenderPassSwapchain" + std::to_string(i)));

			pvrvk::Rect2D renderArea(0, 0, _windowWidth, _windowHeight);

			// Raytrace scene and write to offscreen render target
			_deviceResources->primaryCmdBuffers[i]->executeCommands(_deviceResources->raytracedCmdBuffers[i]);

			pvrvk::ClearValue onscreenClearValues[] = { pvrvk::ClearValue(0.10, 0.10, 0.10, 1.0f), pvrvk::ClearValue(1.f, 0u) };

			// Composite + UI
			_deviceResources->primaryCmdBuffers[i]->beginRenderPass(_deviceResources->onScreenFramebuffer[i], renderArea, false, onscreenClearValues, 2);

			_deviceResources->primaryCmdBuffers[i]->executeCommands(_deviceResources->onScreenCmdBuffers[i]);

			_deviceResources->primaryCmdBuffers[i]->endRenderPass();

			pvr::utils::endCommandBufferDebugLabel(_deviceResources->primaryCmdBuffers[i]);

			_deviceResources->primaryCmdBuffers[i]->end();
		}
	}
}

void VulkanRayTracedHardShadows::recordSecondaryCommandBuffers()
{
	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		_deviceResources->raytracedCmdBuffers[i]->begin();
		recordCommandBufferRaytraces(_deviceResources->raytracedCmdBuffers[i], i);
		_deviceResources->raytracedCmdBuffers[i]->end();

		_deviceResources->onScreenCmdBuffers[i]->begin(_deviceResources->onScreenFramebuffer[i]);
		recordCommandBufferDeferredShading(_deviceResources->onScreenCmdBuffers[i], i);
		recordCommandUIRenderer(_deviceResources->onScreenCmdBuffers[i]);
		_deviceResources->onScreenCmdBuffers[i]->end();
	}
}

void VulkanRayTracedHardShadows::recordCommandBufferRaytraces(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex)
{
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffers, pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Ray Tracing Stage - Swapchain (%i)", swapchainIndex)));

	// Add a pipeline barrier to transform the raytraced image to be writeable so the result of raytracing can be stored to it
	{
		pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_UNDEFINED;
		pvrvk::ImageLayout destinationImageLayout = pvrvk::ImageLayout::e_GENERAL;

		pvrvk::MemoryBarrierSet layoutTransitions;
		layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::AccessFlags::e_SHADER_WRITE_BIT,
			_deviceResources->raytracedImage->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), sourceImageLayout, destinationImageLayout,
			_deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));

		cmdBuffers->pipelineBarrier(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::PipelineStageFlags::e_RAY_TRACING_SHADER_BIT_KHR, layoutTransitions);
	}

	// Bind to the raytracing pipeline
	cmdBuffers->bindPipeline(_deviceResources->raytracePipeline);

	// Bind to the descriptor sets used for the raytracing pipeline, set 0 for image store, set 1 for camera and lights, set 2 for the bindless resources
	pvrvk::DescriptorSet arrayDS[] = { _deviceResources->raytracedImageStoreDescriptorSet, _deviceResources->perFrameDescriptorSet, _deviceResources->bindlessResourcesDescriptorSet };
	uint32_t offsets[2] = {};
	offsets[0] = _deviceResources->CameraBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[1] = 0;
	cmdBuffers->bindDescriptorSets(pvrvk::PipelineBindPoint::e_RAY_TRACING_KHR, _deviceResources->raytracePipelineLayout, 0, arrayDS, 3, offsets, 2);

	// Shaders in the shader binding table are grouped together by stage, need to find the address of the first shader group for each stage
	VkDeviceAddress sbtAddress = _deviceResources->raytraceShaderBindingTable->getDeviceAddress(_deviceResources->device);

	// The address of the shader groups is the start of the sbt + the offset calculated above
	// Note that the stride and the size of the ray generation group have to have the same value, this is a special case that has to be always covered
	pvrvk::StridedDeviceAddressRegionKHR raygenShaderBindingTable = { sbtAddress, _sizeRayGenGroup, _sizeRayGenGroup };
	pvrvk::StridedDeviceAddressRegionKHR missShaderBindingTable = { sbtAddress + _sizeRayGenGroup, _shaderGroupHandleSizeAligned, _sizeRayMissGroup };
	pvrvk::StridedDeviceAddressRegionKHR hitShaderBindingTable = { sbtAddress + _sizeRayGenGroup + _sizeRayMissGroup, _shaderGroupHandleSizeAligned, _sizeRayHitGroup };
	pvrvk::StridedDeviceAddressRegionKHR callableShaderBindingTable = {};

	// Trace the rays
	cmdBuffers->traceRays(raygenShaderBindingTable, missShaderBindingTable, hitShaderBindingTable, callableShaderBindingTable, getWidth(), getHeight(), 1);

	// Add a pipeline barrier to transfrom the raytraced image to be read only, so it can be accessed by the copy to swapchain shader
	{
		pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_GENERAL;
		pvrvk::ImageLayout destinationImageLayout = pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL;

		pvrvk::MemoryBarrierSet layoutTransitions;
		layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::AccessFlags::e_SHADER_WRITE_BIT,
			_deviceResources->raytracedImage->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), sourceImageLayout, destinationImageLayout,
			_deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));

		cmdBuffers->pipelineBarrier(pvrvk::PipelineStageFlags::e_RAY_TRACING_SHADER_BIT_KHR, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, layoutTransitions);
	}

	pvr::utils::endCommandBufferDebugLabel(cmdBuffers);
}

void VulkanRayTracedHardShadows::recordCommandBufferDeferredShading(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex)
{
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffers, pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Deferred Shading - Swapchain (%i)", swapchainIndex)));

	cmdBuffers->bindPipeline(_deviceResources->onScreenPipeline);

	// Bind to the descriptor set containing the raytraced image for sampling
	cmdBuffers->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->onScreenPipelineLayout, 0, _deviceResources->raytracedImageSamplerDescriptorSet);

	// Draw a triangle hardcoded into the vertex shader that covers the whole swapchain image
	cmdBuffers->draw(0, 3);

	pvr::utils::endCommandBufferDebugLabel(cmdBuffers);
}

void VulkanRayTracedHardShadows::recordCommandUIRenderer(pvrvk::SecondaryCommandBuffer& commandBuff)
{
	pvr::utils::beginCommandBufferDebugLabel(commandBuff, pvrvk::DebugUtilsLabel("UI"));

	_deviceResources->uiRenderer.beginRendering(commandBuff);
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.endRendering();

	pvr::utils::endCommandBufferDebugLabel(commandBuff);
}
