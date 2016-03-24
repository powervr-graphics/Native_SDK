/*!*********************************************************************************************************************
\file         OGLESGlass.cpp
\Title        Glass
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Demonstrates dynamic reflection and refraction by rendering two halves of the scene to a single rectangular texture.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVRUIRenderer/PVRUIRenderer.h"
#include "PVRAssets/Shader.h"
#include <limits.h>

// Vertex attributes
namespace VertexAttrib {
enum Enum {	Position, Normal, TEXCOORD_ARRAY, eNumAttribs	};
const char* names[] = {	"inVertex", "inNormal", "inTexCoords"};
}

// Shader uniforms
namespace ShaderUniforms {
enum Enum {	MVPMatrix, MVMatrix, MMatrix, InvVPMatrix, LightDir, EyePos, NumUniforms };
const char* names[] = {"MVPMatrix", "MVMatrix", "MMatrix", "InvVPMatrix", "LightDir", "EyePos"};
}

enum
{
	NumShaderDefines = 3, NumEffects = 5
};


const pvr::float32 CamNear = 1.0f;
const pvr::uint32 ParaboloidTexSize = 1024;
const pvr::float32 CamFar = 5000.0f;
const pvr::float32 CamFov = glm::pi<pvr::float32>() * 0.41f;

const char* BalloonTexFile[2]					= { "BalloonTex.pvr", "BalloonTex2.pvr" };

const char CubeTexFile[]						= "SkyboxTex.pvr";

const char StatueFile[]							= "scene.pod";
const char BalloonFile[]						= "Balloon.pod";

const char FragShaderSrcFile[]					= "DefaultFragShader.fsh";
const char VertShaderSrcFile[]					= "DefaultVertShader.vsh";
const char ReflectionFragShaderSrcFile[]		= "EffectFragShader.fsh";
const char ReflectionVertShaderSrcFile[]		= "EffectVertShader.vsh";
const char SkyboxFragShaderSrcFile[]			= "SkyboxFragShader.fsh";
const char SkyboxVertShaderSrcFile[]			= "SkyboxVertShader.vsh";
const char ParaboloidVertShaderSrcFile[]		= "ParaboloidVertShader.vsh";

const char* EffectShaderDefines[NumEffects][NumShaderDefines] =
{
	{ "REFLECT", "REFRACT", "CHROMATIC" }, { "REFLECT", "REFRACT", 0 }, { "REFLECT", "_UNUSED1_", 0 }, { "REFRACT", "CHROMATIC", 0 }, { "REFRACT", 0, 0 }
};

const int NumEffectShaderDefines[NumEffects] = { 3, 2, 1, 2, 1 };

const char* EffectNames[NumEffects] =
{
	"Reflection + Chromatic Dispersion", "Reflection + Refraction", "Reflection", "Chromatic Dispersion", "Refraction"
};

const glm::vec4 ClearSkyColor(glm::vec4(0.6f, 0.8f, 1.0f, 0.0f));

const pvr::utils::VertexBindings_Name VertexBindings[] =
{
	{ "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoords" }
};

/*!*********************************************************************************************************************
 Class implementing the pvr::Shell functions.
***********************************************************************************************************************/
class OGLESGlass : public pvr::Shell
{
	struct ApiObjects
	{
		// UIRenderer class used to display text
		pvr::ui::UIRenderer uiRenderer;
		// 3D Models
		struct Model
		{
			pvr::assets::ModelHandle      handle;
			std::vector<pvr::api::Buffer> vbos;
			std::vector<pvr::api::Buffer> ibos;
		} statue, balloon;

		pvr::api::TextureView texCube;
		pvr::api::TextureView texBalloon[2];
		pvr::api::Buffer vboSquare;

		pvr::api::Fbo fboOnScreen;
		struct
		{
			pvr::api::Fbo fbo;
			pvr::api::TextureView rtColorImage;
			pvr::api::TextureView rtDsImage;
			pvr::api::ImageStorageFormat rtColorFmt;
			pvr::api::ImageStorageFormat rtDsFmt;

		} fboParaboloid;

		struct Pass
		{
			struct UniformData
			{
				glm::mat4 modelView;
				glm::mat4 modelViewProj;
				glm::mat3 model3x3;
				glm::mat4 invViewProj;
				glm::vec3 eyePos;
				glm::vec3 lightDir;
			};
			std::vector<UniformData> uniformData;
			std::vector<pvr::api::DescriptorSet> imageSamplerDescSets;
		} passSkyBox, passBalloon, passDrawBall;

		struct ParaboloidPass
		{
			Pass passBalloon1;
			Pass passBalloon2;
		} passParaboloid;

		// Group shader programs and their uniform locations together
		struct Pipeline
		{
			pvr::api::GraphicsPipeline pipe;
			pvr::int32 uniformLoc[ShaderUniforms::NumUniforms];
			pvr::api::DescriptorSetLayout descSetLayout; // using single descriptorSet
		}
		pipeDefault, pipeSkyBox, pipeparaboloid[2], pipeEffects[NumEffects];
		pvr::api::CommandBuffer primaryCommandBuffer;
		pvr::api::SecondaryCommandBuffer paraboloidCmdBuffer;
		pvr::api::SecondaryCommandBuffer uiRendererCmdBuffer;
		pvr::api::SecondaryCommandBuffer sceneCmdBuffer;

		pvr::GraphicsContext device;
	};

	std::auto_ptr<ApiObjects> apiObj;
	pvr::api::AssetStore assetManager;

	// Projection, view and model matrices
	glm::mat4 projMtx, viewMtx;

	struct Balloon
	{
		glm::mat4 modelMtx;
		float angle;
	};
	std::vector<Balloon> balloons;

	// Rotation angle for the model
	float cameraAngle;
	int numBalloons;


	int currentEffect;
	float tilt, currentTilt;

public:
	OGLESGlass() : numBalloons(2), tilt(0), currentTilt(0) {}
	virtual pvr::Result::Enum initApplication();
	virtual pvr::Result::Enum initView();
	virtual pvr::Result::Enum releaseView();
	virtual pvr::Result::Enum quitApplication();
	virtual pvr::Result::Enum renderFrame();

private:
	bool createImageSampler();
	bool createPipelines();
	void loadVbos();
	bool createFbo();
	void drawMesh(pvr::api::SecondaryCommandBuffer& cmdBuffer, int i32NodeIndex, ApiObjects::Model& model);

	void eventMappedInput(pvr::SimplifiedInput::Enum action);


	void updateBalloons(ApiObjects::Pipeline& pipeline, const glm::mat4& mProjection, const glm::mat4& mView, ApiObjects::Pass& passBalloon);
	void UpdateScene();
	void updateSkybox();
	void updateStatue();
	void updateParaboloids(const glm::vec3& position);
	void recordPerFrameCommandBuffer();

	void recordSecondaryCommands();
	void recordCmdDrawBalloons(pvr::api::SecondaryCommandBuffer& cmd, ApiObjects::Pipeline& pipeline, pvr::uint32 numBalloon, ApiObjects::Pass& passBallon);
	void recordCmdDrawGlassObject(pvr::api::SecondaryCommandBuffer& cmd);
	void recordCmdDrawSkyBox(pvr::api::SecondaryCommandBuffer& cmd);
};

void OGLESGlass::eventMappedInput(pvr::SimplifiedInput::Enum action)
{
	switch (action)
	{
	case pvr::SimplifiedInput::Left:
		currentEffect -= 1;
		currentEffect = (currentEffect + NumEffects) % NumEffects;
		apiObj->uiRenderer.getDefaultDescription()->setText(EffectNames[currentEffect]);
		apiObj->uiRenderer.getDefaultDescription()->commitUpdates();
		recordSecondaryCommands();
		break;
	case pvr::SimplifiedInput::Up:
		tilt += 5.f;
		break;
	case pvr::SimplifiedInput::Down:
		tilt -= 5.f;
		break;
	case pvr::SimplifiedInput::Right:
		currentEffect += 1;
		currentEffect = (currentEffect + NumEffects) % NumEffects;
		apiObj->uiRenderer.getDefaultDescription()->setText(EffectNames[currentEffect]);
		apiObj->uiRenderer.getDefaultDescription()->commitUpdates();
		recordSecondaryCommands();
		break;
	case pvr::SimplifiedInput::ActionClose: exitShell(); break;
	}
}

/*!*********************************************************************************************************************
\return	Return true if no error occurred
\brief	Loads the textures and samplers required for this training course
***********************************************************************************************************************/
bool OGLESGlass::createImageSampler()
{
	if (!assetManager.getTextureWithCaching(apiObj->device, CubeTexFile, &apiObj->texCube, NULL) ||
	        !assetManager.getTextureWithCaching(apiObj->device, BalloonTexFile[0], &apiObj->texBalloon[0], NULL) ||
	        !assetManager.getTextureWithCaching(apiObj->device, BalloonTexFile[1], &apiObj->texBalloon[1], NULL))
	{
		setExitMessage("Failed to load the textures");
		return pvr::Result::Success;
	}
	pvr::assets::SamplerCreateParam samplerInfo;

	samplerInfo.wrapModeU = pvr::types::SamplerWrap::Clamp;
	samplerInfo.wrapModeV = pvr::types::SamplerWrap::Clamp;

	// create sampler cube
	samplerInfo.minificationFilter = pvr::types::SamplerFilter::Linear;
	samplerInfo.magnificationFilter = pvr::types::SamplerFilter::Linear;

	samplerInfo.mipMappingFilter = pvr::types::SamplerFilter::Linear;

	// create sampler trilinear
	pvr::api::Sampler samplerTrilinear = apiObj->device->createSampler(samplerInfo);


	// DrawBalloon Pass
	{
		pvr::api::DescriptorSetUpdate descSetInfo;
		descSetInfo.setCombinedImageSampler(0, apiObj->texBalloon[0], samplerTrilinear);

		pvr::api::DescriptorSet descSet1 = apiObj->device->createDescriptorSetOnDefaultPool(apiObj->pipeDefault.descSetLayout);
		descSet1->update(descSetInfo);
		apiObj->passBalloon.imageSamplerDescSets.push_back(descSet1);

		descSetInfo.setCombinedImageSampler(0, apiObj->texBalloon[1], samplerTrilinear);
		pvr::api::DescriptorSet descSet2 = apiObj->device->createDescriptorSetOnDefaultPool(apiObj->pipeDefault.descSetLayout);
		descSet2->update(descSetInfo);

		apiObj->passBalloon.imageSamplerDescSets.push_back(descSet2);
		apiObj->passBalloon.uniformData.resize(2);
	}
	// draw paraboild pass
	{
		apiObj->passParaboloid.passBalloon1 = apiObj->passParaboloid.passBalloon2 = apiObj->passBalloon;
	}

	// DrawSkybox Pass
	{
		pvr::api::DescriptorSetUpdate descSetInfo;
		descSetInfo.setCombinedImageSampler(0, apiObj->texCube, samplerTrilinear);
		pvr::api::DescriptorSet descSet = apiObj->device->createDescriptorSetOnDefaultPool(apiObj->pipeSkyBox.descSetLayout);
		descSet->update(descSetInfo);
		apiObj->passSkyBox.imageSamplerDescSets.push_back(descSet);
		apiObj->passSkyBox.uniformData.resize(1);
	}

	// Drawball Pass
	{
		pvr::api::DescriptorSetUpdate descSetInfo;
		descSetInfo.setCombinedImageSampler(0, apiObj->fboParaboloid.rtColorImage, samplerTrilinear).
		setCombinedImageSampler(1, apiObj->texCube, samplerTrilinear);
		pvr::api::DescriptorSet descSet = apiObj->device->createDescriptorSetOnDefaultPool(apiObj->pipeEffects[0].descSetLayout);
		descSet->update(descSetInfo);
		apiObj->passDrawBall.imageSamplerDescSets.push_back(descSet);
		apiObj->passDrawBall.uniformData.resize(apiObj->statue.handle->getNumMeshNodes());

	}

	return true;
}

/*!*********************************************************************************************************************
\return	Return true if no error occurred
\brief	Loads and compiles the shaders and links the shader programs required for this training course
***********************************************************************************************************************/
bool OGLESGlass::createPipelines()
{
	apiObj->primaryCommandBuffer->beginRecording(); //used to set one-shot uniforms

	pvr::api::GraphicsPipelineCreateParam basePipeInfo;
	basePipeInfo.depthStencil.setDepthTestEnable(true).setDepthWrite(true);
	pvr::api::ImageStorageFormat onScreenColorFmt, onScreenDSfmt;
	getDisplayFormat(getDisplayAttributes(), &onScreenColorFmt, &onScreenDSfmt);
	pvr::api::pipelineCreation::ColorBlendAttachmentState colorAttachmentState;

	pvr::assets::ShaderFile fileVersioning;
	fileVersioning.populateValidVersions(FragShaderSrcFile, *this);

	pvr::api::Shader fragShaderDefault = apiObj->device->createShader(*fileVersioning.getBestStreamForApi(getApiType()), pvr::types::ShaderType::FragmentShader);
	pvr::api::pipelineCreation::ColorBlendAttachmentState colorBlend;

	// create the single image sampler pipeline layout pipelines
	{
		pvr::api::DescriptorSetLayoutCreateParam descsetLayoutInfo;
		pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
		descsetLayoutInfo.setBinding(0, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);

		apiObj->pipeDefault.descSetLayout = apiObj->pipeSkyBox.descSetLayout =
		                                        apiObj->pipeparaboloid[0].descSetLayout = apiObj->pipeparaboloid[1].descSetLayout =
		                                                apiObj->device->createDescriptorSetLayout(descsetLayoutInfo);

		pipeLayoutInfo.setDescSetLayout(0, apiObj->pipeDefault.descSetLayout);
		//---------------------------------
		//load the default pipeline
		pvr::api::GraphicsPipelineCreateParam pipeInfo;
		fileVersioning.populateValidVersions(VertShaderSrcFile, *this);
		pipeInfo.vertexShader = apiObj->device->createShader(*fileVersioning.getBestStreamForApi(getApiType()), pvr::types::ShaderType::VertexShader);
		pipeInfo.fragmentShader = fragShaderDefault;
		pipeInfo.pipelineLayout = apiObj->device->createPipelineLayout(pipeLayoutInfo);
		pipeInfo.depthStencil.setDepthWrite(true).setDepthTestEnable(true);

		pipeInfo.colorBlend.addAttachmentState(colorBlend);
		pipeInfo.inputAssembler.setPrimitiveTopology(pvr::types::PrimitiveTopology::TriangleList);
		pvr::utils::createInputAssemblyFromMesh(apiObj->balloon.handle->getMesh(0),
		                                        VertexBindings, sizeof(VertexBindings) / sizeof(VertexBindings[0]), pipeInfo);

		apiObj->pipeSkyBox.pipe = apiObj->device->createGraphicsPipeline(pipeInfo);
		apiObj->pipeDefault.pipe = apiObj->device->createGraphicsPipeline(pipeInfo);
		// Store the location of uniforms for later use
		apiObj->pipeDefault.pipe->getUniformLocation(ShaderUniforms::names, ShaderUniforms::NumUniforms, apiObj->pipeDefault.uniformLoc);

		//set image sampler locations.
		apiObj->primaryCommandBuffer->bindPipeline(apiObj->pipeDefault.pipe);
		apiObj->primaryCommandBuffer->setUniform<pvr::int32>(apiObj->pipeDefault.pipe->getUniformLocation("s2DMap"), 0);

		//--------------------------------
		// load the paraboloid pipeline
		// pipeline1 parent pipeline
		fileVersioning.populateValidVersions(ParaboloidVertShaderSrcFile, *this);
		pipeInfo.vertexShader = apiObj->device->createShader(*fileVersioning.getBestStreamForApi(getApiType()), pvr::types::ShaderType::VertexShader);
		pipeInfo.fragmentShader = fragShaderDefault;

		pipeInfo.rasterizer.setCullFace(pvr::types::Face::Front);
		apiObj->pipeparaboloid[0].pipe = apiObj->device->createParentableGraphicsPipeline(pipeInfo);
		apiObj->pipeparaboloid[0].pipe->getUniformLocation(ShaderUniforms::names, ShaderUniforms::NumUniforms, apiObj->pipeparaboloid[0].uniformLoc);
		apiObj->primaryCommandBuffer->bindPipeline(apiObj->pipeparaboloid[0].pipe);

		apiObj->primaryCommandBuffer->setUniform<pvr::int32>(apiObj->pipeparaboloid[0].pipe->getUniformLocation("s2DMap"), 0);

		// create the child pipeline which has cullface as front
		pipeInfo.rasterizer.setCullFace(pvr::types::Face::Back);

		// null out shader because we are going to use the parent pipeline.
		pipeInfo.vertexShader.setShader(pvr::api::Shader());
		pipeInfo.fragmentShader.setShader(pvr::api::Shader());
		apiObj->pipeparaboloid[1].pipe = apiObj->device->createGraphicsPipeline(pipeInfo,
		                                 pvr::api::ParentableGraphicsPipeline(apiObj->pipeparaboloid[0].pipe));
		apiObj->primaryCommandBuffer->bindPipeline(apiObj->pipeparaboloid[1].pipe);
		apiObj->pipeparaboloid[1].pipe->getUniformLocation(ShaderUniforms::names, ShaderUniforms::NumUniforms, apiObj->pipeparaboloid[1].uniformLoc);

		//--------------------------------
		//load the skybox pipeline
		fileVersioning.populateValidVersions(SkyboxVertShaderSrcFile, *this);
		pipeInfo.vertexShader = apiObj->device->createShader(*fileVersioning.getBestStreamForApi(getApiType()), pvr::types::ShaderType::VertexShader);
		fileVersioning.populateValidVersions(SkyboxFragShaderSrcFile, *this);
		pipeInfo.fragmentShader = apiObj->device->createShader(*fileVersioning.getBestStreamForApi(getApiType()),
		                          pvr::types::ShaderType::FragmentShader);

		//pipeInfo.depthStencil.setDepthTestEnable(false).setDepthWrite(false);
		pipeInfo.inputAssembler.setPrimitiveTopology(pvr::types::PrimitiveTopology::TriangleList);
		pipeInfo.vertexInput.clear();
		pipeInfo.vertexInput.setInputBinding(0, sizeof(float) * 3, pvr::types::StepRate::Vertex);
		pipeInfo.vertexInput.addVertexAttribute(0, 0, pvr::assets::VertexAttributeLayout(pvr::types::DataType::Float32, 3, 0),
		                                        VertexBindings[0].variableName.c_str());
		apiObj->pipeSkyBox.pipe = apiObj->device->createGraphicsPipeline(pipeInfo);
		apiObj->pipeSkyBox.pipe->getUniformLocation(ShaderUniforms::names, ShaderUniforms::NumUniforms, apiObj->pipeSkyBox.uniformLoc);
		apiObj->primaryCommandBuffer->bindPipeline(apiObj->pipeSkyBox.pipe);
		apiObj->primaryCommandBuffer->setUniform<pvr::int32>(apiObj->pipeSkyBox.pipe->getUniformLocation("sSkybox"), 0);
	}

	pvr::api::DescriptorSetLayoutCreateParam descSetLayoutInfo;
	// load the effect pipeline, has two image and sampler
	{
		pvr::api::PipelineLayoutCreateParam effectPipeLayout;
		pvr::api::DescriptorSetLayout descLayout;
		descSetLayoutInfo.setBinding(0, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment)
		.setBinding(1, pvr::types::DescriptorType::CombinedImageSampler, 1, pvr::types::ShaderStageFlags::Fragment);
		descLayout = apiObj->device->createDescriptorSetLayout(descSetLayoutInfo);
		effectPipeLayout.addDescSetLayout(descLayout);
		pvr::api::GraphicsPipelineCreateParam pipeInfo;
		pipeInfo.colorBlend.addAttachmentState(pvr::api::pipelineCreation::ColorBlendAttachmentState());
		pipeInfo.pipelineLayout = apiObj->device->createPipelineLayout(effectPipeLayout);
		pvr::utils::createInputAssemblyFromMesh(apiObj->statue.handle->getMesh(0), VertexBindings, 2, pipeInfo);

		fileVersioning.populateValidVersions(ReflectionVertShaderSrcFile, *this);
		pvr::Stream::ptr_type effectVertShader = fileVersioning.getBestStreamForApi(getApiType());

		fileVersioning.populateValidVersions(ReflectionFragShaderSrcFile, *this);
		pvr::Stream::ptr_type effectFragShader = fileVersioning.getBestStreamForApi(getApiType());

		for (pvr::uint32 i = 0; i < NumEffects; ++i)
		{
			pipeInfo.vertexShader.setShader(apiObj->device->createShader(*effectVertShader, pvr::types::ShaderType::VertexShader,
			                                EffectShaderDefines[i], NumEffectShaderDefines[i]));
			pipeInfo.fragmentShader.setShader(apiObj->device->createShader(*effectFragShader, pvr::types::ShaderType::FragmentShader,
			                                  EffectShaderDefines[i], NumEffectShaderDefines[i]));
			// Store the location of uniforms for later use
			apiObj->pipeEffects[i].pipe = apiObj->device->createGraphicsPipeline(pipeInfo);
			apiObj->pipeEffects[i].pipe->getUniformLocation(ShaderUniforms::names, ShaderUniforms::NumUniforms, apiObj->pipeEffects[i].uniformLoc);
			apiObj->pipeEffects[i].descSetLayout = descLayout;
			apiObj->primaryCommandBuffer->bindPipeline(apiObj->pipeEffects[i].pipe);
			apiObj->primaryCommandBuffer->setUniform<pvr::int32>(apiObj->pipeEffects[i].pipe->getUniformLocation("sParaboloids"), 0);
			apiObj->primaryCommandBuffer->setUniform<pvr::int32>(apiObj->pipeEffects[i].pipe->getUniformLocation("sSkybox"), 1);
			effectVertShader->seek(0, pvr::Stream::SeekOriginFromStart);
			effectFragShader->seek(0, pvr::Stream::SeekOriginFromStart);
		}
	}
	apiObj->primaryCommandBuffer->endRecording();
	apiObj->primaryCommandBuffer->submit();
	apiObj->primaryCommandBuffer->clear();
	return true;
}

/*!*********************************************************************************************************************
\brief	Loads the mesh data required for this training course into	vertex buffer objects
***********************************************************************************************************************/
void OGLESGlass::loadVbos()
{
	//	Load vertex data of all meshes in the scene into VBOs
	//	The meshes have been exported with the "Interleave Vectors" option,
	//	so all data is interleaved in the buffer at pMesh->pInterleaved.
	//	Interleaving data improves the memory access pattern and cache efficiency,
	//	thus it can be read faster by the hardware.
	pvr::utils::appendSingleBuffersFromModel(getGraphicsContext(), *apiObj->statue.handle, apiObj->statue.vbos, apiObj->statue.ibos);
	pvr::utils::appendSingleBuffersFromModel(getGraphicsContext(), *apiObj->balloon.handle, apiObj->balloon.vbos, apiObj->balloon.ibos);

	static pvr::float32 quadVertices[] =
	{
		-1,  1, 0.9999f,
		-1, -1, 0.9999f,
		1,  1, 0.9999f,
		1,  1, 0.9999f,
		-1, -1, 0.9999f,
		1, -1, 0.9999f
	};
	apiObj->vboSquare = apiObj->device->createBuffer(sizeof(quadVertices), pvr::types::BufferBindingUse::VertexBuffer);
	apiObj->vboSquare->update(quadVertices, 0, sizeof(quadVertices));
}

/*!*********************************************************************************************************************
\return	Return true if no error occurred
\brief	Creates the required frame buffers and textures to render into.
***********************************************************************************************************************/
bool OGLESGlass::createFbo()
{
	apiObj->fboOnScreen = apiObj->device->createOnScreenFbo(0);
	pvr::api::SubPass subPass(pvr::types::PipelineBindPoint::Graphics);
	subPass.setColorAttachment(0); // use the first color attachment

	// create paraboloid fbo
	{
		pvr::api::ImageStorageFormat rtDsFmt(pvr::PixelFormat::Depth16, 1, pvr::types::ColorSpace::lRGB, pvr::VariableType::UnsignedShort);
		apiObj->fboParaboloid.rtColorFmt = pvr::api::ImageStorageFormat(pvr::PixelFormat::RGBA_8888, 1, pvr::types::ColorSpace::lRGB,
		                                   pvr::VariableType::UnsignedByteNorm);

		apiObj->fboParaboloid.rtDsFmt = pvr::api::ImageStorageFormat(pvr::PixelFormat::Depth16, 1,
		                                pvr::types::ColorSpace::lRGB, pvr::VariableType::UnsignedShort);

		const pvr::uint32 fboWidth = ParaboloidTexSize * 2;
		const pvr::uint32 fboHeight = ParaboloidTexSize;

		//create the renderpass
		pvr::api::RenderPassCreateParam renderPassInfo;
		renderPassInfo
		.addColorInfo(0, pvr::api::RenderPassColorInfo(apiObj->fboParaboloid.rtColorFmt, pvr::types::LoadOp::Clear))
		.setDepthStencilInfo(pvr::api::RenderPassDepthStencilInfo(rtDsFmt, pvr::types::LoadOp::Clear))
		.addSubPass(0, subPass);

		// create the render-target color texture
		pvr::api::TextureStore rtColorTex = apiObj->device->createTexture();
		rtColorTex->allocate2D(apiObj->fboParaboloid.rtColorFmt, fboWidth, fboHeight);
		apiObj->fboParaboloid.rtColorImage = apiObj->device->createTextureView(rtColorTex);

		// create the render-target depth-stencil texture
		pvr::api::TextureStore rtDsTex = apiObj->device->createTexture();
		rtDsTex->allocate2D(apiObj->fboParaboloid.rtDsFmt, fboWidth, fboHeight);
		apiObj->fboParaboloid.rtDsImage = apiObj->device->createTextureView(rtDsTex);

		// create the fbo
		pvr::api::FboCreateParam fboInfo;
		fboInfo.setRenderPass(apiObj->device->createRenderPass(renderPassInfo))
		.addColor(0, apiObj->fboParaboloid.rtColorImage)
		.setDepthStencil(apiObj->fboParaboloid.rtDsImage);
		apiObj->fboParaboloid.fbo = apiObj->device->createFbo(fboInfo);
		if (!apiObj->fboParaboloid.fbo.isValid())
		{
			setExitMessage("failed to create the paraboloid fbo");
			return false;
		}
	}
	return true;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in initApplication() will be called by PVRShell once perrun, before the rendering context is created.
		Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
		If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result::Enum OGLESGlass::initApplication()
{
	assetManager.init(*this);

	cameraAngle =  glm::pi<glm::float32>() - .6f;
	balloons.resize(numBalloons);
	for (int i = 0; i < numBalloons; ++i) {	balloons[i].angle = glm::pi<glm::float32>() * i / 5.f;	}
	currentEffect = 0;
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.If the rendering context
		is lost, quitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result::Enum OGLESGlass::quitApplication() { return pvr::Result::Success; }

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in initView() will be called by PVRShell upon initialization or after a change in the rendering context.
		Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result::Enum OGLESGlass::initView()
{
	apiObj.reset(new ApiObjects);

	// Load the mask
	if (!assetManager.loadModel(StatueFile, apiObj->statue.handle))
	{
		this->setExitMessage("ERROR: Couldn't load the .pod file\n");
		return pvr::Result::NotInitialized;
	}

	// Load the balloon
	if (!assetManager.loadModel(BalloonFile, apiObj->balloon.handle))
	{
		this->setExitMessage("ERROR: Couldn't load the .pod file\n");
		return pvr::Result::NotInitialized;
	}

	// Store the original FBO
	apiObj->device = getGraphicsContext();
	apiObj->primaryCommandBuffer = apiObj->device->createCommandBufferOnDefaultPool();

	//Initialize VBO data
	loadVbos();
	if (!createFbo()) { return pvr::Result::UnknownError; }
	if (!createPipelines()) { return pvr::Result::UnknownError; }
	if (!createImageSampler())	{ return pvr::Result::UnknownError;	}

	//Initialize UIRenderer
	if (apiObj->uiRenderer.init(apiObj->device, apiObj->fboOnScreen->getRenderPass(), 0) != pvr::Result::Success)
	{
		this->setExitMessage("ERROR: Cannot initialize UIRenderer\n");
		return pvr::Result::UnknownError;
	}

	apiObj->uiRenderer.getDefaultTitle()->setText("Glass");
	apiObj->uiRenderer.getDefaultTitle()->commitUpdates();
	apiObj->uiRenderer.getDefaultDescription()->setText(EffectNames[currentEffect]);
	apiObj->uiRenderer.getDefaultDescription()->commitUpdates();
	apiObj->uiRenderer.getDefaultControls()->setText("Left / Right : Change the effect\nUp / Down  : Tilt camera");
	apiObj->uiRenderer.getDefaultControls()->commitUpdates();
	apiObj->primaryCommandBuffer->beginRecording();
	// set the texture location of the parent pipe
	apiObj->primaryCommandBuffer->bindPipeline(apiObj->pipeparaboloid[0].pipe);
	apiObj->primaryCommandBuffer->setUniform<float>(apiObj->pipeparaboloid[0].pipe->getUniformLocation("Near"), CamNear);
	apiObj->primaryCommandBuffer->setUniform<float>(apiObj->pipeparaboloid[0].pipe->getUniformLocation("Far"), CamFar);
	apiObj->primaryCommandBuffer->endRecording();
	apiObj->primaryCommandBuffer->submit();
	//Calculate the projection and view matrices
	projMtx = glm::perspectiveFov(CamFov, (float)this->getWidth(), (float)this->getHeight(), CamNear, CamFar);
	if (isScreenRotated())
	{
		projMtx = projMtx * glm::rotate(glm::pi<pvr::float32>() * .5f , glm::vec3(.0f, .0f, 1.f));
	}
	recordSecondaryCommands();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result::Enum OGLESGlass::releaseView()
{
	apiObj.reset();
	assetManager.releaseAll();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result::Enum OGLESGlass::renderFrame()
{
	UpdateScene();
	updateParaboloids(glm::vec3(0, 0, 0));

	// Draw the statue
	updateStatue();

	// Draw the balloons
	updateBalloons(apiObj->pipeDefault, projMtx, viewMtx, apiObj->passBalloon);

	// Draw the skybox
	updateSkybox();

	recordPerFrameCommandBuffer();
	apiObj->primaryCommandBuffer->submit();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief	Record draw balloon commands
\param	cmdBuffer Recording command buffer
\param	pipeline Pipeline to bind
\param	numBalloon Number of ballons to draw
\param	passBallon Balloon pass to use
***********************************************************************************************************************/
void OGLESGlass::recordCmdDrawBalloons(pvr::api::SecondaryCommandBuffer& cmdBuffer, ApiObjects::Pipeline& pipeline, pvr::uint32 numBalloon, ApiObjects::Pass& passBallon)
{
	// Use shader program
	cmdBuffer->bindPipeline(pipeline.pipe);
	glm::mat4 mModelView, mMVP;
	for (pvr::uint32 i = 0; i < numBalloon; ++i)
	{
		cmdBuffer->bindDescriptorSet(pipeline.pipe->getPipelineLayout(), 0,
		                             i == 0 ? passBallon.imageSamplerDescSets[0] : passBallon.imageSamplerDescSets[1], 0);

		cmdBuffer->setUniformPtr<glm::mat4>(pipeline.uniformLoc[ShaderUniforms::MVMatrix], 1, &passBallon.uniformData[i].modelView);
		cmdBuffer->setUniformPtr<glm::mat4>(pipeline.uniformLoc[ShaderUniforms::MVPMatrix], 1, &passBallon.uniformData[i].modelViewProj);
		cmdBuffer->setUniformPtr<glm::vec3>(pipeline.uniformLoc[ShaderUniforms::LightDir], 1, &passBallon.uniformData[i].lightDir);
		cmdBuffer->setUniformPtr<glm::vec3>(pipeline.uniformLoc[ShaderUniforms::EyePos], 1, &passBallon.uniformData[i].eyePos);
		// Now that the uniforms are set, call another function to actually draw the mesh.
		drawMesh(cmdBuffer, 0, apiObj->balloon);
	}
}

/*!*********************************************************************************************************************
\brief	Record draw ball commands
\param  cmdBuffer Recording commandbuffer
***********************************************************************************************************************/
void OGLESGlass::recordCmdDrawGlassObject(pvr::api::SecondaryCommandBuffer& cmdBuffer)
{
	// Use shader program
	cmdBuffer->bindPipeline(apiObj->pipeEffects[currentEffect].pipe);

	// bind the texture and samplers
	cmdBuffer->bindDescriptorSet(apiObj->pipeEffects[currentEffect].pipe->getPipelineLayout(),
	                             0, apiObj->passDrawBall.imageSamplerDescSets[0], 0);

	cmdBuffer->setUniformPtr<glm::vec3>(apiObj->pipeEffects[currentEffect].uniformLoc[ShaderUniforms::EyePos], 1, &apiObj->passDrawBall.uniformData[0].eyePos);
	cmdBuffer->setUniformPtr<glm::mat4>(apiObj->pipeEffects[currentEffect].uniformLoc[ShaderUniforms::MVPMatrix], 1, &apiObj->passDrawBall.uniformData[0].modelViewProj);
	cmdBuffer->setUniformPtr<glm::mat3>(apiObj->pipeEffects[currentEffect].uniformLoc[ShaderUniforms::MMatrix], 1, &apiObj->passDrawBall.uniformData[0].model3x3);
	// Now that the uniforms are set, call another function to actually draw the mesh
	drawMesh(cmdBuffer, 0, apiObj->statue);
}

/*!*********************************************************************************************************************
\brief	record draw skybox commands
\param  cmdBuffer Recording commandbuffer
***********************************************************************************************************************/
void OGLESGlass::recordCmdDrawSkyBox(pvr::api::SecondaryCommandBuffer& cmdBuffer)
{
	cmdBuffer->bindPipeline(apiObj->pipeSkyBox.pipe);
	cmdBuffer->setUniformPtr<glm::mat4>(apiObj->pipeSkyBox.uniformLoc[ShaderUniforms::InvVPMatrix], 1, &apiObj->passSkyBox.uniformData[0].invViewProj);

	cmdBuffer->setUniformPtr<glm::vec3>(apiObj->pipeSkyBox.uniformLoc[ShaderUniforms::EyePos], 1, &apiObj->passSkyBox.uniformData[0].eyePos);
	cmdBuffer->bindVertexBuffer(apiObj->vboSquare, 0, 0);

	cmdBuffer->bindDescriptorSet(apiObj->pipeSkyBox.pipe->getPipelineLayout(), 0, apiObj->passSkyBox.imageSamplerDescSets[0], 0);
	cmdBuffer->drawArrays(0, 6, 0, 1);
}

/*!*********************************************************************************************************************
\brief	Update the scene
***********************************************************************************************************************/
void OGLESGlass::UpdateScene()
{
	// Fetch current time and make sure the previous time isn't greater
	pvr::uint64 timeDifference = getFrameTime();
	// Store the current time for the next frame
	cameraAngle += timeDifference * 0.00005f;
	for (pvr::int32 i = 0; i < numBalloons; ++i) { balloons[i].angle += timeDifference * 0.0002f * (pvr::float32(i) * .5f + 1.f); }

	static const glm::vec3 rotateAxis(0.0f, 1.0f, 0.0f);
	pvr::float32 diff = fabs(tilt - currentTilt);
	pvr::float32 diff2 = timeDifference / 20.f;
	currentTilt += glm::sign(tilt - currentTilt) * (std::min)(diff, diff2);

	// Rotate the camera
	viewMtx = glm::lookAt(glm::vec3(0, -4, -10), glm::vec3(0, currentTilt - 3, 0), glm::vec3(0, 1, 0))
	          * glm::rotate(cameraAngle, rotateAxis);

	for (pvr::int32 i = 0; i < numBalloons; ++i)
	{
		// Rotate the balloon model matrices
		balloons[i].modelMtx = glm::rotate(balloons[i].angle, rotateAxis) * glm::translate(glm::vec3(120.f + i * 40.f ,
		                       sin(balloons[i].angle * 3.0f) * 20.0f, 0.0f)) * glm::scale(glm::vec3(3.0f, 3.0f, 3.0f));
	}
}

/*!*********************************************************************************************************************
\param[in]		nodeIndex		Node index of the mesh to draw
\param[in]  cmdBuffer Recording commandbuffer
\param[in]  model A Model which has the rendering mesh
\brief	Draws a pvr::assets::Mesh after the model view matrix has been set and the material prepared.
***********************************************************************************************************************/
void OGLESGlass::drawMesh(pvr::api::SecondaryCommandBuffer& cmdBuffer, int nodeIndex, ApiObjects::Model& model)
{
	pvr::int32 meshId = model.handle->getNode(nodeIndex).getObjectId();
	const pvr::assets::Mesh& mesh = model.handle->getMesh(meshId);

	// bind the VBO for the mesh
	cmdBuffer->bindVertexBuffer(model.vbos[meshId], 0, 0);
	if (mesh.getFaces().getDataSize() != 0)
	{
		// Indexed Triangle list
		cmdBuffer->bindIndexBuffer(model.ibos[meshId], 0, mesh.getFaces().getDataType());
		cmdBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
	}
	else
	{
		// Non-Indexed Triangle list
		cmdBuffer->drawArrays(0, mesh.getNumFaces() * 3, 0, 1);
	}
}

/*!*********************************************************************************************************************
\brief	update the balloon's uniform data
\param	pipeline Pipeline to bind
\param	projMtx Projection matrix
\param	viewMtx View matrix
\param	passBalloon Balloon draw pass
***********************************************************************************************************************/
void OGLESGlass::updateBalloons(ApiObjects::Pipeline& pipeline, const glm::mat4& projMtx, const glm::mat4& viewMtx, ApiObjects::Pass& passBalloon)
{
	for (pvr::int32 i = 0; i < numBalloons; ++i)
	{
		passBalloon.uniformData[i].modelView = viewMtx * balloons[i].modelMtx;
		passBalloon.uniformData[i].modelViewProj = projMtx * passBalloon.uniformData[i].modelView;

		// Calculate and set the model space light direction
		passBalloon.uniformData[i].lightDir = glm::vec3(glm::normalize(glm::inverse(balloons[i].modelMtx) * glm::vec4(19, 22, -50, 0)));

		// Calculate and set the model space eye position
		passBalloon.uniformData[i].eyePos = glm::vec3(glm::inverse(passBalloon.uniformData[i].modelView) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	}
}

/*!*********************************************************************************************************************
\brief	update the skybox uniform's data
***********************************************************************************************************************/
void OGLESGlass::updateSkybox()
{
	apiObj->passSkyBox.uniformData.resize(1);
	apiObj->passSkyBox.uniformData[0].invViewProj = glm::inverse(projMtx * viewMtx);
	apiObj->passSkyBox.uniformData[0].eyePos = glm::vec3(glm::inverse(viewMtx) * glm::vec4(0, 0, 0, 1));
}

/*!*********************************************************************************************************************
\brief	Draws the reflective and refractive statue onto the screen.
***********************************************************************************************************************/
void OGLESGlass::updateStatue()
{
	// The final statue transform brings him with 0.0.0 coordinates at his feet.
	// For this model we want 0.0.0 to be the around the center of the statue, and the statue to be smaller.
	// So, we apply a transformation, AFTER all transforms that have brought him to the center,
	// that will shrink him and move him downwards.
	static const glm::vec3 scale = glm::vec3(0.25f, 0.25f, 0.25f);
	static const glm::vec3 offset = glm::vec3(0.f, -2.f, 0.f);
	static const glm::mat4 local_transform = glm::translate(offset) * glm::scale(scale);
	// Set model view projection matrix
	for (size_t i = 0; i < apiObj->statue.handle->getNumMeshNodes(); ++i)
	{
		glm::mat4 modelMatrix = local_transform * apiObj->statue.handle->getWorldMatrix(i);
		glm::mat4 modelView = viewMtx * modelMatrix;
		apiObj->passDrawBall.uniformData[i].modelViewProj = projMtx * modelView;
		apiObj->passDrawBall.uniformData[i].model3x3 = glm::mat3(modelMatrix);

		// Set eye position in model space
		apiObj->passDrawBall.uniformData[i].eyePos = glm::vec3(glm::inverse(modelView) * glm::vec4(0, 0, 0, 1));
	}
}

/*!*********************************************************************************************************************
\brief	Draws the scene from the position of the statue into the two	paraboloid textures.
\param[in] position New position
***********************************************************************************************************************/
void OGLESGlass::updateParaboloids(const glm::vec3& position)
{
	// Create the first view matrix and make it flip the X coordinate
	glm::mat4 mView = glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
	mView = glm::scale(glm::vec3(-1.0f, 1.0f, 1.0f)) * mView;
	// Draw the balloons
	updateBalloons(apiObj->pipeparaboloid[0], glm::mat4(1.f), mView, apiObj->passParaboloid.passBalloon1);

	// Create the second view matrix
	mView = glm::lookAt(position, position - glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
	// Draw the balloons
	updateBalloons(apiObj->pipeparaboloid[1], glm::mat4(1.f), mView, apiObj->passParaboloid.passBalloon2);
}

/*!*********************************************************************************************************************
\brief	record all the secondary command buffers
***********************************************************************************************************************/
void OGLESGlass::recordSecondaryCommands()
{
	apiObj->paraboloidCmdBuffer = apiObj->device->createSecondaryCommandBufferOnDefaultPool();
	apiObj->paraboloidCmdBuffer->beginRecording(apiObj->fboParaboloid.fbo, 0);
	// Switch to front face culling pipeline due to flipped winding order
	apiObj->paraboloidCmdBuffer->bindPipeline(apiObj->pipeparaboloid[0].pipe);
	apiObj->paraboloidCmdBuffer->setViewport(pvr::Rectanglei(0, 0, ParaboloidTexSize, ParaboloidTexSize));

	// Draw the balloons
	recordCmdDrawBalloons(apiObj->paraboloidCmdBuffer, apiObj->pipeparaboloid[0], 2, apiObj->passParaboloid.passBalloon1);

	// Switch back face culling pipeline
	apiObj->paraboloidCmdBuffer->bindPipeline(apiObj->pipeparaboloid[1].pipe);

	// Shift the viewport to the right
	apiObj->paraboloidCmdBuffer->setViewport(pvr::Rectanglei(ParaboloidTexSize, 0, ParaboloidTexSize, ParaboloidTexSize));

	// Draw the balloons
	recordCmdDrawBalloons(apiObj->paraboloidCmdBuffer, apiObj->pipeparaboloid[1], 2, apiObj->passParaboloid.passBalloon2);
	apiObj->paraboloidCmdBuffer->endRecording();


	apiObj->sceneCmdBuffer = apiObj->device->createSecondaryCommandBufferOnDefaultPool();
	apiObj->sceneCmdBuffer->beginRecording(apiObj->fboOnScreen, 0);
	recordCmdDrawGlassObject(apiObj->sceneCmdBuffer);
	recordCmdDrawBalloons(apiObj->sceneCmdBuffer, apiObj->pipeDefault, 2, apiObj->passBalloon);
	recordCmdDrawSkyBox(apiObj->sceneCmdBuffer);
	apiObj->sceneCmdBuffer->endRecording();

	// render the title, sdk logo and description
	apiObj->uiRendererCmdBuffer = apiObj->device->createSecondaryCommandBufferOnDefaultPool();
	apiObj->uiRenderer.beginRendering(apiObj->uiRendererCmdBuffer);
	apiObj->uiRenderer.getSdkLogo()->render();
	apiObj->uiRenderer.getDefaultTitle()->render();
	apiObj->uiRenderer.getDefaultDescription()->render();
	apiObj->uiRenderer.getDefaultControls()->render();
	apiObj->uiRenderer.endRendering();
}

/*!*********************************************************************************************************************
\brief	Record all the rendering commands for each frame
***********************************************************************************************************************/
void OGLESGlass::recordPerFrameCommandBuffer()
{
	apiObj->primaryCommandBuffer->beginRecording();

	// draw in to paraboloids
	{
		// Bind and clear the paraboloid framebuffer , Use a nice bright blue as clear color
		// Set the renderArea to the left
		apiObj->primaryCommandBuffer->beginRenderPass(apiObj->fboParaboloid.fbo, pvr::Rectanglei(0, 0, 2 * ParaboloidTexSize, ParaboloidTexSize), false, ClearSkyColor);
		apiObj->primaryCommandBuffer->enqueueSecondaryCmds(apiObj->paraboloidCmdBuffer);
		apiObj->primaryCommandBuffer->endRenderPass();
	}

	// Bind back the original frame buffer and reset the viewport
	apiObj->primaryCommandBuffer->beginRenderPass(apiObj->fboOnScreen, pvr::Rectanglei(0, 0, getWidth(), getHeight()), false, ClearSkyColor);
	apiObj->primaryCommandBuffer->enqueueSecondaryCmds(apiObj->sceneCmdBuffer);
	apiObj->primaryCommandBuffer->enqueueSecondaryCmds(apiObj->uiRendererCmdBuffer);
	apiObj->primaryCommandBuffer->endRenderPass();
	apiObj->primaryCommandBuffer->endRecording();
}

/*!*********************************************************************************************************************
\return	auto ptr of the demo supplied by the user
\brief	This function must be implemented by the user of the shell. The user should return its PVRShell object defining the
		behaviour of the application.
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() {	return std::auto_ptr<pvr::Shell>(new OGLESGlass()); }
