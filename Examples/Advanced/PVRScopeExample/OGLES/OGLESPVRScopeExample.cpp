/*!*********************************************************************************************************************
\File         OGLESPVRScopeExample.cpp
\Title        PVRScopeExample
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief  Shows how to use our example PVRScope graph code.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVRUIRenderer/PVRUIRenderer.h"
#include "PVRScopeGraph.h"

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

	struct DeviceResources
	{
		pvr::api::CommandBuffer commandBuffer;
		pvr::api::GraphicsPipeline pipeline;
		pvr::api::TextureView texture;
		std::vector<pvr::api::Buffer> ibos;
		std::vector<pvr::api::Buffer> vbos;
		pvr::api::DescriptorSet descriptorSet;
		pvr::api::DescriptorSetLayout descriptorSetLayout;
		pvr::api::Fbo backBufferFbo;
		pvr::GraphicsContext context;
	};
	std::auto_ptr<DeviceResources> deviceResources;

	// 3D Model
	pvr::assets::ModelHandle scene;
	pvr::api::AssetStore assetStore;
	// Projection and view matrices

	// Group shader programs and their uniform locations together
	struct
	{
		pvr::int32 mvpMtx;
		//pvr::int32 mvMtx;
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
	std::auto_ptr<PVRScopeGraph> scopeGraph;

	// Variables for the graphing code
	pvr::int32 selectedCounter;
	pvr::int32 interval;
public:
	virtual pvr::Result::Enum initApplication();
	virtual pvr::Result::Enum initView();
	virtual pvr::Result::Enum releaseView();
	virtual pvr::Result::Enum quitApplication();
	virtual pvr::Result::Enum renderFrame();

	void eventMappedInput(pvr::SimplifiedInput::Enum key);

	void updateDescription();
	void recordCommandBuffer();
	bool createTexSamplerDescriptorSet();
	bool createPipeline();
	void loadVbos();

	void drawMesh(int nodeIndex);
};

/*!*********************************************************************************************************************
\brief Handle input key events
\param key key event to handle
************************************************************************************************************************/
void OGLESPVRScopeExample::eventMappedInput(pvr::SimplifiedInput::Enum key)
{
	// Keyboard input (cursor up/down to cycle through counters)
	switch (key)
	{
	case pvr::SimplifiedInput::Up:
	case pvr::SimplifiedInput::Right:
	{
		selectedCounter++;
		if (selectedCounter > (int)scopeGraph->getCounterNum()) { selectedCounter = scopeGraph->getCounterNum(); }
	} break;
	case pvr::SimplifiedInput::Down:
	case pvr::SimplifiedInput::Left:
	{
		selectedCounter--;
		if (selectedCounter < 0) { selectedCounter = 0; }
	} break;
	case pvr::SimplifiedInput::Action1:
	{
		scopeGraph->showCounter(selectedCounter, !scopeGraph->isCounterShown(selectedCounter));
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
	if (!assetStore.getTextureWithCaching(getGraphicsContext(), TextureFile, &deviceResources->texture, NULL))
	{
		pvr::Log("ERROR: Failed to load texture.");
		return false;
	}
	// create the bilinear sampler
	pvr::assets::SamplerCreateParam samplerDesc;
	samplerDesc.minificationFilter = pvr::SamplerFilter::Linear;
	samplerDesc.mipMappingFilter = pvr::SamplerFilter::Nearest;
	samplerDesc.magnificationFilter = pvr::SamplerFilter::Linear;
	pvr::api::Sampler bilinearSampler = deviceResources->context->createSampler(samplerDesc);

	pvr::api::DescriptorSetLayoutCreateParam descSetLayoutInfo;

	descSetLayoutInfo.addBinding(0, pvr::api::DescriptorType::CombinedImageSampler, 1,
	                             pvr::api::ShaderStageFlags::Fragment);

	deviceResources->descriptorSetLayout = deviceResources->context->createDescriptorSetLayout(descSetLayoutInfo);

	pvr::api::DescriptorSetUpdateParam descriptorSetUpdate;
	descriptorSetUpdate.addCombinedImageSampler(0, 0, deviceResources->texture, bilinearSampler);
	deviceResources->descriptorSet = deviceResources->context->allocateDescriptorSet(deviceResources->descriptorSetLayout);
	deviceResources->descriptorSet->update(descriptorSetUpdate);
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
	pipeLayoutInfo.addDescSetLayout(deviceResources->descriptorSetLayout);

	pvr::api::GraphicsPipelineCreateParam pipeDesc;
	pvr::assets::ShaderFile fileVersioning;
	fileVersioning.populateValidVersions(VertShaderSrcFile, *this);
	pipeDesc.vertexShader.setShader(deviceResources->context->createShader(*fileVersioning.getBestStreamForApi(getGraphicsContext()->getApiType()), pvr::ShaderType::VertexShader));

	fileVersioning.populateValidVersions(FragShaderSrcFile, *this);
	pipeDesc.fragmentShader.setShader(deviceResources->context->createShader(*fileVersioning.getBestStreamForApi(getGraphicsContext()->getApiType()), pvr::ShaderType::FragmentShader));

	pipeDesc.pipelineLayout = deviceResources->context->createPipelineLayout(pipeLayoutInfo);

	pvr::utils::createInputAssemblyFromMesh(scene->getMesh(0), vertexBindings, 3, pipeDesc);

	deviceResources->pipeline = deviceResources->context->createGraphicsPipeline(pipeDesc);
	if (!deviceResources->pipeline.isValid())
	{
		pvr::Log("ERROR: Failed to create Graphics pipeline.");
		return false;
	}

	// Set the sampler2D variable to the first texture unit
	// Store the location of uniforms for later use
	deviceResources->commandBuffer->beginRecording();
	deviceResources->commandBuffer->bindPipeline(deviceResources->pipeline);
	deviceResources->commandBuffer->setUniform<pvr::int32>(deviceResources->pipeline-> getUniformLocation("sDiffuseMap"), 0);
	deviceResources->commandBuffer->endRecording();
	deviceResources->commandBuffer->submit();

	uniformLocations.mvpMtx = deviceResources->pipeline->getUniformLocation("MVPMatrix");
	//uniformLocations.mvMtx = deviceResources->pipeline->getUniformLocation("MVMatrix");
	uniformLocations.mvITMtx = deviceResources->pipeline->getUniformLocation("MVITMatrix");
	uniformLocations.lightDirView = deviceResources->pipeline->getUniformLocation("ViewLightDirection");

	uniformLocations.specularExponent = deviceResources->pipeline->getUniformLocation("SpecularExponent");
	uniformLocations.metallicity = deviceResources->pipeline->getUniformLocation("Metallicity");
	uniformLocations.reflectivity = deviceResources->pipeline->getUniformLocation("Reflectivity");
	uniformLocations.albedo = deviceResources->pipeline->getUniformLocation("AlbedoModulation");
	return true;
}

/*!*********************************************************************************************************************
\brief Loads the mesh data required for this training course into vertex buffer objects
***********************************************************************************************************************/
void OGLESPVRScopeExample::loadVbos()
{
	pvr::utils::appendSingleBuffersFromModel(getGraphicsContext(), *scene,  deviceResources->vbos, deviceResources->ibos);
}

/*!*********************************************************************************************************************
\return pvr::Result::Success if no error occurred
\brief  Code in initApplication() will be called by pvr::Shell once per run, before the rendering context is created.
	    Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes,etc.)
	    If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result::Enum OGLESPVRScopeExample::initApplication()
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
		return pvr::Result::NotInitialised;
	}

	// Process the command line
	{
		const pvr::system::CommandLine cmdline = getCommandLine();
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
pvr::Result::Enum OGLESPVRScopeExample::quitApplication()
{
	//Instructs the Asset Store to free all resources
	scene.release();
	assetStore.releaseAll();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief Code in initView() will be called by pvr::Shell upon initialization or after a change in the rendering context.
	   Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result::Enum OGLESPVRScopeExample::initView()
{
	deviceResources.reset(new DeviceResources());
	deviceResources->context = getGraphicsContext();
	deviceResources->commandBuffer = deviceResources->context->createCommandBuffer();
	std::string errorStr;

	// Initialize VBO data
	loadVbos();

	// Load textures
	if (!createTexSamplerDescriptorSet())
	{
		this->setExitMessage(errorStr.c_str());
		return pvr::Result::NotInitialised;
	}

	// Load and compile the shaders & link programs
	if (!createPipeline())
	{
		this->setExitMessage(errorStr.c_str());
		return pvr::Result::NotInitialised;
	}

	// Initialize UIRenderer
	if (uiRenderer.init(getGraphicsContext()) != pvr::Result::Success)
	{
		this->setExitMessage("ERROR: Cannot initialize UIRenderer\n");
		return pvr::Result::NotInitialised;
	}

	// Calculate the projection and view matrices
	// Is the screen rotated?
	bool isRotate = this->isScreenRotated() && this->isFullScreen();
	if (isRotate)
	{
		progUniforms.projectionMtx = pvr::math::perspectiveFov(glm::pi<pvr::float32>() / 6, (float)this->getWidth(),
		                             (float)this->getHeight(), scene->getCamera(0).getNear(), scene->getCamera(0).getFar(), glm::pi<pvr::float32>() * .5f);
	}
	else
	{
		progUniforms.projectionMtx = glm::perspectiveFov(glm::pi<pvr::float32>() / 6, (float)this->getWidth(),
		                             (float)this->getHeight(), scene->getCamera(0).getNear(), scene->getCamera(0).getFar());
	}

	// Initialize the graphing code
	scopeGraph.reset(new PVRScopeGraph(deviceResources->context, *this, uiRenderer));

	if (scopeGraph.get())
	{
		// Position the graph
		scopeGraph->position(getWidth(), getHeight(), pvr::Rectanglei((getWidth() * 0.02f), (getHeight() * 0.02f), (getWidth() * 0.96f), (getHeight() * 0.96f) / 3));

		// Output the current active group and a list of all the counters
		pvr::Log(pvr::Log.Information, "PVRScope Number of Hardware Counters: %i\n", scopeGraph->getCounterNum());
		pvr::Log(pvr::Log.Information, "Counters\n-ID---Name-------------------------------------------\n");

		for (pvr::uint32 i = 0; i < scopeGraph->getCounterNum(); ++i)
		{
			pvr::Log(pvr::Log.Information, "[%2i] %s %s\n", i, scopeGraph->getCounterName(i), scopeGraph->isCounterPercentage(i) ? "percentage" : "absolute");
			scopeGraph->showCounter(i, false);
		}

		scopeGraph->ping(1);
		// Tell the graph to show initial counters
		scopeGraph->showCounter(scopeGraph->getStandard3DIndex(), true);
		scopeGraph->showCounter(scopeGraph->getStandardTAIndex(), true);
		scopeGraph->showCounter(scopeGraph->getStandardShaderPixelIndex(), true);
		scopeGraph->showCounter(scopeGraph->getStandardShaderVertexIndex(), true);
		for (pvr::uint32 i = 0; i < scopeGraph->getCounterNum(); ++i)
		{
			std::string s(std::string(scopeGraph->getCounterName(i))); //Better safe than sorry - get a copy...
			pvr::strings::toLower(s);
			if (pvr::strings::startsWith(s, "hsr efficiency"))
			{
				scopeGraph->showCounter(i, true);
			}
			if (pvr::strings::startsWith(s, "shaded pixels per second"))
			{
				scopeGraph->showCounter(i, true);
			}
		}

		// Set the update interval: number of updates [frames] before updating the graph
		scopeGraph->setUpdateInterval(interval);
	}

	// create the default fbo using default params
	deviceResources->backBufferFbo = deviceResources->context->createOnScreenFboWithParams();
	uiRenderer.getDefaultTitle()->setText("PVRScopeExample");
	uiRenderer.getDefaultTitle()->commitUpdates();
	recordCommandBuffer();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief Code in releaseView() will be called by pvr::Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result::Enum OGLESPVRScopeExample::releaseView()
{
	uiRenderer.release();
	deviceResources.reset();
	scene.reset();
	scopeGraph.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result::Enum OGLESPVRScopeExample::renderFrame()
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

	scopeGraph->ping(getFrameTime());
	updateDescription();
	recordCommandBuffer();
	deviceResources->commandBuffer->submit();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\param nodeIndex Node index of the mesh to draw
\brief Draws a pvr::Model::Mesh after the model view matrix has been set and the material prepared.
***********************************************************************************************************************/
void OGLESPVRScopeExample::drawMesh(int nodeIndex)
{
	const pvr::assets::Model::Node& node = scene->getNode(nodeIndex);
	const pvr::assets::Mesh& mesh = scene->getMesh(node.getObjectId());

	// bind the VBO for the mesh
	deviceResources->commandBuffer->bindVertexBuffer(deviceResources->vbos[node.getObjectId()], 0, 0);

	// The geometry can be exported in 4 ways:
	// - Indexed Triangle list
	// - Non-Indexed Triangle list
	// - Indexed Triangle strips
	// - Non-Indexed Triangle strips
	if (mesh.getNumStrips() == 0)
	{
		if (deviceResources->ibos[node.getObjectId()].isValid())
		{
			// Indexed Triangle list
			deviceResources->commandBuffer->bindIndexBuffer(deviceResources->ibos[node.getObjectId()],
			        0, mesh.getFaces().getDataType());
			deviceResources->commandBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
		}
		else
		{
			// Non-Indexed Triangle list
			deviceResources->commandBuffer->drawArrays(0, mesh.getNumFaces(), 0, 1);
		}
	}
	else
	{
		for (pvr::int32 i = 0; i < (pvr::int32)mesh.getNumStrips(); ++i)
		{
			int offset = 0;
			if (deviceResources->ibos[node.getObjectId()].isValid())
			{
				// Indexed Triangle strips
				deviceResources->commandBuffer->bindIndexBuffer(deviceResources->ibos[node.getObjectId()],
				        0, mesh.getFaces().getDataType());
				deviceResources->commandBuffer->drawIndexed(0, mesh.getStripLength(i) + 2, 0, 0, 1);
			}
			else
			{
				// Non-Indexed Triangle strips
				deviceResources->commandBuffer->drawArrays(0, mesh.getStripLength(i) + 2, 0, 1);
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
	deviceResources->commandBuffer->beginRecording();
	deviceResources->commandBuffer->beginRenderPass(deviceResources->backBufferFbo,
	        pvr::Rectanglei(0, 0, getWidth(), getHeight()), glm::vec4(0.00, 0.70, 0.67, 1.0f));
	// Use shader program
	deviceResources->commandBuffer->bindPipeline(deviceResources->pipeline);

	// Bind texture
	deviceResources->commandBuffer->bindDescriptorSets(pvr::api::PipelineBindingPoint::Graphics,
	        deviceResources->pipeline->getPipelineLayout(), deviceResources->descriptorSet, 0);

	deviceResources->commandBuffer->setUniformPtr<glm::vec3>(uniformLocations.lightDirView, 1, &progUniforms.lightDirView);
	deviceResources->commandBuffer->setUniformPtr<pvr::float32>(uniformLocations.specularExponent, 1, &progUniforms.specularExponent);
	deviceResources->commandBuffer->setUniformPtr<pvr::float32>(uniformLocations.metallicity, 1, &progUniforms.metallicity);
	deviceResources->commandBuffer->setUniformPtr<pvr::float32>(uniformLocations.reflectivity, 1, &progUniforms.reflectivity);
	deviceResources->commandBuffer->setUniformPtr<glm::vec3>(uniformLocations.albedo, 1, &progUniforms.albedo);


	// Now that the uniforms are set, call another function to actually draw the mesh.
	deviceResources->commandBuffer->setUniformPtr<glm::mat4>(uniformLocations.mvpMtx, 1, &progUniforms.mvpMatrix1);
	//deviceResources->commandBuffer->setUniformPtr<glm::mat4>(uniformLocations.mvMtx, 1, &progUniforms.mvMatrix1);
	deviceResources->commandBuffer->setUniformPtr<glm::mat3>(uniformLocations.mvITMtx, 1, &progUniforms.mvITMatrix1);
	drawMesh(0);
	// Now that the uniforms are set, call another function to actually draw the mesh.
	deviceResources->commandBuffer->setUniformPtr<glm::mat4>(uniformLocations.mvpMtx, 1, &progUniforms.mvpMatrix2);
	//deviceResources->commandBuffer->setUniformPtr<glm::mat4>(uniformLocations.mvMtx, 1, &progUniforms.mvMatrix2);
	deviceResources->commandBuffer->setUniformPtr<glm::mat3>(uniformLocations.mvITMtx, 1, &progUniforms.mvITMatrix2);
	drawMesh(0);

	scopeGraph->recordCommandBuffer(deviceResources->commandBuffer);
	updateDescription();

	pvr::api::SecondaryCommandBuffer uicmd = deviceResources->context->createSecondaryCommandBuffer();
	uiRenderer.beginRendering(uicmd);
	uiRenderer.getDefaultTitle()->render();
	uiRenderer.getDefaultDescription()->render();
	uiRenderer.getSdkLogo()->render();
	scopeGraph->recordUIElements();
	uiRenderer.endRendering();
	deviceResources->commandBuffer->enqueueSecondaryCmds(uicmd);
	deviceResources->commandBuffer->endRenderPass();
	deviceResources->commandBuffer->endRecording();
}

/*!*********************************************************************************************************************
\brief	Update the description
***********************************************************************************************************************/
void OGLESPVRScopeExample::updateDescription()
{
	static char description[256];

	if (scopeGraph->getCounterNum())
	{
		float maximum = scopeGraph->getMaximumOfData(selectedCounter);
		float userY = scopeGraph->getMaximum(selectedCounter);
		bool isKilos = false;
		if (maximum > 10000)
		{
			maximum /= 1000;
			userY /= 1000;
			isKilos = true;
		}
		bool isPercentage = scopeGraph->isCounterPercentage(selectedCounter);

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
		        isKilos ? kilo : isPercentage ? percentage : standard,
		        selectedCounter,
		        scopeGraph->getCounterName(selectedCounter),
		        scopeGraph->isCounterShown(selectedCounter) ? "Yes" : "No",
		        userY,
		        maximum);
		uiRenderer.getDefaultDescription()->setColor(glm::vec4(1.f));
	}
	else
	{
		sprintf(description, "No counters present");
		uiRenderer.getDefaultDescription()->setColor(glm::vec4(.8f, 0.0f, 0.0f, 1.0f));
	}
	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRUIRenderer
	uiRenderer.getDefaultDescription()->setText(description);
	uiRenderer.getDefaultDescription()->commitUpdates();
}

/*!*********************************************************************************************************************
\return auto ptr to the demo supplied by the user
\brief	This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the
		behavior of the application.
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() { return std::auto_ptr<pvr::Shell>(new OGLESPVRScopeExample()); }
