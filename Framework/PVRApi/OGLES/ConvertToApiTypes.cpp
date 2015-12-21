/*!*********************************************************************************************************************
\file         PVRApi\OGLES\ConvertToApiTypes.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation of the conversions from PVR Framework enums to OpenGL types. See ConvertToApiTypes.h.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/OGLES/ConvertToApiTypes.h"
#include "PVRApi/ApiObjects/FboCreateParam.h"
#include "PVRCore/Assert_.h"
#include "PVRAssets/SamplerDescription.h"

namespace pvr {
namespace api  {
namespace ConvertToGles {
GLenum face(Face::Enum face)
{
	static GLenum glFace[] = { GL_NONE, GL_BACK, GL_FRONT, GL_FRONT_AND_BACK };
	return glFace[face];
}

GLenum polygonWindingOrder(PolygonWindingOrder::Enum order)
{
	static GLenum glWindingOrder[] = { GL_CW, GL_CCW };
	return glWindingOrder[order];
}

GLenum comparisonMode(ComparisonMode::Enum func)
{
	static GLenum glCompareMode[] = { GL_NONE, GL_LEQUAL, GL_LESS, GL_EQUAL, GL_NOTEQUAL, GL_GREATER, GL_GEQUAL, GL_ALWAYS, GL_NEVER };
	return glCompareMode[func];
}

GLenum fboAttachmentType(FboAttachmentType::Enum type)
{
	static	GLenum glFboAttachmentType[] = { GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT, GL_DEPTH_STENCIL_ATTACHMENT, GL_COLOR_ATTACHMENT0 };
	if (type <= FboAttachmentType::Color)
	{
		return glFboAttachmentType[type];
	}

	return GL_COLOR_ATTACHMENT0 + (type - FboAttachmentType::Color);
}

GLenum fboTextureAttachmentTexType(FboTextureTarget::Enum type)
{
	static GLenum glTexType[] =
	{
		GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};
	return glTexType[type];
}

GLenum textureDimension(TextureDimension::Enum texType)
{
	PVR_ASSERT(texType == 2 || texType == 3 || texType == 4 || texType == 6);
#if BUILD_API_MAX<30
	static GLenum glTextureType[] = { GL_NONE, GL_NONE, GL_TEXTURE_2D, GL_TEXTURE_3D_OES, GL_TEXTURE_CUBE_MAP, GL_NONE, GL_TEXTURE_2D_ARRAY, GL_NONE, GL_NONE };
#else
    static GLenum glTextureType[] = { GL_NONE, GL_NONE, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP, GL_NONE, GL_TEXTURE_2D_ARRAY, GL_NONE, GL_NONE };
#endif

	return glTextureType[texType];
}

GLenum dataType(DataType::Enum dataType)
{
	static const GLenum map[] = { GL_NONE, GL_FLOAT, GL_INT, GL_UNSIGNED_SHORT, GL_RGBA,
	                              GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_FIXED,
	                              GL_UNSIGNED_BYTE, GL_SHORT, GL_SHORT,
	                              GL_BYTE, GL_BYTE,
	                              GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT,
	                              GL_UNSIGNED_INT, GL_NONE
	                            };
	return map[dataType];
}

GLenum samplerWrap(SamplerWrap::Enum samplerWrap)
{
	if (samplerWrap > SamplerWrap::Clamp)
	{
		static const char* wrapNames[] = { "Border", "MirrorClamp" };
		pvr::Log("%s SamplerWrap '%s' not support falling back to default", wrapNames[samplerWrap - SamplerWrap::Border]);
		samplerWrap = SamplerWrap::Default;
	}
	const static GLenum glSampler[] = { GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE };
	return glSampler[samplerWrap];
}

GLenum stencilOp(StencilOp::Enum stencilOp)
{
	static GLenum glStencilOpValue[] = { GL_KEEP, GL_ZERO, GL_REPLACE, GL_INCR, GL_INCR_WRAP, GL_DECR, GL_DECR_WRAP, GL_INVERT };
	return glStencilOpValue[stencilOp];
}

GLenum blendEq(BlendOp::Enum blendOp)
{
	static GLenum glOp[] = {GL_FUNC_ADD, GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT, GL_MIN, GL_MAX };
	return glOp[blendOp];
}

GLenum blendFactor(BlendFactor::Enum blendFactor)
{
	static GLenum glBlendFact[] = { GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR,
	                                GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA,
	                                GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA, GL_SRC_ALPHA_SATURATE
	                              };
	return glBlendFact[blendFactor];
}


GLenum memBarrierFlagOut(uint32 mask)
{
#if BUILD_API_MAX>=31
    static const GLenum flags[] =
    {
        0,
        GL_SHADER_STORAGE_BARRIER_BIT,// ShaderWrite
        GL_FRAMEBUFFER_BARRIER_BIT,// ColorAttachment
        GL_FRAMEBUFFER_BARRIER_BIT,//
        GL_TRANSFORM_FEEDBACK_BARRIER_BIT//10
    };
    
    static const MemBarrierFlagout::Enum barrierFlags[] =
    {
        MemBarrierFlagout::CpuWrite,
        MemBarrierFlagout::ShaderWrite,
        MemBarrierFlagout::ColorAttachment,
        MemBarrierFlagout::DepthStencilAttachment,
        MemBarrierFlagout::Transfer,
    };
    GLenum outMask = 0;
    for (uint16 i = 0; i < MemBarrierFlagout::Count; ++i)
    {
        if (mask & barrierFlags[i]) { outMask |= flags[i]; }
    }
    return outMask;
#endif
    Log(Log.Error, "MemBarrierFlagout: MemBarrierFlagout not built into PVRApi (BUILD_API_MAX<=30)");
    return 0;
}

GLenum memBarrierFlagIn(uint32 mask)
{
#if BUILD_API_MAX>=31
	static const GLenum glFlags[] =
	{
		GL_NONE,
		GL_COMMAND_BARRIER_BIT,
		GL_ELEMENT_ARRAY_BARRIER_BIT,
		GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT,
		GL_UNIFORM_BARRIER_BIT,
		GL_SHADER_IMAGE_ACCESS_BARRIER_BIT,
		GL_FRAMEBUFFER_BARRIER_BIT,
		GL_FRAMEBUFFER_BARRIER_BIT,
		GL_TRANSFORM_FEEDBACK_BARRIER_BIT,
	};
	static const api::MemBarrierFlagIn::Enum flags[] =
	{
		api::MemBarrierFlagIn::CpuRead,
		api::MemBarrierFlagIn::IndirectCommand,
		api::MemBarrierFlagIn::IndexFetch,
		api::MemBarrierFlagIn::VertexAttributeFetch,
		api::MemBarrierFlagIn::UniformRead,
		api::MemBarrierFlagIn::ShaderRead,
		api::MemBarrierFlagIn::ColorAttachment,
		api::MemBarrierFlagIn::DepthStencilAttachment,
		api::MemBarrierFlagIn::Transfer,
		api::MemBarrierFlagIn::AllBarrier
	};
	GLenum outVal = 0;
	for (uint32 i = 0; i < api::MemBarrierFlagIn::Count; ++i)
	{
		if (mask & flags[i])
		{
			outVal |= glFlags[i];
		}
	}
	return outVal;
#else
	Log(Log.Error, "MemBarrierFlagIn: MemBarrierFlagIn not built into PVRApi (BUILD_API_MAX<=TRIANTA)");
	return 0;
#endif

}


GLenum drawPrimitiveType(PrimitiveTopology::Enum primitiveType)
{
	static GLenum glPrimtiveType[] = { GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP, GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN };
	return glPrimtiveType[primitiveType];
}

GLenum gpuCapabilitiesTextureAndSamplers(gpuCapabilities::TextureAndSamplers::Enum capabilities)
{
#if BUILD_API_MAX<30
	bool capsSupported = (capabilities != gpuCapabilities::TextureAndSamplers::MaxSamples &&
	                      capabilities != gpuCapabilities::TextureAndSamplers::Max3DTextureSize &&
	                      capabilities != gpuCapabilities::TextureAndSamplers::MaxArrayTextureLayer &&
	                      capabilities != gpuCapabilities::TextureAndSamplers::MaxTextureLodBias);
	if (!capsSupported) { Log(Log.Error, "GpuCapabilities: Specified capability queried not supported for OpenGL ES 2"); }
	PVR_ASSERT(capsSupported && capabilities < gpuCapabilities::TextureAndSamplers::Count && "Invalid GpuCapabilities");
	static const GLenum glCapabilities[] =
	{
		GL_MAX_TEXTURE_IMAGE_UNITS, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_MAX_TEXTURE_SIZE, GL_MAX_CUBE_MAP_TEXTURE_SIZE
	};
#else
	PVR_ASSERT(capabilities < gpuCapabilities::TextureAndSamplers::Count && "Invalid GpuCapabilities");
	static const GLenum glCapabilities[] =
	{
		GL_MAX_TEXTURE_IMAGE_UNITS, GL_MAX_SAMPLES, GL_MAX_3D_TEXTURE_SIZE, GL_MAX_ARRAY_TEXTURE_LAYERS, GL_MAX_TEXTURE_LOD_BIAS, GL_MAX_TEXTURE_SIZE, GL_MAX_CUBE_MAP_TEXTURE_SIZE
	};
#endif
	return glCapabilities[capabilities];
}

GLenum gpuCapabilitiesTransformFeedback(gpuCapabilities::TransformFeedback::Enum caps)
{
#if BUILD_API_MAX<30
	Log(Log.Error,
	    "GpuCapabilities::TransformFeedback: TransformFeedback not built into PVRApi (BUILD_API_MAX is defined and BUILD_API_MAX<30)");
	return 0;
#else
	static const GLenum glCaps[] =
	{
		GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS,
		GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS,
		GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS
	};
	return glCaps[caps];
#endif
}

GLenum gpuCapabilitiesFragment(gpuCapabilities::FragmentShader::Enum caps)
{
#if BUILD_API_MAX<30
	Log(Log.Error,
	    "GpuCapabilities::Fragment Shader capabilities query not built into PVRApi (BUILD_API_MAX is defined and BUILD_API_MAX<30)");
	return 0;
#else
	static const GLenum glCaps[] = { GL_MAX_FRAGMENT_INPUT_COMPONENTS,
	                                 GL_MAX_FRAGMENT_UNIFORM_BLOCKS,
	                                 GL_MAX_FRAGMENT_UNIFORM_COMPONENTS
	                               };
	return glCaps[caps];
#endif
}

GLenum gpuCapabilitiesUniform(gpuCapabilities::Uniform::Enum caps)
{
#if BUILD_API_MAX<30
	Log(Log.Error,
	    "GpuCapabilities::Uniform capabilities query not built into PVRApi (BUILD_API_MAX is defined and BUILD_API_MAX<30)");
	return 0;
#else
	static const GLenum glCaps[] = {GL_MAX_UNIFORM_BLOCK_SIZE,
	                                GL_MAX_UNIFORM_BUFFER_BINDINGS, GL_MAX_COMBINED_UNIFORM_BLOCKS,
	                                GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS
	                               };
	return glCaps[caps];
#endif
}

GLenum gpuCapabilitiesElement(gpuCapabilities::Element::Enum caps)
{
#if BUILD_API_MAX<30
	Log(Log.Error,
	    "GpuCapabilities: Element capabilities query not built into PVRApi (BUILD_API_MAX is defined and BUILD_API_MAX<30)");
	return 0;
#else
	static const GLenum glCaps[] = { GL_MAX_ELEMENT_INDEX, GL_MAX_ELEMENTS_VERTICES };
	return glCaps[caps];
#endif
}

GLenum gpuCapabilitiesBuffers(gpuCapabilities::Buffers::Enum caps)
{
#if BUILD_API_MAX<30
	Log(Log.Error,
	    "GpuCapabilities: Buffers capabilities query not built into PVRApi (BUILD_API_MAX is defined and BUILD_API_MAX<30)");
	return 0;
#else
	static const GLenum glCaps[] = {GL_MAX_DRAW_BUFFERS};
	return glCaps[caps];
#endif
}

GLenum gpuCapabilitiesShaderAndPrograms(gpuCapabilities::ShaderAndProgram::Enum caps)
{
#if BUILD_API_MAX<30
	Log(Log.Error,
	    "GpuCapabilities: Shaders and Programs capabilities query not built into PVRApi (BUILD_API_MAX is defined and BUILD_API_MAX<30)");
	return 0;
#else
	static const GLenum glCaps[] = { GL_MAX_PROGRAM_TEXEL_OFFSET, GL_MIN_PROGRAM_TEXEL_OFFSET, GL_NUM_COMPRESSED_TEXTURE_FORMATS,
	                                 GL_NUM_SHADER_BINARY_FORMATS, GL_NUM_PROGRAM_BINARY_FORMATS
	                               };
	return glCaps[caps];
#endif
}

}
}
}
//!\endcond
