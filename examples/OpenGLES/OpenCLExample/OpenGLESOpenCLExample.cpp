/*!********************************************************************************************
\File         OpenGLESOpenCLExample.cpp
\Title        OpenCLExample
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief
***********************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRAssets/PVRAssets.h"
#include "PVRUtils/PVRUtilsGles.h"
#include "PVRUtils/OpenCL/OpenCLUtils.h"

/**********************************************************************************************
Content file names
***********************************************************************************************/

namespace Files {
const char QuadVertShaderSrc[] = "QuadVertShader_ES3.vsh";
const char QuadFragShaderSrc[] = "QuadFragShader_ES3.fsh";
const char ImageTexture[] = "Lenna.pvr";
const char KernelSrc[] = "ConvolutionKernel.cl";
} // namespace Files
namespace Kernel {
enum Enum
{
	Box,
	Erode,
	Dilate,
	EdgeDetect,
	Sobel,
	Guassian,
	Emboss,
	Sharpen,
	Count,
	Copy = Count
};

const char* entry[Count + 1] = { "box_3x3", "erode_3x3", "dilate_3x3", "edgedetect_3x3", "sobel_3x3", "gaussian_3x3", "emboss_3x3", "sharpen_3x3", "copy" };

const char* names[Count + 1] = { "Box filter", "Erode", "Dilate", "Edge Detection", "Sobel", "Gaussian", "Emboss", "Sharpen", "Original" };
} // namespace Kernel
struct OpenCLObjects
{
	cl::Platform platform;
	cl::Device device;
	cl::Context context;
	cl::CommandQueue commandqueue;
	cl::Program program;
	cl::Kernel kernels[Kernel::Count + 1];
};

/*!********************************************************************************************
Class implementing the pvr::Shell functions.
***********************************************************************************************/
class OpenGLESOpenCLExample : public pvr::Shell
{
	struct DeviceResources
	{
		pvr::EglContext context;
		OpenCLObjects oclContext;
		// Programs
		GLuint progDefault;

		GLuint sharedImageGl;

		// The shared image
		EGLImage sharedImageEgl;

		cl::Image2D imageCl_Input;
		cl::Image2D imageCl_ClToGl;
		cl::Image2D imageCl_Backup;
		cl::Sampler samplerCl;

		bool supportsEglImage;
		bool supportsEglClSharing;

		// Vbos/Ibos
		std::vector<GLuint> vbos;
		std::vector<GLuint> ibos;
		bool useEglClSharing()
		{
			return supportsEglImage && supportsEglClSharing;
		}

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;

		DeviceResources() : progDefault() {}
		~DeviceResources()
		{
			if (vbos.size())
			{
				gl::DeleteBuffers(static_cast<GLsizei>(vbos.size()), vbos.data());
				vbos.clear();
			}
			if (ibos.size())
			{
				gl::DeleteBuffers(static_cast<GLsizei>(ibos.size()), ibos.data());
				ibos.clear();
			}

			if (progDefault)
			{
				gl::DeleteProgram(progDefault);
				progDefault = 0;
			}

			samplerCl = cl::Sampler();
			imageCl_Input = cl::Image2D();
			imageCl_ClToGl = cl::Image2D();
			imageCl_Backup = cl::Image2D();

			gl::DeleteTextures(1, &sharedImageGl);

			egl::ext::DestroyImageKHR(egl::GetCurrentDisplay(), sharedImageEgl);
		}
	};

	std::unique_ptr<DeviceResources> _deviceResources;

	pvr::utils::VertexConfiguration _vertexConfig;

	std::vector<uint8_t> _rawImageData;
	glm::ivec2 _imageDimensions;

	uint32_t _currentKernel;
	float _kernelTime = 0;
	float _modeTime = 0;
	bool _demoMode = true;
	bool _mode = false;

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void createPipeline();
	void updateSubtitleText();
	void drawAxisAlignedQuad();
	void eventMappedInput(pvr::SimplifiedInput e);
	void createOpenCLObjects();
	void initClImages();
	void initKernels();
	pvr::Texture imageData;
	std::vector<unsigned char> imageTexels;
};

void OpenGLESOpenCLExample::createOpenCLObjects()
{
	imageData = pvr::assets::textureLoad(getAssetStream(Files::ImageTexture), pvr::TextureFileFormat::PVR);

	auto& clo = _deviceResources->oclContext;
	createOpenCLContext(clo.platform, clo.device, clo.context, clo.commandqueue, 0, CL_DEVICE_TYPE_GPU, 0, 0);

	auto kernelSrc = getAssetStream(Files::KernelSrc);

	clo.program = loadKernel(clo.context, clo.device, *kernelSrc);

	imageTexels.resize(imageData.getWidth() * imageData.getHeight() * 4);

	gl::GenTextures(1, &_deviceResources->sharedImageGl);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->sharedImageGl);
	gl::TexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, imageData.getWidth(), imageData.getHeight());
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	gl::TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageData.getWidth(), imageData.getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, imageData.getDataPointer());

	_deviceResources->supportsEglImage = egl::isEglExtensionSupported("EGL_KHR_image");
	_deviceResources->supportsEglClSharing = cl::isExtensionSupported(_deviceResources->oclContext.platform, "cl_khr_egl_image");
	if (_deviceResources->supportsEglImage && _deviceResources->supportsEglClSharing)
	{
		Log(LogLevel::Information, "Using EGL Image sharing with CL extension [EGL_KHR_image and cl_khr_egl_image].\n");
		_deviceResources->sharedImageEgl =
			egl::ext::CreateImageKHR(egl::GetCurrentDisplay(), egl::GetCurrentContext(), EGL_GL_TEXTURE_2D_KHR, (EGLClientBuffer)(size_t)_deviceResources->sharedImageGl, NULL);
		assertion(egl::GetError() == EGL_SUCCESS, "Failed to create KHR image");
		Log(LogLevel::Information, "Created EGL object %d as shared from %d", _deviceResources->sharedImageEgl);
	}
	else
	{
		if (!_deviceResources->supportsEglImage)
		{
			Log(LogLevel::Information, "EGL_KHR_image extension not supported\n");
		}
		if (!_deviceResources->supportsEglClSharing)
		{
			Log(LogLevel::Information, "cl_khr_egl_image extension not supported\n");
		}
		Log(LogLevel::Information, "Extensions necessary for Image sharing (interop) path not available. Using CPU fallback.\n");
	}

	initClImages();
}

void OpenGLESOpenCLExample::initKernels()
{
	auto& clobj = _deviceResources->oclContext;

	for (uint32_t i = 0; i < Kernel::Count + 1; ++i)
	{
		cl_int errcode = 0;

		// Create kernel based on function name
		clobj.kernels[i] = cl::Kernel(clobj.program, Kernel::entry[i], &errcode);

		if (clobj.kernels[i]() == NULL || errcode != CL_SUCCESS)
		{
			throw pvr::InvalidOperationError(pvr::strings::createFormatted("Error: Failed to create kernel [%s] with code [%s]", Kernel::entry[i], cl::getOpenCLError(errcode)));
		}

		// Set all kernel arguments
		errcode |= clobj.kernels[i].setArg(0, sizeof(cl_mem), &_deviceResources->imageCl_Input());
		errcode |= clobj.kernels[i].setArg(1, sizeof(cl_mem), &_deviceResources->imageCl_ClToGl());
		errcode |= clobj.kernels[i].setArg(2, sizeof(cl_sampler), &_deviceResources->samplerCl());

		if (errcode != CL_SUCCESS)
		{
			throw pvr::InvalidOperationError(
				pvr::strings::createFormatted("Error: Failed to set kernel arguments for kernel [%s] with error [%s]", Kernel::entry[i], cl::getOpenCLError(errcode)));
		}
	}
}

void OpenGLESOpenCLExample::initClImages()
{
	cl_int errcode;
	cl::ImageFormat format;

	if (imageData.getPixelFormat() != pvr::PixelFormat::RGBA_8888)
	{
		throw pvr::InvalidDataError("Only RGBA8888 format supported for the input image of this application. Please replace InputImage.pvr with a compatible image.");
	}

	format.image_channel_order = CL_RGBA;
	format.image_channel_data_type = CL_UNORM_INT8;
	auto& clobj = _deviceResources->oclContext;

	_deviceResources->imageCl_Input = cl::Image2D(clobj.context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE, format, imageData.getWidth(), imageData.getHeight(), 0, NULL, &errcode);
	if (errcode != CL_SUCCESS || _deviceResources->imageCl_Input.get() == NULL)
	{
		throw pvr::InvalidOperationError(pvr::strings::createFormatted("Failed to create shared OpenCL image input with code %s", cl::getOpenCLError(errcode)));
	}

	std::array<size_t, 3> origin = { 0, 0, 0 };
	std::array<size_t, 3> region = { imageData.getWidth(), imageData.getHeight(), 1 };
	size_t image_row_pitch = imageData.getWidth() * 4;
	char* mappedMemory =
		(char*)clobj.commandqueue.enqueueMapImage(_deviceResources->imageCl_Input, CL_TRUE, CL_MAP_WRITE, origin, region, &image_row_pitch, NULL, NULL, NULL, &errcode);

	if (errcode != CL_SUCCESS || mappedMemory == NULL)
	{
		throw pvr::InvalidOperationError(pvr::strings::createFormatted("ERROR: Failed to map bufferwith code %s", cl::getOpenCLError(errcode)));
	}

	memcpy(mappedMemory, imageData.getDataPointer(), imageData.getHeight() * imageData.getWidth() * 4);

	if (CL_SUCCESS != clobj.commandqueue.enqueueUnmapMemObject(_deviceResources->imageCl_Input, mappedMemory, NULL, NULL))
	{
		throw pvr::InvalidOperationError(pvr::strings::createFormatted("ERROR: Failed to unmap input image", cl::getOpenCLError(errcode)));
	}

	if (_deviceResources->useEglClSharing())
	{
		clCreateFromEGLImageKHR_fn clCreateFromEGLImageKHR = (clCreateFromEGLImageKHR_fn)clGetExtensionFunctionAddressForPlatform(clobj.platform(), "clCreateFromEGLImageKHR");
		cl_mem memImageClToGl = clCreateFromEGLImageKHR(clobj.context.get(), NULL, (CLeglImageKHR)_deviceResources->sharedImageEgl, CL_MEM_READ_WRITE, NULL, &errcode);
		_deviceResources->imageCl_ClToGl = cl::Image2D(memImageClToGl, true);
		clReleaseMemObject(memImageClToGl);
		Log(LogLevel::Information, "Created OpenCL image as shared from object %d", _deviceResources->sharedImageEgl);
	}
	else
	{
		_deviceResources->imageCl_ClToGl =
			cl::Image2D(clobj.context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE, format, imageData.getWidth(), imageData.getHeight(), 0, NULL, &errcode);
	}

	if (_deviceResources->imageCl_ClToGl.get() == NULL || errcode != CL_SUCCESS)
	{
		throw pvr::InvalidOperationError(pvr::strings::createFormatted("ERROR: Failed to create shared image object (output) with code %s", cl::getOpenCLError(errcode)));
	}

	_deviceResources->imageCl_Backup = cl::Image2D(clobj.context, CL_MEM_READ_WRITE, format, imageData.getWidth(), imageData.getHeight(), 0u, NULL, &errcode);
	if (_deviceResources->imageCl_Backup.get() == NULL || errcode != CL_SUCCESS)
	{
		throw pvr::InvalidOperationError(pvr::strings::createFormatted("ERROR: Failed to create shared image object (backup) with code %s", cl::getOpenCLError(errcode)));
	}

	mappedMemory = (char*)clobj.commandqueue.enqueueMapImage(_deviceResources->imageCl_Backup, CL_TRUE, CL_MAP_WRITE, origin, region, &image_row_pitch, NULL, NULL, NULL, &errcode);
	if (errcode != CL_SUCCESS || mappedMemory == NULL)
	{
		throw pvr::InvalidOperationError(pvr::strings::createFormatted("ERROR: Failed to map image (backup) with code %s", cl::getOpenCLError(errcode)));
	}

	memcpy(mappedMemory, imageData.getDataPointer(), imageData.getHeight() * imageData.getWidth() * 4);

	if (CL_SUCCESS != clobj.commandqueue.enqueueUnmapMemObject(_deviceResources->imageCl_Backup, mappedMemory, NULL, NULL))
	{
		throw pvr::InvalidOperationError(pvr::strings::createFormatted("ERROR: Failed to unmap backup image with code %s", cl::getOpenCLError(errcode)));
	}

	_deviceResources->samplerCl = cl::Sampler(clobj.context, CL_FALSE, CL_ADDRESS_CLAMP, CL_FILTER_NEAREST, &errcode);
	if (_deviceResources->samplerCl.get() == NULL || errcode != CL_SUCCESS)
	{
		throw pvr::InvalidOperationError(pvr::strings::createFormatted("ERROR: Failed to create OpenCL sampler with code %s", cl::getOpenCLError(errcode)));
	}

	clobj.commandqueue.finish();

	initKernels();
}

/*!********************************************************************************************
\brief  Handles user imageCl_Input and updates live variables accordingly.
***********************************************************************************************/
void OpenGLESOpenCLExample::eventMappedInput(pvr::SimplifiedInput e)
{
	// Object+Bloom, object, bloom
	switch (e)
	{
	case pvr::SimplifiedInput::Left:
		if (_currentKernel == 0)
		{
			_currentKernel = Kernel::Count - 1;
		}
		else
		{
			--_currentKernel;
		}
		_kernelTime = 0;
		_modeTime = 0;
		_mode = true;
		updateSubtitleText();
		break;
	case pvr::SimplifiedInput::Right:
		if (++_currentKernel == Kernel::Count)
		{
			_currentKernel = 0;
		}
		_kernelTime = 0;
		_modeTime = 0;
		_mode = true;
		updateSubtitleText();
		break;
	case pvr::SimplifiedInput::Action1:
	case pvr::SimplifiedInput::Action2:
	case pvr::SimplifiedInput::Action3:
		_demoMode = !_demoMode;
		_kernelTime = 0;
		_modeTime = 0;
		_mode = true;
		break;
	case pvr::SimplifiedInput::ActionClose:
		this->exitShell();
		break;
	default:
		break;
	}
}

/*!********************************************************************************************
\brief  Loads and compiles the shaders and links the shader programs
\return Return true if no error occurred required for this training course
***********************************************************************************************/
void OpenGLESOpenCLExample::createPipeline()
{
	_deviceResources->progDefault = pvr::utils::createShaderProgram(*this, Files::QuadVertShaderSrc, Files::QuadFragShaderSrc, NULL, NULL, 0);

	// Set the sampler2D variable to the first texture unit
	gl::UseProgram(_deviceResources->progDefault);
	GLint loc = gl::GetUniformLocation(_deviceResources->progDefault, "sTexture");
	gl::Uniform1i(loc, 0);
	gl::UseProgram(0);
}

/*!********************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in initApplication() will be called by pvr::Shell once per run, before the rendering
  context is created.
  Used to initialize variables that are not dependent on it (e.g. external modules,
  loading meshes, etc.)
  If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************/
pvr::Result OpenGLESOpenCLExample::initApplication()
{
	_currentKernel = 0;
	_kernelTime = 0;
	_modeTime = 0;
	_mode = true;
	return pvr::Result::Success;
}

/*!********************************************************************************************
\return Return  pvr::Result::Success if no error occured
\brief  Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.
quitApplication() will not be called every time the rendering context is lost, only before application exit.
***********************************************************************************************/
pvr::Result OpenGLESOpenCLExample::quitApplication()
{
	return pvr::Result::Success;
}

/*!********************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in initView() will be called by pvr::Shell upon initialization or after a change
  in the rendering context. Used to initialize variables that are dependent on the rendering
  context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************/
pvr::Result OpenGLESOpenCLExample::initView()
{
	_deviceResources = std::unique_ptr<DeviceResources>(new DeviceResources());
	_deviceResources->context = pvr::createEglContext();
	_deviceResources->context->init(getWindow(), getDisplay(), getDisplayAttributes(), pvr::Api::OpenGLES3);

	std::vector<cl::Platform> platforms;

	createOpenCLObjects();
	createPipeline();
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen());

	_deviceResources->uiRenderer.getDefaultTitle()->setText("OpenCLExample");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText("Left / right: Rendering mode\n");
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();
	updateSubtitleText();

	gl::Enable(GL_CULL_FACE);
	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);
	return pvr::Result::Success;
}

/*!********************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in releaseView() will be called by pvr::Shell when the application quits or before
a change in the rendering context.
***********************************************************************************************/
pvr::Result OpenGLESOpenCLExample::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}
/*!********************************************************************************************
\return Return Result::Suceess if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************/
pvr::Result OpenGLESOpenCLExample::renderFrame()
{
	debugThrowOnApiError("Frame begin");

	const float modeDuration = 1500.f;
	const float numfilterDisplays = 6.f;

	if (_demoMode)
	{
		_modeTime += getFrameTime();
		_kernelTime += getFrameTime();
		if (_modeTime > modeDuration)
		{
			_mode = !_mode;
			_modeTime = 0.f;
		}
		if (_kernelTime > modeDuration * numfilterDisplays)
		{
			if (++_currentKernel == Kernel::Count)
			{
				_currentKernel = 0;
			}
			_kernelTime = 0.f;
			updateSubtitleText();
		}
	}
	gl::Finish();

	cl_int errcode = 0;

	cl::NDRange dims(imageData.getWidth(), imageData.getHeight());
	cl::NDRange wgs(8, 4);
	cl::NDRange offset(0, 0, 0);

	auto& clobj = _deviceResources->oclContext;
	auto& queue = _deviceResources->oclContext.commandqueue;
	auto& kernel = clobj.kernels[_mode ? _currentKernel : Kernel::Copy];
	if (_deviceResources->useEglClSharing())
	{
		static clEnqueueAcquireEGLObjectsKHR_fn clEnqueueAcquireEGLObjectsKHR =
			(clEnqueueAcquireEGLObjectsKHR_fn)clGetExtensionFunctionAddressForPlatform(clobj.platform(), "clEnqueueAcquireEGLObjectsKHR");
		errcode = clEnqueueAcquireEGLObjectsKHR(queue(), 1, &_deviceResources->imageCl_ClToGl(), 0, NULL, NULL);
		if (errcode != CL_SUCCESS)
		{
			throw pvr::InvalidOperationError(pvr::strings::createFormatted("Failed to acquire EGL Objects with code %s", cl::getOpenCLError(errcode)));
		}
	}
	// Use the original image as starting point for the first iteration
	if (kernel.setArg(0, sizeof(cl_mem), &_deviceResources->imageCl_Backup) != CL_SUCCESS)
	{
		throw pvr::InvalidOperationError(pvr::strings::createFormatted("Failed to set kernel arg 0 with code %s", cl::getOpenCLError(errcode)));
	}
	if (kernel.setArg(1, sizeof(cl_mem), &_deviceResources->imageCl_ClToGl) != CL_SUCCESS)
	{
		throw pvr::InvalidOperationError(pvr::strings::createFormatted("Failed to set kernel arg 1 with code %s", cl::getOpenCLError(errcode)));
	}

	// Launch kernel
	errcode = queue.enqueueNDRangeKernel(kernel, offset, dims, wgs, NULL, NULL);
	if (errcode != CL_SUCCESS)
	{
		throw pvr::InvalidOperationError(pvr::strings::createFormatted("Failed to execure kernel with code %s", cl::getOpenCLError(errcode)));
	}

	if (_deviceResources->useEglClSharing()) // Release the shared image from CL ownership, so we can render with it,...
	{
		static clEnqueueReleaseEGLObjectsKHR_fn clEnqueueReleaseEGLObjectsKHR =
			(clEnqueueReleaseEGLObjectsKHR_fn)clGetExtensionFunctionAddressForPlatform(clobj.platform(), "clEnqueueReleaseEGLObjectsKHR");
		errcode = clEnqueueReleaseEGLObjectsKHR(queue(), 1, &_deviceResources->imageCl_ClToGl(), 0, NULL, NULL);
		if (errcode != CL_SUCCESS)
		{
			throw pvr::InvalidOperationError(pvr::strings::createFormatted("Failed to release EGL Objects with code %s", cl::getOpenCLError(errcode)));
		}
	}
	else // Otherwise, copy the data from the shared image...
	{
		const std::array<size_t, 3> origin = { 0, 0, 0 };
		const std::array<size_t, 3> region = { imageData.getWidth(), imageData.getHeight(), 1 };
		const size_t row_pitch = imageData.getWidth() * 4;
		errcode = queue.enqueueReadImage(_deviceResources->imageCl_ClToGl, CL_TRUE, origin, region, row_pitch, 0, imageTexels.data(), NULL, NULL);
		if (errcode != CL_SUCCESS)
		{
			throw pvr::InvalidOperationError(pvr::strings::createFormatted("Failed to Failed to enqueue read image with code %s", cl::getOpenCLError(errcode)));
		}
	}
	queue.finish();

	gl::UseProgram(_deviceResources->progDefault);
	// Draw quad
	gl::ClearColor(.25f, .25f, .25f, 1.f);
	gl::ClearDepthf(1.0f);
	gl::Viewport(0, 0, getWidth(), getHeight());
	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl::Disable(GL_DEPTH_TEST);

	// bind the  texture
	gl::Uniform1i(gl::GetUniformLocation(_deviceResources->progDefault, "sTexture"), 0);
	gl::ActiveTexture(GL_TEXTURE0);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->sharedImageGl);

	// Read back convolved data and feed back into texture if we're not using CL_KHR_egl_image.
	// If we ARE using CL_KHR_egl_image, there's no point - they're already in the shared image.
	if (!_deviceResources->useEglClSharing())
	{
		gl::TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageData.getWidth(), imageData.getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, imageTexels.data());
	}
	drawAxisAlignedQuad();

	// UIRENDERER
	{
		// record the commands
		_deviceResources->uiRenderer.beginRendering();

		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getDefaultControls()->render();
		_deviceResources->uiRenderer.getDefaultDescription()->render();
		_deviceResources->uiRenderer.endRendering();
	}
	debugThrowOnApiError("Frame end");

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
void OpenGLESOpenCLExample::updateSubtitleText()
{
	_deviceResources->uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("%s", Kernel::names[_currentKernel]));
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
}

/*!********************************************************************************************
\brief  Add the draw commands for a full screen quad to a commandbuffer
***********************************************************************************************/
void OpenGLESOpenCLExample::drawAxisAlignedQuad()
{
	gl::DisableVertexAttribArray(0);
	gl::DisableVertexAttribArray(1);
	gl::DisableVertexAttribArray(2);
	gl::BindBuffer(GL_ARRAY_BUFFER, 0);
	gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	gl::DrawArrays(GL_TRIANGLES, 0, 3);
}

/*!********************************************************************************************
\return Return auto ptr to the demo supplied by the user
\brief  This function must be implemented by the user of the shell.
The user should return its pvr::Shell object defining the behaviour of the application.
***********************************************************************************************/
std::unique_ptr<pvr::Shell> pvr::newDemo()
{
	return std::unique_ptr<pvr::Shell>(new OpenGLESOpenCLExample());
}
