/*!
\brief Implementation of the Android camera interface.
\file PVRCamera/CameraInterface_Android.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Platform independent camera interface API include file.
*/

#include "PVRCamera/CameraInterface.h"
#include <jni.h>
#include "PVRCore/Log.h"
#include "PVRCore/Errors.h"

//!\cond NO_DOXYGEN

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

JNIEXPORT void JNICALL Java_com_powervr_PVRCamera_CameraInterface_cacheJavaObject(JNIEnv* env, jobject obj);
JNIEXPORT void JNICALL Java_com_powervr_PVRCamera_CameraInterface_setTexCoordsProjMatrix(JNIEnv* env, jobject obj, jfloatArray ptr);

#ifdef __cplusplus
}
#endif // __cplusplus

namespace pvr {
class CameraInterfaceImpl;
}

pvr::CameraInterfaceImpl* activeSession = NULL;
jobject jobj;
JavaVM* cachedVM = 0;
jmethodID updateImageMID = 0;

class pvr::CameraInterfaceImpl
{
public:
	GLuint texture;
	glm::mat4 projectionMatrix;
	bool hasProjectionMatrixChanged;

	CameraInterfaceImpl() : texture(0), hasProjectionMatrixChanged(true) { memset(glm::value_ptr(projectionMatrix), 0, sizeof(projectionMatrix)); }

	void initializeSession(HWCamera::Enum eCamera, int width, int height)
	{
		if (strstr((const char*)gl::GetString(GL_EXTENSIONS), "OES_EGL_image_external") == 0)
		{ throw InvalidOperationError("CameraInterface - NativeExtension OES_EGL_image_external not found."); }

		// Create an EGLImage External texture
		// Create a http://www.khronos.org/registry/gles/extensions/OES/OES_EGL_image_external.txt
		gl::GenTextures(1, &texture);

		gl::BindTexture(GL_TEXTURE_EXTERNAL_OES, texture);

		JNIEnv* env = 0;
		jint res = cachedVM->AttachCurrentThread(&env, 0);

		if ((res != 0) || (env == 0)) { throw InvalidOperationError("CameraInterface - NativeAttachCurrentThread failed."); }

		jclass clazz = 0;
		clazz = env->GetObjectClass(jobj);
		if (clazz == 0)
		{
			cachedVM->DetachCurrentThread();
			throw InvalidOperationError("CameraInterface - NativeGetObjectClass failed.");
		}

		jmethodID createCameraMethod = env->GetMethodID(clazz, "createCamera", "(I)I");
		if (createCameraMethod == 0)
		{
			cachedVM->DetachCurrentThread();
			throw InvalidOperationError("CameraInterface - NativeGetMethodID failed.");
		}

		int result = env->CallIntMethod(jobj, createCameraMethod, texture);

		env->DeleteLocalRef(clazz);
		activeSession = this;

		cachedVM->DetachCurrentThread();

		if (!result) throw InvalidOperationError("CameraInterface - Calling the java createCamera method failed.");
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
				cachedVM->DetachCurrentThread();
				throw InvalidOperationError("CameraInterface - updateImage::NativeAttachCurrentThread failed");
			}

			jclass clazz = 0;
			clazz = env->GetObjectClass(jobj);
			if (clazz == 0)
			{
				cachedVM->DetachCurrentThread();
				throw InvalidOperationError("CameraInterface - updateImage::NativeGetObjectClass failed");
			}

			// Get and cache the method ID.
			updateImageMID = env->GetMethodID(clazz, "updateImage", "()Z");
			if (updateImageMID == 0)
			{
				cachedVM->DetachCurrentThread();
				throw InvalidOperationError("CameraInterface - updateImage::NativeGetMethodID failed");
			}

			env->DeleteLocalRef(clazz);
		}

		result = env->CallBooleanMethod(jobj, updateImageMID);
		cachedVM->DetachCurrentThread();

		return (bool)result;
	}

	void getCameraResolution(uint32_t& x, uint32_t& y)
	{
		JNIEnv* env = 0;
		jint res = cachedVM->AttachCurrentThread(&env, 0);

		if ((res != 0) || (env == 0))
		{
			cachedVM->DetachCurrentThread();
			throw InvalidOperationError("CameraInterface - getCameraResolution:NativeAttachCurrentThread failed");
		}

		jclass clazz = 0;
		clazz = env->GetObjectClass(jobj);
		if (clazz == 0)
		{
			throw InvalidOperationError("CameraInterface - getCameraResolution:NativeGetObjectClass failed");
			cachedVM->DetachCurrentThread();
		}

		// Get the X resolution.
		jmethodID midx = env->GetMethodID(clazz, "getCameraResolutionX", "()I");
		if (midx == 0)
		{
			cachedVM->DetachCurrentThread();
			throw InvalidOperationError("CameraInterface - getCameraResolution:NativeGetMethodID failed");
		}

		x = env->CallIntMethod(jobj, midx);

		// Get the Y resolution.
		jmethodID midy = env->GetMethodID(clazz, "getCameraResolutionY", "()I");
		if (midy == 0)
		{
			cachedVM->DetachCurrentThread();
			throw InvalidOperationError("CameraInterface - getCameraResolution:NativeGetMethodID failed");
		}

		y = env->CallIntMethod(jobj, midy);

		env->DeleteLocalRef(clazz);
		cachedVM->DetachCurrentThread();
	}
	void please_dont_strip_jni_functions()
	{
		Java_com_powervr_PVRCamera_CameraInterface_cacheJavaObject(0, 0);
		Java_com_powervr_PVRCamera_CameraInterface_setTexCoordsProjMatrix(0, 0, 0);
		JNI_OnLoad(0, 0);
	}
};

JNIEXPORT void JNICALL Java_com_powervr_PVRCamera_CameraInterface_cacheJavaObject(JNIEnv* env, jobject obj) { jobj = env->NewGlobalRef(obj); }

JNIEXPORT void JNICALL Java_com_powervr_PVRCamera_CameraInterface_setTexCoordsProjMatrix(JNIEnv* env, jobject obj, jfloatArray ptr)
{
	jfloat* flt1 = env->GetFloatArrayElements(ptr, 0);

	if (activeSession)
	{
		memcpy(glm::value_ptr(activeSession->projectionMatrix), flt1, 16 * sizeof(float));
		Log(LogLevel::Debug,
			"CameraInterface - Projection matrix changed. Projection matrix is now\n%4.3f %4.3f %4.3f %4.3f\n%4.3f %4.3f %4.3f %4.3f\n%4.3f %4.3f %4.3f %4.3f\n%4.3f %4.3f %4.3f "
			"%4.3f",
			activeSession->projectionMatrix[0][0], activeSession->projectionMatrix[0][1], activeSession->projectionMatrix[0][2], activeSession->projectionMatrix[0][3],
			activeSession->projectionMatrix[1][0], activeSession->projectionMatrix[1][1], activeSession->projectionMatrix[1][2], activeSession->projectionMatrix[1][3],
			activeSession->projectionMatrix[2][0], activeSession->projectionMatrix[2][1], activeSession->projectionMatrix[2][2], activeSession->projectionMatrix[2][3],
			activeSession->projectionMatrix[3][0], activeSession->projectionMatrix[3][1], activeSession->projectionMatrix[3][2], activeSession->projectionMatrix[3][3]);

		activeSession->hasProjectionMatrixChanged = true;
	}

	env->ReleaseFloatArrayElements(ptr, flt1, 0);
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env = 0;

	if ((vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) || (env == 0))
	{
		Log(LogLevel::Debug, "CameraInterface - NativeGetEnv failed");
		return -1;
	}

	// Cache the VM
	cachedVM = vm;

	return JNI_VERSION_1_6;
}

void pvr::CameraInterface::initializeSession(HWCamera::Enum camera, int width, int height)
{
	static_cast<pvr::CameraInterfaceImpl*>(pImpl)->initializeSession(camera, width, height);
}

bool global_dummy = false;

pvr::CameraInterface::CameraInterface() { pImpl = static_cast<void*>(new CameraInterfaceImpl()); }
pvr::CameraInterface::~CameraInterface()
{
	delete static_cast<pvr::CameraInterfaceImpl*>(pImpl);
	activeSession = NULL;
}

bool pvr::CameraInterface::hasProjectionMatrixChanged() { return static_cast<pvr::CameraInterfaceImpl*>(pImpl)->hasProjectionMatrixChanged; }

void pvr::CameraInterface::getCameraResolution(uint32_t& x, uint32_t& y) { return static_cast<pvr::CameraInterfaceImpl*>(pImpl)->getCameraResolution(x, y); }

void pvr::CameraInterface::destroySession() { activeSession = NULL; }

GLuint pvr::CameraInterface::getRgbTexture() { return static_cast<pvr::CameraInterfaceImpl*>(pImpl)->texture; }

GLuint dummy_texture(0);
GLuint pvr::CameraInterface::getLuminanceTexture() { return dummy_texture; }
GLuint pvr::CameraInterface::getChrominanceTexture() { return dummy_texture; }
bool pvr::CameraInterface::hasRgbTexture() { return true; }
bool pvr::CameraInterface::hasLumaChromaTextures() { return false; }
bool pvr::CameraInterface::updateImage() { return static_cast<pvr::CameraInterfaceImpl*>(pImpl)->updateImage(); }

const glm::mat4& pvr::CameraInterface::getProjectionMatrix()
{
	static_cast<pvr::CameraInterfaceImpl*>(pImpl)->hasProjectionMatrixChanged = false;
	return static_cast<pvr::CameraInterfaceImpl*>(pImpl)->projectionMatrix;
}
//!\endcond