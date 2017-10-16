/*!*********************************************************************************************************************
\File         VulkanPVRScopeRemote.cpp
\Title        PVRScopeRemote
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Shows how to use our example PVRScope graph code.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVREngineUtils/PVREngineUtils.h"
#include "PVRScopeComms.h"
using namespace pvr;
// Source and binary shaders
const char FragShaderSrcFile[] = "FragShader_vk.fsh.spv";
const char VertShaderSrcFile[] = "VertShader_vk.vsh.spv";

// PVR texture files
const char TextureFile[] = "Marble.pvr";

// POD scene files
const char SceneFile[] = "scene.pod";
enum { MaxSwapChains = 8 };
namespace CounterDefs {
enum Enum
{
	Counter, Counter10, NumCounter
};
}

namespace UboMvpElements {
enum Enum { MVP, MVIT, Count };
std::pair<StringHash, types::GpuDatatypes::Enum> Mappings [] =
{
	{"MVP", types::GpuDatatypes::mat4x4 }, { "MVIT", types::GpuDatatypes::mat3x3 }
};
}


namespace PipelineConfigs {
enum DescriptorSetId { DescriptorUbo, DescriptorMaterial, DescriptorCount }; // Pipeline Descriptor sets
enum MaterialBindingId { MaterialBindingTex, MaterialBindingData, MaterialBindingCount }; // Material Descritpor set bindings
}

namespace UboMaterialElements {
enum Enum { AlbdeoModulation, SpecularExponent, Metallicity, Reflectivity, Count };

std::pair<StringHash, types::GpuDatatypes::Enum> Mappings[] =
{
	{"AlbdeoModulation", types::GpuDatatypes::vec3},
	{"SpecularExponent", types::GpuDatatypes::float32},
	{"Metallicity", types::GpuDatatypes::float32},
	{"Reflectivity", types::GpuDatatypes::float32}
};
}

const char* FrameDefs[CounterDefs::NumCounter] = { "Frames", "Frames10" };
using namespace pvr;
/*!*********************************************************************************************************************
\brief Class implementing the PVRShell functions.
***********************************************************************************************************************/
class VulkanPVRScopeRemote : public Shell
{
	struct ApiObjects
	{
		api::GraphicsPipeline	pipeline;
		api::TextureView texture;
		std::vector<api::Buffer> vbos;
		std::vector<api::Buffer> ibos;
		std::vector<api::CommandBuffer> commandBuffer;

		utils::StructuredMemoryView uboMVP;
		utils::StructuredMemoryView uboMaterial;

		api::DescriptorSet uboMvpDesc[MaxSwapChains];
		api::DescriptorSet uboMatDesc;

		api::DescriptorSetLayout descriptorSetLayout;
		api::FboSet onScreenFbo;
		ui::UIRenderer uiRenderer;
		utils::AssetStore assetStore;

		// 3D Model
		assets::ModelHandle scene;
		GraphicsContext context;
	};
	std::auto_ptr<ApiObjects> apiObj;

	glm::mat4 projectionMtx;
	glm::mat4 viewMtx;

	struct UboMaterialData
	{
		glm::vec3	albedo;
		float32 specularExponent;
		float32 metallicity;
		float32 reflectivity;
		bool isDirty;
	} uboMatData;

	// The translation and Rotate parameter of Model
	float32 angleY;

	// Data connection to PVRPerfServer
	bool hasCommunicationError;
	SSPSCommsData*	spsCommsData;
	SSPSCommsLibraryTypeFloat commsLibSpecularExponent;
	SSPSCommsLibraryTypeFloat commsLibMetallicity;
	SSPSCommsLibraryTypeFloat commsLibReflectivity;
	SSPSCommsLibraryTypeFloat commsLibAlbedoR;
	SSPSCommsLibraryTypeFloat commsLibAlbedoG;
	SSPSCommsLibraryTypeFloat commsLibAlbedoB;
	uint32 frameCounter;
	uint32 frame10Counter;
	uint32 counterReadings[CounterDefs::NumCounter];
public:
	virtual Result initApplication();
	virtual Result initView();
	virtual Result releaseView();
	virtual Result quitApplication();
	virtual Result renderFrame();
	void recordCommandBuffer(uint32 swapChain);
	bool createMaterialDescriptorSet();
	bool createPipeline();
	void loadVbos();
	void drawMesh(int i32NodeIndex, api::CommandBuffer& cmd);
	bool createDescriptorSet();
	void updateUbo(uint32 swapChain);
};

/*!*********************************************************************************************************************
\return	Return true if no error occurred
\brief	Loads the textures required for this training course
***********************************************************************************************************************/
bool VulkanPVRScopeRemote::createMaterialDescriptorSet()
{

	return true;
}

/*!*********************************************************************************************************************
\return	Return true if no error occurred
\brief	Loads and compiles the shaders and links the shader programs required for this training course
***********************************************************************************************************************/
bool VulkanPVRScopeRemote::createPipeline()
{
	//Mapping of mesh semantic names to shader variables
	utils::VertexBindings_Name vertexBindings[] =
	{
		{ "POSITION", "inVertex" },
		{ "NORMAL", "inNormal" },
		{ "UV0", "inTexCoord" }
	};

	CPPLProcessingScoped PPLProcessingScoped(spsCommsData, __FUNCTION__, static_cast<uint32>(strlen(__FUNCTION__)), frameCounter);

	api::GraphicsPipelineCreateParam pipeDesc;
	api::PipelineLayoutCreateParam pipeLayoutInfo;
	pipeLayoutInfo
	.setDescSetLayout(PipelineConfigs::DescriptorUbo, apiObj->context->createDescriptorSetLayout(
	                    api::DescriptorSetLayoutCreateParam()
	                    .setBinding(0, types::DescriptorType::UniformBuffer, 1, types::ShaderStageFlags::Vertex)))
	.setDescSetLayout(PipelineConfigs::DescriptorMaterial, apiObj->context->createDescriptorSetLayout(
	                    api::DescriptorSetLayoutCreateParam()
	                    .setBinding(PipelineConfigs::MaterialBindingTex, types::DescriptorType::CombinedImageSampler,
	                                1, types::ShaderStageFlags::Fragment)
	                    .setBinding(PipelineConfigs::MaterialBindingData, types::DescriptorType::UniformBuffer, 1,
	                                types::ShaderStageFlags::Fragment)));

	pipeDesc.pipelineLayout = apiObj->context->createPipelineLayout(pipeLayoutInfo);
	if (!pipeDesc.pipelineLayout.isValid())
	{
		setExitMessage("Failed to create the pipeline layout");
		return false;
	}

	/* Load and compile the shaders from files. */
	pipeDesc.vertexShader.setShader(apiObj->context->createShader(*getAssetStream(VertShaderSrcFile),
	                                types::ShaderType::VertexShader));

	pipeDesc.fragmentShader.setShader(apiObj->context->createShader(*getAssetStream(FragShaderSrcFile),
	                                  types::ShaderType::FragmentShader));

	pipeDesc.rasterizer.setCullFace(pvr::types::Face::Back);
	pipeDesc.depthStencil.setDepthTestEnable(true);
	pipeDesc.depthStencil.setDepthCompareFunc(pvr::types::ComparisonMode::Less);
	pipeDesc.depthStencil.setDepthWrite(true);
	pipeDesc.colorBlend.setAttachmentState(0, types::BlendingConfig());
	pipeDesc.renderPass = apiObj->onScreenFbo[0]->getRenderPass();
	utils::createInputAssemblyFromMesh(apiObj->scene->getMesh(0), vertexBindings, 3, pipeDesc);
	apiObj->pipeline = apiObj->context->createGraphicsPipeline(pipeDesc);
	if (!apiObj->pipeline.isValid()) { setExitMessage("Failed to create the Pipeline "); return false; }
	return true;
}

/*!*********************************************************************************************************************
\brief	Loads the mesh data required for this training course into vertex buffer objects
***********************************************************************************************************************/
void VulkanPVRScopeRemote::loadVbos()
{
	CPPLProcessingScoped PPLProcessingScoped(spsCommsData, __FUNCTION__,
	    static_cast<uint32>(strlen(__FUNCTION__)), frameCounter);

	//	Load vertex data of all meshes in the scene into VBOs
	//	The meshes have been exported with the "Interleave Vectors" option,
	//	so all data is interleaved in the buffer at pMesh->pInterleaved.
	//	Interleaving data improves the memory access pattern and cache efficiency,
	//	thus it can be read faster by the hardware.
	utils::appendSingleBuffersFromModel(getGraphicsContext(), *apiObj->scene, apiObj->vbos, apiObj->ibos);
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in initApplication() will be called by Shell once per run, before the rendering context is created.
		Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
		If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
Result VulkanPVRScopeRemote::initApplication()
{
	apiObj.reset(new ApiObjects());
	apiObj->assetStore.init(*this);
	// Load the scene
	if (!apiObj->assetStore.loadModel(SceneFile, apiObj->scene))
	{
		this->setExitMessage("ERROR: Couldn't load the .pod file\n");
		return Result::NotInitialized;
	}
	// We want a data connection to PVRPerfServer
	{
		spsCommsData = pplInitialise("PVRScopeRemote", 14);
		hasCommunicationError = false;

		// Demonstrate that there is a good chance of the initial data being
		// lost - the connection is normally completed asynchronously.
		pplSendMark(spsCommsData, "lost", static_cast<uint32>(strlen("lost")));

		// This is entirely optional. Wait for the connection to succeed, it will
		// timeout if e.g. PVRPerfServer is not running.
		int isConnected;
		pplWaitForConnection(spsCommsData, &isConnected, 1, 200);
	}
	CPPLProcessingScoped PPLProcessingScoped(spsCommsData, __FUNCTION__, static_cast<uint32>(strlen(__FUNCTION__)), frameCounter);

	uboMatData.specularExponent = 5.f;            // Width of the specular highlights (using low exponent for a brushed metal look)
	uboMatData.albedo = glm::vec3(1.f, .77f, .33f); // Overall color
	uboMatData.metallicity = 1.f;                 // Is the color of the specular white (nonmetallic), or coloured by the object(metallic)
	uboMatData.reflectivity = .8f;                // Percentage of contribution of diffuse / specular
	uboMatData.isDirty = true;
	frameCounter = 0;
	frame10Counter = 0;

	// Get and set the read path for content files
	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	// Load the scene
	if (!apiObj->assetStore.loadModel(SceneFile, apiObj->scene))
	{
		this->setExitMessage("ERROR: Couldn't load the .pod file\n");
		return Result::NotInitialized;
	}

	// set angle of rotation
	angleY = 0.0f;

	//	Remotely editable library items
	if (spsCommsData)
	{
		std::vector<SSPSCommsLibraryItem> communicableItems;

		// Editable: Specular Exponent
		communicableItems.push_back(SSPSCommsLibraryItem());
		commsLibSpecularExponent.fCurrent = uboMatData.specularExponent;
		commsLibSpecularExponent.fMin = 1.1f;
		commsLibSpecularExponent.fMax = 300.0f;
		communicableItems.back().pszName = "Specular Exponent";
		communicableItems.back().nNameLength = (uint32)strlen(communicableItems.back().pszName);
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&commsLibSpecularExponent;
		communicableItems.back().nDataLength = sizeof(commsLibSpecularExponent);

		communicableItems.push_back(SSPSCommsLibraryItem());
		// Editable: Metallicity
		commsLibMetallicity.fCurrent = uboMatData.metallicity;
		commsLibMetallicity.fMin = 0.0f;
		commsLibMetallicity.fMax = 1.0f;
		communicableItems.back().pszName = "Metallicity";
		communicableItems.back().nNameLength = (uint32)strlen(communicableItems.back().pszName);
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&commsLibMetallicity;
		communicableItems.back().nDataLength = sizeof(commsLibMetallicity);

		// Editable: Reflectivity
		communicableItems.push_back(SSPSCommsLibraryItem());
		commsLibReflectivity.fCurrent = uboMatData.reflectivity;
		commsLibReflectivity.fMin = 0.;
		commsLibReflectivity.fMax = 1.;
		communicableItems.back().pszName = "Reflectivity";
		communicableItems.back().nNameLength = (uint32)strlen(communicableItems.back().pszName);
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&commsLibReflectivity;
		communicableItems.back().nDataLength = sizeof(commsLibReflectivity);

		// Editable: Albedo R channel
		communicableItems.push_back(SSPSCommsLibraryItem());
		commsLibAlbedoR.fCurrent = uboMatData.albedo.r;
		commsLibAlbedoR.fMin = 0.0f;
		commsLibAlbedoR.fMax = 1.0f;
		communicableItems.back().pszName = "Albedo R";
		communicableItems.back().nNameLength = (uint32)strlen(communicableItems.back().pszName);
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&commsLibAlbedoR;
		communicableItems.back().nDataLength = sizeof(commsLibAlbedoR);

		// Editable: Albedo R channel
		communicableItems.push_back(SSPSCommsLibraryItem());
		commsLibAlbedoG.fCurrent = uboMatData.albedo.g;
		commsLibAlbedoG.fMin = 0.0f;
		commsLibAlbedoG.fMax = 1.0f;
		communicableItems.back().pszName = "Albedo G";
		communicableItems.back().nNameLength = (uint32)strlen(communicableItems.back().pszName);
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&commsLibAlbedoG;
		communicableItems.back().nDataLength = sizeof(commsLibAlbedoG);

		// Editable: Albedo R channel
		communicableItems.push_back(SSPSCommsLibraryItem());
		commsLibAlbedoB.fCurrent = uboMatData.albedo.b;
		commsLibAlbedoB.fMin = 0.0f;
		commsLibAlbedoB.fMax = 1.0f;
		communicableItems.back().pszName = "Albedo B";
		communicableItems.back().nNameLength = (uint32)strlen(communicableItems.back().pszName);
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&commsLibAlbedoB;
		communicableItems.back().nDataLength = sizeof(commsLibAlbedoB);

		// Ok, submit our library
		if (!pplLibraryCreate(spsCommsData, communicableItems.data(), static_cast<pvr::uint32>(communicableItems.size())))
		{
			Log(Log.Debug, "PVRScopeRemote: pplLibraryCreate() failed\n");
		}

		// User defined counters
		SSPSCommsCounterDef counterDefines[CounterDefs::NumCounter];
		for (uint32 i = 0; i < CounterDefs::NumCounter; ++i)
		{
			counterDefines[i].pszName = FrameDefs[i];
			counterDefines[i].nNameLength = (uint32)strlen(FrameDefs[i]);
		}

		if (!pplCountersCreate(spsCommsData, counterDefines, CounterDefs::NumCounter))
		{
			Log(Log.Debug, "PVRScopeRemote: pplCountersCreate() failed\n");
		}
	}
	return Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in quitApplication() will be called by Shell once per run, just before exiting the program.
		If the rendering context is lost, QuitApplication() will not be called.
***********************************************************************************************************************/
Result VulkanPVRScopeRemote::quitApplication()
{
	if (spsCommsData)
	{
		hasCommunicationError |= !pplSendProcessingBegin(spsCommsData, __FUNCTION__,
		                         static_cast<uint32>(strlen(__FUNCTION__)), frameCounter);

		// Close the data connection to PVRPerfServer
		for (uint32 i = 0; i < 40; ++i)
		{
			char buf[128];
			const int nLen = sprintf(buf, "test %u", i);
			hasCommunicationError |= !pplSendMark(spsCommsData, buf, nLen);
		}
		hasCommunicationError |= !pplSendProcessingEnd(spsCommsData);
		pplShutdown(spsCommsData);
	}
	return Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
		Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
Result VulkanPVRScopeRemote::initView()
{
	apiObj->context = getGraphicsContext();
	CPPLProcessingScoped PPLProcessingScoped(spsCommsData, __FUNCTION__, static_cast<uint32>(strlen(__FUNCTION__)), frameCounter);
	apiObj->onScreenFbo = apiObj->context->createOnScreenFboSet();
	apiObj->commandBuffer.resize(getSwapChainLength());

	//	Initialize VBO data
	loadVbos();

	if (!createPipeline()) {  return Result::NotInitialized; }

	//	Load textures
	if (!createMaterialDescriptorSet()) { return Result::NotInitialized; }

	// create the pipeline
	if (!createDescriptorSet()) { return Result::NotInitialized; }

	//	Initialize the UI Renderer
	if (apiObj->uiRenderer.init(apiObj->onScreenFbo[0]->getRenderPass(), 0) != Result::Success)
	{
		this->setExitMessage("ERROR: Cannot initialize UIRenderer\n");
		return Result::NotInitialized;
	}

	// create the pvrscope connection pass and fail text
	apiObj->uiRenderer.getDefaultTitle()->setText("PVRScopeRemote");
	apiObj->uiRenderer.getDefaultTitle()->commitUpdates();

	apiObj->uiRenderer.getDefaultDescription()->setScale(glm::vec2(.5, .5));
	apiObj->uiRenderer.getDefaultDescription()->setText("Use PVRTune to remotely control the parameters of this application.");
	apiObj->uiRenderer.getDefaultDescription()->commitUpdates();

	// Calculate the projection and view matrices
	// Is the screen rotated?
	bool isRotated = this->isScreenRotated() && this->isFullScreen();
	viewMtx = glm::lookAt(glm::vec3(0.f, 0.f, 75.f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	projectionMtx = math::perspectiveFov(getApiType(), glm::pi<float32>() / 6, (float32)getWidth(),
	                                     (float32)getHeight(), apiObj->scene->getCamera(0).getNear(),
	                                     apiObj->scene->getCamera(0).getFar(), isRotated ? glm::pi<float32>() * .5f : 0.0f);

	for (uint32 i = 0; i < getSwapChainLength(); ++i) {	recordCommandBuffer(i);	}
	return Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief	Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
Result VulkanPVRScopeRemote::releaseView()
{
	CPPLProcessingScoped PPLProcessingScoped(spsCommsData, __FUNCTION__, static_cast<uint32>(strlen(__FUNCTION__)), frameCounter);
	// Release UIRenderer
	apiObj->uiRenderer.release();
	apiObj->assetStore.releaseAll();
	apiObj.reset();
	return Result::Success;
}

/*!*********************************************************************************************************************
\return	Return Result::Success if no error occurred
\brief Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
Result VulkanPVRScopeRemote::renderFrame()
{
	CPPLProcessingScoped PPLProcessingScoped(spsCommsData, __FUNCTION__, static_cast<uint32>(strlen(__FUNCTION__)), frameCounter);
	bool currCommunicationErr = hasCommunicationError;
	if (spsCommsData)
	{
		// mark every N frames
		if (!(frameCounter % 100))
		{
			char buf[128];
			const int nLen = sprintf(buf, "frame %u", frameCounter);
			hasCommunicationError |= !pplSendMark(spsCommsData, buf, nLen);
		}

		// Check for dirty items
		hasCommunicationError |= !pplSendProcessingBegin(spsCommsData, "dirty", static_cast<uint32>(strlen("dirty")), frameCounter);
		{
			uint32 nItem, nNewDataLen;
			const char* pData;
			bool recompile = false;
			while (pplLibraryDirtyGetFirst(spsCommsData, &nItem, &nNewDataLen, &pData))
			{
				Log(Log.Debug, "dirty item %u %u 0x%08x\n", nItem, nNewDataLen, pData);
				switch (nItem)
				{
				case 0:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						uboMatData.specularExponent = psData->fCurrent;
						uboMatData.isDirty = true;
						Log(Log.Information, "Setting Specular Exponent to value [%6.2f]", uboMatData.specularExponent);
					}
					break;
				case 1:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						uboMatData.metallicity = psData->fCurrent;
						uboMatData.isDirty = true;
						Log(Log.Information, "Setting Metallicity to value [%3.2f]", uboMatData.metallicity);
					}
					break;
				case 2:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						uboMatData.reflectivity = psData->fCurrent;
						uboMatData.isDirty = true;
						Log(Log.Information, "Setting Reflectivity to value [%3.2f]", uboMatData.reflectivity);
					}
					break;
				case 3:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						uboMatData.albedo.r = psData->fCurrent;
						uboMatData.isDirty = true;
						Log(Log.Information, "Setting Albedo Red channel to value [%3.2f]", uboMatData.albedo.r);
					}
					break;
				case 4:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						uboMatData.albedo.g = psData->fCurrent;
						uboMatData.isDirty = true;
						Log(Log.Information, "Setting Albedo Green channel to value [%3.2f]", uboMatData.albedo.g);
					}
					break;
				case 5:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						uboMatData.albedo.b = psData->fCurrent;
						uboMatData.isDirty = true;
						Log(Log.Information, "Setting Albedo Blue channel to value [%3.2f]", uboMatData.albedo.b);
					}
					break;
				}
			}

			if (recompile)
			{
				Log(Log.Error, "*** Could not recompile the shaders passed from PVRScopeCommunication ****");
			}
		}
		hasCommunicationError |= !pplSendProcessingEnd(spsCommsData);
	}

	if (spsCommsData)
	{
		hasCommunicationError |= !pplSendProcessingBegin(spsCommsData, "draw", static_cast<uint32>(strlen("draw")), frameCounter);
	}

	updateUbo(getSwapChainIndex());

	// Set eye position in model space
	// Now that the uniforms are set, call another function to actually draw the mesh.
	if (spsCommsData)
	{
		hasCommunicationError |= !pplSendProcessingEnd(spsCommsData);
		hasCommunicationError |= !pplSendProcessingBegin(spsCommsData, "Print3D",
		                         static_cast<uint32>(strlen("Print3D")), frameCounter);
	}

	if (hasCommunicationError)
	{
		apiObj->uiRenderer.getDefaultControls()->setText("Communication Error:\nPVRScopeComms failed\n"
		    "Is PVRPerfServer connected?");
		apiObj->uiRenderer.getDefaultControls()->setColor(glm::vec4(.8f, .3f, .3f, 1.0f));
		apiObj->uiRenderer.getDefaultControls()->commitUpdates();
		hasCommunicationError = false;
	}
	else
	{
		apiObj->uiRenderer.getDefaultControls()->setText("PVRScope Communication established.");
		apiObj->uiRenderer.getDefaultControls()->setColor(glm::vec4(1.f));
		apiObj->uiRenderer.getDefaultControls()->commitUpdates();
	}

	if (spsCommsData) { hasCommunicationError |= !pplSendProcessingEnd(spsCommsData); }

	// send counters
	counterReadings[CounterDefs::Counter] = frameCounter;
	counterReadings[CounterDefs::Counter10] = frame10Counter;
	if (spsCommsData) { hasCommunicationError |= !pplCountersUpdate(spsCommsData, counterReadings); }

	// update some counters
	++frameCounter;
	if (0 == (frameCounter / 10) % 10) { frame10Counter += 10; }
	apiObj->commandBuffer[getSwapChainIndex()]->submit();
	return Result::Success;
}

/*!*********************************************************************************************************************
\brief	Draws a assets::Mesh after the model view matrix has been set and the material prepared.
\param	nodeIndex Node index of the mesh to draw
***********************************************************************************************************************/
void VulkanPVRScopeRemote::drawMesh(int nodeIndex, api::CommandBuffer& cmd)
{
	CPPLProcessingScoped PPLProcessingScoped(spsCommsData, __FUNCTION__, static_cast<uint32>(strlen(__FUNCTION__)), frameCounter);

	int32 meshIndex = apiObj->scene->getNode(nodeIndex).getObjectId();
	const assets::Mesh& mesh = apiObj->scene->getMesh(meshIndex);
	// bind the VBO for the mesh
	cmd->bindVertexBuffer(apiObj->vbos[meshIndex], 0, 0);

	//	The geometry can be exported in 4 ways:
	//	- Indexed Triangle list
	//	- Non-Indexed Triangle list
	//	- Indexed Triangle strips
	//	- Non-Indexed Triangle strips
	if (mesh.getNumStrips() == 0)
	{
		if (apiObj->ibos[meshIndex].isValid())
		{
			// Indexed Triangle list
			cmd->bindIndexBuffer(apiObj->ibos[meshIndex], 0, types::IndexType::IndexType16Bit);
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
			if (apiObj->ibos[meshIndex].isValid())
			{
				cmd->bindIndexBuffer(apiObj->ibos[meshIndex], 0, types::IndexType::IndexType16Bit);

				// Indexed Triangle strips
				cmd->drawIndexed(0, mesh.getStripLength(i) + 2, offset * 2, 0, 1);
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

bool VulkanPVRScopeRemote::createDescriptorSet()
{
	CPPLProcessingScoped PPLProcessingScoped(spsCommsData, __FUNCTION__, static_cast<uint32>(strlen(__FUNCTION__)), frameCounter);
	// create the MVP ubo
	apiObj->uboMVP.addEntriesPacked(UboMvpElements::Mappings, UboMvpElements::Count);
	apiObj->uboMVP.finalize(apiObj->context, 1, types::BufferBindingUse::UniformBuffer, false, false);
	for (uint32 i = 0; i < apiObj->context->getSwapChainLength(); ++i)
	{
		apiObj->uboMVP.connectWithBuffer(i, apiObj->context->createBufferAndView(apiObj->uboMVP.getAlignedElementSize(),
		                                 types::BufferBindingUse::UniformBuffer, true));

		apiObj->uboMvpDesc[i] = apiObj->context->createDescriptorSetOnDefaultPool(
		                          apiObj->pipeline->getPipelineLayout()->getDescriptorSetLayout(0));

		if (!apiObj->uboMvpDesc[i]->update(api::DescriptorSetUpdate().setUbo(0, apiObj->uboMVP.getConnectedBuffer(i))))
		{
			setExitMessage("Failed to update the Model View ubo");
			return false;
		}
	}

	//--- create the material descriptor
	if (!apiObj->assetStore.getTextureWithCaching(getGraphicsContext(), TextureFile, &apiObj->texture, NULL))
	{
		setExitMessage("ERROR: Failed to load texture.");
		return false;
	}

	assets::SamplerCreateParam samplerDesc;
	samplerDesc.minificationFilter = types::SamplerFilter::Linear;
	samplerDesc.mipMappingFilter = types::SamplerFilter::Nearest;
	samplerDesc.magnificationFilter = types::SamplerFilter::Linear;
	api::Sampler bilinearSampler = apiObj->context->createSampler(samplerDesc);

	apiObj->uboMaterial.addEntriesPacked(UboMaterialElements::Mappings, UboMaterialElements::Count);
	apiObj->uboMaterial.finalize(apiObj->context, 1, types::BufferBindingUse::UniformBuffer, false, false);
	apiObj->uboMaterial.connectWithBuffer(0, apiObj->context->createBufferAndView(apiObj->uboMaterial.getAlignedElementSize(),
	                                      types::BufferBindingUse::UniformBuffer, true));

	apiObj->uboMatDesc = apiObj->context->createDescriptorSetOnDefaultPool(
	                       apiObj->pipeline->getPipelineLayout()->getDescriptorSetLayout(PipelineConfigs::DescriptorMaterial));

	if (!apiObj->uboMatDesc->update(api::DescriptorSetUpdate()
	                                .setCombinedImageSampler(PipelineConfigs::MaterialBindingTex, apiObj->texture, bilinearSampler)
	                                .setUbo(PipelineConfigs::MaterialBindingData, apiObj->uboMaterial.getConnectedBuffer(0))))
	{
		setExitMessage("Failed to update the material ubo");
		return false;
	}
	return true;
}

void VulkanPVRScopeRemote::updateUbo(uint32 swapChain)
{
	// Rotate and Translation the model matrix
	glm::mat4 modelMtx = glm::rotate(angleY, glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.6f))
	                     * apiObj->scene->getWorldMatrix(0);
	angleY += (2 * glm::pi<float32>() * getFrameTime() / 1000) / 10;

// Set model view projection matrix
	const glm::mat4 mvMatrix = viewMtx * modelMtx;

	apiObj->uboMVP.map(swapChain);
	apiObj->uboMVP.setValue(0, projectionMtx * mvMatrix);
	apiObj->uboMVP.setValue(1, glm::mat3x4(glm::inverseTranspose(glm::mat3(mvMatrix))));
	apiObj->uboMVP.unmap(swapChain);

	if (uboMatData.isDirty)
	{
		apiObj->context->waitIdle();
		apiObj->uboMaterial.map(0);
		apiObj->uboMaterial.setValue(UboMaterialElements::AlbdeoModulation, glm::vec4(uboMatData.albedo, 0.0f));
		apiObj->uboMaterial.setValue(UboMaterialElements::SpecularExponent, uboMatData.specularExponent);
		apiObj->uboMaterial.setValue(UboMaterialElements::Metallicity, uboMatData.metallicity);
		apiObj->uboMaterial.setValue(UboMaterialElements::Reflectivity, uboMatData.reflectivity);
		apiObj->uboMaterial.unmap(0);
		uboMatData.isDirty = false;
	}

}

/*!*********************************************************************************************************************
\brief	pre-record the rendering the commands
***********************************************************************************************************************/
void VulkanPVRScopeRemote::recordCommandBuffer(uint32 swapChain)
{
	CPPLProcessingScoped PPLProcessingScoped(spsCommsData, __FUNCTION__, static_cast<uint32>(strlen(__FUNCTION__)), frameCounter);
	apiObj->commandBuffer[swapChain] = apiObj->context->createCommandBufferOnDefaultPool();
	apiObj->commandBuffer[swapChain]->beginRecording();
	apiObj->commandBuffer[swapChain]->beginRenderPass(apiObj->onScreenFbo[swapChain],
	    Rectanglei(0, 0, getWidth(), getHeight()), true, glm::vec4(0.00, 0.70, 0.67, 1.0f));

	// Use shader program
	apiObj->commandBuffer[swapChain]->bindPipeline(apiObj->pipeline);

	// Bind texture
	apiObj->commandBuffer[swapChain]->bindDescriptorSet(apiObj->pipeline->getPipelineLayout(), 0, apiObj->uboMvpDesc[swapChain], 0);
	apiObj->commandBuffer[swapChain]->bindDescriptorSet(apiObj->pipeline->getPipelineLayout(), 1, apiObj->uboMatDesc, 0);

	drawMesh(0, apiObj->commandBuffer[swapChain]);

	apiObj->uiRenderer.beginRendering(apiObj->commandBuffer[swapChain]);
	// Displays the demo name using the tools. For a detailed explanation, see the example
	// IntroUIRenderer
	apiObj->uiRenderer.getDefaultTitle()->render();
	apiObj->uiRenderer.getDefaultDescription()->render();
	apiObj->uiRenderer.getSdkLogo()->render();
	apiObj->uiRenderer.getDefaultControls()->render();
	apiObj->uiRenderer.endRendering();
	apiObj->commandBuffer[swapChain]->endRenderPass();
	apiObj->commandBuffer[swapChain]->endRecording();
}

/*!*********************************************************************************************************************
\return	Return auto ptr to the demo supplied by the user
\brief	This function must be implemented by the user of the shell. The user should return its Shell object defining the behavior of the application.
***********************************************************************************************************************/
std::auto_ptr<Shell> pvr::newDemo() { return std::auto_ptr<Shell>(new VulkanPVRScopeRemote()); }
