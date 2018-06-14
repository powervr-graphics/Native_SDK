cmake_minimum_required(VERSION 3.3)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set (CMAKE_CXX_STANDARD 14)
set (SDK_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../..)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${SDK_ROOT}/cmake/modules")  #for find_package

if (WIN32) #Disable a useless MSVC warning
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(NOT CMAKE_BUILD_TYPE) #Default to release if the user passes nothing.
	set(CMAKE_BUILD_TYPE "Release")
endif()

project(OpenGLESHelloAPI)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESHelloAPI)

include_directories(
	${SDK_ROOT}/include/
)

if (WIN32)
	set (SRC_FILES 
		 OpenGLESHelloAPI_Windows.cpp
		"${SDK_ROOT}/res/Windows/shared.rc"
		"${SDK_ROOT}/res/Windows/resource.h")
	add_executable( OpenGLESHelloAPI WIN32 ${SRC_FILES})
elseif (ANDROID)
	find_library( lib-android android)
	find_library( lib-log log )
	find_library( lib-dl dl) 
	set(EXTRA_LIBS ${lib-android} ${lib-log} ${lib-dl} )
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate" )
	set(SRC_FILES 
		OpenGLESHelloAPI_Android.cpp 
		${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
	)
	include_directories(${ANDROID_NDK}/sources/android/native_app_glue)
	add_library( OpenGLESHelloAPI SHARED ${SRC_FILES} )	
elseif (APPLE)
	if (IOS)
		set ( SRC_FILES OpenGLESHelloAPI_iOS.mm)
		set(INFO_PLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/iOS_Info.plist")
		file(GLOB ICONS LIST_DIRECTORIES false ${SDK_ROOT}/SDK/res/iOS/* ${SDK_ROOT}/SDK/res/iOS/OpenGLES3/*)
		list(APPEND ASSET_FILES ${ICONS})
		find_library(lib-uikit UIKit)
		find_library(lib-gles OpenGLES)
		find_library(lib-foundation Foundation)
		find_library(lib-quartz QuartzCore)
		find_library(lib-coregraphics CoreGraphics)
		set (libs ${lib-uikit} ${lib-gles} ${lib-foundation} ${lib-quartz} ${lib-coregraphics})
		set(EXTRA_LIBS ${libs})
	else()
		set (PLATFORM_FOLDER macOS_x86)
		set(BUILDS_LIB_FOLDER "${SDK_ROOT}/lib/${PLATFORM_FOLDER}")
		set(SRC_FILES OpenGLESHelloAPI_macOS.mm)
		set(INFO_PLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/macOS_Info.plist")
		list(APPEND FRAMEWORK_FILES "${BUILDS_LIB_FOLDER}/libEGL.dylib" "${BUILDS_LIB_FOLDER}/libGLESv2.dylib")
		set_source_files_properties(${FRAMEWORK_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
		source_group(Frameworks FILES ${FRAMEWORK_FILES})
		find_library(libs AppKit)
		set(EXTRA_LIBS ${libs})
		list(APPEND ASSET_FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/MainMenu.xib")
	endif()
	source_group(Resources FILES ${ASSET_FILES})
	set_source_files_properties(${ASSET_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

	add_executable(OpenGLESHelloAPI MACOSX_BUNDLE ${SRC_FILES} ${ASSET_FILES} ${FRAMEWORK_FILES})
	target_link_libraries(OpenGLESHelloAPI ${EXTRA_LIBS})
	set_target_properties(OpenGLESHelloAPI PROPERTIES MACOSX_BUNDLE_INFO_PLIST  "${INFO_PLIST_FILE}")
	set_target_properties(OpenGLESHelloAPI PROPERTIES RESOURCE "${ASSET_FILES}")
	
elseif (UNIX)
	if(NOT WS) #We support building for several Windowing Systems. Typical desktop systems support X11 and XCB, Wayland is catching on. NullWS is used by some development platforms/ testchip.
		message ("WS Variable not set. Assuming NullWS. If you wish to build for X11, Wayland or another supported windowing system, please pass -DWS=X11 or -DWS=Wayland etc. to CMake")
		set(WS "NullWS")
	endif()
	
	set(EXTRA_LIBS ${CMAKE_DL_LIBS})
	
	if(NOT DEFINED CMAKE_PREFIX_PATH)
	set(CMAKE_PREFIX_PATH $ENV{CMAKE_PREFIX_PATH})
	endif()
	
	if (${WS} STREQUAL X11) #The following lines add the specified windowing system libraries
		find_package(X11 REQUIRED)
		list (APPEND EXTRA_LIBS ${X11_LIBRARIES})
		include_directories(${X11_INCLUDE_DIR})
		set(SRC_FILES OpenGLESHelloAPI_LinuxX11.cpp)
	elseif(${WS} STREQUAL Wayland)
		find_package(Wayland REQUIRED)
		find_library(lib-ffi ffi HINTS ${PKG_WAYLAND_LIBRARY_DIRS})
		list (APPEND EXTRA_LIBS ${WAYLAND_CLIENT_LIBRARIES} ${WAYLAND_EGL_LIBRARIES} ${lib-ffi})
		include_directories(${WAYLAND_CLIENT_INCLUDE_DIR} ${WAYLAND_EGL_INCLUDE_DIR})
		set(SRC_FILES OpenGLESHelloAPI_LinuxWayland.cpp)
	elseif(${WS} STREQUAL NullWS)
		set(SRC_FILES OpenGLESHelloAPI_NullWS.cpp)
	elseif(${WS} STREQUAL Screen)
		if(CMAKE_SYSTEM_NAME MATCHES "QNX")
			list (APPEND EXTRA_LIBS "screen")
			set(SRC_FILES OpenGLESHelloAPI_QNXScreen.cpp)
		endif()
	else()
		message( FATAL_ERROR "Unrecognised WS: Valid values are NullWS(default), X11, Wayland, Screen." )
	endif()
	
	if(CMAKE_SYSTEM_NAME MATCHES "QNX")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-ignored-attributes")
	endif()

	add_definitions(-D${WS}) #Add a compiler definition so that our header files know what we're building for
	add_executable(OpenGLESHelloAPI ${SRC_FILES})
endif()

if (EXTRA_LIBS)
	target_link_libraries(	OpenGLESHelloAPI ${EXTRA_LIBS} )
endif()
target_compile_definitions( OpenGLESHelloAPI PUBLIC $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:RELEASE=1> ) #Defines DEBUG=1 or RELEASE=1
