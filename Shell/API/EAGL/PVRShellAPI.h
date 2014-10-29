/*!****************************************************************************

 @file         EAGL/PVRShellAPI.h
 @ingroup      API_EAGL
 @copyright    Copyright (c) Imagination Technologies Limited.
 @brief        3D API context management for EAGL.
 @details      Makes programming for 3D APIs easier by wrapping surface
               initialization, Texture allocation and other functions for use by a demo.

******************************************************************************/

#ifndef __PVRSHELLAPI_H_
#define __PVRSHELLAPI_H_

/****************************************************************************
** 3D API header files
****************************************************************************/
#if defined(BUILD_OGLES)
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#elif defined(BUILD_OGLES2)
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#elif defined(BUILD_OGLES3)
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#endif

/*!
 @addtogroup API_EAGL 
 @brief      EAGL API
 @{
*/

/*!***************************************************************************
 @class PVRShellInitAPI
 @brief Initialisation interface with specific API.
****************************************************************************/
class PVRShellInitAPI
{
public:
	int m_ApiMajorVersion;
	int m_ApiMinorVersion;
};
#endif // __PVRSHELLAPI_H_

/*! @} */

/*****************************************************************************
 End of file (PVRShellAPI.h)
*****************************************************************************/

