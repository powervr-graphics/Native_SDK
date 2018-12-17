cmake_minimum_required(VERSION 3.4)

project(PVRPfx)

include (../../cmake/Common.cmake)

file(GLOB PVRRENDERMANAGER_SRC
	   ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	   ${CMAKE_CURRENT_SOURCE_DIR}/*.h
	)

ADD_LIBRARY( PVRPfx STATIC ${PVRRENDERMANAGER_SRC} )

target_link_libraries(PVRPfx PVRUtilsVk)
target_include_directories(PVRPfx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_definitions(PVRPfx PUBLIC  $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1 RELEASE=1> )
