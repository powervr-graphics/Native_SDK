/*!*********************************************************************************************************************
\File         OGLESDeferredShading.cpp
\Title        Deferred Shading
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\Description  Implements a deferred shading technique supporting point and directional lights.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVRUIRenderer/PVRUIRenderer.h"
using namespace pvr::types;
namespace QuadAttributes {
enum Enum
{
	Position = 0,
	TexCoord = 2,
};
}

namespace MeshAttributes {
enum Enum
{
	PositionArray, NormalArray, TexCoordArray, TangentArray
};
}

const pvr::utils::VertexBindings_Name vertexBindings[] =
{
	{ "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoords" }, { "TANGENT", "inTangent" }
};

namespace Fbo {
enum Enum
{
	Albedo = 0,
	Normal,
	Depth,
	Count
};
}

namespace MeshNodes {
enum Enum
{
	Satyr = 0,
	Floor = 1,
	NumberOfMeshNodes
};
}

namespace EffectId {
enum Enum
{
	RenderGBuffer, RenderGBufferFloor, RenderPointLight, RenderDirLight, RenderSolidColor, RenderNullColor, WriteOutColorFromPls, Count
};
}

const pvr::StringHash EffectNames[EffectId::Count] =
{
	"RenderGBuffer", "RenderGBufferFloor", "RenderPointLight", "RenderDirectionalLight", "RenderSolidColor", "RenderNullColor", "WriteOutColorFromPls"
};
namespace Semantics {
enum Enum
{
	WorldView,//< world view
	WorldViewProjection,//< world view projection
	WorldViewIT, //< world view inverse transpose
	WorldIT,//< world inverse transpose
	MaterialColorAmbient,
	LightColor,
	CustomSemanticFarClipDist,
	CustomSemanticSpecularStrength,
	CustomSemanticDiffuseColor,
	CustomSemanticPointLightViewPos,
	CustomSemanticDirLightDirection,
	Count
};
}

const pvr::StringHash semanticsName[Semantics::Count] =
{
	pvr::StringHash("WORLDVIEW"),
	pvr::StringHash("WORLDVIEWPROJECTION"),
	pvr::StringHash("WORLDVIEWIT"),
	pvr::StringHash("WORLDIT"),
	pvr::StringHash("MATERIALCOLORAMBIENT"),
	pvr::StringHash("LIGHTCOLOR"),
	pvr::StringHash("CUSTOMSEMANTIC_FARCLIPDISTANCE"),
	pvr::StringHash("CUSTOMSEMANTIC_SPECULARSTRENGTH"),
	pvr::StringHash("CUSTOMSEMANTIC_DIFFUSECOLOUR"),
	pvr::StringHash("CUSTOMSEMANTIC_POINTLIGHT_VIEWPOSITION"),
	pvr::StringHash("CUSTOMSEMANTIC_DIRECTIONALLIGHT_DIRECTION")
};

struct DrawLightSources
{
	pvr::api::GraphicsPipeline pipeline;
	EffectId::Enum effectId;
	struct Uniforms
	{
		glm::mat4 worldViewProj;
		glm::mat3 worldIT;
		glm::vec4 color;
	};
	std::vector<Uniforms> uniforms;
};

struct DrawPointLightProxy
{
	pvr::api::GraphicsPipeline pipeline;
	EffectId::Enum effectId;
	struct InitialData
	{
		float radial_vel;
		float axial_vel;
		float vertical_vel;
		float angle;
		float distance;
		float height;
	};
	struct Uniforms
	{
		glm::mat4 worldView;
		glm::mat4 worldViewProj;
		glm::mat3 worldIT;
		glm::vec3 lightPosView;
		glm::vec3 lightIntensity;
	};
	std::vector<Uniforms> uniforms;
	std::vector<InitialData> data;
};

struct DrawPointLightGeom
{
	pvr::api::GraphicsPipeline pipeline;
	EffectId::Enum effectId;
	struct Uniforms
	{
		glm::mat4 worldViewProj;
		glm::vec4 color;
	};
	std::vector<Uniforms> uniforms;
};

struct DrawGBuffer
{
	struct Objects
	{
		pvr::api::GraphicsPipeline pipeline;
		EffectId::Enum effectId;
		glm::mat4 world;
		glm::mat4 worldView;
		glm::mat4 worldViewProj;
		glm::mat3 worldViewIT3x3;
	};
	std::vector<Objects> objects;
};

struct DrawDepthStencil
{
	pvr::api::GraphicsPipeline pipeline;
	EffectId::Enum effectId;
	struct Uniforms
	{
		glm::mat4 worldViewProj;
		glm::vec4 color;
	};
	std::vector<Uniforms> uniforms;
};

struct DrawDirLight
{
	pvr::api::GraphicsPipeline pipeline;
	EffectId::Enum effectId;
	struct Uniforms
	{
		glm::vec3 lightIntensity;
		glm::vec4 lightDirView;
	};
	std::vector<Uniforms> uniforms;
};

struct DrawQuad
{
	pvr::api::GraphicsPipeline pipeline;
	EffectId::Enum effectId;
};

struct RenderData
{
	DrawLightSources pointLightSourcesPass;
	DrawPointLightProxy pointLightProxyPass;
	DrawPointLightGeom pointLightGeomPass;
	DrawDirLight directionalLightPass;
	DrawGBuffer storeRenderDataPass;
	DrawDepthStencil depthStencilPass;
	DrawQuad writePlsPass;
};

namespace Files {
const char* const PointLightModelFile = "pointlight.pod";
const char* const SceneFile = "scene.pod";
const char* const PfxSrcFile = "effect_MRT.pfx";
const char* const PfxPlsSrcFile = "effect_PLS.pfx";
}

namespace Configuration {
static pvr::int32 MaxScenePointLights = 5;
static pvr::int32 NumProceduralPointLights = 10;
static bool AdditionalDirectionalLight = true;
const pvr::float32 FrameRate = 1.0f / 120.0f;
pvr::float32 PointLightScale = 40.0f;
pvr::float32 PointlightIntensity = 100.0f;
const pvr::float32 DirLightIntensity = .2f;
}


/*!*********************************************************************************************************************
Class implementing the Shell functions.
***********************************************************************************************************************/
class OGLESDeferredShading : public pvr::Shell
{
public:
	struct Material
	{
		pvr::api::GraphicsPipeline materialPipeline;
		pvr::api::DescriptorSet materialDescriptorSet;
		pvr::float32 specularStrength;
		glm::vec3 diffuseColor;
	};
	pvr::GraphicsContext context;
	struct ApiObjects
	{
		// Handles for FBOs and surfaces
		pvr::api::Fbo  onScreenFbo;
		pvr::api::Fbo  gBufferFBO;
		pvr::api::TextureView renderTextureViews[Fbo::Count];

		pvr::api::RenderPass gBufferRenderPass;
		pvr::api::RenderPass defaultRenderPass;

		pvr::api::Buffer pointLightVbo;
		pvr::api::Buffer pointLightIbo;

		// commandbuffers for each pass
		pvr::api::CommandBuffer cmdBufferMain;
		pvr::api::SecondaryCommandBuffer cmdBuffUIRenderer;
		pvr::api::SecondaryCommandBuffer cmdBuffSceneGeometry;
		pvr::api::SecondaryCommandBuffer cmdBuffRenderGbuffer;
		pvr::api::SecondaryCommandBuffer cmdBuffLighting;
		pvr::api::SecondaryCommandBuffer cmdBuffRenderDepthStencil;

		pvr::api::DescriptorSet pointLightDescriptorSet;
		pvr::api::DescriptorSet directionalLightDescriptorSet;

		//Layouts we will be needing
		pvr::api::DescriptorSetLayout noSamplerLayout;
		pvr::api::DescriptorSetLayout oneSamplerLayout;
		pvr::api::DescriptorSetLayout twoSamplerLayout;
		pvr::api::DescriptorSetLayout threeSamplerLayout;
		pvr::api::DescriptorSetLayout fourSamplerLayout;

		pvr::api::PipelineLayout pipeLayoutNoSamplers;
		pvr::api::PipelineLayout pipeLayoutOneSampler;
		pvr::api::PipelineLayout pipeLayoutTwoSamplers;
		pvr::api::PipelineLayout pipeLayoutThreeSamplers;
		pvr::api::PipelineLayout pipeLayoutFourSamplers;

		std::vector<pvr::api::Buffer> sceneVbos;
		std::vector<pvr::api::Buffer> sceneIbos;

		std::vector<Material>materials;
		pvr::ui::UIRenderer uiRenderer;

		// The effect file handler
		std::vector<pvr::api::EffectApi> effects;

		RenderData renderInfo;
	};

	//Putting all api objects into a pointer just makes it easier to release them all together with RAII
	std::auto_ptr<ApiObjects> apiObj;

	pvr::api::AssetStore assetManager;

	std::vector<std::map<pvr::int32, pvr::int32>/**/> uniformMapping;

	// Frame counters for animation
	pvr::float32 frame;
	bool         isPaused;
	pvr::uint32  cameraId;
	bool animateCamera;

	// Projection and Model View matrices
	glm::vec3   cameraPosition;
	glm::mat4   viewMtx;
	glm::mat4   projMtx;
	glm::mat4   viewProjMtx;
	glm::mat4   invViewMtx;
	pvr::float32   farClipDist;

	pvr::int32   windowWidth, windowHeight, fboWidth, fboHeight;
	pvr::int32   viewportOffsets[2];

	// Light models
	pvr::assets::ModelHandle pointLightModel;
	// Object model
	pvr::assets::ModelHandle scene;

	bool usePixelLocalStorage;

	OGLESDeferredShading() { animateCamera = false; isPaused = false; }
	//	Overriden from pvr::Shell
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	bool setUpRenderPass();
	bool createPipelines();
	bool createGBufferMRT();

	void recordCommandBufferRenderGBuffer(pvr::api::SecondaryCommandBuffer& cmdBuffer);
	void recordCommandBufferDepthStencil(pvr::api::SecondaryCommandBuffer& cmdBuffer);
	void recordCommandsDirectionalLights(pvr::api::SecondaryCommandBuffer& cmdBuffer);
	void recordCommandsPointLights(pvr::api::SecondaryCommandBuffer& cmdBuffer);
	void recordCommandsMRT(pvr::api::CommandBuffer& cmdBuff);
	void recordCommandsPLS(pvr::api::CommandBuffer& cmdBuff);
	void recordCommandUIRenderer(pvr::api::SecondaryCommandBuffer& cmdBuffer);

	void recordSecondaryCommandBuffers();

	void allocateUniforms();

	bool createMaterialsAndDescriptorSets();
	bool loadVbos();
	bool loadPFX();

	void updateSceneUniforms();

	void updateAnimation();

	void updateProceduralPointLight(DrawPointLightProxy::InitialData& data, DrawPointLightProxy::Uniforms& proxy,
	                                DrawPointLightGeom::Uniforms& geom, DrawLightSources::Uniforms& source, bool initial);

	bool setUpEffectHelper(EffectId::Enum effectId, const pvr::assets::PfxReader& reader);

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
\return	Return true if no error occurred
\brief	Loads the textures required for this example and sets up descriptorSets
***********************************************************************************************************************/
bool OGLESDeferredShading::createMaterialsAndDescriptorSets()
{
	if (scene->getNumMaterials() == 0)
	{
		setExitMessage("ERROR: The scene does not contain any materials.");
		return false;
	}


	//1: CREATE THE SAMPLERS
	// create point sampler
	pvr::assets::SamplerCreateParam samplerDesc;
	samplerDesc.minificationFilter = SamplerFilter::Nearest;
	samplerDesc.magnificationFilter = SamplerFilter::Nearest;
	samplerDesc.wrapModeU = samplerDesc.wrapModeV = samplerDesc.wrapModeW = SamplerWrap::Repeat;
	pvr::api::Sampler samplerNearest = context->createSampler(samplerDesc);

	// create trilinear sampler
	samplerDesc.minificationFilter = SamplerFilter::Linear;
	samplerDesc.magnificationFilter = SamplerFilter::Linear;
	samplerDesc.mipMappingFilter = SamplerFilter::Linear;
	pvr::api::Sampler samplerTrilinear = context->createSampler(samplerDesc);


	//2: CREATE THE DESCRIPTOR SET LAYOUTS
	pvr::api::DescriptorSetLayoutCreateParam descSetLayoutInfo;

	// no texture sampler layout
	apiObj->noSamplerLayout = context->createDescriptorSetLayout(descSetLayoutInfo);

	// Single texture sampler layout
	descSetLayoutInfo.setBinding(0, DescriptorType::CombinedImageSampler, 1, ShaderStageFlags::Fragment);
	apiObj->oneSamplerLayout = context->createDescriptorSetLayout(descSetLayoutInfo);

	// Two textures sampler layout
	descSetLayoutInfo.setBinding(1, DescriptorType::CombinedImageSampler, 1, ShaderStageFlags::Fragment);
	apiObj->twoSamplerLayout = context->createDescriptorSetLayout(descSetLayoutInfo);

	// Three textures sampler layout
	descSetLayoutInfo.setBinding(2, DescriptorType::CombinedImageSampler, 1, ShaderStageFlags::Fragment);
	apiObj->threeSamplerLayout = context->createDescriptorSetLayout(descSetLayoutInfo);

	// Four textures sampler layout (for GBuffer rendering)
	descSetLayoutInfo.setBinding(3, DescriptorType::CombinedImageSampler, 1, ShaderStageFlags::Fragment);
	apiObj->fourSamplerLayout = context->createDescriptorSetLayout(descSetLayoutInfo);


	//3: CREATE DESCRIPTOR SETS FOR EACH MATERIAL
	apiObj->materials.resize(scene->getNumMaterials());
	for (pvr::uint32 i = 0; i < scene->getNumMaterials(); ++i)
	{
		pvr::api::DescriptorSetUpdate descSetInfo;

		pvr::api::TextureView diffuseMap;
		pvr::api::TextureView bumpMap;

		// get the current material
		const pvr::assets::Model::Material& material = scene->getMaterial(i);

		// get material properties
		apiObj->materials[i].specularStrength = material.getShininess();
		apiObj->materials[i].diffuseColor = material.getDiffuse();

		int numTextures = 0;

		if (material.getDiffuseTextureIndex() != -1)
		{
			// Load the diffuse texture map
			if (!assetManager.getTextureWithCaching(getGraphicsContext(), scene->getTexture(material.getDiffuseTextureIndex()).getName(), &(diffuseMap), NULL))
			{
				setExitMessage("ERROR: Failed to load texture %s", scene->getTexture(material.getDiffuseTextureIndex()).getName().c_str());
				return false;
			}
			descSetInfo.setCombinedImageSampler(0, diffuseMap, samplerTrilinear);
			++numTextures;
		}
		if (material.getBumpMapTextureIndex() != -1)
		{
			// Load the bumpmap
			if (!assetManager.getTextureWithCaching(getGraphicsContext(), scene->getTexture(material.getBumpMapTextureIndex()).getName(), &(bumpMap), NULL))
			{
				setExitMessage("ERROR: Failed to load texture %s", scene->getTexture(material.getBumpMapTextureIndex()).getName().c_str());
				return false;
			}
			++numTextures;
			descSetInfo.setCombinedImageSampler(1, bumpMap, samplerTrilinear);
		}

		// based on the number of textures select the correct descriptor set
		switch (numTextures)
		{
		case 0: apiObj->materials[i].materialDescriptorSet = context->createDescriptorSetOnDefaultPool(apiObj->noSamplerLayout); break;
		case 1: apiObj->materials[i].materialDescriptorSet = context->createDescriptorSetOnDefaultPool(apiObj->oneSamplerLayout); break;
		case 2: apiObj->materials[i].materialDescriptorSet = context->createDescriptorSetOnDefaultPool(apiObj->twoSamplerLayout); break;
		case 3: apiObj->materials[i].materialDescriptorSet = context->createDescriptorSetOnDefaultPool(apiObj->threeSamplerLayout); break;
		case 4: apiObj->materials[i].materialDescriptorSet = context->createDescriptorSetOnDefaultPool(apiObj->fourSamplerLayout); break;
		default:
			break;
		}

		apiObj->materials[i].materialDescriptorSet->update(descSetInfo);
	}

	// 3: CREATE DESCRIPTOR SET USED TO RENDER THE LIGHTS USING EITHER GBUFFER OR PLS AS INPUT
	if (!usePixelLocalStorage)
	{
		// GBuffer with multiple render targets to sample from
		pvr::api::DescriptorSetUpdate descSetInfo;
		for (pvr::uint32 i = 0; i < Fbo::Count; ++i)
		{
			descSetInfo.setCombinedImageSampler(i, apiObj->renderTextureViews[i], samplerNearest);
		}

		apiObj->pointLightDescriptorSet = context->createDescriptorSetOnDefaultPool(apiObj->threeSamplerLayout);
		apiObj->pointLightDescriptorSet->update(descSetInfo);

		{
			// uses three texture and sampler descriptor setlayout
			pvr::api::DescriptorSetUpdate descSetInfo;
			descSetInfo.setCombinedImageSampler(0, apiObj->renderTextureViews[Fbo::Albedo], samplerNearest);
			descSetInfo.setCombinedImageSampler(1, apiObj->renderTextureViews[Fbo::Normal], samplerNearest);
			apiObj->directionalLightDescriptorSet = context->createDescriptorSetOnDefaultPool(apiObj->twoSamplerLayout);
			apiObj->directionalLightDescriptorSet->update(descSetInfo);
		}
	}
	else
	{
		// Pixel Local Storage data directly available
		pvr::api::DescriptorSetUpdate descSetInfo;
		apiObj->pointLightDescriptorSet = context->createDescriptorSetOnDefaultPool(apiObj->noSamplerLayout);
		apiObj->pointLightDescriptorSet->update(descSetInfo);

		apiObj->directionalLightDescriptorSet = context->createDescriptorSetOnDefaultPool(apiObj->noSamplerLayout);
		apiObj->directionalLightDescriptorSet->update(descSetInfo);
	}

	return true;
}

/*!*********************************************************************************************************************
\brief	Create the pipelines for this example
\return	Return true if no error occurred
***********************************************************************************************************************/
bool OGLESDeferredShading::createPipelines()
{
	using namespace pvr;
	using namespace pvr::api;

	// Set up a little bit in advance - we'll be reusing them for the actual object.
	GraphicsPipelineCreateParam pipeInfo;
	pipeInfo.rasterizer.setCullFace(Face::Back);

	// enable depth testing and depth writing
	pipeInfo.depthStencil.setDepthTestEnable(true);
	pipeInfo.depthStencil.setDepthWrite(true);

	// disable stencil test
	pipeInfo.depthStencil.setStencilTest(false);

	types::BlendingConfig colorAttachment;
	colorAttachment.channelWriteMask = ColorChannel::All;
	colorAttachment.blendEnable = false;

	pipeInfo.colorBlend.setAttachmentState(0, colorAttachment);

	//CREATING THE ACTUAL PIPELINES FOR EACH OF OUR PASSES:

	// 1) RENDER TO EITHER GBUFFER USING MRTS OR INTO PIXEL LOCAL STORAGE
	{
		pipeInfo.vertexInput.clear();
		pvr::utils::createInputAssemblyFromMeshAndEffect(scene->getMesh(0), apiObj->effects[EffectId::RenderGBuffer]->getEffectAsset(), pipeInfo);

		// if using pixel local storage then merge passes 1 and 2
		// Pixel local storage can be used on the default frame buffer object and therefore the stencil buffer can be shared between passes
		// if using GBuffer approach the default fbo stencil buffer cannot be shared with the GBuffer fbo rendered into
		if (usePixelLocalStorage)
		{
			// enable stencil testing only if pixel local storage is used
			pvr::api::pipelineCreation::DepthStencilStateCreateParam::StencilState stencilState;

			// only replace stencil buffer when the depth test passes
			stencilState.opStencilFail = StencilOp::Keep;
			stencilState.opDepthFail = StencilOp::Keep;
			stencilState.opDepthPass = StencilOp::Replace;

			stencilState.compareOp = ComparisonMode::Always;

			pipeInfo.depthStencil.setStencilTest(true);

			pipeInfo.depthStencil.setStencilFront(stencilState);
			pipeInfo.depthStencil.setStencilBack(stencilState);
		}

		// 2 mesh nodes are used in this scene (floor and Satyr model)
		apiObj->renderInfo.storeRenderDataPass.objects.resize(scene->getNumMeshNodes());

		// setup the MRT
		apiObj->renderInfo.storeRenderDataPass.objects[MeshNodes::Satyr].pipeline = context->createGraphicsPipeline(pipeInfo, apiObj->effects[EffectId::RenderGBuffer]->getPipeline());
		apiObj->renderInfo.storeRenderDataPass.objects[MeshNodes::Satyr].effectId = EffectId::RenderGBuffer;

		apiObj->renderInfo.storeRenderDataPass.objects[MeshNodes::Floor].pipeline = context->createGraphicsPipeline(pipeInfo, apiObj->effects[EffectId::RenderGBufferFloor]->getPipeline());
		apiObj->renderInfo.storeRenderDataPass.objects[MeshNodes::Floor].effectId = EffectId::RenderGBufferFloor;
	}

	// as discussed above the depth stencil pass can be avoided if pixel local storage is used
	if (!usePixelLocalStorage)
	{
		// 2) DEPTH STENCIL PASS - That will draw the geometry in the stencil buffer so that we can skip lighting fragments that do not contain objects easier.
		// This is an optimisation pass, could be omitted with a little restructuring but the directional pass would be slower
		{
			pipeInfo.vertexInput.clear();
			pvr::utils::createInputAssemblyFromMeshAndEffect(scene->getMesh(0), apiObj->effects[EffectId::RenderNullColor]->getEffectAsset(), pipeInfo);

			// write only into depth and stencil.
			colorAttachment.channelWriteMask = types::ColorChannel(0);
			pipeInfo.colorBlend.clearAttachments();

			pvr::api::pipelineCreation::DepthStencilStateCreateParam::StencilState stencilState;

			// only replace stencil buffer when the depth test passes
			stencilState.opStencilFail = StencilOp::Keep;
			stencilState.opDepthFail = StencilOp::Keep;
			stencilState.opDepthPass = StencilOp::Replace;

			stencilState.compareOp = ComparisonMode::Always;

			pipeInfo.depthStencil.setStencilTest(true);

			pipeInfo.depthStencil.setStencilFront(stencilState);
			pipeInfo.depthStencil.setStencilBack(stencilState);

			pipeInfo.colorBlend.setAttachmentState(0, colorAttachment);

			apiObj->renderInfo.depthStencilPass.pipeline = context->createGraphicsPipeline(pipeInfo, apiObj->effects[EffectId::RenderNullColor]->getPipeline());
			apiObj->renderInfo.depthStencilPass.effectId = EffectId::RenderNullColor;
		}
	}

	// 3) DIRECTIONAL LIGHTING - A full-screen quad that will apply any global (ambient/directional) lighting
	{
		// disable the depth write as we do not want to modify the depth buffer while rendering directional lights
		// Make use of the stencil buffer contents to only shade pixels where actual geometry is located.
		pvr::api::pipelineCreation::DepthStencilStateCreateParam::StencilState stencilState;

		// keep the stencil states the same as the previous pass - THESE DON'T MATTER
		stencilState.opStencilFail = StencilOp::Keep;
		stencilState.opDepthFail = StencilOp::Keep;
		stencilState.opDepthPass = StencilOp::Replace;

		// if the stencil is equal to the value specified then stencil passes
		stencilState.compareOp = ComparisonMode::Equal;

		// disable depth writing and depth testing
		pipeInfo.depthStencil.setDepthWrite(false);
		pipeInfo.depthStencil.setDepthTestEnable(false);

		// enable stencil testing
		pipeInfo.depthStencil.setStencilTest(true);
		pipeInfo.depthStencil.setStencilFront(stencilState);
		pipeInfo.depthStencil.setStencilBack(stencilState);

		// write in to the color
		colorAttachment.channelWriteMask = ColorChannel::All;
		colorAttachment.blendEnable = false;
		pipeInfo.colorBlend.setAttachmentState(0, colorAttachment);

		//Rendering without attributes
		pipeInfo.vertexInput.clear();

		pipeInfo.inputAssembler.setPrimitiveTopology(PrimitiveTopology::TriangleStrip);

		apiObj->renderInfo.directionalLightPass.pipeline = context->createGraphicsPipeline(pipeInfo, apiObj->effects[EffectId::RenderDirLight]->getPipeline());
		apiObj->renderInfo.directionalLightPass.effectId = EffectId::RenderDirLight;
	}

	// 4) POINT LIGHTS GEOMETRY STENCIL PASS
	// Render the front face of each light volume
	// Z function is set as Less/Equal
	// Z test passes will leave the stencil as 0 i.e. the front of the light is infront of all geometry in the current pixel
	//		This is the condition we want for determining whether the geometry can be affected by the point lights
	// Z test fails will increment the stencil to 1. i.e. the front of the light is behind all of the geometry in the current pixel
	//		Under this condition the current pixel cannot be affected by the current point light as the geometry is infront of the front of the point light
	{
		pipeInfo.vertexInput.clear();
		pvr::utils::createInputAssemblyFromMeshAndEffect(pointLightModel->getMesh(0), apiObj->effects[EffectId::RenderNullColor]->getEffectAsset(), pipeInfo);

		colorAttachment.channelWriteMask = types::ColorChannel(0);// write only in to depth and stencil buffer
		pipeInfo.colorBlend.setAttachmentState(0, colorAttachment);// Additively blend the light contributions

		pipeInfo.rasterizer.setCullFace(Face::Back);

		// disable depth write
		pipeInfo.depthStencil.setDepthWrite(false);

		// set depth comparison to less/equal
		pipeInfo.depthStencil.setDepthCompareFunc(pvr::types::ComparisonMode::LessEqual);
		pipeInfo.depthStencil.setDepthTestEnable(true);
		pipeInfo.depthStencil.setStencilTest(true);

		// by setting the stencilOp we pick only the pixel of the objects which are inside a point light.
		pvr::api::pipelineCreation::DepthStencilStateCreateParam::StencilState stencilState;

		stencilState.compareOp = ComparisonMode::Always;

		// keep current value if the stencil test fails
		stencilState.opStencilFail = StencilOp::Keep;

		// if the depth test fails then increment wrap
		stencilState.opDepthFail = StencilOp::IncrementWrap;
		stencilState.opDepthPass = StencilOp::Keep;

		pipeInfo.depthStencil.setStencilFront(stencilState);

		stencilState.opDepthFail = StencilOp::Keep;
		pipeInfo.depthStencil.setStencilBack(stencilState);

		apiObj->renderInfo.pointLightGeomPass.pipeline = context->createGraphicsPipeline(pipeInfo, apiObj->effects[EffectId::RenderNullColor]->getPipeline());
		apiObj->renderInfo.pointLightGeomPass.effectId = EffectId::RenderNullColor;
	}

	// 5) POINT LIGHTS PROXIES - Actually light the pixels touched by a point light.
	// Render the back faces of the light volumes
	// Z function is set as Greater/Equal
	// Z test passes signify that there is geometry infront of the back face of the light volume i.e. for the current pixel there is some geometry infront of the back face of the light volume
	// Stencil function is Equal i.e. the stencil renference is set to 0
	// Stencil passes signify that for the current pixel there exists a front face of a light volume infront of the current geometry
	// Point light calculations occur every time a pixel passes both the stencil AND Z test
	{
		colorAttachment.channelWriteMask = ColorChannel::All;

		pipeInfo.rasterizer.setCullFace(Face::Front);

		pipeInfo.depthStencil.setStencilTest(true);

		pipeInfo.depthStencil.setDepthTestEnable(true);
		pipeInfo.depthStencil.setDepthCompareFunc(pvr::types::ComparisonMode::GreaterEqual);
		pipeInfo.depthStencil.setDepthWrite(false);

		pipeInfo.vertexInput.clear();
		pvr::utils::createInputAssemblyFromMeshAndEffect(pointLightModel->getMesh(0), apiObj->effects[EffectId::RenderPointLight]->getEffectAsset(), pipeInfo);

		// Set the stencil test to only shade the lit areas and re-enable color writes.
		pipeInfo.depthStencil.setStencilTest(true);

		if (!usePixelLocalStorage)
		{
			colorAttachment.blendEnable = true;
			colorAttachment.srcBlendColor = BlendFactor::One;
			colorAttachment.srcBlendAlpha = BlendFactor::One;
			colorAttachment.destBlendColor = BlendFactor::One;
			colorAttachment.destBlendAlpha = BlendFactor::One;
		}

		pipeInfo.colorBlend.setAttachmentState(0, colorAttachment);

		pvr::api::pipelineCreation::DepthStencilStateCreateParam::StencilState stencilState;
		stencilState.compareOp = ComparisonMode::Always;
		stencilState.reference = 0;

		pipeInfo.depthStencil.setStencilFront(stencilState);
		pipeInfo.depthStencil.setStencilBack(stencilState);

		apiObj->renderInfo.pointLightProxyPass.pipeline = context->createGraphicsPipeline(pipeInfo, apiObj->effects[EffectId::RenderPointLight]->getPipeline());
		apiObj->renderInfo.pointLightProxyPass.effectId = EffectId::RenderPointLight;
	}

	// 6) LIGHT SOURCES : Rendering the "will-o-wisps" that are the sources of the light
	{
		pipeInfo.vertexInput.clear();
		pvr::utils::createInputAssemblyFromMeshAndEffect(pointLightModel->getMesh(0), apiObj->effects[EffectId::RenderSolidColor]->getEffectAsset(), pipeInfo);

		pipeInfo.rasterizer.setCullFace(Face::Back);

		// disable stencil testing
		pipeInfo.depthStencil.setStencilTest(false);

		// re-enable depth testing and depth writing
		pipeInfo.depthStencil.setDepthTestEnable(true);
		pipeInfo.depthStencil.setDepthWrite(true);
		pipeInfo.depthStencil.setDepthCompareFunc(ComparisonMode::LessEqual);

		if (!usePixelLocalStorage)
		{
			// use blending
			colorAttachment.blendEnable = true;
			colorAttachment.srcBlendColor = BlendFactor::One;
			colorAttachment.srcBlendAlpha = BlendFactor::One;
			colorAttachment.destBlendColor = BlendFactor::One;
			colorAttachment.destBlendAlpha = BlendFactor::One;
		}

		pipeInfo.colorBlend.setAttachmentState(0, colorAttachment);

		apiObj->renderInfo.pointLightSourcesPass.pipeline = context->createGraphicsPipeline(pipeInfo, apiObj->effects[EffectId::RenderSolidColor]->getPipeline());
		apiObj->renderInfo.pointLightSourcesPass.effectId = EffectId::RenderSolidColor;
	}

	// &) WRITE OUT PIXEL LOCAL STORAGE: IF using pixel local storage, we need a final pass to write out from the pixel local storage colour to the FBO
	if (usePixelLocalStorage)
	{
		// we don't need to check depth or stencil tests
		pipeInfo.depthStencil.setDepthWrite(false);
		pipeInfo.depthStencil.setDepthTestEnable(false);
		pipeInfo.depthStencil.setStencilTest(false);

		// write in to the color
		colorAttachment.channelWriteMask = ColorChannel::All;
		colorAttachment.blendEnable = false;
		pipeInfo.colorBlend.setAttachmentState(0, colorAttachment);
		pipeInfo.vertexInput.clear();
		pipeInfo.inputAssembler.setPrimitiveTopology(PrimitiveTopology::TriangleStrip);

		apiObj->renderInfo.writePlsPass.pipeline = context->createGraphicsPipeline(pipeInfo, apiObj->effects[EffectId::WriteOutColorFromPls]->getPipeline());
		apiObj->renderInfo.writePlsPass.effectId = EffectId::WriteOutColorFromPls;
	}

	return true;
}

bool OGLESDeferredShading::setUpRenderPass()
{
	// Create on-screen-renderpass/fbo with its subpasses.
	pvr::api::SubPass subPass0(PipelineBindPoint::Graphics);

	// use the first color attachment
	subPass0.setColorAttachment(0, 0);

	pvr::api::RenderPassCreateParam renderPassInfo;
	pvr::api::RenderPassDepthStencilInfo renderPassDepthStencilInfo = pvr::api::RenderPassDepthStencilInfo(
	      getGraphicsContext()->getDepthStencilImageFormat(),
	      LoadOp::Clear, StoreOp::Store, LoadOp::Clear, StoreOp::Store);
	renderPassInfo.setDepthStencilInfo(renderPassDepthStencilInfo);
	renderPassInfo.setColorInfo(0, pvr::api::RenderPassColorInfo(getGraphicsContext()->getPresentationImageFormat(), LoadOp::Clear));
	renderPassInfo.setSubPass(0, subPass0);

	// if using pls then add a second subpass
	if (usePixelLocalStorage)
	{
		pvr::api::SubPass subPass1(PipelineBindPoint::Graphics);
		renderPassInfo.setSubPass(1, subPass1);
	}

	apiObj->onScreenFbo = getGraphicsContext()->createOnScreenFboWithRenderPass(0, getGraphicsContext()->createRenderPass(renderPassInfo));

	return true;
}

/*!*********************************************************************************************************************
\brief	Loads the mesh data required for this example into vertex buffer objects
\return Return true if no error occurred
***********************************************************************************************************************/
bool OGLESDeferredShading::loadVbos()
{
	pvr::utils::appendSingleBuffersFromModel(context, *scene, apiObj->sceneVbos, apiObj->sceneIbos);
	pvr::utils::createSingleBuffersFromMesh(context, pointLightModel->getMesh(0), apiObj->pointLightVbo, apiObj->pointLightIbo);

	if (!apiObj->sceneVbos.size() || !apiObj->sceneIbos.size() || apiObj->pointLightVbo.isNull() || apiObj->pointLightIbo.isNull())
	{
		setExitMessage("Invalid Scene Buffers");
		return false;
	}

	return true;
}


/*!*********************************************************************************************************************
\brief	Parse each effect in the PFX file, and set everything up, notably create the pvr::api::EffectApi objects that we will use for rendering.
\return	Return true if success
\param	effectId Effect id to parse
\param	reader Pfx Reader
***********************************************************************************************************************/
bool OGLESDeferredShading::setUpEffectHelper(EffectId::Enum effectId, const pvr::assets::PfxReader& reader)
{
	//STEP 1: Set up the basic pipeline state. It's pretty much the same for all effects - only the layouts will change.
	pvr::api::GraphicsPipelineCreateParam pipeDesc;
	pipeDesc.rasterizer.setCullFace(Face::Back);
	pipeDesc.depthStencil.setDepthTestEnable(true);
	pvr::types::BlendingConfig colorAttachment;
	colorAttachment.blendEnable = false;
	pipeDesc.colorBlend.setAttachmentState(0, colorAttachment);

	//STEP 2: Load the effect from a file.
	pvr::assets::Effect effectDesc;
	if (!reader.getEffect(effectDesc, EffectNames[effectId])) { return false; }


	//STEP 3: We have created in advance some Pipeline Layouts in order to be able to reuse them. So, we count the number of Texture Semantics
	//for the effect, and select the relevant pipeline layout (that's the only difference between them at the moment).
	//WARNING THIS IS BY CONVENTION ONLY - Texture Uniforms Semantics DO NOT have to start with TEXTURE.
	int countTextures = 0;
	for (auto it = effectDesc.uniforms.begin(); it != effectDesc.uniforms.end(); ++it)
	{
		if (pvr::strings::startsWith(it->semantic, "TEXTURE")) { ++countTextures; }
	}

	switch (countTextures)
	{
	case 0: pipeDesc.pipelineLayout = apiObj->pipeLayoutNoSamplers; break;
	case 1: pipeDesc.pipelineLayout = apiObj->pipeLayoutOneSampler; break;
	case 2: pipeDesc.pipelineLayout = apiObj->pipeLayoutTwoSamplers; break;
	case 3: pipeDesc.pipelineLayout = apiObj->pipeLayoutThreeSamplers; break;
	case 4: pipeDesc.pipelineLayout = apiObj->pipeLayoutFourSamplers; break;
	default:
		pvr::Log("Could not create Effect compatible with the code of this demo. No layout has been created for %d samplers.", countTextures);
		return false;
	}

	pvr::assertion(effectId >= 0, "invalid effect id");

	//STEP 5: Actually create the EffectAPI object from the EffectAPI object.
	apiObj->effects[effectId] = context->createEffectApi(effectDesc, pipeDesc, assetManager);

	if (!apiObj->effects[effectId].isValid())
	{
		setExitMessage("Failed to load effect:%s file:%s ", effectDesc.getMaterial().getEffectName().c_str(), effectDesc.fileName.c_str());
		return false;
	}

	//STEP 4: Assign the textures to texture units. These we will not usually be touching.
	apiObj->cmdBufferMain->bindPipeline(apiObj->effects[effectId]->getPipeline());
	for (int i = 0; i < 4; ++i)
	{
		int semanticId = effectDesc.getUniformSemanticId(pvr::strings::createFormatted("TEXTURE%d", i).c_str());
		if (semanticId != -1)
		{
			pvr::uint32 uniformLoc = apiObj->effects[effectId]->getUniform(semanticId).location;
			apiObj->cmdBufferMain->setUniform<pvr::int32>(uniformLoc, i);
		}
	}

	//STEP 5: In general, deal with the uniforms. Also, put them in a map so we can reference them more easily
	for (pvr::uint32 j = 0; j < Semantics::Count; ++j)
	{
		uniformMapping[effectId][j] = -1;
		pvr::int32 tmpSemanticId = effectDesc.getUniformSemanticId(semanticsName[j]);
		if (tmpSemanticId != -1) { uniformMapping[effectId][j] = apiObj->effects[effectId]->getUniform(tmpSemanticId).location; }
	}
	return true;
}

/*!*********************************************************************************************************************
\return Return true if no error occurred
\brief	Parses the entire PFX file to gather all effects that we will need to render.
***********************************************************************************************************************/
bool OGLESDeferredShading::loadPFX()
{
	pvr::assets::PfxReader  pfxParser;

	pvr::string error;
	// Parse the whole PFX and store all data.
	// Setup all effects in the PFX file so we initialize the shaders and
	// store uniforms and attributes locations.

	if (!pfxParser.parseFromFile(getAssetStream(usePixelLocalStorage ? Files::PfxPlsSrcFile : Files::PfxSrcFile), error))
	{
		setExitMessage(error.c_str());
		return false;
	}

	const pvr::uint32 numEffects = pfxParser.getNumberEffects();
	apiObj->effects.resize(EffectId::Count);
	uniformMapping.resize(numEffects);

	apiObj->cmdBufferMain->beginRecording();

	// create the pipeline layouts
	for (pvr::int32 i = 0; i < EffectId::Count; ++i)
	{
		EffectId::Enum effectId = EffectId::Enum(i);
		if (usePixelLocalStorage || effectId != EffectId::WriteOutColorFromPls)
		{
			if (!setUpEffectHelper(EffectId::Enum(effectId), pfxParser)) { return false; }
		}
	}

	apiObj->cmdBufferMain->endRecording();
	apiObj->cmdBufferMain->submit();

	return true;
}

/*!*********************************************************************************************************************
\return	Return true if no error occurred
\brief	Code in initApplication() will be called by pvr::Shell once per run, before the rendering context is created.
Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result OGLESDeferredShading::initApplication()
{
	setStencilBitsPerPixel(8);
	setMinApiType(pvr::Api::OpenGLES3);

	frame = 0.0f;
	isPaused = false;
	cameraId = 0;

	//Prepare the asset manager for loading our objects
	assetManager.init(*this);

	//  Load the scene and the light
	if (!assetManager.loadModel(Files::SceneFile, scene))
	{
		setExitMessage("ERROR: Couldn't load the scene pod file %s\n", Files::SceneFile);
		return pvr::Result::UnknownError;
	}

	if (scene->getNumCameras() == 0)
	{
		setExitMessage("ERROR: The main scene to display must contain a camera.\n");
		return pvr::Result::InvalidData;
	}

	//	Load light proxy geometry
	if (!assetManager.loadModel(Files::PointLightModelFile, pointLightModel))
	{
		setExitMessage("ERROR: Couldn't load the point light proxy pod file\n");
		return pvr::Result::UnableToOpen;
	}
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
If the rendering context is lost, QuitApplication() will not be called.x
***********************************************************************************************************************/
pvr::Result OGLESDeferredShading::quitApplication() { return pvr::Result::Success; }

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in initView() will be called by PVRShell upon initialization or after a change in the rendering context.
Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result OGLESDeferredShading::initView()
{
	srand((unsigned int)this->getTime());

	//Create the empty API objects.
	apiObj.reset(new ApiObjects);

	//Initialize free-floating objects (commandBuffers).
	context = getGraphicsContext();

	fboWidth = windowWidth = getWidth();
	fboHeight = windowHeight = getHeight();

	// Check if pixel local storage extension is supported
	usePixelLocalStorage = getGraphicsContext()->hasApiCapability(pvr::ApiCapabilities::ShaderPixelLocalStorage);// use pixel local storage by default

	const pvr::platform::CommandLine& cmdOptions = getCommandLine();

	cmdOptions.getIntOption("-fbowidth", fboWidth);
	fboWidth = glm::min<pvr::int32>(fboWidth, windowWidth);
	cmdOptions.getIntOption("-fboheight", fboHeight);
	fboHeight = glm::min<pvr::int32>(fboHeight, windowHeight);
	cmdOptions.getBoolOptionSetTrueIfPresent("-forcepls", usePixelLocalStorage);
	cmdOptions.getBoolOptionSetFalseIfPresent("-forcemrt", usePixelLocalStorage);
	cmdOptions.getIntOption("-numlights", Configuration::NumProceduralPointLights);
	cmdOptions.getFloatOption("-lightscale", Configuration::PointLightScale);
	cmdOptions.getFloatOption("-lightintensity", Configuration::PointlightIntensity);

	if (!usePixelLocalStorage)
	{
		pvr::Log(pvr::Logger::Information, "Pixel local storage is not supported. fall back to MRT");
	}
	else
	{
		pvr::Log(pvr::Logger::Information, "Pixel local storage support detected. Will use Pixel Local Storage path.");
	}

	setUpRenderPass();

	// setup UI renderer
	apiObj->uiRenderer.init(apiObj->onScreenFbo->getRenderPass(), 0);
	apiObj->uiRenderer.getDefaultTitle()->setText("DeferredShading");
	apiObj->uiRenderer.getDefaultTitle()->commitUpdates();
	apiObj->uiRenderer.getDefaultControls()->setText("Action1: Pause\nAction2: Orbit Camera\n");
	apiObj->uiRenderer.getDefaultControls()->commitUpdates();

	// setup command buffers
	apiObj->cmdBufferMain = context->createCommandBufferOnDefaultPool();
	apiObj->cmdBuffSceneGeometry = context->createSecondaryCommandBufferOnDefaultPool();
	apiObj->cmdBuffRenderGbuffer = context->createSecondaryCommandBufferOnDefaultPool();
	apiObj->cmdBuffRenderDepthStencil = context->createSecondaryCommandBufferOnDefaultPool();
	apiObj->cmdBuffLighting = context->createSecondaryCommandBufferOnDefaultPool();
	apiObj->cmdBuffUIRenderer = context->createSecondaryCommandBufferOnDefaultPool();

	viewportOffsets[0] = (windowWidth - fboWidth) / 2;
	viewportOffsets[1] = (windowHeight - fboHeight) / 2;

	pvr::Log(pvr::Log.Information, "FBO dimensions: %d x %d\n", fboWidth, fboHeight);
	pvr::Log(pvr::Log.Information, "Onscreen Framebuffer dimensions: %d x %d\n", windowWidth, windowHeight);

	// Only need to setup GBuffer if not using pixel local storage
	if (!usePixelLocalStorage)
	{
		//	Allocates the gbuffer buffer objects
		if (!createGBufferMRT()) { return pvr::Result::NotInitialized; }
	}

	//  Load textures
	if (!createMaterialsAndDescriptorSets()) { return pvr::Result::NotInitialized; }

	// create the pipeline layout
	pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
	apiObj->pipeLayoutNoSamplers = context->createPipelineLayout(pipeLayoutInfo);

	pipeLayoutInfo.setDescSetLayout(0, apiObj->oneSamplerLayout);
	apiObj->pipeLayoutOneSampler = context->createPipelineLayout(pipeLayoutInfo);

	pipeLayoutInfo.setDescSetLayout(0, apiObj->twoSamplerLayout);
	apiObj->pipeLayoutTwoSamplers = context->createPipelineLayout(pipeLayoutInfo);

	pipeLayoutInfo.setDescSetLayout(0, apiObj->threeSamplerLayout);
	apiObj->pipeLayoutThreeSamplers = context->createPipelineLayout(pipeLayoutInfo);

	pipeLayoutInfo.setDescSetLayout(0, apiObj->fourSamplerLayout);
	apiObj->pipeLayoutFourSamplers = context->createPipelineLayout(pipeLayoutInfo);

	if (isScreenRotated() && isFullScreen())
	{
		projMtx = pvr::math::perspectiveFov(getApiType(), scene->getCamera(0).getFOV(), (pvr::float32)fboHeight, (pvr::float32)fboWidth, scene->getCamera(0).getNear(), scene->getCamera(0).getFar(),
		                                    glm::pi<pvr::float32>() * .5f);
	}
	else
	{
		projMtx = glm::perspectiveFov(scene->getCamera(0).getFOV(), (pvr::float32)fboWidth, (pvr::float32)fboHeight, scene->getCamera(0).getNear(), scene->getCamera(0).getFar());
	}

	//	Load objects from the scene into VBOs
	if (!loadVbos()) { return pvr::Result::UnknownError; }

	//	Load and compile the shaders & link programs
	if (!loadPFX()) { return pvr::Result::UnknownError; }

	createPipelines();

	allocateUniforms();

	recordSecondaryCommandBuffers();

	apiObj->cmdBufferMain->beginRecording();
	if (usePixelLocalStorage)
	{
		recordCommandsPLS(apiObj->cmdBufferMain);
	}
	else
	{
		recordCommandsMRT(apiObj->cmdBufferMain);
	}
	apiObj->cmdBufferMain->endRecording();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result OGLESDeferredShading::releaseView()
{
	apiObj.reset(0);
	assetManager.releaseAll();
	context.release();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result OGLESDeferredShading::renderFrame()
{
	//  Handle user input and update object animations
	updateAnimation();
	updateSceneUniforms();
	apiObj->cmdBufferMain->submit();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief	Allocates the required FBOs and buffer objects. Will not be needed for PLS
***********************************************************************************************************************/
bool OGLESDeferredShading::createGBufferMRT()
{
	//Sets up the RenderPass, FBO's, Textures for MRT rendering.
	pvr::api::RenderPassCreateParam renderpassCreateParam;
	pvr::api::FboCreateParam gbufferFboCreateParam;
	pvr::api::SubPass subPassInfo;
	const pvr::api::ImageStorageFormat internalsFormats[3] =
	{
		pvr::api::ImageStorageFormat(pvr::PixelFormat::RGBA_8888, 1, ColorSpace::lRGB, pvr::VariableType::UnsignedByteNorm),	// albedo
		pvr::api::ImageStorageFormat(pvr::PixelFormat::RGB_888, 1, ColorSpace::lRGB, pvr::VariableType::UnsignedByteNorm),	// normal
		pvr::api::ImageStorageFormat(pvr::PixelFormat::RGBA_8888, 1, ColorSpace::lRGB, pvr::VariableType::UnsignedByteNorm),	// depth
	};

	// Allocate the render targets
	for (pvr::uint32 i = 0; i < Fbo::Count; i++)
	{
		pvr::api::TextureStore renderTexure = context->createTexture();
		renderTexure->allocate2D(internalsFormats[i], fboWidth, fboHeight);
		apiObj->renderTextureViews[i] = context->createTextureView(renderTexure);

		// setup the albedo, normal, depth attachment view
		subPassInfo.setColorAttachment(i, i);
		gbufferFboCreateParam.setColor(i, apiObj->renderTextureViews[i]);
		renderpassCreateParam.setColorInfo(i, pvr::api::RenderPassColorInfo(internalsFormats[i], LoadOp::Clear, StoreOp::Store));
	}

	// create the depth stencil attachment
	const pvr::api::ImageStorageFormat depthStencilFormat(pvr::PixelFormat::Depth24Stencil8, 1, ColorSpace::lRGB, pvr::VariableType::Float);
	pvr::api::TextureStore gbufferDepthStencilTexure = context->createTexture();
	gbufferDepthStencilTexure->allocate2D(depthStencilFormat, fboWidth, fboHeight);
	pvr::api::TextureView gbufferDepthStencilView = context->createTextureView(gbufferDepthStencilTexure);

	// set up the depth stencil view
	gbufferFboCreateParam.setDepthStencil(gbufferDepthStencilView);

	pvr::api::RenderPassDepthStencilInfo renderPassDepthStencilInfo = pvr::api::RenderPassDepthStencilInfo(depthStencilFormat,
	    LoadOp::Clear, StoreOp::Ignore, LoadOp::Clear, StoreOp::Ignore);
	renderpassCreateParam.setDepthStencilInfo(renderPassDepthStencilInfo);

	// add the sub pass information
	renderpassCreateParam.setSubPass(0, subPassInfo);

	// create the gBuffer render pass
	apiObj->gBufferRenderPass = context->createRenderPass(renderpassCreateParam);

	gbufferFboCreateParam.setRenderPass(apiObj->gBufferRenderPass);

	// create the gbuffer fbo
	apiObj->gBufferFBO = context->createFbo(gbufferFboCreateParam);
	if (!apiObj->gBufferFBO.isValid())
	{
		this->setExitMessage("G-Buffer Fbo creation failed");
		return false;
	}

	return true;
}

/*!*********************************************************************************************************************
\brief	Draw deferred shading using pixel local storage
***********************************************************************************************************************/
void OGLESDeferredShading::updateSceneUniforms()
{
	RenderData& pass = apiObj->renderInfo;

	//Update GBuffer uniforms
	for (pvr::uint32 i = 0; i < scene->getNumMeshNodes(); ++i)
	{
		const pvr::assets::Model::Node& node = scene->getNode(i);
		pass.storeRenderDataPass.objects[i].world = scene->getWorldMatrix(node.getObjectId());
		pass.storeRenderDataPass.objects[i].worldView = viewMtx * pass.storeRenderDataPass.objects[i].world;
		pass.storeRenderDataPass.objects[i].worldViewProj = viewProjMtx * pass.storeRenderDataPass.objects[i].world;
		pass.storeRenderDataPass.objects[i].worldViewIT3x3 = glm::inverseTranspose(glm::mat3(pass.storeRenderDataPass.objects[i].worldView));
	}

	// Imprint a 1 into the stencil buffer to indicate where geometry is found.
	// This optimizes the rendering of directional light sources as the shader then
	// only has to be executed where necessary.
	// Render the objects to the depth and stencil buffers but not to the framebuffer
	static const glm::vec4 sRandColors[] =
	{
		glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
		glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
		glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(0.5f, 1.0f, 0.5f, 1.0f),
	};
	for (pvr::uint32 i = 0; i < scene->getNumMeshNodes(); ++i)
	{
		const pvr::assets::Model::Node& node = scene->getNode(i);
		pass.depthStencilPass.uniforms[i].color = sRandColors[i % 9];
		pass.depthStencilPass.uniforms[i].worldViewProj = viewProjMtx * scene->getWorldMatrix(node.getObjectId());
	}

	pvr::int32 pointLight = 0;
	pvr::uint32 directionalLight = 0;
	for (pvr::uint32 i = 0; i < scene->getNumLightNodes(); ++i)
	{
		const pvr::assets::Node& lightNode = scene->getLightNode(i);
		const pvr::assets::Light& light = scene->getLight(lightNode.getObjectId());
		switch (light.getType())
		{
		case pvr::assets::Light::Point:
		{
			if (pointLight >= Configuration::MaxScenePointLights) { continue; }
			const glm::mat4& transMtx = scene->getWorldMatrix(scene->getNodeIdFromLightNodeId(i));
			const glm::mat4& proxyScale = glm::scale(glm::vec3(Configuration::PointLightScale)) * Configuration::PointlightIntensity;
			const glm::mat4 mWorldScale = transMtx * proxyScale;

			//POINT LIGHT GEOMETRY : The spheres that will be used for the stencil pass
			pass.pointLightGeomPass.uniforms[pointLight].color = glm::vec4(light.getColor(), 1.f);
			pass.pointLightGeomPass.uniforms[pointLight].worldViewProj = viewProjMtx * mWorldScale;

			//POINT LIGHT PROXIES : The "drawcalls" that will perform the actual rendering
			pass.pointLightProxyPass.uniforms[pointLight].lightIntensity = light.getColor() * Configuration::PointlightIntensity;
			pass.pointLightProxyPass.uniforms[pointLight].worldView = viewMtx * mWorldScale;
			pass.pointLightProxyPass.uniforms[pointLight].worldViewProj = viewProjMtx * mWorldScale;
			pass.pointLightProxyPass.uniforms[pointLight].worldIT = glm::mat3(glm::inverseTranspose(transMtx));
			pass.pointLightProxyPass.uniforms[pointLight].lightPosView = glm::vec3((viewMtx * transMtx)[3]); //Translation component of the view matrix

			//POINT LIGHT SOURCES : The little balls that we render to show the lights
			pass.pointLightSourcesPass.uniforms[pointLight].color = glm::vec4(light.getColor(), .8f);
			pass.pointLightSourcesPass.uniforms[pointLight].worldViewProj = viewProjMtx * transMtx;
			pass.pointLightSourcesPass.uniforms[pointLight].worldIT = glm::inverseTranspose(glm::mat3(transMtx));
			++pointLight;
		}
		break;
		case pvr::assets::Light::Directional:
		{
			const glm::mat4& transMtx = scene->getWorldMatrix(scene->getNodeIdFromLightNodeId(i));
			pass.directionalLightPass.uniforms[directionalLight].lightIntensity = light.getColor() * Configuration::DirLightIntensity;
			pass.directionalLightPass.uniforms[directionalLight].lightDirView = viewMtx * transMtx * glm::vec4(0.f, -1.f, 0.f, 0.f);
			++directionalLight;
		}
		break;
		}
	}
	int numSceneLights = pointLight;
	if (Configuration::AdditionalDirectionalLight)
	{
		pass.directionalLightPass.uniforms[directionalLight].lightIntensity = glm::vec3(1, 1, 1) * Configuration::DirLightIntensity;
		pass.directionalLightPass.uniforms[directionalLight].lightDirView = viewMtx * glm::vec4(0.f, -1.f, 0.f, 0.f);
		++directionalLight;
	}
	for (; pointLight < numSceneLights + Configuration::NumProceduralPointLights; ++pointLight)
	{
		updateProceduralPointLight(pass.pointLightProxyPass.data[pointLight], pass.pointLightProxyPass.uniforms[pointLight], pass.pointLightGeomPass.uniforms[pointLight],
		                           pass.pointLightSourcesPass.uniforms[pointLight], false);
	}

}

namespace Configuration {
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
}

void OGLESDeferredShading::updateProceduralPointLight(DrawPointLightProxy::InitialData& data, DrawPointLightProxy::Uniforms& proxy,
    DrawPointLightGeom::Uniforms& geom, DrawLightSources::Uniforms& source, bool initial)
{
	if (initial)
	{
		data.distance = pvr::randomrange(Configuration::LightMinDistance, Configuration::LightMaxDistance);
		data.angle = pvr::randomrange(-glm::pi<pvr::float32>(), glm::pi<pvr::float32>());
		data.height = pvr::randomrange(Configuration::LightMinHeight, Configuration::LightMaxHeight);
		data.axial_vel = pvr::randomrange(-Configuration::LightMaxAxialVelocity, Configuration::LightMaxAxialVelocity);
		data.radial_vel = pvr::randomrange(-Configuration::LightMaxRadialVelocity, Configuration::LightMaxRadialVelocity);
		data.vertical_vel = pvr::randomrange(-Configuration::LightMaxVerticalVelocity, Configuration::LightMaxVerticalVelocity);

		glm::vec3 lightColor = glm::vec3(pvr::randomrange(0, 1), pvr::randomrange(0, 1), pvr::randomrange(0, 1));
		lightColor / glm::max(glm::max(lightColor.x, lightColor.y), lightColor.z); //Have at least one component equal to 1... We want them bright-ish...
		geom.color = glm::vec4(lightColor, 1.);//random-looking
		proxy.lightIntensity = lightColor * Configuration::PointlightIntensity;
		source.color = geom.color;
	}

	if (!initial && !isPaused) //Skip for the first frame, as sometimes this moves the light too far...
	{
		pvr::float32 dt = (pvr::float32)std::min(getFrameTime(), 30ull);
		if (data.distance < Configuration::LightMinDistance) { data.axial_vel = glm::abs(data.axial_vel) + (Configuration::LightMaxAxialVelocity * dt * .001f); }
		if (data.distance > Configuration::LightMaxDistance) { data.axial_vel = -glm::abs(data.axial_vel) - (Configuration::LightMaxAxialVelocity * dt * .001f); }
		if (data.height < Configuration::LightMinHeight) { data.vertical_vel = glm::abs(data.vertical_vel) + (Configuration::LightMaxAxialVelocity * dt * .001f); }
		if (data.height > Configuration::LightMaxHeight) { data.vertical_vel = -glm::abs(data.vertical_vel) - (Configuration::LightMaxAxialVelocity * dt * .001f); }

		data.axial_vel += pvr::randomrange(-Configuration::LightAxialVelocityChange, Configuration::LightAxialVelocityChange) * dt;
		data.radial_vel += pvr::randomrange(-Configuration::LightRadialVelocityChange, Configuration::LightRadialVelocityChange) * dt;
		data.vertical_vel += pvr::randomrange(-Configuration::LightVerticalVelocityChange, Configuration::LightVerticalVelocityChange) * dt;
		if (glm::abs(data.axial_vel) > Configuration::LightMaxAxialVelocity) { data.axial_vel *= .8; }
		if (glm::abs(data.radial_vel) > Configuration::LightMaxRadialVelocity) { data.radial_vel *= .8; }
		if (glm::abs(data.vertical_vel) > Configuration::LightMaxVerticalVelocity) { data.vertical_vel *= .8; }


		data.distance += data.axial_vel * dt * 0.001f;
		data.angle += data.radial_vel * dt * 0.001f;
		data.height += data.vertical_vel * dt * 0.001f;
	}

	float x = sin(data.angle) * data.distance;
	float z = cos(data.angle) * data.distance;
	float y = data.height;

	const glm::mat4& transMtx = glm::translate(glm::vec3(x, y, z));
	const glm::mat4& proxyScale = glm::scale(glm::vec3(Configuration::PointLightScale)) * Configuration::PointlightIntensity;
	const glm::mat4 mWorldScale = transMtx * proxyScale;


	//POINT LIGHT GEOMETRY : The spheres that will be used for the stencil pass
	geom.worldViewProj = viewProjMtx * mWorldScale;

	//POINT LIGHT PROXIES : The "drawcalls" that will perform the actual rendering

	proxy.worldView = viewMtx * mWorldScale;
	proxy.worldViewProj = viewProjMtx * mWorldScale;
	proxy.worldIT = glm::mat3(glm::inverseTranspose(transMtx));
	proxy.lightPosView = glm::vec3((viewMtx * transMtx)[3]); //Translation component of the view matrix

	//POINT LIGHT SOURCES : The little balls that we render to show the lights
	source.worldViewProj = viewProjMtx * transMtx;
	source.worldIT = glm::inverseTranspose(glm::mat3(transMtx));
}

/*!*********************************************************************************************************************
\brief	Updates animation variables and camera matrices.
***********************************************************************************************************************/
void OGLESDeferredShading::updateAnimation()
{
	pvr::uint64 deltaTime = getFrameTime();

	if (!isPaused)
	{
		frame += deltaTime * Configuration::FrameRate;
		if (frame > scene->getNumFrames() - 1) { frame = 0; }
		scene->setCurrentFrame(frame);
	}

	glm::vec3 vTo, vUp;
	pvr::float32 fov;
	scene->getCameraProperties(cameraId, fov, cameraPosition, vTo, vUp);

	pvr::float32 nearClipDist = scene->getCamera(cameraId).getNear();
	farClipDist = scene->getCamera(cameraId).getFar();
	// Update camera matrices
	static float angle = 0;
	if (animateCamera) { angle += getFrameTime() / 1000.f; }
	viewMtx = glm::lookAt(glm::vec3(sin(angle) * 100.f + vTo.x, vTo.y + 30., cos(angle) * 100.f + vTo.z), vTo, vUp);
	viewProjMtx = projMtx * viewMtx;
	invViewMtx = glm::inverse(viewMtx);
}

/*!*********************************************************************************************************************
\brief	Record Pixel-Local-Storage rendering commands
\param	cmdBuff Commandbuffer to record
***********************************************************************************************************************/
void OGLESDeferredShading::recordCommandsPLS(pvr::api::CommandBuffer& cmdBuff)
{
	pvr::Rectanglei renderArea(0, 0, windowWidth, windowHeight);
	//WARNING
	//Pixel local storage defines that the value of PLS variables are "a function of the clear value" if the FBO
	//has been cleared, which is NOT necessarily the same value they were cleared to.
	//Only clearing to the value 0.0f is guaranteed to leave the PLS store with the value 0.0f.
	cmdBuff->beginRenderPass(apiObj->onScreenFbo, renderArea, false, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 1.f, 0);

	cmdBuff->enqueueSecondaryCmds(apiObj->cmdBuffRenderGbuffer);
	cmdBuff->enqueueSecondaryCmds(apiObj->cmdBuffLighting);

	cmdBuff->bindPipeline(apiObj->renderInfo.writePlsPass.pipeline);
	cmdBuff->drawArrays(0, 4);
	cmdBuff->enqueueSecondaryCmds(apiObj->cmdBuffUIRenderer);
	cmdBuff->endRenderPass();
}

/*!*********************************************************************************************************************
\brief	Record MRT rendering commands
\param	cmdBuff CommandBuffer to record
***********************************************************************************************************************/
void OGLESDeferredShading::recordCommandsMRT(pvr::api::CommandBuffer& cmdBuff)
{
	pvr::Rectanglei renderArea(0, 0, fboWidth, fboHeight);

	cmdBuff->beginRenderPass(apiObj->gBufferFBO, renderArea, false, glm::vec4(.0f, 0.0f, 0.0f, 1.0f), 1.f, 0);
	cmdBuff->enqueueSecondaryCmds(apiObj->cmdBuffRenderGbuffer);
	cmdBuff->endRenderPass();

	if ((fboWidth != windowWidth) || (fboHeight != windowHeight))
	{
		renderArea = pvr::Rectanglei(viewportOffsets[0], viewportOffsets[1], fboWidth, fboHeight);
	}

	//  Bind the main frame-buffer object, render the geometry to the depth and stencil buffers and
	//  finally add the light contributions using the GBuffer.
	//  At first render the directional light contributions, utilizing the stencil buffer to avoid executing
	//  the shaders in areas that don't need to be lit (e.g. sky box).
	//  After that render the point light source contributions; in order to limit the amount of shaded fragments
	//  make use of the stencil buffer to imprint the areas that are actually affected by the light sources.
	//  This is similar to the stencil buffer shadow algorithm which runs very efficiently on tile based renderer.
	cmdBuff->beginRenderPass(apiObj->onScreenFbo, renderArea, false, glm::vec4(0.f, 0.f, 0.f, 1.0f), 1.f, 0);

	if (!usePixelLocalStorage)
	{
		cmdBuff->enqueueSecondaryCmds(apiObj->cmdBuffRenderDepthStencil);
	}

	cmdBuff->enqueueSecondaryCmds(apiObj->cmdBuffLighting);

	cmdBuff->enqueueSecondaryCmds(apiObj->cmdBuffUIRenderer);
	cmdBuff->endRenderPass();
}

/*!*********************************************************************************************************************
\brief Allocate memory for Uniforms
***********************************************************************************************************************/
void OGLESDeferredShading::allocateUniforms()
{
	pvr::int32 countPoint = 0;
	pvr::uint32 countDirectional = 0;
	for (pvr::uint32 i = 0; i < scene->getNumLightNodes(); ++i)
	{
		switch (scene->getLight(scene->getLightNode(i).getObjectId()).getType())
		{
		case pvr::assets::Light::Directional: ++countDirectional; break;
		case pvr::assets::Light::Point: ++countPoint; break;
		default: break;
		}
	}
	++countDirectional;
	if (countPoint >= Configuration::MaxScenePointLights) { countPoint = Configuration::MaxScenePointLights; }
	countPoint += Configuration::NumProceduralPointLights;

	apiObj->renderInfo.directionalLightPass.uniforms.resize(countDirectional);
	apiObj->renderInfo.pointLightGeomPass.uniforms.resize(countPoint);
	apiObj->renderInfo.pointLightProxyPass.uniforms.resize(countPoint);
	apiObj->renderInfo.pointLightProxyPass.data.resize(countPoint);
	apiObj->renderInfo.pointLightSourcesPass.uniforms.resize(countPoint);
	apiObj->renderInfo.depthStencilPass.uniforms.resize(scene->getNumMeshNodes());
	apiObj->renderInfo.storeRenderDataPass.objects.resize(scene->getNumMeshNodes());

	for (int i = countPoint - Configuration::NumProceduralPointLights; i < countPoint; ++i)
	{
		updateProceduralPointLight(apiObj->renderInfo.pointLightProxyPass.data[i], apiObj->renderInfo.pointLightProxyPass.uniforms[i],
		                           apiObj->renderInfo.pointLightGeomPass.uniforms[i], apiObj->renderInfo.pointLightSourcesPass.uniforms[i], true);
	}
}

/*!*********************************************************************************************************************
\brief	Record all the secondary command buffer for this scene
***********************************************************************************************************************/
void OGLESDeferredShading::recordSecondaryCommandBuffers()
{
	recordCommandUIRenderer(apiObj->cmdBuffUIRenderer);
	recordCommandBufferRenderGBuffer(apiObj->cmdBuffRenderGbuffer);

	if (!usePixelLocalStorage)
	{
		recordCommandBufferDepthStencil(apiObj->cmdBuffRenderDepthStencil);
	}

	pvr::Rectanglei renderArea(0, 0, fboWidth, fboHeight);
	if ((fboWidth != windowWidth) || (fboHeight != windowHeight))
	{
		renderArea = pvr::Rectanglei(viewportOffsets[0], viewportOffsets[1], fboWidth, fboHeight);
	}

	apiObj->cmdBuffLighting->beginRecording(apiObj->defaultRenderPass);
	recordCommandsDirectionalLights(apiObj->cmdBuffLighting);

	// clear stencil to 0's to make use of it again for point lights
	apiObj->cmdBuffLighting->clearStencilAttachment(renderArea, 0);

	recordCommandsPointLights(apiObj->cmdBuffLighting);
	apiObj->cmdBuffLighting->endRecording();
}


/*!*********************************************************************************************************************
\brief Record rendering G-Buffer commands
\param cmdBuffer Commandbuffer to record
***********************************************************************************************************************/
void OGLESDeferredShading::recordCommandBufferRenderGBuffer(pvr::api::SecondaryCommandBuffer& cmdBuffer)
{
	DrawGBuffer& pass = apiObj->renderInfo.storeRenderDataPass;

	if (usePixelLocalStorage)
	{
		cmdBuffer->beginRecording(apiObj->defaultRenderPass);
	}
	else
	{
		cmdBuffer->beginRecording(apiObj->gBufferRenderPass);
	}

	// write 1 to the stencil buffer when the stencil passes

	for (pvr::uint32 i = 0; i < scene->getNumMeshNodes(); ++i)
	{
		cmdBuffer->bindPipeline(pass.objects[i].pipeline);
		if (usePixelLocalStorage)
		{
			// set stencil reference to 1
			cmdBuffer->setStencilReference(StencilFace::FrontBack, 1);

			// enable stencil writing
			cmdBuffer->setStencilWriteMask(pvr::types::StencilFace::FrontBack, 0xFF);
		}
		pvr::int32 effectId = pass.objects[i].effectId;
		cmdBuffer->setUniformPtr<pvr::float32>(uniformMapping[effectId][Semantics::CustomSemanticFarClipDist], 1, &farClipDist);

		const pvr::assets::Model::Node& node = scene->getNode(i);
		const pvr::assets::Mesh& mesh = scene->getMesh(node.getObjectId());

		const Material& material = apiObj->materials[node.getMaterialIndex()];
		// bind the material descriptor sets (diffuse and bumpmap)
		cmdBuffer->bindDescriptorSet(pass.objects[i].pipeline->getPipelineLayout(),
		                             0, apiObj->materials[node.getMaterialIndex()].materialDescriptorSet, 0);

		cmdBuffer->setUniformPtr<glm::mat4>(uniformMapping[effectId][Semantics::WorldView], 1, &pass.objects[i].worldView);
		cmdBuffer->setUniformPtr<glm::mat4>(uniformMapping[effectId][Semantics::WorldViewProjection], 1, &pass.objects[i].worldViewProj);
		cmdBuffer->setUniformPtr<glm::mat3>(uniformMapping[effectId][Semantics::WorldViewIT], 1, &pass.objects[i].worldViewIT3x3);
		cmdBuffer->setUniformPtr<pvr::float32>(uniformMapping[effectId][Semantics::CustomSemanticSpecularStrength], 1, &material.specularStrength);
		cmdBuffer->setUniformPtr<glm::vec3>(uniformMapping[effectId][Semantics::CustomSemanticDiffuseColor], 1, &material.diffuseColor);

		cmdBuffer->bindVertexBuffer(apiObj->sceneVbos[node.getObjectId()], 0, 0);
		cmdBuffer->bindIndexBuffer(apiObj->sceneIbos[node.getObjectId()], 0, mesh.getFaces().getDataType());
		cmdBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
	}
	cmdBuffer->endRecording();
}


/*!*********************************************************************************************************************
\brief	Record UIRenderer commands
\param	cmdBuff Commandbuffer to record
***********************************************************************************************************************/
void OGLESDeferredShading::recordCommandUIRenderer(pvr::api::SecondaryCommandBuffer& cmdBuff)
{
	cmdBuff->beginRecording(apiObj->defaultRenderPass);
	apiObj->uiRenderer.beginRendering(cmdBuff);
	apiObj->uiRenderer.getDefaultTitle()->render();
	apiObj->uiRenderer.getDefaultControls()->render();
	apiObj->uiRenderer.getSdkLogo()->render();
	apiObj->uiRenderer.endRendering();
	cmdBuff->endRecording();
}

/*!*********************************************************************************************************************
\brief	Record draw scene into depth and stencil commands
\param	cmdBuffer Commandbuffer to record
***********************************************************************************************************************/
void OGLESDeferredShading::recordCommandBufferDepthStencil(pvr::api::SecondaryCommandBuffer& cmdBuffer)
{
	DrawDepthStencil& pass = apiObj->renderInfo.depthStencilPass;
	apiObj->cmdBuffRenderDepthStencil->beginRecording(apiObj->defaultRenderPass);
	// Imprint a 1 into the stencil buffer to indicate where geometry is found.
	// This optimizes the rendering of directional light sources as the shader then only has to be executed where necessary.
	cmdBuffer->bindPipeline(pass.pipeline);
	cmdBuffer->setStencilReference(StencilFace::FrontBack, 1);

	// enable stencil writing
	cmdBuffer->setStencilWriteMask(pvr::types::StencilFace::FrontBack, 0xFF);
	for (pvr::uint32 i = 0; i < scene->getNumMeshNodes(); ++i)
	{
		const pvr::uint32 meshId = scene->getNode(i).getObjectId();
		const pvr::assets::Mesh& mesh = scene->getMesh(meshId);

		cmdBuffer->setUniformPtr<glm::mat4>(uniformMapping[pass.effectId][Semantics::WorldViewProjection], 1, &pass.uniforms[i].worldViewProj);

		cmdBuffer->bindVertexBuffer(apiObj->sceneVbos[meshId], 0, 0);
		cmdBuffer->bindIndexBuffer(apiObj->sceneIbos[meshId], 0, mesh.getFaces().getDataType());
		// Indexed Triangle list
		cmdBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
		cmdBuffer->setUniformPtr<glm::vec4>(uniformMapping[pass.effectId][Semantics::MaterialColorAmbient], 1, &pass.uniforms[i].color);
	}
	apiObj->cmdBuffRenderDepthStencil->endRecording();
}

/*!*********************************************************************************************************************
\brief	Record directional light draw commands
\param  cmdBuffer Commandbuffer to record
***********************************************************************************************************************/
void OGLESDeferredShading::recordCommandsDirectionalLights(pvr::api::SecondaryCommandBuffer& cmdBuffer)
{
	DrawDirLight& pass = apiObj->renderInfo.directionalLightPass;

	//The "uniforms" variable is one per directional light...
	if (pass.uniforms.empty()) { return; }
	cmdBuffer->bindPipeline(pass.pipeline);

	cmdBuffer->setStencilReference(pvr::types::StencilFace::FrontBack, 1);

	// disable stencil writing
	cmdBuffer->setStencilWriteMask(pvr::types::StencilFace::FrontBack, 0x00);

	// Make use of the stencil buffer contents to only shade pixels where actual geometry is located.
	// Reset the stencil buffer to 0 at the same time to avoid the stencil clear operation afterwards.
	// bind the albedo and normal textures from the gbuffer
	cmdBuffer->bindDescriptorSet(apiObj->renderInfo.directionalLightPass.pipeline->getPipelineLayout(), 0, apiObj->directionalLightDescriptorSet, 0);
	for (size_t i = 0; i < pass.uniforms.size(); i++)
	{
		cmdBuffer->setUniformPtr<glm::vec3>(uniformMapping[pass.effectId][Semantics::LightColor], 1, &pass.uniforms[i].lightIntensity);
		cmdBuffer->setUniformPtr<glm::vec4>(uniformMapping[pass.effectId][Semantics::CustomSemanticDirLightDirection], 1, &pass.uniforms[i].lightDirView);
		// Draw a quad
		cmdBuffer->drawArrays(0, 4);
	}
}

/*!*********************************************************************************************************************
\brief	Record point lights draw commands
\param	cmdBuffer Commandbuffer to record
***********************************************************************************************************************/
void OGLESDeferredShading::recordCommandsPointLights(pvr::api::SecondaryCommandBuffer& cmdBuffer)
{
	//Any of the geompointlightpass, lightsourcepointlightpass or pointlightproxiepass's uniforms have the same number of elements
	if (apiObj->renderInfo.pointLightProxyPass.uniforms.empty()) { return; }

	const pvr::assets::Mesh& mesh = pointLightModel->getMesh(0);

	cmdBuffer->setStencilReference(StencilFace::FrontBack, 0);

	//POINT LIGHTS: 1) Draw stencil to discard useless pixels
	cmdBuffer->bindPipeline(apiObj->renderInfo.pointLightGeomPass.pipeline);
	// Bind the vertex and index buffer for the point light
	cmdBuffer->bindVertexBuffer(apiObj->pointLightVbo, 0, 0);
	cmdBuffer->bindIndexBuffer(apiObj->pointLightIbo, 0, IndexType::IndexType16Bit);

	for (size_t i = 0; i < apiObj->renderInfo.pointLightGeomPass.uniforms.size(); i++)
	{
		cmdBuffer->setUniformPtr<glm::mat4>(uniformMapping[apiObj->renderInfo.pointLightGeomPass.effectId][Semantics::WorldViewProjection], 1, &apiObj->renderInfo.pointLightGeomPass.uniforms[i].worldViewProj);
		cmdBuffer->setUniformPtr<glm::vec4>(uniformMapping[apiObj->renderInfo.pointLightGeomPass.effectId][Semantics::MaterialColorAmbient], 1, &apiObj->renderInfo.pointLightGeomPass.uniforms[i].color);
		cmdBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
	}

	//POINT LIGHTS: 2) Lighting
	cmdBuffer->bindDescriptorSet(apiObj->renderInfo.pointLightProxyPass.pipeline->getPipelineLayout(), 0, apiObj->pointLightDescriptorSet, 0);

	cmdBuffer->bindPipeline(apiObj->renderInfo.pointLightProxyPass.pipeline);
	if (uniformMapping[apiObj->renderInfo.pointLightProxyPass.effectId][Semantics::CustomSemanticFarClipDist] >= 0)
	{
		cmdBuffer->setUniformPtr<pvr::float32>(uniformMapping[apiObj->renderInfo.pointLightProxyPass.effectId][Semantics::CustomSemanticFarClipDist], 1, &farClipDist);
	}

	// Bind the vertex and index buffer for the point light
	cmdBuffer->bindVertexBuffer(apiObj->pointLightVbo, 0, 0);
	cmdBuffer->bindIndexBuffer(apiObj->pointLightIbo, 0, mesh.getFaces().getDataType());

	for (pvr::uint32 i = 0; i < apiObj->renderInfo.pointLightProxyPass.uniforms.size(); ++i)
	{
		if (uniformMapping[apiObj->renderInfo.pointLightProxyPass.effectId][Semantics::LightColor] >= 0)
		{
			cmdBuffer->setUniformPtr<glm::vec3>(uniformMapping[apiObj->renderInfo.pointLightProxyPass.effectId][Semantics::LightColor], 1, &apiObj->renderInfo.pointLightProxyPass.uniforms[i].lightIntensity);
		}
		if (uniformMapping[apiObj->renderInfo.pointLightProxyPass.effectId][Semantics::WorldViewProjection] >= 0)
		{
			cmdBuffer->setUniformPtr<glm::mat4>(uniformMapping[apiObj->renderInfo.pointLightProxyPass.effectId][Semantics::WorldViewProjection], 1, &apiObj->renderInfo.pointLightProxyPass.uniforms[i].worldViewProj);
		}
		if (uniformMapping[apiObj->renderInfo.pointLightProxyPass.effectId][Semantics::WorldView] >= 0)
		{
			cmdBuffer->setUniformPtr<glm::mat4>(uniformMapping[apiObj->renderInfo.pointLightProxyPass.effectId][Semantics::WorldView], 1, &apiObj->renderInfo.pointLightProxyPass.uniforms[i].worldView);
		}
		if (uniformMapping[apiObj->renderInfo.pointLightProxyPass.effectId][Semantics::WorldIT] >= 0)
		{
			cmdBuffer->setUniformPtr<glm::mat3>(uniformMapping[apiObj->renderInfo.pointLightProxyPass.effectId][Semantics::WorldIT], 1, &apiObj->renderInfo.pointLightProxyPass.uniforms[i].worldIT);
		}
		if (uniformMapping[apiObj->renderInfo.pointLightProxyPass.effectId][Semantics::CustomSemanticPointLightViewPos] >= 0)
		{
			cmdBuffer->setUniformPtr<glm::vec3>(uniformMapping[apiObj->renderInfo.pointLightProxyPass.effectId][Semantics::CustomSemanticPointLightViewPos], 1, &apiObj->renderInfo.pointLightProxyPass.uniforms[i].lightPosView);
		}
		cmdBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
	}

	//POINT LIGHTS: 3) Light sources
	cmdBuffer->bindPipeline(apiObj->renderInfo.pointLightSourcesPass.pipeline);
	cmdBuffer->bindVertexBuffer(apiObj->pointLightVbo, 0, 0);
	cmdBuffer->bindIndexBuffer(apiObj->pointLightIbo, 0, mesh.getFaces().getDataType());

	for (pvr::uint32 i = 0; i < apiObj->renderInfo.pointLightSourcesPass.uniforms.size(); ++i)
	{
		cmdBuffer->setUniformPtr<glm::mat4>(uniformMapping[apiObj->renderInfo.pointLightSourcesPass.effectId][Semantics::WorldViewProjection], 1, &apiObj->renderInfo.pointLightSourcesPass.uniforms[i].worldViewProj);

		if (uniformMapping[apiObj->renderInfo.pointLightSourcesPass.effectId][Semantics::WorldIT] >= 0)
		{
			cmdBuffer->setUniformPtr<glm::mat3>(uniformMapping[apiObj->renderInfo.pointLightSourcesPass.effectId][Semantics::WorldIT], 1, &apiObj->renderInfo.pointLightSourcesPass.uniforms[i].worldIT);
		}
		if (uniformMapping[apiObj->renderInfo.pointLightSourcesPass.effectId][Semantics::MaterialColorAmbient] >= 0)
		{
			cmdBuffer->setUniformPtr<glm::vec4>(uniformMapping[apiObj->renderInfo.pointLightSourcesPass.effectId][Semantics::MaterialColorAmbient], 1, &apiObj->renderInfo.pointLightSourcesPass.uniforms[i].color);
		}
		cmdBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
	}
}

/*!*********************************************************************************************************************
\return	Return an auto_ptr to a new Demo class, supplied by the user
\brief	This function must be implemented by the user of the shell. The user should return its Shell object defining the
behaviour of the application.
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() { return std::auto_ptr<pvr::Shell>(new OGLESDeferredShading()); }
