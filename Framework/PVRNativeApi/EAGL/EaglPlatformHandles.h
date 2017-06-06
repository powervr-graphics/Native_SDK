/*!
\brief Native object handles (display, window, view etc.) for the EAGL (iOS) implementation of PVRNativeApi.
\file PVRNativeApi/EAGL/EaglPlatformHandles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#pragma once

#include "PVRCore/CoreIncludes.h"
#include "OpenGLES/ES2/gl.h"
#include "OpenGLES/ES2/glext.h"
#import <UIKit/UIKit.h>
namespace pvr {
namespace platform {
typedef void* NativeDisplay; //!< void pointer representing the OS Display
typedef void* NativeWindow;  //!< void pointer representing the OS Window


//Objective-C Type Definitions
typedef void		VoidUIView;
typedef void		VoidUIApplicationDelegate;

typedef VoidUIApplicationDelegate*       OSApplication;
typedef void*		OSDisplay;
typedef VoidUIView*	OSWindow;
typedef void*		OSSurface;
typedef void*		OSDATA;

struct NativePlatformHandles_
{
	EAGLContext* context;
	VoidUIView* view;

	GLint numDiscardAttachments = 0;
	GLenum discardAttachments[3];
	GLuint framebuffer = 0;
	GLuint renderbuffer = 0;
	GLuint depthBuffer = 0;

	GLuint msaaFrameBuffer = 0;
	GLuint msaaColorBuffer = 0;
	GLuint msaaDepthBuffer = 0;

	NativePlatformHandles_() {}
};
    
struct NativeSharedPlatformHandles_
{
    EAGLContext* uploadingContext;
    VoidUIView* pBufferSurface;
};
    

/*! \brief Forward-declare friendly container for the native display */
struct NativeDisplayHandle_
{
	NativeDisplay nativeDisplay;
	operator NativeDisplay& () { return nativeDisplay; }
	operator const NativeDisplay& () const { return nativeDisplay; }
};
/*! \brief Forward-declare friendly container for the native window */
struct NativeWindowHandle_
{
	NativeWindow nativeWindow;
	operator NativeWindow& () { return nativeWindow; }
	operator const NativeWindow& () const { return nativeWindow; }
};
}
}
//!\endcond