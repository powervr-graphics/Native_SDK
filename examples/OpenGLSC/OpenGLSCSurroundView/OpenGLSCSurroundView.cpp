/*!
\brief	This example demonstrates how to use OpenGLES to build a surround view application.
\file	OpenGLSCSurroundView.cpp
\author	PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"

// For test on safety critical environment, change the head and cmake definitions
#include "PVRUtils/PVRUtilsGles.h"

#include "glm/glm.hpp"

#ifdef __linux__
#include <linux/videodev2.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <drm/drm_fourcc.h>
#endif

const char QuadrantModelFileName[] = "Environment.gltf";
const char SurroundVertShaderFileName[] = "SurroundVertShader.vsh";
const char SurroundFragShaderFileName[] = "SurroundFragShader.fsh";
const char CarVertShaderFileName[] = "CarVertShader.vsh";
const char CarFragShaderFileName[] = "CarFragShader.fsh";
const char mipmapPostProcessVertShaderFileName[] = "mipmapPostProcessVertShader.vsh";
const char mipmapPostProcessFragShaderFileName[] = "mipmapPostProcessFragShader.fsh";
const char carModelFileName[] = "ToyCar.gltf";
const char carAlbedoTextureFileName[] = "ToyCar_basecolor.pvr";
const char carNormalTextureFileName[] = "ToyCar_normal.pvr";
const char carRoughnessMetallicTextureFileName[] = "ToyCar_roughness_metallic.pvr";
const char lookupTablePBRTextureFileName[] = "brdfLUT.pvr";
const char CarBinaryShaderName[] = "CarBinaryShader.bin";
const char SurroundBinaryShaderName[] = "SurroundBinaryShader.bin";
const std::vector<std::string> vectorCameraDeviceName = { "/dev/video1" /* Front camera */, "/dev/video3" /* Right camera */, "/dev/video2" /* Back camera */,
	"/dev/video0" /* Left camera */ };

/// <summary>Constant with the amount of cubemap faces in case a environment map is built for improved car rendering.</summary>
const GLuint numCubemapFaces = 6;

/// <summary>Enum class encapsulating the IDs of the different uniforms used in the surround view pass.</summary>
enum class EnumSurroundUniformID
{
	/// <summary>Uniform ID with one of the camera indices (in {0, 1, 2, 3}) needed for the surround drawing.</summary>
	camera0 = 0,

	/// <summary>Uniform ID with one of the camera indices (in {0, 1, 2, 3}) needed for the surround drawing.</summary>
	camera1,

	/// <summary>Uniform ID with one of the textures needed for the surround drawing.</summary>
	cameraTexture0,

	/// <summary>Uniform ID with one of the textures needed for the surround drawing.</summary>
	cameraTexture1,

	/// <summary>Uniform ID for the view projection matrix used for the surround drawing.</summary>
	viewProjectionMatrix,

	/// <summary>Uniform ID for the world transfrom matrix used for the surround drawing.</summary>
	worldTransform,

	/// <summary>Number elements in this enum.</summary>
	maxIndex
};

/// <summary>Enum class encapsulating the IDs of the different uniforms used in the car pass.</summary>
enum class EnumCarUniformID
{
	/// <summary>Uniform ID for the world transfrom matrix used for the car drawing.</summary>
	worldTransform = 0,

	/// <summary>Uniform ID for the view projection matrix used for the car drawing.</summary>
	viewProjectionMatrix,

	/// <summary>Uniform ID for the camera position used for the surround drawing.</summary>
	cameraPosition,

	/// <summary>Uniform ID for the number of mip map levels the environment map has.</summary>
	numEnvironmentMipMap,

	/// <summary>Number elements in this enum.</summary>
	maxIndex
};

/// <summary>This struct contains information from each one of the cameras used in the surround view, some is pure goemetrical information like
/// the transform where each camera is located, some is cemara physical device informatio like barrel distortion .</summary>
struct SurroundCamera
{
	/// <summary>Name of the camera for spatial reference, currently one from the set {"Front", "Right", "Back", "Left"}.</summary>
	std::string name;

	/// <summary>Camera transform.</summary>
	glm::mat4 transform;

	/// <summary>Physical camera's barrel distortion.</summary>
	glm::vec3 barrelDistortion;

	/// <summary>Physical camera's tangential distortion.</summary>
	glm::vec2 tangentialDistortion;

	/// <summary>Physical camera's sensor size.</summary>
	glm::vec2 sensorSize;

	/// <summary>Physical camera's sensor centre.</summary>
	glm::vec2 sensorCentre;
};

/// <summary>Struct to store the resources needed by the MMap method for camera video streaming (zero-copy).</summary>
struct MMapCameraResources
{
	std::vector<GLuint> vectorTexture;
	std::vector<EGLImageKHR> vectorImage;
};

/// <summary>Struct to store the resources needed by the read method for camera video streaming where the camera image is copied each time.</summary>
struct ReadCameraResources
{
	/// <summary>Size of the buffer to allocate to store the physical camera image.</summary>
	int bufferSize = 0;

	/// <summary>Pointer to the data.</summary>
	void* pData = nullptr;
};

#ifdef __linux__
int xioctl(int fh, int request, void* arg)
{
	int r;

	do {
		r = ioctl(fh, request, arg);
	} while (-1 == r && EINTR == errno);

	return r;
}

class CameraManager
{
public:
	~CameraManager();
	bool initializeCameras(const std::vector<std::string>& vectorCameraName, glm::ivec2 cameraResolution);
	uint32_t getCameraNumber() { return _cameraNumber; }
	uint32_t getNumBufferCamera(uint32_t cameraIndex) { return _vectorNumBufferPerCamera[cameraIndex]; }
	int getFDForCameraBuffer(uint32_t cameraIndex, uint32_t camereBuffer) { return _vectorFD[cameraIndex][camereBuffer]; }
	int getCameraUpdateIndex(uint32_t cameraIndex) { return _vectorBufferUpdateIndex[cameraIndex]; }
	void setCameraFramerate(uint32_t cameraFramerate) { _cameraFramerate = cameraFramerate; }
	bool getCameraUpdateStatus(uint32_t cameraIndex) { return _vectorCameraUpdateStatus[cameraIndex]; }
	bool getFirstFrame() { return _firstFrame; }
	void setFirstFrame(bool firstFrame) { _firstFrame = firstFrame; }
	bool verifyCameraCapabilities();
	bool limitCameraFramerate();
	bool setupCameraExposure();
	bool setupCameraImageProperties();
	bool allocateCameraResources();
	bool allocateCameraMemoryMapResources();
	bool updateCameraExposure();
	void printCameraInformation(int deviceIndex);
	void startCapturing();
	void stopCapturing();
	void updateCameraFrame();
	bool readCameraFrame(uint32_t deviceIndex);

private:
	/// <summary>Vector with the device names of the physical cameras to use for the surround view.</summary>
	std::vector<std::string> _vectorCameraName;

	/// <summary>Number of physical cameras used.</summary>
	uint32_t _cameraNumber = 0;

	/// <summary>Framerate to set each physical camera (only if the value is > 0).</summary>
	uint32_t _cameraFramerate = 0;

	/// <summary>Vector where to store the file descriptor from each one of the cameras.</summary>
	std::vector<int> _vectorVideoDeviceFileDescriptor;

	/// <summary>Vector where to store the amount of buffers used per physical camera when using zero-copy method.</summary>
	std::vector<uint32_t> _vectorNumBufferPerCamera;

	/// <summary>Vector where to store the index of the latest buffer updated by each physical camera when using zero-copy method.</summary>
	std::vector<uint32_t> _vectorBufferUpdateIndex;

	/// <summary>File descriptor associated with DMABUF (set by driver), see https://github.com/torvalds/linux/blob/master/include/uapi/linux/videodev2.h.</summary>
	std::vector<std::vector<int>> _vectorFD;

	/// <summary>To know whether each camera is offering a new image which can be copied / zerp-copied to display in the surround environment.</summary>
	std::vector<bool> _vectorCameraUpdateStatus;

	/// <summary>Camera exposure parameter (see otput from printCameraInformation to know the range for each physical camera).</summary>
	float _cameraExposure = 64000.0f;

	/// <summary>Number of images read when for each frmae update each physical device is queried for availability.</summary>
	uint32_t _numberImagesRead = 0;

	/// <summary>Flag to know whether the first frame is being processed or not (to start capturing from the physical cameras).</summary>
	bool _firstFrame = true;

	/// <summary>Resolution of the physical device camreas used.</summary>
	glm::ivec2 _cameraResolution;
};

/// <summary>Default destructor.</summary>
CameraManager::~CameraManager()
{
	for (size_t i = 0; i < _vectorVideoDeviceFileDescriptor.size(); ++i)
	{
		if (_vectorVideoDeviceFileDescriptor[i] != -1) { close(_vectorVideoDeviceFileDescriptor[i]); }
	}
}

/// <summary>Takes the list of populated textures used in the scene and loads them into memory, uploads them into a Vulkan image and creates image views.</summary>
/// <param name="vectorCameraName">Name of the camera devices.</param>
/// <param name="cameraResolution">Pixel resolution of the cameras.</param>
bool CameraManager::initializeCameras(const std::vector<std::string>& vectorCameraName, glm::ivec2 cameraResolution)
{
	_vectorCameraName = vectorCameraName;
	_cameraNumber = static_cast<uint32_t>(_vectorCameraName.size());
	_vectorVideoDeviceFileDescriptor.resize(_cameraNumber);
	_vectorNumBufferPerCamera.resize(_cameraNumber);
	_vectorFD.resize(_cameraNumber);
	_vectorCameraUpdateStatus.resize(_cameraNumber);
	_cameraResolution = cameraResolution;
	_vectorBufferUpdateIndex.resize(_cameraNumber);

	for (uint32_t i = 0; i < _cameraNumber; ++i)
	{
		Log("Initializing camera device %s", _vectorCameraName[i].c_str());

		struct stat st;
		if (stat(_vectorCameraName[i].c_str(), &st) == -1)
		{
			Log("ERROR in initializeCamera: Cannot identify '%s'", _vectorCameraName[i].c_str());
			return false;
		}

		if (S_ISCHR(st.st_mode) == 0)
		{
			Log("ERROR in initializeCamera: '%s' is no device", _vectorCameraName[i].c_str());
			return false;
		}

		_vectorVideoDeviceFileDescriptor[i] = open(_vectorCameraName[i].c_str(), O_RDWR | O_NONBLOCK, 0);

		if (_vectorVideoDeviceFileDescriptor[i] == -1)
		{
			Log("ERROR in initializeCamera: Cannot open camera device '%s'", _vectorCameraName[i].c_str());
			return false;
		}
	}

	return true;
}

/// <summary>Verify several capabilities used in this sample for each physical camera device (video capture, read / write of buffers and streaming).</summary>
bool CameraManager::verifyCameraCapabilities()
{
	for (uint32_t i = 0; i < _cameraNumber; ++i)
	{
		Log(LogLevel::Information, "Camera: Init device at index=%d with ID=%d", i, _vectorVideoDeviceFileDescriptor[i]);

		v4l2_capability capability;
		if (xioctl(_vectorVideoDeviceFileDescriptor[i], VIDIOC_QUERYCAP, &capability) == -1)
		{
			Log("ERROR in verifyCameraCapabilities: %d is no V4L2 device", capability);
			return false;
		}

		if (!(capability.capabilities & V4L2_CAP_VIDEO_CAPTURE))
		{
			Log("ERROR in verifyCameraCapabilities: device %d has no video capture capability", i);
			return false;
		}

		if (!(capability.capabilities & V4L2_CAP_STREAMING))
		{
			Log("ERROR in verifyCameraCapabilities: %d does not support stream i/o zero-copy", i);
			return false;
		}

		v4l2_cropcap cropcap;
		// Select video input, video standard and tune here.
		memset(&cropcap, 0, sizeof(cropcap));
		cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (xioctl(_vectorVideoDeviceFileDescriptor[i], VIDIOC_CROPCAP, &cropcap) == 0)
		{
			v4l2_crop crop;
			crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			crop.c = cropcap.defrect; // reset to default
			crop.c.left = 0;
			crop.c.top = 0;
			crop.c.width = _cameraResolution.x;
			crop.c.height = _cameraResolution.y;

			if (xioctl(_vectorVideoDeviceFileDescriptor[i], VIDIOC_S_CROP, &crop) == -1)
			{
				switch (errno)
				{
				case EINVAL:
					// Cropping not supported.
					Log("ERROR in verifyCameraCapabilities: cropResult=-1, cropping not supported");
					break;
				default: {
					Log("ERROR in verifyCameraCapabilities: cropResult=-1");
					break;
				}
				}

				return false;
			}
		}
		else
		{
			Log("ERROR in verifyCameraCapabilities: Crop capability call returned value != 0");
			return false;
		}
	}

	return true;
}

/// <summary>If CameraManager::_cameraFramerate is greater than zero, limit the framerate of each physical device camera.</summary>
bool CameraManager::limitCameraFramerate()
{
	if (_cameraFramerate > 0)
	{
		for (uint32_t i = 0; i < _cameraNumber; ++i)
		{
			struct v4l2_streamparm parm;
			memset(&parm, 0, sizeof(parm));

			parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			if (xioctl(_vectorVideoDeviceFileDescriptor[i], VIDIOC_G_PARM, &parm) == -1)
			{
				Log("ERROR in limitCameraFramerate: VIDIOC_G_PARM: %s", strerror(errno));
				return false;
			}

			parm.parm.capture.timeperframe.numerator = 1;
			parm.parm.capture.timeperframe.denominator = _cameraFramerate;
			if (xioctl(_vectorVideoDeviceFileDescriptor[i], VIDIOC_S_PARM, &parm) == -1)
			{
				Log("ERROR in limitCameraFramerate: VIDIOC_S_PARM: %s", strerror(errno));
				return false;
			}
		}
	}

	return true;
}

/// <summary>Set the sharpness, auto gain and gain of each physical device camera.</summary>
bool CameraManager::setupCameraExposure()
{
	// Exposure control
	struct v4l2_control control;
	memset(&control, 0, sizeof(control));

	for (uint32_t i = 0; i < _cameraNumber; ++i)
	{
		// Sharpness
		control.id = V4L2_CID_SHARPNESS;
		control.value = 2;
		if (xioctl(_vectorVideoDeviceFileDescriptor[i], VIDIOC_S_CTRL, &control) == -1)
		{
			Log("ERROR in setupCameraExposure: could not write V4L2_CID_SHARPNESS ");
			return false;
		}

		// Disable auto gain
		control.id = V4L2_CID_AUTOGAIN;
		control.value = 0;
		if (xioctl(_vectorVideoDeviceFileDescriptor[i], VIDIOC_S_CTRL, &control) == -1)
		{
			Log("ERROR in setupCameraExposure: could not write V4L2_CID_AUTOGAIN");
			return false;
		}

		// Set gain
		control.id = V4L2_CID_GAIN;
		control.value = 128;
		if (xioctl(_vectorVideoDeviceFileDescriptor[i], VIDIOC_S_CTRL, &control) == -1)
		{
			Log("ERROR in setupCameraExposure: could not write V4L2_CID_GAIN ");
			return false;
		}
	}

	return true;
}

/// <summary>Set the image and pixel format details for each physical device camera.</summary>
bool CameraManager::setupCameraImageProperties()
{
	v4l2_format fmt;
	memset(&fmt, 0, sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = _cameraResolution.x;
	fmt.fmt.pix.height = _cameraResolution.y;
	fmt.fmt.pix.field = V4L2_FIELD_NONE;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565;

	for (uint32_t i = 0; i < _cameraNumber; ++i)
	{
		if (xioctl(_vectorVideoDeviceFileDescriptor[i], VIDIOC_S_FMT, &fmt) == -1)
		{
			Log("ERROR in setupCameraImageProperties: VIDIOC_S_FMT: %s", strerror(errno));
			return false;
		}
	}

	return true;
}

/// <summary>Allocate the resources required for the method selected to retrieve image information from each physical camera device.</summary>
bool CameraManager::allocateCameraResources()
{
	if (!allocateCameraMemoryMapResources()) { return false; }

	return true;
}

/// <summary>If the method used to retrieve camera information is zero-copy, generate
/// as many textures and EGL surfaces as buffers each physical camera device has.</summary>
bool CameraManager::allocateCameraMemoryMapResources()
{
	struct v4l2_requestbuffers req;
	memset(&req, 0, sizeof(req));
	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	for (uint32_t i = 0; i < _cameraNumber; ++i)
	{
		if (xioctl(_vectorVideoDeviceFileDescriptor[i], VIDIOC_REQBUFS, &req) == -1)
		{
			Log("ERROR in OpenGLSCSurroundView::allocateCameraMemoryMapResources: VIDIOC_REQBUFS");
			return false;
		}

		if (req.count < 2)
		{
			Log("ERROR in OpenGLSCSurroundView::allocateCameraMemoryMapResources: MMAP could not allocate enough buffers");
			return false;
		}

		_vectorNumBufferPerCamera[i] = req.count;
		_vectorFD[i].resize(req.count);

		for (uint32_t j = 0; j < req.count; ++j)
		{
			struct v4l2_exportbuffer buf;
			memset(&buf, 0, sizeof(buf));
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.index = j;
			buf.flags = O_RDWR;

			if (xioctl(_vectorVideoDeviceFileDescriptor[i], VIDIOC_EXPBUF, &buf) == -1)
			{
				Log("ERROR in CameraManager::allocateCameraMemoryMapResources: VIDIOC_EXPBUF");
				return false;
			}

			_vectorFD[i][j] = buf.fd;
		}
	}

	return true;
}

/// <summary>Set the camera exposure according to the value specified by CameraManager::_cameraExposure.</summary>
bool CameraManager::updateCameraExposure()
{
	struct v4l2_control control;
	memset(&control, 0, sizeof(control));
	control.id = V4L2_CID_EXPOSURE;
	control.value = static_cast<int>(_cameraExposure);

	for (uint32_t i = 0; i < _cameraNumber; ++i)
	{
		if (xioctl(_vectorVideoDeviceFileDescriptor[i], VIDIOC_S_CTRL, &control) == -1)
		{
			Log("ERROR: could not write V4L2_CID_EXPOSURE");
			return false;
		}
	}

	return true;
}

/// <summary>Print through console information for the physical camera device at idex given by parameter.</summary>
/// <param name="deviceIndex">Physical camera index to print information.</param>
void CameraManager::printCameraInformation(int deviceIndex)
{
	Log("Camera controls:");
	struct v4l2_queryctrl queryctrl;
	memset(&queryctrl, 0, sizeof(queryctrl));
	queryctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;

	while (ioctl(_vectorVideoDeviceFileDescriptor[deviceIndex], VIDIOC_QUERYCTRL, &queryctrl) == 0)
	{
		struct v4l2_control control;
		memset(&control, 0, sizeof(control));
		control.id = queryctrl.id;
		xioctl(_vectorVideoDeviceFileDescriptor[deviceIndex], VIDIOC_G_CTRL, &control);
		const bool controlEnabled = (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) == 0;
		Log("[%c] %s (default: %d, current: %d)\n", controlEnabled ? 'X' : ' ', queryctrl.name, (int)queryctrl.default_value, (int)control.value);
		queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}

	Log("Capture format:");
	struct v4l2_fmtdesc fmtdesc;
	memset(&fmtdesc, 0, sizeof(fmtdesc));
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	while (ioctl(_vectorVideoDeviceFileDescriptor[deviceIndex], VIDIOC_ENUM_FMT, &fmtdesc) == 0)
	{
		// Display format
		Log(" %s\n", fmtdesc.description);

		// Enumerate resolutions
		struct v4l2_frmsizeenum frmsize;
		memset(&frmsize, 0, sizeof(frmsize));
		frmsize.pixel_format = fmtdesc.pixelformat;
		frmsize.index = 0;
		while (ioctl(_vectorVideoDeviceFileDescriptor[deviceIndex], VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0)
		{
			if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) { Log(" -%dx%d\n", frmsize.discrete.width, frmsize.discrete.height); }
			else if (frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE)
			{
				Log(" -%dx%d\n", frmsize.stepwise.max_width, frmsize.stepwise.max_height);
			}
			frmsize.index++;
		}

		fmtdesc.index++;
	}
}

/// <summary>If the method to retrieve images from reach physical camera is zero-copy, start capturing from the physical cameras.</summary>
void CameraManager::startCapturing()
{
	for (uint32_t i = 0; i < _cameraNumber; ++i)
	{
		for (uint32_t j = 0; j < _vectorNumBufferPerCamera[i]; ++j)
		{
			struct v4l2_buffer buf;
			memset(&buf, 0, sizeof(buf));
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = j;

			if (xioctl(_vectorVideoDeviceFileDescriptor[i], VIDIOC_QBUF, &buf) == -1)
			{
				Log("ERROR: startCapturing: VIDIOC_QBUF: %s", strerror(errno));
				return;
			}
		}

		enum v4l2_buf_type type;
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (xioctl(_vectorVideoDeviceFileDescriptor[i], VIDIOC_STREAMON, &type) == -1)
		{
			Log("ERROR: VIDIOC_STREAMON");
			return;
		}
	}
}

/// <summary>If the method to retrieve images from reach physical camera is zero-copy, stop capturing from the physical cameras.</summary>
void CameraManager::stopCapturing()
{
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	for (uint32_t i = 0; i < _cameraNumber; ++i)
	{
		if (xioctl(_vectorVideoDeviceFileDescriptor[i], VIDIOC_STREAMOFF, &type) == -1) { Log("ERROR: VIDIOC_STREAMOFF"); }
	}
}

/// <summary>Verify for each physical camera device whether the device is ready to offer a new image and retrieve it if it is the case.</summary>
void CameraManager::updateCameraFrame()
{
	_numberImagesRead = 0;

	fd_set fds;

	int maxFd = -1;
	for (uint32_t i = 0; i < _cameraNumber; ++i)
	{
		FD_SET(_vectorVideoDeviceFileDescriptor[i], &fds);
		maxFd = std::max(maxFd, _vectorVideoDeviceFileDescriptor[i]);
	}

	// Set timeout
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	// Check if any device is ready to be read
	const int r = select(maxFd + 1, &fds, nullptr, nullptr, &tv);
	if (r == -1)
	{
		Log("ERROR: CameraManager::update: Could not select");
		return;
	}
	if (r == 0)
	{
		// It's OK to timeout, it means no frame is ready and we can just try again next frame
		Log("ERROR: CameraManager::updateCameraFrame r == 0 case");
		return;
	}

	// Iterate over all the devices since at least one of them is ready to be read
	bool status = false;
	for (uint32_t i = 0; i < _cameraNumber; ++i)
	{
		if (FD_ISSET(_vectorVideoDeviceFileDescriptor[i], &fds))
		{
			bool frameOk = readCameraFrame(i);
			_numberImagesRead += frameOk ? 1 : 0;
			status |= frameOk;

			_vectorCameraUpdateStatus[i] = frameOk;
		}
	}
}

/// <summary>Retrieve the image from the physical device camera specified by the parameter.</summary>
/// <param name="deviceIndex">Physical camera index to retrieve information from.</param>
bool CameraManager::readCameraFrame(uint32_t deviceIndex)
{
	struct v4l2_buffer buf;
	memset(&buf, 0, sizeof(buf));

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if (xioctl(_vectorVideoDeviceFileDescriptor[deviceIndex], VIDIOC_DQBUF, &buf) == -1)
	{
		switch (errno)
		{
		case EAGAIN: {
			Log("ERROR: OpenGLSCSurroundView::readCameraFrame EAGAIN error");
			return false;
		}
		case EIO: {
			Log("ERROR: OpenGLSCSurroundView::readCameraFrame EIO error");
		}
		default: {
			Log("ERROR: OpenGLSCSurroundView::readCameraFrame VIDIOC_DQBUF error");
			return false;
		}
		}
	}

	assert(buf.index < _vectorNumBufferPerCamera[deviceIndex]);

	// We now know which buffer got updated and can present its relevant texture to the application
	// this->textures[deviceIndex] = this->devices[deviceIndex].buffers[buf.index].texture;
	// Update the index of the teture that was updated (there can be more than one) to be used for display
	_vectorBufferUpdateIndex[deviceIndex] = buf.index;

	if (xioctl(_vectorVideoDeviceFileDescriptor[deviceIndex], VIDIOC_QBUF, &buf) == -1)
	{
		Log("ERROR: OpenGLSCSurroundView::readCameraFrame qbufResult=-1");
		return false;
	}

	return true;
}
#endif

class ModelResources
{
public:
	/// <summary>Index amount of the model 3D meshes.</summary>
	std::vector<uint32_t> _vectorIndexNumber;

	/// <summary>Index type for the model 3D meshes.</summary>
	std::vector<GLenum> _vectorIndexType;

	/// <summary>Vertex array object for the model 3D meshes.</summary>
	std::vector<GLuint> _vectorVAO;

	/// <summary>Vector with the vertex buffer object of each mesh of the 3D model.</summary>
	std::vector<GLuint> _vectorVBO;

	/// <summary>Vector with the index buffer object of each mesh of the 3D model.</summary>
	std::vector<GLuint> _vectorIBO;

	/// <summary>3D model.</summary>
	pvr::assets::ModelHandle _model;

	~ModelResources()
	{
		gl::DeleteBuffers(static_cast<GLsizei>(_vectorVBO.size()), _vectorVBO.data());
		gl::DeleteBuffers(static_cast<GLsizei>(_vectorIBO.size()), _vectorIBO.data());
		gl::DeleteVertexArrays(static_cast<GLsizei>(_vectorVAO.size()), _vectorVAO.data());
	}
};

/// <summary>Class implementing the pvr::Shell functions.</summary>
class OpenGLSCSurroundView : public pvr::Shell
{
	struct DeviceResources
	{
		/// <summary>EGL context.</summary>
		pvr::EglContext context;

		/// <summary>Class encapsulating the resources used for the quadrant 3D mesh.</summary>
		ModelResources quadrantResources;

		/// <summary>Class encapsulating the resources used for the 3D meshes conforimng the car 3D model.</summary>
		ModelResources carResources;

		/// <summary>Model with the camera setup (where each camera is located spatially).</summary>
		pvr::assets::ModelHandle cameraRig;

		/// <summary>Quadrant model for the surround drawing, used to generate the final vertex and index information which has camera weights encoded in the color vertex information.</summary>
		pvr::utils::ModelGles surroundQuadrantGLModel;

		/// <summary>Shader used to draw the surround view.</summary>
		GLuint surroundShader = static_cast<GLuint>(-1);

		/// <summary>Shader used to draw the car.</summary>
		GLuint carShader = static_cast<GLuint>(-1);

		/// <summary>Shader used to manually draw the mip map levels of the surround view texture streamed from camera.</summary>
		GLuint mipmapShader = static_cast<GLuint>(-1);

		/// <summary>Sampler used for the camera images when drawing the surround quadrants.</summary>
		GLuint surroundCameraSampler = static_cast<GLuint>(-1);

		/// <summary>Uniform buffer object ID with camera information.</summary>
		GLuint uboGlobal = static_cast<GLuint>(-1);

		/// <summary>Flag to know whether to destroy the resources allocated when using physical cameras using zero-copy.</summary>
		bool destroyMMapCameraResources = false;

		/// <summary>Variable to hold information for the texture IDs used for the zero copy method to obtain the image from the camera in a GLES texture.</summary>
		std::vector<MMapCameraResources> arrayMMapCameraResources;

		/// <summary>Flag to know whether to destroy the resources allocated when using physical cameras using buffer copy.</summary>
		bool destroyCameraResources = false;

		/// <summary>Vector to store the IDs of the textures used by the different methods implemented in this sample for drawing the surround view (static environment, physical camera image capturing with copy or zero-copy).</summary>
		std::vector<GLuint> vectorSurroundTextureID;

		/// <summary>ID of the albedo texture used for the car 3D model.</summary>
		GLuint albedoTextureID = static_cast<GLuint>(-1);

		/// <summary>ID of the normal texture used for the car 3D model.</summary>
		GLuint normalTextureID = static_cast<GLuint>(-1);

		/// <summary>ID of the PBR texture used for the car 3D model.</summary>
		GLuint roughnessMetallicTextureID = static_cast<GLuint>(-1);

		/// <summary>ID of the cubemap texture used to draw the surround images (either static or dynamic).</summary>
		GLuint cubemapTextureID = static_cast<GLuint>(-1);

		/// <summary>ID of the framebuffer used for the pass where the the surround images (either static or dynamic) are drawn to a cubemap.</summary>
		GLuint framebufferID = static_cast<GLuint>(-1);

		/// <summary>Vector of vectors where to store the IDs of each framebuffer built to generate surround view camera image mipmaps manually for performance reasons, when _useManualMipMapGeneration is true.</summary>
		std::vector<std::vector<GLuint>> vectorVectorFramebufferDownsampleID;

		/// <summary>ID of the depth render buffer used for the pass where the the surround images (either static or dynamic) are drawn to a cubemap.</summary>
		GLuint depthRenderbufferID = static_cast<GLuint>(-1);

		/// <summary>ID of the texture used for PBR material rendering for the car in case the _useHighQualityMaterials is true.</summary>
		GLuint lookupTablePBRTextureID = static_cast<GLuint>(-1);

		/// <summary>Default destructor.</summary>
		~DeviceResources()
		{
			// Note: Delete OpenGL object operation will not work under a safety critical environment
			if (surroundShader != static_cast<GLuint>(-1)) { gl::DeleteProgram(surroundShader); }
			if (carShader != static_cast<GLuint>(-1)) { gl::DeleteProgram(carShader); }
			if (mipmapShader != static_cast<GLuint>(-1)) { gl::DeleteProgram(mipmapShader); }
			if (surroundCameraSampler != static_cast<GLuint>(-1)) { gl::DeleteSamplers(1, &surroundCameraSampler); }

			if (uboGlobal != static_cast<GLuint>(-1)) { gl::DeleteBuffers(1, &uboGlobal); }

			if (destroyCameraResources)
			{
				for (size_t i = 0; i < vectorSurroundTextureID.size(); ++i) { gl::DeleteTextures(static_cast<GLsizei>(1), &vectorSurroundTextureID[i]); }
			}

			if (destroyMMapCameraResources)
			{
				for (size_t i = 0; i < arrayMMapCameraResources.size(); ++i)
				{
					for (size_t j = 0; j < arrayMMapCameraResources[i].vectorTexture.size(); ++j)
					{
						gl::DeleteTextures(static_cast<GLsizei>(1), &arrayMMapCameraResources[i].vectorTexture[j]);
						egl::ext::DestroyImageKHR(context->getNativePlatformHandles().display, arrayMMapCameraResources[i].vectorImage[j]);
					}
				}
			}

			for (size_t i = 0; i < vectorVectorFramebufferDownsampleID.size(); ++i)
			{
				for (size_t j = 0; j < vectorVectorFramebufferDownsampleID[i].size(); ++j) { gl::DeleteFramebuffers(1, &vectorVectorFramebufferDownsampleID[i][j]); }
			}

			if (albedoTextureID != static_cast<GLuint>(-1)) { gl::DeleteTextures(1, &albedoTextureID); }
			if (normalTextureID != static_cast<GLuint>(-1)) { gl::DeleteTextures(1, &normalTextureID); }
			if (roughnessMetallicTextureID != static_cast<GLuint>(-1)) { gl::DeleteTextures(1, &roughnessMetallicTextureID); }
			if (cubemapTextureID != static_cast<GLuint>(-1)) { gl::DeleteTextures(1, &cubemapTextureID); }
			if (framebufferID != static_cast<GLuint>(-1)) { gl::DeleteFramebuffers(1, &framebufferID); }
			if (depthRenderbufferID != static_cast<GLuint>(-1)) { gl::DeleteRenderbuffers(1, &depthRenderbufferID); }
			if (lookupTablePBRTextureID != static_cast<GLuint>(-1)) { gl::DeleteTextures(1, &lookupTablePBRTextureID); }

			context.reset();
		}
	};

	/// <summary>Array with the information and parameters for each camera used in the surround view.</summary>
	std::vector<SurroundCamera> _arraySurroundCameraInfo;

	/// <summary>Array with the information and parameters for each mesh used in the surround view drawing pass.</summary>
	GLuint _surroundUniformID[static_cast<int>(EnumSurroundUniformID::maxIndex)] = {};

	/// <summary>Array with the information and parameters for each mesh used in car drawing pass.</summary>
	GLuint _carUniformID[static_cast<int>(EnumCarUniformID::maxIndex)] = {};

	/// <summary>Array with the names of the four cameras used for the case of a static environment where each quadrant 3D mesh will use static images.</summary>
	std::vector<std::string> _arrayCameraName = { "Front", "Right", "Back", "Left" };

	/// <summary>Uniform buffer object abstraction for populating values from C++ side, containing camera information.</summary>
	pvr::utils::StructuredBufferView _uboView;

	/// <summary>Struct containing all OpenGL ES objects.</summary>
	std::unique_ptr<DeviceResources> _deviceResources;

	/// <summary>Flag to know whether to use real cameras (only for Linux environments) or static images for the surround.</summary>
	bool _useCameraStreaming = false;

	/// <summary>Flag to know whether to draw a cubemap with the surround view images (either static or dynamic).</summary>
	bool _drawCubemap = true;

	/// <summary>Flag to know whether the first frame has been rendered.</summary>
	bool _firstFrame = true;

	/// <summary>Flag to use higher quaility materials for the car rendering, similar to PBR.</summary>
	bool _useHighQualityMaterials = true;

	/// <summary>Number of cameras used by the sample, depending on the value of _useCameraStreaming it is initialized to the amount of physical device cameras used or to the four cameras when using static images.</summary>
	uint32_t _numberCamera = static_cast<uint32_t>(-1);

	/// <summary>Resolution of the physical device camreas used.</summary>
	glm::ivec2 _cameraResolution;

	/// <summary>Flag to know whether to load astc or ovr versions of the textures used in this demo.</summary>
	bool _astcSupported = false;

	/// <summary>Flag to know whether to do a manual mip map generation of the camera images instead of calling gl::generateMipMaps (for performance reasons).</summary>
	bool _useManualMipMapGeneration = false;

	/// <summary>Projection matrix used for the scene rendering passes.</summary>
	glm::mat4 _projectionMatrix;

	/// <summary>Transform used by the car meshes.</summary>
	glm::mat4 _carTransformMatrix;

	/// <summary>Size of each one of the six textures in the cubempa in case a environment map is used for rendering the car model (when _drawCubemap is true).</summary>
	GLuint _cubemapTextureSize = 256;

	/// <summary>Look at positions for each one of the cubemaps when building the cubemap for the car rendering (when _drawCubemap is true).</summary>
	glm::vec3 _cubemapTargetVectors[numCubemapFaces] = { glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f) };

	/// <summary>Up vectors for each one of the cubemaps when building the cubemap for the car rendering (when _drawCubemap is true).</summary>
	glm::vec3 _cubemapUpVectors[numCubemapFaces] = { glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f) };

	/// <summary>Helper variable for when manual calibration of the camera is done, to select one of the four cameras.</summary>
	uint32_t _currentCameraTransformIndex = 0;

	/// <summary>Helper variable for when manual calibration of the camera is done, to move the camera up / down.</summary>
	float _cameraLookAtHeight = 1.0f;

#ifdef __linux__
	CameraManager _cameraManager;
#endif

public:
	void processCommandLineParameters();
	void initSurroundShaderAndTextures();
	void load3DMeshes();
	void loadTextures();
	void loadSurroundCameraInformation();
	void intializeUBO();
	void initializeCubemap();
	void generateSurroundMipMap();
	void drawSurroundMipMap(int cameraIndex);
	void drawEnvironmentCubemap();
	void renderSurroundMeshes();

#ifdef __linux__
	void initTexturesForCameras();
	void updateCameraTextures();
#endif

	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();
	void editSurroundTransforms();
};

/// <summary>Helper function to generate vertex array objects for the car meshes in the sample.</summary>
/// <param name="mesh">Mesh to generate the VAO for.</param>
/// <param name="vertexBindingsName">Vertex specification for the mesh.</param>
/// <param name="numVertexBindings">Number of elements in vertexBindingsName.</param>
/// <param name="vao">ID of the VAO generated.</param>
/// <param name="vbo">ID of the Vertex Buffer Object generated.</param>
/// <param name="ibo">ID of the Index Buffer Object generated.</param>
void bindVertexSpecification(
	const pvr::assets::Mesh& mesh, const pvr::utils::VertexBindings_Name* const vertexBindingsName, const uint32_t numVertexBindings, GLuint& vao, GLuint& vbo, GLuint& ibo)
{
	pvr::utils::VertexConfiguration vertexConfiguration = pvr::utils::createInputAssemblyFromMesh(mesh, vertexBindingsName, (uint16_t)numVertexBindings);

	gl::GenVertexArrays(1, &vao);
	gl::BindVertexArray(vao);
	gl::BindVertexBuffer(0, vbo, 0, mesh.getStride(0));
	gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	for (auto it = vertexConfiguration.attributes.begin(), end = vertexConfiguration.attributes.end(); it != end; ++it)
	{
		gl::EnableVertexAttribArray(it->index);
		gl::VertexAttribBinding(it->index, 0);
		gl::VertexAttribFormat(it->index, it->width, pvr::utils::convertToGles(it->format), pvr::dataTypeIsNormalised(it->format), static_cast<intptr_t>(it->offsetInBytes));
	}

	gl::BindVertexArray(0);
}

/// <summary>Generate the 3D mesh used a as quadrant to build the environment with the surround view, requires to encode in the color vertex information about camera weights for
/// correct image mixing. Load the meshes conforming the car 3D model.</summary>
void OpenGLSCSurroundView::load3DMeshes()
{
	debugThrowOnApiError("OpenGLSCSurroundView::load3DMeshes error");

	// The quadrantModel information for the quadrant used for build the environment map is the mesh #0 in _deviceResources->quadrantModel
	const pvr::utils::VertexBindings_Name vertexBindings[] = { { "POSITION", "inVertex" }, { "NORMAL", "inColor" }, { "TEXCOORD_0", "inTexCoord" } };
	_deviceResources->quadrantResources._model = pvr::assets::loadModel(*this, QuadrantModelFileName);

	pvr::utils::VertexStreamDescription vertexFormat;
	vertexFormat.Add(0, pvr::DataType::Float32, 3, "inVertex", pvr::utils::VertexStreamDescription::POSITION);
	vertexFormat.Add(0, pvr::DataType::UInt16, 4, "inColor", pvr::utils::VertexStreamDescription::COLOR);
	vertexFormat.Add(0, pvr::DataType::Float32, 2, "inTexCoord", pvr::utils::VertexStreamDescription::UV0);

	pvr::utils::convertMeshesData(vertexFormat, _deviceResources->quadrantResources._model->beginMeshes(), _deviceResources->quadrantResources._model->endMeshes());
	_deviceResources->surroundQuadrantGLModel.init(*this, _deviceResources->quadrantResources._model, pvr::utils::ModelGles::Flags::LoadMeshes);

	const pvr::assets::Mesh& mesh = _deviceResources->quadrantResources._model->getMesh(0);
	_deviceResources->quadrantResources._vectorIndexNumber.push_back(mesh.getNumFaces() * 3);
	_deviceResources->quadrantResources._vectorIndexType.push_back(pvr::utils::convertToGles(mesh.getFaces().getDataType()));
	pvr::utils::appendSingleBuffersFromModel(*_deviceResources->quadrantResources._model, _deviceResources->quadrantResources._vectorVBO, _deviceResources->quadrantResources._vectorIBO);
	pvr::utils::VertexConfiguration vertexConfiguration =
		createInputAssemblyFromMesh(_deviceResources->quadrantResources._model->getMesh(0), vertexBindings, ARRAY_SIZE(vertexBindings));

	// Generate the vertex array object
	_deviceResources->quadrantResources._vectorVAO.resize(1);
	gl::GenVertexArrays(1, &_deviceResources->quadrantResources._vectorVAO[0]);
	gl::BindVertexArray(_deviceResources->quadrantResources._vectorVAO[0]);
	gl::BindBuffer(GL_ARRAY_BUFFER, _deviceResources->surroundQuadrantGLModel.getVboByMeshId(0, 0));
	if (_deviceResources->surroundQuadrantGLModel.getIboByMeshId(0) > 0) { gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, _deviceResources->surroundQuadrantGLModel.getIboByMeshId(0)); }

	auto& binding = vertexFormat.GetVertexConfig().bindings[0];
	for (uint32_t i = 0; i < vertexFormat.GetVertexConfig().attributes.size(); ++i)
	{
		gl::EnableVertexAttribArray(i);
		const auto& attrib = vertexFormat.GetVertexConfig().attributes[i];
		gl::VertexAttribPointer(attrib.index, attrib.width, pvr::utils::convertToGles(attrib.format), pvr::dataTypeIsNormalised(attrib.format), binding.strideInBytes,
			reinterpret_cast<const void*>(static_cast<uintptr_t>(attrib.offsetInBytes)));
	}

	gl::BindVertexArray(0);

	_deviceResources->carResources._model = pvr::assets::loadModel(*this, carModelFileName);
	pvr::utils::appendSingleBuffersFromModel(*_deviceResources->carResources._model, _deviceResources->carResources._vectorVBO, _deviceResources->carResources._vectorIBO);
	const pvr::utils::VertexBindings_Name vertexBindingsCar[] = { { "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoords" } };
	uint32_t numMeshes = _deviceResources->carResources._model->getNumMeshes();
	_deviceResources->carResources._vectorVAO.resize(numMeshes);

	for (uint32_t i = 0; i < numMeshes; ++i)
	{
		const pvr::assets::Mesh& carMesh = _deviceResources->carResources._model->getMesh(i);
		_deviceResources->carResources._vectorIndexNumber.push_back(carMesh.getNumFaces() * 3);
		_deviceResources->carResources._vectorIndexType.push_back(pvr::utils::convertToGles(carMesh.getFaces().getDataType()));
		bindVertexSpecification(carMesh, vertexBindingsCar, 3, _deviceResources->carResources._vectorVAO[i], _deviceResources->carResources._vectorVBO[i],
			_deviceResources->carResources._vectorIBO[i]);
	}

	gl::BindVertexArray(0);
}

/// <summary>Load all the textures needed (surround ones if static surround is being used, and car model textures).</summary>
void OpenGLSCSurroundView::loadTextures()
{
	// Load the textures used for the car
	std::string textureName = carAlbedoTextureFileName;
	pvr::assets::helper::getTextureNameWithExtension(textureName, _astcSupported);
	_deviceResources->albedoTextureID = pvr::utils::textureUpload(*this, textureName.c_str());

	textureName = carNormalTextureFileName;
	pvr::assets::helper::getTextureNameWithExtension(textureName, _astcSupported);
	_deviceResources->normalTextureID = pvr::utils::textureUpload(*this, textureName.c_str());

	textureName = carRoughnessMetallicTextureFileName;
	pvr::assets::helper::getTextureNameWithExtension(textureName, _astcSupported);
	_deviceResources->roughnessMetallicTextureID = pvr::utils::textureUpload(*this, textureName.c_str());

	std::vector<GLuint> vectorTextureID = { _deviceResources->albedoTextureID, _deviceResources->normalTextureID, _deviceResources->roughnessMetallicTextureID };

	// If not camera streaming is being used, then load static images to apply for the environment
	if (!_useCameraStreaming)
	{
		size_t numberDevice = vectorCameraDeviceName.size();
		_deviceResources->vectorSurroundTextureID.resize(numberDevice);

		for (size_t i = 0; i < numberDevice; i++)
		{
			_deviceResources->vectorSurroundTextureID[i] = pvr::utils::textureUpload(*this, "Car" + _arrayCameraName[i] + (_astcSupported ? "_astc.pvr" : ".pvr"));
			vectorTextureID.push_back(_deviceResources->vectorSurroundTextureID[i]);
		}

		_deviceResources->destroyCameraResources = true;
	}

	if (_useHighQualityMaterials) { _deviceResources->lookupTablePBRTextureID = pvr::utils::textureUpload(*this, lookupTablePBRTextureFileName); }

	for (size_t i = 0; i < vectorTextureID.size(); i++)
	{
		gl::BindTexture(GL_TEXTURE_2D, vectorTextureID[i]);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	gl::BindTexture(GL_TEXTURE_2D, 0);
}

bool isPowerOfTwo(uint32_t powerOf2)
{
	if (powerOf2 <= 0) return false;

	return (powerOf2 & (powerOf2 - 1)) == 0;
}

/// <summary>Helper function used wen calibrating manually the camera images used in the surround 3D meshes to match each other.</summary>
void OpenGLSCSurroundView::processCommandLineParameters()
{
	const pvr::CommandLine& commandOptions = getCommandLine();

	bool avoidCubemapParameter = false;
	commandOptions.getBoolOptionSetTrueIfPresent("-avoidCubemap", avoidCubemapParameter);
	if (avoidCubemapParameter)
	{
		_drawCubemap = false;
		Log("COMMAND LINE PARAMETER: Cubemap drawing deactivated through command line");
	}

	int cubemapTextureSizeParameter = 0;
	commandOptions.getIntOption("-cubemapTextureSize", cubemapTextureSizeParameter);

	if (cubemapTextureSizeParameter > 0)
	{
		uint32_t cubemapResolution = static_cast<uint32_t>(cubemapTextureSizeParameter);

		// Verify the command line parameter is a power of two
		if ((cubemapResolution > 0) && ((cubemapResolution & (cubemapResolution - 1)) == 0))
		{
			_cubemapTextureSize = cubemapResolution;
			Log("COMMAND LINE PARAMETER: Cubemap resolution changed to %d", _cubemapTextureSize);
		}
		else
		{
			Log("COMMAND LINE PARAMETER: Cubemap resolution has to be a power of two, provided parameter value is %d", cubemapTextureSizeParameter);
		}
	}

#ifdef __linux__
	if (commandOptions.hasOption("-useCameraStreaming"))
	{
		_useCameraStreaming = true;
		Log("COMMAND LINE PARAMETER: Use camera streaming option activated through command line");
	}

	if (commandOptions.hasOption("-useManualMipMapGeneration"))
	{
		_useManualMipMapGeneration = true;
		Log("COMMAND LINE PARAMETER: Do manual mip map generation for the camera images");
	}
#endif

	if (_useCameraStreaming)
	{
		int cameraResolutionWidthParameter = 0;
		int cameraResolutionHeightParameter = 0;
		commandOptions.getIntOption("-cameraResolutionWidth", cameraResolutionWidthParameter);
		commandOptions.getIntOption("-cameraResolutionHeight", cameraResolutionHeightParameter);

		if ((cameraResolutionWidthParameter > 0) && (cameraResolutionHeightParameter > 0))
		{
			_cameraResolution = glm::ivec2(cameraResolutionWidthParameter, cameraResolutionHeightParameter);
			Log("COMMAND LINE PARAMETER: Command line cameraResolutionWidth and cameraResolutionHeight specified (%d, %d)", cameraResolutionWidthParameter,
				cameraResolutionHeightParameter);
		}
		else
		{
			_cameraResolution = glm::ivec2(1280, 1080);
			Log("COMMAND LINE PARAMETER: Command line cameraResolutionWidth and cameraResolutionHeight not specified or <= 0, using internal values (1280, 1080)");
		}

		_numberCamera = static_cast<uint32_t>(vectorCameraDeviceName.size());
		_deviceResources->vectorSurroundTextureID.resize(_numberCamera);
	}
	else
	{
		_numberCamera = 4;
		// If not camera streaming being used, the sample will use the static images provided in
		// examples/assets/Surroundview wich are captures from the cameras
		_cameraResolution = glm::ivec2(1280, 1080);
	}
}

/// <summary>Initialise the shader used by the surround view and the static texttures used to build a static surround view.</summary>
void OpenGLSCSurroundView::initSurroundShaderAndTextures()
{
	std::vector<const char*> defines;
	uint32_t numDefines = 0;

	if (_drawCubemap && !_useHighQualityMaterials)
	{
		defines.push_back("LOW_QUALITY_MATERIALS");
		numDefines++;
	}

	if (_drawCubemap && _useHighQualityMaterials)
	{
		_drawCubemap = true;
		defines.push_back("HIGH_QUALITY_MATERIAL");
		numDefines++;
	}

	_deviceResources->surroundShader =
		pvr::utils::createShaderProgram(*this, SurroundVertShaderFileName, SurroundFragShaderFileName, nullptr, nullptr, 0, defines.data(), static_cast<uint32_t>(defines.size()));

	gl::UseProgram(_deviceResources->surroundShader);
	debugThrowOnApiError("OpenGLSCSurroundView::initSurroundShaderAndTextures: Use surround shader error");

	_surroundUniformID[static_cast<int>(EnumSurroundUniformID::camera0)] = gl::GetUniformLocation(_deviceResources->surroundShader, "uCameraID0");
	_surroundUniformID[static_cast<int>(EnumSurroundUniformID::camera1)] = gl::GetUniformLocation(_deviceResources->surroundShader, "uCameraID1");
	_surroundUniformID[static_cast<int>(EnumSurroundUniformID::cameraTexture0)] = gl::GetUniformLocation(_deviceResources->surroundShader, "sCamera0");
	_surroundUniformID[static_cast<int>(EnumSurroundUniformID::cameraTexture1)] = gl::GetUniformLocation(_deviceResources->surroundShader, "sCamera1");
	_surroundUniformID[static_cast<int>(EnumSurroundUniformID::viewProjectionMatrix)] = gl::GetUniformLocation(_deviceResources->surroundShader, "uViewProjection");
	_surroundUniformID[static_cast<int>(EnumSurroundUniformID::worldTransform)] = gl::GetUniformLocation(_deviceResources->surroundShader, "uWorldTransform");
	debugThrowOnApiError("OpenGLSCSurroundView::initSurroundShaderAndTextures: Getting uniform location error");

	gl::GenSamplers(1, &_deviceResources->surroundCameraSampler);
	gl::SamplerParameteri(_deviceResources->surroundCameraSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gl::SamplerParameteri(_deviceResources->surroundCameraSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::SamplerParameteri(_deviceResources->surroundCameraSampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->surroundCameraSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::SamplerParameteri(_deviceResources->surroundCameraSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	gl::UseProgram(0);

	_deviceResources->carShader =
		pvr::utils::createShaderProgram(*this, CarVertShaderFileName, CarFragShaderFileName, nullptr, nullptr, 0, defines.data(), static_cast<uint32_t>(defines.size()));
	gl::UseProgram(_deviceResources->carShader);
	debugThrowOnApiError("OpenGLSCSurroundView::initSurroundShaderAndTextures: Use car shader error");

	_carUniformID[static_cast<int>(EnumCarUniformID::worldTransform)] = gl::GetUniformLocation(_deviceResources->carShader, "uWorldTransform");
	_carUniformID[static_cast<int>(EnumCarUniformID::viewProjectionMatrix)] = gl::GetUniformLocation(_deviceResources->carShader, "uViewProjection");
	_carUniformID[static_cast<int>(EnumCarUniformID::cameraPosition)] = gl::GetUniformLocation(_deviceResources->carShader, "uCameraPosition");
	_carUniformID[static_cast<int>(EnumCarUniformID::numEnvironmentMipMap)] = gl::GetUniformLocation(_deviceResources->carShader, "uNumEnvironmentMipMap");
	debugThrowOnApiError("OpenGLSCSurroundView::initSurroundShaderAndTextures: Getting uniform location error");

	gl::UniformMatrix4fv(_carUniformID[static_cast<int>(EnumCarUniformID::worldTransform)], 1, GL_FALSE, glm::value_ptr(_carTransformMatrix));

	int numMipMapLevels = log2(_cubemapTextureSize);
	gl::Uniform1i(_carUniformID[static_cast<int>(EnumCarUniformID::numEnvironmentMipMap)], numMipMapLevels);

	gl::UseProgram(0);
}

/// <summary>Load the camera information for the surround view (camera transform, physical camera dvice settings).</summary>
void OpenGLSCSurroundView::loadSurroundCameraInformation()
{
	_arraySurroundCameraInfo.resize(_numberCamera);

	// Each camera has its own trasnform when projecting the image onto its corresponding surround view 3D mesh (the surround view 3D mesh is not affected by this transform)
	_arraySurroundCameraInfo[0].transform = glm::mat4(
		-0.999120, -0.000576, 0.041877, 0.000000, -0.000484, 0.999976, 0.002206, 0.000000, -0.041878, 0.002184, -0.999095, 0.000000, -0.064729, -1.620975, 0.252438, 1.000000);
	_arraySurroundCameraInfo[1].transform = glm::mat4(
		0.001832, -0.427339, 0.904048, 0.000000, -0.000728, 0.904051, 0.427341, 0.000000, -0.999997, -0.001441, 0.001345, 0.000000, -0.630197, -3.795101, -2.598148, 1.000000);
	_arraySurroundCameraInfo[2].transform =
		glm::mat4(0.983363, -0.008812, -0.181383, 0.000000, 0.022894, 0.996849, 0.075690, 0.000000, 0.180144, -0.078585, 0.980468, 0.000000, 0.086125, -2.785456, 0.263816, 1.000000);
	_arraySurroundCameraInfo[3].transform =
		glm::mat4(0.195687, 0.168009, -0.966098, 0.000000, -0.044922, 0.985677, 0.162299, 0.000000, 0.979595, 0.011639, 0.200432, 0.000000, 0.855910, -2.534036, -3.137377, 1.000000);

	// Setup the surround view camera information and load static camera textures
	for (uint32_t i = 0; i < _numberCamera; i++)
	{
		auto& cam = _arraySurroundCameraInfo[i];
		cam.name = _arrayCameraName[i];
		cam.barrelDistortion = glm::vec3(-0.3195085163816964f, 0.08499829326044542f, -0.008842254974808755f);
		cam.tangentialDistortion = glm::vec2(-0.0026617595738698385f, -0.0014907257998599947f);
		cam.sensorCentre = glm::vec2(494.944883277257f, 498.7984019931958f);
		cam.sensorSize = glm::vec2(387.80803649905687f, 387.077024182395f);
	}
}

/// <summary>Initialize the Uniform Buffer Object where camera information is stored (transform, physical device details).</summary>
void OpenGLSCSurroundView::intializeUBO()
{
	debugThrowOnApiError("OpenGLSCSurroundView::intializeUBO error");

	pvr::utils::StructuredMemoryDescription viewDesc;
	viewDesc.addElement("ViewMatrix", pvr::GpuDatatypes::mat4x4, 4);
	viewDesc.addElement("K", pvr::GpuDatatypes::vec3, 4);
	viewDesc.addElement("P", pvr::GpuDatatypes::vec2, 4);
	viewDesc.addElement("sensorSize", pvr::GpuDatatypes::vec2, 4);
	viewDesc.addElement("sensorCentre", pvr::GpuDatatypes::vec2, 4);
	viewDesc.addElement("cameraImageResolution", pvr::GpuDatatypes::vec2, 1);
	_uboView.init(viewDesc);

	gl::GenBuffers(1, &_deviceResources->uboGlobal);
	gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->uboGlobal);
	gl::BufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(_uboView.getSize()), nullptr, GL_STATIC_DRAW);
	debugThrowOnApiError("OpenGLSCSurroundView::intializeUBO: Error allocating memory for the buffer");

	void* uboData = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, (GLsizeiptr)_uboView.getSize(), GL_MAP_WRITE_BIT);
	debugThrowOnApiError("OpenGLSCSurroundView::intializeUBO: Error mapping memory for the buffer");

	_uboView.pointToMappedMemory(uboData);

	for (uint32_t i = 0; i < _numberCamera; i++)
	{
		auto& cam = _arraySurroundCameraInfo[i];
		_uboView.getElement(0, i).setValue(cam.transform);
		_uboView.getElement(1, i).setValue(cam.barrelDistortion);
		_uboView.getElement(2, i).setValue(cam.tangentialDistortion);
		_uboView.getElement(3, i).setValue(cam.sensorSize);
		_uboView.getElement(4, i).setValue(cam.sensorCentre);
	}

	_uboView.getElement(5, 0).setValue(glm::vec2(_cameraResolution));

	gl::UnmapBuffer(GL_UNIFORM_BUFFER);
	debugThrowOnApiError("OpenGLSCSurroundView::intializeUBO: Error unmapping memory from the buffer");
}

/// <summary>Initialize the resources needed to draw the surround meshes to a cubemap, used to draw the car with improved quality.</summary>
void OpenGLSCSurroundView::initializeCubemap()
{
	debugThrowOnApiError("OpenGLSCSurroundView::initializeCubemap Error");

	gl::GenTextures(1, &_deviceResources->cubemapTextureID);
	gl::BindTexture(GL_TEXTURE_CUBE_MAP, _deviceResources->cubemapTextureID);
	gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	gl::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (GLuint i = 0; i < numCubemapFaces; ++i)
	{
		gl::TexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, _cubemapTextureSize, _cubemapTextureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	gl::GenerateMipmap(GL_TEXTURE_CUBE_MAP);

	gl::GenFramebuffers(1, &_deviceResources->framebufferID);
	gl::BindFramebuffer(GL_FRAMEBUFFER, _deviceResources->framebufferID);
	gl::GenRenderbuffers(1, &_deviceResources->depthRenderbufferID);
	gl::BindRenderbuffer(GL_RENDERBUFFER, _deviceResources->depthRenderbufferID);
	gl::RenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _cubemapTextureSize, _cubemapTextureSize);
	gl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, _deviceResources->cubemapTextureID, 0);
	gl::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _deviceResources->depthRenderbufferID);

	// Check if current configuration of framebuffer is correct
	if (gl::CheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { Log("ERROR: Framebuffer not complete"); }

	// Set default framebuffer
	gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
}

/// <summary>Generate the resources needed for maunally generating the mip map levels for each one of the textures streamed from the camera for correct surround view rendering.</summary>
void OpenGLSCSurroundView::generateSurroundMipMap()
{
	_deviceResources->vectorVectorFramebufferDownsampleID.resize(_numberCamera);

	std::vector<const char*> defines;

	_deviceResources->mipmapShader = pvr::utils::createShaderProgram(
		*this, mipmapPostProcessVertShaderFileName, mipmapPostProcessFragShaderFileName, nullptr, nullptr, 0, defines.data(), static_cast<uint32_t>(defines.size()));

	int numMipMapLevels = std::max(log2(_cameraResolution.x), log2(_cameraResolution.y));

	for (uint32_t i = 0; i < _numberCamera; ++i)
	{
		_deviceResources->vectorVectorFramebufferDownsampleID[i].resize(numMipMapLevels - 1);

		for (int j = 0; j < numMipMapLevels - 1; ++j)
		{
			gl::GenFramebuffers(1, &_deviceResources->vectorVectorFramebufferDownsampleID[i][j]);
			gl::BindFramebuffer(GL_FRAMEBUFFER, _deviceResources->vectorVectorFramebufferDownsampleID[i][j]);
			gl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _deviceResources->vectorSurroundTextureID[i], j + 1);
			gl::FramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, _cameraResolution.x / 2);
			gl::FramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, _cameraResolution.y / 2);
			if (gl::CheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { Log("ERROR: Framebuffer not complete"); }
			gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}
}

/// <summary>Draw each one of the mip map levels for each one of the camera streamed images by reading from one mip map level of the texture and drawing into the next one using
/// linear sampling.</summary>
/// <param name="cameraIndex">Camera index (value in {0, 1, 2, 3}) to draw the surround mip maps for.</param>
void OpenGLSCSurroundView::drawSurroundMipMap(int cameraIndex)
{
	int numMipMapLevels = std::max(log2(_cameraResolution.x), log2(_cameraResolution.y));
	glm::ivec2 currentResolution = _cameraResolution;

	gl::Disable(GL_CULL_FACE);
	gl::Disable(GL_DEPTH_TEST);
	gl::UseProgram(_deviceResources->mipmapShader);

	for (int i = 0; i < numMipMapLevels - 1; ++i)
	{
		currentResolution /= 2;
		gl::Viewport(0, 0, currentResolution.x, currentResolution.y);
		gl::BindFramebuffer(GL_FRAMEBUFFER, _deviceResources->vectorVectorFramebufferDownsampleID[cameraIndex][i]);

		gl::Clear(GL_COLOR_BUFFER_BIT);

		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindTexture(GL_TEXTURE_2D, _deviceResources->vectorSurroundTextureID[cameraIndex]);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, i);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, i);
		gl::BindSampler(0, _deviceResources->surroundCameraSampler);

		gl::DrawArrays(GL_TRIANGLES, 0, 6);

		gl::BindSampler(0, 0);
		gl::BindTexture(GL_TEXTURE_2D, 0);
		gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	gl::UseProgram(0);

	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->vectorSurroundTextureID[cameraIndex]);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, numMipMapLevels);
	gl::BindTexture(GL_TEXTURE_2D, 0);

	gl::Enable(GL_CULL_FACE);
	gl::Enable(GL_DEPTH_TEST);

	gl::Viewport(0, 0, getWidth(), getHeight());
}

/// <summary>Used to draw to the cubemap the surround meshes for both the static image and streamed camera image cases.</summary>
void OpenGLSCSurroundView::drawEnvironmentCubemap()
{
	debugThrowOnApiError("OpenGLSCSurroundView::drawEnvironmentCubemap: Error");

	gl::Viewport(0, 0, _cubemapTextureSize, _cubemapTextureSize); // Set size of the viewport as size of cube map
	glm::mat4 projectionMatrix =
		pvr::math::perspectiveFov(pvr::Api::OpenGLES31, glm::radians(90.0f), static_cast<float>(_cubemapTextureSize), static_cast<float>(_cubemapTextureSize), 1.0f, 1000.0f);

	gl::BindFramebuffer(GL_FRAMEBUFFER, _deviceResources->framebufferID); // Bind FBO to render to the texture

	for (GLuint face = 0; face < numCubemapFaces; ++face)
	{
		gl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, _deviceResources->cubemapTextureID, 0);
		gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 currentCubeMapView = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f) + _cubemapTargetVectors[face], _cubemapUpVectors[face]);
		gl::UniformMatrix4fv(_surroundUniformID[static_cast<int>(EnumSurroundUniformID::viewProjectionMatrix)], 1, GL_FALSE, glm::value_ptr(projectionMatrix * currentCubeMapView));
		renderSurroundMeshes();
	}

	gl::BindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind FBO, set default framebuffer
}

#ifdef __linux__
/// <summary>Initialize the OpenGLES and EGL textures used to display the images retrieved from the physical cameras.</summary>
void OpenGLSCSurroundView::initTexturesForCameras()
{
	uint32_t cameraNumber = _cameraManager.getCameraNumber();
	_deviceResources->arrayMMapCameraResources.resize(cameraNumber);

	for (uint32_t i = 0; i < cameraNumber; ++i)
	{
		uint32_t numberBuffer = _cameraManager.getNumBufferCamera(i);
		_deviceResources->arrayMMapCameraResources[i].vectorImage.resize(numberBuffer);
		_deviceResources->arrayMMapCameraResources[i].vectorTexture.resize(numberBuffer);

		for (uint32_t j = 0; j < numberBuffer; ++j)
		{
			int fileDescriptor = _cameraManager.getFDForCameraBuffer(i, j);

			// Create the image buffer
			EGLint attrib_list[] = { EGL_WIDTH, _cameraResolution.x, EGL_HEIGHT, _cameraResolution.y, EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_RGB565, EGL_DMA_BUF_PLANE0_FD_EXT,
				fileDescriptor, EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0, EGL_DMA_BUF_PLANE0_PITCH_EXT, _cameraResolution.x * 2, EGL_NONE };

			if (egl::GetError() != EGL_SUCCESS) { Log("ERROR in OpenGLSCSurroundView::allocateCameraMemoryMapResources: EGLError=%d", egl::GetError()); }

			_deviceResources->arrayMMapCameraResources[i].vectorImage[j] =
				egl::ext::CreateImageKHR(_deviceResources->context->getNativePlatformHandles().display, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, (EGLClientBuffer) nullptr, attrib_list);

			if (egl::GetError() != EGL_SUCCESS) { Log("ERROR in OpenGLSCSurroundView::allocateCameraMemoryMapResources: EGLError=%d", egl::GetError()); }

			if (_deviceResources->arrayMMapCameraResources[i].vectorImage[j] == EGL_NO_IMAGE_KHR)
			{
				Log("ERROR in OpenGLSCSurroundView::allocateCameraMemoryMapResources: Could not create EGL image");
			}

			// Create the external texture (backed by the image buffer)
			gl::GenTextures(1, &_deviceResources->arrayMMapCameraResources[i].vectorTexture[j]);
			gl::BindTexture(GL_TEXTURE_2D, _deviceResources->arrayMMapCameraResources[i].vectorTexture[j]);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			if (egl::GetError() != EGL_SUCCESS) { Log("ERROR in OpenGLSCSurroundView::allocateCameraMemoryMapResources: EGLError=%d", egl::GetError()); }

			gl::ext::EGLImageTargetTexture2DOES(GL_TEXTURE_2D, _deviceResources->arrayMMapCameraResources[i].vectorImage[j]);

			if (egl::GetError() != EGL_SUCCESS) { Log("ERROR in OpenGLSCSurroundView::allocateCameraMemoryMapResources: EGLError=%d", egl::GetError()); }

			gl::BindTexture(GL_TEXTURE_2D, 0);
		}
	}

	_deviceResources->destroyMMapCameraResources = true;
}

/// <summary>Update the OpenGLES textures used to display the images retrieved from the physical cameras.</summary>
void OpenGLSCSurroundView::updateCameraTextures()
{
	uint32_t cameraNumber = _cameraManager.getCameraNumber();

	for (uint32_t i = 0; i < cameraNumber; ++i)
	{
		if (_cameraManager.getCameraUpdateStatus(i))
		{
			int cameraUpdateIndex = _cameraManager.getCameraUpdateIndex(i);

			if (_useManualMipMapGeneration) { drawSurroundMipMap(i); }
			else
			{
				gl::BindTexture(GL_TEXTURE_2D, _deviceResources->arrayMMapCameraResources[i].vectorTexture[cameraUpdateIndex]);
				debugThrowOnApiError("OpenGLSCSurroundView::updateCameraTextures: Error binding texture");
				gl::GenerateMipmap(GL_TEXTURE_2D);
				gl::BindTexture(GL_TEXTURE_2D, 0);
			}
		}
	}
}
#endif

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.). If the rendering
/// context is lost, initApplication() will not be called again.</summary>
pvr::Result OpenGLSCSurroundView::initApplication() { return pvr::Result::Success; }

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context(e.g.textures, vertex buffers, etc.)</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLSCSurroundView::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();
	_deviceResources->context = pvr::createEglContext();
	_deviceResources->context->init(getWindow(), getDisplay(), getDisplayAttributes(), pvr::Api::Unspecified);
	_astcSupported = gl::isGlExtensionSupported("GL_KHR_texture_compression_astc_ldr");
	_projectionMatrix = pvr::math::perspectiveFov(pvr::Api::OpenGLES31, glm::radians(65.f), static_cast<float>(getWidth()), static_cast<float>(getHeight()), 0.1f, 2000.f);
	_carTransformMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.015f, 0.015f, 0.015f)) *
		glm::rotate(glm::radians(-90.f), glm::vec3(-1.0f, 0.0f, 0.0f));

	processCommandLineParameters();
	initSurroundShaderAndTextures();
	load3DMeshes();
	loadTextures();
	if (_useManualMipMapGeneration) { generateSurroundMipMap(); }
	loadSurroundCameraInformation();
	intializeUBO();
	if (_drawCubemap) { initializeCubemap(); }

#ifdef __linux__
	if (_useCameraStreaming)
	{
		bool result = _cameraManager.initializeCameras(vectorCameraDeviceName, _cameraResolution);
		result |= _cameraManager.verifyCameraCapabilities();
		result |= _cameraManager.limitCameraFramerate();
		result |= _cameraManager.setupCameraExposure();
		result |= _cameraManager.setupCameraImageProperties();
		result |= _cameraManager.allocateCameraResources();
		result |= _cameraManager.updateCameraExposure();

		initTexturesForCameras();

		_cameraManager.printCameraInformation(0);

		if (!result) { return pvr::Result::UnsupportedRequest; }
	}
#endif

	gl::Viewport(0, 0, getWidth(), getHeight());

	gl::DepthMask(GL_TRUE);
	gl::CullFace(GL_BACK);
	gl::FrontFace(GL_CCW);
	gl::Enable(GL_CULL_FACE);
	gl::Enable(GL_DEPTH_TEST);
	gl::ClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLSCSurroundView::releaseView()
{
#ifdef __linux__
	_cameraManager.stopCapturing();
#endif

	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by Shell once per run, just before exiting the surroundShader.
/// quitApplication() will not be called every time the rendering context is lost, only before application exit.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLSCSurroundView::quitApplication() { return pvr::Result::Success; }

/// <summary>Helper function used to raw the surround meshes for both the static image and streamed camera image cases.</summary>
void OpenGLSCSurroundView::renderSurroundMeshes()
{
	debugThrowOnApiError("OpenGLSCSurroundView::renderSurroundMeshes error");

	if (!_useCameraStreaming)
	{
		gl::BindSampler(0, _deviceResources->surroundCameraSampler);
		gl::BindSampler(1, _deviceResources->surroundCameraSampler);
	}

	gl::Uniform1i(_surroundUniformID[static_cast<int>(EnumSurroundUniformID::cameraTexture0)], 0);
	gl::Uniform1i(_surroundUniformID[static_cast<int>(EnumSurroundUniformID::cameraTexture1)], 1);

	gl::BindVertexArray(_deviceResources->quadrantResources._vectorVAO[0]);

	// Draw each one of the four quadrants which conform the environment
	for (uint32_t i = 0; i < _numberCamera; ++i)
	{
		// Each quadrantResources needs two textures to mix properly the whole surround view
		glm::mat4 rotation = glm::rotate(glm::radians(-90.f) * i, glm::vec3(0.f, 1.f, 0.f));
		gl::UniformMatrix4fv(_surroundUniformID[static_cast<int>(EnumSurroundUniformID::worldTransform)], 1, GL_FALSE, glm::value_ptr(rotation));

		const uint32_t id0 = i;
		const uint32_t id1 = (id0 + 1) % _numberCamera;

		gl::Uniform1i(_surroundUniformID[static_cast<int>(EnumSurroundUniformID::camera0)], id0);
		gl::Uniform1i(_surroundUniformID[static_cast<int>(EnumSurroundUniformID::camera1)], id1);

#ifdef __linux__
		if (_useCameraStreaming)
		{
			_deviceResources->vectorSurroundTextureID[0] = _deviceResources->arrayMMapCameraResources[0].vectorTexture[_cameraManager.getCameraUpdateIndex(0)];
			_deviceResources->vectorSurroundTextureID[1] = _deviceResources->arrayMMapCameraResources[1].vectorTexture[_cameraManager.getCameraUpdateIndex(1)];
			_deviceResources->vectorSurroundTextureID[2] = _deviceResources->arrayMMapCameraResources[2].vectorTexture[_cameraManager.getCameraUpdateIndex(2)];
			_deviceResources->vectorSurroundTextureID[3] = _deviceResources->arrayMMapCameraResources[3].vectorTexture[_cameraManager.getCameraUpdateIndex(3)];

			gl::BindSampler(0, 0);
			gl::BindSampler(1, 0);
		}
#endif

		gl::ActiveTexture(GL_TEXTURE0);
		gl::BindTexture(GL_TEXTURE_2D, _deviceResources->vectorSurroundTextureID[id0]);
		gl::ActiveTexture(GL_TEXTURE1);
		gl::BindTexture(GL_TEXTURE_2D, _deviceResources->vectorSurroundTextureID[id1]);

		gl::DrawElements(GL_TRIANGLES, _deviceResources->quadrantResources._vectorIndexNumber[0], _deviceResources->quadrantResources._vectorIndexType[0], nullptr);
		debugThrowOnApiError("OpenGLSCSurroundView::renderSurroundMeshes: Error when drawing surround view meshes");
	}

	gl::BindSampler(0, 0);
	gl::BindSampler(1, 0);
}

/// <summary>Helper function to process keyboard evnts in the case of manually adjusting the transforms of the images sreamed from camera (or static if no camera streaming is available) to match each other in the surround view 3D mesh render.</summary>
void OpenGLSCSurroundView::editSurroundTransforms()
{
	// A: rotate left
	// D: rotate right
	// W: rotate upwards
	// S: rotate downwards
	// R: translate back
	// Y: translate forward
	// T: translate up
	// G: translate down
	// F: translate left
	// H: translate right
	// Numpad 0: Select camera 0 (Left camera)
	// Numpad 1: Select camera 1 (Front camera)
	// Numpad 2: Select camera 2 (Back camera)
	// Numpad 3: Select camera 3 (Right camera)

	if (this->isKeyPressed(pvr::Keys::Num0))
	{
		_currentCameraTransformIndex = 0;
		Log("Current camera transform index is %d", _currentCameraTransformIndex);
	}

	if (this->isKeyPressed(pvr::Keys::Num1))
	{
		_currentCameraTransformIndex = 1;
		Log("Current camera transform index is %d", _currentCameraTransformIndex);
	}

	if (this->isKeyPressed(pvr::Keys::Num2))
	{
		_currentCameraTransformIndex = 2;
		Log("Current camera transform index is %d", _currentCameraTransformIndex);
	}

	if (this->isKeyPressed(pvr::Keys::Num3))
	{
		_currentCameraTransformIndex = 3;
		Log("Current camera transform index is %d", _currentCameraTransformIndex);
	}

	float rotationDegrees = 0.1f;
	float rotationRadians = 3.14159265358979323846f * (rotationDegrees / 180.0f);

	bool anyChanges = false;

	if (this->isKeyPressed(pvr::Keys::A))
	{
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), rotationRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		_arraySurroundCameraInfo[_currentCameraTransformIndex].transform = rotation * _arraySurroundCameraInfo[_currentCameraTransformIndex].transform;
		anyChanges = true;
	}

	if (this->isKeyPressed(pvr::Keys::D))
	{
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), -1.0f * rotationRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		_arraySurroundCameraInfo[_currentCameraTransformIndex].transform = rotation * _arraySurroundCameraInfo[_currentCameraTransformIndex].transform;
		anyChanges = true;
	}

	if (this->isKeyPressed(pvr::Keys::W))
	{
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), -1.0f * rotationRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		_arraySurroundCameraInfo[_currentCameraTransformIndex].transform = rotation * _arraySurroundCameraInfo[_currentCameraTransformIndex].transform;
		anyChanges = true;
	}

	if (this->isKeyPressed(pvr::Keys::S))
	{
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), rotationRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		_arraySurroundCameraInfo[_currentCameraTransformIndex].transform = rotation * _arraySurroundCameraInfo[_currentCameraTransformIndex].transform;
		anyChanges = true;
	}

	float translateOffset = 0.01f;
	if (this->isKeyPressed(pvr::Keys::T))
	{
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, translateOffset, 0.0f));
		_arraySurroundCameraInfo[_currentCameraTransformIndex].transform = translation * _arraySurroundCameraInfo[_currentCameraTransformIndex].transform;
		anyChanges = true;
	}

	if (this->isKeyPressed(pvr::Keys::G))
	{
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f * translateOffset, 0.0f));
		_arraySurroundCameraInfo[_currentCameraTransformIndex].transform = translation * _arraySurroundCameraInfo[_currentCameraTransformIndex].transform;
		anyChanges = true;
	}

	if (this->isKeyPressed(pvr::Keys::R))
	{
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f * translateOffset));
		_arraySurroundCameraInfo[_currentCameraTransformIndex].transform = translation * _arraySurroundCameraInfo[_currentCameraTransformIndex].transform;
		anyChanges = true;
	}

	if (this->isKeyPressed(pvr::Keys::Y))
	{
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, translateOffset));
		_arraySurroundCameraInfo[_currentCameraTransformIndex].transform = translation * _arraySurroundCameraInfo[_currentCameraTransformIndex].transform;
		anyChanges = true;
	}

	if (this->isKeyPressed(pvr::Keys::F))
	{
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f * translateOffset, 0.0f, 0.0f));
		_arraySurroundCameraInfo[_currentCameraTransformIndex].transform = translation * _arraySurroundCameraInfo[_currentCameraTransformIndex].transform;
		anyChanges = true;
	}

	if (this->isKeyPressed(pvr::Keys::H))
	{
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(translateOffset, 0.0f, 0.0f));
		_arraySurroundCameraInfo[_currentCameraTransformIndex].transform = translation * _arraySurroundCameraInfo[_currentCameraTransformIndex].transform;
		anyChanges = true;
	}

	if (this->isKeyPressed(pvr::Keys::NumAdd)) { _cameraLookAtHeight += 0.01f; }
	if (this->isKeyPressed(pvr::Keys::NumSub)) { _cameraLookAtHeight -= 0.1f; }

	if (anyChanges)
	{
		gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->uboGlobal);
		void* uboData = gl::MapBufferRange(GL_UNIFORM_BUFFER, 0, (GLsizeiptr)_uboView.getSize(), GL_MAP_WRITE_BIT);
		_uboView.pointToMappedMemory(uboData);

		for (uint32_t i = 0; i < _numberCamera; i++) { _uboView.getElement(0, i).setValue(_arraySurroundCameraInfo[i].transform); }

		gl::UnmapBuffer(GL_UNIFORM_BUFFER);
		debugThrowOnApiError("OpenGLSCSurroundView::editSurroundTransforms: Error updating buffer");

		for (uint32_t i = 0; i < _numberCamera; i++)
		{
			glm::mat4 matrix = _arraySurroundCameraInfo[i].transform;
			Log("Matrix %d = (%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f)", i, matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3], matrix[1][0],
				matrix[1][1], matrix[1][2], matrix[1][3], matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3], matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3]);
		}
	}
}

/// <summary>Draw each one of the quadrants to build the surround view, rotating the 3D mesh 90 degrees and setting the corect camera images for sampling.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result OpenGLSCSurroundView::renderFrame()
{
	debugThrowOnApiError("OpenGLSCSurroundView::renderFrame: Begin Frame");

#ifdef __linux__
	if (_useCameraStreaming)
	{
		if (_cameraManager.getFirstFrame())
		{
			_cameraManager.startCapturing();
			_cameraManager.setFirstFrame(false);
		}

		_cameraManager.updateCameraFrame();
		updateCameraTextures();
	}
#endif

	// In case the transforms associated with each camera need to be manually calibrated, use the controls below to select each
	// one of the cameras and rotate / translate its transform editSurroundTransforms();

	gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gl::UseProgram(_deviceResources->surroundShader);

	gl::BindBuffer(GL_UNIFORM_BUFFER, _deviceResources->uboGlobal);
	gl::BindBufferBase(GL_UNIFORM_BUFFER, 0, _deviceResources->uboGlobal);

	if (_drawCubemap)
	{
		if (_useCameraStreaming || _firstFrame)
		{
			drawEnvironmentCubemap();
			gl::Viewport(0, 0, getWidth(), getHeight());
		}

		if (_firstFrame) { _firstFrame = false; }
	}

	// Camera orbiting around point (0, 1, 0) at a distance of 5 units
	float radius = 5.0f;
	float cameraAngleRadians = glm::radians(fmod(float(getTime()) * 0.01f, 360.0f));
	// Used when adjusting cameras manually
	// float cameraAngleRadians = (float(_currentCameraTransformIndex) * 0.5f) * glm::pi<float>() - glm::pi<float>() * 0.5f; // -glm::pi<float>() * 0.5f

	glm::vec2 circumferencePoint = glm::vec2(radius * cos(cameraAngleRadians), radius * sin(cameraAngleRadians));
	glm::vec3 cameraPosition = glm::vec3(glm::vec3(circumferencePoint.x, 4.0f, circumferencePoint.y));
	glm::mat4 viewTemp = glm::lookAt(cameraPosition, glm::vec3(0.0, _cameraLookAtHeight, 0.0), glm::vec3(0.0, 1.0, 0.0));
	gl::UniformMatrix4fv(_surroundUniformID[static_cast<int>(EnumSurroundUniformID::viewProjectionMatrix)], 1, GL_FALSE, glm::value_ptr(_projectionMatrix * viewTemp));

	renderSurroundMeshes();

	// Draw the car 3D model meshes
	gl::UseProgram(_deviceResources->carShader);
	gl::UniformMatrix4fv(_carUniformID[static_cast<int>(EnumCarUniformID::viewProjectionMatrix)], 1, GL_FALSE, glm::value_ptr(_projectionMatrix * viewTemp));
	gl::Uniform3fv(_carUniformID[static_cast<int>(EnumCarUniformID::cameraPosition)], 1, &cameraPosition[0]);

	gl::ActiveTexture(GL_TEXTURE0);
	gl::BindSampler(0, _deviceResources->surroundCameraSampler);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->albedoTextureID);

	gl::ActiveTexture(GL_TEXTURE1);
	gl::BindSampler(1, _deviceResources->surroundCameraSampler);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->normalTextureID);

	gl::ActiveTexture(GL_TEXTURE2);
	gl::BindSampler(2, _deviceResources->surroundCameraSampler);
	gl::BindTexture(GL_TEXTURE_2D, _deviceResources->roughnessMetallicTextureID);

	if (_drawCubemap)
	{
		gl::ActiveTexture(GL_TEXTURE3);
		gl::BindSampler(3, _deviceResources->surroundCameraSampler);
		gl::BindTexture(GL_TEXTURE_CUBE_MAP, _deviceResources->cubemapTextureID);
	}

	if (_useHighQualityMaterials)
	{
		gl::ActiveTexture(GL_TEXTURE4);
		gl::BindSampler(4, _deviceResources->surroundCameraSampler);
		gl::BindTexture(GL_TEXTURE_2D, _deviceResources->lookupTablePBRTextureID);
	}

	uint32_t numMeshes = _deviceResources->carResources._model->getNumMeshes();

	for (uint32_t i = 0; i < numMeshes; ++i)
	{
		gl::BindVertexArray(_deviceResources->carResources._vectorVAO[i]);
		gl::DrawElements(GL_TRIANGLES, _deviceResources->carResources._vectorIndexNumber[i], _deviceResources->carResources._vectorIndexType[i], nullptr);
		debugThrowOnApiError("OpenGLSCSurroundView::renderFrame: Error drawing car meshes");
	}

	gl::BindVertexArray(0);
	gl::BindSampler(0, 0);
	gl::BindSampler(1, 0);
	gl::BindSampler(2, 0);

	if (_drawCubemap) { gl::BindSampler(3, 0); }

	if (_useHighQualityMaterials) { gl::BindSampler(4, 0); }

	if (this->shouldTakeScreenshot()) { pvr::utils::takeScreenshot(this->getScreenshotFileName(), this->getWidth(), this->getHeight()); }

	_deviceResources->context->swapBuffers();

	return pvr::Result::Success;
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<OpenGLSCSurroundView>(); }