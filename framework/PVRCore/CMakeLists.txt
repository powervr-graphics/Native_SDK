set(source_files
    cameras/CameraKeyFrame.h
    cameras/FPSCamera.h
    cameras/TPSCamera.h
    commandline/CommandLine.h
    Errors.h
    glm.h
    IAssetProvider.h
    Log.h
    math/AxisAlignedBox.h
    math/MathUtils.h
    math/Plane.h
    math/Rectangle.h
    math/Rectangle3D.h
    pfx/Effect.h
    pfx/PFXParser.cpp
    pfx/PFXParser.h
    PVRCore.h
    RefCounted.h
    stream/Asset.h
    stream/AssetReader.h
    stream/AssetWriter.h
    stream/BufferStream.h
    stream/FilePath.h
    stream/FileStream.h
    stream/Stream.h
    strings/CompileTimeHash.h
    strings/StringFunctions.h
    strings/StringHash.h
    strings/UnicodeConverter.cpp
    strings/UnicodeConverter.h
    texture/MetaData.h
    texture/PixelFormat.h
    texture/PVRTDecompress.cpp
    texture/PVRTDecompress.h
    texture/Texture.cpp
    texture/Texture.h
    texture/TextureDefines.h
    texture/TextureHeader.cpp
    texture/TextureHeader.h
    texture/TextureLoad.h
    texture/TextureLoadAsync.h
    texture/TextureUtils.h
    textureio/FileDefinesBMP.h
    textureio/FileDefinesDDS.h
    textureio/FileDefinesKTX.h
    textureio/FileDefinesPVR.h
    textureio/FileDefinesTGA.h
    textureio/FileDefinesXNB.h
    textureio/PaletteExpander.cpp
    textureio/PaletteExpander.h
    textureio/TextureReaderBMP.cpp
    textureio/TextureReaderBMP.h
    textureio/TextureReaderDDS.cpp
    textureio/TextureReaderDDS.h
    textureio/TextureReaderKTX.cpp
    textureio/TextureReaderKTX.h
    textureio/TextureReaderPVR.cpp
    textureio/TextureReaderPVR.h
    textureio/TextureReaderTGA.cpp
    textureio/TextureReaderTGA.h
    textureio/TextureReaderXNB.cpp
    textureio/TextureReaderXNB.h
    textureio/TextureWriterPVR.cpp
    textureio/TextureWriterPVR.h
    textureio/TGAWriter.h
    Threading.h
    types/FreeValue.h
    types/GpuDataTypes.h
    types/Types.h
    Utils.h
)

if(WIN32)
    list(APPEND source_files
        Windows/WindowsResourceStream.h
    )
elseif(ANDROID)
    list(APPEND source_files
        Android/AndroidAssetStream.cpp
        Android/AndroidAssetStream.h
    )
elseif(APPLE)
    message("SET UP APPLE FOR PVRCORE")
endif()

add_library(PVRCore STATIC ${source_files})
target_include_directories(PVRCore PUBLIC ..)
target_compile_definitions(PVRCore PUBLIC
    $<$<CONFIG:Debug>:DEBUG=1>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1 RELEASE=1>
)
target_link_libraries(PVRCore PUBLIC pugixml PowerVR_SDK)
