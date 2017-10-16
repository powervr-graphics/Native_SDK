/*!
\brief Defines used by the PVR reader.
\file PVRCore/Texture/FileDefinesPVR.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/CoreIncludes.h"
//!\cond NO_DOXYGEN
namespace pvr {
namespace texture_legacy {
///////////////////////// Legacy PVR file headers /////////////////////////

// Version 1 of the PVR file format
struct HeaderV1
{
	uint32  headerSize;	//!< size of the structure 
	uint32  height;		 //!< height of surface to be created 
	uint32  width;		//!< width of input surface 
	uint32  mipMapCount;	//!< number of mip-map levels requested 
	uint32  pixelFormatAndFlags; //!< pixel format flags 
	uint32  dataSize;		//!< Size of the compress data 
	uint32  bitCount;		//!< number of bits per pixel  
	uint32  redBitMask;		//!< mask for red bit 
	uint32  greenBitMask;	//!< mask for green bits 
	uint32  blueBitMask;	//!< mask for blue bits 
	uint32  alphaBitMask;	//!< mask for alpha channel 
};

// Version 2 of the PVR file format
struct HeaderV2 : public HeaderV1
{
	uint32 pvrMagic;				/*!< magic number identifying pvr file */
	uint32 numberOfSurfaces;			/*!< the number of surfaces present in the pvr */
};

////////////////////////// Legacy enumerations //////////////////////////
// Pixel types
enum PixelFormat
{
	// MGL Formats
	MGL_ARGB_4444 = 0x00,
	MGL_ARGB_1555,
	MGL_RGB_565,
	MGL_RGB_555,
	MGL_RGB_888,
	MGL_ARGB_8888,
	MGL_ARGB_8332,
	MGL_I_8,
	MGL_AI_88,
	MGL_1_BPP,
	MGL_VY1UY0,
	MGL_Y1VY0U,
	MGL_PVRTC2,
	MGL_PVRTC4,

	// openGL Formats
	GL_RGBA_4444 = 0x10,
	GL_RGBA_5551,
	GL_RGBA_8888,
	GL_RGB_565,
	GL_RGB_555,
	GL_RGB_888,
	GL_I_8,
	GL_AI_88,
	GL_PVRTC2,
	GL_PVRTC4,
	GL_BGRA_8888,
	GL_A_8,
	GL_PVRTCII4,
	GL_PVRTCII2,

	// DirectX 9 and Earlier Formats
	D3D_DXT1 = 0x20,
	D3D_DXT2,
	D3D_DXT3,
	D3D_DXT4,
	D3D_DXT5,
	D3D_RGB_332,
	D3D_AL_44,
	D3D_LVU_655,
	D3D_XLVU_8888,
	D3D_QWVU_8888,
	D3D_ABGR_2101010,
	D3D_ARGB_2101010,
	D3D_AWVU_2101010,
	D3D_GR_1616,
	D3D_VU_1616,
	D3D_ABGR_16161616,
	D3D_R16F,
	D3D_GR_1616F,
	D3D_ABGR_16161616F,
	D3D_R32F,
	D3D_GR_3232F,
	D3D_ABGR_32323232F,

	// Ericsson Texture Compression formats
	e_etc_RGB_4BPP,

	//More DirectX 9 Formats
	D3D_A8 = 0x40,
	D3D_V8U8,
	D3D_L16,
	D3D_L8,
	D3D_AL_88,
	D3D_UYVY,
	D3D_YUY2,

	// DirectX 10+ Formats
	DXGI_R32G32B32A32_FLOAT = 0x50,
	DXGI_R32G32B32A32_UINT,
	DXGI_R32G32B32A32_SINT,
	DXGI_R32G32B32_FLOAT,
	DXGI_R32G32B32_UINT,
	DXGI_R32G32B32_SINT,
	DXGI_R16G16B16A16_FLOAT,
	DXGI_R16G16B16A16_UNORM,
	DXGI_R16G16B16A16_UINT,
	DXGI_R16G16B16A16_SNORM,
	DXGI_R16G16B16A16_SINT,
	DXGI_R32G32_FLOAT,
	DXGI_R32G32_UINT,
	DXGI_R32G32_SINT,
	DXGI_R10G10B10A2_UNORM,
	DXGI_R10G10B10A2_UINT,
	DXGI_R11G11B10_FLOAT,
	DXGI_R8G8B8A8_UNORM,
	DXGI_R8G8B8A8_UNORM_SRGB,
	DXGI_R8G8B8A8_UINT,
	DXGI_R8G8B8A8_SNORM,
	DXGI_R8G8B8A8_SINT,
	DXGI_R16G16_FLOAT,
	DXGI_R16G16_UNORM,
	DXGI_R16G16_UINT,
	DXGI_R16G16_SNORM,
	DXGI_R16G16_SINT,
	DXGI_R32_FLOAT,
	DXGI_R32_UINT,
	DXGI_R32_SINT,
	DXGI_R8G8_UNORM,
	DXGI_R8G8_UINT,
	DXGI_R8G8_SNORM,
	DXGI_R8G8_SINT,
	DXGI_R16_FLOAT,
	DXGI_R16_UNORM,
	DXGI_R16_UINT,
	DXGI_R16_SNORM,
	DXGI_R16_SINT,
	DXGI_R8_UNORM,
	DXGI_R8_UINT,
	DXGI_R8_SNORM,
	DXGI_R8_SINT,
	DXGI_A8_UNORM,
	DXGI_R1_UNORM,
	DXGI_R9G9B9E5_SHAREDEXP,
	DXGI_R8G8_B8G8_UNORM,
	DXGI_G8R8_G8B8_UNORM,
	DXGI_BC1_UNORM,
	DXGI_BC1_UNORM_SRGB,
	DXGI_BC2_UNORM,
	DXGI_BC2_UNORM_SRGB,
	DXGI_BC3_UNORM,
	DXGI_BC3_UNORM_SRGB,
	DXGI_BC4_UNORM,				// unimplemented
	DXGI_BC4_SNORM,				// unimplemented
	DXGI_BC5_UNORM,				// unimplemented
	DXGI_BC5_SNORM,				// unimplemented

	// openVG
	VG_sRGBX_8888  = 0x90,
	VG_sRGBA_8888,
	VG_sRGBA_8888_PRE,
	VG_sRGB_565,
	VG_sRGBA_5551,
	VG_sRGBA_4444,
	VG_sL_8,
	VG_lRGBX_8888,
	VG_lRGBA_8888,
	VG_lRGBA_8888_PRE,
	VG_lL_8,
	VG_A_8,
	VG_BW_1,
	VG_sXRGB_8888,
	VG_sARGB_8888,
	VG_sARGB_8888_PRE,
	VG_sARGB_1555,
	VG_sARGB_4444,
	VG_lXRGB_8888,
	VG_lARGB_8888,
	VG_lARGB_8888_PRE,
	VG_sBGRX_8888,
	VG_sBGRA_8888,
	VG_sBGRA_8888_PRE,
	VG_sBGR_565,
	VG_sBGRA_5551,
	VG_sBGRA_4444,
	VG_lBGRX_8888,
	VG_lBGRA_8888,
	VG_lBGRA_8888_PRE,
	VG_sXBGR_8888,
	VG_sABGR_8888,
	VG_sABGR_8888_PRE,
	VG_sABGR_1555,
	VG_sABGR_4444,
	VG_lXBGR_8888,
	VG_lABGR_8888,
	VG_lABGR_8888_PRE,

	// Number of pixel types, no point iterating beyond this.
	NumPixelTypes,

	//Error type.
	InvalidType = 0xffffffff
};

// Target APIs, used to write to specific API targets.
enum API
{
	ApiOGLES = 1,
	ApiOGLES2,
	ApiD3DM,
	ApiOGL,
	ApiDX9,
	ApiDX10,
	ApiOVG,
	ApiMGL
};

///////////////////////////// Legacy constants /////////////////////////////

// Flags for Legacy TextureHeader versions
static const uint32 c_flagMIPMap          = (1 << 8);  // Texture has MIP Map levels
//	static const uint32 c_flagTwiddled        = (1<<9);    // Texture data is twiddled
static const uint32 c_flagBumpMap         = (1 << 10); // Texture has normals encoded for a bump map
//	static const uint32 c_flagTiledPVRTC      = (1<<11);   // Texture is bordered for tiled PVRTC
static const uint32 c_flagCubeMap         = (1 << 12); // Texture is a cubemap/skybox
//	static const uint32 c_flagFalseColorMIPs = (1<<13);   // Texture has false colored MIP Map levels
static const uint32 c_flagVolumeTexture   = (1 << 14); // Texture is a 3D texture
static const uint32 c_flagHasAlpha        = (1 << 15); // Texture has transparency
static const uint32 c_flagVerticalFlip    = (1 << 16); // Texture is vertically flipped

// Mask for the pixel type - always in the last 16bits of the flags in legacy headers.
static const uint32 c_pixelTypeMask       = 0xff;

// The old PVR header identifier is the characters 'PVR!', V2 only. Usually ignored...
static const uint32 c_identifierV2        = 0x21525650;

// Header sizes, used as identifiers for previous versions of the header.
static const uint32 c_headerSizeV1        = 44;
static const uint32 c_headerSizeV2        = 52;

}
}
//!\endcond