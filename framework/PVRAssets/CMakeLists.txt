cmake_minimum_required(VERSION 3.3)
if(pvrassets_included)
	return()
endif(pvrassets_included)
set(pvrassets_included true)

project(PVRAssets)

include (../../cmake/Common.cmake)

file(GLOB PVRAssets_SRC
	*.h
	*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/FileIO/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/FileIO/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/Model/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Model/*.h
)

add_library(PVRAssets STATIC ${PVRAssets_SRC})
target_compile_definitions( PVRAssets PUBLIC  $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1 RELEASE=1> )
