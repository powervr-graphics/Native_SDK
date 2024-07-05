/*!
\brief Implements Ray Traced Soft Shadows using Vulkan Ray Queries and a simple denoiser.
\file VulkanHybridSoftShadows.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRVk/PVRVk.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRUtils/Vulkan/AccelerationStructure.h"
#include "vulkan/vulkan_beta.h"
#define _USE_MATH_DEFINES
#include <math.h>

#define LIGHT_OUTER_CONE_ANGLE 10.0f
#define LIGHT_INNER_CONE_ANGLE 0.8f
#define NUM_SHADOW_RAYS 1

// Maximum number of swap images supported
enum CONSTANTS
{
	MAX_NUMBER_OF_SWAP_IMAGES = 4
};

/// A list of light radius' to cycle between
const float lightRadiusList[] = { 0.0f, 5.0f, 10.0f, 15.0f, 20.0f };

namespace SceneNodes {
enum class MeshNodes
{
	Satyr = 0,
	Spheres = 1,
	BrickWall = 2,
	Table = 3,
	Num = 4
};

enum class Cameras
{
	SceneCamera = 0,
	NumCameras = 1
};
} // namespace SceneNodes

// Framebuffer colour attachment indices
namespace FramebufferGBufferAttachments {
enum Enum
{
	Albedo_Shininess = 0,
	Normal_Visibility_HitDistance,
	Count
};
}

/// <summary>Shader names for all of the demo passes.</summary>
namespace Files {
const char* const SceneFile = "SoftShadows.POD";

const char* const GBufferVertexShader = "GBufferVertexShader.vsh.spv";
const char* const GBufferFragmentShader = "GBufferFragmentShader.fsh.spv";
const char* const DeferredShadingFragmentShader = "DeferredShadingFragmentShader.fsh.spv";
const char* const FullscreenTriangleVertexShader = "FullscreenTriangleVertexShader.vsh.spv";
} // namespace Files

/// <summary>buffer entry names used for the structured memory views used throughout the demo.
/// These entry names must match the variable names used in the demo shaders.</summary>
namespace BufferEntryNames {
namespace PerScene {
const char* const ViewMatrix = "mViewMatrix";
const char* const ProjectionMatrix = "mProjectionMatrix";
const char* const InvViewProjectionMatrix = "mInvViewProjectionMatrix";
const char* const EyePosition = "vEyePosition";
const char* const ClipPlanes = "vClipPlanes";
const char* const FrameIdx = "uFrameIdx";
} // namespace PerScene

namespace PerMesh {
const char* const WorldMatrix = "mWorldMatrix";
} // namespace PerMesh

namespace PerLightData {
const char* const LightColor = "vLightColor";
const char* const LightPosition = "vLightPosition";
const char* const AmbientColor = "vAmbientColor";
const char* const LightDirection = "vLightDirection";
const char* const PenumbraAngle = "penumbraAngle";
const char* const LightRadius = "lightRadius";
const char* const InnerConeAngle = "innerConeAngle";
const char* const OuterConeAngle = "outerConeAngle";
const char* const NumShadowRays = "numShadowRays";
} // namespace PerLightData
} // namespace BufferEntryNames

// Application wide configuration data
namespace ApplicationConfiguration {
const float FrameRate = 1.0f / 120.0f;
}

// Subpasses used in the renderpass
namespace RenderPassSubpasses {
const uint32_t GBuffer = 0;
// lighting pass
const uint32_t Lighting = 1;
// UI pass
const uint32_t UIRenderer = 1;

const uint32_t NumberOfSubpasses = 2;
} // namespace RenderPassSubpasses

// Light Uniforms
struct PerLightData
{
	glm::vec4 vLightColor;
	glm::vec4 vLightPosition;
	glm::vec4 vAmbientColor;
	glm::vec4 vLightDirection;
	float penumbraAngle;
	float lightRadius;
	float innerConeAngle;
	float outerConeAngle;
	int numShadowRays;
};

// Texture description structure
struct TextureAS
{
	std::string name;
	pvrvk::Format format = pvrvk::Format::e_R8G8B8A8_SRGB;
	pvrvk::Image image;
	pvrvk::ImageView imageView;
};

// Mesh description structure
struct MeshAS
{
	int materialIdx;
	int indexOffset;
	int numIndices;
	glm::mat4 worldMatrix;
	pvrvk::IndexType indexType;
};

struct DeviceResources
{
	pvrvk::Instance instance;
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
	pvrvk::Device device;
	pvrvk::Queue queue;
	pvrvk::Swapchain swapchain;
	pvr::utils::vma::Allocator vmaAllocator;
	pvrvk::CommandPool commandPool;
	pvrvk::DescriptorPool descriptorPool;

	// Stores Texture views for the Images used as attachments on the G-buffer
	pvrvk::ImageView gbufferImages[FramebufferGBufferAttachments::Count];
	pvrvk::ImageView gbufferDepthStencilImage;
	pvrvk::ImageView gbufferVisibilityMipMappedImage;

	// Framebuffer for the G-buffer
	pvrvk::Framebuffer gbufferFramebuffer;

	// Framebuffers created for the swapchain images
	std::vector<pvrvk::Framebuffer> onScreenFramebuffer;

	// Renderpass for the G-buffer
	pvrvk::RenderPass gbufferRenderPass;

	//// Command Buffers ////
	// Main Primary Command Buffer
	pvrvk::CommandBuffer cmdBufferMainDeferred[MAX_NUMBER_OF_SWAP_IMAGES];

	// Secondary command buffers used for each pass
	pvrvk::SecondaryCommandBuffer cmdBufferGBuffer[MAX_NUMBER_OF_SWAP_IMAGES];
	pvrvk::SecondaryCommandBuffer cmdBufferDeferredShading[MAX_NUMBER_OF_SWAP_IMAGES];
	pvrvk::SecondaryCommandBuffer cmdBufferDownsample[MAX_NUMBER_OF_SWAP_IMAGES];

	////  Descriptor Set Layouts ////
	pvrvk::DescriptorSetLayout commonDescriptorSetLayout;
	pvrvk::DescriptorSetLayout gbufferDescriptorSetLayout;

	////  Descriptor Sets ////
	pvrvk::DescriptorSet commonDescriptorSet;
	pvrvk::DescriptorSet gbufferDescriptorSet;

	//// Pipeline Layouts ////
	pvrvk::PipelineLayout gbufferPipelineLayout;
	pvrvk::PipelineLayout deferredShadingPipelineLayout;

	//// Bindless scene resources ////
	std::vector<pvrvk::Buffer> vertexBuffers;
	std::vector<pvrvk::Buffer> indexBuffers;
	std::vector<MeshAS> meshes;
	std::vector<int> verticesSize;
	std::vector<int> indicesSize;
	std::vector<TextureAS> textures;
	pvr::utils::AccelerationStructureWrapper accelerationStructure;

	//// Structured Memory Views ////
	pvrvk::Buffer cameraBuffer;
	pvrvk::Buffer materialBuffer;
	pvrvk::Buffer perMeshBuffer;
	pvrvk::Buffer randomRotationsBuffer;
	pvrvk::Buffer perLightBuffer;
	pvr::utils::StructuredBufferView cameraBufferView;
	pvr::utils::StructuredBufferView perMeshBufferView;
	pvr::utils::StructuredBufferView randomRotationsBufferView;
	pvr::utils::StructuredBufferView perLightBufferView;

	//// Synchronization Primitives ////
	std::vector<pvrvk::Semaphore> imageAcquiredSemaphores;
	std::vector<pvrvk::Semaphore> presentationSemaphores;
	std::vector<pvrvk::Fence> perFrameResourcesFences;

	//// Pipelines ////
	pvrvk::GraphicsPipeline gbufferPipeline;
	pvrvk::GraphicsPipeline defferedShadingPipeline;

	pvrvk::PipelineCache pipelineCache;

	// UIRenderer used to display text
	pvr::ui::UIRenderer uiRenderer;

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

/// <summary>Class implementing the Shell functions.</summary>
class VulkanHybridSoftShadows : public pvr::Shell
{
public:
	//// Frame ////
	uint32_t _numSwapImages;
	uint32_t _swapchainIndex;
	// Putting all API objects into a pointer just makes it easier to release them all together with RAII
	std::unique_ptr<DeviceResources> _deviceResources;

	// Frame counters for animation
	uint32_t _frameId;
	uint32_t _frameNumber;
	float _animationTime = 0.0f;
	bool _animateLight;
	glm::vec3 _lightPos;
	glm::vec3 _eyePos;
	glm::vec3 _satyrCenter;
	uint32_t _lightRadusIdx = 1;
	PerLightData _lightData;
	float _frame = 0.0f;

	// Projection and Model View matrices
	glm::mat4 _viewMatrix;
	glm::mat4 _projectionMatrix;
	glm::mat4 _viewProjectionMatrix;
	glm::mat4 _inverseViewMatrix;
	std::vector<glm::mat4> _meshTransforms;
	float _nearClipDistance;
	float _farClipDistance;

	uint32_t _windowWidth;
	uint32_t _windowHeight;
	uint32_t _framebufferWidth;
	uint32_t _framebufferHeight;

	int32_t _viewportOffsets[2];

	// Scene models
	pvr::assets::ModelHandle _scene;

	/// <summary>Filter several Best Practices performance warnings incompatible with the buffer usage of this demo.</summary>
	std::vector<int> vectorValidationIDFilter;

	/// <summary>Flag to know whether astc iss upported by the physical device.</summary>
	bool _astcSupported;

	VulkanHybridSoftShadows() { _animateLight = false; }

	//  Overridden from pvr::Shell
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void updateScene();
	void createFramebufferAndRenderPass();
	void createPipelines();
	void createGBufferPipelines();
	void createDeferredShadingPipelines();
	void recordMainCommandBuffer();
	void recordCommandBufferRenderGBuffer(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex);
	void recordCommandBufferDeferredShading(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex);
	void recordCommandBufferDownsample(pvrvk::SecondaryCommandBuffer& cmdBuffers);
	void recordCommandUIRenderer(pvrvk::SecondaryCommandBuffer& cmdBuffers);
	void recordSecondaryCommandBuffers();
	void createDescriptorSetLayouts();
	void createDescriptorSets();
	void uploadDynamicSceneData();
	void createCameraBuffer();
	void createMeshTransformBuffer();
	void createRandomRotationsBuffer();
	void createLightBuffer();
	void updateAnimation();
	void initializeLights();
	void createModelBuffers(pvrvk::CommandBuffer& uploadCmd);
	void createTextures(pvrvk::CommandBuffer& uploadCmd);
	uint32_t getTextureIndex(const std::string textureName);

	void eventMappedInput(pvr::SimplifiedInput key)
	{
		switch (key)
		{
		// Handle input
		case pvr::SimplifiedInput::ActionClose: exitShell(); break;
		case pvr::SimplifiedInput::Action1: _lightRadusIdx++; break;
		case pvr::SimplifiedInput::Action2: _animateLight = !_animateLight; break;
		default: break;
		}

		updateDescription();
	}

	void updateDescription()
	{
		std::string lightRadiusString = "Light Radius = " + std::to_string(lightRadiusList[_lightRadusIdx % 5]);

		_deviceResources->uiRenderer.getDefaultDescription()->setText(lightRadiusString);
		_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
	}
};

/// <summary> Code in initApplication() will be called by pvr::Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns> Return true if no error occurred. </returns>
pvr::Result VulkanHybridSoftShadows::initApplication()
{
	// This demo application makes heavy use of the stencil buffer
	setStencilBitsPerPixel(8);
	_frameNumber = 0;
	_frameId = 0;

	//  Load the scene
	_scene = pvr::assets::loadModel(*this, Files::SceneFile);

	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.).</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanHybridSoftShadows::initView()
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

	// device extensions
	std::vector<std::string> vectorExtensionNames{ VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME, VK_KHR_SPIRV_1_4_EXTENSION_NAME, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
		VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME, VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME,
		VK_KHR_RAY_QUERY_EXTENSION_NAME };

	std::vector<int> vectorPhysicalDevicesIndex = pvr::utils::validatePhysicalDeviceExtensions(_deviceResources->instance, vectorExtensionNames);

	if (vectorPhysicalDevicesIndex.size() == 0)
	{
		throw pvrvk::ErrorInitializationFailed("Could not find all the required Vulkan extensions.");
		return pvr::Result::UnsupportedRequest;
	}

	// Create the surface
	pvrvk::Surface surface = pvr::utils::createSurface(
		_deviceResources->instance, _deviceResources->instance->getPhysicalDevice(vectorPhysicalDevicesIndex[0]), this->getWindow(), this->getDisplay(), this->getConnection());

	// Filter several Best Practices performance warnings incompatible with the buffer usage of this demo
	vectorValidationIDFilter.push_back(-602362517);
	vectorValidationIDFilter.push_back(-1277938581);

	// Create a default set of debug utils messengers or debug callbacks using either VK_EXT_debug_utils or VK_EXT_debug_report respectively
	_deviceResources->debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(_deviceResources->instance, (void*)&vectorValidationIDFilter);

	const pvr::utils::QueuePopulateInfo queuePopulateInfo = { pvrvk::QueueFlags::e_GRAPHICS_BIT, surface };
	pvr::utils::QueueAccessInfo queueAccessInfo;

	pvr::utils::DeviceExtensions deviceExtensions = pvr::utils::DeviceExtensions();

	for (const std::string& extensionName : vectorExtensionNames) { deviceExtensions.addExtension(extensionName); }

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

	// Ray Querey
	VkPhysicalDeviceRayQueryFeaturesKHR queryFeatures{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR) };
	scalarFeatures.pNext = &queryFeatures;

	// Descriptor Indexing Features
	VkPhysicalDeviceDescriptorIndexingFeatures indexFeatures{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES) };
	queryFeatures.pNext = &indexFeatures;

	// Fill in all of these device features with one call
	_deviceResources->instance->getVkBindings().vkGetPhysicalDeviceFeatures2(_deviceResources->instance->getPhysicalDevice(vectorPhysicalDevicesIndex[0])->getVkHandle(), &deviceFeatures);

	// Add these device features to the physical device, since they're all connected by a pNext chain, we only need to explicitly attach the top feature
	deviceExtensions.addExtensionFeatureVk<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(&raytracingPipelineFeatures);

	// create device and queues
	_deviceResources->device =
		pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(vectorPhysicalDevicesIndex[0]), &queuePopulateInfo, 1, &queueAccessInfo, deviceExtensions);

	// Get queue
	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);
	_deviceResources->queue->setObjectName("GraphicsQueue");

	// Create vulkan memory allocatortea
	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(vectorPhysicalDevicesIndex[0])->getSurfaceCapabilities(surface);

	// Validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT;
	} // create the swapchain

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

	_astcSupported = pvr::utils::isSupportedFormat(_deviceResources->device->getPhysicalDevice(), pvrvk::Format::e_ASTC_4x4_UNORM_BLOCK);

	const pvr::CommandLine& commandOptions = getCommandLine();
	int32_t intFramebufferWidth = -1;
	int32_t intFramebufferHeight = -1;
	commandOptions.getIntOption("-fbowidth", intFramebufferWidth);
	_framebufferWidth = static_cast<uint32_t>(intFramebufferWidth);
	_framebufferWidth = glm::min<int32_t>(_framebufferWidth, _windowWidth);
	commandOptions.getIntOption("-fboheight", intFramebufferHeight);
	_framebufferHeight = static_cast<uint32_t>(intFramebufferHeight);
	_framebufferHeight = glm::min<int32_t>(_framebufferHeight, _windowHeight);

	_viewportOffsets[0] = (_windowWidth - _framebufferWidth) / 2;
	_viewportOffsets[1] = (_windowHeight - _framebufferHeight) / 2;

	Log(LogLevel::Information, "Framebuffer dimensions: %d x %d\n", _framebufferWidth, _framebufferHeight);
	Log(LogLevel::Information, "On-screen Framebuffer dimensions: %d x %d\n", _windowWidth, _windowHeight);

	_deviceResources->commandPool =
		_deviceResources->device->createCommandPool(pvrvk::CommandPoolCreateInfo(queueAccessInfo.familyId, pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, static_cast<uint16_t>(16 * _numSwapImages))
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, static_cast<uint16_t>(16 * _numSwapImages))
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, static_cast<uint16_t>(16 * _numSwapImages))
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_INPUT_ATTACHMENT, static_cast<uint16_t>(16 * _numSwapImages))
																						  .setMaxDescriptorSets(static_cast<uint16_t>(16 * _numSwapImages)));

	_deviceResources->descriptorPool->setObjectName("DescriptorPool");

	// Setup command buffers
	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		_deviceResources->cmdBufferMainDeferred[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->cmdBufferGBuffer[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->cmdBufferDeferredShading[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->cmdBufferDownsample[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->cmdBufferMainDeferred[i]->setObjectName("DeferredCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->cmdBufferGBuffer[i]->setObjectName("GBufferSecondaryCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->cmdBufferDeferredShading[i]->setObjectName("DeferredShadingSecondaryCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->cmdBufferDownsample[i]->setObjectName("DownsampleSecondaryCommandBufferSwapchain" + std::to_string(i));

		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->presentationSemaphores[i]->setObjectName("PresentationSemaphoreSwapchain" + std::to_string(i));
		_deviceResources->imageAcquiredSemaphores[i]->setObjectName("ImageAcquiredSemaphoreSwapchain" + std::to_string(i));

		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->perFrameResourcesFences[i]->setObjectName("FenceSwapchain" + std::to_string(i));
	}

	_nearClipDistance = _scene->getCamera(static_cast<uint32_t>(SceneNodes::Cameras::SceneCamera)).getNear();
	_farClipDistance = _scene->getCamera(static_cast<uint32_t>(SceneNodes::Cameras::SceneCamera)).getFar();

	// Handle device rotation
	bool isRotated = this->isScreenRotated();
	if (isRotated)
	{
		_projectionMatrix = pvr::math::perspective(pvr::Api::Vulkan, _scene->getCamera(static_cast<uint32_t>(SceneNodes::Cameras::SceneCamera)).getFOV(),
			static_cast<float>(this->getHeight()) / static_cast<float>(this->getWidth()), _scene->getCamera(static_cast<uint32_t>(SceneNodes::Cameras::SceneCamera)).getNear(),
			_scene->getCamera(static_cast<uint32_t>(SceneNodes::Cameras::SceneCamera)).getFar(), glm::pi<float>() * .5f);
	}
	else
	{
		_projectionMatrix = pvr::math::perspective(pvr::Api::Vulkan, _scene->getCamera(static_cast<uint32_t>(SceneNodes::Cameras::SceneCamera)).getFOV(),
			static_cast<float>(this->getWidth()) / static_cast<float>(this->getHeight()), _scene->getCamera(static_cast<uint32_t>(SceneNodes::Cameras::SceneCamera)).getNear(),
			_scene->getCamera(static_cast<uint32_t>(SceneNodes::Cameras::SceneCamera)).getFar());
	}

	// Initialize UIRenderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queue);
	_deviceResources->uiRenderer.getDefaultTitle()->setText("HybridSoftShadows");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText("Action 1: Cycle Light Radius\n"
															   "Action 2: Toggle Animation");
	updateDescription();
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	_deviceResources->cmdBufferMainDeferred[0]->begin();

	createModelBuffers(_deviceResources->cmdBufferMainDeferred[0]);
	createTextures(_deviceResources->cmdBufferMainDeferred[0]);

	_deviceResources->cmdBufferMainDeferred[0]->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->cmdBufferMainDeferred[0];
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle(); // wait

	initializeLights();
	createFramebufferAndRenderPass();
	createCameraBuffer();
	createMeshTransformBuffer();
	createLightBuffer();
	createRandomRotationsBuffer();
	createDescriptorSetLayouts();
	createPipelines();

	_deviceResources->accelerationStructure.buildASModelDescription(
		_deviceResources->vertexBuffers, _deviceResources->indexBuffers, _deviceResources->verticesSize, _deviceResources->indicesSize, _meshTransforms);
	_deviceResources->accelerationStructure.buildAS(_deviceResources->device, _deviceResources->queue, _deviceResources->cmdBufferMainDeferred[0],
		pvrvk::BuildAccelerationStructureFlagsKHR::e_PREFER_FAST_TRACE_BIT_KHR | pvrvk::BuildAccelerationStructureFlagsKHR::e_ALLOW_UPDATE_BIT_KHR);

	createDescriptorSets();
	recordSecondaryCommandBuffers();
	recordMainCommandBuffer();

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanHybridSoftShadows::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
///	If the rendering context is lost, quitApplication() will not be called.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanHybridSoftShadows::quitApplication()
{
	_scene.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return Result::Success if no error occurred</returns>
pvr::Result VulkanHybridSoftShadows::renderFrame()
{
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	_swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameResourcesFences[_swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[_swapchainIndex]->reset();

	//  Handle user input and update object animations
	updateAnimation();

	// Update Acceleration Structure
	updateScene();

	// Upload dynamic data
	uploadDynamicSceneData();

	//--------------------
	// submit the main command buffer
	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags pipeWaitStage = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;

	submitInfo.commandBuffers = &_deviceResources->cmdBufferMainDeferred[_swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->imageAcquiredSemaphores[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.waitDstStageMask = &pipeWaitStage;
	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[_swapchainIndex]);

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->queue, _deviceResources->commandPool, _deviceResources->swapchain, _swapchainIndex, this->getScreenshotFileName(),
			_deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	//--------------------
	// Present
	pvrvk::PresentInfo presentInfo;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.imageIndices = &_swapchainIndex;
	_deviceResources->queue->present(presentInfo);

	_frameId = (_frameId + 1) % _numSwapImages;
	_frameNumber++;

	return pvr::Result::Success;
}

/// <summary>Updates the scene animation and takes the new mesh transforms and updates the TLAS.</summary>
void VulkanHybridSoftShadows::updateScene()
{
	pvr::assets::AnimationInstance& animInst = _scene->getAnimationInstance(0);

	//  Calculates the _frame number to animate in a time-based manner.
	//  get the time in milliseconds.
	_frame += static_cast<float>(getFrameTime()); // design-time target fps for animation

	if (_frame >= animInst.getTotalTimeInMs()) { _frame = 0; }

	// Sets the _scene animation to this _frame
	animInst.updateAnimation(_frame);

	for (uint32_t i = 0; i < _scene->getNumMeshes(); i++)
	{
		const pvr::assets::Model::Node& node = _scene->getNode(i);

		// get the transform matrix of the current mesh
		glm::mat4 transform = _scene->getWorldMatrix(node.getObjectId());

		_meshTransforms[i] = transform;
		_deviceResources->meshes[i].worldMatrix = transform;
	}

	_deviceResources->accelerationStructure.updateInstanceTransformData(_meshTransforms);

	pvrvk::CommandBuffer commandBuffer = _deviceResources->commandPool->allocateCommandBuffer();

	_deviceResources->accelerationStructure.buildTopLevelASAndInstances(_deviceResources->device, commandBuffer, _deviceResources->queue,
		pvrvk::BuildAccelerationStructureFlagsKHR::e_PREFER_FAST_TRACE_BIT_KHR | pvrvk::BuildAccelerationStructureFlagsKHR::e_ALLOW_UPDATE_BIT_KHR, true);
}

/// <summary>Creates descriptor set layouts.</summary>
void VulkanHybridSoftShadows::createDescriptorSetLayouts()
{
	// Common Descriptor Set Layout

	// Static per scene buffer
	pvrvk::DescriptorSetLayoutCreateInfo commonDescSetInfo;
	commonDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u,
		pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	// Per Light Data
	commonDescSetInfo.setBinding(1, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u,
		pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	// Static material data buffer
	commonDescSetInfo.setBinding(2, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	// Textures
	commonDescSetInfo.setBinding(3, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, static_cast<uint16_t>(_deviceResources->textures.size()),
		pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	// TLAS
	commonDescSetInfo.setBinding(4, pvrvk::DescriptorType::e_ACCELERATION_STRUCTURE_KHR, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	// Random Rotations
	commonDescSetInfo.setBinding(
		5, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1u, pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	// Mesh Transforms
	commonDescSetInfo.setBinding(6, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u,
		pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_COMPUTE_BIT);

	_deviceResources->commonDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(commonDescSetInfo);

	// GBuffer Descriptor Set Layout

	pvrvk::DescriptorSetLayoutCreateInfo gbufferDescSetInfo;
	gbufferDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u,
		pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	gbufferDescSetInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u,
		pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	gbufferDescSetInfo.setBinding(2, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u,
		pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_COMPUTE_BIT);

	_deviceResources->gbufferDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(gbufferDescSetInfo);
}

/// <summary>Creates descriptor sets.</summary>
void VulkanHybridSoftShadows::createDescriptorSets()
{
	// Scene Samplers

	pvrvk::SamplerCreateInfo samplerDesc;

	samplerDesc.wrapModeU = samplerDesc.wrapModeV = samplerDesc.wrapModeW = pvrvk::SamplerAddressMode::e_REPEAT;
	samplerDesc.minFilter = pvrvk::Filter::e_LINEAR;
	samplerDesc.magFilter = pvrvk::Filter::e_LINEAR;
	samplerDesc.mipMapMode = pvrvk::SamplerMipmapMode::e_LINEAR;
	pvrvk::Sampler samplerTrilinear = _deviceResources->device->createSampler(samplerDesc);

	samplerDesc.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
	samplerDesc.wrapModeU = samplerDesc.wrapModeV = samplerDesc.wrapModeW = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;
	pvrvk::Sampler samplerBilinearClampToEdge = _deviceResources->device->createSampler(samplerDesc);

	samplerDesc.minFilter = pvrvk::Filter::e_NEAREST;
	samplerDesc.magFilter = pvrvk::Filter::e_NEAREST;
	samplerDesc.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
	pvrvk::Sampler samplerNearestClampToEdge = _deviceResources->device->createSampler(samplerDesc);

	// Allocate Descriptor Sets

	_deviceResources->commonDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->commonDescriptorSetLayout);
	_deviceResources->gbufferDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->gbufferDescriptorSetLayout);

	_deviceResources->commonDescriptorSet->setObjectName("CommonDescriptorSet");
	_deviceResources->gbufferDescriptorSet->setObjectName("GBufferDescriptorSet");

	// Write Common Descriptor Set
	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->commonDescriptorSet, 0)
								.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->cameraBuffer, 0, _deviceResources->cameraBufferView.getDynamicSliceSize())));

	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->commonDescriptorSet, 1)
								.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->perLightBuffer, 0, _deviceResources->perLightBufferView.getDynamicSliceSize())));

	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->commonDescriptorSet, 2)
								.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->materialBuffer, 0, _deviceResources->materialBuffer->getSize())));

	pvrvk::WriteDescriptorSet textureSetWrite = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->commonDescriptorSet, 3);

	for (size_t i = 0; i < _deviceResources->textures.size(); i++)
		textureSetWrite.setImageInfo(i, pvrvk::DescriptorImageInfo(_deviceResources->textures[i].imageView, samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

	writeDescSets.push_back(textureSetWrite);

	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_ACCELERATION_STRUCTURE_KHR, _deviceResources->commonDescriptorSet, 4)
								.setAccelerationStructureInfo(0, _deviceResources->accelerationStructure.getTopLevelAccelerationStructure()));

	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->commonDescriptorSet, 5)
								.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->randomRotationsBuffer, 0, _deviceResources->randomRotationsBufferView.getSize())));

	writeDescSets.push_back(
		pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->commonDescriptorSet, 6)
			.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->perMeshBuffer, 0, _deviceResources->perMeshBufferView.getDynamicSliceSize() * _meshTransforms.size())));

	// Write GBuffer Descriptor Set
	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->gbufferDescriptorSet, 0)
								.setImageInfo(0,
									pvrvk::DescriptorImageInfo(_deviceResources->gbufferImages[FramebufferGBufferAttachments::Albedo_Shininess], samplerNearestClampToEdge,
										pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

	writeDescSets.push_back(
		pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->gbufferDescriptorSet, 1)
			.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->gbufferVisibilityMipMappedImage, samplerBilinearClampToEdge, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

	writeDescSets.push_back(
		pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->gbufferDescriptorSet, 2)
			.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->gbufferDepthStencilImage, samplerNearestClampToEdge, pvrvk::ImageLayout::e_DEPTH_STENCIL_READ_ONLY_OPTIMAL)));

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

/// <summary>Creates the pipeline for the G-Buffer pass.</summary>
void VulkanHybridSoftShadows::createGBufferPipelines()
{
	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

	pipeLayoutInfo.setDescSetLayout(0, _deviceResources->commonDescriptorSetLayout);
	pipeLayoutInfo.addPushConstantRange(pvrvk::PushConstantRange(pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, sizeof(uint32_t)));
	pipeLayoutInfo.addPushConstantRange(pvrvk::PushConstantRange(pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, sizeof(uint32_t), sizeof(uint32_t)));

	_deviceResources->gbufferPipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	pvrvk::GraphicsPipelineCreateInfo renderGBufferPipelineCreateInfo;
	renderGBufferPipelineCreateInfo.viewport.setViewportAndScissor(0,
		pvrvk::Viewport(
			0.0f, 0.0f, static_cast<float>(_deviceResources->swapchain->getDimension().getWidth()), static_cast<float>(_deviceResources->swapchain->getDimension().getHeight())),
		pvrvk::Rect2D(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight()));
	// enable back face culling
	renderGBufferPipelineCreateInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);

	// set counter clockwise winding order for front faces
	renderGBufferPipelineCreateInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

	// enable depth testing
	renderGBufferPipelineCreateInfo.depthStencil.enableDepthTest(true);
	renderGBufferPipelineCreateInfo.depthStencil.enableDepthWrite(true);

	// set the blend state for the colour attachments
	pvrvk::PipelineColorBlendAttachmentState renderGBufferColorAttachment;
	// number of colour blend states must equal number of colour attachments for the subpass
	renderGBufferPipelineCreateInfo.colorBlend.setAttachmentState(0, renderGBufferColorAttachment);
	renderGBufferPipelineCreateInfo.colorBlend.setAttachmentState(1, renderGBufferColorAttachment);

	// load and create appropriate shaders
	renderGBufferPipelineCreateInfo.vertexShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::GBufferVertexShader)->readToEnd<uint32_t>())));

	renderGBufferPipelineCreateInfo.fragmentShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::GBufferFragmentShader)->readToEnd<uint32_t>())));

	// setup vertex inputs
	renderGBufferPipelineCreateInfo.vertexInput.clear();

	// create vertex input attrib desc
	pvrvk::VertexInputAttributeDescription posAttrib;
	posAttrib.setBinding(0);
	posAttrib.setFormat(pvrvk::Format::e_R32G32B32_SFLOAT);
	posAttrib.setLocation(0);
	posAttrib.setOffset(0);

	pvrvk::VertexInputAttributeDescription normalAttrib;
	normalAttrib.setBinding(0);
	normalAttrib.setFormat(pvrvk::Format::e_R32G32B32_SFLOAT);
	normalAttrib.setLocation(1);
	normalAttrib.setOffset(offsetof(pvr::utils::ASVertexFormat, nrm));

	pvrvk::VertexInputAttributeDescription texCoordAttrib;
	texCoordAttrib.setBinding(0);
	texCoordAttrib.setFormat(pvrvk::Format::e_R32G32_SFLOAT);
	texCoordAttrib.setLocation(2);
	texCoordAttrib.setOffset(offsetof(pvr::utils::ASVertexFormat, texCoord));

	pvrvk::VertexInputAttributeDescription tangentAttrib;
	tangentAttrib.setBinding(0);
	tangentAttrib.setFormat(pvrvk::Format::e_R32G32B32_SFLOAT);
	tangentAttrib.setLocation(3);
	tangentAttrib.setOffset(offsetof(pvr::utils::ASVertexFormat, tangent));

	pvrvk::VertexInputBindingDescription binding;
	binding.setBinding(0);
	binding.setInputRate(pvrvk::VertexInputRate::e_VERTEX);
	binding.setStride(sizeof(pvr::utils::ASVertexFormat));

	renderGBufferPipelineCreateInfo.vertexInput.addInputAttribute(posAttrib);
	renderGBufferPipelineCreateInfo.vertexInput.addInputAttribute(normalAttrib);
	renderGBufferPipelineCreateInfo.vertexInput.addInputAttribute(texCoordAttrib);
	renderGBufferPipelineCreateInfo.vertexInput.addInputAttribute(tangentAttrib);
	renderGBufferPipelineCreateInfo.vertexInput.addInputBinding(binding);

	pvrvk::PipelineInputAssemblerStateCreateInfo inputAssembler;
	inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);
	renderGBufferPipelineCreateInfo.inputAssembler = inputAssembler;

	// renderpass/subpass
	renderGBufferPipelineCreateInfo.renderPass = _deviceResources->gbufferRenderPass;
	renderGBufferPipelineCreateInfo.subpass = RenderPassSubpasses::GBuffer;

	// enable stencil testing
	pvrvk::StencilOpState stencilState;

	// only replace stencil buffer when the depth test passes
	stencilState.setFailOp(pvrvk::StencilOp::e_KEEP);
	stencilState.setDepthFailOp(pvrvk::StencilOp::e_KEEP);
	stencilState.setPassOp(pvrvk::StencilOp::e_REPLACE);
	stencilState.setCompareOp(pvrvk::CompareOp::e_ALWAYS);

	// set stencil reference to 1
	stencilState.setReference(1);

	// enable stencil writing
	stencilState.setWriteMask(0xFF);

	// enable the stencil tests
	renderGBufferPipelineCreateInfo.depthStencil.enableStencilTest(true);
	// set stencil states
	renderGBufferPipelineCreateInfo.depthStencil.setStencilFront(stencilState);
	renderGBufferPipelineCreateInfo.depthStencil.setStencilBack(stencilState);

	renderGBufferPipelineCreateInfo.pipelineLayout = _deviceResources->gbufferPipelineLayout;
	_deviceResources->gbufferPipeline = _deviceResources->device->createGraphicsPipeline(renderGBufferPipelineCreateInfo, _deviceResources->pipelineCache);
	_deviceResources->gbufferPipeline->setObjectName("GBufferGraphicsPipeline");
}

/// <summary>Creates the pipeline for the Deferred shading pass.</summary>
void VulkanHybridSoftShadows::createDeferredShadingPipelines()
{
	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

	pipeLayoutInfo.setDescSetLayout(0, _deviceResources->commonDescriptorSetLayout);
	pipeLayoutInfo.setDescSetLayout(1, _deviceResources->gbufferDescriptorSetLayout);

	_deviceResources->deferredShadingPipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	pvrvk::GraphicsPipelineCreateInfo pipelineCreateInfo;

	pipelineCreateInfo.viewport.setViewportAndScissor(0,
		pvrvk::Viewport(
			0.0f, 0.0f, static_cast<float>(_deviceResources->swapchain->getDimension().getWidth()), static_cast<float>(_deviceResources->swapchain->getDimension().getHeight())),
		pvrvk::Rect2D(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight()));

	// enable front face culling
	pipelineCreateInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_NONE);

	// set counter clockwise winding order for front faces
	pipelineCreateInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

	// enable stencil testing
	pvrvk::StencilOpState stencilState;

	// only replace stencil buffer when the depth test passes
	stencilState.setFailOp(pvrvk::StencilOp::e_KEEP);
	stencilState.setDepthFailOp(pvrvk::StencilOp::e_KEEP);
	stencilState.setPassOp(pvrvk::StencilOp::e_REPLACE);
	stencilState.setCompareOp(pvrvk::CompareOp::e_ALWAYS);

	// set stencil reference to 1
	stencilState.setReference(1);

	// disable stencil writing
	stencilState.setWriteMask(0);

	// blend state
	pvrvk::PipelineColorBlendAttachmentState colorAttachmentState;

	colorAttachmentState.setBlendEnable(false);
	pipelineCreateInfo.colorBlend.setAttachmentState(0, colorAttachmentState);

	// enable the stencil tests
	pipelineCreateInfo.depthStencil.enableStencilTest(false);
	// set stencil states
	pipelineCreateInfo.depthStencil.setStencilFront(stencilState);
	pipelineCreateInfo.depthStencil.setStencilBack(stencilState);

	// enable depth testing
	pipelineCreateInfo.pipelineLayout = _deviceResources->deferredShadingPipelineLayout;
	pipelineCreateInfo.depthStencil.enableDepthTest(false);
	pipelineCreateInfo.depthStencil.enableDepthWrite(false);

	// setup vertex inputs
	pipelineCreateInfo.vertexInput.clear();
	pipelineCreateInfo.inputAssembler = pvrvk::PipelineInputAssemblerStateCreateInfo();

	// renderpass/subpass
	pipelineCreateInfo.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();

	// load and create appropriate shaders
	pipelineCreateInfo.vertexShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::FullscreenTriangleVertexShader)->readToEnd<uint32_t>())));
	pipelineCreateInfo.fragmentShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::DeferredShadingFragmentShader)->readToEnd<uint32_t>())));

	_deviceResources->defferedShadingPipeline = _deviceResources->device->createGraphicsPipeline(pipelineCreateInfo, _deviceResources->pipelineCache);
	_deviceResources->defferedShadingPipeline->setObjectName("DefferedShadingGraphicsPipeline");
}

/// <summary>Create the pipelines for this example.</summary>
void VulkanHybridSoftShadows::createPipelines()
{
	createGBufferPipelines();
	createDeferredShadingPipelines();
}

/// <summary>Create the G-Buffer pass framebuffer and renderpass.</summary>
void VulkanHybridSoftShadows::createFramebufferAndRenderPass()
{
	const pvrvk::Extent3D& dimension = pvrvk::Extent3D(_deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight(), 1u);

	const pvrvk::Format renderpassStorageFormats[FramebufferGBufferAttachments::Count] = { pvrvk::Format::e_R8G8B8A8_UNORM, pvrvk::Format::e_R16G16B16A16_SFLOAT };

	// Create images
	for (int i = 0; i < FramebufferGBufferAttachments::Count; i++)
	{
		pvrvk::ImageUsageFlags usageFlags = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT;
		uint32_t mipLevels = 1;

		if (i == FramebufferGBufferAttachments::Normal_Visibility_HitDistance)
		{
			usageFlags |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_DST_BIT;
			mipLevels = 4;
		}

		pvrvk::Image image = pvr::utils::createImage(_deviceResources->device,
			pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, renderpassStorageFormats[i], dimension, usageFlags, mipLevels), pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);

		_deviceResources->gbufferImages[i] = _deviceResources->device->createImageView(
			pvrvk::ImageViewCreateInfo(image, pvrvk::ImageViewType::e_2D, image->getFormat(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0, 1)));

		if (i == FramebufferGBufferAttachments::Normal_Visibility_HitDistance)
		{
			_deviceResources->gbufferVisibilityMipMappedImage = _deviceResources->device->createImageView(
				pvrvk::ImageViewCreateInfo(image, pvrvk::ImageViewType::e_2D, image->getFormat(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0, mipLevels)));
		}
	}

	const std::vector<pvrvk::Format> preferredDepthFormats = { pvrvk::Format::e_D24_UNORM_S8_UINT, pvrvk::Format::e_D32_SFLOAT_S8_UINT, pvrvk::Format::e_D16_UNORM_S8_UINT };
	const pvrvk::Format depthStencilFormat = pvr::utils::getSupportedDepthStencilFormat(_deviceResources->device, preferredDepthFormats);

	pvrvk::Image depthImage = pvr::utils::createImage(_deviceResources->device,
		pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, depthStencilFormat, dimension, pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _deviceResources->vmaAllocator,
		pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);

	_deviceResources->gbufferDepthStencilImage = _deviceResources->device->createImageView(
		pvrvk::ImageViewCreateInfo(depthImage, pvrvk::ImageViewType::e_2D, depthImage->getFormat(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_DEPTH_BIT)));

	// Create render pass
	pvrvk::RenderPassCreateInfo renderPassInfo;

	pvrvk::AttachmentDescription gbufferAttachment0 =
		pvrvk::AttachmentDescription::createColorDescription(renderpassStorageFormats[FramebufferGBufferAttachments::Albedo_Shininess], pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT);
	pvrvk::AttachmentDescription gbufferAttachment1 =
		pvrvk::AttachmentDescription::createColorDescription(renderpassStorageFormats[FramebufferGBufferAttachments::Normal_Visibility_HitDistance], pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT);
	pvrvk::AttachmentDescription gbufferAttachmentDepth = pvrvk::AttachmentDescription::createDepthStencilDescription(depthStencilFormat, pvrvk::ImageLayout::e_UNDEFINED,
		pvrvk::ImageLayout::e_DEPTH_STENCIL_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE);

	pvrvk::AttachmentReference gbufferAttachmentRef0 = pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
	pvrvk::AttachmentReference gbufferAttachmentRef1 = pvrvk::AttachmentReference(1, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
	pvrvk::AttachmentReference gbufferAttachmentRefDepth = pvrvk::AttachmentReference(2, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	pvrvk::SubpassDescription subpassDesc =
		pvrvk::SubpassDescription().setColorAttachmentReference(0, gbufferAttachmentRef0).setColorAttachmentReference(1, gbufferAttachmentRef1).setDepthStencilAttachmentReference(gbufferAttachmentRefDepth);

	pvrvk::SubpassDependency dependency[2];

	dependency[0].setSrcSubpass(VK_SUBPASS_EXTERNAL);
	dependency[0].setDstSubpass(0);
	dependency[0].setSrcStageMask(pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT | pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT);
	dependency[0].setDstStageMask(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT | pvrvk::PipelineStageFlags::e_EARLY_FRAGMENT_TESTS_BIT);
	dependency[0].setSrcAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
	dependency[0].setDstAccessMask(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT | pvrvk::AccessFlags::e_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
	dependency[0].setDependencyFlags(pvrvk::DependencyFlags::e_BY_REGION_BIT);

	dependency[1].setSrcSubpass(0);
	dependency[1].setDstSubpass(VK_SUBPASS_EXTERNAL);
	dependency[1].setSrcStageMask(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT | pvrvk::PipelineStageFlags::e_LATE_FRAGMENT_TESTS_BIT);
	dependency[1].setDstStageMask(pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT | pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT);
	dependency[1].setSrcAccessMask(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT | pvrvk::AccessFlags::e_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
	dependency[1].setDstAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
	dependency[1].setDependencyFlags(pvrvk::DependencyFlags::e_BY_REGION_BIT);

	pvrvk::RenderPassCreateInfo renderPassCreateInfo = pvrvk::RenderPassCreateInfo()
														   .setAttachmentDescription(0, gbufferAttachment0)
														   .setAttachmentDescription(1, gbufferAttachment1)
														   .setAttachmentDescription(2, gbufferAttachmentDepth)
														   .setSubpass(0, subpassDesc)
														   .addSubpassDependencies(dependency, 2);

	pvrvk::ImageView imageViews[] = { _deviceResources->gbufferImages[0], _deviceResources->gbufferImages[1], _deviceResources->gbufferDepthStencilImage };

	_deviceResources->gbufferRenderPass = _deviceResources->device->createRenderPass(renderPassCreateInfo);
	_deviceResources->gbufferRenderPass->setObjectName("GBufferRenderPass");

	_deviceResources->gbufferFramebuffer = _deviceResources->device->createFramebuffer(
		pvrvk::FramebufferCreateInfo(dimension.getWidth(), dimension.getHeight(), 1, _deviceResources->gbufferRenderPass, 3, &imageViews[0]));
}

/// <summary>Add a texture to the list of textures if it doesn't already exist.</summary>
/// <param name="texturePath">String containing the path to the texture.</param>
/// <returns>Return the index of the added texture.</returns>
uint32_t VulkanHybridSoftShadows::getTextureIndex(const std::string texturePath)
{
	// search in existing textures
	for (uint32_t i = 0; i < static_cast<uint32_t>(_deviceResources->textures.size()); i++)
	{
		if (_deviceResources->textures[i].name == texturePath) return i;
	}

	// texture not added yet
	_deviceResources->textures.push_back(TextureAS{});
	uint32_t texIndex = static_cast<uint32_t>(_deviceResources->textures.size()) - 1;
	_deviceResources->textures[texIndex].name = texturePath;
	return texIndex;
}

/// <summary>Takes the list of populated textures used in the scene and loads them into memory, uploads them into a Vulkan image and creates image views.</summary>
/// <param name="uploadCmd">Command Buffer used to record the image upload commands.</param>
void VulkanHybridSoftShadows::createTextures(pvrvk::CommandBuffer& uploadCmd)
{
	// load textures
	for (TextureAS& tex : _deviceResources->textures)
	{
		pvr::Texture textureObject = pvr::textureLoad(*getAssetStream(tex.name.c_str()), pvr::TextureFileFormat::PVR);

		tex.imageView = pvr::utils::uploadImageAndView(_deviceResources->device, textureObject, true, uploadCmd, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
		tex.image = tex.imageView->getImage();
	}

	// dummy texture
	if (_deviceResources->textures.size() == 0)
	{
		TextureAS dummyTexture;
		dummyTexture.name = "empty";
		std::array<unsigned char, 8> color{ 255u, 255u, 255u, 255u, 255u, 255u, 255u, 255u };
		pvr::Texture tex(pvr::TextureHeader(pvr::PixelFormat::RGBA_8888(), 1, 2), color.data()); // height = 2 so the sdk interprets as 2d image

		// image
		dummyTexture.imageView =
			pvr::utils::uploadImageAndView(_deviceResources->device, tex, false, uploadCmd, pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL);
		dummyTexture.image = dummyTexture.imageView->getImage();

		_deviceResources->textures.push_back(dummyTexture);
	}
}

/// <summary>Populates the structure that holds the light data.</summary>
void VulkanHybridSoftShadows::initializeLights()
{
	assert(_scene->getNumLights() != 0);

	glm::vec4 lightPosition;
	_scene->getLightPosition(0, lightPosition);
	_lightPos = glm::vec3(lightPosition.x, lightPosition.y, lightPosition.z);

	pvr::assets::Light light = _scene->getLight(0);

	_lightData.vLightPosition = lightPosition;
	_lightData.vLightColor = glm::vec4(light.getColor(), 1.0f);
	_lightData.vAmbientColor = glm::vec4(0.05f, 0.05f, 0.05f, 1.0);
	_lightData.vLightDirection = glm::vec4(glm::normalize(_satyrCenter - glm::vec3(lightPosition)), 1.0);
	_lightData.lightRadius = lightRadiusList[_lightRadusIdx % 5];
	_lightData.penumbraAngle = 2.0f * (_lightData.lightRadius / 2.0f);
	_lightData.innerConeAngle = cos(glm::radians(LIGHT_INNER_CONE_ANGLE));
	_lightData.outerConeAngle = cos(glm::radians(LIGHT_OUTER_CONE_ANGLE));
	_lightData.numShadowRays = NUM_SHADOW_RAYS;
}

/// <summary>Loads the mesh data required for this example into vertex buffer objects.</summary>
/// <param name="uploadCmd">Command Buffer used to record the buffer upload commands.</param>
void VulkanHybridSoftShadows::createModelBuffers(pvrvk::CommandBuffer& uploadCmd)
{
	struct Material
	{
		glm::ivec4 textureIndices = glm::ivec4(-1);
		glm::vec4 baseColor = glm::vec4(1.0f);
		glm::vec4 shininess = glm::vec4(0.0f);
	};

	uint32_t numMeshes = _scene->getNumMeshes();

	std::vector<Material> materials;
	_deviceResources->meshes.reserve(numMeshes);
	_deviceResources->vertexBuffers.reserve(numMeshes);
	_deviceResources->indexBuffers.reserve(numMeshes);
	_deviceResources->verticesSize.reserve(numMeshes);
	_deviceResources->indicesSize.reserve(numMeshes);
	_meshTransforms.reserve(numMeshes);

	std::unordered_map<uint32_t, SceneNodes::MeshNodes> materialIdToMeshNode;

	// populate vertices, indices and material indices
	glm::vec3 minExtents = glm::vec3(INFINITY);
	glm::vec3 maxExtents = glm::vec3(-INFINITY);

	for (uint32_t meshIdx = 0; meshIdx < numMeshes; meshIdx++)
	{
		std::vector<pvr::utils::ASVertexFormat> vertices;
		std::vector<uint32_t> indices;
		std::vector<uint32_t> materialIndices;

		pvr::assets::Mesh& mesh = _scene->getMesh(meshIdx);

		// populate mesh
		const pvr::assets::Model::Node& node = _scene->getNode(meshIdx);

		// get the transform matrix of the current mesh
		glm::mat4 modelMat = _scene->getWorldMatrix(node.getObjectId());
		_meshTransforms.push_back(modelMat);

		// indices
		uint32_t numIndices = mesh.getNumIndices();
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

		// vertices
		pvr::StridedBuffer verticesWrapper = mesh.getVertexData(0);
		uint32_t vertexStrideBytes = static_cast<uint32_t>(verticesWrapper.stride);
		uint32_t vertexStrideFloats = vertexStrideBytes / sizeof(float);
		uint32_t numVertices = static_cast<uint32_t>(verticesWrapper.size()) / vertexStrideBytes;

		auto verticesStart = reinterpret_cast<float*>(verticesWrapper.data());
		auto verticesEnd = verticesStart + static_cast<size_t>(numVertices) * vertexStrideFloats;
		uint32_t vertexIndex = 0;
		for (auto v = verticesStart; v < verticesEnd; v += vertexStrideFloats)
		{
			glm::vec3 position = glm::vec3(v[0], v[1], v[2]);

			vertices.insert(vertices.begin() + vertexIndex,
				{
					position, // position
					glm::vec3(v[3], v[4], v[5]), // normals
					glm::vec2(v[6], v[7]), // texture coordinates
					glm::vec3(1.0) // tangent
				});
			vertexIndex++;

			glm::vec3 transformedPos = modelMat * glm::vec4(position, 1.0f);

			minExtents = glm::min(minExtents, transformedPos);
			maxExtents = glm::max(maxExtents, transformedPos);
		}

		MeshAS meshAS = { static_cast<int32_t>(node.getMaterialIndex()), 0, static_cast<int32_t>(numIndices), modelMat, pvrvk::IndexType::e_UINT32 };

		_deviceResources->meshes.push_back(meshAS);

		// material indices
		std::vector<uint32_t> materialIndicesTemp(static_cast<size_t>(numIndices / 3 + (numIndices % 3 == 0 ? 0 : 1)), meshAS.materialIdx);
		materialIndices.insert(materialIndices.end(), materialIndicesTemp.begin(), materialIndicesTemp.end());

		// create vertex buffer
		pvrvk::BufferCreateInfo vertexBufferInfo;
		vertexBufferInfo.setSize(sizeof(pvr::utils::ASVertexFormat) * vertices.size());
		vertexBufferInfo.setUsageFlags(pvrvk::BufferUsageFlags::e_VERTEX_BUFFER_BIT | pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT |
			pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT | pvrvk::BufferUsageFlags::e_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);

		_deviceResources->vertexBuffers.push_back(pvr::utils::createBuffer(_deviceResources->device, vertexBufferInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			pvrvk::MemoryPropertyFlags::e_NONE, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE, pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT));
		_deviceResources->vertexBuffers.back()->setObjectName("VBO");

		pvr::utils::updateBufferUsingStagingBuffer(
			_deviceResources->device, _deviceResources->vertexBuffers[meshIdx], uploadCmd, vertices.data(), 0, sizeof(pvr::utils::ASVertexFormat) * vertices.size());

		// create index buffer
		pvrvk::BufferCreateInfo indexBufferInfo;
		indexBufferInfo.setSize(sizeof(uint32_t) * indices.size());
		indexBufferInfo.setUsageFlags(pvrvk::BufferUsageFlags::e_INDEX_BUFFER_BIT | pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT |
			pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT | pvrvk::BufferUsageFlags::e_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);

		_deviceResources->indexBuffers.push_back(pvr::utils::createBuffer(_deviceResources->device, indexBufferInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			pvrvk::MemoryPropertyFlags::e_NONE, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE, pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT));
		_deviceResources->vertexBuffers.back()->setObjectName("IBO");

		pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, _deviceResources->indexBuffers[meshIdx], uploadCmd, indices.data(), 0, sizeof(uint32_t) * indices.size());

		_deviceResources->verticesSize.push_back(static_cast<int32_t>(vertices.size()));
		_deviceResources->indicesSize.push_back(static_cast<int32_t>(indices.size()));

		materialIdToMeshNode[meshAS.materialIdx] = static_cast<SceneNodes::MeshNodes>(meshIdx);
	}

	_satyrCenter = (maxExtents + minExtents) / 2.0f;

	// populate material data
	for (uint32_t i = 0; i < _scene->getNumMaterials(); i++)
	{
		auto& material = _scene->getMaterial(i);

		Material mat;

		int32_t diffuseIndex = material.defaultSemantics().getDiffuseTextureIndex();

		if (diffuseIndex != -1)
		{
			std::string path = _scene->getTexture(diffuseIndex).getName().c_str();
			pvr::assets::helper::getTextureNameWithExtension(path, _astcSupported);

			mat.textureIndices.x = getTextureIndex(path);
		}
		else
		{
			mat.baseColor = glm::vec4(material.defaultSemantics().getDiffuse(), 1.0f);
			mat.baseColor = glm::vec4(glm::pow(glm::vec3(mat.baseColor.x, mat.baseColor.y, mat.baseColor.z), glm::vec3(2.2f)), 0.0f); // Srgb to linear
		}

		if (materialIdToMeshNode[i] == SceneNodes::MeshNodes::Satyr)
			mat.shininess.x = 15.0f;
		else if (materialIdToMeshNode[i] == SceneNodes::MeshNodes::Table)
			mat.shininess.x = 5.0f;
		else if (materialIdToMeshNode[i] == SceneNodes::MeshNodes::BrickWall)
			mat.shininess.x = 5.0f;
		else if (materialIdToMeshNode[i] == SceneNodes::MeshNodes::Spheres)
			mat.shininess.x = 12.0f;

		materials.emplace_back(mat);
	}
	// If there were none, add a default
	if (materials.empty()) materials.emplace_back(Material());

	// create material data buffer
	pvrvk::BufferCreateInfo materialColorBufferInfo;
	materialColorBufferInfo.setSize(sizeof(Material) * materials.size());
	materialColorBufferInfo.setUsageFlags(pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT);
	_deviceResources->materialBuffer = pvr::utils::createBuffer(_deviceResources->device, materialColorBufferInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT);
	_deviceResources->vertexBuffers.back()->setObjectName("MaterialSBO");
	pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, _deviceResources->materialBuffer, uploadCmd, materials.data(), 0, sizeof(Material) * materials.size());
}

/// <summary>Creates the scene wide buffer used throughout the demo.</summary>
void VulkanHybridSoftShadows::createCameraBuffer()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(BufferEntryNames::PerScene::ViewMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::PerScene::ProjectionMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::PerScene::InvViewProjectionMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::PerScene::EyePosition, pvr::GpuDatatypes::vec4);
	desc.addElement(BufferEntryNames::PerScene::ClipPlanes, pvr::GpuDatatypes::vec4);
	desc.addElement(BufferEntryNames::PerScene::FrameIdx, pvr::GpuDatatypes::uinteger);

	_deviceResources->cameraBufferView.initDynamic(desc, _numSwapImages, pvr::BufferUsageFlags::UniformBuffer,
		static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

	_deviceResources->cameraBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->cameraBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->cameraBuffer->setObjectName("CameraUBO");

	_deviceResources->cameraBufferView.pointToMappedMemory(_deviceResources->cameraBuffer->getDeviceMemory()->getMappedData());
}

/// <summary>Creates the scene wide buffer used throughout the demo.</summary>
void VulkanHybridSoftShadows::createMeshTransformBuffer()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(BufferEntryNames::PerMesh::WorldMatrix, pvr::GpuDatatypes::mat4x4, static_cast<uint32_t>(_meshTransforms.size()));

	_deviceResources->perMeshBufferView.initDynamic(desc, _deviceResources->swapchain->getSwapchainLength() * static_cast<uint32_t>(_meshTransforms.size()), pvr::BufferUsageFlags::UniformBuffer,
		static_cast<uint64_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

	_deviceResources->perMeshBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->perMeshBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->perMeshBuffer->setObjectName("PerMeshUBO");

	_deviceResources->perMeshBufferView.pointToMappedMemory(_deviceResources->perMeshBuffer->getDeviceMemory()->getMappedData());
}

double fRand(double fMin, double fMax)
{
	double f = (double)rand() / RAND_MAX;
	return fMin + f * (fMax - fMin);
}

/// <summary>Creates a buffer and populates it with a list of random rotations which are used to rotate the Poisson Disk samples used in denoising.</summary>
void VulkanHybridSoftShadows::createRandomRotationsBuffer()
{
	uint32_t poissonDiscNumberOfRandomnessValues = 32 * 32;
	std::vector<glm::vec4> rotations(poissonDiscNumberOfRandomnessValues);

	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement("rotations", pvr::GpuDatatypes::vec4, poissonDiscNumberOfRandomnessValues);

	_deviceResources->randomRotationsBufferView.init(desc);
	_deviceResources->randomRotationsBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->randomRotationsBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->randomRotationsBuffer->setObjectName("RandomRotationsUBO");

	float currentRandom = 0.0f;

	srand(34563464);

	// Generate the 2D screen-space grid, with each entry having the next value in the halton sequence
	for (unsigned int i = 0; i < poissonDiscNumberOfRandomnessValues; i++)
	{
		currentRandom = static_cast<float>(fRand(0.0f, 1.0f)) * static_cast<float>(M_PI) * 2.0f;

		rotations[i] = glm::vec4(cos(currentRandom), sin(currentRandom), 0.0, 0.0);
	}

	memcpy(_deviceResources->randomRotationsBuffer->getDeviceMemory()->getMappedData(), rotations.data(), sizeof(glm::vec4) * rotations.size());
}

/// <summary>Creates a buffer to store the previously initialized light data.</summary>
void VulkanHybridSoftShadows::createLightBuffer()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(BufferEntryNames::PerLightData::LightColor, pvr::GpuDatatypes::vec4);
	desc.addElement(BufferEntryNames::PerLightData::LightPosition, pvr::GpuDatatypes::vec4);
	desc.addElement(BufferEntryNames::PerLightData::AmbientColor, pvr::GpuDatatypes::vec4);
	desc.addElement(BufferEntryNames::PerLightData::LightDirection, pvr::GpuDatatypes::vec4);
	desc.addElement(BufferEntryNames::PerLightData::PenumbraAngle, pvr::GpuDatatypes::Float);
	desc.addElement(BufferEntryNames::PerLightData::LightRadius, pvr::GpuDatatypes::Float);
	desc.addElement(BufferEntryNames::PerLightData::InnerConeAngle, pvr::GpuDatatypes::Float);
	desc.addElement(BufferEntryNames::PerLightData::OuterConeAngle, pvr::GpuDatatypes::Float);
	desc.addElement(BufferEntryNames::PerLightData::NumShadowRays, pvr::GpuDatatypes::Integer);

	_deviceResources->perLightBufferView.initDynamic(desc, _numSwapImages, pvr::BufferUsageFlags::UniformBuffer,
		static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

	_deviceResources->perLightBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->perLightBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->perLightBuffer->setObjectName("PerLightUBO");

	_deviceResources->perLightBufferView.pointToMappedMemory(_deviceResources->perLightBuffer->getDeviceMemory()->getMappedData());
}

/// <summary>Upload the dynamic data that can change per frame.</summary>
void VulkanHybridSoftShadows::uploadDynamicSceneData()
{
	// static scene properties buffer
	uint32_t dynamicSliceIdx = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->cameraBufferView.getElementByName(BufferEntryNames::PerScene::ViewMatrix, 0, dynamicSliceIdx).setValue(_viewMatrix);
	_deviceResources->cameraBufferView.getElementByName(BufferEntryNames::PerScene::ProjectionMatrix, 0, dynamicSliceIdx).setValue(_projectionMatrix);
	_deviceResources->cameraBufferView.getElementByName(BufferEntryNames::PerScene::InvViewProjectionMatrix, 0, dynamicSliceIdx).setValue(glm::inverse(_viewProjectionMatrix));
	_deviceResources->cameraBufferView.getElementByName(BufferEntryNames::PerScene::EyePosition, 0, dynamicSliceIdx).setValue(glm::vec4(_eyePos, 0.0f));
	_deviceResources->cameraBufferView.getElementByName(BufferEntryNames::PerScene::ClipPlanes, 0, dynamicSliceIdx).setValue(glm::vec4(_nearClipDistance, _farClipDistance, 0.0f, 0.0f));
	_deviceResources->cameraBufferView.getElementByName(BufferEntryNames::PerScene::FrameIdx, 0, dynamicSliceIdx).setValue(_frameNumber);

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->cameraBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->cameraBuffer->getDeviceMemory()->flushRange(
			_deviceResources->cameraBufferView.getDynamicSliceOffset(dynamicSliceIdx), _deviceResources->cameraBufferView.getDynamicSliceSize());
	}

	_lightData.vLightDirection = glm::vec4(glm::normalize(_satyrCenter - glm::vec3(_lightData.vLightPosition)), 1.0);
	_lightData.lightRadius = lightRadiusList[_lightRadusIdx % 5];
	_lightData.penumbraAngle = 2.0f * (_lightData.lightRadius / 2.0f);

	// per light data buffer
	_deviceResources->perLightBufferView.getElementByName(BufferEntryNames::PerLightData::LightColor, 0, dynamicSliceIdx).setValue(_lightData.vLightColor);
	_deviceResources->perLightBufferView.getElementByName(BufferEntryNames::PerLightData::LightPosition, 0, dynamicSliceIdx).setValue(_lightData.vLightPosition);
	_deviceResources->perLightBufferView.getElementByName(BufferEntryNames::PerLightData::AmbientColor, 0, dynamicSliceIdx).setValue(_lightData.vAmbientColor);
	_deviceResources->perLightBufferView.getElementByName(BufferEntryNames::PerLightData::LightDirection, 0, dynamicSliceIdx).setValue(_lightData.vLightDirection);
	_deviceResources->perLightBufferView.getElementByName(BufferEntryNames::PerLightData::PenumbraAngle, 0, dynamicSliceIdx).setValue(_lightData.penumbraAngle);
	_deviceResources->perLightBufferView.getElementByName(BufferEntryNames::PerLightData::LightRadius, 0, dynamicSliceIdx).setValue(_lightData.lightRadius);
	_deviceResources->perLightBufferView.getElementByName(BufferEntryNames::PerLightData::InnerConeAngle, 0, dynamicSliceIdx).setValue(_lightData.innerConeAngle);
	_deviceResources->perLightBufferView.getElementByName(BufferEntryNames::PerLightData::OuterConeAngle, 0, dynamicSliceIdx).setValue(_lightData.outerConeAngle);
	_deviceResources->perLightBufferView.getElementByName(BufferEntryNames::PerLightData::NumShadowRays, 0, dynamicSliceIdx).setValue(_lightData.numShadowRays);

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->perLightBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->perLightBuffer->getDeviceMemory()->flushRange(
			_deviceResources->perLightBufferView.getDynamicSliceOffset(dynamicSliceIdx), _deviceResources->perLightBufferView.getDynamicSliceSize());
	}

	// upload per mesh data
	for (uint32_t i = 0; i < _meshTransforms.size(); i++)
	{
		_deviceResources->perMeshBufferView.getElementByName(BufferEntryNames::PerMesh::WorldMatrix, i, dynamicSliceIdx).setValue(_meshTransforms[i]);
	}

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->perMeshBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->perMeshBuffer->getDeviceMemory()->flushRange(
			_deviceResources->perMeshBufferView.getDynamicSliceOffset(dynamicSliceIdx), _deviceResources->perMeshBufferView.getDynamicSliceSize());
	}
}

/// <summary>Updates animation variables and camera matrices.</summary>
void VulkanHybridSoftShadows::updateAnimation()
{
	glm::vec3 vFrom, vTo, vUp;
	float fov;
	_scene->getCameraProperties(static_cast<uint32_t>(SceneNodes::Cameras::SceneCamera), fov, vFrom, vTo, vUp);

	static float lightYOffset = 0.0f;
	static float lightXOffset = 0.0f;

	if (_animateLight)
	{
		const float MOVEMENT_RANGE_X = 150.0f;
		const float MOVEMENT_RANGE_Y = 100.0f;

		_animationTime += getFrameTime() / 1000.0f;

		lightXOffset = sin(_animationTime) * MOVEMENT_RANGE_X;
		lightYOffset = sin(_animationTime) * MOVEMENT_RANGE_Y;
	}

	_lightData.vLightPosition = glm::vec4(_lightPos.x + lightXOffset, _lightPos.y + lightYOffset, _lightPos.z, 0.0f);

	_eyePos = vFrom;

	_viewMatrix = glm::lookAt(vFrom, vTo, glm::vec3(0.0f, 1.0f, 0.0f));
	_viewProjectionMatrix = _projectionMatrix * _viewMatrix;
	_inverseViewMatrix = glm::inverse(_viewMatrix);
}

/// <summary>Records main command buffer.</summary>
void VulkanHybridSoftShadows::recordMainCommandBuffer()
{
	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		// Record deferred version
		_deviceResources->cmdBufferMainDeferred[i]->begin();

		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->cmdBufferMainDeferred[i], pvrvk::DebugUtilsLabel("MainRenderPass"));

		pvrvk::Rect2D renderArea(0, 0, _windowWidth, _windowHeight);

		// specify a clear colour per attachment
		const uint32_t numClearValues = FramebufferGBufferAttachments::Count + 1;

		pvrvk::ClearValue clearValues[numClearValues] = { pvrvk::ClearValue(0.0, 0.0, 0.0, 0.0f), pvrvk::ClearValue(0.0, 0.0, 0.0, 0.0f), pvrvk::ClearValue(1.f, 0u) };

		// Render G-Buffer
		_deviceResources->cmdBufferMainDeferred[i]->beginRenderPass(_deviceResources->gbufferFramebuffer, renderArea, false, clearValues, numClearValues);

		_deviceResources->cmdBufferMainDeferred[i]->executeCommands(_deviceResources->cmdBufferGBuffer[i]);

		_deviceResources->cmdBufferMainDeferred[i]->endRenderPass();

		// Downsample
		_deviceResources->cmdBufferMainDeferred[i]->executeCommands(_deviceResources->cmdBufferDownsample[i]);

		// Deferred shading + UI
		_deviceResources->cmdBufferMainDeferred[i]->beginRenderPass(_deviceResources->onScreenFramebuffer[i], renderArea, false, clearValues, numClearValues);

		_deviceResources->cmdBufferMainDeferred[i]->executeCommands(_deviceResources->cmdBufferDeferredShading[i]);

		_deviceResources->cmdBufferMainDeferred[i]->endRenderPass();

		pvr::utils::endCommandBufferDebugLabel(_deviceResources->cmdBufferMainDeferred[i]);

		_deviceResources->cmdBufferMainDeferred[i]->end();
	}
}

/// <summary>Record all the secondary command buffers.</summary>
void VulkanHybridSoftShadows::recordSecondaryCommandBuffers()
{
	pvrvk::Rect2D renderArea(0, 0, _framebufferWidth, _framebufferHeight);
	if ((_framebufferWidth != _windowWidth) || (_framebufferHeight != _windowHeight))
	{
		renderArea = pvrvk::Rect2D(_viewportOffsets[0], _viewportOffsets[1], _framebufferWidth, _framebufferHeight);
	}

	pvrvk::ClearValue clearStenciLValue(pvrvk::ClearValue::createStencilClearValue(0));

	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		_deviceResources->cmdBufferGBuffer[i]->begin(_deviceResources->gbufferFramebuffer);
		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->cmdBufferGBuffer[i], pvrvk::DebugUtilsLabel("GBufferRenderPass"));
		recordCommandBufferRenderGBuffer(_deviceResources->cmdBufferGBuffer[i], i);
		pvr::utils::endCommandBufferDebugLabel(_deviceResources->cmdBufferGBuffer[i]);
		_deviceResources->cmdBufferGBuffer[i]->end();

		_deviceResources->cmdBufferDeferredShading[i]->begin(_deviceResources->onScreenFramebuffer[i]);
		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->cmdBufferGBuffer[i], pvrvk::DebugUtilsLabel("DeferredShadingRenderPass"));
		recordCommandBufferDeferredShading(_deviceResources->cmdBufferDeferredShading[i], i);
		pvr::utils::endCommandBufferDebugLabel(_deviceResources->cmdBufferGBuffer[i]);
		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->cmdBufferGBuffer[i], pvrvk::DebugUtilsLabel("UIRenderPass"));
		recordCommandUIRenderer(_deviceResources->cmdBufferDeferredShading[i]);
		pvr::utils::endCommandBufferDebugLabel(_deviceResources->cmdBufferGBuffer[i]);
		_deviceResources->cmdBufferDeferredShading[i]->end();

		_deviceResources->cmdBufferDownsample[i]->begin();
		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->cmdBufferDownsample[i], pvrvk::DebugUtilsLabel("DownsampleRenderPass"));
		recordCommandBufferDownsample(_deviceResources->cmdBufferDownsample[i]);
		pvr::utils::endCommandBufferDebugLabel(_deviceResources->cmdBufferGBuffer[i]);
		_deviceResources->cmdBufferDownsample[i]->end();
	}
}

/// <summary>Record rendering G-Buffer commands.</summary>
/// <param name="cmdBuffers">SecondaryCommandbuffer to record.</param>
/// <param name="swapchainIndex">Index of the current swapchain image.</param>
void VulkanHybridSoftShadows::recordCommandBufferRenderGBuffer(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex)
{
	uint32_t offsets[3] = {};
	offsets[0] = _deviceResources->cameraBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[1] = _deviceResources->perLightBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[2] = _deviceResources->perMeshBufferView.getDynamicSliceOffset(swapchainIndex);

	pvrvk::DescriptorSet arrayDS[] = { _deviceResources->commonDescriptorSet };

	cmdBuffers->bindDescriptorSets(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->gbufferPipelineLayout, 0, arrayDS, 1, offsets, 3);

	for (uint32_t meshIdx = 0; meshIdx < _deviceResources->meshes.size(); meshIdx++)
	{
		auto& mesh = _deviceResources->meshes[meshIdx];

		cmdBuffers->bindPipeline(_deviceResources->gbufferPipeline);

		cmdBuffers->pushConstants(_deviceResources->gbufferPipeline->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, sizeof(uint32_t), &meshIdx);

		uint32_t matID = static_cast<uint32_t>(mesh.materialIdx);
		cmdBuffers->pushConstants(_deviceResources->gbufferPipeline->getPipelineLayout(), pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, sizeof(uint32_t), sizeof(uint32_t), &matID);

		cmdBuffers->bindVertexBuffer(_deviceResources->vertexBuffers[meshIdx], 0, 0);
		cmdBuffers->bindIndexBuffer(_deviceResources->indexBuffers[meshIdx], 0, mesh.indexType);
		cmdBuffers->drawIndexed(mesh.indexOffset, mesh.numIndices, 0, 0, 1);
	}
}

/// <summary>Record deferred shading commands.</summary>
/// <param name="cmdBuffers">SecondaryCommandbuffer to record.</param>
/// <param name="swapchainIndex">Index of the current swapchain image.</param>
void VulkanHybridSoftShadows::recordCommandBufferDeferredShading(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex)
{
	cmdBuffers->bindPipeline(_deviceResources->defferedShadingPipeline);

	uint32_t offsets[3] = {};
	offsets[0] = _deviceResources->cameraBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[1] = _deviceResources->perLightBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[2] = _deviceResources->perMeshBufferView.getDynamicSliceOffset(swapchainIndex);

	pvrvk::DescriptorSet arrayDS[] = { _deviceResources->commonDescriptorSet, _deviceResources->gbufferDescriptorSet };

	cmdBuffers->bindDescriptorSets(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->deferredShadingPipelineLayout, 0, arrayDS, 2, offsets, 3);

	cmdBuffers->draw(0, 3);
}

/// <summary>Record commands to downsample the Visibility/Hit Distance G-Buffer attachment so that the higher mip levels can be used to determine penumbra regions.</summary>
/// <param name="cmdBuffers">SecondaryCommandbuffer to record.</param>
/// <param name="swapchainIndex">Index of the current swapchain image.</param>
void VulkanHybridSoftShadows::recordCommandBufferDownsample(pvrvk::SecondaryCommandBuffer& cmdBuffers)
{
	// The starting and ending image layouts of the image
	pvrvk::ImageLayout srcLayout = pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL;
	pvrvk::ImageLayout dstLayout = pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL;

	// Since the remaining 3 mip levels of the image are undefined at the moment, we'll transition them to e_TRANSFER_DST_OPTIMAL.
	{
		pvrvk::ImageSubresourceRange initialSubresourceRange = pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT, 1, 3);

		pvrvk::MemoryBarrierSet layoutTransitions;

		pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_UNDEFINED;
		pvrvk::ImageLayout destinationImageLayout = pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL;

		layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_WRITE_BIT, pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT,
			_deviceResources->gbufferVisibilityMipMappedImage->getImage(), initialSubresourceRange, sourceImageLayout, destinationImageLayout,
			_deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));

		cmdBuffers->pipelineBarrier(pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_TRANSFER_BIT, layoutTransitions);
	}

	pvrvk::ImageSubresourceRange subresourceRange = pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0, 1);

	int32_t mipWidth = _deviceResources->swapchain->getDimension().getWidth();
	int32_t mipHeight = _deviceResources->swapchain->getDimension().getHeight();

	for (uint32_t mipIdx = 1; mipIdx < 4; mipIdx++)
	{
		subresourceRange.setBaseMipLevel(mipIdx - 1);

		pvrvk::ImageLayout layout = pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL;

		if (mipIdx == 1) layout = srcLayout;

		if (layout != pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL)
		{
			pvrvk::MemoryBarrierSet layoutTransitions;

			pvrvk::ImageLayout sourceImageLayout = layout;
			pvrvk::ImageLayout destinationImageLayout = pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL;

			layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(mipIdx == 1 ? pvrvk::AccessFlags::e_SHADER_WRITE_BIT : pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT,
				pvrvk::AccessFlags::e_TRANSFER_READ_BIT, _deviceResources->gbufferVisibilityMipMappedImage->getImage(), subresourceRange, sourceImageLayout, destinationImageLayout,
				_deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));

			cmdBuffers->pipelineBarrier(mipIdx == 1 ? pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT : pvrvk::PipelineStageFlags::e_TRANSFER_BIT,
				pvrvk::PipelineStageFlags::e_TRANSFER_BIT, layoutTransitions);
		}

		pvrvk::Offset3D srcOffsets[2] = { { 0, 0, 0 }, { mipWidth, mipHeight, 1 } };
		pvrvk::Offset3D dstOffsets[2] = { { 0, 0, 0 }, { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 } };

		pvrvk::ImageBlit blit;

		blit.setSrcOffsets(srcOffsets);
		blit.setDstOffsets(dstOffsets);
		blit.setSrcSubresource({ pvrvk::ImageAspectFlags::e_COLOR_BIT, mipIdx - 1, 0, 1 });
		blit.setDstSubresource({ pvrvk::ImageAspectFlags::e_COLOR_BIT, mipIdx, 0, 1 });

		cmdBuffers->blitImage(_deviceResources->gbufferVisibilityMipMappedImage->getImage(), _deviceResources->gbufferVisibilityMipMappedImage->getImage(), &blit, 1,
			pvrvk::Filter::e_LINEAR, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL);

		{
			pvrvk::MemoryBarrierSet layoutTransitions;

			pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL;
			pvrvk::ImageLayout destinationImageLayout = dstLayout;

			layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_TRANSFER_READ_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT,
				_deviceResources->gbufferVisibilityMipMappedImage->getImage(), subresourceRange, sourceImageLayout, destinationImageLayout,
				_deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));

			cmdBuffers->pipelineBarrier(pvrvk::PipelineStageFlags::e_TRANSFER_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, layoutTransitions);
		}

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	subresourceRange.setBaseMipLevel(3);

	// Transition the last blitted mip level back to e_SHADER_READ_ONLY_OPTIMAL
	{
		pvrvk::MemoryBarrierSet layoutTransitions;

		pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL;
		pvrvk::ImageLayout destinationImageLayout = dstLayout;

		layoutTransitions.addBarrier(
			pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, _deviceResources->gbufferVisibilityMipMappedImage->getImage(),
				subresourceRange, sourceImageLayout, destinationImageLayout, _deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));

		cmdBuffers->pipelineBarrier(pvrvk::PipelineStageFlags::e_TRANSFER_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, layoutTransitions);
	}
}

/// <summary>Record UIRenderer commands.</summary>
/// <param name="commandBuff">Commandbuffer to record.</param>
void VulkanHybridSoftShadows::recordCommandUIRenderer(pvrvk::SecondaryCommandBuffer& commandBuff)
{
	_deviceResources->uiRenderer.beginRendering(commandBuff);
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.endRendering();
}

/// <summary>This function must be implemented by the user of the shell. The user should return its Shell object defining the
/// behaviour of the application.</summary>
/// <returns>Return an unique_ptr to a new Demo class,supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanHybridSoftShadows>(); }
