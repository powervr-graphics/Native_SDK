cmake_minimum_required(VERSION 3.3)
if(pvrutilsgles_included)
	return()
endif(pvrutilsgles_included)
set(pvrutilsgles_included true)

project(PVRUtilsGles)

include (../../../cmake/Common.cmake)

if (IOS)
	file(GLOB PVRUTILSGLES_SRC
	  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
	  ${CMAKE_CURRENT_SOURCE_DIR}/../*.cpp
	  ${CMAKE_CURRENT_SOURCE_DIR}/../*.h
	  ${CMAKE_CURRENT_SOURCE_DIR}/../EAGL/*.cpp
	  ${CMAKE_CURRENT_SOURCE_DIR}/../EAGL/*.h
	  ${CMAKE_CURRENT_SOURCE_DIR}/../EAGL/*.mm
	)
else()
	file(GLOB PVRUTILSGLES_SRC
	  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
	  ${CMAKE_CURRENT_SOURCE_DIR}/../*.cpp
	  ${CMAKE_CURRENT_SOURCE_DIR}/../*.h
	  ${CMAKE_CURRENT_SOURCE_DIR}/../EGL/*.cpp
	  ${CMAKE_CURRENT_SOURCE_DIR}/../EGL/*.h
	)
endif()
ADD_LIBRARY( PVRUtilsGles STATIC ${PVRUTILSGLES_SRC})
target_compile_definitions( PVRUtilsGles PUBLIC  $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1 RELEASE=1> )
