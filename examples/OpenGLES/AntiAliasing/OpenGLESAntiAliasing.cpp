
/*!
\brief  Shows how to implement different anti-aliasing techniques.
\file

\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsGles.h"
#include "PVRAssets/fileio/GltfReader.h"
#include "PVRCore/cameras/TPSCamera.h"
#include <thread>
#include <mutex>

const float RotateY = glm::pi<float>() / 150;
const glm::vec4 LightDir(.24f, .685f, -.685f, 0.0f);

// Antialiasing methods
const char NoAntialiasing[] = "No Anti Aliasing";
const char MsAntialiasing[] = "Multi Sampled Anti Aliasing";
const char FxAntiAliasing[] = "Fast Approximate Anti Aliasing";
const char TxAntiAliasing[] = "Temporal Approximate Anti Aliasing";

// Shader Source Files
const char VertexShaderFile[] = "VertShader.vsh";
const char FragmentShaderFile[] = "FragShader.fsh";
const char AttributelessVertexShaderFile[] = "AttributelessVertexShader.vsh";
const char OffscreenFragmentShaderFile[] = "Offscreen_FragShader.fsh";
const char NOAAVertexShaderFile[] = "NOAA_VertShader.vsh";
const char NOAAFragmentShaderFile[] = "NOAA_FragShader.fsh";
const char MSAAFragmentShaderFile[] = "MSAA_FragShader.fsh";
const char VelocityTXAAVertexShaderFile[] = "VelocityTXAA_VertShader.vsh";
const char VelocityTXAAFragmentShaderFile[] = "VelocityTXAA_FragShader.fsh";
const char ResolveTXAAFragmentShaderFile[] = "ResolveTXAA_FragShader.fsh";
const char FXAAFragmentShaderFile[] = "FXAA_FragShader.fsh";

// PVR texture files
std::string TextureFileName = "Marble";
std::string BumpTextureFileName = "MarbleNormalMap";

// POD scene files
const char SceneFileName[] = "Satyr.pod";

// Anti-Aliasing methods
enum AntiAliasingMethod
{
	NOAA,
	MSAA,
	FXAA,
	TXAA
};

/*
- Vertex shader uniforms list
- PreModel, PreProjView, PreWorld, Jitter enums declared for previous frame information to be used for TXAA
*/
enum VertexUniforms
{
	PreModel,
	PreProjView,
	PreWorld,

	CurrMVPMatrix,
	CurrLightDir,

	CurrModel,
	CurrProjView,
	CurrWorld,
	Jitter,

	UniformCount
};

/*
- Uniform names to be used in shaders
*/
const char* names[] = { "MVPMatrix", "LightDirModel" };
const char* uniformNames[] = { "PreModel", "PreProjView", "PreWorld", "CurrMVPMatrix", "CurrLightDir", "CurrModel", "CurrProjView", "CurrWorld", "uJitter" };

// <summary> Implementing the pvr::Shell functions <summary>
class OpenGLESAntiAliasing : public pvr::Shell
{
	// 3D Model
	pvr::assets::ModelHandle _scene;

	// Orbit camera
	pvr::TPSOrbitCamera _camera;

	// Projection and view matrix
	glm::mat4 _projMtx, _viewMtx;
	glm::mat4 _worldMtx;

	// Projection and view matrices of previous frame for TXAA
	glm::mat4 _preModelMtx = glm::mat4(1.0);
	glm::mat4 _preProjViewMtx = glm::mat4(1.0);
	glm::mat4 _preWorldMtx = glm::mat4(1.0);

	struct DrawPass
	{
		glm::mat4 mvp;
		glm::vec3 lightDir;
	};

	struct DeviceResources
	{
		pvr::EglContext context;

		// The Vertex buffer object handle array.
		std::vector<GLuint> _vbos;
		std::vector<GLuint> _ibos;

		GLuint _program;
		GLuint _offscreenProgram;

		GLuint _texture;
		GLuint _bumpTexture;

		// Texture samplers
		GLuint _samplerNearest;
		GLuint _samplerLinear;
		GLuint _samplerTrilinear;

		const GLenum buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

		// UIRenderer used to display text
		pvr::ui::UIRenderer _uiRenderer;

		DeviceResources() : _program(0), _texture(0), _bumpTexture(0), _samplerNearest(0), _samplerLinear(0), _samplerTrilinear(0){};

		~DeviceResources(){};
	};

	/// <summary>
	/// Resources for No Anti-Aliasing
	/// noaaProgram used for No Anti-Aliasing shaders
	/// </summary>
	struct NOAAResources
	{
		GLuint _noaaProgram;
		int32_t _uniformLocations[VertexUniforms::UniformCount];

		NOAAResources() : _noaaProgram(0) {}
	};

	/// <summary>
	/// Resources for Multi Sampled Anti-Aliasing
	///
	/// msaaProgram used for Multi Sampled Anti-Aliasing shaders
	/// offscreenTexture is a multisampled texture for offscreen framebuffer
	/// offscreenDepthTexture is a multisampled depth texture for offscreen framebuffer
	/// multisampledFbo is a multisampled framebuffer object that offscreenTexture and offscreenDepthTexture is attached
	///
	/// </summary>
	struct MSAAResources
	{
		GLuint _msaaProgram;
		GLuint _multisampledFbo;
		GLuint _offscreenTexture;
		GLuint _offscreenDepthTexture;

		int32_t _uniformLocations[VertexUniforms::UniformCount];

		MSAAResources() : _msaaProgram(0), _multisampledFbo(0), _offscreenTexture(0), _offscreenDepthTexture(0) {}
	};

	/// <summary>
	/// Resources for Fast Approximate Anti-Aliasing
	///
	/// fxaaProgram used for Fast Approximate Anti-Aliasing shaders
	/// offscreenTexture and offscreenDepthTexture does not require any specification for FXAA
	///
	/// </summary>
	struct FXAAResources
	{
		GLuint _fxaaProgram;
		GLuint _offscreenFbo;
		GLuint _offscreenTexture;
		GLuint _offscreenDepthTexture;

		int32_t _uniformLocations[VertexUniforms::UniformCount];

		FXAAResources() : _fxaaProgram(0), _offscreenFbo(0), _offscreenTexture(0), _offscreenDepthTexture(0) {}
	};

	/// <summary>
	/// Resources for Temporal Anti-Aliasing
	///
	/// velocityProgram is used for TXAA pipelines first step
	/// resolveProgram is used for TXAA pipelines second step
	///
	/// velocityTexture is a texture that storages pixel position differences
	/// historyTexture is a texture that storages last frame
	/// updatedHistoryTexture is a texture that storages last frame after processes
	///
	/// resolveFbo is a framebuffer object that historyTexture object is attached
	/// updateHistoryFbo is a framebuffer object that updatedHistoryTexture object is attached
	///
	/// calculateJitterParameter function creates jitter amount for each frame based on frame offset
	///
	/// </summary>
	struct TXAAResources
	{
		GLuint _txaaProgram;

		GLuint _velocityProgram;
		GLuint _resolveProgram;

		GLuint _offscreenFbo;
		GLuint _resolveFbo;
		GLuint _updateHistoryFbo;

		GLuint _offscreenTexture;
		GLuint _offscreenDepthTexture;

		GLuint _velocityTexture;
		GLuint _historyTexture;
		GLuint _updatedHistoryTexture;

		int _frameOffset = 0;
		int _frameCount = 16;
		float _jitter[16][2];

		int32_t _uniformLocations[VertexUniforms::UniformCount];

		TXAAResources() : _txaaProgram(0), _resolveProgram(), _velocityProgram(0), _offscreenFbo(0), _resolveFbo(0), _updateHistoryFbo(0) {}

		float createHaltonSequence(unsigned int index, int base)
		{
			float f = 1;
			float r = 0;

			int current = index;
			do {
				f = f / base;
				r = r + f * (current % base);
				current = glm::floor(current / base);
			} while (current > 0);

			return r;
		}

		void calculateJitterParameter(int screenWidth, int screenHeight)
		{
			for (int i = 0; i < 16; i++)
			{
				float x = createHaltonSequence(i + 1, 2);
				float y = createHaltonSequence(i + 1, 3);

				_jitter[i][0] = x;
				_jitter[i][1] = y;

				_jitter[i][0] = ((x - 0.5f) / (float)screenWidth) * 2;
				_jitter[i][1] = ((y - 0.5f) / (float)screenHeight) * 2;
			}
		}
	};

	glm::vec3 _clearColor;

	// The translation and Rotate parameter of Model
	float _angleY;
	glm::vec3 _lightdir;

	std::unique_ptr<DeviceResources> _deviceResources;

	// Resources objects for each anti-aliasing method
	std::unique_ptr<NOAAResources> _noaaResources;
	std::unique_ptr<MSAAResources> _msaaResources;
	std::unique_ptr<FXAAResources> _fxaaResources;
	std::unique_ptr<TXAAResources> _txaaResources;

	int32_t _uniformLocations[VertexUniforms::UniformCount];
	pvr::utils::VertexConfiguration _vertexConfiguration;

	// Current selected AA method to be changed later with inputs
	AntiAliasingMethod _currentMethod = AntiAliasingMethod::NOAA;

	/// <summary>Flag to know whether astc iss upported by the physical device.</summary>
	bool _astcSupported;
	int _inputIndex = 0;

public:
	OpenGLESAntiAliasing() {}

	virtual pvr::Result initApplication();
	virtual pvr::Result initView();

	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	virtual void eventMappedInput(pvr::SimplifiedInput key);
	void changeCurrentMethod();

	void createProgram();

	// Creating OpenGL ES pipeline for each AA technique
	void createMSAAPipeline();
	void createFXAAPipeline();
	void createTXAAPipeline();

	// Rendering functions for each AA technique
	void renderNOAA();
	void renderMSAA();
	void renderFXAA();
	void renderTXAA();

	// Rendering UI with AA technique name
	void renderUI(const std::string antiAliasingName);
	void renderMesh(uint32_t nodeIndex);
	void renderOffscreenQuad();
};

/// <summary>
/// Changes current selected anti-aliasing technique based on input
/// On touchscreen devices "Up" means swipe up, "Left" means swipe left, "Right" is swipe to right, default one is swipe down...
/// </summary>
void OpenGLESAntiAliasing::eventMappedInput(pvr::SimplifiedInput key)
{
	switch (key)
	{
	case pvr::SimplifiedInput::ActionClose: exitShell(); break;
	case pvr::SimplifiedInput::Action1: changeCurrentMethod(); break;
	default: _currentMethod = AntiAliasingMethod::NOAA; break;
	}
}

/// <summary>
/// Changes current anti-aliasing method based on user input
/// </summary>
void OpenGLESAntiAliasing::changeCurrentMethod()
{
	_inputIndex++;
	_inputIndex = _inputIndex % 4;
	_currentMethod = static_cast<AntiAliasingMethod>(_inputIndex);
}

/// <summary>
/// - Renders UI elements like logo and title with given AA technique as parameter
/// </summary>
void OpenGLESAntiAliasing::renderUI(const std::string antiAliasingMethod)
{
	_deviceResources->_uiRenderer.getDefaultTitle()->setText(antiAliasingMethod);
	_deviceResources->_uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->_uiRenderer.beginRendering();
	_deviceResources->_uiRenderer.getDefaultTitle()->render();
	_deviceResources->_uiRenderer.getDefaultDescription()->render();
	_deviceResources->_uiRenderer.getSdkLogo()->render();
	_deviceResources->_uiRenderer.endRendering();
}

/// <summary>
/// - Creates shaders with given attributes and definitions
/// - Attributes are filled with data while reading from mesh
///
/// </summary>
void OpenGLESAntiAliasing::createProgram()
{
	static const char* attribs[] = { "inVertex", "inNormal", "inTexCoord" };
	static const uint16_t attribIndices[] = { 0, 1, 2 };
	static const uint16_t ppAttribIndices[] = { 0, 2, 1 };

	// Enable or disable gamma correction based on if it is automatically performed on the framebuffer or we need to do it in the shader.
	const char* defines[] = { "FRAMEBUFFER_SRGB" };
	uint32_t numDefines = 1;

	glm::vec3 clearColorLinearSpace(0.7f, 0.8f, 0.9f);
	_clearColor = clearColorLinearSpace;

	if (getBackBufferColorspace() != pvr::ColorSpace::sRGB)
	{
		_clearColor = pvr::utils::convertLRGBtoSRGB(clearColorLinearSpace); // Gamma correct the clear colour...
		numDefines = 0;
	}

	_deviceResources->_program = pvr::utils::createShaderProgram(*this, VertexShaderFile, FragmentShaderFile, attribs, attribIndices, 3, defines, numDefines);
	_deviceResources->_offscreenProgram =
		pvr::utils::createShaderProgram(*this, AttributelessVertexShaderFile, OffscreenFragmentShaderFile, attribs, ppAttribIndices, 3, defines, numDefines);

	_noaaResources->_noaaProgram = pvr::utils::createShaderProgram(*this, NOAAVertexShaderFile, NOAAFragmentShaderFile, attribs, attribIndices, 3, defines, numDefines);
	_msaaResources->_msaaProgram = pvr::utils::createShaderProgram(*this, AttributelessVertexShaderFile, MSAAFragmentShaderFile, attribs, ppAttribIndices, 3, defines, numDefines);
	_fxaaResources->_fxaaProgram = pvr::utils::createShaderProgram(*this, AttributelessVertexShaderFile, FXAAFragmentShaderFile, attribs, ppAttribIndices, 3, defines, numDefines);
	_txaaResources->_velocityProgram =
		pvr::utils::createShaderProgram(*this, VelocityTXAAVertexShaderFile, VelocityTXAAFragmentShaderFile, attribs, attribIndices, 3, defines, numDefines);
	_txaaResources->_resolveProgram =
		pvr::utils::createShaderProgram(*this, AttributelessVertexShaderFile, ResolveTXAAFragmentShaderFile, attribs, ppAttribIndices, 3, defines, numDefines);

	_uniformLocations[VertexUniforms::CurrMVPMatrix] = gl::GetUniformLocation(_deviceResources->_program, "MVPMatrix");
	_uniformLocations[VertexUniforms::CurrLightDir] = gl::GetUniformLocation(_deviceResources->_program, "LightDirModel");

	gl::UseProgram(_deviceResources->_program);
	gl::Uniform1i(gl::GetUniformLocation(_deviceResources->_program, "sBaseTex"), 0);
	gl::Uniform1i(gl::GetUniformLocation(_deviceResources->_program, "sNormalMap"), 1);

	gl::UseProgram(_deviceResources->_offscreenProgram);
	gl::Uniform1i(gl::GetUniformLocation(_deviceResources->_offscreenProgram, "screenTexture"), 2);

	gl::UseProgram(_noaaResources->_noaaProgram);
	gl::Uniform1i(gl::GetUniformLocation(_noaaResources->_noaaProgram, "sBaseTex"), 0);
	gl::Uniform1i(gl::GetUniformLocation(_noaaResources->_noaaProgram, "sNormalMap"), 1);

	gl::UseProgram(_msaaResources->_msaaProgram);
	gl::Uniform1i(gl::GetUniformLocation(_msaaResources->_msaaProgram, "screenTexture"), 2);

	gl::UseProgram(_fxaaResources->_fxaaProgram);
	gl::Uniform1i(gl::GetUniformLocation(_fxaaResources->_fxaaProgram, "screenTexture"), 2);

	gl::UseProgram(_txaaResources->_velocityProgram);
	gl::Uniform1i(gl::GetUniformLocation(_txaaResources->_velocityProgram, "sBaseTex"), 0);
	gl::Uniform1i(gl::GetUniformLocation(_txaaResources->_velocityProgram, "sNormalMap"), 1);

	gl::UseProgram(_txaaResources->_resolveProgram);
	gl::Uniform1i(gl::GetUniformLocation(_txaaResources->_resolveProgram, "screenTexture"), 2);
	gl::Uniform1i(gl::GetUniformLocation(_txaaResources->_resolveProgram, "historyTexture"), 3);
	gl::Uniform1i(gl::GetUniformLocation(_txaaResources->_resolveProgram, "velocityTexture"), 4);

	// Set uniform data for TXAA shaders
	_txaaResources->_uniformLocations[VertexUniforms::PreModel] = gl::GetUniformLocation(_txaaResources->_velocityProgram, uniformNames[VertexUniforms::PreModel]);
	_txaaResources->_uniformLocations[VertexUniforms::PreProjView] = gl::GetUniformLocation(_txaaResources->_velocityProgram, uniformNames[VertexUniforms::PreProjView]);
	_txaaResources->_uniformLocations[VertexUniforms::PreWorld] = gl::GetUniformLocation(_txaaResources->_velocityProgram, uniformNames[VertexUniforms::PreWorld]);

	_txaaResources->_uniformLocations[VertexUniforms::CurrMVPMatrix] = gl::GetUniformLocation(_txaaResources->_velocityProgram, uniformNames[VertexUniforms::CurrMVPMatrix]);
	_txaaResources->_uniformLocations[VertexUniforms::CurrLightDir] = gl::GetUniformLocation(_txaaResources->_velocityProgram, uniformNames[VertexUniforms::CurrLightDir]);
	_txaaResources->_uniformLocations[VertexUniforms::CurrModel] = gl::GetUniformLocation(_txaaResources->_velocityProgram, uniformNames[VertexUniforms::CurrModel]);
	_txaaResources->_uniformLocations[VertexUniforms::CurrProjView] = gl::GetUniformLocation(_txaaResources->_velocityProgram, uniformNames[VertexUniforms::CurrProjView]);
	_txaaResources->_uniformLocations[VertexUniforms::CurrWorld] = gl::GetUniformLocation(_txaaResources->_velocityProgram, uniformNames[VertexUniforms::CurrWorld]);

	_txaaResources->_uniformLocations[VertexUniforms::Jitter] = gl::GetUniformLocation(_txaaResources->_velocityProgram, uniformNames[VertexUniforms::Jitter]);

	_txaaResources->calculateJitterParameter(this->getWidth(), this->getHeight());

	const pvr::utils::VertexBindings_Name vertexBindings[] = {
		{ "POSITION", "inVertex" },
		{ "NORMAL", "inNormal" },
		{ "UV0", "inTexCoord" },
		{ "TANGENT", "inTangent" },
	};

	_vertexConfiguration = pvr::utils::createInputAssemblyFromMesh(_scene->getMesh(0), vertexBindings, 4);

	gl::UseProgram(0);
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.</summary>
/// Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result OpenGLESAntiAliasing::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();
	_deviceResources->context = pvr::createEglContext();
	_deviceResources->context->init(getWindow(), getDisplay(), getDisplayAttributes(), pvr::Api::OpenGLES2, pvr::Api::OpenGLES31);

	_astcSupported = gl::isGlExtensionSupported("GL_KHR_texture_compression_astc_ldr");

	_deviceResources->_texture = pvr::utils::textureUpload(*this, TextureFileName + (_astcSupported ? "_astc.pvr" : ".pvr"), _deviceResources->context->getApiVersion() == pvr::Api::OpenGLES31);
	_deviceResources->_bumpTexture = pvr::utils::textureUpload(*this, BumpTextureFileName + (_astcSupported ? "_astc.pvr" : ".pvr"), _deviceResources->context->getApiVersion() == pvr::Api::OpenGLES31);

	_deviceResources->_uiRenderer.init(
		getWidth(), getHeight(), isFullScreen(), (_deviceResources->context->getApiVersion() == pvr::Api::OpenGLES31) || (getBackBufferColorspace() == pvr::ColorSpace::sRGB));
	_deviceResources->_uiRenderer.getDefaultTitle()->setText("AntiAliasing Techniques");
	_deviceResources->_uiRenderer.getDefaultTitle()->commitUpdates();

	_noaaResources = std::make_unique<NOAAResources>();
	_msaaResources = std::make_unique<MSAAResources>();
	_fxaaResources = std::make_unique<FXAAResources>();
	_txaaResources = std::make_unique<TXAAResources>();

	createProgram();

	createMSAAPipeline();
	createFXAAPipeline();
	createTXAAPipeline();

	gl::GenSamplers(1, &_deviceResources->_samplerTrilinear);
	gl::SamplerParameteri(_deviceResources->_samplerTrilinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gl::SamplerParameteri(_deviceResources->_samplerTrilinear, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_deviceResources->_samplerTrilinear, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->_samplerTrilinear, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->_samplerTrilinear, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	pvr::utils::throwOnGlError("Trilinear Sampler creation failed");

	gl::GenSamplers(1, &_deviceResources->_samplerLinear);
	gl::SamplerParameteri(_deviceResources->_samplerLinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_deviceResources->_samplerLinear, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_deviceResources->_samplerLinear, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->_samplerLinear, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->_samplerLinear, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	pvr::utils::throwOnGlError("Linear Sampler creation failed");

	gl::GenSamplers(1, &_deviceResources->_samplerNearest);
	gl::SamplerParameteri(_deviceResources->_samplerNearest, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl::SamplerParameteri(_deviceResources->_samplerNearest, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl::SamplerParameteri(_deviceResources->_samplerNearest, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->_samplerNearest, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->_samplerNearest, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	pvr::utils::throwOnGlError("Nearest Sampler creation failed");

	// Load the vbo and ibo data
	pvr::utils::appendSingleBuffersFromModel(*_scene, _deviceResources->_vbos, _deviceResources->_ibos);

	auto lastError = gl::GetError();

	bool isRotated = this->isScreenRotated();
	if (!isRotated)
	{
		_projMtx = glm::perspective(_scene->getCamera(0).getFOV(), static_cast<float>(this->getWidth()) / static_cast<float>(this->getHeight()), _scene->getCamera(0).getNear(),
			_scene->getCamera(0).getFar());
	}
	else
	{
		_projMtx = pvr::math::perspective(pvr::Api::OpenGLES31, _scene->getCamera(0).getFOV(), static_cast<float>(this->getHeight()) / static_cast<float>(this->getWidth()),
			_scene->getCamera(0).getNear(), _scene->getCamera(0).getFar(), glm::pi<float>() * .5f);
	}

	float fov;
	glm::vec3 cameraPos, cameraTarget, cameraUp;

	_scene->getCameraProperties(0, fov, cameraPos, cameraTarget, cameraUp);
	_viewMtx = glm::lookAt(cameraPos, cameraTarget, cameraUp);

	debugThrowOnApiError("InitView: Exit");

	return pvr::Result::Success;
}

/// <summary>
///
/// - Creates Multi Sampled Anti Aliasing pipeline with textures and framebuffer objects
/// - First step of pipeline requires multisampled framebuffer object with multisampled texture attached to it
///
/// </summary>
void OpenGLESAntiAliasing::createMSAAPipeline()
{
	// Create a framebuffer object
	gl::GenFramebuffers(1, &_msaaResources->_multisampledFbo);
	gl::BindFramebuffer(GL_FRAMEBUFFER, _msaaResources->_multisampledFbo);

	// Create a multisampled color texture with 4 samples and attach it to framebuffer
	gl::GenTextures(1, &_msaaResources->_offscreenTexture);
	gl::BindTexture(GL_TEXTURE_2D_MULTISAMPLE, _msaaResources->_offscreenTexture);
	gl::TexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_SRGB8_ALPHA8, getWidth(), getHeight(), GL_TRUE);
	gl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, _msaaResources->_offscreenTexture, 0);

	// Create a multisampled depth texture with 4 samples and attach it to framebuffer
	gl::GenTextures(1, &_msaaResources->_offscreenDepthTexture);
	gl::BindTexture(GL_TEXTURE_2D_MULTISAMPLE, _msaaResources->_offscreenDepthTexture);
	gl::TexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_DEPTH_COMPONENT32F, getWidth(), getHeight(), GL_TRUE);
	gl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, _msaaResources->_offscreenDepthTexture, 0);

	// Check if any error occurs while doing framebuffer operations
	GLenum frameBufferStatus = gl::CheckFramebufferStatus(GL_FRAMEBUFFER);
	if (frameBufferStatus != GL_FRAMEBUFFER_COMPLETE) { debugThrowOnApiError("Framebuffer operation is not complete for MSAA!"); }

	// bind to default
	gl::BindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
}

/// <summary>
///
/// - FXAA technique pipeline requires a simple offscreen framebuffer creation that does not necessarily require texture specification like multisampling
///
/// </summary>
void OpenGLESAntiAliasing::createFXAAPipeline()
{
	// Create offscreen framebuffer object
	gl::GenFramebuffers(1, &_fxaaResources->_offscreenFbo);
	gl::BindFramebuffer(GL_FRAMEBUFFER, _fxaaResources->_offscreenFbo);

	// Create color texture and attach it to framebuffer
	gl::GenTextures(1, &_fxaaResources->_offscreenTexture);
	gl::BindTexture(GL_TEXTURE_2D, _fxaaResources->_offscreenTexture);
	gl::TexStorage2D(GL_TEXTURE_2D, 1, GL_SRGB8_ALPHA8, getWidth(), getHeight());
	gl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _fxaaResources->_offscreenTexture, 0);

	// Create depth texture and attach it to framebuffer
	gl::GenTextures(1, &_fxaaResources->_offscreenDepthTexture);
	gl::BindTexture(GL_TEXTURE_2D, _fxaaResources->_offscreenDepthTexture);
	gl::TexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, getWidth(), getHeight());
	gl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _fxaaResources->_offscreenDepthTexture, 0);

	// Bind to default
	gl::BindTexture(GL_TEXTURE_2D, 0);
	gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
}

/// <summary>
///
/// - Creates pipeline for Temporal Approximate Anti Aliasing
///
/// </summary>
void OpenGLESAntiAliasing::createTXAAPipeline()
{
	// Create framebuffer object for offscreen rendering
	gl::GenFramebuffers(1, &_txaaResources->_offscreenFbo);
	gl::BindFramebuffer(GL_FRAMEBUFFER, _txaaResources->_offscreenFbo);
	auto bindingFramebufferError = gl::GetError();

	// color texture for framebuffer object
	gl::GenTextures(1, &_txaaResources->_offscreenTexture);
	gl::BindTexture(GL_TEXTURE_2D, _txaaResources->_offscreenTexture);
	gl::TexStorage2D(GL_TEXTURE_2D, 1, GL_SRGB8_ALPHA8, getWidth(), getHeight());
	gl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _txaaResources->_offscreenTexture, 0);

	// create velocity texture
	gl::GenTextures(1, &_txaaResources->_velocityTexture);
	gl::BindTexture(GL_TEXTURE_2D, _txaaResources->_velocityTexture);
	gl::TexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, getWidth(), getHeight());
	gl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _txaaResources->_velocityTexture, 0);

	// create depth texture
	gl::GenTextures(1, &_txaaResources->_offscreenDepthTexture);
	gl::BindTexture(GL_TEXTURE_2D, _txaaResources->_offscreenDepthTexture);
	// gl::TexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, getWidth(), getHeight(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	gl::TexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, getWidth(), getHeight());
	gl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _txaaResources->_offscreenDepthTexture, 0);
	gl::BindTexture(GL_TEXTURE_2D, 0);

	// binding default
	gl::BindFramebuffer(GL_FRAMEBUFFER, 0);

	// resolve framebuffer object for history
	gl::GenFramebuffers(1, &_txaaResources->_resolveFbo);
	gl::BindFramebuffer(GL_FRAMEBUFFER, _txaaResources->_resolveFbo);

	// history texture
	gl::GenTextures(1, &_txaaResources->_historyTexture);
	gl::BindTexture(GL_TEXTURE_2D, _txaaResources->_historyTexture);
	gl::TexStorage2D(GL_TEXTURE_2D, 1, GL_SRGB8_ALPHA8, getWidth(), getHeight());
	gl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _txaaResources->_historyTexture, 0);

	// updated history framebuffer
	gl::GenFramebuffers(1, &_txaaResources->_updateHistoryFbo);
	gl::BindFramebuffer(GL_FRAMEBUFFER, _txaaResources->_updateHistoryFbo);

	// updated history texture
	gl::GenTextures(1, &_txaaResources->_updatedHistoryTexture);
	gl::BindTexture(GL_TEXTURE_2D, _txaaResources->_updatedHistoryTexture);
	gl::TexStorage2D(GL_TEXTURE_2D, 1, GL_SRGB8_ALPHA8, getWidth(), getHeight());
	gl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _txaaResources->_updatedHistoryTexture, 0);

	// bind to default
	gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
}

/// <summary>
/// Main rendering loop function of the program. The shell will call this function every frame.
/// </summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result OpenGLESAntiAliasing::renderFrame()
{
	debugThrowOnApiError("RenderFrame: Entrance");

	switch (_currentMethod)
	{
	case NOAA: renderNOAA(); break;
	case MSAA: renderMSAA(); break;
	case FXAA: renderFXAA(); break;
	case TXAA: renderTXAA(); break;
	default: renderNOAA(); break;
	}

	if (this->shouldTakeScreenshot()) { pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight()); }

	_deviceResources->context->swapBuffers();

	return pvr::Result::Success;
}

/// <summary>
/// - Rendering with no anti-aliasing.
/// - Renders without any post-processing or multiple render passes.
/// </summary>
void OpenGLESAntiAliasing::renderNOAA()
{
	gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
	gl::ClearColor(_clearColor.r, _clearColor.g, _clearColor.b, 1.f);
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl::Enable(GL_CULL_FACE);
	gl::UseProgram(_deviceResources->_program);

	gl::StencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	gl::StencilFunc(GL_ALWAYS, 1, 255);
	gl::StencilMask(255);

	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);
	gl::Enable(GL_DEPTH_TEST);

	gl::ActiveTexture(GL_TEXTURE0);
	gl::BindSampler(0, _deviceResources->_samplerTrilinear);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->_texture);

	gl::ActiveTexture(GL_TEXTURE1);
	gl::BindSampler(1, _deviceResources->_samplerTrilinear);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->_bumpTexture);

	glm::mat4 mModel = glm::rotate(_angleY, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(1.8f));
	_angleY += -RotateY * 0.05f * getFrameTime();

	gl::Uniform3fv(_uniformLocations[VertexUniforms::CurrLightDir], 1, glm::value_ptr(LightDir * mModel));

	glm::mat4 mvp = (_projMtx * _viewMtx) * mModel * _scene->getWorldMatrix(_scene->getNode(0).getObjectId());
	gl::UniformMatrix4fv(_uniformLocations[VertexUniforms::CurrMVPMatrix], 1, GL_FALSE, glm::value_ptr(mvp));

	// Now that the uniforms are set, call another function to actually draw the mesh.
	renderMesh(0);
	renderUI(NoAntialiasing);
}

/// <summary>
/// - Rendering with multisampled framebuffer
/// - Firstly creates a multisampled framebuffer. Fetches from that buffer and creates an average color.
/// </summary>
void OpenGLESAntiAliasing::renderMSAA()
{
	const GLenum buffers[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

	gl::BindFramebuffer(GL_FRAMEBUFFER, _msaaResources->_multisampledFbo);
	gl::ClearColor(_clearColor.r, _clearColor.g, _clearColor.b, 1.f);
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl::Enable(GL_CULL_FACE);
	gl::UseProgram(_deviceResources->_program);

	gl::StencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	gl::StencilFunc(GL_ALWAYS, 1, 255);
	gl::StencilMask(255);

	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);
	gl::Enable(GL_DEPTH_TEST);

	gl::ActiveTexture(GL_TEXTURE0);
	gl::BindSampler(0, _deviceResources->_samplerTrilinear);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->_texture);

	gl::ActiveTexture(GL_TEXTURE1);
	gl::BindSampler(1, _deviceResources->_samplerTrilinear);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->_bumpTexture);

	glm::mat4 mModel = glm::rotate(_angleY, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(1.8f));
	_angleY += -RotateY * 0.05f * getFrameTime();

	gl::Uniform3fv(_uniformLocations[VertexUniforms::CurrLightDir], 1, glm::value_ptr(LightDir * mModel));

	glm::mat4 mvp = (_projMtx * _viewMtx) * mModel * _scene->getWorldMatrix(_scene->getNode(0).getObjectId());
	gl::UniformMatrix4fv(_uniformLocations[VertexUniforms::CurrMVPMatrix], 1, GL_FALSE, glm::value_ptr(mvp));

	// Now that the uniforms are set, call another function to actually draw the mesh.
	renderMesh(0);

	gl::DrawBuffers(2, buffers);

	gl::BindVertexArray(0);
	gl::BindBuffer(GL_ARRAY_BUFFER, 0);
	gl::Disable(GL_DEPTH_TEST);

	gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
	gl::ClearColor(_clearColor.r, _clearColor.g, _clearColor.b, 1.f);
	gl::Clear(GL_COLOR_BUFFER_BIT);
	gl::UseProgram(_msaaResources->_msaaProgram);

	gl::ActiveTexture(GL_TEXTURE2);
	gl::BindSampler(2, _deviceResources->_samplerNearest);
	gl::BindTexture(GL_TEXTURE_2D_MULTISAMPLE, _msaaResources->_offscreenTexture);

	renderOffscreenQuad();
	renderUI(MsAntialiasing);
}

/// <summary>
/// - FXAA is basically a post-process anti aliasing technique
/// - Rendering with Fast Approximate Anti Aliasing technique that helps cleaning up jagged edges
/// </summary>
void OpenGLESAntiAliasing::renderFXAA()
{
	gl::BindFramebuffer(GL_FRAMEBUFFER, _fxaaResources->_offscreenFbo); // _deviceResources->onScreenFbo BUT ITS 0
	gl::ClearColor(_clearColor.r, _clearColor.g, _clearColor.b, 1.0f);
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl::UseProgram(_deviceResources->_program);

	gl::StencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	gl::StencilFunc(GL_ALWAYS, 1, 255);
	gl::StencilMask(255);

	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);
	gl::Enable(GL_DEPTH_TEST);

	gl::ActiveTexture(GL_TEXTURE0);
	gl::BindSampler(0, _deviceResources->_samplerTrilinear);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->_texture);

	gl::ActiveTexture(GL_TEXTURE1);
	gl::BindSampler(1, _deviceResources->_samplerTrilinear);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->_bumpTexture);

	glm::mat4 mModel = glm::rotate(_angleY, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(1.8f));
	_angleY += -RotateY * 0.05f * getFrameTime();

	gl::Uniform3fv(_uniformLocations[VertexUniforms::CurrLightDir], 1, glm::value_ptr(LightDir * mModel));

	glm::mat4 mvp = (_projMtx * _viewMtx) * mModel * _scene->getWorldMatrix(_scene->getNode(0).getObjectId());
	gl::UniformMatrix4fv(_uniformLocations[VertexUniforms::CurrMVPMatrix], 1, GL_FALSE, glm::value_ptr(mvp));

	// Now that the uniforms are set, call another function to actually draw the mesh.
	renderMesh(0);

	gl::BindVertexArray(0);
	gl::BindBuffer(GL_ARRAY_BUFFER, 0);
	gl::Disable(GL_DEPTH_TEST);

	gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
	gl::ClearColor(_clearColor.r, _clearColor.g, _clearColor.b, 1.f);
	gl::Clear(GL_COLOR_BUFFER_BIT);

	gl::UseProgram(_fxaaResources->_fxaaProgram);

	gl::ActiveTexture(GL_TEXTURE0 + 2);
	gl::BindSampler(2, _deviceResources->_samplerNearest);
	gl::BindTexture(GL_TEXTURE_2D, _fxaaResources->_offscreenTexture);

	renderOffscreenQuad();
	renderUI(FxAntiAliasing);
}

/*
*
*  The diagram shows how TXAA pipeline flows across different framebuffer objects.
*
				 ** Apply Jittering to Scene **
		   -----------------------------------------
		  |                                        |
		  |                                        |
**********************                  **********************
*                    *                  *                    *
*                    *                  *                    *
*                    *                  *                    *
*   Current Frame    *                  *     Velocity       *
*       Buffer       *                  *      Buffer        *
*                    *                  *                    *
*                    *                  *                    *
**********************                  **********************
		 ↓                                        ↓
		 |                                        |
		 |                                        |
		 |                                        |
		 |                                        |
		 ------------------------------------------
							 ↓
							 |
						* Resolve * ←-------------------------------|
							 |                                      |
							 ↓                                      ↑
				   **********************                **********************
				   *                    *                *                    *
				   *                    *                *                    *
				   *                    *                *                    *
				   *   Resolved Color   *                *   Updated History  *
				   *      Buffer        *                *      Buffer        *
				   *                    *                *                    *
				   *                    *                *                    *
				   **********************                **********************
							 ↓                                      ↑
							 |                                      |
							 |→----------- * Update History * -------
							 |
							 |
					* Render to Screen *
							 |
							 ↓
					**********************
					*                    *
					*                    *
					*                    *
					*       Frame        *
					*                    *
					*                    *
					*                    *
					**********************
*/
void OpenGLESAntiAliasing::renderTXAA()
{
	gl::BindFramebuffer(GL_FRAMEBUFFER, _txaaResources->_offscreenFbo);
	gl::ClearColor(_clearColor.r, _clearColor.g, _clearColor.b, 1.f);
	gl::Enable(GL_DEPTH_TEST);
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl::UseProgram(_txaaResources->_velocityProgram);

	gl::StencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	gl::StencilFunc(GL_ALWAYS, 1, 255);
	gl::StencilMask(255);

	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);
	gl::Enable(GL_DEPTH_TEST);

	gl::ActiveTexture(GL_TEXTURE0);
	gl::BindSampler(0, _deviceResources->_samplerLinear);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->_texture);

	gl::ActiveTexture(GL_TEXTURE1);
	gl::BindSampler(1, _deviceResources->_samplerLinear);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->_bumpTexture);

	// model matrix for rotating mesh turn-table style
	glm::mat4 mModel = glm::rotate(_angleY, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(1.8f));
	_angleY += -RotateY * 0.05f * getFrameTime();
	gl::Uniform3fv(_txaaResources->_uniformLocations[VertexUniforms::CurrLightDir], 1, glm::value_ptr(LightDir * mModel));

	// setup matrices and calculate Model-View-Projection matrix
	glm::mat4 modelMtx = mModel;
	glm::mat4 projViewMtx = _projMtx * _viewMtx;
	glm::mat4 worldMtx = _scene->getWorldMatrix(_scene->getNode(0).getObjectId());
	glm::mat4 mvp = projViewMtx * modelMtx * worldMtx;
	gl::UniformMatrix4fv(_txaaResources->_uniformLocations[VertexUniforms::CurrMVPMatrix], 1, GL_FALSE, glm::value_ptr(mvp));

	// sending current frame matrices to shaders
	gl::UniformMatrix4fv(_txaaResources->_uniformLocations[VertexUniforms::CurrModel], 1, GL_FALSE, glm::value_ptr(modelMtx));
	gl::UniformMatrix4fv(_txaaResources->_uniformLocations[VertexUniforms::CurrProjView], 1, GL_FALSE, glm::value_ptr(projViewMtx));
	gl::UniformMatrix4fv(_txaaResources->_uniformLocations[VertexUniforms::CurrWorld], 1, GL_FALSE, glm::value_ptr(worldMtx));

	gl::UniformMatrix4fv(_txaaResources->_uniformLocations[VertexUniforms::PreModel], 1, GL_FALSE, glm::value_ptr(_preModelMtx));
	gl::UniformMatrix4fv(_txaaResources->_uniformLocations[VertexUniforms::PreWorld], 1, GL_FALSE, glm::value_ptr(_preWorldMtx));
	gl::UniformMatrix4fv(_txaaResources->_uniformLocations[VertexUniforms::PreProjView], 1, GL_FALSE, glm::value_ptr(_preProjViewMtx));

	// Increasing frame offset to select jitter amount
	_txaaResources->_frameOffset++;
	_txaaResources->_frameOffset = _txaaResources->_frameOffset % _txaaResources->_frameCount;

	float jitterX = _txaaResources->_jitter[_txaaResources->_frameOffset][0] * (1.0 / getWidth());
	float jitterY = _txaaResources->_jitter[_txaaResources->_frameOffset][1] * (1.0 / getHeight());

	// add jitter moving dynamically
	gl::Uniform2fv(_txaaResources->_uniformLocations[VertexUniforms::Jitter], 1, glm::value_ptr(glm::vec2(jitterX, jitterY)));

	// Now that the uniforms are set, call another function to actually draw the mesh.
	renderMesh(0);

	// Bind VAO & VBO
	gl::BindVertexArray(0);
	gl::BindBuffer(GL_ARRAY_BUFFER, 0);

	// ************************* RESOLVE  ************************* //
	gl::BindFramebuffer(GL_FRAMEBUFFER, _txaaResources->_resolveFbo); // output is history texture
	gl::ClearColor(_clearColor.r, _clearColor.g, _clearColor.b, 1.f);
	gl::Clear(GL_COLOR_BUFFER_BIT);
	gl::Disable(GL_DEPTH_TEST);

	gl::UseProgram(_txaaResources->_resolveProgram);

	gl::ActiveTexture(GL_TEXTURE2);
	gl::BindSampler(3, _deviceResources->_samplerNearest); // 2 - samplerNearest
	gl::BindTexture(GL_TEXTURE_2D, _txaaResources->_offscreenTexture);

	gl::ActiveTexture(GL_TEXTURE3);
	gl::BindSampler(3, _deviceResources->_samplerLinear);
	gl::BindTexture(GL_TEXTURE_2D, _txaaResources->_updatedHistoryTexture);

	gl::ActiveTexture(GL_TEXTURE4);
	gl::BindSampler(4, _deviceResources->_samplerNearest);
	gl::BindTexture(GL_TEXTURE_2D, _txaaResources->_velocityTexture);

	renderOffscreenQuad();
	// ***************************************************************** //

	// ******************** COPY HISTORY BUFFER *****************************//
	gl::BindFramebuffer(GL_FRAMEBUFFER, _txaaResources->_updateHistoryFbo);
	gl::ClearColor(_clearColor.r, _clearColor.g, _clearColor.b, 1.f);
	gl::Clear(GL_COLOR_BUFFER_BIT);

	gl::UseProgram(_deviceResources->_offscreenProgram);

	gl::ActiveTexture(GL_TEXTURE2);
	gl::BindSampler(2, _deviceResources->_samplerNearest);
	gl::BindTexture(GL_TEXTURE_2D, _txaaResources->_historyTexture);

	renderOffscreenQuad();
	// ***************************************************************** //

	// ******************* RENDERING TO SCREEN ********************* //
	gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
	gl::ClearColor(_clearColor.r, _clearColor.g, _clearColor.b, 1.f);
	gl::Clear(GL_COLOR_BUFFER_BIT);

	gl::UseProgram(_deviceResources->_offscreenProgram);

	gl::ActiveTexture(GL_TEXTURE2);
	gl::BindSampler(2, _deviceResources->_samplerNearest);
	gl::BindTexture(GL_TEXTURE_2D, _txaaResources->_historyTexture);

	renderOffscreenQuad();
	renderUI(TxAntiAliasing);
	// ************************************************************

	// updating pre-frame matrices with current frame matrices
	_preModelMtx = modelMtx;
	_preProjViewMtx = projViewMtx;
	_preWorldMtx = worldMtx;
}

/// <summary>
/// - Renders a simple quad with Attributeless Vertex Shader which does not need any vertex buffer for drawing triangle
/// </summary>
void OpenGLESAntiAliasing::renderOffscreenQuad() { gl::DrawArrays(GL_TRIANGLES, 0, 6); }

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result OpenGLESAntiAliasing::initApplication()
{
	_scene = pvr::assets::loadModel(*this, SceneFileName);
	_angleY = 0.0f;

	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by Shell once per run, just before exiting the program.
///	If the rendering context is lost, QuitApplication() will not be called.</summary>
/// <returns>Return pvr::Result::Success if no error occurred.</returns>
pvr::Result OpenGLESAntiAliasing::quitApplication() { return pvr::Result::Success; }

/// <summary>Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context. </summary>
/// <returns>Return pvr::Result::Success if no error occurred </returns>
pvr::Result OpenGLESAntiAliasing::releaseView()
{
	_deviceResources.reset();
	_scene.reset();
	return pvr::Result::Success;
}

/// <summary>Draws a pvr::assets::Mesh after the model view matrix has been set and the material prepared.</summary>
/// <param name="nodeIndex">Node index of the mesh to draw </param>
void OpenGLESAntiAliasing::renderMesh(uint32_t nodeIndex)
{
	const pvr::assets::Model::Node& node = _scene->getNode(nodeIndex);
	const pvr::assets::Mesh& mesh = _scene->getMesh(node.getObjectId());

	gl::BindBuffer(GL_ARRAY_BUFFER, _deviceResources->_vbos[node.getObjectId()]);

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
		if (_deviceResources->_ibos[node.getObjectId()])
		{
			// Indexed Triangle list
			gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _deviceResources->_ibos[node.getObjectId()]);
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
			if (_deviceResources->_ibos[node.getObjectId()])
			{
				// Indexed Triangle strips
				gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _deviceResources->_ibos[node.getObjectId()]);
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
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::unique_ptr<pvr::Shell>(new OpenGLESAntiAliasing()); }