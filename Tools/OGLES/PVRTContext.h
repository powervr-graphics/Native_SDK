/*!****************************************************************************

 @file         OGLES/PVRTContext.h
 @ingroup      API_OGLES
 @copyright    Copyright (c) Imagination Technologies Limited.
 @brief        Context specific stuff - i.e. 3D API-related.

******************************************************************************/

#ifndef _PVRTCONTEXT_H_
#define _PVRTCONTEXT_H_

#include <stdio.h>
#if defined(__APPLE__)
#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE==1
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#else
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#endif
#else
#if !defined(EGL_NOT_PRESENT)
#include <EGL/egl.h>
#endif
#include <GLES/gl.h>
#endif

#include "PVRTglesExt.h"

/*!
 @addtogroup    API_OGLES
 @{
*/

/****************************************************************************
** Macros
****************************************************************************/
#define PVRTRGBA(r, g, b, a)   ((GLuint) (((a) << 24) | ((b) << 16) | ((g) << 8) | (r)))

/****************************************************************************
** Defines
****************************************************************************/

/****************************************************************************
** Enumerations
****************************************************************************/

/****************************************************************************
** Structures
****************************************************************************/
/*!**************************************************************************
 @struct SPVRTContext
 @brief A structure for storing API specific variables
****************************************************************************/
struct SPVRTContext
{
	CPVRTglesExt * pglesExt;
};

/****************************************************************************
** Functions
****************************************************************************/

/*! @} */

#endif /* _PVRTCONTEXT_H_ */

/*****************************************************************************
 End of file (PVRTContext.h)
*****************************************************************************/

