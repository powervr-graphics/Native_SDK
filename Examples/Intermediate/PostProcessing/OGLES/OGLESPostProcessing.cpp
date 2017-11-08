/*!********************************************************************************************
\File         OGLESPostProcessing.cpp
\Title        Bloom
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief      Shows how to do a bloom effect
***********************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRAssets/PVRAssets.h"
#include "PVRUtils/PVRUtilsGles.h"

pvr::utils::VertexBindings_Name vertexBindings[] =
{
	{ "POSITION", "inVertex" },
	{ "NORMAL", "inNormal" },
	{ "UV0", "inTexCoord" },
};

const char* attribNames[] =
{
	"inVertex",
	"inNormal",
	"inTexCoord",
};

const uint16_t attribIndices[] =
{
	0,
	1,
	2
};

namespace FboPass {
enum Enum { OnScreen, RenderScene, BlurFbo0, BlurFbo1, Count, NumBlurFbo = 2 };
}

/**********************************************************************************************
Consts
**********************************************************************************************/
const glm::vec3 LightPos(-1.5f, 0.0f, 10.0f);
const uint32_t TexSize = 256;    // Blur render target size (power-of-two)

/**********************************************************************************************
Content file names
***********************************************************************************************/
const char BlurVertSrcFile[] = "BlurVertShader_ES3.vsh";
const char QuadVertShaderSrcFile[] = "QuadVertShader_ES3.vsh";
const char VertShaderSrcFile[] = "VertShader_ES3.vsh";

const char FragShaderSrcFile[] = "FragShader_ES3.fsh";
const char PreBloomFragShaderSrcFile[] = "PreBloomFragShader_ES3.fsh";
const char PostBloomFragShaderSrcFile[] = "PostBloomFragShader_ES3.fsh";
const char BlurFragSrcFile[] = "BlurFragShader_ES3.fsh";

// PVR texture files
const char BaseTexFile[] = "Marble.pvr";
// POD _scene files
const char SceneFile[] = "scene.pod";

/*!********************************************************************************************
Class implementing the pvr::Shell functions.
***********************************************************************************************/
class OGLESPostProcessing : public pvr::Shell
{
	struct FrameBuffer
	{
		GLuint fbo;
		GLuint renderTex;
		GLuint depthTex;
		pvr::Rectanglei renderArea;
		FrameBuffer(): fbo(0), renderTex(0), depthTex(0) { }
		~FrameBuffer()
		{
			if (fbo) { gl::DeleteFramebuffers(1, &fbo); fbo = 0; }
			if (renderTex) { gl::DeleteTextures(1, &renderTex); renderTex = 0; }
			if (depthTex) { gl::DeleteTextures(1, &depthTex); depthTex = 0; }
		}
	};

	struct DeviceResources
	{
		//Vbos/Ibos
		std::vector<GLuint> vbos;
		std::vector<GLuint> ibos;

		//Fbo
		FrameBuffer fbo[FboPass::Count];

		//Textures
		GLuint baseTex;
		GLuint bloomMapTex;

		//Samplers
		GLuint samplerRepeat;
		GLuint samplerClamp;

		//Programs
		GLuint progDefault;
		GLuint progPreBloom;
		GLuint progPostBloom;
		GLuint progBlur;

		pvr::EglContext context;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;

		void beginPass(uint32_t pass, const glm::vec4& clearColor);
		DeviceResources() : baseTex(0), bloomMapTex(0), samplerRepeat(), samplerClamp(),
			progDefault(), progPreBloom(), progPostBloom(), progBlur()
		{ }
		~DeviceResources()
		{
			if (vbos.size()) { gl::DeleteBuffers(vbos.size(), vbos.data()); vbos.clear(); }
			if (ibos.size()) { gl::DeleteBuffers(ibos.size(), ibos.data()); ibos.clear(); }
			if (baseTex) { gl::DeleteTextures(1, &baseTex); baseTex = 0; }
			if (bloomMapTex) { gl::DeleteTextures(1, &bloomMapTex); bloomMapTex = 0; }

			if (samplerRepeat) { gl::DeleteSamplers(1, &samplerRepeat); samplerRepeat = 0; }
			if (samplerClamp) { gl::DeleteSamplers(1, &samplerClamp); samplerClamp = 0; }

			if (progDefault) { gl::DeleteProgram(progDefault); progDefault = 0; }
			if (progPreBloom) { gl::DeleteProgram(progPreBloom); progPreBloom = 0; }
			if (progPostBloom) { gl::DeleteProgram(progPostBloom); progPostBloom = 0; }
			if (progBlur) { gl::DeleteProgram(progBlur); samplerRepeat = progBlur; }
		}
	};

	std::auto_ptr<DeviceResources> _deviceResources;

	pvr::utils::VertexConfiguration _vertexConfig;

	// 3D Model
	pvr::assets::ModelHandle _scene;

	float _bloomIntensity;
	bool _applyBloom;
	bool _drawObject;
	bool _animating;

	float _rotation;
	// Group shader programs and their uniform locations together
	struct
	{
		uint32_t mvpLoc;
		uint32_t mvInvLoc;
		uint32_t lightDirLoc;
		uint32_t shininess;
	} _basicProgUniform;

	struct
	{
		uint32_t texOffsetX;
		uint32_t texOffsetY;
	} _blurProgUnifom;

	struct
	{
		uint32_t texFactor;
		uint32_t blurTexFactor;
	} _postBloomProgUniform;

	struct
	{
		uint32_t _bloomIntensity;
	} _preBloomProgUniform;

	struct DrawPass
	{
		glm::vec3 lightPos;
		glm::mat4 mvp;
		glm::mat4 mvInv;
		float texelOffset;

	};
	DrawPass _passDrawMesh;
	DrawPass _passBloom;

	glm::mat4 _world, _view, _proj;

public:
	OGLESPostProcessing() : _bloomIntensity(1.f) {}

	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	bool createTexturesAndSamplers();
	bool createPipeline();
	bool loadVbos();
	bool createBlurFbo();
	// create fbo for rendering the screen
	bool createRenderFbo();
	void updateSubtitleText();
	void drawMesh(int i32NodeIndex);
	void drawAxisAlignedQuad();
	void eventMappedInput(pvr::SimplifiedInput e);
	void updateAnimation();
	void executeCommands();
};

/*!********************************************************************************************
\brief  Handles user input and updates live variables accordingly.
***********************************************************************************************/
void OGLESPostProcessing::eventMappedInput(pvr::SimplifiedInput e)
{
	static int mode = 0;
	//Object+Bloom, object, bloom
	switch (e)
	{
	case pvr::SimplifiedInput::Left:
		if (--mode < 0) { mode = 2; }
		_applyBloom = (mode != 1); _drawObject = (mode != 2);
		updateSubtitleText();
		break;
	case pvr::SimplifiedInput::Right:
		++mode %= 3;
		_applyBloom = (mode != 1); _drawObject = (mode != 2);
		updateSubtitleText();
		break;
	case pvr::SimplifiedInput::Up:
		_bloomIntensity = int(.5f + 10.f * std::min(_bloomIntensity + .2f, 5.f)) * .1f;
		updateSubtitleText();
		break;
	case pvr::SimplifiedInput::Down:
		_bloomIntensity = int(.5f + 10.f * std::max(_bloomIntensity - .2f, 0.f)) * .1f;
		updateSubtitleText();
		break;
	case pvr::SimplifiedInput::ActionClose:
		this->exitShell();
		break;
	case pvr::SimplifiedInput::Action1:
	case pvr::SimplifiedInput::Action2:
	case pvr::SimplifiedInput::Action3:
		_animating = !_animating;
		break;
	default:
		break;
	}
}

/*!********************************************************************************************
\return Return true if no error occurred
\brief  Loads the textures required for this training course
***********************************************************************************************/
bool OGLESPostProcessing::createTexturesAndSamplers()
{
	// Load Textures
	if (!pvr::utils::textureUpload(*this, BaseTexFile, _deviceResources->baseTex, _deviceResources->context->getApiVersion() == pvr::Api::OpenGLES2))
	{
		setExitMessage("FAILED to load texture %s.", BaseTexFile);
		return false;
	}
	gl::GenSamplers(1, &_deviceResources->samplerRepeat);
	gl::GenSamplers(1, &_deviceResources->samplerClamp);

	gl::SamplerParameteri(_deviceResources->samplerRepeat, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	gl::SamplerParameteri(_deviceResources->samplerRepeat, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_deviceResources->samplerRepeat, GL_TEXTURE_WRAP_R, GL_REPEAT);
	gl::SamplerParameteri(_deviceResources->samplerRepeat, GL_TEXTURE_WRAP_S, GL_REPEAT);
	gl::SamplerParameteri(_deviceResources->samplerRepeat, GL_TEXTURE_WRAP_T, GL_REPEAT);

	gl::SamplerParameteri(_deviceResources->samplerClamp, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	gl::SamplerParameteri(_deviceResources->samplerClamp, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_deviceResources->samplerClamp, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->samplerClamp, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->samplerClamp, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	return true;
}

/*!********************************************************************************************
\brief  Loads and compiles the shaders and links the shader programs
\return Return true if no error occurred required for this training course
***********************************************************************************************/
bool OGLESPostProcessing::createPipeline()
{
	const pvr::assets::Mesh& mesh = _scene->getMesh(0);

	// create render _scene pipeline
	{
		_vertexConfig = pvr::utils::createInputAssemblyFromMesh(mesh, vertexBindings, 3);

		if (!(_deviceResources->progDefault = pvr::utils::createShaderProgram(*this, VertShaderSrcFile, FragShaderSrcFile, attribNames, attribIndices, 3)))
		{
			setExitMessage("Unable to create default program (%s, %s)", VertShaderSrcFile, FragShaderSrcFile);
			return false;
		}
		gl::UseProgram(_deviceResources->progDefault);
		gl::Uniform1i(gl::GetUniformLocation(_deviceResources->progDefault, "sTexture"), 0);

		// Store the location of uniforms for later use
		_basicProgUniform.mvpLoc = gl::GetUniformLocation(_deviceResources->progDefault, "MVPMatrix");
		_basicProgUniform.mvInvLoc = gl::GetUniformLocation(_deviceResources->progDefault, "MVInv");
		_basicProgUniform.lightDirLoc = gl::GetUniformLocation(_deviceResources->progDefault, "LightDirection");
		_basicProgUniform.shininess = gl::GetUniformLocation(_deviceResources->progDefault, "Shininess");
	}

	// create prebloom pipeline
	{
		if (!(_deviceResources->progPreBloom = pvr::utils::createShaderProgram(*this, QuadVertShaderSrcFile, PreBloomFragShaderSrcFile, NULL, NULL, 0)))
		{
			setExitMessage("Unable to create PreBloom program (%s, %s)", QuadVertShaderSrcFile, FragShaderSrcFile);
			return false;
		}

		gl::UseProgram(_deviceResources->progPreBloom);
		// Store the location of uniforms for later use
		_preBloomProgUniform._bloomIntensity = gl::GetUniformLocation(_deviceResources->progPreBloom, "BloomIntensity");

		gl::Uniform1i(gl::GetUniformLocation(_deviceResources->progPreBloom, "sTexture"), 0);
	}

	//   Blur Pipeline
	{
		if (!(_deviceResources->progBlur = pvr::utils::createShaderProgram(*this, BlurVertSrcFile, BlurFragSrcFile, NULL, NULL, 0)))
		{
			setExitMessage("Unable to create Blur program (%s, %s)", BlurVertSrcFile, BlurFragSrcFile);
			return false;
		}

		gl::UseProgram(_deviceResources->progBlur);
		gl::Uniform1i(gl::GetUniformLocation(_deviceResources->progBlur, "sTexture"), 0);

		// Store the location of uniforms for later use
		_blurProgUnifom.texOffsetX = gl::GetUniformLocation(_deviceResources->progBlur, "TexelOffsetX");
		_blurProgUnifom.texOffsetY = gl::GetUniformLocation(_deviceResources->progBlur, "TexelOffsetY");
	}

	// create Post-Bloom Pipeline
	{

		if (!(_deviceResources->progPostBloom = pvr::utils::createShaderProgram(*this, QuadVertShaderSrcFile, PostBloomFragShaderSrcFile, NULL, NULL, 0)))
		{
			setExitMessage("Unable to create Post Bloom program (%s, %s)", QuadVertShaderSrcFile, PostBloomFragShaderSrcFile);
			return false;
		}

		// Set the sampler2D variable to the first texture unit
		gl::UseProgram(_deviceResources->progPostBloom);
		gl::Uniform1i(gl::GetUniformLocation(_deviceResources->progPostBloom, "sTexture"), 0);
		gl::Uniform1i(gl::GetUniformLocation(_deviceResources->progPostBloom, "sBlurTexture"), 1);

		// Store the location of uniforms for later use
		_postBloomProgUniform.texFactor = gl::GetUniformLocation(_deviceResources->progPostBloom, "sTexFactor");
		_postBloomProgUniform.blurTexFactor = gl::GetUniformLocation(_deviceResources->progPostBloom, "sBlurTexFactor");
	}
	return true;
}

/*!****************************************************************************
\brief  Loads the mesh data required for this training course into vertex buffer objects
******************************************************************************/
bool OGLESPostProcessing::loadVbos()
{
	// Load vertex data of all meshes in the _scene into VBOs
	// The meshes have been exported with the "Interleave Vectors" option,
	// so all data is interleaved in the buffer at pMesh->pInterleaved.
	// Interleaving data improves the memory access pattern and cache efficiency,
	// thus it can be read faster by the hardware.
	pvr::utils::appendSingleBuffersFromModel(*_scene, _deviceResources->vbos, _deviceResources->ibos);

	return true;
}

/*!********************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in initApplication() will be called by pvr::Shell once per run, before the rendering
  context is created.
  Used to initialize variables that are not dependent on it (e.g. external modules,
  loading meshes, etc.)
  If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************/
pvr::Result OGLESPostProcessing::initApplication()
{
	// Apply bloom per default
	_applyBloom = true;
	_drawObject = true;
	_animating = true;
	// Initial number of blur passes, can be changed during runtime
	_rotation = 0.0f;

	// Texel offset for blur filter kernel
	_passBloom.texelOffset = 1.0f / (float)TexSize;
	// Altered weights for the faster filter kernel
	float w1 = 0.0555555f;
	float w2 = 0.2777777f;
	float intraTexelOffset = (w1 / (w1 + w2)) * _passBloom.texelOffset;
	_passBloom.texelOffset += intraTexelOffset;
	// Intensity multiplier for the bloom effect
	// Load the _scene
	if (!pvr::utils::loadModel(*this, SceneFile, _scene))
	{
		this->setExitMessage("Error: Couldn't load the %s file\n", SceneFile);
		return pvr::Result::UnknownError;
	}
	return pvr::Result::Success;
}

/*!********************************************************************************************
\return Return  pvr::Result::Success if no error occured
\brief  Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.
quitApplication() will not be called every time the rendering context is lost, only before application exit.
***********************************************************************************************/
pvr::Result OGLESPostProcessing::quitApplication()
{
	return pvr::Result::Success;
}

/*!********************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in initView() will be called by pvr::Shell upon initialization or after a change
  in the rendering context. Used to initialize variables that are dependent on the rendering
  context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************/
pvr::Result OGLESPostProcessing::initView()
{
	if (this->getMinApi() < pvr::Api::OpenGLES3)
	{
		Log(LogLevel::Information, "This demo requires a minimum api of OpenGLES3.");
	}

	_deviceResources.reset(new DeviceResources());
	_deviceResources->context = pvr::createEglContext();
	_deviceResources->context->init(getWindow(), getDisplay(), getDisplayAttributes(), pvr::Api::OpenGLES3);

	//  Initialize VBO data
	if (!loadVbos()) { return pvr::Result::NotInitialized; }

	//  Load and compile the shaders & link programs
	if (!createPipeline()) { return pvr::Result::NotInitialized; }

	_deviceResources->fbo[FboPass::OnScreen].fbo = _deviceResources->context->getOnScreenFbo();
	_deviceResources->fbo[FboPass::OnScreen].renderArea = pvr::Rectanglei(0, 0, getWidth(), getHeight());

	if (!createRenderFbo() || !createBlurFbo())
	{
		return pvr::Result::NotInitialized;
	}

	//  Load textures
	if (!createTexturesAndSamplers()) { return pvr::Result::NotInitialized; }

	if (!_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen()))
	{
		setExitMessage("Error: Failed to initialize the UIRenderer\n");
		return pvr::Result::NotInitialized;
	}

	_deviceResources->uiRenderer.getDefaultTitle()->setText("PostProcessing");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText(
	  "Left / right: Rendering mode\n"
	  "Up / down: Bloom intensity\n"
	  "Action:     Pause\n"
	);
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();
	updateSubtitleText();
	float fov;
	glm::vec3 from, to, up;
	_scene->getCameraProperties(0, fov, from, to, up);
	_view = glm::lookAt(from, to, up);

	bool bRotate = isFullScreen() && isScreenRotated();
	if (bRotate)
	{
		_proj = pvr::math::perspectiveFov(pvr::Api::OpenGLES3, fov, (float)getHeight(), (float)getWidth(), _scene->getCamera(0).getNear(),
		                                  _scene->getCamera(0).getFar(), glm::pi<float>() * .5f);
	}
	else
	{
		_proj = glm::perspectiveFov<glm::float32>(fov, (float)getWidth(), (float)getHeight(),
		        _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar());
	}
	updateSubtitleText();

	gl::Enable(GL_CULL_FACE);
	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);
	return pvr::Result::Success;
}

/*!********************************************************************************************
\brief Create render fbo for rendering the _scene
\return Return true if success
***********************************************************************************************/
bool OGLESPostProcessing::createRenderFbo()
{
	pvr::ImageStorageFormat depthTexFormat(pvr::PixelFormat::Depth16, 1, pvr::ColorSpace::lRGB, pvr::VariableType::Float);
	pvr::ImageStorageFormat colorTexFormat(pvr::GeneratePixelType3 < 'b', 'g', 'r', 10, 11, 11>::ID, 1, pvr::ColorSpace::lRGB, pvr::VariableType::UnsignedFloat);
	GLenum depthFmt, depthInternalFmt, depthGltype, depthTypeSize;
	GLenum colorFmt, colorInternalFmt, colorGltype, colorTypeSize;

	auto& fbo = _deviceResources->fbo[FboPass::RenderScene];
	fbo.renderArea = pvr::Rectanglei(0, 0, getWidth(), getHeight());

	bool isCompressed;
	pvr::utils::getOpenGLFormat(depthTexFormat, depthInternalFmt, depthFmt, depthGltype, depthTypeSize, isCompressed);
	pvr::utils::getOpenGLFormat(colorTexFormat, colorInternalFmt, colorFmt, colorGltype, colorTypeSize, isCompressed);


	gl::GenTextures(1, &fbo.depthTex);
	gl::GenTextures(1, &fbo.renderTex);
	gl::BindTexture(GL_TEXTURE_2D, fbo.depthTex);
	gl::TexStorage2D(GL_TEXTURE_2D, 1, depthInternalFmt, getWidth(), getHeight());
	gl::BindTexture(GL_TEXTURE_2D, fbo.renderTex);
	gl::TexStorage2D(GL_TEXTURE_2D, 1, colorInternalFmt, getWidth(), getHeight());


	gl::GenFramebuffers(1, &fbo.fbo);
	gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo.fbo);
	// create the render pass.
	gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo.renderTex, 0);
	gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbo.depthTex, 0);
	return pvr::utils::checkFboStatus();
}

void OGLESPostProcessing::DeviceResources::beginPass(uint32_t pass, const glm::vec4& clearColor)
{
	gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[pass].fbo);
	gl::ClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	gl::ClearDepthf(1.0f);
	gl::Viewport(0, 0, fbo[pass].renderArea.width, fbo[pass].renderArea.height);
	gl::Clear(GL_COLOR_BUFFER_BIT | (pass == FboPass::RenderScene ? GL_DEPTH_BUFFER_BIT : 0));
}


/*!********************************************************************************************
\brief  Create the blur fbo
\return Return  true on success
***********************************************************************************************/
bool OGLESPostProcessing::createBlurFbo()
{
	pvr::ImageStorageFormat colorTexFormat(pvr::PixelFormat::RGB_888, 1, pvr::ColorSpace::lRGB, pvr::VariableType::UnsignedByteNorm);
	for (uint32_t i = 0; i < FboPass::NumBlurFbo; i++)
	{
		auto& fbo = _deviceResources->fbo[FboPass::BlurFbo0 + i];
		fbo.depthTex = 0;
		fbo.renderArea = pvr::Rectanglei(0, 0, TexSize, TexSize);

		GLenum colorFmt, colorInternalFmt, colorGltype, colorTypeSize;
		bool isCompressed;
		pvr::utils::getOpenGLFormat(colorTexFormat, colorInternalFmt, colorFmt, colorGltype, colorTypeSize, isCompressed);

		gl::GenTextures(1, &fbo.renderTex);
		gl::BindTexture(GL_TEXTURE_2D, fbo.renderTex);
		gl::TexStorage2D(GL_TEXTURE_2D, 1, colorInternalFmt, TexSize, TexSize);

		_deviceResources->fbo[FboPass::BlurFbo0 + i].renderArea = pvr::Rectanglei(0, 0, TexSize, TexSize);

		gl::GenFramebuffers(1, &fbo.fbo);
		gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo.fbo);
		// create the render pass.
		gl::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo.renderTex, 0);
		if (!pvr::utils::checkFboStatus()) { return false; }
	}
	return true;
}

/*!********************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in releaseView() will be called by pvr::Shell when the application quits or before
a change in the rendering context.
***********************************************************************************************/
pvr::Result OGLESPostProcessing::releaseView()
{
	_scene.reset();
	_deviceResources.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief Update the animation
***********************************************************************************************************************/
void OGLESPostProcessing::updateAnimation()
{
	// Calculate the mask and light _rotation based on the passed time
	float const twoPi = glm::pi<float>() * 2.f;

	if (_animating)
	{
		_rotation += glm::pi<float>() * getFrameTime() * 0.0002f;
		// wrap it
		if (_rotation > twoPi) { _rotation -= twoPi; }
	}

	// Calculate the model, _view and projection matrix
	_world = glm::rotate((-_rotation), glm::vec3(0.f, 1.f, 0.f)) * glm::scale(glm::vec3(1.65f));

	float fov;
	fov = _scene->getCamera(0).getFOV(0);

	glm::mat4x4 viewProj = _proj * _view;
	// Simple rotating directional light in model-space)
	_passDrawMesh.lightPos = glm::normalize(glm::inverse(glm::mat3(_world)) * LightPos);
	_passDrawMesh.mvInv = glm::inverse(_view * _world * _scene->getWorldMatrix(_scene->getNode(0).getObjectId()));
	_passDrawMesh.mvp = viewProj * _world * _scene->getWorldMatrix(_scene->getNode(0).getObjectId());
}

/*!********************************************************************************************
\return Return Result::Suceess if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************/
pvr::Result OGLESPostProcessing::renderFrame()
{
	debugLogApiError("Frame begin");
	updateAnimation();
	executeCommands();
	debugLogApiError("Frame end");

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight());
	}

	_deviceResources->context->swapBuffers();
	return pvr::Result::Success;
}

/*!********************************************************************************************
\brief  update the subtitle sprite
***********************************************************************************************/
void OGLESPostProcessing::updateSubtitleText()
{
	if (_applyBloom)
	{
		if (_drawObject)
		{
			_deviceResources->uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("Object with bloom effect, intensity % 2.1f", _bloomIntensity));
		}
		else
		{
			_deviceResources->uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("Bloom effect textures, intensity % 2.1f", _bloomIntensity));
		}
	}
	else
	{
		if (_drawObject)
		{
			_deviceResources->uiRenderer.getDefaultDescription()->setText("Object without bloom");
		}
		else
		{
			_deviceResources->uiRenderer.getDefaultDescription()->setText("Use up - down to draw object and / or bloom textures");
		}
	}
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
}


/*!********************************************************************************************
\param  nodeIndex Node index of the mesh to draw
\brief  Draws a pvr::Model::Mesh after the model _view matrix has been set and the material prepared.
***********************************************************************************************/
void OGLESPostProcessing::drawMesh(int nodeIndex)
{
	int meshIndex = _scene->getNode(nodeIndex).getObjectId();
	const pvr::assets::Model::Mesh& mesh = _scene->getMesh(meshIndex);
	// bind the VBO for the mesh
	if (_deviceResources->vbos[meshIndex]) { gl::BindBuffer(GL_ARRAY_BUFFER, _deviceResources->vbos[meshIndex]); }

	// bind the index buffer, won't hurt if the handle is 0
	if (_deviceResources->ibos[meshIndex]) { gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _deviceResources->ibos[meshIndex]); }


	assertion(_vertexConfig.bindings.size() == 1, "This demo assumes only one VBO per mesh");
	for (auto it = _vertexConfig.attributes.begin(), end = _vertexConfig.attributes.end(); it != end; ++it)
	{
		gl::EnableVertexAttribArray(it->index);
		gl::VertexAttribPointer(it->index, it->width, pvr::utils::convertToGles(it->format),
		                        pvr::dataTypeIsNormalised(it->format), _vertexConfig.bindings[it->binding].strideInBytes,
		                        reinterpret_cast<const void*>(static_cast<uintptr_t>(it->offsetInBytes)));
	}

	GLenum primitiveType = pvr::utils::convertToGles(mesh.getPrimitiveType());
	if (mesh.getMeshInfo().isIndexed)
	{
		auto indextype = mesh.getFaces().getDataType();
		GLenum indexgltype = (indextype == pvr::IndexType::IndexType16Bit ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT);
		// Indexed Triangle list
		gl::DrawElements(primitiveType, mesh.getNumFaces() * 3, indexgltype, 0);
	}
	else
	{
		// Non-Indexed Triangle list
		gl::DrawArrays(primitiveType, 0, mesh.getNumFaces() * 3);
	}
	for (auto it = _vertexConfig.attributes.begin(), end = _vertexConfig.attributes.end(); it != end; ++it)
	{
		gl::DisableVertexAttribArray(it->index);
	}
}

/*!********************************************************************************************
\brief  Add the draw commands for a full screen quad to a commandbuffer
***********************************************************************************************/
void OGLESPostProcessing::drawAxisAlignedQuad()
{
	gl::BindBuffer(GL_ARRAY_BUFFER, 0);
	gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 3);
}

/*!********************************************************************************************
\brief  Record the command buffer
***********************************************************************************************/
void OGLESPostProcessing::executeCommands()
{
	// draw the _scene
	{
		// Use simple shader program to render the mask
		gl::UseProgram(_deviceResources->progDefault);
		gl::Disable(GL_BLEND);
		gl::Enable(GL_DEPTH_TEST); // depth test
		gl::DepthMask(GL_TRUE); // depth write enabled

		_deviceResources->beginPass(FboPass::RenderScene, glm::vec4(0.00f, 0.70f, 0.67f, 1.f));
		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindTexture(GL_TEXTURE_2D, _deviceResources->baseTex);
		gl::BindSampler(0, _deviceResources->samplerRepeat);

		// bind the albedo texture

		gl::Uniform1f(_basicProgUniform.shininess, .6f);
		gl::Uniform3f(_basicProgUniform.lightDirLoc, _passDrawMesh.lightPos.x, -_passDrawMesh.lightPos.y, _passDrawMesh.lightPos.z);

		// Draw the mesh
		gl::UniformMatrix4fv(_basicProgUniform.mvpLoc, 1, GL_FALSE, glm::value_ptr(_passDrawMesh.mvp));
		gl::UniformMatrix4fv(_basicProgUniform.mvInvLoc, 1, GL_FALSE, glm::value_ptr(_passDrawMesh.mvInv));

		drawMesh(0);
	}
	// Full screen draws follow:
	gl::Disable(GL_DEPTH_TEST);
	gl::DepthMask(GL_FALSE);
	// not applying bloom, render the _scene without bloom and return
	if (!_applyBloom)
	{
		gl::Enable(GL_BLEND);
		// Draw _scene with bloom
		gl::UseProgram(_deviceResources->progPostBloom);
		_deviceResources->beginPass(FboPass::OnScreen, glm::vec4(1.f, 0, 0, 1.f));
		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindTexture(GL_TEXTURE_2D, _deviceResources->fbo[FboPass::RenderScene].renderTex);
		gl::BindSampler(0, _deviceResources->samplerClamp);

		// The following section will draw a quad on the screen where the post processing pixel
		// shader shall be executed.Try to minimize the area by only drawing where the actual
		// post processing should happen, as this is a very costly operation.
		gl::Uniform1f(_postBloomProgUniform.texFactor, 1.f);
		gl::Uniform1f(_postBloomProgUniform.blurTexFactor, 0.f);
		drawAxisAlignedQuad();
	}
	else
	{
		// bloom
		{
			// filter the bright portion of the image
			gl::UseProgram(_deviceResources->progPreBloom);
			gl::Disable(GL_BLEND);
			_deviceResources->beginPass(FboPass::BlurFbo0, glm::vec4(0.f, 1.f, 0.f, 1.0f));

			// bind the render texture
			gl::ActiveTexture(GL_TEXTURE0);
			gl::BindTexture(GL_TEXTURE_2D, _deviceResources->fbo[FboPass::RenderScene].renderTex);
			gl::BindSampler(0, _deviceResources->samplerClamp);

			gl::Uniform1f(_preBloomProgUniform._bloomIntensity, _bloomIntensity);
			drawAxisAlignedQuad();
		}
		// BLUR render pass
		{
			// Horizontal blur
			gl::UseProgram(_deviceResources->progBlur);
			gl::Enable(GL_BLEND);
			_deviceResources->beginPass(FboPass::BlurFbo1, glm::vec4(1.f, 1.f, 0.f, 1.0f));
			gl::ActiveTexture(GL_TEXTURE0);

			// blur pass1 descriptor set (blur pass0 texture)
			gl::BindTexture(GL_TEXTURE_2D, _deviceResources->fbo[FboPass::BlurFbo0].renderTex);
			gl::BindSampler(0, _deviceResources->samplerClamp);

			gl::Uniform1f(_blurProgUnifom.texOffsetX, _passBloom.texelOffset);
			gl::Uniform1f(_blurProgUnifom.texOffsetY, 0.0f);
			drawAxisAlignedQuad();

			// Vertical Blur
			_deviceResources->beginPass(FboPass::BlurFbo0, glm::vec4(0.f, 1.f, 1.f, 1.0f));
			// blur pass0 descriptor set (blur pass1 texture)
			gl::BindTexture(GL_TEXTURE_2D, _deviceResources->fbo[FboPass::BlurFbo1].renderTex);
			gl::BindSampler(0, _deviceResources->samplerClamp);

			gl::Uniform1f(_blurProgUnifom.texOffsetX, 0.0f);
			gl::Uniform1f(_blurProgUnifom.texOffsetY, _passBloom.texelOffset);

			drawAxisAlignedQuad();
		}
		{
			// Draw _scene with bloom
			gl::UseProgram(_deviceResources->progPostBloom);
			gl::Enable(GL_BLEND);
			_deviceResources->beginPass(FboPass::OnScreen, glm::vec4(glm::vec4(1.f, 0.f, 1.f, 1.0f)));
			// bind the blurred texture
			gl::ActiveTexture(GL_TEXTURE0);
			gl::BindTexture(GL_TEXTURE_2D, _deviceResources->fbo[FboPass::RenderScene].renderTex);
			gl::BindSampler(0, _deviceResources->samplerRepeat);
			gl::ActiveTexture(GL_TEXTURE1);
			gl::BindTexture(GL_TEXTURE_2D, _deviceResources->fbo[FboPass::BlurFbo0].renderTex);
			gl::BindSampler(1, _deviceResources->samplerRepeat);
			gl::ActiveTexture(GL_TEXTURE0);

			// The following section will draw a quad on the screen where the post processing pixel
			// shader shall be executed.Try to minimize the area by only drawing where the actual
			// post processing should happen, as this is a very costly operation.
			gl::Uniform1f(_postBloomProgUniform.blurTexFactor, 1.f);

			if (_drawObject)
			{
				gl::Uniform1f(_postBloomProgUniform.texFactor, 1.f);
			}
			else  // Hide the object to show the bloom textures...
			{
				gl::Uniform1f(_postBloomProgUniform.texFactor, 0.f);
			}

			drawAxisAlignedQuad();
		}
	}

	debugLogApiError("OGLESPostProcessing::ExecuteCommands UIRenderer beginning");
	//UIRENDERER
	{
		// record the commands
		_deviceResources->uiRenderer.beginRendering();

		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getDefaultControls()->render();
		_deviceResources->uiRenderer.getDefaultDescription()->render();
		_deviceResources->uiRenderer.endRendering();
	}
}

/*!********************************************************************************************
\return Return auto ptr to the demo supplied by the user
\brief  This function must be implemented by the user of the shell.
The user should return its pvr::Shell object defining the behaviour of the application.
***********************************************************************************************/
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::unique_ptr<pvr::Shell>(new OGLESPostProcessing()); }
