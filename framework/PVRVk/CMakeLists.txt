cmake_minimum_required(VERSION 3.4)
if(pvrvk_included)
	return()
endif(pvrvk_included)
set(pvrvk_included true)

project(PVRVk)

include (../../cmake/Common.cmake)

file(GLOB PVRVK_SRC
	   ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	   ${CMAKE_CURRENT_SOURCE_DIR}/*.h
	   )

ADD_LIBRARY( PVRVk STATIC ${PVRVK_SRC} )
target_compile_definitions( PVRVk PUBLIC  $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1 RELEASE=1> )
