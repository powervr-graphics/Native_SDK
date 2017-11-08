/*!*********************************************************************************************************************
\File         OGLESIntroducingPVRCamera.cpp
\Title        Texture Streaming
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief  Demonstrates texture streaming using platform-specific functionality
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsGles.h"
#include "PVRCamera/PVRCamera.h"

namespace Configuration {
#if defined(__ANDROID__)
const char* ShaderDefines[] = { "ANDROID=1" };
int NumShaderDefines = 1;
#elif defined(TARGET_OS_IPHONE)
const char* ShaderDefines[] = { "IOS=1" };
int NumShaderDefines = 1;
#else
const char** ShaderDefines = NULL;
int NumShaderDefines = 0;
#endif
const char* VertexShaderFile = "VertShader.vsh";
const char* FragShaderFile = "FragShader.fsh";
}

/*!*********************************************************************************************************************
Class implementing the pvr::Shell functions.
***********************************************************************************************************************/
class OGLESIntroducingPVRCamera : public pvr::Shell
{
	pvr::EglContext _context;
	int32_t _uvTransformLocation;
	GLuint _program;

	// UIRenderer class used to display text
	pvr::ui::UIRenderer _uiRenderer;

	// Camera interface
	pvr::CameraInterface _camera;

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();
};

const glm::vec2 VBOmem[] =
{
	//POSITION,
	{ 1., -1. },  //1:BR
	{ -1., -1. }, //0:BL
	{ 1., 1. },   //2:TR
	{ -1., 1. },  //3:TL
};

/*!*********************************************************************************************************************
\return Return ::pvr::Result::Success if no error occured
\brief  Code in initApplication() will be called by Shell once per run, before the rendering _context is created.
  Used to configure the application window and rendering _context (API version, vsync, window size etc.), and to load modules
  and objects not dependent on the _context.
***********************************************************************************************************************/
pvr::Result OGLESIntroducingPVRCamera::initApplication()
{
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return ::pvr::Result::Success if no error occurred
\brief  Will be called by Shell once per run, just before exiting the _program. Nothing to do in this demo.
***********************************************************************************************************************/
pvr::Result OGLESIntroducingPVRCamera::quitApplication() { return pvr::Result::Success; }

/*!*********************************************************************************************************************
\return Return ::pvr::Result::Success if no error occured
\brief  Code in initView() will be called by PVRShell upon initialization, and after any change to the rendering _context.Used to
  initialize variables that are dependent on the rendering _context (i.e. API objects)
***********************************************************************************************************************/
pvr::Result OGLESIntroducingPVRCamera::initView()
{
	_context = pvr::createEglContext();
	_context->init(getWindow(), getDisplay(), getDisplayAttributes());

	if (!_camera.initializeSession(pvr::HWCamera::Front, getWidth(), getHeight()))
	{
		return pvr::Result::UnknownError;
	}

	//  Load and compile the shaders & link programs
	{
		const char* attribNames[] = { "inVertex" };
		uint16_t attribIndices[] = { 0 };
		uint16_t numAttribs = 1;
		_program = pvr::utils::createShaderProgram(*this, Configuration::VertexShaderFile, Configuration::FragShaderFile,
		           attribNames, attribIndices, numAttribs, Configuration::ShaderDefines, Configuration::NumShaderDefines);
		if (!_program) { return pvr::Result::UnknownError; }
		_uvTransformLocation = gl::GetUniformLocation(_program, "uvTransform");
	}
	if (!_uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _context->getApiVersion() == pvr::Api::OpenGLES2)) { return pvr::Result::UnknownError; }
	_uiRenderer.getDefaultDescription()->setText("Streaming of hardware Camera video preview");
	_uiRenderer.getDefaultDescription()->commitUpdates();
	_uiRenderer.getDefaultTitle()->setText("IntroducingPVRCamera");
	_uiRenderer.getDefaultTitle()->commitUpdates();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in releaseView() will be called by Shell when the application quits or before a change in the rendering _context.
***********************************************************************************************************************/
pvr::Result OGLESIntroducingPVRCamera::releaseView()
{
	// Clean up AV capture
	_camera.destroySession();

	// Release UIRenderer resources
	_uiRenderer.release();
	gl::DeleteProgram(_program);
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Main rendering loop function of the _program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result OGLESIntroducingPVRCamera::renderFrame()
{
	gl::Clear(GL_COLOR_BUFFER_BIT);
	_camera.updateImage();

	gl::BindBuffer(GL_ARRAY_BUFFER, 0);
	gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#ifndef IOS
	if (_camera.hasRgbTexture()) // IF: is not IOS...
	{
		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindTexture(GL_TEXTURE_2D, _camera.getRgbTexture());
	}
	else
	{
		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindTexture(GL_TEXTURE_2D, _camera.getLuminanceTexture());
		gl::ActiveTexture(GL_TEXTURE1);
		gl::BindTexture(GL_TEXTURE_2D, _camera.getChrominanceTexture());
	}
#endif
	gl::UseProgram(_program);
	gl::EnableVertexAttribArray(0);
	gl::DisableVertexAttribArray(1);
	gl::DisableVertexAttribArray(2);
	gl::UniformMatrix4fv(_uvTransformLocation, 1, GL_FALSE, glm::value_ptr(_camera.getProjectionMatrix()));
	gl::VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8, VBOmem);
	gl::DrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	_uiRenderer.beginRendering();
	_uiRenderer.getDefaultTitle()->render();
	_uiRenderer.getDefaultDescription()->render();
	_uiRenderer.getSdkLogo()->render();
	_uiRenderer.endRendering();

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight());
	}

	_context->swapBuffers();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return an auto ptr to the demo supplied by the user
\brief  This function must be implemented by the user of the shell. The user should return its PVRShell object defining the behaviour of the application.
***********************************************************************************************************************/
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::unique_ptr<pvr::Shell>(new OGLESIntroducingPVRCamera()); }
