/*!*********************************************************************************************************************
\file         PVRPlatformGlue/EGL/EglPlatformHandles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains platform objects required for EGL (EGLDisplay, EGLSurface, EGLContext etc).
***********************************************************************************************************************/
#pragma once

#include "PVRCore/CoreIncludes.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace pvr {
namespace platform {
/*!*********************************************************************************************************************
\brief     EGL display type.
***********************************************************************************************************************/
typedef EGLNativeDisplayType NativeDisplay;

/*!*********************************************************************************************************************
\brief     EGL window type.
***********************************************************************************************************************/
typedef EGLNativeWindowType NativeWindow;

/*!*********************************************************************************************************************
\brief     Forward-declare and smart pointer friendly handle to all the objects that EGL needs to identify a rendering context.
***********************************************************************************************************************/
struct NativePlatformHandles_
{
	EGLDisplay display;
	EGLSurface drawSurface;
	EGLSurface readSurface;
	EGLContext context;

	NativePlatformHandles_() : display(EGL_NO_DISPLAY), drawSurface(EGL_NO_SURFACE), readSurface(EGL_NO_SURFACE),
		context(EGL_NO_CONTEXT) {}
};

/*!*********************************************************************************************************************
\brief     Forward-declare and smart pointer friendly handle to an EGL display
***********************************************************************************************************************/
struct NativeDisplayHandle_
{
	NativeDisplay nativeDisplay;
	operator NativeDisplay& () { return nativeDisplay; }
	operator const NativeDisplay& () const { return nativeDisplay; }
};

/*!*********************************************************************************************************************
\brief     Forward-declare and smart pointer friendly handle to an EGL window
***********************************************************************************************************************/
struct NativeWindowHandle_
{
	NativeWindow nativeWindow;
	operator NativeWindow& () { return nativeWindow; }
	operator const NativeWindow& () const { return nativeWindow; }
};

}
}
