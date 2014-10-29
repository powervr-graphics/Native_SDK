/*!****************************************************************************

 @file         OGLES3/PVRTgles3Ext.h
 @ingroup      API_OGLES3
 @copyright    Copyright (c) Imagination Technologies Limited.
 @brief        OpenGL ES 3.0 extensions

******************************************************************************/
#ifndef _PVRTgles3Ext_H_
#define _PVRTgles3Ext_H_

/*!
 @addtogroup   API_OGLES3
 @{
*/

#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE==1
#import  <OpenGLES/ES3/gl.h>
#import  <OpenGLES/ES3/glext.h>
// No binary shaders are allowed on the iphone and so this value is not defined
// Defining here allows for a more graceful fail of binary shader loading at runtime
// which can be recovered from instead of fail at compile time
#define GL_SGX_BINARY_IMG 0
#else
#ifdef BUILD_OGLES31
	#include <GLES3/gl31.h>
#else
	#include <GLES3/gl3.h>
#endif
#endif

#if !defined(EGL_NOT_PRESENT)
#define PVRGetProcAddress(x) eglGetProcAddress(#x)
#endif

/****************************************************************************
** Build options
****************************************************************************/

#define GL_PVRTGLESEXT_VERSION 3

/**************************************************************************
****************************** GL EXTENSIONS ******************************
**************************************************************************/

/*!**************************************************************************
 @class         CPVRTgles3Ext
 @brief         A class for initialising and managing OGLES3 extensions
****************************************************************************/
class CPVRTgles3Ext
{
public:
	/*!***********************************************************************
	@brief		Initialises IMG extensions
	*************************************************************************/
	void LoadExtensions();

	/*!***********************************************************************
	@brief		Queries for support of an extension
	@param[in]	extension    Extension to query for
	@return		True if the extension is supported
	*************************************************************************/
	static bool IsGLExtensionSupported(const char * const extension);
};

/*! @} */

#endif /* _PVRTgles3Ext_H_ */

/*****************************************************************************
 End of file (PVRTgles3Ext.h)
*****************************************************************************/

