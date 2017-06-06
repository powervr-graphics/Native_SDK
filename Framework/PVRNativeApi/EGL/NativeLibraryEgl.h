/*!
\brief Contains the class (egl) with the EGL function pointers loaded on application start. Include this header if you
wish to directly call EGL functions in your application.
\file PVRNativeApi/EGL/NativeLibraryEGL.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRNativeApi/EGL/ApiEgl.h"

/// <summary>This class's static members are function pointers to the EGL functions. The Shell kicks off their
/// initialisation on before context creation. Use the class name like a namespace: egl::ChooseConfig.</summary>
class egl
{
public:
	static bool initEgl(); //!<Automatically called just before context initialisation.
	static bool isEglExtensionSupported(const char* extension);//!<Check for the presense of an EGL extension in the current display in the EGL extension string
	static bool isEglExtensionSupported(EGLDisplay display, const char* extension);//!<Check for the presense of an EGL extension for the specified display in the EGL extension string
	static bool isApiExtensionSupported(const char* extension);//!<Check for the presense of an EGL extension for the current context in the EGL extension string

//!\cond NO_DOXYGEN

	static PROC_EGL_eglChooseConfig ChooseConfig;
	static PROC_EGL_eglCopyBuffers CopyBuffers;
	static PROC_EGL_eglCreateContext CreateContext;
	static PROC_EGL_eglCreatePbufferSurface CreatePbufferSurface;
	static PROC_EGL_eglCreatePixmapSurface CreatePixmapSurface;
	static PROC_EGL_eglCreateWindowSurface CreateWindowSurface;
	static PROC_EGL_eglDestroyContext DestroyContext;
	static PROC_EGL_eglDestroySurface DestroySurface;
	static PROC_EGL_eglGetConfigAttrib GetConfigAttrib;
	static PROC_EGL_eglGetConfigs GetConfigs;
	static PROC_EGL_eglGetCurrentContext GetCurrentContext;
	static PROC_EGL_eglGetCurrentDisplay GetCurrentDisplay;
	static PROC_EGL_eglGetCurrentSurface GetCurrentSurface;
	static PROC_EGL_eglGetDisplay GetDisplay;
	static PROC_EGL_eglGetError GetError;
	static PROC_EGL_eglGetProcAddress GetProcAddress;
	static PROC_EGL_eglInitialize Initialize;
	static PROC_EGL_eglMakeCurrent MakeCurrent;
	static PROC_EGL_eglQueryContext QueryContext;
	static PROC_EGL_eglQueryString QueryString;
	static PROC_EGL_eglQuerySurface QuerySurface;
	static PROC_EGL_eglSwapBuffers SwapBuffers;
	static PROC_EGL_eglTerminate Terminate;
	static PROC_EGL_eglWaitGL WaitGL;
	static PROC_EGL_eglWaitNative WaitNative;
	static PROC_EGL_eglBindTexImage BindTexImage;
	static PROC_EGL_eglReleaseTexImage ReleaseTexImage;
	static PROC_EGL_eglSurfaceAttrib SurfaceAttrib;
	static PROC_EGL_eglSwapInterval SwapInterval;
	static PROC_EGL_eglBindAPI BindAPI;
	static PROC_EGL_eglCreatePbufferFromClientBuffer CreatePbufferFromClientBuffer;
	static PROC_EGL_eglQueryAPI QueryAPI;
	static PROC_EGL_eglReleaseThread ReleaseThread;
	static PROC_EGL_eglWaitClient WaitClient;
//!\endcond
};