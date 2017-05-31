/*!
\brief Contains the typedef's required for the EGL bindings.
\file PVRNativeApi/EGL/ApiEgl.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN

#pragma once
#include <EGL/egl.h>
#include <EGL/eglext.h>

#ifdef None
#undef None
#endif

#ifdef Success
#undef Success
#endif

#ifdef Always
#undef Always
#endif

#define PVR_APIENTRY KHRONOS_APIENTRY

typedef void* EXT_PROC;

#ifndef TARGET_OS_IPHONE
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglChooseConfig)(EGLDisplay, const EGLint*, EGLConfig*, EGLint, EGLint*);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglCopyBuffers)(EGLDisplay, EGLSurface, EGLNativePixmapType);
typedef EGLContext(PVR_APIENTRY* PROC_EGL_eglCreateContext)(EGLDisplay, EGLConfig, EGLContext, const EGLint*);

typedef EGLSurface(PVR_APIENTRY* PROC_EGL_eglCreatePbufferSurface)(EGLDisplay, EGLConfig, const EGLint*);
typedef EGLSurface(PVR_APIENTRY* PROC_EGL_eglCreatePixmapSurface)(EGLDisplay, EGLConfig, EGLNativePixmapType, const EGLint*);
typedef EGLSurface(PVR_APIENTRY* PROC_EGL_eglCreateWindowSurface)(EGLDisplay, EGLConfig, EGLNativeWindowType, const EGLint*);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglDestroyContext)(EGLDisplay, EGLContext);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglDestroySurface)(EGLDisplay, EGLSurface);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglGetConfigAttrib)(EGLDisplay, EGLConfig, EGLint, EGLint*);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglGetConfigs)(EGLDisplay, EGLConfig*, EGLint, EGLint*);
typedef EGLContext(PVR_APIENTRY* PROC_EGL_eglGetCurrentContext)();
typedef EGLDisplay(PVR_APIENTRY* PROC_EGL_eglGetCurrentDisplay)();
typedef EGLSurface(PVR_APIENTRY* PROC_EGL_eglGetCurrentSurface)(EGLint);
typedef EGLDisplay(PVR_APIENTRY* PROC_EGL_eglGetDisplay)(EGLNativeDisplayType);
typedef EGLint(PVR_APIENTRY* PROC_EGL_eglGetError)();
typedef EXT_PROC(PVR_APIENTRY* PROC_EGL_eglGetProcAddress)(const char*);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglInitialize)(EGLDisplay, EGLint*, EGLint*);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglMakeCurrent)(EGLDisplay, EGLSurface, EGLSurface, EGLContext);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglQueryContext)(EGLDisplay, EGLContext, EGLint, EGLint*);
typedef const char* (PVR_APIENTRY* PROC_EGL_eglQueryString)(EGLDisplay, EGLint);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglQuerySurface)(EGLDisplay, EGLSurface, EGLint, EGLint*);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglSwapBuffers)(EGLDisplay, EGLSurface);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglTerminate)(EGLDisplay);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglWaitGL)();
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglWaitNative)(EGLint);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglBindTexImage)(EGLDisplay, EGLSurface, EGLint);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglReleaseTexImage)(EGLDisplay, EGLSurface, EGLint);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglSurfaceAttrib)(EGLDisplay, EGLSurface, EGLint, EGLint);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglSwapInterval)(EGLDisplay, EGLint);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglBindAPI)(EGLenum);
typedef EGLSurface(PVR_APIENTRY* PROC_EGL_eglCreatePbufferFromClientBuffer)(EGLDisplay, EGLenum, EGLClientBuffer, EGLConfig, const EGLint*);
typedef EGLenum(PVR_APIENTRY* PROC_EGL_eglQueryAPI)();
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglReleaseThread)();
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglWaitClient)();

// EGL extensions (Should these use the PROC_EXT prefix like the gl extensions? I'm going with no as they'll be unique to EGL)
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglSetSwapRectangleANDROID)(EGLDisplay dpy, EGLSurface draw, EGLint left, EGLint top, EGLint width, EGLint height);
typedef EGLClientBuffer(PVR_APIENTRY* PROC_EGL_eglGetRenderBufferANDROID)(EGLDisplay dpy, EGLSurface draw);

// KHR_lock_surface
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglLockSurfaceKHR)(EGLDisplay display, EGLSurface surface, const EGLint* attrib_list);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglUnlockSurfaceKHR)(EGLDisplay display, EGLSurface surface);

// EGL_KHR_image_base
typedef EGLImageKHR(PVR_APIENTRY* PROC_EGL_eglCreateImageKHR)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint* attrib_list);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglDestroyImageKHR)(EGLDisplay dpy, EGLImageKHR image);

// EGL_KHR_reusable_sync
typedef EGLSyncKHR(PVR_APIENTRY* PROC_EGL_eglCreateSyncKHR)(EGLDisplay dpy, EGLenum type, const EGLint* attrib_list);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglDestroySyncKHR)(EGLDisplay dpy, EGLSyncKHR sync);
typedef EGLint(PVR_APIENTRY* PROC_EGL_eglClientWaitSyncKHR)(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglSignalSyncKHR)(EGLDisplay dpy, EGLSyncKHR sync, EGLenum mode);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglGetSyncAttribKHR)(EGLDisplay dpy, EGLSyncKHR sync, EGLint attribute, EGLint* value);

// EGL_ANDROID_blob_cache
typedef khronos_ssize_t EGLsizeiANDROID;
typedef void (*PROC_EGL_EGLSetBlobFuncANDROID)(const void* key, EGLsizeiANDROID keySize, const void* value, EGLsizeiANDROID valueSize);
typedef EGLsizeiANDROID(*PROC_EGL_EGLGetBlobFuncANDROID)(const void* key, EGLsizeiANDROID keySize, void* value, EGLsizeiANDROID valueSize);
typedef void (PVR_APIENTRY* PROC_EGL_eglSetBlobCacheFuncsANDROID)(EGLDisplay dpy, PROC_EGL_EGLSetBlobFuncANDROID set, PROC_EGL_EGLGetBlobFuncANDROID get);

// EGL_ANDROID_native_fence_sync
typedef EGLint(PVR_APIENTRY* PROC_EGL_eglDupNativeFenceFDANDROID)(EGLDisplay dpy, EGLSyncKHR sync);
typedef EGLint(PVR_APIENTRY* PROC_EGL_eglWaitSyncANDROID)(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags);

// EGL_KHR_wait_sync
typedef EGLint(PVR_APIENTRY* PROC_EGL_eglWaitSyncKHR)(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags);

// IMG
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglHibernateProcessIMG)(void);
typedef EGLBoolean(PVR_APIENTRY* PROC_EGL_eglAwakenProcessIMG)(void);

// EGL_EXT_swap_buffers_with_damage
typedef EGLBoolean(KHRONOS_APIENTRY * PROC_EGL_eglSwapBuffersWithDamageEXT)(EGLDisplay, EGLSurface, EGLint*, EGLint);
#endif
//!\endcond