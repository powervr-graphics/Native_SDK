/*!*********************************************************************************************************************
\File         OGLESGaussianBlur.cpp
\Title        Gaussian Blur
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright(c) Imagination Technologies Limited.
\brief        Shows how to perform a single pass Gaussian Blur using Compute shader.
***********************************************************************************************************************/

#include "PVRShell/PVRShell.h"
#include "PVRAssets/PVRAssets.h"
#include "PVRUtils/PVRUtilsGles.h"

// Source and binary shaders
const char FragShaderSrcFile[] = "FragShader_ES3.fsh";
const char VertShaderSrcFile[] = "VertShader_ES3.vsh";
const char CompShaderSrcFile[] = "CompShader_ES3.csh";

// PVR texture files
const char StatueTexFile[] = "Lenna.pvr";
const char* attribNames[] =
{
	"inPosition",
	"inTexCoord",
};
pvr::utils::VertexBindings_Name vertexBindings[] =
{
	{ "POSITION", "inPosition" },
	{ "UV0", "inTexCoord"},
};
const uint16_t attribIndices[] =
{
	0,
	1,
};

/*!*********************************************************************************************************************
Class implementing the Shell functions.
***********************************************************************************************************************/
class OGLESGaussianBlur : public pvr::Shell
{
private:
	struct Framebuffer
	{
		GLuint fbo;
		GLuint renderTex;
		pvr::Rectanglei renderArea;

		Framebuffer() : fbo(0), renderTex(0) {}

		~Framebuffer() { if (fbo) { gl::DeleteFramebuffers(1, &fbo); fbo = 0; } }
	};

	struct DeviceResources
	{
		// Vbos/Ibos
		GLuint vbo;
		GLuint ibo;

		// Fbo
		Framebuffer fbo;

		GLuint inputTex;
		GLuint outputTex;

		GLuint computeProgram;
		GLuint graphicProgram;

		pvr::EglContext context;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;

		DeviceResources() : inputTex(0), outputTex(0) { }
	};

	std::unique_ptr<DeviceResources> _deviceResources;

public:
	OGLESGaussianBlur() {}
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();
	bool loadVbos();
	bool createResources();
	void renderBlurredQuad();
	void configureOnScreenFBO();
	void beginPass();
};

/*!*********************************************************************************************************************
\return Return GL_NO_ERROR if no error occurred
\brief  Code in loadVbos() creates a vertex buffer object and an index buffer object. It allocates them and copies the
data used later to render the quad on screen.
***********************************************************************************************************************/
bool OGLESGaussianBlur::loadVbos()
{
	// We create the Qaud.
	float vertPos = 1;
	float quad [] =
	{
		//       Position		   Texcoords
		-vertPos,  vertPos, 1.0,   0.0f, 0.0f, // Top-left
		vertPos,   vertPos, 1.0,   1.0f, 0.0f, // Top-right
		vertPos,  -vertPos, 1.0,   1.0f, 1.0f, // Bottom-right
		-vertPos, -vertPos, 1.0,   0.0f, 1.0f // Bottom-left
	};

	// Create the required Vertex buffer
	gl::GenBuffers(1, &_deviceResources->vbo);
	gl::BindBuffer(GL_ARRAY_BUFFER, _deviceResources->vbo);
	gl::BufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);


	unsigned int vIndices[] = { 0, 1, 3, 1, 3, 2 };

	// Create the required index buffer
	gl::GenBuffers(1, &_deviceResources->ibo);
	gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _deviceResources->ibo);
	gl::BufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vIndices), vIndices, GL_STATIC_DRAW);

	if (gl::GetError() == GL_NO_ERROR)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*!*********************************************************************************************************************
\return Return GL_NO_ERROR if no error occurred
\brief  Code in  createResources() loads the compute, fragment and vertex shaders. It loads the input texture on which
we'll perform the Gaussian blur. It also generates the output texture that will be filled by the comptue shader and
used by the fragment shader.
***********************************************************************************************************************/
bool OGLESGaussianBlur::createResources()
{
	// Load the compute shader and create the associated program.
	if (!(_deviceResources->computeProgram = pvr::utils::createComputeShaderProgram(*this, CompShaderSrcFile)))
	{
		setExitMessage("Unable to create default program (%s, %s)", CompShaderSrcFile);
		return true;
	}

	// Load the fragment and vertex shaders and create the associated programs.
	if (!(_deviceResources->graphicProgram = pvr::utils::createShaderProgram(*this, VertShaderSrcFile, FragShaderSrcFile, attribNames, attribIndices, 2)))
	{
		setExitMessage("Unable to create default program (%s, %s)", VertShaderSrcFile, FragShaderSrcFile);
		return true;
	}

	gl::UseProgram(_deviceResources->graphicProgram);
	gl::Uniform1f(gl::GetUniformLocation(_deviceResources->graphicProgram, "WindowWidth"), getWidth() * 1.2f);

	// Load the textuer from disk
	if (!pvr::utils::textureUpload(*this, "Lenna.pvr", _deviceResources->inputTex))
	{
		this->setExitMessage("Error: Couldn't load the %s file\n", _deviceResources->inputTex);
		return true;
	}

	// Create and Allocate Output texture.
	gl::GenTextures(1, &_deviceResources->outputTex);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->outputTex);
	gl::TexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 512, 512);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	gl::BindTexture(GL_TEXTURE_2D, 0);

	if (gl::GetError() == GL_NO_ERROR)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*!*********************************************************************************************************************
\brief  configureOnScreenFbo sets up the rendering FBO.
***********************************************************************************************************************/
void OGLESGaussianBlur::configureOnScreenFBO()
{
	// Set up the FBO to render to screen.
	_deviceResources->fbo.fbo = 0;
	_deviceResources->fbo.renderArea = pvr::Rectanglei(0, 0, getWidth(), getHeight());
}

void OGLESGaussianBlur::renderBlurredQuad()
{
	// We Execute the Compute shader, we bind the input and output texture.
	gl::UseProgram(_deviceResources->computeProgram);

	gl::BindImageTexture(0, _deviceResources->inputTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
	gl::BindImageTexture(1, _deviceResources->outputTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

	gl::DispatchCompute(512 / 32, 1, 1);
	gl::MemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	// Excute the Graphic program (Vertex and Fragment) and pass the output texture
	gl::UseProgram(_deviceResources->graphicProgram);

	gl::EnableVertexAttribArray(0);
	gl::EnableVertexAttribArray(1);
	gl::VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	gl::VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<const void*>(3 * sizeof(float)));

	gl::ActiveTexture(GL_TEXTURE0);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->inputTex);
	gl::ActiveTexture(GL_TEXTURE1);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->outputTex);
	gl::Uniform1i(gl::GetUniformLocation(_deviceResources->graphicProgram, "sOriginalTexture"), 0);
	gl::Uniform1i(gl::GetUniformLocation(_deviceResources->graphicProgram, "sTexture"), 1);

	// draw our quad
	gl::DrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	// render - cleanup.
	gl::DisableVertexAttribArray(0);
	gl::DisableVertexAttribArray(1);
	gl::BindTexture(GL_TEXTURE_2D, 0);

	gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in initApplication() will be called by Shell once per run, before the rendering context is created.
Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result OGLESGaussianBlur::initApplication()
{
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result OGLESGaussianBlur::initView()
{
	// initialize the device resources object
	_deviceResources.reset(new DeviceResources());

	// create an OGLES context
	_deviceResources->context = pvr::createEglContext();
	_deviceResources->context->init(getWindow(), getDisplay(), getDisplayAttributes(), pvr::Api::OpenGLES31);

	if (this->_deviceResources->context->getApiVersion() < pvr::Api::OpenGLES31)
	{
		setExitMessage("This demo requires a minimum api of OpenGLES31. Setting the minimum Api to OpenGLES31");
		return pvr::Result::NotInitialized;
	}

	// set up the application for rendering.
	if (!loadVbos()) { return pvr::Result::UnknownError; }
	if (!createResources()) { return pvr::Result::UnknownError; }
	configureOnScreenFBO();

	if (!_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen()))
	{
		setExitMessage("Error: Failed to initialize the UIRenderer\n");
		return pvr::Result::NotInitialized;
	}

	_deviceResources->uiRenderer.getDefaultTitle()->setText("Gaussian Blur");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultDescription()->setText("Left hand side samples from the original texture.\nRight hand side samples from the Gaussian Blurred texture.");
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();

	gl::Disable(GL_DEPTH_TEST);
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  Code in releaseView() will be called by PVRShell when theapplication quits or before a change in the rendering context.
\return Return Result::Success if no error occurred
***********************************************************************************************************************/
pvr::Result OGLESGaussianBlur::releaseView()
{
	_deviceResources.reset();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
If the rendering context is lost, quitApplication() will not be called.x
***********************************************************************************************************************/
pvr::Result OGLESGaussianBlur::quitApplication()
{
	_deviceResources.release();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  Code in beginpass() sets up the FBO for the next render in our current frame. Binds, clears and sets up the
viewport.
***********************************************************************************************************************/
void OGLESGaussianBlur::beginPass()
{
	// Setup the Framebuffer for rendering!
	gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, _deviceResources->fbo.fbo);
	gl::ClearColor(.5f, .5f, .5f, 1.0f);
	gl::Viewport(0, 0, _deviceResources->fbo.renderArea.width, _deviceResources->fbo.renderArea.height);
	gl::Clear(GL_COLOR_BUFFER_BIT);
}

/*!*********************************************************************************************************************
\return Return Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result OGLESGaussianBlur::renderFrame()
{
	debugLogApiError("Frame begin");
	beginPass();
	renderBlurredQuad();
	debugLogApiError("Frame end");

	//UIRENDERER
	{
		// record the commands
		_deviceResources->uiRenderer.beginRendering();

		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getDefaultDescription()->render();
		_deviceResources->uiRenderer.endRendering();
	}

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight());
	}

	_deviceResources->context->swapBuffers();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return an auto ptr to the demo supplied by the user
\brief  This function must be implemented by the user of the shell. The user should return its
Shell object defining the behavior of the application.
***********************************************************************************************************************/
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::unique_ptr<pvr::Shell>(new OGLESGaussianBlur()); }
