/*!*********************************************************************************************************************
\file         PVRApi\GpuCapabilities.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Contains enumerations that represent several kinds of queries for the GPU capabilities
***********************************************************************************************************************/
#pragma once
#include "PVRCore/Types.h"
namespace pvr {
class IGraphicsContext;
namespace api {

/*!****************************************************************************************************************
\brief	Contains enumerations for GPU capability queries
*******************************************************************************************************************/
namespace gpuCapabilities {

/*!****************************************************************************************************************
\brief	Texture and Sampler query.
*******************************************************************************************************************/
enum class TextureAndSamplers
{
	MaxTextureImageUnit,//< max texture binding supported
	MaxSamples,//< max samples supported
	Max3DTextureSize,//< max 3d texture supported
	MaxArrayTextureLayer,//< max array-texture layer supported
	MaxTextureLodBias,//< max texture LOD bias supported
	MaxTextureSize,//< max texture size supported
	MaxCubeMapTexSize,//< max cube map texture size supported
	Count
};

/*!****************************************************************************************************************
\brief	Shader and Program query.
*******************************************************************************************************************/
enum class ShaderAndProgram
{
	MaxProgramTexelOffset,//< max texel offset supported by the shader program
	MinProgramTexelOffset,//< min texel offset supported by the shader program
        NumProgramBinaryFormats, //< number of program binary formats supported
	NumCompressedTextureFormats,//< number of compressed texture formats supported
	NumShaderBinaryFormats, //< number of shader binary format supported
	Count
};


/*!****************************************************************************************************************
\brief	TransformFeedback query.
*******************************************************************************************************************/
enum class TransformFeedback
{
	MaxInterleavedComponent,//< max transform-feedback interleaved component supported
	MaxSeparateAttribs,//< max transform feedback separate attribute supported
	SeparateComponents,//< max transform feedback
	Count
};


/*!****************************************************************************************************************
\brief	Uniform query.
*******************************************************************************************************************/
enum class Uniform
{
	MaxUniformBlockSize,//< maximum size in basic machine units of a uniform block.
	MaxUniformBufferBindings,//< maximum number of uniform buffer binding points
	MaxCombinedUniformBlocks,//<maximum number of uniform blocks per program
	MaxCombinedVertexUniformComponent,//< maximum number of vertex uniform supported
	Count
};


/*!****************************************************************************************************************
\brief	Element query.
*******************************************************************************************************************/
enum class Element
{
	MaxIndices,//< max indices supported
	MaxVertices, //< max vertices supported
	Count
};


/*!****************************************************************************************************************
\brief	Buffers query.
*******************************************************************************************************************/
enum class Buffers  { MaxDrawBuffers, Count };

/*!****************************************************************************************************************
\brief	FragmentShader query.
*******************************************************************************************************************/
enum class FragmentShader
{
	MaxFragmentInputComponents,//< max fragment input components  supported
	MaxfragmentUniformBlocks,//< max fragment uniform blocks  supported
	MaxFragmentUniformComponent,//< max fragment uniform component supported
	MaxFragmentUniformVectors,
	Count
};

/*!****************************************************************************************************************
\brief  Query gpu's texture and sampler capabilities.
\return	The capability requested, as an Int32
\param	context The context whose capability is queried
\param	query A TextureAndSamplers enum representing the capability to be queried
*******************************************************************************************************************/
int32 get(IGraphicsContext& context, TextureAndSamplers query);


/*!****************************************************************************************************************
\brief	Query gpu's fragment capabilities.
\return	The capability requested, as an Int32
\param	context The context whose capability is queried
\param	query A FragmentShader enum representing the capability to be queried
*******************************************************************************************************************/
int32 get(IGraphicsContext& context, FragmentShader query);

/*!****************************************************************************************************************
\brief	Query gpu's shader and program capabilities.
\return	The capability requested, as an Int32
\param	context The context whose capability is queried
\param	query A ShaderAndProgram enum representing the capability to be queried
*******************************************************************************************************************/
int32 get(IGraphicsContext& context, ShaderAndProgram query);

/*!****************************************************************************************************************
\brief	Query gpu's Buffers capabilities.
\return	The capability requested, as an Int32
\param	context The context whose capability is queried
\param	query A Buffers enum representing the capability to be queried
*******************************************************************************************************************/
int32 get(IGraphicsContext& context, Buffers query);

/*!****************************************************************************************************************
\brief	Query gpu's Element capabilities.
\return	The capability requested, as an Int32
\param	context The context whose capability is queried
\param	query A Element enum representing the capability to be queried
*******************************************************************************************************************/
int32 get(IGraphicsContext& context, Element query);

/*!****************************************************************************************************************
\brief	Query gpu's transformfeedback capabilities.
\return	The capability requested, as an Int32
\param	context The context whose capability is queried
\param	query A TransformFeedback enum representing the capability to be queried
*******************************************************************************************************************/
int32 get(IGraphicsContext& context, TransformFeedback query);

/*!****************************************************************************************************************
\brief  Query gpu's Uniform capabilities.
\return	The capability requested, as an Int32
\param	context The context whose capability is queried
\param	query A Uniform enum representing the capability to be queried
*******************************************************************************************************************/
int32 get(IGraphicsContext& context, Uniform query);

}
}
}
