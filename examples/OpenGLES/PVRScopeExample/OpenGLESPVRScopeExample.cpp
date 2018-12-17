/*!*********************************************************************************************************************
\File         OpenGLESPVRScopeExample.cpp
\Title        PVRScopeExample
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief  Shows how to use our example PVRScope graph code.
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/OpenGLES/BindingsGles.h"
#include "PVRUtils/PVRUtilsGles.h"
#include "PVRUtils/OpenGLES/HelperGles.h"
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
const char SceneFile[] = "Satyr.pod";

/*!*********************************************************************************************************************
\brief Class implementing the pvr::Shell functions.
***********************************************************************************************************************/
class OpenGLESPVRScopeExample : public pvr::Shell
{
	glm::vec3 ClearColor;

	struct DeviceResources
	{
		pvr::EglContext context;

		std::vector<GLuint> vbos;
		std::vector<GLuint> ibos;
		GLuint program;
		GLuint texture;
		GLuint onScreenFbo;
		PVRScopeGraph scopeGraph;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;
	};
	std::unique_ptr<DeviceResources> _deviceResources;

	// 3D Model
	pvr::assets::ModelHandle _scene;
	// Projection and view matrices
	pvr::utils::VertexConfiguration _vertexConfig;
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
		glm::mat4 mvpMatrix1;
		glm::mat4 mvpMatrix2;
		glm::mat4 mvMatrix1;
		glm::mat4 mvMatrix2;
		glm::mat3 mvITMatrix1;
		glm::mat3 mvITMatrix2;
		glm::vec3 lightDirView;
		float specularExponent;
		float metallicity;
		float reflectivity;
		glm::vec3 albedo;
	} _progUniforms;

	// The translation and Rotate parameter of Model
	float _angleY;

	// The PVRScopeGraph variable

	// Variables for the graphing code
	int32_t _selectedCounter;
	int32_t _interval;

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void eventMappedInput(pvr::SimplifiedInput key);

	void updateDescription();
	void executeGlCommands();
	void createTexSamplerDescriptorSet();
	void loadVbos();
	void createProgram();
	void drawMesh(uint32_t nodeIndex);
};

/*!*********************************************************************************************************************
\brief Handle input key events
\param key key event to handle
************************************************************************************************************************/
void OpenGLESPVRScopeExample::eventMappedInput(pvr::SimplifiedInput key)
{
	// Keyboard input (cursor up/down to cycle through counters)
	switch (key)
	{
	case pvr::SimplifiedInput::Up:
	case pvr::SimplifiedInput::Right:
	{
		_selectedCounter++;
		if (_selectedCounter > static_cast<int32_t>(_deviceResources->scopeGraph.getCounterNum()))
		{
			_selectedCounter = _deviceResources->scopeGraph.getCounterNum();
		}
	}
	break;
	case pvr::SimplifiedInput::Down:
	case pvr::SimplifiedInput::Left:
	{
		_selectedCounter--;
		if (_selectedCounter < 0)
		{
			_selectedCounter = 0;
		}
	}
	break;
	case pvr::SimplifiedInput::Action1:
	{
		_deviceResources->scopeGraph.showCounter(_selectedCounter, !_deviceResources->scopeGraph.isCounterShown(_selectedCounter));
	}
	break;
	// Keyboard input (cursor left/right to change active group)
	case pvr::SimplifiedInput::ActionClose:
		exitShell();
		break;
	default:
		break;
	}
	updateDescription();
}

/*!*********************************************************************************************************************
\brief Loads the textures required for this training course
\return Return true if no error occurred
***********************************************************************************************************************/
void OpenGLESPVRScopeExample::createTexSamplerDescriptorSet()
{
	_deviceResources->texture = pvr::utils::textureUpload(*this, TextureFile, _deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2);
	// create the bilinear sampler
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->texture);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	pvr::utils::throwOnGlError("Texture and sampler creation failed");
}

void OpenGLESPVRScopeExample::createProgram()
{
	const char* attribNames[] = { "inVertex", "inNormal", "inTexCoord" };
	const pvr::utils::VertexBindings_Name vertexBindings[] = {
		{ "POSITION", "inVertex" },
		{ "NORMAL", "inNormal" },
		{ "UV0", "inTexCoord" },
	};

	// Enable or disable gamma correction based on if it is automatically performed on the framebuffer or we need to do it in the shader.
	const char* defines[] = { "FRAMEBUFFER_SRGB" };
	uint32_t numDefines = 1;

	glm::vec3 clearColorLinearSpace(0.0f, 0.45f, 0.41f);
	ClearColor = clearColorLinearSpace;
	if (getBackBufferColorspace() != pvr::ColorSpace::sRGB)
	{
		ClearColor = pvr::utils::convertLRGBtoSRGB(clearColorLinearSpace); // Gamma correct the clear color...
		numDefines = 0;
	}

	uint16_t attribIndices[ARRAY_SIZE(attribNames)] = { 0, 1, 2 };

	_deviceResources->program = 0;
	if (_deviceResources->context->getApiVersion() < pvr::Api::OpenGLES3)
	{
		_deviceResources->program =
			pvr::utils::createShaderProgram(*this, "VertShader_ES2.vsh", "FragShader_ES2.fsh", attribNames, attribIndices, ARRAY_SIZE(attribIndices), defines, numDefines);
	}
	else
	{
		_deviceResources->program =
			pvr::utils::createShaderProgram(*this, "VertShader_ES3.vsh", "FragShader_ES3.fsh", attribNames, attribIndices, ARRAY_SIZE(attribIndices), defines, numDefines);
	}

	_uniformLocations.mvpMtx = gl::GetUniformLocation(_deviceResources->program, "MVPMatrix");
	_uniformLocations.mvITMtx = gl::GetUniformLocation(_deviceResources->program, "MVITMatrix");
	_uniformLocations.lightDirView = gl::GetUniformLocation(_deviceResources->program, "ViewLightDirection");

	_uniformLocations.specularExponent = gl::GetUniformLocation(_deviceResources->program, "SpecularExponent");
	_uniformLocations.metallicity = gl::GetUniformLocation(_deviceResources->program, "Metallicity");
	_uniformLocations.reflectivity = gl::GetUniformLocation(_deviceResources->program, "Reflectivity");
	_uniformLocations.albedo = gl::GetUniformLocation(_deviceResources->program, "AlbedoModulation");

	gl::UseProgram(_deviceResources->program);
	gl::Uniform1i(gl::GetUniformLocation(_deviceResources->program, "sDiffuseMap"), 0);
	_vertexConfig = pvr::utils::createInputAssemblyFromMesh(_scene->getMesh(0), vertexBindings, 3);
}

/*!*********************************************************************************************************************
\brief Loads the mesh data required for this training course into vertex buffer objects
***********************************************************************************************************************/
void OpenGLESPVRScopeExample::loadVbos()
{
	pvr::utils::appendSingleBuffersFromModel(*_scene, _deviceResources->vbos, _deviceResources->ibos);
}

/*!*********************************************************************************************************************
\return pvr::Result::Success if no error occurred
\brief  Code in initApplication() will be called by pvr::Shell once per run, before the rendering context is created.
	  Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes,etc.)
	  If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result OpenGLESPVRScopeExample::initApplication()
{
	// Blue-ish marble
	_progUniforms.specularExponent = 100.f; // Width of the specular highlights (High exponent for small shiny highlights)
	_progUniforms.albedo = glm::vec3(.78f, .82f, 1.f); // Overall color
	_progUniforms.metallicity = 1.f; // Doesn't make much of a difference in this material.
	_progUniforms.reflectivity = .2f; // Low reflectivity - color mostly diffuse.

	// At the time of writing, this counter is the USSE load for vertex + pixel processing
	_selectedCounter = 0;
	_interval = 0;
	_angleY = 0.0f;
	// Load the _scene
	if (!pvr::utils::loadModel(*this, SceneFile, _scene))
	{
		this->setExitMessage("ERROR: Couldn't load the .pod file\n");
		return pvr::Result::NotInitialized;
	}

	// Process the command line
	{
		const pvr::CommandLine commandline = getCommandLine();
		commandline.getIntOption("-counter", _selectedCounter);
		commandline.getIntOption("-interval", _interval);
	}
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in quitApplication() will be called by pvr::Shell once per run, just before exiting
	  the program. If the rendering context is lost, quitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result OpenGLESPVRScopeExample::quitApplication()
{
	_scene.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief Code in initView() will be called by pvr::Shell upon initialization or after a change in the rendering context.
	 Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result OpenGLESPVRScopeExample::initView()
{
	_deviceResources = std::unique_ptr<DeviceResources>(new DeviceResources());
	_deviceResources->context = pvr::createEglContext();

	_deviceResources->context->init(getWindow(), getDisplay(), getDisplayAttributes());

	// create the default fbo using default params
	_deviceResources->onScreenFbo = _deviceResources->context->getOnScreenFbo();

	// Initialize VBO data
	loadVbos();

	// Load textures
	createTexSamplerDescriptorSet();

	// Load and compile the shaders & link programs
	createProgram();

	// Initialize UIRenderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), getBackBufferColorspace() == pvr::ColorSpace::sRGB);

	// Calculate the projection and view matrices
	// Is the screen rotated?
	bool isRotate = this->isScreenRotated();
	if (isRotate)
	{
		_progUniforms.projectionMtx = pvr::math::perspectiveFov(_deviceResources->context->getApiVersion(), glm::pi<float>() / 6, static_cast<float>(this->getWidth()),
			static_cast<float>(this->getHeight()), _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar(), glm::pi<float>() * .5f);
	}
	else
	{
		_progUniforms.projectionMtx = pvr::math::perspectiveFov(_deviceResources->context->getApiVersion(), glm::pi<float>() / 6, static_cast<float>(this->getWidth()),
			static_cast<float>(this->getHeight()), _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar());
	}

	// Initialize the graphing code

	std::string errorStr;
	if (_deviceResources->scopeGraph.init(_deviceResources->context, *this, _deviceResources->uiRenderer, errorStr))
	{
		// Position the graph
		_deviceResources->scopeGraph.position(getWidth(), getHeight(),
			pvr::Rectanglei(static_cast<uint32_t>(getWidth() * 0.02f), static_cast<uint32_t>(getHeight() * 0.02f), static_cast<uint32_t>(getWidth() * 0.96f),
				static_cast<uint32_t>(getHeight() * 0.96f) / 3));

		// Output the current active group and a list of all the counters
		Log(LogLevel::Information, "PVRScope Number of Hardware Counters: %i\n", _deviceResources->scopeGraph.getCounterNum());
		Log(LogLevel::Information, "Counters\n-ID---Name-------------------------------------------\n");

		for (uint32_t i = 0; i < _deviceResources->scopeGraph.getCounterNum(); ++i)
		{
			Log(LogLevel::Information, "[%2i] %s %s\n", i, _deviceResources->scopeGraph.getCounterName(i),
				_deviceResources->scopeGraph.isCounterPercentage(i) ? "percentage" : "absolute");
			_deviceResources->scopeGraph.showCounter(i, false);
		}

		_deviceResources->scopeGraph.ping(1);
		// Tell the graph to show initial counters
		_deviceResources->scopeGraph.showCounter(_deviceResources->scopeGraph.getStandard3DIndex(), true);
		_deviceResources->scopeGraph.showCounter(_deviceResources->scopeGraph.getStandardTAIndex(), true);
		_deviceResources->scopeGraph.showCounter(_deviceResources->scopeGraph.getStandardShaderPixelIndex(), true);
		_deviceResources->scopeGraph.showCounter(_deviceResources->scopeGraph.getStandardShaderVertexIndex(), true);
		for (uint32_t i = 0; i < _deviceResources->scopeGraph.getCounterNum(); ++i)
		{
			std::string s(std::string(_deviceResources->scopeGraph.getCounterName(i))); // Better safe than sorry - get a copy...
			pvr::strings::toLower(s);
			if (pvr::strings::startsWith(s, "hsr efficiency"))
			{
				_deviceResources->scopeGraph.showCounter(i, true);
			}
			if (pvr::strings::startsWith(s, "shaded pixels per second"))
			{
				_deviceResources->scopeGraph.showCounter(i, true);
			}
		}

		// Set the update interval: number of updates [frames] before updating the graph
		_deviceResources->scopeGraph.setUpdateInterval(_interval);
	}
	else
	{
		Log(errorStr.c_str());
	}
	_deviceResources->uiRenderer.getDefaultTitle()->setText("PVRScopeExample");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();

	updateDescription();

	gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _deviceResources->onScreenFbo);
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief Code in releaseView() will be called by pvr::Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result OpenGLESPVRScopeExample::releaseView()
{
	// Instructs the Asset Store to free all resources
	_deviceResources.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result OpenGLESPVRScopeExample::renderFrame()
{
	// Rotate and Translation the model matrix
	glm::mat4x4 mModel1, mModel2;
	mModel1 = glm::translate(glm::vec3(0.0f, -1.0f, 0.0f)) * glm::rotate((_angleY), glm::vec3(0.f, 1.f, 0.f)) * glm::translate(glm::vec3(.5f, 0.f, -1.0f)) *
		glm::scale(glm::vec3(0.5f, 0.5f, 0.5f)) * _scene->getWorldMatrix(0);
	// Create two instances of the mesh, offset to the sides.
	mModel2 = mModel1 * glm::translate(glm::vec3(0, 0, -2000));
	mModel1 = mModel1 * glm::translate(glm::vec3(0, 0, 2000));

	_angleY += (2 * glm::pi<float>() * getFrameTime() / 1000) / 10;

	_progUniforms.viewMtx = glm::lookAt(glm::vec3(0, 0, 75), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	glm::mat4 vp = _progUniforms.projectionMtx * _progUniforms.viewMtx;

	_progUniforms.mvMatrix1 = _progUniforms.viewMtx * mModel1;
	_progUniforms.mvMatrix2 = _progUniforms.viewMtx * mModel2;
	_progUniforms.mvITMatrix1 = glm::inverseTranspose(glm::mat3(_progUniforms.mvMatrix1));
	_progUniforms.mvITMatrix2 = glm::inverseTranspose(glm::mat3(_progUniforms.mvMatrix2));
	_progUniforms.mvpMatrix1 = vp * mModel1;
	_progUniforms.mvpMatrix2 = vp * mModel2;

	// Set light direction in model space
	_progUniforms.lightDirView = glm::normalize(glm::vec3(1., 1., -1.));

	_deviceResources->scopeGraph.ping(static_cast<float>(getFrameTime()));
	executeGlCommands();

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight());
	}

	_deviceResources->context->swapBuffers();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\param nodeIndex Node index of the mesh to draw
\brief Draws a pvr::Model::Mesh after the model view matrix has been set and the material prepared.
***********************************************************************************************************************/
void OpenGLESPVRScopeExample::drawMesh(uint32_t nodeIndex)
{
	const pvr::assets::Model::Node& node = _scene->getNode(nodeIndex);
	const pvr::assets::Mesh& mesh = _scene->getMesh(node.getObjectId());

	// bind the VBO for the mesh
	gl::BindBuffer(GL_ARRAY_BUFFER, _deviceResources->vbos[node.getObjectId()]);

	assertion(_vertexConfig.bindings.size() == 1, "This demo assumes only one VBO per mesh");
	for (auto it = _vertexConfig.attributes.begin(), end = _vertexConfig.attributes.end(); it != end; ++it)
	{
		gl::EnableVertexAttribArray(it->index);
		gl::VertexAttribPointer(it->index, it->width, pvr::utils::convertToGles(it->format), pvr::dataTypeIsNormalised(it->format),
			_vertexConfig.bindings[it->binding].strideInBytes, reinterpret_cast<const void*>(static_cast<uintptr_t>(it->offsetInBytes)));
	}

	GLenum indexType = mesh.getFaces().getDataType() == pvr::IndexType::IndexType32Bit ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
	// The geometry can be exported in 4 ways:
	// - Indexed Triangle list
	// - Non-Indexed Triangle list
	// - Indexed Triangle strips
	// - Non-Indexed Triangle strips
	if (mesh.getNumStrips() == 0)
	{
		if (_deviceResources->ibos[node.getObjectId()])
		{
			// Indexed Triangle list
			gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _deviceResources->ibos[node.getObjectId()]);
			gl::DrawElements(GL_TRIANGLES, mesh.getNumFaces() * 3, indexType, 0);
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
			if (_deviceResources->ibos[node.getObjectId()])
			{
				// Indexed Triangle strips
				gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _deviceResources->ibos[node.getObjectId()]);
				gl::DrawElements(GL_TRIANGLE_STRIP, mesh.getStripLength(i) + 2, indexType, 0);
			}
			else
			{
				// Non-Indexed Triangle strips
				gl::DrawArrays(GL_TRIANGLE_STRIP, 0, mesh.getStripLength(i) + 2);
			}
			offset += mesh.getStripLength(i) + 2;
		}
	}
	for (auto it = _vertexConfig.attributes.begin(), end = _vertexConfig.attributes.end(); it != end; ++it)
	{
		gl::DisableVertexAttribArray(it->index);
	}
}

/*!*********************************************************************************************************************
\brief  Pre-record the rendering commands
***********************************************************************************************************************/
void OpenGLESPVRScopeExample::executeGlCommands()
{
	gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _deviceResources->onScreenFbo);
	gl::ClearColor(ClearColor.r, ClearColor.g, ClearColor.b, 1.f);
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl::Enable(GL_CULL_FACE);
	gl::UseProgram(_deviceResources->program);

	//--- create the pipeline layout
	gl::CullFace(GL_BACK);

	gl::FrontFace(GL_CCW);

	gl::Enable(GL_DEPTH_TEST);

	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->texture);

	gl::Uniform3fv(_uniformLocations.lightDirView, 1, glm::value_ptr(_progUniforms.lightDirView));
	gl::Uniform1fv(_uniformLocations.specularExponent, 1, &_progUniforms.specularExponent);
	gl::Uniform1fv(_uniformLocations.metallicity, 1, &_progUniforms.metallicity);
	gl::Uniform1fv(_uniformLocations.reflectivity, 1, &_progUniforms.reflectivity);
	gl::Uniform3fv(_uniformLocations.albedo, 1, glm::value_ptr(_progUniforms.albedo));

	// Now that the uniforms are set, call another function to actually draw the mesh.
	gl::UniformMatrix4fv(_uniformLocations.mvpMtx, 1, GL_FALSE, glm::value_ptr(_progUniforms.mvpMatrix1));
	gl::UniformMatrix3fv(_uniformLocations.mvITMtx, 1, GL_FALSE, glm::value_ptr(_progUniforms.mvITMatrix1));
	drawMesh(0);
	// Now that the uniforms are set, call another function to actually draw the mesh.
	gl::UniformMatrix4fv(_uniformLocations.mvpMtx, 1, GL_FALSE, glm::value_ptr(_progUniforms.mvpMatrix2));
	gl::UniformMatrix3fv(_uniformLocations.mvITMtx, 1, GL_FALSE, glm::value_ptr(_progUniforms.mvITMatrix2));
	drawMesh(0);

	_deviceResources->scopeGraph.executeCommands();

	_deviceResources->uiRenderer.beginRendering();
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->scopeGraph.executeUICommands();
	_deviceResources->uiRenderer.endRendering();
}

/*!*********************************************************************************************************************
\brief  Update the description
***********************************************************************************************************************/
void OpenGLESPVRScopeExample::updateDescription()
{
	static char description[256];

	if (_deviceResources->scopeGraph.getCounterNum())
	{
		float maximum = _deviceResources->scopeGraph.getMaximumOfData(_selectedCounter);
		float userY = _deviceResources->scopeGraph.getMaximum(_selectedCounter);
		bool isKilos = false;
		if (maximum > 10000)
		{
			maximum /= 1000;
			userY /= 1000;
			isKilos = true;
		}
		bool isPercentage = _deviceResources->scopeGraph.isCounterPercentage(_selectedCounter);

		const char* standard = "Use up-down to select a counter, click to enable/disable it\n"
							   "Counter [%i]\n"
							   "Name: %s\n"
							   "Shown: %s\n"
							   "user y-axis: %.2f  max: %.2f\n";
		const char* percentage = "Use up-down to select a counter, click to enable/disable it\n"
								 "Counter [%i]\n"
								 "Name: %s\n"
								 "Shown: %s\n"
								 "user y-axis: %.2f%%  max: %.2f%%\n";
		const char* kilo = "Use up-down to select a counter, click to enable/disable it\n"
						   "Counter [%i]\n"
						   "Name: %s\n"
						   "Shown: %s\n"
						   "user y-axis: %.0fK  max: %.0fK\n";

		sprintf(description, isKilos ? kilo : isPercentage ? percentage : standard, _selectedCounter, _deviceResources->scopeGraph.getCounterName(_selectedCounter),
			_deviceResources->scopeGraph.isCounterShown(_selectedCounter) ? "Yes" : "No", userY, maximum);
		_deviceResources->uiRenderer.getDefaultDescription()->setColor(glm::vec4(1.f));
	}
	else
	{
		sprintf(description, "No counters present");
		_deviceResources->uiRenderer.getDefaultDescription()->setColor(glm::vec4(.8f, 0.0f, 0.0f, 1.0f));
	}
	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroUIRenderer
	_deviceResources->uiRenderer.getDefaultDescription()->setText(description);
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo()
{
	return std::unique_ptr<pvr::Shell>(new OpenGLESPVRScopeExample());
}
