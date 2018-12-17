cmake_minimum_required(VERSION 3.3)

project(PVRCore)

include (../../cmake/Common.cmake)

file(GLOB PVRCORE_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cameras/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cameras/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/math/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/math/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/pfx/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/pfx/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/strings/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/strings/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/stream/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/stream/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/texture/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/texture/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/textureio/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/textureio/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/types/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/types/*.h
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
target_include_directories(PVRCore PUBLIC ${CMAKE_CURRENT_LIST_DIR}/..)
target_compile_definitions(PVRCore PUBLIC  $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1 RELEASE=1> )
