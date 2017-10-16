/*!
\brief Contains an implementation of extension tracking and loading for EGL. See relevant header file.
\file PVRNativeApi/EGL/ExtensionLoaderEgl.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRNativeApi/EGL/ExtensionLoaderEgl.h"
#include <string.h>

PROC_EGL_eglLockSurfaceKHR eglext::LockSurfaceKHR = NULL;
PROC_EGL_eglUnlockSurfaceKHR eglext::UnlockSurfaceKHR = NULL;
PROC_EGL_eglCreateImageKHR eglext::CreateImageKHR = NULL;
PROC_EGL_eglDestroyImageKHR eglext::DestroyImageKHR = NULL;
PROC_EGL_eglCreateSyncKHR eglext::CreateSyncKHR = NULL;
PROC_EGL_eglDestroySyncKHR eglext::DestroySyncKHR = NULL;
PROC_EGL_eglClientWaitSyncKHR eglext::ClientWaitSyncKHR = NULL;
PROC_EGL_eglSignalSyncKHR eglext::SignalSyncKHR = NULL;
PROC_EGL_eglGetSyncAttribKHR eglext::GetSyncAttribKHR = NULL;
PROC_EGL_eglWaitSyncKHR eglext::WaitSyncKHR = NULL;
PROC_EGL_eglSetSwapRectangleANDROID eglext::SetSwapRectangleANDROID = NULL;
PROC_EGL_eglGetRenderBufferANDROID eglext::GetRenderBufferANDROID = NULL;
PROC_EGL_eglSetBlobCacheFuncsANDROID eglext::SetBlobCacheFuncsANDROID = NULL;
PROC_EGL_eglDupNativeFenceFDANDROID eglext::DupNativeFenceFDANDROID = NULL;
PROC_EGL_eglWaitSyncANDROID eglext::WaitSyncANDROID = NULL;
PROC_EGL_eglHibernateProcessIMG eglext::HibernateProcessIMG = NULL;
PROC_EGL_eglAwakenProcessIMG eglext::AwakenProcessIMG = NULL;
PROC_EGL_eglSwapBuffersWithDamageEXT eglext::SwapBuffersWithDamageEXT = NULL;

namespace pvr {
namespace native {

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

EglExtensions::EglExtensions() { memset(this, 0, sizeof(EglExtensions)); }

void EglExtensions::init(const char* const extensions)
{
	// EGL_KHR_lock_surface
	supports_EGL_KHR_lock_surface = isExtensionSupported(extensions, "EGL_KHR_lock_surface");

	// EGL_KHR_image
	supports_EGL_KHR_image_base = isExtensionSupported(extensions, "EGL_KHR_image_base");
	supports_EGL_KHR_image = isExtensionSupported(extensions, "EGL_KHR_image");

	// EGL_ANDROID_swap_rectangle
	supports_EGL_ANDROID_swap_rectangle = isExtensionSupported(extensions, "EGL_ANDROID_swap_rectangle");

	// EGL_ANDROID_get_render_buffer
	supports_EGL_ANDROID_get_render_buffer = isExtensionSupported(extensions, "EGL_ANDROID_get_render_buffer");

	// EGL_ANDROID_blob_cache
	supports_EGL_ANDROID_blob_cache = isExtensionSupported(extensions, "EGL_ANDROID_blob_cache");

	// EGL_ANDROID_native_fence_sync
	supports_EGL_ANDROID_native_fence_sync = isExtensionSupported(extensions, "EGL_ANDROID_native_fence_sync");

	// EGL_IMG_hibernate_process
	supports_EGL_IMG_hibernate_process = isExtensionSupported(extensions, "EGL_IMG_hibernate_process");

	// EGL_EXT_swap_buffers_with_damage
	supports_EGL_EXT_swap_buffers_with_damage = isExtensionSupported(extensions, "EGL_EXT_swap_buffers_with_damage");

	// EGL_KHR_reusable_sync & EGL_KHR_fence_sync
	supports_EGL_KHR_reusable_sync = isExtensionSupported(extensions, "EGL_KHR_reusable_sync");
	supports_EGL_KHR_fence_sync = isExtensionSupported(extensions, "EGL_KHR_fence_sync");

	// EGL_KHR_wait_sync
	supports_EGL_KHR_wait_sync = isExtensionSupported(extensions, "EGL_KHR_wait_sync");

	// EGL_ANDROID_image_native_buffer
	supports_EGL_ANDROID_image_native_buffer = isExtensionSupported(extensions, "EGL_ANDROID_image_native_buffer");

	// EGL_KHR_gl_texture_2D_image
	supports_EGL_KHR_gl_texture_2D_image = isExtensionSupported(extensions, "EGL_KHR_gl_texture_2D_image");

	// EGL_KHR_gl_texture_cubemap_image
	supports_EGL_KHR_gl_texture_cubemap_image = isExtensionSupported(extensions, "EGL_KHR_gl_texture_cubemap_image");

	// EGL_KHR_gl_texture_3D_image
	supports_EGL_KHR_gl_texture_3D_image = isExtensionSupported(extensions, "EGL_KHR_gl_texture_3D_image");

	// EGL_KHR_gl_renderbuffer_image
	supports_EGL_KHR_gl_renderbuffer_image = isExtensionSupported(extensions, "EGL_KHR_gl_renderbuffer_image");

	// EGL_EXT_create_context_robustness
	supports_EGL_EXT_create_context_robustness = isExtensionSupported(extensions, "EGL_EXT_create_context_robustness");

	// EGL_KHR_create_context
	supports_EGL_KHR_create_context = isExtensionSupported(extensions, "EGL_KHR_create_context");

	// EGL_KHR_create_context
	supports_EGL_ANDROID_recordable = isExtensionSupported(extensions, "EGL_ANDROID_recordable");

	// EGL_ANDROID_framebuffer_target
	supports_EGL_ANDROID_framebuffer_target = isExtensionSupported(extensions, "EGL_ANDROID_framebuffer_target");

	// EGL_NOK_texture_from_pixmap
	supports_EGL_NOK_texture_from_pixmap = isExtensionSupported(extensions, "EGL_NOK_texture_from_pixmap");

	// EGL_EXT_buffer_age
	supports_EGL_EXT_buffer_age = isExtensionSupported(extensions, "EGL_EXT_buffer_age");
	isInitialized = true;
}
}
}
void eglext::initEglExt()
{
#ifndef TARGET_OS_IPHONE
	// EGL_KHR_lock_surface
	eglext::LockSurfaceKHR = reinterpret_cast<PROC_EGL_eglLockSurfaceKHR>(egl::GetProcAddress("eglLockSurfaceKHR"));
	eglext::UnlockSurfaceKHR = reinterpret_cast<PROC_EGL_eglUnlockSurfaceKHR>(egl::GetProcAddress("eglUnlockSurfaceKHR"));

	// EGL_KHR_image
	eglext::CreateImageKHR = reinterpret_cast<PROC_EGL_eglCreateImageKHR>(egl::GetProcAddress("eglCreateImageKHR"));
	eglext::DestroyImageKHR = reinterpret_cast<PROC_EGL_eglDestroyImageKHR>(egl::GetProcAddress("eglDestroyImageKHR"));

	// EGL_ANDROID_swap_rectangle
	eglext::SetSwapRectangleANDROID = reinterpret_cast<PROC_EGL_eglSetSwapRectangleANDROID>(egl::GetProcAddress("eglSetSwapRectangleANDROID"));

	// EGL_ANDROID_get_render_buffer
	eglext::GetRenderBufferANDROID = reinterpret_cast<PROC_EGL_eglGetRenderBufferANDROID>(egl::GetProcAddress("eglGetRenderBufferANDROID"));

	// EGL_ANDROID_blob_cache
	eglext::SetBlobCacheFuncsANDROID = reinterpret_cast<PROC_EGL_eglSetBlobCacheFuncsANDROID>(egl::GetProcAddress("eglSetBlobCacheFuncsANDROID"));

	// EGL_ANDROID_native_fence_sync
	eglext::DupNativeFenceFDANDROID = reinterpret_cast<PROC_EGL_eglDupNativeFenceFDANDROID>(egl::GetProcAddress("eglDupNativeFenceFDANDROID"));
	eglext::WaitSyncANDROID = reinterpret_cast<PROC_EGL_eglWaitSyncANDROID>(egl::GetProcAddress("eglWaitSyncANDROID"));

	// EGL_IMG_hibernate_process
	eglext::HibernateProcessIMG = reinterpret_cast<PROC_EGL_eglHibernateProcessIMG>(egl::GetProcAddress("eglHibernateProcessIMG"));
	eglext::AwakenProcessIMG = reinterpret_cast<PROC_EGL_eglAwakenProcessIMG>(egl::GetProcAddress("eglAwakenProcessIMG"));

	// EGL_EXT_swap_buffers_with_damage

	eglext::SwapBuffersWithDamageEXT = reinterpret_cast<PROC_EGL_eglSwapBuffersWithDamageEXT>(egl::GetProcAddress("eglSwapBuffersWithDamageEXT"));

	// EGL_KHR_reusable_sync & EGL_KHR_fence_sync
	eglext::CreateSyncKHR = reinterpret_cast<PROC_EGL_eglCreateSyncKHR>(egl::GetProcAddress("eglCreateSyncKHR"));
	eglext::DestroySyncKHR = reinterpret_cast<PROC_EGL_eglDestroySyncKHR>(egl::GetProcAddress("eglDestroySyncKHR"));
	eglext::ClientWaitSyncKHR = reinterpret_cast<PROC_EGL_eglClientWaitSyncKHR>(egl::GetProcAddress("eglClientWaitSyncKHR"));
	eglext::GetSyncAttribKHR = reinterpret_cast<PROC_EGL_eglGetSyncAttribKHR>(egl::GetProcAddress("eglGetSyncAttribKHR"));
	eglext::SignalSyncKHR = reinterpret_cast<PROC_EGL_eglSignalSyncKHR>(egl::GetProcAddress("eglSignalSyncKHR"));

	// EGL_KHR_wait_sync
	eglext::WaitSyncKHR = reinterpret_cast<PROC_EGL_eglWaitSyncKHR>(egl::GetProcAddress("eglWaitSyncKHR"));
#else
	// EGL_KHR_lock_surface
	eglext::LockSurfaceKHR = &eglLockSurfaceKHR;
	eglext::UnlockSurfaceKHR = &eglUnlockSurfaceKHR;

	// EGL_KHR_image
	eglext::CreateImageKHR = &eglCreateImageKHR;
	eglext::DestroyImageKHR = &eglDestroyImageKHR;

	// EGL_ANDROID_swap_rectangle
	eglext::SetSwapRectangleANDROID = &eglSetSwapRectangleANDROID;

	// EGL_ANDROID_get_render_buffer
	eglext::GetRenderBufferANDROID = &eglGetRenderBufferANDROID;

	// EGL_ANDROID_blob_cache
	eglext::SetBlobCacheFuncsANDROID = &eglSetBlobCacheFuncsANDROID;

	// EGL_ANDROID_native_fence_sync
	eglext::DupNativeFenceFDANDROID = &eglDupNativeFenceFDANDROID;
	eglext::WaitSyncANDROID = &eglWaitSyncANDROID;

	// EGL_IMG_hibernate_process
	eglext::HibernateProcessIMG = &eglHibernateProcessIMG;
	eglext::AwakenProcessIMG = &eglAwakenProcessIMG;

	// EGL_EXT_swap_buffers_with_damage

	eglext::SwapBuffersWithDamageEXT = &eglSwapBuffersWithDamageEXT;

	// EGL_KHR_reusable_sync & EGL_KHR_fence_sync
	eglext::CreateSyncKHR = &eglCreateSyncKHR;
	eglext::DestroySyncKHR = &eglDestroySyncKHR;
	eglext::ClientWaitSyncKHR = &eglClientWaitSyncKHR;
	eglext::GetSyncAttribKHR = &eglGetSyncAttribKHR;
	eglext::SignalSyncKHR = &eglSignalSyncKHR;

	// EGL_KHR_wait_sync
	eglext::WaitSyncKHR = &eglWaitSyncKHR;
#endif
}
//!\endcond