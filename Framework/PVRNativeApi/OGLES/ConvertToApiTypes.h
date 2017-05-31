/*!
\brief Contains conversions of pvr Enumerations to OpenGL ES types.
\file PVRNativeApi/OGLES/ConvertToApiTypes.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/PVRCore.h"
#include "PVRCore/Texture.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
<<<<<<< HEAD
#include "PVRAssets/PixelFormat.h"
#include "PVRAssets/Texture/TextureDefines.h"
namespace pvr {
namespace api {
/*!****************************************************************************************************************
\brief  Contain functions to convert several PowerVR Framework types to their Native, OpenGL ES representations,
usually, from an enumeration to a GLenum.
*******************************************************************************************************************/
namespace ConvertToGles {

bool getOpenGLFormat(PixelFormat pixelFormat, pvr::types::ColorSpace colorSpace,
=======
#include "PVRNativeApi/OGLES/GpuCapabilitiesGles.h"
#include "PVRNativeApi/OGLES/ConvertToApiTypes.h"

namespace pvr {
namespace nativeGles {
/// <summary>Contain functions to convert several PowerVR Framework types to their Native, OpenGL ES representations,
/// usually, from an enumeration to a GLenum.</summary>
namespace ConvertToGles {


bool getOpenGLFormat(PixelFormat pixelFormat, types::ColorSpace colorSpace,
>>>>>>> 1776432f... 4.3
                     VariableType dataType, uint32& glInternalFormat,
                     uint32& glFormat, uint32& glType,
                     uint32& glTypeSize, bool& isCompressedFormat);

<<<<<<< HEAD
bool getOpenGLStorageFormat(PixelFormat pixelFormat, pvr::types::ColorSpace colorSpace,
                            VariableType dataType, GLenum& glInternalFormat);

/*!****************************************************************************************************************
\brief  Convert to opengl face.
\return A GLenum representing a face (GL_FRONT, GL_BACK, GL_FRONT_AND_BACK, GL_NONE)
\param  face A Face enum
*******************************************************************************************************************/
GLenum face(types::Face face);

/*!****************************************************************************************************************
\brief  Convert to opengl winding-order.
\return A GLenum representing a winding order (GL_CW, GL_CCW)
\param  windingOrder A PolygonWindingOrder enum
*******************************************************************************************************************/
GLenum polygonWindingOrder(types::PolygonWindingOrder windingOrder);

/*!****************************************************************************************************************
\brief  Convert to opengl comparison mode.
\return A GLenum representing a ComparisonMode (GL_LESS, GL_EQUAL etc)
\param  func A ComparisonMode enum
*******************************************************************************************************************/
GLenum comparisonMode(types::ComparisonMode func);

/*!****************************************************************************************************************
\brief  Convert to opengl fbo attachment type.
\return A GLenum representing an Fbo Attachment (GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT,
        GL_DEPTH_STENCIL_ATTACHMENT, GL_COLOR_ATTACHMENT0}
\param  type A FboAttachmentType enum
*******************************************************************************************************************/
GLenum imageAspect(types::ImageAspect type);

/*!****************************************************************************************************************
\brief  Convert to opengl fbo's texture-attachment target texture type.
\return A GLenum representing an Fbo texture target (GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP_POSITIVE_X etc.)
\param  type A FboTextureTarget enum
*******************************************************************************************************************/
GLenum fboTextureAttachmentTexType(types::FboTextureTarget type);

/*!****************************************************************************************************************
\brief  Convert to opengl texture type.
\param  texType A TextureDimension enum
\return A GLenum representing a texture dimension (GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_2D_ARRAY)
*******************************************************************************************************************/
GLenum textureViewType(types::ImageViewType texType);

/*!****************************************************************************************************************
\brief  Convert to opengl data type.
\return A GLenum representing a DataType (GL_FLOAT, GL_UNSIGNED_BYTE etc)
\param  dataType A DataType enum
*******************************************************************************************************************/
=======
inline bool getOpenGLFormat(
  ImageStorageFormat fmt, uint32& glInternalFormat, uint32& glFormat, uint32& glType,
  uint32& glTypeSize, bool& isCompressedFormat)
{
	return getOpenGLFormat(fmt.format, fmt.colorSpace, fmt.dataType, glInternalFormat, glFormat, glType, glTypeSize, isCompressedFormat);
}

bool getOpenGLStorageFormat(PixelFormat pixelFormat, types::ColorSpace colorSpace,
                            VariableType dataType, GLenum& glInternalFormat);
inline bool getOpenGLStorageFormat(ImageStorageFormat fmt, GLenum& glInternalFormat)
{
	return getOpenGLStorageFormat(fmt.format, fmt.colorSpace, fmt.dataType, glInternalFormat);
}

/// <summary>Convert to opengl face.</summary>
/// <param name="face">A Face enum</param>
/// <returns>A GLenum representing a face (GL_FRONT, GL_BACK, GL_FRONT_AND_BACK, GL_NONE)</returns>
GLenum face(types::Face face);

/// <summary>Convert to opengl winding-order.</summary>
/// <param name="windingOrder">A PolygonWindingOrder enum</param>
/// <returns>A GLenum representing a winding order (GL_CW, GL_CCW)</returns>
GLenum polygonWindingOrder(types::PolygonWindingOrder windingOrder);

/// <summary>Convert to opengl comparison mode.</summary>
/// <param name="func">A ComparisonMode enum</param>
/// <returns>A GLenum representing a ComparisonMode (GL_LESS, GL_EQUAL etc)</returns>
GLenum comparisonMode(types::ComparisonMode func);

/// <summary>Convert to opengl fbo attachment type.</summary>
/// <param name="type">A FboAttachmentType enum</param>
/// <returns>A GLenum representing an Fbo Attachment (GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT,
/// GL_DEPTH_STENCIL_ATTACHMENT, GL_COLOR_ATTACHMENT0}</returns>
GLenum imageAspect(types::ImageAspect type);

/// <summary>Convert to opengl fbo's texture-attachment target texture type.</summary>
/// <param name="type">A FboTextureTarget enum</param>
/// <returns>A GLenum representing an Fbo texture target (GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP_POSITIVE_X etc.)
/// </returns>
GLenum fboTextureAttachmentTexType(types::FboTextureTarget type);

/// <summary>Convert to opengl texture type.</summary>
/// <param name="texType">A TextureDimension enum</param>
/// <returns>A GLenum representing a texture dimension (GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP,
/// GL_TEXTURE_2D_ARRAY)</returns>
GLenum textureViewType(types::ImageViewType texType);

/// <summary>Convert to opengl data type.</summary>
/// <param name="dataType">A DataType enum</param>
/// <returns>A GLenum representing a DataType (GL_FLOAT, GL_UNSIGNED_BYTE etc)</returns>
>>>>>>> 1776432f... 4.3
GLenum dataType(types::DataType dataType);

//****************************************************************************************************************
//\brief  Convert to opengl minification filter.
//\return A GLenum representing a
//\param  A Sa enum
//\param  uint32 mipFilter
//*******************************************************************************************************************/
//GLenum samplerMinFilter(SamplerFilter, SamplerFilter);

<<<<<<< HEAD
/*!****************************************************************************************************************
\brief  Convert to opengl priitive type.
\return A GLenum representing a primitive type (GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_POINTS etc)
\param  primitiveType a PrimitiveTopology enum
*******************************************************************************************************************/
GLenum drawPrimitiveType(types::PrimitiveTopology primitiveType);

/*!****************************************************************************************************************
\brief  Convert to opengl gpu query texture and samplers.
\return A GLenum representing a Textures or Samplers capability query (GL_MAX_TEXTURE_IMAGE_UNITS, GL_MAX_TEXTURE_SIZE etc)
\param  capabilities A gpuCapabilities::TexturesAndSamplers enum
*******************************************************************************************************************/
GLenum gpuCapabilitiesTextureAndSamplers(gpuCapabilities::TextureAndSamplers capabilities);

/*!****************************************************************************************************************
\brief  Convert to opengl gpu query transformfeedback.
\return A GLenum representing a TransformFeedback capability query (GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS etc.)
\param  capabilities A gpuCapabilities::TransformFeedback enum
*******************************************************************************************************************/
GLenum gpuCapabilitiesTransformFeedback(gpuCapabilities::TransformFeedback capabilities);

/*!****************************************************************************************************************
\brief  Convert to opengl gpu query fragment.
\return A GLenum representing a Fragment capability query (GL_MAX_FRAGMENT_UNIFORM_BLOCKS etc.)
\param  capabilities A gpuCapabilities::FragmentShader enum
*******************************************************************************************************************/
GLenum gpuCapabilitiesFragment(gpuCapabilities::FragmentShader capabilities);

/*!****************************************************************************************************************
\brief  Convert to opengl gpu query uniform.
\return A GLenum representing a Uniforms capability query (GL_MAX_UNIFORM_BLOCK_SIZE etc.)
\param  capabilities A gpuCapabilities::Uniform enum
*******************************************************************************************************************/
GLenum gpuCapabilitiesUniform(gpuCapabilities::Uniform capabilities);

/*!****************************************************************************************************************
\brief  Convert to opengl gpu query Buffers.
\return A GLenum representing a Buffers capability query (GL_MAX_DRAW_BUFFERS)
\param  capabilities A gpuCapabilities::Buffers enum
*******************************************************************************************************************/
GLenum gpuCapabilitiesBuffers(gpuCapabilities::Buffers capabilities);

/*!****************************************************************************************************************
\brief  Convert to opengl gpu query element.
\return A GLenum representing an Elements capability query  (GL_MAX_ELEMENT_INDEX, GL_MAX_ELEMENTS_VERTICES)
\param  capabilities A gpuCapabilities::Element enum
*******************************************************************************************************************/
GLenum gpuCapabilitiesElement(gpuCapabilities::Element capabilities);

/*!****************************************************************************************************************
\brief  Convert to opengl gpu query shader and programs.
\return A GLenum representing a Shader/ShaderProgram capability query (GL_MAX_PROGRAM_TEXEL_OFFSET etc.)
\param  capabilities A gpuCapabilities::ShaderAndProgram enum
*******************************************************************************************************************/
GLenum gpuCapabilitiesShaderAndPrograms(gpuCapabilities::ShaderAndProgram capabilities);

/*!****************************************************************************************************************
\brief  Convert to opengl sampler wrap.
\return A GLenum representing a Sampler Wrap mode (GL_CLAMP_TO_EDGE, GL_REPEAT etc)
\param  samplerWrap A SamplerWrap enum
*******************************************************************************************************************/
GLenum samplerWrap(types::SamplerWrap samplerWrap);

/*!***************************************************************************************************************
\brief  Convert to opengl magnification filter.
\return A GLenum representing a Magnification filter (GL_LINEAR, GL_NEAREST)
\param  filter A SamplerFilter
*******************************************************************************************************************/
GLenum samplerMagFilter(types::SamplerFilter filter);

/*!***************************************************************************************************************
\brief  Convert to opengl magnification filter.
\return A GLenum representing a Minification filter (GL_LINEAR, GL_NEAREST_MIPMAP_LINEAR etc)
\param  minFilter A SamplerFilter enum representing the minification filter
\param  mipFilter A SamplerFilter enum representing the mipmapping filter
*******************************************************************************************************************/
GLenum samplerMinFilter(types::SamplerFilter minFilter, types::SamplerFilter mipFilter);

/*!****************************************************************************************************************
\brief  Convert to opengl memory barrier write flag
\return A GLenum representing a Memory Barrier output type (GL_SHADER_STORAGE_BARRIER_BIT, GL_FRAMEBUFFER_BARRIER_BIT etc.)
\param  mask A mask of ORed MemoryBarierFlagOut enums
*******************************************************************************************************************/
GLenum memBarrierFlagOut(uint32 mask);


/*!****************************************************************************************************************
\brief  Convert to opengl stencil op output.
\return A GLenum representing a Stencil Operation (GL_INC_WRAP, GL_ZERO etc)
\param  stencilOp A StencilOp enum
*******************************************************************************************************************/
GLenum stencilOp(types::StencilOp stencilOp);

/*!****************************************************************************************************************
\brief  Convert to opengl blend op output.
\return A GLenum representing a Blend Operation (GL_FUNC_ADD, GL_MIN etc)
\param  blendOp Α BlendOp enum
*******************************************************************************************************************/
GLenum blendEq(types::BlendOp blendOp);

/*!****************************************************************************************************************
\brief  Convert to opengl blend factor output.
\return A GLenum representing a BlendFactor (GL_ZERO, GL_SRC_COLOR, GL_ONE_MINUS_SRC_ALPHA etc)
\param  blendFactor A BlendFactor enum
*******************************************************************************************************************/
GLenum blendFactor(types::BlendFactor blendFactor);

=======
/// <summary>Convert to opengl priitive type.</summary>
/// <param name="primitiveType">a PrimitiveTopology enum</param>
/// <returns>A GLenum representing a primitive type (GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_POINTS etc)</returns>
GLenum drawPrimitiveType(types::PrimitiveTopology primitiveType);

/// <summary>Convert to opengl gpu query texture and samplers.</summary>
/// <param name="capabilities">A gpuCapabilities::TexturesAndSamplers enum</param>
/// <returns>A GLenum representing a Textures or Samplers capability query (GL_MAX_TEXTURE_IMAGE_UNITS,
/// GL_MAX_TEXTURE_SIZE etc)</returns>
GLenum gpuCapabilitiesTextureAndSamplers(gpuCapabilities::TextureAndSamplers capabilities);

/// <summary>Convert to opengl gpu query transformfeedback.</summary>
/// <param name="capabilities">A gpuCapabilities::TransformFeedback enum</param>
/// <returns>A GLenum representing a TransformFeedback capability query (GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS
/// etc.)</returns>
GLenum gpuCapabilitiesTransformFeedback(gpuCapabilities::TransformFeedback capabilities);

/// <summary>Convert to opengl gpu query fragment.</summary>
/// <param name="capabilities">A gpuCapabilities::FragmentShader enum</param>
/// <returns>A GLenum representing a Fragment capability query (GL_MAX_FRAGMENT_UNIFORM_BLOCKS etc.)</returns>
GLenum gpuCapabilitiesFragment(gpuCapabilities::FragmentShader capabilities);

/// <summary>Convert to opengl gpu query uniform.</summary>
/// <param name="capabilities">A gpuCapabilities::Uniform enum</param>
/// <returns>A GLenum representing a Uniforms capability query (GL_MAX_UNIFORM_BLOCK_SIZE etc.)</returns>
GLenum gpuCapabilitiesUniform(gpuCapabilities::Uniform capabilities);

/// <summary>Convert to opengl gpu query Buffers.</summary>
/// <param name="capabilities">A gpuCapabilities::Buffers enum</param>
/// <returns>A GLenum representing a Buffers capability query (GL_MAX_DRAW_BUFFERS)</returns>
GLenum gpuCapabilitiesBuffers(gpuCapabilities::Buffers capabilities);

/// <summary>Convert to opengl gpu query element.</summary>
/// <param name="capabilities">A gpuCapabilities::Element enum</param>
/// <returns>A GLenum representing an Elements capability query (GL_MAX_ELEMENT_INDEX, GL_MAX_ELEMENTS_VERTICES)
/// </returns>
GLenum gpuCapabilitiesElement(gpuCapabilities::Element capabilities);

/// <summary>Convert to opengl gpu query shader and programs.</summary>
/// <param name="capabilities">A gpuCapabilities::ShaderAndProgram enum</param>
/// <returns>A GLenum representing a Shader/ShaderProgram capability query (GL_MAX_PROGRAM_TEXEL_OFFSET etc.)
/// </returns>
GLenum gpuCapabilitiesShaderAndPrograms(gpuCapabilities::ShaderAndProgram capabilities);

/// <summary>Convert to opengl sampler wrap.</summary>
/// <param name="samplerWrap">A SamplerWrap enum</param>
/// <returns>A GLenum representing a Sampler Wrap mode (GL_CLAMP_TO_EDGE, GL_REPEAT etc)</returns>
GLenum samplerWrap(types::SamplerWrap samplerWrap);

/// <summary>Convert to opengl magnification filter.</summary>
/// <param name="filter">A SamplerFilter</param>
/// <returns>A GLenum representing a Magnification filter (GL_LINEAR, GL_NEAREST)</returns>
GLenum samplerMagFilter(types::SamplerFilter filter);

/// <summary>Convert to opengl magnification filter.</summary>
/// <param name="minFilter">A SamplerFilter enum representing the minification filter</param>
/// <param name="mipFilter">A SamplerFilter enum representing the mipmapping filter</param>
/// <returns>A GLenum representing a Minification filter (GL_LINEAR, GL_NEAREST_MIPMAP_LINEAR etc)</returns>
GLenum samplerMinFilter(types::SamplerFilter minFilter, types::SamplerFilter mipFilter);

/// <summary>Convert to opengl memory barrier write flag</summary>
/// <param name="mask">A mask of ORed MemoryBarierFlagOut enums</param>
/// <returns>A GLenum representing a Memory Barrier output type (GL_SHADER_STORAGE_BARRIER_BIT,
/// GL_FRAMEBUFFER_BARRIER_BIT etc.)</returns>
GLenum memBarrierFlagOut(uint32 mask);


/// <summary>Convert to opengl stencil op output.</summary>
/// <param name="stencilOp">A StencilOp enum</param>
/// <returns>A GLenum representing a Stencil Operation (GL_INC_WRAP, GL_ZERO etc)</returns>
GLenum stencilOp(types::StencilOp stencilOp);

/// <summary>Convert to opengl blend op output.</summary>
/// <param name="blendOp">Α BlendOp enum</param>
/// <returns>A GLenum representing a Blend Operation (GL_FUNC_ADD, GL_MIN etc)</returns>
GLenum blendEq(types::BlendOp blendOp);

/// <summary>Convert to opengl blend factor output.</summary>
/// <param name="blendFactor">A BlendFactor enum</param>
/// <returns>A GLenum representing a BlendFactor (GL_ZERO, GL_SRC_COLOR, GL_ONE_MINUS_SRC_ALPHA etc)</returns>
GLenum blendFactor(types::BlendFactor blendFactor);

>>>>>>> 1776432f... 4.3
inline GLsizei samplesCount(types::SampleCount samples) { return (GLsizei)samples; }


}
}// namespace api
}//namespace pvr