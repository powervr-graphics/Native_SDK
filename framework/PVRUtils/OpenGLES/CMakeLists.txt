cmake_minimum_required(VERSION 3.3)

project(PVRUtilsGles)

include (../../../cmake/Common.cmake)

if (IOS)
	file(GLOB PVRUTILSGLES_SRC
	  ${CMAKE_CURRENT_LIST_DIR}/*.cpp
	  ${CMAKE_CURRENT_LIST_DIR}/*.h
	  ${CMAKE_CURRENT_LIST_DIR}/../*.cpp
	  ${CMAKE_CURRENT_LIST_DIR}/../*.h
	  ${CMAKE_CURRENT_LIST_DIR}/../EAGL/*.cpp
	  ${CMAKE_CURRENT_LIST_DIR}/../EAGL/*.h
	  ${CMAKE_CURRENT_LIST_DIR}/../EAGL/*.mm
	)
else()
	file(GLOB PVRUTILSGLES_SRC
	  ${CMAKE_CURRENT_LIST_DIR}/*.cpp
	  ${CMAKE_CURRENT_LIST_DIR}/*.h
	  ${CMAKE_CURRENT_LIST_DIR}/../*.cpp
	  ${CMAKE_CURRENT_LIST_DIR}/../*.h
	  ${CMAKE_CURRENT_LIST_DIR}/../EGL/*.cpp
	  ${CMAKE_CURRENT_LIST_DIR}/../EGL/*.h
	)
endif()
ADD_LIBRARY(PVRUtilsGles STATIC ${PVRUTILSGLES_SRC})
target_compile_definitions(PVRUtilsGles PUBLIC  $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1 RELEASE=1>)
target_include_directories(PVRUtilsGles PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../..)
