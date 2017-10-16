/*!
\brief Include this file if you want to directly use EGL extensions in your code. eglext class is populated
automatically.
\file PVRNativeApi/EGL/ExtensionLoaderEgl.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRNativeApi/EGL/ApiEgl.h"
#include "PVRNativeApi/EGL/NativeLibraryEgl.h"

/// <summary>This class contains function pointers for all EGL extensions that existed at the time of publishing.
/// Can be updated as required. The function pointers get automatically populated on application start during State
/// Machine creation. (function pvr::createNativePlatformContext). Use as a namespace (e.g. eglext::WaitSyncKHR).
/// </summary>
class eglext
{
public:
//!\cond NO_DOXYGEN

	// EGL_KHR_lock_surface
	static PROC_EGL_eglLockSurfaceKHR LockSurfaceKHR;
	static PROC_EGL_eglUnlockSurfaceKHR UnlockSurfaceKHR;

	// EGL_KHR_image*
	static PROC_EGL_eglCreateImageKHR CreateImageKHR;
	static PROC_EGL_eglDestroyImageKHR DestroyImageKHR;

	// EGL_KHR_reusable_sync
	static PROC_EGL_eglCreateSyncKHR CreateSyncKHR;
	static PROC_EGL_eglDestroySyncKHR DestroySyncKHR;
	static PROC_EGL_eglClientWaitSyncKHR ClientWaitSyncKHR;
	static PROC_EGL_eglGetSyncAttribKHR GetSyncAttribKHR;
	static PROC_EGL_eglSignalSyncKHR SignalSyncKHR;

	// EGL_KHR_wait_sync
	static PROC_EGL_eglWaitSyncKHR WaitSyncKHR;

	// EGL_ANDROID_swap_rectangle
	static PROC_EGL_eglSetSwapRectangleANDROID SetSwapRectangleANDROID;

	// EGL_ANDROID_get_render_buffer
	static PROC_EGL_eglGetRenderBufferANDROID GetRenderBufferANDROID;

	// EGL_ANDROID_blob_cache
	static PROC_EGL_eglSetBlobCacheFuncsANDROID SetBlobCacheFuncsANDROID;

	// EGL_ANDROID_native_fence_sync
	static PROC_EGL_eglDupNativeFenceFDANDROID DupNativeFenceFDANDROID ;
	static PROC_EGL_eglWaitSyncANDROID WaitSyncANDROID;

	// EGL_IMG_hibernate_process
	static PROC_EGL_eglHibernateProcessIMG HibernateProcessIMG;
	static PROC_EGL_eglAwakenProcessIMG AwakenProcessIMG;

	// EGL_EXT_swap_buffers_with_damage
	static PROC_EGL_eglSwapBuffersWithDamageEXT SwapBuffersWithDamageEXT;

	static void initEglExt();
};

namespace pvr {
/// <summary>The pvr::native namespace contains low-level classes and functions that are relevant to the
/// underlying Graphics API</summary>
namespace native {
/// <summary>EglExtensions class can be used to facilitate loading and using EGL extensions</summary>
class EglExtensions
{
public:
	// EGL_KHR_lock_surface
	bool supports_EGL_KHR_lock_surface;

	// EGL_KHR_image*
	bool supports_EGL_KHR_image_base;
	bool supports_EGL_KHR_image;

	// EGL_KHR_reusable_sync
	bool supports_EGL_KHR_fence_sync;
	bool supports_EGL_KHR_reusable_sync;

	// EGL_KHR_wait_sync
	bool supports_EGL_KHR_wait_sync;

	// EGL_ANDROID_swap_rectangle
	bool supports_EGL_ANDROID_swap_rectangle;

	// EGL_ANDROID_get_render_buffer
	bool supports_EGL_ANDROID_get_render_buffer;

	// EGL_ANDROID_blob_cache
	bool supports_EGL_ANDROID_blob_cache;

	// EGL_ANDROID_native_fence_sync
	bool supports_EGL_ANDROID_native_fence_sync;

	// EGL_IMG_hibernate_process
	bool supports_EGL_IMG_hibernate_process;

	// EGL_EXT_swap_buffers_with_damage
	bool supports_EGL_EXT_swap_buffers_with_damage;

	// EGL_ANDROID_image_native_buffer
	bool supports_EGL_ANDROID_image_native_buffer;

	// EGL_KHR_gl_texture_2D_image
	bool supports_EGL_KHR_gl_texture_2D_image;

	// EGL_KHR_gl_texture_cubemap_image
	bool supports_EGL_KHR_gl_texture_cubemap_image;

	// EGL_KHR_gl_texture_3D_image
	bool supports_EGL_KHR_gl_texture_3D_image;

	// EGL_KHR_gl_renderbuffer_image
	bool supports_EGL_KHR_gl_renderbuffer_image;

	// EGL_EXT_create_context_robustness
	bool supports_EGL_EXT_create_context_robustness;

	// EGL_KHR_create_context
	bool supports_EGL_KHR_create_context;

	// EGL_ANDROID_recordable
	bool supports_EGL_ANDROID_recordable;

	// EGL_ANDROID_framebuffer_target
	bool supports_EGL_ANDROID_framebuffer_target;

	// EGL_NOK_texture_from_pixmap
	bool supports_EGL_NOK_texture_from_pixmap;

	// EGL_EXT_buffer_age
	bool supports_EGL_EXT_buffer_age;

	bool isInitialized;

	EglExtensions();
	void init(const char* const extensions);
//!\endcond
};
}
}