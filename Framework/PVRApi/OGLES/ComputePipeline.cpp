/*!*********************************************************************************************************************
\file         PVRApi\OGLES\ComputePipeline.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         OpenGL ES Implementation of the ComputePipeline class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/ApiObjects/ComputePipeline.h"
#include "PVRApi/OGLES/NativeObjectsGles.h"
#include "PVRApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/OGLES/ShaderGles.h"
#include "PVRApi/ShaderUtils.h"

namespace pvr {
namespace api {
namespace impl {
Result::Enum ComputePipelineImpl::createProgram()
{
	native::HShaderProgram computeProg;
	ComputeShaderProgramState* program = new ComputeShaderProgramState();
	program->generate();
	native::HShader_ shader =  native::useNativeHandle(m_states.computeShader);
	if ((!pvr::utils::createShaderProgram(&shader, 1, 0,0, 0, program->getNativeHandle(), 0,
	            &m_context->getApiCapabilities())))
	{
		return Result::UnknownError;
	}
	m_states.states.push_back(program);
	return pvr::Result::Success;
}

void ComputePipelineImpl::bind(IGraphicsContext& context)
{
	pvr::platform::ContextGles& contextES = static_cast<pvr::platform::ContextGles&>(context);
	if (!contextES.isLastBoundPipelineCompute() || contextES.getBoundComputePipeline() != this){
		setAll();
		static_cast<pvr::platform::ContextGles&>(context).onBind(this);
	}
}

int32 ComputePipelineImpl::getUniformLocation(const char8* uniform)
{
	GLint prog;
	gl::GetIntegerv(GL_CURRENT_PROGRAM, &prog);
	gl::UseProgram(getShaderProgram().getNativeHandle()->handle);
	int32 ret = gl::GetUniformLocation(getShaderProgram().getNativeHandle()->handle, uniform);
	gl::UseProgram(prog);
	return ret;
}

}
}
}
//!\endcond
