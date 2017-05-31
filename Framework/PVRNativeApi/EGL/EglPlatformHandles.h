/*!
\brief Contains platform objects required for EGL (EGLDisplay, EGLSurface, EGLContext etc).
\file PVRNativeApi/EGL/EglPlatformHandles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/CoreIncludes.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace pvr {
namespace platform {
/// <summary>EGL display type.</summary>
typedef EGLNativeDisplayType NativeDisplay;

/// <summary>EGL window type.</summary>
typedef EGLNativeWindowType NativeWindow;

/// <summary>Forward-declare and smart pointer friendly handle to all the objects that EGL needs to identify a rendering
/// context.</summary>
struct NativePlatformHandles_
{
	EGLDisplay display;
	EGLSurface drawSurface;
	EGLSurface readSurface;
	EGLContext context;

	NativePlatformHandles_() : display(EGL_NO_DISPLAY), drawSurface(EGL_NO_SURFACE), readSurface(EGL_NO_SURFACE),
		context(EGL_NO_CONTEXT) {}
};


/// <summary>A structure containing the native handles defining a Shared Context, i.e. an EGL context that is suitable
/// for the Framework requires to use to upload textures and perform other functions in a different thread. Contains an
/// EGL context and the EGL p-buffer surface it is tied to.</summary>
struct NativeSharedPlatformHandles_
{
	EGLContext uploadingContext;
	EGLSurface pBufferSurface;

	NativeSharedPlatformHandles_() : pBufferSurface(EGL_NO_SURFACE), uploadingContext(EGL_NO_CONTEXT) {}
};
/// <summary>Forward-declare and smart pointer friendly handle to an EGL display</summary>
struct NativeDisplayHandle_
{
	NativeDisplay nativeDisplay;
	operator NativeDisplay& () { return nativeDisplay; }
	operator const NativeDisplay& () const { return nativeDisplay; }
};

/// <summary>Forward-declare and smart pointer friendly handle to an EGL window</summary>
struct NativeWindowHandle_
{
	NativeWindow nativeWindow;
	operator NativeWindow& () { return nativeWindow; }
	operator const NativeWindow& () const { return nativeWindow; }
};

}
}