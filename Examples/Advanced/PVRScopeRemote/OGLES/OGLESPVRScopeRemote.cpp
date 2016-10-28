/*!*********************************************************************************************************************
\File         OGLESPVRScopeRemote.cpp
\Title        PVRScopeRemote
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief      Shows how to use our example PVRScope graph code.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVRUIRenderer/PVRUIRenderer.h"
#include "PVRScopeComms.h"

// Source and binary shaders
const char FragShaderSrcFile[] = "FragShader.fsh";
const char VertShaderSrcFile[] = "VertShader.vsh";

// PVR texture files
const char TextureFile[] = "Marble.pvr";

// POD scene files
const char SceneFile[] = "scene.pod";
namespace CounterDefs {
enum Enum
{
	Counter, Counter10, NumCounter
};
}
const char* FrameDefs[CounterDefs::NumCounter] = { "Frames", "Frames10" };
using namespace pvr;
/*!*********************************************************************************************************************
\brief Class implementing the PVRShell functions.
***********************************************************************************************************************/
class OGLESPVRScopeRemote : public pvr::Shell
{
	struct DeviceResources
	{
		pvr::api::GraphicsPipeline pipeline;
		pvr::api::TextureView texture;
		std::vector<pvr::api::Buffer> vbos;
		std::vector<pvr::api::Buffer> ibos;
		pvr::api::DescriptorSet  descriptorSet;
		pvr::api::DescriptorSetLayout descriptorSetLayout;
		pvr::api::CommandBuffer commandBuffer;
		pvr::api::Fbo onScreenFbo;
	};
	std::auto_ptr<DeviceResources> m_deviceResource;
	// Print3D class used to display text
	pvr::ui::UIRenderer uiRenderer;

	// OpenGL handles for shaders, textures and VBOs
	pvr::GraphicsContext m_context;

	// 3D Model
	pvr::assets::ModelHandle scene;
	pvr::api::AssetStore assetStore;
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
		glm::mat4 mvpMatrix;
		glm::mat4 mvMatrix;
		glm::mat3 mvITMatrix;
		glm::vec3 lightDirView;
		pvr::float32 specularExponent;
		pvr::float32 metallicity;
		pvr::float32 reflectivity;
		glm::vec3    albedo;
	} progUniforms;

	// The translation and Rotate parameter of Model
	pvr::float32 angleY;

	// Data connection to PVRPerfServer
	bool hasCommunicationError;
	SSPSCommsData*  spsCommsData;
	SSPSCommsLibraryTypeFloat commsLibSpecularExponent;
	SSPSCommsLibraryTypeFloat commsLibMetallicity;
	SSPSCommsLibraryTypeFloat commsLibReflectivity;
	SSPSCommsLibraryTypeFloat commsLibAlbedoR;
	SSPSCommsLibraryTypeFloat commsLibAlbedoG;
	SSPSCommsLibraryTypeFloat commsLibAlbedoB;


	std::vector<char> vertShaderSrc;
	std::vector<char> fragShaderSrc;
	pvr::uint32 frameCounter;
	pvr::uint32 frame10Counter;
	pvr::uint32 counterReadings[CounterDefs::NumCounter];
public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();
	void recordCommandBuffer();
	bool createTexSamplerDescriptorSet();
	bool createPipeline(const char* const pszFrag, const char* const pszVert);
	void loadVbos();
	void drawMesh(int i32NodeIndex);
};

/*!*********************************************************************************************************************
\return Return true if no error occurred
\brief  Loads the textures required for this training course
***********************************************************************************************************************/
bool OGLESPVRScopeRemote::createTexSamplerDescriptorSet()
{
	CPPLProcessingScoped PPLProcessingScoped(spsCommsData, __FUNCTION__, static_cast<pvr::uint32>(strlen(__FUNCTION__)), frameCounter);

	if (!assetStore.getTextureWithCaching(getGraphicsContext(), TextureFile, &m_deviceResource->texture, NULL))
	{
		pvr::Log("ERROR: Failed to load texture.");
		return false;
	}

	pvr::assets::SamplerCreateParam samplerDesc;
	samplerDesc.minificationFilter = types::SamplerFilter::Linear;
	samplerDesc.mipMappingFilter = types::SamplerFilter::Nearest;
	samplerDesc.magnificationFilter = types::SamplerFilter::Linear;
	pvr::api::Sampler bilinearSampler = m_context->createSampler(samplerDesc);

	pvr::api::DescriptorSetLayoutCreateParam descSetLayoutInfo;
	descSetLayoutInfo.setBinding(0, types::DescriptorType::CombinedImageSampler, 1, types::ShaderStageFlags::Fragment);
	m_deviceResource->descriptorSetLayout = m_context->createDescriptorSetLayout(descSetLayoutInfo);

	pvr::api::DescriptorSetUpdate descriptorSetUpdate;
	descriptorSetUpdate.setCombinedImageSampler(0, m_deviceResource->texture, bilinearSampler);
	m_deviceResource->descriptorSet = m_context->createDescriptorSetOnDefaultPool(m_deviceResource->descriptorSetLayout);
	m_deviceResource->descriptorSet->update(descriptorSetUpdate);
	return true;
}

/*!*********************************************************************************************************************
\return Return true if no error occurred
\brief  Loads and compiles the shaders and links the shader programs required for this training course
***********************************************************************************************************************/
bool OGLESPVRScopeRemote::createPipeline(const char* const fragShaderSource, const char* const vertShaderSource)
{
	//Mapping of mesh semantic names to shader variables
	pvr::utils::VertexBindings_Name vertexBindings[] =
	{
		{ "POSITION", "inVertex" },
		{ "NORMAL", "inNormal" },
		{ "UV0", "inTexCoord" }
	};

	CPPLProcessingScoped PPLProcessingScoped(spsCommsData, __FUNCTION__, static_cast<pvr::uint32>(strlen(__FUNCTION__)), frameCounter);

	pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
	pipeLayoutInfo.addDescSetLayout(m_deviceResource->descriptorSetLayout);

	// set the pipeline configurations
	pvr::api::GraphicsPipelineCreateParam pipeDesc;
	/* Load and compile the shaders from files. */
	pvr::BufferStream vertexShaderStream("", vertShaderSource, strlen(vertShaderSource));
	pvr::BufferStream fragShaderStream("", fragShaderSource, strlen(fragShaderSource));
	pipeDesc.rasterizer.setCullFace(pvr::types::Face::Back).setFrontFaceWinding(pvr::types::PolygonWindingOrder::FrontFaceCCW);
	pipeDesc.depthStencil.setDepthTestEnable(true);
	pipeDesc.vertexShader.setShader(m_context->createShader(vertexShaderStream, types::ShaderType::VertexShader));
	pipeDesc.fragmentShader.setShader(m_context->createShader(fragShaderStream, types::ShaderType::FragmentShader));
	pipeDesc.pipelineLayout = m_context->createPipelineLayout(pipeLayoutInfo);
	pipeDesc.colorBlend.setAttachmentState(0, pvr::types::BlendingConfig());
	pvr::utils::createInputAssemblyFromMesh(scene->getMesh(0), vertexBindings, 3, pipeDesc);

	pvr::api::GraphicsPipeline tmpPipeline = m_context->createGraphicsPipeline(pipeDesc);
	pvr::Log(pvr::Log.Debug, "Created pipeline...");
	if (!tmpPipeline.isValid()) { pvr::Log(pvr::Log.Debug, "Pipeline Failure."); return false; }
	m_deviceResource->pipeline = tmpPipeline;
	pvr::Log(pvr::Log.Debug, "Pipeline Success.");

	// Set the sampler2D variable to the first texture unit
	m_deviceResource->commandBuffer->beginRecording();
	m_deviceResource->commandBuffer->bindPipeline(m_deviceResource->pipeline);

	m_deviceResource->commandBuffer->setUniform<pvr::int32>(m_deviceResource->pipeline-> getUniformLocation("sTexture"), 0);

	m_deviceResource->commandBuffer->endRecording();
	m_deviceResource->commandBuffer->submit();
	// Store the location of uniforms for later use
	uniformLocations.mvpMtx = m_deviceResource->pipeline->getUniformLocation("MVPMatrix");
	uniformLocations.mvITMtx = m_deviceResource->pipeline->getUniformLocation("MVITMatrix");
	uniformLocations.lightDirView = m_deviceResource->pipeline->getUniformLocation("ViewLightDirection");

	uniformLocations.specularExponent = m_deviceResource->pipeline->getUniformLocation("SpecularExponent");
	uniformLocations.metallicity = m_deviceResource->pipeline->getUniformLocation("Metallicity");
	uniformLocations.reflectivity = m_deviceResource->pipeline->getUniformLocation("Reflectivity");
	uniformLocations.albedo = m_deviceResource->pipeline->getUniformLocation("AlbedoModulation");
	return true;
}

/*!*********************************************************************************************************************
\brief  Loads the mesh data required for this training course into vertex buffer objects
***********************************************************************************************************************/
void OGLESPVRScopeRemote::loadVbos()
{
	CPPLProcessingScoped PPLProcessingScoped(spsCommsData, __FUNCTION__,
	    static_cast<pvr::uint32>(strlen(__FUNCTION__)), frameCounter);

	//  Load vertex data of all meshes in the scene into VBOs
	//  The meshes have been exported with the "Interleave Vectors" option,
	//  so all data is interleaved in the buffer at pMesh->pInterleaved.
	//  Interleaving data improves the memory access pattern and cache efficiency,
	//  thus it can be read faster by the hardware.
	pvr::utils::appendSingleBuffersFromModel(getGraphicsContext(), *scene, m_deviceResource->vbos, m_deviceResource->ibos);
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in initApplication() will be called by Shell once per run, before the rendering context is created.
    Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
    If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result OGLESPVRScopeRemote::initApplication()
{
	assetStore.init(*this);
	// Load the scene
	if (!assetStore.loadModel(SceneFile, scene))
	{
		this->setExitMessage("ERROR: Couldn't load the .pod file\n");
		return pvr::Result::NotInitialized;
	}
	// We want a data connection to PVRPerfServer
	{
		spsCommsData = pplInitialise("PVRScopeRemote", 14);
		hasCommunicationError = false;

		// Demonstrate that there is a good chance of the initial data being
		// lost - the connection is normally completed asynchronously.
		pplSendMark(spsCommsData, "lost", static_cast<pvr::uint32>(strlen("lost")));

		// This is entirely optional. Wait for the connection to succeed, it will
		// timeout if e.g. PVRPerfServer is not running.
		int isConnected;
		pplWaitForConnection(spsCommsData, &isConnected, 1, 200);
	}
	CPPLProcessingScoped PPLProcessingScoped(spsCommsData, __FUNCTION__, static_cast<pvr::uint32>(strlen(__FUNCTION__)), frameCounter);

	progUniforms.specularExponent = 5.f;            // Width of the specular highlights (using low exponent for a brushed metal look)
	progUniforms.albedo = glm::vec3(1.f, .77f, .33f); // Overall color
	progUniforms.metallicity = 1.f;                 // Is the color of the specular white (nonmetallic), or coloured by the object(metallic)
	progUniforms.reflectivity = .8f;                // Percentage of contribution of diffuse / specular
	frameCounter = 0;
	frame10Counter = 0;

	// Get and set the read path for content files
	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	// Load the scene
	if (!assetStore.loadModel(SceneFile, scene))
	{
		this->setExitMessage("ERROR: Couldn't load the .pod file\n");
		return pvr::Result::NotInitialized;
	}

	// set angle of rotation
	angleY = 0.0f;

	//  Remotely editable library items
	if (spsCommsData)
	{
		std::vector<SSPSCommsLibraryItem> communicableItems;
		size_t dataRead;
		//  Editable shaders
		pvr::assets::ShaderFile fileVersioning;
		fileVersioning.populateValidVersions(FragShaderSrcFile, *this);
		pvr::Stream::ptr_type FragShaderFile = fileVersioning.getBestStreamForApi(getMaxApiLevel());

		fileVersioning.populateValidVersions(VertShaderSrcFile, *this);
		pvr::Stream::ptr_type VertShaderFile = fileVersioning.getBestStreamForApi(getMaxApiLevel());
		struct SLibList
		{
			const char* const pszName;
			const pvr::Stream::ptr_type file;
		}
		aShaders[2] =
		{
			{ FragShaderSrcFile, FragShaderFile },
			{ VertShaderSrcFile, VertShaderFile }
		};

		std::vector<char> data[sizeof(aShaders) / sizeof(*aShaders)];
		for (pvr::uint32 i = 0; i < sizeof(aShaders) / sizeof(*aShaders); ++i)
		{
			if (aShaders[i].file->open())
			{
				communicableItems.push_back(SSPSCommsLibraryItem());
				communicableItems.back().pszName = aShaders[i].pszName;
				communicableItems.back().nNameLength = (pvr::uint32)strlen(aShaders[i].pszName);
				communicableItems.back().eType = eSPSCommsLibTypeString;
				data[i].resize(aShaders[i].file->getSize());
				aShaders[i].file->read(aShaders[i].file->getSize(), 1, &data[i][0], dataRead);
				communicableItems.back().pData = &data[i][0];
				communicableItems.back().nDataLength = (pvr::uint32)aShaders[i].file->getSize();
			}
		}

		// Editable: Specular Exponent
		communicableItems.push_back(SSPSCommsLibraryItem());
		commsLibSpecularExponent.fCurrent = progUniforms.specularExponent;
		commsLibSpecularExponent.fMin = 1.1f;
		commsLibSpecularExponent.fMax = 300.0f;
		communicableItems.back().pszName = "Specular Exponent";
		communicableItems.back().nNameLength = (pvr::uint32)strlen(communicableItems.back().pszName);
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&commsLibSpecularExponent;
		communicableItems.back().nDataLength = sizeof(commsLibSpecularExponent);

		communicableItems.push_back(SSPSCommsLibraryItem());
		// Editable: Metallicity
		commsLibMetallicity.fCurrent = progUniforms.metallicity;
		commsLibMetallicity.fMin = 0.0f;
		commsLibMetallicity.fMax = 1.0f;
		communicableItems.back().pszName = "Metallicity";
		communicableItems.back().nNameLength = (pvr::uint32)strlen(communicableItems.back().pszName);
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&commsLibMetallicity;
		communicableItems.back().nDataLength = sizeof(commsLibMetallicity);

		// Editable: Reflectivity
		communicableItems.push_back(SSPSCommsLibraryItem());
		commsLibReflectivity.fCurrent = progUniforms.reflectivity;
		commsLibReflectivity.fMin = 0.;
		commsLibReflectivity.fMax = 1.;
		communicableItems.back().pszName = "Reflectivity";
		communicableItems.back().nNameLength = (pvr::uint32)strlen(communicableItems.back().pszName);
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&commsLibReflectivity;
		communicableItems.back().nDataLength = sizeof(commsLibReflectivity);


		// Editable: Albedo R channel
		communicableItems.push_back(SSPSCommsLibraryItem());
		commsLibAlbedoR.fCurrent = progUniforms.albedo.r;
		commsLibAlbedoR.fMin = 0.0f;
		commsLibAlbedoR.fMax = 1.0f;
		communicableItems.back().pszName = "Albedo R";
		communicableItems.back().nNameLength = (pvr::uint32)strlen(communicableItems.back().pszName);
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&commsLibAlbedoR;
		communicableItems.back().nDataLength = sizeof(commsLibAlbedoR);

		// Editable: Albedo R channel
		communicableItems.push_back(SSPSCommsLibraryItem());
		commsLibAlbedoG.fCurrent = progUniforms.albedo.g;
		commsLibAlbedoG.fMin = 0.0f;
		commsLibAlbedoG.fMax = 1.0f;
		communicableItems.back().pszName = "Albedo G";
		communicableItems.back().nNameLength = (pvr::uint32)strlen(communicableItems.back().pszName);
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&commsLibAlbedoG;
		communicableItems.back().nDataLength = sizeof(commsLibAlbedoG);

		// Editable: Albedo R channel
		communicableItems.push_back(SSPSCommsLibraryItem());
		commsLibAlbedoB.fCurrent = progUniforms.albedo.b;
		commsLibAlbedoB.fMin = 0.0f;
		commsLibAlbedoB.fMax = 1.0f;
		communicableItems.back().pszName = "Albedo B";
		communicableItems.back().nNameLength = (pvr::uint32)strlen(communicableItems.back().pszName);
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&commsLibAlbedoB;
		communicableItems.back().nDataLength = sizeof(commsLibAlbedoB);

		// Ok, submit our library
		if (!pplLibraryCreate(spsCommsData, communicableItems.data(), (unsigned int)communicableItems.size()))
		{
			pvr::Log(pvr::Log.Debug, "PVRScopeRemote: pplLibraryCreate() failed\n");
		}
	}

	// User defined counters
	if (spsCommsData)
	{
		SSPSCommsCounterDef counterDefines[CounterDefs::NumCounter];
		for (pvr::uint32 i = 0; i < CounterDefs::NumCounter; ++i)
		{
			counterDefines[i].pszName = FrameDefs[i];
			counterDefines[i].nNameLength = (pvr::uint32)strlen(FrameDefs[i]);
		}

		if (!pplCountersCreate(spsCommsData, counterDefines, CounterDefs::NumCounter))
		{
			pvr::Log(pvr::Log.Debug, "PVRScopeRemote: pplCountersCreate() failed\n");
		}
	}
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in quitApplication() will be called by Shell once per run, just before exiting the program.
    If the rendering context is lost, QuitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result OGLESPVRScopeRemote::quitApplication()
{
	if (spsCommsData)
	{
		hasCommunicationError |= !pplSendProcessingBegin(spsCommsData, __FUNCTION__,  static_cast<pvr::uint32>(strlen(__FUNCTION__)), frameCounter);

		// Close the data connection to PVRPerfServer
		for (pvr::uint32 i = 0; i < 40; ++i)
		{
			char buf[128];
			const int nLen = sprintf(buf, "test %u", i);
			hasCommunicationError |= !pplSendMark(spsCommsData, buf, nLen);
		}
		hasCommunicationError |= !pplSendProcessingEnd(spsCommsData);
		pplShutdown(spsCommsData);
	}
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
    Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result OGLESPVRScopeRemote::initView()
{
	m_context = getGraphicsContext();
	m_deviceResource.reset(new DeviceResources());
	CPPLProcessingScoped PPLProcessingScoped(spsCommsData, __FUNCTION__, static_cast<pvr::uint32>(strlen(__FUNCTION__)), frameCounter);
	m_deviceResource->onScreenFbo = m_context->createOnScreenFbo(0);
	m_deviceResource->commandBuffer = m_context->createCommandBufferOnDefaultPool();
	//  Initialize VBO data
	loadVbos();

	//  Load textures
	if (!createTexSamplerDescriptorSet())
	{
		setExitMessage("ERROR:Failed to create DescriptorSets.");
		return pvr::Result::NotInitialized;
	}

	size_t dataRead;
	pvr::assets::ShaderFile shaderVersioning;
	// Take our initial vertex shader source
	{
		shaderVersioning.populateValidVersions(VertShaderSrcFile, *this);
		pvr::Stream::ptr_type vertShader = shaderVersioning.getBestStreamForApi(m_context->getApiType());
		vertShaderSrc.resize(vertShader->getSize() + 1, 0);
		vertShader->read(vertShader->getSize(), 1, &vertShaderSrc[0], dataRead);
	}
	// Take our initial fragment shader source
	{
		shaderVersioning.populateValidVersions(FragShaderSrcFile, *this);
		pvr::Stream::ptr_type fragShader = shaderVersioning.getBestStreamForApi(m_context->getApiType());
		fragShaderSrc.resize(fragShader->getSize() + 1, 0);
		fragShader->read(fragShader->getSize(), 1, &fragShaderSrc[0], dataRead);
	}

	// create the pipeline
	if (!createPipeline(&fragShaderSrc[0], &vertShaderSrc[0]))
	{
		setExitMessage("ERROR:Failed to create pipelines.");
		return pvr::Result::NotInitialized;
	}

	//  Initialize the UI Renderer
	if (uiRenderer.init(m_deviceResource->onScreenFbo->getRenderPass(), 0) != pvr::Result::Success)
	{
		this->setExitMessage("ERROR: Cannot initialize UIRenderer\n");
		return pvr::Result::NotInitialized;
	}

	// create the pvrscope connection pass and fail text
	uiRenderer.getDefaultTitle()->setText("PVRScopeRemote");
	uiRenderer.getDefaultTitle()->commitUpdates();

	uiRenderer.getDefaultDescription()->setScale(glm::vec2(.5, .5));
	uiRenderer.getDefaultDescription()->setText("Use PVRTune to remotely control the parameters of this application.");
	uiRenderer.getDefaultDescription()->commitUpdates();

	// Calculate the projection and view matrices
	// Is the screen rotated?
	bool isRotated = this->isScreenRotated() && this->isFullScreen();
	if (isRotated)
	{
		progUniforms.projectionMtx = pvr::math::perspectiveFov(getApiType(), glm::pi<pvr::float32>() / 6, (pvr::float32)getHeight(),
		                             (pvr::float32)getWidth(), scene->getCamera(0).getNear(), scene->getCamera(0).getFar(), glm::pi<pvr::float32>() * .5f);
	}
	else
	{
		progUniforms.projectionMtx = glm::perspectiveFov(glm::pi<pvr::float32>() / 6, (pvr::float32)getWidth(),
		                             (pvr::float32)getHeight(), scene->getCamera(0).getNear(), scene->getCamera(0).getFar());
	}
	recordCommandBuffer();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result OGLESPVRScopeRemote::releaseView()
{
	CPPLProcessingScoped PPLProcessingScoped(spsCommsData, __FUNCTION__, static_cast<pvr::uint32>(strlen(__FUNCTION__)), frameCounter);
	// Release UIRenderer
	uiRenderer.release();
	assetStore.releaseAll();
	m_deviceResource.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result OGLESPVRScopeRemote::renderFrame()
{
	CPPLProcessingScoped PPLProcessingScoped(spsCommsData, __FUNCTION__, static_cast<pvr::uint32>(strlen(__FUNCTION__)), frameCounter);
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
		hasCommunicationError |= !pplSendProcessingBegin(spsCommsData, "dirty", static_cast<pvr::uint32>(strlen("dirty")), frameCounter);
		{
			pvr::uint32 nItem, nNewDataLen;
			const char* pData;
			bool recompile = false;
			while (pplLibraryDirtyGetFirst(spsCommsData, &nItem, &nNewDataLen, &pData))
			{
				pvr::Log(pvr::Log.Debug, "dirty item %u %u 0x%08x\n", nItem, nNewDataLen, pData);
				switch (nItem)
				{
				case 0:
					fragShaderSrc.assign(pData, pData + nNewDataLen);
					fragShaderSrc.push_back(0);
					recompile = true;
					break;

				case 1:
					vertShaderSrc.assign(pData, pData + nNewDataLen);
					vertShaderSrc.push_back(0);
					recompile = true;
					break;

				case 2:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						progUniforms.specularExponent = psData->fCurrent;
						pvr::Log(pvr::Log.Information, "Setting Specular Exponent to value [%6.2f]", progUniforms.specularExponent);
					}
					break;
				case 3:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						progUniforms.metallicity = psData->fCurrent;
						pvr::Log(pvr::Log.Information, "Setting Metallicity to value [%3.2f]", progUniforms.metallicity);
					}
					break;
				case 4:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						progUniforms.reflectivity = psData->fCurrent;
						pvr::Log(pvr::Log.Information, "Setting Reflectivity to value [%3.2f]", progUniforms.reflectivity);
					}
					break;
				case 5:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						progUniforms.albedo.r = psData->fCurrent;
						pvr::Log(pvr::Log.Information, "Setting Albedo Red channel to value [%3.2f]", progUniforms.albedo.r);
					}
					break;
				case 6:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						progUniforms.albedo.g = psData->fCurrent;
						pvr::Log(pvr::Log.Information, "Setting Albedo Green channel to value [%3.2f]", progUniforms.albedo.g);
					}
					break;
				case 7:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						progUniforms.albedo.b = psData->fCurrent;
						pvr::Log(pvr::Log.Information, "Setting Albedo Blue channel to value [%3.2f]", progUniforms.albedo.b);
					}
					break;
				}
			}

			if (recompile)
			{
				if (!createPipeline(&fragShaderSrc[0], &vertShaderSrc[0]))
				{
					pvr::Log(pvr::Log.Error, "*** Could not recompile the shaders passed from PVRScopeCommunication ****");
				}
			}
		}
		hasCommunicationError |= !pplSendProcessingEnd(spsCommsData);
	}

	if (spsCommsData)
	{
		hasCommunicationError |= !pplSendProcessingBegin(spsCommsData, "draw", static_cast<pvr::uint32>(strlen("draw")), frameCounter);
	}

// Rotate and Translation the model matrix
	glm::mat4 modelMtx = glm::rotate(angleY, glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.6f)) * scene->getWorldMatrix(0);
	angleY += (2 * glm::pi<glm::float32>() * getFrameTime() / 1000) / 10;

	progUniforms.viewMtx = glm::lookAt(glm::vec3(0.f, 0.f, 75.f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

// Set model view projection matrix
	progUniforms.mvMatrix = progUniforms.viewMtx * modelMtx;
	progUniforms.mvpMatrix = progUniforms.projectionMtx * progUniforms.mvMatrix;

	progUniforms.mvITMatrix = glm::inverseTranspose(glm::mat3(progUniforms.mvMatrix));

	// Set light direction in model space
	progUniforms.lightDirView = glm::normalize(glm::vec3(1., 1., -1.));

	// Set eye position in model space
	// Now that the uniforms are set, call another function to actually draw the mesh.
	if (spsCommsData)
	{
		hasCommunicationError |= !pplSendProcessingEnd(spsCommsData);

		hasCommunicationError |= !pplSendProcessingBegin(spsCommsData, "Print3D", static_cast<pvr::uint32>(strlen("Print3D")), frameCounter);
	}

	if (hasCommunicationError)
	{
		uiRenderer.getDefaultControls()->setText("Communication Error:\nPVRScopeComms failed\n"
		    "Is PVRPerfServer connected?");
		uiRenderer.getDefaultControls()->setColor(glm::vec4(.8f, .3f, .3f, 1.0f));
		uiRenderer.getDefaultControls()->commitUpdates();
		recordCommandBuffer();
		hasCommunicationError = false;
	}
	else
	{
		uiRenderer.getDefaultControls()->setText("PVRScope Communication established.");
		uiRenderer.getDefaultControls()->setColor(glm::vec4(1.f));
		uiRenderer.getDefaultControls()->commitUpdates();
		recordCommandBuffer();
	}

	if (spsCommsData) { hasCommunicationError |= !pplSendProcessingEnd(spsCommsData); }

	// send counters
	counterReadings[CounterDefs::Counter] = frameCounter;
	counterReadings[CounterDefs::Counter10] = frame10Counter;
	if (spsCommsData) { hasCommunicationError |= !pplCountersUpdate(spsCommsData, counterReadings); }

	// update some counters
	++frameCounter;
	if (0 == (frameCounter / 10) % 10) { frame10Counter += 10; }
	m_deviceResource->commandBuffer->submit();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  Draws a pvr::assets::Mesh after the model view matrix has been set and the material prepared.
\param  nodeIndex Node index of the mesh to draw
***********************************************************************************************************************/
void OGLESPVRScopeRemote::drawMesh(int nodeIndex)
{
	CPPLProcessingScoped PPLProcessingScoped(spsCommsData, __FUNCTION__, static_cast<pvr::uint32>(strlen(__FUNCTION__)), frameCounter);

	pvr::int32 meshIndex = scene->getNode(nodeIndex).getObjectId();
	const pvr::assets::Mesh& mesh = scene->getMesh(meshIndex);
	// bind the VBO for the mesh
	m_deviceResource->commandBuffer->bindVertexBuffer(m_deviceResource->vbos[meshIndex], 0, 0);


	//  The geometry can be exported in 4 ways:
	//  - Indexed Triangle list
	//  - Non-Indexed Triangle list
	//  - Indexed Triangle strips
	//  - Non-Indexed Triangle strips
	if (mesh.getNumStrips() == 0)
	{
		if (m_deviceResource->ibos[meshIndex].isValid())
		{
			// Indexed Triangle list
			m_deviceResource->commandBuffer->bindIndexBuffer(m_deviceResource->ibos[meshIndex], 0, types::IndexType::IndexType16Bit);
			m_deviceResource->commandBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
		}
		else
		{
			// Non-Indexed Triangle list
			m_deviceResource->commandBuffer->drawArrays(0, mesh.getNumFaces(), 0, 1);
		}
	}
	else
	{
		for (pvr::int32 i = 0; i < (pvr::int32)mesh.getNumStrips(); ++i)
		{
			int offset = 0;
			if (m_deviceResource->ibos[meshIndex].isValid())
			{
				m_deviceResource->commandBuffer->bindIndexBuffer(m_deviceResource->ibos[meshIndex], 0, types::IndexType::IndexType16Bit);

				// Indexed Triangle strips
				m_deviceResource->commandBuffer->drawIndexed(0, mesh.getStripLength(i) + 2, offset * 2, 0, 1);
			}
			else
			{
				// Non-Indexed Triangle strips
				m_deviceResource->commandBuffer->drawArrays(0, mesh.getStripLength(i) + 2, 0, 1);
			}
			offset += mesh.getStripLength(i) + 2;
		}
	}
}

/*!*********************************************************************************************************************
\brief  pre-record the rendering the commands
***********************************************************************************************************************/
void OGLESPVRScopeRemote::recordCommandBuffer()
{
	CPPLProcessingScoped PPLProcessingScoped(spsCommsData, __FUNCTION__, static_cast<pvr::uint32>(strlen(__FUNCTION__)), frameCounter);
	m_deviceResource->commandBuffer->beginRecording();

	m_deviceResource->commandBuffer->beginRenderPass(m_deviceResource->onScreenFbo, pvr::Rectanglei(0, 0, getWidth(), getHeight()), true, glm::vec4(0.00, 0.70, 0.67, 1.0f));

	// Use shader program
	m_deviceResource->commandBuffer->bindPipeline(m_deviceResource->pipeline);
	// Bind texture
	m_deviceResource->commandBuffer->bindDescriptorSet(m_deviceResource->pipeline->getPipelineLayout(), 0, m_deviceResource->descriptorSet, 0);

	m_deviceResource->commandBuffer->setUniformPtr<glm::vec3>(uniformLocations.lightDirView, 1, &progUniforms.lightDirView);
	m_deviceResource->commandBuffer->setUniformPtr<glm::mat4>(uniformLocations.mvpMtx, 1, &progUniforms.mvpMatrix);
	m_deviceResource->commandBuffer->setUniformPtr<glm::mat3>(uniformLocations.mvITMtx, 1, &progUniforms.mvITMatrix);
	m_deviceResource->commandBuffer->setUniformPtr<pvr::float32>(uniformLocations.specularExponent, 1, &progUniforms.specularExponent);
	m_deviceResource->commandBuffer->setUniformPtr<pvr::float32>(uniformLocations.metallicity, 1, &progUniforms.metallicity);
	m_deviceResource->commandBuffer->setUniformPtr<pvr::float32>(uniformLocations.reflectivity, 1, &progUniforms.reflectivity);
	m_deviceResource->commandBuffer->setUniformPtr<glm::vec3>(uniformLocations.albedo, 1, &progUniforms.albedo);

	drawMesh(0);

	pvr::api::SecondaryCommandBuffer uicmd = m_context->createSecondaryCommandBufferOnDefaultPool();
	uiRenderer.beginRendering(uicmd);
	// Displays the demo name using the tools. For a detailed explanation, see the example
	// IntroUIRenderer
	uiRenderer.getDefaultTitle()->render();
	uiRenderer.getDefaultDescription()->render();
	uiRenderer.getSdkLogo()->render();
	uiRenderer.getDefaultControls()->render();
	uiRenderer.endRendering();
	m_deviceResource->commandBuffer->enqueueSecondaryCmds(uicmd);
	m_deviceResource->commandBuffer->endRenderPass();
	m_deviceResource->commandBuffer->endRecording();
}

/*!*********************************************************************************************************************
\return Return auto ptr to the demo supplied by the user
\brief  This function must be implemented by the user of the shell. The user should return its Shell object defining the behavior of the application.
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() { return std::auto_ptr<pvr::Shell>(new OGLESPVRScopeRemote()); }
