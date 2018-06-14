cmake_minimum_required(VERSION 3.3)
if(pvrshell_included)
	return()
endif(pvrshell_included)
set(pvrshell_included true)

project(PVRShell)

include (../../cmake/Common.cmake)

file(GLOB SHELL_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/OS/*.h
)

#Each platform has its own implementation of PVRShell
if(WIN32)			   
	file(GLOB SHELLOS_SRC
	  ${CMAKE_CURRENT_SOURCE_DIR}/OS/Windows/*.cpp
	  ${CMAKE_CURRENT_SOURCE_DIR}/OS/Windows/*.h
	  ${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint/WinMain/*.cpp
	  ${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint/WinMain/*.h
	)
elseif (APPLE)
	if (IOS)
		file(GLOB SHELLOS_SRC
			${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint/UIApplicationMain/*.m
			${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint/UIApplicationMain/*.mm
			${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint/UIApplicationMain/*.h
			${CMAKE_CURRENT_SOURCE_DIR}/OS/UIKit/*.mm
			)
	else()
		file(GLOB SHELLOS_SRC
			${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint/NSApplicationMain/*.m
			${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint/NSApplicationMain/*.mm
			${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint/NSApplicationMain/*.h
			${CMAKE_CURRENT_SOURCE_DIR}/OS/AppKit/*.mm
			)
	endif()
elseif(ANDROID)
	file(GLOB SHELLOS_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/OS/Android/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/OS/Android/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint/android_main/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint/android_main/*.h
	${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
	)
	include_directories(${ANDROID_NDK}/sources/android/native_app_glue)
	#The following line is to prevent Android from stripping out the JNI glue function where Java calls C++
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
elseif(UNIX)
	#Each windowing system has its own implementation of PVRShell
	if ("${WS}" STREQUAL "")
		message("PVRShell:[WS] variable not set: [${WS}].")
		set (WS "NullWS")
		message("PVRShell:[WS] variable now set to: [${WS}]. Otherwise, pass -DWS=[NullWS,X11,Wayland,Screen] as valid values for Unix")
	endif()

	if ("${WS}" STREQUAL "NullWS")
		message("CMake: Generating PVRShell for NullWS")
		if(CMAKE_SYSTEM_NAME MATCHES "QNX")
			file(GLOB SHELLOS_SRC
				${CMAKE_CURRENT_SOURCE_DIR}/OS/QNXNullWS/*.cpp
				${CMAKE_CURRENT_SOURCE_DIR}/OS/QNXNullWS/*.h
				${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint/main/*.cpp
				${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint/main/*.h
				)
		else()
			file(GLOB SHELLOS_SRC
				${CMAKE_CURRENT_SOURCE_DIR}/OS/NullWS/*.cpp
				${CMAKE_CURRENT_SOURCE_DIR}/OS/NullWS/*.h
				${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint/main/*.cpp
				${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint/main/*.h
				)
		endif()
	elseif("${WS}" STREQUAL "X11")
		message("CMake: Generating PVRShell for X11")
		file(GLOB SHELLOS_SRC
			${CMAKE_CURRENT_SOURCE_DIR}/OS/X11/*.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/OS/X11/*.h
			${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint/main/*.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint/main/*.h
			)
	elseif("${WS}" STREQUAL "Wayland")
		message("CMake: Generating PVRShell for Wayland")
		file(GLOB SHELLOS_SRC
			${CMAKE_CURRENT_SOURCE_DIR}/OS/Wayland/*.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/OS/Wayland/*.h
			${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint/main/*.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint/main/*.h
			)
	elseif("${WS}" STREQUAL "Screen")
		if(CMAKE_SYSTEM_NAME MATCHES "QNX")
			message("CMake: Generating PVRShell for Screen")
			file(GLOB SHELLOS_SRC
				${CMAKE_CURRENT_SOURCE_DIR}/OS/QNXScreen/*.cpp
				${CMAKE_CURRENT_SOURCE_DIR}/OS/QNXScreen/*.h
				${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint/main/*.cpp
				${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint/main/*.h
				)
		else()
			message(FATAL_ERROR "[WS] variable set to [${WS}] which is only supported on QNX platforms.")
		endif()
	else()
		message(FATAL_ERROR "[WS] variable set to invalid value [${WS}]. Please set WS to a value among: [NullWS, X11, Wayland]")
	endif()
endif()

ADD_LIBRARY(PVRShell STATIC
	${SHELL_SRC}
	${SHELLOS_SRC}
)
target_compile_definitions( PVRShell PUBLIC $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:RELEASE=1> )
