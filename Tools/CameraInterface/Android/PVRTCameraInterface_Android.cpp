/******************************************************************************

 @File         PVRTCameraInterface_Android.cpp

 @Title        PVRTCameraInterface

 @Version

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     ANSI compatible

 @Description  Android implementation of the camera streaming interface.

 ******************************************************************************/

/*****************************************************************************
 ** Includes
 ******************************************************************************/
#include "PVRTCameraInterface_Android.h"
#include <android/log.h>
#include <string.h>

/*
 Few macro definitions here.
 */
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "CameraInterface - Native", __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "CameraInterface - Native", __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "CameraInterface - Native", __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "CameraInterface - Native", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "CameraInterface - Native", __VA_ARGS__)

jobject     g_obj;
JavaVM *    g_cachedVM       = 0;
jmethodID   g_updateImageMID = 0;

CPVRTCameraInterfaceAndroid* g_pActiveSession = NULL;

/*
 JNI Functionality
 */
JNIEXPORT void JNICALL Java_com_powervr_CameraInterface_CameraInterface_cacheJavaObject (JNIEnv * env, jobject obj)
{
	g_obj = env->NewGlobalRef(obj);
}

JNIEXPORT void JNICALL Java_com_powervr_CameraInterface_CameraInterface_setTexCoordsProjMatrix (JNIEnv * env, jobject obj, jfloatArray ptr)
{
	jfloat* flt1 = env->GetFloatArrayElements(ptr,0);

	if(g_pActiveSession)
	{
		for (int i=0; i < 16; ++i)
			g_pActiveSession->m_projectionMatrix[i] = flt1[i];

		LOGD("SurfaceTexture projection matrix changed");
		g_pActiveSession->m_hasProjectionMatrixChanged = true;
	}

	env->ReleaseFloatArrayElements(ptr, flt1, 0);
}

jint JNI_OnLoad(JavaVM * vm, void * reserved)
{

    JNIEnv * env = 0;

    if ((vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) ||
    		(env == 0))
    {
    	LOGV("GetEnv failed" );
        return -1;
    }

    // Cache the VM
    g_cachedVM = vm;

    return JNI_VERSION_1_6;
}

/****************************************************************************/

CPVRTCameraInterfaceAndroid::CPVRTCameraInterfaceAndroid()
{
}

/****************************************************************************/

CPVRTCameraInterfaceAndroid::~CPVRTCameraInterfaceAndroid()
{
}

/****************************************************************************/

bool CPVRTCameraInterfaceAndroid::InitialiseSession(EPVRTHWCamera eCamera)
{
	if(strstr((const char *)glGetString(GL_EXTENSIONS), "OES_EGL_image_external") == 0)
	{
		LOGE("Extension OES_EGL_image_external not found.\n");
		return false;
	}

	// Create an EGLImage External texture
	// Create a http://www.khronos.org/registry/gles/extensions/OES/OES_EGL_image_external.txt
	glGenTextures(1, &m_yuvTexture);

	glBindTexture(GL_TEXTURE_EXTERNAL_OES, m_yuvTexture);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	JNIEnv * env = 0;
	jint res = g_cachedVM->AttachCurrentThread(&env, 0);
	
	if((res != 0) || (env == 0))
	{
		LOGV("AttachCurrentThread failed" );
		return false;
	}

	jclass clazz = 0;
    clazz = env->GetObjectClass(g_obj);
    if(clazz == 0)
    {
    	LOGV("GetObjectClass failed" );
		g_cachedVM->DetachCurrentThread();
		return false;
    }

	jmethodID mid = env->GetMethodID(clazz, "createCamera", "(I)I");
	if(mid == 0)
	{
		LOGV("GetMethodID failed" );
		g_cachedVM->DetachCurrentThread();
		return false;
	}

	int result = env->CallIntMethod(g_obj, mid, m_yuvTexture);

	env->DeleteLocalRef(clazz);
	g_pActiveSession = this;
	
	g_cachedVM->DetachCurrentThread();

	return result;
}

/****************************************************************************/

bool CPVRTCameraInterfaceAndroid::HasImageChanged()
{
	JNIEnv * env = 0;
	jint res = g_cachedVM->AttachCurrentThread(&env, 0);
	bool result = 0;

	// If the update image method has not been cached
	if (g_updateImageMID == 0)
	{
		if((res != 0) || (env == 0))
		{
			LOGV("AttachCurrentThread failed" );
			g_cachedVM->DetachCurrentThread();
			return false;
		}

		jclass clazz = 0;
		clazz = env->GetObjectClass(g_obj);
		if(clazz == 0)
		{
			LOGV("GetObjectClass failed" );
			g_cachedVM->DetachCurrentThread();
			return false;
		}

		// Get and cache the method ID
		g_updateImageMID = env->GetMethodID(clazz, "updateImage", "()Z");
		if(g_updateImageMID == 0)
		{
			LOGV("GetMethodID failed");
			g_cachedVM->DetachCurrentThread();
			return false;
		}

		env->DeleteLocalRef(clazz);
	}

	result = env->CallIntMethod(g_obj, g_updateImageMID);
	g_cachedVM->DetachCurrentThread();

	return (bool)result;
}

/****************************************************************************/

bool CPVRTCameraInterfaceAndroid::HasProjectionMatrixChanged()
{
	return m_hasProjectionMatrixChanged;
}

/****************************************************************************/

const float* const CPVRTCameraInterfaceAndroid::GetProjectionMatrix()
{
	m_hasProjectionMatrixChanged = false;
	return m_projectionMatrix;
}

/****************************************************************************/

GLuint CPVRTCameraInterfaceAndroid::GetYUVTexture()
{
	return m_yuvTexture;
}

/****************************************************************************/



void CPVRTCameraInterfaceAndroid::DestroySession()
{
	g_pActiveSession = NULL;
}

/****************************************************************************/

void CPVRTCameraInterfaceAndroid::GetCameraResolution(unsigned int& x, unsigned int& y)
{
	JNIEnv * env = 0;
	jint res = g_cachedVM->AttachCurrentThread(&env, 0);

	if ((res != 0) || (env == 0))
	{
		LOGV("AttachCurrentThread failed");
		g_cachedVM->DetachCurrentThread();
		return;
	}

	jclass clazz = 0;
	clazz = env->GetObjectClass(g_obj);
	if (clazz == 0)
	{
		LOGV("GetObjectClass failed");
		g_cachedVM->DetachCurrentThread();
		return;
	}

	// Get the X resolution
    jmethodID midx = env->GetMethodID(clazz, "getCameraResolutionX", "()I");
    if(midx == 0)
    {
        LOGV("GetMethodID failed");
        g_cachedVM->DetachCurrentThread();
        return;
    }

    x = env->CallIntMethod(g_obj, midx);

	// Get the Y resolution
    jmethodID midy = env->GetMethodID(clazz, "getCameraResolutionY", "()I");
    if (midy == 0)
    {
        LOGV("GetMethodID failed");
        g_cachedVM->DetachCurrentThread();
        return;
    }

    y = env->CallIntMethod(g_obj, midy);

	env->DeleteLocalRef(clazz);
	g_cachedVM->DetachCurrentThread();
}

/*****************************************************************************
 End of file (PVRTCameraInterface_Android.cpp)
 *****************************************************************************/
