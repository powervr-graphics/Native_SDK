cmake_minimum_required(VERSION 3.4)

project(PVRVk)

include (../../cmake/Common.cmake)

file(GLOB PVRVK_SRC
	   ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	   ${CMAKE_CURRENT_SOURCE_DIR}/*.h
	   ${SDK_ROOT}/include/vk_bindings.h
	   ${SDK_ROOT}/include/pvrvk_vulkan_wrapper.h
	   ${SDK_ROOT}/include/vk_bindings_helper.h
	   )

ADD_LIBRARY( PVRVk STATIC ${PVRVK_SRC} )
target_compile_definitions(PVRVk PUBLIC  $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1 RELEASE=1> )
target_include_directories(PVRVk PUBLIC ${CMAKE_CURRENT_LIST_DIR}/..)
