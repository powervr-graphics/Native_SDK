/*!
\brief Contains enumerations that represent several kinds of queries for the GPU capabilities
\file PVRNativeApi/OGLES/GpuCapabilitiesGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Base/Types.h"
namespace pvr {
class IGraphicsContext;

/// <summary>Contains enumerations for GPU capability queries</summary>
namespace gpuCapabilities {

/// <summary>Texture and Sampler query.</summary>
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

/// <summary>Shader and Program query.</summary>
enum class ShaderAndProgram
{
	MaxProgramTexelOffset,//< max texel offset supported by the shader program
	MinProgramTexelOffset,//< min texel offset supported by the shader program
	NumProgramBinaryFormats, //< number of program binary formats supported
	NumCompressedTextureFormats,//< number of compressed texture formats supported
	NumShaderBinaryFormats, //< number of shader binary format supported
	Count
};


/// <summary>TransformFeedback query.</summary>
enum class TransformFeedback
{
	MaxInterleavedComponent,//< max transform-feedback interleaved component supported
	MaxSeparateAttribs,//< max transform feedback separate attribute supported
	SeparateComponents,//< max transform feedback
	Count
};


/// <summary>Uniform query.</summary>
enum class Uniform
{
	MaxUniformBlockSize,//< maximum size in basic machine units of a uniform block.
	MaxUniformBufferBindings,//< maximum number of uniform buffer binding points
	MaxCombinedUniformBlocks,//<maximum number of uniform blocks per program
	MaxCombinedVertexUniformComponent,//< maximum number of vertex uniform supported
	Count
};


/// <summary>Element query.</summary>
enum class Element
{
	MaxIndices,//< max indices supported
	MaxVertices, //< max vertices supported
	Count
};


/// <summary>Buffers query.</summary>
enum class Buffers  { MaxDrawBuffers, Count };

/// <summary>FragmentShader query.</summary>
enum class FragmentShader
{
	MaxFragmentInputComponents,//< max fragment input components  supported
	MaxfragmentUniformBlocks,//< max fragment uniform blocks  supported
	MaxFragmentUniformComponent,//< max fragment uniform component supported
	MaxFragmentUniformVectors,
	Count
};

/// <summary>Query gpu's texture and sampler capabilities.</summary>
/// <param name="context">The context whose capability is queried</param>
/// <param name="query">A TextureAndSamplers enum representing the capability to be queried</param>
/// <returns>The capability requested, as an Int32</returns>
int32 get(IGraphicsContext& context, TextureAndSamplers query);


/// <summary>Query gpu's fragment capabilities.</summary>
/// <param name="context">The context whose capability is queried</param>
/// <param name="query">A FragmentShader enum representing the capability to be queried</param>
/// <returns>The capability requested, as an Int32</returns>
int32 get(IGraphicsContext& context, FragmentShader query);

/// <summary>Query gpu's shader and program capabilities.</summary>
/// <param name="context">The context whose capability is queried</param>
/// <param name="query">A ShaderAndProgram enum representing the capability to be queried</param>
/// <returns>The capability requested, as an Int32</returns>
int32 get(IGraphicsContext& context, ShaderAndProgram query);

/// <summary>Query gpu's Buffers capabilities.</summary>
/// <param name="context">The context whose capability is queried</param>
/// <param name="query">A Buffers enum representing the capability to be queried</param>
/// <returns>The capability requested, as an Int32</returns>
int32 get(IGraphicsContext& context, Buffers query);

/// <summary>Query gpu's Element capabilities.</summary>
/// <param name="context">The context whose capability is queried</param>
/// <param name="query">A Element enum representing the capability to be queried</param>
/// <returns>The capability requested, as an Int32</returns>
int32 get(IGraphicsContext& context, Element query);

/// <summary>Query gpu's transformfeedback capabilities.</summary>
/// <param name="context">The context whose capability is queried</param>
/// <param name="query">A TransformFeedback enum representing the capability to be queried</param>
/// <returns>The capability requested, as an Int32</returns>
int32 get(IGraphicsContext& context, TransformFeedback query);

/// <summary>Query gpu's Uniform capabilities.</summary>
/// <param name="context">The context whose capability is queried</param>
/// <param name="query">A Uniform enum representing the capability to be queried</param>
/// <returns>The capability requested, as an Int32</returns>
int32 get(IGraphicsContext& context, Uniform query);

}
}