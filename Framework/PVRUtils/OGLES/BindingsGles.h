/*!
\brief Includes the OpenGL ES Headers necessary for using OpenGL ES directly in your code, through the provided gl::
and glext:: classes.
\file PVRUtils/OGLES/BindingsGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Log.h"

/// <summary>Uses the defined Logger "Log" to log an error with the provided arguments.</summary>
#define PVR_LOG_ERROR(...) do{ Log(LogLevel::Error,__VA_ARGS__); }while(false)

/// <summary>Uses the defined Logger "Log" to log a information message with the provided arguments.</summary>
#define PVR_LOG_INFO(...) do{ Log(LogLevel::Information,__VA_ARGS__); }while(false)

#if defined(TARGET_OS_IPHONE)
/// <summary>Specifies OpenGL ES 3 is to be used with dynamic function pointer loading.</summary>
#define DYNAMICGLES_GLES30
#else
/// <summary>Specifies OpenGL ES 3.2 is to be used with dynamic function pointer loading.</summary>
#define DYNAMICGLES_GLES32
#endif

#include "DynamicGles.h"

#if defined(GL_ES_VERSION_2_0)
#if !defined(GL_KHR_debug)
typedef void (GL_APIENTRY* GLDEBUGPROCKHR)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam);
#endif
#endif

#ifndef WIN32_LEAN_AND_MEAN
/// <summary>Specifies that a set of Windows headers designed to speed up the build process should be used.</summary>
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef NOMINMAX
/// <summary>Instructs Windows.h to not define the min and max macros.</summary>
#define NOMINMAX 1
#endif
#include "PVRUtils/EGL/EglPlatformContext.h"