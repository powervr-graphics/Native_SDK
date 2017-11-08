/*!*********************************************************************************************************************
\File         VulkanDeferredShadingPFX.cpp
\Title        Deferred Shading
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\Description  Implements a deferred shading technique supporting point and directional lights using pfx.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRVk/PVRVk.h"
#include "PVRUtils/PVRUtilsVk.h"
// Light mesh nodes
enum class LightNodes
{
	PointLightMeshNode = 0,
	NumberOfPointLightMeshNodes
};

// mesh nodes
enum class MeshNodes
{
	Satyr = 0,
	Floor = 1,
	NumberOfMeshNodes
};

static const pvr::StringHash PFXSemanticsStr[] =
{
	"MODELVIEWPROJECTIONMATRIX",
	"MODELVIEWMATRIX",
	"MODELWORLDITMATRIX",
	"VIEWPOSITION",
	"PROXYMODELVIEWPROJECTIONMATRIX",
	"PROXYMODELVIEWMATRIX",
	"PROXYVIEWPOSITION",
	"LIGHTINTENSITY",
	"LIGHTRADIUS",
	"LIGHTCOLOR",
	"LIGHTSOURCECOLOR",
	"FARCLIPDIST"
};

enum class PFXSemanticId
{
	MODELVIEWPROJECTIONMATRIX,
	MODELVIEWMATRIX,
	MODELWORLDITMATRIX,
	VIEWPOSITION,
	PROXYMODELVIEPROJECTIONMATRIX,
	PROXYMODELVIEWMATRIX,
	PROXYVIEWPOSITION,
	LIGHTINTENSITY,
	LIGHTRADIUS,
	LIGHTCOLOR,
	LIGHTSOURCECOLOR,
	FARCLIPDIST
};

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

// structure used to render directional lighting
struct DrawDirectionalLight
{
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
		pvr::FreeValue world;
		pvr::FreeValue worldView;
		pvr::FreeValue worldViewProj;
		pvr::FreeValue worldViewIT4x4;
	};
	std::vector<Objects> objects;
};

// structure used to hold the rendering information for the demo
struct RenderData
{
	DrawGBuffer storeLocalMemoryPass; // Subpass 0
	DrawDirectionalLight directionalLightPass; // Subpass 1
	PointLightPasses pointLightPasses; // holds point light data
};

// Shader names for all of the demo passes
namespace Files {
const char* const SceneFile = "scene.pod";
const char* const EffectPfx = "effect_MRT_PFX3.pfx";
const char* const PointLightModelFile = "pointlight.pod";
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
static int32_t NumProceduralPointLights  = 10;
float PointLightScale = 32.0f; // PointLightScale handles the size of the scaled light geometry. This effects the areas of the screen which will go through point light rendering
float PointLightRadius = PointLightScale / 2.0f; // PointLightRadius handles the actual point light falloff. Modifying one of these requires also modifying the other
float PointlightIntensity = 5.0f;
}

// Subpasses used in the renderpass
enum class RenderPassSubPass
{
	GBuffer,
	Lighting,
	NumberOfSubpasses,
};

// Lighting Subpass's groups
enum class LightingSubpassGroup
{
	DirectionalLight,
	PointLightStep1,// Stencil
	PointLightStep2,// Proxy
	PointLightStep3,// Render Source
	Count
};

// Lighting Subpass groups pipelines
enum class LightingSubpassPipeline
{
	DirectionalLighting,

	// Point light passes
	PointLightStencil,
	PointLightProxy,
	PointLightSource,
	NumPipelines
};

/*!*********************************************************************************************************************
Class implementing the Shell functions.
***********************************************************************************************************************/
class VulkanDeferredShadingPFX : public pvr::Shell
{
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

		pvrvk::Semaphore semaphoreImageAcquired[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Fence perFrameAcquireFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Semaphore semaphorePresent[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Fence perFrameCommandBufferFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

		//// Command Buffers ////
		// Main Primary Command Buffer
		pvr::Multi<pvrvk::CommandBuffer> commandBufferMain;
		pvr::utils::RenderManager render_mgr;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;
	};

	//// Frame ////
	uint32_t _numSwapImages;
	uint32_t _swapchainIndex;
	uint32_t _frameId;
	//Putting all api objects into a pointer just makes it easier to release them all together with RAII
	std::auto_ptr<DeviceResources> _deviceResources;

	// Frame counters for animation
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

	RenderData _renderInfo;

public:
	VulkanDeferredShadingPFX() { _animateCamera = false; _isPaused = false; }

	//	Overriden from Shell
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void recordCommandsPointLightGeometryStencil(pvrvk::CommandBuffer& commandBuffer, uint32_t swapChainIndex, const uint32_t pointLight);
	void recordMainCommandBuffer();
	void allocateLights();
	void uploadStaticData();
	void uploadStaticSceneData();
	void uploadStaticDirectionalLightData();
	void uploadStaticPointLightData();
	void initialiseStaticLightProperties();
	void updateDynamicSceneData(uint32_t swapchain);
	void updateDynamicLightData(uint32_t swapchain);
	void updateAnimation();
	void updateProceduralPointLight(PointLightPasses::InitialData& data,
	                                PointLightPasses::PointLightProperties& pointLightProperties, uint32_t pointLightIndex);

	void updateGBufferPass()
	{
		auto& pipeline = _deviceResources->render_mgr.toPipeline(0, 0, static_cast<uint32_t>(RenderPassSubPass::GBuffer), 0, 0);
		pipeline.updateAutomaticModelSemantics(0);
		_deviceResources->render_mgr.toSubpassGroupModel(0, 0, static_cast<uint32_t>(RenderPassSubPass::GBuffer), 0, 0).updateFrame(0);
	}

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
	pvr::assets::ModelHandle createFullScreenQuadMesh();
	void setProceduralPointLightInitialData(PointLightPasses::InitialData& data,
	                                        PointLightPasses::PointLightProperties& pointLightProperties);
private:
};

/*!*********************************************************************************************************************
\return	Return true if no error occurred
\brief	Code in initApplication() will be called by pvr::Shell once per run, before the rendering context is created.
Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result VulkanDeferredShadingPFX::initApplication()
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

pvr::assets::ModelHandle VulkanDeferredShadingPFX::createFullScreenQuadMesh()
{
	pvr::assets::ModelHandle model;	model.construct();
	model->allocMeshes(_numberOfDirectionalLights);
	model->allocMeshNodes(_numberOfDirectionalLights);
	// create a dummy material with a material attribute which will be identified by the pfx.
	model->addMaterial(pvr::assets::Material());
	model->getMaterial(0).setMaterialAttribute("DIR_LIGHT", pvr::FreeValue());
	for (uint32_t i = 0; i < _numberOfDirectionalLights; ++i)
	{
		model->getMesh(i).setPrimitiveType(pvr::PrimitiveTopology::TriangleStrip);
		model->getMesh(i).setNumVertices(3);
		model->connectMeshWithMeshNode(i, i);
		model->getMeshNode(i).setMaterialIndex(0);
	}
	return model;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in initView() will be called by PVRShell upon initialization or after a change in the rendering context.
Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result VulkanDeferredShadingPFX::initView()
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
		{ VkQueueFlags::e_GRAPHICS_BIT, _deviceResources->surface },
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

	// Create the swapchain
	_deviceResources->swapchain = pvr::utils::createSwapchain(_deviceResources->device, _deviceResources->surface, getDisplayAttributes(), swapchainImageUsage);

	if (_deviceResources->swapchain.isNull())
	{
		return pvr::Result::UnknownError;
	}

	// Get the number of swap images
	_numSwapImages = _deviceResources->swapchain->getSwapchainLength();

	// Get current swap index
	_swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

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

	Log("Framebuffer dimensions: %d x %d\n", _framebufferWidth, _framebufferHeight);
	Log("Onscreen Framebuffer dimensions: %d x %d\n", _windowWidth, _windowHeight);

	// create the commandpool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(queueAccessInfo.familyId,
	                                VkCommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT);

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(
	                                     pvrvk::DescriptorPoolCreateInfo()
	                                     .addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER, 32)
	                                     .addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 32)
	                                     .addDescriptorInfo(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 32)
	                                     .addDescriptorInfo(VkDescriptorType::e_INPUT_ATTACHMENT, 32)
	                                     .setMaxDescriptorSets(32));

	// Initialise lighting structures
	allocateLights();

	// Setup per swapchain Resources
	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		_deviceResources->commandBufferMain[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->semaphorePresent[i] = _deviceResources->device->createSemaphore();
		_deviceResources->semaphoreImageAcquired[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameCommandBufferFence[i] = _deviceResources->device->createFence(VkFenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->perFrameAcquireFence[i] = _deviceResources->device->createFence(VkFenceCreateFlags::e_SIGNALED_BIT);

		if (i == 0)
		{
			_deviceResources->commandBufferMain[0]->begin();
		}
		pvr::utils::setImageLayout(_deviceResources->swapchain->getImage(i), VkImageLayout::e_UNDEFINED,
		                           VkImageLayout::e_PRESENT_SRC_KHR, _deviceResources->commandBufferMain[0]);
	}

	const float_t aspectRatio = (float_t)_deviceResources->swapchain->getDimension().width /
	                            _deviceResources->swapchain->getDimension().height;
	_projectionMatrix = pvr::math::perspective(pvr::Api::Vulkan,
	                    _mainScene->getCamera(0).getFOV(), aspectRatio, _mainScene->getCamera(0).getNear(),
	                    _mainScene->getCamera(0).getFar());

	// allocate number of point light mesh nodes which will uses the same material and the mesh
	_numberOfPointLights = PointLightConfiguration::NumProceduralPointLights;

	_pointLightModel->allocMeshNodes(_numberOfPointLights);
	_pointLightModel->connectMeshWithMeshNodes(0, 0, _numberOfPointLights - 1);
	_pointLightModel->addMaterial(pvr::assets::Material());
	_pointLightModel->getMaterial(0).setMaterialAttribute("POINT_LIGHT", pvr::FreeValue());
	_pointLightModel->assignMaterialToMeshNodes(0, 0, _numberOfPointLights - 1);

	//--- create the pfx effect
	std::vector<pvr::utils::ImageUploadResults> uploadResults;
	pvr::assets::pfx::PfxParser rd(Files::EffectPfx, this);
	if (!_deviceResources->render_mgr.init(*this, _deviceResources->swapchain, _deviceResources->descriptorPool))
	{
		return pvr::Result::UnknownError;
	}
	_deviceResources->render_mgr.addEffect(*rd.getAssetHandle(), _deviceResources->commandBufferMain[0], uploadResults);

	//--- Gbuffer renders the scene
	_deviceResources->render_mgr.addModelForAllSubpassGroups(_mainScene, 0, static_cast<uint32_t>(RenderPassSubPass::GBuffer), 0);

	//--- add the full screen quad mesh to the directional light subpass group in lighting subpass
	_deviceResources->render_mgr.addModelForSubpassGroup(createFullScreenQuadMesh(), 0, static_cast<uint32_t>(RenderPassSubPass::Lighting),
	    static_cast<uint32_t>(LightingSubpassGroup::DirectionalLight));

	//--- add the point lights to the Pointlight subpass groups in lighting subpass
	_deviceResources->render_mgr.addModelForSubpassGroup(_pointLightModel, 0, static_cast<uint32_t>(RenderPassSubPass::Lighting),
	    static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1));

	_deviceResources->render_mgr.addModelForSubpassGroup(_pointLightModel, 0, static_cast<uint32_t>(RenderPassSubPass::Lighting),
	    static_cast<uint32_t>(LightingSubpassGroup::PointLightStep2));

	_deviceResources->render_mgr.addModelForSubpassGroup(_pointLightModel, 0, static_cast<uint32_t>(RenderPassSubPass::Lighting),
	    static_cast<uint32_t>(LightingSubpassGroup::PointLightStep3));

	// build all the renderman objects
	_deviceResources->render_mgr.buildRenderObjects(_deviceResources->commandBufferMain[0], uploadResults);

	_deviceResources->commandBufferMain[0]->end();
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->commandBufferMain[0];
	submitInfo.numCommandBuffers = 1;

	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle();// wait for the commands to be flushed

	// initialize the UIRenderer and set the title text
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(),
	                                  _deviceResources->render_mgr.toPass(0, 0).getFramebuffer(0)->getRenderPass(),
	                                  static_cast<uint32_t>(RenderPassSubPass::Lighting), _deviceResources->commandPool, _deviceResources->queue);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("DeferredShadingPFX").commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText("Action1: Pause\nAction2: Orbit Camera\n");
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	// initialise the gbuffer renderpass list
	_renderInfo.storeLocalMemoryPass.objects.resize(_mainScene->getNumMeshNodes());

	// calculate the frame buffer width and heights
	_framebufferWidth = _windowWidth = this->getWidth();
	_framebufferHeight = _windowHeight = this->getHeight();

	// Upload static data
	initialiseStaticLightProperties();
	uploadStaticData();

	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		updateDynamicSceneData(i);
	}

	// Record the main command buffer
	recordMainCommandBuffer();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result VulkanDeferredShadingPFX::releaseView()
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
\return	Return pvr::Result::Success if no error occurred
\brief	Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
If the rendering context is lost, QuitApplication() will not be called.x
***********************************************************************************************************************/
pvr::Result VulkanDeferredShadingPFX::quitApplication() { return pvr::Result::Success; }

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every m_frameNumber.
***********************************************************************************************************************/
pvr::Result VulkanDeferredShadingPFX::renderFrame()
{
	_deviceResources->perFrameAcquireFence[_frameId]->wait();
	_deviceResources->perFrameAcquireFence[_frameId]->reset();
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->semaphoreImageAcquired[_frameId], _deviceResources->perFrameAcquireFence[_frameId]);

	_swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameCommandBufferFence[_swapchainIndex]->wait();
	_deviceResources->perFrameCommandBufferFence[_swapchainIndex]->reset();

	//  Handle user input and update object animations
	updateAnimation();

	_deviceResources->render_mgr.updateAutomaticSemantics(_swapchainIndex);

	// update the scene dynamic buffer
	updateDynamicSceneData(_swapchainIndex);

	// update dynamic light buffers
	updateDynamicLightData(_swapchainIndex);

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
	// present
	pvrvk::PresentInfo presentInfo;
	presentInfo.waitSemaphores = &_deviceResources->semaphorePresent[_frameId];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.imageIndices = &_swapchainIndex;
	pvr::Result result = _deviceResources->queue->present(presentInfo) ==
	                     VkResult::e_SUCCESS ? pvr::Result::Success : pvr::Result::UnknownError;

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief	Upload the static data to the buffers which do not change per frame
***********************************************************************************************************************/
void VulkanDeferredShadingPFX::uploadStaticSceneData()
{
	// static scene properties buffer
	_farClipDistance = _mainScene->getCamera(0).getFar();

	pvr::FreeValue farClipDist;
	farClipDist.setValue(_farClipDistance);

	pvr::FreeValue specStrength;
	specStrength.setValue(0.0f);

	pvr::FreeValue diffColor;
	diffColor.setValue(glm::vec4(0.f));

	pvr::utils::RendermanSubpassGroupModel& model = _deviceResources->render_mgr.toSubpassGroupModel(
	      0, 0, static_cast<uint32_t>(RenderPassSubPass::GBuffer), 0, 0);

	_deviceResources->render_mgr.toEffect(0).updateBufferEntryEffectSemantic("FARCLIPDIST", farClipDist, 0);

	for (uint32_t i = 0; i < model.getNumRendermanNodes(); ++i)
	{
		auto& node = model.toRendermanNode(i);

		pvr::assets::Material& material = _mainScene->getMaterial(_mainScene->getMeshNode(node.assetNodeId).getMaterialIndex());
		specStrength.setValue(material.defaultSemantics().getShininess());
		diffColor.setValue(glm::vec4(material.defaultSemantics().getDiffuse(), 1.f));
		node.updateNodeValueSemantic("SPECULARSTRENGTH", specStrength, 0);
		node.updateNodeValueSemantic("DIFFUSECOLOUR", diffColor, 0);
	}
}

/*!*********************************************************************************************************************
\brief	Upload the static data to the buffers which do not change per frame
***********************************************************************************************************************/
void VulkanDeferredShadingPFX::uploadStaticDirectionalLightData()
{
	pvr::FreeValue mem;
	for (uint32_t i = 0; i < _numberOfDirectionalLights; ++i)
	{
		mem.setValue(_renderInfo.directionalLightPass.lightProperties[i].lightIntensity);
		_deviceResources->render_mgr.toPipeline(0, 0, static_cast<uint32_t>(RenderPassSubPass::Lighting),
		                                        static_cast<uint32_t>(LightingSubpassGroup::DirectionalLight), static_cast<uint32_t>(LightingSubpassPipeline::DirectionalLighting))
		.updateBufferEntryNodeSemantic("LIGHTINTENSITY", mem, 0,
		                               _deviceResources->render_mgr.toSubpassGroupModel(0, 0, static_cast<uint32_t>(RenderPassSubPass::Lighting),
		                                   static_cast<uint32_t>(LightingSubpassGroup::DirectionalLight),
		                                   static_cast<uint32_t>(LightingSubpassPipeline::DirectionalLighting)).toRendermanNode(i));
	}
}

/*!*********************************************************************************************************************
\brief	Upload the static data to the buffers which do not change per frame
***********************************************************************************************************************/
void VulkanDeferredShadingPFX::uploadStaticPointLightData()
{
	// static point lighting buffer
	pvr::FreeValue values[4];
	for (uint32_t lightGroups = 0; lightGroups < 3; ++lightGroups)
	{
		for (uint32_t i = 0; i < _numberOfPointLights; ++i)
		{
			// LIGHTINTENSITY
			values[0].setValue(_renderInfo.pointLightPasses.lightProperties[i].lightIntensity);
			// LIGHTRADIUS
			values[1].setValue(_renderInfo.pointLightPasses.lightProperties[i].lightRadius);
			// LIGHTCOLOR
			values[2].setValue(_renderInfo.pointLightPasses.lightProperties[i].lightColor);
			// LIGHTSOURCECOLOR
			values[3].setValue(_renderInfo.pointLightPasses.lightProperties[i].lightSourceColor);

			// Point light data
			{
				pvr::utils::RendermanNode& node = _deviceResources->render_mgr.toSubpassGroupModel(0, 0,
				                                  static_cast<uint32_t>(RenderPassSubPass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1) + lightGroups, 0).toRendermanNode(i);
				_deviceResources->render_mgr.toPipeline(0, 0, static_cast<uint32_t>(RenderPassSubPass::Lighting),
				                                        static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1) + lightGroups, static_cast<uint32_t>(0)).updateBufferEntryNodeSemantic(
				                                            PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::LIGHTINTENSITY)], values[0], 0, node);
			}
			{
				pvr::utils::RendermanNode& node = _deviceResources->render_mgr.toSubpassGroupModel(0, 0,
				                                  static_cast<uint32_t>(RenderPassSubPass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1) + lightGroups, 0).toRendermanNode(i);
				_deviceResources->render_mgr.toPipeline(0, 0, static_cast<uint32_t>(RenderPassSubPass::Lighting),
				                                        static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1) + lightGroups, static_cast<uint32_t>(0)).updateBufferEntryNodeSemantic(
				                                            PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::LIGHTRADIUS)], values[1], 0, node);
			}

			{
				pvr::utils::RendermanNode& node = _deviceResources->render_mgr.toSubpassGroupModel(0, 0,
				                                  static_cast<uint32_t>(RenderPassSubPass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1) + lightGroups, 0).toRendermanNode(i);
				_deviceResources->render_mgr.toPipeline(0, 0, static_cast<uint32_t>(RenderPassSubPass::Lighting),
				                                        static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1) + lightGroups, static_cast<uint32_t>(0)).updateBufferEntryNodeSemantic(
				                                            PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::LIGHTCOLOR)], values[2], 0, node);
			}

			{
				pvr::utils::RendermanNode& node = _deviceResources->render_mgr.toSubpassGroupModel(0, 0,
				                                  static_cast<uint32_t>(RenderPassSubPass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1) + lightGroups, 0).toRendermanNode(i);
				_deviceResources->render_mgr.toPipeline(0, 0, static_cast<uint32_t>(RenderPassSubPass::Lighting),
				                                        static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1) + lightGroups, static_cast<uint32_t>(0)).updateBufferEntryNodeSemantic(
				                                            PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::LIGHTSOURCECOLOR)], values[3], 0, node);
			}
		}
	}
}

/*!*********************************************************************************************************************
\brief	Upload the static data to the buffers which do not change per frame
***********************************************************************************************************************/
void VulkanDeferredShadingPFX::uploadStaticData()
{
	uploadStaticDirectionalLightData();
	uploadStaticSceneData();
	uploadStaticPointLightData();
}

/*!*********************************************************************************************************************
\brief	Update the CPU visible buffers containing dynamic data
***********************************************************************************************************************/
void VulkanDeferredShadingPFX::updateDynamicSceneData(uint32_t swapchain)
{
	RenderData& pass = _renderInfo;

	// update the model matrices
	static glm::mat4 world, worldView;
	auto subPassGroupModels = _deviceResources->render_mgr.toSubpassGroupModel(0, 0,
	                          static_cast<uint32_t>(RenderPassSubPass::GBuffer), 0, 0).nodes;

	for (auto it = subPassGroupModels.begin(); it != subPassGroupModels.end(); ++it)
	{
		pvr::utils::RendermanNode& rendermanNode = *it;
		const pvr::assets::Model::Node& node = *rendermanNode.assetNode;
		world = _mainScene->getWorldMatrix(node.getObjectId());
		worldView = _viewMatrix * world;
		pass.storeLocalMemoryPass.objects[rendermanNode.assetNodeId].world.setValue(world);
		pass.storeLocalMemoryPass.objects[rendermanNode.assetNodeId].worldView.setValue(worldView);
		pass.storeLocalMemoryPass.objects[rendermanNode.assetNodeId].worldViewIT4x4.setValue(glm::inverseTranspose(worldView));
		pass.storeLocalMemoryPass.objects[rendermanNode.assetNodeId].worldViewProj.setValue(_projectionMatrix * worldView);

		pvr::utils::RendermanPipeline& pipe = rendermanNode.toRendermanPipeline();
		pipe.updateBufferEntryNodeSemantic(PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::MODELVIEWPROJECTIONMATRIX)],
		                                   pass.storeLocalMemoryPass.objects[rendermanNode.assetNodeId].worldViewProj,
		                                   swapchain, rendermanNode);

		pipe.updateBufferEntryNodeSemantic(PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::MODELVIEWMATRIX)],
		                                   pass.storeLocalMemoryPass.objects[rendermanNode.assetNodeId].worldView,
		                                   swapchain, rendermanNode);

		pipe.updateBufferEntryNodeSemantic(PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::MODELWORLDITMATRIX)],
		                                   pass.storeLocalMemoryPass.objects[rendermanNode.assetNodeId].worldViewIT4x4,
		                                   swapchain, rendermanNode);
	}
}

void VulkanDeferredShadingPFX::updateDynamicLightData(uint32_t swapchain)
{
	int32_t pointLight = 0;
	uint32_t directionalLight = 0;
	RenderData& pass = _renderInfo;
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
			const glm::mat4& proxyScale = glm::scale(glm::vec3(PointLightConfiguration::PointLightScale)) *
			                              PointLightConfiguration::PointlightIntensity;

			const glm::mat4 mWorldScale = transMtx * proxyScale;

			//POINT LIGHT GEOMETRY : The spheres that will be used for the stencil pass
			pass.pointLightPasses.lightProperties[pointLight].proxyWorldViewProjectionMatrix = _viewProjectionMatrix * mWorldScale;

			//POINT LIGHT PROXIES : The "drawcalls" that will perform the actual rendering
			pass.pointLightPasses.lightProperties[pointLight].proxyWorldViewMatrix = _viewMatrix * mWorldScale;
			//Translation component of the view matrix
			pass.pointLightPasses.lightProperties[pointLight].proxyViewSpaceLightPosition = glm::vec4((_viewMatrix * transMtx)[3]);

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

	// update the directional light pipeline
	for (uint32_t i = 0; i < _numberOfDirectionalLights; ++i)
	{
		pvr::FreeValue viewDir;
		viewDir.setValue(pass.directionalLightPass.lightProperties[i].viewSpaceLightDirection);
		auto& pipeline = _deviceResources->render_mgr.toPipeline(0, 0, static_cast<uint32_t>(RenderPassSubPass::Lighting),
		                 static_cast<uint32_t>(LightingSubpassGroup::DirectionalLight), static_cast<uint32_t>(LightingSubpassPipeline::DirectionalLighting));

		pvr::utils::RendermanNode& node = _deviceResources->render_mgr.toSubpassGroupModel(0, 0, static_cast<uint32_t>(RenderPassSubPass::Lighting),
		                                  static_cast<uint32_t>(LightingSubpassGroup::DirectionalLight), 0).toRendermanNode(i);
		pipeline.updateBufferEntryNodeSemantic("VIEWDIRECTION", viewDir, swapchain, node);
	}

	// update the procedural point lights
	for (; pointLight < numSceneLights + _numberOfPointLights; ++pointLight)
	{
		updateProceduralPointLight(pass.pointLightPasses.initialData[pointLight],
		                           _renderInfo.pointLightPasses.lightProperties[pointLight], pointLight);
	}
}

void VulkanDeferredShadingPFX::setProceduralPointLightInitialData(PointLightPasses::InitialData& data,
    PointLightPasses::PointLightProperties& pointLightProperties)
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


/*!*********************************************************************************************************************
\brief	Update the procedural point lights
***********************************************************************************************************************/
void VulkanDeferredShadingPFX::updateProceduralPointLight(PointLightPasses::InitialData& data,
    PointLightPasses::PointLightProperties& pointLightProperties, uint32_t pointLightIndex)
{
	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();
	if (!_isPaused) //Skip for the first m_frameNumber, as sometimes this moves the light too far...
	{
		float_t dt = std::min(getFrameTime(), uint64_t(30));
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
			data.axial_vel *= .8f;
		}
		if (glm::abs(data.radial_vel) > PointLightConfiguration::LightMaxRadialVelocity)
		{
			data.radial_vel *= .8f;
		}
		if (glm::abs(data.vertical_vel) > PointLightConfiguration::LightMaxVerticalVelocity)
		{
			data.vertical_vel *= .8f;
		}

		data.distance += data.axial_vel * dt * 0.001f;
		data.angle += data.radial_vel * dt * 0.001f;
		data.height += data.vertical_vel * dt * 0.001f;
	}

	const float x = sin(data.angle) * data.distance;
	const float z = cos(data.angle) * data.distance;
	const float y = data.height;
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

	pvr::FreeValue val;

	// update the Point Light step 1
	{
		pvr::utils::RendermanNode& pointLightNode = _deviceResources->render_mgr.toSubpassGroupModel(
		      0, 0, static_cast<uint32_t>(RenderPassSubPass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1), 0)
		    .toRendermanNode(pointLightIndex);

		pvr::utils::RendermanPipeline& pipeline = _deviceResources->render_mgr.toPipeline(0, 0,
		    static_cast<uint32_t>(RenderPassSubPass::Lighting), static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1), 0);

		val.setValue(pointLightProperties.proxyWorldViewProjectionMatrix);
		pipeline.updateBufferEntryNodeSemantic(PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::PROXYMODELVIEPROJECTIONMATRIX)],
		                                       val, swapchainIndex, pointLightNode);
	}

	// update the point light step 2
	{
		pvr::utils::RendermanNode& pointLightNode = _deviceResources->render_mgr.toSubpassGroupModel(
		      0, 0, static_cast<uint32_t>(RenderPassSubPass::Lighting),
		      static_cast<uint32_t>(LightingSubpassGroup::PointLightStep2), 0).toRendermanNode(pointLightIndex);

		pvr::utils::RendermanPipeline& pipeline = _deviceResources->render_mgr.toPipeline(0, 0,
		    static_cast<uint32_t>(RenderPassSubPass::Lighting),
		    static_cast<uint32_t>(LightingSubpassGroup::PointLightStep2), 0);

		val.setValue(pointLightProperties.proxyWorldViewMatrix);
		pipeline.updateBufferEntryNodeSemantic(PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::PROXYMODELVIEWMATRIX)],
		                                       val, swapchainIndex, pointLightNode);

		val.setValue(pointLightProperties.proxyWorldViewProjectionMatrix);
		pipeline.updateBufferEntryNodeSemantic(PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::PROXYMODELVIEPROJECTIONMATRIX)],
		                                       val, swapchainIndex, pointLightNode);

		val.setValue(pointLightProperties.proxyViewSpaceLightPosition);
		pipeline.updateBufferEntryNodeSemantic(PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::PROXYVIEWPOSITION)],
		                                       val, swapchainIndex, pointLightNode);
	}


	// update the Point Light step 3
	{
		pvr::utils::RendermanNode& pointLightNode = _deviceResources->render_mgr.toSubpassGroupModel(
		      0, 0, static_cast<uint32_t>(RenderPassSubPass::Lighting),
		      static_cast<uint32_t>(LightingSubpassGroup::PointLightStep3), 0).toRendermanNode(pointLightIndex);

		pvr::utils::RendermanPipeline& pipeline = _deviceResources->render_mgr.toPipeline(
		      0, 0, static_cast<uint32_t>(RenderPassSubPass::Lighting),
		      static_cast<uint32_t>(LightingSubpassGroup::PointLightStep3), 0);

		// update the Point light's dynamic buffers
		val.setValue(pointLightProperties.worldViewProjectionMatrix);
		pipeline.updateBufferEntryNodeSemantic(PFXSemanticsStr[static_cast<uint32_t>(PFXSemanticId::MODELVIEWPROJECTIONMATRIX)],
		                                       val, swapchainIndex, pointLightNode);
	}
}

/*!*********************************************************************************************************************
\brief	Updates animation variables and camera matrices.
***********************************************************************************************************************/
void VulkanDeferredShadingPFX::updateAnimation()
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
\brief	Initialise the static light properties
***********************************************************************************************************************/
void VulkanDeferredShadingPFX::initialiseStaticLightProperties()
{
	RenderData& pass = _renderInfo;

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

			//POINT LIGHT GEOMETRY : The spheres that will be used for the stencil pass
			pass.pointLightPasses.lightProperties[pointLight].lightColor = glm::vec4(light.getColor(), 1.f);

			//POINT LIGHT PROXIES : The "drawcalls" that will perform the actual rendering
			pass.pointLightPasses.lightProperties[pointLight].lightIntensity = PointLightConfiguration::PointlightIntensity;

			//POINT LIGHT PROXIES : The "drawcalls" that will perform the actual rendering
			pass.pointLightPasses.lightProperties[pointLight].lightRadius = PointLightConfiguration::PointLightRadius;

			//POINT LIGHT SOURCES : The little balls that we render to show the lights
			pass.pointLightPasses.lightProperties[pointLight].lightSourceColor = glm::vec4(light.getColor(), .8f);
			++pointLight;
		}
		break;
		case pvr::assets::Light::Directional:
		{
			pass.directionalLightPass.lightProperties[directionalLight].lightIntensity = glm::vec4(light.getColor(), 1.0f) *
			    DirectionalLightConfiguration::DirectionalLightIntensity;
			++directionalLight;
		}
		break;
		}
	}

	if (DirectionalLightConfiguration::AdditionalDirectionalLight)
	{
		pass.directionalLightPass.lightProperties[directionalLight].lightIntensity =
		  glm::vec4(1, 1, 1, 1) * DirectionalLightConfiguration::DirectionalLightIntensity;
		++directionalLight;
	}
}

/*!*********************************************************************************************************************
\brief Allocate memory for lighting data
***********************************************************************************************************************/
void VulkanDeferredShadingPFX::allocateLights()
{
	int32_t countPoint = 0;
	uint32_t countDirectional = 0;
	for (uint32_t i = 0; i < _mainScene->getNumLightNodes(); ++i)
	{
		switch (_mainScene->getLight(_mainScene->getLightNode(i).getObjectId()).getType())
		{
		case pvr::assets::Light::Directional:
			++countDirectional;
			break;
		case pvr::assets::Light::Point:
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

	_renderInfo.directionalLightPass.lightProperties.resize(countDirectional);
	_renderInfo.pointLightPasses.lightProperties.resize(countPoint);
	_renderInfo.pointLightPasses.initialData.resize(countPoint);

	for (int i = countPoint - PointLightConfiguration::NumProceduralPointLights; i < countPoint; ++i)
	{
		setProceduralPointLightInitialData(_renderInfo.pointLightPasses.initialData[i],
		                                   _renderInfo.pointLightPasses.lightProperties[i]);
	}
}

/*!*********************************************************************************************************************
\brief	Records main command buffer
***********************************************************************************************************************/
void VulkanDeferredShadingPFX::recordMainCommandBuffer()
{
	const pvrvk::Rect2Di renderArea(0, 0, _windowWidth, _windowHeight);

	// Populate the clear values
	pvrvk::ClearValue clearValue[8];
	pvr::utils::populateClearValues(_deviceResources->render_mgr.toPass(0, 0).getFramebuffer(0)->getRenderPass(),
	                                pvrvk::ClearValue(0.f, 0.0f, 0.0f, 1.0f), pvrvk::ClearValue(1.f, 0u), clearValue);

	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		_deviceResources->commandBufferMain[i]->begin();

		pvrvk::Framebuffer framebuffer  = _deviceResources->render_mgr.toPass(0, 0).getFramebuffer(i);

		// Prepare the image for Presenting
		pvr::utils::setImageLayout(_deviceResources->swapchain->getImage(i), VkImageLayout::e_PRESENT_SRC_KHR,
		                           VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, _deviceResources->commandBufferMain[i]);

		/// 1) Begin the render pass
		_deviceResources->commandBufferMain[i]->beginRenderPass(_deviceResources->render_mgr.toPass(0, 0).framebuffer[i],
		    renderArea, true, clearValue, framebuffer->getNumAttachments());

		/// 2) Record the scene in to the gbuffer
		_deviceResources->render_mgr.toSubpass(0, 0, static_cast<uint32_t>(RenderPassSubPass::GBuffer))
		.recordRenderingCommands(_deviceResources->commandBufferMain[i], i, false);

		/// 3) Begin the next subpass
		_deviceResources->commandBufferMain[i]->nextSubpass(VkSubpassContents::e_INLINE);

		/// 4) record the directional lights Geometry stencil. Draw stencil to discard useless pixels
		_deviceResources->render_mgr.toSubpassGroup(0, 0, static_cast<uint32_t>(RenderPassSubPass::Lighting),
		    static_cast<uint32_t>(LightingSubpassGroup::DirectionalLight)).recordRenderingCommands(_deviceResources->commandBufferMain[i], i);

		for (uint32_t j = 0; j < _numberOfPointLights; j++)
		{
			/// 5) record the point light stencil
			recordCommandsPointLightGeometryStencil(_deviceResources->commandBufferMain[i], i, j);

			/// 6) record the point light proxy
			_deviceResources->render_mgr.toSubpassGroup(0, 0, static_cast<uint32_t>(RenderPassSubPass::Lighting),
			    static_cast<uint32_t>(LightingSubpassGroup::PointLightStep2)).toSubpassGroupModel(0).nodes[j].recordRenderingCommands(_deviceResources->commandBufferMain[i], i);
		}

		/// 7) record the pointlight source
		_deviceResources->render_mgr.toSubpassGroup(0, 0, static_cast<uint32_t>(RenderPassSubPass::Lighting),
		    static_cast<uint32_t>(LightingSubpassGroup::PointLightStep3)).recordRenderingCommands(_deviceResources->commandBufferMain[i], i);

		/// 8) Render ui
		_deviceResources->uiRenderer.beginRendering(_deviceResources->commandBufferMain[i]);
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getDefaultControls()->render();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.endRendering();
		_deviceResources->commandBufferMain[i]->endRenderPass();

		// Prepare the image for Presenting
		pvr::utils::setImageLayout(_deviceResources->swapchain->getImage(i), VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL,
		                           VkImageLayout::e_PRESENT_SRC_KHR, _deviceResources->commandBufferMain[i]);
		_deviceResources->commandBufferMain[i]->end();
	}
}

/*!*********************************************************************************************************************
\brief	Record point light stencil commands
\param  commandBuffer SecondaryCommandBuffer to record
\param swapChainIndex Current swap chain index
\param subpass Current sub pass
***********************************************************************************************************************/
void VulkanDeferredShadingPFX::recordCommandsPointLightGeometryStencil(
  pvrvk::CommandBuffer& commandBuffer, uint32_t swapChainIndex, const uint32_t pointLight)
{
	pvrvk::ClearRect clearArea(pvrvk::Rect2Di(0, 0, _framebufferWidth, _framebufferHeight));
	if ((_framebufferWidth != _windowWidth) || (_framebufferHeight != _windowHeight))
	{
		clearArea.rect = pvrvk::Rect2Di(_viewportOffsets[0], _viewportOffsets[1], _framebufferWidth, _framebufferHeight);
	}

	// clear stencil to 0's to make use of it again for point lights
	commandBuffer->clearAttachment(pvrvk::ClearAttachment::createStencilClearAttachment(0u), clearArea);

	// record the rendering commands for the point light stencil pass
	_deviceResources->render_mgr.toSubpassGroup(0, 0, static_cast<uint32_t>(RenderPassSubPass::Lighting),
	    static_cast<uint32_t>(LightingSubpassGroup::PointLightStep1)).toSubpassGroupModel(0).nodes[pointLight].recordRenderingCommands(commandBuffer, swapChainIndex);
}

/*!*********************************************************************************************************************
\return	Return an unique_ptr to a new Demo class, supplied by the user
\brief	This function must be implemented by the user of the shell. The user should return its Shell object defining the
behaviour of the application.
***********************************************************************************************************************/
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::unique_ptr<Shell>(new VulkanDeferredShadingPFX()); }
