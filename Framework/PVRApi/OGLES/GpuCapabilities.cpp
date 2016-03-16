/*!*********************************************************************************************************************
\file         PVRApi\OGLES\GpuCapabilities.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         OpenGL ES Implementation of the GPU Capabilities functions. See GpuCapabilities.h.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/GpuCapabilities.h"
#include "PVRNativeApi/OGLES/ConvertToApiTypes.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"

namespace pvr {
namespace api {
namespace gpuCapabilities {


int32 get(IGraphicsContext& context, TextureAndSamplers::Enum query)
{
	GLint val;
	gl::GetIntegerv(api::ConvertToGles::gpuCapabilitiesTextureAndSamplers(query), &val);
	return val;
}

int32 get(IGraphicsContext& context, TransformFeedback::Enum query)
{
	GLint val;
	gl::GetIntegerv(api::ConvertToGles::gpuCapabilitiesTransformFeedback(query), &val);
	return val;
}

int32 get(IGraphicsContext& context, FragmentShader::Enum query)
{
	GLint val;
	gl::GetIntegerv(api::ConvertToGles::gpuCapabilitiesFragment(query), &val);
	return val;
}

int32 get(IGraphicsContext& context, Uniform::Enum query)
{
	GLint val;
	gl::GetIntegerv(api::ConvertToGles::gpuCapabilitiesUniform(query), &val);
	return val;
}

int32 get(IGraphicsContext& context, Buffers::Enum query)
{
	GLint val;
	gl::GetIntegerv(api::ConvertToGles::gpuCapabilitiesBuffers(query), &val);
	return val;
}

int32 get(IGraphicsContext& context, Element::Enum query)
{
	GLint val;
	gl::GetIntegerv(api::ConvertToGles::gpuCapabilitiesElement(query), &val);
	return val;
}

int32 get(IGraphicsContext& context, ShaderAndProgram::Enum query)
{
	GLint val;
	gl::GetIntegerv(api::ConvertToGles::gpuCapabilitiesShaderAndPrograms(query), &val);
	return val;
}

}
}
}
//!\endcond
