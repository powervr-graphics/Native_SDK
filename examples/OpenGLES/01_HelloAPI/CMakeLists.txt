cmake_minimum_required(VERSION 3.3)
project(OpenGLESHelloAPI)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set (CMAKE_CXX_STANDARD 14)
set (SDK_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../..)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${SDK_ROOT}/cmake/modules")  #for find_package

if (WIN32)
	#Get rid of the "this function is unsafe" warning in Visual Studio
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(NOT CMAKE_BUILD_TYPE) #Default to release if the user passes nothing.
	set(CMAKE_BUILD_TYPE "Release")
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESHelloAPI)

set(HelloAPI_INCLUDE_DIRECTORIES ${SDK_ROOT}/include/)

list(APPEND PLATFORM_LIBS ${CMAKE_DL_LIBS})

if (WIN32)
	set (SRC_FILES 
		 OpenGLESHelloAPI_Windows.cpp
		"${SDK_ROOT}/res/Windows/shared.rc"
		"${SDK_ROOT}/res/Windows/resource.h")
	add_executable(OpenGLESHelloAPI WIN32 ${SRC_FILES})
elseif (ANDROID)
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate") #This is to prevent the compiler for stripping out our "main" function
	find_library(lib-android android) #The following lines add Android specific libraries
	find_library(lib-log log)
	list(APPEND PLATFORM_LIBS ${lib-android} ${lib-log})
	set(SRC_FILES 
		OpenGLESHelloAPI_Android.cpp 
		${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
	)
	list(APPEND HelloAPI_INCLUDE_DIRECTORIES ${ANDROID_NDK}/sources/android/native_app_glue)
	add_library(OpenGLESHelloAPI SHARED ${SRC_FILES})
	# Force export ANativeActivity_onCreate(),
	# Refer to: https://github.com/android-ndk/ndk/issues/381
	set_property(TARGET OpenGLESHelloAPI APPEND_STRING PROPERTY LINK_FLAGS " -u ANativeActivity_onCreate")
elseif (APPLE)
	if (IOS)
		set(SRC_FILES OpenGLESHelloAPI_iOS.mm)
		set(INFO_PLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/iOS_Info.plist")
		file(GLOB ICONS LIST_DIRECTORIES false ${SDK_ROOT}/SDK/res/iOS/* ${SDK_ROOT}/SDK/res/iOS/OpenGLES3/*)
		list(APPEND ASSET_FILES ${ICONS})
		find_library(lib-uikit UIKit)
		find_library(lib-gles OpenGLES)
		find_library(lib-foundation Foundation)
		find_library(lib-quartz QuartzCore)
		find_library(lib-coregraphics CoreGraphics)
		list(APPEND PLATFORM_LIBS ${lib-uikit} ${lib-gles} ${lib-foundation} ${lib-quartz} ${lib-coregraphics})
	else()
		set(PLATFORM_FOLDER macOS_x86)
		set(BUILDS_LIB_FOLDER "${SDK_ROOT}/lib/${PLATFORM_FOLDER}")
		set(SRC_FILES OpenGLESHelloAPI_macOS.mm)
		set(INFO_PLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/macOS_Info.plist")
		list(APPEND FRAMEWORK_FILES "${BUILDS_LIB_FOLDER}/libEGL.dylib" "${BUILDS_LIB_FOLDER}/libGLESv2.dylib")
		set_source_files_properties(${FRAMEWORK_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
		source_group(Frameworks FILES ${FRAMEWORK_FILES})
		find_library(lib-AppKit AppKit)
		list(APPEND PLATFORM_LIBS ${lib-AppKit})
		list(APPEND ASSET_FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/MainMenu.xib")
	endif()
	
	if (IOS) #Kill some annoying warnings
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type" CACHE STRING "")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-return-type" CACHE STRING "")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wno-return-type" CACHE STRING "")
	elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
		set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
		set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
		set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
	endif()
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -all_load") # PREVENT THE LINKER FROM STRIPPING OUT THE FUNCTIONS THAT macOS CALLS REFLECTIVELY
	
	source_group(Resources FILES ${ASSET_FILES})
	set_source_files_properties(${ASSET_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

	add_executable(OpenGLESHelloAPI MACOSX_BUNDLE ${SRC_FILES} ${ASSET_FILES} ${FRAMEWORK_FILES})
	set_target_properties(OpenGLESHelloAPI PROPERTIES MACOSX_BUNDLE_INFO_PLIST  "${INFO_PLIST_FILE}")
	set_target_properties(OpenGLESHelloAPI PROPERTIES RESOURCE "${ASSET_FILES}")
elseif (UNIX)
	set(WS_DEFINE "")
	if(NOT WS) #We support building for several Windowing Systems. Typical desktop systems support X11 and Wayland is catching on. NullWS is used by some development platforms/ testchip.
		message ("WS Variable not set. Assuming NullWS. If you wish to build for X11, Wayland or another supported windowing system, please pass -DWS=X11, -DWS=Wayland etc. to CMake")
		set(WS "NullWS")
		set(WS_DEFINE "${WS}")
	endif()
	
	if(NOT DEFINED CMAKE_PREFIX_PATH)
		set(CMAKE_PREFIX_PATH $ENV{CMAKE_PREFIX_PATH})
	endif()
	
	if (${WS} STREQUAL X11 OR ${WS} STREQUAL XCB OR ${WS} STREQUAL NullWS OR ${WS} STREQUAL Screen)
		set(WS_DEFINE "${WS}")
	elseif(${WS} STREQUAL Wayland)
		set(WS_DEFINE "WAYLAND")
	else()
		message( FATAL_ERROR "Unrecognised WS: Valid values are NullWS(default), X11, Wayland, Screen." )
	endif()
	add_definitions (-D${WS_DEFINE}) # Add a compiler definition so that our header files know what we're building for
	
	if (${WS} STREQUAL X11) #The following lines add the specified windowing system libraries
		find_package(X11 REQUIRED)
		
		if(NOT ${X11_FOUND})
			message(FATAL_ERROR "X11 libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your X11 libraries")
		endif()
		
		list (APPEND PLATFORM_LIBS ${X11_LIBRARIES})
		include_directories(${X11_INCLUDE_DIR})
		
		set(SRC_FILES OpenGLESHelloAPI_LinuxX11.cpp)
	elseif(${WS} STREQUAL Wayland)
		find_package(Wayland REQUIRED)
		find_library(lib-ffi ffi HINTS ${PKG_WAYLAND_LIBRARY_DIRS})
		
		if(NOT ${WAYLAND_FOUND})
			message("Wayland libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your Wayland libraries")
		endif()
		
		list (APPEND PLATFORM_LIBS ${WAYLAND_CLIENT_LIBRARIES} ${WAYLAND_EGL_LIBRARIES} ${lib-ffi})
		include_directories(${WAYLAND_CLIENT_INCLUDE_DIR} ${WAYLAND_EGL_INCLUDE_DIR})
		
		set(SRC_FILES OpenGLESHelloAPI_LinuxWayland.cpp)
	elseif(${WS} STREQUAL NullWS)
		set(SRC_FILES OpenGLESHelloAPI_NullWS.cpp)
	elseif(${WS} STREQUAL Screen)
		if(CMAKE_SYSTEM_NAME MATCHES "QNX")
			list (APPEND PLATFORM_LIBS "screen")
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

if(PLATFORM_LIBS)
	target_link_libraries(OpenGLESHelloAPI ${PLATFORM_LIBS})
endif()

target_include_directories(OpenGLESHelloAPI PUBLIC ${HelloAPI_INCLUDE_DIRECTORIES})

target_compile_definitions(OpenGLESHelloAPI PUBLIC $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:RELEASE=1> ) #Defines DEBUG=1 or RELEASE=1