/*!*********************************************************************************************************************
\file         PVRAssets/Texture/PVRTDecompress.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains functions to decompress PVRTC or ETC formats into RGBA8888.
***********************************************************************************************************************/
#pragma once
namespace pvr {

/*!*******************************************************************************************************************************
\brief	Decompresses PVRTC to RGBA 8888.
\return	Return the amount of data that was decompressed.
\param	compressedData The PVRTC texture data to decompress
\param	do2bitMode Signifies whether the data is PVRTC2 or PVRTC4
\param	xDim X dimension of the texture
\param	yDim Y dimension of the texture
\param	outResultImage The decompressed texture data
**********************************************************************************************************************************/
int PVRTDecompressPVRTC(const void* compressedData, int do2bitMode, int xDim, int yDim, unsigned char* outResultImage);

/*!*******************************************************************************************************************************
\brief	Decompresses ETC to RGBA 8888.
\return	Return The number of bytes of ETC data decompressed
\param	srcData The ETC texture data to decompress
\param	xDim X dimension of the texture
\param	yDim Y dimension of the texture
\param	destData The decompressed texture data
\param	mode The format of the data
**********************************************************************************************************************************/
int PVRTDecompressETC(const void* srcData, unsigned int xDim, unsigned int yDim, void* destData, int mode);
}