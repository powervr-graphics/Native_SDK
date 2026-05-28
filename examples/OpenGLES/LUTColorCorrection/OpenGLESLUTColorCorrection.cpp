/*!
\brief Shows how to perform a lookup table color correction.
\file OpenGLESLUTColorCorrection.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsGles.h"
#include "PVRCore/textureio/TextureReaderPVR.h"
#include <thread>
#include <mutex>

const float RotateY = glm::pi<float>() / 150;
const glm::vec4 LightDir(.24f, .685f, -.685f, 0.0f);

#define DEFAULT_LUT_TEXTURE "lut_warm"
#define DEFAULT_LUT_SIZE "16"

// shader uniforms
namespace Uniforms {
enum Enum
{
	MVPMatrix,
	LightDir,
	Count
};

const char* names[] = { "MVPMatrix", "LightDirModel" };

} // namespace Uniforms

// content file names

// shader Source
const char VertexShaderFile[] = "VertShader_ES3.vsh";
const char FragmentShaderFile[] = "FragShader_ES3.fsh";
const char VertexShaderFile_PostProcessing[] = "VertShader_PostProcessing.vsh";
const char FragmentShaderFile_PostProcessing[] = "FragShader_PostProcessing.fsh";
const char FragmentShaderFile_LUTConversion[] = "FragShader_LUTConvert.fsh";

// PVR texture files
std::string TextureFileName = "Marble";
std::string BumpTextureFileName = "MarbleNormalMap";

// POD scene files
const char SceneFileName[] = "Satyr.pod";

bool astcSupported = false;

/// <summary>Class implementing the pvr::Shell functions.</summary>
class OpenGLESLUTColorCorrection : public pvr::Shell
{
	/// <summary>Platform agnostic command line argument parser.</summary>
	pvr::CommandLine _cmdLine{};

	// command line parameters
	std::string _inputLUTTextureFileName = DEFAULT_LUT_TEXTURE;
	std::string _inputLUTSize = DEFAULT_LUT_SIZE;
	int _currentNameIndex = 1;
	int _currentSizeIndex = 0;

	std::vector<std::string> _availableLUTBaseNames = { "lut_warm", "lut_cool" };
	std::vector<std::string> _availableLUTSizes = { "16", "32" };

	// 3D Model
	pvr::assets::ModelHandle _scene;

	// Projection and view matrix
	glm::mat4 _projMtx{1.0f};
	glm::mat4 _viewMtx{1.0f};

	struct DrawPass
	{
		glm::mat4 mvp;
		glm::vec3 lightDir;
	};

	struct DeviceResources
	{
		pvr::EglContext context{};

		// The Vertex buffer object handle array.
		std::vector<GLuint> vbos{};
		std::vector<GLuint> ibos{};
		GLuint program{ 0 };
		GLuint texture{ 0 };
		GLuint bumpTexture{ 0 };
		GLuint onScreenFbo{ 0 };

		// PostProcessing
		GLuint postProgram{ 0 };
		GLuint lutConversionProgram{ 0 };
		GLuint offscreenFbo{ 0 };
		GLuint offscreenColorTex{ 0 };
		GLuint offscreenDepthRbo{ 0 };
		GLuint lutConversionFbo{ 0 };

		// GLuint lutTexture;
		pvr::Texture lutInput2DTexture{};
		GLuint lut2DTexture{ 0 };
		GLuint lut3DTexture{ 0 };
		// Samplers
		GLuint samplerBilinear{ 0 };
		GLuint samplerTrilinear{ 0 };

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer{};

		~DeviceResources()
		{
			if (!vbos.empty()) { gl::DeleteBuffers(static_cast<GLsizei>(vbos.size()), vbos.data()); }
			if (!ibos.empty()) { gl::DeleteBuffers(static_cast<GLsizei>(ibos.size()), ibos.data()); }

			if (program) { gl::DeleteProgram(program); }
			if (postProgram) { gl::DeleteProgram(postProgram); }
			if (lutConversionProgram) { gl::DeleteProgram(lutConversionProgram); }

			if (texture) { gl::DeleteTextures(1, &texture); }
			if (bumpTexture) { gl::DeleteTextures(1, &bumpTexture); }
			if (lut2DTexture) { gl::DeleteTextures(1, &lut2DTexture); }
			if (lut3DTexture) { gl::DeleteTextures(1, &lut3DTexture); }
			if (offscreenColorTex) { gl::DeleteTextures(1, &offscreenColorTex); }

			if (samplerBilinear) { gl::DeleteSamplers(1, &samplerBilinear); }
			if (samplerTrilinear) { gl::DeleteSamplers(1, &samplerTrilinear); }

			if (onScreenFbo) { gl::DeleteFramebuffers(1, &onScreenFbo); }
			if (lutConversionFbo) { gl::DeleteFramebuffers(1, &lutConversionFbo); }
			if (offscreenFbo) { gl::DeleteFramebuffers(1, &offscreenFbo); }

			if (offscreenDepthRbo) { gl::DeleteRenderbuffers(1, &offscreenDepthRbo); }
			  
			uiRenderer.release();
			context.reset();
		}
	};

	glm::vec3 _clearColor = glm::vec3(0,0,0);

	// The translation and Rotate parameter of Model
	float _angleY = 0.0f;
	glm::vec3 _lightdir = glm::vec3(0, 0, 0);
	std::unique_ptr<DeviceResources> _deviceResources;

	pvr::utils::VertexConfiguration _vertexConfiguration;

	int32_t _uniformLocations[Uniforms::Count]{};

public:
	OpenGLESLUTColorCorrection() {}
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void eventMappedInput(pvr::SimplifiedInput key);

	// void executeGlCommands();
	void createProgram();
	void createPostProcessingProgram();
	void renderMesh(uint32_t nodeIndex);
	void renderLUT(const std::string name);
	void printTextureDetailsUI(const std::string& fullFileName, uint32_t size);
};

/// <summary>Code in printHelp() will be called by Shell once per run, before the rendering context is created
/// if the program is ran with '-options' argument.
/// Prints the available command-line options supported by the application.</summary>
/// <returns>None</returns>
static void printHelp()
{
	std::cout << std::endl;
	std::cout << "Supported command line options:" << std::endl;
	std::cout << "    -options              : Displays this help message" << std::endl;
	std::cout << "    -t or - texture       : Texture to be used as LUT input from the following: lut_cool, lut_warm." << std::endl;
	std::cout << "    -s or - size          : Size of the LUT texture: 16, 32." << std::endl;
}

/// <summary>Code in printTextureDetailsUI() is called at the end of loadLUT().
/// Prints in the display window the name and size of the LUT texture.</summary>
/// <param name="fullFileName"> The complete filename, including extension, of the LUT texture being displayed.</param>
/// <param name="size"> The dimension of the 3D LUT.</param>
/// <returns>None</returns>
void OpenGLESLUTColorCorrection::printTextureDetailsUI(const std::string& fullFileName, uint32_t size)
{
	if (!_deviceResources) return;

	if (!_deviceResources->uiRenderer.getDefaultTitle()) return;

	std::stringstream stream;

	stream << fullFileName << "\n";
	stream << size << " x " << size << " x " << size;

	_deviceResources->uiRenderer.getDefaultDescription()->setText(stream.str().c_str());
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
}

/// <summary> Code in eventMappedInput() is called when the user interacts with the touchscreen
/// of the mobile device, presses the arrow keys on keyboard or clicks the left mouse button.
/// Cycles between LUT textures with left, right arrow keys, or right side of screen tap/left click.
/// Cycles between LUT texture size with up, down arrow keys, or left side of screen tap/left click</summary>
/// <param name="key">The input key to be processed.</param>
/// <returns>None</returns>
void OpenGLESLUTColorCorrection::eventMappedInput(pvr::SimplifiedInput key)
{
	switch (key)
	{
		// mobile tap or left mouse click to select in the left 30% of screen. On keyboard is key '1'
		// cycles between texture sizes
	case pvr::SimplifiedInput::Action2:
		_currentSizeIndex++;
		if (_currentSizeIndex >= _availableLUTSizes.size()) _currentSizeIndex = 0;
		break;

		// mobile tap or left mouse click to select in the right 30% of screen. On keyboard is key '2'
		// cycles between texture names
	case pvr::SimplifiedInput::Action3:
		_currentNameIndex++;
		if (_currentNameIndex >= _availableLUTBaseNames.size()) _currentNameIndex = 0;
		break;

		// cycles between texture names
	case pvr::SimplifiedInput::Left:
		_currentNameIndex--;
		if (_currentNameIndex < 0) _currentNameIndex = static_cast<int>(_availableLUTBaseNames.size()) - 1;
		break;

		// cycles between texture names
	case pvr::SimplifiedInput::Right:
		_currentNameIndex++;
		if (_currentNameIndex >= _availableLUTBaseNames.size()) _currentNameIndex = 0;
		break;

		// cycles between texture sizes
	case pvr::SimplifiedInput::Up:
		_currentSizeIndex++;
		if (_currentSizeIndex >= _availableLUTSizes.size()) _currentSizeIndex = 0;
		break;

		// cycles between texture sizes
	case pvr::SimplifiedInput::Down:
		_currentSizeIndex--;
		if (_currentSizeIndex < 0) _currentSizeIndex = static_cast<int>(_availableLUTSizes.size()) - 1;
		break;

		// closes the application
	case pvr::SimplifiedInput::ActionClose:
		exitShell();
		break;

	default: return;
	}

	std::cout << "switching to: " << _availableLUTBaseNames[_currentNameIndex] << "_" << _availableLUTSizes[_currentSizeIndex] << std::endl;

	// loads the new LUT configuration
	renderLUT(_availableLUTBaseNames[_currentNameIndex]);
}

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result OpenGLESLUTColorCorrection::initApplication()
{
	std::cout << "Type -options to see the available command-line options for this demo." << std::endl;

	_cmdLine = this->getCommandLine();

	bool help = false;

	_cmdLine.getBoolOptionSetTrueIfPresent("-options", help);

	if (help)
	{
		printHelp();

		return pvr::Result::ExitRenderFrame;
	}

	_scene = pvr::assets::loadModel(*this, SceneFileName);
	_angleY = 0.0f;

	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by Shell once per run, just before exiting the program.
///	If the rendering context is lost, QuitApplication() will not be called.</summary>
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result OpenGLESLUTColorCorrection::quitApplication() { return pvr::Result::Success; }

/// <summary>Code in renderLUT() will be called in initView() and then every time there is a user input in eventMappedInput().</summary>
/// <param name="name">The base filename of the LUT texture to be loaded ("warm" or "cool").</param>
/// <returns>None</returns>
void OpenGLESLUTColorCorrection::renderLUT(const std::string name)
{
	std::string finalName = name + "_" + _availableLUTSizes[_currentSizeIndex];

	std::string fullFileName = finalName + (astcSupported ? "_astc.pvr" : ".pvr");

	std::cout << "Using file: " << fullFileName << std::endl;

	std::unique_ptr<pvr::Stream> stream;

	try
	{
		stream = getAssetStream(fullFileName);
	}
	catch (const std::exception&)
	{
		std::cout << "LUT file not found: " << fullFileName << ". Falling back to default." << std::endl;

		fullFileName = DEFAULT_LUT_TEXTURE "_" DEFAULT_LUT_SIZE ".pvr";
		stream = getAssetStream(fullFileName);
	}

	// read the LUT input texture
	_deviceResources->lutInput2DTexture = pvr::assetReaders::readPVR(*stream);

	pvr::ColorSpace colorSpace = _deviceResources->lutInput2DTexture.getColorSpace();

	if (colorSpace == pvr::ColorSpace::sRGB)
		std::cout << "LUT is sRGB" << std::endl;
	else
		std::cout << "LUT is Linear RGB" << std::endl;

	uint32_t iLUTTextureWidth = _deviceResources->lutInput2DTexture.getWidth();
	uint32_t iLUTTextureHeight = _deviceResources->lutInput2DTexture.getHeight();

	std::cout << "Input LUT sizes:" << std::endl;
	std::cout << iLUTTextureWidth << std::endl;
	std::cout << iLUTTextureHeight << std::endl;

	if (_deviceResources->lut2DTexture) { gl::DeleteTextures(1, &_deviceResources->lut2DTexture); }

	// send the 2D texture to the GPU
	_deviceResources->lut2DTexture = pvr::utils::textureUpload(*this, fullFileName);

	// cleanup existing LUT 3D texture
	if (_deviceResources->lut3DTexture)
	{
		gl::DeleteTextures(1, &_deviceResources->lut3DTexture);
		_deviceResources->lut3DTexture = 0;
	}

	gl::GenTextures(1, &_deviceResources->lut3DTexture);
	gl::BindTexture(GL_TEXTURE_3D, _deviceResources->lut3DTexture);
	gl::TexImage3D(GL_TEXTURE_3D, 0, GL_RGB8, iLUTTextureHeight, iLUTTextureHeight, iLUTTextureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	gl::TexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl::TexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::TexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::TexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	gl::TexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	GLint width;
	gl::GetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &width);
	std::cout << "3D texture width: " << width << std::endl;

	// create LUT fbo
	if (!_deviceResources->lutConversionFbo)
	{
		gl::GenFramebuffers(1, &_deviceResources->lutConversionFbo);
	}

	gl::BindFramebuffer(GL_FRAMEBUFFER, _deviceResources->lutConversionFbo);

	gl::ActiveTexture(GL_TEXTURE0);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->lut2DTexture);

	gl::Viewport(0, 0, iLUTTextureHeight, iLUTTextureHeight);

	gl::Disable(GL_DEPTH_TEST);

	gl::UseProgram(_deviceResources->lutConversionProgram);

	for (int slice = 0; slice < iLUTTextureHeight; slice++)
	{
		gl::FramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _deviceResources->lut3DTexture, 0, slice);

		gl::Uniform1f(gl::GetUniformLocation(_deviceResources->lutConversionProgram, "sliceIndex"), (float)slice);
		gl::Uniform1f(gl::GetUniformLocation(_deviceResources->lutConversionProgram, "lutSize"), (float)iLUTTextureHeight);

		gl::DrawArrays(GL_TRIANGLES, 0, 6);
	}

	gl::BindFramebuffer(GL_FRAMEBUFFER, _deviceResources->onScreenFbo);

	printTextureDetailsUI(fullFileName, iLUTTextureHeight);
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.</summary>
/// Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result OpenGLESLUTColorCorrection::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();
	_deviceResources->context = pvr::createEglContext();
	_deviceResources->context->init(getWindow(), getDisplay(), getDisplayAttributes());

	astcSupported = gl::isGlExtensionSupported("GL_KHR_texture_compression_astc_ldr");
	
	// init the UI
	_deviceResources->uiRenderer.init(
		getWidth(), getHeight(), isFullScreen(), (_deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2) || (getBackBufferColorspace() == pvr::ColorSpace::sRGB));
	_deviceResources->uiRenderer.getDefaultTitle()->setText("LUTColorCorrection");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();

	// create the default fbo using default params
	_deviceResources->onScreenFbo = _deviceResources->context->getOnScreenFbo();

	// create framebuffer
	gl::GenFramebuffers(1, &_deviceResources->offscreenFbo);
	gl::BindFramebuffer(GL_FRAMEBUFFER, _deviceResources->offscreenFbo);

	// generate a texture in the offscreen color texture
	gl::GenTextures(1, &_deviceResources->offscreenColorTex);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->offscreenColorTex);
	gl::TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWidth(), getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	// set the texture wrapping/filtering options (on the currently bound texture object)
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// extract the LUT texture name from the command line
	if (_cmdLine.hasOption("-t"))
	{
		bool stringOptionResult = _cmdLine.getStringOption("-t", _inputLUTTextureFileName);

		if (false == stringOptionResult) { _inputLUTTextureFileName = DEFAULT_LUT_TEXTURE; }
	}
	else if (_cmdLine.hasOption("-texture"))
	{
		bool stringOptionResult = _cmdLine.getStringOption("-texture", _inputLUTTextureFileName);

		if (false == stringOptionResult) { _inputLUTTextureFileName = DEFAULT_LUT_TEXTURE; }
	}

	// extract the LUT texture size from the command line
	if (_cmdLine.hasOption("-s"))
	{
		bool intOptionResult = _cmdLine.getStringOption("-s", _inputLUTSize);

		if (false == intOptionResult) { _inputLUTSize = DEFAULT_LUT_SIZE; }
	}
	else if (_cmdLine.hasOption("-size"))
	{
		bool intOptionResult = _cmdLine.getStringOption("-size", _inputLUTSize);

		if (false == intOptionResult) { _inputLUTSize = DEFAULT_LUT_SIZE; }
	}

	// set texture name index
	for (int i = 0; i < _availableLUTBaseNames.size(); i++)
	{
		if (_availableLUTBaseNames[i] == _inputLUTTextureFileName)
		{
			_currentNameIndex = i;
			break;
		}
	}

	// set texture size index
	for (int i = 0; i < _availableLUTSizes.size(); i++)
	{
		if (_availableLUTSizes[i] == _inputLUTSize)
		{
			_currentSizeIndex = i;
			break;
		}
	}

	// use the LUT 2D strip to 3D texture conversion program
	_deviceResources->lutConversionProgram = pvr::utils::createShaderProgram(*this, VertexShaderFile_PostProcessing, FragmentShaderFile_LUTConversion, nullptr, nullptr, 0);

	// make the 2D input texture strip to 3D texture conversion
	renderLUT(_availableLUTBaseNames[_currentNameIndex]);

	gl::BindFramebuffer(GL_FRAMEBUFFER, _deviceResources->offscreenFbo);

	// create depthbuffer
	gl::GenRenderbuffers(1, &_deviceResources->offscreenDepthRbo);
	gl::BindRenderbuffer(GL_RENDERBUFFER, _deviceResources->offscreenDepthRbo);

	// 16 bits of depht for each pixel stored in the depth buffer (enough for mobile applications)
	gl::RenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, getWidth(), getHeight());
	gl::Enable(GL_DEPTH_TEST);

	// color attachment 0 because we output the color to layout location 0
	// attach the offscreen texture to the framebuffer
	gl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _deviceResources->offscreenColorTex, 0);
	gl::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _deviceResources->offscreenDepthRbo);

	gl::GenSamplers(1, &_deviceResources->samplerBilinear);
	gl::SamplerParameteri(_deviceResources->samplerBilinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_deviceResources->samplerBilinear, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_deviceResources->samplerBilinear, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->samplerBilinear, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->samplerBilinear, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	pvr::utils::throwOnGlError("Sampler creation failed");

	_deviceResources->texture =
		pvr::utils::textureUpload(*this, TextureFileName + (astcSupported ? "_astc.pvr" : ".pvr"), _deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2);
	_deviceResources->bumpTexture =
		pvr::utils::textureUpload(*this, BumpTextureFileName + (astcSupported ? "_astc.pvr" : ".pvr"), _deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2);
	pvr::utils::throwOnGlError("Texture creation failed");

	gl::GenSamplers(1, &_deviceResources->samplerTrilinear);
	gl::SamplerParameteri(_deviceResources->samplerTrilinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gl::SamplerParameteri(_deviceResources->samplerTrilinear, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_deviceResources->samplerTrilinear, GL_TEXTURE_WRAP_R, GL_REPEAT);
	gl::SamplerParameteri(_deviceResources->samplerTrilinear, GL_TEXTURE_WRAP_S, GL_REPEAT);
	gl::SamplerParameteri(_deviceResources->samplerTrilinear, GL_TEXTURE_WRAP_T, GL_REPEAT);
	pvr::utils::throwOnGlError("Sampler creation failed");

	// Load the vbo and ibo data
	pvr::utils::appendSingleBuffersFromModel(*_scene, _deviceResources->vbos, _deviceResources->ibos);

	createProgram();
	createPostProcessingProgram();

	bool isRotated = this->isScreenRotated();
	if (!isRotated)
	{
		_projMtx = glm::perspective(_scene->getCamera(0).getFOV(), static_cast<float>(this->getWidth()) / static_cast<float>(this->getHeight()), _scene->getCamera(0).getNear(),
			_scene->getCamera(0).getFar());
	}
	else
	{
		_projMtx = pvr::math::perspective(pvr::Api::OpenGLES2, _scene->getCamera(0).getFOV(), static_cast<float>(this->getHeight()) / static_cast<float>(this->getWidth()),
			_scene->getCamera(0).getNear(), _scene->getCamera(0).getFar(), glm::pi<float>() * .5f);
	}

	float fov;
	glm::vec3 cameraPos, cameraTarget, cameraUp;

	_scene->getCameraProperties(0, fov, cameraPos, cameraTarget, cameraUp);
	_viewMtx = glm::lookAt(cameraPos, cameraTarget, cameraUp);
	debugThrowOnApiError("InitView: Exit");

	return pvr::Result::Success;
}

/// <summary>Code in createProgram() will be called upon initialization.</summary>
/// Used to create the program 
/// <returns>None</returns>
void OpenGLESLUTColorCorrection::createProgram()
{
	static const char* attribs[] = { "inVertex", "inNormal", "inTexCoord" };
	static const uint16_t attribIndices[] = { 0, 1, 2 };

	// Enable or disable gamma correction based on if it is automatically performed on the framebuffer or we need to do it in the shader.
	const char* defines[] = { "FRAMEBUFFER_SRGB" };
	uint32_t numDefines = 1;

	glm::vec3 clearColorLinearSpace(0.0f, 0.45f, 0.41f);
	_clearColor = clearColorLinearSpace;
	if (getBackBufferColorspace() != pvr::ColorSpace::sRGB)
	{
		_clearColor = pvr::utils::convertLRGBtoSRGB(clearColorLinearSpace); // Gamma correct the clear colour...
		numDefines = 0;
	}

	_deviceResources->program = pvr::utils::createShaderProgram(*this, VertexShaderFile, FragmentShaderFile, attribs, attribIndices, 3, defines, numDefines);

	_uniformLocations[Uniforms::MVPMatrix] = gl::GetUniformLocation(_deviceResources->program, Uniforms::names[Uniforms::MVPMatrix]);
	_uniformLocations[Uniforms::LightDir] = gl::GetUniformLocation(_deviceResources->program, Uniforms::names[Uniforms::LightDir]);

	gl::UseProgram(_deviceResources->program);
	gl::Uniform1i(gl::GetUniformLocation(_deviceResources->program, "sBaseTex"), 0);
	gl::Uniform1i(gl::GetUniformLocation(_deviceResources->program, "sNormalMap"), 1);

	const pvr::utils::VertexBindings_Name vertexBindings[] = {
		{ "POSITION", "inVertex" },
		{ "NORMAL", "inNormal" },
		{ "UV0", "inTexCoord" },
		{ "TANGENT", "inTangent" },
	};

	_vertexConfiguration = pvr::utils::createInputAssemblyFromMesh(_scene->getMesh(0), vertexBindings, 4);
}

/// <summary>Code in createPostProcessingProgram() will be called upon initialization.</summary>
/// Used to create the program
/// <returns>None</returns>
void OpenGLESLUTColorCorrection::createPostProcessingProgram()
{
	_deviceResources->postProgram = pvr::utils::createShaderProgram(*this, VertexShaderFile_PostProcessing, FragmentShaderFile_PostProcessing, nullptr, nullptr, 0);
}

/// <summary>Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context. </summary>
/// <returns>Return pvr::Result::Success if no error occurred </returns>
pvr::Result OpenGLESLUTColorCorrection::releaseView()
{
	_deviceResources.reset();
	if (_deviceResources) _deviceResources->uiRenderer.release();
	_scene.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result OpenGLESLUTColorCorrection::renderFrame()
{
	debugThrowOnApiError("RenderFrame: Entrance");

	// Rotate and Translation the model matrix
	gl::BindFramebuffer(GL_FRAMEBUFFER, _deviceResources->offscreenFbo);
	gl::ClearColor(_clearColor.r, _clearColor.g, _clearColor.b, 1.f);
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl::Enable(GL_CULL_FACE);
	gl::UseProgram(_deviceResources->program);

	// create the pipeline layout
	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);
	gl::Enable(GL_DEPTH_TEST);

	gl::ActiveTexture(GL_TEXTURE0);
	gl::BindSampler(0, _deviceResources->samplerTrilinear);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->texture);

	gl::ActiveTexture(GL_TEXTURE1);
	gl::BindSampler(1, _deviceResources->samplerTrilinear);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->bumpTexture);

	glm::mat4 mModel = glm::rotate(_angleY, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(1.8f));
	_angleY += -RotateY * 0.05f * getFrameTime();

	gl::Uniform3fv(_uniformLocations[Uniforms::LightDir], 1, glm::value_ptr(LightDir * mModel));

	glm::mat4 mvp = (_projMtx * _viewMtx) * mModel * _scene->getWorldMatrix(_scene->getNode(0).getObjectId());
	gl::UniformMatrix4fv(_uniformLocations[Uniforms::MVPMatrix], 1, GL_FALSE, glm::value_ptr(mvp));

	// Now that the uniforms are set, call another function to actually draw the mesh.
	renderMesh(0);

	// postprocessing pass
	gl::BindFramebuffer(GL_FRAMEBUFFER, _deviceResources->onScreenFbo);
	gl::Viewport(0, 0, getWidth(), getHeight());

	gl::Disable(GL_DEPTH_TEST);

	gl::UseProgram(_deviceResources->postProgram);

	gl::ActiveTexture(GL_TEXTURE0);
	gl::BindSampler(0, _deviceResources->samplerBilinear);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->offscreenColorTex);

	gl::ActiveTexture(GL_TEXTURE1);
	gl::BindSampler(1, _deviceResources->samplerBilinear);
	gl::BindTexture(GL_TEXTURE_3D, _deviceResources->lut3DTexture);

	gl::Uniform1i(gl::GetUniformLocation(_deviceResources->postProgram, "sLUTexture"), 1);

	gl::Uniform1i(gl::GetUniformLocation(_deviceResources->postProgram, "sSceneTexture"), 0);
	gl::DrawArrays(GL_TRIANGLES, 0, 6);

	_deviceResources->uiRenderer.beginRendering();
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.endRendering();

	if (this->shouldTakeScreenshot()) { pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight()); }

	_deviceResources->context->swapBuffers();

	return pvr::Result::Success;
}

/// <summary>Draws a pvr::assets::Mesh after the model view matrix has been set and the material prepared.</summary>
/// <param name="nodeIndex">Node index of the mesh to draw </param>
/// <returns>None</returns>
void OpenGLESLUTColorCorrection::renderMesh(uint32_t nodeIndex)
{
	const pvr::assets::Model::Node& node = _scene->getNode(nodeIndex);
	const pvr::assets::Mesh& mesh = _scene->getMesh(node.getObjectId());

	gl::BindBuffer(GL_ARRAY_BUFFER, _deviceResources->vbos[node.getObjectId()]);

	assertion(_vertexConfiguration.bindings.size() == 1, "This demo assumes only one VBO per mesh");

	for (auto it = _vertexConfiguration.attributes.begin(), end = _vertexConfiguration.attributes.end(); it != end; ++it)
	{
		gl::EnableVertexAttribArray(it->index);
		gl::VertexAttribPointer(it->index, it->width, pvr::utils::convertToGles(it->format), pvr::dataTypeIsNormalised(it->format),
			_vertexConfiguration.bindings[it->binding].strideInBytes, reinterpret_cast<const void*>(static_cast<uintptr_t>(it->offsetInBytes)));
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
	for (auto it = _vertexConfiguration.attributes.begin(), end = _vertexConfiguration.attributes.end(); it != end; ++it) { gl::DisableVertexAttribArray(it->index); }
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::unique_ptr<pvr::Shell>(new OpenGLESLUTColorCorrection()); }
