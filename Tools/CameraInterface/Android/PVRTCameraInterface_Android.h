/*!****************************************************************************

 @file         PVRTCameraInterface_Android.h
 @ingroup      API_CAMERAINTERFACE
 @copyright    Copyright (c) Imagination Technologies Limited.
 @brief        Android implementation of the camera streaming interface.

 ******************************************************************************/

#ifndef __PVRTCAMERAINTERFACEANDROID_H__
#define __PVRTCAMERAINTERFACEANDROID_H__

#include <jni.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#if !defined(EGL_NOT_PRESENT)
#include <EGL/egl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

JNIEXPORT void JNICALL Java_com_powervr_CameraInterface_CameraInterface_cacheJavaObject (JNIEnv * env, jobject obj);
JNIEXPORT void JNICALL Java_com_powervr_CameraInterface_CameraInterface_setTexCoordsProjMatrix (JNIEnv * env, jobject obj, jfloatArray ptr);

#ifdef __cplusplus
}
#endif // __cplusplus

/*!
 @addtogroup API_CAMERAINTERFACE
 @{
 */

/*!**************************************************************************
 @enum         EPVRTHWCamera
 @brief        Enumeration of available hardware cameras
 ***************************************************************************/
enum EPVRTHWCamera
{
	ePVRTHWCamera_Front,
	ePVRTHWCamera_Back,
};

/*!**************************************************************************
 @class CPVRTCameraInterfaceiOS
 @brief iOS Camera Interface API
 ****************************************************************************/
class CPVRTCameraInterfaceAndroid
{
public:
	/*!***************************************************************************
	 @brief      		Class constructor
	 *****************************************************************************/
	CPVRTCameraInterfaceAndroid();

	/*!***************************************************************************
	 @brief      		Class destructor
	 *****************************************************************************/
	~CPVRTCameraInterfaceAndroid();

	/*!***************************************************************************
	 @brief      		Initialises the capture session using the given hardware camera, if it is available.
	 @param[in]         eCamera       The hardware camera to attempt to stream from
	 @return            true if successful
	 *****************************************************************************/
	bool InitialiseSession(EPVRTHWCamera eCamera);

	/*!***************************************************************************
	 @brief      		Checks to see if the image has been updated.
	 *****************************************************************************/
	bool HasImageChanged();

	/*!***************************************************************************
	 @brief      		Checks to see if the projection matrix has changed.
	 *****************************************************************************/
	bool HasProjectionMatrixChanged();

	/*!***************************************************************************
	 @brief      		Retrieves the current texture projection matrix and resets
	                    the 'changed' flag.
	 @return            pointer to 16 float values representing the matrix
	 *****************************************************************************/
	const float* const GetProjectionMatrix();

	/*!***************************************************************************
	 @brief      		Retrieves the texture name for the YUV camera texture
	 @return            GL texture ID
	 *****************************************************************************/
	GLuint GetYUVTexture();

	/*!***************************************************************************
	 @brief      		Shutdown the AV capture session and release associated objects.
	 *****************************************************************************/
	void DestroySession();

	/*!***************************************************************************
	 @brief      		Returns the resolution of the currently active camera
	 @param[out]        x
	 @param[out]        y
	 *****************************************************************************/
	void GetCameraResolution(unsigned int& x, unsigned int& y);

private:
	float        m_projectionMatrix[16];
	bool         m_hasProjectionMatrixChanged;
	GLuint       m_yuvTexture;

	friend void JNICALL Java_com_powervr_CameraInterface_CameraInterface_setTexCoordsProjMatrix (JNIEnv *, jobject, jfloatArray);
};

/*! @} */

#endif // __PVRTCAMERAINTERFACEANDROID_H__

/*****************************************************************************
 End of file (PVRTCameraInterface_Android.h)
 *****************************************************************************/
