/*!*********************************************************************************************************************
\File         VulkanParticleSystem.cpp
\Title        ParticleSystem
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Particle animation system using Compute Shaders. Requires the PVRShell.
***********************************************************************************************************************/
#include "ParticleSystemGPU.h"
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVREngineUtils/PVREngineUtils.h"
using namespace pvr::types;
using namespace pvr;
namespace Files {
// Asset files
const char SphereModelFile[] = "sphere.pod";
const char FragShaderSrcFile[] = "FragShader_vk.fsh.spv";
const char VertShaderSrcFile[] = "VertShader_vk.vsh.spv";
const char FloorVertShaderSrcFile[] = "FloorVertShader_vk.vsh.spv";
const char ParticleShaderFragSrcFile[] = "ParticleFragShader_vk.fsh.spv";
const char ParticleShaderVertSrcFile[] = "ParticleVertShader_vk.vsh.spv";
}

namespace Configuration {
enum
{
	MinNoParticles = 128,
	MaxNoParticles = 65536 * 64,
	InitialNoParticles = 32768,
	NumberOfSpheres = 8,
};

const float CameraNear = .1f;
const float CameraFar = 1000.0f;
const glm::vec3 LightPosition(0.0f, 10.0f, 0.0f);

const Sphere Spheres[] =
{
	Sphere(glm::vec3(-20.0f, 6.0f, -20.0f), 5.f),
	Sphere(glm::vec3(-20.0f, 6.0f,   0.0f), 5.f),
	Sphere(glm::vec3(-20.0f, 6.0f,  20.0f), 5.f),
	Sphere(glm::vec3(0.0f, 6.0f, -20.0f), 5.f),
	Sphere(glm::vec3(0.0f, 6.0f,  20.0f), 5.f),
	Sphere(glm::vec3(20.0f, 6.0f, -20.0f), 5.f),
	Sphere(glm::vec3(20.0f, 6.0f,   0.0f), 5.f),
	Sphere(glm::vec3(20.0f, 6.0f,  20.0f), 5.f),
};

BufferViewMapping SpherePipeUboMapping[] =
{
	BufferViewMapping("uModelViewMatrix", pvr::types::GpuDatatypes::mat4x4),
	BufferViewMapping("uModelViewProjectionMatrix", pvr::types::GpuDatatypes::mat4x4),
	BufferViewMapping("uModelViewITMatrix", pvr::types::GpuDatatypes::mat3x3),
};

namespace SpherePipeDynamicUboElements {
enum Enum { ModelViewMatrix, ModelViewProjectionMatrix, ModelViewITMatrix, Count };
}

BufferViewMapping FloorPipeUboMapping[] =
{
	BufferViewMapping("uModelViewMatrix", pvr::types::GpuDatatypes::mat4x4),
	BufferViewMapping("uModelViewProjectionMatrix", pvr::types::GpuDatatypes::mat4x4),
	BufferViewMapping("uModelViewITMatrix", pvr::types::GpuDatatypes::mat3x3),
	BufferViewMapping("uLightPos", pvr::types::GpuDatatypes::vec3),
};

namespace FloorPipeDynamicUboElements {
enum Enum { ModelViewMatrix, ModelViewProjectionMatrix, ModelViewITMatrix, LightPos, Count };
}
}

// Index to bind the attributes to vertex shaders

namespace Attributes {
enum Enum
{
	ParticlePositionArray = 0, ParticleLifespanArray = 1,
	VertexArray = 0, NormalArray = 1, TexCoordArray = 2,
	BindingIndex0 = 0
};
}

/*!*********************************************************************************************************************
Class implementing the PVRShell functions.
***********************************************************************************************************************/
class VulkanParticleSystem : public pvr::Shell
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
	struct PassSphere
	{
		utils::StructuredMemoryView uboPerModel;// per swapchain
		utils::StructuredMemoryView uboLightProp;// per swapchain
		api::DescriptorSet	descriptoruboPerModel[MaxSwapChains];// per swapchains
		api::DescriptorSet  descriptorLighProp[MaxSwapChains];
		api::GraphicsPipeline pipeline;
		pvr::api::Buffer vbo;
		pvr::api::Buffer ibo;
	};

	struct PassParticles
	{
		utils::StructuredMemoryView uboMvp;// per swapchain
		api::DescriptorSet	descriptorMvp[MaxSwapChains];// per swapchains
		api::GraphicsPipeline pipeline;

	};

	struct PassFloor
	{
		utils::StructuredMemoryView uboPerModel;// per swapchain
		api::DescriptorSet	descriptorUbo[MaxSwapChains];// per swapchains
		api::GraphicsPipeline pipeline;
		pvr::api::Buffer vbo;
	};


	struct ApiObjects
	{
		// UIRenderer class used to display text
		pvr::ui::UIRenderer uiRenderer;

		pvr::api::TextureView  particleTex;
		pvr::api::CommandBuffer commandBuffers[MaxSwapChains];
		pvr::GraphicsContext context;
		api::FboSet onscreenFbo;

		PassSphere	passSphere;
		PassParticles passParticles;
		PassFloor passFloor;
		api::DescriptorSetLayout descLayoutuboPerModel;
		api::DescriptorSetLayout descLayoutUbo;

		ParticleSystemGPU particleSystemGPU;
		ApiObjects(VulkanParticleSystem& thisApp) : particleSystemGPU(thisApp) { }
	};
	std::auto_ptr<ApiObjects> apiObj;
public:
	VulkanParticleSystem();

	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();
	virtual void eventMappedInput(pvr::SimplifiedInput key);

	bool createBuffers();
	bool createPipelines();
	void recordCommandBuffers();
	void recordCommandBuffer(pvr::uint8 idx);
	void recordCmdDrawFloor(pvr::uint8 idx);
	void recordCmdDrawParticles(pvr::uint8 idx);
	void recordCmdDrawSphere(pvr::uint32 sphereId, pvr::uint8 idx);
	bool createDescriptors();
	void updateFloor();
	void updateSpheres();

	void updateParticleUniforms();
};

/*!*********************************************************************************************************************
\brief	Handles user input and updates live variables accordingly.
\param key Input key to handle
***********************************************************************************************************************/
void VulkanParticleSystem::eventMappedInput(pvr::SimplifiedInput key)
{
	switch (key)
	{
	case pvr::SimplifiedInput::Left:
	{
		unsigned int numParticles = apiObj->particleSystemGPU.getNumberOfParticles();
		if (numParticles / 2 >= Configuration::MinNoParticles)
		{
			apiObj->context->waitIdle();
			apiObj->particleSystemGPU.setNumberOfParticles(numParticles / 2);
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
			apiObj->context->waitIdle();
			apiObj->particleSystemGPU.setNumberOfParticles(numParticles * 2);
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
\brief ctor
***********************************************************************************************************************/
VulkanParticleSystem::VulkanParticleSystem() : isCameraPaused(0) { }

/*!*********************************************************************************************************************
\brief	Loads the mesh data required for this training course into vertex buffer objects
\return Return true on success
***********************************************************************************************************************/
bool VulkanParticleSystem::createBuffers()
{
	// create the spheres vertex and index buffers
	pvr::utils::createSingleBuffersFromMesh(getGraphicsContext(), scene->getMesh(0), apiObj->passSphere.vbo, apiObj->passSphere.ibo);

	//Initialize the vertex buffer data for the floor - 3*Position data, 3* normal data
	glm::vec2 maxCorner(40, 40);
	const float afVertexBufferData[] =
	{
		-maxCorner.x, 0.0f, -maxCorner.y, 0.0f, 1.0f, 0.0f,
		-maxCorner.x, 0.0f, maxCorner.y, 0.0f, 1.0f, 0.0f,
		maxCorner.x, 0.0f, -maxCorner.y, 0.0f, 1.0f, 0.0f,
		maxCorner.x, 0.0f, maxCorner.y, 0.0f, 1.0f, 0.0f
	};

	apiObj->passFloor.vbo = apiObj->context->createBuffer(sizeof(afVertexBufferData), BufferBindingUse::VertexBuffer, true);
	apiObj->passFloor.vbo->update(afVertexBufferData, 0, sizeof(afVertexBufferData));
	return true;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occured
\brief	Loads and compiles the shaders and links the shader programs required for this training course
***********************************************************************************************************************/
bool VulkanParticleSystem::createPipelines()
{

	const pvr::assets::Mesh& mesh = scene->getMesh(0);
	pvr::api::Shader fragShader = apiObj->context->createShader(*getAssetStream(Files::FragShaderSrcFile),
	                              ShaderType::FragmentShader);
	// Sphere Pipeline
	{
		pvr::api::Shader vertShader = apiObj->context->createShader(*getAssetStream(Files::VertShaderSrcFile),
		                              ShaderType::VertexShader);
		pvr::utils::VertexBindings attributes[] =
		{
			{ "POSITION", 0 }, { "NORMAL", 1}
		};
		const pvr::uint32 numSimpleAttribs = sizeof(attributes) / sizeof(attributes[0]);
		pvr::api::GraphicsPipelineCreateParam pipeCreateInfo;
		pipeCreateInfo.vertexShader.setShader(vertShader);
		pipeCreateInfo.fragmentShader.setShader(fragShader);

		pipeCreateInfo.colorBlend.setAttachmentState(0, pvr::types::BlendingConfig());
		pipeCreateInfo.depthStencil.setDepthWrite(true).setDepthTestEnable(true);
		pipeCreateInfo.renderPass = apiObj->onscreenFbo[0]->getRenderPass();
		pipeCreateInfo.inputAssembler.setPrimitiveTopology(PrimitiveTopology::TriangleList);

		pvr::utils::createInputAssemblyFromMesh(scene->getMesh(0), attributes, sizeof(attributes) /
		                                        sizeof(attributes[0]), pipeCreateInfo);

		pipeCreateInfo.pipelineLayout = apiObj->context->createPipelineLayout(pvr::api::PipelineLayoutCreateParam()
		                                .addDescSetLayout(apiObj->descLayoutuboPerModel)
		                                .addDescSetLayout(apiObj->descLayoutUbo));

		apiObj->passSphere.pipeline = apiObj->context->createGraphicsPipeline(pipeCreateInfo);
		if (!apiObj->passSphere.pipeline.isValid())
		{
			setExitMessage("Failed to create Sphere pipeline");
			return false;
		}
	}

	//	Floor Pipeline
	{
		pvr::api::Shader vertShader = apiObj->context->createShader(*getAssetStream(Files::FloorVertShaderSrcFile),
		                              ShaderType::VertexShader);
		pvr::api::VertexAttributeInfo attributes[] =
		{
			pvr::api::VertexAttributeInfo(0, DataType::Float32, 3, 0, "inPosition"),
			pvr::api::VertexAttributeInfo(1, DataType::Float32, 3, sizeof(pvr::float32) * 3, "inNormal")
		};
		pvr::api::GraphicsPipelineCreateParam pipeCreateInfo;
		pipeCreateInfo.vertexShader.setShader(vertShader);
		pipeCreateInfo.fragmentShader.setShader(fragShader);

		pipeCreateInfo.colorBlend.setAttachmentState(0, pvr::types::BlendingConfig());
		pipeCreateInfo.depthStencil.setDepthWrite(true).setDepthTestEnable(true);
		pipeCreateInfo.renderPass = apiObj->onscreenFbo[0]->getRenderPass();
		pipeCreateInfo.inputAssembler.setPrimitiveTopology(PrimitiveTopology::TriangleStrip);
		pipeCreateInfo.vertexInput
		.addVertexAttributes(0, attributes, sizeof(attributes) / sizeof(attributes[0]))
		.setInputBinding(0, sizeof(pvr::float32) * 6);

		pipeCreateInfo.pipelineLayout = apiObj->context->createPipelineLayout(pvr::api::PipelineLayoutCreateParam()
		                                .addDescSetLayout(apiObj->descLayoutUbo));

		apiObj->passFloor.pipeline = apiObj->context->createGraphicsPipeline(pipeCreateInfo);
		if (!apiObj->passFloor.pipeline.isValid())
		{
			setExitMessage("Failed to create Floor pipeline");
			return false;
		}
	}

	//  Particle Pipeline
	{
		pvr::api::VertexAttributeInfo attributes[] =
		{
			pvr::api::VertexAttributeInfo(Attributes::ParticlePositionArray, DataType::Float32, 3, 0, "inPosition"),
			pvr::api::VertexAttributeInfo(Attributes::ParticleLifespanArray, DataType::Float32, 1,
			sizeof(glm::vec3) * 2 + sizeof(float32), "inLifespan")
		};
		api::GraphicsPipelineCreateParam pipeCreateInfo;
		pipeCreateInfo.colorBlend.setAttachmentState(0,  pvr::types::BlendingConfig(
		      true, BlendFactor::SrcAlpha, BlendFactor::One, BlendOp::Add));

		pipeCreateInfo.depthStencil.setDepthWrite(true).setDepthTestEnable(true);

		pipeCreateInfo.vertexShader.setShader(apiObj->context->createShader(*getAssetStream(
		                                        Files::ParticleShaderVertSrcFile), ShaderType::VertexShader));

		pipeCreateInfo.fragmentShader.setShader(apiObj->context->createShader(*getAssetStream(
		    Files::ParticleShaderFragSrcFile), ShaderType::FragmentShader));

		pipeCreateInfo.renderPass = apiObj->onscreenFbo[0]->getRenderPass();
		pipeCreateInfo.vertexInput.addVertexAttribute(0, attributes[0]).addVertexAttribute(0, attributes[1])
		.setInputBinding(0, sizeof(Particle));

		pipeCreateInfo.inputAssembler.setPrimitiveTopology(PrimitiveTopology::PointList);
		pipeCreateInfo.pipelineLayout = apiObj->context->createPipelineLayout(pvr::api::PipelineLayoutCreateParam()
		                                .addDescSetLayout(apiObj->descLayoutUbo));
		apiObj->passParticles.pipeline = apiObj->context->createGraphicsPipeline(pipeCreateInfo);

		if (!apiObj->passParticles.pipeline.isValid())
		{
			setExitMessage("Failed to create Particle pipeline");
			return false;
		}
	}
	return true;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in initApplication() will be called by the Shell once per run, before the rendering context is created.
		Used to initialize variables that are not dependent on it  (e.g. external modules, loading meshes, etc.)
		If the rendering context is lost, InitApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result VulkanParticleSystem::initApplication()
{
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

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in quitApplication() will be called by the Shell once per run, just before exiting the program.
	    If the rendering context is lost, QuitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result VulkanParticleSystem::quitApplication() { return pvr::Result::Success; }


bool VulkanParticleSystem::createDescriptors()
{
	pvr::api::DescriptorSetLayoutCreateParam descLayoutInfo;
	// create dynamic ubo descriptor set layout
	{
		descLayoutInfo.setBinding(0, DescriptorType::UniformBufferDynamic, 1, ShaderStageFlags::Vertex);
		apiObj->descLayoutuboPerModel = apiObj->context->createDescriptorSetLayout(descLayoutInfo);
	}
	// create static ubo descriptor set layout
	{
		descLayoutInfo.setBinding(0, DescriptorType::UniformBuffer, 1, ShaderStageFlags::Vertex);
		apiObj->descLayoutUbo = apiObj->context->createDescriptorSetLayout(descLayoutInfo);
	}

	apiObj->passSphere.uboPerModel.addEntriesPacked(Configuration::SpherePipeUboMapping,
	    Configuration::SpherePipeDynamicUboElements::Count);
	apiObj->passSphere.uboPerModel.finalize(apiObj->context, Configuration::NumberOfSpheres,
	                                        BufferBindingUse::UniformBuffer, true, false);

	apiObj->passFloor.uboPerModel.addEntriesPacked(Configuration::FloorPipeUboMapping,
	    Configuration::FloorPipeDynamicUboElements::Count);
	apiObj->passFloor.uboPerModel.finalize(apiObj->context, 1, BufferBindingUse::UniformBuffer, false, false);

	apiObj->passSphere.uboLightProp.addEntryPacked(pvr::StringHash("uLightPosition"), GpuDatatypes::vec3);
	apiObj->passSphere.uboLightProp.finalize(apiObj->context, 1, BufferBindingUse::UniformBuffer, false, false);

	apiObj->passParticles.uboMvp.addEntryPacked("uModelViewProjectionMatrix", GpuDatatypes::mat4x4);
	apiObj->passParticles.uboMvp.finalize(apiObj->context, 1, BufferBindingUse::UniformBuffer, false, false);

	for (pvr::uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		// sphere descriptors
		{
			// create the ubo dynamic buffer
			apiObj->passSphere.uboPerModel.connectWithBuffer(i, apiObj->context->createBufferView(
			      apiObj->context->createBuffer(apiObj->passSphere.uboPerModel.getAlignedTotalSize(),
			                                    types::BufferBindingUse::UniformBuffer, true), 0, apiObj->passSphere.uboPerModel.getAlignedElementSize()));

			// create the ubo dynamic descriptor set
			apiObj->passSphere.descriptoruboPerModel[i] =
			  apiObj->context->createDescriptorSetOnDefaultPool(apiObj->descLayoutuboPerModel);
			apiObj->passSphere.descriptoruboPerModel[i]->update(api::DescriptorSetUpdate()
			    .setDynamicUbo(0, apiObj->passSphere.uboPerModel.getConnectedBuffer(i)));

			apiObj->passSphere.uboLightProp.connectWithBuffer(i,
			    apiObj->context->createBufferView(apiObj->context->createBuffer(
			                                        apiObj->passSphere.uboLightProp.getAlignedTotalSize(), BufferBindingUse::UniformBuffer, true), 0,
			                                      apiObj->passSphere.uboLightProp.getAlignedElementSize()));

			// create the ubo static descriptor set
			apiObj->passSphere.descriptorLighProp[i] =
			  apiObj->context->createDescriptorSetOnDefaultPool(apiObj->descLayoutUbo);
			apiObj->passSphere.descriptorLighProp[i]->update(api::DescriptorSetUpdate().setUbo(0,
			    apiObj->passSphere.uboLightProp.getConnectedBuffer(i)));
		}

		// particle descriptor
		{
			apiObj->passParticles.uboMvp.connectWithBuffer(i,
			    apiObj->context->createBufferAndView(apiObj->passParticles.uboMvp.getAlignedElementSize(),
			        BufferBindingUse::UniformBuffer, true));

			apiObj->passParticles.descriptorMvp[i] =
			  apiObj->context->createDescriptorSetOnDefaultPool(apiObj->descLayoutUbo);
			apiObj->passParticles.descriptorMvp[i]->update(api::DescriptorSetUpdate().setUbo(0,
			    apiObj->passParticles.uboMvp.getConnectedBuffer(i)));
		}

		// floor descriptors
		{
			// create the ubo dynamic buffer
			api::Buffer buffer =
			  apiObj->context->createBuffer(apiObj->passFloor.uboPerModel.getAlignedElementSize(),
			                                types::BufferBindingUse::UniformBuffer, true);

			apiObj->passFloor.uboPerModel.connectWithBuffer(i, apiObj->context->createBufferView(
			      buffer, 0, apiObj->passFloor.uboPerModel.getAlignedElementSize()));

			// create the ubo dynamic descriptor set
			apiObj->passFloor.descriptorUbo[i] = apiObj->context->createDescriptorSetOnDefaultPool(apiObj->descLayoutUbo);
			apiObj->passFloor.descriptorUbo[i]->update(api::DescriptorSetUpdate()
			    .setUbo(0, apiObj->passFloor.uboPerModel.getConnectedBuffer(i)));
		}
	}
	return true;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Code in initView() will be called by the Shell upon initialization or after a change in the rendering context.
		Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result VulkanParticleSystem::initView()
{
	srand((pvr::uint32)this->getTime());

	apiObj.reset(new ApiObjects(*this));
	apiObj->context = getGraphicsContext();
	for (pvr::uint8 i = 0; i < getSwapChainLength(); ++i)
	{
		apiObj->commandBuffers[i] = apiObj->context->createCommandBufferOnDefaultPool();
	}

	apiObj->onscreenFbo = apiObj->context->createOnScreenFboSet();

	// Initialize Print3D textures
	if (apiObj->uiRenderer.init(apiObj->onscreenFbo[0]->getRenderPass(), 0) != Result::Success)
	{
		setExitMessage("Could not initialize UIRenderer");
		return pvr::Result::UnknownError;
	}

	std::string errorStr;
	if (!apiObj->particleSystemGPU.init(Configuration::MaxNoParticles, Configuration::Spheres, Configuration::NumberOfSpheres, errorStr))
	{
		setExitMessage(errorStr.c_str());
		return pvr::Result::UnknownError;
	}

	//	Create the Buffers
	if (!createBuffers()) {	return pvr::Result::UnknownError;	}

	if (!createDescriptors()) { return Result::UnknownError; }

	//	Load and compile the shaders & link programs
	if (!createPipelines())	{	return pvr::Result::UnknownError;	}

	// Create view matrices
	mLightView = glm::lookAt(glm::vec3(0.0f, 80.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));

	// Creates the projection matrix.
	projMtx = math::perspectiveFov(getApiType(), glm::pi<pvr::float32>() / 3.0f, (pvr::float32)getWidth(),
	                               (pvr::float32)getHeight(), Configuration::CameraNear, Configuration::CameraFar);

	// Create a bias matrix
	mBiasMatrix = glm::mat4(0.5f, 0.0f, 0.0f, 0.0f,
	                        0.0f, 0.5f, 0.0f, 0.0f,
	                        0.0f, 0.0f, 0.5f, 0.0f,
	                        0.5f, 0.5f, 0.5f, 1.0f);

	apiObj->particleSystemGPU.setGravity(glm::vec3(0.f, -9.81f, 0.f));
	apiObj->particleSystemGPU.setNumberOfParticles(Configuration::InitialNoParticles);

	apiObj->uiRenderer.getDefaultTitle()->setText("Vulkan Compute Particle System");
	apiObj->uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("No. of Particles: %d",
	    Configuration::InitialNoParticles));
	apiObj->uiRenderer.getDefaultControls()->setText("Action1: Pause rotation\nLeft: Decrease particles\n"
	    "Right: Increase particles");
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
pvr::Result VulkanParticleSystem::releaseView()
{
	apiObj.reset();
	scene.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Return pvr::Result::Success if no error occurred
\brief	Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result VulkanParticleSystem::renderFrame()
{
	if (!isCameraPaused)
	{
		static float angle = 0;
		angle += getFrameTime() / 5000.0f;
		glm::vec3 vFrom = glm::vec3(sinf(angle) * 50.0f, 30.0f, cosf(angle) * 50.0f);

		viewMtx = glm::lookAt(vFrom, glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		viewIT = glm::inverseTranspose(glm::mat3(viewMtx));
		lightPos = glm::vec3(viewMtx * glm::vec4(Configuration::LightPosition, 1.0f));
		viewProjMtx = projMtx * viewMtx;
	}

	// Render floor
	updateParticleUniforms();
	updateFloor();
	updateSpheres();
	// render compute
	apiObj->particleSystemGPU.renderFrame(getSwapChainIndex());
	apiObj->commandBuffers[getSwapChainIndex()]->submitEndOfFrame(
	  apiObj->particleSystemGPU.getWaitSemaphore(getSwapChainIndex()));
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief	Updates the memory from where the command buffer will read the values to update the uniforms for the spheres
\param[in] proj projection matrix
\param[in] view view matrix
***********************************************************************************************************************/
void VulkanParticleSystem::updateSpheres()
{
	utils::StructuredMemoryView& bufferView = apiObj->passSphere.uboPerModel;
	bufferView.mapMultipleArrayElements(getSwapChainIndex(), 0, Configuration::NumberOfSpheres, MapBufferFlags::Write);
	glm::mat4 modelView;
	for (uint32 i = 0; i < Configuration::NumberOfSpheres; ++i)
	{
		const glm::vec3& position = Configuration::Spheres[i].vPosition;
		float32 radius = Configuration::Spheres[i].fRadius;
		modelView = viewMtx * glm::translate(position) * glm::scale(glm::vec3(radius));
		bufferView.setArrayValue(Configuration::SpherePipeDynamicUboElements::ModelViewMatrix, i, modelView);
		bufferView.setArrayValue(Configuration::SpherePipeDynamicUboElements::ModelViewProjectionMatrix, i,
		                         projMtx * modelView);
		bufferView.setArrayValue(Configuration::SpherePipeDynamicUboElements::ModelViewITMatrix, i,
		                         glm::mat3x4(glm::inverseTranspose(glm::mat3(modelView))));
	}
	bufferView.unmap(getSwapChainIndex());

	apiObj->passSphere.uboLightProp.map(getSwapChainIndex(), MapBufferFlags::Write);
	apiObj->passSphere.uboLightProp.setValue(0, lightPos);
	apiObj->passSphere.uboLightProp.unmap(getSwapChainIndex());
}

/*!*********************************************************************************************************************
\brief	Updates the memory from where the commandbuffer will read the values to update the uniforms for the floor
***********************************************************************************************************************/
void VulkanParticleSystem::updateFloor()
{
	pvr::utils::StructuredMemoryView& uboView = apiObj->passFloor.uboPerModel;
	uboView.map(getSwapChainIndex(), MapBufferFlags::Write);
	uboView.setValue(Configuration::FloorPipeDynamicUboElements::ModelViewMatrix, viewMtx)
	.setValue(Configuration::FloorPipeDynamicUboElements::ModelViewProjectionMatrix, viewProjMtx)
	.setValue(Configuration::FloorPipeDynamicUboElements::ModelViewITMatrix, viewIT)
	.setValue(Configuration::FloorPipeDynamicUboElements::LightPos, lightPos);
	uboView.unmap(getSwapChainIndex());
}

/*!*********************************************************************************************************************
\brief	Updates particle positions and attributes, e.g. lifespan, position, velocity etc.
		Will update the buffer that was "just used" as the Input, as output, so that we can exploit more GPU parallelization.
************************************************************************************************************************/
void VulkanParticleSystem::updateParticleUniforms()
{
	float step = (float)getFrameTime();

	static pvr::float32 rot_angle = 0.0f;
	rot_angle += step / 500.0f;
	pvr::float32 el_angle = (sinf(rot_angle / 4.0f) + 1.0f) * 0.2f + 0.2f;

	glm::mat4 rot = glm::rotate(rot_angle, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 skew = glm::rotate(el_angle, glm::vec3(0.0f, 0.0f, 1.0f));

	Emitter sEmitter(rot * skew, 1.3f, 1.0f);

	apiObj->particleSystemGPU.setEmitter(sEmitter);
	apiObj->particleSystemGPU.updateUniforms(getSwapChainIndex(), step);

	apiObj->passParticles.uboMvp.map(getSwapChainIndex(), MapBufferFlags::Write);
	apiObj->passParticles.uboMvp.setValue(0, viewProjMtx);
	apiObj->passParticles.uboMvp.unmap(getSwapChainIndex());
}

/*!*********************************************************************************************************************
\brief	Pre record the rendering commands
***********************************************************************************************************************/
void VulkanParticleSystem::recordCommandBuffers()
{
	for (pvr::uint8 i = 0; i < getSwapChainLength(); ++i) { recordCommandBuffer(i); }
}

/*!*********************************************************************************************************************
\brief	Record the commands buffer
\param	idx Commandbuffer index
***********************************************************************************************************************/
void VulkanParticleSystem::recordCommandBuffer(pvr::uint8 swapchain)
{
	apiObj->commandBuffers[swapchain]->beginRecording();
	pvr::api::MemoryBarrierSet memBarrierSet;
	memBarrierSet.addBarrier(pvr::api::BufferRangeBarrier(pvr::types::AccessFlags::ShaderWrite,
	                         pvr::types::AccessFlags::VertexAttributeRead, apiObj->particleSystemGPU.getParticleBufferView(), 0,
	                         apiObj->particleSystemGPU.getParticleBufferView()->getSize()));
	apiObj->commandBuffers[swapchain]->pipelineBarrier(PipelineStageFlags::AllCommands,
	    PipelineStageFlags::TopOfPipeline, memBarrierSet, false);
	apiObj->commandBuffers[swapchain]->beginRenderPass(apiObj->onscreenFbo[swapchain],
	    pvr::Rectanglei(0, 0, getWidth(), getHeight()), true);

	// Render floor
	recordCmdDrawFloor(swapchain);
	// render the spheres
	apiObj->commandBuffers[swapchain]->bindPipeline(apiObj->passSphere.pipeline);
	apiObj->commandBuffers[swapchain]->bindDescriptorSet(apiObj->passSphere.pipeline->getPipelineLayout(), 1,
	    apiObj->passSphere.descriptorLighProp[0]);
	for (pvr::uint32 i = 0; i < Configuration::NumberOfSpheres; i++) { recordCmdDrawSphere(i, swapchain); }

	// Render particles
	recordCmdDrawParticles(swapchain);
	apiObj->uiRenderer.beginRendering(apiObj->commandBuffers[swapchain]);
	apiObj->uiRenderer.getDefaultTitle()->render();
	apiObj->uiRenderer.getDefaultDescription()->render();
	apiObj->uiRenderer.getDefaultControls()->render();
	apiObj->uiRenderer.getSdkLogo()->render();
	apiObj->uiRenderer.endRendering();
	apiObj->commandBuffers[swapchain]->endRenderPass();
	apiObj->commandBuffers[swapchain]->endRecording();
}

/*!*********************************************************************************************************************
\brief	Record the draw particles commands
\param	idx Commandbuffer index
***********************************************************************************************************************/
void VulkanParticleSystem::recordCmdDrawParticles(pvr::uint8 idx)
{
	apiObj->commandBuffers[idx]->bindPipeline(apiObj->passParticles.pipeline);
	apiObj->commandBuffers[idx]->bindDescriptorSet(apiObj->passParticles.pipeline->getPipelineLayout(), 0,
	    apiObj->passParticles.descriptorMvp[idx]);
	apiObj->commandBuffers[idx]->bindVertexBuffer(apiObj->particleSystemGPU.getParticleBufferView(), 0, 0);
	apiObj->commandBuffers[idx]->drawArrays(0, apiObj->particleSystemGPU.getNumberOfParticles(), 0, 1);
}

/*!*********************************************************************************************************************
\brief	Renders a sphere at the specified position.
\param[in] passSphere Sphere draw pass
\param[in] idx Commandbuffer index
***********************************************************************************************************************/
void VulkanParticleSystem::recordCmdDrawSphere(pvr::uint32 sphereId, pvr::uint8 swapchain)
{
	static const pvr::assets::Mesh& mesh = scene->getMesh(0);
	pvr::uint32 offset = apiObj->passSphere.uboPerModel.getAlignedElementArrayOffset(sphereId);
	apiObj->commandBuffers[swapchain]->bindDescriptorSet(apiObj->passSphere.pipeline->getPipelineLayout(), 0,
	    apiObj->passSphere.descriptoruboPerModel[swapchain], &offset, 1);

	apiObj->commandBuffers[swapchain]->bindVertexBuffer(apiObj->passSphere.vbo, 0, 0);
	apiObj->commandBuffers[swapchain]->bindIndexBuffer(apiObj->passSphere.ibo, 0, mesh.getFaces().getDataType());
	// Indexed Triangle list
	apiObj->commandBuffers[swapchain]->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
}

/*!*********************************************************************************************************************
\brief	Renders the floor as a quad.
\param idx Commandbuffer index
***********************************************************************************************************************/
void VulkanParticleSystem::recordCmdDrawFloor(pvr::uint8 swapchain)
{
	// Enables depth testing
	// We need to calculate the texture projection matrix. This matrix takes the pixels from world space to previously rendered light projection space
	// where we can look up values from our saved depth buffer. The matrix is constructed from the light view and projection matrices as used for the previous render and
	// then multiplied by the inverse of the current view matrix.
	apiObj->commandBuffers[swapchain]->bindPipeline(apiObj->passFloor.pipeline);
	apiObj->commandBuffers[swapchain]->bindDescriptorSet(apiObj->passFloor.pipeline->getPipelineLayout(), 0,
	    apiObj->passFloor.descriptorUbo[swapchain]);
	apiObj->commandBuffers[swapchain]->bindVertexBuffer(apiObj->passFloor.vbo, 0, 0);
	// Draw the quad
	apiObj->commandBuffers[swapchain]->drawArrays(0, 4);
}

/*!*********************************************************************************************************************
\return Return a smart pointer to the application class.
\brief	This function must be implemented by the user of the shell. It should return the Application class (a class inheriting from pvr::Shell.
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() {	return std::auto_ptr<pvr::Shell>(new VulkanParticleSystem());  }
