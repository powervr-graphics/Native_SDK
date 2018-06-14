cmake_minimum_required(VERSION 3.3)
if(pvrcamera_included)
	return()
endif(pvrcamera_included)
set(pvrcamera_included true)

project(PVRCamera)

include (../../cmake/Common.cmake)

if(ANDROID)
	set (SRCFILE ${CMAKE_CURRENT_SOURCE_DIR}/CameraInterface_Android.cpp)
elseif (IOS)
		set (SRCFILE ${CMAKE_CURRENT_SOURCE_DIR}/CameraInterface_iOS.mm)
else()
		set (SRCFILE ${CMAKE_CURRENT_SOURCE_DIR}/CameraInterface_Dummy.cpp)
endif()

ADD_LIBRARY( PVRCamera STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/PVRCamera.h
	${CMAKE_CURRENT_SOURCE_DIR}/CameraInterface.h
	${SRCFILE})

target_compile_definitions(PVRCamera PUBLIC  $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1 RELEASE=1>)
