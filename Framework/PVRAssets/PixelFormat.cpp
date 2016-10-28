/*!*********************************************************************************************************************
\file         PVRAssets\PixelFormat.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Definitions of static, global, pre-defined common pixel formats (RGBA8888 etc.).
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRAssets/PixelFormat.h"
namespace pvr {
const PixelFormat PixelFormat::Intensity8('i', '\0', '\0', '\0', 8, 0, 0, 0);
const PixelFormat PixelFormat::RGBA_8888('r', 'g', 'b', 'a', 8, 8, 8, 8);

const PixelFormat PixelFormat::R_32('r', '\0', '\0', '\0', 32, 0, 0, 0);

const PixelFormat PixelFormat::RGBA_32323232('r', 'g', 'b', 'a', 32, 32, 32, 32);
const PixelFormat PixelFormat::RGB_323232('r', 'g', 'b', '\0', 32, 32, 32, 0);

const PixelFormat PixelFormat::RG_88('r', 'g', '\0', '\0', 8, 8, 0, 0);

const PixelFormat PixelFormat::RGB_565('r', 'g', 'b', '\0', 5, 6, 5, 0);
const PixelFormat PixelFormat::RGB_888('r', 'g', 'b', '\0', 8, 8, 8, 0);
const PixelFormat PixelFormat::RGBA_4444('r', 'g', 'b', 'a', 4, 4, 4, 4);
const PixelFormat PixelFormat::RGBA_5551('r', 'g', 'b', 'a', 5, 5, 5, 1);

const PixelFormat PixelFormat::BGR_888('b', 'g', 'r', '\0', 8, 8, 8, 0);
const PixelFormat PixelFormat::BGRA_8888('b', 'g', 'r', 'a', 8, 8, 8, 8);

const PixelFormat PixelFormat::Depth8('d', '\0', '\0', '\0', 8, 0, 0, 0);
const PixelFormat PixelFormat::Depth16('d', '\0', '\0', '\0', 16, 0, 0, 0);
const PixelFormat PixelFormat::Depth24('d', '\0', '\0', '\0', 24, 0, 0, 0);
const PixelFormat PixelFormat::Depth32('d', '\0', '\0', '\0', 32, 0, 0, 0);
const PixelFormat PixelFormat::Depth16Stencil8('d', 's', '\0', '\0', 16, 8, 0, 0);
const PixelFormat PixelFormat::Depth24Stencil8('d', 's', '\0', '\0', 24, 8, 0, 0);
const PixelFormat PixelFormat::Depth32Stencil8('d', 's', '\0', '\0', 32, 8, 0, 0);
const PixelFormat PixelFormat::Stencil8('s', '\0', '\0', '\0', 8, 0, 0, 0);
const PixelFormat PixelFormat::Unknown(0, 0, 0, 0, 0, 0, 0, 0);


}
//!\endcond