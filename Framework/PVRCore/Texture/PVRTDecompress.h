/*!
\brief Contains functions to decompress PVRTC or ETC formats into RGBA8888.
\file PVRCore/Texture/PVRTDecompress.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
namespace pvr {

/// <summary>Decompresses PVRTC to RGBA 8888.</summary>
/// <param name="compressedData">The PVRTC texture data to decompress</param>
/// <param name="do2bitMode">Signifies whether the data is PVRTC2 or PVRTC4</param>
/// <param name="xDim">X dimension of the texture</param>
/// <param name="yDim">Y dimension of the texture</param>
/// <param name="outResultImage">The decompressed texture data</param>
/// <returns>Return the amount of data that was decompressed.</returns>
int PVRTDecompressPVRTC(const void* compressedData, int do2bitMode, int xDim, int yDim, unsigned char* outResultImage);

/// <summary>Decompresses ETC to RGBA 8888.</summary>
/// <param name="srcData">The ETC texture data to decompress</param>
/// <param name="xDim">X dimension of the texture</param>
/// <param name="yDim">Y dimension of the texture</param>
/// <param name="destData">The decompressed texture data</param>
/// <param name="mode">The format of the data</param>
/// <returns>Return The number of bytes of ETC data decompressed</returns>
int PVRTDecompressETC(const void* srcData, unsigned int xDim, unsigned int yDim, void* destData, int mode);
}