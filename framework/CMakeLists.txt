cmake_minimum_required(VERSION 3.10)
# Copyright (c) Imagination Technologies Limited.

project(PowerVR_Framework)

# Add the default parts of the framework
add_subdirectory_if_exists(PVRCore PVRCore)
add_subdirectory_if_exists(PVRAssets PVRAssets)
add_subdirectory_if_exists(PVRShell PVRShell)
add_subdirectory_if_exists(PVRCamera PVRCamera)
add_subdirectory_if_exists(PVRUtilsGles PVRUtils/OpenGLES)
add_subdirectory_if_exists(PVRUtilsGlsc PVRUtils/OpenGLSC)

set_property(GLOBAL APPEND PROPERTY PVR_FRAMEWORK_TARGETS PVRCore PVRShell PVRAssets PVRCamera PVRUtilsGles)

if(NOT IOS)
	add_subdirectory_if_exists(PVRUtilsCL PVRUtils/OpenCL)
endif()

###################################################################################
# NOTE: Test whether the code below has isues with Android builds
###################################################################################
if(((${MOLTENVK_FOUND}) OR (NOT APPLE)) AND NOT CMAKE_SYSTEM_NAME MATCHES "QNX")
	add_subdirectory_if_exists(PVRUtilsVk PVRUtils/Vulkan)
	add_subdirectory_if_exists(PVRPfx PVRPfx)
	set(PVRVK_USE_PLATFORM "" CACHE INTERNAL "")

	if (WIN32)
		list(APPEND PVRVK_USE_PLATFORM "VK_USE_PLATFORM_WIN32_KHR")
	elseif(ANDROID)
		list(APPEND PVRVK_USE_PLATFORM "VK_USE_PLATFORM_ANDROID_KHR")
	elseif(APPLE)
		if (${MOLTENVK_FOUND})
			list(APPEND PVRVK_USE_PLATFORM "VK_USE_PLATFORM_MACOS_MVK")
		endif()	
	elseif(UNIX)
		if(PVR_WINDOW_SYSTEM)
			if (${PVR_WINDOW_SYSTEM} STREQUAL X11)
				list(APPEND PVRVK_USE_PLATFORM "VK_USE_PLATFORM_XLIB_KHR")
			elseif (${PVR_WINDOW_SYSTEM} STREQUAL XCB)
				list(APPEND PVRVK_USE_PLATFORM "VK_USE_PLATFORM_XCB_KHR")
			elseif (${PVR_WINDOW_SYSTEM} STREQUAL Wayland)
				list(APPEND PVRVK_USE_PLATFORM "VK_USE_PLATFORM_WAYLAND_KHR")
			endif()
		endif()
	endif()

	if(${PVR_ENABLE_INSTALL})
		if(NOT PVRVK_SKIP_INSTALL)
  			set(PVRVK_SKIP_INSTALL OFF)
		endif()
	endif()
	add_subdirectory_if_exists(PVRVk PVRVk)
	set_property(GLOBAL APPEND PROPERTY PVR_FRAMEWORK_TARGETS PVRVk PVRUtilsVk PVRPfx)
endif()