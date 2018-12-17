cmake_minimum_required(VERSION 3.3)

project(PVRCamera)

include (../../cmake/Common.cmake)

if(ANDROID)
	set (SRCFILE ${CMAKE_CURRENT_LIST_DIR}/CameraInterface_Android.cpp)
elseif (IOS)
		set (SRCFILE ${CMAKE_CURRENT_LIST_DIR}/CameraInterface_iOS.mm)
else()
		set (SRCFILE ${CMAKE_CURRENT_LIST_DIR}/CameraInterface_Dummy.cpp)
endif()

add_library( PVRCamera STATIC
	${CMAKE_CURRENT_LIST_DIR}/PVRCamera.h
	${CMAKE_CURRENT_LIST_DIR}/CameraInterface.h
	${SRCFILE})

target_include_directories(PVRCamera PUBLIC ${CMAKE_CURRENT_LIST_DIR}/..)
target_compile_definitions(PVRCamera PUBLIC  $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1 RELEASE=1>)
