/*!*********************************************************************************************************************
\File         VulkanDeferredShadingPFX.cpp
\Title        Deferred Shading
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\Description  Implements a deferred shading technique supporting point and directional lights using pfx.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVREngineUtils/PVREngineUtils.h"
#include "PVRApi/ApiUtils.h"
using namespace pvr;

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

static const StringHash PFXSemanticsStr[] =
{
	"MODELVIEWPROJECTIONMATRIX",
	"MODELVIEWMATRIX",
	"MODELWORLDITMATRIX",
	"VIEWPOSITION",
	"PROXYMODELVIEWPROJECTIONMATRIX",
	"PROXYMODELVIEWMATRIX",
	"PROXYVIEWPOSITION",
	"LIGHTINTENSITY",
	"LIGHTCOLOR",
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
	LIGHTCOLOR,
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
		glm::vec4 lightIntensity;
	};

	std::vector<PointLightProperties> lightProperties;

	struct InitialData
	{
		float32 radial_vel;
		float32 axial_vel;
		float32 vertical_vel;
		float32 angle;
		float32 distance;
		float32 height;
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
		FreeValue world;
		FreeValue worldView;
		FreeValue worldViewProj;
		FreeValue worldViewIT4x4;
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
const float32 FrameRate = 1.0f / 120.0f;
}

// Directional lighting configuration data
namespace DirectionalLightConfiguration {
static bool AdditionalDirectionalLight = true;
const float32 DirectionalLightIntensity = .2f;
}

// Point lighting configuration data
namespace PointLightConfiguration {
float32 LightMaxDistance = 40.f;
float32 LightMinDistance = 20.f;
float32 LightMinHeight = -30.f;
float32 LightMaxHeight = 40.f;
float32 LightAxialVelocityChange = .01f;
float32 LightRadialVelocityChange = .003f;
float32 LightVerticalVelocityChange = .01f;
float32 LightMaxAxialVelocity = 5.f;
float32 LightMaxRadialVelocity = 1.5f;
float32 LightMaxVerticalVelocity = 5.f;

static int32 MaxScenePointLights = 5;
static int32 NumProceduralPointLights  = 10;
float32 PointLightScale = 40.0f;
float32 PointlightIntensity = 100.0f;
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
class VulkanDeferredShadingPFX : public Shell
{
public:
	struct Material
	{
		api::GraphicsPipeline materialPipeline;
		std::vector<api::DescriptorSet> materialDescriptorSet;
		float32 specularStrength;
		glm::vec3 diffuseColor;
	};

	struct DeviceResources
	{
		//// Command Buffers ////
		// Main Primary Command Buffer
		api::CommandBuffer cmdBufferMain[(uint32)FrameworkCaps::MaxSwapChains];

		//// UI Renderer ////
		ui::UIRenderer uiRenderer;
		utils::RenderManager render_mgr;

		//// Frame ////
		uint32 numSwapImages;
		uint8 swapIndex;
	};

	// Context
	GraphicsContext _context;

	//Putting all api objects into a pointer just makes it easier to release them all together with RAII
	std::auto_ptr<DeviceResources> _devObj;

	// Provides easy management of assets
	utils::AssetStore _assetManager;

	// Frame counters for animation
	float32 _frameNumber;
	bool _isPaused;
	uint32 _cameraId;
	bool _animateCamera;

	uint32 _numberOfPointLights;
	uint32 _numberOfDirectionalLights;

	// Projection and Model View matrices
	glm::vec3 _cameraPosition;
	glm::mat4 _viewMatrix;
	glm::mat4 _projectionMatrix;
	glm::mat4 _viewProjectionMatrix;
	glm::mat4 _inverseViewMatrix;
	float32 _farClipDistance;

	int32 _windowWidth;
	int32 _windowHeight;
	int32 _framebufferWidth;
	int32 _framebufferHeight;

	int32 _viewportOffsets[2];

	// Light models
	assets::ModelHandle _pointLightModel;

	// Object model
	assets::ModelHandle _mainScene;

	RenderData _renderInfo;

	VulkanDeferredShadingPFX() { _animateCamera = false; _isPaused = false; }

	//	Overriden from Shell
	virtual Result initApplication();
	virtual Result initView();
	virtual Result releaseView();
	virtual Result quitApplication();
	virtual Result renderFrame();

	void recordCommandsPointLightGeometryStencil(api::CommandBuffer& cmdBuffer, uint32 swapChainIndex);
	void recordMainCommandBuffer();
	void allocateLights();
	void uploadStaticData();
	void uploadStaticSceneData();
	void uploadStaticDirectionalLightData();
	void uploadStaticPointLightData();
	void initialiseStaticLightProperties();
	void updateDynamicSceneData(uint32 swapchain);
	void updateDynamicLightData(uint32 swapchain);
	void updateAnimation(bool forceUpdate = false);
	void updateProceduralPointLight(PointLightPasses::InitialData& data,
	                                PointLightPasses::PointLightProperties& pointLightProperties, uint32 pointLightIndex);

	void updateGBufferPass()
	{
		auto& pipeline = _devObj->render_mgr.toPipeline(0, 0, (uint32)RenderPassSubPass::GBuffer, 0, 0);
		pipeline.updateAutomaticModelSemantics(0);
		_devObj->render_mgr.toSubpassGroupModel(0, 0, (uint32)RenderPassSubPass::GBuffer, 0, 0).updateFrame(0);
	}

	void eventMappedInput(SimplifiedInput key)
	{
		switch (key)
		{
		// Handle input
		case SimplifiedInput::ActionClose: exitShell(); break;
		case SimplifiedInput::Action1: _isPaused = !_isPaused; break;
		case SimplifiedInput::Action2: _animateCamera = !_animateCamera; break;
		}
	}
	assets::ModelHandle createFullScreenQuadMesh();
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
Result VulkanDeferredShadingPFX::initApplication()
{
	// This demo application makes heavy use of the stencil buffer
	setStencilBitsPerPixel(8);

	_frameNumber = 0.0f;
	_isPaused = false;
	_cameraId = 0;

	//Prepare the asset manager for loading our objects
	return Result::Success;
}

assets::ModelHandle VulkanDeferredShadingPFX::createFullScreenQuadMesh()
{
	assets::ModelHandle m;	m.construct();
	m->allocMeshes(_numberOfDirectionalLights);
	m->allocMeshNodes(_numberOfDirectionalLights);
	// create a dummy material with a material attribute which will be identified by the pfx.
	m->addMaterial(assets::Material());
	m->getMaterial(0).setMaterialAttribute("DIR_LIGHT", FreeValue());
	const uint32 data[] = {0, 1, 2, 1, 2, 3};
	for (uint32 i = 0; i < _numberOfDirectionalLights; ++i)
	{
		m->getMesh(i).addFaces((const byte*)data, sizeof(data), types::IndexType::IndexType32Bit);
		m->getMesh(i).setPrimitiveType(types::PrimitiveTopology::TriangleStrip);
		m->getMesh(i).setNumVertices(4);
		m->getMesh(i).setNumFaces(2);
		m->getMesh(i).addVertexAttribute("POSITION", types::DataType::Float32, 3, 0, 0);
		m->getMesh(i).addVertexAttribute("UV", types::DataType::Float32, 2, sizeof(float32) * 3, 0);
		m->connectMeshWithMeshNode(i, i);
		m->getMeshNode(i).setMaterialIndex(0);
	}
	return m;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in initView() will be called by PVRShell upon initialization or after a change in the rendering context.
Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
Result VulkanDeferredShadingPFX::initView()
{
	//Create the empty API objects.
	_devObj.reset(new DeviceResources);

	//Initialize free-floating objects (commandBuffers).
	_context = getGraphicsContext();

	// Get the number of swap images
	_devObj->numSwapImages = getPlatformContext().getSwapChainLength();

	// Get current swap index
	_devObj->swapIndex = _context->getPlatformContext().getSwapChainIndex();

	_assetManager.init(*this);

	//  Load the scene and the light
	if (!_assetManager.loadModel(Files::SceneFile, _mainScene))
	{
		setExitMessage("ERROR: Couldn't load the scene pod file %s\n", Files::SceneFile);
		return Result::UnknownError;
	}
	// Initialise lighting structures
	allocateLights();

	//	Load light proxy geometry
	if (!_assetManager.loadModel(Files::PointLightModelFile, _pointLightModel))
	{
		setExitMessage("ERROR: Couldn't load the point light proxy pod file\n");
		return Result::UnableToOpen;
	}

	const platform::CommandLine& cmdOptions = getCommandLine();

	cmdOptions.getIntOption("-fbowidth", _framebufferWidth);
	_framebufferWidth = glm::min<int32>(_framebufferWidth, _windowWidth);
	cmdOptions.getIntOption("-fboheight", _framebufferHeight);
	_framebufferHeight = glm::min<int32>(_framebufferHeight, _windowHeight);
	cmdOptions.getIntOption("-numlights", PointLightConfiguration::NumProceduralPointLights);
	cmdOptions.getFloatOption("-lightscale", PointLightConfiguration::PointLightScale);
	cmdOptions.getFloatOption("-lightintensity", PointLightConfiguration::PointlightIntensity);

	_viewportOffsets[0] = (_windowWidth - _framebufferWidth) / 2;
	_viewportOffsets[1] = (_windowHeight - _framebufferHeight) / 2;

	Log(Log.Information, "FBO dimensions: %d x %d\n", _framebufferWidth, _framebufferHeight);
	Log(Log.Information, "Onscreen Framebuffer dimensions: %d x %d\n", _windowWidth, _windowHeight);

	// setup command buffers
	for (uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		_devObj->cmdBufferMain[i] = _context->createCommandBufferOnDefaultPool();
	}

	_projectionMatrix = utils::getPerspectiveMatrix(*getGraphicsContext(), _mainScene->getCamera(0).getFOV(),
	                    _mainScene->getCamera(0).getNear(), _mainScene->getCamera(0).getFar());


	// allocate number of point light mesh nodes which will uses the same material and the mesh
	_numberOfPointLights = PointLightConfiguration::NumProceduralPointLights;
	_pointLightModel->allocMeshNodes(_numberOfPointLights);
	_pointLightModel->connectMeshWithMeshNodes(0, 0, _numberOfPointLights - 1);
	_pointLightModel->addMaterial(assets::Material());
	_pointLightModel->getMaterial(0).setMaterialAttribute("POINT_LIGHT", FreeValue());
	_pointLightModel->assignMaterialToMeshNodes(0, 0,_numberOfPointLights - 1);


	//--- create the pfx effect
	assets::pfx::PfxParser rd(Files::EffectPfx, this);
	_devObj->render_mgr.addEffect(*rd.getAssetHandle(), getGraphicsContext(), _assetManager);

	//--- Gbuffer renders the scene
	_devObj->render_mgr.addModelForAllSubpassGroups(_mainScene, 0, (uint32)RenderPassSubPass::GBuffer, 0);

	//--- add the full screen quad mesh to the directional light subpass group in lighting subpass
	_devObj->render_mgr.addModelForSubpassGroup(createFullScreenQuadMesh(), 0, (uint32)RenderPassSubPass::Lighting,
	    (uint32)LightingSubpassGroup::DirectionalLight);

	//--- add the point lights to the Pointlight subpass groups in lighting subpass
	_devObj->render_mgr.addModelForSubpassGroup(_pointLightModel, 0, (uint32)RenderPassSubPass::Lighting,
	    (uint32)LightingSubpassGroup::PointLightStep1);

	_devObj->render_mgr.addModelForSubpassGroup(_pointLightModel, 0, (uint32)RenderPassSubPass::Lighting,
	    (uint32)LightingSubpassGroup::PointLightStep2);

	_devObj->render_mgr.addModelForSubpassGroup(_pointLightModel, 0, (uint32)RenderPassSubPass::Lighting,
	    (uint32)LightingSubpassGroup::PointLightStep3);

	// build all the renderman objects
	_devObj->render_mgr.buildRenderObjects();

	// initialize the UIRenderer and set the title text
	_devObj->uiRenderer.init(_devObj->render_mgr.toPass(0, 0).getFbo(0)->getRenderPass(),
	                         (uint32)RenderPassSubPass::Lighting);
	_devObj->uiRenderer.getDefaultTitle()->setText("DeferredShadingPFX").commitUpdates();

	_devObj->uiRenderer.getDefaultControls()->setText("Action1: Pause\nAction2: Orbit Camera\n");
	_devObj->uiRenderer.getDefaultControls()->commitUpdates();

	// initialise the gbuffer renderpass list
	_renderInfo.storeLocalMemoryPass.objects.resize(_mainScene->getNumMeshNodes());

	// calculate the frame buffer width and heights
	_framebufferWidth = _windowWidth = this->getWidth();
	_framebufferHeight = _windowHeight = this->getHeight();

	updateAnimation(true);
	// Upload static data
	initialiseStaticLightProperties();
	uploadStaticData();
	for (uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		updateDynamicSceneData(i);
	}

	// Record the main command buffer
	recordMainCommandBuffer();
	return Result::Success;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
Result VulkanDeferredShadingPFX::releaseView()
{
	_assetManager.releaseAll();
	_devObj.reset(0);
	_context.release();
	return Result::Success;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
If the rendering context is lost, QuitApplication() will not be called.x
***********************************************************************************************************************/
Result VulkanDeferredShadingPFX::quitApplication() { return Result::Success; }

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every m_frameNumber.
***********************************************************************************************************************/
Result VulkanDeferredShadingPFX::renderFrame()
{
	// Get the current swap index
	_devObj->swapIndex = getSwapChainIndex();

	//  Handle user input and update object animations
	updateAnimation();

	_devObj->render_mgr.updateAutomaticSemantics(_devObj->swapIndex);

	// update the scene dynamic buffer only if the camera is animated.
	if (_animateCamera)
	{
		updateDynamicSceneData(_devObj->swapIndex);
	}
	updateDynamicLightData(getSwapChainIndex());

	// submit the main command buffer
	_devObj->cmdBufferMain[_devObj->swapIndex]->submit();

	return Result::Success;
}

/*!*********************************************************************************************************************
\brief	Upload the static data to the buffers which do not change per frame
***********************************************************************************************************************/
void VulkanDeferredShadingPFX::uploadStaticSceneData()
{
	// static scene properties buffer
	_farClipDistance = _mainScene->getCamera(0).getFar();

	FreeValue farClipDist;
	farClipDist.setValue(_farClipDistance);

	FreeValue specStrength;
	specStrength.setValue(.5f);

	FreeValue diffColor;
	diffColor.setValue(glm::vec4(1.f));

	utils::RendermanSubpassGroupModel& model = _devObj->render_mgr.toSubpassGroupModel(0, 0, (uint32)RenderPassSubPass::GBuffer, 0, 0);
	for (uint32 i = 0; i < model.getNumRendermanNodes(); ++i)
	{
		auto& pipeline = model.toRendermanNode(i).toRendermanPipeline();
		assets::Material& material =  _mainScene->getMaterial(_mainScene->getMeshNode(model.toRendermanNode(i).assetNodeId).getMaterialIndex());
		specStrength.setValue(material.defaultSemantics().getShininess());
		diffColor.setValue(glm::vec4(material.defaultSemantics().getDiffuse(), 1.f));
		pipeline.updateBufferEntryModelSemantic("FARCLIPDIST", farClipDist, 0, i);
		pipeline.updateBufferEntryModelSemantic("SPECULARSTRENGTH", specStrength, 0, i);
		pipeline.updateBufferEntryModelSemantic("DIFFUSECOLOUR", diffColor, 0, i);
	}
}

/*!*********************************************************************************************************************
\brief	Upload the static data to the buffers which do not change per frame
***********************************************************************************************************************/
void VulkanDeferredShadingPFX::uploadStaticDirectionalLightData()
{
	FreeValue mem;
	for (uint32 i = 0; i < _numberOfDirectionalLights; ++i)
	{
		mem.setValue(_renderInfo.directionalLightPass.lightProperties[i].lightIntensity);
		_devObj->render_mgr.toPipeline(0, 0, (uint32)RenderPassSubPass::Lighting,
		    (uint32) LightingSubpassGroup::DirectionalLight, (uint32)LightingSubpassPipeline::DirectionalLighting)
		.updateBufferEntryNodeSemantic("LIGHTINTENSITY", mem, getSwapChainIndex(),
		    _devObj->render_mgr.toSubpassGroupModel(0, 0, (uint32)RenderPassSubPass::Lighting,
		    (uint32) LightingSubpassGroup::DirectionalLight,
		    (uint32)LightingSubpassPipeline::DirectionalLighting).toRendermanNode(i));
	}
}

/*!*********************************************************************************************************************
\brief	Upload the static data to the buffers which do not change per frame
***********************************************************************************************************************/
void VulkanDeferredShadingPFX::uploadStaticPointLightData()
{
	// static point lighting buffer
	FreeValue values[2];
	for (uint32 i = 0; i < _numberOfPointLights; ++i)
	{
		// LIGHTINTENSITY
		values[0].setValue(_renderInfo.pointLightPasses.lightProperties[i].lightIntensity);
		// LIGHTCOLOR
		values[1].setValue(_renderInfo.pointLightPasses.lightProperties[i].lightColor);

		// Point light proxy pass
		{
			utils::RendermanNode& node = _devObj->render_mgr.toSubpassGroupModel(0, 0, (uint32)RenderPassSubPass::Lighting,
			                             (uint32)LightingSubpassGroup::PointLightStep2, 0).toRendermanNode(i);
			_devObj->render_mgr.toPipeline(0, 0, (uint32)RenderPassSubPass::Lighting,
			                               (uint32)LightingSubpassGroup::PointLightStep2, (uint32)0).updateBufferEntryNodeSemantic(
			                                 PFXSemanticsStr[(uint32)PFXSemanticId::LIGHTINTENSITY], values[0], 0, node);
		}

		// Point light source pass
		{
			utils::RendermanNode& node = _devObj->render_mgr.toSubpassGroupModel(0, 0, (uint32)RenderPassSubPass::Lighting,
			                             (uint32)LightingSubpassGroup::PointLightStep3, 0).toRendermanNode(i);
			_devObj->render_mgr.toPipeline(0, 0, (uint32)RenderPassSubPass::Lighting,
			                               (uint32)LightingSubpassGroup::PointLightStep3, (uint32)0).updateBufferEntryNodeSemantic(
			                                 PFXSemanticsStr[(uint32)PFXSemanticId::LIGHTCOLOR], values[1], 0, node);
		}
	}

	// set the far clip distance for the point light step 2
	const float32 val = 1000.f;
	values[0].setValue(val);
	_devObj->render_mgr.toPipeline(0, 0, (uint32)RenderPassSubPass::Lighting, (uint32)LightingSubpassGroup::PointLightStep2,
	                               (uint32)0).updateBufferEntryModelSemantic(PFXSemanticsStr[(uint32)PFXSemanticId::FARCLIPDIST], values[0], 0);
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
void VulkanDeferredShadingPFX::updateDynamicSceneData(uint32 swapchain)
{
	RenderData& pass = _renderInfo;

	// update the model matrices
	static glm::mat4 world, worldView;
	for (uint32 i = 0; i < _mainScene->getNumMeshNodes(); ++i)
	{
		const assets::Model::Node& node = _mainScene->getNode(i);

		world = _mainScene->getWorldMatrix(node.getObjectId());
		worldView = _viewMatrix * world;
		pass.storeLocalMemoryPass.objects[i].world.setValue(world);
		pass.storeLocalMemoryPass.objects[i].worldView.setValue(worldView);
		pass.storeLocalMemoryPass.objects[i].worldViewIT4x4.setValue(glm::inverseTranspose(worldView));
		pass.storeLocalMemoryPass.objects[i].worldViewProj.setValue(_projectionMatrix * worldView);

		utils::RendermanNode& rendermanNode = _devObj->render_mgr.toSubpassGroupModel(0, 0,
		                                      (uint32)RenderPassSubPass::GBuffer, 0, 0).toRendermanNode(i);

		utils::RendermanPipeline& pipe = rendermanNode.toRendermanPipeline();

		pipe.updateBufferEntryNodeSemantic(PFXSemanticsStr[(uint32)PFXSemanticId::MODELVIEWPROJECTIONMATRIX],
		                                   pass.storeLocalMemoryPass.objects[i].worldViewProj, swapchain, rendermanNode);

		pipe.updateBufferEntryNodeSemantic(PFXSemanticsStr[(uint32)PFXSemanticId::MODELVIEWMATRIX],
		                                   pass.storeLocalMemoryPass.objects[i].worldView, swapchain, rendermanNode);

		pipe.updateBufferEntryNodeSemantic(PFXSemanticsStr[(uint32)PFXSemanticId::MODELWORLDITMATRIX],
		                                   pass.storeLocalMemoryPass.objects[i].worldViewIT4x4, swapchain, rendermanNode);
	}
}

void VulkanDeferredShadingPFX::updateDynamicLightData(uint32 swapchain)
{
	int32 pointLight = 0;
	uint32 directionalLight = 0;
	RenderData& pass = _renderInfo;
	// update the lighting data
	for (uint32 i = 0; i < _mainScene->getNumLightNodes(); ++i)
	{
		const assets::Node& lightNode = _mainScene->getLightNode(i);
		const assets::Light& light = _mainScene->getLight(lightNode.getObjectId());
		switch (light.getType())
		{
		case assets::Light::Point:
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
		case assets::Light::Directional:
		{
			const glm::mat4& transMtx = _mainScene->getWorldMatrix(_mainScene->getNodeIdFromLightNodeId(i));
			pass.directionalLightPass.lightProperties[directionalLight].viewSpaceLightDirection =
			  _viewMatrix * transMtx * glm::vec4(0.f, -1.f, 0.f, 0.f);
			++directionalLight;
		}
		break;
		}
	}

	int numSceneLights = pointLight;
	if (DirectionalLightConfiguration::AdditionalDirectionalLight)
	{
		pass.directionalLightPass.lightProperties[directionalLight].viewSpaceLightDirection =
		  _viewMatrix * glm::vec4(0.f, -1.f, 0.f, 0.f);
		++directionalLight;
	}


	// update the directional light pipeline
	for (uint32 i = 0; i < _numberOfDirectionalLights; ++i)
	{
		FreeValue viewDir; viewDir.setValue(pass.directionalLightPass.lightProperties[i].viewSpaceLightDirection);
		auto& pipeline = _devObj->render_mgr.toPipeline(0, 0, (uint32)RenderPassSubPass::Lighting,
		                 (uint32)LightingSubpassGroup::DirectionalLight, (uint32)LightingSubpassPipeline::DirectionalLighting);

		utils::RendermanNode& node = _devObj->render_mgr.toSubpassGroupModel(0, 0, (uint32)RenderPassSubPass::Lighting,
		                             (uint32)LightingSubpassGroup::DirectionalLight, 0).toRendermanNode(i);
		pipeline.updateBufferEntryNodeSemantic("VIEWDIRECTION", viewDir, swapchain, node);
	}

	// update the procedural point lights
	for (; pointLight < numSceneLights + PointLightConfiguration::NumProceduralPointLights; ++pointLight)
	{
		updateProceduralPointLight(pass.pointLightPasses.initialData[pointLight],
		                           _renderInfo.pointLightPasses.lightProperties[pointLight], pointLight);
	}
}


void VulkanDeferredShadingPFX::setProceduralPointLightInitialData(PointLightPasses::InitialData& data,
    PointLightPasses::PointLightProperties& pointLightProperties)
{
	data.distance = randomrange(PointLightConfiguration::LightMinDistance, PointLightConfiguration::LightMaxDistance);
	data.angle = randomrange(-glm::pi<float32>(), glm::pi<float32>());
	data.height = randomrange(PointLightConfiguration::LightMinHeight, PointLightConfiguration::LightMaxHeight);
	data.axial_vel = randomrange(-PointLightConfiguration::LightMaxAxialVelocity, PointLightConfiguration::LightMaxAxialVelocity);
	data.radial_vel = randomrange(-PointLightConfiguration::LightMaxRadialVelocity, PointLightConfiguration::LightMaxRadialVelocity);
	data.vertical_vel = randomrange(-PointLightConfiguration::LightMaxVerticalVelocity, PointLightConfiguration::LightMaxVerticalVelocity);

	glm::vec3 lightColor = glm::vec3(randomrange(0, 1), randomrange(0, 1), randomrange(0, 1));
	lightColor / glm::max(glm::max(lightColor.x, lightColor.y), lightColor.z); //Have at least one component equal to 1... We want them bright-ish...
	pointLightProperties.lightColor = glm::vec4(lightColor, 1.);//random-looking
	pointLightProperties.lightIntensity = glm::vec4(lightColor, 1.0f) * PointLightConfiguration::PointlightIntensity;
}


/*!*********************************************************************************************************************
\brief	Update the procedural point lights
***********************************************************************************************************************/
void VulkanDeferredShadingPFX::updateProceduralPointLight(PointLightPasses::InitialData& data,
    PointLightPasses::PointLightProperties& pointLightProperties, uint32 pointLightIndex)
{
	if (!_isPaused) //Skip for the first m_frameNumber, as sometimes this moves the light too far...
	{
		float32 dt = (float32)std::min(getFrameTime(), 30ull);
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

		data.axial_vel += randomrange(-PointLightConfiguration::LightAxialVelocityChange,
		                              PointLightConfiguration::LightAxialVelocityChange) * dt;

		data.radial_vel += randomrange(-PointLightConfiguration::LightRadialVelocityChange,
		                               PointLightConfiguration::LightRadialVelocityChange) * dt;

		data.vertical_vel += randomrange(-PointLightConfiguration::LightVerticalVelocityChange,
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

	const float32 x = sin(data.angle) * data.distance;
	const float32 z = cos(data.angle) * data.distance;
	const float32 y = data.height;
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

	FreeValue val;

	// update the Point Light step 1
	{

		utils::RendermanNode& pointLightNode = _devObj->render_mgr.toSubpassGroupModel(0, 0, (uint32)RenderPassSubPass::Lighting,
		                                       (uint32)LightingSubpassGroup::PointLightStep1, 0).toRendermanNode(pointLightIndex);

		utils::RendermanPipeline& pipeline = _devObj->render_mgr.toPipeline(0, 0, (uint32)RenderPassSubPass::Lighting,
		                                     (uint32)LightingSubpassGroup::PointLightStep1, 0);

		// update the Point light's dynamic buffers
//		val.setValue(pointLightProperties.worldViewProjectionMatrix);
//		pipeline.updateBufferEntryNodeSemantic(PFXSemanticsStr[(uint32)PFXSemanticId::MODELVIEWPROJECTIONMATRIX],
//		                                       val, getSwapChainIndex(), pointLightNode);


//		val.setValue(pointLightProperties.proxyWorldViewMatrix);
//		pipeline.updateBufferEntryNodeSemantic(PFXSemanticsStr[(uint32)PFXSemanticId::PROXYMODELVIEWMATRIX],
//		                                       val, getSwapChainIndex(), pointLightNode);

		val.setValue(pointLightProperties.proxyWorldViewProjectionMatrix);
		pipeline.updateBufferEntryNodeSemantic(PFXSemanticsStr[(uint32)PFXSemanticId::PROXYMODELVIEPROJECTIONMATRIX],
		                                       val, getSwapChainIndex(), pointLightNode);
	}

	// update the point light step 2
	{
		utils::RendermanNode& pointLightNode = _devObj->render_mgr.toSubpassGroupModel(0, 0, (uint32)RenderPassSubPass::Lighting,
		                                       (uint32)LightingSubpassGroup::PointLightStep2, 0).toRendermanNode(pointLightIndex);

		utils::RendermanPipeline& pipeline = _devObj->render_mgr.toPipeline(0, 0, (uint32)RenderPassSubPass::Lighting,
		                                     (uint32)LightingSubpassGroup::PointLightStep2, 0);

		val.setValue(pointLightProperties.proxyWorldViewMatrix);
		pipeline.updateBufferEntryNodeSemantic(PFXSemanticsStr[(uint32)PFXSemanticId::PROXYMODELVIEWMATRIX],
		                                       val, getSwapChainIndex(), pointLightNode);

		val.setValue(pointLightProperties.proxyWorldViewProjectionMatrix);
		pipeline.updateBufferEntryNodeSemantic(PFXSemanticsStr[(uint32)PFXSemanticId::PROXYMODELVIEPROJECTIONMATRIX],
		                                       val, getSwapChainIndex(), pointLightNode);

		val.setValue(pointLightProperties.proxyViewSpaceLightPosition);
		pipeline.updateBufferEntryNodeSemantic(PFXSemanticsStr[(uint32)PFXSemanticId::PROXYVIEWPOSITION],
		                                       val, getSwapChainIndex(), pointLightNode);
	}


	// update the Point Light step 3
	{
		utils::RendermanNode& pointLightNode = _devObj->render_mgr.toSubpassGroupModel(0, 0, (uint32)RenderPassSubPass::Lighting,
		                                       (uint32)LightingSubpassGroup::PointLightStep3, 0).toRendermanNode(pointLightIndex);

		utils::RendermanPipeline& pipeline = _devObj->render_mgr.toPipeline(0, 0, (uint32)RenderPassSubPass::Lighting,
		                                     (uint32)LightingSubpassGroup::PointLightStep3, 0);

		// update the Point light's dynamic buffers
		val.setValue(pointLightProperties.worldViewProjectionMatrix);
		pipeline.updateBufferEntryNodeSemantic(PFXSemanticsStr[(uint32)PFXSemanticId::MODELVIEWPROJECTIONMATRIX],
		                                       val, getSwapChainIndex(), pointLightNode);
	}
}

/*!*********************************************************************************************************************
\brief	Updates animation variables and camera matrices.
***********************************************************************************************************************/
void VulkanDeferredShadingPFX::updateAnimation(bool forceUpdate)
{
	// Update camera matrices
	static float32 angle = 0;
	if (!_isPaused)
	{
		_frameNumber += getFrameTime() * ApplicationConfiguration::FrameRate;
		if (_frameNumber > _mainScene->getNumFrames() - 1) { _frameNumber = 0; }
		_mainScene->setCurrentFrame(_frameNumber);
	}
	if (_animateCamera || forceUpdate)
	{
		glm::vec3 vTo, vUp;
		float32 fov;
		_mainScene->getCameraProperties(_cameraId, fov, _cameraPosition, vTo, vUp);
		angle += getFrameTime() / 1000.f;

		// recalculate the matrix
		_viewMatrix = glm::lookAt(glm::vec3(sin(angle) * 100.f + vTo.x, vTo.y + 30., cos(angle) * 100.f + vTo.z), vTo, vUp);
		_viewProjectionMatrix = _projectionMatrix * _viewMatrix;
		_inverseViewMatrix = glm::inverse(_viewMatrix);
	}
}

/*!*********************************************************************************************************************
\brief	Initialise the static light properties
***********************************************************************************************************************/
void VulkanDeferredShadingPFX::initialiseStaticLightProperties()
{
	RenderData& pass = _renderInfo;

	int32 pointLight = 0;
	uint32 directionalLight = 0;
	for (uint32 i = 0; i < _mainScene->getNumLightNodes(); ++i)
	{
		const assets::Node& lightNode = _mainScene->getLightNode(i);
		const assets::Light& light = _mainScene->getLight(lightNode.getObjectId());
		switch (light.getType())
		{
		case assets::Light::Point:
		{
			if (pointLight >= PointLightConfiguration::MaxScenePointLights)
			{
				continue;
			}

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
		case assets::Light::Directional:
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
	int32 countPoint = 0;
	uint32 countDirectional = 0;
	for (uint32 i = 0; i < _mainScene->getNumLightNodes(); ++i)
	{
		switch (_mainScene->getLight(_mainScene->getLightNode(i).getObjectId()).getType())
		{
		case assets::Light::Directional:
			++countDirectional;
			break;
		case assets::Light::Point:
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
	for (uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		_devObj->cmdBufferMain[i]->beginRecording();
		Rectanglei renderArea(0, 0, _windowWidth, _windowHeight);
		float32 depthClear = 1.f;
		uint32 stencilClear = 0u;

		/// 1) Begin the render pass
		_devObj->cmdBufferMain[i]->beginRenderPass(_devObj->render_mgr.toPass(0, 0).fbo[i], renderArea, true,
		    glm::vec4(0.f, 0.0f, 0.0f, 1.0f), depthClear, stencilClear);

		/// 2) Record the scene in to the gbuffer
		_devObj->render_mgr.toSubpass(0, 0, (uint32)RenderPassSubPass::GBuffer)
		.recordRenderingCommands(_devObj->cmdBufferMain[i], i, false, false);

		/// 3) Begin the next subpass
		_devObj->cmdBufferMain[i]->nextSubPassInline();

		/// 4) record the directional lights Geometry stencil. Draw stencil to discard useless pixels
		_devObj->render_mgr.toSubpassGroup(0, 0, (uint32)RenderPassSubPass::Lighting,
		    (uint32)LightingSubpassGroup::DirectionalLight).recordRenderingCommands(_devObj->cmdBufferMain[i], i, false);

		/// 5) record the point light stencil
		recordCommandsPointLightGeometryStencil(_devObj->cmdBufferMain[i], i);

		/// 6) record the point light proxy
		_devObj->render_mgr.toSubpassGroup(0, 0, (uint32)RenderPassSubPass::Lighting,
		    (uint32)LightingSubpassGroup::PointLightStep2).recordRenderingCommands(_devObj->cmdBufferMain[i], i, false);

		/// 7) record the pointlight source
		_devObj->render_mgr.toSubpassGroup(0, 0, (uint32)RenderPassSubPass::Lighting,
		                                   (uint32)LightingSubpassGroup::PointLightStep3).recordRenderingCommands(_devObj->cmdBufferMain[i], i, false);

		/// 8) Render ui
		_devObj->uiRenderer.beginRendering(_devObj->cmdBufferMain[i]);
		_devObj->uiRenderer.getDefaultTitle()->render();
		_devObj->uiRenderer.getDefaultControls()->render();
		_devObj->uiRenderer.getSdkLogo()->render();
		_devObj->uiRenderer.endRendering();
		_devObj->cmdBufferMain[i]->endRenderPass();
		_devObj->cmdBufferMain[i]->endRecording();
	}
}

/*!*********************************************************************************************************************
\brief	Record point light stencil commands
\param  cmdBuffer SecondaryCommandBuffer to record
\param swapChainIndex Current swap chain index
\param subpass Current sub pass
***********************************************************************************************************************/
void VulkanDeferredShadingPFX::recordCommandsPointLightGeometryStencil(api::CommandBuffer& cmdBuffer, uint32 swapChainIndex)
{
	Rectanglei renderArea(0, 0, _framebufferWidth, _framebufferHeight);
	if ((_framebufferWidth != _windowWidth) || (_framebufferHeight != _windowHeight))
	{
		renderArea = Rectanglei(_viewportOffsets[0], _viewportOffsets[1], _framebufferWidth, _framebufferHeight);
	}

	// clear stencil to 0's to make use of it again for point lights
	cmdBuffer->clearStencilAttachment(renderArea, 0);
	_devObj->render_mgr.toSubpassGroup(0, 0, (uint32)RenderPassSubPass::Lighting,
	                                   (uint32)LightingSubpassGroup::PointLightStep1).recordRenderingCommands(cmdBuffer, swapChainIndex, false);
}


/*!*********************************************************************************************************************
\return	Return an auto_ptr to a new Demo class, supplied by the user
\brief	This function must be implemented by the user of the shell. The user should return its Shell object defining the
behaviour of the application.
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() { return std::auto_ptr<Shell>(new VulkanDeferredShadingPFX()); }
