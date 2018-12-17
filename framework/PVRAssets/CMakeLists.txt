cmake_minimum_required(VERSION 3.3)

project(PVRAssets)

include (../../cmake/Common.cmake)

file(GLOB PVRAssets_SRC
	*.h
	*.cpp
	${CMAKE_CURRENT_LIST_DIR}/fileio/*.cpp
	${CMAKE_CURRENT_LIST_DIR}/fileio/*.h
	${CMAKE_CURRENT_LIST_DIR}/model/*.cpp
	${CMAKE_CURRENT_LIST_DIR}/model/*.h
)

add_library(PVRAssets STATIC ${PVRAssets_SRC})
target_include_directories(PVRAssets PUBLIC ${CMAKE_CURRENT_LIST_DIR}/..)
target_compile_definitions(PVRAssets PUBLIC  $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1 RELEASE=1>)
