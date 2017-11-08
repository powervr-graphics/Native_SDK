/*!*********************************************************************************************************************
\File         VulkanDeferredShading.cpp
\Title        Deferred Shading
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\Description  Implements a deferred shading technique supporting point and directional lights.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRVk/PVRVk.h"
#include "PVRUtils/PVRUtilsVk.h"

// Maximum number of swap images supported
enum CONSTANTS
{
	MAX_NUMBER_OF_SWAP_IMAGES = 4
};

// Shader vertex Bindings
const pvr::utils::VertexBindings_Name vertexBindings[] =
{
	{ "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoords" }, { "TANGENT", "inTangent" }
};

const pvr::utils::VertexBindings_Name floorVertexBindings[] =
{
	{ "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoords" }
};

const pvr::utils::VertexBindings_Name pointLightVertexBindings[] =
{
	{ "POSITION", "inVertex" }
};

// Framebuffer color attachment indices
namespace FramebufferGBufferAttachments {
enum Enum
{
	Albedo = 0,
	Normal,
	Depth,
	Count
};
}

// Light mesh nodes
namespace LightNodes {
enum Enum
{
	PointLightMeshNode = 0,
	NumberOfPointLightMeshNodes
};
}

// mesh nodes
namespace MeshNodes {
enum Enum
{
	Satyr = 0,
	Floor = 1,
	NumberOfMeshNodes
};
}

// Structures used for storing the shared point light data for the point light passes
struct PointLightPasses
{
	struct PointLightProperties
	{
		glm::mat4 worldViewProjectionMatrix;
		glm::mat4 proxyWorldViewMatrix;
		glm::mat4 proxyWorldViewProjectionMatrix;
		glm::vec4 proxyViewSpaceLightPosition;
		glm::vec4 lightColor;
		glm::vec4 lightSourceColor;
		float lightIntensity;
		float lightRadius;
	};

	std::vector<PointLightProperties> lightProperties;

	struct InitialData
	{
		float radial_vel;
		float axial_vel;
		float vertical_vel;
		float angle;
		float distance;
		float height;
	};

	std::vector<InitialData> initialData;
};

// structure used to draw the point light sources
struct DrawPointLightSources
{
	pvrvk::GraphicsPipeline pipeline;
};

// structure used to draw the proxy point light
struct DrawPointLightProxy
{
	pvrvk::GraphicsPipeline pipeline;
};

// structure used to fill the stencil buffer used for optimsing the the proxy point light pass
struct PointLightGeometryStencil
{
	pvrvk::GraphicsPipeline pipeline;
};

// structure used to render directional lighting
struct DrawDirectionalLight
{
	pvrvk::GraphicsPipeline pipeline;
	struct DirectionalLightProperties
	{
		glm::vec4 lightIntensity;
		glm::vec4 viewSpaceLightDirection;
	};
	std::vector<DirectionalLightProperties> lightProperties;
};

// structure used to fill the GBuffer
struct DrawGBuffer
{
	struct Objects
	{
		pvrvk::GraphicsPipeline pipeline;
		glm::mat4 world;
		glm::mat4 worldView;
		glm::mat4 worldViewProj;
		glm::mat4 worldViewIT4x4;
	};
	std::vector<Objects> objects;
};

// structure used to hold the rendering information for the demo
struct RenderData
{
	DrawGBuffer storeLocalMemoryPass; // Subpass 0
	DrawDirectionalLight directionalLightPass; // Subpass 1
	PointLightGeometryStencil pointLightGeometryStencilPass; // Subpass 1
	DrawPointLightProxy pointLightProxyPass; // Subpass 1
	DrawPointLightSources pointLightSourcesPass; // Subpass 1
	PointLightPasses pointLightPasses; // holds point light data
};

// Shader names for all of the demo passes
namespace Files {
const char* const PointLightModelFile = "pointlight.pod";
const char* const SceneFile = "scene.pod";

const char* const GBufferVertexShader = "GBufferVertexShader_vk.vsh.spv";
const char* const GBufferFragmentShader = "GBufferFragmentShader_vk.fsh.spv";

const char* const GBufferFloorVertexShader = "GBufferFloorVertexShader_vk.vsh.spv";
const char* const GBufferFloorFragmentShader = "GBufferFloorFragmentShader_vk.fsh.spv";

const char* const AttributelessVertexShader = "AttributelessVertexShader_vk.vsh.spv";

const char* const DirectionalLightingFragmentShader = "DirectionalLightFragmentShader_vk.fsh.spv";

const char* const PointLightPass1FragmentShader = "PointLightPass1FragmentShader_vk.fsh.spv";
const char* const PointLightPass1VertexShader = "PointLightPass1VertexShader_vk.vsh.spv";

const char* const PointLightPass2FragmentShader = "PointLightPass2FragmentShader_vk.fsh.spv";
const char* const PointLightPass2VertexShader = "PointLightPass2VertexShader_vk.vsh.spv";

const char* const PointLightPass3FragmentShader = "PointLightPass3FragmentShader_vk.fsh.spv";
const char* const PointLightPass3VertexShader = "PointLightPass3VertexShader_vk.vsh.spv";
}

// buffer entry names used for the structured memory views used throughout the demo
// These entry names must match the variable names used in the demo shaders
namespace BufferEntryNames {
namespace PerScene {
const char* const FarClipDistance = "fFarClipDistance";
}

namespace PerModelMaterial {
const char* const SpecularStrength = "fSpecularStrength";
const char* const DiffuseColor = "vDiffuseColor";
}

namespace PerModel {
const char* const WorldViewProjectionMatrix = "mWorldViewProjectionMatrix";
const char* const WorldViewMatrix = "mWorldViewMatrix";
const char* const WorldViewITMatrix = "mWorldViewITMatrix";
}

namespace PerPointLight {
const char* const LightIntensity = "vLightIntensity";
const char* const LightRadius = "vLightRadius";
const char* const LightColor = "vLightColor";
const char* const LightSourceColor = "vLightSourceColor";
const char* const WorldViewProjectionMatrix = "mWorldViewProjectionMatrix";
const char* const ProxyLightViewPosition = "vViewPosition";
const char* const ProxyWorldViewProjectionMatrix = "mProxyWorldViewProjectionMatrix";
const char* const ProxyWorldViewMatrix = "mProxyWorldViewMatrix";
}

namespace PerDirectionalLight {
const char* const LightIntensity = "fLightIntensity";
const char* const LightViewDirection = "vViewDirection";
}
}

// Application wide configuration data
namespace ApplicationConfiguration {
const float FrameRate = 1.0f / 120.0f;
}

// Directional lighting configuration data
namespace DirectionalLightConfiguration {
static bool AdditionalDirectionalLight = true;
const float DirectionalLightIntensity = .2f;
}

// Point lighting configuration data
namespace PointLightConfiguration {
float LightMaxDistance = 40.f;
float LightMinDistance = 20.f;
float LightMinHeight = -30.f;
float LightMaxHeight = 40.f;
float LightAxialVelocityChange = .01f;
float LightRadialVelocityChange = .003f;
float LightVerticalVelocityChange = .01f;
float LightMaxAxialVelocity = 5.f;
float LightMaxRadialVelocity = 1.5f;
float LightMaxVerticalVelocity = 5.f;

static int32_t MaxScenePointLights = 5;
static int32_t NumProceduralPointLights = 10;
float PointLightScale = 32.0f; // PointLightScale handles the size of the scaled light geometry. This effects the areas of the screen which will go through point light rendering
float PointLightRadius = PointLightScale / 2.0f; // PointLightRadius handles the actual point light falloff. Modifying one of these requires also modifying the other
float PointlightIntensity = 5.0f;
}

// Subpasses used in the renderpass
namespace RenderPassSubPasses {
const uint32_t GBuffer = 0;
// lighting pass
const uint32_t Lighting = 1;
// UI pass
const uint32_t UIRenderer = 1;

const uint32_t NumberOfSubpasses = 2;
}

/*!*********************************************************************************************************************
Class implementing the Shell functions.
***********************************************************************************************************************/
class VulkanDeferredShading : public pvr::Shell
{
public:
	struct Material
	{
		pvrvk::GraphicsPipeline materialPipeline;
		std::vector<pvrvk::DescriptorSet> materialDescriptorSet;
		float specularStrength;
		glm::vec3 diffuseColor;
	};

	struct DeviceResources
	{
		pvrvk::Instance instance;
		pvrvk::Device device;
		pvrvk::Surface surface;
		pvrvk::Queue queue;
		pvrvk::Swapchain swapchain;

		pvrvk::CommandPool commandPool;
		pvrvk::DescriptorPool descriptorPool;

		// Local memory frame buffer
		pvr::Multi<pvrvk::Framebuffer> onScreenLocalMemoryFramebuffer;
		pvr::Multi<pvrvk::ImageView> depthStencilImages;
		pvr::Multi<pvrvk::FramebufferCreateInfo> onScreenFramebufferCreateInfos;

		// Stores Texture views for the Images used as attachments on the local memory frame buffer
		pvr::Multi<pvrvk::ImageView> framebufferGbufferImages[FramebufferGBufferAttachments::Count];

		// Common renderpass used for the demo
		pvrvk::RenderPass onScreenLocalMemoryRenderPass;

		// Vbo and Ibos used for lighting data
		pvrvk::Buffer pointLightVbo;
		pvrvk::Buffer pointLightIbo;


		//// Command Buffers ////
		// Main Primary Command Buffer
		pvrvk::CommandBuffer commandBufferMain[MAX_NUMBER_OF_SWAP_IMAGES];

		// Secondary commandbuffers used for each pass
		pvrvk::SecondaryCommandBuffer commandBufferRenderToLocalMemory[MAX_NUMBER_OF_SWAP_IMAGES];
		pvrvk::SecondaryCommandBuffer commandBufferLighting[MAX_NUMBER_OF_SWAP_IMAGES];

		////  Descriptor Set Layouts ////
		// Layouts used for GBuffer rendering
		pvrvk::DescriptorSetLayout staticSceneLayout;
		pvrvk::DescriptorSetLayout noSamplerLayout;
		pvrvk::DescriptorSetLayout oneSamplerLayout;
		pvrvk::DescriptorSetLayout twoSamplerLayout;
		pvrvk::DescriptorSetLayout threeSamplerLayout;
		pvrvk::DescriptorSetLayout fourSamplerLayout;

		// Directional lighting descriptor set layout
		pvrvk::DescriptorSetLayout directionalLightingDescriptorLayout;
		// Point light stencil pass descriptor set layout
		pvrvk::DescriptorSetLayout pointLightGeometryStencilDescriptorLayout;
		// Point Proxy light pass descriptor set layout used for buffers
		pvrvk::DescriptorSetLayout pointLightProxyDescriptorLayout;
		// Point Proxy light pass descriptor set layout used for local memory
		pvrvk::DescriptorSetLayout pointLightProxyLocalMemoryDescriptorLayout;
		// Point light source descriptor set layout used for buffers
		pvrvk::DescriptorSetLayout pointLightSourceDescriptorLayout;


		////  Descriptor Sets ////
		// GBuffer Materials structures
		std::vector<Material>materials;
		// Directional Lighting descriptor set
		pvr::Multi<pvrvk::DescriptorSet> directionalLightingDescriptorSets;
		// Point light stencil descriptor set
		pvr::Multi<pvrvk::DescriptorSet> pointLightGeometryStencilDescriptorSets;
		// Point light Proxy descriptor set
		pvr::Multi<pvrvk::DescriptorSet> pointLightProxyDescriptorSets;
		pvr::Multi<pvrvk::DescriptorSet> pointLightProxyLocalMemoryDescriptorSets;
		// Point light Source descriptor set
		pvr::Multi<pvrvk::DescriptorSet> pointLightSourceDescriptorSets;
		// Scene wide descriptor set
		pvrvk::DescriptorSet sceneDescriptorSet;


		//// Pipeline Layouts ////
		// GBuffer pipeline layouts
		pvrvk::PipelineLayout pipeLayoutNoSamplers;
		pvrvk::PipelineLayout pipeLayoutOneSampler;
		pvrvk::PipelineLayout pipeLayoutTwoSamplers;
		pvrvk::PipelineLayout pipeLayoutThreeSamplers;
		pvrvk::PipelineLayout pipeLayoutFourSamplers;

		// Directional lighting pipeline layout
		pvrvk::PipelineLayout directionalLightingPipelineLayout;
		// Point lighting stencil pipeline layout
		pvrvk::PipelineLayout pointLightGeometryStencilPipelineLayout;
		// Point lighting proxy pipeline layout
		pvrvk::PipelineLayout pointLightProxyPipelineLayout;
		// Point lighting source pipeline layout
		pvrvk::PipelineLayout pointLightSourcePipelineLayout;
		// Scene Wide pipeline layout
		pvrvk::PipelineLayout scenePipelineLayout;

		// scene Vbos and Ibos
		std::vector<pvrvk::Buffer> sceneVbos;
		std::vector<pvrvk::Buffer> sceneIbos;


		//// Structured Memory Views ////
		// scene wide buffers
		pvr::utils::StructuredBufferView farClipDistanceBufferView;
		pvrvk::Buffer farClipDistanceBuffer;
		// Static materials buffers
		pvr::utils::StructuredBufferView modelMaterialBufferView;
		pvrvk::Buffer modelMaterialBuffer;
		// Dynamic matrices buffers
		pvr::utils::StructuredBufferView modelMatrixBufferView;
		pvrvk::Buffer modelMatrixBuffer;
		// Static point light buffers
		pvr::utils::StructuredBufferView staticPointLightBufferView;
		pvrvk::Buffer staticPointLightBuffer;
		// Dynamic point light buffer
		pvr::utils::StructuredBufferView dynamicPointLightBufferView;
		pvrvk::Buffer dynamicPointLightBuffer;
		// Static Directional lighting buffer
		pvr::utils::StructuredBufferView staticDirectionalLightBufferView;
		pvrvk::Buffer staticDirectionalLightBuffer;
		// Dynamic Directional lighting buffers
		pvr::utils::StructuredBufferView dynamicDirectionalLightBufferView;
		pvrvk::Buffer dynamicDirectionalLightBuffer;

		pvrvk::Semaphore semaphoreImageAcquired[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Fence perFrameAcquireFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Semaphore semaphorePresent[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Fence perFrameCommandBufferFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

		RenderData renderInfo;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;
	};

	//// Frame ////
	uint32_t _numSwapImages;
	uint32_t _swapchainIndex;
	//Putting all api objects into a pointer just makes it easier to release them all together with RAII
	std::unique_ptr<DeviceResources> _deviceResources;

	// Frame counters for animation
	uint32_t _frameId;
	float _frameNumber;
	bool _isPaused;
	uint32_t _cameraId;
	bool _animateCamera;

	uint32_t _numberOfPointLights;
	uint32_t _numberOfDirectionalLights;

	// Projection and Model View matrices
	glm::vec3 _cameraPosition;
	glm::mat4 _viewMatrix;
	glm::mat4 _projectionMatrix;
	glm::mat4 _viewProjectionMatrix;
	glm::mat4 _inverseViewMatrix;
	float _farClipDistance;

	int32_t _windowWidth;
	int32_t _windowHeight;
	int32_t _framebufferWidth;
	int32_t _framebufferHeight;

	int32_t _viewportOffsets[2];

	// Light models
	pvr::assets::ModelHandle _pointLightModel;

	// Object model
	pvr::assets::ModelHandle _mainScene;

	VulkanDeferredShading() { _animateCamera = false; _isPaused = false; }

	//  Overriden from pvr::Shell
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void createFramebufferAndRenderpass();
	void createPipelines();
	void createModelPipelines();
	void createDirectionalLightingPipeline();
	void createPointLightStencilPipeline();
	void createPointLightProxyPipeline();
	void createPointLightSourcePipeline();
	void recordCommandBufferRenderGBuffer(pvrvk::SecondaryCommandBuffer& commandBuffer, uint32_t swapChainIndex, uint32_t subpass);
	void recordCommandsDirectionalLights(pvrvk::SecondaryCommandBuffer& commandBuffer, uint32_t swapChainIndex, uint32_t subpass);
	void recordCommandsPointLightGeometryStencil(pvrvk::SecondaryCommandBuffer& commandBuffer, uint32_t swapChainIndex, uint32_t subpass, const uint32_t pointLight, const pvr::assets::Mesh& pointLightMesh);
	void recordCommandsPointLightProxy(pvrvk::SecondaryCommandBuffer& commandBuffer, uint32_t swapChainIndex, uint32_t subpass, const uint32_t pointLight, const pvr::assets::Mesh& pointLightMesh);
	void recordCommandsPointLightSourceLighting(pvrvk::SecondaryCommandBuffer& commandBuffer, uint32_t swapChainIndex, uint32_t subpass);
	void recordMainCommandBuffer();
	void recordCommandUIRenderer(pvrvk::SecondaryCommandBuffer& commandBuffer, uint32_t swapChainIndex, uint32_t subpass);
	void recordSecondaryCommandBuffers();
	void allocateLights();
	bool createMaterialsAndDescriptorSets(std::vector<pvr::utils::ImageUploadResults>& uploadResults);
	void createStaticSceneDescriptorSet();
	bool loadVbos();
	void uploadStaticData();
	void uploadStaticSceneData();
	void uploadStaticModelData();
	void uploadStaticDirectionalLightData();
	void uploadStaticPointLightData();
	void initialiseStaticLightProperties();
	void updateDynamicSceneData();
	void createBuffers();
	void createSceneWideBuffers();
	void updateAnimation();
	void updateProceduralPointLight(PointLightPasses::InitialData& data,
	                                PointLightPasses::PointLightProperties& pointLightProperties, bool initia);
	void createModelBuffers();
	void createLightingBuffers();
	void createDirectionalLightingBuffers();
	void createPointLightBuffers();
	void createDirectionalLightDescriptorSets();
	void createPointLightGeometryStencilPassDescriptorSets();
	void createPointLightProxyPassDescriptorSets();
	void createPointLightSourcePassDescriptorSets();

	void eventMappedInput(pvr::SimplifiedInput key)
	{
		switch (key)
		{
		// Handle input
		case pvr::SimplifiedInput::ActionClose: exitShell(); break;
		case pvr::SimplifiedInput::Action1: _isPaused = !_isPaused; break;
		case pvr::SimplifiedInput::Action2: _animateCamera = !_animateCamera; break;
		}
	}
};

/*!*********************************************************************************************************************
\return Return true if no error occurred
\brief  Code in initApplication() will be called by pvr::Shell once per run, before the rendering context is created.
Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result VulkanDeferredShading::initApplication()
{
	// This demo application makes heavy use of the stencil buffer
	setStencilBitsPerPixel(8);

	_frameNumber = 0.0f;
	_isPaused = false;
	_cameraId = 0;

	//  Load the scene and the light
	if (!pvr::assets::helper::loadModel(*this, Files::SceneFile, _mainScene))
	{
		setExitMessage("ERROR: Couldn't load the scene pod file %s\n", Files::SceneFile);
		return pvr::Result::UnknownError;
	}

	if (_mainScene->getNumCameras() == 0)
	{
		setExitMessage("ERROR: The main scene to display must contain a camera.\n");
		return pvr::Result::UnknownError;
	}

	//  Load light proxy geometry
	if (!pvr::assets::helper::loadModel(*this, Files::PointLightModelFile, _pointLightModel))
	{
		setExitMessage("ERROR: Couldn't load the point light proxy pod file\n");
		return pvr::Result::UnknownError;
	}

	//Create the empty API objects.
	_deviceResources.reset(new DeviceResources);

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in initView() will be called by PVRShell upon initialization or after a change in the rendering context.
Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result VulkanDeferredShading::initView()
{
	_frameId = 0;
	bool instanceResult = pvr::utils::createInstanceAndSurface(this->getApplicationName(), this->getWindow(), this->getDisplay(),
	                      _deviceResources->instance, _deviceResources->surface);
	if (!instanceResult || !_deviceResources->instance.isValid())
	{
		setExitMessage("Failed to create the instance.\n");
		return pvr::Result::InitializationError;
	}

	pvr::utils::QueuePopulateInfo queueFlagsInfo[] =
	{
		{VkQueueFlags::e_GRAPHICS_BIT, _deviceResources->surface },
	};
	pvr::utils::QueueAccessInfo queueAccessInfo;

	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0),
	                           queueFlagsInfo, ARRAY_SIZE(queueFlagsInfo), &queueAccessInfo);

	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(_deviceResources->surface);

	// validate the supported swapchain image usage
	VkImageUsageFlags swapchainImageUsage = VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, VkImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= VkImageUsageFlags::e_TRANSFER_SRC_BIT;
	}

	// create the swapchain
	bool swapchainResult = pvr::utils::createSwapchainAndDepthStencilImageView(_deviceResources->device,
	                       _deviceResources->surface, getDisplayAttributes(), _deviceResources->swapchain,
	                       _deviceResources->depthStencilImages, swapchainImageUsage,
	                       VkImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | VkImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT);

	if (swapchainResult == false)
	{
		return pvr::Result::UnknownError;
	}

	// Get the number of swap images
	_numSwapImages = _deviceResources->swapchain->getSwapchainLength();

	// Get current swap index
	_swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	// initialise the gbuffer renderpass list
	_deviceResources->renderInfo.storeLocalMemoryPass.objects.resize(_mainScene->getNumMeshNodes());

	// calculate the frame buffer width and heights
	_framebufferWidth = _windowWidth = this->getWidth();
	_framebufferHeight = _windowHeight = this->getHeight();

	const pvr::CommandLine& commandOptions = getCommandLine();

	commandOptions.getIntOption("-fbowidth", _framebufferWidth);
	_framebufferWidth = glm::min<int32_t>(_framebufferWidth, _windowWidth);
	commandOptions.getIntOption("-fboheight", _framebufferHeight);
	_framebufferHeight = glm::min<int32_t>(_framebufferHeight, _windowHeight);
	commandOptions.getIntOption("-numlights", PointLightConfiguration::NumProceduralPointLights);
	commandOptions.getFloatOption("-lightintensity", PointLightConfiguration::PointlightIntensity);

	_viewportOffsets[0] = (_windowWidth - _framebufferWidth) / 2;
	_viewportOffsets[1] = (_windowHeight - _framebufferHeight) / 2;

	Log(LogLevel::Information, "Framebuffer dimensions: %d x %d\n", _framebufferWidth, _framebufferHeight);
	Log(LogLevel::Information, "Onscreen Framebuffer dimensions: %d x %d\n", _windowWidth, _windowHeight);

	// create the commandpool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(queueAccessInfo.familyId,
	                                VkCommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT);

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(
	                                     pvrvk::DescriptorPoolCreateInfo()
	                                     .addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER, 32)
	                                     .addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 32)
	                                     .addDescriptorInfo(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 32)
	                                     .addDescriptorInfo(VkDescriptorType::e_INPUT_ATTACHMENT, 32)
	                                     .setMaxDescriptorSets(16));

	// setup command buffers
	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		// main command buffer
		_deviceResources->commandBufferMain[i] = _deviceResources->commandPool->allocateCommandBuffer();

		// Subpass 0
		_deviceResources->commandBufferRenderToLocalMemory[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();

		// Subpass 1
		_deviceResources->commandBufferLighting[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();

		_deviceResources->semaphorePresent[i] = _deviceResources->device->createSemaphore();
		_deviceResources->semaphoreImageAcquired[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameCommandBufferFence[i] = _deviceResources->device->createFence(VkFenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->perFrameAcquireFence[i] = _deviceResources->device->createFence(VkFenceCreateFlags::e_SIGNALED_BIT);
	}

	// Create the renderpass using subpasses
	createFramebufferAndRenderpass();

	// Initialise lighting structures
	allocateLights();

	// Create buffers used in the demo
	createBuffers();

	// Initialise the static light properties
	initialiseStaticLightProperties();

	// Create static scene wide descriptor set
	createStaticSceneDescriptorSet();

	_deviceResources->commandBufferMain[0]->begin();
	std::vector<pvr::utils::ImageUploadResults> uploadResults;
	// Create the descriptor sets used for the GBuffer pass
	if (!createMaterialsAndDescriptorSets(uploadResults))
	{
		return pvr::Result::NotInitialized;
	}
	_deviceResources->commandBufferMain[0]->end();
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->commandBufferMain[0];
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle();// wait

	// Upload static data
	uploadStaticData();

	// Create lighting descriptor sets
	createDirectionalLightDescriptorSets();
	createPointLightGeometryStencilPassDescriptorSets();
	createPointLightProxyPassDescriptorSets();
	createPointLightSourcePassDescriptorSets();

	// setup UI renderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenLocalMemoryRenderPass, RenderPassSubPasses::UIRenderer, _deviceResources->commandPool, _deviceResources->queue);
	_deviceResources->uiRenderer.getDefaultTitle()->setText("DeferredShading");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText("Action1: Pause\nAction2: Orbit Camera\n");
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	// Handle device rotation
	bool isRotated = this->isScreenRotated() && this->isFullScreen();
	if (isRotated)
	{
		_projectionMatrix = pvr::math::perspective(pvr::Api::Vulkan,
		                    _mainScene->getCamera(0).getFOV(), (float)this->getHeight() / (float)this->getWidth(),
		                    _mainScene->getCamera(0).getNear(), _mainScene->getCamera(0).getFar(), glm::pi<float>() * .5f);
	}
	else
	{
		_projectionMatrix = pvr::math::perspective(pvr::Api::Vulkan,
		                    _mainScene->getCamera(0).getFOV(), (float)this->getWidth() / (float)this->getHeight(),
		                    _mainScene->getCamera(0).getNear(), _mainScene->getCamera(0).getFar());
	}

	//  Load objects from the scene into VBOs
	if (!loadVbos()) { return pvr::Result::UnknownError; }

	// Create demo pipelines
	createPipelines();

	// Record all secondary command buffers
	recordSecondaryCommandBuffers();

	// Record the main command buffer
	recordMainCommandBuffer();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result VulkanDeferredShading::releaseView()
{
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); i++)
	{
		_deviceResources->perFrameAcquireFence[i]->wait();
		_deviceResources->perFrameAcquireFence[i]->reset();

		_deviceResources->perFrameCommandBufferFence[i]->wait();
		_deviceResources->perFrameCommandBufferFence[i]->reset();
	}

	_deviceResources->device->waitIdle();
	_deviceResources.reset(0);
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
If the rendering context is lost, QuitApplication() will not be called.x
***********************************************************************************************************************/
pvr::Result VulkanDeferredShading::quitApplication() { return pvr::Result::Success; }

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every _frameNumber.
***********************************************************************************************************************/
pvr::Result VulkanDeferredShading::renderFrame()
{
	_deviceResources->perFrameAcquireFence[_frameId]->wait();
	_deviceResources->perFrameAcquireFence[_frameId]->reset();
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->semaphoreImageAcquired[_frameId], _deviceResources->perFrameAcquireFence[_frameId]);

	_swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameCommandBufferFence[_swapchainIndex]->wait();
	_deviceResources->perFrameCommandBufferFence[_swapchainIndex]->reset();

	//  Handle user input and update object animations
	updateAnimation();

	// update dynamic buffers
	updateDynamicSceneData();

	//--------------------
	// submit the main command buffer
	pvrvk::SubmitInfo submitInfo;
	VkPipelineStageFlags pipeWaitStage = VkPipelineStageFlags::e_ALL_GRAPHICS_BIT;
	submitInfo.commandBuffers = &_deviceResources->commandBufferMain[_swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->semaphoreImageAcquired[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->semaphorePresent[_frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.waitDestStages = &pipeWaitStage;
	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameCommandBufferFence[_swapchainIndex]);

	if (this->shouldTakeScreenshot())
	{
		if (_deviceResources->swapchain->supportsUsage(VkImageUsageFlags::e_TRANSFER_SRC_BIT))
		{
			pvr::utils::takeScreenshot(_deviceResources->swapchain, _swapchainIndex, _deviceResources->commandPool, _deviceResources->queue, this->getScreenshotFileName());
		}
		else
		{
			Log(LogLevel::Warning, "Could not take screenshot as the swapchain does not support TRANSFER_SRC_BIT");
		}
	}

	//--------------------
	// Present
	pvrvk::PresentInfo presentInfo;
	presentInfo.waitSemaphores = &_deviceResources->semaphorePresent[_frameId];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.imageIndices = &_swapchainIndex;
	pvr::Result result = _deviceResources->queue->present(presentInfo)  ==
	                     VkResult::e_SUCCESS ? pvr::Result::Success : pvr::Result::UnknownError;

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();

	return result;
}

/*!*********************************************************************************************************************
\brief  Creates directional lighting descriptor sets.
***********************************************************************************************************************/
void VulkanDeferredShading::createDirectionalLightDescriptorSets()
{
	{
		// create the descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;

		// Buffers
		descSetInfo.setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, VkShaderStageFlags::e_FRAGMENT_BIT);
		descSetInfo.setBinding(1, VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, VkShaderStageFlags::e_FRAGMENT_BIT);

		// Input attachments
		descSetInfo.setBinding(2, VkDescriptorType::e_INPUT_ATTACHMENT, 1u, VkShaderStageFlags::e_FRAGMENT_BIT);
		descSetInfo.setBinding(3, VkDescriptorType::e_INPUT_ATTACHMENT, 1u, VkShaderStageFlags::e_FRAGMENT_BIT);
		descSetInfo.setBinding(4, VkDescriptorType::e_INPUT_ATTACHMENT, 1u, VkShaderStageFlags::e_FRAGMENT_BIT);

		_deviceResources->directionalLightingDescriptorLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);

		{
			// create the pipeline layout
			pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

			pipeLayoutInfo.setDescSetLayout(0, _deviceResources->directionalLightingDescriptorLayout);
			_deviceResources->directionalLightingPipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
		}
		pvrvk::WriteDescriptorSet descSetUpdate[pvrvk::FrameworkCaps::MaxSwapChains *  5];

		// create the swapchain descriptor sets with corresponding buffers/images
		for (uint32_t i = 0; i < _numSwapImages; ++i)
		{
			_deviceResources->directionalLightingDescriptorSets.add(
			  _deviceResources->descriptorPool->allocateDescriptorSet(
			    _deviceResources->directionalLightingDescriptorLayout));
			descSetUpdate[i * 5]
			.set(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC,
			     _deviceResources->directionalLightingDescriptorSets[i], 0)
			.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->staticDirectionalLightBuffer, 0, _deviceResources->staticDirectionalLightBufferView.getDynamicSliceSize()));

			descSetUpdate[i * 5 + 1]
			.set(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->directionalLightingDescriptorSets[i], 1)
			.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->dynamicDirectionalLightBuffer, 0, _deviceResources->dynamicDirectionalLightBufferView.getDynamicSliceSize()));

			descSetUpdate[i * 5 + 2]
			.set(VkDescriptorType::e_INPUT_ATTACHMENT, _deviceResources->directionalLightingDescriptorSets[i], 2)
			.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->framebufferGbufferImages[FramebufferGBufferAttachments::Albedo][i],
			              VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

			descSetUpdate[i * 5 + 3]
			.set(VkDescriptorType::e_INPUT_ATTACHMENT, _deviceResources->directionalLightingDescriptorSets[i], 3)
			.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->framebufferGbufferImages[FramebufferGBufferAttachments::Normal][i],
			              VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

			descSetUpdate[i * 5 + 4]
			.set(VkDescriptorType::e_INPUT_ATTACHMENT, _deviceResources->directionalLightingDescriptorSets[i], 4)
			.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->framebufferGbufferImages[FramebufferGBufferAttachments::Depth][i],
			              VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
		}
		_deviceResources->device->updateDescriptorSets(descSetUpdate, _numSwapImages * 5, nullptr, 0);
	}
}

/*!*********************************************************************************************************************
\brief  Creates point lighting stencil pass descriptor sets.
***********************************************************************************************************************/
void VulkanDeferredShading::createPointLightGeometryStencilPassDescriptorSets()
{
	{
		// create descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;

		// buffers
		descSetLayoutInfo.setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, VkShaderStageFlags::e_FRAGMENT_BIT);
		descSetLayoutInfo.setBinding(1, VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, VkShaderStageFlags::e_VERTEX_BIT);

		_deviceResources->pointLightGeometryStencilDescriptorLayout =
		  _deviceResources->device->createDescriptorSetLayout(descSetLayoutInfo);

		{
			// create the pipeline layout
			pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

			pipeLayoutInfo.setDescSetLayout(0, _deviceResources->staticSceneLayout);
			pipeLayoutInfo.setDescSetLayout(1, _deviceResources->pointLightGeometryStencilDescriptorLayout);
			_deviceResources->pointLightGeometryStencilPipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
		}
		std::vector<pvrvk::WriteDescriptorSet> writeDescSets(_numSwapImages * 2);
		// create the swapchain descriptor sets with corresponding buffers
		for (uint32_t i = 0; i < _numSwapImages; ++i)
		{
			_deviceResources->pointLightGeometryStencilDescriptorSets.add(
			  _deviceResources->descriptorPool->allocateDescriptorSet(
			    _deviceResources->pointLightGeometryStencilDescriptorLayout));

			pvrvk::WriteDescriptorSet* descSetUpdate = &writeDescSets[i * _numSwapImages];
			descSetUpdate->set(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC,
			                   _deviceResources->pointLightGeometryStencilDescriptorSets[i], 0);
			descSetUpdate->setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->staticPointLightBuffer, 0, _deviceResources->staticPointLightBufferView.getDynamicSliceSize()));

			descSetUpdate = &writeDescSets[i * 2 + 1];
			descSetUpdate->set(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC,
			                   _deviceResources->pointLightGeometryStencilDescriptorSets[i], 1);
			descSetUpdate->setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->dynamicPointLightBuffer, 0, _deviceResources->dynamicPointLightBufferView.getDynamicSliceSize()));
		}
		_deviceResources->device->updateDescriptorSets(writeDescSets.data(), writeDescSets.size(), nullptr, 0);
	}
}

/*!*********************************************************************************************************************
\brief  Creates point lighting proxy pass descriptor sets.
***********************************************************************************************************************/
void VulkanDeferredShading::createPointLightProxyPassDescriptorSets()
{
	{
		// create buffer descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;

		// Buffers
		descSetInfo.setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, VkShaderStageFlags::e_FRAGMENT_BIT);

		descSetInfo.setBinding(1, VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, VkShaderStageFlags::e_VERTEX_BIT | VkShaderStageFlags::e_FRAGMENT_BIT);

		_deviceResources->pointLightProxyDescriptorLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);

		pvrvk::DescriptorSetLayoutCreateInfo localMemoryDescSetInfo;

		// Input attachment descriptor set layout
		localMemoryDescSetInfo.setBinding(0, VkDescriptorType::e_INPUT_ATTACHMENT, 1u, VkShaderStageFlags::e_FRAGMENT_BIT);

		localMemoryDescSetInfo.setBinding(1, VkDescriptorType::e_INPUT_ATTACHMENT, 1u, VkShaderStageFlags::e_FRAGMENT_BIT);

		localMemoryDescSetInfo.setBinding(2, VkDescriptorType::e_INPUT_ATTACHMENT, 1u, VkShaderStageFlags::e_FRAGMENT_BIT);

		_deviceResources->pointLightProxyLocalMemoryDescriptorLayout = _deviceResources->device->createDescriptorSetLayout(localMemoryDescSetInfo);

		{
			// create the pipeline layout
			pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
			pipeLayoutInfo.setDescSetLayout(0, _deviceResources->staticSceneLayout);
			pipeLayoutInfo.setDescSetLayout(1, _deviceResources->pointLightProxyDescriptorLayout);
			pipeLayoutInfo.setDescSetLayout(2, _deviceResources->pointLightProxyLocalMemoryDescriptorLayout);
			_deviceResources->pointLightProxyPipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
		}

		// create the swapchain descriptor sets with corresponding buffers
		std::vector<pvrvk::WriteDescriptorSet> descSetWrites;
		for (uint32_t i = 0; i < _numSwapImages; ++i)
		{
			_deviceResources->pointLightProxyDescriptorSets.add(_deviceResources->descriptorPool->allocateDescriptorSet(
			      _deviceResources->pointLightProxyDescriptorLayout));

			descSetWrites.push_back(pvrvk::WriteDescriptorSet(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC,
			                        _deviceResources->pointLightProxyDescriptorSets[i], 0)
			                        .setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->staticPointLightBuffer, 0, _deviceResources->staticPointLightBufferView.getDynamicSliceSize())));

			descSetWrites.push_back(pvrvk::WriteDescriptorSet(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC,
			                        _deviceResources->pointLightProxyDescriptorSets[i], 1)
			                        .setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->dynamicPointLightBuffer, 0, _deviceResources->dynamicPointLightBufferView.getDynamicSliceSize())));

		}

		_deviceResources->pointLightProxyLocalMemoryDescriptorLayout = _deviceResources->device->createDescriptorSetLayout(localMemoryDescSetInfo);
		// create the swapchain descriptor sets with corresponding images
		for (uint32_t i = 0; i < _numSwapImages; ++i)
		{
			_deviceResources->pointLightProxyLocalMemoryDescriptorSets.add(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->pointLightProxyLocalMemoryDescriptorLayout));
			descSetWrites.push_back(
			  pvrvk::WriteDescriptorSet(VkDescriptorType::e_INPUT_ATTACHMENT,
			                            _deviceResources->pointLightProxyLocalMemoryDescriptorSets[i], 0)
			  .setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->framebufferGbufferImages[FramebufferGBufferAttachments::Albedo][i],
			                VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

			descSetWrites.push_back(
			  pvrvk::WriteDescriptorSet(VkDescriptorType::e_INPUT_ATTACHMENT,
			                            _deviceResources->pointLightProxyLocalMemoryDescriptorSets[i], 1)
			  .setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->framebufferGbufferImages[FramebufferGBufferAttachments::Normal][i],
			                VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

			descSetWrites.push_back(
			  pvrvk::WriteDescriptorSet(VkDescriptorType::e_INPUT_ATTACHMENT,
			                            _deviceResources->pointLightProxyLocalMemoryDescriptorSets[i], 2)
			  .setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->framebufferGbufferImages[FramebufferGBufferAttachments::Depth][i],
			                VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
		}
		_deviceResources->device->updateDescriptorSets(descSetWrites.data(), descSetWrites.size(), nullptr, 0);
	}
}

/*!*********************************************************************************************************************
\brief  Creates point lighting source pass descriptor sets.
***********************************************************************************************************************/
void VulkanDeferredShading::createPointLightSourcePassDescriptorSets()
{
	{
		// create descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;

		descSetInfo.setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, VkShaderStageFlags::e_FRAGMENT_BIT);
		descSetInfo.setBinding(1, VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, VkShaderStageFlags::e_VERTEX_BIT);

		_deviceResources->pointLightSourceDescriptorLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);

		{
			// create the pipeline layout
			pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

			pipeLayoutInfo.setDescSetLayout(0, _deviceResources->staticSceneLayout);
			pipeLayoutInfo.setDescSetLayout(1, _deviceResources->pointLightSourceDescriptorLayout);
			_deviceResources->pointLightSourcePipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
		}

		// create the swapchain descriptor sets with corresponding buffers
		std::vector<pvrvk::WriteDescriptorSet> descSetUpdate;
		for (uint32_t i = 0; i < _numSwapImages; ++i)
		{
			_deviceResources->pointLightSourceDescriptorSets.add(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->pointLightSourceDescriptorLayout));
			descSetUpdate.push_back(
			  pvrvk::WriteDescriptorSet(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->pointLightSourceDescriptorSets[i], 0)
			  .setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->staticPointLightBuffer, 0, _deviceResources->staticPointLightBufferView.getDynamicSliceSize())));

			descSetUpdate.push_back(
			  pvrvk::WriteDescriptorSet(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->pointLightSourceDescriptorSets[i], 1)
			  .setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->dynamicPointLightBuffer, 0, _deviceResources->dynamicPointLightBufferView.getDynamicSliceSize())));
		}
		_deviceResources->device->updateDescriptorSets(descSetUpdate.data(), descSetUpdate.size(), nullptr, 0);
	}
}

/*!*********************************************************************************************************************
\brief  Creates static scene wide descriptor set.
***********************************************************************************************************************/
void VulkanDeferredShading::createStaticSceneDescriptorSet()
{
	// static per scene buffer
	pvrvk::DescriptorSetLayoutCreateInfo staticSceneDescSetInfo;
	staticSceneDescSetInfo.setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER, 1u, VkShaderStageFlags::e_FRAGMENT_BIT);
	_deviceResources->staticSceneLayout = _deviceResources->device->createDescriptorSetLayout(staticSceneDescSetInfo);

	// Create static descriptor set for the scene

	_deviceResources->sceneDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->staticSceneLayout);
	pvrvk::WriteDescriptorSet descSetUpdate(VkDescriptorType::e_UNIFORM_BUFFER,
	                                        _deviceResources->sceneDescriptorSet, 0);
	descSetUpdate.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->farClipDistanceBuffer, 0, _deviceResources->farClipDistanceBufferView.getDynamicSliceSize()));
	_deviceResources->device->updateDescriptorSets(&descSetUpdate, 1, nullptr, 0);
	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

	pipeLayoutInfo.setDescSetLayout(0, _deviceResources->staticSceneLayout);
	_deviceResources->scenePipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
}

/*!*********************************************************************************************************************
\return Return true if no error occurred
\brief  Loads the textures required for this example and sets up the GBuffer descriptor sets
***********************************************************************************************************************/
bool VulkanDeferredShading::createMaterialsAndDescriptorSets(
  std::vector<pvr::utils::ImageUploadResults>& uploadResults)
{
	if (_mainScene->getNumMaterials() == 0)
	{
		setExitMessage("ERROR: The scene does not contain any materials.");
		return false;
	}
	// CREATE THE SAMPLERS
	// create trilinear sampler
	pvrvk::SamplerCreateInfo samplerDesc;
	samplerDesc.wrapModeU = samplerDesc.wrapModeV =
	                          samplerDesc.wrapModeW = VkSamplerAddressMode::e_REPEAT;

	samplerDesc.minFilter = VkFilter::e_LINEAR;
	samplerDesc.magFilter = VkFilter::e_LINEAR;
	samplerDesc.mipMapMode = VkSamplerMipmapMode::e_LINEAR;
	pvrvk::Sampler samplerTrilinear = _deviceResources->device->createSampler(samplerDesc);

	// CREATE THE DESCRIPTOR SET LAYOUTS
	// Per Model Descriptor set layout
	pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
	// create the ubo descriptor setlayout
	//static material ubo
	descSetInfo.setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, VkShaderStageFlags::e_FRAGMENT_BIT);

	// static model ubo
	descSetInfo.setBinding(1, VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u, VkShaderStageFlags::e_VERTEX_BIT);

	// no texture sampler layout
	_deviceResources->noSamplerLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);

	// Single texture sampler layout
	descSetInfo.setBinding(2, VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, VkShaderStageFlags::e_FRAGMENT_BIT);
	_deviceResources->oneSamplerLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);

	// Two textures sampler layout
	descSetInfo.setBinding(3, VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, VkShaderStageFlags::e_FRAGMENT_BIT);
	_deviceResources->twoSamplerLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);

	// Three textures sampler layout
	descSetInfo.setBinding(4, VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, VkShaderStageFlags::e_FRAGMENT_BIT);
	_deviceResources->threeSamplerLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);

	// Four textures sampler layout (for GBuffer rendering)
	descSetInfo.setBinding(5, VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, VkShaderStageFlags::e_FRAGMENT_BIT);
	_deviceResources->fourSamplerLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);

	// create the pipeline layouts
	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

	pipeLayoutInfo.setDescSetLayout(0, _deviceResources->staticSceneLayout);

	pipeLayoutInfo.setDescSetLayout(1, _deviceResources->noSamplerLayout);
	_deviceResources->pipeLayoutNoSamplers = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	pipeLayoutInfo.setDescSetLayout(1, _deviceResources->oneSamplerLayout);
	_deviceResources->pipeLayoutOneSampler = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	pipeLayoutInfo.setDescSetLayout(1, _deviceResources->twoSamplerLayout);
	_deviceResources->pipeLayoutTwoSamplers = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	pipeLayoutInfo.setDescSetLayout(1, _deviceResources->threeSamplerLayout);
	_deviceResources->pipeLayoutThreeSamplers = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	pipeLayoutInfo.setDescSetLayout(1, _deviceResources->fourSamplerLayout);
	_deviceResources->pipeLayoutFourSamplers = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	// CREATE DESCRIPTOR SETS FOR EACH MATERIAL
	_deviceResources->materials.resize(_mainScene->getNumMaterials());
	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;
	for (uint32_t i = 0; i < _mainScene->getNumMaterials(); ++i)
	{
		_deviceResources->materials[i].materialDescriptorSet.resize(_numSwapImages);
		// get the current material
		const pvr::assets::Model::Material& material = _mainScene->getMaterial(i);
		// get material properties
		_deviceResources->materials[i].specularStrength = material.defaultSemantics().getShininess();
		_deviceResources->materials[i].diffuseColor = material.defaultSemantics().getDiffuse();
		pvrvk::ImageView diffuseMap;
		uint32_t numTextures = 0;
		pvrvk::ImageView bumpMap;
		if (material.defaultSemantics().getDiffuseTextureIndex() != -1)
		{
			// Load the diffuse texture map
			uploadResults.push_back(pvr::utils::loadAndUploadImage(
			                          _deviceResources->device, _mainScene->getTexture(
			                            material.defaultSemantics().getDiffuseTextureIndex()).getName().c_str(),
			                          true, _deviceResources->commandBufferMain[0], *this));

			diffuseMap = uploadResults.back().getImageView();
			if (diffuseMap.isNull())
			{
				_deviceResources->commandBufferMain[0]->end();
				return false;
			}
			++numTextures;
		}
		if (material.defaultSemantics().getBumpMapTextureIndex() != -1)
		{
			// Load the bumpmap
			uploadResults.push_back(pvr::utils::loadAndUploadImage(
			                          _deviceResources->device, _mainScene->getTexture(
			                            material.defaultSemantics().getBumpMapTextureIndex()).getName().c_str(),
			                          true, _deviceResources->commandBufferMain[0], *this));

			bumpMap = uploadResults.back().getImageView();
			if (bumpMap.isNull())
			{
				_deviceResources->commandBufferMain[0]->end();
				return false;
			}
			++numTextures;
		}
		for (uint32_t j = 0; j < _numSwapImages; ++j)
		{
			// based on the number of textures select the correct descriptor set
			switch (numTextures)
			{
			case 0: _deviceResources->materials[i].materialDescriptorSet[j] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->noSamplerLayout);
				break;
			case 1: _deviceResources->materials[i].materialDescriptorSet[j] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->oneSamplerLayout);
				break;
			case 2: _deviceResources->materials[i].materialDescriptorSet[j] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->twoSamplerLayout);
				break;
			case 3: _deviceResources->materials[i].materialDescriptorSet[j] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->threeSamplerLayout);
				break;
			case 4: _deviceResources->materials[i].materialDescriptorSet[j] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->fourSamplerLayout);
				break;
			default:
				break;
			}

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC,
			                        _deviceResources->materials[i].materialDescriptorSet[j], 0).setBufferInfo(0,
			                            pvrvk::DescriptorBufferInfo(_deviceResources->modelMaterialBuffer, 0, _deviceResources->modelMaterialBufferView.getDynamicSliceSize())));

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC,
			                        _deviceResources->materials[i].materialDescriptorSet[j], 1).setBufferInfo(0,
			                            pvrvk::DescriptorBufferInfo(_deviceResources->modelMatrixBuffer, 0, _deviceResources->modelMatrixBufferView.getDynamicSliceSize())));

			if (diffuseMap.isValid())
			{
				writeDescSets.push_back(pvrvk::WriteDescriptorSet(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER,
				                        _deviceResources->materials[i].materialDescriptorSet[j], 2).setImageInfo(
				                          0, pvrvk::DescriptorImageInfo(diffuseMap, samplerTrilinear, VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
			}
			if (bumpMap.isValid())
			{
				writeDescSets.push_back(pvrvk::WriteDescriptorSet(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER,
				                        _deviceResources->materials[i].materialDescriptorSet[j], 3).setImageInfo(0,
				                            pvrvk::DescriptorImageInfo(bumpMap, samplerTrilinear, VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
			}
		}
	}
	_deviceResources->device->updateDescriptorSets(writeDescSets.data(),
	    writeDescSets.size(), nullptr, 0);
	return true;
}

/*!*********************************************************************************************************************
\brief  Creates model pipelines.
***********************************************************************************************************************/
void VulkanDeferredShading::createModelPipelines()
{
	pvrvk::GraphicsPipelineCreateInfo renderGBufferPipelineCreateInfo;
	renderGBufferPipelineCreateInfo.viewport.setViewportAndScissor(0,
	    pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(_deviceResources->swapchain->getDimension().width), static_cast<float>(_deviceResources->swapchain->getDimension().height)),
	    pvrvk::Rect2Di(0, 0, _deviceResources->swapchain->getDimension().width, _deviceResources->swapchain->getDimension().height));
	// enable back face culling
	renderGBufferPipelineCreateInfo.rasterizer.setCullMode(VkCullModeFlags::e_BACK_BIT);

	// set counter clockwise winding order for front faces
	renderGBufferPipelineCreateInfo.rasterizer.setFrontFaceWinding(VkFrontFace::e_COUNTER_CLOCKWISE);

	// enable depth testing
	renderGBufferPipelineCreateInfo.depthStencil.enableDepthTest(true);
	renderGBufferPipelineCreateInfo.depthStencil.enableDepthWrite(true);

	// set the blend state for the color attachments
	pvrvk::PipelineColorBlendAttachmentState renderGBufferColorAttachment;
	// number of color blend states must equal number of color attachments for the subpass
	renderGBufferPipelineCreateInfo.colorBlend.setAttachmentState(0, renderGBufferColorAttachment);
	renderGBufferPipelineCreateInfo.colorBlend.setAttachmentState(1, renderGBufferColorAttachment);
	renderGBufferPipelineCreateInfo.colorBlend.setAttachmentState(2, renderGBufferColorAttachment);

	// load and create appropriate shaders
	renderGBufferPipelineCreateInfo.vertexShader.setShader(
	  _deviceResources->device->createShader(getAssetStream(Files::GBufferVertexShader)->readToEnd<uint32_t>()));

	renderGBufferPipelineCreateInfo.fragmentShader.setShader(
	  _deviceResources->device->createShader(getAssetStream(Files::GBufferFragmentShader)->readToEnd<uint32_t>()));

	// setup vertex inputs
	renderGBufferPipelineCreateInfo.vertexInput.clear();
	pvr::utils::populateInputAssemblyFromMesh(_mainScene->getMesh(MeshNodes::Satyr), vertexBindings, 4,
	    renderGBufferPipelineCreateInfo.vertexInput, renderGBufferPipelineCreateInfo.inputAssembler);

	// renderpass/subpass
	renderGBufferPipelineCreateInfo.renderPass = _deviceResources->onScreenLocalMemoryRenderPass;
	renderGBufferPipelineCreateInfo.subpass = RenderPassSubPasses::GBuffer;

	// enable stencil testing
	pvrvk::StencilOpState stencilState;

	// only replace stencil buffer when the depth test passes
	stencilState.failOp = VkStencilOp::e_KEEP;
	stencilState.depthFailOp = VkStencilOp::e_KEEP;
	stencilState.passOp = VkStencilOp::e_REPLACE;
	stencilState.compareOp = VkCompareOp::e_ALWAYS;

	// set stencil reference to 1
	stencilState.reference = 1;

	// enable stencil writing
	stencilState.writeMask = 0xFF;

	// enable the stencil tests
	renderGBufferPipelineCreateInfo.depthStencil.enableStencilTest(true);
	// set stencil states
	renderGBufferPipelineCreateInfo.depthStencil.setStencilFront(stencilState);
	renderGBufferPipelineCreateInfo.depthStencil.setStencilBack(stencilState);

	renderGBufferPipelineCreateInfo.pipelineLayout = _deviceResources->pipeLayoutTwoSamplers;
	_deviceResources->renderInfo.storeLocalMemoryPass.objects[MeshNodes::Satyr].pipeline =
	  _deviceResources->device->createGraphicsPipeline(renderGBufferPipelineCreateInfo);

	// load and create appropriate shaders
	renderGBufferPipelineCreateInfo.vertexShader.setShader(
	  _deviceResources->device->createShader(getAssetStream(Files::GBufferFloorVertexShader)->readToEnd<uint32_t>()));

	renderGBufferPipelineCreateInfo.fragmentShader.setShader(
	  _deviceResources->device->createShader(getAssetStream(Files::GBufferFloorFragmentShader)->readToEnd<uint32_t>()));

	// setup vertex inputs
	renderGBufferPipelineCreateInfo.vertexInput.clear();
	pvr::utils::populateInputAssemblyFromMesh(_mainScene->getMesh(MeshNodes::Floor),
	    floorVertexBindings, 3, renderGBufferPipelineCreateInfo.vertexInput,
	    renderGBufferPipelineCreateInfo.inputAssembler);

	renderGBufferPipelineCreateInfo.pipelineLayout = _deviceResources->pipeLayoutOneSampler;
	_deviceResources->renderInfo.storeLocalMemoryPass.objects[MeshNodes::Floor].pipeline =
	  _deviceResources->device->createGraphicsPipeline(renderGBufferPipelineCreateInfo);
}

/*!*********************************************************************************************************************
\brief  Creates direcitonal lighting pipeline.
***********************************************************************************************************************/
void VulkanDeferredShading::createDirectionalLightingPipeline()
{
	// DIRECTIONAL LIGHTING - A full-screen quad that will apply any global (ambient/directional) lighting
	// disable the depth write as we do not want to modify the depth buffer while rendering directional lights

	pvrvk::GraphicsPipelineCreateInfo renderDirectionalLightingPipelineInfo;
	renderDirectionalLightingPipelineInfo.viewport.setViewportAndScissor(0,
	    pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(_deviceResources->swapchain->getDimension().width), static_cast<float>(_deviceResources->swapchain->getDimension().height)),
	    pvrvk::Rect2Di(0, 0, _deviceResources->swapchain->getDimension().width, _deviceResources->swapchain->getDimension().height));
	// enable back face culling
	renderDirectionalLightingPipelineInfo.rasterizer.setCullMode(VkCullModeFlags::e_FRONT_BIT);

	// set counter clockwise winding order for front faces
	renderDirectionalLightingPipelineInfo.rasterizer.setFrontFaceWinding(
	  VkFrontFace::e_COUNTER_CLOCKWISE);

	// Make use of the stencil buffer contents to only shade pixels where actual geometry is located.
	pvrvk::StencilOpState stencilState;

	// keep the stencil states the same as the previous pass. These aren't important to this pass.
	stencilState.failOp = VkStencilOp::e_KEEP;
	stencilState.depthFailOp = VkStencilOp::e_KEEP;
	stencilState.passOp = VkStencilOp::e_REPLACE;

	// if the stencil is equal to the value specified then stencil passes
	stencilState.compareOp = VkCompareOp::e_EQUAL;

	// if for the current fragment the stencil has been filled then there is geometry present
	// and directional lighting calculations should be carried out
	stencilState.reference = 1;

	stencilState.writeMask = 0x00;

	// disable depth writing and depth testing
	renderDirectionalLightingPipelineInfo.depthStencil.enableDepthWrite(false);
	renderDirectionalLightingPipelineInfo.depthStencil.enableDepthTest(false);

	// enable stencil testing
	renderDirectionalLightingPipelineInfo.depthStencil.enableStencilTest(true);
	renderDirectionalLightingPipelineInfo.depthStencil.setStencilFront(stencilState);
	renderDirectionalLightingPipelineInfo.depthStencil.setStencilBack(stencilState);

	// set the blend state for the color attachments
	renderDirectionalLightingPipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

	// load and create appropriate shaders
	renderDirectionalLightingPipelineInfo.vertexShader.setShader(
	  _deviceResources->device->createShader(getAssetStream(Files::AttributelessVertexShader)->readToEnd<uint32_t>()));
	renderDirectionalLightingPipelineInfo.fragmentShader.setShader(
	  _deviceResources->device->createShader(getAssetStream(Files::DirectionalLightingFragmentShader)->readToEnd<uint32_t>()));

	// setup vertex inputs
	renderDirectionalLightingPipelineInfo.vertexInput.clear();
	renderDirectionalLightingPipelineInfo.inputAssembler.setPrimitiveTopology(VkPrimitiveTopology::e_TRIANGLE_STRIP);

	renderDirectionalLightingPipelineInfo.pipelineLayout = _deviceResources->directionalLightingPipelineLayout;

	// renderpass/subpass
	renderDirectionalLightingPipelineInfo.renderPass = _deviceResources->onScreenLocalMemoryRenderPass;
	renderDirectionalLightingPipelineInfo.subpass = RenderPassSubPasses::Lighting;

	_deviceResources->renderInfo.directionalLightPass.pipeline = _deviceResources->device->createGraphicsPipeline(renderDirectionalLightingPipelineInfo);
}

/*!*********************************************************************************************************************
\brief  Creates point lighting stencil pass pipeline.
***********************************************************************************************************************/
void VulkanDeferredShading::createPointLightStencilPipeline()
{
	// POINT LIGHTS GEOMETRY STENCIL PASS
	// Render the front face of each light volume
	// Z function is set as Less/Equal
	// Z test passes will leave the stencil as 0 i.e. the front of the light is infront of all geometry in the current pixel
	//    This is the condition we want for determining whether the geometry can be affected by the point lights
	// Z test fails will increment the stencil to 1. i.e. the front of the light is behind all of the geometry in the current pixel
	//    Under this condition the current pixel cannot be affected by the current point light as the geometry is infront of the front of the point light
	pvrvk::GraphicsPipelineCreateInfo pointLightStencilPipelineCreateInfo;
	pointLightStencilPipelineCreateInfo.viewport.setViewportAndScissor(0,
	    pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(_deviceResources->swapchain->getDimension().width), static_cast<float>(_deviceResources->swapchain->getDimension().height)),
	    pvrvk::Rect2Di(0, 0, _deviceResources->swapchain->getDimension().width, _deviceResources->swapchain->getDimension().height));
	pvrvk::PipelineColorBlendAttachmentState stencilPassColorAttachmentBlendState;
	stencilPassColorAttachmentBlendState.colorWriteMask = static_cast<VkColorComponentFlags>(0);

	// set the blend state for the color attachments
	pointLightStencilPipelineCreateInfo.colorBlend.setAttachmentState(0,
	    stencilPassColorAttachmentBlendState);

	// enable back face culling
	pointLightStencilPipelineCreateInfo.rasterizer.setCullMode(VkCullModeFlags::e_BACK_BIT);

	// set counter clockwise winding order for front faces
	pointLightStencilPipelineCreateInfo.rasterizer.setFrontFaceWinding(
	  VkFrontFace::e_COUNTER_CLOCKWISE);

	// disable depth write. This pass reuses previously written depth buffer
	pointLightStencilPipelineCreateInfo.depthStencil.enableDepthTest(true);
	pointLightStencilPipelineCreateInfo.depthStencil.enableDepthWrite(false);

	// set depth comparison to less/equal
	pointLightStencilPipelineCreateInfo.depthStencil
	.setDepthCompareFunc(VkCompareOp::e_LESS_OR_EQUAL).enableStencilTest(true);

	// load and create appropriate shaders
	pointLightStencilPipelineCreateInfo.vertexShader.setShader(
	  _deviceResources->device->createShader(getAssetStream(Files::PointLightPass1VertexShader)->readToEnd<uint32_t>()));
	pointLightStencilPipelineCreateInfo.fragmentShader.setShader(
	  _deviceResources->device->createShader(getAssetStream(Files::PointLightPass1FragmentShader)->readToEnd<uint32_t>()));

	// setup vertex inputs
	pointLightStencilPipelineCreateInfo.vertexInput.clear();
	pvr::utils::populateInputAssemblyFromMesh(
	  _pointLightModel->getMesh(LightNodes::PointLightMeshNode),
	  pointLightVertexBindings, 1, pointLightStencilPipelineCreateInfo.vertexInput,
	  pointLightStencilPipelineCreateInfo.inputAssembler);

	pvrvk::StencilOpState stencilState;
	stencilState.compareOp = VkCompareOp::e_ALWAYS;
	// keep current value if the stencil test fails
	stencilState.failOp = VkStencilOp::e_KEEP;
	// if the depth test fails then increment wrap
	stencilState.depthFailOp = VkStencilOp::e_INCREMENT_AND_WRAP;
	stencilState.passOp = VkStencilOp::e_KEEP;

	stencilState.reference = 0;

	// set stencil state for the front face of the light sources
	pointLightStencilPipelineCreateInfo.depthStencil.setStencilFront(stencilState);

	// set stencil state for the back face of the light sources
	stencilState.depthFailOp = VkStencilOp::e_KEEP;
	pointLightStencilPipelineCreateInfo.depthStencil.setStencilBack(stencilState);

	// renderpass/subpass
	pointLightStencilPipelineCreateInfo.renderPass = _deviceResources->onScreenLocalMemoryRenderPass;
	pointLightStencilPipelineCreateInfo.subpass = RenderPassSubPasses::Lighting;

	pointLightStencilPipelineCreateInfo.pipelineLayout = _deviceResources->pointLightGeometryStencilPipelineLayout;

	_deviceResources->renderInfo.pointLightGeometryStencilPass.pipeline =
	  _deviceResources->device->createGraphicsPipeline(pointLightStencilPipelineCreateInfo);
}

/*!*********************************************************************************************************************
\brief  Creates point lighting proxy pass pipeline.
***********************************************************************************************************************/
void VulkanDeferredShading::createPointLightProxyPipeline()
{
	// POINT LIGHTS PROXIES - Actually light the pixels touched by a point light.
	// Render the back faces of the light volumes
	// Z function is set as Greater/Equal
	// Z test passes signify that there is geometry infront of the back face of the light volume i.e. for the current pixel there is
	// some geometry infront of the back face of the light volume
	// Stencil function is Equal i.e. the stencil renference is set to 0
	// Stencil passes signify that for the current pixel there exists a front face of a light volume infront of the current geometry
	// Point light calculations occur every time a pixel passes both the stencil AND Z test
	pvrvk::GraphicsPipelineCreateInfo pointLightProxyPipelineCreateInfo;
	pointLightProxyPipelineCreateInfo.viewport.setViewportAndScissor(0,
	    pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(_deviceResources->swapchain->getDimension().width), static_cast<float>(_deviceResources->swapchain->getDimension().height)),
	    pvrvk::Rect2Di(0, 0, _deviceResources->swapchain->getDimension().width, _deviceResources->swapchain->getDimension().height));

	// enable front face culling - cull the front faces of the light sources
	pointLightProxyPipelineCreateInfo.rasterizer.setCullMode(VkCullModeFlags::e_FRONT_BIT);

	// set counter clockwise winding order for front faces
	pointLightProxyPipelineCreateInfo.rasterizer.setFrontFaceWinding(
	  VkFrontFace::e_COUNTER_CLOCKWISE);

	// enable stencil testing
	pointLightProxyPipelineCreateInfo.depthStencil.enableStencilTest(true);

	// enable depth testing
	pointLightProxyPipelineCreateInfo.depthStencil.enableDepthTest(true);
	pointLightProxyPipelineCreateInfo.depthStencil.setDepthCompareFunc(
	  VkCompareOp::e_GREATER_OR_EQUAL);
	// disable depth writes
	pointLightProxyPipelineCreateInfo.depthStencil.enableDepthWrite(false);

	// enable blending
	// blend lighting on top of existing directional lighting
	pvrvk::PipelineColorBlendAttachmentState blendConfig;
	blendConfig.blendEnable = true;
	blendConfig.srcColorBlendFactor = VkBlendFactor::e_ONE;
	blendConfig.srcAlphaBlendFactor = VkBlendFactor::e_ONE;
	blendConfig.dstColorBlendFactor = VkBlendFactor::e_ONE;
	blendConfig.dstAlphaBlendFactor = VkBlendFactor::e_ONE;
	pointLightProxyPipelineCreateInfo.colorBlend.setAttachmentState(0, blendConfig);

	// load and create appropriate shaders
	pointLightProxyPipelineCreateInfo.vertexShader.setShader(
	  _deviceResources->device->createShader(getAssetStream(Files::PointLightPass2VertexShader)->readToEnd<uint32_t>()));

	pointLightProxyPipelineCreateInfo.fragmentShader.setShader(
	  _deviceResources->device->createShader(getAssetStream(Files::PointLightPass2FragmentShader)->readToEnd<uint32_t>()));

	// setup vertex states
	pointLightProxyPipelineCreateInfo.vertexInput.clear();
	pvr::utils::populateInputAssemblyFromMesh(_pointLightModel->getMesh(LightNodes::PointLightMeshNode), pointLightVertexBindings, 1,
	    pointLightProxyPipelineCreateInfo.vertexInput, pointLightProxyPipelineCreateInfo.inputAssembler);

	// if stencil state equals 0 then the lighting should take place as there is geometry inside the point lights area
	pvrvk::StencilOpState stencilState;
	stencilState.compareOp = VkCompareOp::e_ALWAYS;
	stencilState.reference = 0;

	pointLightProxyPipelineCreateInfo.depthStencil.setStencilFront(stencilState);
	pointLightProxyPipelineCreateInfo.depthStencil.setStencilBack(stencilState);

	// renderpass/subpass
	pointLightProxyPipelineCreateInfo.renderPass = _deviceResources->onScreenLocalMemoryRenderPass;
	pointLightProxyPipelineCreateInfo.subpass = RenderPassSubPasses::Lighting;

	pointLightProxyPipelineCreateInfo.pipelineLayout = _deviceResources->pointLightProxyPipelineLayout;

	_deviceResources->renderInfo.pointLightProxyPass.pipeline = _deviceResources->device->createGraphicsPipeline(pointLightProxyPipelineCreateInfo);
}

/*!*********************************************************************************************************************
\brief  Creates point lighting source pass pipeline.
***********************************************************************************************************************/
void VulkanDeferredShading::createPointLightSourcePipeline()
{
	// LIGHT SOURCES : Rendering the "will-o-wisps" that are the sources of the light
	pvrvk::GraphicsPipelineCreateInfo pointLightSourcePipelineCreateInfo;
	pointLightSourcePipelineCreateInfo.viewport.setViewportAndScissor(0,
	    pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(_deviceResources->swapchain->getDimension().width), static_cast<float>(_deviceResources->swapchain->getDimension().height)),
	    pvrvk::Rect2Di(0, 0, _deviceResources->swapchain->getDimension().width, _deviceResources->swapchain->getDimension().height));
	// enable back face culling
	pointLightSourcePipelineCreateInfo.rasterizer.setCullMode(VkCullModeFlags::e_BACK_BIT);

	// set counter clockwise winding order for front faces
	pointLightSourcePipelineCreateInfo.rasterizer.setFrontFaceWinding(VkFrontFace::e_COUNTER_CLOCKWISE);

	// disable stencil testing
	pointLightSourcePipelineCreateInfo.depthStencil.enableStencilTest(false);

	// enable depth testing
	pointLightSourcePipelineCreateInfo.depthStencil.enableDepthTest(true);
	pointLightSourcePipelineCreateInfo.depthStencil.setDepthCompareFunc(
	  VkCompareOp::e_LESS_OR_EQUAL);
	pointLightSourcePipelineCreateInfo.depthStencil.enableDepthWrite(true);

	// enable blending
	pvrvk::PipelineColorBlendAttachmentState colorAttachment;
	colorAttachment.blendEnable = true;
	colorAttachment.srcColorBlendFactor = VkBlendFactor::e_ONE;
	colorAttachment.srcAlphaBlendFactor = VkBlendFactor::e_ONE;
	colorAttachment.dstColorBlendFactor = VkBlendFactor::e_ONE;
	colorAttachment.dstAlphaBlendFactor = VkBlendFactor::e_ONE;
	pointLightSourcePipelineCreateInfo.colorBlend.setAttachmentState(0, colorAttachment);

	// load and create appropriate shaders
	pointLightSourcePipelineCreateInfo.vertexShader.setShader(
	  _deviceResources->device->createShader(getAssetStream(Files::PointLightPass3VertexShader)->readToEnd<uint32_t>()));

	pointLightSourcePipelineCreateInfo.fragmentShader.setShader(
	  _deviceResources->device->createShader(getAssetStream(Files::PointLightPass3FragmentShader)->readToEnd<uint32_t>()));

	// setup vertex states
	pointLightSourcePipelineCreateInfo.vertexInput.clear();
	pvr::utils::populateInputAssemblyFromMesh(_pointLightModel->getMesh(LightNodes::PointLightMeshNode), pointLightVertexBindings, 1,
	    pointLightSourcePipelineCreateInfo.vertexInput, pointLightSourcePipelineCreateInfo.inputAssembler);

	// renderpass/subpass
	pointLightSourcePipelineCreateInfo.renderPass = _deviceResources->onScreenLocalMemoryRenderPass;
	pointLightSourcePipelineCreateInfo.subpass = RenderPassSubPasses::Lighting;

	pointLightSourcePipelineCreateInfo.pipelineLayout = _deviceResources->pointLightSourcePipelineLayout;

	_deviceResources->renderInfo.pointLightSourcesPass.pipeline =
	  _deviceResources->device->createGraphicsPipeline(pointLightSourcePipelineCreateInfo);
}

/*!*********************************************************************************************************************
\brief  Create the pipelines for this example
***********************************************************************************************************************/
void VulkanDeferredShading::createPipelines()
{
	createModelPipelines();
	createDirectionalLightingPipeline();
	createPointLightStencilPipeline();
	createPointLightProxyPipeline();
	createPointLightSourcePipeline();
}

/*!*********************************************************************************************************************
\brief  Create the renderpass using local memory for this example
***********************************************************************************************************************/
void VulkanDeferredShading::createFramebufferAndRenderpass()
{
	pvrvk::RenderPassCreateInfo renderPassInfo;

	// On-Screen attachment
	renderPassInfo.setAttachmentDescription(0,
	                                        pvrvk::AttachmentDescription::createColorDescription(_deviceResources->swapchain->getImageFormat(),
	                                            VkImageLayout::e_UNDEFINED, VkImageLayout::e_PRESENT_SRC_KHR,
	                                            VkAttachmentLoadOp::e_CLEAR, VkAttachmentStoreOp::e_STORE, VkSampleCountFlags::e_1_BIT));

	const VkFormat renderpassStorageFormats[FramebufferGBufferAttachments::Count] =
	{
		VkFormat::e_R8G8B8A8_UNORM,     // albedo
		VkFormat::e_R16G16B16A16_SFLOAT,// normal
		VkFormat::e_R32_SFLOAT,         // depth attachment
	};

	renderPassInfo.setAttachmentDescription(1, pvrvk::AttachmentDescription::createColorDescription(
	    renderpassStorageFormats[FramebufferGBufferAttachments::Albedo], VkImageLayout::e_UNDEFINED,
	    VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, VkAttachmentLoadOp::e_CLEAR,
	    VkAttachmentStoreOp::e_DONT_CARE, VkSampleCountFlags::e_1_BIT));

	renderPassInfo.setAttachmentDescription(2, pvrvk::AttachmentDescription::createColorDescription(
	    renderpassStorageFormats[FramebufferGBufferAttachments::Normal], VkImageLayout::e_UNDEFINED,
	    VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, VkAttachmentLoadOp::e_CLEAR,
	    VkAttachmentStoreOp::e_DONT_CARE, VkSampleCountFlags::e_1_BIT));

	renderPassInfo.setAttachmentDescription(3, pvrvk::AttachmentDescription::createColorDescription(
	    renderpassStorageFormats[FramebufferGBufferAttachments::Depth], VkImageLayout::e_UNDEFINED,
	    VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, VkAttachmentLoadOp::e_CLEAR,
	    VkAttachmentStoreOp::e_DONT_CARE, VkSampleCountFlags::e_1_BIT));

	renderPassInfo.setAttachmentDescription(4, pvrvk::AttachmentDescription::createDepthStencilDescription(
	    _deviceResources->depthStencilImages[0]->getImage()->getFormat(),
	    VkImageLayout::e_UNDEFINED, VkImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	    VkAttachmentLoadOp::e_CLEAR, VkAttachmentStoreOp::e_DONT_CARE,
	    VkAttachmentLoadOp::e_CLEAR, VkAttachmentStoreOp::e_DONT_CARE, VkSampleCountFlags::e_1_BIT));

	// Create on-screen-renderpass/framebuffer with its subpasses
	pvrvk::SubPassDescription localMemorySubpasses[RenderPassSubPasses::NumberOfSubpasses];

	// GBuffer subpass
	localMemorySubpasses[RenderPassSubPasses::GBuffer].setColorAttachmentReference(0, pvrvk::AttachmentReference(1, VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	localMemorySubpasses[RenderPassSubPasses::GBuffer].setColorAttachmentReference(1, pvrvk::AttachmentReference(2, VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	localMemorySubpasses[RenderPassSubPasses::GBuffer].setColorAttachmentReference(2, pvrvk::AttachmentReference(3, VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	localMemorySubpasses[RenderPassSubPasses::GBuffer].setDepthStencilAttachmentReference(pvrvk::AttachmentReference(4, VkImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
	localMemorySubpasses[RenderPassSubPasses::GBuffer].setPreserveAttachmentReference(0, 0);

	// Main scene lighting
	localMemorySubpasses[RenderPassSubPasses::Lighting].setInputAttachmentReference(0, pvrvk::AttachmentReference(1, VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
	localMemorySubpasses[RenderPassSubPasses::Lighting].setInputAttachmentReference(1, pvrvk::AttachmentReference(2, VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
	localMemorySubpasses[RenderPassSubPasses::Lighting].setInputAttachmentReference(2, pvrvk::AttachmentReference(3, VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
	localMemorySubpasses[RenderPassSubPasses::Lighting].setDepthStencilAttachmentReference(pvrvk::AttachmentReference(4, VkImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
	localMemorySubpasses[RenderPassSubPasses::Lighting].setColorAttachmentReference(0, pvrvk::AttachmentReference(0, VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));

	// add subpasses to the renderpass
	renderPassInfo.setSubPass(RenderPassSubPasses::GBuffer, localMemorySubpasses[RenderPassSubPasses::GBuffer]);
	renderPassInfo.setSubPass(RenderPassSubPasses::Lighting, localMemorySubpasses[RenderPassSubPasses::Lighting]);

	// add the sub pass depdendency between sub passes
	pvrvk::SubPassDependency subPassDependency;
	subPassDependency.srcStageMask = VkPipelineStageFlags::e_FRAGMENT_SHADER_BIT;
	subPassDependency.dstStageMask = VkPipelineStageFlags::e_FRAGMENT_SHADER_BIT;
	subPassDependency.srcAccessMask = VkAccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT | VkAccessFlags::e_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	subPassDependency.dstAccessMask = VkAccessFlags::e_INPUT_ATTACHMENT_READ_BIT | VkAccessFlags::e_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	subPassDependency.dependencyByRegion = VkDependencyFlags::e_BY_REGION_BIT;

	// GBuffer -> Directional Lighting
	subPassDependency.srcSubPass = RenderPassSubPasses::GBuffer;
	subPassDependency.dstSubPass = RenderPassSubPasses::Lighting;
	renderPassInfo.addSubPassDependency(subPassDependency);

	// Create the renderpass
	_deviceResources->onScreenLocalMemoryRenderPass = _deviceResources->device->createRenderPass(renderPassInfo);

	// create and add the transient framebuffer attachments used as color/input attachments
	const pvrvk::Extent3D& dimension = pvrvk::Extent3D(_deviceResources->swapchain->getDimension(), 1u);
	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		pvrvk::FramebufferCreateInfo onScreenFramebufferCreateInfo;
		onScreenFramebufferCreateInfo.setAttachment(0, _deviceResources->swapchain->getImageView(i));

		// Allocate the render targets
		for (uint32_t currentIndex = 0; currentIndex < FramebufferGBufferAttachments::Count; ++currentIndex)
		{
			pvrvk::Image transientColorAttachmentTexture = pvr::utils::createImage(_deviceResources->device, VkImageType::e_2D, renderpassStorageFormats[currentIndex], dimension,
			    VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT | VkImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT | VkImageUsageFlags::e_INPUT_ATTACHMENT_BIT,
			    static_cast<VkImageCreateFlags>(0), pvrvk::ImageLayersSize(), VkSampleCountFlags::e_1_BIT, VkMemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT);

			_deviceResources->framebufferGbufferImages[currentIndex].add(_deviceResources->device->createImageView(transientColorAttachmentTexture));
			onScreenFramebufferCreateInfo.setAttachment(currentIndex + 1, _deviceResources->framebufferGbufferImages[currentIndex][i]);
		}
		onScreenFramebufferCreateInfo.setAttachment(FramebufferGBufferAttachments::Count + 1u, _deviceResources->depthStencilImages[i]);
		onScreenFramebufferCreateInfo.setDimensions(_deviceResources->swapchain->getDimension());
		onScreenFramebufferCreateInfo.setRenderPass(_deviceResources->onScreenLocalMemoryRenderPass);
		_deviceResources->onScreenLocalMemoryFramebuffer[i] = _deviceResources->device->createFramebuffer(onScreenFramebufferCreateInfo);
		_deviceResources->onScreenFramebufferCreateInfos.add(onScreenFramebufferCreateInfo);
	}
}

/*!*********************************************************************************************************************
\brief  Loads the mesh data required for this example into vertex buffer objects
\return Return true if no error occurred
***********************************************************************************************************************/
bool VulkanDeferredShading::loadVbos()
{
	pvr::utils::appendSingleBuffersFromModel(_deviceResources->device,
	    *_mainScene, _deviceResources->sceneVbos, _deviceResources->sceneIbos);

	pvr::utils::createSingleBuffersFromMesh(_deviceResources->device,
	                                        _pointLightModel->getMesh(LightNodes::PointLightMeshNode),
	                                        _deviceResources->pointLightVbo, _deviceResources->pointLightIbo);

	if (!_deviceResources->sceneVbos.size() || !_deviceResources->sceneIbos.size() ||
	    _deviceResources->pointLightVbo.isNull() || _deviceResources->pointLightIbo.isNull())
	{
		setExitMessage("Invalid Scene Buffers");
		return false;
	}

	return true;
}

/*!*********************************************************************************************************************
\brief  Creates the buffers used for rendering the models
***********************************************************************************************************************/
void VulkanDeferredShading::createModelBuffers()
{
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement(BufferEntryNames::PerModelMaterial::SpecularStrength, pvr::GpuDatatypes::Float);
		desc.addElement(BufferEntryNames::PerModelMaterial::DiffuseColor, pvr::GpuDatatypes::vec3);

		_deviceResources->modelMaterialBufferView.initDynamic(desc, _mainScene->getNumMeshNodes(), pvr::BufferUsageFlags::UniformBuffer,
		    _deviceResources->device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment);

		_deviceResources->modelMaterialBuffer = pvr::utils::createBuffer(_deviceResources->device, _deviceResources->modelMaterialBufferView.getSize(),
		                                        VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT,
		                                        VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);
	}

	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement(BufferEntryNames::PerModel::WorldViewProjectionMatrix, pvr::GpuDatatypes::mat4x4);
		desc.addElement(BufferEntryNames::PerModel::WorldViewMatrix, pvr::GpuDatatypes::mat4x4);
		desc.addElement(BufferEntryNames::PerModel::WorldViewITMatrix, pvr::GpuDatatypes::mat4x4);
		_deviceResources->modelMatrixBufferView.initDynamic(desc, _mainScene->getNumMeshNodes() * _numSwapImages, pvr::BufferUsageFlags::UniformBuffer,
		    _deviceResources->device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment);

		_deviceResources->modelMatrixBuffer = pvr::utils::createBuffer(_deviceResources->device, _deviceResources->modelMatrixBufferView.getSize(),
		                                      VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT,
		                                      VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);
	}
}

/*!*********************************************************************************************************************
\brief  Creates the buffers used for rendering the directional lighting
***********************************************************************************************************************/
void VulkanDeferredShading::createDirectionalLightingBuffers()
{
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement(BufferEntryNames::PerDirectionalLight::LightIntensity, pvr::GpuDatatypes::vec4);

		_deviceResources->staticDirectionalLightBufferView.initDynamic(desc, _numberOfDirectionalLights, pvr::BufferUsageFlags::UniformBuffer,
		    static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment));

		_deviceResources->staticDirectionalLightBuffer = pvr::utils::createBuffer(_deviceResources->device, _deviceResources->staticDirectionalLightBufferView.getSize(),
		    VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);
	}

	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement(BufferEntryNames::PerDirectionalLight::LightViewDirection, pvr::GpuDatatypes::vec4);

		_deviceResources->dynamicDirectionalLightBufferView.initDynamic(desc, _numberOfDirectionalLights * _numSwapImages, pvr::BufferUsageFlags::UniformBuffer,
		    static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment));

		_deviceResources->dynamicDirectionalLightBuffer = pvr::utils::createBuffer(_deviceResources->device, _deviceResources->dynamicDirectionalLightBufferView.getSize(),
		    VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);
	}
}

/*!*********************************************************************************************************************
\brief  Creates the buffers used for rendering the point lighting
***********************************************************************************************************************/
void VulkanDeferredShading::createPointLightBuffers()
{
	// create static point light buffers
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement(BufferEntryNames::PerPointLight::LightIntensity, pvr::GpuDatatypes::Float);
		desc.addElement(BufferEntryNames::PerPointLight::LightRadius, pvr::GpuDatatypes::Float);
		desc.addElement(BufferEntryNames::PerPointLight::LightColor, pvr::GpuDatatypes::vec4);
		desc.addElement(BufferEntryNames::PerPointLight::LightSourceColor, pvr::GpuDatatypes::vec4);

		_deviceResources->staticPointLightBufferView.initDynamic(desc, _numberOfPointLights, pvr::BufferUsageFlags::UniformBuffer,
		    static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment));
		_deviceResources->staticPointLightBuffer = pvr::utils::createBuffer(_deviceResources->device, _deviceResources->staticPointLightBufferView.getSize(),
		    VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);

	}

	// create point light buffers
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement(BufferEntryNames::PerPointLight::WorldViewProjectionMatrix, pvr::GpuDatatypes::mat4x4);
		desc.addElement(BufferEntryNames::PerPointLight::ProxyLightViewPosition, pvr::GpuDatatypes::vec4);
		desc.addElement(BufferEntryNames::PerPointLight::ProxyWorldViewProjectionMatrix, pvr::GpuDatatypes::mat4x4);
		desc.addElement(BufferEntryNames::PerPointLight::ProxyWorldViewMatrix, pvr::GpuDatatypes::mat4x4);

		_deviceResources->dynamicPointLightBufferView.initDynamic(desc, _numberOfPointLights * _numSwapImages, pvr::BufferUsageFlags::UniformBuffer,
		    static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment));
		_deviceResources->dynamicPointLightBuffer = pvr::utils::createBuffer(_deviceResources->device, _deviceResources->dynamicPointLightBufferView.getSize(),
		    VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);
	}
}

/*!*********************************************************************************************************************
\brief  Creates the buffers used for rendering the lighting
***********************************************************************************************************************/
void VulkanDeferredShading::createLightingBuffers()
{
	// directional light sources
	createDirectionalLightingBuffers();

	// point light sources
	createPointLightBuffers();
}

/*!*********************************************************************************************************************
\brief  Creates the scene wide buffer used throughout the demo
***********************************************************************************************************************/
void VulkanDeferredShading::createSceneWideBuffers()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(BufferEntryNames::PerScene::FarClipDistance, pvr::GpuDatatypes::Float);

	_deviceResources->farClipDistanceBufferView.init(desc);
	_deviceResources->farClipDistanceBuffer = pvr::utils::createBuffer(_deviceResources->device, _deviceResources->farClipDistanceBufferView.getSize(),
	    VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);
}

/*!*********************************************************************************************************************
\brief  Creates the buffers used throughout the demo
***********************************************************************************************************************/
void VulkanDeferredShading::createBuffers()
{
	// create scene wide buffer
	createSceneWideBuffers();

	// create model buffers
	createModelBuffers();

	// create lighting buffers
	createLightingBuffers();
}

/*!*********************************************************************************************************************
\brief  Upload the static data to the buffers which do not change per frame
***********************************************************************************************************************/
void VulkanDeferredShading::uploadStaticSceneData()
{
	// static scene properties buffer
	void* memory;
	_farClipDistance = _mainScene->getCamera(0).getFar();
	_deviceResources->farClipDistanceBuffer->getDeviceMemory()->map(&memory);
	_deviceResources->farClipDistanceBufferView.pointToMappedMemory(memory);
	_deviceResources->farClipDistanceBufferView.getElementByName(BufferEntryNames::PerScene::FarClipDistance).setValue(&_farClipDistance);
	_deviceResources->farClipDistanceBuffer->getDeviceMemory()->unmap();
}

/*!*********************************************************************************************************************
\brief  Upload the static data to the buffers which do not change per frame
***********************************************************************************************************************/
void VulkanDeferredShading::uploadStaticModelData()
{
	// static model buffer
	void* memory;
	_deviceResources->modelMaterialBuffer->getDeviceMemory()->map(&memory);
	_deviceResources->modelMaterialBufferView.pointToMappedMemory(memory);
	for (uint32_t i = 0; i < _mainScene->getNumMeshNodes(); ++i)
	{
		_deviceResources->modelMaterialBufferView.getElementByName(BufferEntryNames::PerModelMaterial::SpecularStrength, 0, i).setValue(&_deviceResources->materials[i].specularStrength);
		_deviceResources->modelMaterialBufferView.getElementByName(BufferEntryNames::PerModelMaterial::DiffuseColor, 0, i).setValue(&_deviceResources->materials[i].diffuseColor);
	}
	_deviceResources->modelMaterialBuffer->getDeviceMemory()->unmap();
}

/*!*********************************************************************************************************************
\brief  Upload the static data to the buffers which do not change per frame
***********************************************************************************************************************/
void VulkanDeferredShading::uploadStaticDirectionalLightData()
{
	// static directional lighting buffer
	void* memory;
	_deviceResources->staticDirectionalLightBuffer->getDeviceMemory()->map(&memory);
	_deviceResources->staticDirectionalLightBufferView.pointToMappedMemory(memory);
	for (uint32_t i = 0; i < _numberOfDirectionalLights; ++i)
	{
		_deviceResources->staticDirectionalLightBufferView.getElementByName(BufferEntryNames::PerDirectionalLight::LightIntensity,
		    0, i).setValue(&_deviceResources->renderInfo.directionalLightPass.lightProperties[i].lightIntensity);
	}
	_deviceResources->staticDirectionalLightBuffer->getDeviceMemory()->unmap();
}

/*!*********************************************************************************************************************
\brief  Upload the static data to the buffers which do not change per frame
***********************************************************************************************************************/
void VulkanDeferredShading::uploadStaticPointLightData()
{
	// static point lighting buffer
	void* memory;
	_deviceResources->staticPointLightBuffer->getDeviceMemory()->map(&memory);
	_deviceResources->staticPointLightBufferView.pointToMappedMemory(memory);
	for (uint32_t i = 0; i < _numberOfPointLights; ++i)
	{
		_deviceResources->staticPointLightBufferView.getElementByName(BufferEntryNames::PerPointLight::LightIntensity,
		    0, i).setValue(&_deviceResources->renderInfo.pointLightPasses.lightProperties[i].lightIntensity);
		_deviceResources->staticPointLightBufferView.getElementByName(BufferEntryNames::PerPointLight::LightRadius,
		    0, i).setValue(&_deviceResources->renderInfo.pointLightPasses.lightProperties[i].lightRadius);
		_deviceResources->staticPointLightBufferView.getElementByName(BufferEntryNames::PerPointLight::LightColor,
		    0, i).setValue(&_deviceResources->renderInfo.pointLightPasses.lightProperties[i].lightColor);
		_deviceResources->staticPointLightBufferView.getElementByName(BufferEntryNames::PerPointLight::LightSourceColor,
		    0, i).setValue(&_deviceResources->renderInfo.pointLightPasses.lightProperties[i].lightSourceColor);
	}
	_deviceResources->staticPointLightBuffer->getDeviceMemory()->unmap();
}

/*!*********************************************************************************************************************
\brief  Upload the static data to the buffers which do not change per frame
***********************************************************************************************************************/
void VulkanDeferredShading::uploadStaticData()
{
	uploadStaticSceneData();
	uploadStaticModelData();
	uploadStaticDirectionalLightData();
	uploadStaticPointLightData();
}

/*!*********************************************************************************************************************
\brief  Update the CPU visible buffers containing dynamic data
***********************************************************************************************************************/
void VulkanDeferredShading::updateDynamicSceneData()
{
	RenderData& pass = _deviceResources->renderInfo;

	void* memory;
	uint32_t mappedDynamicSlice = _swapchainIndex * _mainScene->getNumMeshNodes();
	_deviceResources->modelMatrixBuffer->getDeviceMemory()->map(&memory, _deviceResources->modelMatrixBufferView.getDynamicSliceOffset(mappedDynamicSlice),
	    _deviceResources->modelMatrixBufferView.getDynamicSliceSize() * _mainScene->getNumMeshNodes());

	_deviceResources->modelMatrixBufferView.pointToMappedMemory(memory, mappedDynamicSlice);

	// update the model matrices
	for (uint32_t i = 0; i < _mainScene->getNumMeshNodes(); ++i)
	{
		uint32_t dynamicSlice = i + mappedDynamicSlice;

		const pvr::assets::Model::Node& node = _mainScene->getNode(i);
		pass.storeLocalMemoryPass.objects[i].world = _mainScene->getWorldMatrix(node.getObjectId());
		pass.storeLocalMemoryPass.objects[i].worldView = _viewMatrix * pass.storeLocalMemoryPass.objects[i].world;
		pass.storeLocalMemoryPass.objects[i].worldViewProj = _viewProjectionMatrix * pass.storeLocalMemoryPass.objects[i].world;
		pass.storeLocalMemoryPass.objects[i].worldViewIT4x4 = glm::inverseTranspose(pass.storeLocalMemoryPass.objects[i].worldView);

		_deviceResources->modelMatrixBufferView.getElementByName(BufferEntryNames::PerModel::WorldViewMatrix,
		    0, dynamicSlice).setValue(&pass.storeLocalMemoryPass.objects[i].worldView);

		_deviceResources->modelMatrixBufferView.getElementByName(BufferEntryNames::PerModel::WorldViewProjectionMatrix,
		    0, dynamicSlice).setValue(&pass.storeLocalMemoryPass.objects[i].worldViewProj);

		_deviceResources->modelMatrixBufferView.getElementByName(BufferEntryNames::PerModel::WorldViewITMatrix,
		    0, dynamicSlice).setValue(&pass.storeLocalMemoryPass.objects[i].worldViewIT4x4);
	}

	_deviceResources->modelMatrixBuffer->getDeviceMemory()->unmap();

	int32_t pointLight = 0;
	uint32_t directionalLight = 0;

	// update the lighting data
	for (uint32_t i = 0; i < _mainScene->getNumLightNodes(); ++i)
	{
		const pvr::assets::Node& lightNode = _mainScene->getLightNode(i);
		const pvr::assets::Light& light = _mainScene->getLight(lightNode.getObjectId());
		switch (light.getType())
		{
		case pvr::assets::Light::Point:
		{
			if (pointLight >= PointLightConfiguration::MaxScenePointLights) { continue; }

			const glm::mat4& transMtx = _mainScene->getWorldMatrix(_mainScene->getNodeIdFromLightNodeId(i));
			const glm::mat4& proxyScale = glm::scale(glm::vec3(PointLightConfiguration::PointLightScale)) * PointLightConfiguration::PointlightIntensity;
			const glm::mat4 mWorldScale = transMtx * proxyScale;

			//POINT LIGHT GEOMETRY : The spheres that will be used for the stencil pass
			pass.pointLightPasses.lightProperties[pointLight].proxyWorldViewProjectionMatrix = _viewProjectionMatrix * mWorldScale;

			//POINT LIGHT PROXIES : The "drawcalls" that will perform the actual rendering
			pass.pointLightPasses.lightProperties[pointLight].proxyWorldViewMatrix = _viewMatrix * mWorldScale;
			pass.pointLightPasses.lightProperties[pointLight].proxyViewSpaceLightPosition = glm::vec4((_viewMatrix * transMtx)[3]); //Translation component of the view matrix

			//POINT LIGHT SOURCES : The little balls that we render to show the lights
			pass.pointLightPasses.lightProperties[pointLight].worldViewProjectionMatrix = _viewProjectionMatrix * transMtx;
			++pointLight;
		}
		break;
		case pvr::assets::Light::Directional:
		{
			const glm::mat4& transMtx = _mainScene->getWorldMatrix(_mainScene->getNodeIdFromLightNodeId(i));
			pass.directionalLightPass.lightProperties[directionalLight].viewSpaceLightDirection = _viewMatrix * transMtx * glm::vec4(0.f, -1.f, 0.f, 0.f);
			++directionalLight;
		}
		break;
		}
	}
	int numSceneLights = pointLight;
	if (DirectionalLightConfiguration::AdditionalDirectionalLight)
	{
		pass.directionalLightPass.lightProperties[directionalLight].viewSpaceLightDirection = _viewMatrix * glm::vec4(0.f, -1.f, 0.f, 0.f);
		++directionalLight;
	}

	for (; pointLight < numSceneLights + PointLightConfiguration::NumProceduralPointLights; ++pointLight)
	{
		updateProceduralPointLight(pass.pointLightPasses.initialData[pointLight],
		                           _deviceResources->renderInfo.pointLightPasses.lightProperties[pointLight], false);
	}

	{
		// directional Light data
		void* memory;
		uint32_t mappedDynamicSlice = _swapchainIndex * _numberOfDirectionalLights;
		_deviceResources->dynamicDirectionalLightBuffer->getDeviceMemory()->map(&memory, _deviceResources->dynamicDirectionalLightBufferView.getDynamicSliceOffset(mappedDynamicSlice),
		    _deviceResources->dynamicDirectionalLightBufferView.getDynamicSliceSize() * _numberOfDirectionalLights);

		_deviceResources->dynamicDirectionalLightBufferView.pointToMappedMemory(memory, mappedDynamicSlice);

		for (uint32_t i = 0; i < _numberOfDirectionalLights; ++i)
		{
			uint32_t dynamicSlice = i + mappedDynamicSlice;
			_deviceResources->dynamicDirectionalLightBufferView.getElementByName(BufferEntryNames::PerDirectionalLight::LightViewDirection, 0,
			    dynamicSlice).setValue(&_deviceResources->renderInfo.directionalLightPass.lightProperties[i].viewSpaceLightDirection);
		}
		_deviceResources->dynamicDirectionalLightBuffer->getDeviceMemory()->unmap();
	}

	{
		// dynamic point light data
		void* memory;
		uint32_t mappedDynamicSlice = _swapchainIndex * _numberOfPointLights;
		_deviceResources->dynamicPointLightBuffer->getDeviceMemory()->map(&memory, _deviceResources->dynamicPointLightBufferView.getDynamicSliceOffset(mappedDynamicSlice),
		    _deviceResources->dynamicPointLightBufferView.getDynamicSliceSize() * _numberOfPointLights);
		_deviceResources->dynamicPointLightBufferView.pointToMappedMemory(memory, mappedDynamicSlice);

		for (uint32_t i = 0; i < _numberOfPointLights; ++i)
		{
			uint32_t dynamicSlice = i + mappedDynamicSlice;
			_deviceResources->dynamicPointLightBufferView.getElementByName(BufferEntryNames::PerPointLight::ProxyWorldViewProjectionMatrix,
			    0, dynamicSlice).setValue(&_deviceResources->renderInfo.pointLightPasses.lightProperties[i].proxyWorldViewProjectionMatrix);

			_deviceResources->dynamicPointLightBufferView.getElementByName(BufferEntryNames::PerPointLight::ProxyWorldViewMatrix,
			    0, dynamicSlice).setValue(&_deviceResources->renderInfo.pointLightPasses.lightProperties[i].proxyWorldViewMatrix);

			_deviceResources->dynamicPointLightBufferView.getElementByName(BufferEntryNames::PerPointLight::ProxyLightViewPosition,
			    0, dynamicSlice).setValue(&_deviceResources->renderInfo.pointLightPasses.lightProperties[i].proxyViewSpaceLightPosition);

			_deviceResources->dynamicPointLightBufferView.getElementByName(BufferEntryNames::PerPointLight::WorldViewProjectionMatrix,
			    0, dynamicSlice).setValue(&_deviceResources->renderInfo.pointLightPasses.lightProperties[i].worldViewProjectionMatrix);
		}
		_deviceResources->dynamicPointLightBuffer->getDeviceMemory()->unmap();
	}
}

/*!*********************************************************************************************************************
\brief  Update the procedural point lights
***********************************************************************************************************************/
void VulkanDeferredShading::updateProceduralPointLight(
  PointLightPasses::InitialData& data,
  PointLightPasses::PointLightProperties& pointLightProperties, bool initial)
{
	if (initial)
	{
		data.distance = pvr::randomrange(PointLightConfiguration::LightMinDistance,
		                                 PointLightConfiguration::LightMaxDistance);
		data.angle = pvr::randomrange(-glm::pi<float>(), glm::pi<float>());
		data.height = pvr::randomrange(PointLightConfiguration::LightMinHeight,
		                               PointLightConfiguration::LightMaxHeight);
		data.axial_vel = pvr::randomrange(-PointLightConfiguration::LightMaxAxialVelocity,
		                                  PointLightConfiguration::LightMaxAxialVelocity);
		data.radial_vel = pvr::randomrange(-PointLightConfiguration::LightMaxRadialVelocity,
		                                   PointLightConfiguration::LightMaxRadialVelocity);
		data.vertical_vel = pvr::randomrange(-PointLightConfiguration::LightMaxVerticalVelocity,
		                                     PointLightConfiguration::LightMaxVerticalVelocity);

		glm::vec3 lightColor = glm::vec3(pvr::randomrange(0, 1),
		                                 pvr::randomrange(0, 1), pvr::randomrange(0, 1));

		pointLightProperties.lightColor = glm::vec4(lightColor, 1.);//random-looking
		pointLightProperties.lightSourceColor = glm::vec4(lightColor, .8);//random-looking
		pointLightProperties.lightIntensity = PointLightConfiguration::PointlightIntensity;
		pointLightProperties.lightRadius = PointLightConfiguration::PointLightRadius;
	}

	if (!initial && !_isPaused) //Skip for the first _frameNumber, as sometimes this moves the light too far...
	{
		float dt = (float)std::min(getFrameTime(), uint64_t(30));
		if (data.distance < PointLightConfiguration::LightMinDistance)
		{
			data.axial_vel = glm::abs(data.axial_vel) +
			                 (PointLightConfiguration::LightMaxAxialVelocity * dt * .001f);
		}
		if (data.distance > PointLightConfiguration::LightMaxDistance)
		{
			data.axial_vel = -glm::abs(data.axial_vel) -
			                 (PointLightConfiguration::LightMaxAxialVelocity * dt * .001f);
		}
		if (data.height < PointLightConfiguration::LightMinHeight)
		{
			data.vertical_vel = glm::abs(data.vertical_vel) +
			                    (PointLightConfiguration::LightMaxAxialVelocity * dt * .001f);
		}
		if (data.height > PointLightConfiguration::LightMaxHeight)
		{
			data.vertical_vel = -glm::abs(data.vertical_vel) -
			                    (PointLightConfiguration::LightMaxAxialVelocity * dt * .001f);
		}

		data.axial_vel += pvr::randomrange(-PointLightConfiguration::LightAxialVelocityChange,
		                                   PointLightConfiguration::LightAxialVelocityChange) * dt;

		data.radial_vel += pvr::randomrange(-PointLightConfiguration::LightRadialVelocityChange,
		                                    PointLightConfiguration::LightRadialVelocityChange) * dt;

		data.vertical_vel += pvr::randomrange(-PointLightConfiguration::LightVerticalVelocityChange,
		                                      PointLightConfiguration::LightVerticalVelocityChange) * dt;

		if (glm::abs(data.axial_vel) > PointLightConfiguration::LightMaxAxialVelocity)
		{
			data.axial_vel *= .8;
		}
		if (glm::abs(data.radial_vel) > PointLightConfiguration::LightMaxRadialVelocity)
		{
			data.radial_vel *= .8;
		}
		if (glm::abs(data.vertical_vel) > PointLightConfiguration::LightMaxVerticalVelocity)
		{
			data.vertical_vel *= .8;
		}

		data.distance += data.axial_vel * dt * 0.001f;
		data.angle += data.radial_vel * dt * 0.001f;
		data.height += data.vertical_vel * dt * 0.001f;
	}

	float x = sin(data.angle) * data.distance;
	float z = cos(data.angle) * data.distance;
	float y = data.height;


	const glm::mat4& transMtx = glm::translate(glm::vec3(x, y, z));
	const glm::mat4& proxyScale = glm::scale(glm::vec3(PointLightConfiguration::PointLightScale)) *
	                              PointLightConfiguration::PointlightIntensity;

	const glm::mat4 mWorldScale = transMtx * proxyScale;


	//POINT LIGHT GEOMETRY : The spheres that will be used for the stencil pass
	pointLightProperties.proxyWorldViewProjectionMatrix = _viewProjectionMatrix * mWorldScale;

	//POINT LIGHT PROXIES : The "drawcalls" that will perform the actual rendering
	pointLightProperties.proxyWorldViewMatrix = _viewMatrix * mWorldScale;
	pointLightProperties.proxyViewSpaceLightPosition = glm::vec4((_viewMatrix * transMtx)[3]); //Translation component of the view matrix

	//POINT LIGHT SOURCES : The little balls that we render to show the lights
	pointLightProperties.worldViewProjectionMatrix = _viewProjectionMatrix * transMtx;
}

/*!*********************************************************************************************************************
\brief  Updates animation variables and camera matrices.
***********************************************************************************************************************/
void VulkanDeferredShading::updateAnimation()
{
	uint64_t deltaTime = getFrameTime();

	if (!_isPaused)
	{
		_frameNumber += deltaTime * ApplicationConfiguration::FrameRate;
		if (_frameNumber > _mainScene->getNumFrames() - 1) { _frameNumber = 0; }
		_mainScene->setCurrentFrame(_frameNumber);
	}

	glm::vec3 vTo, vUp;
	float fov;
	_mainScene->getCameraProperties(_cameraId, fov, _cameraPosition, vTo, vUp);

	// Update camera matrices
	static float angle = 0;
	if (_animateCamera) { angle += getFrameTime() / 1000.f; }
	_viewMatrix = glm::lookAt(glm::vec3(sin(angle) * 100.f + vTo.x, vTo.y + 30.,
	                                    cos(angle) * 100.f + vTo.z), vTo, vUp);
	_viewProjectionMatrix = _projectionMatrix * _viewMatrix;
	_inverseViewMatrix = glm::inverse(_viewMatrix);
}

/*!*********************************************************************************************************************
\brief  Records main command buffer
***********************************************************************************************************************/
void VulkanDeferredShading::recordMainCommandBuffer()
{
	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		_deviceResources->commandBufferMain[i]->begin();

		pvrvk::Rect2Di renderArea(0, 0, _windowWidth, _windowHeight);

		// specify a clear color per attachment
		const uint32_t numClearValues = FramebufferGBufferAttachments::Count + 1u + 1u;

		pvrvk::ClearValue clearValues[numClearValues] =
		{
			pvrvk::ClearValue(0.0, 0.0, 0.0, 1.0f),
			pvrvk::ClearValue(0.0, 0.0, 0.0, 1.0f),
			pvrvk::ClearValue(0.0, 0.0, 0.0, 1.0f),
			pvrvk::ClearValue(0.0, 0.0, 0.0, 1.0f),
			pvrvk::ClearValue(1.f, 0u)
		};

		// begin the local memory renderpass
		_deviceResources->commandBufferMain[i]->beginRenderPass(_deviceResources->onScreenLocalMemoryFramebuffer[i], renderArea, false, clearValues, numClearValues);

		// Render the models
		_deviceResources->commandBufferMain[i]->executeCommands(_deviceResources->commandBufferRenderToLocalMemory[i]);

		// Render lighting + ui render text
		_deviceResources->commandBufferMain[i]->nextSubpass(VkSubpassContents::e_SECONDARY_COMMAND_BUFFERS);
		_deviceResources->commandBufferMain[i]->executeCommands(_deviceResources->commandBufferLighting[i]);

		_deviceResources->commandBufferMain[i]->endRenderPass();
		_deviceResources->commandBufferMain[i]->end();
	}
}

/*!*********************************************************************************************************************
\brief  Initialise the static light properties
***********************************************************************************************************************/
void VulkanDeferredShading::initialiseStaticLightProperties()
{
	RenderData& pass = _deviceResources->renderInfo;

	int32_t pointLight = 0;
	uint32_t directionalLight = 0;
	for (uint32_t i = 0; i < _mainScene->getNumLightNodes(); ++i)
	{
		const pvr::assets::Node& lightNode = _mainScene->getLightNode(i);
		const pvr::assets::Light& light = _mainScene->getLight(lightNode.getObjectId());
		switch (light.getType())
		{
		case pvr::assets::Light::Point:
		{
			if (pointLight >= PointLightConfiguration::MaxScenePointLights)
			{
				continue;
			}

			const glm::mat4& transMtx = _mainScene->getWorldMatrix(_mainScene->getNodeIdFromLightNodeId(i));
			const glm::mat4& proxyScale = glm::scale(glm::vec3(PointLightConfiguration::PointLightScale)) *
			                              PointLightConfiguration::PointlightIntensity;
			const glm::mat4 mWorldScale = transMtx * proxyScale;

			//POINT LIGHT GEOMETRY : The spheres that will be used for the stencil pass
			pass.pointLightPasses.lightProperties[pointLight].lightColor = glm::vec4(light.getColor(), 1.f);

			//POINT LIGHT PROXIES : The "drawcalls" that will perform the actual rendering
			pass.pointLightPasses.lightProperties[pointLight].lightIntensity =
			  PointLightConfiguration::PointlightIntensity;

			//POINT LIGHT PROXIES : The "drawcalls" that will perform the actual rendering
			pass.pointLightPasses.lightProperties[pointLight].lightRadius = PointLightConfiguration::PointLightRadius;

			//POINT LIGHT SOURCES : The little balls that we render to show the lights
			pass.pointLightPasses.lightProperties[pointLight].lightSourceColor = glm::vec4(light.getColor(), .8f);
			++pointLight;
		}
		break;
		case pvr::assets::Light::Directional:
		{
			const glm::mat4& transMtx = _mainScene->getWorldMatrix(_mainScene->getNodeIdFromLightNodeId(i));
			pass.directionalLightPass.lightProperties[directionalLight].lightIntensity = glm::vec4(light.getColor(), 1.0f) *
			    DirectionalLightConfiguration::DirectionalLightIntensity;
			++directionalLight;
		}
		break;
		}
	}
	int numSceneLights = pointLight;
	if (DirectionalLightConfiguration::AdditionalDirectionalLight)
	{
		pass.directionalLightPass.lightProperties[directionalLight].lightIntensity = glm::vec4(1, 1, 1, 1) *
		    DirectionalLightConfiguration::DirectionalLightIntensity;
		++directionalLight;
	}
}

/*!*********************************************************************************************************************
\brief Allocate memory for lighting data
***********************************************************************************************************************/
void VulkanDeferredShading::allocateLights()
{
	int32_t countPoint = 0;
	uint32_t countDirectional = 0;
	for (uint32_t i = 0; i < _mainScene->getNumLightNodes(); ++i)
	{
		switch (_mainScene->getLight(_mainScene->getLightNode(i).getObjectId()).getType())
		{
		case
				pvr::assets::Light::Directional:
			++countDirectional;
			break;
		case
				pvr::assets::Light::Point:
			++countPoint;
			break;
		default:
			break;
		}
	}

	if (DirectionalLightConfiguration::AdditionalDirectionalLight)
	{
		++countDirectional;
	}

	if (countPoint >= PointLightConfiguration::MaxScenePointLights)
	{
		countPoint = PointLightConfiguration::MaxScenePointLights;
	}

	countPoint += PointLightConfiguration::NumProceduralPointLights;

	_numberOfPointLights = countPoint;
	_numberOfDirectionalLights = countDirectional;

	_deviceResources->renderInfo.directionalLightPass.lightProperties.resize(countDirectional);
	_deviceResources->renderInfo.pointLightPasses.lightProperties.resize(countPoint);
	_deviceResources->renderInfo.pointLightPasses.initialData.resize(countPoint);

	for (int i = countPoint - PointLightConfiguration::NumProceduralPointLights; i < countPoint; ++i)
	{
		updateProceduralPointLight(_deviceResources->renderInfo.pointLightPasses.initialData[i], _deviceResources->renderInfo.pointLightPasses.lightProperties[i], true);
	}
}

/*!*********************************************************************************************************************
\brief  Record all the secondary command buffers
***********************************************************************************************************************/
void VulkanDeferredShading::recordSecondaryCommandBuffers()
{
	pvrvk::Rect2Di renderArea(0, 0, _framebufferWidth, _framebufferHeight);
	if ((_framebufferWidth != _windowWidth) || (_framebufferHeight != _windowHeight))
	{
		renderArea = pvrvk::Rect2Di(_viewportOffsets[0], _viewportOffsets[1], _framebufferWidth, _framebufferHeight);
	}

	pvrvk::ClearValue clearStenciLValue(pvrvk::ClearValue::createStencilClearValue(0));

	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		_deviceResources->commandBufferRenderToLocalMemory[i]->begin(_deviceResources->onScreenLocalMemoryFramebuffer[i], RenderPassSubPasses::GBuffer);
		recordCommandBufferRenderGBuffer(_deviceResources->commandBufferRenderToLocalMemory[i], i, RenderPassSubPasses::GBuffer);
		_deviceResources->commandBufferRenderToLocalMemory[i]->end();

		_deviceResources->commandBufferLighting[i]->begin(_deviceResources->onScreenLocalMemoryFramebuffer[i], RenderPassSubPasses::Lighting);
		recordCommandsDirectionalLights(_deviceResources->commandBufferLighting[i], i, RenderPassSubPasses::Lighting);

		_deviceResources->commandBufferLighting[i]->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
		    _deviceResources->scenePipelineLayout, 0u, _deviceResources->sceneDescriptorSet);

		const pvr::assets::Mesh& pointLightMesh = _pointLightModel->getMesh(LightNodes::PointLightMeshNode);

		// Bind the vertex and index buffer for the point light
		_deviceResources->commandBufferLighting[i]->bindVertexBuffer(_deviceResources->pointLightVbo, 0, 0);
		_deviceResources->commandBufferLighting[i]->bindIndexBuffer(_deviceResources->pointLightIbo, 0, pvr::utils::convertToVk(pointLightMesh.getFaces().getDataType()));

		for (uint32_t j = 0; j < _numberOfPointLights; j++)
		{
			// clear stencil to 0's to make use of it again for point lights
			_deviceResources->commandBufferLighting[i]->clearAttachment(pvrvk::ClearAttachment(VkImageAspectFlags::e_STENCIL_BIT,
			    FramebufferGBufferAttachments::Count + 1u, clearStenciLValue), renderArea);

			recordCommandsPointLightGeometryStencil(_deviceResources->commandBufferLighting[i], i, RenderPassSubPasses::Lighting, j, pointLightMesh);
			recordCommandsPointLightProxy(_deviceResources->commandBufferLighting[i], i, RenderPassSubPasses::Lighting, j, pointLightMesh);
		}
		recordCommandsPointLightSourceLighting(_deviceResources->commandBufferLighting[i], i, RenderPassSubPasses::Lighting);

		recordCommandUIRenderer(_deviceResources->commandBufferLighting[i], i, RenderPassSubPasses::UIRenderer);
		_deviceResources->commandBufferLighting[i]->end();
	}
}

/*!*********************************************************************************************************************
\brief Record rendering G-Buffer commands
\param commandBuffer SecondaryCommandbuffer to record
\param swapChainIndex Current swap chain index
\param subpass Current sub pass
***********************************************************************************************************************/
void VulkanDeferredShading::recordCommandBufferRenderGBuffer(
  pvrvk::SecondaryCommandBuffer& commandBuffer,
  uint32_t swapChainIndex, uint32_t subpass)
{
	DrawGBuffer& pass = _deviceResources->renderInfo.storeLocalMemoryPass;

	commandBuffer->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
	                                 _deviceResources->scenePipelineLayout, 0u, _deviceResources->sceneDescriptorSet);

	for (uint32_t i = 0; i < _mainScene->getNumMeshNodes(); ++i)
	{
		commandBuffer->bindPipeline(pass.objects[i].pipeline);

		const pvr::assets::Model::Node& node = _mainScene->getNode(i);
		const pvr::assets::Mesh& mesh = _mainScene->getMesh(node.getObjectId());

		const Material& material = _deviceResources->materials[node.getMaterialIndex()];

		uint32_t offsets[2];
		offsets[0] = _deviceResources->modelMaterialBufferView.getDynamicSliceOffset(i);
		offsets[1] = _deviceResources->modelMatrixBufferView.getDynamicSliceOffset(i + swapChainIndex * _mainScene->getNumMeshNodes());

		commandBuffer->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
		                                 pass.objects[i].pipeline->getPipelineLayout(), 1u,
		                                 material.materialDescriptorSet[swapChainIndex], offsets, 2u);

		commandBuffer->bindVertexBuffer(_deviceResources->sceneVbos[node.getObjectId()], 0, 0);
		commandBuffer->bindIndexBuffer(_deviceResources->sceneIbos[node.getObjectId()], 0, pvr::utils::convertToVk(mesh.getFaces().getDataType()));
		commandBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
	}

}

/*!*********************************************************************************************************************
\brief  Record UIRenderer commands
\param  commandBuff Commandbuffer to record
***********************************************************************************************************************/
void VulkanDeferredShading::recordCommandUIRenderer(
  pvrvk::SecondaryCommandBuffer& commandBuff,
  uint32_t swapChainIndex, uint32_t subpass)
{
	_deviceResources->uiRenderer.beginRendering(commandBuff);
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.endRendering();
}

/*!*********************************************************************************************************************
\brief  Record directional light draw commands
\param  commandBuffer SecondaryCommandBuffer to record
\param swapChainIndex Current swap chain index
\param subpass Current sub pass
***********************************************************************************************************************/
void VulkanDeferredShading::recordCommandsDirectionalLights(pvrvk::SecondaryCommandBuffer& commandBuffer,
    uint32_t swapChainIndex, uint32_t subpass)
{
	DrawDirectionalLight& directionalPass = _deviceResources->renderInfo.directionalLightPass;

	commandBuffer->bindPipeline(directionalPass.pipeline);

	// keep the descriptor set bound even though for this pass we don't need it
	// avoids unbinding before rebinding in the next passes
	commandBuffer->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
	                                 _deviceResources->scenePipelineLayout, 0u, _deviceResources->sceneDescriptorSet);

	// Make use of the stencil buffer contents to only shade pixels where actual geometry is located.
	// Reset the stencil buffer to 0 at the same time to avoid the stencil clear operation afterwards.
	// bind the albedo and normal textures from the gbuffer
	for (uint32_t i = 0; i < _numberOfDirectionalLights; i++)
	{
		uint32_t offsets[2] = {};
		offsets[0] = _deviceResources->staticDirectionalLightBufferView.getDynamicSliceOffset(i);
		offsets[1] = _deviceResources->dynamicDirectionalLightBufferView.getDynamicSliceOffset(i + swapChainIndex * _numberOfDirectionalLights);

		commandBuffer->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
		                                 directionalPass.pipeline->getPipelineLayout(), 0u,
		                                 _deviceResources->directionalLightingDescriptorSets[swapChainIndex], offsets, 2u);

		// Draw a quad
		commandBuffer->draw(0, 3);
	}
}

/*!*********************************************************************************************************************
\brief  Record point light stencil commands
\param  commandBuffer SecondaryCommandBuffer to record
\param swapChainIndex Current swap chain index
\param subpass Current sub pass
***********************************************************************************************************************/
void VulkanDeferredShading::recordCommandsPointLightGeometryStencil(pvrvk::SecondaryCommandBuffer& commandBuffer,
    uint32_t swapChainIndex, uint32_t subpass, const uint32_t pointLight, const pvr::assets::Mesh& pointLightMesh)
{
	PointLightGeometryStencil& pointGeometryStencilPass = _deviceResources->renderInfo.pointLightGeometryStencilPass;
	PointLightPasses& pointPasses = _deviceResources->renderInfo.pointLightPasses;

	//POINT LIGHTS: 1) Draw stencil to discard useless pixels
	commandBuffer->bindPipeline(pointGeometryStencilPass.pipeline);

	uint32_t offsets[2] = {};
	offsets[0] = _deviceResources->staticPointLightBufferView.getDynamicSliceOffset(pointLight);
	offsets[1] = _deviceResources->dynamicPointLightBufferView.getDynamicSliceOffset(pointLight + swapChainIndex * static_cast<uint32_t>(pointPasses.lightProperties.size()));

	commandBuffer->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS, pointGeometryStencilPass.pipeline->getPipelineLayout(), 1u,
	                                 _deviceResources->pointLightGeometryStencilDescriptorSets[swapChainIndex], offsets, 2u);

	commandBuffer->drawIndexed(0, pointLightMesh.getNumFaces() * 3, 0, 0, 1);
}

/*!*********************************************************************************************************************
\brief  Record point light proxy commands
\param  commandBuffer SecondaryCommandBuffer to record
\param swapChainIndex Current swap chain index
\param subpass Current sub pass
***********************************************************************************************************************/
void VulkanDeferredShading::recordCommandsPointLightProxy(pvrvk::SecondaryCommandBuffer& commandBuffer,
    uint32_t swapChainIndex, uint32_t subpass, const uint32_t pointLight, const pvr::assets::Mesh& pointLightMesh)
{
	DrawPointLightProxy& pointLightProxyPass = _deviceResources->renderInfo.pointLightProxyPass;
	PointLightPasses& pointPasses = _deviceResources->renderInfo.pointLightPasses;

	// Any of the geompointlightpass, lightsourcepointlightpass
	// or pointlightproxiepass's uniforms have the same number of elements
	if (pointPasses.lightProperties.empty()) { return; }

	commandBuffer->bindPipeline(_deviceResources->renderInfo.pointLightProxyPass.pipeline);

	const uint32_t numberOfOffsets = 2;
	uint32_t offsets[numberOfOffsets] = {};
	offsets[0] = _deviceResources->staticPointLightBufferView.getDynamicSliceOffset(pointLight);
	offsets[1] = _deviceResources->dynamicPointLightBufferView.getDynamicSliceOffset(pointLight + swapChainIndex * static_cast<uint32_t>(pointPasses.lightProperties.size()));

	commandBuffer->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
	                                 pointLightProxyPass.pipeline->getPipelineLayout(), 1u,
	                                 _deviceResources->pointLightProxyDescriptorSets[swapChainIndex], offsets, numberOfOffsets);

	commandBuffer->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
	                                 pointLightProxyPass.pipeline->getPipelineLayout(), 2u,
	                                 _deviceResources->pointLightProxyLocalMemoryDescriptorSets[swapChainIndex]);

	commandBuffer->drawIndexed(0, pointLightMesh.getNumFaces() * 3, 0, 0, 1);
}

/*!*********************************************************************************************************************
\brief  Record point light source commands
\param  commandBuffer SecondaryCommandBuffer to record
\param swapChainIndex Current swap chain index
\param subpass Current sub pass
***********************************************************************************************************************/
void VulkanDeferredShading::recordCommandsPointLightSourceLighting(
  pvrvk::SecondaryCommandBuffer& commandBuffer,
  uint32_t swapChainIndex, uint32_t subpass)
{
	DrawPointLightSources& pointLightSourcePass = _deviceResources->renderInfo.pointLightSourcesPass;
	PointLightPasses& pointPasses = _deviceResources->renderInfo.pointLightPasses;

	const pvr::assets::Mesh& mesh = _pointLightModel->getMesh(LightNodes::PointLightMeshNode);

	//POINT LIGHTS: 3) Light sources
	commandBuffer->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
	                                 _deviceResources->scenePipelineLayout, 0u, _deviceResources->sceneDescriptorSet);

	commandBuffer->bindPipeline(pointLightSourcePass.pipeline);
	commandBuffer->bindVertexBuffer(_deviceResources->pointLightVbo, 0, 0);
	commandBuffer->bindIndexBuffer(_deviceResources->pointLightIbo, 0, pvr::utils::convertToVk(mesh.getFaces().getDataType()));

	for (uint32_t i = 0; i < pointPasses.lightProperties.size(); ++i)
	{
		const uint32_t numberOfOffsets = 2u;

		uint32_t offsets[numberOfOffsets] = {};
		offsets[0] = _deviceResources->staticPointLightBufferView.getDynamicSliceOffset(i);
		offsets[1] = _deviceResources->dynamicPointLightBufferView.getDynamicSliceOffset(i + swapChainIndex * static_cast<uint32_t>(pointPasses.lightProperties.size()));

		commandBuffer->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
		                                 pointLightSourcePass.pipeline->getPipelineLayout(), 1u,
		                                 _deviceResources->pointLightSourceDescriptorSets[swapChainIndex], offsets, numberOfOffsets);

		commandBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
	}
}

/*!*********************************************************************************************************************
\return Return an unique_ptr to a new Demo class, supplied by the user
\brief  This function must be implemented by the user of the shell. The user should return its Shell object defining the
behaviour of the application.
***********************************************************************************************************************/
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::unique_ptr<pvr::Shell>(new VulkanDeferredShading()); }
