/*!******************************************************************************************************************
\file         PVRCamera\CameraInterface_Android.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Platform independent camera interface API include file.
\brief         Implementation of the Android camera interface.
********************************************************************************************************************/

#include "PVRCamera/CameraInterface.h"
#include "PVRApi/Api.h"
#include "PVRApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/OGLES/NativeObjectsGles.h"
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

JNIEXPORT void JNICALL Java_com_powervr_PVRCamera_CameraInterface_cacheJavaObject(JNIEnv* env, jobject obj);
JNIEXPORT void JNICALL Java_com_powervr_PVRCamera_CameraInterface_setTexCoordsProjMatrix(JNIEnv* env, jobject obj, jfloatArray ptr);

#ifdef __cplusplus
}
#endif // __cplusplus

namespace pvr
{
	class CameraInterfaceImpl;
}

pvr::CameraInterfaceImpl* activeSession = NULL;
jobject     jobj;
JavaVM*     cachedVM = 0;
jmethodID   updateImageMID = 0;


class pvr::CameraInterfaceImpl
{
public:
	native::HTexture_ hTexture;
	glm::mat4 projectionMatrix;
	bool hasProjectionMatrixChanged;

	CameraInterfaceImpl():hTexture(0,GL_TEXTURE_EXTERNAL_OES),hasProjectionMatrixChanged(true)
	{
		memset(glm::value_ptr(projectionMatrix),0,sizeof(projectionMatrix));
		projectionMatrix[0][0] = projectionMatrix[1][1] = projectionMatrix[2][2] = projectionMatrix[3][3] = 2.;
	}


	bool initialiseSession(HWCamera::Enum eCamera, int width, int height)
	{
		if (strstr((const char*)gl::GetString(GL_EXTENSIONS), "OES_EGL_image_external") == 0)
		{
			pvr::Log(pvr::Log.Critical, "CameraInterface - NativeExtension OES_EGL_image_external not found.\n");
			return false;
		}

		// Create an EGLImage External texture
		// Create a http://www.khronos.org/registry/gles/extensions/OES/OES_EGL_image_external.txt
		gl::GenTextures(1, &hTexture.handle);

		gl::BindTexture(GL_TEXTURE_EXTERNAL_OES, hTexture);
		gl::TexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl::TexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		JNIEnv* env = 0;
		jint res = cachedVM->AttachCurrentThread(&env, 0);

		if ((res != 0) || (env == 0))
		{
			pvr::Log(pvr::Log.Verbose, "CameraInterface - NativeAttachCurrentThread failed");
			return false;
		}

		jclass clazz = 0;
		clazz = env->GetObjectClass(jobj);
		if (clazz == 0)
		{
			pvr::Log(pvr::Log.Verbose, "CameraInterface - NativeGetObjectClass failed");
			cachedVM->DetachCurrentThread();
			return false;
		}

		jmethodID createCameraMethod = env->GetMethodID(clazz, "createCamera", "(I)I");
		if (createCameraMethod == 0)
		{
			pvr::Log(pvr::Log.Verbose, "CameraInterface - NativeGetMethodID failed");
			cachedVM->DetachCurrentThread();
			return false;
		}

		int result = env->CallIntMethod(jobj, createCameraMethod, hTexture);

		env->DeleteLocalRef(clazz);
		activeSession = this;

		cachedVM->DetachCurrentThread();

		return result;
	}

	/****************************************************************************/

	bool updateImage()
	{
		JNIEnv* env = 0;
		jint res = cachedVM->AttachCurrentThread(&env, 0);
		bool result = 0;

		// If the update image method has not been cached.
		if (updateImageMID == 0)
		{
			if ((res != 0) || (env == 0))
			{
				pvr::Log(pvr::Log.Verbose, "CameraInterface - NativeAttachCurrentThread failed");
				cachedVM->DetachCurrentThread();
				return false;
			}

			jclass clazz = 0;
			clazz = env->GetObjectClass(jobj);
			if (clazz == 0)
			{
				pvr::Log(pvr::Log.Verbose, "CameraInterface - NativeGetObjectClass failed");
				cachedVM->DetachCurrentThread();
				return false;
			}

			// Get and cache the method ID.
			updateImageMID = env->GetMethodID(clazz, "updateImage", "()Z");
			if (updateImageMID == 0)
			{
				pvr::Log(pvr::Log.Verbose, "CameraInterface - NativeGetMethodID failed");
				cachedVM->DetachCurrentThread();
				return false;
			}

			env->DeleteLocalRef(clazz);
		}

		result = env->CallBooleanMethod(jobj, updateImageMID);
		cachedVM->DetachCurrentThread();

		return (bool)result;
	}

	void getCameraResolution(unsigned int& x, unsigned int& y)
	{
		JNIEnv* env = 0;
		jint res = cachedVM->AttachCurrentThread(&env, 0);

		if ((res != 0) || (env == 0))
		{
			pvr::Log(pvr::Log.Verbose, "CameraInterface - NativeAttachCurrentThread failed");
			cachedVM->DetachCurrentThread();
			return;
		}

		jclass clazz = 0;
		clazz = env->GetObjectClass(jobj);
		if (clazz == 0)
		{
			pvr::Log(pvr::Log.Verbose, "CameraInterface - NativeGetObjectClass failed");
			cachedVM->DetachCurrentThread();
			return;
		}

		// Get the X resolution.
		jmethodID midx = env->GetMethodID(clazz, "getCameraResolutionX", "()I");
		if (midx == 0)
		{
			pvr::Log(pvr::Log.Verbose, "CameraInterface - NativeGetMethodID failed");
			cachedVM->DetachCurrentThread();
			return;
		}

		x = env->CallIntMethod(jobj, midx);

		// Get the Y resolution.
		jmethodID midy = env->GetMethodID(clazz, "getCameraResolutionY", "()I");
		if (midy == 0)
		{
			pvr::Log(pvr::Log.Verbose, "CameraInterface - NativeGetMethodID failed");
			cachedVM->DetachCurrentThread();
			return;
		}

		y = env->CallIntMethod(jobj, midy);

		env->DeleteLocalRef(clazz);
		cachedVM->DetachCurrentThread();
	}
	void please_dont_strip_jni_functions()
	{
		Java_com_powervr_PVRCamera_CameraInterface_cacheJavaObject(0,0);
		Java_com_powervr_PVRCamera_CameraInterface_setTexCoordsProjMatrix(0,0,0);
		JNI_OnLoad(0,0);
	}
};

JNIEXPORT void JNICALL Java_com_powervr_PVRCamera_CameraInterface_cacheJavaObject(JNIEnv* env, jobject obj)
{
	jobj = env->NewGlobalRef(obj);
}

JNIEXPORT void JNICALL Java_com_powervr_PVRCamera_CameraInterface_setTexCoordsProjMatrix(JNIEnv* env, jobject obj, jfloatArray ptr)
{
	jfloat* flt1 = env->GetFloatArrayElements(ptr, 0);

	if (activeSession)
	{
	    memcpy(glm::value_ptr(activeSession->projectionMatrix), flt1, 16*sizeof(float));
		pvr::Log(pvr::Log.Debug, "CameraInterface - Native SurfaceTexture projection matrix changed!");
		activeSession->hasProjectionMatrixChanged = true;
	}

	env->ReleaseFloatArrayElements(ptr, flt1, 0);
}


jint JNI_OnLoad(JavaVM* vm, void* reserved)
{

	JNIEnv* env = 0;

	if ((vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) ||
	    (env == 0))
	{
		pvr::Log(pvr::Log.Verbose, "CameraInterface - NativeGetEnv failed");
		return -1;
	}

	// Cache the VM
	cachedVM = vm;

	return JNI_VERSION_1_6;
}

bool pvr::CameraInterface::initialiseSession(HWCamera::Enum camera, int width, int height)
{
	return static_cast<pvr::CameraInterfaceImpl*>(pImpl)->initialiseSession(camera, width, height);
}

bool global_dummy = false;

pvr::CameraInterface::CameraInterface()
{
	pImpl = static_cast<void*>(new CameraInterfaceImpl());
}
pvr::CameraInterface::~CameraInterface()
{
	delete static_cast<pvr::CameraInterfaceImpl*>(pImpl);
	activeSession = NULL;
}

bool pvr::CameraInterface::hasProjectionMatrixChanged()
{
    if (static_cast<pvr::CameraInterfaceImpl*>(pImpl)->hasProjectionMatrixChanged)
	{
        pvr::Log(pvr::Log.Debug, "CameraInterface - Projection matrix has changed since last call. Projection matrix is\n%4.3f %4.3f %4.3f %4.3f\n%4.3f %4.3f %4.3f %4.3f\n%4.3f %4.3f %4.3f %4.3f\n%4.3f %4.3f %4.3f %4.3f",
	        static_cast<pvr::CameraInterfaceImpl*>(pImpl)->projectionMatrix[0][0], static_cast<pvr::CameraInterfaceImpl*>(pImpl)->projectionMatrix[0][1], static_cast<pvr::CameraInterfaceImpl*>(pImpl)->projectionMatrix[0][2], static_cast<pvr::CameraInterfaceImpl*>(pImpl)->projectionMatrix[0][3],
		    static_cast<pvr::CameraInterfaceImpl*>(pImpl)->projectionMatrix[1][0], static_cast<pvr::CameraInterfaceImpl*>(pImpl)->projectionMatrix[1][1], static_cast<pvr::CameraInterfaceImpl*>(pImpl)->projectionMatrix[1][2], static_cast<pvr::CameraInterfaceImpl*>(pImpl)->projectionMatrix[1][3],
		    static_cast<pvr::CameraInterfaceImpl*>(pImpl)->projectionMatrix[2][0], static_cast<pvr::CameraInterfaceImpl*>(pImpl)->projectionMatrix[2][1], static_cast<pvr::CameraInterfaceImpl*>(pImpl)->projectionMatrix[2][2], static_cast<pvr::CameraInterfaceImpl*>(pImpl)->projectionMatrix[2][3],
		    static_cast<pvr::CameraInterfaceImpl*>(pImpl)->projectionMatrix[3][0], static_cast<pvr::CameraInterfaceImpl*>(pImpl)->projectionMatrix[3][1], static_cast<pvr::CameraInterfaceImpl*>(pImpl)->projectionMatrix[3][2], static_cast<pvr::CameraInterfaceImpl*>(pImpl)->projectionMatrix[3][3]);
	}
	return static_cast<pvr::CameraInterfaceImpl*>(pImpl)->hasProjectionMatrixChanged;
}

void pvr::CameraInterface::getCameraResolution(unsigned int& x, unsigned int& y)
{
	return static_cast<pvr::CameraInterfaceImpl*>(pImpl)->getCameraResolution(x, y);
}

void pvr::CameraInterface::destroySession()
{
	activeSession = NULL;
}

const pvr::native::HTexture_& pvr::CameraInterface::getRgbTexture()
{
	return static_cast<pvr::CameraInterfaceImpl*>(pImpl)->hTexture;
}

pvr::native::HTexture_ dummy_texture(0, 0);
const pvr::native::HTexture_& pvr::CameraInterface::getLuminanceTexture()
{
	return dummy_texture;
}
const pvr::native::HTexture_& pvr::CameraInterface::getChrominanceTexture()
{
	return dummy_texture;
}
bool pvr::CameraInterface::hasRgbTexture()
{
	return true;
}
bool pvr::CameraInterface::hasLumaChromaTextures()
{
	return false;
}
bool pvr::CameraInterface::updateImage()
{
	return static_cast<pvr::CameraInterfaceImpl*>(pImpl)->updateImage();
}

const glm::mat4& pvr::CameraInterface::getProjectionMatrix()
{
	static_cast<pvr::CameraInterfaceImpl*>(pImpl)->hasProjectionMatrixChanged = false;
	return static_cast<pvr::CameraInterfaceImpl*>(pImpl)->projectionMatrix;
}

namespace pvr{
api::TextureView getTextureFromPVRCameraHandle(pvr::GraphicsContext& context, const native::HTexture_& cameraTexture)
{
	Log(Log.Verbose, "Camera interface util: Handle %d, Target 0x%08X", cameraTexture.handle, cameraTexture.target);
	api::TextureView tex;
	tex.construct(context, cameraTexture);
	return tex;
}
}
