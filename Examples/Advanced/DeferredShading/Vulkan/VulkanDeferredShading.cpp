/*!*********************************************************************************************************************
\File         VulkanDeferredShading.cpp
\Title        Deferred Shading
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\Description  Implements a deferred shading technique supporting point and directional lights.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVRUIRenderer/PVRUIRenderer.h"

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
namespace Fbo {
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
		glm::vec4 lightIntensity;
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
	pvr::api::GraphicsPipeline pipeline;
};

// structure used to draw the proxy point light
struct DrawPointLightProxy
{
	pvr::api::GraphicsPipeline pipeline;
};

// structure used to fill the stencil buffer used for optimsing the the proxy point light pass
struct PointLightGeometryStencil
{
	pvr::api::GraphicsPipeline pipeline;
};

// structure used to render directional lighting
struct DrawDirectionalLight
{
	pvr::api::GraphicsPipeline pipeline;
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
		pvr::api::GraphicsPipeline pipeline;
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

const char* const GBufferVertexShader = "GBufferVertexShader.vsh.spv";
const char* const GBufferFragmentShader = "GBufferFragmentShader.fsh.spv";

const char* const GBufferFloorVertexShader = "GBufferFloorVertexShader.vsh.spv";
const char* const GBufferFloorFragmentShader = "GBufferFloorFragmentShader.fsh.spv";

const char* const AttributelessVertexShader = "AttributelessVertexShader.vsh.spv";

const char* const DirectionalLightingFragmentShader = "DirectionalLightFragmentShader.fsh.spv";

const char* const PointLightPass1FragmentShader = "PointLightPass1FragmentShader.fsh.spv";
const char* const PointLightPass1VertexShader = "PointLightPass1VertexShader.vsh.spv";

const char* const PointLightPass2FragmentShader = "PointLightPass2FragmentShader.fsh.spv";
const char* const PointLightPass2VertexShader = "PointLightPass2VertexShader.vsh.spv";

const char* const PointLightPass3FragmentShader = "PointLightPass3FragmentShader.fsh.spv";
const char* const PointLightPass3VertexShader = "PointLightPass3VertexShader.vsh.spv";
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
const pvr::float32 FrameRate = 1.0f / 120.0f;
}

// Directional lighting configuration data
namespace DirectionalLightConfiguration {
static bool AdditionalDirectionalLight = true;
const pvr::float32 DirectionalLightIntensity = .2f;
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

static pvr::int32 MaxScenePointLights = 5;
static pvr::int32 NumProceduralPointLights = 10;
pvr::float32 PointLightScale = 40.0f;
pvr::float32 PointlightIntensity = 100.0f;
}

// Subpasses used in the renderpass
namespace RenderPassSubPasses {
const pvr::uint32 GBuffer = 0;
// lighting pass
const pvr::uint32 Lighting = 1;
// UI pass
const pvr::uint32 UIRenderer = 1;

const pvr::uint32 NumberOfSubpasses = 2;
}

/*!*********************************************************************************************************************
Class implementing the Shell functions.
***********************************************************************************************************************/
class VulkanDeferredShading : public pvr::Shell
{
public:
	struct Material
	{
		pvr::api::GraphicsPipeline materialPipeline;
		std::vector<pvr::api::DescriptorSet> materialDescriptorSet;
		pvr::float32 specularStrength;
		glm::vec3 diffuseColor;
	};

	struct DeviceResources
	{
		// Local memory frame buffer
		pvr::Multi<pvr::api::Fbo> onScreenLocalMemoryFbo;
		pvr::Multi<pvr::api::OnScreenFboCreateParam> onScreenFboCreateParams;

		// Stores Texture views for the Images used as attachments on the local memory frame buffer
		pvr::Multi<pvr::api::TextureView> onScreenFboTextureViews[Fbo::Count];

		// Common renderpass used for the demo
		pvr::api::RenderPass onScreenLocalMemoryRenderPass;

		// Vbo and Ibos used for lighting data
		pvr::api::Buffer pointLightVbo;
		pvr::api::Buffer pointLightIbo;


		//// Command Buffers ////
		// Main Primary Command Buffer
		pvr::api::CommandBuffer cmdBufferMain[MAX_NUMBER_OF_SWAP_IMAGES];

		// Secondary commandbuffers used for each pass
		pvr::api::SecondaryCommandBuffer cmdBufferRenderToLocalMemory[MAX_NUMBER_OF_SWAP_IMAGES];
		pvr::api::SecondaryCommandBuffer cmdBufferUpdateDynamicBuffers[MAX_NUMBER_OF_SWAP_IMAGES];
		pvr::api::SecondaryCommandBuffer cmdBufferLighting[MAX_NUMBER_OF_SWAP_IMAGES];

		// Primary command buffer used to upload static data
		pvr::api::CommandBuffer cmdBufferStaticBufferUpload;


		////  Descriptor Set Layouts ////
		// Layouts used for GBuffer rendering
		pvr::api::DescriptorSetLayout staticSceneLayout;
		pvr::api::DescriptorSetLayout noSamplerLayout;
		pvr::api::DescriptorSetLayout oneSamplerLayout;
		pvr::api::DescriptorSetLayout twoSamplerLayout;
		pvr::api::DescriptorSetLayout threeSamplerLayout;
		pvr::api::DescriptorSetLayout fourSamplerLayout;

		// Directional lighting descriptor set layout
		pvr::api::DescriptorSetLayout directionalLightingDescriptorLayout;
		// Point light stencil pass descriptor set layout
		pvr::api::DescriptorSetLayout pointLightGeometryStencilDescriptorLayout;
		// Point Proxy light pass descriptor set layout used for buffers
		pvr::api::DescriptorSetLayout pointLightProxyDescriptorLayout;
		// Point Proxy light pass descriptor set layout used for local memory
		pvr::api::DescriptorSetLayout pointLightProxyLocalMemoryDescriptorLayout;
		// Point light source descriptor set layout used for buffers
		pvr::api::DescriptorSetLayout pointLightSourceDescriptorLayout;


		////  Descriptor Sets ////
		// GBuffer Materials structures
		std::vector<Material>materials;
		// Directional Lighting descriptor set
		pvr::Multi<pvr::api::DescriptorSet> directionalLightingDescriptorSets;
		// Point light stencil descriptor set
		pvr::Multi<pvr::api::DescriptorSet> pointLightGeometryStencilDescriptorSets;
		// Point light Proxy descriptor set
		pvr::Multi<pvr::api::DescriptorSet> pointLightProxyDescriptorSets;
		pvr::Multi<pvr::api::DescriptorSet> pointLightProxyLocalMemoryDescriptorSets;
		// Point light Source descriptor set
		pvr::Multi<pvr::api::DescriptorSet> pointLightSourceDescriptorSets;
		// Scene wide descriptor set
		pvr::api::DescriptorSet sceneDescriptorSet;


		//// Pipeline Layouts ////
		// GBuffer pipeline layouts
		pvr::api::PipelineLayout pipeLayoutNoSamplers;
		pvr::api::PipelineLayout pipeLayoutOneSampler;
		pvr::api::PipelineLayout pipeLayoutTwoSamplers;
		pvr::api::PipelineLayout pipeLayoutThreeSamplers;
		pvr::api::PipelineLayout pipeLayoutFourSamplers;

		// Directional lighting pipeline layout
		pvr::api::PipelineLayout directionalLightingPipelineLayout;
		// Point lighting stencil pipeline layout
		pvr::api::PipelineLayout pointLightGeometryStencilPipelineLayout;
		// Point lighting proxy pipeline layout
		pvr::api::PipelineLayout pointLightProxyPipelineLayout;
		// Point lighting source pipeline layout
		pvr::api::PipelineLayout pointLightSourcePipelineLayout;
		// Scene Wide pipeline layout
		pvr::api::PipelineLayout scenePipelineLayout;

		// scene Vbos and Ibos
		std::vector<pvr::api::Buffer> sceneVbos;
		std::vector<pvr::api::Buffer> sceneIbos;


		//// Structured Memory Views ////
		// scene wide buffers
		pvr::utils::StructuredMemoryView stagingStaticFarClipDistanceUbo;
		pvr::api::BufferView staticFarClipDistanceUbo;
		// Static materials buffers
		pvr::utils::StructuredMemoryView stagingStaticModelMaterialUbo;
		pvr::api::BufferView staticModelMaterialUbo;
		// Dynamic matrices buffers
		pvr::utils::StructuredMemoryView stagingDynamicModelMatrixUbo;
		std::vector<pvr::api::BufferView> dynamicModelMatrixUbo;
		// Static point light buffers
		pvr::utils::StructuredMemoryView stagingStaticPointLightUbo;
		pvr::api::BufferView staticPointLightUbo;
		// Dynamic point light buffer
		pvr::utils::StructuredMemoryView stagingDynamicPointLightUbo;
		std::vector<pvr::api::BufferView> dynamicPointLightUbo;
		// Static Directional lighting buffer
		pvr::utils::StructuredMemoryView stagingStaticDirectionalLightUbo;
		pvr::api::BufferView staticDirectionalLightUbo;
		// Dynamic Directional lighting buffers
		pvr::utils::StructuredMemoryView stagingDynamicDirectionalLightUbo;
		std::vector<pvr::api::BufferView> dynamicDirectionalLightUbo;

		//// UI Renderer ////
		pvr::ui::UIRenderer uiRenderer;


		//// Frame ////
		pvr::uint32 numSwapImages;
		pvr::uint8 swapIndex;

		RenderData renderInfo;
	};

	// Context
	pvr::GraphicsContext m_context;

	//Putting all api objects into a pointer just makes it easier to release them all together with RAII
	std::auto_ptr<DeviceResources> m_deviceResources;

	// Provides easy management of assets
	pvr::api::AssetStore m_assetManager;

	// Frame counters for animation
	pvr::float32 m_frameNumber;
	bool m_isPaused;
	pvr::uint32 m_cameraId;
	bool m_animateCamera;

	pvr::uint32 m_numberOfPointLights;
	pvr::uint32 m_numberOfDirectionalLights;

	// Projection and Model View matrices
	glm::vec3 m_cameraPosition;
	glm::mat4 m_viewMatrix;
	glm::mat4 m_projectionMatrix;
	glm::mat4 m_viewProjectionMatrix;
	glm::mat4 m_inverseViewMatrix;
	pvr::float32 m_farClipDistance;

	pvr::int32 m_windowWidth;
	pvr::int32 m_windowHeight;
	pvr::int32 m_framebufferWidth;
	pvr::int32 m_framebufferHeight;

	pvr::int32 m_viewportOffsets[2];

	// Light models
	pvr::assets::ModelHandle m_pointLightModel;

	// Object model
	pvr::assets::ModelHandle m_mainScene;

	VulkanDeferredShading() { m_animateCamera = false; m_isPaused = false; }

	//  Overriden from pvr::Shell
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void createLocalMemoryRenderPass();
	void createPipelines();
	void createModelPipelines();
	void createDirectionalLightingPipeline();
	void createPointLightStencilPipeline();
	void createPointLightProxyPipeline();
	void createPointLightSourcePipeline();
	void recordCommandBufferRenderGBuffer(pvr::api::SecondaryCommandBuffer& cmdBuffer, pvr::uint32 swapChainIndex, pvr::uint32 subpass);
	void recordCommandsDirectionalLights(pvr::api::SecondaryCommandBuffer& cmdBuffer, pvr::uint32 swapChainIndex, pvr::uint32 subpass);
	void recordCommandsPointLightGeometryStencil(pvr::api::SecondaryCommandBuffer& cmdBuffer, pvr::uint32 swapChainIndex, pvr::uint32 subpass);
	void recordCommandsPointLightProxy(pvr::api::SecondaryCommandBuffer& cmdBuffer, pvr::uint32 swapChainIndex, pvr::uint32 subpass);
	void recordCommandsPointLightSourceLighting(pvr::api::SecondaryCommandBuffer& cmdBuffer, pvr::uint32 swapChainIndex, pvr::uint32 subpass);
	void recordMainCommandBuffer();
	void recordCommandUIRenderer(pvr::api::SecondaryCommandBuffer& cmdBuffer, pvr::uint32 swapChainIndex, pvr::uint32 subpass);
	void recordSecondaryCommandBuffers();
	void allocateLights();
	bool createMaterialsAndDescriptorSets();
	void createStaticSceneDescriptorSet();
	bool loadVbos();
	void uploadStaticData();
	void uploadStaticSceneData();
	void uploadStaticModelData();
	void uploadStaticDirectionalLightData();
	void uploadStaticPointLightData();
	void initialiseStaticLightProperties();
	void updateDynamicSceneData();
	void recordUpdateDynamicBuffersCommandBuffer(pvr::uint32 swapIndex);
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
		case pvr::SimplifiedInput::Action1: m_isPaused = !m_isPaused; break;
		case pvr::SimplifiedInput::Action2: m_animateCamera = !m_animateCamera; break;
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

	m_frameNumber = 0.0f;
	m_isPaused = false;
	m_cameraId = 0;

	//Prepare the asset manager for loading our objects
	m_assetManager.init(*this);

	//  Load the scene and the light
	if (!m_assetManager.loadModel(Files::SceneFile, m_mainScene))
	{
		setExitMessage("ERROR: Couldn't load the scene pod file %s\n", Files::SceneFile);
		return pvr::Result::UnknownError;
	}

	if (m_mainScene->getNumCameras() == 0)
	{
		setExitMessage("ERROR: The main scene to display must contain a camera.\n");
		return pvr::Result::InvalidData;
	}

	//  Load light proxy geometry
	if (!m_assetManager.loadModel(Files::PointLightModelFile, m_pointLightModel))
	{
		setExitMessage("ERROR: Couldn't load the point light proxy pod file\n");
		return pvr::Result::UnableToOpen;
	}
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in initView() will be called by PVRShell upon initialization or after a change in the rendering context.
Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result VulkanDeferredShading::initView()
{
	//Create the empty API objects.
	m_deviceResources.reset(new DeviceResources);

	//Initialize free-floating objects (commandBuffers).
	m_context = getGraphicsContext();

	// Get the number of swap images
	m_deviceResources->numSwapImages = getPlatformContext().getSwapChainLength();

	// Get current swap index
	m_deviceResources->swapIndex = m_context->getPlatformContext().getSwapChainIndex();

	// initialise the gbuffer renderpass list
	m_deviceResources->renderInfo.storeLocalMemoryPass.objects.resize(m_mainScene->getNumMeshNodes());

	// calculate the frame buffer width and heights
	m_framebufferWidth = m_windowWidth = this->getWidth();
	m_framebufferHeight = m_windowHeight = this->getHeight();

	const pvr::platform::CommandLine& cmdOptions = getCommandLine();

	cmdOptions.getIntOption("-fbowidth", m_framebufferWidth);
	m_framebufferWidth = glm::min<pvr::int32>(m_framebufferWidth, m_windowWidth);
	cmdOptions.getIntOption("-fboheight", m_framebufferHeight);
	m_framebufferHeight = glm::min<pvr::int32>(m_framebufferHeight, m_windowHeight);
	cmdOptions.getIntOption("-numlights", PointLightConfiguration::NumProceduralPointLights);
	cmdOptions.getFloatOption("-lightscale", PointLightConfiguration::PointLightScale);
	cmdOptions.getFloatOption("-lightintensity", PointLightConfiguration::PointlightIntensity);

	m_viewportOffsets[0] = (m_windowWidth - m_framebufferWidth) / 2;
	m_viewportOffsets[1] = (m_windowHeight - m_framebufferHeight) / 2;

	pvr::Log(pvr::Log.Information, "FBO dimensions: %d x %d\n", m_framebufferWidth, m_framebufferHeight);
	pvr::Log(pvr::Log.Information, "Onscreen Framebuffer dimensions: %d x %d\n", m_windowWidth, m_windowHeight);

	// setup command buffers
	for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		// main command buffer
		m_deviceResources->cmdBufferMain[i] = m_context->createCommandBufferOnDefaultPool();

		// Subpass 0
		m_deviceResources->cmdBufferRenderToLocalMemory[i] = m_context->createSecondaryCommandBufferOnDefaultPool();
		// Subpass 1
		m_deviceResources->cmdBufferLighting[i] = m_context->createSecondaryCommandBufferOnDefaultPool();

		// command buffer used for copying from staging to server side buffers
		m_deviceResources->cmdBufferUpdateDynamicBuffers[i] = m_context->createSecondaryCommandBufferOnDefaultPool();
	}

	// command buffer used for uploading static data
	m_deviceResources->cmdBufferStaticBufferUpload = m_context->createCommandBufferOnDefaultPool();

	// Create the renderpass using subpasses
	createLocalMemoryRenderPass();

	// Initialise lighting structures
	allocateLights();

	// Create buffers used in the demo
	createBuffers();

	// Initialise the static light properties
	initialiseStaticLightProperties();

	// Create static scene wide descriptor set
	createStaticSceneDescriptorSet();

	// Create the descriptor sets used for the GBuffer pass
	if (!createMaterialsAndDescriptorSets())
	{
		return pvr::Result::NotInitialized;
	}

	// Upload static data
	uploadStaticData();

	// Create lighting descriptor sets
	createDirectionalLightDescriptorSets();
	createPointLightGeometryStencilPassDescriptorSets();
	createPointLightProxyPassDescriptorSets();
	createPointLightSourcePassDescriptorSets();

	// setup UI renderer
	m_deviceResources->uiRenderer.init(m_deviceResources->onScreenLocalMemoryRenderPass,
	                                   RenderPassSubPasses::UIRenderer);
	m_deviceResources->uiRenderer.getDefaultTitle()->setText("DeferredShading");
	m_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	m_deviceResources->uiRenderer.getDefaultControls()->setText("Action1: Pause\nAction2: Orbit Camera\n");
	m_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	// Handle device rotation
	bool isRotated = this->isScreenRotated() && this->isFullScreen();
	if (isRotated)
	{
		m_projectionMatrix = pvr::math::perspective(getApiType(), m_mainScene->getCamera(0).getFOV(), (float)this->getHeight() / (float)this->getWidth(),
		                     m_mainScene->getCamera(0).getNear(), m_mainScene->getCamera(0).getFar(), glm::pi<pvr::float32>() * .5f);
	}
	else
	{
		m_projectionMatrix = pvr::math::perspective(getApiType(), m_mainScene->getCamera(0).getFOV(), (float)this->getWidth() / (float)this->getHeight(),
		                     m_mainScene->getCamera(0).getNear(), m_mainScene->getCamera(0).getFar());
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
	m_assetManager.releaseAll();
	m_deviceResources.reset(0);
	m_context.release();

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
\brief  Main rendering loop function of the program. The shell will call this function every m_frameNumber.
***********************************************************************************************************************/
pvr::Result VulkanDeferredShading::renderFrame()
{
	// Get the current swap index
	m_deviceResources->swapIndex = m_context->getPlatformContext().getSwapChainIndex();

	//  Handle user input and update object animations
	updateAnimation();

	// update dynamic buffers
	updateDynamicSceneData();

	// submit the main command buffer
	m_deviceResources->cmdBufferMain[m_deviceResources->swapIndex]->submit();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  Creates directional lighting descriptor sets.
***********************************************************************************************************************/
void VulkanDeferredShading::createDirectionalLightDescriptorSets()
{
	{
		// create the descriptor set layout
		pvr::api::DescriptorSetLayoutCreateParam descSetInfo;

		// Buffers
		descSetInfo.setBinding(0, pvr::types::DescriptorType::UniformBufferDynamic, 1u, pvr::types::ShaderStageFlags::Fragment);
		descSetInfo.setBinding(1, pvr::types::DescriptorType::UniformBufferDynamic, 1u, pvr::types::ShaderStageFlags::Fragment);

		// Input attachments
		descSetInfo.setBinding(2, pvr::types::DescriptorType::InputAttachment, 1u, pvr::types::ShaderStageFlags::Fragment);
		descSetInfo.setBinding(3, pvr::types::DescriptorType::InputAttachment, 1u, pvr::types::ShaderStageFlags::Fragment);
		descSetInfo.setBinding(4, pvr::types::DescriptorType::InputAttachment, 1u, pvr::types::ShaderStageFlags::Fragment);

		m_deviceResources->directionalLightingDescriptorLayout = m_context->createDescriptorSetLayout(descSetInfo);

		{
			// create the pipeline layout
			pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;

			pipeLayoutInfo.setDescSetLayout(0, m_deviceResources->staticSceneLayout);
			pipeLayoutInfo.setDescSetLayout(1, m_deviceResources->directionalLightingDescriptorLayout);
			m_deviceResources->directionalLightingPipelineLayout = m_context->createPipelineLayout(pipeLayoutInfo);
		}

		// create the swapchain descriptor sets with corresponding buffers/images
		for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
		{
			pvr::api::DescriptorSetUpdate descSetUpdate;

			descSetUpdate.setDynamicUbo(0, m_deviceResources->staticDirectionalLightUbo);
			descSetUpdate.setDynamicUbo(1, m_deviceResources->dynamicDirectionalLightUbo[i]);

			descSetUpdate.setInputImageAttachment(2, m_deviceResources->onScreenFboTextureViews[Fbo::Albedo][i]);
			descSetUpdate.setInputImageAttachment(3, m_deviceResources->onScreenFboTextureViews[Fbo::Normal][i]);
			descSetUpdate.setInputImageAttachment(4, m_deviceResources->onScreenFboTextureViews[Fbo::Depth][i]);

			m_deviceResources->directionalLightingDescriptorSets.add(m_context->createDescriptorSetOnDefaultPool(
			      m_deviceResources->directionalLightingDescriptorLayout));

			m_deviceResources->directionalLightingDescriptorSets[i]->update(descSetUpdate);
		}
	}
}

/*!*********************************************************************************************************************
\brief  Creates point lighting stencil pass descriptor sets.
***********************************************************************************************************************/
void VulkanDeferredShading::createPointLightGeometryStencilPassDescriptorSets()
{
	{
		// create descriptor set layout
		pvr::api::DescriptorSetLayoutCreateParam descSetInfo;

		// buffers
		descSetInfo.setBinding(0, pvr::types::DescriptorType::UniformBufferDynamic, 1u, pvr::types::ShaderStageFlags::Fragment);
		descSetInfo.setBinding(1, pvr::types::DescriptorType::UniformBufferDynamic, 1u, pvr::types::ShaderStageFlags::Vertex);

		m_deviceResources->pointLightGeometryStencilDescriptorLayout = m_context->createDescriptorSetLayout(descSetInfo);

		{
			// create the pipeline layout
			pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;

			pipeLayoutInfo.setDescSetLayout(0, m_deviceResources->staticSceneLayout);
			pipeLayoutInfo.setDescSetLayout(1, m_deviceResources->pointLightGeometryStencilDescriptorLayout);
			m_deviceResources->pointLightGeometryStencilPipelineLayout = m_context->createPipelineLayout(pipeLayoutInfo);
		}

		// create the swapchain descriptor sets with corresponding buffers
		for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
		{
			pvr::api::DescriptorSetUpdate descSetUpdate;

			descSetUpdate.setDynamicUbo(0, m_deviceResources->staticPointLightUbo);
			descSetUpdate.setDynamicUbo(1, m_deviceResources->dynamicPointLightUbo[i]);

			m_deviceResources->pointLightGeometryStencilDescriptorSets.add(m_context->createDescriptorSetOnDefaultPool(
			      m_deviceResources->pointLightGeometryStencilDescriptorLayout));

			m_deviceResources->pointLightGeometryStencilDescriptorSets[i]->update(descSetUpdate);
		}
	}
}

/*!*********************************************************************************************************************
\brief  Creates point lighting proxy pass descriptor sets.
***********************************************************************************************************************/
void VulkanDeferredShading::createPointLightProxyPassDescriptorSets()
{
	{
		// create buffer descriptor set layout
		pvr::api::DescriptorSetLayoutCreateParam descSetInfo;

		// Buffers
		descSetInfo.setBinding(0, pvr::types::DescriptorType::UniformBufferDynamic, 1u, pvr::types::ShaderStageFlags::Fragment);
		descSetInfo.setBinding(1, pvr::types::DescriptorType::UniformBufferDynamic, 1u, pvr::types::ShaderStageFlags::Vertex);
		descSetInfo.setBinding(2, pvr::types::DescriptorType::UniformBufferDynamic, 1u, pvr::types::ShaderStageFlags::Fragment);

		m_deviceResources->pointLightProxyDescriptorLayout = m_context->createDescriptorSetLayout(descSetInfo);

		pvr::api::DescriptorSetLayoutCreateParam localMemoryDescSetInfo;

		// Input attachment descriptor set layout
		localMemoryDescSetInfo.setBinding(0, pvr::types::DescriptorType::InputAttachment, 1u, pvr::types::ShaderStageFlags::Fragment);
		localMemoryDescSetInfo.setBinding(1, pvr::types::DescriptorType::InputAttachment, 1u, pvr::types::ShaderStageFlags::Fragment);
		localMemoryDescSetInfo.setBinding(2, pvr::types::DescriptorType::InputAttachment, 1u, pvr::types::ShaderStageFlags::Fragment);

		m_deviceResources->pointLightProxyLocalMemoryDescriptorLayout = m_context->createDescriptorSetLayout(localMemoryDescSetInfo);

		{
			// create the pipeline layout
			pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;

			pipeLayoutInfo.setDescSetLayout(0, m_deviceResources->staticSceneLayout);
			pipeLayoutInfo.setDescSetLayout(1, m_deviceResources->pointLightProxyDescriptorLayout);
			pipeLayoutInfo.setDescSetLayout(2, m_deviceResources->pointLightProxyLocalMemoryDescriptorLayout);
			m_deviceResources->pointLightProxyPipelineLayout = m_context->createPipelineLayout(pipeLayoutInfo);
		}

		// create the swapchain descriptor sets with corresponding buffers
		for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
		{
			pvr::api::DescriptorSetUpdate descSetUpdate;

			descSetUpdate.setDynamicUbo(0, m_deviceResources->staticPointLightUbo);
			descSetUpdate.setDynamicUbo(1, m_deviceResources->dynamicPointLightUbo[i]);
			descSetUpdate.setDynamicUbo(2, m_deviceResources->dynamicPointLightUbo[i]);

			m_deviceResources->pointLightProxyDescriptorSets.add(m_context->createDescriptorSetOnDefaultPool(
			      m_deviceResources->pointLightProxyDescriptorLayout));

			m_deviceResources->pointLightProxyDescriptorSets[i]->update(descSetUpdate);
		}

		// create the swapchain descriptor sets with corresponding images
		for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
		{
			pvr::api::DescriptorSetUpdate descSetUpdate;

			descSetUpdate.setInputImageAttachment(0, m_deviceResources->onScreenFboTextureViews[Fbo::Albedo][i]);
			descSetUpdate.setInputImageAttachment(1, m_deviceResources->onScreenFboTextureViews[Fbo::Normal][i]);
			descSetUpdate.setInputImageAttachment(2, m_deviceResources->onScreenFboTextureViews[Fbo::Depth][i]);

			m_deviceResources->pointLightProxyLocalMemoryDescriptorLayout = m_context->createDescriptorSetLayout(localMemoryDescSetInfo);

			m_deviceResources->pointLightProxyLocalMemoryDescriptorSets.add(m_context->createDescriptorSetOnDefaultPool(
			      m_deviceResources->pointLightProxyLocalMemoryDescriptorLayout));

			m_deviceResources->pointLightProxyLocalMemoryDescriptorSets[i]->update(descSetUpdate);
		}
	}
}

/*!*********************************************************************************************************************
\brief  Creates point lighting source pass descriptor sets.
***********************************************************************************************************************/
void VulkanDeferredShading::createPointLightSourcePassDescriptorSets()
{
	{
		// create descriptor set layout
		pvr::api::DescriptorSetLayoutCreateParam descSetInfo;

		descSetInfo.setBinding(0, pvr::types::DescriptorType::UniformBufferDynamic, 1u, pvr::types::ShaderStageFlags::Fragment);
		descSetInfo.setBinding(1, pvr::types::DescriptorType::UniformBufferDynamic, 1u, pvr::types::ShaderStageFlags::Vertex);

		m_deviceResources->pointLightSourceDescriptorLayout = m_context->createDescriptorSetLayout(descSetInfo);

		{
			// create the pipeline layout
			pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;

			pipeLayoutInfo.setDescSetLayout(0, m_deviceResources->staticSceneLayout);
			pipeLayoutInfo.setDescSetLayout(1, m_deviceResources->pointLightSourceDescriptorLayout);
			m_deviceResources->pointLightSourcePipelineLayout = m_context->createPipelineLayout(pipeLayoutInfo);
		}

		// create the swapchain descriptor sets with corresponding buffers
		for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
		{
			pvr::api::DescriptorSetUpdate descSetUpdate;

			descSetUpdate.setDynamicUbo(0, m_deviceResources->staticPointLightUbo);
			descSetUpdate.setDynamicUbo(1, m_deviceResources->dynamicPointLightUbo[i]);

			m_deviceResources->pointLightSourceDescriptorSets.add(m_context->createDescriptorSetOnDefaultPool(
			      m_deviceResources->pointLightSourceDescriptorLayout));

			m_deviceResources->pointLightSourceDescriptorSets[i]->update(descSetUpdate);
		}
	}
}

/*!*********************************************************************************************************************
\brief  Creates static scene wide descriptor set.
***********************************************************************************************************************/
void VulkanDeferredShading::createStaticSceneDescriptorSet()
{
	// static per scene buffer
	pvr::api::DescriptorSetLayoutCreateParam staticSceneDescSetInfo;
	staticSceneDescSetInfo.setBinding(0, pvr::types::DescriptorType::UniformBuffer, 1u, pvr::types::ShaderStageFlags::Fragment);
	m_deviceResources->staticSceneLayout = m_context->createDescriptorSetLayout(staticSceneDescSetInfo);

	// Create static descriptor set for the scene
	{
		pvr::api::DescriptorSetUpdate descSetUpdate;
		descSetUpdate.setUbo(0, m_deviceResources->staticFarClipDistanceUbo);

		pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;

		pipeLayoutInfo.setDescSetLayout(0, m_deviceResources->staticSceneLayout);
		m_deviceResources->scenePipelineLayout = m_context->createPipelineLayout(pipeLayoutInfo);

		m_deviceResources->sceneDescriptorSet = m_context->createDescriptorSetOnDefaultPool(m_deviceResources->staticSceneLayout);
		m_deviceResources->sceneDescriptorSet->update(descSetUpdate);
	}
}

/*!*********************************************************************************************************************
\return Return true if no error occurred
\brief  Loads the textures required for this example and sets up the GBuffer descriptor sets
***********************************************************************************************************************/
bool VulkanDeferredShading::createMaterialsAndDescriptorSets()
{
	if (m_mainScene->getNumMaterials() == 0)
	{
		setExitMessage("ERROR: The scene does not contain any materials.");
		return false;
	}

	// CREATE THE SAMPLERS
	// create trilinear sampler
	pvr::assets::SamplerCreateParam samplerDesc;
	samplerDesc.wrapModeU = samplerDesc.wrapModeV = samplerDesc.wrapModeW = pvr::types::SamplerWrap::Repeat;
	samplerDesc.minificationFilter = pvr::types::SamplerFilter::Linear;
	samplerDesc.magnificationFilter = pvr::types::SamplerFilter::Linear;
	samplerDesc.mipMappingFilter = pvr::types::SamplerFilter::Linear;
	pvr::api::Sampler samplerTrilinear = m_context->createSampler(samplerDesc);

	// CREATE THE DESCRIPTOR SET LAYOUTS
	// Per Model Descriptor set layout
	pvr::api::DescriptorSetLayoutCreateParam descSetInfo;
	// create the ubo descriptor setlayout
	//static material ubo
	descSetInfo.setBinding(0, pvr::types::DescriptorType::UniformBufferDynamic, 1u, pvr::types::ShaderStageFlags::Fragment);

	// static model ubo
	descSetInfo.setBinding(1, pvr::types::DescriptorType::UniformBufferDynamic, 1u, pvr::types::ShaderStageFlags::Vertex);

	// no texture sampler layout
	m_deviceResources->noSamplerLayout = m_context->createDescriptorSetLayout(descSetInfo);

	// Single texture sampler layout
	descSetInfo.setBinding(2, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
	m_deviceResources->oneSamplerLayout = m_context->createDescriptorSetLayout(descSetInfo);

	// Two textures sampler layout
	descSetInfo.setBinding(3, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
	m_deviceResources->twoSamplerLayout = m_context->createDescriptorSetLayout(descSetInfo);

	// Three textures sampler layout
	descSetInfo.setBinding(4, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
	m_deviceResources->threeSamplerLayout = m_context->createDescriptorSetLayout(descSetInfo);

	// Four textures sampler layout (for GBuffer rendering)
	descSetInfo.setBinding(5, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
	m_deviceResources->fourSamplerLayout = m_context->createDescriptorSetLayout(descSetInfo);

	// create the pipeline layouts
	pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;

	pipeLayoutInfo.setDescSetLayout(0, m_deviceResources->staticSceneLayout);

	pipeLayoutInfo.setDescSetLayout(1, m_deviceResources->noSamplerLayout);
	m_deviceResources->pipeLayoutNoSamplers = m_context->createPipelineLayout(pipeLayoutInfo);

	pipeLayoutInfo.setDescSetLayout(1, m_deviceResources->oneSamplerLayout);
	m_deviceResources->pipeLayoutOneSampler = m_context->createPipelineLayout(pipeLayoutInfo);

	pipeLayoutInfo.setDescSetLayout(1, m_deviceResources->twoSamplerLayout);
	m_deviceResources->pipeLayoutTwoSamplers = m_context->createPipelineLayout(pipeLayoutInfo);

	pipeLayoutInfo.setDescSetLayout(1, m_deviceResources->threeSamplerLayout);
	m_deviceResources->pipeLayoutThreeSamplers = m_context->createPipelineLayout(pipeLayoutInfo);

	pipeLayoutInfo.setDescSetLayout(1, m_deviceResources->fourSamplerLayout);
	m_deviceResources->pipeLayoutFourSamplers = m_context->createPipelineLayout(pipeLayoutInfo);

	// CREATE DESCRIPTOR SETS FOR EACH MATERIAL
	m_deviceResources->materials.resize(m_mainScene->getNumMaterials());
	for (pvr::uint32 i = 0; i < m_mainScene->getNumMaterials(); ++i)
	{
		m_deviceResources->materials[i].materialDescriptorSet.resize(getPlatformContext().getSwapChainLength());

		for (pvr::uint32 j = 0; j < getPlatformContext().getSwapChainLength(); ++j)
		{
			pvr::api::DescriptorSetUpdate descSetUpdate;

			descSetUpdate.setDynamicUbo(0, m_deviceResources->staticModelMaterialUbo);
			descSetUpdate.setDynamicUbo(1, m_deviceResources->dynamicModelMatrixUbo[j]);

			pvr::api::TextureView diffuseMap;
			pvr::api::TextureView bumpMap;

			// get the current material
			const pvr::assets::Model::Material& material = m_mainScene->getMaterial(i);

			// get material properties
			m_deviceResources->materials[i].specularStrength = material.getShininess();
			m_deviceResources->materials[i].diffuseColor = material.getDiffuse();

			int numTextures = 0;

			if (material.getDiffuseTextureIndex() != -1)
			{
				// Load the diffuse texture map
				if (!m_assetManager.getTextureWithCaching(getGraphicsContext(),
				    m_mainScene->getTexture(material.getDiffuseTextureIndex()).getName(), &(diffuseMap), NULL))
				{
					setExitMessage("ERROR: Failed to load texture %s", m_mainScene->getTexture(material.getDiffuseTextureIndex()).getName().c_str());
					return false;
				}
				descSetUpdate.setCombinedImageSampler(2, diffuseMap, samplerTrilinear);
				++numTextures;
			}
			if (material.getBumpMapTextureIndex() != -1)
			{
				// Load the bumpmap
				if (!m_assetManager.getTextureWithCaching(getGraphicsContext(),
				    m_mainScene->getTexture(material.getBumpMapTextureIndex()).getName(), &(bumpMap), NULL))
				{
					setExitMessage("ERROR: Failed to load texture %s", m_mainScene->getTexture(material.getBumpMapTextureIndex()).getName().c_str());
					return false;
				}
				++numTextures;
				descSetUpdate.setCombinedImageSampler(3, bumpMap, samplerTrilinear);
			}

			// based on the number of textures select the correct descriptor set
			switch (numTextures)
			{
			case 0: m_deviceResources->materials[i].materialDescriptorSet[j] =
				  m_context->createDescriptorSetOnDefaultPool(m_deviceResources->noSamplerLayout);
				break;
			case 1: m_deviceResources->materials[i].materialDescriptorSet[j] =
				  m_context->createDescriptorSetOnDefaultPool(m_deviceResources->oneSamplerLayout);
				break;
			case 2: m_deviceResources->materials[i].materialDescriptorSet[j] =
				  m_context->createDescriptorSetOnDefaultPool(m_deviceResources->twoSamplerLayout);
				break;
			case 3: m_deviceResources->materials[i].materialDescriptorSet[j] =
				  m_context->createDescriptorSetOnDefaultPool(m_deviceResources->threeSamplerLayout);
				break;
			case 4: m_deviceResources->materials[i].materialDescriptorSet[j] =
				  m_context->createDescriptorSetOnDefaultPool(m_deviceResources->fourSamplerLayout);
				break;
			default:
				break;
			}

			m_deviceResources->materials[i].materialDescriptorSet[j]->update(descSetUpdate);
		}
	}

	return true;
}

/*!*********************************************************************************************************************
\brief  Creates model pipelines.
***********************************************************************************************************************/
void VulkanDeferredShading::createModelPipelines()
{
	pvr::api::GraphicsPipelineCreateParam renderGBufferPipelineCreateParam;

	// enable back face culling
	renderGBufferPipelineCreateParam.rasterizer.setCullFace(pvr::types::Face::Back);

	// set counter clockwise winding order for front faces
	renderGBufferPipelineCreateParam.rasterizer.setFrontFaceWinding(pvr::types::PolygonWindingOrder::FrontFaceCCW);

	// enable depth testing
	renderGBufferPipelineCreateParam.depthStencil.setDepthTestEnable(true);
	renderGBufferPipelineCreateParam.depthStencil.setDepthWrite(true);

	// set the blend state for the color attachments
	pvr::types::BlendingConfig renderGBufferColorAttachment;
	// number of color blend states must equal number of color attachments for the subpass
	renderGBufferPipelineCreateParam.colorBlend.setAttachmentState(0, renderGBufferColorAttachment);
	renderGBufferPipelineCreateParam.colorBlend.setAttachmentState(1, renderGBufferColorAttachment);
	renderGBufferPipelineCreateParam.colorBlend.setAttachmentState(2, renderGBufferColorAttachment);

	// load and create appropriate shaders
	pvr::Stream::ptr_type gbufferVertSource = getAssetStream(Files::GBufferVertexShader);
	pvr::Stream::ptr_type gbufferFragSource = getAssetStream(Files::GBufferFragmentShader);
	renderGBufferPipelineCreateParam.vertexShader.setShader(m_context->createShader(*gbufferVertSource, pvr::types::ShaderType::VertexShader));
	renderGBufferPipelineCreateParam.fragmentShader.setShader(m_context->createShader(*gbufferFragSource, pvr::types::ShaderType::FragmentShader));

	// setup vertex inputs
	renderGBufferPipelineCreateParam.vertexInput.clear();
	pvr::utils::createInputAssemblyFromMesh(m_mainScene->getMesh(MeshNodes::Satyr), vertexBindings, 4, renderGBufferPipelineCreateParam);

	// renderpass/subpass
	renderGBufferPipelineCreateParam.renderPass = m_deviceResources->onScreenLocalMemoryRenderPass;
	renderGBufferPipelineCreateParam.subPass = RenderPassSubPasses::GBuffer;

	// enable stencil testing
	pvr::api::pipelineCreation::DepthStencilStateCreateParam::StencilState stencilState;

	// only replace stencil buffer when the depth test passes
	stencilState.opStencilFail = pvr::types::StencilOp::Keep;
	stencilState.opDepthFail = pvr::types::StencilOp::Keep;
	stencilState.opDepthPass = pvr::types::StencilOp::Replace;
	stencilState.compareOp = pvr::types::ComparisonMode::Always;

	// enable the stencil tests
	renderGBufferPipelineCreateParam.depthStencil.setStencilTest(true);
	// set stencil states
	renderGBufferPipelineCreateParam.depthStencil.setStencilFront(stencilState);
	renderGBufferPipelineCreateParam.depthStencil.setStencilBack(stencilState);

	renderGBufferPipelineCreateParam.pipelineLayout = m_deviceResources->pipeLayoutTwoSamplers;
	m_deviceResources->renderInfo.storeLocalMemoryPass.objects[MeshNodes::Satyr].pipeline = m_context->createGraphicsPipeline(renderGBufferPipelineCreateParam);

	// load and create appropriate shaders
	pvr::Stream::ptr_type gbufferFloorVertSource = getAssetStream(Files::GBufferFloorVertexShader);
	pvr::Stream::ptr_type gbufferFloorFragSource = getAssetStream(Files::GBufferFloorFragmentShader);
	renderGBufferPipelineCreateParam.vertexShader.setShader(m_context->createShader(*gbufferFloorVertSource, pvr::types::ShaderType::VertexShader));
	renderGBufferPipelineCreateParam.fragmentShader.setShader(m_context->createShader(*gbufferFloorFragSource, pvr::types::ShaderType::FragmentShader));

	// setup vertex inputs
	renderGBufferPipelineCreateParam.vertexInput.clear();
	pvr::utils::createInputAssemblyFromMesh(m_mainScene->getMesh(MeshNodes::Floor), floorVertexBindings, 3, renderGBufferPipelineCreateParam);

	renderGBufferPipelineCreateParam.pipelineLayout = m_deviceResources->pipeLayoutOneSampler;
	m_deviceResources->renderInfo.storeLocalMemoryPass.objects[MeshNodes::Floor].pipeline = m_context->createGraphicsPipeline(renderGBufferPipelineCreateParam);
}

/*!*********************************************************************************************************************
\brief  Creates direcitonal lighting pipeline.
***********************************************************************************************************************/
void VulkanDeferredShading::createDirectionalLightingPipeline()
{
	// DIRECTIONAL LIGHTING - A full-screen quad that will apply any global (ambient/directional) lighting
	// disable the depth write as we do not want to modify the depth buffer while rendering directional lights

	pvr::api::GraphicsPipelineCreateParam renderDirectionalLightingPipelineCreateParam;

	// enable back face culling
	renderDirectionalLightingPipelineCreateParam.rasterizer.setCullFace(pvr::types::Face::Back);

	// set counter clockwise winding order for front faces
	renderDirectionalLightingPipelineCreateParam.rasterizer.setFrontFaceWinding(pvr::types::PolygonWindingOrder::FrontFaceCCW);

	// Make use of the stencil buffer contents to only shade pixels where actual geometry is located.
	pvr::api::pipelineCreation::DepthStencilStateCreateParam::StencilState stencilState;

	// keep the stencil states the same as the previous pass. These aren't important to this pass.
	stencilState.opStencilFail = pvr::types::StencilOp::Keep;
	stencilState.opDepthFail = pvr::types::StencilOp::Keep;
	stencilState.opDepthPass = pvr::types::StencilOp::Replace;

	// if the stencil is equal to the value specified then stencil passes
	stencilState.compareOp = pvr::types::ComparisonMode::Equal;

	// disable depth writing and depth testing
	renderDirectionalLightingPipelineCreateParam.depthStencil.setDepthWrite(false);
	renderDirectionalLightingPipelineCreateParam.depthStencil.setDepthTestEnable(false);

	// enable stencil testing
	renderDirectionalLightingPipelineCreateParam.depthStencil.setStencilTest(true);
	renderDirectionalLightingPipelineCreateParam.depthStencil.setStencilFront(stencilState);
	renderDirectionalLightingPipelineCreateParam.depthStencil.setStencilBack(stencilState);

	// set the blend state for the color attachments
	renderDirectionalLightingPipelineCreateParam.colorBlend.setAttachmentState(0, pvr::types::BlendingConfig());

	// load and create appropriate shaders
	pvr::Stream::ptr_type gbufferVertSource = getAssetStream(Files::AttributelessVertexShader);
	pvr::Stream::ptr_type gbufferFragSource = getAssetStream(Files::DirectionalLightingFragmentShader);
	renderDirectionalLightingPipelineCreateParam.vertexShader.setShader(m_context->createShader(*gbufferVertSource, pvr::types::ShaderType::VertexShader));
	renderDirectionalLightingPipelineCreateParam.fragmentShader.setShader(m_context->createShader(*gbufferFragSource, pvr::types::ShaderType::FragmentShader));

	// setup vertex inputs
	renderDirectionalLightingPipelineCreateParam.vertexInput.clear();
	renderDirectionalLightingPipelineCreateParam.inputAssembler.setPrimitiveTopology(pvr::types::PrimitiveTopology::TriangleStrip);

	renderDirectionalLightingPipelineCreateParam.pipelineLayout = m_deviceResources->directionalLightingPipelineLayout;

	// renderpass/subpass
	renderDirectionalLightingPipelineCreateParam.renderPass = m_deviceResources->onScreenLocalMemoryRenderPass;
	renderDirectionalLightingPipelineCreateParam.subPass = RenderPassSubPasses::Lighting;

	m_deviceResources->renderInfo.directionalLightPass.pipeline = m_context->createGraphicsPipeline(renderDirectionalLightingPipelineCreateParam);
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
	pvr::api::GraphicsPipelineCreateParam pointLightStencilPipelineCreateParam;

	pvr::types::BlendingConfig stencilPassColorAttachmentBlendState;
	stencilPassColorAttachmentBlendState.channelWriteMask = pvr::types::ColorChannel::None;

	// set the blend state for the color attachments
	pointLightStencilPipelineCreateParam.colorBlend.setAttachmentState(0, stencilPassColorAttachmentBlendState);

	// enable back face culling
	pointLightStencilPipelineCreateParam.rasterizer.setCullFace(pvr::types::Face::Back);

	// set counter clockwise winding order for front faces
	pointLightStencilPipelineCreateParam.rasterizer.setFrontFaceWinding(pvr::types::PolygonWindingOrder::FrontFaceCCW);

	// disable depth write. This pass reuses previously written depth buffer
	pointLightStencilPipelineCreateParam.depthStencil.setDepthTestEnable(true);
	pointLightStencilPipelineCreateParam.depthStencil.setDepthWrite(false);

	// set depth comparison to less/equal
	pointLightStencilPipelineCreateParam.depthStencil.setDepthCompareFunc(pvr::types::ComparisonMode::LessEqual);
	pointLightStencilPipelineCreateParam.depthStencil.setStencilTest(true);

	// load and create appropriate shaders
	pvr::Stream::ptr_type vertSource = getAssetStream(Files::PointLightPass1VertexShader);
	pvr::Stream::ptr_type fragSource = getAssetStream(Files::PointLightPass1FragmentShader);
	pointLightStencilPipelineCreateParam.vertexShader.setShader(m_context->createShader(*vertSource, pvr::types::ShaderType::VertexShader));
	pointLightStencilPipelineCreateParam.fragmentShader.setShader(m_context->createShader(*fragSource, pvr::types::ShaderType::FragmentShader));

	// setup vertex inputs
	pointLightStencilPipelineCreateParam.vertexInput.clear();
	pvr::utils::createInputAssemblyFromMesh(m_pointLightModel->getMesh(LightNodes::PointLightMeshNode), pointLightVertexBindings, 1, pointLightStencilPipelineCreateParam);

	pvr::api::pipelineCreation::DepthStencilStateCreateParam::StencilState stencilState;
	stencilState.compareOp = pvr::types::ComparisonMode::Always;
	// keep current value if the stencil test fails
	stencilState.opStencilFail = pvr::types::StencilOp::Keep;
	// if the depth test fails then increment wrap
	stencilState.opDepthFail = pvr::types::StencilOp::IncrementWrap;
	stencilState.opDepthPass = pvr::types::StencilOp::Keep;

	// set stencil state for the front face of the light sources
	pointLightStencilPipelineCreateParam.depthStencil.setStencilFront(stencilState);

	// set stencil state for the back face of the light sources
	stencilState.opDepthFail = pvr::types::StencilOp::Keep;
	pointLightStencilPipelineCreateParam.depthStencil.setStencilBack(stencilState);

	// renderpass/subpass
	pointLightStencilPipelineCreateParam.renderPass = m_deviceResources->onScreenLocalMemoryRenderPass;
	pointLightStencilPipelineCreateParam.subPass = RenderPassSubPasses::Lighting;

	pointLightStencilPipelineCreateParam.pipelineLayout = m_deviceResources->pointLightGeometryStencilPipelineLayout;

	m_deviceResources->renderInfo.pointLightGeometryStencilPass.pipeline = m_context->createGraphicsPipeline(pointLightStencilPipelineCreateParam);
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
	pvr::api::GraphicsPipelineCreateParam pointLightProxyPipelineCreateParam;

	// enable front face culling - cull the front faces of the light sources
	pointLightProxyPipelineCreateParam.rasterizer.setCullFace(pvr::types::Face::Front);

	// set counter clockwise winding order for front faces
	pointLightProxyPipelineCreateParam.rasterizer.setFrontFaceWinding(pvr::types::PolygonWindingOrder::FrontFaceCCW);

	// enable stencil testing
	pointLightProxyPipelineCreateParam.depthStencil.setStencilTest(true);

	// enable depth testing
	pointLightProxyPipelineCreateParam.depthStencil.setDepthTestEnable(true);
	pointLightProxyPipelineCreateParam.depthStencil.setDepthCompareFunc(pvr::types::ComparisonMode::GreaterEqual);
	// disable depth writes
	pointLightProxyPipelineCreateParam.depthStencil.setDepthWrite(false);

	// enable blending
	// blend lighting on top of existing directional lighting
	pvr::types::BlendingConfig blendConfig;
	blendConfig.blendEnable = true;
	blendConfig.srcBlendColor = pvr::types::BlendFactor::One;
	blendConfig.srcBlendAlpha = pvr::types::BlendFactor::One;
	blendConfig.destBlendColor = pvr::types::BlendFactor::One;
	blendConfig.destBlendAlpha = pvr::types::BlendFactor::One;
	blendConfig.channelWriteMask = pvr::types::ColorChannel::All;
	pointLightProxyPipelineCreateParam.colorBlend.setAttachmentState(0, blendConfig);

	// load and create appropriate shaders
	pvr::Stream::ptr_type vertSource = getAssetStream(Files::PointLightPass2VertexShader);
	pvr::Stream::ptr_type fragSource = getAssetStream(Files::PointLightPass2FragmentShader);
	pointLightProxyPipelineCreateParam.vertexShader.setShader(m_context->createShader(*vertSource, pvr::types::ShaderType::VertexShader));
	pointLightProxyPipelineCreateParam.fragmentShader.setShader(m_context->createShader(*fragSource, pvr::types::ShaderType::FragmentShader));

	// setup vertex states
	pointLightProxyPipelineCreateParam.vertexInput.clear();
	pvr::utils::createInputAssemblyFromMesh(m_pointLightModel->getMesh(LightNodes::PointLightMeshNode),
	                                        pointLightVertexBindings, 1, pointLightProxyPipelineCreateParam);

	// if stencil state equals 0 then the lighting should take place as there is geometry inside the point lights area
	pvr::api::pipelineCreation::DepthStencilStateCreateParam::StencilState stencilState;
	stencilState.compareOp = pvr::types::ComparisonMode::Always;
	stencilState.reference = 0;

	pointLightProxyPipelineCreateParam.depthStencil.setStencilFront(stencilState);
	pointLightProxyPipelineCreateParam.depthStencil.setStencilBack(stencilState);

	// renderpass/subpass
	pointLightProxyPipelineCreateParam.renderPass = m_deviceResources->onScreenLocalMemoryRenderPass;
	pointLightProxyPipelineCreateParam.subPass = RenderPassSubPasses::Lighting;

	pointLightProxyPipelineCreateParam.pipelineLayout = m_deviceResources->pointLightProxyPipelineLayout;

	m_deviceResources->renderInfo.pointLightProxyPass.pipeline = m_context->createGraphicsPipeline(pointLightProxyPipelineCreateParam);
}

/*!*********************************************************************************************************************
\brief  Creates point lighting source pass pipeline.
***********************************************************************************************************************/
void VulkanDeferredShading::createPointLightSourcePipeline()
{
	// LIGHT SOURCES : Rendering the "will-o-wisps" that are the sources of the light
	pvr::api::GraphicsPipelineCreateParam pointLightSourcePipelineCreateParam;

	// enable back face culling
	pointLightSourcePipelineCreateParam.rasterizer.setCullFace(pvr::types::Face::Back);

	// set counter clockwise winding order for front faces
	pointLightSourcePipelineCreateParam.rasterizer.setFrontFaceWinding(pvr::types::PolygonWindingOrder::FrontFaceCCW);

	// disable stencil testing
	pointLightSourcePipelineCreateParam.depthStencil.setStencilTest(false);

	// enable depth testing
	pointLightSourcePipelineCreateParam.depthStencil.setDepthTestEnable(true);
	pointLightSourcePipelineCreateParam.depthStencil.setDepthCompareFunc(pvr::types::ComparisonMode::LessEqual);
	pointLightSourcePipelineCreateParam.depthStencil.setDepthWrite(true);

	// enable blending
	pvr::types::BlendingConfig colorAttachment;
	colorAttachment.blendEnable = true;
	colorAttachment.srcBlendColor = pvr::types::BlendFactor::One;
	colorAttachment.srcBlendAlpha = pvr::types::BlendFactor::One;
	colorAttachment.destBlendColor = pvr::types::BlendFactor::One;
	colorAttachment.destBlendAlpha = pvr::types::BlendFactor::One;
	colorAttachment.channelWriteMask = pvr::types::ColorChannel::All;
	pointLightSourcePipelineCreateParam.colorBlend.setAttachmentState(0, colorAttachment);

	// load and create appropriate shaders
	pvr::Stream::ptr_type vertSource = getAssetStream(Files::PointLightPass3VertexShader);
	pvr::Stream::ptr_type fragSource = getAssetStream(Files::PointLightPass3FragmentShader);
	pointLightSourcePipelineCreateParam.vertexShader.setShader(m_context->createShader(*vertSource, pvr::types::ShaderType::VertexShader));
	pointLightSourcePipelineCreateParam.fragmentShader.setShader(m_context->createShader(*fragSource, pvr::types::ShaderType::FragmentShader));

	// setup vertex states
	pointLightSourcePipelineCreateParam.vertexInput.clear();
	pvr::utils::createInputAssemblyFromMesh(m_pointLightModel->getMesh(LightNodes::PointLightMeshNode),
	                                        pointLightVertexBindings, 1, pointLightSourcePipelineCreateParam);

	// renderpass/subpass
	pointLightSourcePipelineCreateParam.renderPass = m_deviceResources->onScreenLocalMemoryRenderPass;
	pointLightSourcePipelineCreateParam.subPass = RenderPassSubPasses::Lighting;

	pointLightSourcePipelineCreateParam.pipelineLayout = m_deviceResources->pointLightSourcePipelineLayout;

	m_deviceResources->renderInfo.pointLightSourcesPass.pipeline = m_context->createGraphicsPipeline(pointLightSourcePipelineCreateParam);
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
void VulkanDeferredShading::createLocalMemoryRenderPass()
{
	pvr::api::RenderPassCreateParam renderPassInfo;
	pvr::api::RenderPassDepthStencilInfo renderPassDepthStencilInfo = pvr::api::RenderPassDepthStencilInfo(
	      m_context->getDepthStencilImageFormat(),
	      pvr::types::LoadOp::Clear, pvr::types::StoreOp::Ignore, pvr::types::LoadOp::Clear, pvr::types::StoreOp::Ignore);

	renderPassInfo.setDepthStencilInfo(renderPassDepthStencilInfo);

	renderPassInfo.setColorInfo(0, pvr::api::RenderPassColorInfo(m_context->getPresentationImageFormat(), pvr::types::LoadOp::Clear));

	const pvr::api::ImageStorageFormat renderpassStorageFormats[Fbo::Count] =
	{
		pvr::api::ImageStorageFormat(pvr::PixelFormat::RGBA_8888, 1, pvr::types::ColorSpace::lRGB, pvr::VariableType::UnsignedByteNorm),  // albedo
		pvr::api::ImageStorageFormat(pvr::PixelFormat('r', 'g', 'b', 'a', 16, 16, 16, 16) , 1, pvr::types::ColorSpace::lRGB, pvr::VariableType::SignedFloat), // normal
		pvr::api::ImageStorageFormat(pvr::PixelFormat::R_32, 1, pvr::types::ColorSpace::lRGB, pvr::VariableType::Float),          // depth attachment
	};

	renderPassInfo.setColorInfo(1, pvr::api::RenderPassColorInfo(renderpassStorageFormats[Fbo::Albedo], pvr::types::LoadOp::Clear,
	                            pvr::types::StoreOp::Ignore, 1u, pvr::types::ImageLayout::ColorAttachmentOptimal, pvr::types::ImageLayout::ColorAttachmentOptimal));
	renderPassInfo.setColorInfo(2, pvr::api::RenderPassColorInfo(renderpassStorageFormats[Fbo::Normal], pvr::types::LoadOp::Clear,
	                            pvr::types::StoreOp::Ignore, 1u, pvr::types::ImageLayout::ColorAttachmentOptimal, pvr::types::ImageLayout::ColorAttachmentOptimal));
	renderPassInfo.setColorInfo(3, pvr::api::RenderPassColorInfo(renderpassStorageFormats[Fbo::Depth], pvr::types::LoadOp::Clear,
	                            pvr::types::StoreOp::Ignore, 1u, pvr::types::ImageLayout::ColorAttachmentOptimal, pvr::types::ImageLayout::ColorAttachmentOptimal));

	// Create on-screen-renderpass/fbo with its subpasses
	pvr::api::SubPass localMemorySubpasses[RenderPassSubPasses::NumberOfSubpasses];

	// GBuffer subpass
	localMemorySubpasses[RenderPassSubPasses::GBuffer].setColorAttachment(0, 1);
	localMemorySubpasses[RenderPassSubPasses::GBuffer].setColorAttachment(1, 2);
	localMemorySubpasses[RenderPassSubPasses::GBuffer].setColorAttachment(2, 3);
	localMemorySubpasses[RenderPassSubPasses::GBuffer].setDepthStencilAttachment(true);
	localMemorySubpasses[RenderPassSubPasses::GBuffer].setPreserveAttachment(0, 0);

	// Main scene lighting
	localMemorySubpasses[RenderPassSubPasses::Lighting].setInputAttachment(0, 1);
	localMemorySubpasses[RenderPassSubPasses::Lighting].setInputAttachment(1, 2);
	localMemorySubpasses[RenderPassSubPasses::Lighting].setInputAttachment(2, 3);
	localMemorySubpasses[RenderPassSubPasses::Lighting].setDepthStencilAttachment(true);
	localMemorySubpasses[RenderPassSubPasses::Lighting].setColorAttachment(0, 0);

	// add subpasses to the renderpass
	renderPassInfo.setSubPass(RenderPassSubPasses::GBuffer, localMemorySubpasses[RenderPassSubPasses::GBuffer]);
	renderPassInfo.setSubPass(RenderPassSubPasses::Lighting, localMemorySubpasses[RenderPassSubPasses::Lighting]);

	// add the sub pass depdendency between sub passes
	pvr::api::SubPassDependency subPassDependency;
	subPassDependency.srcStageMask = pvr::types::ShaderStageFlags::Fragment;
	subPassDependency.dstStageMask = pvr::types::ShaderStageFlags::Fragment;
	subPassDependency.srcAccessMask = pvr::types::AccessFlags::ColorAttachmentWrite | pvr::types::AccessFlags::DepthStencilAttachmentWrite;
	subPassDependency.dstAccessMask = pvr::types::AccessFlags::InputAttachmentRead | pvr::types::AccessFlags::DepthStencilAttachmentRead;
	subPassDependency.dependencyByRegion = true;

	// GBuffer -> Directional Lighting
	subPassDependency.srcSubPass = RenderPassSubPasses::GBuffer;
	subPassDependency.dstSubPass = RenderPassSubPasses::Lighting;
	renderPassInfo.addSubPassDependency(subPassDependency);

	// Create the renderpass
	m_deviceResources->onScreenLocalMemoryRenderPass = getGraphicsContext()->createRenderPass(renderPassInfo);

	// create and add the transient framebuffer attachments used as color/input attachments
	for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		pvr::api::OnScreenFboCreateParam onScreenFboCreateParam;

		// Allocate the render targets
		for (pvr::uint32 currentIndex = 0; currentIndex < Fbo::Count; ++currentIndex)
		{
			pvr::api::TextureStore transientColorAttachmentTexture = m_context->createTexture();
			transientColorAttachmentTexture->allocateTransient(renderpassStorageFormats[currentIndex], getDisplayAttributes().width, getDisplayAttributes().height);

			m_deviceResources->onScreenFboTextureViews[currentIndex].add(
			  m_context->createTextureView(transientColorAttachmentTexture));

			onScreenFboCreateParam.addOffScreenColor(m_deviceResources->onScreenFboTextureViews[currentIndex][i]);
		}

		m_deviceResources->onScreenFboCreateParams.add(onScreenFboCreateParam);
	}

	m_deviceResources->onScreenLocalMemoryFbo = m_context->createOnScreenFboSetWithRenderPass(
	      m_deviceResources->onScreenLocalMemoryRenderPass, m_deviceResources->onScreenFboCreateParams);
}

/*!*********************************************************************************************************************
\brief  Loads the mesh data required for this example into vertex buffer objects
\return Return true if no error occurred
***********************************************************************************************************************/
bool VulkanDeferredShading::loadVbos()
{
	pvr::utils::appendSingleBuffersFromModel(m_context, *m_mainScene, m_deviceResources->sceneVbos, m_deviceResources->sceneIbos);
	pvr::utils::createSingleBuffersFromMesh(m_context,
	                                        m_pointLightModel->getMesh(LightNodes::PointLightMeshNode), m_deviceResources->pointLightVbo, m_deviceResources->pointLightIbo);

	if (!m_deviceResources->sceneVbos.size() || !m_deviceResources->sceneIbos.size() ||
	    m_deviceResources->pointLightVbo.isNull() || m_deviceResources->pointLightIbo.isNull())
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
	// create the CPU side staging buffer
	// this buffer will be used as the destination for CPU side uploads/copies
	// this buffer will then be copied to the GPU side server buffer
	{
		// static materials staging buffer
		pvr::utils::StructuredMemoryView memoryView;
		memoryView.setupArray(m_context, m_mainScene->getNumMeshNodes(), pvr::types::BufferViewTypes::UniformBufferDynamic);
		memoryView.addEntryPacked(BufferEntryNames::PerModelMaterial::SpecularStrength, pvr::types::GpuDatatypes::float32);
		memoryView.addEntryPacked(BufferEntryNames::PerModelMaterial::DiffuseColor, pvr::types::GpuDatatypes::vec3);

		memoryView.createConnectedBuffer(0, m_context,
		                                 static_cast<pvr::types::BufferBindingUse>(
		                                   pvr::types::BufferBindingUse::UniformBuffer |
		                                   pvr::types::BufferBindingUse::TransferSrc),
		                                 true);
		m_deviceResources->stagingStaticModelMaterialUbo = memoryView;
	}

	// create the GPU side buffer
	{
		// static materials buffer
		auto staticModelMaterialBuffer = m_context->createBuffer(
		                                   m_deviceResources->stagingStaticModelMaterialUbo.getAlignedTotalSize(),
		                                   static_cast<pvr::types::BufferBindingUse>(
		                                     pvr::types::BufferBindingUse::UniformBuffer |
		                                     pvr::types::BufferBindingUse::TransferDest),
		                                   false);

		m_deviceResources->staticModelMaterialUbo = m_context->createBufferView(
		      staticModelMaterialBuffer, 0u, m_deviceResources->stagingStaticModelMaterialUbo.getAlignedElementSize());
	}

	{
		m_deviceResources->dynamicModelMatrixUbo.resize(getPlatformContext().getSwapChainLength());

		// create the CPU side staging buffer
		pvr::utils::StructuredMemoryView memoryView;
		memoryView.setupArray(m_context, m_mainScene->getNumMeshNodes(), pvr::types::BufferViewTypes::UniformBufferDynamic);

		memoryView.addEntryPacked(BufferEntryNames::PerModel::WorldViewProjectionMatrix, pvr::types::GpuDatatypes::mat4x4);
		memoryView.addEntryPacked(BufferEntryNames::PerModel::WorldViewMatrix, pvr::types::GpuDatatypes::mat4x4);
		memoryView.addEntryPacked(BufferEntryNames::PerModel::WorldViewITMatrix, pvr::types::GpuDatatypes::mat4x4);

		memoryView.createConnectedBuffers(getPlatformContext().getSwapChainLength(), m_context,
		                                  static_cast<pvr::types::BufferBindingUse>(
		                                    pvr::types::BufferBindingUse::UniformBuffer |
		                                    pvr::types::BufferBindingUse::TransferSrc),
		                                  true);

		// dynamic staging model matrices buffer
		m_deviceResources->stagingDynamicModelMatrixUbo = memoryView;

		// create the GPU side buffers
		for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
		{
			auto dynamicModelMatrixBuffer = m_context->createBuffer(
			                                  m_deviceResources->stagingDynamicModelMatrixUbo.getAlignedTotalSize(),
			                                  static_cast<pvr::types::BufferBindingUse>(
			                                    pvr::types::BufferBindingUse::UniformBuffer |
			                                    pvr::types::BufferBindingUse::TransferDest),
			                                  false);

			// dynamic model matrices buffer
			m_deviceResources->dynamicModelMatrixUbo[i] = m_context->createBufferView(
			      dynamicModelMatrixBuffer, 0u, m_deviceResources->stagingDynamicModelMatrixUbo.getAlignedElementSize());
		}
	}
}

/*!*********************************************************************************************************************
\brief  Creates the buffers used for rendering the directional lighting
***********************************************************************************************************************/
void VulkanDeferredShading::createDirectionalLightingBuffers()
{
	// create the CPU side staging buffer
	{
		// static staging buffer
		pvr::utils::StructuredMemoryView memoryView;
		memoryView.setupArray(m_context, m_numberOfDirectionalLights, pvr::types::BufferViewTypes::UniformBufferDynamic);
		memoryView.addEntryPacked(BufferEntryNames::PerDirectionalLight::LightIntensity, pvr::types::GpuDatatypes::vec4);

		memoryView.createConnectedBuffer(0, m_context,
		                                 static_cast<pvr::types::BufferBindingUse>(
		                                   pvr::types::BufferBindingUse::UniformBuffer |
		                                   pvr::types::BufferBindingUse::TransferSrc),
		                                 true);
		m_deviceResources->stagingStaticDirectionalLightUbo = memoryView;
	}

	// create the GPU side buffer
	{
		// static buffer
		auto staticDirectionalLightUbo = m_context->createBuffer(
		                                   m_deviceResources->stagingStaticDirectionalLightUbo.getAlignedTotalSize(),
		                                   static_cast<pvr::types::BufferBindingUse>(
		                                     pvr::types::BufferBindingUse::UniformBuffer |
		                                     pvr::types::BufferBindingUse::TransferDest),
		                                   false);

		m_deviceResources->staticDirectionalLightUbo = m_context->createBufferView(
		      staticDirectionalLightUbo, 0u, m_deviceResources->stagingStaticDirectionalLightUbo.getAlignedElementSize());
	}


	// create CPU mappable staging buffer
	{
		m_deviceResources->dynamicDirectionalLightUbo.resize(getPlatformContext().getSwapChainLength());

		pvr::utils::StructuredMemoryView memoryView;
		memoryView.setupArray(m_context, m_numberOfDirectionalLights, pvr::types::BufferViewTypes::UniformBufferDynamic);
		memoryView.addEntryPacked(BufferEntryNames::PerDirectionalLight::LightViewDirection, pvr::types::GpuDatatypes::vec4);

		memoryView.createConnectedBuffers(getPlatformContext().getSwapChainLength(), m_context,
		                                  static_cast<pvr::types::BufferBindingUse>(
		                                    pvr::types::BufferBindingUse::UniformBuffer |
		                                    pvr::types::BufferBindingUse::TransferSrc),
		                                  true);
		m_deviceResources->stagingDynamicDirectionalLightUbo = memoryView;

		for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
		{
			// create the GPU side buffer
			auto dynamicDirectionalLightUbo = m_context->createBuffer(
			                                    m_deviceResources->stagingDynamicDirectionalLightUbo.getAlignedTotalSize(),
			                                    static_cast<pvr::types::BufferBindingUse>(
			                                      pvr::types::BufferBindingUse::UniformBuffer |
			                                      pvr::types::BufferBindingUse::TransferDest),
			                                    false);

			m_deviceResources->dynamicDirectionalLightUbo[i] = m_context->createBufferView(
			      dynamicDirectionalLightUbo, 0u, m_deviceResources->stagingDynamicDirectionalLightUbo.getAlignedElementSize());
		}
	}
}

/*!*********************************************************************************************************************
\brief  Creates the buffers used for rendering the point lighting
***********************************************************************************************************************/
void VulkanDeferredShading::createPointLightBuffers()
{
	// create the CPU side staging buffer
	// this buffer will be used as the destination for CPU side uploads/copies
	// this buffer will then be copied to the GPU side server buffer
	// create static point light buffers
	{
		pvr::utils::StructuredMemoryView memoryView;
		memoryView.setupArray(m_context, m_numberOfPointLights, pvr::types::BufferViewTypes::UniformBufferDynamic);
		memoryView.addEntryPacked(BufferEntryNames::PerPointLight::LightIntensity, pvr::types::GpuDatatypes::vec4);
		memoryView.addEntryPacked(BufferEntryNames::PerPointLight::LightSourceColor, pvr::types::GpuDatatypes::vec4);

		memoryView.createConnectedBuffer(0, m_context,
		                                 static_cast<pvr::types::BufferBindingUse>(
		                                   pvr::types::BufferBindingUse::UniformBuffer |
		                                   pvr::types::BufferBindingUse::TransferSrc),
		                                 true);
		m_deviceResources->stagingStaticPointLightUbo = memoryView;
	}

	// create the GPU side buffer
	{
		// create buffer
		auto staticPointLightUbo = m_context->createBuffer(
		                             m_deviceResources->stagingStaticPointLightUbo.getAlignedTotalSize(),
		                             static_cast<pvr::types::BufferBindingUse>(
		                               pvr::types::BufferBindingUse::UniformBuffer |
		                               pvr::types::BufferBindingUse::TransferDest),
		                             false);

		m_deviceResources->staticPointLightUbo = m_context->createBufferView(
		      staticPointLightUbo, 0u, m_deviceResources->stagingStaticPointLightUbo.getAlignedElementSize());
	}

	// create point light buffers
	{
		m_deviceResources->dynamicPointLightUbo.resize(getPlatformContext().getSwapChainLength());

		pvr::utils::StructuredMemoryView memoryView;
		memoryView.setupArray(m_context, m_numberOfPointLights, pvr::types::BufferViewTypes::UniformBufferDynamic);

		memoryView.addEntryPacked(BufferEntryNames::PerPointLight::WorldViewProjectionMatrix, pvr::types::GpuDatatypes::mat4x4);
		memoryView.addEntryPacked(BufferEntryNames::PerPointLight::ProxyLightViewPosition, pvr::types::GpuDatatypes::vec4);
		memoryView.addEntryPacked(BufferEntryNames::PerPointLight::ProxyWorldViewProjectionMatrix, pvr::types::GpuDatatypes::mat4x4);
		memoryView.addEntryPacked(BufferEntryNames::PerPointLight::ProxyWorldViewMatrix, pvr::types::GpuDatatypes::mat4x4);

		memoryView.createConnectedBuffers(getPlatformContext().getSwapChainLength(), m_context,
		                                  static_cast<pvr::types::BufferBindingUse>(
		                                    pvr::types::BufferBindingUse::UniformBuffer |
		                                    pvr::types::BufferBindingUse::TransferSrc),
		                                  true);
		m_deviceResources->stagingDynamicPointLightUbo = memoryView;

		for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
		{
			// create the GPU side buffer
			auto dynamicPointLightUbo = m_context->createBuffer(
			                              m_deviceResources->stagingDynamicPointLightUbo.getAlignedTotalSize(),
			                              static_cast<pvr::types::BufferBindingUse>(
			                                pvr::types::BufferBindingUse::UniformBuffer |
			                                pvr::types::BufferBindingUse::TransferDest),
			                              false);

			m_deviceResources->dynamicPointLightUbo[i] = m_context->createBufferView(
			      dynamicPointLightUbo, 0u, m_deviceResources->stagingDynamicPointLightUbo.getAlignedElementSize());
		}
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
	// create the CPU side staging buffer
	// this buffer will be used as the destination for CPU side uploads/copies
	// this buffer will then be copied to the GPU side server buffer
	{
		pvr::utils::StructuredMemoryView memoryView;
		memoryView.setupArray(m_context, 1u, pvr::types::BufferViewTypes::UniformBuffer);
		memoryView.addEntryPacked(BufferEntryNames::PerScene::FarClipDistance, pvr::types::GpuDatatypes::float32);
		memoryView.createConnectedBuffer(0, m_context,
		                                 static_cast<pvr::types::BufferBindingUse>(
		                                   pvr::types::BufferBindingUse::UniformBuffer |
		                                   pvr::types::BufferBindingUse::TransferSrc),
		                                 false);
		m_deviceResources->stagingStaticFarClipDistanceUbo = memoryView;
	}

	// create the GPU side buffer
	{
		auto buffer = m_context->createBuffer(
		                m_deviceResources->stagingStaticFarClipDistanceUbo.getAlignedTotalSize(),
		                static_cast<pvr::types::BufferBindingUse>(
		                  pvr::types::BufferBindingUse::UniformBuffer |
		                  pvr::types::BufferBindingUse::TransferDest),
		                false);

		m_deviceResources->staticFarClipDistanceUbo = m_context->createBufferView(
		      buffer, 0u, m_deviceResources->stagingStaticFarClipDistanceUbo.getAlignedElementSize());
	}
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
	m_farClipDistance = m_mainScene->getCamera(0).getFar();
	m_deviceResources->stagingStaticFarClipDistanceUbo.map(0, pvr::types::MapBufferFlags::Write);
	m_deviceResources->stagingStaticFarClipDistanceUbo.setValue(BufferEntryNames::PerScene::FarClipDistance, m_farClipDistance);
	m_deviceResources->stagingStaticFarClipDistanceUbo.unmap(0);
}

/*!*********************************************************************************************************************
\brief  Upload the static data to the buffers which do not change per frame
***********************************************************************************************************************/
void VulkanDeferredShading::uploadStaticModelData()
{
	// static model buffer
	m_deviceResources->stagingStaticModelMaterialUbo.mapMultipleArrayElements(0, 0, m_mainScene->getNumMeshNodes(), pvr::types::MapBufferFlags::Write);
	for (pvr::uint32 i = 0; i < m_mainScene->getNumMeshNodes(); ++i)
	{
		m_deviceResources->stagingStaticModelMaterialUbo.setArrayValue(
		  BufferEntryNames::PerModelMaterial::SpecularStrength, i, m_deviceResources->materials[i].specularStrength);

		m_deviceResources->stagingStaticModelMaterialUbo.setArrayValue(
		  BufferEntryNames::PerModelMaterial::DiffuseColor, i, m_deviceResources->materials[i].diffuseColor);
	}
	m_deviceResources->stagingStaticModelMaterialUbo.unmap(0);
}

/*!*********************************************************************************************************************
\brief  Upload the static data to the buffers which do not change per frame
***********************************************************************************************************************/
void VulkanDeferredShading::uploadStaticDirectionalLightData()
{
	// static directional lighting buffer
	m_deviceResources->stagingStaticDirectionalLightUbo.mapMultipleArrayElements(0, 0, m_numberOfDirectionalLights, pvr::types::MapBufferFlags::Write);
	for (pvr::uint32 i = 0; i < m_numberOfDirectionalLights; ++i)
	{
		m_deviceResources->stagingStaticDirectionalLightUbo.setArrayValue(
		  BufferEntryNames::PerDirectionalLight::LightIntensity, i, m_deviceResources->renderInfo.directionalLightPass.lightProperties[i].lightIntensity);
	}
	m_deviceResources->stagingStaticDirectionalLightUbo.unmap(0);
}

/*!*********************************************************************************************************************
\brief  Upload the static data to the buffers which do not change per frame
***********************************************************************************************************************/
void VulkanDeferredShading::uploadStaticPointLightData()
{
	// static point lighting buffer
	m_deviceResources->stagingStaticPointLightUbo.mapMultipleArrayElements(0, 0, m_numberOfPointLights, pvr::types::MapBufferFlags::Write);
	for (pvr::uint32 i = 0; i < m_numberOfPointLights; ++i)
	{
		m_deviceResources->stagingStaticPointLightUbo.setArrayValue(
		  BufferEntryNames::PerPointLight::LightIntensity, i, m_deviceResources->renderInfo.pointLightPasses.lightProperties[i].lightIntensity);

		m_deviceResources->stagingStaticPointLightUbo.setArrayValue(
		  BufferEntryNames::PerPointLight::LightSourceColor, i, m_deviceResources->renderInfo.pointLightPasses.lightProperties[i].lightColor);
	}
	m_deviceResources->stagingStaticPointLightUbo.unmap(0);
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

	// Now copy from the staging buffers to the GPU side buffers
	m_deviceResources->cmdBufferStaticBufferUpload->beginRecording();

	// copy from staging to GPU side buffers
	m_deviceResources->cmdBufferStaticBufferUpload->copyBuffer(m_deviceResources->stagingStaticFarClipDistanceUbo.getConnectedBuffer(0)->getResource(),
	    m_deviceResources->staticFarClipDistanceUbo->getResource(), 0u, 0u, m_deviceResources->stagingStaticFarClipDistanceUbo.getAlignedTotalSize());

	m_deviceResources->cmdBufferStaticBufferUpload->copyBuffer(m_deviceResources->stagingStaticModelMaterialUbo.getConnectedBuffer(0)->getResource(),
	    m_deviceResources->staticModelMaterialUbo->getResource(), 0u, 0u, m_deviceResources->stagingStaticModelMaterialUbo.getAlignedTotalSize());

	m_deviceResources->cmdBufferStaticBufferUpload->copyBuffer(m_deviceResources->stagingStaticDirectionalLightUbo.getConnectedBuffer(0)->getResource(),
	    m_deviceResources->staticDirectionalLightUbo->getResource(), 0u, 0u, m_deviceResources->stagingStaticDirectionalLightUbo.getAlignedTotalSize());

	m_deviceResources->cmdBufferStaticBufferUpload->copyBuffer(m_deviceResources->stagingStaticPointLightUbo.getConnectedBuffer(0)->getResource(),
	    m_deviceResources->staticPointLightUbo->getResource(), 0u, 0u, m_deviceResources->stagingStaticPointLightUbo.getAlignedTotalSize());


	// add buffer range barriers
	pvr::api::MemoryBarrierSet barriers;
	barriers.addBarrier(pvr::api::BufferRangeBarrier(pvr::types::AccessFlags::HostWrite, pvr::types::AccessFlags::ShaderRead,
	                    m_deviceResources->staticFarClipDistanceUbo->getResource(), 0u, m_deviceResources->stagingStaticFarClipDistanceUbo.getAlignedTotalSize()));

	barriers.addBarrier(pvr::api::BufferRangeBarrier(pvr::types::AccessFlags::HostWrite, pvr::types::AccessFlags::ShaderRead,
	                    m_deviceResources->staticModelMaterialUbo->getResource(), 0u, m_deviceResources->stagingStaticModelMaterialUbo.getAlignedTotalSize()));

	barriers.addBarrier(pvr::api::BufferRangeBarrier(pvr::types::AccessFlags::HostWrite, pvr::types::AccessFlags::ShaderRead,
	                    m_deviceResources->staticDirectionalLightUbo->getResource(), 0u, m_deviceResources->stagingStaticDirectionalLightUbo.getAlignedTotalSize()));

	barriers.addBarrier(pvr::api::BufferRangeBarrier(pvr::types::AccessFlags::HostWrite, pvr::types::AccessFlags::ShaderRead,
	                    m_deviceResources->staticPointLightUbo->getResource(), 0u, m_deviceResources->stagingStaticPointLightUbo.getAlignedTotalSize()));


	// add the pipeline barrier to the command buffer
	m_deviceResources->cmdBufferStaticBufferUpload->pipelineBarrier(
	  pvr::types::PipelineStageFlags::AllCommands, pvr::types::PipelineStageFlags::AllCommands, barriers);

	m_deviceResources->cmdBufferStaticBufferUpload->endRecording();

	// submit and wait on the fence
	pvr::api::Fence fence = m_context->createFence(false);
	m_deviceResources->cmdBufferStaticBufferUpload->submit(pvr::api::Semaphore(), pvr::api::Semaphore(), fence);
	fence->wait();
}

/*!*********************************************************************************************************************
\brief  Record the command buffers used for updating the dynamic buffers
***********************************************************************************************************************/
void VulkanDeferredShading::recordUpdateDynamicBuffersCommandBuffer(pvr::uint32 swapIndex)
{
	// copy from staging to GPU side buffers
	m_deviceResources->cmdBufferMain[swapIndex]->copyBuffer(
	  m_deviceResources->stagingDynamicModelMatrixUbo.getConnectedBuffer(swapIndex)->getResource(),
	  m_deviceResources->dynamicModelMatrixUbo[swapIndex]->getResource(),
	  0u, 0u, m_deviceResources->stagingDynamicModelMatrixUbo.getAlignedTotalSize());

	// copy from staging to GPU side buffers
	m_deviceResources->cmdBufferMain[swapIndex]->copyBuffer(
	  m_deviceResources->stagingDynamicDirectionalLightUbo.getConnectedBuffer(swapIndex)->getResource(),
	  m_deviceResources->dynamicDirectionalLightUbo[swapIndex]->getResource(),
	  0u, 0u, m_deviceResources->stagingDynamicDirectionalLightUbo.getAlignedTotalSize());

	// copy from staging to GPU side buffers
	m_deviceResources->cmdBufferMain[swapIndex]->copyBuffer(
	  m_deviceResources->stagingDynamicPointLightUbo.getConnectedBuffer(swapIndex)->getResource(),
	  m_deviceResources->dynamicPointLightUbo[swapIndex]->getResource(),
	  0u, 0u, m_deviceResources->stagingDynamicPointLightUbo.getAlignedTotalSize());

	// Use a Buffer range barrier

	// add buffer range barriers
	pvr::api::MemoryBarrierSet barriers;
	barriers.addBarrier(pvr::api::BufferRangeBarrier(
	                      pvr::types::AccessFlags::HostWrite, pvr::types::AccessFlags::ShaderRead,
	                      m_deviceResources->dynamicModelMatrixUbo[swapIndex]->getResource(),
	                      0u, m_deviceResources->stagingDynamicModelMatrixUbo.getAlignedTotalSize()));

	barriers.addBarrier(pvr::api::BufferRangeBarrier(
	                      pvr::types::AccessFlags::HostWrite, pvr::types::AccessFlags::ShaderRead,
	                      m_deviceResources->dynamicDirectionalLightUbo[swapIndex]->getResource(),
	                      0u, m_deviceResources->stagingDynamicDirectionalLightUbo.getAlignedTotalSize()));

	barriers.addBarrier(pvr::api::BufferRangeBarrier(
	                      pvr::types::AccessFlags::HostWrite, pvr::types::AccessFlags::ShaderRead,
	                      m_deviceResources->dynamicPointLightUbo[swapIndex]->getResource(),
	                      0u, m_deviceResources->stagingDynamicPointLightUbo.getAlignedTotalSize()));

	// add the pipeline barrier to the command buffer
	m_deviceResources->cmdBufferMain[swapIndex]->pipelineBarrier(
	  pvr::types::PipelineStageFlags::AllCommands, pvr::types::PipelineStageFlags::AllCommands, barriers);
}

/*!*********************************************************************************************************************
\brief  Update the CPU visible buffers containing dynamic data
***********************************************************************************************************************/
void VulkanDeferredShading::updateDynamicSceneData()
{
	RenderData& pass = m_deviceResources->renderInfo;

	m_deviceResources->stagingDynamicModelMatrixUbo.mapMultipleArrayElements(m_deviceResources->swapIndex, 0, m_mainScene->getNumMeshNodes(), pvr::types::MapBufferFlags::Write);

	// update the model matrices
	for (pvr::uint32 i = 0; i < m_mainScene->getNumMeshNodes(); ++i)
	{
		const pvr::assets::Model::Node& node = m_mainScene->getNode(i);
		pass.storeLocalMemoryPass.objects[i].world = m_mainScene->getWorldMatrix(node.getObjectId());
		pass.storeLocalMemoryPass.objects[i].worldView = m_viewMatrix * pass.storeLocalMemoryPass.objects[i].world;
		pass.storeLocalMemoryPass.objects[i].worldViewProj = m_viewProjectionMatrix * pass.storeLocalMemoryPass.objects[i].world;
		pass.storeLocalMemoryPass.objects[i].worldViewIT4x4 = glm::inverseTranspose(pass.storeLocalMemoryPass.objects[i].worldView);

		m_deviceResources->stagingDynamicModelMatrixUbo.setArrayValue(
		  BufferEntryNames::PerModel::WorldViewMatrix, i, pass.storeLocalMemoryPass.objects[i].worldView);

		m_deviceResources->stagingDynamicModelMatrixUbo.setArrayValue(
		  BufferEntryNames::PerModel::WorldViewProjectionMatrix, i, pass.storeLocalMemoryPass.objects[i].worldViewProj);

		m_deviceResources->stagingDynamicModelMatrixUbo.setArrayValue(
		  BufferEntryNames::PerModel::WorldViewITMatrix, i, pass.storeLocalMemoryPass.objects[i].worldViewIT4x4);
	}

	m_deviceResources->stagingDynamicModelMatrixUbo.unmap(m_deviceResources->swapIndex);

	pvr::int32 pointLight = 0;
	pvr::uint32 directionalLight = 0;

	// update the lighting data
	for (pvr::uint32 i = 0; i < m_mainScene->getNumLightNodes(); ++i)
	{
		const pvr::assets::Node& lightNode = m_mainScene->getLightNode(i);
		const pvr::assets::Light& light = m_mainScene->getLight(lightNode.getObjectId());
		switch (light.getType())
		{
		case pvr::assets::Light::Point:
		{
			if (pointLight >= PointLightConfiguration::MaxScenePointLights) { continue; }

			const glm::mat4& transMtx = m_mainScene->getWorldMatrix(m_mainScene->getNodeIdFromLightNodeId(i));
			const glm::mat4& proxyScale = glm::scale(glm::vec3(PointLightConfiguration::PointLightScale)) * PointLightConfiguration::PointlightIntensity;
			const glm::mat4 mWorldScale = transMtx * proxyScale;

			//POINT LIGHT GEOMETRY : The spheres that will be used for the stencil pass
			pass.pointLightPasses.lightProperties[pointLight].proxyWorldViewProjectionMatrix = m_viewProjectionMatrix * mWorldScale;

			//POINT LIGHT PROXIES : The "drawcalls" that will perform the actual rendering
			pass.pointLightPasses.lightProperties[pointLight].proxyWorldViewMatrix = m_viewMatrix * mWorldScale;
			pass.pointLightPasses.lightProperties[pointLight].proxyViewSpaceLightPosition = glm::vec4((m_viewMatrix * transMtx)[3]); //Translation component of the view matrix

			//POINT LIGHT SOURCES : The little balls that we render to show the lights
			pass.pointLightPasses.lightProperties[pointLight].worldViewProjectionMatrix = m_viewProjectionMatrix * transMtx;
			++pointLight;
		}
		break;
		case pvr::assets::Light::Directional:
		{
			const glm::mat4& transMtx = m_mainScene->getWorldMatrix(m_mainScene->getNodeIdFromLightNodeId(i));
			pass.directionalLightPass.lightProperties[directionalLight].viewSpaceLightDirection = m_viewMatrix * transMtx * glm::vec4(0.f, -1.f, 0.f, 0.f);
			++directionalLight;
		}
		break;
		}
	}
	int numSceneLights = pointLight;
	if (DirectionalLightConfiguration::AdditionalDirectionalLight)
	{
		pass.directionalLightPass.lightProperties[directionalLight].viewSpaceLightDirection = m_viewMatrix * glm::vec4(0.f, -1.f, 0.f, 0.f);
		++directionalLight;
	}

	for (; pointLight < numSceneLights + PointLightConfiguration::NumProceduralPointLights; ++pointLight)
	{
		updateProceduralPointLight(pass.pointLightPasses.initialData[pointLight],
		                           m_deviceResources->renderInfo.pointLightPasses.lightProperties[pointLight], false);
	}

	// directional Light data
	m_deviceResources->stagingDynamicDirectionalLightUbo.mapMultipleArrayElements(m_deviceResources->swapIndex, 0, m_numberOfDirectionalLights, pvr::types::MapBufferFlags::Write);

	for (pvr::uint32 i = 0; i < m_numberOfDirectionalLights; ++i)
	{
		m_deviceResources->stagingDynamicDirectionalLightUbo.setArrayValue(
		  BufferEntryNames::PerDirectionalLight::LightViewDirection, i, m_deviceResources->renderInfo.directionalLightPass.lightProperties[i].viewSpaceLightDirection);
	}

	m_deviceResources->stagingDynamicDirectionalLightUbo.unmap(m_deviceResources->swapIndex);


	m_deviceResources->stagingDynamicPointLightUbo.mapMultipleArrayElements(m_deviceResources->swapIndex, 0, m_numberOfPointLights, pvr::types::MapBufferFlags::Write);

	for (pvr::uint32 i = 0; i < m_numberOfPointLights; ++i)
	{
		m_deviceResources->stagingDynamicPointLightUbo.setArrayValue(
		  BufferEntryNames::PerPointLight::ProxyWorldViewProjectionMatrix, i, m_deviceResources->renderInfo.pointLightPasses.lightProperties[i].proxyWorldViewProjectionMatrix);
		m_deviceResources->stagingDynamicPointLightUbo.setArrayValue(
		  BufferEntryNames::PerPointLight::ProxyWorldViewMatrix, i, m_deviceResources->renderInfo.pointLightPasses.lightProperties[i].proxyWorldViewMatrix);
		m_deviceResources->stagingDynamicPointLightUbo.setArrayValue(
		  BufferEntryNames::PerPointLight::ProxyLightViewPosition, i, m_deviceResources->renderInfo.pointLightPasses.lightProperties[i].proxyViewSpaceLightPosition);
		m_deviceResources->stagingDynamicPointLightUbo.setArrayValue(
		  BufferEntryNames::PerPointLight::WorldViewProjectionMatrix, i, m_deviceResources->renderInfo.pointLightPasses.lightProperties[i].worldViewProjectionMatrix);
	}

	m_deviceResources->stagingDynamicPointLightUbo.unmap(m_deviceResources->swapIndex);
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
		data.distance = pvr::randomrange(PointLightConfiguration::LightMinDistance, PointLightConfiguration::LightMaxDistance);
		data.angle = pvr::randomrange(-glm::pi<pvr::float32>(), glm::pi<pvr::float32>());
		data.height = pvr::randomrange(PointLightConfiguration::LightMinHeight, PointLightConfiguration::LightMaxHeight);
		data.axial_vel = pvr::randomrange(-PointLightConfiguration::LightMaxAxialVelocity, PointLightConfiguration::LightMaxAxialVelocity);
		data.radial_vel = pvr::randomrange(-PointLightConfiguration::LightMaxRadialVelocity, PointLightConfiguration::LightMaxRadialVelocity);
		data.vertical_vel = pvr::randomrange(-PointLightConfiguration::LightMaxVerticalVelocity, PointLightConfiguration::LightMaxVerticalVelocity);

		glm::vec3 lightColor = glm::vec3(pvr::randomrange(0, 1), pvr::randomrange(0, 1), pvr::randomrange(0, 1));
		lightColor / glm::max(glm::max(lightColor.x, lightColor.y), lightColor.z); //Have at least one component equal to 1... We want them bright-ish...
		pointLightProperties.lightColor = glm::vec4(lightColor, 1.);//random-looking
		pointLightProperties.lightIntensity = glm::vec4(lightColor, 1.0f) * PointLightConfiguration::PointlightIntensity;
	}

	if (!initial && !m_isPaused) //Skip for the first m_frameNumber, as sometimes this moves the light too far...
	{
		pvr::float32 dt = (pvr::float32)std::min(getFrameTime(), 30ull);
		if (data.distance < PointLightConfiguration::LightMinDistance)
		{
			data.axial_vel = glm::abs(data.axial_vel) + (PointLightConfiguration::LightMaxAxialVelocity * dt * .001f);
		}
		if (data.distance > PointLightConfiguration::LightMaxDistance)
		{
			data.axial_vel = -glm::abs(data.axial_vel) - (PointLightConfiguration::LightMaxAxialVelocity * dt * .001f);
		}
		if (data.height < PointLightConfiguration::LightMinHeight)
		{
			data.vertical_vel = glm::abs(data.vertical_vel) + (PointLightConfiguration::LightMaxAxialVelocity * dt * .001f);
		}
		if (data.height > PointLightConfiguration::LightMaxHeight)
		{
			data.vertical_vel = -glm::abs(data.vertical_vel) - (PointLightConfiguration::LightMaxAxialVelocity * dt * .001f);
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
	pointLightProperties.proxyWorldViewProjectionMatrix = m_viewProjectionMatrix * mWorldScale;

	//POINT LIGHT PROXIES : The "drawcalls" that will perform the actual rendering
	pointLightProperties.proxyWorldViewMatrix = m_viewMatrix * mWorldScale;
	pointLightProperties.proxyViewSpaceLightPosition = glm::vec4((m_viewMatrix * transMtx)[3]); //Translation component of the view matrix

	//POINT LIGHT SOURCES : The little balls that we render to show the lights
	pointLightProperties.worldViewProjectionMatrix = m_viewProjectionMatrix * transMtx;
}

/*!*********************************************************************************************************************
\brief  Updates animation variables and camera matrices.
***********************************************************************************************************************/
void VulkanDeferredShading::updateAnimation()
{
	pvr::uint64 deltaTime = getFrameTime();

	if (!m_isPaused)
	{
		m_frameNumber += deltaTime * ApplicationConfiguration::FrameRate;
		if (m_frameNumber > m_mainScene->getNumFrames() - 1) { m_frameNumber = 0; }
		m_mainScene->setCurrentFrame(m_frameNumber);
	}

	glm::vec3 vTo, vUp;
	pvr::float32 fov;
	m_mainScene->getCameraProperties(m_cameraId, fov, m_cameraPosition, vTo, vUp);

	// Update camera matrices
	static float angle = 0;
	if (m_animateCamera) { angle += getFrameTime() / 1000.f; }
	m_viewMatrix = glm::lookAt(glm::vec3(sin(angle) * 100.f + vTo.x, vTo.y + 30., cos(angle) * 100.f + vTo.z), vTo, vUp);
	m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
	m_inverseViewMatrix = glm::inverse(m_viewMatrix);
}

/*!*********************************************************************************************************************
\brief  Records main command buffer
***********************************************************************************************************************/
void VulkanDeferredShading::recordMainCommandBuffer()
{
	for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		m_deviceResources->cmdBufferMain[i]->beginRecording();

		// update the dynamic buffers
		recordUpdateDynamicBuffersCommandBuffer(i);

		pvr::Rectanglei renderArea(0, 0, m_windowWidth, m_windowHeight);

		// specify a clear color per attachment
		const pvr::uint32 numClearColors = Fbo::Count + 1u;
		glm::vec4 clearColors[numClearColors];
		clearColors[0] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		clearColors[1] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		clearColors[2] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		clearColors[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		pvr::float32 depthClear = 1.0f;
		pvr::uint32 stencilClear = 0u;

		// begin the local memory renderpass
		m_deviceResources->cmdBufferMain[i]->beginRenderPass(
		  m_deviceResources->onScreenLocalMemoryFbo[i], renderArea, false,
		  clearColors, numClearColors, depthClear, stencilClear);

		// Render the models
		m_deviceResources->cmdBufferMain[i]->enqueueSecondaryCmds(m_deviceResources->cmdBufferRenderToLocalMemory[i]);

		// Render lighting + ui render text
		m_deviceResources->cmdBufferMain[i]->nextSubPassSecondaryCmds(m_deviceResources->cmdBufferLighting[i]);

		m_deviceResources->cmdBufferMain[i]->endRenderPass();
		m_deviceResources->cmdBufferMain[i]->endRecording();
	}
}

/*!*********************************************************************************************************************
\brief  Initialise the static light properties
***********************************************************************************************************************/
void VulkanDeferredShading::initialiseStaticLightProperties()
{
	RenderData& pass = m_deviceResources->renderInfo;

	pvr::int32 pointLight = 0;
	pvr::uint32 directionalLight = 0;
	for (pvr::uint32 i = 0; i < m_mainScene->getNumLightNodes(); ++i)
	{
		const pvr::assets::Node& lightNode = m_mainScene->getLightNode(i);
		const pvr::assets::Light& light = m_mainScene->getLight(lightNode.getObjectId());
		switch (light.getType())
		{
		case pvr::assets::Light::Point:
		{
			if (pointLight >= PointLightConfiguration::MaxScenePointLights)
			{
				continue;
			}

			const glm::mat4& transMtx = m_mainScene->getWorldMatrix(m_mainScene->getNodeIdFromLightNodeId(i));
			const glm::mat4& proxyScale = glm::scale(glm::vec3(PointLightConfiguration::PointLightScale)) *
			                              PointLightConfiguration::PointlightIntensity;
			const glm::mat4 mWorldScale = transMtx * proxyScale;

			//POINT LIGHT GEOMETRY : The spheres that will be used for the stencil pass
			pass.pointLightPasses.lightProperties[pointLight].lightColor = glm::vec4(light.getColor(), 1.f);

			//POINT LIGHT PROXIES : The "drawcalls" that will perform the actual rendering
			pass.pointLightPasses.lightProperties[pointLight].lightIntensity = glm::vec4(light.getColor(), 1.0f) *
			    PointLightConfiguration::PointlightIntensity;

			//POINT LIGHT SOURCES : The little balls that we render to show the lights
			pass.pointLightPasses.lightProperties[pointLight].lightSourceColor = glm::vec4(light.getColor(), .8f);
			++pointLight;
		}
		break;
		case pvr::assets::Light::Directional:
		{
			const glm::mat4& transMtx = m_mainScene->getWorldMatrix(m_mainScene->getNodeIdFromLightNodeId(i));
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
	pvr::int32 countPoint = 0;
	pvr::uint32 countDirectional = 0;
	for (pvr::uint32 i = 0; i < m_mainScene->getNumLightNodes(); ++i)
	{
		switch (m_mainScene->getLight(m_mainScene->getLightNode(i).getObjectId()).getType())
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

	m_numberOfPointLights = countPoint;
	m_numberOfDirectionalLights = countDirectional;

	m_deviceResources->renderInfo.directionalLightPass.lightProperties.resize(countDirectional);
	m_deviceResources->renderInfo.pointLightPasses.lightProperties.resize(countPoint);
	m_deviceResources->renderInfo.pointLightPasses.initialData.resize(countPoint);

	for (int i = countPoint - PointLightConfiguration::NumProceduralPointLights; i < countPoint; ++i)
	{
		updateProceduralPointLight(m_deviceResources->renderInfo.pointLightPasses.initialData[i], m_deviceResources->renderInfo.pointLightPasses.lightProperties[i], true);
	}
}

/*!*********************************************************************************************************************
\brief  Record all the secondary command buffers
***********************************************************************************************************************/
void VulkanDeferredShading::recordSecondaryCommandBuffers()
{
	for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		m_deviceResources->cmdBufferRenderToLocalMemory[i]->beginRecording(m_deviceResources->onScreenLocalMemoryFbo[i], RenderPassSubPasses::GBuffer);
		recordCommandBufferRenderGBuffer(m_deviceResources->cmdBufferRenderToLocalMemory[i], i, RenderPassSubPasses::GBuffer);
		m_deviceResources->cmdBufferRenderToLocalMemory[i]->endRecording();

		m_deviceResources->cmdBufferLighting[i]->beginRecording(m_deviceResources->onScreenLocalMemoryFbo[i], RenderPassSubPasses::Lighting);
		recordCommandsDirectionalLights(m_deviceResources->cmdBufferLighting[i], i, RenderPassSubPasses::Lighting);
		recordCommandsPointLightGeometryStencil(m_deviceResources->cmdBufferLighting[i], i, RenderPassSubPasses::Lighting);
		recordCommandsPointLightProxy(m_deviceResources->cmdBufferLighting[i], i, RenderPassSubPasses::Lighting);
		recordCommandsPointLightSourceLighting(m_deviceResources->cmdBufferLighting[i], i, RenderPassSubPasses::Lighting);
		recordCommandUIRenderer(m_deviceResources->cmdBufferLighting[i], i, RenderPassSubPasses::UIRenderer);
		m_deviceResources->cmdBufferLighting[i]->endRecording();
	}
}


/*!*********************************************************************************************************************
\brief Record rendering G-Buffer commands
\param cmdBuffer SecondaryCommandbuffer to record
\param swapChainIndex Current swap chain index
\param subpass Current sub pass
***********************************************************************************************************************/
void VulkanDeferredShading::recordCommandBufferRenderGBuffer(pvr::api::SecondaryCommandBuffer& cmdBuffer, pvr::uint32 swapChainIndex, pvr::uint32 subpass)
{
	DrawGBuffer& pass = m_deviceResources->renderInfo.storeLocalMemoryPass;

	cmdBuffer->bindDescriptorSet(m_deviceResources->scenePipelineLayout, 0u, m_deviceResources->sceneDescriptorSet);

	for (pvr::uint32 i = 0; i < m_mainScene->getNumMeshNodes(); ++i)
	{
		cmdBuffer->bindPipeline(pass.objects[i].pipeline);

		// set stencil reference to 1
		cmdBuffer->setStencilReference(pvr::types::StencilFace::FrontBack, 1);

		// enable stencil writing
		cmdBuffer->setStencilWriteMask(pvr::types::StencilFace::FrontBack, 0xFF);

		const pvr::assets::Model::Node& node = m_mainScene->getNode(i);
		const pvr::assets::Mesh& mesh = m_mainScene->getMesh(node.getObjectId());

		const Material& material = m_deviceResources->materials[node.getMaterialIndex()];

		pvr::uint32 offsets[2];
		offsets[0] = m_deviceResources->stagingStaticModelMaterialUbo.getAlignedElementArrayOffset(i);
		offsets[1] = m_deviceResources->stagingDynamicModelMatrixUbo.getAlignedElementArrayOffset(i);

		cmdBuffer->bindDescriptorSet(
		  pass.objects[i].pipeline->getPipelineLayout(), 1u,
		  material.materialDescriptorSet[swapChainIndex],
		  offsets, 2u);

		cmdBuffer->bindVertexBuffer(m_deviceResources->sceneVbos[node.getObjectId()], 0, 0);
		cmdBuffer->bindIndexBuffer(m_deviceResources->sceneIbos[node.getObjectId()], 0, mesh.getFaces().getDataType());
		cmdBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
	}

}

/*!*********************************************************************************************************************
\brief  Record UIRenderer commands
\param  cmdBuff Commandbuffer to record
***********************************************************************************************************************/
void VulkanDeferredShading::recordCommandUIRenderer(pvr::api::SecondaryCommandBuffer& cmdBuff, pvr::uint32 swapChainIndex, pvr::uint32 subpass)
{
	m_deviceResources->uiRenderer.beginRendering(cmdBuff);
	m_deviceResources->uiRenderer.getDefaultTitle()->render();
	m_deviceResources->uiRenderer.getDefaultControls()->render();
	m_deviceResources->uiRenderer.getSdkLogo()->render();
	m_deviceResources->uiRenderer.endRendering();
}

/*!*********************************************************************************************************************
\brief  Record directional light draw commands
\param  cmdBuffer SecondaryCommandBuffer to record
\param swapChainIndex Current swap chain index
\param subpass Current sub pass
***********************************************************************************************************************/
void VulkanDeferredShading::recordCommandsDirectionalLights(pvr::api::SecondaryCommandBuffer& cmdBuffer, pvr::uint32 swapChainIndex, pvr::uint32 subpass)
{
	DrawDirectionalLight& directionalPass = m_deviceResources->renderInfo.directionalLightPass;

	cmdBuffer->bindPipeline(directionalPass.pipeline);

	// if for the current fragment the stencil has been filled then there is geometry present
	// and directional lighting calculations should be carried out
	cmdBuffer->setStencilReference(pvr::types::StencilFace::FrontBack, 1);

	// disable stencil writing
	cmdBuffer->setStencilWriteMask(pvr::types::StencilFace::FrontBack, 0x00);

	// keep the descriptor set bound even though for this pass we don't need it
	// avoids unbinding before rebinding in the next passes
	cmdBuffer->bindDescriptorSet(m_deviceResources->scenePipelineLayout, 0u, m_deviceResources->sceneDescriptorSet);

	// Make use of the stencil buffer contents to only shade pixels where actual geometry is located.
	// Reset the stencil buffer to 0 at the same time to avoid the stencil clear operation afterwards.
	// bind the albedo and normal textures from the gbuffer
	for (pvr::uint32 i = 0; i < m_numberOfDirectionalLights; i++)
	{
		pvr::uint32 offsets[2] = {};
		offsets[0] = m_deviceResources->stagingStaticDirectionalLightUbo.getAlignedElementArrayOffset(i);
		offsets[1] = m_deviceResources->stagingDynamicDirectionalLightUbo.getAlignedElementArrayOffset(i);

		cmdBuffer->bindDescriptorSet(
		  directionalPass.pipeline->getPipelineLayout(), 1u,
		  m_deviceResources->directionalLightingDescriptorSets[swapChainIndex],
		  offsets, 2u);

		// Draw a quad
		cmdBuffer->drawArrays(0, 4);
	}
}

/*!*********************************************************************************************************************
\brief  Record point light stencil commands
\param  cmdBuffer SecondaryCommandBuffer to record
\param swapChainIndex Current swap chain index
\param subpass Current sub pass
***********************************************************************************************************************/
void VulkanDeferredShading::recordCommandsPointLightGeometryStencil(pvr::api::SecondaryCommandBuffer& cmdBuffer, pvr::uint32 swapChainIndex, pvr::uint32 subpass)
{
	PointLightGeometryStencil& pointGeometryStencilPass = m_deviceResources->renderInfo.pointLightGeometryStencilPass;
	PointLightPasses& pointPasses = m_deviceResources->renderInfo.pointLightPasses;

	const pvr::assets::Mesh& mesh = m_pointLightModel->getMesh(LightNodes::PointLightMeshNode);

	pvr::Rectanglei renderArea(0, 0, m_framebufferWidth, m_framebufferHeight);
	if ((m_framebufferWidth != m_windowWidth) || (m_framebufferHeight != m_windowHeight))
	{
		renderArea = pvr::Rectanglei(m_viewportOffsets[0], m_viewportOffsets[1], m_framebufferWidth, m_framebufferHeight);
	}

	// clear stencil to 0's to make use of it again for point lights
	cmdBuffer->clearStencilAttachment(renderArea, 0);

	cmdBuffer->bindDescriptorSet(m_deviceResources->scenePipelineLayout, 0u, m_deviceResources->sceneDescriptorSet);

	cmdBuffer->setStencilReference(pvr::types::StencilFace::FrontBack, 0);

	//POINT LIGHTS: 1) Draw stencil to discard useless pixels
	cmdBuffer->bindPipeline(pointGeometryStencilPass.pipeline);
	// Bind the vertex and index buffer for the point light
	cmdBuffer->bindVertexBuffer(m_deviceResources->pointLightVbo, 0, 0);
	cmdBuffer->bindIndexBuffer(m_deviceResources->pointLightIbo, 0, pvr::types::IndexType::IndexType16Bit);

	for (pvr::uint32 i = 0; i < pointPasses.lightProperties.size(); i++)
	{
		pvr::uint32 offsets[2] = {};
		offsets[0] = m_deviceResources->stagingStaticPointLightUbo.getAlignedElementArrayOffset(i);
		offsets[1] = m_deviceResources->stagingDynamicPointLightUbo.getAlignedElementArrayOffset(i);

		cmdBuffer->bindDescriptorSet(
		  pointGeometryStencilPass.pipeline->getPipelineLayout(), 1u,
		  m_deviceResources->pointLightGeometryStencilDescriptorSets[swapChainIndex],
		  offsets, 2u);

		cmdBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
	}
}

/*!*********************************************************************************************************************
\brief  Record point light proxy commands
\param  cmdBuffer SecondaryCommandBuffer to record
\param swapChainIndex Current swap chain index
\param subpass Current sub pass
***********************************************************************************************************************/
void VulkanDeferredShading::recordCommandsPointLightProxy(pvr::api::SecondaryCommandBuffer& cmdBuffer, pvr::uint32 swapChainIndex, pvr::uint32 subpass)
{
	DrawPointLightProxy& pointLightProxyPass = m_deviceResources->renderInfo.pointLightProxyPass;
	PointLightPasses& pointPasses = m_deviceResources->renderInfo.pointLightPasses;

	const pvr::assets::Mesh& mesh = m_pointLightModel->getMesh(LightNodes::PointLightMeshNode);

	//Any of the geompointlightpass, lightsourcepointlightpass or pointlightproxiepass's uniforms have the same number of elements
	if (pointPasses.lightProperties.empty()) { return; }

	//POINT LIGHTS: 2) Lighting
	cmdBuffer->bindDescriptorSet(m_deviceResources->scenePipelineLayout, 0u, m_deviceResources->sceneDescriptorSet);

	cmdBuffer->bindPipeline(m_deviceResources->renderInfo.pointLightProxyPass.pipeline);

	// Bind the vertex and index buffer for the point light
	cmdBuffer->bindVertexBuffer(m_deviceResources->pointLightVbo, 0, 0);
	cmdBuffer->bindIndexBuffer(m_deviceResources->pointLightIbo, 0, mesh.getFaces().getDataType());

	for (pvr::uint32 i = 0; i < pointPasses.lightProperties.size(); ++i)
	{
		const pvr::uint32 numberOfOffsets = 3u;
		pvr::uint32 offsets[numberOfOffsets] = {};
		offsets[0] = m_deviceResources->stagingStaticPointLightUbo.getAlignedElementArrayOffset(i);
		offsets[1] = m_deviceResources->stagingDynamicPointLightUbo.getAlignedElementArrayOffset(i);
		offsets[2] = m_deviceResources->stagingDynamicPointLightUbo.getAlignedElementArrayOffset(i);

		cmdBuffer->bindDescriptorSet(
		  pointLightProxyPass.pipeline->getPipelineLayout(), 1u,
		  m_deviceResources->pointLightProxyDescriptorSets[swapChainIndex],
		  offsets, numberOfOffsets);

		cmdBuffer->bindDescriptorSet(pointLightProxyPass.pipeline->getPipelineLayout(), 2u,
		                             m_deviceResources->pointLightProxyLocalMemoryDescriptorSets[swapChainIndex]);

		cmdBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
	}
}

/*!*********************************************************************************************************************
\brief  Record point light source commands
\param  cmdBuffer SecondaryCommandBuffer to record
\param swapChainIndex Current swap chain index
\param subpass Current sub pass
***********************************************************************************************************************/
void VulkanDeferredShading::recordCommandsPointLightSourceLighting(pvr::api::SecondaryCommandBuffer& cmdBuffer, pvr::uint32 swapChainIndex, pvr::uint32 subpass)
{
	DrawPointLightSources& pointLightSourcePass = m_deviceResources->renderInfo.pointLightSourcesPass;
	PointLightPasses& pointPasses = m_deviceResources->renderInfo.pointLightPasses;

	const pvr::assets::Mesh& mesh = m_pointLightModel->getMesh(LightNodes::PointLightMeshNode);

	//POINT LIGHTS: 3) Light sources
	cmdBuffer->bindDescriptorSet(m_deviceResources->scenePipelineLayout, 0u, m_deviceResources->sceneDescriptorSet);

	cmdBuffer->bindPipeline(pointLightSourcePass.pipeline);
	cmdBuffer->bindVertexBuffer(m_deviceResources->pointLightVbo, 0, 0);
	cmdBuffer->bindIndexBuffer(m_deviceResources->pointLightIbo, 0, mesh.getFaces().getDataType());

	for (pvr::uint32 i = 0; i < pointPasses.lightProperties.size(); ++i)
	{
		const pvr::uint32 numberOfOffsets = 2u;

		pvr::uint32 offsets[numberOfOffsets] = {};
		offsets[0] = m_deviceResources->stagingStaticPointLightUbo.getAlignedElementArrayOffset(i);
		offsets[1] = m_deviceResources->stagingDynamicPointLightUbo.getAlignedElementArrayOffset(i);

		cmdBuffer->bindDescriptorSet(
		  pointLightSourcePass.pipeline->getPipelineLayout(), 1u,
		  m_deviceResources->pointLightSourceDescriptorSets[swapChainIndex],
		  offsets, numberOfOffsets);

		cmdBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
	}
}

/*!*********************************************************************************************************************
\return Return an auto_ptr to a new Demo class, supplied by the user
\brief  This function must be implemented by the user of the shell. The user should return its Shell object defining the
behaviour of the application.
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() { return std::auto_ptr<pvr::Shell>(new VulkanDeferredShading()); }
