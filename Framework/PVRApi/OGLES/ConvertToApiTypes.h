/*!*********************************************************************************************************************
\file         PVRApi\OGLES\ConvertToApiTypes.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains conversions of pvr Enumerations to OpenGL ES types.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/Defines.h"
#include "PVRCore/Types.h"
#include "PVRApi/ApiObjectTypes.h"
#include "PVRApi/GpuCapabilities.h"
#include "PVRAssets/SamplerDescription.h"
#include "PVRApi/OGLES/NativeObjectsGles.h"
namespace pvr {
namespace api {
/*!****************************************************************************************************************
\brief	Contain functions to convert several PowerVR Framework types to their Native, OpenGL ES representations,
usually, from an enumeration to a GLenum.
*******************************************************************************************************************/
namespace ConvertToGles {
/*!****************************************************************************************************************
\brief	Convert to opengl face.
\return	A GLenum representing a face (GL_FRONT, GL_BACK, GL_FRONT_AND_BACK, GL_NONE)
\param	face A Face enum
*******************************************************************************************************************/
GLenum face(Face::Enum face);

/*!****************************************************************************************************************
\brief	Convert to opengl winding-order.
\return	A GLenum representing a winding order (GL_CW, GL_CCW)
\param	windingOrder A PolygonWindingOrder enum
*******************************************************************************************************************/
GLenum polygonWindingOrder(PolygonWindingOrder::Enum windingOrder);

/*!****************************************************************************************************************
\brief	Convert to opengl comparison mode.
\return	A GLenum representing a ComparisonMode (GL_LESS, GL_EQUAL etc)
\param	func A ComparisonMode enum
*******************************************************************************************************************/
GLenum comparisonMode(ComparisonMode::Enum func);

/*!****************************************************************************************************************
\brief	Convert to opengl fbo attachment type.
\return	A GLenum representing an Fbo Attachment (GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT, 
        GL_DEPTH_STENCIL_ATTACHMENT, GL_COLOR_ATTACHMENT0}
\param	type A FboAttachmentType enum
*******************************************************************************************************************/
GLenum fboAttachmentType(FboAttachmentType::Enum type);

/*!****************************************************************************************************************
\brief	Convert to opengl fbo's texture-attachment target texture type.
\return	A GLenum representing an Fbo texture target (GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP_POSITIVE_X etc.)
\param	type A FboTextureTarget enum
*******************************************************************************************************************/
GLenum fboTextureAttachmentTexType(FboTextureTarget::Enum type);

/*!****************************************************************************************************************
\brief	Convert to opengl texture type.
\return	A GLenum representing a texture dimension (GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_2D_ARRAY)
\param	texType A TextureDimension enum
*******************************************************************************************************************/
GLenum textureDimension(TextureDimension::Enum texType);

/*!****************************************************************************************************************
\brief	Convert to opengl data type.
\return	A GLenum representing a DataType (GL_FLOAT, GL_UNSIGNED_BYTE etc)
\param	dataType A DataType enum
*******************************************************************************************************************/
GLenum dataType(DataType::Enum dataType);

//****************************************************************************************************************
//\brief	Convert to opengl minification filter.
//\return	A GLenum representing a
//\param	A Sa enum
//\param	uint32 mipFilter
//*******************************************************************************************************************/
//GLenum samplerMinFilter(SamplerFilter::Enum, SamplerFilter::Enum);

/*!****************************************************************************************************************
\brief	Convert to opengl priitive type.
\return	A GLenum representing a primitive type (GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_POINTS etc)
\param	primitiveType a PrimitiveTopology enum
*******************************************************************************************************************/
GLenum drawPrimitiveType(PrimitiveTopology::Enum primitiveType);

/*!****************************************************************************************************************
\brief	Convert to opengl gpu query texture and samplers.
\return	A GLenum representing a Textures or Samplers capability query (GL_MAX_TEXTURE_IMAGE_UNITS, GL_MAX_TEXTURE_SIZE etc)
\param	capabilities A gpuCapabilities::TexturesAndSamplers enum
*******************************************************************************************************************/
GLenum gpuCapabilitiesTextureAndSamplers(gpuCapabilities::TextureAndSamplers::Enum capabilities);

/*!****************************************************************************************************************
\brief	Convert to opengl gpu query transformfeedback.
\return	A GLenum representing a TransformFeedback capability query (GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS etc.)
\param	capabilities A gpuCapabilities::TransformFeedback enum
*******************************************************************************************************************/
GLenum gpuCapabilitiesTransformFeedback(gpuCapabilities::TransformFeedback::Enum capabilities);

/*!****************************************************************************************************************
\brief	Convert to opengl gpu query fragment.
\return	A GLenum representing a Fragment capability query (GL_MAX_FRAGMENT_UNIFORM_BLOCKS etc.)
\param	capabilities A gpuCapabilities::FragmentShader enum
*******************************************************************************************************************/
GLenum gpuCapabilitiesFragment(gpuCapabilities::FragmentShader::Enum capabilities);

/*!****************************************************************************************************************
\brief	Convert to opengl gpu query uniform.
\return	A GLenum representing a Uniforms capability query (GL_MAX_UNIFORM_BLOCK_SIZE etc.)
\param	capabilities A gpuCapabilities::Uniform enum
*******************************************************************************************************************/
GLenum gpuCapabilitiesUniform(gpuCapabilities::Uniform::Enum capabilities);

/*!****************************************************************************************************************
\brief	Convert to opengl gpu query Buffers.
\return	A GLenum representing a Buffers capability query (GL_MAX_DRAW_BUFFERS)
\param	capabilities A gpuCapabilities::Buffers enum
*******************************************************************************************************************/
GLenum gpuCapabilitiesBuffers(gpuCapabilities::Buffers::Enum capabilities);

/*!****************************************************************************************************************
\brief	Convert to opengl gpu query element.
\return	A GLenum representing an Elements capability query  (GL_MAX_ELEMENT_INDEX, GL_MAX_ELEMENTS_VERTICES)
\param	capabilities A gpuCapabilities::Element enum
*******************************************************************************************************************/
GLenum gpuCapabilitiesElement(gpuCapabilities::Element::Enum capabilities);

/*!****************************************************************************************************************
\brief	Convert to opengl gpu query shader and programs.
\return	A GLenum representing a Shader/ShaderProgram capability query (GL_MAX_PROGRAM_TEXEL_OFFSET etc.)
\param	capabilities A gpuCapabilities::ShaderAndProgram enum
*******************************************************************************************************************/
GLenum gpuCapabilitiesShaderAndPrograms(gpuCapabilities::ShaderAndProgram::Enum capabilities);

/*!****************************************************************************************************************
\brief	Convert to opengl sampler wrap.
\return	A GLenum representing a Sampler Wrap mode (GL_CLAMP_TO_EDGE, GL_REPEAT etc)
\param	samplerWrap A SamplerWrap enum
*******************************************************************************************************************/
GLenum samplerWrap(SamplerWrap::Enum samplerWrap);

/*!***************************************************************************************************************
\brief	Convert to opengl magnification filter.
\return	A GLenum representing a Magnification filter (GL_LINEAR, GL_NEAREST)
\param	filter A SamplerFilter
*******************************************************************************************************************/
GLenum samplerMagFilter(SamplerFilter::Enum filter);

/*!***************************************************************************************************************
\brief	Convert to opengl magnification filter.
\return	A GLenum representing a Minification filter (GL_LINEAR, GL_NEAREST_MIPMAP_LINEAR etc)
\param	minFilter A SamplerFilter enum representing the minification filter
\param	mipFilter A SamplerFilter enum representing the mipmapping filter
*******************************************************************************************************************/
GLenum samplerMinFilter(SamplerFilter::Enum minFilter, SamplerFilter::Enum mipFilter);

/*!****************************************************************************************************************
\brief	Convert to opengl memory barrier write flag
\return	A GLenum representing a Memory Barrier output type (GL_SHADER_STORAGE_BARRIER_BIT, GL_FRAMEBUFFER_BARRIER_BIT etc.)
\param	mask A mask of ORed MemoryBarierFlagOut enums
*******************************************************************************************************************/
GLenum memBarrierFlagOut(uint32 mask);

/*!****************************************************************************************************************
\brief	Convert to opengl memory barrier input.
\return	A GLenum representing a Memory Barrier Input type (GL_SHADER_STORAGE_BARRIER_BIT, GL_VERTEX_ATTRIB_ARRAY_BIT etc.)
\param	mask A mask of ORed MemoryBarierFlagIn enums
*******************************************************************************************************************/
GLenum memBarrierFlagIn(uint32 mask);

/*!****************************************************************************************************************
\brief	Convert to opengl stencil op output.
\return	A GLenum representing a Stencil Operation (GL_INC_WRAP, GL_ZERO etc)
\param	stencilOp A StencilOp enum
*******************************************************************************************************************/
GLenum stencilOp(StencilOp::Enum stencilOp);

/*!****************************************************************************************************************
\brief	Convert to opengl blend op output.
\return	A GLenum representing a Blend Operation (GL_FUNC_ADD, GL_MIN etc)
\param	blendOp Α BlendOp enum 
*******************************************************************************************************************/
GLenum blendEq(BlendOp::Enum blendOp);

/*!****************************************************************************************************************
\brief	Convert to opengl blend factor output.
\return	A GLenum representing a BlendFactor (GL_ZERO, GL_SRC_COLOR, GL_ONE_MINUS_SRC_ALPHA etc)
\param	blendFactor A BlendFactor enum
*******************************************************************************************************************/
GLenum blendFactor(BlendFactor::Enum blendFactor);


}
}// namespace api
}//namespace pvr
