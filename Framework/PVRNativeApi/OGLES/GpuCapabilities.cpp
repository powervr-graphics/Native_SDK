/*!
\brief OpenGL ES Implementation of the GPU Capabilities functions. See GpuCapabilities.h.
\file PVRNativeApi/OGLES/GpuCapabilities.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRNativeApi/OGLES/GpuCapabilitiesGles.h"
#include "PVRNativeApi/OGLES/ConvertToApiTypes.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"

namespace pvr {
namespace gpuCapabilities {


int32 get(IGraphicsContext& context, TextureAndSamplers query)
{
	GLint val;
	gl::GetIntegerv(nativeGles::ConvertToGles::gpuCapabilitiesTextureAndSamplers(query), &val);
	return val;
}

int32 get(IGraphicsContext& context, TransformFeedback query)
{
	GLint val;
	gl::GetIntegerv(nativeGles::ConvertToGles::gpuCapabilitiesTransformFeedback(query), &val);
	return val;
}

int32 get(IGraphicsContext& context, FragmentShader query)
{
	GLint val;
	gl::GetIntegerv(nativeGles::ConvertToGles::gpuCapabilitiesFragment(query), &val);
	return val;
}

int32 get(IGraphicsContext& context, Uniform query)
{
	GLint val;
	gl::GetIntegerv(nativeGles::ConvertToGles::gpuCapabilitiesUniform(query), &val);
	return val;
}

int32 get(IGraphicsContext& context, Buffers query)
{
	GLint val;
	gl::GetIntegerv(nativeGles::ConvertToGles::gpuCapabilitiesBuffers(query), &val);
	return val;
}

int32 get(IGraphicsContext& context, Element query)
{
	GLint val;
	gl::GetIntegerv(nativeGles::ConvertToGles::gpuCapabilitiesElement(query), &val);
	return val;
}

int32 get(IGraphicsContext& context, ShaderAndProgram query)
{
	GLint val;
	gl::GetIntegerv(nativeGles::ConvertToGles::gpuCapabilitiesShaderAndPrograms(query), &val);
	return val;
}

}
}
//!\endcond