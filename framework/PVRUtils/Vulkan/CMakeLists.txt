cmake_minimum_required(VERSION 3.3)

project(PVRUtilsVk)

include (../../../cmake/Common.cmake) #Setting a few flags up...

file(GLOB PVRUTILSVK_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../*.h
)

ADD_LIBRARY( PVRUtilsVk STATIC ${PVRUTILSVK_SRC})
target_compile_definitions(PVRUtilsVk PUBLIC  $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1 RELEASE=1>)
target_include_directories(PVRUtilsVk PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
