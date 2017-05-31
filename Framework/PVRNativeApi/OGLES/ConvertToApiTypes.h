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
#include "PVRNativeApi/OGLES/GpuCapabilitiesGles.h"
#include "PVRNativeApi/OGLES/ConvertToApiTypes.h"

namespace pvr {
namespace nativeGles {
/// <summary>Contain functions to convert several PowerVR Framework types to their Native, OpenGL ES representations,
/// usually, from an enumeration to a GLenum.</summary>
namespace ConvertToGles {


bool getOpenGLFormat(PixelFormat pixelFormat, types::ColorSpace colorSpace,
                     VariableType dataType, uint32& glInternalFormat,
                     uint32& glFormat, uint32& glType,
                     uint32& glTypeSize, bool& isCompressedFormat);

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
GLenum dataType(types::DataType dataType);

//****************************************************************************************************************
//\brief  Convert to opengl minification filter.
//\return A GLenum representing a
//\param  A Sa enum
//\param  uint32 mipFilter
//*******************************************************************************************************************/
//GLenum samplerMinFilter(SamplerFilter, SamplerFilter);

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

inline GLsizei samplesCount(types::SampleCount samples) { return (GLsizei)samples; }


}
}// namespace api
}//namespace pvr