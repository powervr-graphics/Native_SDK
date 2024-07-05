/*!
\brief Implements Ray Traced Hard Shadows using the Vulkan Ray Tracing Pipeline and Ray Queries.
\file VulkanHybridHardShadows.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRVk/PVRVk.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRUtils/Vulkan/AccelerationStructure.h"
#include "vulkan/vulkan_beta.h"

//#define USE_EXTRA_LIGHTS 1

#ifdef USE_EXTRA_LIGHTS
#define NUM_EXTRA_LIGHTS 1
#endif

// Maximum number of swap images supported
enum CONSTANTS
{
	MAX_NUMBER_OF_SWAP_IMAGES = 4,
	MAX_LIGHTS = 3
};

// Framebuffer colour attachment indices
namespace FramebufferGBufferAttachments {
enum Enum
{
	Albedo_Metallic = 0,
	Normal_Reflectivity,
	WorldPosition_F90,
	F0_Roughness,
	Count
};
}

namespace SceneNodes {
enum class MeshNodes
{
	Satyr = 0,
	Table = 1,
	Box = 2,
	Torus = 3,
	Hedra = 4,
	Num = 5
};

enum class Cameras
{
	SceneCamera = 0,
	NumCameras = 1
};
} // namespace SceneNodes

/// <summary>Shader names for all of the demo passes.</summary>
namespace Files {
const char* const SceneFile = "HardShadows.POD";
const char* const GBufferVertexShader = "GBufferVertexShader.vsh.spv";
const char* const GBufferFragmentShader = "GBufferFragmentShader.fsh.spv";
const char* const DeferredShadingFragmentShader = "DeferredShadingFragmentShader.fsh.spv";
const char* const ForwardShadingFragmentShader = "ForwardShadingFragmentShader.fsh.spv";
const char* const FullscreenTriangleVertexShader = "FullscreenTriangleVertexShader.vsh.spv";
} // namespace Files

/// <summary>buffer entry names used for the structured memory views used throughout the demo.
/// These entry names must match the variable names used in the demo shaders.</summary>
namespace BufferEntryNames {
namespace PerScene {
const char* const ViewMatrix = "mViewMatrix";
const char* const ProjectionMatrix = "mProjectionMatrix";
const char* const AmbientLightColor = "vAmbientLightColor";
const char* const CameraPosition = "vCameraPosition";
const char* const NumLights = "uNumLights";
} // namespace PerScene

namespace PerMesh {
const char* const WorldMatrix = "mWorldMatrix";
} // namespace PerMesh

namespace PerPointLightData {
const char* const LightColor = "vLightColor";
const char* const LightPosition = "vLightPosition";
const char* const LightIntensity = "fLightIntensity";
} // namespace PerPointLightData
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

//// Demo Configuration data
// Light configuration data including handling the way the procedural light source moves
namespace LightConfiguration {
glm::vec4 AmbientColorScaler = glm::vec4(0.18f, 0.18f, 0.18f, 1.0);
uint32_t MaxNumLights = 3;

float LightMaxDistance = 250.f;
float LightMinDistance = 120.f;
float LightMinHeight = 30.f;
float LightMaxHeight = 100.f;
float LightAxialVelocityChange = .01f;
float LightRadialVelocityChange = .003f;
float LightVerticalVelocityChange = .01f;
float LightMaxAxialVelocity = 5.f;
float LightMaxRadialVelocity = 1.5f;
float LightMaxVerticalVelocity = 5.f;
} // namespace LightConfiguration

// Light Uniforms
struct PerLightData
{
	glm::vec4 lightColor;
	glm::vec4 lightPosition;
	float lightIntensity;

	bool isProcedural;
	float radial_vel;
	float axial_vel;
	float vertical_vel;
	float angle;
	float distance;
	float height;
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

	// Image view for the Shadow Mask image
	pvrvk::ImageView raytraceShadowsImage;

	// Framebuffer for the G-buffer
	pvrvk::Framebuffer gbufferFramebuffer;

	// Framebuffers created for the swapchain images
	std::vector<pvrvk::Framebuffer> onScreenFramebuffer;

	// Renderpass for the G-buffer
	pvrvk::RenderPass gbufferRenderPass;

	//// Command Buffers ////
	// Main Primary Command Buffer
	pvrvk::CommandBuffer cmdBufferMainDeferred[MAX_NUMBER_OF_SWAP_IMAGES];
	pvrvk::CommandBuffer cmdBufferMainForward[MAX_NUMBER_OF_SWAP_IMAGES];

	// Secondary command buffers used for each pass
	pvrvk::SecondaryCommandBuffer cmdBufferGBuffer[MAX_NUMBER_OF_SWAP_IMAGES];
	pvrvk::SecondaryCommandBuffer cmdBufferDeferredShading[MAX_NUMBER_OF_SWAP_IMAGES];
	pvrvk::SecondaryCommandBuffer cmdBufferForwadShading[MAX_NUMBER_OF_SWAP_IMAGES];
	pvrvk::SecondaryCommandBuffer cmdBufferRayTracedShadows[MAX_NUMBER_OF_SWAP_IMAGES];

	////  Descriptor Set Layouts ////
	pvrvk::DescriptorSetLayout commonDescriptorSetLayout;
	pvrvk::DescriptorSetLayout gbufferDescriptorSetLayout;
	pvrvk::DescriptorSetLayout imageDescriptorSetLayout;
	pvrvk::DescriptorSetLayout deferredShadingDescriptorSetLayout;

	////  Descriptor Sets ////
	pvrvk::DescriptorSet commonDescriptorSet;
	pvrvk::DescriptorSet gbufferDescriptorSet;
	pvrvk::DescriptorSet imageDescriptorSet;
	pvrvk::DescriptorSet deferredShadingDescriptorSet;

	//// Pipeline Layouts ////
	// GBuffer pipeline layouts
	pvrvk::PipelineLayout gbufferPipelineLayout;

	// Deferred shading pipeline layout
	pvrvk::PipelineLayout deferredShadingPipelineLayout;
	pvrvk::PipelineLayout forwardShadingPipelineLayout;

	// Bindless scene resources
	std::vector<pvrvk::Buffer> vertexBuffers;
	std::vector<pvrvk::Buffer> indexBuffers;
	std::vector<MeshAS> meshes;
	std::vector<int> verticesSize;
	std::vector<int> indicesSize;
	pvr::utils::AccelerationStructureWrapper accelerationStructure;

	//// Structured Memory Views ////
	// scene wide buffers
	pvr::utils::StructuredBufferView globalBufferView;
	pvrvk::Buffer globalBuffer;
	pvrvk::Buffer materialBuffer;

	// light buffer
	pvr::utils::StructuredBufferView lightDataBufferView;
	pvrvk::Buffer lightDataBuffer;

	// mesh data
	pvr::utils::StructuredBufferView perMeshBufferView;
	pvrvk::Buffer perMeshBuffer;

	std::vector<pvrvk::Semaphore> imageAcquiredSemaphores;
	std::vector<pvrvk::Semaphore> presentationSemaphores;
	std::vector<pvrvk::Fence> perFrameResourcesFences;

	//// Pipelines ////
	// G-buffer pass
	pvrvk::GraphicsPipeline gbufferPipeline;

	// Deferred shading pass
	pvrvk::GraphicsPipeline defferedShadingPipeline;

	// Forward shading pass
	pvrvk::GraphicsPipeline forwardShadingPipeline;

	// RT shadow pass
	pvrvk::PipelineLayout raytraceShadowPipelineLayout;
	pvrvk::RaytracingPipeline raytraceShadowPipeline;
	pvrvk::Buffer raytraceShadowShaderBindingTable;

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
class VulkanHybridHardShadows : public pvr::Shell
{
public:
	//// Frame ////
	uint32_t _numSwapImages;
	uint32_t _swapchainIndex;
	// Putting all API objects into a pointer just makes it easier to release them all together with RAII
	std::unique_ptr<DeviceResources> _deviceResources;

	VkPhysicalDeviceRayTracingPipelinePropertiesKHR _rtProperties;
	uint32_t _shaderGroupCount = 0;

	// per light data
	std::vector<PerLightData> _lightData;
	glm::vec4 _averageLightColor;

	// Frame counters for animation
	uint32_t _frameId;
	float _frameNumber;
	bool _animateCamera;
	bool _useDeferred;
	float _frame = 0.0f;

	// Projection and Model View matrices
	glm::mat4 _viewMatrix;
	glm::mat4 _projectionMatrix;
	glm::mat4 _viewProjectionMatrix;
	glm::mat4 _inverseViewMatrix;
	glm::vec3 _cameraPosition;
	std::vector<glm::mat4> _meshTransforms;
	float _farClipDistance;

	uint32_t _windowWidth;
	uint32_t _windowHeight;
	uint32_t _framebufferWidth;
	uint32_t _framebufferHeight;

	int32_t _viewportOffsets[2];

	// Scene models
	pvr::assets::ModelHandle _scene;

	/// <summary>Filter performance warning UNASSIGNED-BestPractices-vkAllocateMemory-small-allocation Best Practices which
	/// has ID -602362517 for TLAS buffer build and update. This warning recommends buffer allocations to be of size at least
	/// 256KB which collides with each BLAS node built for each scene element and the size of the TLAS buffer, details of the warning:
	/// https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/master/layers/best_practices_validation.h</summary>
	std::vector<int> vectorValidationIDFilter;

	VulkanHybridHardShadows()
	{
		_animateCamera = false;
		_useDeferred = false;
	}

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
	void createRayTracingPipelines();
	void createDeferredShadingPipelines();
	void createForwardShadingPipelines();
	void createShaderBindingTable();
	void recordMainCommandBuffer();
	void recordCommandBufferRenderGBuffer(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex);
	void recordCommandBufferForwardShading(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex);
	void recordCommandBufferDeferredShading(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex);
	void recordCommandBufferRayTraceShadows(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex);
	void recordCommandUIRenderer(pvrvk::SecondaryCommandBuffer& cmdBuffers);
	void recordSecondaryCommandBuffers();
	void createDescriptorSetLayouts();
	void createDescriptorSets();
	void uploadDynamicSceneData();
	void createCameraBuffer();
	void initializeLights();
	void createMeshTransformBuffer();
	void createLightBuffer();
	void updateAnimation();
	void updateProceduralLights();
	void createModelBuffers(pvrvk::CommandBuffer& uploadCmd);

	void eventMappedInput(pvr::SimplifiedInput key)
	{
		switch (key)
		{
		// Handle input
		case pvr::SimplifiedInput::ActionClose: exitShell(); break;
		case pvr::SimplifiedInput::Action1: _useDeferred = !_useDeferred; break;
		case pvr::SimplifiedInput::Action2: _animateCamera = !_animateCamera; break;
		default: break;
		}

		updateDescription();
	}

	void updateDescription()
	{
		std::string modeString = _useDeferred ? "Mode = Deferred (RT Pipeline)" : "Mode = Forward (Ray Queries)";

		_deviceResources->uiRenderer.getDefaultDescription()->setText(modeString);
		_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
	}
};

/// <summary> Code in initApplication() will be called by pvr::Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns> Return true if no error occurred. </returns>
pvr::Result VulkanHybridHardShadows::initApplication()
{
	// This demo application makes heavy use of the stencil buffer
	setStencilBitsPerPixel(8);
	_frameNumber = 0.0f;
	_frameId = 0;

	//  Load the scene
	_scene = pvr::assets::loadModel(*this, Files::SceneFile);

	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.).</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanHybridHardShadows::initView()
{
	initializeLights();

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

	// Filter UNASSIGNED-BestPractices-vkAllocateMemory-small-allocation Best Practices performance warning which has ID -602362517 for TLAS buffer build and
	// update (VkBufferDeviceAddressInfo requires VkBuffer handle so in general it's not possible to make a single buffer to put all information
	// and use offsets inside it
	vectorValidationIDFilter.push_back(-602362517);
	// Filter UNASSIGNED-BestPractices-vkBindMemory-small-dedicated-allocation performance warning recommending to do buffer allocations of at least 1048576 bytes
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

	// Create the command pool
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
		_deviceResources->cmdBufferMainForward[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->cmdBufferGBuffer[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->cmdBufferDeferredShading[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->cmdBufferForwadShading[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->cmdBufferRayTracedShadows[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();

		_deviceResources->cmdBufferMainDeferred[i]->setObjectName("DeferredCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->cmdBufferMainForward[i]->setObjectName("ForwardCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->cmdBufferGBuffer[i]->setObjectName("GBufferSecondaryCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->cmdBufferDeferredShading[i]->setObjectName("DeferredSecondaryShadingCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->cmdBufferForwadShading[i]->setObjectName("ForwadShadingSecondaryCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->cmdBufferRayTracedShadows[i]->setObjectName("RayTracedShadowsSecondaryCommandBufferSwapchain" + std::to_string(i));

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
	_deviceResources->uiRenderer.getDefaultTitle()->setText("HybridHardShadows");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText("Action 1: Toggle Mode\n"
															   "Action 2: Toggle Animation");
	updateDescription();
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	// get ray tracing properties
	_rtProperties.sType = static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR);
	_rtProperties.pNext = nullptr;
	VkPhysicalDeviceProperties2 properties{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_PROPERTIES_2) };
	properties.pNext = &_rtProperties;
	_deviceResources->instance->getVkBindings().vkGetPhysicalDeviceProperties2(_deviceResources->instance->getPhysicalDevice(vectorPhysicalDevicesIndex[0])->getVkHandle(), &properties);

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	_deviceResources->cmdBufferMainDeferred[0]->begin();

	createModelBuffers(_deviceResources->cmdBufferMainDeferred[0]);

	_deviceResources->cmdBufferMainDeferred[0]->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->cmdBufferMainDeferred[0];
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle(); // wait

	createFramebufferAndRenderPass();
	createLightBuffer();
	createMeshTransformBuffer();
	createCameraBuffer();
	createDescriptorSetLayouts();
	createPipelines();
	createShaderBindingTable();

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
pvr::Result VulkanHybridHardShadows::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
///	If the rendering context is lost, quitApplication() will not be called.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanHybridHardShadows::quitApplication()
{
	_scene.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return Result::Success if no error occurred</returns>
pvr::Result VulkanHybridHardShadows::renderFrame()
{
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	_swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameResourcesFences[_swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[_swapchainIndex]->reset();

	//  Handle user input and update object animations
	updateAnimation();
	updateProceduralLights();

	// Update Acceleration Structure
	updateScene();

	// Upload dynamic data
	uploadDynamicSceneData();

	//--------------------
	// submit the main command buffer
	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags pipeWaitStage = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;

	if (_useDeferred)
		submitInfo.commandBuffers = &_deviceResources->cmdBufferMainDeferred[_swapchainIndex];
	else
		submitInfo.commandBuffers = &_deviceResources->cmdBufferMainForward[_swapchainIndex];

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

	return pvr::Result::Success;
}

/// <summary>Updates the scene animation and takes the new mesh transforms and updates the TLAS.</summary>
void VulkanHybridHardShadows::updateScene()
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
void VulkanHybridHardShadows::createDescriptorSetLayouts()
{
	// Common Descriptor Set Layout
	pvrvk::DescriptorSetLayoutCreateInfo commonDescSetInfo;

	// Dynamic per scene buffer
	commonDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u,
		pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR);
	// Dynamic per light buffer
	commonDescSetInfo.setBinding(1, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u,
		pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR);
	// Static material data buffer
	commonDescSetInfo.setBinding(2, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1u,
		pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR | pvrvk::ShaderStageFlags::e_ANY_HIT_BIT_KHR);
	// TLAS
	commonDescSetInfo.setBinding(3, pvrvk::DescriptorType::e_ACCELERATION_STRUCTURE_KHR, 1u, pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	// Dynamic per mesh buffer
	commonDescSetInfo.setBinding(4, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	_deviceResources->commonDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(commonDescSetInfo);

	// GBuffer Descriptor Set Layout

	pvrvk::DescriptorSetLayoutCreateInfo gbufferDescSetInfo;
	gbufferDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	gbufferDescSetInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	gbufferDescSetInfo.setBinding(2, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	gbufferDescSetInfo.setBinding(3, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	_deviceResources->gbufferDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(gbufferDescSetInfo);

	// Image Descriptor Set Layout

	pvrvk::DescriptorSetLayoutCreateInfo imageDescSetInfo;
	imageDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_STORAGE_IMAGE, 1u, pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	_deviceResources->imageDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(imageDescSetInfo);

	// Deferred Shading Descriptor Set Layout

	pvrvk::DescriptorSetLayoutCreateInfo defferedShadingDescSetInfo;
	defferedShadingDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	defferedShadingDescSetInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	defferedShadingDescSetInfo.setBinding(2, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	defferedShadingDescSetInfo.setBinding(3, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	defferedShadingDescSetInfo.setBinding(4, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	_deviceResources->deferredShadingDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(defferedShadingDescSetInfo);
}

/// <summary>Creates descriptor sets.</summary>
void VulkanHybridHardShadows::createDescriptorSets()
{
	// Scene Samplers

	pvrvk::SamplerCreateInfo samplerDesc;
	samplerDesc.wrapModeU = samplerDesc.wrapModeV = samplerDesc.wrapModeW = pvrvk::SamplerAddressMode::e_REPEAT;

	samplerDesc.minFilter = pvrvk::Filter::e_LINEAR;
	samplerDesc.magFilter = pvrvk::Filter::e_LINEAR;
	samplerDesc.mipMapMode = pvrvk::SamplerMipmapMode::e_LINEAR;
	pvrvk::Sampler samplerTrilinear = _deviceResources->device->createSampler(samplerDesc);

	samplerDesc.minFilter = pvrvk::Filter::e_NEAREST;
	samplerDesc.magFilter = pvrvk::Filter::e_NEAREST;
	samplerDesc.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
	pvrvk::Sampler samplerNearest = _deviceResources->device->createSampler(samplerDesc);

	// Allocate Descriptor Sets

	_deviceResources->commonDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->commonDescriptorSetLayout);
	_deviceResources->gbufferDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->gbufferDescriptorSetLayout);
	_deviceResources->imageDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->imageDescriptorSetLayout);
	_deviceResources->deferredShadingDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->deferredShadingDescriptorSetLayout);

	_deviceResources->commonDescriptorSet->setObjectName("CommonDescriptorSet");
	_deviceResources->gbufferDescriptorSet->setObjectName("GBufferDescriptorSet");
	_deviceResources->imageDescriptorSet->setObjectName("ImageDescriptorSet");
	_deviceResources->deferredShadingDescriptorSet->setObjectName("DeferredShadingDescriptorSet");

	// Write Common Descriptor Set

	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->commonDescriptorSet, 0)
								.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->globalBuffer, 0, _deviceResources->globalBufferView.getDynamicSliceSize())));

	writeDescSets.push_back(
		pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->commonDescriptorSet, 1)
			.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->lightDataBuffer, 0, _deviceResources->lightDataBufferView.getDynamicSliceSize() * _lightData.size())));

	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->commonDescriptorSet, 2)
								.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->materialBuffer, 0, _deviceResources->materialBuffer->getSize())));

	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_ACCELERATION_STRUCTURE_KHR, _deviceResources->commonDescriptorSet, 3)
								.setAccelerationStructureInfo(0, _deviceResources->accelerationStructure.getTopLevelAccelerationStructure()));
	writeDescSets.push_back(
		pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->commonDescriptorSet, 4)
			.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->perMeshBuffer, 0, _deviceResources->perMeshBufferView.getDynamicSliceSize() * _meshTransforms.size())));

	// Write GBuffer Descriptor Set

	for (int i = 0; i < FramebufferGBufferAttachments::Count; i++)
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->gbufferDescriptorSet, i)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->gbufferImages[i], samplerNearest, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

	// Write Image Descriptor Set

	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_IMAGE, _deviceResources->imageDescriptorSet, 0)
								.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->raytraceShadowsImage, pvrvk::ImageLayout::e_GENERAL)));

	// Write Deferred Shading Descriptor Set

	for (int i = 0; i < FramebufferGBufferAttachments::Count; i++)
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->deferredShadingDescriptorSet, i)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->gbufferImages[i], samplerNearest, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

	writeDescSets.push_back(
		pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->deferredShadingDescriptorSet, FramebufferGBufferAttachments::Count)
			.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->raytraceShadowsImage, samplerNearest, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

/// <summary>Creates the pipeline for the G-Buffer pass.</summary>
void VulkanHybridHardShadows::createGBufferPipelines()
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
	renderGBufferPipelineCreateInfo.colorBlend.setAttachmentState(2, renderGBufferColorAttachment);
	renderGBufferPipelineCreateInfo.colorBlend.setAttachmentState(3, renderGBufferColorAttachment);

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

/// <summary>Creates the pipeline for the Ray-Traced shadows pass.</summary>
void VulkanHybridHardShadows::createRayTracingPipelines()
{
	// pipeline layout
	pvrvk::PipelineLayoutCreateInfo pipeLayout;
	pipeLayout.addDescSetLayout(_deviceResources->gbufferDescriptorSetLayout);
	pipeLayout.addDescSetLayout(_deviceResources->imageDescriptorSetLayout);
	pipeLayout.addDescSetLayout(_deviceResources->commonDescriptorSetLayout);

	_deviceResources->raytraceShadowPipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayout);

	// Raygen
	pvrvk::ShaderModule raygenSM = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("RayTraceShadows.rgen.spv")->readToEnd<uint32_t>()));
	pvrvk::ShaderModule missSM = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("RayTraceShadows.rmiss.spv")->readToEnd<uint32_t>()));
	pvrvk::ShaderModule chitSM = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("RayTraceShadows.rchit.spv")->readToEnd<uint32_t>()));

	pvrvk::RaytracingPipelineCreateInfo raytracingPipeline;

	pvrvk::PipelineShaderStageCreateInfo generateCreateInfo;
	generateCreateInfo.setShader(raygenSM);
	generateCreateInfo.setShaderStage(pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR);
	raytracingPipeline.stages.push_back(generateCreateInfo);

	pvrvk::PipelineShaderStageCreateInfo missCreateInfo;
	missCreateInfo.setShader(missSM);
	missCreateInfo.setShaderStage(pvrvk::ShaderStageFlags::e_MISS_BIT_KHR);
	raytracingPipeline.stages.push_back(missCreateInfo);

	pvrvk::PipelineShaderStageCreateInfo hitCreateInfo;
	hitCreateInfo.setShader(chitSM);
	hitCreateInfo.setShaderStage(pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR);
	raytracingPipeline.stages.push_back(hitCreateInfo);

	pvrvk::RayTracingShaderGroupCreateInfo rg = pvrvk::RayTracingShaderGroupCreateInfo(pvrvk::RayTracingShaderGroupTypeKHR::e_GENERAL_KHR);
	pvrvk::RayTracingShaderGroupCreateInfo mg = pvrvk::RayTracingShaderGroupCreateInfo(pvrvk::RayTracingShaderGroupTypeKHR::e_GENERAL_KHR);
	pvrvk::RayTracingShaderGroupCreateInfo hg = pvrvk::RayTracingShaderGroupCreateInfo(pvrvk::RayTracingShaderGroupTypeKHR::e_TRIANGLES_HIT_GROUP_KHR);

	rg.setGeneralShader(static_cast<uint32_t>(0));
	mg.setGeneralShader(static_cast<uint32_t>(1));
	hg.setClosestHitShader(static_cast<uint32_t>(2));

	raytracingPipeline.shaderGroups = { rg, mg, hg };
	_shaderGroupCount = static_cast<uint32_t>(raytracingPipeline.shaderGroups.size());

	raytracingPipeline.maxRecursionDepth = 1; // Ray depth
	raytracingPipeline.pipelineLayout = _deviceResources->raytraceShadowPipelineLayout;

	_deviceResources->raytraceShadowPipeline = _deviceResources->device->createRaytracingPipeline(raytracingPipeline, nullptr);
	_deviceResources->raytraceShadowPipeline->setObjectName("ShadowRaytracingPipeline");
}

/// <summary>Creates the pipeline for the Deferred shading pass.</summary>
void VulkanHybridHardShadows::createDeferredShadingPipelines()
{
	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

	pipeLayoutInfo.setDescSetLayout(0, _deviceResources->commonDescriptorSetLayout);
	pipeLayoutInfo.setDescSetLayout(1, _deviceResources->deferredShadingDescriptorSetLayout);

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
	_deviceResources->defferedShadingPipeline->setObjectName("DeferredShadingGraphicsPipeline");
}

/// <summary>Creates the pipeline for the Forward shading pass.</summary>
void VulkanHybridHardShadows::createForwardShadingPipelines()
{
	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

	pipeLayoutInfo.setDescSetLayout(0, _deviceResources->commonDescriptorSetLayout);
	pipeLayoutInfo.addPushConstantRange(pvrvk::PushConstantRange(pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, sizeof(uint32_t)));
	pipeLayoutInfo.addPushConstantRange(pvrvk::PushConstantRange(pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, sizeof(uint32_t), sizeof(uint32_t)));

	_deviceResources->forwardShadingPipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	pvrvk::GraphicsPipelineCreateInfo pipelineCreateInfo;
	pipelineCreateInfo.viewport.setViewportAndScissor(0,
		pvrvk::Viewport(
			0.0f, 0.0f, static_cast<float>(_deviceResources->swapchain->getDimension().getWidth()), static_cast<float>(_deviceResources->swapchain->getDimension().getHeight())),
		pvrvk::Rect2D(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight()));
	// enable back face culling
	pipelineCreateInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);

	// set counter clockwise winding order for front faces
	pipelineCreateInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

	// enable depth testing
	pipelineCreateInfo.depthStencil.enableDepthTest(true);
	pipelineCreateInfo.depthStencil.enableDepthWrite(true);

	// set the blend state for the colour attachments
	pvrvk::PipelineColorBlendAttachmentState renderGBufferColorAttachment;
	// number of colour blend states must equal number of colour attachments for the subpass
	pipelineCreateInfo.colorBlend.setAttachmentState(0, renderGBufferColorAttachment);

	// load and create appropriate shaders
	pipelineCreateInfo.vertexShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::GBufferVertexShader)->readToEnd<uint32_t>())));

	pipelineCreateInfo.fragmentShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::ForwardShadingFragmentShader)->readToEnd<uint32_t>())));

	// setup vertex inputs
	pipelineCreateInfo.vertexInput.clear();

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

	pipelineCreateInfo.vertexInput.addInputAttribute(posAttrib);
	pipelineCreateInfo.vertexInput.addInputAttribute(normalAttrib);
	pipelineCreateInfo.vertexInput.addInputAttribute(texCoordAttrib);
	pipelineCreateInfo.vertexInput.addInputAttribute(tangentAttrib);
	pipelineCreateInfo.vertexInput.addInputBinding(binding);

	pvrvk::PipelineInputAssemblerStateCreateInfo inputAssembler;
	inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);
	pipelineCreateInfo.inputAssembler = inputAssembler;

	// renderpass/subpass
	pipelineCreateInfo.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();

	// disable stencil testing
	pvrvk::StencilOpState stencilState;

	// only replace stencil buffer when the depth test passes
	stencilState.setFailOp(pvrvk::StencilOp::e_KEEP);
	stencilState.setDepthFailOp(pvrvk::StencilOp::e_KEEP);
	stencilState.setPassOp(pvrvk::StencilOp::e_REPLACE);
	stencilState.setCompareOp(pvrvk::CompareOp::e_ALWAYS);

	// disable the stencil tests
	pipelineCreateInfo.depthStencil.enableStencilTest(false);
	// set stencil states
	pipelineCreateInfo.depthStencil.setStencilFront(stencilState);
	pipelineCreateInfo.depthStencil.setStencilBack(stencilState);

	pipelineCreateInfo.pipelineLayout = _deviceResources->forwardShadingPipelineLayout;
	_deviceResources->forwardShadingPipeline = _deviceResources->device->createGraphicsPipeline(pipelineCreateInfo, _deviceResources->pipelineCache);
	_deviceResources->forwardShadingPipeline->setObjectName("ForwardShadingGraphicsPipeline");
}

/// <summary>Creates the shader binding table for the Ray-Traced shadows pass.</summary>
void VulkanHybridHardShadows::createShaderBindingTable()
{
	uint32_t groupHandleSize = _rtProperties.shaderGroupHandleSize; // Size of a program identifier
	uint32_t baseAlignment = _rtProperties.shaderGroupBaseAlignment; // Size of shader alignment

	// Fetch all the shader handles used in the pipeline, so that they can be written in the SBT
	uint32_t dataSize = _shaderGroupCount * groupHandleSize;

	std::vector<uint8_t> shaderHandleStorage(dataSize);
	_deviceResources->device->getVkBindings().vkGetRayTracingShaderGroupHandlesKHR(
		_deviceResources->device->getVkHandle(), _deviceResources->raytraceShadowPipeline->getVkHandle(), 0, _shaderGroupCount, dataSize, shaderHandleStorage.data());

	uint32_t sbtSize = _shaderGroupCount * baseAlignment;

	// Create a buffer to store Shader Binding Table in
	_deviceResources->raytraceShadowShaderBindingTable = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(
			sbtSize, pvrvk::BufferUsageFlags::e_TRANSFER_SRC_BIT | pvrvk::BufferUsageFlags::e_SHADER_BINDING_TABLE_BIT_KHR | pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		pvrvk::MemoryPropertyFlags::e_NONE, nullptr, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT, pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT);
	_deviceResources->raytraceShadowShaderBindingTable->setObjectName("RaytraceShadowShaderBindingTableBuffer");

	// Write the handles in the SBT
	void* mapped = _deviceResources->raytraceShadowShaderBindingTable->getDeviceMemory()->map(0, VK_WHOLE_SIZE);

	auto* pData = reinterpret_cast<uint8_t*>(mapped);
	for (uint32_t g = 0; g < _shaderGroupCount; g++)
	{
		memcpy(pData, shaderHandleStorage.data() + g * groupHandleSize, groupHandleSize); // raygen
		pData += baseAlignment;
	}

	_deviceResources->raytraceShadowShaderBindingTable->getDeviceMemory()->unmap();
}

/// <summary>Create the pipelines for this example.</summary>
void VulkanHybridHardShadows::createPipelines()
{
	createGBufferPipelines();
	createRayTracingPipelines();
	createDeferredShadingPipelines();
	createForwardShadingPipelines();
}

/// <summary>Create the G-Buffer pass framebuffer and renderpass.</summary>
void VulkanHybridHardShadows::createFramebufferAndRenderPass()
{
	const pvrvk::Extent3D& dimension = pvrvk::Extent3D(_deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight(), 1u);

	const pvrvk::Format renderpassStorageFormats[FramebufferGBufferAttachments::Count] = { pvrvk::Format::e_R8G8B8A8_UNORM, pvrvk::Format::e_R16G16B16A16_SFLOAT,
		pvrvk::Format::e_R16G16B16A16_SFLOAT, pvrvk::Format::e_R8G8B8A8_UNORM };

	// Create images
	for (int i = 0; i < FramebufferGBufferAttachments::Count; i++)
	{
		pvrvk::Image image = pvr::utils::createImage(_deviceResources->device,
			pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, renderpassStorageFormats[i], dimension, pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _deviceResources->vmaAllocator,
			pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);

		_deviceResources->gbufferImages[i] = _deviceResources->device->createImageView(
			pvrvk::ImageViewCreateInfo(image, pvrvk::ImageViewType::e_2D, image->getFormat(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT)));
	}

	const std::vector<pvrvk::Format> preferredDepthFormats = { pvrvk::Format::e_D24_UNORM_S8_UINT, pvrvk::Format::e_D32_SFLOAT_S8_UINT, pvrvk::Format::e_D16_UNORM_S8_UINT };
	const pvrvk::Format depthStencilFormat = pvr::utils::getSupportedDepthStencilFormat(_deviceResources->device, preferredDepthFormats);

	pvrvk::Image image = pvr::utils::createImage(_deviceResources->device,
		pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, depthStencilFormat, dimension, pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _deviceResources->vmaAllocator,
		pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);

	_deviceResources->gbufferDepthStencilImage = _deviceResources->device->createImageView(
		pvrvk::ImageViewCreateInfo(image, pvrvk::ImageViewType::e_2D, image->getFormat(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_DEPTH_BIT)));

	pvrvk::Image raytraceShadowsImage = pvr::utils::createImage(_deviceResources->device,
		pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, pvrvk::Format::e_R8G8B8A8_SNORM, dimension, pvrvk::ImageUsageFlags::e_STORAGE_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _deviceResources->vmaAllocator,
		pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);

	_deviceResources->raytraceShadowsImage = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(
		raytraceShadowsImage, pvrvk::ImageViewType::e_2D, raytraceShadowsImage->getFormat(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT)));

	// Create render pass
	pvrvk::RenderPassCreateInfo renderPassInfo;

	pvrvk::AttachmentDescription gbufferAttachment0 =
		pvrvk::AttachmentDescription::createColorDescription(renderpassStorageFormats[FramebufferGBufferAttachments::Albedo_Metallic], pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT);
	pvrvk::AttachmentDescription gbufferAttachment1 =
		pvrvk::AttachmentDescription::createColorDescription(renderpassStorageFormats[FramebufferGBufferAttachments::Normal_Reflectivity], pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT);
	pvrvk::AttachmentDescription gbufferAttachment2 =
		pvrvk::AttachmentDescription::createColorDescription(renderpassStorageFormats[FramebufferGBufferAttachments::WorldPosition_F90], pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT);
	pvrvk::AttachmentDescription gbufferAttachment3 =
		pvrvk::AttachmentDescription::createColorDescription(renderpassStorageFormats[FramebufferGBufferAttachments::F0_Roughness], pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT);
	pvrvk::AttachmentDescription gbufferAttachmentDepth = pvrvk::AttachmentDescription::createDepthStencilDescription(depthStencilFormat, pvrvk::ImageLayout::e_UNDEFINED,
		pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE);

	pvrvk::AttachmentReference gbufferAttachmentRef0 = pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
	pvrvk::AttachmentReference gbufferAttachmentRef1 = pvrvk::AttachmentReference(1, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
	pvrvk::AttachmentReference gbufferAttachmentRef2 = pvrvk::AttachmentReference(2, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
	pvrvk::AttachmentReference gbufferAttachmentRef3 = pvrvk::AttachmentReference(3, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
	pvrvk::AttachmentReference gbufferAttachmentRefDepth = pvrvk::AttachmentReference(4, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	pvrvk::SubpassDescription subpassDesc = pvrvk::SubpassDescription()
												.setColorAttachmentReference(0, gbufferAttachmentRef0)
												.setColorAttachmentReference(1, gbufferAttachmentRef1)
												.setColorAttachmentReference(2, gbufferAttachmentRef2)
												.setColorAttachmentReference(3, gbufferAttachmentRef3)
												.setDepthStencilAttachmentReference(gbufferAttachmentRefDepth);

	pvrvk::SubpassDependency dependency[2];

	dependency[0].setSrcSubpass(VK_SUBPASS_EXTERNAL);
	dependency[0].setDstSubpass(0);
	dependency[0].setSrcStageMask(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT);
	dependency[0].setDstStageMask(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT);
	dependency[0].setSrcAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
	dependency[0].setDstAccessMask(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT);
	dependency[0].setDependencyFlags(pvrvk::DependencyFlags::e_BY_REGION_BIT);

	dependency[1].setSrcSubpass(0);
	dependency[1].setDstSubpass(VK_SUBPASS_EXTERNAL);
	dependency[1].setSrcStageMask(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT);
	dependency[1].setDstStageMask(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT);
	dependency[1].setSrcAccessMask(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT);
	dependency[1].setDstAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
	dependency[1].setDependencyFlags(pvrvk::DependencyFlags::e_BY_REGION_BIT);

	pvrvk::RenderPassCreateInfo renderPassCreateInfo = pvrvk::RenderPassCreateInfo()
														   .setAttachmentDescription(0, gbufferAttachment0)
														   .setAttachmentDescription(1, gbufferAttachment1)
														   .setAttachmentDescription(2, gbufferAttachment2)
														   .setAttachmentDescription(3, gbufferAttachment3)
														   .setAttachmentDescription(4, gbufferAttachmentDepth)
														   .setSubpass(0, subpassDesc)
														   .addSubpassDependencies(dependency, 2);

	pvrvk::ImageView imageViews[] = { _deviceResources->gbufferImages[0], _deviceResources->gbufferImages[1], _deviceResources->gbufferImages[2],
		_deviceResources->gbufferImages[3], _deviceResources->gbufferDepthStencilImage };

	_deviceResources->gbufferRenderPass = _deviceResources->device->createRenderPass(renderPassCreateInfo);
	_deviceResources->gbufferRenderPass->setObjectName("GBufferRenderPass");

	_deviceResources->gbufferFramebuffer = _deviceResources->device->createFramebuffer(
		pvrvk::FramebufferCreateInfo(dimension.getWidth(), dimension.getHeight(), 1, _deviceResources->gbufferRenderPass, 5, &imageViews[0]));
}

/// <summary>Loads the mesh data required for this example into vertex and index buffer objects and populates material data.</summary>
/// <param name="uploadCmd">Command Buffer used to record the buffer upload commands.</param>
void VulkanHybridHardShadows::createModelBuffers(pvrvk::CommandBuffer& uploadCmd)
{
	struct Material
	{
		glm::vec4 baseColor = glm::vec4(1.0f);
		glm::vec4 metallicRoughnessReflectivity = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
		glm::vec4 f0f90;
	};

	uint32_t numMeshes = _scene->getNumMeshes();

	std::vector<Material> materials;
	_deviceResources->meshes.reserve(numMeshes);
	_deviceResources->vertexBuffers.reserve(numMeshes);
	_deviceResources->indexBuffers.reserve(numMeshes);
	_deviceResources->verticesSize.reserve(numMeshes);
	_deviceResources->indicesSize.reserve(numMeshes);
	_meshTransforms.reserve(numMeshes);

	// populate material data
	for (uint32_t i = 0; i < _scene->getNumMaterials(); i++)
	{
		auto& material = _scene->getMaterial(i);

		Material mat;

		mat.baseColor = glm::vec4(material.defaultSemantics().getDiffuse(), 1.0f);
		mat.baseColor = glm::vec4(glm::pow(glm::vec3(mat.baseColor.x, mat.baseColor.y, mat.baseColor.z), glm::vec3(2.2f)), 0.0f); // Srgb to linear
		mat.metallicRoughnessReflectivity = glm::vec4(1.0f, 0.1f, 0.85f, 0.0f);

		mat.metallicRoughnessReflectivity.r = mat.metallicRoughnessReflectivity.r > 0.001f ? 0.04f : mat.metallicRoughnessReflectivity.r;
		mat.f0f90 = glm::vec4(glm::vec3((0.16f * pow(mat.metallicRoughnessReflectivity.b, 2.0f))), 0.0f);
		mat.f0f90.w = glm::clamp(50.0f * glm::dot(glm::vec4(mat.f0f90.r, mat.f0f90.g, mat.f0f90.b, 1.0f), glm::vec4(0.33f)), 0.0f, 1.0f);

		// clamp roughness
		mat.metallicRoughnessReflectivity.g = glm::max(mat.metallicRoughnessReflectivity.g, 0.0004f);

		materials.emplace_back(mat);
	}

	// If there were none, add a default
	if (materials.empty()) materials.emplace_back(Material());

	// populate vertices, indices and material indices
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
			vertices.insert(vertices.begin() + vertexIndex,
				{
					glm::vec3(v[0], v[1], v[2]), // position
					glm::vec3(v[3], v[4], v[5]), // normals
					glm::vec2(v[6], v[7]), // texture coordinates
					glm::vec3(1.0) // tangent
				});
			vertexIndex++;
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
		_deviceResources->vertexBuffers.back()->setObjectName("AccelerationStructureMeshVBO");

		pvr::utils::updateBufferUsingStagingBuffer(
			_deviceResources->device, _deviceResources->vertexBuffers[meshIdx], uploadCmd, vertices.data(), 0, sizeof(pvr::utils::ASVertexFormat) * vertices.size());

		// create index buffer
		pvrvk::BufferCreateInfo indexBufferInfo;
		indexBufferInfo.setSize(sizeof(uint32_t) * indices.size());
		indexBufferInfo.setUsageFlags(pvrvk::BufferUsageFlags::e_INDEX_BUFFER_BIT | pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT |
			pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT | pvrvk::BufferUsageFlags::e_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);

		_deviceResources->indexBuffers.push_back(pvr::utils::createBuffer(_deviceResources->device, indexBufferInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			pvrvk::MemoryPropertyFlags::e_NONE, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE, pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT));
		_deviceResources->indexBuffers.back()->setObjectName("AccelerationStructureMeshIBO");

		pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, _deviceResources->indexBuffers[meshIdx], uploadCmd, indices.data(), 0, sizeof(uint32_t) * indices.size());

		_deviceResources->verticesSize.push_back(static_cast<int32_t>(vertices.size()));
		_deviceResources->indicesSize.push_back(static_cast<int32_t>(indices.size()));
	}

	// create material data buffer
	pvrvk::BufferCreateInfo materialColorBufferInfo;
	materialColorBufferInfo.setSize(sizeof(Material) * materials.size());
	materialColorBufferInfo.setUsageFlags(pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT);

	_deviceResources->materialBuffer = pvr::utils::createBuffer(_deviceResources->device, materialColorBufferInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT);
	_deviceResources->materialBuffer->setObjectName("MaterialSBO");

	pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, _deviceResources->materialBuffer, uploadCmd, materials.data(), 0, sizeof(Material) * materials.size());
}

/// <summary>Creates the scene wide buffer used throughout the demo.</summary>
void VulkanHybridHardShadows::createCameraBuffer()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(BufferEntryNames::PerScene::ViewMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::PerScene::ProjectionMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::PerScene::AmbientLightColor, pvr::GpuDatatypes::vec4);
	desc.addElement(BufferEntryNames::PerScene::CameraPosition, pvr::GpuDatatypes::vec4);
	desc.addElement(BufferEntryNames::PerScene::NumLights, pvr::GpuDatatypes::uinteger);

	_deviceResources->globalBufferView.initDynamic(desc, _numSwapImages, pvr::BufferUsageFlags::UniformBuffer,
		static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

	_deviceResources->globalBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->globalBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->globalBuffer->setObjectName("GlobalUBO");

	_deviceResources->globalBufferView.pointToMappedMemory(_deviceResources->globalBuffer->getDeviceMemory()->getMappedData());
}

/// <summary>Initializes the light sources in the Scene.</summary>
void VulkanHybridHardShadows::initializeLights()
{
	_averageLightColor = glm::vec4(glm::vec3(0.0f), 1.0f);

	// initiailise the scene lights
	for (uint32_t i = 0; i < _scene->getNumLights(); i++)
	{
		glm::vec4 lightPosition;
		_scene->getLightPosition(i, lightPosition);
		pvr::assets::Light light = _scene->getLight(i);

		PerLightData sceneLight;
		sceneLight.lightColor = glm::vec4(light.getColor(), 1.0f);
		sceneLight.lightPosition = lightPosition;
		sceneLight.lightIntensity = 1.5f;
		sceneLight.isProcedural = false;

		_lightData.push_back(sceneLight);

		_averageLightColor += glm::vec4(light.getColor(), 0.0f) * sceneLight.lightIntensity;
	}

#ifdef USE_EXTRA_LIGHTS
	for (uint32_t i = 0; i < NUM_EXTRA_LIGHTS; i++)
	{
		glm::vec3 lightColor = glm::vec3(pvr::randomrange(0, 1), pvr::randomrange(0, 1), pvr::randomrange(0, 1));
		lightColor / glm::max(glm::max(lightColor.x, lightColor.y), lightColor.z);

		PerLightData proceduralLight;
		proceduralLight.lightColor = glm::vec4(lightColor, 0.0);
		proceduralLight.lightIntensity = 1.5f;
		proceduralLight.isProcedural = true;

		proceduralLight.distance = pvr::randomrange(LightConfiguration::LightMinDistance, LightConfiguration::LightMaxDistance);
		proceduralLight.angle = pvr::randomrange(-glm::pi<float>(), glm::pi<float>());
		proceduralLight.height = pvr::randomrange(LightConfiguration::LightMinHeight, LightConfiguration::LightMaxHeight);
		proceduralLight.axial_vel = pvr::randomrange(-LightConfiguration::LightMaxAxialVelocity, LightConfiguration::LightMaxAxialVelocity);
		proceduralLight.radial_vel = pvr::randomrange(-LightConfiguration::LightMaxRadialVelocity, LightConfiguration::LightMaxRadialVelocity);
		proceduralLight.vertical_vel = pvr::randomrange(-LightConfiguration::LightMaxVerticalVelocity, LightConfiguration::LightMaxVerticalVelocity);

		float x = sin(proceduralLight.angle) * proceduralLight.distance;
		float z = cos(proceduralLight.angle) * proceduralLight.distance;
		float y = proceduralLight.height;

		proceduralLight.lightPosition = glm::vec4(x, y, z, 1.0);

		_lightData.push_back(proceduralLight);

		_averageLightColor += proceduralLight.lightColor * proceduralLight.lightIntensity;
	}
#endif

	// calculate an average ambient light color
	_averageLightColor /= (_scene->getNumLights()
#ifdef USE_EXTRA_LIGHTS
		+ NUM_EXTRA_LIGHTS
#endif
	);
	_averageLightColor *= LightConfiguration::AmbientColorScaler;
}

/// <summary>Creates the scene wide buffer used throughout the demo.</summary>
void VulkanHybridHardShadows::createMeshTransformBuffer()
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

/// <summary>Creates the Light data buffer.</summary>
void VulkanHybridHardShadows::createLightBuffer()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(BufferEntryNames::PerPointLightData::LightPosition, pvr::GpuDatatypes::vec4);
	desc.addElement(BufferEntryNames::PerPointLightData::LightColor, pvr::GpuDatatypes::vec4);
	desc.addElement(BufferEntryNames::PerPointLightData::LightIntensity, pvr::GpuDatatypes::Float);

	_deviceResources->lightDataBufferView.initDynamic(desc, LightConfiguration::MaxNumLights * _numSwapImages,
		pvr::BufferUsageFlags::UniformBuffer, static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

	_deviceResources->lightDataBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->lightDataBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->lightDataBuffer->setObjectName("LightDataBufferUBO");

	_deviceResources->lightDataBufferView.pointToMappedMemory(_deviceResources->lightDataBuffer->getDeviceMemory()->getMappedData());
}

/// <summary>Upload the static data to the buffers which do not change per frame.</summary>
void VulkanHybridHardShadows::uploadDynamicSceneData()
{
	// static scene properties buffer
	_farClipDistance = _scene->getCamera(static_cast<uint32_t>(SceneNodes::Cameras::SceneCamera)).getFar();

	uint32_t cameraDynamicSliceIdx = _deviceResources->swapchain->getSwapchainIndex();
	_deviceResources->globalBufferView.getElementByName(BufferEntryNames::PerScene::ViewMatrix, 0, cameraDynamicSliceIdx).setValue(_viewMatrix);
	_deviceResources->globalBufferView.getElementByName(BufferEntryNames::PerScene::ProjectionMatrix, 0, cameraDynamicSliceIdx).setValue(_projectionMatrix);
	_deviceResources->globalBufferView.getElementByName(BufferEntryNames::PerScene::AmbientLightColor, 0, cameraDynamicSliceIdx).setValue(_averageLightColor);
	_deviceResources->globalBufferView.getElementByName(BufferEntryNames::PerScene::CameraPosition, 0, cameraDynamicSliceIdx).setValue(glm::vec4(_cameraPosition, 0.0f));
	_deviceResources->globalBufferView.getElementByName(BufferEntryNames::PerScene::NumLights, 0, cameraDynamicSliceIdx).setValue(static_cast<uint32_t>(_lightData.size()));

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->globalBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->globalBuffer->getDeviceMemory()->flushRange(
			_deviceResources->globalBufferView.getDynamicSliceOffset(cameraDynamicSliceIdx), _deviceResources->globalBufferView.getDynamicSliceSize());
	}

	// upload light data
	uint32_t lightDynamicSliceIdx = _deviceResources->swapchain->getSwapchainIndex() * LightConfiguration::MaxNumLights;

	for (uint32_t i = 0; i < _lightData.size(); i++)
	{
		uint32_t dynamicSlice = i + lightDynamicSliceIdx;
		_deviceResources->lightDataBufferView.getElementByName(BufferEntryNames::PerPointLightData::LightPosition, 0, dynamicSlice).setValue(_lightData[i].lightPosition);
		_deviceResources->lightDataBufferView.getElementByName(BufferEntryNames::PerPointLightData::LightColor, 0, dynamicSlice).setValue(_lightData[i].lightColor);
		_deviceResources->lightDataBufferView.getElementByName(BufferEntryNames::PerPointLightData::LightIntensity, 0, dynamicSlice).setValue(_lightData[i].lightIntensity);
	}

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->lightDataBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->lightDataBuffer->getDeviceMemory()->flushRange(
			_deviceResources->lightDataBufferView.getDynamicSliceOffset(lightDynamicSliceIdx), _deviceResources->lightDataBufferView.getDynamicSliceSize() * _lightData.size());
	}

	// upload per mesh data
	for (uint32_t i = 0; i < _meshTransforms.size(); i++)
	{
		_deviceResources->perMeshBufferView.getElementByName(BufferEntryNames::PerMesh::WorldMatrix, i, _deviceResources->swapchain->getSwapchainIndex()).setValue(_meshTransforms[i]);
	}

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->perMeshBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->perMeshBuffer->getDeviceMemory()->flushRange(
			_deviceResources->perMeshBufferView.getDynamicSliceOffset(_deviceResources->swapchain->getSwapchainIndex()), _deviceResources->perMeshBufferView.getDynamicSliceSize());
	}
}

/// <summary>Updates animation variables and camera matrices.</summary>
void VulkanHybridHardShadows::updateAnimation()
{
	glm::vec3 vFrom, vTo, vUp;
	float fov;
	_scene->getCameraProperties(static_cast<uint32_t>(SceneNodes::Cameras::SceneCamera), fov, vFrom, vTo, vUp);

	static float angle = 0.0f;

	if (_animateCamera) angle += getFrameTime() * 0.01f;

	vFrom = glm::mat4_cast(glm::angleAxis(glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f))) * glm::vec4(vFrom, 1.0f);

	// Update camera matrices
	_cameraPosition = vFrom;
	_viewMatrix = glm::lookAt(_cameraPosition, vTo, glm::vec3(0.0f, 1.0f, 0.0f));
	_viewProjectionMatrix = _projectionMatrix * _viewMatrix;
	_inverseViewMatrix = glm::inverse(_viewMatrix);
}

/// <summary>Animates the procedural light sources in the scene.</summary>
void VulkanHybridHardShadows::updateProceduralLights()
{
#ifdef USE_EXTRA_LIGHTS

	for (uint32_t i = 0; i < _lightData.size(); i++)
	{
		if (_lightData[i].isProcedural)
		{
			uint64_t maxFrameTime = 30;
			float dt = (float)std::min(getFrameTime(), maxFrameTime);
			if (_lightData[i].distance < LightConfiguration::LightMinDistance)
			{
				_lightData[i].axial_vel = glm::abs(_lightData[i].axial_vel) + (LightConfiguration::LightMaxAxialVelocity * dt * .001f);
			}
			if (_lightData[i].distance > LightConfiguration::LightMaxDistance)
			{
				_lightData[i].axial_vel = -glm::abs(_lightData[i].axial_vel) - (LightConfiguration::LightMaxAxialVelocity * dt * .001f);
			}
			if (_lightData[i].height < LightConfiguration::LightMinHeight)
			{
				_lightData[i].vertical_vel = glm::abs(_lightData[i].vertical_vel) + (LightConfiguration::LightMaxAxialVelocity * dt * .001f);
			}
			if (_lightData[i].height > LightConfiguration::LightMaxHeight)
			{
				_lightData[i].vertical_vel = -glm::abs(_lightData[i].vertical_vel) - (LightConfiguration::LightMaxAxialVelocity * dt * .001f);
			}

			_lightData[i].axial_vel += pvr::randomrange(-LightConfiguration::LightAxialVelocityChange, LightConfiguration::LightAxialVelocityChange) * dt;

			_lightData[i].radial_vel += pvr::randomrange(-LightConfiguration::LightRadialVelocityChange, LightConfiguration::LightRadialVelocityChange) * dt;

			_lightData[i].vertical_vel += pvr::randomrange(-LightConfiguration::LightVerticalVelocityChange, LightConfiguration::LightVerticalVelocityChange) * dt;

			if (glm::abs(_lightData[i].axial_vel) > LightConfiguration::LightMaxAxialVelocity) { _lightData[i].axial_vel *= .8; }
			if (glm::abs(_lightData[i].radial_vel) > LightConfiguration::LightMaxRadialVelocity) { _lightData[i].radial_vel *= .8; }
			if (glm::abs(_lightData[i].vertical_vel) > LightConfiguration::LightMaxVerticalVelocity) { _lightData[i].vertical_vel *= .8; }

			_lightData[i].distance += _lightData[i].axial_vel * dt * 0.001f;
			_lightData[i].angle += _lightData[i].radial_vel * dt * 0.001f;
			_lightData[i].height += _lightData[i].vertical_vel * dt * 0.001f;

			float x = sin(_lightData[i].angle) * _lightData[i].distance;
			float z = cos(_lightData[i].angle) * _lightData[i].distance;
			float y = _lightData[i].height;

			_lightData[i].lightPosition = glm::vec4(x, y, z, 1.0);
		}
	}
#endif
}

/// <summary>Records main command buffer.</summary>
void VulkanHybridHardShadows::recordMainCommandBuffer()
{
	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		// Record deferred version
		{
			_deviceResources->cmdBufferMainDeferred[i]->begin();

			pvrvk::Rect2D renderArea(0, 0, _windowWidth, _windowHeight);

			// specify a clear colour per attachment
			const uint32_t numClearValues = FramebufferGBufferAttachments::Count + 1u;

			pvrvk::ClearValue gbufferClearValues[numClearValues] = { pvrvk::ClearValue(0.0, 0.0, 0.0, 0.0f), pvrvk::ClearValue(0.0, 0.0, 0.0, 0.0f),
				pvrvk::ClearValue(0.0, 0.0, 0.0, 0.0f), pvrvk::ClearValue(0.0, 0.0, 0.0, 0.0f), pvrvk::ClearValue(1.f, 0u) };

			// Render G-Buffer
			_deviceResources->cmdBufferMainDeferred[i]->beginRenderPass(_deviceResources->gbufferFramebuffer, renderArea, false, gbufferClearValues, numClearValues);

			_deviceResources->cmdBufferMainDeferred[i]->executeCommands(_deviceResources->cmdBufferGBuffer[i]);

			_deviceResources->cmdBufferMainDeferred[i]->endRenderPass();

			// Ray-trace shadows
			_deviceResources->cmdBufferMainDeferred[i]->executeCommands(_deviceResources->cmdBufferRayTracedShadows[i]);

			pvrvk::ClearValue onscreenClearValues[] = { pvrvk::ClearValue(0.0, 0.0, 0.0, 1.0f), pvrvk::ClearValue(1.f, 0u) };

			// Deferred shading + UI
			_deviceResources->cmdBufferMainDeferred[i]->beginRenderPass(_deviceResources->onScreenFramebuffer[i], renderArea, false, onscreenClearValues, 2);

			_deviceResources->cmdBufferMainDeferred[i]->executeCommands(_deviceResources->cmdBufferDeferredShading[i]);

			_deviceResources->cmdBufferMainDeferred[i]->endRenderPass();

			_deviceResources->cmdBufferMainDeferred[i]->end();
		}

		// Record forward version
		{
			_deviceResources->cmdBufferMainForward[i]->begin();

			pvrvk::Rect2D renderArea(0, 0, _windowWidth, _windowHeight);

			// specify a clear colour per attachment
			const uint32_t numClearValues = 2;

			pvrvk::ClearValue clearValues[numClearValues] = { pvrvk::ClearValue(0.0, 0.0, 0.0, 1.0f), pvrvk::ClearValue(1.f, 0u) };

			// begin the renderpass
			_deviceResources->cmdBufferMainForward[i]->beginRenderPass(_deviceResources->onScreenFramebuffer[i], renderArea, false, clearValues, numClearValues);

			// Render the forward shading + ui
			_deviceResources->cmdBufferMainForward[i]->executeCommands(_deviceResources->cmdBufferForwadShading[i]);

			_deviceResources->cmdBufferMainForward[i]->endRenderPass();

			_deviceResources->cmdBufferMainForward[i]->end();
		}
	}
}

/// <summary>Record all the secondary command buffers.</summary>
void VulkanHybridHardShadows::recordSecondaryCommandBuffers()
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
		recordCommandBufferRenderGBuffer(_deviceResources->cmdBufferGBuffer[i], i);
		_deviceResources->cmdBufferGBuffer[i]->end();

		_deviceResources->cmdBufferDeferredShading[i]->begin(_deviceResources->onScreenFramebuffer[i]);
		recordCommandBufferDeferredShading(_deviceResources->cmdBufferDeferredShading[i], i);
		recordCommandUIRenderer(_deviceResources->cmdBufferDeferredShading[i]);
		_deviceResources->cmdBufferDeferredShading[i]->end();

		_deviceResources->cmdBufferForwadShading[i]->begin(_deviceResources->onScreenFramebuffer[i]);
		recordCommandBufferForwardShading(_deviceResources->cmdBufferForwadShading[i], i);
		recordCommandUIRenderer(_deviceResources->cmdBufferForwadShading[i]);
		_deviceResources->cmdBufferForwadShading[i]->end();

		_deviceResources->cmdBufferRayTracedShadows[i]->begin();
		recordCommandBufferRayTraceShadows(_deviceResources->cmdBufferRayTracedShadows[i], i);
		_deviceResources->cmdBufferRayTracedShadows[i]->end();
	}
}

/// <summary>Record rendering G-Buffer commands.</summary>
/// <param name="cmdBuffers">SecondaryCommandbuffer to record.</param>
/// <param name="swapchainIndex">Index of the current swapchain image.</param>
void VulkanHybridHardShadows::recordCommandBufferRenderGBuffer(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex)
{
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffers, pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("G-Buffer - Swapchain (%i)", swapchainIndex)));

	uint32_t offsets[3] = {};
	offsets[0] = _deviceResources->globalBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[1] = _deviceResources->lightDataBufferView.getDynamicSliceOffset(swapchainIndex * LightConfiguration::MaxNumLights);
	offsets[2] = _deviceResources->perMeshBufferView.getDynamicSliceOffset(swapchainIndex);

	cmdBuffers->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->gbufferPipelineLayout, 0u, _deviceResources->commonDescriptorSet, offsets, 3);

	for (uint32_t meshIdx = 0; meshIdx < _deviceResources->meshes.size(); meshIdx++)
	{
		auto& mesh = _deviceResources->meshes[meshIdx];

		cmdBuffers->bindPipeline(_deviceResources->gbufferPipeline);

		cmdBuffers->pushConstants(_deviceResources->gbufferPipeline->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, sizeof(uint32_t), &meshIdx);

		int32_t matID = static_cast<uint32_t>(mesh.materialIdx);
		cmdBuffers->pushConstants(_deviceResources->gbufferPipeline->getPipelineLayout(), pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, sizeof(uint32_t), sizeof(uint32_t), &matID);

		cmdBuffers->bindVertexBuffer(_deviceResources->vertexBuffers[meshIdx], 0, 0);
		cmdBuffers->bindIndexBuffer(_deviceResources->indexBuffers[meshIdx], 0, mesh.indexType);
		cmdBuffers->drawIndexed(mesh.indexOffset, mesh.numIndices, 0, 0, 1);
	}

	pvr::utils::endCommandBufferDebugLabel(cmdBuffers);
}

/// <summary>Record forward rendering commands.</summary>
/// <param name="cmdBuffers">SecondaryCommandbuffer to record.</param>
/// <param name="swapchainIndex">Index of the current swapchain image.</param>
void VulkanHybridHardShadows::recordCommandBufferForwardShading(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex)
{
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffers, pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Forward Shading - Swapchain (%i)", swapchainIndex)));

	uint32_t offsets[3] = {};
	offsets[0] = _deviceResources->globalBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[1] = _deviceResources->lightDataBufferView.getDynamicSliceOffset(swapchainIndex * LightConfiguration::MaxNumLights);
	offsets[2] = _deviceResources->perMeshBufferView.getDynamicSliceOffset(swapchainIndex);

	cmdBuffers->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->forwardShadingPipelineLayout, 0u, _deviceResources->commonDescriptorSet, offsets, 3);

	for (uint32_t meshIdx = 0; meshIdx < _deviceResources->meshes.size(); meshIdx++)
	{
		auto& mesh = _deviceResources->meshes[meshIdx];

		cmdBuffers->bindPipeline(_deviceResources->forwardShadingPipeline);

		cmdBuffers->pushConstants(_deviceResources->forwardShadingPipeline->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, sizeof(uint32_t), &meshIdx);

		uint32_t matID = static_cast<uint32_t>(mesh.materialIdx);
		cmdBuffers->pushConstants(_deviceResources->forwardShadingPipeline->getPipelineLayout(), pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, sizeof(uint32_t), sizeof(uint32_t), &matID);

		cmdBuffers->bindVertexBuffer(_deviceResources->vertexBuffers[meshIdx], 0, 0);
		cmdBuffers->bindIndexBuffer(_deviceResources->indexBuffers[meshIdx], 0, mesh.indexType);
		cmdBuffers->drawIndexed(mesh.indexOffset, mesh.numIndices, 0, 0, 1);
	}

	pvr::utils::endCommandBufferDebugLabel(cmdBuffers);
}

/// <summary>Record ray-tracing commands.</summary>
/// <param name="cmdBuffers">SecondaryCommandbuffer to record.</param>
/// <param name="swapchainIndex">Index of the current swapchain image.</param>
void VulkanHybridHardShadows::recordCommandBufferRayTraceShadows(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex)
{
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffers, pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Ray Trace Shadows - Swapchain (%i)", swapchainIndex)));

	// NOTE:
	// Adapt the way the shader group size is computed. In a recent update in the original demos, they advice to use a new way
	// to do it. An example can be found in VulkanHelloRayTracing::buildShaderBindingTable and VulkanHelloRayTracing::raytrace
	// uint32_t shaderGroupSize = (_rtProperties.shaderGroupHandleSize + (uint32_t(_rtProperties.shaderGroupBaseAlignment) - 1)) & ~uint32_t(_rtProperties.shaderGroupBaseAlignment - 1);

	{
		pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_UNDEFINED;
		pvrvk::ImageLayout destinationImageLayout = pvrvk::ImageLayout::e_GENERAL;

		pvrvk::MemoryBarrierSet layoutTransitions;
		layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::AccessFlags::e_SHADER_WRITE_BIT,
			_deviceResources->raytraceShadowsImage->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), sourceImageLayout, destinationImageLayout,
			_deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));

		cmdBuffers->pipelineBarrier(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::PipelineStageFlags::e_RAY_TRACING_SHADER_BIT_KHR, layoutTransitions);
	}

	cmdBuffers->bindPipeline(_deviceResources->raytraceShadowPipeline);

	pvrvk::DescriptorSet arrayDS[] = { _deviceResources->gbufferDescriptorSet, _deviceResources->imageDescriptorSet, _deviceResources->commonDescriptorSet };

	uint32_t offsets[3] = {};
	offsets[0] = _deviceResources->globalBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[1] = _deviceResources->lightDataBufferView.getDynamicSliceOffset(swapchainIndex * LightConfiguration::MaxNumLights);
	offsets[2] = _deviceResources->perMeshBufferView.getDynamicSliceOffset(swapchainIndex);

	cmdBuffers->bindDescriptorSets(pvrvk::PipelineBindPoint::e_RAY_TRACING_KHR, _deviceResources->raytraceShadowPipelineLayout, 0, arrayDS, 3, offsets, 3);

	VkDeviceAddress sbtAddress = _deviceResources->raytraceShadowShaderBindingTable->getDeviceAddress(_deviceResources->device);

	uint32_t shaderGroupSize = (_rtProperties.shaderGroupHandleSize + (uint32_t(_rtProperties.shaderGroupBaseAlignment) - 1)) & ~uint32_t(_rtProperties.shaderGroupBaseAlignment - 1);
	uint32_t shaderGroupStride = shaderGroupSize;

	VkDeviceSize rayGenOffset = 0u * shaderGroupSize; // Start at the beginning of the SBT
	VkDeviceSize missOffset = 1u * shaderGroupSize; // Jump over raygen
	VkDeviceSize hitGroupOffset = 2u * shaderGroupSize; // Jump over the previous shaders

	pvrvk::StridedDeviceAddressRegionKHR raygenShaderBindingTable = { sbtAddress + rayGenOffset, shaderGroupStride, shaderGroupStride };
	pvrvk::StridedDeviceAddressRegionKHR missShaderBindingTable = { sbtAddress + missOffset, shaderGroupStride, shaderGroupStride };
	pvrvk::StridedDeviceAddressRegionKHR hitShaderBindingTable = { sbtAddress + hitGroupOffset, shaderGroupStride, shaderGroupStride };
	pvrvk::StridedDeviceAddressRegionKHR callableShaderBindingTable = {};

	cmdBuffers->traceRays(raygenShaderBindingTable, missShaderBindingTable, hitShaderBindingTable, callableShaderBindingTable, getWidth(), getHeight(), 1);

	{
		pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_GENERAL;
		pvrvk::ImageLayout destinationImageLayout = pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL;

		pvrvk::MemoryBarrierSet layoutTransitions;
		layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::AccessFlags::e_SHADER_WRITE_BIT,
			_deviceResources->raytraceShadowsImage->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), sourceImageLayout, destinationImageLayout,
			_deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));

		cmdBuffers->pipelineBarrier(pvrvk::PipelineStageFlags::e_RAY_TRACING_SHADER_BIT_KHR, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, layoutTransitions);
	}

	pvr::utils::endCommandBufferDebugLabel(cmdBuffers);
}

/// <summary>Record deferred shading commands.</summary>
/// <param name="cmdBuffers">SecondaryCommandbuffer to record.</param>
/// <param name="swapchainIndex">Index of the current swapchain image.</param>
void VulkanHybridHardShadows::recordCommandBufferDeferredShading(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex)
{
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffers, pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Deferred Shading - Swapchain (%i)", swapchainIndex)));

	cmdBuffers->bindPipeline(_deviceResources->defferedShadingPipeline);

	pvrvk::DescriptorSet arrayDS[] = { _deviceResources->commonDescriptorSet, _deviceResources->deferredShadingDescriptorSet };

	uint32_t offsets[3] = {};
	offsets[0] = _deviceResources->globalBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[1] = _deviceResources->lightDataBufferView.getDynamicSliceOffset(swapchainIndex * LightConfiguration::MaxNumLights);
	offsets[2] = _deviceResources->perMeshBufferView.getDynamicSliceOffset(swapchainIndex);

	cmdBuffers->bindDescriptorSets(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->deferredShadingPipelineLayout, 0, arrayDS, 2, offsets, 3);

	cmdBuffers->draw(0, 3);

	pvr::utils::endCommandBufferDebugLabel(cmdBuffers);
}

/// <summary>Record UIRenderer commands.</summary>
/// <param name="commandBuff">Commandbuffer to record.</param>
void VulkanHybridHardShadows::recordCommandUIRenderer(pvrvk::SecondaryCommandBuffer& commandBuff)
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

/// <summary>This function must be implemented by the user of the shell. The user should return its Shell object defining the
/// behaviour of the application.</summary>
/// <returns>Return an unique_ptr to a new Demo class,supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanHybridHardShadows>(); }
