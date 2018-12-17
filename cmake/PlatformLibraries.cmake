
if (WIN32)
# No extra libraries for windows. We need this if though as if it is an unknown platform we error out
elseif (ANDROID) #log and basic android library for Android
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate") #This is to prevent the compiler for stripping out our "main" function
	find_library(lib-android android) #The following lines add Android specific libraries
	find_library(lib-log log)
	list(APPEND EXTRA_LIBS ${lib-android} ${lib-log})
elseif (APPLE)
	if (IOS) #A ton of libraries for iOS and OSX. 
		set (PLATFORM_FOLDER iOS)
		find_library(lib-uikit UIKit)
		find_library(lib-gles OpenGLES)
		find_library(lib-foundation Foundation)
		find_library(lib-avfoundation AVFoundation)
		find_library(lib-quartz QuartzCore)
		find_library(lib-coremedia CoreMedia)
		find_library(lib-corevideo CoreVideo)
		list(APPEND EXTRA_LIBS ${lib-uikit} ${lib-gles} ${lib-foundation} ${lib-avfoundation} ${lib-quartz} ${lib-coremedia} ${lib-corevideo})
	else()
		set (PLATFORM_FOLDER macOS_x86)
		find_library(lib-appkit AppKit)
		list(APPEND EXTRA_LIBS ${lib-appkit})
		if(DEFINED OPENGLES_EXAMPLE)
			find_library(lib-opencl OpenCL)
			list(APPEND EXTRA_LIBS ${lib-opencl})
		endif()
	endif()
elseif (UNIX) # Mainly, add the windowing system libraries and include folders
	set(WS_DEFINE "")

	if(NOT DEFINED CMAKE_PREFIX_PATH)
		set(CMAKE_PREFIX_PATH $ENV{CMAKE_PREFIX_PATH})
	endif()

	if (${WS} STREQUAL X11) # The user has requested the X11 libraries
		find_package(X11 REQUIRED)

		if(NOT ${X11_FOUND})
			message(FATAL_ERROR "X11 libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your X11 libraries")
		endif()

		list (APPEND EXTRA_LIBS ${X11_LIBRARIES})
		include_directories(${X11_INCLUDE_DIR})

		if(X11_FOUND)
			add_definitions(-DVK_USE_PLATFORM_XLIB_KHR)
		endif()
		find_package(X11_XCB)
		if(X11_XCB_FOUND)
			add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
		endif()
	elseif(${WS} STREQUAL XCB) # The user has requested the XCB libraries
		# XCB isn't currently being used by the PowerVR SDK but the following would allow you to link against the XCB libraries
		find_package(XCB REQUIRED)
		
		if(NOT ${XCB_FOUND})
			message("XCB libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your XCB libraries")
		endif()
		
		list (APPEND EXTRA_LIBS ${XCB_LIBRARIES})
		include_directories(${XCB_INCLUDE_DIRS})
	elseif(${WS} STREQUAL Wayland) # The user has requested the Wayland libraries
		find_package(Wayland REQUIRED)
		if(DEFINED OPENGLES_EXAMPLE)
			list (APPEND EXTRA_LIBS ${WAYLAND_EGL_LIBRARIES})
			include_directories(${WAYLAND_EGL_INCLUDE_DIR})
		endif()
		
		if(NOT ${WAYLAND_FOUND})
			message("Wayland libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your Wayland libraries")
		endif()
		
		find_library(lib-ffi ffi HINTS ${PKG_WAYLAND_LIBRARY_DIRS})
		list (APPEND EXTRA_LIBS ${WAYLAND_CLIENT_LIBRARIES} ${lib-ffi})
		include_directories(${WAYLAND_CLIENT_INCLUDE_DIR})
		add_definitions(-DVK_USE_PLATFORM_WAYLAND_KHR)
	elseif(${WS} STREQUAL Screen)
		if(CMAKE_SYSTEM_NAME MATCHES "QNX")
			list (APPEND EXTRA_LIBS "screen")
		endif()
	elseif(${WS} STREQUAL NullWS) # The user has requested no windowing system (direct-to-framebuffer)
	else()
		message(FATAL_ERROR "Unrecognised WS: Valid values are NullWS(default), X11, Wayland, Screen." )
	endif()

	if(NOT CMAKE_SYSTEM_NAME MATCHES "QNX")
		find_package(Threads)
		list (APPEND EXTRA_LIBS ${CMAKE_THREAD_LIBS_INIT})
		list (APPEND EXTRA_LIBS "rt")
	endif()
else()
	message(FATAL_ERROR "UNKNOWN PLATFORM - Please set this up with platform-specific dependencies")
endif()
