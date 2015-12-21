/*!*********************************************************************************************************************
\file         PVRApi\OGLES\TextureUtils.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains OpenGL ES specific Helper utilities. Use only if directly using the underlying API's.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/IGraphicsContext.h"
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRAssets/Model/Mesh.h"
#include "PVRAssets/Texture/Texture.h"
#include "PVRApi/OGLES/TextureFormats.h"
#include "PVRApi/OGLES/NativeObjectsGles.h"


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
bool getOpenGLFormat(PixelFormat pixelFormat, ColorSpace::Enum colorSpace, VariableType::Enum dataType,
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
bool getOpenGLStorageFormat(PixelFormat pixelFormat, ColorSpace::Enum colorSpace, VariableType::Enum dataType,  GLenum& glInternalFormat);


/*!*********************************************************************************************************************
\brief Upload a texture to the GPU and retrieve the into native handle.
\param device
\param[in] texture texture to upload
\param[in] allowDecompress allow de-compress before upload the texture
\param[out] outTextureName native texture handle to upload into
\return			Result::Success on success, errorcode otherwise
***********************************************************************************************************************/
Result::Enum textureUpload(GraphicsContext& device, const assets::Texture& texture, native::HTexture_& outTextureName, bool allowDecompress = true);

/*!*********************************************************************************************************************
\brief Upload texture into the GPU, retrieve a pvr::API TextureView object.
\param context The GraphicsContext to use
\param[in] texture The texture to upload
\param[in] allowDecompress Allow de-compress a compressed format if the format is not natively supported. If this is set
           to true and an unsupported compressed format before uploading the texture, the implementation will uncompress
		   the texture on the CPU and upload the uncompressed texture. If set to false, the implementation will return
		   failure in this case.
\param[out] outTexture The api texture to upload into
\return			Result::Success on success, errorcode otherwise
***********************************************************************************************************************/
Result::Enum textureUpload(GraphicsContext& context, const assets::Texture& texture, api::TextureView& outTexture, bool allowDecompress = true);
}// namespace utils
}// namespace pvr