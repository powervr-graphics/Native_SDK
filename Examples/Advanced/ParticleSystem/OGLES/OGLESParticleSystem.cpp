/*!*********************************************************************************************************************
\File         OGLESParticleSystem.cpp
\Title        ParticleSystem
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Particle animation system using Compute Shaders. Requires the PVRShell.
***********************************************************************************************************************/
#include "ParticleSystemGPU.h"
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVRUIRenderer/PVRUIRenderer.h"
using namespace pvr::types;

namespace Files {
// Asset files
const char SphereModelFile[] = "sphere.pod";

const char FragShaderSrcFile[] = "FragShader.fsh";
const char VertShaderSrcFile[] = "VertShader.vsh";
const char ParticleShaderFragSrcFile[] = "ParticleFragShader.fsh";
const char ParticleShaderVertSrcFile[] = "ParticleVertShader.vsh";
}

namespace Configuration {
enum
{
	MinNoParticles = 128,
	MaxNoParticles = 131072 * 64,
	InitialNoParticles = 32768,
	NumberOfSpheres = 8,
};

const float CameraNear = .1f;
const float CameraFar = 1000.0f;
const glm::vec3 LightPosition(0.0f, 10.0f, 0.0f);

const Sphere Spheres[] =
{
	Sphere(glm::vec3(-20.0f, 6.0f, -20.0f), 5.f) ,
	Sphere(glm::vec3(-20.0f, 6.0f,   0.0f), 5.f) ,
	Sphere(glm::vec3(-20.0f, 6.0f,  20.0f), 5.f) ,
	Sphere(glm::vec3(0.0f, 6.0f, -20.0f), 5.f) ,
	Sphere(glm::vec3(0.0f, 6.0f,  20.0f), 5.f) ,
	Sphere(glm::vec3(20.0f, 6.0f, -20.0f), 5.f) ,
	Sphere(glm::vec3(20.0f, 6.0f,   0.0f), 5.f) ,
	Sphere(glm::vec3(20.0f, 6.0f,  20.0f), 5.f) ,
};
}

// Index to bind the attributes to vertex shaders

namespace Attributes {
enum Enum
{
	ParticlePoisitionArray = 0, ParticleLifespanArray = 1,
	VertexArray = 0, NormalArray = 1, TexCoordArray = 2,
	BindingIndex0 = 0
};
}

/*!*********************************************************************************************************************
Class implementing the PVRShell functions.
***********************************************************************************************************************/
class OGLESParticleSystem : public pvr::Shell
{
private:
	pvr::assets::ModelHandle scene;
	bool isCameraPaused;
	pvr::uint8 currentBufferIdx;

	// View matrix
	glm::mat4 viewMtx, projMtx, viewProjMtx;
	glm::mat3 viewIT;
	glm::mat4 mLightView, mBiasMatrix;
	glm::vec3 lightPos;
	struct DrawPass
	{
		glm::mat4 model;
		glm::mat4 modelView;
		glm::mat4 modelViewProj;
		glm::mat3 modelViewIT;
		glm::vec3 lightPos;
	};
	std::vector<DrawPass> passSphere;
	struct ApiObjects
	{
		// UIRenderer class used to display text
		pvr::ui::UIRenderer uiRenderer;

		pvr::api::TextureView  particleTex;
		pvr::api::Buffer sphereVbo;
		pvr::api::Buffer sphereIbo;

		pvr::api::Buffer floorVbo;
		pvr::api::Buffer particleVbos[NumBuffers];

		pvr::api::CommandBuffer commandBuffers[NumBuffers];
		pvr::GraphicsContext context;
		pvr::api::Fbo onscreenFbo;

		struct
		{
			pvr::api::GraphicsPipeline pipe;
			pvr::uint32 iPositionArrayLoc;
			pvr::uint32 iLifespanArrayLoc;
			pvr::uint32 mvpMatrixLoc;
		} pipeParticle;

		struct
		{
			pvr::api::GraphicsPipeline pipe;
			pvr::uint32 mvMatrixLoc;
			pvr::uint32 mvITMatrixLoc;
			pvr::uint32 mvpMatrixLoc;
			pvr::uint32 lightPosition;
		}
		pipelineSimple;

		struct
		{
			pvr::api::GraphicsPipeline pipe;
			pvr::int32 mvMatrixLoc;
			pvr::int32 mvITMatrixLoc;
			pvr::int32 mvpMatrixLoc;
			pvr::int32 lightPosition;
		} pipelineFloor;

		ParticleSystemGPU particleSystemGPU;
		ApiObjects(OGLESParticleSystem& thisApp) : particleSystemGPU(thisApp) { }
	};
	std::auto_ptr<ApiObjects> apiObj;
public:
	OGLESParticleSystem();

	virtual pvr::Result::Enum initApplication();
	virtual pvr::Result::Enum initView();
	virtual pvr::Result::Enum releaseView();
	virtual pvr::Result::Enum quitApplication();
	virtual pvr::Result::Enum renderFrame();
	virtual void eventMappedInput(pvr::SimplifiedInput::Enum key);

	bool createBuffers();
	bool createPipelines();
	void recordCommandBuffers();
	void recordCommandBuffer(pvr::uint8 idx);
	void recordCmdDrawFloor(pvr::uint8 idx);
	void recordCmdDrawParticles(pvr::uint8 idx);
	void recordCmdDrawSphere(DrawPass& passSphere, pvr::uint8 idx);
	void respecifyParticleBuffer(pvr::uint32 numberOfParticles);

	void updateFloor();
	void updateSpheres(const glm::mat4& proj, const glm::mat4& view);
	void renderParticles(const glm::mat4& proj, const glm::mat4& view);

	void updateParticleUniforms();
	bool SetCollisionSpheres(const Sphere* pSpheres, unsigned int uiNumSpheres);
};

/*!*********************************************************************************************************************
\brief	Handles user input and updates live variables accordingly.
\param key Input key to handle
***********************************************************************************************************************/
void OGLESParticleSystem::eventMappedInput(pvr::SimplifiedInput::Enum key)
{
	switch (key)
	{
	case pvr::SimplifiedInput::Left:
	{
		unsigned int numParticles = apiObj->particleSystemGPU.getNumberOfParticles();
		if (numParticles / 2 >= Configuration::MinNoParticles)
		{
			respecifyParticleBuffer(numParticles / 2);
			apiObj->uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("No. of Particles: %d", numParticles / 2));
			apiObj->uiRenderer.getDefaultDescription()->commitUpdates();
			recordCommandBuffers();
		}
	} break;
	case pvr::SimplifiedInput::Right:
	{
		unsigned int numParticles = apiObj->particleSystemGPU.getNumberOfParticles();
		if (numParticles * 2 <= Configuration::MaxNoParticles)
		{
			respecifyParticleBuffer(numParticles * 2);
			apiObj->uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("No. of Particles: %d", numParticles * 2));
			apiObj->uiRenderer.getDefaultDescription()->commitUpdates();
			recordCommandBuffers();
		}
	} break;
	case pvr::SimplifiedInput::Action1: isCameraPaused = !isCameraPaused; break;
	case pvr::SimplifiedInput::ActionClose: exitShell(); break;
	}
}

/*!*********************************************************************************************************************
\brief Resize the particle buffer size
\param numberOfParticles Number of particles to allocate in the buffer
***********************************************************************************************************************/
void OGLESParticleSystem::respecifyParticleBuffer(pvr::uint32 numberOfParticles)
{
	//We do not need to update a descriptor set as VBOs are set directly in the Command Buffer
	//We DO need to notify the ParticleSystemGpu of our buffer re-specification as the SSBO view
	//of our buffer will also need to be respecified.
	for (pvr::uint32 i = 0; i < NumBuffers; ++i)
	{
		apiObj->particleVbos[i] = apiObj->context->createBuffer(sizeof(Particle) * numberOfParticles,
		                          BufferBindingUse::VertexBuffer | BufferBindingUse::StorageBuffer);
	}
	apiObj->particleSystemGPU.setParticleVboBuffers(apiObj->particleVbos);
	apiObj->particleSystemGPU.setNumberOfParticles(numberOfParticles);
}

/*!*********************************************************************************************************************
\brief ctor
***********************************************************************************************************************/
OGLESParticleSystem::OGLESParticleSystem() : isCameraPaused(0) { }

/*!*********************************************************************************************************************
\brief	Loads the mesh data required for this training course into vertex buffer objects
\return Return true on success
***********************************************************************************************************************/
bool OGLESParticleSystem::createBuffers()
{
	pvr::utils::createSingleBuffersFromMesh(getGraphicsContext(), scene->getMesh(0), apiObj->sphereVbo, apiObj->sphereIbo);

	//Initialize the vertex buffer data for the floor - 3*Position data, 3* normal data
	glm::vec2 maxCorner(40, 40);
	const float afVertexBufferData[] =
	{
		-maxCorner.x, 0.0f, -maxCorner.y, 0.0f, 1.0f, 0.0f,
		-maxCorner.x, 0.0f, maxCorner.y, 0.0f, 1.0f, 0.0f,
		maxCorner.x, 0.0f, -maxCorner.y, 0.0f, 1.0f, 0.0f,
		maxCorner.x, 0.0f, maxCorner.y, 0.0f, 1.0f, 0.0f
	};

	apiObj->floorVbo = apiObj->context->createBuffer(sizeof(afVertexBufferData), BufferBindingUse::VertexBuffer);
	apiObj->floorVbo->update(afVertexBufferData, 0, sizeof(afVertexBufferData));
	for (int i = 0; i < NumBuffers; ++i)
	{
		apiObj->particleVbos[i] = apiObj->context->createBuffer(sizeof(Particle) * Configuration::InitialNoParticles,
		                          BufferBindingUse::VertexBuffer | BufferBindingUse::StorageBuffer);
	}
	return true;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occured
\brief	Loads and compiles the shaders and links the shader programs required for this training course
***********************************************************************************************************************/
bool OGLESParticleSystem::createPipelines()
{
	const pvr::assets::Mesh& mesh = scene->getMesh(0);
	//No textures etc. for our rendering...
	pvr::api::PipelineLayout pipeLayout = apiObj->context->createPipelineLayout(pvr::api::PipelineLayoutCreateParam());
	pvr::assets::ShaderFile fileVersioning;

	// Simple Pipeline
	{
		pvr::api::VertexAttributeInfo attributes[] =
		{
			pvr::api::VertexAttributeInfo(Attributes::VertexArray, DataType::Float32, 3, 0, "inVertex"),
			pvr::api::VertexAttributeInfo(Attributes::NormalArray, DataType::Float32, 3, 3 * sizeof(float), "inNormal")
		};
		const char* simplePipeAttributes[] = { "inVertex", "inNormal" };
		const unsigned int numSimpleAttribs = sizeof(simplePipeAttributes) / sizeof(simplePipeAttributes[0]);

		pvr::api::GraphicsPipelineCreateParam pipeCreateInfo;
		fileVersioning.populateValidVersions(Files::VertShaderSrcFile, *this);
		pipeCreateInfo.vertexShader.setShader(apiObj->context->createShader(*fileVersioning.getBestStreamForApi(pvr::Api::OpenGLES31), ShaderType::VertexShader));

		fileVersioning.populateValidVersions(Files::FragShaderSrcFile, *this);
		pipeCreateInfo.fragmentShader.setShader(apiObj->context->createShader(*fileVersioning.getBestStreamForApi(pvr::Api::OpenGLES31), ShaderType::FragmentShader));

		pipeCreateInfo.colorBlend.addAttachmentState(pvr::api::pipelineCreation::ColorBlendAttachmentState());

		pipeCreateInfo.vertexInput.addVertexAttribute(0, attributes[0]).addVertexAttribute(0, attributes[1])
		.setInputBinding(0, mesh.getStride(0));

		pipeCreateInfo.depthStencil.setDepthWrite(true).setDepthTestEnable(true);

		pipeCreateInfo.inputAssembler.setPrimitiveTopology(PrimitiveTopology::TriangleList);
		pipeCreateInfo.pipelineLayout = pipeLayout;
		apiObj->pipelineSimple.pipe = apiObj->context->createGraphicsPipeline(pipeCreateInfo);
		apiObj->pipelineSimple.mvMatrixLoc = apiObj->pipelineSimple.pipe->getUniformLocation("uModelViewMatrix");
		apiObj->pipelineSimple.mvITMatrixLoc = apiObj->pipelineSimple.pipe->getUniformLocation("uModelViewITMatrix");
		apiObj->pipelineSimple.mvpMatrixLoc = apiObj->pipelineSimple.pipe->getUniformLocation("uModelViewProjectionMatrix");
		apiObj->pipelineSimple.lightPosition = apiObj->pipelineSimple.pipe->getUniformLocation("uLightPosition");
	}

	//	Floor Pipeline
	{
		pvr::api::VertexAttributeInfo attributes[] =
		{
			pvr::api::VertexAttributeInfo(Attributes::VertexArray, DataType::Float32, 3, 0, "inVertex"),
			pvr::api::VertexAttributeInfo(Attributes::NormalArray, DataType::Float32, 3, 3 * sizeof(float), "inNormal")
		};

		const char* floorPipeAttributes[] = { "inVertex", "inNormal" };
		const unsigned int numFloorAttribs = sizeof(floorPipeAttributes) / sizeof(floorPipeAttributes[0]);
		pvr::api::GraphicsPipelineCreateParam pipeCreateInfo;
		pipeCreateInfo.colorBlend.addAttachmentState(pvr::api::pipelineCreation::ColorBlendAttachmentState());

		fileVersioning.populateValidVersions(Files::VertShaderSrcFile, *this);
		pipeCreateInfo.vertexShader.setShader(apiObj->context->createShader(*fileVersioning.getBestStreamForContext(apiObj->context), ShaderType::VertexShader));

		fileVersioning.populateValidVersions(Files::FragShaderSrcFile, *this);
		pipeCreateInfo.fragmentShader.setShader(apiObj->context->createShader(*fileVersioning.getBestStreamForContext(apiObj->context), ShaderType::FragmentShader));
		pipeCreateInfo.vertexInput.addVertexAttribute(0, attributes[0]).addVertexAttribute(0, attributes[1]).setInputBinding(0, 6 * sizeof(float));

		pipeCreateInfo.inputAssembler.setPrimitiveTopology(PrimitiveTopology::TriangleStrips);
		pipeCreateInfo.pipelineLayout = pipeLayout;
		apiObj->pipelineFloor.pipe = apiObj->context->createGraphicsPipeline(pipeCreateInfo);
		apiObj->pipelineFloor.mvMatrixLoc = apiObj->pipelineFloor.pipe->getUniformLocation("uModelViewMatrix");
		apiObj->pipelineFloor.mvITMatrixLoc = apiObj->pipelineFloor.pipe->getUniformLocation("uModelViewITMatrix");
		apiObj->pipelineFloor.mvpMatrixLoc = apiObj->pipelineFloor.pipe->getUniformLocation("uModelViewProjectionMatrix");
		apiObj->pipelineFloor.lightPosition = apiObj->pipelineFloor.pipe->getUniformLocation("uLightPosition");
	}

	//  Particle Pipeline
	{
		const char* particleAttribs[] = { "inPosition", "inLifespan" };
		pvr::api::VertexAttributeInfo attributes[] =
		{
			pvr::api::VertexAttributeInfo(Attributes::ParticlePoisitionArray, DataType::Float32, 3, 0, "inPosition"),
			pvr::api::VertexAttributeInfo(Attributes::ParticleLifespanArray, DataType::Float32, 1, (sizeof(float) * 7), "inLifespan")
		};
		const unsigned int numParticleAttribs = sizeof(particleAttribs) / sizeof(particleAttribs[0]);
		pvr::api::ImageDataFormat colorFmt;
		pvr::api::GraphicsPipelineCreateParam pipeCreateInfo;
		pipeCreateInfo.colorBlend.addAttachmentState(
		  pvr::api::pipelineCreation::ColorBlendAttachmentState(
		    true, BlendFactor::SrcAlpha, BlendFactor::One, BlendOp::Add));

		pipeCreateInfo.depthStencil.setDepthWrite(true).setDepthTestEnable(true);
		fileVersioning.populateValidVersions(Files::ParticleShaderVertSrcFile, *this);

		pipeCreateInfo.vertexShader.setShader(apiObj->context->createShader(*fileVersioning.getBestStreamForContext(apiObj->context), ShaderType::VertexShader));

		fileVersioning.populateValidVersions(Files::ParticleShaderFragSrcFile, *this);
		pipeCreateInfo.fragmentShader.setShader(apiObj->context->createShader(*fileVersioning.getBestStreamForContext(apiObj->context), ShaderType::FragmentShader));

		pipeCreateInfo.vertexInput.addVertexAttribute(0, attributes[0]).addVertexAttribute(0, attributes[1])
		.setInputBinding(0, sizeof(Particle));

		pipeCreateInfo.inputAssembler.setPrimitiveTopology(PrimitiveTopology::Points);
		pipeCreateInfo.pipelineLayout = pipeLayout;
		apiObj->pipeParticle.pipe = apiObj->context->createGraphicsPipeline(pipeCreateInfo);
		apiObj->pipeParticle.mvpMatrixLoc = apiObj->pipeParticle.pipe->getUniformLocation("uModelViewProjectionMatrix");
	}
	return true;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in initApplication() will be called by the Shell once per run, before the rendering context is created.
		Used to initialize variables that are not dependent on it  (e.g. external modules, loading meshes, etc.)
		If the rendering context is lost, InitApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result::Enum OGLESParticleSystem::initApplication()
{
	setSwapInterval(0);
	setDeviceQueueTypesRequired(pvr::DeviceQueueType::Compute);
	setMinApiType(pvr::Api::OpenGLES31);
	// Load the scene
	scene.construct();
	pvr::assets::PODReader(getAssetStream(Files::SphereModelFile)).readAsset(*scene);

	for (pvr::uint32 i = 0; i < scene->getNumMeshes(); ++i)
	{
		scene->getMesh(i).setVertexAttributeIndex("POSITION0", Attributes::VertexArray);
		scene->getMesh(i).setVertexAttributeIndex("NORMAL0", Attributes::NormalArray);
		scene->getMesh(i).setVertexAttributeIndex("UV0", Attributes::TexCoordArray);
	}
	srand((unsigned int)this->getTime());
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in quitApplication() will be called by the Shell once per run, just before exiting the program.
	    If the rendering context is lost, QuitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result::Enum OGLESParticleSystem::quitApplication() { return pvr::Result::Success; }

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in initView() will be called by the Shell upon initialization or after a change in the rendering context.
		Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result::Enum OGLESParticleSystem::initView()
{
	apiObj.reset(new ApiObjects(*this));
	apiObj->context = getGraphicsContext();
	for (pvr::uint8 i = 0; i < NumBuffers; ++i)	{	apiObj->commandBuffers[i] = apiObj->context->createCommandBufferOnDefaultPool();	}

	apiObj->onscreenFbo = apiObj->context->createOnScreenFbo(0);

	// Initialize Print3D textures
	if (apiObj->uiRenderer.init(apiObj->context, apiObj->onscreenFbo->getRenderPass(), 0))
	{
		setExitMessage("Could not initialize UIRenderer");
		return pvr::Result::UnknownError;
	}

	//	Create the Buffers
	if (!createBuffers()) {	return pvr::Result::UnknownError;	}

	//	Load and compile the shaders & link programs
	if (!createPipelines())	{	return pvr::Result::UnknownError;	}

	// Create view matrices
	mLightView = glm::lookAt(glm::vec3(0.0f, 80.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));

	// Creates the projection matrix.
	projMtx = glm::perspectiveFov(glm::pi<pvr::float32>() / 3.0f, (pvr::float32)getWidth(),
	                              (pvr::float32)getHeight(), Configuration::CameraNear, Configuration::CameraFar);

	// Create a bias matrix
	mBiasMatrix = glm::mat4(0.5f, 0.0f, 0.0f, 0.0f,
	                        0.0f, 0.5f, 0.0f, 0.0f,
	                        0.0f, 0.0f, 0.5f, 0.0f,
	                        0.5f, 0.5f, 0.5f, 1.0f);

	std::string errorStr;
	if (!apiObj->particleSystemGPU.init(errorStr))
	{
		pvr::Log(errorStr.c_str());
		return pvr::Result::UnknownError;
	}

	apiObj->particleSystemGPU.setGravity(glm::vec3(0.f, -9.81f, 0.f));
	apiObj->particleSystemGPU.setCollisionSpheres(Configuration::Spheres, Configuration::NumberOfSpheres);
	apiObj->particleSystemGPU.setParticleVboBuffers(apiObj->particleVbos);
	apiObj->particleSystemGPU.setNumberOfParticles(Configuration::InitialNoParticles);

	apiObj->uiRenderer.getDefaultTitle()->setText("OpenGL ES 3.1 Compute Particle System");
	apiObj->uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("No. of Particles: %d", Configuration::InitialNoParticles));
	apiObj->uiRenderer.getDefaultControls()->setText("Action1: Pause rotation\nLeft: Decrease particles\nRight: Increase particles");
	apiObj->uiRenderer.getDefaultTitle()->commitUpdates();
	apiObj->uiRenderer.getDefaultDescription()->commitUpdates();
	apiObj->uiRenderer.getDefaultControls()->commitUpdates();
	recordCommandBuffers();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in releaseView() will be called by pvr::Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result::Enum OGLESParticleSystem::releaseView()
{
	apiObj.reset();
	scene.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result::Enum OGLESParticleSystem::renderFrame()
{
	currentBufferIdx++;
	if (currentBufferIdx >= NumBuffers) { currentBufferIdx = 0;}

	updateParticleUniforms();

	if (!isCameraPaused)
	{
		static float angle = 0;
		angle += getFrameTime() / 5000.0f;
		glm::vec3 vFrom = glm::vec3(sinf(angle) * 50.0f, 30.0f, cosf(angle) * 50.0f);

		viewMtx = glm::lookAt(vFrom, glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		viewProjMtx = projMtx * viewMtx;
	}

	// Render floor
	updateFloor();
	updateSpheres(projMtx, viewMtx);

	// Render particles
	apiObj->commandBuffers[currentBufferIdx]->submit();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief	Updates the memory from where the command buffer will read the values to update the uniforms for the spheres
\param[in] proj projection matrix
\param[in] view view matrix
***********************************************************************************************************************/
void OGLESParticleSystem::updateSpheres(const glm::mat4& proj, const glm::mat4& view)
{
	for (int i = 0; i < Configuration::NumberOfSpheres; ++i)
	{
		const glm::vec3& position = Configuration::Spheres[i].vPosition;
		float radius = Configuration::Spheres[i].fRadius;
		DrawPass& pass = passSphere[i];

		const glm::mat4 mModel = glm::translate(position) * glm::scale(glm::vec3(radius, radius, radius));
		pass.modelView = view * mModel;
		pass.modelViewProj = proj * pass.modelView;
		pass.modelViewIT = glm::inverseTranspose(glm::mat3(pass.modelView));
		pass.lightPos = glm::vec3(view * glm::vec4(Configuration::LightPosition, 1.0f));
	}
}

/*!*********************************************************************************************************************
\brief	Updates the memory from where the commandbuffer will read the values to update the uniforms for the floor
***********************************************************************************************************************/
void OGLESParticleSystem::updateFloor()
{
	viewIT = glm::inverseTranspose(glm::mat3(viewMtx));
	lightPos = glm::vec3(viewMtx * glm::vec4(Configuration::LightPosition, 1.0f));
	viewProjMtx = projMtx * viewMtx;
}

/*!*********************************************************************************************************************
\brief	Updates particle positions and attributes, e.g. lifespan, position, velocity etc.
		Will update the buffer that was "just used" as the Input, as output, so that we can exploit more GPU parallelization.
************************************************************************************************************************/
void OGLESParticleSystem::updateParticleUniforms()
{
	float step = (float)getFrameTime();

	static pvr::float32 rot_angle = 0.0f;
	rot_angle += step / 500.0f;
	pvr::float32 el_angle = (sinf(rot_angle / 4.0f) + 1.0f) * 0.2f + 0.2f;

	glm::mat4 rot = glm::rotate(rot_angle, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 skew = glm::rotate(el_angle, glm::vec3(0.0f, 0.0f, 1.0f));

	Emitter sEmitter(rot * skew, 1.3f, 1.0f);

	apiObj->particleSystemGPU.setEmitter(sEmitter);
	apiObj->particleSystemGPU.updateUniforms(step);
}

/*!*********************************************************************************************************************
\brief	Pre record the rendering commands
***********************************************************************************************************************/
void OGLESParticleSystem::recordCommandBuffers()
{
	for (pvr::uint8 i = 0; i < NumBuffers; ++i) { recordCommandBuffer(i); }
}

/*!*********************************************************************************************************************
\brief	Record the commands buffer
\param	idx Commandbuffer index
***********************************************************************************************************************/
void OGLESParticleSystem::recordCommandBuffer(pvr::uint8 idx)
{
	apiObj->commandBuffers[idx]->beginRecording();
	apiObj->commandBuffers[idx]->beginRenderPass(apiObj->onscreenFbo, pvr::Rectanglei(0, 0, getWidth(), getHeight()), true);
	const char* err = 0;

	// Render floor
	recordCmdDrawFloor(idx);
	passSphere.resize(Configuration::NumberOfSpheres);
	for (pvr::uint32 i = 0; i < Configuration::NumberOfSpheres; i++) { recordCmdDrawSphere(passSphere[i], idx); }

	// Render particles
	recordCmdDrawParticles(idx);
	pvr::api::SecondaryCommandBuffer uicmd = apiObj->context->createSecondaryCommandBufferOnDefaultPool();
	apiObj->uiRenderer.beginRendering(uicmd);
	apiObj->uiRenderer.getDefaultTitle()->render();
	apiObj->uiRenderer.getDefaultDescription()->render();
	apiObj->uiRenderer.getDefaultControls()->render();
	apiObj->uiRenderer.getSdkLogo()->render();
	apiObj->uiRenderer.endRendering();
	apiObj->commandBuffers[idx]->enqueueSecondaryCmds(uicmd);
	apiObj->commandBuffers[idx]->endRenderPass();

	apiObj->particleSystemGPU.recordCommandBuffer(apiObj->commandBuffers[idx], idx);

	pvr::api::MemoryBarrierSet memBarrierSet;
	memBarrierSet.addBarrier(pvr::api::MemoryBarrier(pvr::types::AccessFlags::ShaderWrite, pvr::types::AccessFlags::VertexAttributeRead));

	apiObj->commandBuffers[idx]->pipelineBarrier(pvr::types::ShaderStageFlags::Compute, pvr::types::ShaderStageFlags::Vertex, memBarrierSet);
	apiObj->commandBuffers[idx]->endRecording();
}

/*!*********************************************************************************************************************
\brief	Record the draw particles commands
\param	idx Commandbuffer index
***********************************************************************************************************************/
void OGLESParticleSystem::recordCmdDrawParticles(pvr::uint8 idx)
{
	apiObj->commandBuffers[idx]->bindPipeline(apiObj->pipeParticle.pipe);
	apiObj->commandBuffers[idx]->bindVertexBuffer(apiObj->particleVbos[idx], 0, 0);
	apiObj->commandBuffers[idx]->setUniformPtr<glm::mat4>(apiObj->pipeParticle.mvpMatrixLoc, 1, &viewProjMtx);
	apiObj->commandBuffers[idx]->drawArrays(0, apiObj->particleSystemGPU.getNumberOfParticles(), 0, 1);
}

/*!*********************************************************************************************************************
\brief	Renders a sphere at the specified position.
\param[in] passSphere Sphere draw pass
\param[in] idx Commandbuffer index
***********************************************************************************************************************/
void OGLESParticleSystem::recordCmdDrawSphere(DrawPass& passSphere, pvr::uint8 idx)
{
	apiObj->commandBuffers[idx]->bindPipeline(apiObj->pipelineSimple.pipe);
	apiObj->commandBuffers[idx]->setUniformPtr<glm::mat4>(apiObj->pipelineSimple.mvpMatrixLoc, 1, &passSphere.modelViewProj);
	apiObj->commandBuffers[idx]->setUniformPtr<glm::mat4>(apiObj->pipelineSimple.mvMatrixLoc, 1, &passSphere.modelView);
	apiObj->commandBuffers[idx]->setUniformPtr<glm::mat3>(apiObj->pipelineSimple.mvITMatrixLoc, 1, &passSphere.modelViewIT);
	apiObj->commandBuffers[idx]->setUniformPtr<glm::vec3>(apiObj->pipelineSimple.lightPosition, 1, &passSphere.lightPos);

	static const pvr::assets::Mesh& mesh = scene->getMesh(0);
	apiObj->commandBuffers[idx]->bindVertexBuffer(apiObj->sphereVbo, 0, 0);
	apiObj->commandBuffers[idx]->bindIndexBuffer(apiObj->sphereIbo, 0, mesh.getFaces().getDataType());
	// Indexed Triangle list
	apiObj->commandBuffers[idx]->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
}

/*!*********************************************************************************************************************
\brief	Renders the floor as a quad.
\param idx Commandbuffer index
***********************************************************************************************************************/
void OGLESParticleSystem::recordCmdDrawFloor(pvr::uint8 idx)
{
	// Enables depth testing
	// We need to calculate the texture projection matrix. This matrix takes the pixels from world space to previously rendered light projection space
	// where we can look up values from our saved depth buffer. The matrix is constructed from the light view and projection matrices as used for the previous render and
	// then multiplied by the inverse of the current view matrix.
	apiObj->commandBuffers[idx]->bindPipeline(apiObj->pipelineFloor.pipe);

	apiObj->commandBuffers[idx]->setUniformPtr<glm::mat4>(apiObj->pipelineFloor.mvpMatrixLoc, 1, &viewProjMtx);
	apiObj->commandBuffers[idx]->setUniformPtr<glm::mat4>(apiObj->pipelineFloor.mvMatrixLoc, 1, &viewMtx);
	apiObj->commandBuffers[idx]->setUniformPtr<glm::mat3>(apiObj->pipelineFloor.mvITMatrixLoc, 1, &viewIT);
	apiObj->commandBuffers[idx]->setUniformPtr<glm::vec3>(apiObj->pipelineFloor.lightPosition, 1, &lightPos);

	apiObj->commandBuffers[idx]->bindVertexBuffer(apiObj->floorVbo, 0, 0);
	// Draw the quad
	apiObj->commandBuffers[idx]->drawArrays(0, 4);
}

/*!*********************************************************************************************************************
\return Return a smart pointer to the application class.
\brief	This function must be implemented by the user of the shell. It should return the Application class (a class inheriting from pvr::Shell.
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() {	return std::auto_ptr<pvr::Shell>(new OGLESParticleSystem());  }
