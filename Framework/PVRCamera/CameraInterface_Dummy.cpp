/*!
\brief Android implementation of the camera streaming interface.
\file PVRCamera/CameraInterface_Dummy.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRCamera/CameraInterface.h"
#include "PVRApi/Api.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/OGLES/TextureGles.h"
namespace pvr {
class CameraInterfaceImpl
{
public:
	native::HTexture_ myTexture;
	int height, width;
	CameraInterfaceImpl() : myTexture(0, 0), height(512), width(512) {}
	bool generateTexture()
	{
		if (myTexture.handle)
		{
			destroyTexture();
		}
		gl::GetError();
		gl::GenTextures(1, &myTexture.handle);
		myTexture.target = GL_TEXTURE_2D;

		gl::BindTexture(GL_TEXTURE_2D, myTexture.handle);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		std::vector<pvr::uint32> rawBuffer; rawBuffer.resize(height * width);
		bool one = false, two = false;
		int hfheight = height / 2, hfwidth = width / 2;
		for (int j = 0; j < height; ++j)
			for (int i = 0; i < width; ++i)
			{
				if ((i < hfwidth + 64) && (i > hfwidth - 65) && (j < hfheight + 72) && (j > hfheight - 57))
				{
					one = (((i + hfwidth) / 8) % 2) != 0;
					two = (((j + hfheight) / 8) % 2) != 0;
				}
				else if ((i < hfwidth + 128) && (i > hfwidth - 129) && (j < hfheight + 136) && (j > hfheight - 121))
				{
					one = (((i + hfwidth) / 16) % 2) != 0;
					two = (((j + hfheight) / 16) % 2) != 0;
				}
				else if ((i < hfwidth + 256) && (i > hfwidth - 257) && (j < hfheight + 264) && (j > hfheight - 249))
				{
					one = (((i + hfwidth) / 32) % 2) != 0;
					two = (((j + hfheight) / 32) % 2) != 0;
				}
				else
				{
					one = false;
					two = false;
				}

				rawBuffer[j * width + i] = (one ^ two ? 0xFFC0C0C0 : 0xFF202020);
			}
		gl::TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rawBuffer.data());
		GLint err = gl::GetError();
		if (err != GL_NO_ERROR)
		{
			Log(Log.Error, "PVRCamera, Dummy version - Error while generating the dummy camera texture. Possible bug.");
			return false;
		}
		return true;
	}
	void destroyTexture()
	{
		gl::DeleteTextures(1, &myTexture.handle);
		myTexture.handle = 0;
	}
	const native::HTexture_& getRgbTexture()
	{
		return myTexture;
	}
};

CameraInterface::CameraInterface()
{
	pImpl = new CameraInterfaceImpl;
}
CameraInterface::~CameraInterface()
{
	delete static_cast<CameraInterfaceImpl*>(pImpl);
}

bool CameraInterface::initializeSession(HWCamera::Enum eCamera, int preferredResX, int preferredResY)
{
	Log(Log.Verbose, "PVRCamera: Initialising session.");
	static_cast<CameraInterfaceImpl*>(pImpl)->width = preferredResX ? preferredResX : 512;
	static_cast<CameraInterfaceImpl*>(pImpl)->height = preferredResY ? preferredResY : 512;
	return static_cast<CameraInterfaceImpl*>(pImpl)->generateTexture();
}

bool CameraInterface::updateImage() { return false; }

bool CameraInterface::hasProjectionMatrixChanged() { return false; }

const glm::mat4& CameraInterface::getProjectionMatrix()
{
	static const glm::mat4 proj(1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1.); return proj;
}

const native::HTexture_& CameraInterface::getRgbTexture()
{
	return static_cast<CameraInterfaceImpl*>(pImpl)->getRgbTexture();
}

static pvr::native::HTexture_ dummy_tex(0, 0);

const native::HTexture_& CameraInterface::getLuminanceTexture()
{
	return dummy_tex;
}

const native::HTexture_& CameraInterface::getChrominanceTexture()
{
	return dummy_tex;
}

void CameraInterface::destroySession() { static_cast<CameraInterfaceImpl*>(pImpl)->destroyTexture(); }

void CameraInterface::getCameraResolution(unsigned int& x, unsigned int& y)
{
	x = static_cast<CameraInterfaceImpl*>(pImpl)->width; y = static_cast<CameraInterfaceImpl*>(pImpl)->height;
}

bool CameraInterface::hasRgbTexture()
{
	return true;
}

bool CameraInterface::hasLumaChromaTextures()
{
	return false;
}

api::TextureView getTextureFromPVRCameraHandle(pvr::GraphicsContext& context, const native::HTexture_& cameraTexture)
{
	Log(Log.Verbose, "Camera interface util: Handle %d, Target 0x%08X", cameraTexture.handle, cameraTexture.target);

	api::TextureStore texStore = context->createTexture();
	static_cast<native::HTexture_&>(api::native_cast(*texStore)) = cameraTexture;
	return context->createTextureView(texStore);
}

}