cmake_minimum_required(VERSION 3.3)
project (PVRCore)

set(PVRCore_SRC
	${CMAKE_CURRENT_LIST_DIR}/cameras/CameraKeyFrame.h
    ${CMAKE_CURRENT_LIST_DIR}/cameras/FPSCamera.h
    ${CMAKE_CURRENT_LIST_DIR}/cameras/TPSCamera.h
    ${CMAKE_CURRENT_LIST_DIR}/commandline/CommandLine.h
    ${CMAKE_CURRENT_LIST_DIR}/Errors.h
    ${CMAKE_CURRENT_LIST_DIR}/glm.h
    ${CMAKE_CURRENT_LIST_DIR}/IAssetProvider.h
    ${CMAKE_CURRENT_LIST_DIR}/Log.h
    ${CMAKE_CURRENT_LIST_DIR}/math/AxisAlignedBox.h
    ${CMAKE_CURRENT_LIST_DIR}/math/MathUtils.h
    ${CMAKE_CURRENT_LIST_DIR}/math/Plane.h
    ${CMAKE_CURRENT_LIST_DIR}/math/Rectangle.h
    ${CMAKE_CURRENT_LIST_DIR}/math/Rectangle3D.h
    ${CMAKE_CURRENT_LIST_DIR}/pfx/Effect.h
    ${CMAKE_CURRENT_LIST_DIR}/pfx/PFXParser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/pfx/PFXParser.h
    ${CMAKE_CURRENT_LIST_DIR}/PVRCore.h
    ${CMAKE_CURRENT_LIST_DIR}/RefCounted.h
    ${CMAKE_CURRENT_LIST_DIR}/stream/Asset.h
    ${CMAKE_CURRENT_LIST_DIR}/stream/AssetReader.h
    ${CMAKE_CURRENT_LIST_DIR}/stream/AssetWriter.h
    ${CMAKE_CURRENT_LIST_DIR}/stream/BufferStream.h
    ${CMAKE_CURRENT_LIST_DIR}/stream/FilePath.h
    ${CMAKE_CURRENT_LIST_DIR}/stream/FileStream.h
    ${CMAKE_CURRENT_LIST_DIR}/stream/Stream.h
    ${CMAKE_CURRENT_LIST_DIR}/strings/CompileTimeHash.h
    ${CMAKE_CURRENT_LIST_DIR}/strings/StringFunctions.h
    ${CMAKE_CURRENT_LIST_DIR}/strings/StringHash.h
    ${CMAKE_CURRENT_LIST_DIR}/strings/UnicodeConverter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/strings/UnicodeConverter.h
    ${CMAKE_CURRENT_LIST_DIR}/texture/MetaData.h
    ${CMAKE_CURRENT_LIST_DIR}/texture/PixelFormat.h
    ${CMAKE_CURRENT_LIST_DIR}/texture/PVRTDecompress.cpp
    ${CMAKE_CURRENT_LIST_DIR}/texture/PVRTDecompress.h
    ${CMAKE_CURRENT_LIST_DIR}/texture/Texture.cpp
    ${CMAKE_CURRENT_LIST_DIR}/texture/Texture.h
    ${CMAKE_CURRENT_LIST_DIR}/texture/TextureDefines.h
    ${CMAKE_CURRENT_LIST_DIR}/texture/TextureHeader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/texture/TextureHeader.h
    ${CMAKE_CURRENT_LIST_DIR}/texture/TextureLoad.h
    ${CMAKE_CURRENT_LIST_DIR}/texture/TextureLoadAsync.h
    ${CMAKE_CURRENT_LIST_DIR}/textureio/FileDefinesBMP.h
    ${CMAKE_CURRENT_LIST_DIR}/textureio/FileDefinesDDS.h
    ${CMAKE_CURRENT_LIST_DIR}/textureio/FileDefinesKTX.h
    ${CMAKE_CURRENT_LIST_DIR}/textureio/FileDefinesPVR.h
    ${CMAKE_CURRENT_LIST_DIR}/textureio/FileDefinesTGA.h
    ${CMAKE_CURRENT_LIST_DIR}/textureio/FileDefinesXNB.h
    ${CMAKE_CURRENT_LIST_DIR}/textureio/PaletteExpander.cpp
    ${CMAKE_CURRENT_LIST_DIR}/textureio/PaletteExpander.h
    ${CMAKE_CURRENT_LIST_DIR}/textureio/TextureReaderBMP.cpp
    ${CMAKE_CURRENT_LIST_DIR}/textureio/TextureReaderBMP.h
    ${CMAKE_CURRENT_LIST_DIR}/textureio/TextureReaderDDS.cpp
    ${CMAKE_CURRENT_LIST_DIR}/textureio/TextureReaderDDS.h
    ${CMAKE_CURRENT_LIST_DIR}/textureio/TextureReaderKTX.cpp
    ${CMAKE_CURRENT_LIST_DIR}/textureio/TextureReaderKTX.h
    ${CMAKE_CURRENT_LIST_DIR}/textureio/TextureReaderPVR.cpp
    ${CMAKE_CURRENT_LIST_DIR}/textureio/TextureReaderPVR.h
    ${CMAKE_CURRENT_LIST_DIR}/textureio/TextureReaderTGA.cpp
    ${CMAKE_CURRENT_LIST_DIR}/textureio/TextureReaderTGA.h
    ${CMAKE_CURRENT_LIST_DIR}/textureio/TextureReaderXNB.cpp
    ${CMAKE_CURRENT_LIST_DIR}/textureio/TextureReaderXNB.h
    ${CMAKE_CURRENT_LIST_DIR}/textureio/TextureWriterPVR.cpp
    ${CMAKE_CURRENT_LIST_DIR}/textureio/TextureWriterPVR.h
    ${CMAKE_CURRENT_LIST_DIR}/textureio/TGAWriter.h
    ${CMAKE_CURRENT_LIST_DIR}/Threading.h
    ${CMAKE_CURRENT_LIST_DIR}/types/FreeValue.h
    ${CMAKE_CURRENT_LIST_DIR}/types/GpuDataTypes.h
    ${CMAKE_CURRENT_LIST_DIR}/types/Types.h
    ${CMAKE_CURRENT_LIST_DIR}/Utils.h)
	
if(WIN32)
	list(APPEND PVRCore_SRC
	${CMAKE_CURRENT_LIST_DIR}/Windows/WindowsResourceStream.h)
elseif(ANDROID)
	list(APPEND PVRCore_SRC
		${CMAKE_CURRENT_LIST_DIR}/Android/AndroidAssetStream.h
		${CMAKE_CURRENT_LIST_DIR}/Android/AndroidAssetStream.cpp)
endif()

set(PVRCore_LINK_LIBS PowerVR_SDK pugixml PlatformInterface)

get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT AND USE_PREBUILT_DEPENDENCIES)
add_library(PVRCore STATIC IMPORTED GLOBAL)

# add the subdirectory after the add_library so that the target exists
add_subdirectory(../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk EXCLUDE_FROM_ALL)

set_target_properties(PVRCore PROPERTIES
	IMPORTED_LOCATION_DEBUG ${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRCore${CMAKE_STATIC_LIBRARY_SUFFIX}
	IMPORTED_LOCATION_RELEASE ${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRCore${CMAKE_STATIC_LIBRARY_SUFFIX}
	INTERFACE_LINK_LIBRARIES "${PVRCore_LINK_LIBS}")
else()
add_library(PVRCore STATIC EXCLUDE_FROM_ALL ${PVRCore_SRC})

# add the subdirectory after the add_library so that the target exists
add_subdirectory(../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk EXCLUDE_FROM_ALL)

target_link_libraries(PVRCore ${PVRCore_LINK_LIBS})
set_target_properties(PVRCore PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${FRAMEWORK_LIB_FOLDER}
	LIBRARY_OUTPUT_DIRECTORY ${FRAMEWORK_LIB_FOLDER})
endif()
