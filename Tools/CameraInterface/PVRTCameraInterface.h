/*!****************************************************************************

 @file         CameraInterface/PVRTCameraInterface.h
 @ingroup      API_CAMERAINTERFACE
 @copyright    Copyright (c) Imagination Technologies Limited.
 @brief        Platform independent camera interface API

******************************************************************************/

#ifndef __PVRTCAMERAINTERFACE_H__
#define __PVRTCAMERAINTERFACE_H__

/*!
 @addtogroup   API_CAMERAINTERFACE
 @{
*/

#if defined(__ANDROID__)
#include "Android/PVRTCameraInterface_Android.h"
#elif defined(__APPLE__) && defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE==1
#include "iOS/PVRTCameraInterface_iOS.h"
#else
#error "This platform does not have a CameraInterface implementation!"
#endif

/*! @} */

#endif // __PVRTCAMERAINTERFACE_H__

/*****************************************************************************
 End of file (PVRTCameraInterface.h)
*****************************************************************************/
