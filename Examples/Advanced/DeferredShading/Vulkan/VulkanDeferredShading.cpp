/*!*********************************************************************************************************************
\File         VulkanDeferredShading.cpp
\Title        Deferred Shading
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\Description  Implements a deferred shading technique supporting point and directional lights.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVREngineUtils/PVREngineUtils.h"

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

	struct ApiObjects
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
		pvr::api::SecondaryCommandBuffer cmdBufferLighting[MAX_NUMBER_OF_SWAP_IMAGES];

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
		pvr::utils::StructuredMemoryView farClipDistanceUbo;
		// Static materials buffers
		pvr::utils::StructuredMemoryView modelMaterialUbo;
		// Dynamic matrices buffers
		pvr::utils::StructuredMemoryView modelMatrixUbo;
		// Static point light buffers
		pvr::utils::StructuredMemoryView staticPointLightUbo;
		// Dynamic point light buffer
		pvr::utils::StructuredMemoryView dynamicPointLightUbo;
		// Static Directional lighting buffer
		pvr::utils::StructuredMemoryView staticDirectionalLightUbo;
		// Dynamic Directional lighting buffers
		pvr::utils::StructuredMemoryView dynamicDirectionalLightUbo;

		//// UI Renderer ////
		pvr::ui::UIRenderer uiRenderer;


		//// Frame ////
		pvr::uint32 numSwapImages;
		pvr::uint8 swapIndex;

		RenderData renderInfo;
	};

	// Context
	pvr::GraphicsContext _context;

	//Putting all api objects into a pointer just makes it easier to release them all together with RAII
	std::auto_ptr<ApiObjects> apiObj;

	// Provides easy management of assets
	pvr::utils::AssetStore assetManager;

	// Frame counters for animation
	pvr::float32 frameNumber;
	bool isPaused;
	pvr::uint32 cameraId;
	bool animateCamera;

	pvr::uint32 numberOfPointLights;
	pvr::uint32 numberOfDirectionalLights;

	// Projection and Model View matrices
	glm::vec3 cameraPosition;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::mat4 viewProjectionMatrix;
	glm::mat4 inverseViewMatrix;
	pvr::float32 farClipDistance;

	pvr::int32 windowWidth;
	pvr::int32 windowHeight;
	pvr::int32 framebufferWidth;
	pvr::int32 framebufferHeight;

	pvr::int32 viewportOffsets[2];

	// Light models
	pvr::assets::ModelHandle pointLightModel;

	// Object model
	pvr::assets::ModelHandle mainScene;

	VulkanDeferredShading() { animateCamera = false; isPaused = false; }

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
		case pvr::SimplifiedInput::Action1: isPaused = !isPaused; break;
		case pvr::SimplifiedInput::Action2: animateCamera = !animateCamera; break;
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

	frameNumber = 0.0f;
	isPaused = false;
	cameraId = 0;

	//Prepare the asset manager for loading our objects
	assetManager.init(*this);

	//  Load the scene and the light
	if (!assetManager.loadModel(Files::SceneFile, mainScene))
	{
		setExitMessage("ERROR: Couldn't load the scene pod file %s\n", Files::SceneFile);
		return pvr::Result::UnknownError;
	}

	if (mainScene->getNumCameras() == 0)
	{
		setExitMessage("ERROR: The main scene to display must contain a camera.\n");
		return pvr::Result::InvalidData;
	}

	//  Load light proxy geometry
	if (!assetManager.loadModel(Files::PointLightModelFile, pointLightModel))
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
	apiObj.reset(new ApiObjects);

	//Initialize free-floating objects (commandBuffers).
	_context = getGraphicsContext();

	// Get the number of swap images
	apiObj->numSwapImages = getPlatformContext().getSwapChainLength();

	// Get current swap index
	apiObj->swapIndex = _context->getPlatformContext().getSwapChainIndex();

	// initialise the gbuffer renderpass list
	apiObj->renderInfo.storeLocalMemoryPass.objects.resize(mainScene->getNumMeshNodes());

	// calculate the frame buffer width and heights
	framebufferWidth = windowWidth = this->getWidth();
	framebufferHeight = windowHeight = this->getHeight();

	const pvr::platform::CommandLine& cmdOptions = getCommandLine();

	cmdOptions.getIntOption("-fbowidth", framebufferWidth);
	framebufferWidth = glm::min<pvr::int32>(framebufferWidth, windowWidth);
	cmdOptions.getIntOption("-fboheight", framebufferHeight);
	framebufferHeight = glm::min<pvr::int32>(framebufferHeight, windowHeight);
	cmdOptions.getIntOption("-numlights", PointLightConfiguration::NumProceduralPointLights);
	cmdOptions.getFloatOption("-lightscale", PointLightConfiguration::PointLightScale);
	cmdOptions.getFloatOption("-lightintensity", PointLightConfiguration::PointlightIntensity);

	viewportOffsets[0] = (windowWidth - framebufferWidth) / 2;
	viewportOffsets[1] = (windowHeight - framebufferHeight) / 2;

	pvr::Log(pvr::Log.Information, "FBO dimensions: %d x %d\n", framebufferWidth, framebufferHeight);
	pvr::Log(pvr::Log.Information, "Onscreen Framebuffer dimensions: %d x %d\n", windowWidth, windowHeight);

	// setup command buffers
	for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		// main command buffer
		apiObj->cmdBufferMain[i] = _context->createCommandBufferOnDefaultPool();

		// Subpass 0
		apiObj->cmdBufferRenderToLocalMemory[i] = _context->createSecondaryCommandBufferOnDefaultPool();
		// Subpass 1
		apiObj->cmdBufferLighting[i] = _context->createSecondaryCommandBufferOnDefaultPool();
	}

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
	apiObj->uiRenderer.init(apiObj->onScreenLocalMemoryRenderPass,
	                        RenderPassSubPasses::UIRenderer);
	apiObj->uiRenderer.getDefaultTitle()->setText("DeferredShading");
	apiObj->uiRenderer.getDefaultTitle()->commitUpdates();
	apiObj->uiRenderer.getDefaultControls()->setText("Action1: Pause\nAction2: Orbit Camera\n");
	apiObj->uiRenderer.getDefaultControls()->commitUpdates();

	// Handle device rotation
	bool isRotated = this->isScreenRotated() && this->isFullScreen();
	if (isRotated)
	{
		projectionMatrix = pvr::math::perspective(getApiType(), mainScene->getCamera(0).getFOV(), (float)this->getHeight() / (float)this->getWidth(),
		                   mainScene->getCamera(0).getNear(), mainScene->getCamera(0).getFar(), glm::pi<pvr::float32>() * .5f);
	}
	else
	{
		projectionMatrix = pvr::math::perspective(getApiType(), mainScene->getCamera(0).getFOV(), (float)this->getWidth() / (float)this->getHeight(),
		                   mainScene->getCamera(0).getNear(), mainScene->getCamera(0).getFar());
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
	assetManager.releaseAll();
	apiObj.reset(0);
	_context.release();

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
\brief  Main rendering loop function of the program. The shell will call this function every frameNumber.
***********************************************************************************************************************/
pvr::Result VulkanDeferredShading::renderFrame()
{
	// Get the current swap index
	apiObj->swapIndex = getSwapChainIndex();

	//  Handle user input and update object animations
	updateAnimation();

	// update dynamic buffers
	updateDynamicSceneData();

	auto& platformctx = getPlatformContext();

	// submit the main command buffer
	apiObj->cmdBufferMain[apiObj->swapIndex]->submit();

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

		apiObj->directionalLightingDescriptorLayout = _context->createDescriptorSetLayout(descSetInfo);

		{
			// create the pipeline layout
			pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;

			pipeLayoutInfo.setDescSetLayout(0, apiObj->directionalLightingDescriptorLayout);
			apiObj->directionalLightingPipelineLayout = _context->createPipelineLayout(pipeLayoutInfo);
		}

		// create the swapchain descriptor sets with corresponding buffers/images
		for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
		{
			pvr::api::DescriptorSetUpdate descSetUpdate;

			descSetUpdate.setDynamicUbo(0, apiObj->staticDirectionalLightUbo.getConnectedBuffer(0));
			descSetUpdate.setDynamicUbo(1, apiObj->dynamicDirectionalLightUbo.getConnectedBuffer(i));

			descSetUpdate.setInputImageAttachment(2, apiObj->onScreenFboTextureViews[Fbo::Albedo][i]);
			descSetUpdate.setInputImageAttachment(3, apiObj->onScreenFboTextureViews[Fbo::Normal][i]);
			descSetUpdate.setInputImageAttachment(4, apiObj->onScreenFboTextureViews[Fbo::Depth][i]);

			apiObj->directionalLightingDescriptorSets.add(_context->createDescriptorSetOnDefaultPool(
			      apiObj->directionalLightingDescriptorLayout));

			apiObj->directionalLightingDescriptorSets[i]->update(descSetUpdate);
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

		apiObj->pointLightGeometryStencilDescriptorLayout = _context->createDescriptorSetLayout(descSetInfo);

		{
			// create the pipeline layout
			pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;

			pipeLayoutInfo.setDescSetLayout(0, apiObj->staticSceneLayout);
			pipeLayoutInfo.setDescSetLayout(1, apiObj->pointLightGeometryStencilDescriptorLayout);
			apiObj->pointLightGeometryStencilPipelineLayout = _context->createPipelineLayout(pipeLayoutInfo);
		}

		// create the swapchain descriptor sets with corresponding buffers
		for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
		{
			pvr::api::DescriptorSetUpdate descSetUpdate;

			descSetUpdate.setDynamicUbo(0, apiObj->staticPointLightUbo.getConnectedBuffer(0));
			descSetUpdate.setDynamicUbo(1, apiObj->dynamicPointLightUbo.getConnectedBuffer(i));

			apiObj->pointLightGeometryStencilDescriptorSets.add(_context->createDescriptorSetOnDefaultPool(
			      apiObj->pointLightGeometryStencilDescriptorLayout));

			apiObj->pointLightGeometryStencilDescriptorSets[i]->update(descSetUpdate);
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
		descSetInfo.setBinding(1, pvr::types::DescriptorType::UniformBufferDynamic, 1u, pvr::types::ShaderStageFlags::Vertex | pvr::types::ShaderStageFlags::Fragment);

		apiObj->pointLightProxyDescriptorLayout = _context->createDescriptorSetLayout(descSetInfo);

		pvr::api::DescriptorSetLayoutCreateParam localMemoryDescSetInfo;

		// Input attachment descriptor set layout
		localMemoryDescSetInfo.setBinding(0, pvr::types::DescriptorType::InputAttachment, 1u, pvr::types::ShaderStageFlags::Fragment);
		localMemoryDescSetInfo.setBinding(1, pvr::types::DescriptorType::InputAttachment, 1u, pvr::types::ShaderStageFlags::Fragment);
		localMemoryDescSetInfo.setBinding(2, pvr::types::DescriptorType::InputAttachment, 1u, pvr::types::ShaderStageFlags::Fragment);

		apiObj->pointLightProxyLocalMemoryDescriptorLayout = _context->createDescriptorSetLayout(localMemoryDescSetInfo);

		{
			// create the pipeline layout
			pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;

			pipeLayoutInfo.setDescSetLayout(0, apiObj->staticSceneLayout);
			pipeLayoutInfo.setDescSetLayout(1, apiObj->pointLightProxyDescriptorLayout);
			pipeLayoutInfo.setDescSetLayout(2, apiObj->pointLightProxyLocalMemoryDescriptorLayout);
			apiObj->pointLightProxyPipelineLayout = _context->createPipelineLayout(pipeLayoutInfo);
		}

		// create the swapchain descriptor sets with corresponding buffers
		for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
		{
			pvr::api::DescriptorSetUpdate descSetUpdate;

			descSetUpdate.setDynamicUbo(0, apiObj->staticPointLightUbo.getConnectedBuffer(0));
			descSetUpdate.setDynamicUbo(1, apiObj->dynamicPointLightUbo.getConnectedBuffer(i));

			apiObj->pointLightProxyDescriptorSets.add(_context->createDescriptorSetOnDefaultPool(
			      apiObj->pointLightProxyDescriptorLayout));

			apiObj->pointLightProxyDescriptorSets[i]->update(descSetUpdate);
		}

		// create the swapchain descriptor sets with corresponding images
		for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
		{
			pvr::api::DescriptorSetUpdate descSetUpdate;

			descSetUpdate.setInputImageAttachment(0, apiObj->onScreenFboTextureViews[Fbo::Albedo][i]);
			descSetUpdate.setInputImageAttachment(1, apiObj->onScreenFboTextureViews[Fbo::Normal][i]);
			descSetUpdate.setInputImageAttachment(2, apiObj->onScreenFboTextureViews[Fbo::Depth][i]);

			apiObj->pointLightProxyLocalMemoryDescriptorLayout = _context->createDescriptorSetLayout(localMemoryDescSetInfo);

			apiObj->pointLightProxyLocalMemoryDescriptorSets.add(_context->createDescriptorSetOnDefaultPool(
			      apiObj->pointLightProxyLocalMemoryDescriptorLayout));

			apiObj->pointLightProxyLocalMemoryDescriptorSets[i]->update(descSetUpdate);
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

		apiObj->pointLightSourceDescriptorLayout = _context->createDescriptorSetLayout(descSetInfo);

		{
			// create the pipeline layout
			pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;

			pipeLayoutInfo.setDescSetLayout(0, apiObj->staticSceneLayout);
			pipeLayoutInfo.setDescSetLayout(1, apiObj->pointLightSourceDescriptorLayout);
			apiObj->pointLightSourcePipelineLayout = _context->createPipelineLayout(pipeLayoutInfo);
		}

		// create the swapchain descriptor sets with corresponding buffers
		for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
		{
			pvr::api::DescriptorSetUpdate descSetUpdate;

			descSetUpdate.setDynamicUbo(0, apiObj->staticPointLightUbo.getConnectedBuffer(0));
			descSetUpdate.setDynamicUbo(1, apiObj->dynamicPointLightUbo.getConnectedBuffer(i));

			apiObj->pointLightSourceDescriptorSets.add(_context->createDescriptorSetOnDefaultPool(
			      apiObj->pointLightSourceDescriptorLayout));

			apiObj->pointLightSourceDescriptorSets[i]->update(descSetUpdate);
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
	apiObj->staticSceneLayout = _context->createDescriptorSetLayout(staticSceneDescSetInfo);

	// Create static descriptor set for the scene
	{
		pvr::api::DescriptorSetUpdate descSetUpdate;
		descSetUpdate.setUbo(0, apiObj->farClipDistanceUbo.getConnectedBuffer(0));

		pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;

		pipeLayoutInfo.setDescSetLayout(0, apiObj->staticSceneLayout);
		apiObj->scenePipelineLayout = _context->createPipelineLayout(pipeLayoutInfo);

		apiObj->sceneDescriptorSet = _context->createDescriptorSetOnDefaultPool(apiObj->staticSceneLayout);
		apiObj->sceneDescriptorSet->update(descSetUpdate);
	}
}

/*!*********************************************************************************************************************
\return Return true if no error occurred
\brief  Loads the textures required for this example and sets up the GBuffer descriptor sets
***********************************************************************************************************************/
bool VulkanDeferredShading::createMaterialsAndDescriptorSets()
{
	if (mainScene->getNumMaterials() == 0)
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
	pvr::api::Sampler samplerTrilinear = _context->createSampler(samplerDesc);

	// CREATE THE DESCRIPTOR SET LAYOUTS
	// Per Model Descriptor set layout
	pvr::api::DescriptorSetLayoutCreateParam descSetInfo;
	// create the ubo descriptor setlayout
	//static material ubo
	descSetInfo.setBinding(0, pvr::types::DescriptorType::UniformBufferDynamic, 1u, pvr::types::ShaderStageFlags::Fragment);

	// static model ubo
	descSetInfo.setBinding(1, pvr::types::DescriptorType::UniformBufferDynamic, 1u, pvr::types::ShaderStageFlags::Vertex);

	// no texture sampler layout
	apiObj->noSamplerLayout = _context->createDescriptorSetLayout(descSetInfo);

	// Single texture sampler layout
	descSetInfo.setBinding(2, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
	apiObj->oneSamplerLayout = _context->createDescriptorSetLayout(descSetInfo);

	// Two textures sampler layout
	descSetInfo.setBinding(3, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
	apiObj->twoSamplerLayout = _context->createDescriptorSetLayout(descSetInfo);

	// Three textures sampler layout
	descSetInfo.setBinding(4, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
	apiObj->threeSamplerLayout = _context->createDescriptorSetLayout(descSetInfo);

	// Four textures sampler layout (for GBuffer rendering)
	descSetInfo.setBinding(5, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
	apiObj->fourSamplerLayout = _context->createDescriptorSetLayout(descSetInfo);

	// create the pipeline layouts
	pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;

	pipeLayoutInfo.setDescSetLayout(0, apiObj->staticSceneLayout);

	pipeLayoutInfo.setDescSetLayout(1, apiObj->noSamplerLayout);
	apiObj->pipeLayoutNoSamplers = _context->createPipelineLayout(pipeLayoutInfo);

	pipeLayoutInfo.setDescSetLayout(1, apiObj->oneSamplerLayout);
	apiObj->pipeLayoutOneSampler = _context->createPipelineLayout(pipeLayoutInfo);

	pipeLayoutInfo.setDescSetLayout(1, apiObj->twoSamplerLayout);
	apiObj->pipeLayoutTwoSamplers = _context->createPipelineLayout(pipeLayoutInfo);

	pipeLayoutInfo.setDescSetLayout(1, apiObj->threeSamplerLayout);
	apiObj->pipeLayoutThreeSamplers = _context->createPipelineLayout(pipeLayoutInfo);

	pipeLayoutInfo.setDescSetLayout(1, apiObj->fourSamplerLayout);
	apiObj->pipeLayoutFourSamplers = _context->createPipelineLayout(pipeLayoutInfo);

	// CREATE DESCRIPTOR SETS FOR EACH MATERIAL
	apiObj->materials.resize(mainScene->getNumMaterials());
	for (pvr::uint32 i = 0; i < mainScene->getNumMaterials(); ++i)
	{
		apiObj->materials[i].materialDescriptorSet.resize(getPlatformContext().getSwapChainLength());

		for (pvr::uint32 j = 0; j < getPlatformContext().getSwapChainLength(); ++j)
		{
			pvr::api::DescriptorSetUpdate descSetUpdate;

			descSetUpdate.setDynamicUbo(0, apiObj->modelMaterialUbo.getConnectedBuffer(0));
			descSetUpdate.setDynamicUbo(1, apiObj->modelMatrixUbo.getConnectedBuffer(j));

			pvr::api::TextureView diffuseMap;
			pvr::api::TextureView bumpMap;

			// get the current material
			const pvr::assets::Model::Material& material = mainScene->getMaterial(i);

			// get material properties
			apiObj->materials[i].specularStrength = material.defaultSemantics().getShininess();
			apiObj->materials[i].diffuseColor = material.defaultSemantics().getDiffuse();

			int numTextures = 0;

			if (material.defaultSemantics().getDiffuseTextureIndex() != -1)
			{
				// Load the diffuse texture map
				if (!assetManager.getTextureWithCaching(getGraphicsContext(),
				                                        mainScene->getTexture(material.defaultSemantics().getDiffuseTextureIndex()).getName(), &(diffuseMap), NULL))
				{
					setExitMessage("ERROR: Failed to load texture %s", mainScene->getTexture(material.defaultSemantics().getDiffuseTextureIndex()).getName().c_str());
					return false;
				}
				descSetUpdate.setCombinedImageSampler(2, diffuseMap, samplerTrilinear);
				++numTextures;
			}
			if (material.defaultSemantics().getBumpMapTextureIndex() != -1)
			{
				// Load the bumpmap
				if (!assetManager.getTextureWithCaching(getGraphicsContext(),
				                                        mainScene->getTexture(material.defaultSemantics().getBumpMapTextureIndex()).getName(), &(bumpMap), NULL))
				{
					setExitMessage("ERROR: Failed to load texture %s", mainScene->getTexture(material.defaultSemantics().getBumpMapTextureIndex()).getName().c_str());
					return false;
				}
				++numTextures;
				descSetUpdate.setCombinedImageSampler(3, bumpMap, samplerTrilinear);
			}

			// based on the number of textures select the correct descriptor set
			switch (numTextures)
			{
			case 0: apiObj->materials[i].materialDescriptorSet[j] =
				  _context->createDescriptorSetOnDefaultPool(apiObj->noSamplerLayout);
				break;
			case 1: apiObj->materials[i].materialDescriptorSet[j] =
				  _context->createDescriptorSetOnDefaultPool(apiObj->oneSamplerLayout);
				break;
			case 2: apiObj->materials[i].materialDescriptorSet[j] =
				  _context->createDescriptorSetOnDefaultPool(apiObj->twoSamplerLayout);
				break;
			case 3: apiObj->materials[i].materialDescriptorSet[j] =
				  _context->createDescriptorSetOnDefaultPool(apiObj->threeSamplerLayout);
				break;
			case 4: apiObj->materials[i].materialDescriptorSet[j] =
				  _context->createDescriptorSetOnDefaultPool(apiObj->fourSamplerLayout);
				break;
			default:
				break;
			}

			apiObj->materials[i].materialDescriptorSet[j]->update(descSetUpdate);
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
	renderGBufferPipelineCreateParam.vertexShader.setShader(_context->createShader(*gbufferVertSource, pvr::types::ShaderType::VertexShader));
	renderGBufferPipelineCreateParam.fragmentShader.setShader(_context->createShader(*gbufferFragSource, pvr::types::ShaderType::FragmentShader));

	// setup vertex inputs
	renderGBufferPipelineCreateParam.vertexInput.clear();
	pvr::utils::createInputAssemblyFromMesh(mainScene->getMesh(MeshNodes::Satyr), vertexBindings, 4, renderGBufferPipelineCreateParam);

	// renderpass/subpass
	renderGBufferPipelineCreateParam.renderPass = apiObj->onScreenLocalMemoryRenderPass;
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

	renderGBufferPipelineCreateParam.pipelineLayout = apiObj->pipeLayoutTwoSamplers;
	apiObj->renderInfo.storeLocalMemoryPass.objects[MeshNodes::Satyr].pipeline = _context->createGraphicsPipeline(renderGBufferPipelineCreateParam);

	// load and create appropriate shaders
	pvr::Stream::ptr_type gbufferFloorVertSource = getAssetStream(Files::GBufferFloorVertexShader);
	pvr::Stream::ptr_type gbufferFloorFragSource = getAssetStream(Files::GBufferFloorFragmentShader);
	renderGBufferPipelineCreateParam.vertexShader.setShader(_context->createShader(*gbufferFloorVertSource, pvr::types::ShaderType::VertexShader));
	renderGBufferPipelineCreateParam.fragmentShader.setShader(_context->createShader(*gbufferFloorFragSource, pvr::types::ShaderType::FragmentShader));

	// setup vertex inputs
	renderGBufferPipelineCreateParam.vertexInput.clear();
	pvr::utils::createInputAssemblyFromMesh(mainScene->getMesh(MeshNodes::Floor), floorVertexBindings, 3, renderGBufferPipelineCreateParam);

	renderGBufferPipelineCreateParam.pipelineLayout = apiObj->pipeLayoutOneSampler;
	apiObj->renderInfo.storeLocalMemoryPass.objects[MeshNodes::Floor].pipeline = _context->createGraphicsPipeline(renderGBufferPipelineCreateParam);
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
	renderDirectionalLightingPipelineCreateParam.vertexShader.setShader(_context->createShader(*gbufferVertSource, pvr::types::ShaderType::VertexShader));
	renderDirectionalLightingPipelineCreateParam.fragmentShader.setShader(_context->createShader(*gbufferFragSource, pvr::types::ShaderType::FragmentShader));

	// setup vertex inputs
	renderDirectionalLightingPipelineCreateParam.vertexInput.clear();
	renderDirectionalLightingPipelineCreateParam.inputAssembler.setPrimitiveTopology(pvr::types::PrimitiveTopology::TriangleStrip);

	renderDirectionalLightingPipelineCreateParam.pipelineLayout = apiObj->directionalLightingPipelineLayout;

	// renderpass/subpass
	renderDirectionalLightingPipelineCreateParam.renderPass = apiObj->onScreenLocalMemoryRenderPass;
	renderDirectionalLightingPipelineCreateParam.subPass = RenderPassSubPasses::Lighting;

	apiObj->renderInfo.directionalLightPass.pipeline = _context->createGraphicsPipeline(renderDirectionalLightingPipelineCreateParam);
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
	pointLightStencilPipelineCreateParam.vertexShader.setShader(_context->createShader(*vertSource, pvr::types::ShaderType::VertexShader));
	pointLightStencilPipelineCreateParam.fragmentShader.setShader(_context->createShader(*fragSource, pvr::types::ShaderType::FragmentShader));

	// setup vertex inputs
	pointLightStencilPipelineCreateParam.vertexInput.clear();
	pvr::utils::createInputAssemblyFromMesh(pointLightModel->getMesh(LightNodes::PointLightMeshNode), pointLightVertexBindings, 1, pointLightStencilPipelineCreateParam);

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
	pointLightStencilPipelineCreateParam.renderPass = apiObj->onScreenLocalMemoryRenderPass;
	pointLightStencilPipelineCreateParam.subPass = RenderPassSubPasses::Lighting;

	pointLightStencilPipelineCreateParam.pipelineLayout = apiObj->pointLightGeometryStencilPipelineLayout;

	apiObj->renderInfo.pointLightGeometryStencilPass.pipeline = _context->createGraphicsPipeline(pointLightStencilPipelineCreateParam);
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
	pointLightProxyPipelineCreateParam.vertexShader.setShader(_context->createShader(*vertSource, pvr::types::ShaderType::VertexShader));
	pointLightProxyPipelineCreateParam.fragmentShader.setShader(_context->createShader(*fragSource, pvr::types::ShaderType::FragmentShader));

	// setup vertex states
	pointLightProxyPipelineCreateParam.vertexInput.clear();
	pvr::utils::createInputAssemblyFromMesh(pointLightModel->getMesh(LightNodes::PointLightMeshNode),
	                                        pointLightVertexBindings, 1, pointLightProxyPipelineCreateParam);

	// if stencil state equals 0 then the lighting should take place as there is geometry inside the point lights area
	pvr::api::pipelineCreation::DepthStencilStateCreateParam::StencilState stencilState;
	stencilState.compareOp = pvr::types::ComparisonMode::Always;
	stencilState.reference = 0;

	pointLightProxyPipelineCreateParam.depthStencil.setStencilFront(stencilState);
	pointLightProxyPipelineCreateParam.depthStencil.setStencilBack(stencilState);

	// renderpass/subpass
	pointLightProxyPipelineCreateParam.renderPass = apiObj->onScreenLocalMemoryRenderPass;
	pointLightProxyPipelineCreateParam.subPass = RenderPassSubPasses::Lighting;

	pointLightProxyPipelineCreateParam.pipelineLayout = apiObj->pointLightProxyPipelineLayout;

	apiObj->renderInfo.pointLightProxyPass.pipeline = _context->createGraphicsPipeline(pointLightProxyPipelineCreateParam);
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
	pointLightSourcePipelineCreateParam.vertexShader.setShader(_context->createShader(*vertSource, pvr::types::ShaderType::VertexShader));
	pointLightSourcePipelineCreateParam.fragmentShader.setShader(_context->createShader(*fragSource, pvr::types::ShaderType::FragmentShader));

	// setup vertex states
	pointLightSourcePipelineCreateParam.vertexInput.clear();
	pvr::utils::createInputAssemblyFromMesh(pointLightModel->getMesh(LightNodes::PointLightMeshNode),
	                                        pointLightVertexBindings, 1, pointLightSourcePipelineCreateParam);

	// renderpass/subpass
	pointLightSourcePipelineCreateParam.renderPass = apiObj->onScreenLocalMemoryRenderPass;
	pointLightSourcePipelineCreateParam.subPass = RenderPassSubPasses::Lighting;

	pointLightSourcePipelineCreateParam.pipelineLayout = apiObj->pointLightSourcePipelineLayout;

	apiObj->renderInfo.pointLightSourcesPass.pipeline = _context->createGraphicsPipeline(pointLightSourcePipelineCreateParam);
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
	      _context->getDepthStencilImageFormat(),
	      pvr::types::LoadOp::Clear, pvr::types::StoreOp::Ignore, pvr::types::LoadOp::Clear, pvr::types::StoreOp::Ignore);

	renderPassInfo.setDepthStencilInfo(renderPassDepthStencilInfo);

	renderPassInfo.setColorInfo(0, pvr::api::RenderPassColorInfo(_context->getPresentationImageFormat(), pvr::types::LoadOp::Clear));

	const pvr::ImageStorageFormat renderpassStorageFormats[Fbo::Count] =
	{
		pvr::ImageStorageFormat(pvr::PixelFormat::RGBA_8888, 1, pvr::types::ColorSpace::lRGB, pvr::VariableType::UnsignedByteNorm),  // albedo
		pvr::ImageStorageFormat(pvr::PixelFormat('r', 'g', 'b', 'a', 16, 16, 16, 16), 1, pvr::types::ColorSpace::lRGB, pvr::VariableType::SignedFloat),  // normal
		pvr::ImageStorageFormat(pvr::PixelFormat::R_32, 1, pvr::types::ColorSpace::lRGB, pvr::VariableType::Float),          // depth attachment
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
	localMemorySubpasses[RenderPassSubPasses::GBuffer].setDepthStencilAttachment(0);
	localMemorySubpasses[RenderPassSubPasses::GBuffer].enableDepthStencilAttachment(true);
	localMemorySubpasses[RenderPassSubPasses::GBuffer].setPreserveAttachment(0, 0);

	// Main scene lighting
	localMemorySubpasses[RenderPassSubPasses::Lighting].setInputAttachment(0, 1);
	localMemorySubpasses[RenderPassSubPasses::Lighting].setInputAttachment(1, 2);
	localMemorySubpasses[RenderPassSubPasses::Lighting].setInputAttachment(2, 3);
	localMemorySubpasses[RenderPassSubPasses::Lighting].setDepthStencilAttachment(0);
	localMemorySubpasses[RenderPassSubPasses::Lighting].enableDepthStencilAttachment(true);
	localMemorySubpasses[RenderPassSubPasses::Lighting].setColorAttachment(0, 0);

	// add subpasses to the renderpass
	renderPassInfo.setSubPass(RenderPassSubPasses::GBuffer, localMemorySubpasses[RenderPassSubPasses::GBuffer]);
	renderPassInfo.setSubPass(RenderPassSubPasses::Lighting, localMemorySubpasses[RenderPassSubPasses::Lighting]);

	// add the sub pass depdendency between sub passes
	pvr::api::SubPassDependency subPassDependency;
	subPassDependency.srcStageMask = pvr::types::PipelineStageFlags::FragmentShader;
	subPassDependency.dstStageMask = pvr::types::PipelineStageFlags::FragmentShader;
	subPassDependency.srcAccessMask = pvr::types::AccessFlags::ColorAttachmentWrite | pvr::types::AccessFlags::DepthStencilAttachmentWrite;
	subPassDependency.dstAccessMask = pvr::types::AccessFlags::InputAttachmentRead | pvr::types::AccessFlags::DepthStencilAttachmentRead;
	subPassDependency.dependencyByRegion = true;

	// GBuffer -> Directional Lighting
	subPassDependency.srcSubPass = RenderPassSubPasses::GBuffer;
	subPassDependency.dstSubPass = RenderPassSubPasses::Lighting;
	renderPassInfo.addSubPassDependency(subPassDependency);

	// Create the renderpass
	apiObj->onScreenLocalMemoryRenderPass = getGraphicsContext()->createRenderPass(renderPassInfo);

	// create and add the transient framebuffer attachments used as color/input attachments
	for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		pvr::api::OnScreenFboCreateParam onScreenFboCreateParam;

		// Allocate the render targets
		for (pvr::uint32 currentIndex = 0; currentIndex < Fbo::Count; ++currentIndex)
		{
			pvr::api::TextureStore transientColorAttachmentTexture = _context->createTexture();
			transientColorAttachmentTexture->allocateTransient(renderpassStorageFormats[currentIndex], getDisplayAttributes().width, getDisplayAttributes().height);

			apiObj->onScreenFboTextureViews[currentIndex].add(
			  _context->createTextureView(transientColorAttachmentTexture));

			onScreenFboCreateParam.addOffScreenColor(apiObj->onScreenFboTextureViews[currentIndex][i]);
		}

		apiObj->onScreenFboCreateParams.add(onScreenFboCreateParam);
	}

	apiObj->onScreenLocalMemoryFbo = _context->createOnScreenFboSetWithRenderPass(
	                                   apiObj->onScreenLocalMemoryRenderPass, apiObj->onScreenFboCreateParams);
}

/*!*********************************************************************************************************************
\brief  Loads the mesh data required for this example into vertex buffer objects
\return Return true if no error occurred
***********************************************************************************************************************/
bool VulkanDeferredShading::loadVbos()
{
	pvr::utils::appendSingleBuffersFromModel(_context, *mainScene, apiObj->sceneVbos, apiObj->sceneIbos);
	pvr::utils::createSingleBuffersFromMesh(_context,
	                                        pointLightModel->getMesh(LightNodes::PointLightMeshNode), apiObj->pointLightVbo, apiObj->pointLightIbo);

	if (!apiObj->sceneVbos.size() || !apiObj->sceneIbos.size() ||
	    apiObj->pointLightVbo.isNull() || apiObj->pointLightIbo.isNull())
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
		apiObj->modelMaterialUbo.addEntryPacked(BufferEntryNames::PerModelMaterial::SpecularStrength, pvr::types::GpuDatatypes::float32);
		apiObj->modelMaterialUbo.addEntryPacked(BufferEntryNames::PerModelMaterial::DiffuseColor, pvr::types::GpuDatatypes::vec3);
		apiObj->modelMaterialUbo.finalize(_context, mainScene->getNumMeshNodes(), pvr::types::BufferBindingUse::UniformBuffer, true, false);

		apiObj->modelMaterialUbo.createConnectedBuffer(0, _context);
	}

	{
		apiObj->modelMatrixUbo.addEntryPacked(BufferEntryNames::PerModel::WorldViewProjectionMatrix, pvr::types::GpuDatatypes::mat4x4);
		apiObj->modelMatrixUbo.addEntryPacked(BufferEntryNames::PerModel::WorldViewMatrix, pvr::types::GpuDatatypes::mat4x4);
		apiObj->modelMatrixUbo.addEntryPacked(BufferEntryNames::PerModel::WorldViewITMatrix, pvr::types::GpuDatatypes::mat4x4);
		apiObj->modelMatrixUbo.finalize(_context, mainScene->getNumMeshNodes(), pvr::types::BufferBindingUse::UniformBuffer, true, false);

		apiObj->modelMatrixUbo.createConnectedBuffers(getPlatformContext().getSwapChainLength(), _context);
	}
}

/*!*********************************************************************************************************************
\brief  Creates the buffers used for rendering the directional lighting
***********************************************************************************************************************/
void VulkanDeferredShading::createDirectionalLightingBuffers()
{
	{
		apiObj->staticDirectionalLightUbo.addEntryPacked(BufferEntryNames::PerDirectionalLight::LightIntensity, pvr::types::GpuDatatypes::vec4);
		apiObj->staticDirectionalLightUbo.finalize(_context, numberOfDirectionalLights, pvr::types::BufferBindingUse::UniformBuffer, true, false);

		apiObj->staticDirectionalLightUbo.createConnectedBuffer(0, _context);
	}

	{
		apiObj->dynamicDirectionalLightUbo.addEntryPacked(BufferEntryNames::PerDirectionalLight::LightViewDirection, pvr::types::GpuDatatypes::vec4);
		apiObj->dynamicDirectionalLightUbo.finalize(_context, numberOfDirectionalLights, pvr::types::BufferBindingUse::UniformBuffer, true, false);

		apiObj->dynamicDirectionalLightUbo.createConnectedBuffers(getPlatformContext().getSwapChainLength(), _context);
	}
}

/*!*********************************************************************************************************************
\brief  Creates the buffers used for rendering the point lighting
***********************************************************************************************************************/
void VulkanDeferredShading::createPointLightBuffers()
{
	// create static point light buffers
	{
		apiObj->staticPointLightUbo.addEntryPacked(BufferEntryNames::PerPointLight::LightIntensity, pvr::types::GpuDatatypes::vec4);
		apiObj->staticPointLightUbo.addEntryPacked(BufferEntryNames::PerPointLight::LightSourceColor, pvr::types::GpuDatatypes::vec4);
		apiObj->staticPointLightUbo.finalize(_context, numberOfPointLights, pvr::types::BufferBindingUse::UniformBuffer, true, false);

		apiObj->staticPointLightUbo.createConnectedBuffer(0, _context);
	}

	// create point light buffers
	{
		apiObj->dynamicPointLightUbo.addEntryPacked(BufferEntryNames::PerPointLight::WorldViewProjectionMatrix, pvr::types::GpuDatatypes::mat4x4);
		apiObj->dynamicPointLightUbo.addEntryPacked(BufferEntryNames::PerPointLight::ProxyLightViewPosition, pvr::types::GpuDatatypes::vec4);
		apiObj->dynamicPointLightUbo.addEntryPacked(BufferEntryNames::PerPointLight::ProxyWorldViewProjectionMatrix, pvr::types::GpuDatatypes::mat4x4);
		apiObj->dynamicPointLightUbo.addEntryPacked(BufferEntryNames::PerPointLight::ProxyWorldViewMatrix, pvr::types::GpuDatatypes::mat4x4);
		apiObj->dynamicPointLightUbo.finalize(_context, numberOfPointLights, pvr::types::BufferBindingUse::UniformBuffer, true, false);

		apiObj->dynamicPointLightUbo.createConnectedBuffers(getPlatformContext().getSwapChainLength(), _context);
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
	apiObj->farClipDistanceUbo.addEntryPacked(BufferEntryNames::PerScene::FarClipDistance, pvr::types::GpuDatatypes::float32);
	apiObj->farClipDistanceUbo.finalize(_context, 1u, pvr::types::BufferBindingUse::UniformBuffer);
	apiObj->farClipDistanceUbo.createConnectedBuffer(0, _context);
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
	farClipDistance = mainScene->getCamera(0).getFar();
	apiObj->farClipDistanceUbo.map(0, pvr::types::MapBufferFlags::Write);
	apiObj->farClipDistanceUbo.setValue(BufferEntryNames::PerScene::FarClipDistance, farClipDistance);
	apiObj->farClipDistanceUbo.unmap(0);
}

/*!*********************************************************************************************************************
\brief  Upload the static data to the buffers which do not change per frame
***********************************************************************************************************************/
void VulkanDeferredShading::uploadStaticModelData()
{
	// static model buffer
	apiObj->modelMaterialUbo.mapMultipleArrayElements(0, 0, mainScene->getNumMeshNodes(), pvr::types::MapBufferFlags::Write);
	for (pvr::uint32 i = 0; i < mainScene->getNumMeshNodes(); ++i)
	{
		apiObj->modelMaterialUbo.setArrayValue(
		  BufferEntryNames::PerModelMaterial::SpecularStrength, i, apiObj->materials[i].specularStrength);

		apiObj->modelMaterialUbo.setArrayValue(
		  BufferEntryNames::PerModelMaterial::DiffuseColor, i, apiObj->materials[i].diffuseColor);
	}
	apiObj->modelMaterialUbo.unmap(0);
}

/*!*********************************************************************************************************************
\brief  Upload the static data to the buffers which do not change per frame
***********************************************************************************************************************/
void VulkanDeferredShading::uploadStaticDirectionalLightData()
{
	// static directional lighting buffer
	apiObj->staticDirectionalLightUbo.mapMultipleArrayElements(0, 0, numberOfDirectionalLights, pvr::types::MapBufferFlags::Write);
	for (pvr::uint32 i = 0; i < numberOfDirectionalLights; ++i)
	{
		apiObj->staticDirectionalLightUbo.setArrayValue(BufferEntryNames::PerDirectionalLight::LightIntensity, i, apiObj->renderInfo.directionalLightPass.lightProperties[i].lightIntensity);
	}
	apiObj->staticDirectionalLightUbo.unmap(0);
}

/*!*********************************************************************************************************************
\brief  Upload the static data to the buffers which do not change per frame
***********************************************************************************************************************/
void VulkanDeferredShading::uploadStaticPointLightData()
{
	// static point lighting buffer
	apiObj->staticPointLightUbo.mapMultipleArrayElements(0, 0, numberOfPointLights, pvr::types::MapBufferFlags::Write);
	for (pvr::uint32 i = 0; i < numberOfPointLights; ++i)
	{
		apiObj->staticPointLightUbo.setArrayValue(BufferEntryNames::PerPointLight::LightIntensity, i, apiObj->renderInfo.pointLightPasses.lightProperties[i].lightIntensity);
		apiObj->staticPointLightUbo.setArrayValue(BufferEntryNames::PerPointLight::LightSourceColor, i, apiObj->renderInfo.pointLightPasses.lightProperties[i].lightColor);
	}
	apiObj->staticPointLightUbo.unmap(0);
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
	RenderData& pass = apiObj->renderInfo;

	apiObj->modelMatrixUbo.mapMultipleArrayElements(apiObj->swapIndex, 0, mainScene->getNumMeshNodes(), pvr::types::MapBufferFlags::Write);

	// update the model matrices
	for (pvr::uint32 i = 0; i < mainScene->getNumMeshNodes(); ++i)
	{
		const pvr::assets::Model::Node& node = mainScene->getNode(i);
		pass.storeLocalMemoryPass.objects[i].world = mainScene->getWorldMatrix(node.getObjectId());
		pass.storeLocalMemoryPass.objects[i].worldView = viewMatrix * pass.storeLocalMemoryPass.objects[i].world;
		pass.storeLocalMemoryPass.objects[i].worldViewProj = viewProjectionMatrix * pass.storeLocalMemoryPass.objects[i].world;
		pass.storeLocalMemoryPass.objects[i].worldViewIT4x4 = glm::inverseTranspose(pass.storeLocalMemoryPass.objects[i].worldView);

		apiObj->modelMatrixUbo.setArrayValue(
		  BufferEntryNames::PerModel::WorldViewMatrix, i, pass.storeLocalMemoryPass.objects[i].worldView);

		apiObj->modelMatrixUbo.setArrayValue(
		  BufferEntryNames::PerModel::WorldViewProjectionMatrix, i, pass.storeLocalMemoryPass.objects[i].worldViewProj);

		apiObj->modelMatrixUbo.setArrayValue(
		  BufferEntryNames::PerModel::WorldViewITMatrix, i, pass.storeLocalMemoryPass.objects[i].worldViewIT4x4);
	}

	apiObj->modelMatrixUbo.unmap(apiObj->swapIndex);

	pvr::int32 pointLight = 0;
	pvr::uint32 directionalLight = 0;

	// update the lighting data
	for (pvr::uint32 i = 0; i < mainScene->getNumLightNodes(); ++i)
	{
		const pvr::assets::Node& lightNode = mainScene->getLightNode(i);
		const pvr::assets::Light& light = mainScene->getLight(lightNode.getObjectId());
		switch (light.getType())
		{
		case pvr::assets::Light::Point:
		{
			if (pointLight >= PointLightConfiguration::MaxScenePointLights) { continue; }

			const glm::mat4& transMtx = mainScene->getWorldMatrix(mainScene->getNodeIdFromLightNodeId(i));
			const glm::mat4& proxyScale = glm::scale(glm::vec3(PointLightConfiguration::PointLightScale)) * PointLightConfiguration::PointlightIntensity;
			const glm::mat4 mWorldScale = transMtx * proxyScale;

			//POINT LIGHT GEOMETRY : The spheres that will be used for the stencil pass
			pass.pointLightPasses.lightProperties[pointLight].proxyWorldViewProjectionMatrix = viewProjectionMatrix * mWorldScale;

			//POINT LIGHT PROXIES : The "drawcalls" that will perform the actual rendering
			pass.pointLightPasses.lightProperties[pointLight].proxyWorldViewMatrix = viewMatrix * mWorldScale;
			pass.pointLightPasses.lightProperties[pointLight].proxyViewSpaceLightPosition = glm::vec4((viewMatrix * transMtx)[3]); //Translation component of the view matrix

			//POINT LIGHT SOURCES : The little balls that we render to show the lights
			pass.pointLightPasses.lightProperties[pointLight].worldViewProjectionMatrix = viewProjectionMatrix * transMtx;
			++pointLight;
		}
		break;
		case pvr::assets::Light::Directional:
		{
			const glm::mat4& transMtx = mainScene->getWorldMatrix(mainScene->getNodeIdFromLightNodeId(i));
			pass.directionalLightPass.lightProperties[directionalLight].viewSpaceLightDirection = viewMatrix * transMtx * glm::vec4(0.f, -1.f, 0.f, 0.f);
			++directionalLight;
		}
		break;
		}
	}
	int numSceneLights = pointLight;
	if (DirectionalLightConfiguration::AdditionalDirectionalLight)
	{
		pass.directionalLightPass.lightProperties[directionalLight].viewSpaceLightDirection = viewMatrix * glm::vec4(0.f, -1.f, 0.f, 0.f);
		++directionalLight;
	}

	for (; pointLight < numSceneLights + PointLightConfiguration::NumProceduralPointLights; ++pointLight)
	{
		updateProceduralPointLight(pass.pointLightPasses.initialData[pointLight],
		                           apiObj->renderInfo.pointLightPasses.lightProperties[pointLight], false);
	}

	// directional Light data
	apiObj->dynamicDirectionalLightUbo.mapMultipleArrayElements(apiObj->swapIndex, 0, numberOfDirectionalLights, pvr::types::MapBufferFlags::Write);

	for (pvr::uint32 i = 0; i < numberOfDirectionalLights; ++i)
	{
		apiObj->dynamicDirectionalLightUbo.setArrayValue(
		  BufferEntryNames::PerDirectionalLight::LightViewDirection, i, apiObj->renderInfo.directionalLightPass.lightProperties[i].viewSpaceLightDirection);
	}

	apiObj->dynamicDirectionalLightUbo.unmap(apiObj->swapIndex);


	apiObj->dynamicPointLightUbo.mapMultipleArrayElements(apiObj->swapIndex, 0, numberOfPointLights, pvr::types::MapBufferFlags::Write);

	for (pvr::uint32 i = 0; i < numberOfPointLights; ++i)
	{
		apiObj->dynamicPointLightUbo.setArrayValue(
		  BufferEntryNames::PerPointLight::ProxyWorldViewProjectionMatrix, i, apiObj->renderInfo.pointLightPasses.lightProperties[i].proxyWorldViewProjectionMatrix);
		apiObj->dynamicPointLightUbo.setArrayValue(
		  BufferEntryNames::PerPointLight::ProxyWorldViewMatrix, i, apiObj->renderInfo.pointLightPasses.lightProperties[i].proxyWorldViewMatrix);
		apiObj->dynamicPointLightUbo.setArrayValue(
		  BufferEntryNames::PerPointLight::ProxyLightViewPosition, i, apiObj->renderInfo.pointLightPasses.lightProperties[i].proxyViewSpaceLightPosition);
		apiObj->dynamicPointLightUbo.setArrayValue(
		  BufferEntryNames::PerPointLight::WorldViewProjectionMatrix, i, apiObj->renderInfo.pointLightPasses.lightProperties[i].worldViewProjectionMatrix);
	}

	apiObj->dynamicPointLightUbo.unmap(apiObj->swapIndex);
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

	if (!initial && !isPaused) //Skip for the first frameNumber, as sometimes this moves the light too far...
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
	pointLightProperties.proxyWorldViewProjectionMatrix = viewProjectionMatrix * mWorldScale;

	//POINT LIGHT PROXIES : The "drawcalls" that will perform the actual rendering
	pointLightProperties.proxyWorldViewMatrix = viewMatrix * mWorldScale;
	pointLightProperties.proxyViewSpaceLightPosition = glm::vec4((viewMatrix * transMtx)[3]); //Translation component of the view matrix

	//POINT LIGHT SOURCES : The little balls that we render to show the lights
	pointLightProperties.worldViewProjectionMatrix = viewProjectionMatrix * transMtx;
}

/*!*********************************************************************************************************************
\brief  Updates animation variables and camera matrices.
***********************************************************************************************************************/
void VulkanDeferredShading::updateAnimation()
{
	pvr::uint64 deltaTime = getFrameTime();

	if (!isPaused)
	{
		frameNumber += deltaTime * ApplicationConfiguration::FrameRate;
		if (frameNumber > mainScene->getNumFrames() - 1) { frameNumber = 0; }
		mainScene->setCurrentFrame(frameNumber);
	}

	glm::vec3 vTo, vUp;
	pvr::float32 fov;
	mainScene->getCameraProperties(cameraId, fov, cameraPosition, vTo, vUp);

	// Update camera matrices
	static float angle = 0;
	if (animateCamera) { angle += getFrameTime() / 1000.f; }
	viewMatrix = glm::lookAt(glm::vec3(sin(angle) * 100.f + vTo.x, vTo.y + 30., cos(angle) * 100.f + vTo.z), vTo, vUp);
	viewProjectionMatrix = projectionMatrix * viewMatrix;
	inverseViewMatrix = glm::inverse(viewMatrix);
}

/*!*********************************************************************************************************************
\brief  Records main command buffer
***********************************************************************************************************************/
void VulkanDeferredShading::recordMainCommandBuffer()
{
	for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		apiObj->cmdBufferMain[i]->beginRecording();

		pvr::Rectanglei renderArea(0, 0, windowWidth, windowHeight);

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
		apiObj->cmdBufferMain[i]->beginRenderPass(
		  apiObj->onScreenLocalMemoryFbo[i], renderArea, false,
		  clearColors, numClearColors, depthClear, stencilClear);

		// Render the models
		apiObj->cmdBufferMain[i]->enqueueSecondaryCmds(apiObj->cmdBufferRenderToLocalMemory[i]);

		// Render lighting + ui render text
		apiObj->cmdBufferMain[i]->nextSubPassSecondaryCmds(apiObj->cmdBufferLighting[i]);

		apiObj->cmdBufferMain[i]->endRenderPass();
		apiObj->cmdBufferMain[i]->endRecording();
	}
}

/*!*********************************************************************************************************************
\brief  Initialise the static light properties
***********************************************************************************************************************/
void VulkanDeferredShading::initialiseStaticLightProperties()
{
	RenderData& pass = apiObj->renderInfo;

	pvr::int32 pointLight = 0;
	pvr::uint32 directionalLight = 0;
	for (pvr::uint32 i = 0; i < mainScene->getNumLightNodes(); ++i)
	{
		const pvr::assets::Node& lightNode = mainScene->getLightNode(i);
		const pvr::assets::Light& light = mainScene->getLight(lightNode.getObjectId());
		switch (light.getType())
		{
		case pvr::assets::Light::Point:
		{
			if (pointLight >= PointLightConfiguration::MaxScenePointLights)
			{
				continue;
			}

			const glm::mat4& transMtx = mainScene->getWorldMatrix(mainScene->getNodeIdFromLightNodeId(i));
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
			const glm::mat4& transMtx = mainScene->getWorldMatrix(mainScene->getNodeIdFromLightNodeId(i));
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
	for (pvr::uint32 i = 0; i < mainScene->getNumLightNodes(); ++i)
	{
		switch (mainScene->getLight(mainScene->getLightNode(i).getObjectId()).getType())
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

	numberOfPointLights = countPoint;
	numberOfDirectionalLights = countDirectional;

	apiObj->renderInfo.directionalLightPass.lightProperties.resize(countDirectional);
	apiObj->renderInfo.pointLightPasses.lightProperties.resize(countPoint);
	apiObj->renderInfo.pointLightPasses.initialData.resize(countPoint);

	for (int i = countPoint - PointLightConfiguration::NumProceduralPointLights; i < countPoint; ++i)
	{
		updateProceduralPointLight(apiObj->renderInfo.pointLightPasses.initialData[i], apiObj->renderInfo.pointLightPasses.lightProperties[i], true);
	}
}

/*!*********************************************************************************************************************
\brief  Record all the secondary command buffers
***********************************************************************************************************************/
void VulkanDeferredShading::recordSecondaryCommandBuffers()
{
	for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		apiObj->cmdBufferRenderToLocalMemory[i]->beginRecording(apiObj->onScreenLocalMemoryFbo[i], RenderPassSubPasses::GBuffer);
		recordCommandBufferRenderGBuffer(apiObj->cmdBufferRenderToLocalMemory[i], i, RenderPassSubPasses::GBuffer);
		apiObj->cmdBufferRenderToLocalMemory[i]->endRecording();

		apiObj->cmdBufferLighting[i]->beginRecording(apiObj->onScreenLocalMemoryFbo[i], RenderPassSubPasses::Lighting);
		recordCommandsDirectionalLights(apiObj->cmdBufferLighting[i], i, RenderPassSubPasses::Lighting);
		recordCommandsPointLightGeometryStencil(apiObj->cmdBufferLighting[i], i, RenderPassSubPasses::Lighting);
		recordCommandsPointLightProxy(apiObj->cmdBufferLighting[i], i, RenderPassSubPasses::Lighting);
		recordCommandsPointLightSourceLighting(apiObj->cmdBufferLighting[i], i, RenderPassSubPasses::Lighting);
		recordCommandUIRenderer(apiObj->cmdBufferLighting[i], i, RenderPassSubPasses::UIRenderer);
		apiObj->cmdBufferLighting[i]->endRecording();
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
	DrawGBuffer& pass = apiObj->renderInfo.storeLocalMemoryPass;

	cmdBuffer->bindDescriptorSet(apiObj->scenePipelineLayout, 0u, apiObj->sceneDescriptorSet);

	for (pvr::uint32 i = 0; i < mainScene->getNumMeshNodes(); ++i)
	{
		cmdBuffer->bindPipeline(pass.objects[i].pipeline);

		// set stencil reference to 1
		cmdBuffer->setStencilReference(pvr::types::StencilFace::FrontBack, 1);

		// enable stencil writing
		cmdBuffer->setStencilWriteMask(pvr::types::StencilFace::FrontBack, 0xFF);

		const pvr::assets::Model::Node& node = mainScene->getNode(i);
		const pvr::assets::Mesh& mesh = mainScene->getMesh(node.getObjectId());

		const Material& material = apiObj->materials[node.getMaterialIndex()];

		pvr::uint32 offsets[2];
		offsets[0] = apiObj->modelMaterialUbo.getAlignedElementArrayOffset(i);
		offsets[1] = apiObj->modelMatrixUbo.getAlignedElementArrayOffset(i);

		cmdBuffer->bindDescriptorSet(pass.objects[i].pipeline->getPipelineLayout(), 1u, material.materialDescriptorSet[swapChainIndex], offsets, 2u);

		cmdBuffer->bindVertexBuffer(apiObj->sceneVbos[node.getObjectId()], 0, 0);
		cmdBuffer->bindIndexBuffer(apiObj->sceneIbos[node.getObjectId()], 0, mesh.getFaces().getDataType());
		cmdBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
	}

}

/*!*********************************************************************************************************************
\brief  Record UIRenderer commands
\param  cmdBuff Commandbuffer to record
***********************************************************************************************************************/
void VulkanDeferredShading::recordCommandUIRenderer(pvr::api::SecondaryCommandBuffer& cmdBuff, pvr::uint32 swapChainIndex, pvr::uint32 subpass)
{
	apiObj->uiRenderer.beginRendering(cmdBuff);
	apiObj->uiRenderer.getDefaultTitle()->render();
	apiObj->uiRenderer.getDefaultControls()->render();
	apiObj->uiRenderer.getSdkLogo()->render();
	apiObj->uiRenderer.endRendering();
}

/*!*********************************************************************************************************************
\brief  Record directional light draw commands
\param  cmdBuffer SecondaryCommandBuffer to record
\param swapChainIndex Current swap chain index
\param subpass Current sub pass
***********************************************************************************************************************/
void VulkanDeferredShading::recordCommandsDirectionalLights(pvr::api::SecondaryCommandBuffer& cmdBuffer, pvr::uint32 swapChainIndex, pvr::uint32 subpass)
{
	DrawDirectionalLight& directionalPass = apiObj->renderInfo.directionalLightPass;

	cmdBuffer->bindPipeline(directionalPass.pipeline);

	// if for the current fragment the stencil has been filled then there is geometry present
	// and directional lighting calculations should be carried out
	cmdBuffer->setStencilReference(pvr::types::StencilFace::FrontBack, 1);

	// disable stencil writing
	cmdBuffer->setStencilWriteMask(pvr::types::StencilFace::FrontBack, 0x00);

	// keep the descriptor set bound even though for this pass we don't need it
	// avoids unbinding before rebinding in the next passes
	cmdBuffer->bindDescriptorSet(apiObj->scenePipelineLayout, 0u, apiObj->sceneDescriptorSet);

	// Make use of the stencil buffer contents to only shade pixels where actual geometry is located.
	// Reset the stencil buffer to 0 at the same time to avoid the stencil clear operation afterwards.
	// bind the albedo and normal textures from the gbuffer
	for (pvr::uint32 i = 0; i < numberOfDirectionalLights; i++)
	{
		pvr::uint32 offsets[2] = {};
		offsets[0] = apiObj->staticDirectionalLightUbo.getAlignedElementArrayOffset(i);
		offsets[1] = apiObj->dynamicDirectionalLightUbo.getAlignedElementArrayOffset(i);

		cmdBuffer->bindDescriptorSet(directionalPass.pipeline->getPipelineLayout(), 0u, apiObj->directionalLightingDescriptorSets[swapChainIndex], offsets, 2u);

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
	PointLightGeometryStencil& pointGeometryStencilPass = apiObj->renderInfo.pointLightGeometryStencilPass;
	PointLightPasses& pointPasses = apiObj->renderInfo.pointLightPasses;

	const pvr::assets::Mesh& mesh = pointLightModel->getMesh(LightNodes::PointLightMeshNode);

	pvr::Rectanglei renderArea(0, 0, framebufferWidth, framebufferHeight);
	if ((framebufferWidth != windowWidth) || (framebufferHeight != windowHeight))
	{
		renderArea = pvr::Rectanglei(viewportOffsets[0], viewportOffsets[1], framebufferWidth, framebufferHeight);
	}

	// clear stencil to 0's to make use of it again for point lights
	cmdBuffer->clearStencilAttachment(renderArea, 0);

	cmdBuffer->bindDescriptorSet(apiObj->scenePipelineLayout, 0u, apiObj->sceneDescriptorSet);

	cmdBuffer->setStencilReference(pvr::types::StencilFace::FrontBack, 0);

	//POINT LIGHTS: 1) Draw stencil to discard useless pixels
	cmdBuffer->bindPipeline(pointGeometryStencilPass.pipeline);
	// Bind the vertex and index buffer for the point light
	cmdBuffer->bindVertexBuffer(apiObj->pointLightVbo, 0, 0);
	cmdBuffer->bindIndexBuffer(apiObj->pointLightIbo, 0, pvr::types::IndexType::IndexType16Bit);

	for (pvr::uint32 i = 0; i < pointPasses.lightProperties.size(); i++)
	{
		pvr::uint32 offsets[2] = {};
		offsets[0] = apiObj->staticPointLightUbo.getAlignedElementArrayOffset(i);
		offsets[1] = apiObj->dynamicPointLightUbo.getAlignedElementArrayOffset(i);

		cmdBuffer->bindDescriptorSet(pointGeometryStencilPass.pipeline->getPipelineLayout(), 1u,
		                             apiObj->pointLightGeometryStencilDescriptorSets[swapChainIndex], offsets, 2u);

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
	DrawPointLightProxy& pointLightProxyPass = apiObj->renderInfo.pointLightProxyPass;
	PointLightPasses& pointPasses = apiObj->renderInfo.pointLightPasses;

	const pvr::assets::Mesh& mesh = pointLightModel->getMesh(LightNodes::PointLightMeshNode);

	//Any of the geompointlightpass, lightsourcepointlightpass or pointlightproxiepass's uniforms have the same number of elements
	if (pointPasses.lightProperties.empty()) { return; }

	//POINT LIGHTS: 2) Lighting
	cmdBuffer->bindDescriptorSet(apiObj->scenePipelineLayout, 0u, apiObj->sceneDescriptorSet);

	cmdBuffer->bindPipeline(apiObj->renderInfo.pointLightProxyPass.pipeline);

	// Bind the vertex and index buffer for the point light
	cmdBuffer->bindVertexBuffer(apiObj->pointLightVbo, 0, 0);
	cmdBuffer->bindIndexBuffer(apiObj->pointLightIbo, 0, mesh.getFaces().getDataType());

	for (pvr::uint32 i = 0; i < pointPasses.lightProperties.size(); ++i)
	{
		const pvr::uint32 numberOfOffsets = 2;
		pvr::uint32 offsets[numberOfOffsets] = {};
		offsets[0] = apiObj->staticPointLightUbo.getAlignedElementArrayOffset(i);
		offsets[1] = apiObj->dynamicPointLightUbo.getAlignedElementArrayOffset(i);

		cmdBuffer->bindDescriptorSet(pointLightProxyPass.pipeline->getPipelineLayout(), 1u,
		                             apiObj->pointLightProxyDescriptorSets[swapChainIndex], offsets, numberOfOffsets);

		cmdBuffer->bindDescriptorSet(pointLightProxyPass.pipeline->getPipelineLayout(), 2u,
		                             apiObj->pointLightProxyLocalMemoryDescriptorSets[swapChainIndex]);

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
	DrawPointLightSources& pointLightSourcePass = apiObj->renderInfo.pointLightSourcesPass;
	PointLightPasses& pointPasses = apiObj->renderInfo.pointLightPasses;

	const pvr::assets::Mesh& mesh = pointLightModel->getMesh(LightNodes::PointLightMeshNode);

	//POINT LIGHTS: 3) Light sources
	cmdBuffer->bindDescriptorSet(apiObj->scenePipelineLayout, 0u, apiObj->sceneDescriptorSet);

	cmdBuffer->bindPipeline(pointLightSourcePass.pipeline);
	cmdBuffer->bindVertexBuffer(apiObj->pointLightVbo, 0, 0);
	cmdBuffer->bindIndexBuffer(apiObj->pointLightIbo, 0, mesh.getFaces().getDataType());

	for (pvr::uint32 i = 0; i < pointPasses.lightProperties.size(); ++i)
	{
		const pvr::uint32 numberOfOffsets = 2u;

		pvr::uint32 offsets[numberOfOffsets] = {};
		offsets[0] = apiObj->staticPointLightUbo.getAlignedElementArrayOffset(i);
		offsets[1] = apiObj->dynamicPointLightUbo.getAlignedElementArrayOffset(i);

		cmdBuffer->bindDescriptorSet(pointLightSourcePass.pipeline->getPipelineLayout(), 1u,
		                             apiObj->pointLightSourceDescriptorSets[swapChainIndex], offsets, numberOfOffsets);

		cmdBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
	}
}

/*!*********************************************************************************************************************
\return Return an auto_ptr to a new Demo class, supplied by the user
\brief  This function must be implemented by the user of the shell. The user should return its Shell object defining the
behaviour of the application.
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() { return std::auto_ptr<pvr::Shell>(new VulkanDeferredShading()); }
