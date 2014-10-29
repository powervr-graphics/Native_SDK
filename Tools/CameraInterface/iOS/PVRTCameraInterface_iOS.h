/*!****************************************************************************

 @file         PVRTCameraInterface_iOS.h
 @ingroup      API_CAMERAINTERFACE
 @copyright    Copyright (c) Imagination Technologies Limited.
 @brief        iOS implementation of the camera streaming interface.

 ******************************************************************************/

#ifndef __PVRTCAMERAINTERFACEIOS_H__
#define __PVRTCAMERAINTERFACEIOS_H__

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
class CPVRTCameraInterfaceiOS
{
public:
	/*!***************************************************************************
	 @brief      		Class constructor
	 *****************************************************************************/
	CPVRTCameraInterfaceiOS();

	/*!***************************************************************************
	 @brief      		Class destructor
	 *****************************************************************************/
	~CPVRTCameraInterfaceiOS();

	/*!***************************************************************************
	 @brief      		Initialises the capture session using the given hardware camera, if it is available.
	 @param[in]         eCamera       The hardware camera to attempt to stream from
	 @return            true if successful
	 *****************************************************************************/
	bool InitialiseSession(EPVRTHWCamera eCamera);

	/*!***************************************************************************
	 @brief      		Shutdown the AV capture session and release associated objects.
	 *****************************************************************************/
	void DestroySession();

	/*!***************************************************************************
	 @brief      		Queries CoreVideo for the texture name associated with this texture.
	 @return            The texture name ID
	 *****************************************************************************/
	GLuint GetLuminanceTexture();

	/*!***************************************************************************
	 @brief      		Queries CoreVideo for the texture name associated with this texture.
	 @return            The texture name ID
	 *****************************************************************************/
	GLuint GetChrominanceTexture();

	/*!***************************************************************************
	 @brief      		Queries CoreVideo for the target associated with this texture.
	 @return            The target enumerator
	 *****************************************************************************/
	GLenum GetLuminanceTextureTarget();

	/*!***************************************************************************
	 @brief      		Queries CoreVideo for the target associated with this texture.
	 @return            The target enumerator
	 *****************************************************************************/
	GLenum GetChrominanceTextureTarget();

private:
	void*        m_pImpl;
};

/*! @} */

#endif // __PVRTCAMERAINTERFACEIOS_H__

/*****************************************************************************
 End of file (PVRTCameraInterface_iOS.h)
 *****************************************************************************/
