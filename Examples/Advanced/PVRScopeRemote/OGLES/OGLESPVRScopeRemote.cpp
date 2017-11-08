/*!*********************************************************************************************************************
\File         OGLESPVRScopeRemote.cpp
\Title        PVRScopeRemote
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief      Shows how to use our example PVRScope graph code.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsGles.h"
#include "PVRScopeComms.h"
#include "PVRAssets/Helper.h"
#include "PVRAssets/Shader.h"
// Source and binary shaders
const char FragShaderSrcFile[] = "FragShader.fsh";
const char VertShaderSrcFile[] = "VertShader.vsh";

// PVR texture files
const char TextureFile[] = "Marble.pvr";

// POD _scene files
const char SceneFile[] = "scene.pod";
namespace CounterDefs {
enum Enum
{
	Counter, Counter10, NumCounter
};
}
const char* FrameDefs[CounterDefs::NumCounter] = { "Frames", "Frames10" };

/*!*********************************************************************************************************************
\brief Class implementing the PVRShell functions.
***********************************************************************************************************************/
class OGLESPVRScopeRemote : public pvr::Shell
{
	struct DeviceResources
	{
		GLuint program;
		GLuint texture;
		std::vector<GLuint> vbos;
		std::vector<GLuint> ibos;
		GLuint onScreenFbo;

		GLuint shaders[2];

		pvr::EglContext context;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;

		DeviceResources() : program(0), texture(0), vbos(0), ibos(0)
		{
		}

		~DeviceResources()
		{
			gl::DeleteProgram(program);
			gl::DeleteTextures(1, &texture);
			gl::DeleteBuffers(vbos.size(), vbos.data());
			gl::DeleteBuffers(ibos.size(), ibos.data());
		}
	};

	std::unique_ptr<DeviceResources> _deviceResources;
	// 3D Model
	pvr::assets::ModelHandle _scene;
	// Projection and view matrices

	// Group shader programs and their uniform locations together
	struct
	{
		int32_t mvpMtx;
		int32_t mvITMtx;
		int32_t lightDirView;
		int32_t albedo;
		int32_t specularExponent;
		int32_t metallicity;
		int32_t reflectivity;
	} _uniformLocations;

	struct Uniforms
	{
		glm::mat4 projectionMtx;
		glm::mat4 viewMtx;
		glm::mat4 mvpMatrix;
		glm::mat4 mvMatrix;
		glm::mat3 mvITMatrix;
		glm::vec3 lightDirView;
		float specularExponent;
		float metallicity;
		float reflectivity;
		glm::vec3    albedo;
	} _progUniforms;

	// The translation and Rotate parameter of Model
	float _angleY;

	// Data connection to PVRPerfServer
	bool _hasCommunicationError;
	SSPSCommsData* _spsCommsData;
	SSPSCommsLibraryTypeFloat _commsLibSpecularExponent;
	SSPSCommsLibraryTypeFloat _commsLibMetallicity;
	SSPSCommsLibraryTypeFloat _commsLibReflectivity;
	SSPSCommsLibraryTypeFloat _commsLibAlbedoR;
	SSPSCommsLibraryTypeFloat _commsLibAlbedoG;
	SSPSCommsLibraryTypeFloat _commsLibAlbedoB;

	std::vector<char> _vertShaderSrc;
	std::vector<char> _fragShaderSrc;
	uint32_t _frameCounter;
	uint32_t _frame10Counter;
	uint32_t _counterReadings[CounterDefs::NumCounter];
	pvr::utils::VertexConfiguration _vertexConfiguration;
public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();
	void executeCommands();
	bool createSamplerTexture();
	bool createProgram(const char* const pszFrag, const char* const pszVert, bool recompile = false);
	void loadVbos();
	void drawMesh(int i32NodeIndex);
};

/*!*********************************************************************************************************************
\return Return true if no error occurred
\brief  Loads the textures required for this training course
***********************************************************************************************************************/
bool OGLESPVRScopeRemote::createSamplerTexture()
{
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);
	auto texStream = getAssetStream(TextureFile);
	pvr::Texture tex;
	if (!pvr::assets::textureLoad(texStream, pvr::TextureFileFormat::PVR, tex))
	{
		return false;
	}
	pvr::utils::TextureUploadResults uploadResults = pvr::utils::textureUpload(tex, _deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2, true);
	if (!uploadResults.successful)
	{
		Log("ERROR: Failed to load texture.");
		return false;
	}
	_deviceResources->texture = uploadResults.image;
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->texture);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::BindTexture(GL_TEXTURE_2D, 0);
	return true;
}

/*!*********************************************************************************************************************
\return Return true if no error occurred
\brief  Loads and compiles the shaders and links the shader programs required for this training course
***********************************************************************************************************************/
bool OGLESPVRScopeRemote::createProgram(const char* const fragShaderSource, const char* const vertShaderSource, bool recompile)
{
	//Mapping of mesh semantic names to shader variables
	const char* vertexBindings[] =
	{
		"inVertex",
		"inNormal",
		"inTexCoord"
	};
	const uint16_t attribIndices[3] = {0, 1, 2};
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__,
	    static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);


	/* Load and compile the shaders from files. */
	pvr::BufferStream vertexShaderStream("", vertShaderSource, strlen(vertShaderSource));
	pvr::BufferStream fragShaderStream("", fragShaderSource, strlen(fragShaderSource));
	if (recompile)
	{
		gl::DetachShader(_deviceResources->program, _deviceResources->shaders[0]);
		gl::DetachShader(_deviceResources->program, _deviceResources->shaders[1]);
	}
	pvr::utils::loadShader(vertexShaderStream, pvr::ShaderType::VertexShader, nullptr, 0, _deviceResources->shaders[0]);
	pvr::utils::loadShader(fragShaderStream, pvr::ShaderType::FragmentShader, nullptr, 0, _deviceResources->shaders[1]);

	if (!pvr::utils::createShaderProgram(_deviceResources->shaders, ARRAY_SIZE(_deviceResources->shaders),
	                                     vertexBindings, attribIndices, ARRAY_SIZE(attribIndices), _deviceResources->program))
	{
		Log(LogLevel::Debug, "Failed to Created pipeline...");
		return false;
	}

	// Set the sampler2D variable to the first texture unit
	gl::UseProgram(_deviceResources->program);
	gl::Uniform1i(gl::GetUniformLocation(_deviceResources->program, "sTexture"), 0);
	gl::UseProgram(0);

	// Store the location of uniforms for later use
	_uniformLocations.mvpMtx = gl::GetUniformLocation(_deviceResources->program, "MVPMatrix");
	_uniformLocations.mvITMtx = gl::GetUniformLocation(_deviceResources->program, "MVITMatrix");
	_uniformLocations.lightDirView = gl::GetUniformLocation(_deviceResources->program, "ViewLightDirection");

	_uniformLocations.specularExponent = gl::GetUniformLocation(_deviceResources->program, "SpecularExponent");
	_uniformLocations.metallicity = gl::GetUniformLocation(_deviceResources->program, "Metallicity");
	_uniformLocations.reflectivity = gl::GetUniformLocation(_deviceResources->program, "Reflectivity");
	_uniformLocations.albedo = gl::GetUniformLocation(_deviceResources->program, "AlbedoModulation");
	return true;
}

/*!*********************************************************************************************************************
\brief  Loads the mesh data required for this training course into vertex buffer objects
***********************************************************************************************************************/
void OGLESPVRScopeRemote::loadVbos()
{
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__,
	    static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

	//  Load vertex data of all meshes in the _scene into VBOs
	//  The meshes have been exported with the "Interleave Vectors" option,
	//  so all data is interleaved in the buffer at pMesh->pInterleaved.
	//  Interleaving data improves the memory access pattern and cache efficiency,
	//  thus it can be read faster by the hardware.
	pvr::utils::appendSingleBuffersFromModel(*_scene, _deviceResources->vbos, _deviceResources->ibos);
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in initApplication() will be called by Shell once per run, before the rendering context is created.
    Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
    If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result OGLESPVRScopeRemote::initApplication()
{
	// Load the _scene
	if (!pvr::assets::helper::loadModel(*this, SceneFile, _scene))
	{
		this->setExitMessage("ERROR: Couldn't load the .pod file\n");
		return pvr::Result::NotInitialized;
	}
	pvr::utils::VertexBindings_Name vertexBindings[] = { { "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoord" } };
	_vertexConfiguration = createInputAssemblyFromMesh(_scene->getMesh(0), vertexBindings, 3);

	_progUniforms.specularExponent = 5.f;            // Width of the specular highlights (using low exponent for a brushed metal look)
	_progUniforms.albedo = glm::vec3(1.f, .77f, .33f); // Overall color
	_progUniforms.metallicity = 1.f;                 // Is the color of the specular white (nonmetallic), or coloured by the object(metallic)
	_progUniforms.reflectivity = .8f;                // Percentage of contribution of diffuse / specular
	_frameCounter = 0;
	_frame10Counter = 0;

	// set angle of rotation
	_angleY = 0.0f;

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in quitApplication() will be called by Shell once per run, just before exiting the program.
    If the rendering context is lost, QuitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result OGLESPVRScopeRemote::quitApplication()
{
	if (_spsCommsData)
	{
		_hasCommunicationError |= !pplSendProcessingBegin(_spsCommsData, __FUNCTION__,
		                          static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

		// Close the data connection to PVRPerfServer
		for (uint32_t i = 0; i < 40; ++i)
		{
			char buf[128];
			const int nLen = sprintf(buf, "test %u", i);
			_hasCommunicationError |= !pplSendMark(_spsCommsData, buf, nLen);
		}
		_hasCommunicationError |= !pplSendProcessingEnd(_spsCommsData);
		pplShutdown(_spsCommsData);
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
	_deviceResources.reset(new DeviceResources());

	_deviceResources->context = pvr::createEglContext();
	_deviceResources->context->init(getWindow(), getDisplay(), getDisplayAttributes(), this->getMinApi(), this->getMaxApi());

	_deviceResources->shaders[0] = 0;
	_deviceResources->shaders[1] = 0;

	// We want a data connection to PVRPerfServer
	{
		_spsCommsData = pplInitialise("PVRScopeRemote", 14);
		_hasCommunicationError = false;

		// Demonstrate that there is a good chance of the initial data being
		// lost - the connection is normally completed asynchronously.
		pplSendMark(_spsCommsData, "lost", static_cast<uint32_t>(strlen("lost")));

		// This is entirely optional. Wait for the connection to succeed, it will
		// timeout if e.g. PVRPerfServer is not running.
		int isConnected;
		pplWaitForConnection(_spsCommsData, &isConnected, 1, 200);
	}
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__,
	    static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

	//  Remotely editable library items
	if (_spsCommsData)
	{
		std::vector<SSPSCommsLibraryItem> communicableItems;
		size_t dataRead;
		//  Editable shaders
		pvr::assets::ShaderFile vertShaderVersioning;
		pvr::assets::ShaderFile fragShaderVersioning;
		vertShaderVersioning.populateValidVersions(FragShaderSrcFile, *this);
		fragShaderVersioning.populateValidVersions(VertShaderSrcFile, *this);

		const std::pair<const char*, pvr::Stream::ptr_type> aShaders[2] =
		{
			{ FragShaderSrcFile, vertShaderVersioning.getBestStreamForApi(_deviceResources->context->getApiVersion()) },
			{ VertShaderSrcFile, fragShaderVersioning.getBestStreamForApi(_deviceResources->context->getApiVersion()) }
		};

		std::vector<char> data[sizeof(aShaders) / sizeof(*aShaders)];
		for (uint32_t i = 0; i < sizeof(aShaders) / sizeof(*aShaders); ++i)
		{
			if (aShaders[i].second->open())
			{
				communicableItems.push_back(SSPSCommsLibraryItem());
				communicableItems.back().pszName = aShaders[i].first;
				communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(aShaders[i].first));
				communicableItems.back().eType = eSPSCommsLibTypeString;
				data[i].resize(aShaders[i].second->getSize());
				aShaders[i].second->read(aShaders[i].second->getSize(), 1, &data[i][0], dataRead);
				communicableItems.back().pData = &data[i][0];
				communicableItems.back().nDataLength = static_cast<uint32_t>(aShaders[i].second->getSize());
			}
		}

		// Editable: Specular Exponent
		communicableItems.push_back(SSPSCommsLibraryItem());
		_commsLibSpecularExponent.fCurrent = _progUniforms.specularExponent;
		_commsLibSpecularExponent.fMin = 1.1f;
		_commsLibSpecularExponent.fMax = 300.0f;
		communicableItems.back().pszName = "Specular Exponent";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibSpecularExponent;
		communicableItems.back().nDataLength = sizeof(_commsLibSpecularExponent);

		communicableItems.push_back(SSPSCommsLibraryItem());
		// Editable: Metallicity
		_commsLibMetallicity.fCurrent = _progUniforms.metallicity;
		_commsLibMetallicity.fMin = 0.0f;
		_commsLibMetallicity.fMax = 1.0f;
		communicableItems.back().pszName = "Metallicity";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibMetallicity;
		communicableItems.back().nDataLength = sizeof(_commsLibMetallicity);

		// Editable: Reflectivity
		communicableItems.push_back(SSPSCommsLibraryItem());
		_commsLibReflectivity.fCurrent = _progUniforms.reflectivity;
		_commsLibReflectivity.fMin = 0.;
		_commsLibReflectivity.fMax = 1.;
		communicableItems.back().pszName = "Reflectivity";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibReflectivity;
		communicableItems.back().nDataLength = sizeof(_commsLibReflectivity);

		// Editable: Albedo R channel
		communicableItems.push_back(SSPSCommsLibraryItem());
		_commsLibAlbedoR.fCurrent = _progUniforms.albedo.r;
		_commsLibAlbedoR.fMin = 0.0f;
		_commsLibAlbedoR.fMax = 1.0f;
		communicableItems.back().pszName = "Albedo R";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibAlbedoR;
		communicableItems.back().nDataLength = sizeof(_commsLibAlbedoR);

		// Editable: Albedo R channel
		communicableItems.push_back(SSPSCommsLibraryItem());
		_commsLibAlbedoG.fCurrent = _progUniforms.albedo.g;
		_commsLibAlbedoG.fMin = 0.0f;
		_commsLibAlbedoG.fMax = 1.0f;
		communicableItems.back().pszName = "Albedo G";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibAlbedoG;
		communicableItems.back().nDataLength = sizeof(_commsLibAlbedoG);

		// Editable: Albedo R channel
		communicableItems.push_back(SSPSCommsLibraryItem());
		_commsLibAlbedoB.fCurrent = _progUniforms.albedo.b;
		_commsLibAlbedoB.fMin = 0.0f;
		_commsLibAlbedoB.fMax = 1.0f;
		communicableItems.back().pszName = "Albedo B";
		communicableItems.back().nNameLength = static_cast<uint32_t>(strlen(communicableItems.back().pszName));
		communicableItems.back().eType = eSPSCommsLibTypeFloat;
		communicableItems.back().pData = (const char*)&_commsLibAlbedoB;
		communicableItems.back().nDataLength = sizeof(_commsLibAlbedoB);

		// Ok, submit our library
		if (!pplLibraryCreate(_spsCommsData, communicableItems.data(), (unsigned int)communicableItems.size()))
		{
			Log(LogLevel::Debug, "PVRScopeRemote: pplLibraryCreate() failed\n");
		}
		// User defined counters


		SSPSCommsCounterDef counterDefines[CounterDefs::NumCounter];
		for (uint32_t i = 0; i < CounterDefs::NumCounter; ++i)
		{
			counterDefines[i].pszName = FrameDefs[i];
			counterDefines[i].nNameLength = static_cast<uint32_t>(strlen(FrameDefs[i]));
		}

		if (!pplCountersCreate(_spsCommsData, counterDefines, CounterDefs::NumCounter))
		{
			Log(LogLevel::Debug, "PVRScopeRemote: pplCountersCreate() failed\n");
		}
	}
	_deviceResources->onScreenFbo = _deviceResources->context->getOnScreenFbo();

	//  Initialize VBO data
	loadVbos();
	debugLogApiError("initView loadVbos");
	//  Load textures
	if (!createSamplerTexture())
	{
		setExitMessage("ERROR:Failed to create DescriptorSets.");
		return pvr::Result::NotInitialized;
	}
	debugLogApiError("initView createSamplerTexture");
	size_t dataRead;
	pvr::assets::ShaderFile shaderVersioning;
	// Take our initial vertex shader source
	{
		shaderVersioning.populateValidVersions(VertShaderSrcFile, *this);
		pvr::Stream::ptr_type vertShader = shaderVersioning.getBestStreamForApi(_deviceResources->context->getApiVersion());
		_vertShaderSrc.resize(vertShader->getSize() + 1, 0);
		vertShader->read(vertShader->getSize(), 1, &_vertShaderSrc[0], dataRead);
	}
	// Take our initial fragment shader source
	{
		shaderVersioning.populateValidVersions(FragShaderSrcFile, *this);
		pvr::Stream::ptr_type fragShader = shaderVersioning.getBestStreamForApi(_deviceResources->context->getApiVersion());
		_fragShaderSrc.resize(fragShader->getSize() + 1, 0);
		fragShader->read(fragShader->getSize(), 1, &_fragShaderSrc[0], dataRead);
	}

	// create the pipeline
	if (!createProgram(&_fragShaderSrc[0], &_vertShaderSrc[0], false))
	{
		setExitMessage("ERROR:Failed to create pipelines.");
		return pvr::Result::NotInitialized;
	}
	debugLogApiError("createProgram");
	//  Initialize the UI Renderer
	if (!_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2))
	{
		this->setExitMessage("ERROR: Cannot initialize UIRenderer\n");
		return pvr::Result::NotInitialized;
	}

	// create the pvrscope connection pass and fail text
	_deviceResources->uiRenderer.getDefaultTitle()->setText("PVRScopeRemote");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();

	_deviceResources->uiRenderer.getDefaultDescription()->setScale(glm::vec2(.5, .5));
	_deviceResources->uiRenderer.getDefaultDescription()->setText("Use PVRTune to remotely control the parameters of this application.");
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();

	// Calculate the projection and view matrices
	// Is the screen rotated?
	bool isRotated = this->isScreenRotated() && this->isFullScreen();
	if (isRotated)
	{
		_progUniforms.projectionMtx = pvr::math::perspectiveFov(_deviceResources->context->getApiVersion(), glm::pi<float>() / 6, (float)getHeight(),
		                              (float)getWidth(), _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar(), glm::pi<float>() * .5f);
	}
	else
	{
		_progUniforms.projectionMtx = pvr::math::perspectiveFov(_deviceResources->context->getApiVersion(), glm::pi<float>() / 6, (float)getWidth(),
		                              (float)getHeight(), _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar());
	}
	gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _deviceResources->onScreenFbo);
	gl::ClearColor(0.00f, 0.70f, 0.67f, 1.0f);
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result OGLESPVRScopeRemote::releaseView()
{
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);
	// Release UIRenderer
	_deviceResources->uiRenderer.release();
	_deviceResources.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result OGLESPVRScopeRemote::renderFrame()
{
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);
	bool currCommunicationErr = _hasCommunicationError;
	if (_spsCommsData)
	{
		// mark every N frames
		if (!(_frameCounter % 100))
		{
			char buf[128];
			const int nLen = sprintf(buf, "frame %u", _frameCounter);
			_hasCommunicationError |= !pplSendMark(_spsCommsData, buf, nLen);
		}

		// Check for dirty items
		_hasCommunicationError |= !pplSendProcessingBegin(_spsCommsData, "dirty", static_cast<uint32_t>(strlen("dirty")), _frameCounter);
		{
			uint32_t nItem, nNewDataLen;
			const char* pData;
			bool recompile = false;
			while (pplLibraryDirtyGetFirst(_spsCommsData, &nItem, &nNewDataLen, &pData))
			{
				(LogLevel::Debug, "dirty item %u %u 0x%08x\n", nItem, nNewDataLen, pData);
				switch (nItem)
				{
				case 0:
					_fragShaderSrc.assign(pData, pData + nNewDataLen);
					_fragShaderSrc.push_back(0);
					recompile = true;
					break;

				case 1:
					_vertShaderSrc.assign(pData, pData + nNewDataLen);
					_vertShaderSrc.push_back(0);
					recompile = true;
					break;

				case 2:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						_progUniforms.specularExponent = psData->fCurrent;
						Log(LogLevel::Information, "Setting Specular Exponent to value [%6.2f]", _progUniforms.specularExponent);
					}
					break;
				case 3:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						_progUniforms.metallicity = psData->fCurrent;
						Log(LogLevel::Information, "Setting Metallicity to value [%3.2f]", _progUniforms.metallicity);
					}
					break;
				case 4:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						_progUniforms.reflectivity = psData->fCurrent;
						Log(LogLevel::Information, "Setting Reflectivity to value [%3.2f]", _progUniforms.reflectivity);
					}
					break;
				case 5:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						_progUniforms.albedo.r = psData->fCurrent;
						Log(LogLevel::Information, "Setting Albedo Red channel to value [%3.2f]", _progUniforms.albedo.r);
					}
					break;
				case 6:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						_progUniforms.albedo.g = psData->fCurrent;
						Log(LogLevel::Information, "Setting Albedo Green channel to value [%3.2f]", _progUniforms.albedo.g);
					}
					break;
				case 7:
					if (nNewDataLen == sizeof(SSPSCommsLibraryTypeFloat))
					{
						const SSPSCommsLibraryTypeFloat* const psData = (SSPSCommsLibraryTypeFloat*)pData;
						_progUniforms.albedo.b = psData->fCurrent;
						Log(LogLevel::Information, "Setting Albedo Blue channel to value [%3.2f]", _progUniforms.albedo.b);
					}
					break;
				}
			}

			if (recompile)
			{
				if (!createProgram(&_fragShaderSrc[0], &_vertShaderSrc[0], true))
				{
					Log(LogLevel::Error, "*** Could not recompile the shaders passed from PVRScopeCommunication ****");
				}
			}
		}
		_hasCommunicationError |= !pplSendProcessingEnd(_spsCommsData);
	}

	if (_spsCommsData)
	{
		_hasCommunicationError |= !pplSendProcessingBegin(_spsCommsData, "draw", static_cast<uint32_t>(strlen("draw")), _frameCounter);
	}

// Rotate and Translation the model matrix
	glm::mat4 modelMtx = glm::rotate(_angleY, glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.6f)) * _scene->getWorldMatrix(0);
	_angleY += (2 * glm::pi<float>() * getFrameTime() / 1000) / 10;

	_progUniforms.viewMtx = glm::lookAt(glm::vec3(0.f, 0.f, 75.f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

// Set model view projection matrix
	_progUniforms.mvMatrix = _progUniforms.viewMtx * modelMtx;
	_progUniforms.mvpMatrix = _progUniforms.projectionMtx * _progUniforms.mvMatrix;

	_progUniforms.mvITMatrix = glm::inverseTranspose(glm::mat3(_progUniforms.mvMatrix));

	// Set light direction in model space
	_progUniforms.lightDirView = glm::normalize(glm::vec3(1., 1., -1.));

	// Set eye position in model space
	// Now that the uniforms are set, call another function to actually draw the mesh.
	if (_spsCommsData)
	{
		_hasCommunicationError |= !pplSendProcessingEnd(_spsCommsData);

		_hasCommunicationError |= !pplSendProcessingBegin(_spsCommsData, "UIRenderer",
		                          static_cast<uint32_t>(strlen("UIRenderer")), _frameCounter);
	}

	if (_hasCommunicationError)
	{
		_deviceResources->uiRenderer.getDefaultControls()->setText("Communication Error:\nPVRScopeComms failed\n"
		    "Is PVRPerfServer connected?");
		_deviceResources->uiRenderer.getDefaultControls()->setColor(glm::vec4(.8f, .3f, .3f, 1.0f));
		_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();
		executeCommands();
		_hasCommunicationError = false;
	}
	else
	{
		_deviceResources->uiRenderer.getDefaultControls()->setText("PVRScope Communication established.");
		_deviceResources->uiRenderer.getDefaultControls()->setColor(glm::vec4(1.f));
		_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();
		executeCommands();
	}

	if (_spsCommsData) { _hasCommunicationError |= !pplSendProcessingEnd(_spsCommsData); }

	// send counters
	_counterReadings[CounterDefs::Counter] = _frameCounter;
	_counterReadings[CounterDefs::Counter10] = _frame10Counter;
	if (_spsCommsData) { _hasCommunicationError |= !pplCountersUpdate(_spsCommsData, _counterReadings); }

	// update some counters
	++_frameCounter;
	if (0 == (_frameCounter / 10) % 10) { _frame10Counter += 10; }
	executeCommands();

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight());
	}

	_deviceResources->context->swapBuffers();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  Draws a pvr::assets::Mesh after the model view matrix has been set and the material prepared.
\param  nodeIndex Node index of the mesh to draw
***********************************************************************************************************************/
void OGLESPVRScopeRemote::drawMesh(int nodeIndex)
{
	debugLogApiError("draw mesh");
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__, static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

	int32_t meshIndex = _scene->getNode(nodeIndex).getObjectId();
	const pvr::assets::Mesh& mesh = _scene->getMesh(meshIndex);
	// bind the VBO for the mesh
	gl::BindBuffer(GL_ARRAY_BUFFER, _deviceResources->vbos[meshIndex]);
	debugLogApiError("draw mesh");
	for (int i = 0; i < 3; ++i)
	{
		auto& attrib = _vertexConfiguration.attributes[i];
		auto& binding = _vertexConfiguration.bindings[0];
		gl::EnableVertexAttribArray(attrib.index);
		gl::VertexAttribPointer(attrib.index, attrib.width, pvr::utils::convertToGles(attrib.format),
		                        dataTypeIsNormalised(attrib.format), binding.strideInBytes,
		                        reinterpret_cast<const void*>(static_cast<uintptr_t>(attrib.offsetInBytes)));
		debugLogApiError("draw mesh");
	}

	debugLogApiError("draw mesh");

	//  The geometry can be exported in 4 ways:
	//  - Indexed Triangle list
	//  - Non-Indexed Triangle list
	//  - Indexed Triangle strips
	//  - Non-Indexed Triangle strips
	if (mesh.getNumStrips() == 0)
	{
		if (_deviceResources->ibos[meshIndex])
		{
			// Indexed Triangle list
			gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _deviceResources->ibos[meshIndex]);
			gl::DrawElements(GL_TRIANGLES, mesh.getNumFaces() * 3, GL_UNSIGNED_SHORT, nullptr);
		}
		else
		{
			// Non-Indexed Triangle list
			gl::DrawArrays(GL_TRIANGLES, 0, mesh.getNumFaces());
		}
	}
	else
	{
		for (int32_t i = 0; i < (int32_t)mesh.getNumStrips(); ++i)
		{
			int offset = 0;
			if (_deviceResources->ibos[meshIndex])
			{
				// Indexed Triangle strips
				gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _deviceResources->ibos[meshIndex]);
				gl::DrawElements(GL_TRIANGLE_STRIP, mesh.getStripLength(i) + 2, GL_UNSIGNED_SHORT, 0);
			}
			else
			{
				// Non-Indexed Triangle strips
				gl::DrawArrays(GL_TRIANGLE_STRIP, 0, mesh.getStripLength(i) + 2);
			}
			offset += mesh.getStripLength(i) + 2;
		}
	}
	for (int i = 0; i < 3; ++i)
	{
		auto& attrib = _vertexConfiguration.attributes[i];
		gl::DisableVertexAttribArray(attrib.index);
	}
}

/*!*********************************************************************************************************************
\brief  pre-record the rendering the commands
***********************************************************************************************************************/
void OGLESPVRScopeRemote::executeCommands()
{
	debugLogApiError("executeCommands");
	CPPLProcessingScoped PPLProcessingScoped(_spsCommsData, __FUNCTION__,
	    static_cast<uint32_t>(strlen(__FUNCTION__)), _frameCounter);

	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl::UseProgram(_deviceResources->program);
	debugLogApiError("executeCommands");
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->texture);
	debugLogApiError("executeCommands");
	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);
	gl::Enable(GL_DEPTH_TEST);
	debugLogApiError("executeCommands");
	gl::Uniform3fv(_uniformLocations.lightDirView, 1, glm::value_ptr(_progUniforms.lightDirView));
	debugLogApiError("executeCommands");
	gl::UniformMatrix4fv(_uniformLocations.mvpMtx, 1, false, glm::value_ptr(_progUniforms.mvpMatrix));
	debugLogApiError("executeCommands");
	gl::UniformMatrix3fv(_uniformLocations.mvITMtx, 1, false, glm::value_ptr(_progUniforms.mvITMatrix));
	debugLogApiError("executeCommands");
	gl::Uniform1fv(_uniformLocations.specularExponent, 1, &_progUniforms.specularExponent);
	debugLogApiError("executeCommands");
	gl::Uniform1fv(_uniformLocations.metallicity, 1, &_progUniforms.metallicity);
	debugLogApiError("executeCommands");
	gl::Uniform1fv(_uniformLocations.reflectivity, 1, &_progUniforms.reflectivity);
	debugLogApiError("executeCommands");
	gl::Uniform3fv(_uniformLocations.albedo, 1, glm::value_ptr(_progUniforms.albedo));
	debugLogApiError("executeCommands");
	drawMesh(0);

	_deviceResources->uiRenderer.beginRendering();
	// Displays the demo name using the tools. For a detailed explanation, see the example
	// IntroducingUIRenderer
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.endRendering();
}

/*!*********************************************************************************************************************
\return Return auto ptr to the demo supplied by the user
\brief  This function must be implemented by the user of the shell. The user should return its Shell object defining the behavior of the application.
***********************************************************************************************************************/
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::unique_ptr<pvr::Shell>(new OGLESPVRScopeRemote()); }
