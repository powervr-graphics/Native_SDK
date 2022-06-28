cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)
project(PVRCore)
include(GNUInstallDirs)
include(../FrameworkCommon.cmake)
# Copyright (c) Imagination Technologies Limited.

# Prevent PVRCore being added multiple times
if(TARGET PVRCore)
	return()
endif()

message(STATUS "Adding PVRCore")

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

# Add GLM and pugixml to the cmake build system
add_submodule_to_sdk(${SDK_ROOT_INTERNAL_DIR}/external/glm)
add_submodule_to_sdk(${SDK_ROOT_INTERNAL_DIR}/external/pugixml)

# Append the GLM wrappers to the PVRCore headers
list(APPEND PVRCore_HEADERS
	glm.h
	cameras/CameraKeyFrame.h
	cameras/FPSCamera.h
	cameras/TPSCamera.h
	math/AxisAlignedBox.h
	math/MathUtils.h
	math/Plane.h
	math/Rectangle.h
	math/Rectangle3D.h
	pfx/Effect.h)


# Append Pugixml source files and headers to PVRCore	
list(APPEND PVRCore_HEADERS pfx/PFXParser.h)
list(APPEND PVRCore_SRC	pfx/PFXParser.cpp)


# Add platform specific PVRCore sources which handle asset finding
if(WIN32)
	list(APPEND PVRCore_HEADERS
	Windows/WindowsResourceStream.h)
elseif(ANDROID)
	list(APPEND PVRCore_HEADERS
		Android/AndroidAssetStream.h)
		
	list(APPEND PVRCore_SRC
		Android/AndroidAssetStream.cpp)
endif()


# Create the PVRCore library
add_library(PVRCore STATIC ${PVRCore_SRC} ${PVRCore_HEADERS})

# Link to the libraries that contain dlopen and dlclose, 
# this allows PVRCore to dynamically open shared libraries, such as libvulkan.so
target_link_libraries(PVRCore PRIVATE ${CMAKE_DL_LIBS})

# Link to GLM and Pugixml
target_link_libraries(PVRCore PUBLIC glm)
target_link_libraries(PVRCore PRIVATE pugixml)

# Link to the platform specific libraries 
if (WIN32)
	# No extra libraries for windows. We need this so that windows 
	# isn't treated as an unknown platform
elseif (ANDROID)
	# On Android we need extra logging libraries to log out to the logcat
	find_library(lib-log log)
	target_link_libraries(PVRCore PRIVATE ${lib-log})
elseif (UNIX)
	# On Unix we need to specifically link to threads library
	find_package(Threads)
	target_link_libraries(PVRCore PRIVATE ${CMAKE_THREAD_LIBS_INIT})
	target_link_libraries(PVRCore PRIVATE "rt")
else()
	message(FATAL_ERROR "Unknown Platform - Please set this up with platform-specific dependencies")
endif()

# Set the include directories of PVRCore to allow it to see the rest of the framework
# And also GLM and pugixml
target_include_directories(PVRCore PUBLIC ${CMAKE_CURRENT_LIST_DIR}/..
	${SDK_ROOT_INTERNAL_DIR}/external/glm/source
	${SDK_ROOT_INTERNAL_DIR}/external/pugixml/source)

# Apply the common framework compile definitions to PVRCore
# This sets certain warnings on/off and adds appropriate
# debug and release flags in the compiler and linker  
apply_framework_compile_options_to_target(PVRCore)