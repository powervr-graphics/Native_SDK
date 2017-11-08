#pragma once
#ifdef TARGET_OS_IPHONE
#ifdef __OBJC__
#import <UIKit/UIKit.h>
#else
class EAGLContext;
#endif
#else
#if defined( _WIN32 )
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#elif defined(WAYLAND)
#include <wayland-egl.h>
#endif

#define EGL_NO_PROTOTYPES
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include <stdio.h>
#include <cstring>
#ifndef __GNUC__
#pragma region Load Library defs
#endif

#if !defined(Log_Error)
#ifndef PVR_LOG_ERROR
#define Log_Error(...) do { printf(__VA_ARGS__); }while(false)
#else
#define Log_Error(...) do { PVR_LOG_ERROR(__VA_ARGS__); }while(false)
#endif
#endif
#if !defined(Log_Info)
#ifndef PVR_LOG_INFO
#define Log_Info(...) do { printf(__VA_ARGS__); }while(false)
#else
#define Log_Info(...) do { PVR_LOG_INFO(__VA_ARGS__); }while(false)
#endif
#endif
namespace egl {
namespace internal {
#ifdef _WIN32
static const char* libName = "libEGL.dll";
#elif defined(__APPLE__)
static const char* libName = "libEGL.dylib";
#else
static const char* libName = "libEGL.so";
#endif
}
}
#if defined( _WIN32 )
namespace egl {
namespace internal {
typedef HINSTANCE LIBTYPE;

inline LIBTYPE OpenLibrary(const char* pszPath)
{
#if defined(_UNICODE) // UNDER_CE
	if (!pszPath)
	{
		return NULL;
	}

	// Get full path of executable
	wchar_t pszPathW[_MAX_PATH];

	// Convert char to wchar
	DWORD i = 0;

	for (i = 0; i <= strlen(pszPath); ++i)
	{
		pszPathW[i] = static_cast<wchar_t>(pszPath[i]);
	}

	pszPathW[i] = '\0';
	return  LoadLibraryW(pszPathW);
#else
	return  LoadLibraryA(pszPath);
#endif
}

inline void CloseLibrary(LIBTYPE hLib)
{
	FreeLibrary(hLib);
}

inline void* GetLibFunction(LIBTYPE hLib, const char* pszName)
{
	if (hLib)
	{
#if defined(UNDER_CE)
		return  win32::GetProcAddressA(hLib, pszName);
#else
		return  GetProcAddress(hLib, pszName);
#endif
	}
	return NULL;
}
}
}
#elif defined( __linux__ ) || defined(__QNXNTO__) || defined(__APPLE__)
#if defined(__APPLE__)
void* OpenFramework(const char* pszPath);
#endif
#include <unistd.h>
#include <dlfcn.h>
namespace egl {
namespace internal {
typedef void* LIBTYPE;

#if defined(__APPLE__)
inline LIBTYPE OpenLibrary(const char* pszPath)
{
	return OpenFramework(pszPath); // An objective-C function that uses dlopen
}

#else

inline LIBTYPE OpenLibrary(const char* pszPath)
{
	LIBTYPE lt = dlopen(pszPath, RTLD_LAZY | RTLD_GLOBAL);

	if (!lt)
	{
		char pathMod[256];
		strcpy(pathMod, "./");
		strcat(pathMod, pszPath);

		lt = dlopen(pathMod, RTLD_LAZY | RTLD_GLOBAL);
	}
	return lt;
}
#endif

inline void CloseLibrary(LIBTYPE hLib) { dlclose(hLib); }

inline void* GetLibFunction(LIBTYPE hLib, const char* pszName)
{
	if (hLib)
	{
		void* fnct = dlsym(hLib, pszName);
		return fnct;
	}
	return NULL;
}
}
}
#elif defined (ANDROID)

#include <dlfcn.h>
namespace egl {
namespace internal {
inline LIBTYPE OpenLibrary(const char* pszPath)
{
	LIBTYPE lt = dlopen(pszPath, RTLD_LAZY | RTLD_GLOBAL);
	return lt;
}

inline void CloseLibrary(LIBTYPE hLib) { dlclose(hLib); }

inline void* GetLibFunction(LIBTYPE hLib, const char* pszName)
{
	void* fnct = dlsym(hLib, pszName);
	return fnct;
}
}
}
#else
#error Unsupported platform
#endif
namespace egl {
namespace internal {
inline void* GetLibFunctionChecked(LIBTYPE hLib, const char* name)
{
	void* func = GetLibFunction(hLib, name);
	if (!func)
	{
		Log_Error("Failed to load function [%s] from EGL library.\n", name);
	}
	return func;
}
}
}
#ifndef __GNUC__
#pragma endregion
#endif

#ifndef __GNUC__
#pragma region EGL_CORE
#endif

namespace egl {
namespace internal {
namespace EglFuncName {
enum EglFunctionName
{
	ChooseConfig, CopyBuffers, CreateContext, CreatePbufferSurface, CreatePixmapSurface, CreateWindowSurface, DestroyContext, DestroySurface, GetConfigAttrib,
	GetConfigs, GetCurrentDisplay, GetCurrentSurface, GetDisplay, GetError, GetProcAddress, Initialize, MakeCurrent, QueryContext, QueryString, QuerySurface,
	SwapBuffers, Terminate, WaitGL, WaitNative, BindTexImage, ReleaseTexImage, SurfaceAttrib, SwapInterval, BindAPI, QueryAPI, CreatePbufferFromClientBuffer,
	ReleaseThread, WaitClient, GetCurrentContext, CreateSync, DestroySync, ClientWaitSync, GetSyncAttrib, CreateImage, DestroyImage, GetPlatformDisplay,
	CreatePlatformWindowSurface, CreatePlatformPixmapSurface, WaitSync,
	NUMBER_OF_EGL_FUNCTIONS
};
}

inline void* getEglFunction(::egl::internal::EglFuncName::EglFunctionName funcname)
{
	static void* FunctionTable[EglFuncName::NUMBER_OF_EGL_FUNCTIONS];

	//  GET FUNCTION POINTERS --- ONCE!!!! ///
	if (!FunctionTable[0])
	{
		LIBTYPE lib = OpenLibrary(egl::internal::libName);
		if (!lib)
		{
			Log_Error("EGL Bindings: Failed to open library %s\n", egl::internal::libName);
		}
		else
		{
			Log_Info("EGL Bindings: Successfully loaded library %s\n", egl::internal::libName);
		}
		FunctionTable[EglFuncName::ChooseConfig] = GetLibFunctionChecked(lib, "eglChooseConfig");
		FunctionTable[EglFuncName::CopyBuffers] = GetLibFunctionChecked(lib, "eglCopyBuffers");
		FunctionTable[EglFuncName::CreateContext] = GetLibFunctionChecked(lib, "eglCreateContext");
		FunctionTable[EglFuncName::CreatePbufferSurface] = GetLibFunctionChecked(lib, "eglCreatePbufferSurface");
		FunctionTable[EglFuncName::CreatePixmapSurface] = GetLibFunctionChecked(lib, "eglCreatePixmapSurface");
		FunctionTable[EglFuncName::CreateWindowSurface] = GetLibFunctionChecked(lib, "eglCreateWindowSurface");
		FunctionTable[EglFuncName::DestroyContext] = GetLibFunctionChecked(lib, "eglDestroyContext");
		FunctionTable[EglFuncName::DestroySurface] = GetLibFunctionChecked(lib, "eglDestroySurface");
		FunctionTable[EglFuncName::GetConfigAttrib] = GetLibFunctionChecked(lib, "eglGetConfigAttrib");
		FunctionTable[EglFuncName::GetConfigs] = GetLibFunctionChecked(lib, "eglGetConfigs");
		FunctionTable[EglFuncName::GetCurrentDisplay] = GetLibFunctionChecked(lib, "eglGetCurrentDisplay");
		FunctionTable[EglFuncName::GetCurrentSurface] = GetLibFunctionChecked(lib, "eglGetCurrentSurface");
		FunctionTable[EglFuncName::GetDisplay] = GetLibFunctionChecked(lib, "eglGetDisplay");
		FunctionTable[EglFuncName::GetError] = GetLibFunctionChecked(lib, "eglGetError");
		FunctionTable[EglFuncName::GetProcAddress] = GetLibFunctionChecked(lib, "eglGetProcAddress");
		FunctionTable[EglFuncName::Initialize] = GetLibFunctionChecked(lib, "eglInitialize");
		FunctionTable[EglFuncName::MakeCurrent] = GetLibFunctionChecked(lib, "eglMakeCurrent");
		FunctionTable[EglFuncName::QueryContext] = GetLibFunctionChecked(lib, "eglQueryContext");
		FunctionTable[EglFuncName::QueryString] = GetLibFunctionChecked(lib, "eglQueryString");
		FunctionTable[EglFuncName::QuerySurface] = GetLibFunctionChecked(lib, "eglQuerySurface");
		FunctionTable[EglFuncName::SwapBuffers] = GetLibFunctionChecked(lib, "eglSwapBuffers");
		FunctionTable[EglFuncName::Terminate] = GetLibFunctionChecked(lib, "eglTerminate");
		FunctionTable[EglFuncName::WaitGL] = GetLibFunctionChecked(lib, "eglWaitGL");
		FunctionTable[EglFuncName::WaitNative] = GetLibFunctionChecked(lib, "eglWaitNative");
		FunctionTable[EglFuncName::BindTexImage] = GetLibFunctionChecked(lib, "eglBindTexImage");
		FunctionTable[EglFuncName::ReleaseTexImage] = GetLibFunctionChecked(lib, "eglReleaseTexImage");
		FunctionTable[EglFuncName::SurfaceAttrib] = GetLibFunctionChecked(lib, "eglSurfaceAttrib");
		FunctionTable[EglFuncName::SwapInterval] = GetLibFunctionChecked(lib, "eglSwapInterval");
		FunctionTable[EglFuncName::BindAPI] = GetLibFunctionChecked(lib, "eglBindAPI");
		FunctionTable[EglFuncName::QueryAPI] = GetLibFunctionChecked(lib, "eglQueryAPI");
		FunctionTable[EglFuncName::CreatePbufferFromClientBuffer] = GetLibFunctionChecked(lib, "eglCreatePbufferFromClientBuffer");
		FunctionTable[EglFuncName::ReleaseThread] = GetLibFunctionChecked(lib, "eglReleaseThread");
		FunctionTable[EglFuncName::WaitClient] = GetLibFunctionChecked(lib, "eglWaitClient");
		FunctionTable[EglFuncName::GetCurrentContext] = GetLibFunctionChecked(lib, "eglGetCurrentContext");
		FunctionTable[EglFuncName::CreateSync] = GetLibFunctionChecked(lib, "eglCreateSync");
		FunctionTable[EglFuncName::DestroySync] = GetLibFunctionChecked(lib, "eglDestroySync");
		FunctionTable[EglFuncName::ClientWaitSync] = GetLibFunctionChecked(lib, "eglClientWaitSync");
		FunctionTable[EglFuncName::GetSyncAttrib] = GetLibFunctionChecked(lib, "eglGetSyncAttrib");
		FunctionTable[EglFuncName::CreateImage] = GetLibFunctionChecked(lib, "eglCreateImage");
		FunctionTable[EglFuncName::DestroyImage] = GetLibFunctionChecked(lib, "eglDestroyImage");
		FunctionTable[EglFuncName::GetPlatformDisplay] = GetLibFunctionChecked(lib, "eglGetPlatformDisplay");
		FunctionTable[EglFuncName::CreatePlatformWindowSurface] = GetLibFunctionChecked(lib, "eglCreatePlatformWindowSurface");
		FunctionTable[EglFuncName::CreatePlatformPixmapSurface] = GetLibFunctionChecked(lib, "eglCreatePlatformPixmapSurface");
		FunctionTable[EglFuncName::WaitSync] = GetLibFunctionChecked(lib, "eglWaitSync");
	}
	return FunctionTable[funcname];
}
}
}

#ifndef DYNAMICEGL_NO_NAMESPACE
#define DYNAMICEGL_FUNCTION(name) EGLAPIENTRY name
#define DYNAMICEGL_CALL_FUNCTION(name) ::egl::name
#else
#define DYNAMICEGL_FUNCTION(name) EGLAPIENTRY egl##name
#define DYNAMICEGL_CALL_FUNCTION(name) egl##name
#endif


#ifndef DYNAMICEGL_NO_NAMESPACE
namespace egl {
#endif

#if defined(__QNXNTO__)
EGLBoolean DYNAMICEGL_FUNCTION(GetConfigAttrib)(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint* value);
#endif

inline EGLBoolean DYNAMICEGL_FUNCTION(ChooseConfig)(EGLDisplay dpy, const EGLint* attrib_list, EGLConfig* configs, EGLint config_size, EGLint* num_config)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_ChooseConfig)(EGLDisplay dpy, const EGLint * attrib_list, EGLConfig * configs, EGLint config_size, EGLint * num_config);
	static PROC_EGL_ChooseConfig _ChooseConfig = (PROC_EGL_ChooseConfig)::egl::internal::getEglFunction(::egl::internal::EglFuncName::ChooseConfig);
	return _ChooseConfig(dpy, attrib_list, configs, config_size, num_config);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(CopyBuffers)(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_CopyBuffers)(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target);
	static PROC_EGL_CopyBuffers _CopyBuffers = (PROC_EGL_CopyBuffers)::egl::internal::getEglFunction(::egl::internal::EglFuncName::CopyBuffers);
	return _CopyBuffers(dpy, surface, target);
}
inline EGLContext DYNAMICEGL_FUNCTION(CreateContext)(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint* attrib_list)
{
	typedef EGLContext(EGLAPIENTRY * PROC_EGL_CreateContext)(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint * attrib_list);
	static PROC_EGL_CreateContext _CreateContext = (PROC_EGL_CreateContext)::egl::internal::getEglFunction(::egl::internal::EglFuncName::CreateContext);

	EGLContext context = _CreateContext(dpy, config, share_context, attrib_list);

#if defined(__QNXNTO__)
	/*
	  On QNX NTO the libGLESv2 library needs to be loaded before the first eglMakeCurrent call
	*/
	static bool once = false;
	if (!once)
	{
		if (context != EGL_NO_CONTEXT)
		{
			EGLint type = 0;
			GetConfigAttrib(dpy, config, EGL_RENDERABLE_TYPE, &type);

			if ((type & EGL_OPENGL_ES2_BIT) == EGL_OPENGL_ES2_BIT)
			{
				Log_Info("EGL Bindings: Preloading libGLESv2.so\n");
				egl::internal::OpenLibrary("libGLESv2.so");
				once = true;
			}
		}
	}
#endif
	return context;
}
inline EGLSurface DYNAMICEGL_FUNCTION(CreatePbufferSurface)(EGLDisplay dpy, EGLConfig config, const EGLint* attrib_list)
{
	typedef EGLSurface(EGLAPIENTRY * PROC_EGL_CreatePbufferSurface)(EGLDisplay dpy, EGLConfig config, const EGLint * attrib_list);
	static PROC_EGL_CreatePbufferSurface _CreatePbufferSurface = (PROC_EGL_CreatePbufferSurface)::egl::internal::getEglFunction(::egl::internal::EglFuncName::CreatePbufferSurface);
	return _CreatePbufferSurface(dpy, config, attrib_list);
}
inline EGLSurface DYNAMICEGL_FUNCTION(CreatePixmapSurface)(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint* attrib_list)
{
	typedef EGLSurface(EGLAPIENTRY * PROC_EGL_CreatePixmapSurface)(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint * attrib_list);
	static PROC_EGL_CreatePixmapSurface _CreatePixmapSurface = (PROC_EGL_CreatePixmapSurface)::egl::internal::getEglFunction(::egl::internal::EglFuncName::CreatePixmapSurface);
	return _CreatePixmapSurface(dpy, config,  pixmap, attrib_list);
}
inline EGLSurface DYNAMICEGL_FUNCTION(CreateWindowSurface)(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint* attrib_list)
{
	typedef EGLSurface(EGLAPIENTRY * PROC_EGL_CreateWindowSurface)(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint * attrib_list);
	static PROC_EGL_CreateWindowSurface _CreateWindowSurface = (PROC_EGL_CreateWindowSurface)::egl::internal::getEglFunction(::egl::internal::EglFuncName::CreateWindowSurface);
	return _CreateWindowSurface(dpy, config,  win, attrib_list);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(DestroyContext)(EGLDisplay dpy, EGLContext ctx)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_DestroyContext)(EGLDisplay dpy, EGLContext ctx);
	static PROC_EGL_DestroyContext _DestroyContext = (PROC_EGL_DestroyContext)::egl::internal::getEglFunction(::egl::internal::EglFuncName::DestroyContext);
	return _DestroyContext(dpy, ctx);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(DestroySurface)(EGLDisplay dpy, EGLSurface surface)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_DestroySurface)(EGLDisplay dpy, EGLSurface surface);
	static PROC_EGL_DestroySurface _DestroySurface = (PROC_EGL_DestroySurface)::egl::internal::getEglFunction(::egl::internal::EglFuncName::DestroySurface);
	return _DestroySurface(dpy, surface);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(GetConfigAttrib)(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint* value)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_GetConfigAttrib)(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint * value);
	static PROC_EGL_GetConfigAttrib _GetConfigAttrib = (PROC_EGL_GetConfigAttrib)::egl::internal::getEglFunction(::egl::internal::EglFuncName::GetConfigAttrib);
	return _GetConfigAttrib(dpy, config, attribute, value);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(GetConfigs)(EGLDisplay dpy, EGLConfig* configs, EGLint config_size, EGLint* num_config)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_GetConfigs)(EGLDisplay dpy, EGLConfig * configs, EGLint config_size, EGLint * num_config);
	static PROC_EGL_GetConfigs _GetConfigs = (PROC_EGL_GetConfigs)::egl::internal::getEglFunction(::egl::internal::EglFuncName::GetConfigs);
	return _GetConfigs(dpy, configs, config_size, num_config);
}
inline EGLDisplay DYNAMICEGL_FUNCTION(GetCurrentDisplay)(void)
{
	typedef EGLDisplay(EGLAPIENTRY * PROC_EGL_GetCurrentDisplay)(void);
	static PROC_EGL_GetCurrentDisplay _GetCurrentDisplay = (PROC_EGL_GetCurrentDisplay)::egl::internal::getEglFunction(::egl::internal::EglFuncName::GetCurrentDisplay);
	return _GetCurrentDisplay();
}
inline EGLSurface DYNAMICEGL_FUNCTION(GetCurrentSurface)(EGLint readdraw)
{
	typedef EGLSurface(EGLAPIENTRY * PROC_EGL_GetCurrentSurface)(EGLint readdraw);
	static PROC_EGL_GetCurrentSurface _GetCurrentSurface = (PROC_EGL_GetCurrentSurface)::egl::internal::getEglFunction(::egl::internal::EglFuncName::GetCurrentSurface);
	return _GetCurrentSurface(readdraw);
}
inline EGLDisplay DYNAMICEGL_FUNCTION(GetDisplay)(EGLNativeDisplayType display_id)
{
	typedef EGLDisplay(EGLAPIENTRY * PROC_EGL_GetDisplay)(EGLNativeDisplayType display_id);
	static PROC_EGL_GetDisplay _GetDisplay = (PROC_EGL_GetDisplay)::egl::internal::getEglFunction(::egl::internal::EglFuncName::GetDisplay);
	return _GetDisplay(display_id);
}
inline EGLint DYNAMICEGL_FUNCTION(GetError)(void)
{
	typedef EGLint(EGLAPIENTRY * PROC_EGL_GetError)(void);
	static PROC_EGL_GetError _GetError = (PROC_EGL_GetError)::egl::internal::getEglFunction(::egl::internal::EglFuncName::GetError);
	return _GetError();
}
inline __eglMustCastToProperFunctionPointerType DYNAMICEGL_FUNCTION(GetProcAddress)(const char* procname)
{
	typedef __eglMustCastToProperFunctionPointerType(EGLAPIENTRY * PROC_EGL_GetProcAddress)(const char* procname);
	static PROC_EGL_GetProcAddress _GetProcAddress = (PROC_EGL_GetProcAddress)::egl::internal::getEglFunction(::egl::internal::EglFuncName::GetProcAddress);
	return _GetProcAddress(procname);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(Initialize)(EGLDisplay dpy, EGLint* major, EGLint* minor)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_Initialize)(EGLDisplay dpy, EGLint * major, EGLint * minor);
	static PROC_EGL_Initialize _Initialize = (PROC_EGL_Initialize)::egl::internal::getEglFunction(::egl::internal::EglFuncName::Initialize);
	return _Initialize(dpy, major, minor);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(MakeCurrent)(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_MakeCurrent)(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
	static PROC_EGL_MakeCurrent _MakeCurrent = (PROC_EGL_MakeCurrent)::egl::internal::getEglFunction(::egl::internal::EglFuncName::MakeCurrent);
	return _MakeCurrent(dpy, draw, read, ctx);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(QueryContext)(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint* value)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_QueryContext)(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint * value);
	static PROC_EGL_QueryContext _QueryContext = (PROC_EGL_QueryContext)::egl::internal::getEglFunction(::egl::internal::EglFuncName::QueryContext);
	return _QueryContext(dpy, ctx, attribute, value);
}
inline const char* DYNAMICEGL_FUNCTION(QueryString)(EGLDisplay dpy, EGLint name)
{
	typedef const char* (EGLAPIENTRY * PROC_EGL_QueryString)(EGLDisplay dpy, EGLint name);
	static PROC_EGL_QueryString _QueryString = (PROC_EGL_QueryString)::egl::internal::getEglFunction(::egl::internal::EglFuncName::QueryString);
	return _QueryString(dpy, name);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(QuerySurface)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint* value)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_QuerySurface)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint * value);
	static PROC_EGL_QuerySurface _QuerySurface = (PROC_EGL_QuerySurface)::egl::internal::getEglFunction(::egl::internal::EglFuncName::QuerySurface);
	return _QuerySurface(dpy, surface, attribute, value);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(SwapBuffers)(EGLDisplay dpy, EGLSurface surface)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_SwapBuffers)(EGLDisplay dpy, EGLSurface surface);
	static PROC_EGL_SwapBuffers _SwapBuffers = (PROC_EGL_SwapBuffers)::egl::internal::getEglFunction(::egl::internal::EglFuncName::SwapBuffers);
	return _SwapBuffers(dpy, surface);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(Terminate)(EGLDisplay dpy)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_Terminate)(EGLDisplay dpy);
	static PROC_EGL_Terminate _Terminate = (PROC_EGL_Terminate)::egl::internal::getEglFunction(::egl::internal::EglFuncName::Terminate);
	return _Terminate(dpy);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(WaitGL)(void)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_WaitGL)(void);
	static PROC_EGL_WaitGL _WaitGL = (PROC_EGL_WaitGL)::egl::internal::getEglFunction(::egl::internal::EglFuncName::WaitGL);
	return _WaitGL();
}
inline EGLBoolean DYNAMICEGL_FUNCTION(WaitNative)(EGLint engine)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_WaitNative)(EGLint engine);
	static PROC_EGL_WaitNative _WaitNative = (PROC_EGL_WaitNative)::egl::internal::getEglFunction(::egl::internal::EglFuncName::WaitNative);
	return _WaitNative(engine);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(BindTexImage)(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_BindTexImage)(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
	static PROC_EGL_BindTexImage _BindTexImage = (PROC_EGL_BindTexImage)::egl::internal::getEglFunction(::egl::internal::EglFuncName::BindTexImage);
	return _BindTexImage(dpy, surface, buffer);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(ReleaseTexImage)(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_ReleaseTexImage)(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
	static PROC_EGL_ReleaseTexImage _ReleaseTexImage = (PROC_EGL_ReleaseTexImage)::egl::internal::getEglFunction(::egl::internal::EglFuncName::ReleaseTexImage);
	return _ReleaseTexImage(dpy, surface, buffer);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(SurfaceAttrib)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_SurfaceAttrib)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value);
	static PROC_EGL_SurfaceAttrib _SurfaceAttrib = (PROC_EGL_SurfaceAttrib)::egl::internal::getEglFunction(::egl::internal::EglFuncName::SurfaceAttrib);
	return _SurfaceAttrib(dpy, surface, attribute, value);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(SwapInterval)(EGLDisplay dpy, EGLint interval)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_SwapInterval)(EGLDisplay dpy, EGLint interval);
	static PROC_EGL_SwapInterval _SwapInterval = (PROC_EGL_SwapInterval)::egl::internal::getEglFunction(::egl::internal::EglFuncName::SwapInterval);
	return _SwapInterval(dpy, interval);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(BindAPI)(EGLenum api)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_BindAPI)(EGLenum api);
	static PROC_EGL_BindAPI _BindAPI = (PROC_EGL_BindAPI)::egl::internal::getEglFunction(::egl::internal::EglFuncName::BindAPI);
	return _BindAPI(api);
}
inline EGLenum DYNAMICEGL_FUNCTION(QueryAPI)(void)
{
	typedef EGLenum(EGLAPIENTRY * PROC_EGL_QueryAPI)(void);
	static PROC_EGL_QueryAPI _QueryAPI = (PROC_EGL_QueryAPI)::egl::internal::getEglFunction(::egl::internal::EglFuncName::QueryAPI);
	return _QueryAPI();
}
inline EGLSurface DYNAMICEGL_FUNCTION(CreatePbufferFromClientBuffer)(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint* attrib_list)
{
	typedef EGLSurface(EGLAPIENTRY * PROC_EGL_CreatePbufferFromClientBuffer)(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint * attrib_list);
	static PROC_EGL_CreatePbufferFromClientBuffer _CreatePbufferFromClientBuffer = (PROC_EGL_CreatePbufferFromClientBuffer)::egl::internal::getEglFunction(::egl::internal::EglFuncName::CreatePbufferFromClientBuffer);
	return _CreatePbufferFromClientBuffer(dpy, buftype, buffer, config, attrib_list);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(ReleaseThread)(void)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_ReleaseThread)(void);
	static PROC_EGL_ReleaseThread _ReleaseThread = (PROC_EGL_ReleaseThread)::egl::internal::getEglFunction(::egl::internal::EglFuncName::ReleaseThread);
	return _ReleaseThread();
}
inline EGLBoolean DYNAMICEGL_FUNCTION(WaitClient)(void)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_WaitClient)(void);
	static PROC_EGL_WaitClient _WaitClient = (PROC_EGL_WaitClient)::egl::internal::getEglFunction(::egl::internal::EglFuncName::WaitClient);
	return _WaitClient();
}
inline EGLContext DYNAMICEGL_FUNCTION(GetCurrentContext)(void)
{
	typedef EGLContext(EGLAPIENTRY * PROC_EGL_GetCurrentContext)(void);
	static PROC_EGL_GetCurrentContext _GetCurrentContext = (PROC_EGL_GetCurrentContext)::egl::internal::getEglFunction(::egl::internal::EglFuncName::GetCurrentContext);
	return _GetCurrentContext();
}
inline EGLSync DYNAMICEGL_FUNCTION(CreateSync)(EGLDisplay dpy, EGLenum type, const EGLAttrib* attrib_list)
{
	typedef EGLSync(EGLAPIENTRY * PROC_EGL_CreateSync)(EGLDisplay dpy, EGLenum type, const EGLAttrib * attrib_list);
	static PROC_EGL_CreateSync _CreateSync = (PROC_EGL_CreateSync)::egl::internal::getEglFunction(::egl::internal::EglFuncName::CreateSync);
	return _CreateSync(dpy, type, attrib_list);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(DestroySync)(EGLDisplay dpy, EGLSync sync)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_DestroySync)(EGLDisplay dpy, EGLSync sync);
	static PROC_EGL_DestroySync _DestroySync = (PROC_EGL_DestroySync)::egl::internal::getEglFunction(::egl::internal::EglFuncName::DestroySync);
	return _DestroySync(dpy, sync);
}
inline EGLint DYNAMICEGL_FUNCTION(ClientWaitSync)(EGLDisplay dpy, EGLSync sync, EGLint flags, EGLTime timeout)
{
	typedef EGLint(EGLAPIENTRY * PROC_EGL_ClientWaitSync)(EGLDisplay dpy, EGLSync sync, EGLint flags, EGLTime timeout);
	static PROC_EGL_ClientWaitSync _ClientWaitSync = (PROC_EGL_ClientWaitSync)::egl::internal::getEglFunction(::egl::internal::EglFuncName::ClientWaitSync);
	return _ClientWaitSync(dpy, sync, flags, timeout);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(GetSyncAttrib)(EGLDisplay dpy, EGLSync sync, EGLint attribute, EGLAttrib* value)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_GetSyncAttrib)(EGLDisplay dpy, EGLSync sync, EGLint attribute, EGLAttrib * value);
	static PROC_EGL_GetSyncAttrib _GetSyncAttrib = (PROC_EGL_GetSyncAttrib)::egl::internal::getEglFunction(::egl::internal::EglFuncName::GetSyncAttrib);
	return _GetSyncAttrib(dpy, sync, attribute, value);
}
inline EGLImage DYNAMICEGL_FUNCTION(CreateImage)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLAttrib* attrib_list)
{
	typedef EGLImage(EGLAPIENTRY * PROC_EGL_CreateImage)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLAttrib * attrib_list);
	static PROC_EGL_CreateImage _CreateImage = (PROC_EGL_CreateImage)::egl::internal::getEglFunction(::egl::internal::EglFuncName::CreateImage);
	return _CreateImage(dpy, ctx, target, buffer, attrib_list);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(DestroyImage)(EGLDisplay dpy, EGLImage image)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_DestroyImage)(EGLDisplay dpy, EGLImage image);
	static PROC_EGL_DestroyImage _DestroyImage = (PROC_EGL_DestroyImage)::egl::internal::getEglFunction(::egl::internal::EglFuncName::DestroyImage);
	return _DestroyImage(dpy, image);
}
inline EGLDisplay DYNAMICEGL_FUNCTION(GetPlatformDisplay)(EGLenum platform, void* native_display, const EGLAttrib* attrib_list)
{
	typedef EGLDisplay(EGLAPIENTRY * PROC_EGL_GetPlatformDisplay)(EGLenum platform, void* native_display, const EGLAttrib * attrib_list);
	static PROC_EGL_GetPlatformDisplay _GetPlatformDisplay = (PROC_EGL_GetPlatformDisplay)::egl::internal::getEglFunction(::egl::internal::EglFuncName::GetPlatformDisplay);
	return _GetPlatformDisplay(platform, native_display, attrib_list);
}
inline EGLSurface DYNAMICEGL_FUNCTION(CreatePlatformWindowSurface)(EGLDisplay dpy, EGLConfig config, void* native_window, const EGLAttrib* attrib_list)
{
	typedef EGLSurface(EGLAPIENTRY * PROC_EGL_CreatePlatformWindowSurface)(EGLDisplay dpy, EGLConfig config, void* native_window, const EGLAttrib * attrib_list);
	static PROC_EGL_CreatePlatformWindowSurface _CreatePlatformWindowSurface = (PROC_EGL_CreatePlatformWindowSurface)::egl::internal::getEglFunction(::egl::internal::EglFuncName::CreatePlatformWindowSurface);
	return _CreatePlatformWindowSurface(dpy, config, native_window, attrib_list);
}
inline EGLSurface DYNAMICEGL_FUNCTION(CreatePlatformPixmapSurface)(EGLDisplay dpy, EGLConfig config, void* native_pixmap, const EGLAttrib* attrib_list)
{
	typedef EGLSurface(EGLAPIENTRY * PROC_EGL_CreatePlatformPixmapSurface)(EGLDisplay dpy, EGLConfig config, void* native_pixmap, const EGLAttrib * attrib_list);
	static PROC_EGL_CreatePlatformPixmapSurface _CreatePlatformPixmapSurface = (PROC_EGL_CreatePlatformPixmapSurface)::egl::internal::getEglFunction(::egl::internal::EglFuncName::CreatePlatformPixmapSurface);
	return _CreatePlatformPixmapSurface(dpy, config, native_pixmap, attrib_list);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(WaitSync)(EGLDisplay dpy, EGLSync sync, EGLint flags)
{
	typedef EGLBoolean(EGLAPIENTRY * PROC_EGL_WaitSync)(EGLDisplay dpy, EGLSync sync, EGLint flags);
	static PROC_EGL_WaitSync _WaitSync = (PROC_EGL_WaitSync)::egl::internal::getEglFunction(::egl::internal::EglFuncName::WaitSync);
	return _WaitSync(dpy, sync, flags);
}
#ifndef DYNAMICEGL_NO_NAMESPACE
}
#endif
#ifndef __GNUC__
#pragma endregion
#endif

#ifndef __GNUC__
#pragma region EGL_EXT
#endif

namespace egl {
namespace internal {
namespace EglExtFuncName {
enum EglExtFunctionName
{
	CreateSync64KHR, DebugMessageControlKHR, QueryDebugKHR, LabelObjectKHR, QueryDisplayAttribKHR, CreateSyncKHR, DestroySyncKHR,
	ClientWaitSyncKHR, GetSyncAttribKHR, CreateImageKHR, DestroyImageKHR, LockSurfaceKHR, UnlockSurfaceKHR, QuerySurface64KHR,
	SetDamageRegionKHR, SignalSyncKHR, CreateStreamKHR, DestroyStreamKHR, StreamAttribKHR, QueryStreamKHR, QueryStreamu64KHR,
	CreateStreamAttribKHR, SetStreamAttribKHR, QueryStreamAttribKHR, StreamConsumerAcquireAttribKHR, StreamConsumerReleaseAttribKHR,
	StreamConsumerGLTextureExternalKHR, StreamConsumerAcquireKHR, StreamConsumerReleaseKHR, GetStreamFileDescriptorKHR,
	CreateStreamFromFileDescriptorKHR, QueryStreamTimeKHR, CreateStreamProducerSurfaceKHR, SwapBuffersWithDamageKHR, WaitSyncKHR,
	SetBlobCacheFuncsANDROID, CreateNativeClientBufferANDROID, DupNativeFenceFDANDROID, PresentationTimeANDROID, QuerySurfacePointerANGLE,
	CompositorSetContextListEXT, CompositorSetContextAttributesEXT, CompositorSetWindowListEXT, CompositorSetWindowAttributesEXT,
	CompositorBindTexWindowEXT, CompositorSetSizeEXT, CompositorSwapPolicyEXT, QueryDeviceAttribEXT, QueryDeviceStringEXT,
	QueryDevicesEXT, QueryDisplayAttribEXT, QueryDmaBufFormatsEXT, QueryDmaBufModifiersEXT, GetOutputLayersEXT, GetOutputPortsEXT,
	OutputLayerAttribEXT, QueryOutputLayerAttribEXT, QueryOutputLayerStringEXT, OutputPortAttribEXT, QueryOutputPortAttribEXT,
	QueryOutputPortStringEXT, GetPlatformDisplayEXT, CreatePlatformWindowSurfaceEXT, CreatePlatformPixmapSurfaceEXT, StreamConsumerOutputEXT,
	SwapBuffersWithDamageEXT, CreatePixmapSurfaceHI, CreateDRMImageMESA, ExportDRMImageMESA, ExportDMABUFImageQueryMESA,
	ExportDMABUFImageMESA, SwapBuffersRegionNOK, SwapBuffersRegion2NOK, QueryNativeDisplayNV, QueryNativeWindowNV, QueryNativePixmapNV,
	PostSubBufferNV, StreamConsumerGLTextureExternalAttribsNV, QueryDisplayAttribNV, SetStreamMetadataNV, QueryStreamMetadataNV,
	ResetStreamNV, CreateStreamSyncNV, CreateFenceSyncNV, DestroySyncNV, FenceNV, ClientWaitSyncNV, SignalSyncNV, GetSyncAttribNV,
	GetSystemTimeFrequencyNV, GetSystemTimeNV,
	NUMBER_OF_EGL_EXT_FUNCTIONS
};
}

static inline void* GetEglExtensionFunction(const char* funcName)
{
	return (void*)DYNAMICEGL_CALL_FUNCTION(GetProcAddress)(funcName);
}

static inline bool isExtensionSupported(const char* const extensionString, const char* const extension)
{
	if (!extensionString)
	{
		return false;
	}

	// The recommended technique for querying OpenGL extensions;
	// from http://opengl.org/resources/features/OGLextensions/
	const char* start = extensionString;
	char* position, *terminator;

	// Extension names should not have spaces.
	position = (char*)strchr(extension, ' ');

	if (position || *extension == '\0')
	{
		return 0;
	}

	/* It takes a bit of care to be fool-proof about parsing the
	OpenGL extensions string. Don't be fooled by sub-strings, etc. */
	for (;;)
	{
		position = (char*)strstr((char*)start, extension);

		if (!position)
		{
			break;
		}

		terminator = position + strlen(extension);

		if (position == start || *(position - 1) == ' ')
		{
			if (*terminator == ' ' || *terminator == '\0')
			{
				return true;
			}
		}

		start = terminator;
	}

	return false;
}

inline void* getEglExtFunction(egl::internal::EglExtFuncName::EglExtFunctionName funcname, bool reset = false)
{
	using namespace ::egl::internal;
	static void* FunctionTable[EglExtFuncName::NUMBER_OF_EGL_EXT_FUNCTIONS];

	//  GET FUNCTION POINTERS --- ONCE!!!! ///
	if (!FunctionTable[0] || reset)
	{
		FunctionTable[EglExtFuncName::CreateSync64KHR] = GetEglExtensionFunction("eglCreateSync64KHR");
		FunctionTable[EglExtFuncName::DebugMessageControlKHR] = GetEglExtensionFunction("eglDebugMessageControlKHR");
		FunctionTable[EglExtFuncName::QueryDebugKHR] = GetEglExtensionFunction("eglQueryDebugKHR");
		FunctionTable[EglExtFuncName::LabelObjectKHR] = GetEglExtensionFunction("eglLabelObjectKHR");
		FunctionTable[EglExtFuncName::QueryDisplayAttribKHR] = GetEglExtensionFunction("eglQueryDisplayAttribKHR");
		FunctionTable[EglExtFuncName::CreateSyncKHR] = GetEglExtensionFunction("eglCreateSyncKHR");
		FunctionTable[EglExtFuncName::DestroySyncKHR] = GetEglExtensionFunction("eglDestroySyncKHR");
		FunctionTable[EglExtFuncName::ClientWaitSyncKHR] = GetEglExtensionFunction("eglClientWaitSyncKHR");
		FunctionTable[EglExtFuncName::GetSyncAttribKHR] = GetEglExtensionFunction("eglGetSyncAttribKHR");
		FunctionTable[EglExtFuncName::CreateImageKHR] = GetEglExtensionFunction("eglCreateImageKHR");
		FunctionTable[EglExtFuncName::DestroyImageKHR] = GetEglExtensionFunction("eglDestroyImageKHR");
		FunctionTable[EglExtFuncName::LockSurfaceKHR] = GetEglExtensionFunction("eglLockSurfaceKHR");
		FunctionTable[EglExtFuncName::UnlockSurfaceKHR] = GetEglExtensionFunction("eglUnlockSurfaceKHR");
		FunctionTable[EglExtFuncName::QuerySurface64KHR] = GetEglExtensionFunction("eglQuerySurface64KHR");
		FunctionTable[EglExtFuncName::SetDamageRegionKHR] = GetEglExtensionFunction("eglSetDamageRegionKHR");
		FunctionTable[EglExtFuncName::SignalSyncKHR] = GetEglExtensionFunction("eglSignalSyncKHR");
		FunctionTable[EglExtFuncName::CreateStreamKHR] = GetEglExtensionFunction("eglCreateStreamKHR");
		FunctionTable[EglExtFuncName::DestroyStreamKHR] = GetEglExtensionFunction("eglDestroyStreamKHR");
		FunctionTable[EglExtFuncName::StreamAttribKHR] = GetEglExtensionFunction("eglStreamAttribKHR");
		FunctionTable[EglExtFuncName::QueryStreamKHR] = GetEglExtensionFunction("eglQueryStreamKHR");
		FunctionTable[EglExtFuncName::QueryStreamu64KHR] = GetEglExtensionFunction("eglQueryStreamu64KHR");
		FunctionTable[EglExtFuncName::CreateStreamAttribKHR] = GetEglExtensionFunction("eglCreateStreamAttribKHR");
		FunctionTable[EglExtFuncName::SetStreamAttribKHR] = GetEglExtensionFunction("eglSetStreamAttribKHR");
		FunctionTable[EglExtFuncName::QueryStreamAttribKHR] = GetEglExtensionFunction("eglQueryStreamAttribKHR");
		FunctionTable[EglExtFuncName::StreamConsumerAcquireAttribKHR] = GetEglExtensionFunction("eglStreamConsumerAcquireAttribKHR");
		FunctionTable[EglExtFuncName::StreamConsumerReleaseAttribKHR] = GetEglExtensionFunction("eglStreamConsumerReleaseAttribKHR");
		FunctionTable[EglExtFuncName::StreamConsumerGLTextureExternalKHR] = GetEglExtensionFunction("eglStreamConsumerGLTextureExternalKHR");
		FunctionTable[EglExtFuncName::StreamConsumerAcquireKHR] = GetEglExtensionFunction("eglStreamConsumerAcquireKHR");
		FunctionTable[EglExtFuncName::StreamConsumerReleaseKHR] = GetEglExtensionFunction("eglStreamConsumerReleaseKHR");
		FunctionTable[EglExtFuncName::GetStreamFileDescriptorKHR] = GetEglExtensionFunction("eglGetStreamFileDescriptorKHR");
		FunctionTable[EglExtFuncName::CreateStreamFromFileDescriptorKHR] = GetEglExtensionFunction("eglCreateStreamFromFileDescriptorKHR");
		FunctionTable[EglExtFuncName::QueryStreamTimeKHR] = GetEglExtensionFunction("eglQueryStreamTimeKHR");
		FunctionTable[EglExtFuncName::CreateStreamProducerSurfaceKHR] = GetEglExtensionFunction("eglCreateStreamProducerSurfaceKHR");
		FunctionTable[EglExtFuncName::SwapBuffersWithDamageKHR] = GetEglExtensionFunction("eglSwapBuffersWithDamageKHR");
		FunctionTable[EglExtFuncName::WaitSyncKHR] = GetEglExtensionFunction("eglWaitSyncKHR");
		FunctionTable[EglExtFuncName::SetBlobCacheFuncsANDROID] = GetEglExtensionFunction("eglSetBlobCacheFuncsANDROID");
		FunctionTable[EglExtFuncName::CreateNativeClientBufferANDROID] = GetEglExtensionFunction("eglCreateNativeClientBufferANDROID");
		FunctionTable[EglExtFuncName::DupNativeFenceFDANDROID] = GetEglExtensionFunction("eglDupNativeFenceFDANDROID");
		FunctionTable[EglExtFuncName::PresentationTimeANDROID] = GetEglExtensionFunction("eglPresentationTimeANDROID");
		FunctionTable[EglExtFuncName::QuerySurfacePointerANGLE] = GetEglExtensionFunction("eglQuerySurfacePointerANGLE");
		FunctionTable[EglExtFuncName::CompositorSetContextListEXT] = GetEglExtensionFunction("eglCompositorSetContextListEXT");
		FunctionTable[EglExtFuncName::CompositorSetContextAttributesEXT] = GetEglExtensionFunction("eglCompositorSetContextAttributesEXT");
		FunctionTable[EglExtFuncName::CompositorSetWindowListEXT] = GetEglExtensionFunction("eglCompositorSetWindowListEXT");
		FunctionTable[EglExtFuncName::CompositorSetWindowAttributesEXT] = GetEglExtensionFunction("eglCompositorSetWindowAttributesEXT");
		FunctionTable[EglExtFuncName::CompositorBindTexWindowEXT] = GetEglExtensionFunction("eglCompositorBindTexWindowEXT");
		FunctionTable[EglExtFuncName::CompositorSetSizeEXT] = GetEglExtensionFunction("eglCompositorSetSizeEXT");
		FunctionTable[EglExtFuncName::CompositorSwapPolicyEXT] = GetEglExtensionFunction("eglCompositorSwapPolicyEXT");
		FunctionTable[EglExtFuncName::QueryDeviceAttribEXT] = GetEglExtensionFunction("eglQueryDeviceAttribEXT");
		FunctionTable[EglExtFuncName::QueryDeviceStringEXT] = GetEglExtensionFunction("eglQueryDeviceStringEXT");
		FunctionTable[EglExtFuncName::QueryDevicesEXT] = GetEglExtensionFunction("eglQueryDevicesEXT");
		FunctionTable[EglExtFuncName::QueryDisplayAttribEXT] = GetEglExtensionFunction("eglQueryDisplayAttribEXT");
		FunctionTable[EglExtFuncName::QueryDmaBufFormatsEXT] = GetEglExtensionFunction("eglQueryDmaBufFormatsEXT");
		FunctionTable[EglExtFuncName::QueryDmaBufModifiersEXT] = GetEglExtensionFunction("eglQueryDmaBufModifiersEXT");
		FunctionTable[EglExtFuncName::GetOutputLayersEXT] = GetEglExtensionFunction("eglGetOutputLayersEXT");
		FunctionTable[EglExtFuncName::GetOutputPortsEXT] = GetEglExtensionFunction("eglGetOutputPortsEXT");
		FunctionTable[EglExtFuncName::OutputLayerAttribEXT] = GetEglExtensionFunction("eglOutputLayerAttribEXT");
		FunctionTable[EglExtFuncName::QueryOutputLayerAttribEXT] = GetEglExtensionFunction("eglQueryOutputLayerAttribEXT");
		FunctionTable[EglExtFuncName::QueryOutputLayerStringEXT] = GetEglExtensionFunction("eglQueryOutputLayerStringEXT");
		FunctionTable[EglExtFuncName::OutputPortAttribEXT] = GetEglExtensionFunction("eglOutputPortAttribEXT");
		FunctionTable[EglExtFuncName::QueryOutputPortAttribEXT] = GetEglExtensionFunction("eglQueryOutputPortAttribEXT");
		FunctionTable[EglExtFuncName::QueryOutputPortStringEXT] = GetEglExtensionFunction("eglQueryOutputPortStringEXT");
		FunctionTable[EglExtFuncName::GetPlatformDisplayEXT] = GetEglExtensionFunction("eglGetPlatformDisplayEXT");
		FunctionTable[EglExtFuncName::CreatePlatformWindowSurfaceEXT] = GetEglExtensionFunction("eglCreatePlatformWindowSurfaceEXT");
		FunctionTable[EglExtFuncName::CreatePlatformPixmapSurfaceEXT] = GetEglExtensionFunction("eglCreatePlatformPixmapSurfaceEXT");
		FunctionTable[EglExtFuncName::StreamConsumerOutputEXT] = GetEglExtensionFunction("eglStreamConsumerOutputEXT");
		FunctionTable[EglExtFuncName::SwapBuffersWithDamageEXT] = GetEglExtensionFunction("eglSwapBuffersWithDamageEXT");
		FunctionTable[EglExtFuncName::CreatePixmapSurfaceHI] = GetEglExtensionFunction("eglCreatePixmapSurfaceHI");
		FunctionTable[EglExtFuncName::CreateDRMImageMESA] = GetEglExtensionFunction("eglCreateDRMImageMESA");
		FunctionTable[EglExtFuncName::ExportDRMImageMESA] = GetEglExtensionFunction("eglExportDRMImageMESA");
		FunctionTable[EglExtFuncName::ExportDMABUFImageQueryMESA] = GetEglExtensionFunction("eglExportDMABUFImageQueryMESA");
		FunctionTable[EglExtFuncName::ExportDMABUFImageMESA] = GetEglExtensionFunction("eglExportDMABUFImageMESA");
		FunctionTable[EglExtFuncName::SwapBuffersRegionNOK] = GetEglExtensionFunction("eglSwapBuffersRegionNOK");
		FunctionTable[EglExtFuncName::SwapBuffersRegion2NOK] = GetEglExtensionFunction("eglSwapBuffersRegion2NOK");
		FunctionTable[EglExtFuncName::QueryNativeDisplayNV] = GetEglExtensionFunction("eglQueryNativeDisplayNV");
		FunctionTable[EglExtFuncName::QueryNativeWindowNV] = GetEglExtensionFunction("eglQueryNativeWindowNV");
		FunctionTable[EglExtFuncName::QueryNativePixmapNV] = GetEglExtensionFunction("eglQueryNativePixmapNV");
		FunctionTable[EglExtFuncName::PostSubBufferNV] = GetEglExtensionFunction("eglPostSubBufferNV");
		FunctionTable[EglExtFuncName::StreamConsumerGLTextureExternalAttribsNV] = GetEglExtensionFunction("eglStreamConsumerGLTextureExternalAttribsNV");
		FunctionTable[EglExtFuncName::QueryDisplayAttribNV] = GetEglExtensionFunction("eglQueryDisplayAttribNV");
		FunctionTable[EglExtFuncName::SetStreamMetadataNV] = GetEglExtensionFunction("eglSetStreamMetadataNV");
		FunctionTable[EglExtFuncName::QueryStreamMetadataNV] = GetEglExtensionFunction("eglQueryStreamMetadataNV");
		FunctionTable[EglExtFuncName::ResetStreamNV] = GetEglExtensionFunction("eglResetStreamNV");
		FunctionTable[EglExtFuncName::CreateStreamSyncNV] = GetEglExtensionFunction("eglCreateStreamSyncNV");
		FunctionTable[EglExtFuncName::CreateFenceSyncNV] = GetEglExtensionFunction("eglCreateFenceSyncNV");
		FunctionTable[EglExtFuncName::DestroySyncNV] = GetEglExtensionFunction("eglDestroySyncNV");
		FunctionTable[EglExtFuncName::FenceNV] = GetEglExtensionFunction("eglFenceNV");
		FunctionTable[EglExtFuncName::ClientWaitSyncNV] = GetEglExtensionFunction("eglClientWaitSyncNV");
		FunctionTable[EglExtFuncName::SignalSyncNV] = GetEglExtensionFunction("eglSignalSyncNV");
		FunctionTable[EglExtFuncName::GetSyncAttribNV] = GetEglExtensionFunction("eglGetSyncAttribNV");
		FunctionTable[EglExtFuncName::GetSystemTimeFrequencyNV] = GetEglExtensionFunction("eglGetSystemTimeFrequencyNV");
		FunctionTable[EglExtFuncName::GetSystemTimeNV] = GetEglExtensionFunction("eglGetSystemTimeNV");

	}
	return FunctionTable[funcname];
}
}
}
#ifndef __GNUC__
#pragma endregion
#endif

#ifndef DYNAMICEGL_NO_NAMESPACE
namespace egl {
namespace ext {
#endif
inline EGLSyncKHR DYNAMICEGL_FUNCTION(CreateSync64KHR)(EGLDisplay dpy, EGLenum type, const EGLAttribKHR* attrib_list)
{
	typedef EGLSyncKHR(EGLAPIENTRY * PFNEGLCreateSync64KHRPROC)(EGLDisplay dpy, EGLenum type, const EGLAttribKHR * attrib_list);
	static PFNEGLCreateSync64KHRPROC _CreateSync64KHR = (PFNEGLCreateSync64KHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::CreateSync64KHR);
	return _CreateSync64KHR(dpy, type, attrib_list);
}
inline EGLint DYNAMICEGL_FUNCTION(DebugMessageControlKHR)(EGLDEBUGPROCKHR callback, const EGLAttrib* attrib_list)
{
	typedef EGLint(EGLAPIENTRY * PFNEGLDebugMessageControlKHRPROC)(EGLDEBUGPROCKHR callback, const EGLAttrib * attrib_list);
	static PFNEGLDebugMessageControlKHRPROC _DebugMessageControlKHR = (PFNEGLDebugMessageControlKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::DebugMessageControlKHR);
	return _DebugMessageControlKHR(callback, attrib_list);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(QueryDebugKHR)(EGLint attribute, EGLAttrib* value)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLQueryDebugKHRPROC)(EGLint attribute, EGLAttrib * value);
	static PFNEGLQueryDebugKHRPROC _QueryDebugKHR = (PFNEGLQueryDebugKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QueryDebugKHR);
	return _QueryDebugKHR(attribute, value);
}
inline EGLint DYNAMICEGL_FUNCTION(LabelObjectKHR)(EGLDisplay display, EGLenum objectType, EGLObjectKHR object, EGLLabelKHR label)
{
	typedef EGLint(EGLAPIENTRY * PFNEGLLabelObjectKHRPROC)(EGLDisplay display, EGLenum objectType, EGLObjectKHR object, EGLLabelKHR label);
	static PFNEGLLabelObjectKHRPROC _LabelObjectKHR = (PFNEGLLabelObjectKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::LabelObjectKHR);
	return _LabelObjectKHR(display, objectType, object, label);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(QueryDisplayAttribKHR)(EGLDisplay dpy, EGLint name, EGLAttrib* value)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLQueryDisplayAttribKHRPROC)(EGLDisplay dpy, EGLint name, EGLAttrib * value);
	static PFNEGLQueryDisplayAttribKHRPROC _QueryDisplayAttribKHR = (PFNEGLQueryDisplayAttribKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QueryDisplayAttribKHR);
	return _QueryDisplayAttribKHR(dpy, name, value);
}
inline EGLSyncKHR DYNAMICEGL_FUNCTION(CreateSyncKHR)(EGLDisplay dpy, EGLenum type, const EGLint* attrib_list)
{
	typedef EGLSyncKHR(EGLAPIENTRY * PFNEGLCreateSyncKHRPROC)(EGLDisplay dpy, EGLenum type, const EGLint * attrib_list);
	static PFNEGLCreateSyncKHRPROC _CreateSyncKHR = (PFNEGLCreateSyncKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::CreateSyncKHR);
	return _CreateSyncKHR(dpy, type, attrib_list);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(DestroySyncKHR)(EGLDisplay dpy, EGLSyncKHR sync)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLDestroySyncKHRPROC)(EGLDisplay dpy, EGLSyncKHR sync);
	static PFNEGLDestroySyncKHRPROC _DestroySyncKHR = (PFNEGLDestroySyncKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::DestroySyncKHR);
	return _DestroySyncKHR(dpy, sync);
}
inline EGLint DYNAMICEGL_FUNCTION(ClientWaitSyncKHR)(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout)
{
	typedef EGLint(EGLAPIENTRY * PFNEGLClientWaitSyncKHRPROC)(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout);
	static PFNEGLClientWaitSyncKHRPROC _ClientWaitSyncKHR = (PFNEGLClientWaitSyncKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::ClientWaitSyncKHR);
	return _ClientWaitSyncKHR(dpy, sync, flags, timeout);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(GetSyncAttribKHR)(EGLDisplay dpy, EGLSyncKHR sync, EGLint attribute, EGLint* value)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLGetSyncAttribKHRPROC)(EGLDisplay dpy, EGLSyncKHR sync, EGLint attribute, EGLint * value);
	static PFNEGLGetSyncAttribKHRPROC _GetSyncAttribKHR = (PFNEGLGetSyncAttribKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::GetSyncAttribKHR);
	return _GetSyncAttribKHR(dpy, sync, attribute, value);
}
inline EGLImageKHR DYNAMICEGL_FUNCTION(CreateImageKHR)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint* attrib_list)
{
	typedef EGLImageKHR(EGLAPIENTRY * PFNEGLCreateImageKHRPROC)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint * attrib_list);
	static PFNEGLCreateImageKHRPROC _CreateImageKHR = (PFNEGLCreateImageKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::CreateImageKHR);
	return _CreateImageKHR(dpy, ctx, target, buffer, attrib_list);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(DestroyImageKHR)(EGLDisplay dpy, EGLImageKHR image)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLDestroyImageKHRPROC)(EGLDisplay dpy, EGLImageKHR image);
	static PFNEGLDestroyImageKHRPROC _DestroyImageKHR = (PFNEGLDestroyImageKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::DestroyImageKHR);
	return _DestroyImageKHR(dpy, image);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(LockSurfaceKHR)(EGLDisplay dpy, EGLSurface surface, const EGLint* attrib_list)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLLockSurfaceKHRPROC)(EGLDisplay dpy, EGLSurface surface, const EGLint * attrib_list);
	static PFNEGLLockSurfaceKHRPROC _LockSurfaceKHR = (PFNEGLLockSurfaceKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::LockSurfaceKHR);
	return _LockSurfaceKHR(dpy, surface, attrib_list);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(UnlockSurfaceKHR)(EGLDisplay dpy, EGLSurface surface)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLUnlockSurfaceKHRPROC)(EGLDisplay dpy, EGLSurface surface);
	static PFNEGLUnlockSurfaceKHRPROC _UnlockSurfaceKHR = (PFNEGLUnlockSurfaceKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::UnlockSurfaceKHR);
	return _UnlockSurfaceKHR(dpy, surface);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(QuerySurface64KHR)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLAttribKHR* value)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLQuerySurface64KHRPROC)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLAttribKHR * value);
	static PFNEGLQuerySurface64KHRPROC _QuerySurface64KHR = (PFNEGLQuerySurface64KHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QuerySurface64KHR);
	return _QuerySurface64KHR(dpy, surface, attribute, value);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(SetDamageRegionKHR)(EGLDisplay dpy, EGLSurface surface, EGLint* rects, EGLint n_rects)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLSetDamageRegionKHRPROC)(EGLDisplay dpy, EGLSurface surface, EGLint * rects, EGLint n_rects);
	static PFNEGLSetDamageRegionKHRPROC _SetDamageRegionKHR = (PFNEGLSetDamageRegionKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::SetDamageRegionKHR);
	return _SetDamageRegionKHR(dpy, surface, rects, n_rects);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(SignalSyncKHR)(EGLDisplay dpy, EGLSyncKHR sync, EGLenum mode)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLSignalSyncKHRPROC)(EGLDisplay dpy, EGLSyncKHR sync, EGLenum mode);
	static PFNEGLSignalSyncKHRPROC _SignalSyncKHR = (PFNEGLSignalSyncKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::SignalSyncKHR);
	return _SignalSyncKHR(dpy, sync, mode);
} inline EGLStreamKHR DYNAMICEGL_FUNCTION(CreateStreamKHR)(EGLDisplay dpy, const EGLint* attrib_list)
{
	typedef EGLStreamKHR(EGLAPIENTRY * PFNEGLCreateStreamKHRPROC)(EGLDisplay dpy, const EGLint * attrib_list);
	static PFNEGLCreateStreamKHRPROC _CreateStreamKHR = (PFNEGLCreateStreamKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::CreateStreamKHR);
	return _CreateStreamKHR(dpy, attrib_list);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(DestroyStreamKHR)(EGLDisplay dpy, EGLStreamKHR stream)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLDestroyStreamKHRPROC)(EGLDisplay dpy, EGLStreamKHR stream);
	static PFNEGLDestroyStreamKHRPROC _DestroyStreamKHR = (PFNEGLDestroyStreamKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::DestroyStreamKHR);
	return _DestroyStreamKHR(dpy, stream);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(StreamAttribKHR)(EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute, EGLint value)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLStreamAttribKHRPROC)(EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute, EGLint value);
	static PFNEGLStreamAttribKHRPROC _StreamAttribKHR = (PFNEGLStreamAttribKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::StreamAttribKHR);
	return _StreamAttribKHR(dpy, stream, attribute, value);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(QueryStreamKHR)(EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute, EGLint* value)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLQueryStreamKHRPROC)(EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute, EGLint * value);
	static PFNEGLQueryStreamKHRPROC _QueryStreamKHR = (PFNEGLQueryStreamKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QueryStreamKHR);
	return _QueryStreamKHR(dpy, stream, attribute, value);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(QueryStreamu64KHR)(EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute, EGLuint64KHR* value)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLQueryStreamu64KHRPROC)(EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute, EGLuint64KHR * value);
	static PFNEGLQueryStreamu64KHRPROC _QueryStreamu64KHR = (PFNEGLQueryStreamu64KHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QueryStreamu64KHR);
	return _QueryStreamu64KHR(dpy, stream, attribute, value);
} inline EGLStreamKHR DYNAMICEGL_FUNCTION(CreateStreamAttribKHR)(EGLDisplay dpy, const EGLAttrib* attrib_list)
{
	typedef EGLStreamKHR(EGLAPIENTRY * PFNEGLCreateStreamAttribKHRPROC)(EGLDisplay dpy, const EGLAttrib * attrib_list);
	static PFNEGLCreateStreamAttribKHRPROC _CreateStreamAttribKHR = (PFNEGLCreateStreamAttribKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::CreateStreamAttribKHR);
	return _CreateStreamAttribKHR(dpy, attrib_list);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(SetStreamAttribKHR)(EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute, EGLAttrib value)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLSetStreamAttribKHRPROC)(EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute, EGLAttrib value);
	static PFNEGLSetStreamAttribKHRPROC _SetStreamAttribKHR = (PFNEGLSetStreamAttribKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::SetStreamAttribKHR);
	return _SetStreamAttribKHR(dpy, stream, attribute, value);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(QueryStreamAttribKHR)(EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute, EGLAttrib* value)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLQueryStreamAttribKHRPROC)(EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute, EGLAttrib * value);
	static PFNEGLQueryStreamAttribKHRPROC _QueryStreamAttribKHR = (PFNEGLQueryStreamAttribKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QueryStreamAttribKHR);
	return _QueryStreamAttribKHR(dpy, stream, attribute, value);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(StreamConsumerAcquireAttribKHR)(EGLDisplay dpy, EGLStreamKHR stream, const EGLAttrib* attrib_list)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLStreamConsumerAcquireAttribKHRPROC)(EGLDisplay dpy, EGLStreamKHR stream, const EGLAttrib * attrib_list);
	static PFNEGLStreamConsumerAcquireAttribKHRPROC _StreamConsumerAcquireAttribKHR = (PFNEGLStreamConsumerAcquireAttribKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::StreamConsumerAcquireAttribKHR);
	return _StreamConsumerAcquireAttribKHR(dpy, stream, attrib_list);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(StreamConsumerReleaseAttribKHR)(EGLDisplay dpy, EGLStreamKHR stream, const EGLAttrib* attrib_list)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLStreamConsumerReleaseAttribKHRPROC)(EGLDisplay dpy, EGLStreamKHR stream, const EGLAttrib * attrib_list);
	static PFNEGLStreamConsumerReleaseAttribKHRPROC _StreamConsumerReleaseAttribKHR = (PFNEGLStreamConsumerReleaseAttribKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::StreamConsumerReleaseAttribKHR);
	return _StreamConsumerReleaseAttribKHR(dpy, stream, attrib_list);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(StreamConsumerGLTextureExternalKHR)(EGLDisplay dpy, EGLStreamKHR stream)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLStreamConsumerGLTextureExternalKHRPROC)(EGLDisplay dpy, EGLStreamKHR stream);
	static PFNEGLStreamConsumerGLTextureExternalKHRPROC _StreamConsumerGLTextureExternalKHR = (PFNEGLStreamConsumerGLTextureExternalKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::StreamConsumerGLTextureExternalKHR);
	return _StreamConsumerGLTextureExternalKHR(dpy, stream);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(StreamConsumerAcquireKHR)(EGLDisplay dpy, EGLStreamKHR stream)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLStreamConsumerAcquireKHRPROC)(EGLDisplay dpy, EGLStreamKHR stream);
	static PFNEGLStreamConsumerAcquireKHRPROC _StreamConsumerAcquireKHR = (PFNEGLStreamConsumerAcquireKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::StreamConsumerAcquireKHR);
	return _StreamConsumerAcquireKHR(dpy, stream);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(StreamConsumerReleaseKHR)(EGLDisplay dpy, EGLStreamKHR stream)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLStreamConsumerReleaseKHRPROC)(EGLDisplay dpy, EGLStreamKHR stream);
	static PFNEGLStreamConsumerReleaseKHRPROC _StreamConsumerReleaseKHR = (PFNEGLStreamConsumerReleaseKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::StreamConsumerReleaseKHR);
	return _StreamConsumerReleaseKHR(dpy, stream);
}
inline EGLNativeFileDescriptorKHR DYNAMICEGL_FUNCTION(GetStreamFileDescriptorKHR)(EGLDisplay dpy, EGLStreamKHR stream)
{
	typedef EGLNativeFileDescriptorKHR(EGLAPIENTRY * PFNEGLGetStreamFileDescriptorKHRPROC)(EGLDisplay dpy, EGLStreamKHR stream);
	static PFNEGLGetStreamFileDescriptorKHRPROC _GetStreamFileDescriptorKHR = (PFNEGLGetStreamFileDescriptorKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::GetStreamFileDescriptorKHR);
	return _GetStreamFileDescriptorKHR(dpy, stream);
}
inline EGLStreamKHR DYNAMICEGL_FUNCTION(CreateStreamFromFileDescriptorKHR)(EGLDisplay dpy, EGLNativeFileDescriptorKHR file_descriptor)
{
	typedef EGLStreamKHR(EGLAPIENTRY * PFNEGLCreateStreamFromFileDescriptorKHRPROC)(EGLDisplay dpy, EGLNativeFileDescriptorKHR file_descriptor);
	static PFNEGLCreateStreamFromFileDescriptorKHRPROC _CreateStreamFromFileDescriptorKHR = (PFNEGLCreateStreamFromFileDescriptorKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::CreateStreamFromFileDescriptorKHR);
	return _CreateStreamFromFileDescriptorKHR(dpy, file_descriptor);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(QueryStreamTimeKHR)(EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute, EGLTimeKHR* value)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLQueryStreamTimeKHRPROC)(EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute, EGLTimeKHR * value);
	static PFNEGLQueryStreamTimeKHRPROC _QueryStreamTimeKHR = (PFNEGLQueryStreamTimeKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QueryStreamTimeKHR);
	return _QueryStreamTimeKHR(dpy, stream, attribute, value);
} inline EGLSurface DYNAMICEGL_FUNCTION(CreateStreamProducerSurfaceKHR)(EGLDisplay dpy, EGLConfig config, EGLStreamKHR stream, const EGLint* attrib_list)
{
	typedef EGLSurface(EGLAPIENTRY * PFNEGLCreateStreamProducerSurfaceKHRPROC)(EGLDisplay dpy, EGLConfig config, EGLStreamKHR stream, const EGLint * attrib_list);
	static PFNEGLCreateStreamProducerSurfaceKHRPROC _CreateStreamProducerSurfaceKHR = (PFNEGLCreateStreamProducerSurfaceKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::CreateStreamProducerSurfaceKHR);
	return _CreateStreamProducerSurfaceKHR(dpy, config, stream, attrib_list);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(SwapBuffersWithDamageKHR)(EGLDisplay dpy, EGLSurface surface, EGLint* rects, EGLint n_rects)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLSwapBuffersWithDamageKHRPROC)(EGLDisplay dpy, EGLSurface surface, EGLint * rects, EGLint n_rects);
	static PFNEGLSwapBuffersWithDamageKHRPROC _SwapBuffersWithDamageKHR = (PFNEGLSwapBuffersWithDamageKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::SwapBuffersWithDamageKHR);
	return _SwapBuffersWithDamageKHR(dpy, surface, rects, n_rects);
}
inline EGLint DYNAMICEGL_FUNCTION(WaitSyncKHR)(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags)
{
	typedef EGLint(EGLAPIENTRY * PFNEGLWaitSyncKHRPROC)(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags);
	static PFNEGLWaitSyncKHRPROC _WaitSyncKHR = (PFNEGLWaitSyncKHRPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::WaitSyncKHR);
	return _WaitSyncKHR(dpy, sync, flags);
}
inline void DYNAMICEGL_FUNCTION(SetBlobCacheFuncsANDROID)(EGLDisplay dpy, EGLSetBlobFuncANDROID set, EGLGetBlobFuncANDROID get)
{
	typedef void (EGLAPIENTRY * PFNEGLSetBlobCacheFuncsANDROIDPROC)(EGLDisplay dpy, EGLSetBlobFuncANDROID set, EGLGetBlobFuncANDROID get);
	static PFNEGLSetBlobCacheFuncsANDROIDPROC _SetBlobCacheFuncsANDROID = (PFNEGLSetBlobCacheFuncsANDROIDPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::SetBlobCacheFuncsANDROID);
	return _SetBlobCacheFuncsANDROID(dpy, set, get);
}
inline EGLClientBuffer DYNAMICEGL_FUNCTION(CreateNativeClientBufferANDROID)(const EGLint* attrib_list)
{
	typedef EGLClientBuffer(EGLAPIENTRY * PFNEGLCreateNativeClientBufferANDROIDPROC)(const EGLint * attrib_list);
	static PFNEGLCreateNativeClientBufferANDROIDPROC _CreateNativeClientBufferANDROID = (PFNEGLCreateNativeClientBufferANDROIDPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::CreateNativeClientBufferANDROID);
	return _CreateNativeClientBufferANDROID(attrib_list);
}
inline EGLint DYNAMICEGL_FUNCTION(DupNativeFenceFDANDROID)(EGLDisplay dpy, EGLSyncKHR sync)
{
	typedef EGLint(EGLAPIENTRY * PFNEGLDupNativeFenceFDANDROIDPROC)(EGLDisplay dpy, EGLSyncKHR sync);
	static PFNEGLDupNativeFenceFDANDROIDPROC _DupNativeFenceFDANDROID = (PFNEGLDupNativeFenceFDANDROIDPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::DupNativeFenceFDANDROID);
	return _DupNativeFenceFDANDROID(dpy, sync);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(PresentationTimeANDROID)(EGLDisplay dpy, EGLSurface surface, EGLnsecsANDROID time)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLPresentationTimeANDROIDPROC)(EGLDisplay dpy, EGLSurface surface, EGLnsecsANDROID time);
	static PFNEGLPresentationTimeANDROIDPROC _PresentationTimeANDROID = (PFNEGLPresentationTimeANDROIDPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::PresentationTimeANDROID);
	return _PresentationTimeANDROID(dpy, surface, time);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(QuerySurfacePointerANGLE)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, void** value)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLQuerySurfacePointerANGLEPROC)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, void** value);
	static PFNEGLQuerySurfacePointerANGLEPROC _QuerySurfacePointerANGLE = (PFNEGLQuerySurfacePointerANGLEPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QuerySurfacePointerANGLE);
	return _QuerySurfacePointerANGLE(dpy, surface, attribute, value);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(CompositorSetContextListEXT)(const EGLint* external_ref_ids, EGLint num_entries)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLCompositorSetContextListEXTPROC)(const EGLint * external_ref_ids, EGLint num_entries);
	static PFNEGLCompositorSetContextListEXTPROC _CompositorSetContextListEXT = (PFNEGLCompositorSetContextListEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::CompositorSetContextListEXT);
	return _CompositorSetContextListEXT(external_ref_ids, num_entries);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(CompositorSetContextAttributesEXT)(EGLint external_ref_id, const EGLint* context_attributes, EGLint num_entries)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLCompositorSetContextAttributesEXTPROC)(EGLint external_ref_id, const EGLint * context_attributes, EGLint num_entries);
	static PFNEGLCompositorSetContextAttributesEXTPROC _CompositorSetContextAttributesEXT = (PFNEGLCompositorSetContextAttributesEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::CompositorSetContextAttributesEXT);
	return _CompositorSetContextAttributesEXT(external_ref_id, context_attributes, num_entries);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(CompositorSetWindowListEXT)(EGLint external_ref_id, const EGLint* external_win_ids, EGLint num_entries)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLCompositorSetWindowListEXTPROC)(EGLint external_ref_id, const EGLint * external_win_ids, EGLint num_entries);
	static PFNEGLCompositorSetWindowListEXTPROC _CompositorSetWindowListEXT = (PFNEGLCompositorSetWindowListEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::CompositorSetWindowListEXT);
	return _CompositorSetWindowListEXT(external_ref_id, external_win_ids, num_entries);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(CompositorSetWindowAttributesEXT)(EGLint external_win_id, const EGLint* window_attributes, EGLint num_entries)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLCompositorSetWindowAttributesEXTPROC)(EGLint external_win_id, const EGLint * window_attributes, EGLint num_entries);
	static PFNEGLCompositorSetWindowAttributesEXTPROC _CompositorSetWindowAttributesEXT = (PFNEGLCompositorSetWindowAttributesEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::CompositorSetWindowAttributesEXT);
	return _CompositorSetWindowAttributesEXT(external_win_id, window_attributes, num_entries);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(CompositorBindTexWindowEXT)(EGLint external_win_id)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLCompositorBindTexWindowEXTPROC)(EGLint external_win_id);
	static PFNEGLCompositorBindTexWindowEXTPROC _CompositorBindTexWindowEXT = (PFNEGLCompositorBindTexWindowEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::CompositorBindTexWindowEXT);
	return _CompositorBindTexWindowEXT(external_win_id);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(CompositorSetSizeEXT)(EGLint external_win_id, EGLint width, EGLint height)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLCompositorSetSizeEXTPROC)(EGLint external_win_id, EGLint width, EGLint height);
	static PFNEGLCompositorSetSizeEXTPROC _CompositorSetSizeEXT = (PFNEGLCompositorSetSizeEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::CompositorSetSizeEXT);
	return _CompositorSetSizeEXT(external_win_id, width, height);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(CompositorSwapPolicyEXT)(EGLint external_win_id, EGLint policy)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLCompositorSwapPolicyEXTPROC)(EGLint external_win_id, EGLint policy);
	static PFNEGLCompositorSwapPolicyEXTPROC _CompositorSwapPolicyEXT = (PFNEGLCompositorSwapPolicyEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::CompositorSwapPolicyEXT);
	return _CompositorSwapPolicyEXT(external_win_id, policy);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(QueryDeviceAttribEXT)(EGLDeviceEXT device, EGLint attribute, EGLAttrib* value)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLQueryDeviceAttribEXTPROC)(EGLDeviceEXT device, EGLint attribute, EGLAttrib * value);
	static PFNEGLQueryDeviceAttribEXTPROC _QueryDeviceAttribEXT = (PFNEGLQueryDeviceAttribEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QueryDeviceAttribEXT);
	return _QueryDeviceAttribEXT(device, attribute, value);
}
inline const char* DYNAMICEGL_FUNCTION(QueryDeviceStringEXT)(EGLDeviceEXT device, EGLint name)
{
	typedef const char* (EGLAPIENTRY * PFNEGLQueryDeviceStringEXTPROC)(EGLDeviceEXT device, EGLint name);
	static PFNEGLQueryDeviceStringEXTPROC _QueryDeviceStringEXT = (PFNEGLQueryDeviceStringEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QueryDeviceStringEXT);
	return _QueryDeviceStringEXT(device, name);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(QueryDevicesEXT)(EGLint max_devices, EGLDeviceEXT* devices, EGLint* num_devices)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLQueryDevicesEXTPROC)(EGLint max_devices, EGLDeviceEXT * devices, EGLint * num_devices);
	static PFNEGLQueryDevicesEXTPROC _QueryDevicesEXT = (PFNEGLQueryDevicesEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QueryDevicesEXT);
	return _QueryDevicesEXT(max_devices, devices, num_devices);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(QueryDisplayAttribEXT)(EGLDisplay dpy, EGLint attribute, EGLAttrib* value)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLQueryDisplayAttribEXTPROC)(EGLDisplay dpy, EGLint attribute, EGLAttrib * value);
	static PFNEGLQueryDisplayAttribEXTPROC _QueryDisplayAttribEXT = (PFNEGLQueryDisplayAttribEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QueryDisplayAttribEXT);
	return _QueryDisplayAttribEXT(dpy, attribute, value);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(QueryDmaBufFormatsEXT)(EGLDisplay dpy, EGLint max_formats, EGLint* formats, EGLint* num_formats)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLQueryDmaBufFormatsEXTPROC)(EGLDisplay dpy, EGLint max_formats, EGLint * formats, EGLint * num_formats);
	static PFNEGLQueryDmaBufFormatsEXTPROC _QueryDmaBufFormatsEXT = (PFNEGLQueryDmaBufFormatsEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QueryDmaBufFormatsEXT);
	return _QueryDmaBufFormatsEXT(dpy, max_formats, formats, num_formats);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(QueryDmaBufModifiersEXT)(EGLDisplay dpy, EGLint format, EGLint max_modifiers, EGLuint64KHR* modifiers, EGLBoolean* external_only, EGLint* num_modifiers)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLQueryDmaBufModifiersEXTPROC)(EGLDisplay dpy, EGLint format, EGLint max_modifiers, EGLuint64KHR * modifiers, EGLBoolean * external_only, EGLint * num_modifiers);
	static PFNEGLQueryDmaBufModifiersEXTPROC _QueryDmaBufModifiersEXT = (PFNEGLQueryDmaBufModifiersEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QueryDmaBufModifiersEXT);
	return _QueryDmaBufModifiersEXT(dpy, format, max_modifiers, modifiers, external_only, num_modifiers);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(GetOutputLayersEXT)(EGLDisplay dpy, const EGLAttrib* attrib_list, EGLOutputLayerEXT* layers, EGLint max_layers, EGLint* num_layers)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLGetOutputLayersEXTPROC)(EGLDisplay dpy, const EGLAttrib * attrib_list, EGLOutputLayerEXT * layers, EGLint max_layers, EGLint * num_layers);
	static PFNEGLGetOutputLayersEXTPROC _GetOutputLayersEXT = (PFNEGLGetOutputLayersEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::GetOutputLayersEXT);
	return _GetOutputLayersEXT(dpy, attrib_list, layers, max_layers, num_layers);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(GetOutputPortsEXT)(EGLDisplay dpy, const EGLAttrib* attrib_list, EGLOutputPortEXT* ports, EGLint max_ports, EGLint* num_ports)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLGetOutputPortsEXTPROC)(EGLDisplay dpy, const EGLAttrib * attrib_list, EGLOutputPortEXT * ports, EGLint max_ports, EGLint * num_ports);
	static PFNEGLGetOutputPortsEXTPROC _GetOutputPortsEXT = (PFNEGLGetOutputPortsEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::GetOutputPortsEXT);
	return _GetOutputPortsEXT(dpy, attrib_list, ports, max_ports, num_ports);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(OutputLayerAttribEXT)(EGLDisplay dpy, EGLOutputLayerEXT layer, EGLint attribute, EGLAttrib value)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLOutputLayerAttribEXTPROC)(EGLDisplay dpy, EGLOutputLayerEXT layer, EGLint attribute, EGLAttrib value);
	static PFNEGLOutputLayerAttribEXTPROC _OutputLayerAttribEXT = (PFNEGLOutputLayerAttribEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::OutputLayerAttribEXT);
	return _OutputLayerAttribEXT(dpy, layer, attribute, value);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(QueryOutputLayerAttribEXT)(EGLDisplay dpy, EGLOutputLayerEXT layer, EGLint attribute, EGLAttrib* value)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLQueryOutputLayerAttribEXTPROC)(EGLDisplay dpy, EGLOutputLayerEXT layer, EGLint attribute, EGLAttrib * value);
	static PFNEGLQueryOutputLayerAttribEXTPROC _QueryOutputLayerAttribEXT = (PFNEGLQueryOutputLayerAttribEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QueryOutputLayerAttribEXT);
	return _QueryOutputLayerAttribEXT(dpy, layer, attribute, value);
}
inline const char* DYNAMICEGL_FUNCTION(QueryOutputLayerStringEXT)(EGLDisplay dpy, EGLOutputLayerEXT layer, EGLint name)
{
	typedef const char* (EGLAPIENTRY * PFNEGLQueryOutputLayerStringEXTPROC)(EGLDisplay dpy, EGLOutputLayerEXT layer, EGLint name);
	static PFNEGLQueryOutputLayerStringEXTPROC _QueryOutputLayerStringEXT = (PFNEGLQueryOutputLayerStringEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QueryOutputLayerStringEXT);
	return _QueryOutputLayerStringEXT(dpy, layer, name);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(OutputPortAttribEXT)(EGLDisplay dpy, EGLOutputPortEXT port, EGLint attribute, EGLAttrib value)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLOutputPortAttribEXTPROC)(EGLDisplay dpy, EGLOutputPortEXT port, EGLint attribute, EGLAttrib value);
	static PFNEGLOutputPortAttribEXTPROC _OutputPortAttribEXT = (PFNEGLOutputPortAttribEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::OutputPortAttribEXT);
	return _OutputPortAttribEXT(dpy, port, attribute, value);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(QueryOutputPortAttribEXT)(EGLDisplay dpy, EGLOutputPortEXT port, EGLint attribute, EGLAttrib* value)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLQueryOutputPortAttribEXTPROC)(EGLDisplay dpy, EGLOutputPortEXT port, EGLint attribute, EGLAttrib * value);
	static PFNEGLQueryOutputPortAttribEXTPROC _QueryOutputPortAttribEXT = (PFNEGLQueryOutputPortAttribEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QueryOutputPortAttribEXT);
	return _QueryOutputPortAttribEXT(dpy, port, attribute, value);
}
inline const char* DYNAMICEGL_FUNCTION(QueryOutputPortStringEXT)(EGLDisplay dpy, EGLOutputPortEXT port, EGLint name)
{
	typedef const char* (EGLAPIENTRY * PFNEGLQueryOutputPortStringEXTPROC)(EGLDisplay dpy, EGLOutputPortEXT port, EGLint name);
	static PFNEGLQueryOutputPortStringEXTPROC _QueryOutputPortStringEXT = (PFNEGLQueryOutputPortStringEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QueryOutputPortStringEXT);
	return _QueryOutputPortStringEXT(dpy, port, name);
}
inline EGLDisplay DYNAMICEGL_FUNCTION(GetPlatformDisplayEXT)(EGLenum platform, void* native_display, const EGLint* attrib_list)
{
	typedef EGLDisplay(EGLAPIENTRY * PFNEGLGetPlatformDisplayEXTPROC)(EGLenum platform, void* native_display, const EGLint * attrib_list);
	static PFNEGLGetPlatformDisplayEXTPROC _GetPlatformDisplayEXT = (PFNEGLGetPlatformDisplayEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::GetPlatformDisplayEXT);
	return _GetPlatformDisplayEXT(platform, native_display, attrib_list);
}
inline EGLSurface DYNAMICEGL_FUNCTION(CreatePlatformWindowSurfaceEXT)(EGLDisplay dpy, EGLConfig config, void* native_window, const EGLint* attrib_list)
{
	typedef EGLSurface(EGLAPIENTRY * PFNEGLCreatePlatformWindowSurfaceEXTPROC)(EGLDisplay dpy, EGLConfig config, void* native_window, const EGLint * attrib_list);
	static PFNEGLCreatePlatformWindowSurfaceEXTPROC _CreatePlatformWindowSurfaceEXT = (PFNEGLCreatePlatformWindowSurfaceEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::CreatePlatformWindowSurfaceEXT);
	return _CreatePlatformWindowSurfaceEXT(dpy, config, native_window, attrib_list);
}
inline EGLSurface DYNAMICEGL_FUNCTION(CreatePlatformPixmapSurfaceEXT)(EGLDisplay dpy, EGLConfig config, void* native_pixmap, const EGLint* attrib_list)
{
	typedef EGLSurface(EGLAPIENTRY * PFNEGLCreatePlatformPixmapSurfaceEXTPROC)(EGLDisplay dpy, EGLConfig config, void* native_pixmap, const EGLint * attrib_list);
	static PFNEGLCreatePlatformPixmapSurfaceEXTPROC _CreatePlatformPixmapSurfaceEXT = (PFNEGLCreatePlatformPixmapSurfaceEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::CreatePlatformPixmapSurfaceEXT);
	return _CreatePlatformPixmapSurfaceEXT(dpy, config, native_pixmap, attrib_list);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(StreamConsumerOutputEXT)(EGLDisplay dpy, EGLStreamKHR stream, EGLOutputLayerEXT layer)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLStreamConsumerOutputEXTPROC)(EGLDisplay dpy, EGLStreamKHR stream, EGLOutputLayerEXT layer);
	static PFNEGLStreamConsumerOutputEXTPROC _StreamConsumerOutputEXT = (PFNEGLStreamConsumerOutputEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::StreamConsumerOutputEXT);
	return _StreamConsumerOutputEXT(dpy, stream, layer);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(SwapBuffersWithDamageEXT)(EGLDisplay dpy, EGLSurface surface, EGLint* rects, EGLint n_rects)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLSwapBuffersWithDamageEXTPROC)(EGLDisplay dpy, EGLSurface surface, EGLint * rects, EGLint n_rects);
	static PFNEGLSwapBuffersWithDamageEXTPROC _SwapBuffersWithDamageEXT = (PFNEGLSwapBuffersWithDamageEXTPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::SwapBuffersWithDamageEXT);
	return _SwapBuffersWithDamageEXT(dpy, surface, rects, n_rects);
}
inline EGLSurface DYNAMICEGL_FUNCTION(CreatePixmapSurfaceHI)(EGLDisplay dpy, EGLConfig config, struct EGLClientPixmapHI* pixmap)
{
	typedef EGLSurface(EGLAPIENTRY * PFNEGLCreatePixmapSurfaceHIPROC)(EGLDisplay dpy, EGLConfig config, struct EGLClientPixmapHI * pixmap);
	static PFNEGLCreatePixmapSurfaceHIPROC _CreatePixmapSurfaceHI = (PFNEGLCreatePixmapSurfaceHIPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::CreatePixmapSurfaceHI);
	return _CreatePixmapSurfaceHI(dpy, config, pixmap);
}
inline EGLImageKHR DYNAMICEGL_FUNCTION(CreateDRMImageMESA)(EGLDisplay dpy, const EGLint* attrib_list)
{
	typedef EGLImageKHR(EGLAPIENTRY * PFNEGLCreateDRMImageMESAPROC)(EGLDisplay dpy, const EGLint * attrib_list);
	static PFNEGLCreateDRMImageMESAPROC _CreateDRMImageMESA = (PFNEGLCreateDRMImageMESAPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::CreateDRMImageMESA);
	return _CreateDRMImageMESA(dpy, attrib_list);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(ExportDRMImageMESA)(EGLDisplay dpy, EGLImageKHR image, EGLint* name, EGLint* handle, EGLint* stride)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLExportDRMImageMESAPROC)(EGLDisplay dpy, EGLImageKHR image, EGLint * name, EGLint * handle, EGLint * stride);
	static PFNEGLExportDRMImageMESAPROC _ExportDRMImageMESA = (PFNEGLExportDRMImageMESAPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::ExportDRMImageMESA);
	return _ExportDRMImageMESA(dpy, image, name, handle, stride);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(ExportDMABUFImageQueryMESA)(EGLDisplay dpy, EGLImageKHR image, int* fourcc, int* num_planes, EGLuint64KHR* modifiers)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLExportDMABUFImageQueryMESAPROC)(EGLDisplay dpy, EGLImageKHR image, int* fourcc, int* num_planes, EGLuint64KHR * modifiers);
	static PFNEGLExportDMABUFImageQueryMESAPROC _ExportDMABUFImageQueryMESA = (PFNEGLExportDMABUFImageQueryMESAPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::ExportDMABUFImageQueryMESA);
	return _ExportDMABUFImageQueryMESA(dpy, image, fourcc, num_planes, modifiers);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(ExportDMABUFImageMESA)(EGLDisplay dpy, EGLImageKHR image, int* fds, EGLint* strides, EGLint* offsets)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLExportDMABUFImageMESAPROC)(EGLDisplay dpy, EGLImageKHR image, int* fds, EGLint * strides, EGLint * offsets);
	static PFNEGLExportDMABUFImageMESAPROC _ExportDMABUFImageMESA = (PFNEGLExportDMABUFImageMESAPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::ExportDMABUFImageMESA);
	return _ExportDMABUFImageMESA(dpy, image, fds, strides, offsets);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(SwapBuffersRegionNOK)(EGLDisplay dpy, EGLSurface surface, EGLint numRects, const EGLint* rects)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLSwapBuffersRegionNOKPROC)(EGLDisplay dpy, EGLSurface surface, EGLint numRects, const EGLint * rects);
	static PFNEGLSwapBuffersRegionNOKPROC _SwapBuffersRegionNOK = (PFNEGLSwapBuffersRegionNOKPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::SwapBuffersRegionNOK);
	return _SwapBuffersRegionNOK(dpy, surface, numRects, rects);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(SwapBuffersRegion2NOK)(EGLDisplay dpy, EGLSurface surface, EGLint numRects, const EGLint* rects)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLSwapBuffersRegion2NOKPROC)(EGLDisplay dpy, EGLSurface surface, EGLint numRects, const EGLint * rects);
	static PFNEGLSwapBuffersRegion2NOKPROC _SwapBuffersRegion2NOK = (PFNEGLSwapBuffersRegion2NOKPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::SwapBuffersRegion2NOK);
	return _SwapBuffersRegion2NOK(dpy, surface, numRects, rects);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(QueryNativeDisplayNV)(EGLDisplay dpy, EGLNativeDisplayType* display_id)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLQueryNativeDisplayNVPROC)(EGLDisplay dpy, EGLNativeDisplayType * display_id);
	static PFNEGLQueryNativeDisplayNVPROC _QueryNativeDisplayNV = (PFNEGLQueryNativeDisplayNVPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QueryNativeDisplayNV);
	return _QueryNativeDisplayNV(dpy, display_id);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(QueryNativeWindowNV)(EGLDisplay dpy, EGLSurface surf, EGLNativeWindowType* window)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLQueryNativeWindowNVPROC)(EGLDisplay dpy, EGLSurface surf, EGLNativeWindowType * window);
	static PFNEGLQueryNativeWindowNVPROC _QueryNativeWindowNV = (PFNEGLQueryNativeWindowNVPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QueryNativeWindowNV);
	return _QueryNativeWindowNV(dpy, surf, window);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(QueryNativePixmapNV)(EGLDisplay dpy, EGLSurface surf, EGLNativePixmapType* pixmap)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLQueryNativePixmapNVPROC)(EGLDisplay dpy, EGLSurface surf, EGLNativePixmapType * pixmap);
	static PFNEGLQueryNativePixmapNVPROC _QueryNativePixmapNV = (PFNEGLQueryNativePixmapNVPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QueryNativePixmapNV);
	return _QueryNativePixmapNV(dpy, surf, pixmap);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(PostSubBufferNV)(EGLDisplay dpy, EGLSurface surface, EGLint x, EGLint y, EGLint width, EGLint height)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLPostSubBufferNVPROC)(EGLDisplay dpy, EGLSurface surface, EGLint x, EGLint y, EGLint width, EGLint height);
	static PFNEGLPostSubBufferNVPROC _PostSubBufferNV = (PFNEGLPostSubBufferNVPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::PostSubBufferNV);
	return _PostSubBufferNV(dpy, surface, x, y, width, height);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(StreamConsumerGLTextureExternalAttribsNV)(EGLDisplay dpy, EGLStreamKHR stream, EGLAttrib* attrib_list)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLStreamConsumerGLTextureExternalAttribsNVPROC)(EGLDisplay dpy, EGLStreamKHR stream, EGLAttrib * attrib_list);
	static PFNEGLStreamConsumerGLTextureExternalAttribsNVPROC _StreamConsumerGLTextureExternalAttribsNV = (PFNEGLStreamConsumerGLTextureExternalAttribsNVPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::StreamConsumerGLTextureExternalAttribsNV);
	return _StreamConsumerGLTextureExternalAttribsNV(dpy, stream, attrib_list);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(QueryDisplayAttribNV)(EGLDisplay dpy, EGLint attribute, EGLAttrib* value)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLQueryDisplayAttribNVPROC)(EGLDisplay dpy, EGLint attribute, EGLAttrib * value);
	static PFNEGLQueryDisplayAttribNVPROC _QueryDisplayAttribNV = (PFNEGLQueryDisplayAttribNVPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QueryDisplayAttribNV);
	return _QueryDisplayAttribNV(dpy, attribute, value);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(SetStreamMetadataNV)(EGLDisplay dpy, EGLStreamKHR stream, EGLint n, EGLint offset, EGLint size, const void* data)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLSetStreamMetadataNVPROC)(EGLDisplay dpy, EGLStreamKHR stream, EGLint n, EGLint offset, EGLint size, const void* data);
	static PFNEGLSetStreamMetadataNVPROC _SetStreamMetadataNV = (PFNEGLSetStreamMetadataNVPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::SetStreamMetadataNV);
	return _SetStreamMetadataNV(dpy, stream, n, offset, size, data);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(QueryStreamMetadataNV)(EGLDisplay dpy, EGLStreamKHR stream, EGLenum name, EGLint n, EGLint offset, EGLint size, void* data)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLQueryStreamMetadataNVPROC)(EGLDisplay dpy, EGLStreamKHR stream, EGLenum name, EGLint n, EGLint offset, EGLint size, void* data);
	static PFNEGLQueryStreamMetadataNVPROC _QueryStreamMetadataNV = (PFNEGLQueryStreamMetadataNVPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::QueryStreamMetadataNV);
	return _QueryStreamMetadataNV(dpy, stream, name, n, offset, size, data);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(ResetStreamNV)(EGLDisplay dpy, EGLStreamKHR stream)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLResetStreamNVPROC)(EGLDisplay dpy, EGLStreamKHR stream);
	static PFNEGLResetStreamNVPROC _ResetStreamNV = (PFNEGLResetStreamNVPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::ResetStreamNV);
	return _ResetStreamNV(dpy, stream);
}
inline EGLSyncKHR DYNAMICEGL_FUNCTION(CreateStreamSyncNV)(EGLDisplay dpy, EGLStreamKHR stream, EGLenum type, const EGLint* attrib_list)
{
	typedef EGLSyncKHR(EGLAPIENTRY * PFNEGLCreateStreamSyncNVPROC)(EGLDisplay dpy, EGLStreamKHR stream, EGLenum type, const EGLint * attrib_list);
	static PFNEGLCreateStreamSyncNVPROC _CreateStreamSyncNV = (PFNEGLCreateStreamSyncNVPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::CreateStreamSyncNV);
	return _CreateStreamSyncNV(dpy, stream, type, attrib_list);
}
inline EGLSyncNV DYNAMICEGL_FUNCTION(CreateFenceSyncNV)(EGLDisplay dpy, EGLenum condition, const EGLint* attrib_list)
{
	typedef EGLSyncNV(EGLAPIENTRY * PFNEGLCreateFenceSyncNVPROC)(EGLDisplay dpy, EGLenum condition, const EGLint * attrib_list);
	static PFNEGLCreateFenceSyncNVPROC _CreateFenceSyncNV = (PFNEGLCreateFenceSyncNVPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::CreateFenceSyncNV);
	return _CreateFenceSyncNV(dpy, condition, attrib_list);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(DestroySyncNV)(EGLSyncNV sync)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLDestroySyncNVPROC)(EGLSyncNV sync);
	static PFNEGLDestroySyncNVPROC _DestroySyncNV = (PFNEGLDestroySyncNVPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::DestroySyncNV);
	return _DestroySyncNV(sync);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(FenceNV)(EGLSyncNV sync)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLFenceNVPROC)(EGLSyncNV sync);
	static PFNEGLFenceNVPROC _FenceNV = (PFNEGLFenceNVPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::FenceNV);
	return _FenceNV(sync);
}
inline EGLint DYNAMICEGL_FUNCTION(ClientWaitSyncNV)(EGLSyncNV sync, EGLint flags, EGLTimeNV timeout)
{
	typedef EGLint(EGLAPIENTRY * PFNEGLClientWaitSyncNVPROC)(EGLSyncNV sync, EGLint flags, EGLTimeNV timeout);
	static PFNEGLClientWaitSyncNVPROC _ClientWaitSyncNV = (PFNEGLClientWaitSyncNVPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::ClientWaitSyncNV);
	return _ClientWaitSyncNV(sync, flags, timeout);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(SignalSyncNV)(EGLSyncNV sync, EGLenum mode)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLSignalSyncNVPROC)(EGLSyncNV sync, EGLenum mode);
	static PFNEGLSignalSyncNVPROC _SignalSyncNV = (PFNEGLSignalSyncNVPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::SignalSyncNV);
	return _SignalSyncNV(sync, mode);
}
inline EGLBoolean DYNAMICEGL_FUNCTION(GetSyncAttribNV)(EGLSyncNV sync, EGLint attribute, EGLint* value)
{
	typedef EGLBoolean(EGLAPIENTRY * PFNEGLGetSyncAttribNVPROC)(EGLSyncNV sync, EGLint attribute, EGLint * value);
	static PFNEGLGetSyncAttribNVPROC _GetSyncAttribNV = (PFNEGLGetSyncAttribNVPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::GetSyncAttribNV);
	return _GetSyncAttribNV(sync, attribute, value);
}
inline EGLuint64NV DYNAMICEGL_FUNCTION(GetSystemTimeFrequencyNV)(void)
{
	typedef EGLuint64NV(EGLAPIENTRY * PFNEGLGetSystemTimeFrequencyNVPROC)(void);
	static PFNEGLGetSystemTimeFrequencyNVPROC _GetSystemTimeFrequencyNV = (PFNEGLGetSystemTimeFrequencyNVPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::GetSystemTimeFrequencyNV);
	return _GetSystemTimeFrequencyNV();
}
inline EGLuint64NV DYNAMICEGL_FUNCTION(GetSystemTimeNV)(void)
{
	typedef EGLuint64NV(EGLAPIENTRY * PFNEGLGetSystemTimeNVPROC)(void);
	static PFNEGLGetSystemTimeNVPROC _GetSystemTimeNV = (PFNEGLGetSystemTimeNVPROC)::egl::internal::getEglExtFunction(::egl::internal::EglExtFuncName::GetSystemTimeNV);
	return _GetSystemTimeNV();
}
#ifndef DYNAMICEGL_NO_NAMESPACE
}
#endif

inline bool isEglExtensionSupported(const char* extensionName, bool resetExtensionCache = false)
{
	static const char* extensionString = DYNAMICEGL_CALL_FUNCTION(QueryString)(DYNAMICEGL_CALL_FUNCTION(GetCurrentDisplay)(), EGL_EXTENSIONS);

	if (resetExtensionCache) { extensionString = DYNAMICEGL_CALL_FUNCTION(QueryString)(DYNAMICEGL_CALL_FUNCTION(GetCurrentDisplay)(), EGL_EXTENSIONS); }
	return ::egl::internal::isExtensionSupported(extensionString, extensionName);
}
inline bool isEglExtensionSupported(EGLDisplay display, const char* extensionName)
{
	const char* extensionString = DYNAMICEGL_CALL_FUNCTION(QueryString)(display, EGL_EXTENSIONS);
	return ::egl::internal::isExtensionSupported(extensionString, extensionName);
}
#ifndef DYNAMICEGL_NO_NAMESPACE
}
#endif
#undef _Log
#endif
