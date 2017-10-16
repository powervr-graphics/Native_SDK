/*!
\brief OpenGL ES Implementation of the ComputePipeline class.
\file PVRApi/OGLES/ComputePipelineGles.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/OGLES/StateContainerGles.h"
#include "PVRNativeApi/NativeGles.h"
#include "PVRApi/OGLES/ShaderGles.h"
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRApi/OGLES/ComputePipelineGles.h"

namespace pvr {
namespace api {
namespace pipelineCreation {
void createStateObjects(const ComputeShaderStageCreateParam& thisobject, gles::ComputeStateContainer& storage);
}
namespace gles {

inline int32 getUniformLocation_(const char8* uniform, GLuint prog)
{
	int32 ret = gl::GetUniformLocation(prog, uniform);
	if (ret == -1)
	{
		Log(Log.Debug, "GraphicsPipeline::getUniformLocation [%s] for program [%d]  returned -1: Uniform was not active", uniform, prog);
	}
	return ret;
}

int32 ComputePipelineImplGles::getUniformLocation(const char8* uniform) const
{
	return getUniformLocation_(uniform, native_cast(getShaderProgram()));
}

void ComputePipelineImplGles::getUniformLocation(const char8** uniforms, uint32 numUniforms, int32* outLocation) const
{
	GLuint prog = native_cast(getShaderProgram());
	for (uint32 i = 0; i < numUniforms; ++i)
	{
		outLocation[i] = getUniformLocation_(uniforms[i], prog);
	}
}

const ComputeShaderProgramState& ComputePipelineImplGles::getShaderProgram()const
{
	assertion(_states.states.size() >= 1);
	return *(static_cast<gles::ComputeShaderProgramState*>(_states.states[0]));
}

ComputeShaderProgramState& ComputePipelineImplGles::getShaderProgram()
{
	assertion(_states.states.size() >= 1);
	return *(static_cast<gles::ComputeShaderProgramState*>(_states.states[0]));
}

const ComputePipelineCreateParam& ComputePipelineImplGles::getCreateParam()const
{
	return _createParam;
}


const pvr::api::PipelineLayout& ComputePipelineImplGles::getPipelineLayout() const
{
	assertion(!_states.pipelineLayout.isNull(), "invalid pipeline layout");
	return _states.pipelineLayout;
}

void ComputePipelineImplGles::destroy()
{
	_context.reset();
	for (gles::ComputeStateContainer::StateContainer::iterator it = _states.states.begin();
	     it != _states.states.end(); ++it)
	{
		delete *it;
	}
	_states.computeShader.reset();
	_states.pipelineLayout.reset();
	_states.states.clear();
	_initialized = false;
}

bool ComputePipelineImplGles::init(const ComputePipelineCreateParam& desc, ComputePipeline& owner)
{
	if (_initialized) { return true; }
	assertion(!desc.pipelineLayout.isNull());
	createStateObjects(desc.computeShader, _states);
	_states.pipelineLayout = desc.pipelineLayout;
	_owner = owner.get();
	if (_states.computeShader.isValid())
	{
		if (createProgram() == Result::Success)
		{
			_initialized = true;
			return true;
		}
	}
	return false;
}

Result ComputePipelineImplGles::createProgram()
{
	native::HPipeline computeProg;
	gles::ComputeShaderProgramState* program = new gles::ComputeShaderProgramState();
	program->generate();
	native::HShader_ shader = native_cast(*_states.computeShader);
	if ((!nativeGles::createShaderProgram(&shader, 1, 0, 0, 0, native_cast(*program), 0,
	                                      &_context->getApiCapabilities())))
	{
		delete program;
		return Result::UnknownError;
	}
	_states.states.push_back(program);
	return pvr::Result::Success;
}

void ComputePipelineImplGles::setAll()
{
	debugLogApiError("ComputePipeline::setAll entry");

	for (gles::ComputeStateContainer::StateContainerIter it = _states.states.begin();
	     it != _states.states.end(); ++it)
	{
		(*it)->set(*_context);
		debugLogApiError("GraphicsPipeline::setFromParent::set");
	}
}
}
}
}
