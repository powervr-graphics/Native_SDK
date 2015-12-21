/*!*********************************************************************************************************************
\file         PVRAssets/FileIO/FileDefinesKTX.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Defines used internally by the KTX reader.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/CoreIncludes.h"
//!\cond NO_DOXYGEN
namespace pvr {
namespace texture_ktx {
// Khronos texture file header
struct FileHeader {
    uint8  identifier[12];
    uint32 endianness;
    uint32 glType;
    uint32 glTypeSize;
    uint32 glFormat;
    uint32 glInternalFormat;
    uint32 glBaseInternalFormat;
    uint32 pixelWidth;
    uint32 pixelHeight;
    uint32 pixelDepth;
    uint32 numberOfArrayElements;
    uint32 numberOfFaces;
    uint32 numberOfMipmapLevels;
    uint32 bytesOfKeyValueData;
};

// Magic identifier
static const uint8  c_identifier[] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };

// Reference number to verify endianness of a file
static const uint32 c_endianReference = 0x04030201;

// Expected size of a header in file
static const uint32 c_expectedHeaderSize = 64;

// Identifier for the orientation meta data
static const char8  c_orientationMetaDataKey[] = "KTXOrientation";

}
}
//!\endcond