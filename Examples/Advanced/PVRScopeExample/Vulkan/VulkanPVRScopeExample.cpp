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

#if !defined(_WIN32) || defined(__WINSCW__)
#define _stricmp strcasecmp
#endif
using namespace pvr;
// Shader Source
const char FragShaderSrcFile[] = "FragShader_vk.fsh.spv";
const char VertShaderSrcFile[] = "VertShader_vk.vsh.spv";

// PVR texture files
const char TextureFile[] = "Marble.pvr";

// POD scene files
const char SceneFile[] = "scene.pod";
enum { MaxSwapChain = 8, NumModelInstance = 2 };


namespace MvpUboElements {
enum Enum { MVP, MVIT, Count };
std::pair<StringHash, types::GpuDatatypes::Enum> Mapping[Count] =
{
	{ "MVPMatrix", types::GpuDatatypes::mat4x4 },
	{ "MVITMatrix", types::GpuDatatypes::mat3x3 }
};
}//namespace MvpUboElements


namespace MaterialUboElements {
enum Enum
{
	ViewLightDirection, AlbedoModulation, SpecularExponent, Metallicity, Reflectivity, Count
};
std::pair<StringHash, types::GpuDatatypes::Enum> Mapping[Count] =
{
	{ "ViewLightDirection", types::GpuDatatypes::vec3 },
	{ "AlbedoModulation",	types::GpuDatatypes::vec3 },
	{ "SpecularExponent",	types::GpuDatatypes::float32 },
	{ "Metallicity",		types::GpuDatatypes::float32 },
	{ "Reflectivity",		types::GpuDatatypes::float32 },
};
}// namespace MaterialUboElements

/*!*********************************************************************************************************************
\brief Class implementing the Shell functions.
***********************************************************************************************************************/
class VulkanPVRScopeExample : public Shell
{
	// Print3D class used to display text

	struct ApiObjects
	{
		api::CommandBuffer commandBuffer[MaxSwapChain];
		api::SecondaryCommandBuffer secCmdBuffer[MaxSwapChain];
		api::GraphicsPipeline pipeline;
		api::TextureView texture;
		std::vector<api::Buffer> ibos;
		std::vector<api::Buffer> vbos;
		api::DescriptorSet texSamplerDescriptor;
		api::DescriptorSet	mvpDescriptor[MaxSwapChain];
		api::DescriptorSet materialDescriptor[MaxSwapChain];
		api::DescriptorSetLayout texSamplerLayout;
		api::DescriptorSetLayout uboLayoutVert;
		api::DescriptorSetLayout uboLayoutFrag;
		utils::StructuredMemoryView mvpUboView;
		utils::StructuredMemoryView materialUboView;
		Multi<api::Fbo> onScreenFbo;
		ui::UIRenderer uiRenderer;
		PVRScopeGraph scopeGraph;
		GraphicsContext context;
		utils::AssetStore assetStore;
	};
	std::auto_ptr<ApiObjects> apiObj;

	// 3D Model
	assets::ModelHandle scene;
	// Projection and view matrices


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
		float32 specularExponent;
		float32 metallicity;
		float32 reflectivity;
		glm::vec3    albedo;
	} progUniforms;


	struct MaterialData
	{
		glm::vec3 lightDirView;
		glm::vec3 albedoMod;
		float32	 specExponent;
		float32  metalicity;
		float32  reflectivity;
	} materialData;

	// The translation and Rotate parameter of Model
	float32 angleY;

	// The PVRScopeGraph variable

	// Variables for the graphing code
	int32 selectedCounter;
	int32 interval;
	glm::mat4 projMtx;
	glm::mat4 viewMtx;
public:
	virtual Result initApplication();
	virtual Result initView();
	virtual Result releaseView();
	virtual Result quitApplication();
	virtual Result renderFrame();

	void eventMappedInput(SimplifiedInput key);

	void updateDescription();
	void recordCommandBuffer(uint32 swapChain);
	bool createTexSamplerDescriptorSet();
	bool createUboDescriptorSet();
	bool createPipeline();
	void loadVbos();
	void updateMVPMatrix(uint32 swapChain);

	void drawMesh(int nodeIndex, api::SecondaryCommandBuffer& cmd);
};

/*!*********************************************************************************************************************
\brief Handle input key events
\param key key event to handle
************************************************************************************************************************/
void VulkanPVRScopeExample::eventMappedInput(SimplifiedInput key)
{
	// Keyboard input (cursor up/down to cycle through counters)
	switch (key)
	{
	case SimplifiedInput::Up:
	case SimplifiedInput::Right:
	{
		selectedCounter++;
		if (selectedCounter > (int)apiObj->scopeGraph.getCounterNum()) { selectedCounter = apiObj->scopeGraph.getCounterNum(); }
	} break;
	case SimplifiedInput::Down:
	case SimplifiedInput::Left:
	{
		selectedCounter--;
		if (selectedCounter < 0) { selectedCounter = 0; }
	} break;
	case SimplifiedInput::Action1:
	{
		apiObj->scopeGraph.showCounter(selectedCounter, !apiObj->scopeGraph.isCounterShown(selectedCounter));
	} break;
	// Keyboard input (cursor left/right to change active group)
	case SimplifiedInput::ActionClose: exitShell(); break;
	default: break;
	}
}

/*!*********************************************************************************************************************
\brief Loads the textures required for this training course
\return Return true if no error occurred
***********************************************************************************************************************/
bool VulkanPVRScopeExample::createTexSamplerDescriptorSet()
{
	if (!apiObj->assetStore.getTextureWithCaching(getGraphicsContext(), TextureFile, &apiObj->texture, NULL))
	{
		Log("ERROR: Failed to load texture.");
		return false;
	}
	// create the bilinear sampler
	assets::SamplerCreateParam samplerDesc;
	samplerDesc.minificationFilter = types::SamplerFilter::Linear;
	samplerDesc.mipMappingFilter = types::SamplerFilter::Nearest;
	samplerDesc.magnificationFilter = types::SamplerFilter::Linear;
	api::Sampler bilinearSampler = apiObj->context->createSampler(samplerDesc);

	apiObj->texSamplerDescriptor = apiObj->context->createDescriptorSetOnDefaultPool(
	                                 apiObj->texSamplerLayout);

	apiObj->texSamplerDescriptor->update(api::DescriptorSetUpdate()
	                                     .setCombinedImageSampler(0, apiObj->texture, bilinearSampler));
	return true;
}

bool VulkanPVRScopeExample::createUboDescriptorSet()
{
	// create the mvp ubo
	utils::StructuredMemoryView& mvpMemView = apiObj->mvpUboView;
	mvpMemView.addEntriesPacked(MvpUboElements::Mapping, MvpUboElements::Count);
	mvpMemView.finalize(apiObj->context, NumModelInstance, types::BufferBindingUse::UniformBuffer, true, false);
	for (uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		api::Buffer buffer = apiObj->context->createBuffer(mvpMemView.getAlignedTotalSize(),
		                     types::BufferBindingUse::UniformBuffer, true);


		mvpMemView.connectWithBuffer(i, apiObj->context->createBufferView(buffer, 0,
		                             mvpMemView.getAlignedElementSize()));

		api::DescriptorSet& descSet = apiObj->mvpDescriptor[i];
		descSet = apiObj->context->createDescriptorSetOnDefaultPool(apiObj->uboLayoutVert);
		if (!descSet->update(api::DescriptorSetUpdate().setDynamicUbo(0, mvpMemView.getConnectedBuffer(i))))
		{
			setExitMessage("Failed to create the mvp ubo descriptor set");
			return false;
		}
	}

	// create the material ubo
	utils::StructuredMemoryView& matMemView = apiObj->materialUboView;
	matMemView.addEntriesPacked(MaterialUboElements::Mapping, MaterialUboElements::Count);
	matMemView.finalize(apiObj->context, 1, types::BufferBindingUse::UniformBuffer, false, false);
	for (uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		matMemView.connectWithBuffer(i, apiObj->context->createBufferAndView(
		                               matMemView.getAlignedElementSize(), types::BufferBindingUse::UniformBuffer, true));

		api::DescriptorSet& descSet = apiObj->materialDescriptor[i];
		descSet = apiObj->context->createDescriptorSetOnDefaultPool(apiObj->uboLayoutFrag);
		if (!descSet->update(api::DescriptorSetUpdate().setUbo(0, matMemView.getConnectedBuffer(i))))
		{
			setExitMessage("Failed to create the material ubo descriptor set");
			return false;
		}

		// fill the buffer with initial values
		matMemView.map(i);
		matMemView.setValue(MaterialUboElements::ViewLightDirection, glm::vec4(materialData.lightDirView, 0));
		matMemView.setValue(MaterialUboElements::AlbedoModulation,  glm::vec4(materialData.albedoMod, 0));
		matMemView.setValue(MaterialUboElements::SpecularExponent, materialData.specExponent);
		matMemView.setValue(MaterialUboElements::Metallicity, materialData.metalicity);
		matMemView.setValue(MaterialUboElements::Reflectivity, materialData.reflectivity);
		matMemView.unmap(i);
	}
	return true;
}

/*!*********************************************************************************************************************
\brief	Create a graphics pipeline required for this training course
\return	Return true if no error occurred
***********************************************************************************************************************/
bool VulkanPVRScopeExample::createPipeline()
{
	utils::VertexBindings_Name vertexBindings[] =
	{
		{ "POSITION", "inVertex" },
		{ "NORMAL", "inNormal" },
		{ "UV0", "inTexCoord" }
	};

	//--- create the descriptor set Layout
	apiObj->texSamplerLayout = apiObj->context->createDescriptorSetLayout(
	                             api::DescriptorSetLayoutCreateParam()
	                             .setBinding(0, types::DescriptorType::CombinedImageSampler, 1,
	                                 types::ShaderStageFlags::Fragment));

	apiObj->uboLayoutVert = apiObj->context->createDescriptorSetLayout(
	                          api::DescriptorSetLayoutCreateParam()
	                          .setBinding(0, types::DescriptorType::UniformBufferDynamic, 1,
	                                      types::ShaderStageFlags::Vertex));

	apiObj->uboLayoutFrag = apiObj->context->createDescriptorSetLayout(
	                          api::DescriptorSetLayoutCreateParam()
	                          .setBinding(0, types::DescriptorType::UniformBuffer, 1,
	                                      types::ShaderStageFlags::Fragment));

	//--- create the pipeline layout
	api::PipelineLayoutCreateParam pipeLayoutInfo;
	pipeLayoutInfo
	.setDescSetLayout(0, apiObj->uboLayoutVert)// mvp
	.setDescSetLayout(1, apiObj->texSamplerLayout) // albedo
	.setDescSetLayout(2, apiObj->uboLayoutFrag);// material

	api::GraphicsPipelineCreateParam pipeDesc;
	pipeDesc.vertexShader.setShader(apiObj->context->createShader(*getAssetStream(VertShaderSrcFile),
	                                types::ShaderType::VertexShader));

	pipeDesc.fragmentShader.setShader(apiObj->context->createShader(*getAssetStream(FragShaderSrcFile),
	                                  types::ShaderType::FragmentShader));

	pipeDesc.rasterizer.setCullFace(pvr::types::Face::Back);
	pipeDesc.depthStencil.setDepthTestEnable(true);
	pipeDesc.depthStencil.setDepthCompareFunc(pvr::types::ComparisonMode::Less);
	pipeDesc.depthStencil.setDepthWrite(true);
	pipeDesc.pipelineLayout = apiObj->context->createPipelineLayout(pipeLayoutInfo);
	pipeDesc.colorBlend.setAttachmentState(0, types::BlendingConfig());
	utils::createInputAssemblyFromMesh(scene->getMesh(0), vertexBindings, 3, pipeDesc);
	pipeDesc.renderPass = apiObj->onScreenFbo[0]->getRenderPass();
	apiObj->pipeline = apiObj->context->createGraphicsPipeline(pipeDesc);
	if (!apiObj->pipeline.isValid())
	{
		setExitMessage("ERROR: Failed to create Graphics pipeline.");
		return false;
	}
	return true;
}

/*!*********************************************************************************************************************
\brief Loads the mesh data required for this training course into vertex buffer objects
***********************************************************************************************************************/
void VulkanPVRScopeExample::loadVbos()
{
	utils::appendSingleBuffersFromModel(getGraphicsContext(), *scene,  apiObj->vbos, apiObj->ibos);
}

void VulkanPVRScopeExample::updateMVPMatrix(uint32 swapChain)
{
	glm::mat4 instance1, instance2;
	instance1 = glm::translate(glm::vec3(0.0f, -1.0f, 0.0f)) * glm::rotate((angleY), glm::vec3(0.f, 1.f, 0.f)) *
	            glm::translate(glm::vec3(.5f, 0.f, -1.0f)) * glm::scale(glm::vec3(0.5f, 0.5f, 0.5f)) * scene->getWorldMatrix(0);

	//Create two instances of the mesh, offset to the sides.
	instance2 = viewMtx * instance1 * glm::translate(glm::vec3(0, 0, -2000));
	instance1 = viewMtx * instance1 * glm::translate(glm::vec3(0, 0, 2000));

	// update the angle for the next frame.
	angleY += (2 * glm::pi<glm::float32>() * getFrameTime() / 1000) / 10;
	utils::StructuredMemoryView& memView = apiObj->mvpUboView;
	memView.map(swapChain);
	memView.setArrayValue(MvpUboElements::MVP, 0, projMtx * instance1);
	memView.setArrayValue(MvpUboElements::MVIT, 0, glm::mat3x4(glm::inverseTranspose(glm::mat3(instance1))));

	memView.setArrayValue(MvpUboElements::MVP, 1, projMtx * instance2);
	memView.setArrayValue(MvpUboElements::MVIT, 1, glm::mat3x4(glm::inverseTranspose(glm::mat3(instance2))));
	memView.unmap(swapChain);

}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Code in initApplication() will be called by Shell once per run, before the rendering context is created.
	    Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes,etc.)
	    If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
Result VulkanPVRScopeExample::initApplication()
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
	apiObj.reset(new ApiObjects());
	apiObj->assetStore.init(*this);
	// Load the scene
	if (!apiObj->assetStore.loadModel(SceneFile, scene))
	{
		this->setExitMessage("ERROR: Couldn't load the .pod file\n");
		return Result::NotInitialized;
	}

	// Process the command line
	{
		const platform::CommandLine cmdline = getCommandLine();
		cmdline.getIntOption("-counter", selectedCounter);
		cmdline.getIntOption("-interval", interval);
	}
	return Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in quitApplication() will be called by Shell once per run, just before exiting
	    the program. If the rendering context is lost, quitApplication() will not be called.x
***********************************************************************************************************************/
Result VulkanPVRScopeExample::quitApplication()
{
	return Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
	   Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
Result VulkanPVRScopeExample::initView()
{
	apiObj->context = getGraphicsContext();
	// create the default fbo using default params
	apiObj->onScreenFbo = apiObj->context->createOnScreenFboSet();

	//set up the material
	materialData.specExponent = 100.f; // Width of the specular highlights (High exponent for small shiny highlights)
	materialData.albedoMod = glm::vec3(.78f, .82f, 1.f); // Overall color
	materialData.metalicity = 1.f; // Doesn't make much of a difference in this material.
	materialData.reflectivity = .2f; // Low reflectivity - color mostly diffuse.
	materialData.lightDirView = glm::normalize(glm::vec3(1.f, 1.f, -1.f)); 	// Set light direction in model space

	// Initialize VBO data
	loadVbos();

	// Load and compile the shaders & link programs
	if (!createPipeline())
	{
		return Result::NotInitialized;
	}

	if (!createUboDescriptorSet())
	{
		return Result::NotInitialized;
	}

	// Load textures
	if (!createTexSamplerDescriptorSet())
	{
		return Result::NotInitialized;
	}

	// Initialize UIRenderer
	if (apiObj->uiRenderer.init(apiObj->onScreenFbo[0]->getRenderPass(), 0) != Result::Success)
	{
		this->setExitMessage("ERROR: Cannot initialize UIRenderer\n");
		return Result::NotInitialized;
	}

	// Calculate the projection and view matrices
	// Is the screen rotated?
	const bool isRotate = this->isScreenRotated() && this->isFullScreen();
	projMtx = math::perspectiveFov(getApiType(), glm::pi<float32>() / 6, (float32)getWidth(),
	                               (float32)this->getHeight(), scene->getCamera(0).getNear(),
	                               scene->getCamera(0).getFar(), (isRotate ? glm::pi<float32>() * .5f : 0.0f));

	viewMtx = glm::lookAt(glm::vec3(0, 0, 75), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));


	// Initialize the graphing code
	std::string errorStr;
	if (!apiObj->scopeGraph.init(getGraphicsContext(), *this, apiObj->uiRenderer, apiObj->onScreenFbo[0]->getRenderPass(), errorStr))
	{
		setExitMessage(errorStr.c_str());
		return Result::NotInitialized;
	}

	if (apiObj->scopeGraph.isInitialized())
	{
		// Position the graph
		apiObj->scopeGraph.position(getWidth(), getHeight(), Rectanglei((static_cast<pvr::uint32>(getWidth() * 0.02f)), (static_cast<pvr::uint32>(getHeight() * 0.02f)),
		                            (static_cast<pvr::uint32>(getWidth() * 0.96f)), (static_cast<pvr::uint32>(getHeight() * 0.96f) / 3)));

		// Output the current active group and a list of all the counters
		Log(Log.Information, "PVRScope Number of Hardware Counters: %i\n", apiObj->scopeGraph.getCounterNum());
		Log(Log.Information, "Counters\n-ID---Name-------------------------------------------\n");

		for (uint32 i = 0; i < apiObj->scopeGraph.getCounterNum(); ++i)
		{
			Log(Log.Information, "[%2i] %s %s\n", i, apiObj->scopeGraph.getCounterName(i),
			    apiObj->scopeGraph.isCounterPercentage(i) ? "percentage" : "absolute");

			apiObj->scopeGraph.showCounter(i, false);
		}

		apiObj->scopeGraph.ping(1);
		// Tell the graph to show initial counters
		apiObj->scopeGraph.showCounter(apiObj->scopeGraph.getStandard3DIndex(), true);
		apiObj->scopeGraph.showCounter(apiObj->scopeGraph.getStandardTAIndex(), true);
		apiObj->scopeGraph.showCounter(apiObj->scopeGraph.getStandardShaderPixelIndex(), true);
		apiObj->scopeGraph.showCounter(apiObj->scopeGraph.getStandardShaderVertexIndex(), true);
		for (uint32 i = 0; i < apiObj->scopeGraph.getCounterNum(); ++i)
		{
			std::string s(std::string(apiObj->scopeGraph.getCounterName(i))); //Better safe than sorry - get a copy...
			strings::toLower(s);
			if (strings::startsWith(s, "hsr efficiency"))
			{
				apiObj->scopeGraph.showCounter(i, true);
			}
			if (strings::startsWith(s, "shaded pixels per second"))
			{
				apiObj->scopeGraph.showCounter(i, true);
			}
		}
		// Set the update interval: number of updates [frames] before updating the graph
		apiObj->scopeGraph.setUpdateInterval(interval);
	}
	apiObj->uiRenderer.getDefaultTitle()->setText("PVRScopeExample");
	apiObj->uiRenderer.getDefaultTitle()->commitUpdates();

	// create the commandbuffer
	for (uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		apiObj->secCmdBuffer[i] = apiObj->context->createSecondaryCommandBufferOnDefaultPool();
		apiObj->commandBuffer[i] = apiObj->context->createCommandBufferOnDefaultPool();
	}

	return Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
Result VulkanPVRScopeExample::releaseView()
{
	apiObj.reset();
	scene.reset();
	return Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
Result VulkanPVRScopeExample::renderFrame()
{
	updateMVPMatrix(getSwapChainIndex());
	apiObj->scopeGraph.ping(static_cast<pvr::float32>(getFrameTime()));
	updateDescription();
	recordCommandBuffer(getSwapChainIndex());
	apiObj->commandBuffer[getSwapChainIndex()]->beginRecording();
	apiObj->commandBuffer[getSwapChainIndex()]->beginRenderPass(apiObj->onScreenFbo[getSwapChainIndex()],
	    Rectanglei(0, 0, getWidth(), getHeight()),  false, glm::vec4(0.00, 0.70, 0.67, 1.0f));
	apiObj->commandBuffer[getSwapChainIndex()]->enqueueSecondaryCmds(apiObj->secCmdBuffer[getSwapChainIndex()]);
	apiObj->commandBuffer[getSwapChainIndex()]->endRenderPass();
	apiObj->commandBuffer[getSwapChainIndex()]->endRecording();
	apiObj->commandBuffer[getSwapChainIndex()]->submit();
	return Result::Success;
}

/*!*********************************************************************************************************************
\param nodeIndex Node index of the mesh to draw
\brief Draws a Model::Mesh after the model view matrix has been set and the material prepared.
***********************************************************************************************************************/
void VulkanPVRScopeExample::drawMesh(int nodeIndex, api::SecondaryCommandBuffer& cmd)
{
	const assets::Model::Node& node = scene->getNode(nodeIndex);
	const assets::Mesh& mesh = scene->getMesh(node.getObjectId());

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
		for (int32 i = 0; i < (int32)mesh.getNumStrips(); ++i)
		{
			int offset = 0;
			if (apiObj->ibos[node.getObjectId()].isValid())
			{
				// Indexed Triangle strips
				cmd->bindIndexBuffer(apiObj->ibos[node.getObjectId()], 0, mesh.getFaces().getDataType());
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
void VulkanPVRScopeExample::recordCommandBuffer(uint32 swapChain)
{
	api::SecondaryCommandBuffer& cmd = apiObj->secCmdBuffer[swapChain];
	cmd->beginRecording(apiObj->onScreenFbo[swapChain]);
	// Use shader program
	cmd->bindPipeline(apiObj->pipeline);

	// Bind the descriptors
	cmd->bindDescriptorSet(apiObj->pipeline->getPipelineLayout(), 1, apiObj->texSamplerDescriptor, 0);
	cmd->bindDescriptorSet(apiObj->pipeline->getPipelineLayout(), 2, apiObj->materialDescriptor[swapChain], 0);

	// draw the first instance
	uint32 offset = apiObj->mvpUboView.getAlignedElementArrayOffset(0);
	cmd->bindDescriptorSet(apiObj->pipeline->getPipelineLayout(), 0, apiObj->mvpDescriptor[swapChain], &offset, 1);
	drawMesh(0, cmd);

	// draw the second instance
	offset = apiObj->mvpUboView.getAlignedElementArrayOffset(1);
	cmd->bindDescriptorSet(apiObj->pipeline->getPipelineLayout(), 0, apiObj->mvpDescriptor[swapChain], &offset, 1);
	drawMesh(0, cmd);

	apiObj->scopeGraph.recordCommandBuffer(cmd, swapChain);
	updateDescription();
	apiObj->uiRenderer.beginRendering(cmd);
	apiObj->uiRenderer.getDefaultTitle()->render();
	apiObj->uiRenderer.getDefaultDescription()->render();
	apiObj->uiRenderer.getSdkLogo()->render();
	apiObj->scopeGraph.recordUIElements();
	apiObj->uiRenderer.endRendering();
	cmd->endRecording();
}

/*!*********************************************************************************************************************
\brief	Update the description
***********************************************************************************************************************/
void VulkanPVRScopeExample::updateDescription()
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
		        isKilos ? kilo : isPercentage ? percentage : standard,
		        selectedCounter,
		        apiObj->scopeGraph.getCounterName(selectedCounter),
		        apiObj->scopeGraph.isCounterShown(selectedCounter) ? "Yes" : "No",
		        userY,
		        maximum);
		apiObj->uiRenderer.getDefaultDescription()->setColor(glm::vec4(1.f));
	}
	else
	{
		sprintf(description, "No counters present");
		apiObj->uiRenderer.getDefaultDescription()->setColor(glm::vec4(.8f, 0.0f, 0.0f, 1.0f));
	}
	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroUIRenderer
	apiObj->uiRenderer.getDefaultDescription()->setText(description);
	apiObj->uiRenderer.getDefaultDescription()->commitUpdates();
}

/*!*********************************************************************************************************************
\return auto ptr to the demo supplied by the user
\brief	This function must be implemented by the user of the shell. The user should return its Shell object defining the
		behavior of the application.
***********************************************************************************************************************/
std::auto_ptr<Shell> pvr::newDemo() { return std::auto_ptr<Shell>(new VulkanPVRScopeExample()); }
