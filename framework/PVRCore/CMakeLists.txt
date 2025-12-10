cmake_minimum_required(VERSION 3.10)
# Copyright (c) Imagination Technologies Limited.

project(PVRCore)

include(GNUInstallDirs)
include(CMakeFindDependencyMacro)
include(../FrameworkCommon.cmake)

# PVRCore include files
set(PVRCore_HEADERS
	Errors.h
	IAssetProvider.h
	Log.h
	PVRCore.h
	RefCounted.h
	Time.cpp
	Time_.h
	Threading.h
	Utils.h
	commandline/CommandLine.h
	stream/BufferStream.h
	stream/FilePath.h
	stream/FileStream.h
	stream/Stream.h
	strings/CompileTimeHash.h
	strings/StringFunctions.h
	strings/StringHash.h
	strings/UnicodeConverter.h
	texture/MetaData.h
	texture/PixelFormat.h
	texture/PVRTDecompress.h
	texture/Texture.h
	texture/TextureDefines.h
	texture/TextureHeader.h
	texture/TextureLoad.h
	texture/TextureLoadAsync.h
	textureio/FileDefinesBMP.h
	textureio/FileDefinesDDS.h
	textureio/FileDefinesKTX.h
	textureio/FileDefinesPVR.h
	textureio/FileDefinesTGA.h
	textureio/FileDefinesXNB.h
	textureio/PaletteExpander.h
	textureio/TextureReaderBMP.h
	textureio/TextureReaderDDS.h
	textureio/TextureReaderKTX.h
	textureio/TextureReaderPVR.h
	textureio/TextureReaderTGA.h
	textureio/TextureReaderXNB.h
	textureio/TextureWriterPVR.h
	textureio/TGAWriter.h
	types/FreeValue.h
	types/GpuDataTypes.h
	types/Types.h)
	
# PVRCore source files
set(PVRCore_SRC
	strings/UnicodeConverter.cpp
	texture/PVRTDecompress.cpp
	texture/Texture.cpp
	texture/TextureHeader.cpp
	textureio/PaletteExpander.cpp
	textureio/TextureReaderBMP.cpp
	textureio/TextureReaderDDS.cpp
	textureio/TextureReaderKTX.cpp
	textureio/TextureReaderPVR.cpp
	textureio/TextureReaderTGA.cpp
	textureio/TextureReaderXNB.cpp
	textureio/TextureWriterPVR.cpp
	Time.cpp)

list(APPEND PVRCore_HEADERS
	pfx/PFXParser.h)
list(APPEND PVRCore_SRC
	pfx/PFXParser.cpp)

# Add platform specific PVRCore sources
if(WIN32)
	list(APPEND PVRCore_HEADERS
	Windows/WindowsResourceStream.h)
elseif(ANDROID)
	list(APPEND PVRCore_HEADERS
		Android/AndroidAssetStream.h)
		
	list(APPEND PVRCore_SRC
		Android/AndroidAssetStream.cpp)
endif()

# Set the include directories for PVRCore - relying on FrameworkCommon.cmake to set PVR_FRAMEWORK_ROOT
# We need to find "PVRCore/..." headers, so we add the parent directory of PVRCore
include_directories("${PVR_FRAMEWORK_ROOT}")

# Create the library
add_library(PVRCore STATIC ${PVRCore_SRC} ${PVRCore_HEADERS})

target_link_libraries(PVRCore PRIVATE ${CMAKE_DL_LIBS})

if (WIN32)
# No extra libraries for windows. We need this if though as if it is an unknown platform we error out
elseif (ANDROID)
	find_library(lib-log log)
	target_link_libraries(PVRCore PRIVATE ${lib-log})
elseif (APPLE)
	if (IOS)
		find_library(lib-foundation Foundation)
		find_library(lib-avfoundation AVFoundation)
		target_link_libraries(PVRCore PRIVATE ${lib-foundation} ${lib-avfoundation})
	endif()
elseif (UNIX)
	if(NOT CMAKE_SYSTEM_NAME MATCHES "QNX")
		find_package(Threads)
		target_link_libraries(PVRCore PRIVATE ${CMAKE_THREAD_LIBS_INIT})
		target_link_libraries(PVRCore PRIVATE "rt")
	endif()
else()
	message(FATAL_ERROR "Unknown Platform - Please set this up with platform-specific dependencies")
endif()

if(PVR_PREBUILT_DEPENDENCIES)
	list(APPEND CMAKE_MODULE_PATH "${PVR_SDK_ROOT}/cmake/modules")
	find_dependency(glm MODULE REQUIRED)
	find_dependency(pugixml MODULE REQUIRED)
endif()

apply_framework_compile_options_to_target(PVRCore)

target_link_libraries(PVRCore PUBLIC glm)
target_link_libraries(PVRCore PRIVATE pugixml)

# Ensure dependencies are found using the global root variables
target_include_directories(PVRCore PRIVATE 
    "${PVR_EXTERNAL_ROOT}/glm/source"
    "${PVR_EXTERNAL_ROOT}/pugixml/source/src"
)

# Set the include directories for PVRCore
# Using generator expressions for INTERFACE properties to avoid absolute path issues during export
target_include_directories(PVRCore 
	PUBLIC 
		$<BUILD_INTERFACE:${PVR_FRAMEWORK_ROOT}>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Use the consolidated install function
pvr_install_target(TARGET PVRCore HEADERS ${PVRCore_HEADERS})
