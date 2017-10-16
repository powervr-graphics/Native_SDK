/*!
\brief Contains implementation of functions required for the EGL wrapper library (namespace egl::).
\file PVRNativeApi/EGL/NativeLibraryEgl.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRNativeApi/EGL/NativeLibraryEgl.h"
#include "PVRCore/Base/NativeLibrary.h"
#include <memory>
#include <string.h>
#ifdef _WIN32
static const char* eglLibraryPath = "libEGL.dll";
#elif defined(TARGET_OS_MAC)
static const char* eglLibraryPath = "libEGL.dylib";
#else
static const char* eglLibraryPath = "libEGL.so";
#endif


static pvr::native::NativeLibrary& eglLib()
{
	static pvr::native::NativeLibrary mylib(eglLibraryPath);
	return mylib;
}

bool egl::initEgl()
{
	if (eglLib().LoadFailed())
	{
		return false;
	}
	egl::ChooseConfig = eglLib().getFunction<PROC_EGL_eglChooseConfig>("eglChooseConfig");
	egl::CopyBuffers = eglLib().getFunction<PROC_EGL_eglCopyBuffers>("eglCopyBuffers");
	egl::CreateContext = eglLib().getFunction<PROC_EGL_eglCreateContext>("eglCreateContext");
	egl::CreatePbufferSurface = eglLib().getFunction<PROC_EGL_eglCreatePbufferSurface>("eglCreatePbufferSurface");
	egl::CreatePixmapSurface = eglLib().getFunction<PROC_EGL_eglCreatePixmapSurface>("eglCreatePixmapSurface");
	egl::CreateWindowSurface = eglLib().getFunction<PROC_EGL_eglCreateWindowSurface>("eglCreateWindowSurface");
	egl::DestroyContext = eglLib().getFunction<PROC_EGL_eglDestroyContext>("eglDestroyContext");
	egl::DestroySurface = eglLib().getFunction<PROC_EGL_eglDestroySurface>("eglDestroySurface");
	egl::GetConfigAttrib = eglLib().getFunction<PROC_EGL_eglGetConfigAttrib>("eglGetConfigAttrib");
	egl::GetConfigs = eglLib().getFunction<PROC_EGL_eglGetConfigs>("eglGetConfigs");
	egl::GetCurrentContext = eglLib().getFunction<PROC_EGL_eglGetCurrentContext>("eglGetCurrentContext");
	egl::GetCurrentDisplay = eglLib().getFunction<PROC_EGL_eglGetCurrentDisplay>("eglGetCurrentDisplay");
	egl::GetCurrentSurface = eglLib().getFunction<PROC_EGL_eglGetCurrentSurface>("eglGetCurrentSurface");
	egl::GetDisplay = eglLib().getFunction<PROC_EGL_eglGetDisplay>("eglGetDisplay");
	egl::GetError = eglLib().getFunction<PROC_EGL_eglGetError>("eglGetError");
	egl::GetProcAddress = eglLib().getFunction<PROC_EGL_eglGetProcAddress>("eglGetProcAddress");
	egl::Initialize = eglLib().getFunction<PROC_EGL_eglInitialize>("eglInitialize");
	egl::MakeCurrent = eglLib().getFunction<PROC_EGL_eglMakeCurrent>("eglMakeCurrent");
	egl::QueryContext = eglLib().getFunction<PROC_EGL_eglQueryContext>("eglQueryContext");
	egl::QueryString = eglLib().getFunction<PROC_EGL_eglQueryString>("eglQueryString");
	egl::QuerySurface = eglLib().getFunction<PROC_EGL_eglQuerySurface>("eglQuerySurface");
	egl::SwapBuffers = eglLib().getFunction<PROC_EGL_eglSwapBuffers>("eglSwapBuffers");
	egl::Terminate = eglLib().getFunction<PROC_EGL_eglTerminate>("eglTerminate");
	egl::WaitGL = eglLib().getFunction<PROC_EGL_eglWaitGL>("eglWaitGL");
	egl::WaitNative = eglLib().getFunction<PROC_EGL_eglWaitNative>("eglWaitNative");
	egl::BindTexImage = eglLib().getFunction<PROC_EGL_eglBindTexImage>("eglBindTexImage");
	egl::ReleaseTexImage = eglLib().getFunction<PROC_EGL_eglReleaseTexImage>("eglReleaseTexImage");
	egl::SurfaceAttrib = eglLib().getFunction<PROC_EGL_eglSurfaceAttrib>("eglSurfaceAttrib");
	egl::SwapInterval = eglLib().getFunction<PROC_EGL_eglSwapInterval>("eglSwapInterval");
	egl::BindAPI = eglLib().getFunction<PROC_EGL_eglBindAPI>("eglBindAPI");
	egl::CreatePbufferFromClientBuffer = eglLib().getFunction<PROC_EGL_eglCreatePbufferFromClientBuffer>("eglCreatePbufferFromClientBuffer");
	egl::QueryAPI = eglLib().getFunction<PROC_EGL_eglQueryAPI>("eglQueryAPI");
	egl::ReleaseThread = eglLib().getFunction<PROC_EGL_eglReleaseThread>("eglReleaseThread");
	egl::WaitClient = eglLib().getFunction<PROC_EGL_eglWaitClient>("eglWaitClient");
	return true;
}

//STANDARD GUARANTEES ZERO-INITIALIZATION.
PROC_EGL_eglChooseConfig egl::ChooseConfig;
PROC_EGL_eglCopyBuffers egl::CopyBuffers;
PROC_EGL_eglCreateContext egl::CreateContext;
PROC_EGL_eglCreatePbufferSurface egl::CreatePbufferSurface;
PROC_EGL_eglCreatePixmapSurface egl::CreatePixmapSurface;
PROC_EGL_eglCreateWindowSurface egl::CreateWindowSurface;
PROC_EGL_eglDestroyContext egl::DestroyContext;
PROC_EGL_eglDestroySurface egl::DestroySurface;
PROC_EGL_eglGetConfigAttrib egl::GetConfigAttrib;
PROC_EGL_eglGetConfigs egl::GetConfigs;
PROC_EGL_eglGetCurrentContext egl::GetCurrentContext;
PROC_EGL_eglGetCurrentDisplay egl::GetCurrentDisplay;
PROC_EGL_eglGetCurrentSurface egl::GetCurrentSurface;
PROC_EGL_eglGetDisplay egl::GetDisplay;
PROC_EGL_eglGetError egl::GetError;
PROC_EGL_eglGetProcAddress egl::GetProcAddress;
PROC_EGL_eglInitialize egl::Initialize;
PROC_EGL_eglMakeCurrent egl::MakeCurrent;
PROC_EGL_eglQueryContext egl::QueryContext;
PROC_EGL_eglQueryString egl::QueryString;
PROC_EGL_eglQuerySurface egl::QuerySurface;
PROC_EGL_eglSwapBuffers egl::SwapBuffers;
PROC_EGL_eglTerminate egl::Terminate;
PROC_EGL_eglWaitGL egl::WaitGL;
PROC_EGL_eglWaitNative egl::WaitNative;
PROC_EGL_eglBindTexImage egl::BindTexImage;
PROC_EGL_eglReleaseTexImage egl::ReleaseTexImage;
PROC_EGL_eglSurfaceAttrib egl::SurfaceAttrib;
PROC_EGL_eglSwapInterval egl::SwapInterval;
PROC_EGL_eglBindAPI egl::BindAPI;
PROC_EGL_eglCreatePbufferFromClientBuffer egl::CreatePbufferFromClientBuffer;
PROC_EGL_eglQueryAPI egl::QueryAPI;
PROC_EGL_eglReleaseThread egl::ReleaseThread;
PROC_EGL_eglWaitClient egl::WaitClient;

bool egl::isEglExtensionSupported(const char* extension)
{
	EGLDisplay dpy = egl::GetCurrentDisplay();
	// The recommended technique for querying EGL extensions matches OpenGLES;
	// from http://opengl.org/resources/features/OGLextensions/
	const char* extensions = NULL;
	const char* start;
	char* terminator;

	/* Extension names should not have spaces. */
	char* where = (char*)strchr(extension, ' ');
	if (where || *extension == '\0')
	{
		return 0;
	}

	extensions = egl::QueryString(dpy, EGL_EXTENSIONS);
	if (!extensions)
	{
		return false;
	}

	/* It takes a bit of care to be fool-proof about parsing the
	OpenGL extensions string. Don't be fooled by sub-strings, etc. */
	start = extensions;
	for (;;)
	{
		where = (char*)strstr((const char*)start, extension);
		if (!where)
		{
			break;
		}
		terminator = where + strlen(extension);
		if ((where == start || *(where - 1) == ' ') && (*terminator == ' ' || *terminator == '\0'))
		{
			return true;
		}
		start = terminator;
	}
	return false;
}


bool egl::isEglExtensionSupported(EGLDisplay dpy, const char* extension)
{
	// The recommended technique for querying EGL extensions matches OpenGLES;
	// from http://opengl.org/resources/features/OGLextensions/
	const char* extensions = NULL;
	const char* start;
	char* terminator;

	/* Extension names should not have spaces. */
	char* where = (char*)strchr(extension, ' ');
	if (where || *extension == '\0')
	{
		return 0;
	}

	extensions = egl::QueryString(dpy, EGL_EXTENSIONS);
	if (!extensions)
	{
		return false;
	}

	/* It takes a bit of care to be fool-proof about parsing the
	OpenGL extensions string. Don't be fooled by sub-strings, etc. */
	start = extensions;
	for (;;)
	{
		where = (char*)strstr((const char*)start, extension);
		if (!where)
		{
			break;
		}
		terminator = where + strlen(extension);
		if ((where == start || *(where - 1) == ' ') && (*terminator == ' ' || *terminator == '\0'))
		{
			return true;
		}
		start = terminator;
	}
	return false;
}


namespace pvr {
namespace native {
void* glueGetProcAddress(const char* functionName)
{
	return egl::GetProcAddress(functionName);
}
}
}
//!\endcond