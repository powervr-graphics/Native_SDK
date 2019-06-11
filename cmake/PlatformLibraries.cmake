# This file provides various interface libraries used by the PowerVR SDK including windowing systems and platform specific libraries

if(NOT TARGET PlatformInterface)
	add_library(PlatformInterface INTERFACE)
endif()
if(NOT TARGET WindowSystemInterface)
	add_library(WindowSystemInterface INTERFACE)
endif()
if(NOT TARGET OpenGLESPlatformInterface)
	add_library(OpenGLESPlatformInterface INTERFACE)
endif()
if(NOT TARGET VulkanPlatformInterface)
	add_library(VulkanPlatformInterface INTERFACE)
endif()

list(APPEND PlatformInterface_Link_LIBS ${CMAKE_DL_LIBS})

if (WIN32)
# No extra libraries for windows. We need this if though as if it is an unknown platform we error out
elseif (ANDROID) #log and basic android library for Android
	find_library(lib-android android) #The following lines add Android specific libraries
	find_library(lib-log log)
	list(APPEND PlatformInterface_Link_LIBS ${lib-android} ${lib-log})
elseif (APPLE)
	if (IOS) #A ton of libraries for iOS and OSX. 
		find_library(lib-uikit UIKit)
		find_library(lib-foundation Foundation)
		find_library(lib-avfoundation AVFoundation)
		find_library(lib-quartz QuartzCore)
		find_library(lib-coremedia CoreMedia)
		find_library(lib-corevideo CoreVideo)
		list(APPEND PlatformInterface_Link_LIBS ${lib-uikit} ${lib-foundation} ${lib-avfoundation} ${lib-quartz} ${lib-coremedia} ${lib-corevideo})
		
		find_library(lib-gles OpenGLES)
		list(APPEND OpenGLESPlatformInterface_LINK_LIBS ${lib-gles})
	else()
		find_library(lib-appkit AppKit)
		list(APPEND PlatformInterface_Link_LIBS ${lib-appkit})
		
		find_library(lib-opencl OpenCL)
		list(APPEND OpenGLESPlatformInterface_LINK_LIBS ${lib-opencl})
	endif()
elseif (UNIX) # Mainly, add the windowing system libraries and include folders
	set(WS_DEFINE "" CACHE INTERNAL "")
	if(NOT WS) #We support building for several Windowing Systems. Typical desktop systems support X11 and Wayland is catching on. NullWS is used by some development platforms/ testchip.
		message (FATAL_ERROR "WS (Window System) Variable not set. Supported windowing systems can be enabled by passing: -DWS=NullWS, -DWS=X11, -DWS=Wayland, -DWS=Screen to CMake")
	endif()
	
	# Validate the use of -DWS
	if (${WS} STREQUAL X11 OR ${WS} STREQUAL XCB OR ${WS} STREQUAL NullWS OR ${WS} STREQUAL Screen)
		set(WS_DEFINE "${WS}" CACHE INTERNAL "")
	elseif(${WS} STREQUAL Wayland)
		set(WS_DEFINE "WAYLAND" CACHE INTERNAL "")
	else()
		message(FATAL_ERROR "Unrecognised WS: Valid values are NullWS(default), X11, Wayland, Screen.")
	endif()
	
	# Add a compiler definition so that our header files know what we're building for
	set(WindowSystemInterface_COMPILE_DEFINITIONS "${WindowSystemInterface_COMPILE_DEFINITIONS}" "${WS_DEFINE}")
	
	# Attempt to grab the CMAKE_PREFIX_PATH from the environment if it hasn't been set via command line
	if(NOT DEFINED CMAKE_PREFIX_PATH)
		set(CMAKE_PREFIX_PATH $ENV{CMAKE_PREFIX_PATH})
	endif()

	if (${WS} STREQUAL X11) # The user has requested the X11 libraries
		find_package(X11 REQUIRED)

		if(NOT ${X11_FOUND})
			message(FATAL_ERROR "X11 libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your X11 libraries")
		endif()
		
		list (APPEND WindowSystemInterface_LINK_LIBS ${X11_LIBRARIES})
		list (APPEND WindowSystemInterface_INCLUDE_DIRECTORIES ${X11_INCLUDE_DIR})

		if(X11_FOUND)
			set(VulkanPlatformInterface_COMPILE_DEFINITIONS "${VulkanPlatformInterface_COMPILE_DEFINITIONS}" "VK_USE_PLATFORM_XLIB_KHR")
		endif()
		find_package(X11_XCB)
		if(X11_XCB_FOUND)
			set(VulkanPlatformInterface_COMPILE_DEFINITIONS "${VulkanPlatformInterface_COMPILE_DEFINITIONS}" "VK_USE_PLATFORM_XCB_KHR")
		endif()
	elseif(${WS} STREQUAL XCB) # The user has requested the XCB libraries
		# XCB isn't currently being used by the PowerVR SDK but the following would allow you to link against the XCB libraries
		find_package(XCB REQUIRED)
		
		if(NOT ${XCB_FOUND})
			message("XCB libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your XCB libraries")
		endif()
			
		list (APPEND WindowSystemInterface_LINK_LIBS ${XCB_LIBRARIES})
		list (APPEND WindowSystemInterface_INCLUDE_DIRECTORIES ${XCB_INCLUDE_DIRS})
	elseif(${WS} STREQUAL Wayland) # The user has requested the Wayland libraries
		find_package(Wayland REQUIRED)
		
		if(NOT ${WAYLAND_FOUND})
			message("Wayland libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your Wayland libraries")
		endif()
		
		find_library(lib-ffi ffi HINTS ${PKG_WAYLAND_LIBRARY_DIRS})
		
		list (APPEND WindowSystemInterface_LINK_LIBS ${WAYLAND_CLIENT_LIBRARIES} ${lib-ffi})
		list (APPEND WindowSystemInterface_INCLUDE_DIRECTORIES ${WAYLAND_CLIENT_INCLUDE_DIR})
		
		list (APPEND OpenGLESPlatformInterface_LINK_LIBS ${WAYLAND_EGL_LIBRARIES})
		list (APPEND OpenGLESPlatformInterface_INCLUDE_DIRECTORIES ${WAYLAND_EGL_INCLUDE_DIR})
		
		set(VulkanPlatformInterface_COMPILE_DEFINITIONS "${VulkanPlatformInterface_COMPILE_DEFINITIONS}" "VK_USE_PLATFORM_WAYLAND_KHR")
	elseif(${WS} STREQUAL Screen)
		if(CMAKE_SYSTEM_NAME MATCHES "QNX")
			list (APPEND WindowSystemInterface_LINK_LIBS "screen")
		endif()
	elseif(${WS} STREQUAL NullWS) # The user has requested no windowing system (direct-to-framebuffer)
	else()
		message(FATAL_ERROR "Unrecognised WS: Valid values are NullWS(default), X11, Wayland, Screen." )
	endif()

	if(NOT CMAKE_SYSTEM_NAME MATCHES "QNX")
		find_package(Threads)
		list(APPEND PlatformInterface_Link_LIBS ${CMAKE_THREAD_LIBS_INIT})
		list(APPEND PlatformInterface_Link_LIBS "rt")
	endif()
else()
	message(FATAL_ERROR "UNKNOWN PLATFORM - Please set this up with platform-specific dependencies")
endif()

set_target_properties(PlatformInterface PROPERTIES
	INTERFACE_LINK_LIBRARIES "${PlatformInterface_Link_LIBS}")

set_target_properties(WindowSystemInterface PROPERTIES
	INTERFACE_LINK_LIBRARIES "${WindowSystemInterface_LINK_LIBS}"
	INTERFACE_INCLUDE_DIRECTORIES "${WindowSystemInterface_INCLUDE_DIRECTORIES}"
	INTERFACE_COMPILE_DEFINITIONS "${WindowSystemInterface_COMPILE_DEFINITIONS}")
	
list(APPEND OpenGLESPlatformInterface_LINK_LIBS WindowSystemInterface PlatformInterface)
set_target_properties(OpenGLESPlatformInterface PROPERTIES
	INTERFACE_LINK_LIBRARIES "${OpenGLESPlatformInterface_LINK_LIBS}"
	INTERFACE_INCLUDE_DIRECTORIES "${OpenGLESPlatformInterface_INCLUDE_DIRECTORIES}")	
	
list(APPEND VulkanPlatformInterface_LINK_LIBS WindowSystemInterface PlatformInterface)
set_target_properties(VulkanPlatformInterface PROPERTIES
	INTERFACE_LINK_LIBRARIES "${VulkanPlatformInterface_LINK_LIBS}"
	INTERFACE_COMPILE_DEFINITIONS "${VulkanPlatformInterface_COMPILE_DEFINITIONS}")