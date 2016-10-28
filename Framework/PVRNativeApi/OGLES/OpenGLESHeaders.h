/*!*********************************************************************************************************************
\file         PVRNativeApi\OGLES\OpenGLESHeaders.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Includes the headers for OpenGL ES. Prefer to directly include the OpenGLESBindings.h which will include the
              gl:: and glext:: class for you to use.
***********************************************************************************************************************/
#pragma once
#define GL_NO_PROTOTYPES

#if !defined(BUILD_API_MAX)
#if defined(TARGET_OS_IPHONE)
#define BUILD_API_MAX 30
#elif defined(TARGET_OS_MAC)
#define BUILD_API_MAX 30
#else
#define BUILD_API_MAX 31
#endif
#endif

#if BUILD_API_MAX>=31 //OGLES31

#include <GLES3/gl31.h>
#include <GLES2/gl2ext.h>

#elif BUILD_API_MAX>=30 //OGLES3

#if defined(TARGET_OS_IPHONE)
#import <openGLES/ES3/gl.h>
#import <openGLES/ES3/glext.h>
#import <openGLES/ES2/glext.h>
#else
#include "GLES3/gl3.h"
#include <GLES2/gl2ext.h>
#endif

#elif BUILD_API_MAX>=20

#if defined(TARGET_OS_IPHONE)
#import <openGLES/ES2/gl.h>
#import <openGLES/ES2/glext.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
//#include <GLES2/gl2extimg.h>
#endif
#else

#error OpenGL ES level under 2.0 (BUILD_API_MAX=20) not supported.

#endif

#if defined(GL_ES_VERSION_2_0)
#if !defined(GL_KHR_debug)
typedef void (GL_APIENTRY* GLDEBUGPROCKHR)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam);
#endif
#endif

#if defined(TARGET_OS_IPHONE)
#import <openGLES/ES2/gl.h>
#import <openGLES/ES2/glext.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
//#include <GLES2/gl2extimg.h>
#endif

#ifdef KHRONOS_APIENTRY
#define PVR_APIENTRY KHRONOS_APIENTRY
#else
#define PVR_APIENTRY GL_APIENTRY
#endif
