/*!*********************************************************************************************************************
\File         OGLESPVRScopeExample.cpp
\Title        PVRScopeExample
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief  Shows how to use our example PVRScope graph code.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVREngineUtils/PVREngineUtils.h"
#include "PVRScopeGraph.h"
using namespace pvr;
#if !defined(_WIN32) || defined(__WINSCW__)
#define _stricmp strcasecmp
#endif

// Shader Source
const char FragShaderSrcFile[] = "FragShader.fsh";
const char VertShaderSrcFile[] = "VertShader.vsh";

// PVR texture files
const char TextureFile[] = "Marble.pvr";

// POD scene files
const char SceneFile[] = "scene.pod";

/*!*********************************************************************************************************************
\brief Class implementing the pvr::Shell functions.
***********************************************************************************************************************/
class OGLESPVRScopeExample : public pvr::Shell
{
	// Print3D class used to display text
	pvr::ui::UIRenderer uiRenderer;

	struct ApiObjects
	{
		pvr::api::CommandBuffer commandBuffer;
		pvr::api::SecondaryCommandBuffer secCmd;
		pvr::api::GraphicsPipeline pipeline;
		pvr::api::TextureView texture;
		std::vector<pvr::api::Buffer> ibos;
		std::vector<pvr::api::Buffer> vbos;
		pvr::api::DescriptorSet descriptorSet;
		pvr::api::DescriptorSetLayout descriptorSetLayout;
		pvr::api::Fbo onScreenFbo;
		PVRScopeGraph scopeGraph;
		pvr::GraphicsContext context;
	};
<<<<<<< HEAD
	std::auto_ptr<DeviceResources> apiObj;
=======
	std::auto_ptr<ApiObjects> apiObj;
>>>>>>> 1776432f... 4.3

	// 3D Model
	pvr::assets::ModelHandle scene;
	pvr::utils::AssetStore assetStore;
	// Projection and view matrices

	// Group shader programs and their uniform locations together
	struct
	{
		pvr::int32 mvpMtx;
		pvr::int32 mvITMtx;
		pvr::int32 lightDirView;
		pvr::int32 albedo;
		pvr::int32 specularExponent;
		pvr::int32 metallicity;
		pvr::int32 reflectivity;
	} uniformLocations;

	struct Uniforms
	{
		glm::mat4 projectionMtx;
		glm::mat4 viewMtx;
		glm::mat4 mvpMatrix1;
		glm::mat4 mvpMatrix2;
		glm::mat4 mvMatrix1;
		glm::mat4 mvMatrix2;
		glm::mat3 mvITMatrix1;
		glm::mat3 mvITMatrix2;
		glm::vec3 lightDirView;
		pvr::float32 specularExponent;
		pvr::float32 metallicity;
		pvr::float32 reflectivity;
		glm::vec3    albedo;
	} progUniforms;

	// The translation and Rotate parameter of Model
	pvr::float32 angleY;

	// The PVRScopeGraph variable

	// Variables for the graphing code
	pvr::int32 selectedCounter;
	pvr::int32 interval;
public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void eventMappedInput(pvr::SimplifiedInput key);

	void updateDescription();
	void recordCommandBuffer();
	bool createTexSamplerDescriptorSet();
	bool createPipeline();
	void loadVbos();

	void drawMesh(int nodeIndex, api::SecondaryCommandBuffer& cmd);
};

/*!*********************************************************************************************************************
\brief Handle input key events
\param key key event to handle
************************************************************************************************************************/
void OGLESPVRScopeExample::eventMappedInput(pvr::SimplifiedInput key)
{
	// Keyboard input (cursor up/down to cycle through counters)
	switch (key)
	{
	case pvr::SimplifiedInput::Up:
	case pvr::SimplifiedInput::Right:
	{
		selectedCounter++;
		if (selectedCounter > (int)apiObj->scopeGraph.getCounterNum()) { selectedCounter = apiObj->scopeGraph.getCounterNum(); }
	} break;
	case pvr::SimplifiedInput::Down:
	case pvr::SimplifiedInput::Left:
	{
		selectedCounter--;
		if (selectedCounter < 0) { selectedCounter = 0; }
	} break;
	case pvr::SimplifiedInput::Action1:
	{
		apiObj->scopeGraph.showCounter(selectedCounter, !apiObj->scopeGraph.isCounterShown(selectedCounter));
	} break;
	// Keyboard input (cursor left/right to change active group)
	case pvr::SimplifiedInput::ActionClose: exitShell(); break;
	default: break;
	}
}

/*!*********************************************************************************************************************
\brief Loads the textures required for this training course
\return Return true if no error occurred
***********************************************************************************************************************/
bool OGLESPVRScopeExample::createTexSamplerDescriptorSet()
{
	if (!assetStore.getTextureWithCaching(getGraphicsContext(), TextureFile, &apiObj->texture, NULL))
	{
		pvr::Log("ERROR: Failed to load texture.");
		return false;
	}
	// create the bilinear sampler
	pvr::assets::SamplerCreateParam samplerDesc;
	samplerDesc.minificationFilter = pvr::types::SamplerFilter::Linear;
	samplerDesc.mipMappingFilter = pvr::types::SamplerFilter::Nearest;
	samplerDesc.magnificationFilter = pvr::types::SamplerFilter::Linear;
	pvr::api::Sampler bilinearSampler = apiObj->context->createSampler(samplerDesc);

	pvr::api::DescriptorSetLayoutCreateParam descSetLayoutInfo;

	descSetLayoutInfo.setBinding(0, pvr::types::DescriptorType::CombinedImageSampler, 1,
	                             pvr::types::ShaderStageFlags::Fragment);

	apiObj->descriptorSetLayout = apiObj->context->createDescriptorSetLayout(descSetLayoutInfo);

	pvr::api::DescriptorSetUpdate descriptorSetUpdate;
	descriptorSetUpdate.setCombinedImageSampler(0, apiObj->texture, bilinearSampler);
	apiObj->descriptorSet = apiObj->context->createDescriptorSetOnDefaultPool(apiObj->descriptorSetLayout);
	apiObj->descriptorSet->update(descriptorSetUpdate);
	return true;
}

/*!*********************************************************************************************************************
\brief	Create a graphics pipeline required for this training course
\return	Return true if no error occurred
***********************************************************************************************************************/
bool OGLESPVRScopeExample::createPipeline()
{
	pvr::utils::VertexBindings_Name vertexBindings[] = { { "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoord" } };

	//--- create the pipeline layout
	pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
	pipeLayoutInfo.addDescSetLayout(apiObj->descriptorSetLayout);

	pvr::api::GraphicsPipelineCreateParam pipeDesc;
	pipeDesc.rasterizer.setCullFace(pvr::types::Face::Back).setFrontFaceWinding(pvr::types::PolygonWindingOrder::FrontFaceCCW);
	pipeDesc.depthStencil.setDepthTestEnable(true);
	pvr::assets::ShaderFile fileVersioning;
	fileVersioning.populateValidVersions(VertShaderSrcFile, *this);
	pipeDesc.vertexShader.setShader(apiObj->context->createShader(*fileVersioning.getBestStreamForApi(getGraphicsContext()->getApiType()), pvr::types::ShaderType::VertexShader));

	fileVersioning.populateValidVersions(FragShaderSrcFile, *this);
	pipeDesc.fragmentShader.setShader(apiObj->context->createShader(*fileVersioning.getBestStreamForApi(getGraphicsContext()->getApiType()), pvr::types::ShaderType::FragmentShader));

	pipeDesc.pipelineLayout = apiObj->context->createPipelineLayout(pipeLayoutInfo);
	pipeDesc.colorBlend.setAttachmentState(0, pvr::types::BlendingConfig());
	pvr::utils::createInputAssemblyFromMesh(scene->getMesh(0), vertexBindings, 3, pipeDesc);

	apiObj->pipeline = apiObj->context->createGraphicsPipeline(pipeDesc);
	if (!apiObj->pipeline.isValid())
	{
		pvr::Log("ERROR: Failed to create Graphics pipeline.");
		return false;
	}

	// Set the sampler2D variable to the first texture unit
	// Store the location of uniforms for later use
	apiObj->commandBuffer->beginRecording();
	apiObj->commandBuffer->bindPipeline(apiObj->pipeline);
<<<<<<< HEAD
	apiObj->commandBuffer->setUniform<pvr::int32>(apiObj->pipeline-> getUniformLocation("sDiffuseMap"), 0);
=======
	apiObj->commandBuffer->setUniform(apiObj->pipeline-> getUniformLocation("sDiffuseMap"), 0);
>>>>>>> 1776432f... 4.3
	apiObj->commandBuffer->endRecording();
	apiObj->commandBuffer->submit();

	uniformLocations.mvpMtx = apiObj->pipeline->getUniformLocation("MVPMatrix");
	uniformLocations.mvITMtx = apiObj->pipeline->getUniformLocation("MVITMatrix");
	uniformLocations.lightDirView = apiObj->pipeline->getUniformLocation("ViewLightDirection");

	uniformLocations.specularExponent = apiObj->pipeline->getUniformLocation("SpecularExponent");
	uniformLocations.metallicity = apiObj->pipeline->getUniformLocation("Metallicity");
	uniformLocations.reflectivity = apiObj->pipeline->getUniformLocation("Reflectivity");
	uniformLocations.albedo = apiObj->pipeline->getUniformLocation("AlbedoModulation");
	return true;
}

/*!*********************************************************************************************************************
\brief Loads the mesh data required for this training course into vertex buffer objects
***********************************************************************************************************************/
void OGLESPVRScopeExample::loadVbos()
{
	pvr::utils::appendSingleBuffersFromModel(getGraphicsContext(), *scene,  apiObj->vbos, apiObj->ibos);
}

/*!*********************************************************************************************************************
\return pvr::Result::Success if no error occurred
\brief  Code in initApplication() will be called by pvr::Shell once per run, before the rendering context is created.
	    Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes,etc.)
	    If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result OGLESPVRScopeExample::initApplication()
{
	//Blue-ish marble
	progUniforms.specularExponent = 100.f;            // Width of the specular highlights (High exponent for small shiny highlights)
	progUniforms.albedo = glm::vec3(.78f, .82f, 1.f); // Overall color
	progUniforms.metallicity = 1.f;                 // Doesn't make much of a difference in this material.
	progUniforms.reflectivity = .2f;                // Low reflectivity - color mostly diffuse.

	// At the time of writing, this counter is the USSE load for vertex + pixel processing
	selectedCounter = 0;
	interval = 0;
	angleY = 0.0f;
	assetStore.init(*this);
	// Load the scene
	if (!assetStore.loadModel(SceneFile, scene))
	{
		this->setExitMessage("ERROR: Couldn't load the .pod file\n");
		return pvr::Result::NotInitialized;
	}

	// Process the command line
	{
		const pvr::platform::CommandLine cmdline = getCommandLine();
		cmdline.getIntOption("-counter", selectedCounter);
		cmdline.getIntOption("-interval", interval);
	}
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in quitApplication() will be called by pvr::Shell once per run, just before exiting
	    the program. If the rendering context is lost, quitApplication() will not be called.x
***********************************************************************************************************************/
pvr::Result OGLESPVRScopeExample::quitApplication()
{
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief Code in initView() will be called by pvr::Shell upon initialization or after a change in the rendering context.
	   Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result OGLESPVRScopeExample::initView()
{
<<<<<<< HEAD
	apiObj.reset(new DeviceResources());
=======
	apiObj.reset(new ApiObjects());
>>>>>>> 1776432f... 4.3
	apiObj->context = getGraphicsContext();
	// create the default fbo using default params
	apiObj->onScreenFbo = apiObj->context->createOnScreenFbo(0);
	apiObj->commandBuffer = apiObj->context->createCommandBufferOnDefaultPool();
	apiObj->secCmd = apiObj->context->createSecondaryCommandBufferOnDefaultPool();
	// Initialize VBO data
	loadVbos();

	// Load textures
	if (!createTexSamplerDescriptorSet())
	{
		return pvr::Result::NotInitialized;
	}

	// Load and compile the shaders & link programs
	if (!createPipeline())
	{
		return pvr::Result::NotInitialized;
	}

	// Initialize UIRenderer
	if (uiRenderer.init(apiObj->onScreenFbo->getRenderPass(), 0) != pvr::Result::Success)
	{
		this->setExitMessage("ERROR: Cannot initialize UIRenderer\n");
		return pvr::Result::NotInitialized;
	}

	// Calculate the projection and view matrices
	// Is the screen rotated?
	bool isRotate = this->isScreenRotated() && this->isFullScreen();
	if (isRotate)
	{
		progUniforms.projectionMtx = pvr::math::perspectiveFov(getApiType(), glm::pi<pvr::float32>() / 6, (float)this->getWidth(),
		                             (float)this->getHeight(), scene->getCamera(0).getNear(), scene->getCamera(0).getFar(), glm::pi<pvr::float32>() * .5f);
	}
	else
	{
		progUniforms.projectionMtx = glm::perspectiveFov(glm::pi<pvr::float32>() / 6, (float)this->getWidth(),
		                             (float)this->getHeight(), scene->getCamera(0).getNear(), scene->getCamera(0).getFar());
	}

	// Initialize the graphing code

	std::string errorStr;
	if (apiObj->scopeGraph.init(apiObj->context, *this, uiRenderer, apiObj->onScreenFbo->getRenderPass(), errorStr))
	{
		// Position the graph
		apiObj->scopeGraph.position(getWidth(), getHeight(), pvr::Rectanglei(static_cast<pvr::uint32>(getWidth() * 0.02f),
		                            static_cast<pvr::uint32>(getHeight() * 0.02f), static_cast<pvr::uint32>(getWidth() * 0.96f), static_cast<pvr::uint32>(getHeight() * 0.96f) / 3));

		// Output the current active group and a list of all the counters
		pvr::Log(pvr::Log.Information, "PVRScope Number of Hardware Counters: %i\n", apiObj->scopeGraph.getCounterNum());
		pvr::Log(pvr::Log.Information, "Counters\n-ID---Name-------------------------------------------\n");

		for (pvr::uint32 i = 0; i < apiObj->scopeGraph.getCounterNum(); ++i)
		{
			pvr::Log(pvr::Log.Information, "[%2i] %s %s\n", i, apiObj->scopeGraph.getCounterName(i),
			         apiObj->scopeGraph.isCounterPercentage(i) ? "percentage" : "absolute");
			apiObj->scopeGraph.showCounter(i, false);
		}

		apiObj->scopeGraph.ping(1);
		// Tell the graph to show initial counters
		apiObj->scopeGraph.showCounter(apiObj->scopeGraph.getStandard3DIndex(), true);
		apiObj->scopeGraph.showCounter(apiObj->scopeGraph.getStandardTAIndex(), true);
		apiObj->scopeGraph.showCounter(apiObj->scopeGraph.getStandardShaderPixelIndex(), true);
		apiObj->scopeGraph.showCounter(apiObj->scopeGraph.getStandardShaderVertexIndex(), true);
		for (pvr::uint32 i = 0; i < apiObj->scopeGraph.getCounterNum(); ++i)
		{
			std::string s(std::string(apiObj->scopeGraph.getCounterName(i))); //Better safe than sorry - get a copy...
			pvr::strings::toLower(s);
			if (pvr::strings::startsWith(s, "hsr efficiency"))
			{
				apiObj->scopeGraph.showCounter(i, true);
			}
			if (pvr::strings::startsWith(s, "shaded pixels per second"))
			{
				apiObj->scopeGraph.showCounter(i, true);
			}
		}

		// Set the update interval: number of updates [frames] before updating the graph
		apiObj->scopeGraph.setUpdateInterval(interval);
	}
	else
	{
		Log(errorStr.c_str());
	}
	uiRenderer.getDefaultTitle()->setText("PVRScopeExample");
	uiRenderer.getDefaultTitle()->commitUpdates();
	recordCommandBuffer();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief Code in releaseView() will be called by pvr::Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result OGLESPVRScopeExample::releaseView()
{
	//Instructs the Asset Store to free all resources
	scene.reset();
	assetStore.releaseAll();
	uiRenderer.release();
	apiObj.reset();
	scene.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result OGLESPVRScopeExample::renderFrame()
{
	// Rotate and Translation the model matrix
	glm::mat4x4 mModel1, mModel2;
	mModel1 = glm::translate(glm::vec3(0.0f, -1.0f, 0.0f)) * glm::rotate((angleY), glm::vec3(0.f, 1.f, 0.f)) *
	          glm::translate(glm::vec3(.5f, 0.f, -1.0f)) * glm::scale(glm::vec3(0.5f, 0.5f, 0.5f)) * scene->getWorldMatrix(0);
	//Create two instances of the mesh, offset to the sides.
	mModel2 = mModel1 * glm::translate(glm::vec3(0, 0, -2000));
	mModel1 = mModel1 * glm::translate(glm::vec3(0, 0, 2000));

	angleY += (2 * glm::pi<glm::float32>() * getFrameTime() / 1000) / 10;

	progUniforms.viewMtx = glm::lookAt(glm::vec3(0, 0, 75), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	glm::mat4 vp = progUniforms.projectionMtx * progUniforms.viewMtx;

	progUniforms.mvMatrix1 = progUniforms.viewMtx * mModel1;
	progUniforms.mvMatrix2 = progUniforms.viewMtx * mModel2;
	progUniforms.mvITMatrix1 = glm::inverseTranspose(glm::mat3(progUniforms.mvMatrix1));
	progUniforms.mvITMatrix2 = glm::inverseTranspose(glm::mat3(progUniforms.mvMatrix2));
	progUniforms.mvpMatrix1 = vp * mModel1;
	progUniforms.mvpMatrix2 = vp * mModel2;

	// Set light direction in model space
	progUniforms.lightDirView = glm::normalize(glm::vec3(1., 1., -1.));

	apiObj->scopeGraph.ping(static_cast<pvr::float32>(getFrameTime()));
	updateDescription();
	recordCommandBuffer();
	apiObj->commandBuffer->beginRecording();
	apiObj->commandBuffer->beginRenderPass(apiObj->onScreenFbo, Rectanglei(0, 0, getWidth(), getHeight()),
	                                       false, glm::vec4(0.00, 0.70, 0.67, 1.0f));
	apiObj->commandBuffer->enqueueSecondaryCmds(apiObj->secCmd);
	apiObj->commandBuffer->endRenderPass();
	apiObj->commandBuffer->endRecording();
	apiObj->commandBuffer->submit();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\param nodeIndex Node index of the mesh to draw
\brief Draws a pvr::Model::Mesh after the model view matrix has been set and the material prepared.
***********************************************************************************************************************/
void OGLESPVRScopeExample::drawMesh(int nodeIndex, api::SecondaryCommandBuffer& cmd)
{
	const pvr::assets::Model::Node& node = scene->getNode(nodeIndex);
	const pvr::assets::Mesh& mesh = scene->getMesh(node.getObjectId());

	// bind the VBO for the mesh
	cmd->bindVertexBuffer(apiObj->vbos[node.getObjectId()], 0, 0);

	// The geometry can be exported in 4 ways:
	// - Indexed Triangle list
	// - Non-Indexed Triangle list
	// - Indexed Triangle strips
	// - Non-Indexed Triangle strips
	if (mesh.getNumStrips() == 0)
	{
		if (apiObj->ibos[node.getObjectId()].isValid())
		{
			// Indexed Triangle list
			cmd->bindIndexBuffer(apiObj->ibos[node.getObjectId()],
			                     0, mesh.getFaces().getDataType());
			cmd->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
		}
		else
		{
			// Non-Indexed Triangle list
			cmd->drawArrays(0, mesh.getNumFaces(), 0, 1);
		}
	}
	else
	{
		for (pvr::int32 i = 0; i < (pvr::int32)mesh.getNumStrips(); ++i)
		{
			int offset = 0;
			if (apiObj->ibos[node.getObjectId()].isValid())
			{
				// Indexed Triangle strips
				cmd->bindIndexBuffer(apiObj->ibos[node.getObjectId()],
				                     0, mesh.getFaces().getDataType());
				cmd->drawIndexed(0, mesh.getStripLength(i) + 2, 0, 0, 1);
			}
			else
			{
				// Non-Indexed Triangle strips
				cmd->drawArrays(0, mesh.getStripLength(i) + 2, 0, 1);
			}
			offset += mesh.getStripLength(i) + 2;
		}
	}
}

/*!*********************************************************************************************************************
\brief	Pre-record the rendering commands
***********************************************************************************************************************/
void OGLESPVRScopeExample::recordCommandBuffer()
{
	apiObj->secCmd->beginRecording(apiObj->onScreenFbo);
	// Use shader program
	apiObj->secCmd->bindPipeline(apiObj->pipeline);

	// Bind texture
	apiObj->secCmd->bindDescriptorSet(
	  apiObj->pipeline->getPipelineLayout(), 0, apiObj->descriptorSet, 0);

<<<<<<< HEAD
	apiObj->secCmd->setUniformPtr<glm::vec3>(uniformLocations.lightDirView, 1, &progUniforms.lightDirView);
	apiObj->secCmd->setUniformPtr<pvr::float32>(uniformLocations.specularExponent, 1, &progUniforms.specularExponent);
	apiObj->secCmd->setUniformPtr<pvr::float32>(uniformLocations.metallicity, 1, &progUniforms.metallicity);
	apiObj->secCmd->setUniformPtr<pvr::float32>(uniformLocations.reflectivity, 1, &progUniforms.reflectivity);
	apiObj->secCmd->setUniformPtr<glm::vec3>(uniformLocations.albedo, 1, &progUniforms.albedo);


	// Now that the uniforms are set, call another function to actually draw the mesh.
	apiObj->secCmd->setUniformPtr<glm::mat4>(uniformLocations.mvpMtx, 1, &progUniforms.mvpMatrix1);
	apiObj->secCmd->setUniformPtr<glm::mat3>(uniformLocations.mvITMtx, 1, &progUniforms.mvITMatrix1);
	drawMesh(0, apiObj->secCmd);
	// Now that the uniforms are set, call another function to actually draw the mesh.
	apiObj->secCmd->setUniformPtr<glm::mat4>(uniformLocations.mvpMtx, 1, &progUniforms.mvpMatrix2);
	apiObj->secCmd->setUniformPtr<glm::mat3>(uniformLocations.mvITMtx, 1, &progUniforms.mvITMatrix2);
=======
	apiObj->secCmd->setUniformPtr(uniformLocations.lightDirView, 1, &progUniforms.lightDirView);
	apiObj->secCmd->setUniformPtr(uniformLocations.specularExponent, 1, &progUniforms.specularExponent);
	apiObj->secCmd->setUniformPtr(uniformLocations.metallicity, 1, &progUniforms.metallicity);
	apiObj->secCmd->setUniformPtr(uniformLocations.reflectivity, 1, &progUniforms.reflectivity);
	apiObj->secCmd->setUniformPtr(uniformLocations.albedo, 1, &progUniforms.albedo);


	// Now that the uniforms are set, call another function to actually draw the mesh.
	apiObj->secCmd->setUniformPtr(uniformLocations.mvpMtx, 1, &progUniforms.mvpMatrix1);
	apiObj->secCmd->setUniformPtr(uniformLocations.mvITMtx, 1, &progUniforms.mvITMatrix1);
	drawMesh(0, apiObj->secCmd);
	// Now that the uniforms are set, call another function to actually draw the mesh.
	apiObj->secCmd->setUniformPtr(uniformLocations.mvpMtx, 1, &progUniforms.mvpMatrix2);
	apiObj->secCmd->setUniformPtr(uniformLocations.mvITMtx, 1, &progUniforms.mvITMatrix2);
>>>>>>> 1776432f... 4.3
	drawMesh(0, apiObj->secCmd);

	apiObj->scopeGraph.recordCommandBuffer(apiObj->secCmd, 0);
	updateDescription();

	uiRenderer.beginRendering(apiObj->secCmd);
	uiRenderer.getDefaultTitle()->render();
	uiRenderer.getDefaultDescription()->render();
	uiRenderer.getSdkLogo()->render();
	apiObj->scopeGraph.recordUIElements();
	uiRenderer.endRendering();
	apiObj->secCmd->endRecording();
}

/*!*********************************************************************************************************************
\brief	Update the description
***********************************************************************************************************************/
void OGLESPVRScopeExample::updateDescription()
{
	static char description[256];

	if (apiObj->scopeGraph.getCounterNum())
	{
		float maximum = apiObj->scopeGraph.getMaximumOfData(selectedCounter);
		float userY = apiObj->scopeGraph.getMaximum(selectedCounter);
		bool isKilos = false;
		if (maximum > 10000)
		{
			maximum /= 1000;
			userY /= 1000;
			isKilos = true;
		}
		bool isPercentage = apiObj->scopeGraph.isCounterPercentage(selectedCounter);

		const char* standard =
		  "Use up-down to select a counter, click to enable/disable it\n"
		  "Counter [%i]\n"
		  "Name: %s\n"
		  "Shown: %s\n"
		  "user y-axis: %.2f  max: %.2f\n";
		const char* percentage =
		  "Use up-down to select a counter, click to enable/disable it\n"
		  "Counter [%i]\n"
		  "Name: %s\n"
		  "Shown: %s\n"
		  "user y-axis: %.2f%%  max: %.2f%%\n";
		const char* kilo =
		  "Use up-down to select a counter, click to enable/disable it\n"
		  "Counter [%i]\n"
		  "Name: %s\n"
		  "Shown: %s\n"
		  "user y-axis: %.0fK  max: %.0fK\n";

		sprintf(description,
		        isKilos ? kilo : isPercentage ? percentage : standard, selectedCounter,
		        apiObj->scopeGraph.getCounterName(selectedCounter),
		        apiObj->scopeGraph.isCounterShown(selectedCounter) ? "Yes" : "No", userY, maximum);
		uiRenderer.getDefaultDescription()->setColor(glm::vec4(1.f));
	}
	else
	{
		sprintf(description, "No counters present");
		uiRenderer.getDefaultDescription()->setColor(glm::vec4(.8f, 0.0f, 0.0f, 1.0f));
	}
	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroUIRenderer
	uiRenderer.getDefaultDescription()->setText(description);
	uiRenderer.getDefaultDescription()->commitUpdates();
}

/*!*********************************************************************************************************************
\return auto ptr to the demo supplied by the user
\brief	This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the
		behavior of the application.
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() { return std::auto_ptr<pvr::Shell>(new OGLESPVRScopeExample()); }
