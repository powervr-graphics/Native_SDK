/*!*********************************************************************************************************************
\file         PVRNativeApi\OGLES\TextureUtils.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains OpenGL ES specific Helper utilities. Use only if directly using the underlying API's.
***********************************************************************************************************************/
#pragma once
#include "PVRNativeApi/TextureUtils.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
namespace pvr {
namespace utils {
/*!*********************************************************************************************************************
\brief	Gets the openGL/ES equivalent texture format values, as per the
		Khronos KTX specification:
		http://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec/
\param[in]			pixelFormat The pixel format (RGBA8888 etc)
\param[in]			colorSpace  The colorspace   (srgb/lrgb)
\param[in]			dataType	The datatype     (Float32, unsignedbyte...)
\param[out]			glInternalFormat Set to The internal format (a GLenum of the specified format GL_RGBA8888_sRGB)
\param[out]			glFormat         Set to The format (a GLenum of the specified format GL_RGBA)
\param[out]			glType			 Set to The type	(a GLenum of the specified format GL_FLOAT, GL_UNSIGNED_SHORT_1_5_5_5_REV)
\param[out]			glTypeSize       Set to the number of components in the format
\param[out]			isCompressedFormat  Set to true if the format is compressed, false otherwise
\return			true on success, false if a suitable type cannot be matched
***********************************************************************************************************************/
bool getOpenGLFormat(PixelFormat pixelFormat, types::ColorSpace::Enum colorSpace, VariableType::Enum dataType,
                     uint32& glInternalFormat, uint32& glFormat,
                     uint32& glType, uint32& glTypeSize, bool& isCompressedFormat);

/*!*********************************************************************************************************************
\brief Gets OpenGL/ES equivalent texture storage format values, as per the khronos KTX specification
		http://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec/
\param[in]			pixelFormat The pixel format (RGBA8888 etc)
\param[in]			colorSpace  The colorspace   (srgb/lrgb)
\param[in]			dataType	The datatype     (Float32, unsignedbyte...)
\param[out]			glInternalFormat             (a GLenum of the specified format GL_RGBA8888_sRGB)
\return			true on success, returns false if it cannot find a suitable type
***********************************************************************************************************************/
bool getOpenGLStorageFormat(PixelFormat pixelFormat, types::ColorSpace::Enum colorSpace, VariableType::Enum dataType,  GLenum& glInternalFormat);
}
}