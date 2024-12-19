/*!
\brief Implements a hybrid rendering technique with ray traced soft shadows with Temporal and Spatial denoising.
\file VulkanRayTracingDenoising.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "PVRShell/PVRShell.h"
#include "PVRVk/PVRVk.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRUtils/Vulkan/AccelerationStructure.h"
#include "vulkan/vulkan_beta.h"

// Maximum number of swap images supported
enum CONSTANTS
{
	MAX_NUMBER_OF_SWAP_IMAGES = 4
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
const char* const FullscreenTriangleVertexShader = "FullscreenTriangleVertexShader.vsh.spv";
const char* const ShadowsTemporalDenoiseComputeShader = "ShadowsTemporalDenoiseComputeShader.csh.spv";
const char* const ShadowsSpatialDenoiseComputeShader = "ShadowsSpatialDenoiseComputeShader.csh.spv";
const char* const ShadowsDownsampleComputeShader = "ShadowsDownsampleComputeShader.csh.spv";
} // namespace Files

/// <summary>buffer entry names used for the structured memory views used throughout the demo.
/// These entry names must match the variable names used in the demo shaders.</summary>
namespace BufferEntryNames {
namespace PerScene {
const char* const ViewMatrix = "mViewMatrix";
const char* const ProjectionMatrix = "mProjectionMatrix";
const char* const PrevViewProjMatrix = "mPrevViewProjMatrix";
const char* const ViewProjInverseMatrix = "mViewProjInverseMatrix";
const char* const PrevViewProjInverseMatrix = "mPrevViewProjInverseMatrix";
const char* const AmbientLightColor = "vAmbientLightColor";
const char* const CameraPosition = "vCameraPosition";
} // namespace PerScene

namespace PerMesh {
const char* const WorldMatrix = "mWorldMatrix";
} // namespace PerMesh

namespace PerPointLightData {
const char* const LightColor = "vLightColor";
const char* const LightPosition = "vLightPosition";
const char* const LightIntensity = "fLightIntensity";
const char* const LightRadius = "fLightRadius";
} // namespace PerPointLightData
} // namespace BufferEntryNames

// Application wide configuration data
namespace ApplicationConfiguration {
const float MaxAnimatedLightRadius = 0.15f;
} // namespace ApplicationConfiguration

//// Demo Configuration data
// Light configuration data including handling the way the procedural light source moves
namespace LightConfiguration {
glm::vec4 AmbientColorScaler = glm::vec4(0.18f, 0.18f, 0.18f, 1.0);

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

/// <summary>Struct where to store information each Light source in the scene.</summary>
struct PerLightData
{
	glm::vec4 lightColor;
	glm::vec4 lightPosition;
	float lightIntensity = 0.0f;
	float lightRadius = 0.1f;
	bool isProcedural = false;
	float radial_vel = 0.0f;
	float axial_vel = 0.0f;
	float vertical_vel = 0.0f;
	float angle = 0.0f;
	float distance = 0.0f;
	float height = 0.0f;
};

/// <summary>Struct where to store information about the scene elements for the deferred shading pass.</summary>
struct MeshAS
{
	/// <summary>Parameter constructor.</summary>
	MeshAS(int materialIdxParam, int indexOffsetParam, int numIndicesParam, glm::mat4 worldMatrixParam, pvrvk::IndexType indexTypeParam)
		: materialIdx(materialIdxParam), indexOffset(indexOffsetParam), numIndices(numIndicesParam), worldMatrix(worldMatrixParam), indexType(indexTypeParam)
	{}

	/// <summary>Material index used by this scene element.</summary>
	int materialIdx;

	/// <summary>Offset inside the index buffer for rasterizing this scene element.</summary>
	int indexOffset;

	/// <summary>Num indices of this scene element, used when rasterizing.</summary>
	int numIndices;

	/// <summary>Scene element transform.</summary>
	glm::mat4 worldMatrix;

	/// <summary>Enum to specify whether the indices of the index buffer are 16-bit or 32-bit unsigned int values.</summary>
	pvrvk::IndexType indexType;
};

struct DeviceResources
{
	pvrvk::Instance instance;
	pvrvk::Surface surface;
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
	pvrvk::Device device;
	pvrvk::Queue queue;
	pvr::utils::QueueAccessInfo queueAccessInfo;
	pvrvk::Swapchain swapchain;
	pvr::utils::vma::Allocator vmaAllocator;
	pvrvk::CommandPool commandPool;
	pvrvk::DescriptorPool descriptorPool;

	// Stores Texture views for the Images used as attachments on the G-buffer
	pvrvk::ImageView gbufferImages[2][FramebufferGBufferAttachments::Count];
	pvrvk::ImageView gbufferDepthStencilImage[2];

	// Image view for the Shadow Mask image
	pvrvk::ImageView rtShadowsImage;
	pvrvk::ImageView rtShadowsDownsampledMipImageViews[4];
	pvrvk::ImageView rtShadowsDownsampledAllMipsImageView;

	// Image view for Shadows temporal accumulation
	pvrvk::ImageView rtShadowsTemporalImage[2];

	// Image view for the Spatial Mask image
	pvrvk::ImageView rtShadowsSpatialImage;

	// Framebuffer for the G-buffer
	pvrvk::Framebuffer gbufferFramebuffer[2];

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
	pvrvk::SecondaryCommandBuffer cmdBufferShadowsDownsample[MAX_NUMBER_OF_SWAP_IMAGES];
	pvrvk::SecondaryCommandBuffer cmdBufferShadowsTemporal[MAX_NUMBER_OF_SWAP_IMAGES];
	pvrvk::SecondaryCommandBuffer cmdBufferShadowsSpatial[MAX_NUMBER_OF_SWAP_IMAGES];

	////  Descriptor Set Layouts ////
	pvrvk::DescriptorSetLayout commonDescriptorSetLayout;
	pvrvk::DescriptorSetLayout gbufferDescriptorSetLayout;
	pvrvk::DescriptorSetLayout storageImageDescriptorSetLayout;
	pvrvk::DescriptorSetLayout combinedSamplerDescriptorSetLayout;
	pvrvk::DescriptorSetLayout deferredShadingDescriptorSetLayout;
	pvrvk::DescriptorSetLayout temporalWriteDescriptorSetLayout;
	pvrvk::DescriptorSetLayout shadowsDownsampleDescriptorSetLayout;

	////  Descriptor Sets ////
	pvrvk::DescriptorSet commonDescriptorSet;
	pvrvk::DescriptorSet iblDescriptorSet;
	pvrvk::DescriptorSet gbufferDescriptorSet[2];
	pvrvk::DescriptorSet rtShadowsTemporalImageWriteDescriptorSet[2];
	pvrvk::DescriptorSet rtShadowsTemporalImageReadDescriptorSet[2];
	pvrvk::DescriptorSet rtShadowsSpatialImageWriteDescriptorSet;
	pvrvk::DescriptorSet deferredShadingDescriptorSet[2];
	pvrvk::DescriptorSet deferredShadingNoDenoisingDescriptorSet[2];
	pvrvk::DescriptorSet shadowsDownsampleDescriptorSet;

	//// Pipeline Layouts ////
	// GBuffer pipeline layouts
	pvrvk::PipelineLayout gbufferPipelineLayout;

	// Deferred shading pipeline layout
	pvrvk::PipelineLayout deferredShadingPipelineLayout;

	// Bindless scene resources
	std::vector<pvrvk::Buffer> vertexBuffers;
	std::vector<pvrvk::Buffer> indexBuffers;
	std::vector<pvrvk::Buffer> materialIndexBuffers;
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
	pvr::utils::StructuredBufferView perMeshPrevTransformBufferView;
	pvrvk::Buffer perMeshPrevTransformBuffer;

	std::vector<pvrvk::Semaphore> imageAcquiredSemaphores;
	std::vector<pvrvk::Semaphore> presentationSemaphores;
	std::vector<pvrvk::Fence> perFrameResourcesFences;

	//// Pipelines ////
	// G-buffer pass
	pvrvk::GraphicsPipeline gbufferPipeline;

	// Deferred shading pass
	pvrvk::GraphicsPipeline defferedShadingPipeline;

	// RT shadow temporal denoise pass
	pvrvk::PipelineLayout shadowsTemporalPipelineLayout;
	pvrvk::ComputePipeline shadowsTemporalPipeline;

	// RT shadow spatial denoise pass
	pvrvk::PipelineLayout shadowsSpatialPipelineLayout;
	pvrvk::ComputePipeline shadowsSpatialPipeline;

	// RT shadow downsample pass
	pvrvk::PipelineLayout shadowsDownsamplePipelineLayout;
	pvrvk::ComputePipeline shadowsDownsamplePipeline;

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
class VulkanRayTracingDenoising : public pvr::Shell
{
public:
	//// Frame ////
	uint32_t _numSwapImages;
	uint32_t _swapchainIndex;
	// Putting all API objects into a pointer just makes it easier to release them all together with RAII
	std::unique_ptr<DeviceResources> _deviceResources;

	VkPhysicalDeviceRayTracingPipelinePropertiesKHR _rtProperties;
	uint32_t _rtShadowsShaderGroupCount = 0;
	uint32_t _rtReflectionsShaderGroupCount = 0;

	// per light data
	PerLightData _lightData;
	glm::vec4 _averageLightColor;

	// Frame counters for animation
	uint32_t _frameId;
	uint32_t _frameIdx;
	bool _animateCamera;
	bool _animateLightRadius;
	bool _denoise;
	bool _pingPong;
	float _frame = 0.0f;

	// Projection and Model View matrices
	glm::mat4 _viewMatrix;
	glm::mat4 _projectionMatrix;
	glm::mat4 _viewProjectionMatrix;
	glm::mat4 _prevViewProjectionMatrix;
	glm::mat4 _inverseViewProjectionMatrix;
	glm::mat4 _inversePrevViewProjectionMatrix;
	glm::vec3 _cameraPosition;
	std::vector<glm::mat4> _meshTransforms;
	std::vector<glm::mat4> _prevMeshTransforms;
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

	VulkanRayTracingDenoising()
	{
		_animateCamera = false;
		_animateLightRadius = false;
		_denoise = true;
	}

	//  Overridden from pvr::Shell
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void updateScene();
	pvr::Result createDeviceAndQueues();
	void createImages();
	void createFramebufferAndRenderPass();
	void createPipelines();
	void createGBufferPipelines();
	void createDeferredShadingPipelines();
	void createShadowsDownsamplePipeline();
	void createShadowsTemporalPipeline();
	void createShadowsSpatialPipeline();
	void recordMainCommandBuffer(uint32_t swapchainIndex);
	void recordSecondaryCommandBuffers(uint32_t swapchainIndex);
	void recordCommandBufferRenderGBuffer(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex);
	void recordCommandBufferDeferredShading(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex);
	void recordCommandBufferShadowsDownsample(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex);
	void recordCommandBufferShadowsTemporal(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex);
	void recordCommandBufferShadowsSpatial(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex);
	void recordCommandUIRenderer(pvrvk::SecondaryCommandBuffer& cmdBuffers);
	void createDescriptorSetLayouts();
	void createDescriptorSets();
	void uploadDynamicSceneData();
	void createCameraBuffer();
	void createMeshTransformBuffer();
	void initializeLights();
	void createLightBuffer();
	void updateAnimation();
	void createModelBuffers(pvrvk::CommandBuffer& uploadCmd);

	void eventMappedInput(pvr::SimplifiedInput key)
	{
		switch (key)
		{
		// Handle input
		case pvr::SimplifiedInput::ActionClose: exitShell(); break;
		case pvr::SimplifiedInput::Action1: _denoise = !_denoise; break;
		case pvr::SimplifiedInput::Action2: _animateCamera = !_animateCamera; break;
		case pvr::SimplifiedInput::Action3: _animateLightRadius = !_animateLightRadius; break;
		default: break;
		}

		updateDescription();
	}

	void updateDescription() {}
};

/// <summary> Code in initApplication() will be called by pvr::Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns> Return true if no error occurred. </returns>
pvr::Result VulkanRayTracingDenoising::initApplication()
{
	_frameIdx = 0;
	_frameId = 0;
	_pingPong = false;

	//  Load the scene
	_scene = pvr::assets::loadModel(*this, Files::SceneFile);

	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.).</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanRayTracingDenoising::initView()
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

	// Create the surface
	_deviceResources->surface =
		pvr::utils::createSurface(_deviceResources->instance, _deviceResources->instance->getPhysicalDevice(0), this->getWindow(), this->getDisplay(), this->getConnection());

	// Filter UNASSIGNED-BestPractices-vkAllocateMemory-small-allocation Best Practices performance warning which has ID -602362517 for TLAS buffer build and
	// update (VkBufferDeviceAddressInfo requires VkBuffer handle so in general it's not possible to make a single buffer to put all information
	// and use offsets inside it
	vectorValidationIDFilter.push_back(-602362517);
	// Filter UNASSIGNED-BestPractices-vkBindMemory-small-dedicated-allocation performance warning recommending to do buffer allocations of at least 1048576 bytes
	vectorValidationIDFilter.push_back(-1277938581);

	// Create a default set of debug utils messengers or debug callbacks using either VK_EXT_debug_utils or VK_EXT_debug_report respectively
	_deviceResources->debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(_deviceResources->instance, (void*)&vectorValidationIDFilter);

	// Create device and queues
	pvr::Result resultDeviceAndQueues = createDeviceAndQueues();

	if (resultDeviceAndQueues != pvr::Result::Success) { return resultDeviceAndQueues; }

	// get queue
	_deviceResources->queue = _deviceResources->device->getQueue(_deviceResources->queueAccessInfo.familyId, _deviceResources->queueAccessInfo.queueId);
	_deviceResources->queue->setObjectName("GraphicsQueue");

	// Create vulkan memory allocatortea
	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->device->getPhysicalDevice()->getSurfaceCapabilities(_deviceResources->surface);

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
	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, _deviceResources->surface, getDisplayAttributes(),
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
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(
		pvrvk::CommandPoolCreateInfo(_deviceResources->queueAccessInfo.familyId, pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

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
		_deviceResources->cmdBufferShadowsDownsample[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->cmdBufferShadowsTemporal[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->cmdBufferShadowsSpatial[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();

		_deviceResources->cmdBufferMainDeferred[i]->setObjectName("DeferredCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->cmdBufferGBuffer[i]->setObjectName("GBufferSecondaryCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->cmdBufferDeferredShading[i]->setObjectName("DeferredShadingSecondaryCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->cmdBufferShadowsDownsample[i]->setObjectName("ShadowsDownsampleSecondaryCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->cmdBufferShadowsTemporal[i]->setObjectName("ShadowsTemporalSecondaryCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->cmdBufferShadowsSpatial[i]->setObjectName("ShadowsSpatialSecondaryCommandBufferSwapchain" + std::to_string(i));

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
			_scene->getCamera(0).getFar(), _scene->getCamera(0).getNear(), glm::pi<float>() * .5f);
	}
	else
	{
		_projectionMatrix = pvr::math::perspective(pvr::Api::Vulkan, _scene->getCamera(0).getFOV(), static_cast<float>(this->getWidth()) / static_cast<float>(this->getHeight()),
			_scene->getCamera(0).getFar(), _scene->getCamera(0).getNear());
	}

	// Initialize UIRenderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queue);
	_deviceResources->uiRenderer.getDefaultTitle()->setText("RayTracingDenoising");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText("Action 1: Toggle Denoising\n"
															   "Action 2: Toggle Camera Animation\n"
															   "Action 3: Toggle Light Radius Animation");
	updateDescription();
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	// get ray tracing properties
	_rtProperties.sType = static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR);
	_rtProperties.pNext = nullptr;
	VkPhysicalDeviceProperties2 properties{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_PROPERTIES_2) };
	properties.pNext = &_rtProperties;
	_deviceResources->instance->getVkBindings().vkGetPhysicalDeviceProperties2(_deviceResources->device->getPhysicalDevice()->getVkHandle(), &properties);

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

	initializeLights();
	createImages();
	createFramebufferAndRenderPass();
	createLightBuffer();
	createCameraBuffer();
	createMeshTransformBuffer();
	createDescriptorSetLayouts();
	createPipelines();

	_deviceResources->accelerationStructure.buildASModelDescription(
		_deviceResources->vertexBuffers, _deviceResources->indexBuffers, _deviceResources->verticesSize, _deviceResources->indicesSize, _meshTransforms);
	_deviceResources->accelerationStructure.buildAS(_deviceResources->device, _deviceResources->queue, _deviceResources->cmdBufferMainDeferred[0],
		pvrvk::BuildAccelerationStructureFlagsKHR::e_PREFER_FAST_TRACE_BIT_KHR | pvrvk::BuildAccelerationStructureFlagsKHR::e_ALLOW_UPDATE_BIT_KHR);

	createDescriptorSets();

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanRayTracingDenoising::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
///	If the rendering context is lost, quitApplication() will not be called.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanRayTracingDenoising::quitApplication()
{
	_scene.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return Result::Success if no error occurred</returns>
pvr::Result VulkanRayTracingDenoising::renderFrame()
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

	// Record command buffers
	recordSecondaryCommandBuffers(_swapchainIndex);
	recordMainCommandBuffer(_swapchainIndex);

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

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();
	_frameIdx++;
	_pingPong = !_pingPong;

	return pvr::Result::Success;
}

/// <summary>Updates the scene animation and takes the new mesh transforms and updates the TLAS.</summary>
void VulkanRayTracingDenoising::updateScene()
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

		_prevMeshTransforms[i] = _frameIdx == 0 ? transform : _meshTransforms[i];
		_meshTransforms[i] = transform;
		_deviceResources->meshes[i].worldMatrix = transform;
	}

	_deviceResources->accelerationStructure.updateInstanceTransformData(_meshTransforms);

	pvrvk::CommandBuffer commandBuffer = _deviceResources->commandPool->allocateCommandBuffer();

	_deviceResources->accelerationStructure.buildTopLevelASAndInstances(_deviceResources->device, commandBuffer, _deviceResources->queue,
		pvrvk::BuildAccelerationStructureFlagsKHR::e_PREFER_FAST_TRACE_BIT_KHR | pvrvk::BuildAccelerationStructureFlagsKHR::e_ALLOW_UPDATE_BIT_KHR, true);
}

/// <summary>Build the device and queues, adding the required extensions for the demo, including the Vulkan ray tracing ones.</summary>
/// <returns>pvr::Result Success if initialisation succeeded, pvr::Result UnsupportedRequest if the extensions are not
/// supported by the physical device.</returns>
pvr::Result VulkanRayTracingDenoising::createDeviceAndQueues()
{
	const pvr::utils::QueuePopulateInfo queuePopulateInfo = { pvrvk::QueueFlags::e_GRAPHICS_BIT, _deviceResources->surface };

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
	_deviceResources->device = pvr::utils::createDeviceAndQueues(
		_deviceResources->instance->getPhysicalDevice(vectorPhysicalDevicesIndex[0]), &queuePopulateInfo, 1, &_deviceResources->queueAccessInfo, deviceExtensions);

	return pvr::Result::Success;
}

/// <summary>Creates descriptor set layouts.</summary>
void VulkanRayTracingDenoising::createDescriptorSetLayouts()
{
	// Common Descriptor Set Layout
	pvrvk::DescriptorSetLayoutCreateInfo commonDescSetInfo;

	// Dynamic per scene buffer
	commonDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u,
		pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	// Dynamic per light buffer
	commonDescSetInfo.setBinding(1, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u,
		pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR);
	// Static material data buffer
	commonDescSetInfo.setBinding(2, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1u,
		pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR | pvrvk::ShaderStageFlags::e_ANY_HIT_BIT_KHR);
	// Static material indices buffer
	commonDescSetInfo.setBinding(3, pvrvk::DescriptorType::e_STORAGE_BUFFER, static_cast<uint16_t>(_deviceResources->materialIndexBuffers.size()),
		pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR | pvrvk::ShaderStageFlags::e_ANY_HIT_BIT_KHR);
	// TLAS
	commonDescSetInfo.setBinding(4, pvrvk::DescriptorType::e_ACCELERATION_STRUCTURE_KHR, 1u,
		pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	// Vertex buffers
	commonDescSetInfo.setBinding(5, pvrvk::DescriptorType::e_STORAGE_BUFFER, static_cast<uint16_t>(_deviceResources->vertexBuffers.size()),
		pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	// Index buffers
	commonDescSetInfo.setBinding(6, pvrvk::DescriptorType::e_STORAGE_BUFFER, static_cast<uint16_t>(_deviceResources->indexBuffers.size()),
		pvrvk::ShaderStageFlags::e_CLOSEST_HIT_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	// Dynamic per mesh buffer
	commonDescSetInfo.setBinding(7, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	// Dynamic prev per mesh buffer
	commonDescSetInfo.setBinding(8, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	_deviceResources->commonDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(commonDescSetInfo);

	// GBuffer Descriptor Set Layout

	pvrvk::DescriptorSetLayoutCreateInfo gbufferDescSetInfo;
	gbufferDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u,
		pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	gbufferDescSetInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u,
		pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	gbufferDescSetInfo.setBinding(2, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u,
		pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	gbufferDescSetInfo.setBinding(3, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u,
		pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	gbufferDescSetInfo.setBinding(4, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u,
		pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_COMPUTE_BIT);

	_deviceResources->gbufferDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(gbufferDescSetInfo);

	// Storage Image Descriptor Set Layout

	pvrvk::DescriptorSetLayoutCreateInfo storageImageDescSetInfo;
	storageImageDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_STORAGE_IMAGE, 1u, pvrvk::ShaderStageFlags::e_COMPUTE_BIT | pvrvk::ShaderStageFlags::e_RAYGEN_BIT_KHR);

	_deviceResources->storageImageDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(storageImageDescSetInfo);

	// Combined Sampler Descriptor Set Layout

	pvrvk::DescriptorSetLayoutCreateInfo combinedSamplerDescSetInfo;
	combinedSamplerDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_COMPUTE_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	_deviceResources->combinedSamplerDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(combinedSamplerDescSetInfo);

	// Deferred Shading Descriptor Set Layout

	pvrvk::DescriptorSetLayoutCreateInfo defferedShadingDescSetInfo;
	defferedShadingDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	defferedShadingDescSetInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	defferedShadingDescSetInfo.setBinding(2, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	defferedShadingDescSetInfo.setBinding(3, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	defferedShadingDescSetInfo.setBinding(4, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	defferedShadingDescSetInfo.setBinding(5, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	_deviceResources->deferredShadingDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(defferedShadingDescSetInfo);

	// Temporal Write Descriptor Set Layout
	pvrvk::DescriptorSetLayoutCreateInfo temporalWriteDescSetInfo;
	temporalWriteDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_STORAGE_IMAGE, 1u, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	temporalWriteDescSetInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	temporalWriteDescSetInfo.setBinding(2, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	temporalWriteDescSetInfo.setBinding(3, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);

	_deviceResources->temporalWriteDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(temporalWriteDescSetInfo);

	// Downsample Descriptor Set Layout
	pvrvk::DescriptorSetLayoutCreateInfo downsampleDescSetInfo;
	downsampleDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_STORAGE_IMAGE, 1u, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	downsampleDescSetInfo.setBinding(1, pvrvk::DescriptorType::e_STORAGE_IMAGE, 1u, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	downsampleDescSetInfo.setBinding(2, pvrvk::DescriptorType::e_STORAGE_IMAGE, 1u, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	downsampleDescSetInfo.setBinding(3, pvrvk::DescriptorType::e_STORAGE_IMAGE, 1u, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	downsampleDescSetInfo.setBinding(4, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);

	_deviceResources->shadowsDownsampleDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(downsampleDescSetInfo);
}

/// <summary>Creates descriptor sets.</summary>
void VulkanRayTracingDenoising::createDescriptorSets()
{
	// Scene Samplers

	pvrvk::SamplerCreateInfo samplerDesc;
	samplerDesc.wrapModeU = samplerDesc.wrapModeV = samplerDesc.wrapModeW = pvrvk::SamplerAddressMode::e_REPEAT;

	samplerDesc.minFilter = pvrvk::Filter::e_LINEAR;
	samplerDesc.magFilter = pvrvk::Filter::e_LINEAR;
	samplerDesc.mipMapMode = pvrvk::SamplerMipmapMode::e_LINEAR;
	pvrvk::Sampler samplerTrilinear = _deviceResources->device->createSampler(samplerDesc);

	samplerDesc.wrapModeU = samplerDesc.wrapModeV = samplerDesc.wrapModeW = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;

	samplerDesc.minFilter = pvrvk::Filter::e_NEAREST;
	samplerDesc.magFilter = pvrvk::Filter::e_NEAREST;
	samplerDesc.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
	pvrvk::Sampler samplerNearest = _deviceResources->device->createSampler(samplerDesc);

	samplerDesc.minFilter = pvrvk::Filter::e_LINEAR;
	samplerDesc.magFilter = pvrvk::Filter::e_LINEAR;
	samplerDesc.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
	pvrvk::Sampler samplerBilinear = _deviceResources->device->createSampler(samplerDesc);

	// Allocate Descriptor Sets

	_deviceResources->commonDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->commonDescriptorSetLayout);
	_deviceResources->rtShadowsTemporalImageWriteDescriptorSet[0] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->temporalWriteDescriptorSetLayout);
	_deviceResources->rtShadowsTemporalImageWriteDescriptorSet[1] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->temporalWriteDescriptorSetLayout);
	_deviceResources->rtShadowsTemporalImageReadDescriptorSet[0] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->combinedSamplerDescriptorSetLayout);
	_deviceResources->rtShadowsTemporalImageReadDescriptorSet[1] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->combinedSamplerDescriptorSetLayout);
	_deviceResources->rtShadowsSpatialImageWriteDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->storageImageDescriptorSetLayout);
	_deviceResources->deferredShadingDescriptorSet[0] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->deferredShadingDescriptorSetLayout);
	_deviceResources->deferredShadingDescriptorSet[1] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->deferredShadingDescriptorSetLayout);
	_deviceResources->deferredShadingNoDenoisingDescriptorSet[0] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->deferredShadingDescriptorSetLayout);
	_deviceResources->deferredShadingNoDenoisingDescriptorSet[1] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->deferredShadingDescriptorSetLayout);
	_deviceResources->gbufferDescriptorSet[0] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->gbufferDescriptorSetLayout);
	_deviceResources->gbufferDescriptorSet[1] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->gbufferDescriptorSetLayout);
	_deviceResources->shadowsDownsampleDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->shadowsDownsampleDescriptorSetLayout);

	_deviceResources->commonDescriptorSet->setObjectName("");
	_deviceResources->rtShadowsTemporalImageWriteDescriptorSet[0]->setObjectName("RTShadowsTemporalImage0WriteDescriptorSet");
	_deviceResources->rtShadowsTemporalImageWriteDescriptorSet[1]->setObjectName("RTShadowsTemporalImage1WriteDescriptorSet");
	_deviceResources->rtShadowsTemporalImageReadDescriptorSet[0]->setObjectName("RTShadowsTemporalImage0ReadDescriptorSet");
	_deviceResources->rtShadowsTemporalImageReadDescriptorSet[1]->setObjectName("RTShadowsTemporalImage1ReadDescriptorSet");
	_deviceResources->rtShadowsSpatialImageWriteDescriptorSet->setObjectName("RTShadowsSpatialImageWriteDescriptorSet");
	_deviceResources->deferredShadingDescriptorSet[0]->setObjectName("DeferredShading0DescriptorSet");
	_deviceResources->deferredShadingDescriptorSet[1]->setObjectName("DeferredShading1DescriptorSet");
	_deviceResources->deferredShadingNoDenoisingDescriptorSet[0]->setObjectName("DeferredShadingNoDenoising0DescriptorSet");
	_deviceResources->deferredShadingNoDenoisingDescriptorSet[1]->setObjectName("DeferredShadingNoDenoising1DescriptorSet");
	_deviceResources->gbufferDescriptorSet[0]->setObjectName("GBuffer0DescriptorSet");
	_deviceResources->gbufferDescriptorSet[1]->setObjectName("GBuffer1DescriptorSet");
	_deviceResources->shadowsDownsampleDescriptorSet->setObjectName("ShadowsDownsampleDescriptorSet");

	// Write Common Descriptor Set

	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->commonDescriptorSet, 0)
								.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->globalBuffer, 0, _deviceResources->globalBufferView.getDynamicSliceSize())));

	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->commonDescriptorSet, 1)
								.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->lightDataBuffer, 0, _deviceResources->lightDataBufferView.getDynamicSliceSize())));

	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->commonDescriptorSet, 2)
								.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->materialBuffer, 0, _deviceResources->materialBuffer->getSize())));

	// Write Material Indices

	pvrvk::WriteDescriptorSet materialIndicesSetWrite = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->commonDescriptorSet, 3);

	for (size_t i = 0; i < _deviceResources->materialIndexBuffers.size(); i++)
		materialIndicesSetWrite.setBufferInfo(i, pvrvk::DescriptorBufferInfo(_deviceResources->materialIndexBuffers[i], 0, _deviceResources->materialIndexBuffers[i]->getSize()));

	writeDescSets.push_back(materialIndicesSetWrite);

	// Write TLAS

	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_ACCELERATION_STRUCTURE_KHR, _deviceResources->commonDescriptorSet, 4)
								.setAccelerationStructureInfo(0, _deviceResources->accelerationStructure.getTopLevelAccelerationStructure()));

	// Write Vertices

	pvrvk::WriteDescriptorSet verticesSetWrite = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->commonDescriptorSet, 5);

	for (size_t i = 0; i < _deviceResources->vertexBuffers.size(); i++)
		verticesSetWrite.setBufferInfo(i, pvrvk::DescriptorBufferInfo(_deviceResources->vertexBuffers[i], 0, _deviceResources->vertexBuffers[i]->getSize()));

	writeDescSets.push_back(verticesSetWrite);

	// Write Indices

	pvrvk::WriteDescriptorSet indicesSetWrite = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->commonDescriptorSet, 6);

	for (size_t i = 0; i < _deviceResources->indexBuffers.size(); i++)
		indicesSetWrite.setBufferInfo(i, pvrvk::DescriptorBufferInfo(_deviceResources->indexBuffers[i], 0, _deviceResources->indexBuffers[i]->getSize()));

	writeDescSets.push_back(indicesSetWrite);

	// Write Dynamic mesh transforms UBO

	writeDescSets.push_back(
		pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->commonDescriptorSet, 7)
			.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->perMeshBuffer, 0, _deviceResources->perMeshBufferView.getDynamicSliceSize() * _meshTransforms.size())));

	// Write Dynamic prev mesh transforms UBO

	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->commonDescriptorSet, 8)
								.setBufferInfo(0,
									pvrvk::DescriptorBufferInfo(_deviceResources->perMeshPrevTransformBuffer, 0,
										_deviceResources->perMeshPrevTransformBufferView.getDynamicSliceSize() * _prevMeshTransforms.size())));

	// Write GBuffer Descriptor Set

	for (int pingPong = 0; pingPong < 2; pingPong++)
	{
		for (int i = 0; i < FramebufferGBufferAttachments::Count; i++)
			writeDescSets.push_back(
				pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->gbufferDescriptorSet[pingPong], i)
					.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->gbufferImages[pingPong][i], samplerNearest, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->gbufferDescriptorSet[pingPong], FramebufferGBufferAttachments::Count)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->gbufferDepthStencilImage[pingPong], samplerNearest, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
	}

	// Write RT Shadows Spatial Image Write Descriptor Set

	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_IMAGE, _deviceResources->rtShadowsSpatialImageWriteDescriptorSet, 0)
								.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->rtShadowsSpatialImage, pvrvk::ImageLayout::e_GENERAL)));

	for (int pingPong = 0; pingPong < 2; pingPong++)
	{
		// Write Temporal Image Write Descriptor Set

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_IMAGE, _deviceResources->rtShadowsTemporalImageWriteDescriptorSet[pingPong], 0)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->rtShadowsTemporalImage[pingPong], pvrvk::ImageLayout::e_GENERAL)));
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->rtShadowsTemporalImageWriteDescriptorSet[pingPong], 1)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->rtShadowsImage, samplerBilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->rtShadowsTemporalImageWriteDescriptorSet[pingPong], 2)
				.setImageInfo(
					0, pvrvk::DescriptorImageInfo(_deviceResources->rtShadowsTemporalImage[abs(pingPong - 1)], samplerBilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->rtShadowsTemporalImageWriteDescriptorSet[pingPong], 3)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->rtShadowsDownsampledAllMipsImageView, samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		// Write Temporal Image Read Descriptor Set

		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->rtShadowsTemporalImageReadDescriptorSet[pingPong], 0)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->rtShadowsTemporalImage[pingPong], samplerBilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		// Write Deferred Shading Descriptor Set
		for (int i = 0; i < FramebufferGBufferAttachments::Count; i++)
			writeDescSets.push_back(
				pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->deferredShadingDescriptorSet[pingPong], i)
					.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->gbufferImages[pingPong][i], samplerNearest, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->deferredShadingDescriptorSet[pingPong], FramebufferGBufferAttachments::Count)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->gbufferDepthStencilImage[pingPong], samplerNearest, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->deferredShadingDescriptorSet[pingPong], FramebufferGBufferAttachments::Count + 1)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->rtShadowsSpatialImage, samplerNearest, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		// Write Deferred Shading No Denoising Descriptor Set
		for (int i = 0; i < FramebufferGBufferAttachments::Count; i++)
			writeDescSets.push_back(
				pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->deferredShadingNoDenoisingDescriptorSet[pingPong], i)
					.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->gbufferImages[pingPong][i], samplerNearest, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(
				pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->deferredShadingNoDenoisingDescriptorSet[pingPong], FramebufferGBufferAttachments::Count)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->gbufferDepthStencilImage[pingPong], samplerNearest, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(
			pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->deferredShadingNoDenoisingDescriptorSet[pingPong], FramebufferGBufferAttachments::Count + 1)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->rtShadowsImage, samplerNearest, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
	}

	// Write Downsample Descriptor Set

	for (int i = 0; i < 4; i++)
	{
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_IMAGE, _deviceResources->shadowsDownsampleDescriptorSet, i)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->rtShadowsDownsampledMipImageViews[i], pvrvk::ImageLayout::e_GENERAL)));
	}

	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->shadowsDownsampleDescriptorSet, 4)
								.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->rtShadowsImage, samplerBilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

/// <summary>Creates the pipeline for the G-Buffer pass.</summary>
void VulkanRayTracingDenoising::createGBufferPipelines()
{
	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

	pipeLayoutInfo.setDescSetLayout(0, _deviceResources->commonDescriptorSetLayout);
	pipeLayoutInfo.addPushConstantRange(pvrvk::PushConstantRange(pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, sizeof(uint32_t)));
	pipeLayoutInfo.addPushConstantRange(pvrvk::PushConstantRange(pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, sizeof(uint32_t), sizeof(uint32_t) * 2));

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
	renderGBufferPipelineCreateInfo.colorBlend.setAttachmentState(4, renderGBufferColorAttachment);

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

	// renderpass
	renderGBufferPipelineCreateInfo.renderPass = _deviceResources->gbufferRenderPass;

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
	renderGBufferPipelineCreateInfo.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_GREATER_OR_EQUAL);
	renderGBufferPipelineCreateInfo.depthStencil.enableStencilTest(true);
	// set stencil states
	renderGBufferPipelineCreateInfo.depthStencil.setStencilFront(stencilState);
	renderGBufferPipelineCreateInfo.depthStencil.setStencilBack(stencilState);

	renderGBufferPipelineCreateInfo.pipelineLayout = _deviceResources->gbufferPipelineLayout;
	_deviceResources->gbufferPipeline = _deviceResources->device->createGraphicsPipeline(renderGBufferPipelineCreateInfo, _deviceResources->pipelineCache);
	_deviceResources->gbufferPipeline->setObjectName("GbufferGraphicsPipeline");
}

/// <summary>Creates the pipeline for the Deferred shading pass.</summary>
void VulkanRayTracingDenoising::createDeferredShadingPipelines()
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
	_deviceResources->defferedShadingPipeline->setObjectName("DefferedShadingGraphicsPipeline");
}

/// <summary>Creates the pipeline for the Shadows downsample pass.</summary>
void VulkanRayTracingDenoising::createShadowsDownsamplePipeline()
{
	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

	pipeLayoutInfo.setDescSetLayout(0, _deviceResources->shadowsDownsampleDescriptorSetLayout);

	_deviceResources->shadowsDownsamplePipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	pvrvk::ComputePipelineCreateInfo pipelineCreateInfo;

	pipelineCreateInfo.computeShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::ShadowsDownsampleComputeShader)->readToEnd<uint32_t>())));
	pipelineCreateInfo.pipelineLayout = _deviceResources->shadowsDownsamplePipelineLayout;

	_deviceResources->shadowsDownsamplePipeline = _deviceResources->device->createComputePipeline(pipelineCreateInfo, _deviceResources->pipelineCache);
	_deviceResources->shadowsDownsamplePipeline->setObjectName("ShadowsDownsampleComputePipeline");
}

/// <summary>Creates the pipeline for the Shadows temporal denoise pass.</summary>
void VulkanRayTracingDenoising::createShadowsTemporalPipeline()
{
	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

	pipeLayoutInfo.setDescSetLayout(0, _deviceResources->commonDescriptorSetLayout);
	pipeLayoutInfo.setDescSetLayout(1, _deviceResources->gbufferDescriptorSetLayout);
	pipeLayoutInfo.setDescSetLayout(2, _deviceResources->gbufferDescriptorSetLayout);
	pipeLayoutInfo.setDescSetLayout(3, _deviceResources->temporalWriteDescriptorSetLayout);

	_deviceResources->shadowsTemporalPipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	pvrvk::ComputePipelineCreateInfo pipelineCreateInfo;

	pipelineCreateInfo.computeShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::ShadowsTemporalDenoiseComputeShader)->readToEnd<uint32_t>())));
	pipelineCreateInfo.pipelineLayout = _deviceResources->shadowsTemporalPipelineLayout;

	_deviceResources->shadowsTemporalPipeline = _deviceResources->device->createComputePipeline(pipelineCreateInfo, _deviceResources->pipelineCache);
	_deviceResources->shadowsTemporalPipeline->setObjectName("ShadowsTemporalComputePipeline");
}

/// <summary>Creates the pipeline for the Shadows spatial denoise pass.</summary>
void VulkanRayTracingDenoising::createShadowsSpatialPipeline()
{
	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

	pipeLayoutInfo.setDescSetLayout(0, _deviceResources->commonDescriptorSetLayout);
	pipeLayoutInfo.setDescSetLayout(1, _deviceResources->gbufferDescriptorSetLayout);
	pipeLayoutInfo.setDescSetLayout(2, _deviceResources->combinedSamplerDescriptorSetLayout);
	pipeLayoutInfo.setDescSetLayout(3, _deviceResources->storageImageDescriptorSetLayout);

	_deviceResources->shadowsSpatialPipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	pvrvk::ComputePipelineCreateInfo pipelineCreateInfo;

	pipelineCreateInfo.computeShader.setShader(
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::ShadowsSpatialDenoiseComputeShader)->readToEnd<uint32_t>())));
	pipelineCreateInfo.pipelineLayout = _deviceResources->shadowsSpatialPipelineLayout;

	_deviceResources->shadowsSpatialPipeline = _deviceResources->device->createComputePipeline(pipelineCreateInfo, _deviceResources->pipelineCache);
	_deviceResources->shadowsSpatialPipeline->setObjectName("ShadowsSpatialComputePipeline");
}

/// <summary>Create the pipelines for this example.</summary>
void VulkanRayTracingDenoising::createPipelines()
{
	createGBufferPipelines();
	createDeferredShadingPipelines();
	createShadowsDownsamplePipeline();
	createShadowsTemporalPipeline();
	createShadowsSpatialPipeline();
}

/// <summary>Creates all the Images and Image Views used in this example.</summary>
void VulkanRayTracingDenoising::createImages()
{
	const pvrvk::Extent3D& dimension = pvrvk::Extent3D(_deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight(), 1u);
	const pvrvk::Extent3D& dimensionHalf = pvrvk::Extent3D(_deviceResources->swapchain->getDimension().getWidth() / 2, _deviceResources->swapchain->getDimension().getHeight() / 2, 1u);

	const pvrvk::Format renderpassStorageFormats[FramebufferGBufferAttachments::Count] = { pvrvk::Format::e_R8G8B8A8_UNORM, pvrvk::Format::e_R16G16B16A16_SFLOAT,
		pvrvk::Format::e_R16G16B16A16_SFLOAT, pvrvk::Format::e_R8G8B8A8_UNORM };

	// Create images
	for (int pingPong = 0; pingPong < 2; pingPong++)
	{
		for (int i = 0; i < FramebufferGBufferAttachments::Count; i++)
		{
			pvrvk::Image image = pvr::utils::createImage(_deviceResources->device,
				pvrvk::ImageCreateInfo(
					pvrvk::ImageType::e_2D, renderpassStorageFormats[i], dimension, pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT),
				pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _deviceResources->vmaAllocator,
				pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);
			image->setObjectName("gbufferImage[" + std::to_string(pingPong) + "][" + std::to_string(i) + "]");

			_deviceResources->gbufferImages[pingPong][i] = _deviceResources->device->createImageView(
				pvrvk::ImageViewCreateInfo(image, pvrvk::ImageViewType::e_2D, image->getFormat(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT)));
		}

		pvrvk::Image image = pvr::utils::createImage(_deviceResources->device,
			pvrvk::ImageCreateInfo(
				pvrvk::ImageType::e_2D, pvrvk::Format::e_D32_SFLOAT, dimension, pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _deviceResources->vmaAllocator,
			pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);
		image->setObjectName("gbufferDepthImage");

		_deviceResources->gbufferDepthStencilImage[pingPong] = _deviceResources->device->createImageView(
			pvrvk::ImageViewCreateInfo(image, pvrvk::ImageViewType::e_2D, image->getFormat(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_DEPTH_BIT)));
	}

	// RT Shadows
	pvrvk::Image rtShadowsImage = pvr::utils::createImage(_deviceResources->device,
		pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, pvrvk::Format::e_R8_SNORM, dimension, pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _deviceResources->vmaAllocator,
		pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);
	rtShadowsImage->setObjectName("rtShadowsImage");

	_deviceResources->rtShadowsImage = _deviceResources->device->createImageView(
		pvrvk::ImageViewCreateInfo(rtShadowsImage, pvrvk::ImageViewType::e_2D, rtShadowsImage->getFormat(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT)));

	// RT Shadows Downsample
	pvrvk::Image rtShadowsDownsampleImage = pvr::utils::createImage(_deviceResources->device,
		pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, pvrvk::Format::e_R16_SFLOAT, dimensionHalf, pvrvk::ImageUsageFlags::e_STORAGE_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT, 4),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _deviceResources->vmaAllocator,
		pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);
	rtShadowsDownsampleImage->setObjectName("rtShadowsDownsampleImage");

	_deviceResources->rtShadowsDownsampledAllMipsImageView = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(
		rtShadowsDownsampleImage, pvrvk::ImageViewType::e_2D, rtShadowsDownsampleImage->getFormat(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0, 4)));

	for (int i = 0; i < 4; i++)
	{
		_deviceResources->rtShadowsDownsampledMipImageViews[i] = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(
			rtShadowsDownsampleImage, pvrvk::ImageViewType::e_2D, rtShadowsDownsampleImage->getFormat(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT, i)));
	}

	// RT Shadows Temporal
	for (int i = 0; i < 2; i++)
	{
		pvrvk::Image rtShadowsTemporalImage = pvr::utils::createImage(_deviceResources->device,
			pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, pvrvk::Format::e_R16_SFLOAT, dimension, pvrvk::ImageUsageFlags::e_STORAGE_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _deviceResources->vmaAllocator,
			pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);
		rtShadowsTemporalImage->setObjectName("rtShadowsTemporalImage[" + std::to_string(i) + "]");

		_deviceResources->rtShadowsTemporalImage[i] = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(
			rtShadowsTemporalImage, pvrvk::ImageViewType::e_2D, rtShadowsTemporalImage->getFormat(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT)));
	}

	// RT Shadows Spatial
	pvrvk::Image rtShadowsSpatialImage = pvr::utils::createImage(_deviceResources->device,
		pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, pvrvk::Format::e_R16_SFLOAT, dimension, pvrvk::ImageUsageFlags::e_STORAGE_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _deviceResources->vmaAllocator,
		pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);
	rtShadowsSpatialImage->setObjectName("rtShadowsSpatialImage");

	_deviceResources->rtShadowsSpatialImage = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(
		rtShadowsSpatialImage, pvrvk::ImageViewType::e_2D, rtShadowsSpatialImage->getFormat(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT)));
}

/// <summary>Create the G-Buffer pass framebuffer and renderpass.</summary>
void VulkanRayTracingDenoising::createFramebufferAndRenderPass()
{
	const pvrvk::Extent3D& dimension = pvrvk::Extent3D(_deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight(), 1u);

	const pvrvk::Format renderpassStorageFormats[FramebufferGBufferAttachments::Count] = { pvrvk::Format::e_R8G8B8A8_UNORM, pvrvk::Format::e_R16G16B16A16_SFLOAT,
		pvrvk::Format::e_R16G16B16A16_SFLOAT, pvrvk::Format::e_R8G8B8A8_UNORM };

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
	pvrvk::AttachmentDescription gbufferAttachment4 = pvrvk::AttachmentDescription::createColorDescription(_deviceResources->rtShadowsImage->getFormat(), pvrvk::ImageLayout::e_UNDEFINED,
		pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT);
	pvrvk::AttachmentDescription gbufferAttachmentDepth = pvrvk::AttachmentDescription::createDepthStencilDescription(pvrvk::Format::e_D32_SFLOAT, pvrvk::ImageLayout::e_UNDEFINED,
		pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE);

	pvrvk::AttachmentReference gbufferAttachmentRef0 = pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
	pvrvk::AttachmentReference gbufferAttachmentRef1 = pvrvk::AttachmentReference(1, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
	pvrvk::AttachmentReference gbufferAttachmentRef2 = pvrvk::AttachmentReference(2, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
	pvrvk::AttachmentReference gbufferAttachmentRef3 = pvrvk::AttachmentReference(3, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
	pvrvk::AttachmentReference gbufferAttachmentRef4 = pvrvk::AttachmentReference(4, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
	pvrvk::AttachmentReference gbufferAttachmentRefDepth = pvrvk::AttachmentReference(5, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	pvrvk::SubpassDescription subpassDesc = pvrvk::SubpassDescription()
												.setColorAttachmentReference(0, gbufferAttachmentRef0)
												.setColorAttachmentReference(1, gbufferAttachmentRef1)
												.setColorAttachmentReference(2, gbufferAttachmentRef2)
												.setColorAttachmentReference(3, gbufferAttachmentRef3)
												.setColorAttachmentReference(4, gbufferAttachmentRef4)
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
	dependency[1].setDstStageMask(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT | pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT);
	dependency[1].setSrcAccessMask(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT);
	dependency[1].setDstAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
	dependency[1].setDependencyFlags(pvrvk::DependencyFlags::e_BY_REGION_BIT);

	pvrvk::RenderPassCreateInfo renderPassCreateInfo = pvrvk::RenderPassCreateInfo()
														   .setAttachmentDescription(0, gbufferAttachment0)
														   .setAttachmentDescription(1, gbufferAttachment1)
														   .setAttachmentDescription(2, gbufferAttachment2)
														   .setAttachmentDescription(3, gbufferAttachment3)
														   .setAttachmentDescription(4, gbufferAttachment4)
														   .setAttachmentDescription(5, gbufferAttachmentDepth)
														   .setSubpass(0, subpassDesc)
														   .addSubpassDependencies(dependency, 2);

	_deviceResources->gbufferRenderPass = _deviceResources->device->createRenderPass(renderPassCreateInfo);
	_deviceResources->gbufferRenderPass->setObjectName("GBufferRenderPass");

	for (int pingPong = 0; pingPong < 2; pingPong++)
	{
		pvrvk::ImageView imageViews[] = { _deviceResources->gbufferImages[pingPong][0], _deviceResources->gbufferImages[pingPong][1], _deviceResources->gbufferImages[pingPong][2],
			_deviceResources->gbufferImages[pingPong][3], _deviceResources->rtShadowsImage, _deviceResources->gbufferDepthStencilImage[pingPong] };

		_deviceResources->gbufferFramebuffer[pingPong] = _deviceResources->device->createFramebuffer(
			pvrvk::FramebufferCreateInfo(dimension.getWidth(), dimension.getHeight(), 1, _deviceResources->gbufferRenderPass, 6, &imageViews[0]));
	}
}

/// <summary>Loads the mesh data required for this example into vertex and index buffer objects and populates material data.</summary>
/// <param name="uploadCmd">Command Buffer used to record the buffer upload commands.</param>
void VulkanRayTracingDenoising::createModelBuffers(pvrvk::CommandBuffer& uploadCmd)
{
	// A structure to represent a material used in this example
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
	_deviceResources->materialIndexBuffers.reserve(numMeshes);
	_deviceResources->verticesSize.reserve(numMeshes);
	_deviceResources->indicesSize.reserve(numMeshes);
	_meshTransforms.reserve(numMeshes);
	_prevMeshTransforms.reserve(numMeshes);

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
		_prevMeshTransforms.push_back(modelMat);

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
		_deviceResources->indexBuffers.back()->setObjectName("VBO");

		pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, _deviceResources->indexBuffers[meshIdx], uploadCmd, indices.data(), 0, sizeof(uint32_t) * indices.size());

		// create material index buffer
		pvrvk::BufferCreateInfo materialIndexBufferInfo;
		materialIndexBufferInfo.setSize(sizeof(uint32_t) * materialIndices.size());
		materialIndexBufferInfo.setUsageFlags(pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT);

		_deviceResources->materialIndexBuffers.push_back(pvr::utils::createBuffer(_deviceResources->device, materialIndexBufferInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT));
		_deviceResources->materialIndexBuffers.back()->setObjectName("MaterialIndexSBO");

		pvr::utils::updateBufferUsingStagingBuffer(
			_deviceResources->device, _deviceResources->materialIndexBuffers[meshIdx], uploadCmd, materialIndices.data(), 0, sizeof(uint32_t) * materialIndices.size());

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
void VulkanRayTracingDenoising::createCameraBuffer()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(BufferEntryNames::PerScene::ViewMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::PerScene::ProjectionMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::PerScene::PrevViewProjMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::PerScene::ViewProjInverseMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::PerScene::PrevViewProjInverseMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::PerScene::AmbientLightColor, pvr::GpuDatatypes::vec4);
	desc.addElement(BufferEntryNames::PerScene::CameraPosition, pvr::GpuDatatypes::vec4);

	_deviceResources->globalBufferView.initDynamic(desc, _deviceResources->swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
		static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

	_deviceResources->globalBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->globalBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->globalBuffer->setObjectName("GlobalUBO");

	_deviceResources->globalBufferView.pointToMappedMemory(_deviceResources->globalBuffer->getDeviceMemory()->getMappedData());
}

/// <summary>Creates the scene wide buffer used throughout the demo.</summary>
void VulkanRayTracingDenoising::createMeshTransformBuffer()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(BufferEntryNames::PerMesh::WorldMatrix, pvr::GpuDatatypes::mat4x4, static_cast<uint32_t>(_meshTransforms.size()));

	_deviceResources->perMeshBufferView.initDynamic(desc, _deviceResources->swapchain->getSwapchainLength() * static_cast<uint32_t>(_meshTransforms.size()), pvr::BufferUsageFlags::UniformBuffer,
		static_cast<uint64_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
	_deviceResources->perMeshPrevTransformBufferView.initDynamic(desc, _deviceResources->swapchain->getSwapchainLength() * static_cast<uint32_t>(_prevMeshTransforms.size()),
		pvr::BufferUsageFlags::UniformBuffer, static_cast<uint64_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

	_deviceResources->perMeshBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->perMeshBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->perMeshBuffer->setObjectName("PerMeshUBO");
	_deviceResources->perMeshPrevTransformBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->perMeshPrevTransformBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->perMeshPrevTransformBuffer->setObjectName("PerMeshPrevTransformUBO");

	_deviceResources->perMeshBufferView.pointToMappedMemory(_deviceResources->perMeshBuffer->getDeviceMemory()->getMappedData());
	_deviceResources->perMeshPrevTransformBufferView.pointToMappedMemory(_deviceResources->perMeshPrevTransformBuffer->getDeviceMemory()->getMappedData());
}

/// <summary>Initializes the light sources in the Scene.</summary>
void VulkanRayTracingDenoising::initializeLights()
{
	_averageLightColor = glm::vec4(glm::vec3(0.0f), 1.0f);

	glm::vec4 lightPosition;
	_scene->getLightPosition(0, lightPosition);
	pvr::assets::Light light = _scene->getLight(0);

	_lightData.lightColor = glm::vec4(light.getColor(), 1.0f);
	_lightData.lightPosition = lightPosition;
	_lightData.lightIntensity = 1.5f;
	_lightData.isProcedural = false;

	_averageLightColor += glm::vec4(light.getColor(), 0.0f) * _lightData.lightIntensity;

	// calculate an average ambient light color
	_averageLightColor *= LightConfiguration::AmbientColorScaler;
}

/// <summary>Creates the Light data buffer.</summary>
void VulkanRayTracingDenoising::createLightBuffer()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(BufferEntryNames::PerPointLightData::LightPosition, pvr::GpuDatatypes::vec4);
	desc.addElement(BufferEntryNames::PerPointLightData::LightColor, pvr::GpuDatatypes::vec4);
	desc.addElement(BufferEntryNames::PerPointLightData::LightIntensity, pvr::GpuDatatypes::Float);
	desc.addElement(BufferEntryNames::PerPointLightData::LightRadius, pvr::GpuDatatypes::Float);

	_deviceResources->lightDataBufferView.initDynamic(desc, _deviceResources->swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
		static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

	_deviceResources->lightDataBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->lightDataBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->lightDataBuffer->setObjectName("LightDataUBO");

	_deviceResources->lightDataBufferView.pointToMappedMemory(_deviceResources->lightDataBuffer->getDeviceMemory()->getMappedData());
}

/// <summary>Upload the dynamic scene data to the buffers.</summary>
void VulkanRayTracingDenoising::uploadDynamicSceneData()
{
	// static scene properties buffer
	{
		_farClipDistance = _scene->getCamera(static_cast<uint32_t>(SceneNodes::Cameras::SceneCamera)).getFar();

		uint32_t cameraDynamicSliceIdx = _deviceResources->swapchain->getSwapchainIndex();
		_deviceResources->globalBufferView.getElementByName(BufferEntryNames::PerScene::ViewMatrix, 0, cameraDynamicSliceIdx).setValue(_viewMatrix);
		_deviceResources->globalBufferView.getElementByName(BufferEntryNames::PerScene::ProjectionMatrix, 0, cameraDynamicSliceIdx).setValue(_projectionMatrix);
		_deviceResources->globalBufferView.getElementByName(BufferEntryNames::PerScene::PrevViewProjMatrix, 0, cameraDynamicSliceIdx).setValue(_prevViewProjectionMatrix);
		_deviceResources->globalBufferView.getElementByName(BufferEntryNames::PerScene::ViewProjInverseMatrix, 0, cameraDynamicSliceIdx).setValue(_inverseViewProjectionMatrix);
		_deviceResources->globalBufferView.getElementByName(BufferEntryNames::PerScene::PrevViewProjInverseMatrix, 0, cameraDynamicSliceIdx).setValue(_inversePrevViewProjectionMatrix);
		_deviceResources->globalBufferView.getElementByName(BufferEntryNames::PerScene::AmbientLightColor, 0, cameraDynamicSliceIdx).setValue(_averageLightColor);
		_deviceResources->globalBufferView.getElementByName(BufferEntryNames::PerScene::CameraPosition, 0, cameraDynamicSliceIdx).setValue(glm::vec4(_cameraPosition, 0.0f));

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(_deviceResources->globalBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_deviceResources->globalBuffer->getDeviceMemory()->flushRange(
				_deviceResources->globalBufferView.getDynamicSliceOffset(cameraDynamicSliceIdx), _deviceResources->globalBufferView.getDynamicSliceSize());
		}
	}

	// upload light data
	{
		uint32_t lightDynamicSliceIdx = _deviceResources->swapchain->getSwapchainIndex();

		_deviceResources->lightDataBufferView.getElementByName(BufferEntryNames::PerPointLightData::LightPosition, 0, lightDynamicSliceIdx).setValue(_lightData.lightPosition);
		_deviceResources->lightDataBufferView.getElementByName(BufferEntryNames::PerPointLightData::LightColor, 0, lightDynamicSliceIdx).setValue(_lightData.lightColor);
		_deviceResources->lightDataBufferView.getElementByName(BufferEntryNames::PerPointLightData::LightIntensity, 0, lightDynamicSliceIdx).setValue(_lightData.lightIntensity);
		_deviceResources->lightDataBufferView.getElementByName(BufferEntryNames::PerPointLightData::LightRadius, 0, lightDynamicSliceIdx)
			.setValue(_animateLightRadius ? (cosf(static_cast<float>(getTime()) * 0.001f) * 0.5f + 0.5f) * ApplicationConfiguration::MaxAnimatedLightRadius : _lightData.lightRadius);

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(_deviceResources->lightDataBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_deviceResources->lightDataBuffer->getDeviceMemory()->flushRange(
				_deviceResources->lightDataBufferView.getDynamicSliceOffset(lightDynamicSliceIdx), _deviceResources->lightDataBufferView.getDynamicSliceSize());
		}
	}

	// upload per mesh data
	{
		for (uint32_t i = 0; i < _meshTransforms.size(); i++)
		{
			_deviceResources->perMeshBufferView.getElementByName(BufferEntryNames::PerMesh::WorldMatrix, i, _deviceResources->swapchain->getSwapchainIndex()).setValue(_meshTransforms[i]);
		}

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(_deviceResources->perMeshBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_deviceResources->perMeshBuffer->getDeviceMemory()->flushRange(_deviceResources->perMeshBufferView.getDynamicSliceOffset(_deviceResources->swapchain->getSwapchainIndex()),
				_deviceResources->perMeshBufferView.getDynamicSliceSize());
		}
	}

	// upload prev per mesh data
	{
		for (uint32_t i = 0; i < _prevMeshTransforms.size(); i++)
		{
			_deviceResources->perMeshPrevTransformBufferView.getElementByName(BufferEntryNames::PerMesh::WorldMatrix, i, _deviceResources->swapchain->getSwapchainIndex())
				.setValue(_prevMeshTransforms[i]);
		}

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(_deviceResources->perMeshPrevTransformBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_deviceResources->perMeshPrevTransformBuffer->getDeviceMemory()->flushRange(
				_deviceResources->perMeshPrevTransformBufferView.getDynamicSliceOffset(_deviceResources->swapchain->getSwapchainIndex()),
				_deviceResources->perMeshPrevTransformBufferView.getDynamicSliceSize());
		}
	}
}

/// <summary>Updates animation variables and camera matrices.</summary>
void VulkanRayTracingDenoising::updateAnimation()
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
	glm::mat4 preViewProj = _viewProjectionMatrix;
	_viewProjectionMatrix = _projectionMatrix * _viewMatrix;
	_prevViewProjectionMatrix = _frameIdx == 0 ? _viewProjectionMatrix : preViewProj;
	_inverseViewProjectionMatrix = glm::inverse(_viewProjectionMatrix);
	_inversePrevViewProjectionMatrix = glm::inverse(_prevViewProjectionMatrix);
}

/// <summary>Records main command buffer.</summary>
void VulkanRayTracingDenoising::recordMainCommandBuffer(uint32_t swapchainIndex)
{
	_deviceResources->cmdBufferMainDeferred[swapchainIndex]->begin();

	pvr::utils::beginCommandBufferDebugLabel(
		_deviceResources->cmdBufferMainDeferred[swapchainIndex], pvrvk::DebugUtilsLabel("MainDeferredRenderPassSwapchain" + std::to_string(swapchainIndex)));

	pvrvk::Rect2D renderArea(0, 0, _windowWidth, _windowHeight);

	// Specify a clear colour per attachment
	const uint32_t numClearValues = FramebufferGBufferAttachments::Count + 2u;

	pvrvk::ClearValue gbufferClearValues[numClearValues] = { pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 0.0f), pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 0.0f),
		pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 0.0f), pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 0.0f), pvrvk::ClearValue(1.0f, 1.0f, 1.0f, 1.0f), pvrvk::ClearValue(0.0f, 0u) };

	uint32_t currentFrameIdx = static_cast<uint32_t>(_pingPong);

	// Render G-Buffer
	_deviceResources->cmdBufferMainDeferred[swapchainIndex]->beginRenderPass(_deviceResources->gbufferFramebuffer[currentFrameIdx], renderArea, false, gbufferClearValues, numClearValues);

	_deviceResources->cmdBufferMainDeferred[swapchainIndex]->executeCommands(_deviceResources->cmdBufferGBuffer[swapchainIndex]);

	_deviceResources->cmdBufferMainDeferred[swapchainIndex]->endRenderPass();

	if (_denoise)
	{
		// Shadows downsample
		_deviceResources->cmdBufferMainDeferred[swapchainIndex]->executeCommands(_deviceResources->cmdBufferShadowsDownsample[swapchainIndex]);

		// Shadows temporal accumulation
		_deviceResources->cmdBufferMainDeferred[swapchainIndex]->executeCommands(_deviceResources->cmdBufferShadowsTemporal[swapchainIndex]);

		// Shadows spatial accumulation
		_deviceResources->cmdBufferMainDeferred[swapchainIndex]->executeCommands(_deviceResources->cmdBufferShadowsSpatial[swapchainIndex]);
	}

	pvrvk::ClearValue onscreenClearValues[] = { pvrvk::ClearValue(0.0, 0.0, 0.0, 1.0f), pvrvk::ClearValue(1.f, 0u) };

	// Deferred shading + UI
	_deviceResources->cmdBufferMainDeferred[swapchainIndex]->beginRenderPass(_deviceResources->onScreenFramebuffer[swapchainIndex], renderArea, false, onscreenClearValues, 2);

	_deviceResources->cmdBufferMainDeferred[swapchainIndex]->executeCommands(_deviceResources->cmdBufferDeferredShading[swapchainIndex]);

	_deviceResources->cmdBufferMainDeferred[swapchainIndex]->endRenderPass();

	pvr::utils::endCommandBufferDebugLabel(_deviceResources->cmdBufferMainDeferred[swapchainIndex]);

	_deviceResources->cmdBufferMainDeferred[swapchainIndex]->end();
}

/// <summary>Record all the secondary command buffers.</summary>
void VulkanRayTracingDenoising::recordSecondaryCommandBuffers(uint32_t swapchainIndex)
{
	pvrvk::Rect2D renderArea(0, 0, _framebufferWidth, _framebufferHeight);
	if ((_framebufferWidth != _windowWidth) || (_framebufferHeight != _windowHeight))
	{
		renderArea = pvrvk::Rect2D(_viewportOffsets[0], _viewportOffsets[1], _framebufferWidth, _framebufferHeight);
	}

	pvrvk::ClearValue clearStenciLValue(pvrvk::ClearValue::createStencilClearValue(0));

	uint32_t currentFrameIdx = static_cast<uint32_t>(_pingPong);

	_deviceResources->cmdBufferGBuffer[swapchainIndex]->begin(_deviceResources->gbufferFramebuffer[currentFrameIdx]);
	recordCommandBufferRenderGBuffer(_deviceResources->cmdBufferGBuffer[swapchainIndex], swapchainIndex);
	_deviceResources->cmdBufferGBuffer[swapchainIndex]->end();

	_deviceResources->cmdBufferDeferredShading[swapchainIndex]->begin(_deviceResources->onScreenFramebuffer[swapchainIndex]);
	recordCommandBufferDeferredShading(_deviceResources->cmdBufferDeferredShading[swapchainIndex], swapchainIndex);
	recordCommandUIRenderer(_deviceResources->cmdBufferDeferredShading[swapchainIndex]);
	_deviceResources->cmdBufferDeferredShading[swapchainIndex]->end();

	if (_denoise)
	{
		_deviceResources->cmdBufferShadowsDownsample[swapchainIndex]->begin();
		recordCommandBufferShadowsDownsample(_deviceResources->cmdBufferShadowsDownsample[swapchainIndex], swapchainIndex);
		_deviceResources->cmdBufferShadowsDownsample[swapchainIndex]->end();

		_deviceResources->cmdBufferShadowsTemporal[swapchainIndex]->begin();
		recordCommandBufferShadowsTemporal(_deviceResources->cmdBufferShadowsTemporal[swapchainIndex], swapchainIndex);
		_deviceResources->cmdBufferShadowsTemporal[swapchainIndex]->end();

		_deviceResources->cmdBufferShadowsSpatial[swapchainIndex]->begin();
		recordCommandBufferShadowsSpatial(_deviceResources->cmdBufferShadowsSpatial[swapchainIndex], swapchainIndex);
		_deviceResources->cmdBufferShadowsSpatial[swapchainIndex]->end();
	}
}

/// <summary>Record rendering G-Buffer commands.</summary>
/// <param name="cmdBuffers">SecondaryCommandbuffer to record.</param>
/// <param name="swapchainIndex">Index of the current swapchain image.</param>
void VulkanRayTracingDenoising::recordCommandBufferRenderGBuffer(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex)
{
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffers, pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("G-Buffer - Swapchain (%i)", swapchainIndex)));

	uint32_t offsets[4] = {};
	offsets[0] = _deviceResources->globalBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[1] = _deviceResources->lightDataBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[2] = _deviceResources->perMeshBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[3] = _deviceResources->perMeshPrevTransformBufferView.getDynamicSliceOffset(swapchainIndex);

	cmdBuffers->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->gbufferPipelineLayout, 0u, _deviceResources->commonDescriptorSet, offsets, 4);

	for (uint32_t meshIdx = 0; meshIdx < _deviceResources->meshes.size(); meshIdx++)
	{
		auto& mesh = _deviceResources->meshes[meshIdx];

		cmdBuffers->bindPipeline(_deviceResources->gbufferPipeline);

		cmdBuffers->pushConstants(_deviceResources->gbufferPipeline->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, sizeof(uint32_t), &meshIdx);

		uint32_t pushConstants[2] = { static_cast<uint32_t>(mesh.materialIdx), _frameIdx };
		cmdBuffers->pushConstants(_deviceResources->gbufferPipeline->getPipelineLayout(), pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, sizeof(uint32_t), sizeof(uint32_t) * 2, pushConstants);

		cmdBuffers->bindVertexBuffer(_deviceResources->vertexBuffers[meshIdx], 0, 0);
		cmdBuffers->bindIndexBuffer(_deviceResources->indexBuffers[meshIdx], 0, mesh.indexType);
		cmdBuffers->drawIndexed(mesh.indexOffset, mesh.numIndices, 0, 0, 1);
	}

	pvr::utils::endCommandBufferDebugLabel(cmdBuffers);
}

/// <summary>Record compute commands for downsampling the Shadow mask.</summary>
/// <param name="cmdBuffers">SecondaryCommandbuffer to record.</param>
/// <param name="swapchainIndex">Index of the current swapchain image.</param>
void VulkanRayTracingDenoising::recordCommandBufferShadowsDownsample(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex)
{
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffers, pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Shadows Downsample - Swapchain (%i)", swapchainIndex)));

	{
		pvrvk::MemoryBarrierSet layoutTransitions;
		layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::AccessFlags::e_SHADER_WRITE_BIT,
			_deviceResources->rtShadowsDownsampledMipImageViews[0]->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0, 4),
			pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_GENERAL, _deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));

		if (_frameIdx == 0)
		{
			uint32_t historyFrameIdx = static_cast<uint32_t>(!_pingPong);

			for (int i = 0; i < FramebufferGBufferAttachments::Count; i++)
				layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT,
					_deviceResources->gbufferImages[historyFrameIdx][i]->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_UNDEFINED,
					pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, _deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));

			layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT,
				_deviceResources->gbufferDepthStencilImage[historyFrameIdx]->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_DEPTH_BIT),
				pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, _deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));
		}

		cmdBuffers->pipelineBarrier(pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, layoutTransitions);
	}

	cmdBuffers->bindPipeline(_deviceResources->shadowsDownsamplePipeline);

	pvrvk::DescriptorSet arrayDS[] = { _deviceResources->shadowsDownsampleDescriptorSet };

	cmdBuffers->bindDescriptorSets(pvrvk::PipelineBindPoint::e_COMPUTE, _deviceResources->shadowsDownsamplePipelineLayout, 0, arrayDS, 1);

	const uint32_t WORK_GROUP_SIZE_X = 8;
	const uint32_t WORK_GROUP_SIZE_Y = 8;

	const uint32_t NUM_WORK_GROUPS_X = static_cast<uint32_t>(ceilf(static_cast<float>(getWidth() / 2) / static_cast<float>(WORK_GROUP_SIZE_X)));
	const uint32_t NUM_WORK_GROUPS_Y = static_cast<uint32_t>(ceilf(static_cast<float>(getHeight() / 2) / static_cast<float>(WORK_GROUP_SIZE_Y)));

	cmdBuffers->dispatch(NUM_WORK_GROUPS_X, NUM_WORK_GROUPS_Y, 1);

	{
		pvrvk::MemoryBarrierSet layoutTransitions;
		layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT,
			_deviceResources->rtShadowsDownsampledMipImageViews[0]->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0, 4),
			pvrvk::ImageLayout::e_GENERAL, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, _deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));

		cmdBuffers->pipelineBarrier(pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, layoutTransitions);
	}

	pvr::utils::endCommandBufferDebugLabel(cmdBuffers);
}

/// <summary>Record compute commands for temporally denoising the Shadow mask.</summary>
/// <param name="cmdBuffers">SecondaryCommandbuffer to record.</param>
/// <param name="swapchainIndex">Index of the current swapchain image.</param>
void VulkanRayTracingDenoising::recordCommandBufferShadowsTemporal(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex)
{
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffers, pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Shadows Temporal Accumulation - Swapchain (%i)", swapchainIndex)));

	uint32_t gbufferCurrentIdx = static_cast<uint32_t>(_pingPong);
	uint32_t gbufferHistoryIdx = static_cast<uint32_t>(!_pingPong);
	uint32_t temporalWriteIdx = static_cast<uint32_t>(_pingPong);
	uint32_t temporalReadIdx = static_cast<uint32_t>(!_pingPong);

	{
		pvrvk::MemoryBarrierSet layoutTransitions;
		layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::AccessFlags::e_SHADER_WRITE_BIT,
			_deviceResources->rtShadowsTemporalImage[temporalWriteIdx]->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT),
			pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_GENERAL, _deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));

		if (_frameIdx == 0)
		{
			layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT,
				_deviceResources->rtShadowsTemporalImage[temporalReadIdx]->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT),
				pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, _deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));
		}

		cmdBuffers->pipelineBarrier(pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, layoutTransitions);
	}

	cmdBuffers->bindPipeline(_deviceResources->shadowsTemporalPipeline);

	pvrvk::DescriptorSet arrayDS[] = { _deviceResources->commonDescriptorSet, _deviceResources->gbufferDescriptorSet[gbufferCurrentIdx],
		_deviceResources->gbufferDescriptorSet[gbufferHistoryIdx], _deviceResources->rtShadowsTemporalImageWriteDescriptorSet[temporalWriteIdx] };

	uint32_t offsets[4] = {};
	offsets[0] = _deviceResources->globalBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[1] = _deviceResources->lightDataBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[2] = _deviceResources->perMeshBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[3] = _deviceResources->perMeshPrevTransformBufferView.getDynamicSliceOffset(swapchainIndex);

	cmdBuffers->bindDescriptorSets(pvrvk::PipelineBindPoint::e_COMPUTE, _deviceResources->shadowsTemporalPipelineLayout, 0, arrayDS, 4, offsets, 4);

	const uint32_t WORK_GROUP_SIZE_X = 8;
	const uint32_t WORK_GROUP_SIZE_Y = 8;

	const uint32_t NUM_WORK_GROUPS_X = static_cast<uint32_t>(ceilf(static_cast<float>(getWidth()) / static_cast<float>(WORK_GROUP_SIZE_X)));
	const uint32_t NUM_WORK_GROUPS_Y = static_cast<uint32_t>(ceilf(static_cast<float>(getHeight()) / static_cast<float>(WORK_GROUP_SIZE_Y)));

	cmdBuffers->dispatch(NUM_WORK_GROUPS_X, NUM_WORK_GROUPS_Y, 1);

	{
		pvrvk::MemoryBarrierSet layoutTransitions;
		layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT,
			_deviceResources->rtShadowsTemporalImage[temporalWriteIdx]->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT),
			pvrvk::ImageLayout::e_GENERAL, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, _deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));

		cmdBuffers->pipelineBarrier(pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, layoutTransitions);
	}

	pvr::utils::endCommandBufferDebugLabel(cmdBuffers);
}

/// <summary>Record compute commands for spatially denoising the Shadow mask using an 8-tap poisson disc blur.</summary>
/// <param name="cmdBuffers">SecondaryCommandbuffer to record.</param>
/// <param name="swapchainIndex">Index of the current swapchain image.</param>
void VulkanRayTracingDenoising::recordCommandBufferShadowsSpatial(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex)
{
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffers, pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Shadows Spatial Denoise - Swapchain (%i)", swapchainIndex)));

	uint32_t gbufferCurrentIdx = static_cast<uint32_t>(_pingPong);
	uint32_t temporalReadIdx = static_cast<uint32_t>(_pingPong);

	{
		pvrvk::MemoryBarrierSet layoutTransitions;
		layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::AccessFlags::e_SHADER_WRITE_BIT,
			_deviceResources->rtShadowsSpatialImage->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_GENERAL, _deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));

		cmdBuffers->pipelineBarrier(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, layoutTransitions);
	}

	cmdBuffers->bindPipeline(_deviceResources->shadowsSpatialPipeline);

	pvrvk::DescriptorSet arrayDS[] = { _deviceResources->commonDescriptorSet, _deviceResources->gbufferDescriptorSet[gbufferCurrentIdx],
		_deviceResources->rtShadowsTemporalImageReadDescriptorSet[temporalReadIdx], _deviceResources->rtShadowsSpatialImageWriteDescriptorSet };

	uint32_t offsets[4] = {};
	offsets[0] = _deviceResources->globalBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[1] = _deviceResources->lightDataBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[2] = _deviceResources->perMeshBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[3] = _deviceResources->perMeshPrevTransformBufferView.getDynamicSliceOffset(swapchainIndex);

	cmdBuffers->bindDescriptorSets(pvrvk::PipelineBindPoint::e_COMPUTE, _deviceResources->shadowsSpatialPipelineLayout, 0, arrayDS, 4, offsets, 4);

	const uint32_t WORK_GROUP_SIZE_X = 8;
	const uint32_t WORK_GROUP_SIZE_Y = 8;

	const uint32_t NUM_WORK_GROUPS_X = static_cast<uint32_t>(ceilf(static_cast<float>(getWidth()) / static_cast<float>(WORK_GROUP_SIZE_X)));
	const uint32_t NUM_WORK_GROUPS_Y = static_cast<uint32_t>(ceilf(static_cast<float>(getHeight()) / static_cast<float>(WORK_GROUP_SIZE_Y)));

	cmdBuffers->dispatch(NUM_WORK_GROUPS_X, NUM_WORK_GROUPS_Y, 1);

	{
		pvrvk::MemoryBarrierSet layoutTransitions;
		layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT,
			_deviceResources->rtShadowsSpatialImage->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_GENERAL,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, _deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));

		cmdBuffers->pipelineBarrier(pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, layoutTransitions);
	}

	pvr::utils::endCommandBufferDebugLabel(cmdBuffers);
}

/// <summary>Record deferred shading commands.</summary>
/// <param name="cmdBuffers">SecondaryCommandbuffer to record.</param>
/// <param name="swapchainIndex">Index of the current swapchain image.</param>
void VulkanRayTracingDenoising::recordCommandBufferDeferredShading(pvrvk::SecondaryCommandBuffer& cmdBuffers, uint32_t swapchainIndex)
{
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffers, pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Deferred Shading - Swapchain (%i)", swapchainIndex)));

	cmdBuffers->bindPipeline(_deviceResources->defferedShadingPipeline);

	uint32_t currentFrameIdx = static_cast<uint32_t>(_pingPong);

	pvrvk::DescriptorSet arrayDS[] = { _deviceResources->commonDescriptorSet,
		_denoise ? _deviceResources->deferredShadingDescriptorSet[currentFrameIdx] : _deviceResources->deferredShadingNoDenoisingDescriptorSet[currentFrameIdx] };

	uint32_t offsets[4] = {};
	offsets[0] = _deviceResources->globalBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[1] = _deviceResources->lightDataBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[2] = _deviceResources->perMeshBufferView.getDynamicSliceOffset(swapchainIndex);
	offsets[3] = _deviceResources->perMeshPrevTransformBufferView.getDynamicSliceOffset(swapchainIndex);

	cmdBuffers->bindDescriptorSets(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->deferredShadingPipelineLayout, 0, arrayDS, 2, offsets, 4);

	cmdBuffers->draw(0, 3);

	pvr::utils::endCommandBufferDebugLabel(cmdBuffers);
}

/// <summary>Record UIRenderer commands.</summary>
/// <param name="commandBuff">Commandbuffer to record.</param>
void VulkanRayTracingDenoising::recordCommandUIRenderer(pvrvk::SecondaryCommandBuffer& commandBuff)
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
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanRayTracingDenoising>(); }
