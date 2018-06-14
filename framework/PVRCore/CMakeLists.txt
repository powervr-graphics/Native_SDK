cmake_minimum_required(VERSION 3.3)
if(pvrcore_included)
	return()
endif(pvrcore_included)
set(pvrcore_included true)

project(PVRCore)

include (../../cmake/Common.cmake)

file(GLOB PVRCORE_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Base/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Base/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/DataStructures/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/DataStructures/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Interfaces/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Interfaces/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/IO/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/IO/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Math/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Math/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Strings/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Strings/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Texture/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Texture/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../../external/pugixml/pugixml.cpp
)

if(WIN32)
	file(GLOB PVRCORE_PLATFORM_SRC
	./Windows/*.h
	./Windows/*.cpp
	)
	LIST(APPEND PVRCORE_SRC ${PVRCORE_PLATFORM_SRC})
elseif(ANDROID)
	file(GLOB PVRCORE_PLATFORM_SRC
		./Android/*.h
		./Android/*.cpp
	)
	LIST(APPEND PVRCORE_SRC ${PVRCORE_PLATFORM_SRC})
elseif(APPLE)
	message("SET UP APPLE FOR PVRCORE")
elseif(UNIX)
	#... Nothing special for Unix
endif()

add_library(PVRCore STATIC ${PVRCORE_SRC})

target_compile_definitions( PVRCore PUBLIC  $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1 RELEASE=1> )
