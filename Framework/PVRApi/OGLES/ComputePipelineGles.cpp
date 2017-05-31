<<<<<<< HEAD
/*!*********************************************************************************************************************
\file         PVRApi\OGLES\ComputePipelineGles.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         OpenGL ES Implementation of the ComputePipeline class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/OGLES/ComputePipelineGles.h"
=======
/*!
\brief OpenGL ES Implementation of the ComputePipeline class.
\file PVRApi/OGLES/ComputePipelineGles.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRApi/ApiIncludes.h"
>>>>>>> 1776432f... 4.3
#include "PVRApi/OGLES/StateContainerGles.h"
#include "PVRNativeApi/NativeGles.h"
#include "PVRApi/OGLES/ShaderGles.h"
<<<<<<< HEAD
#include "PVRNativeApi/ShaderUtils.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
=======
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRApi/OGLES/ComputePipelineGles.h"
>>>>>>> 1776432f... 4.3

namespace pvr {
namespace api {
namespace pipelineCreation {
void createStateObjects(const ComputeShaderStageCreateParam& thisobject, gles::ComputeStateContainer& storage);
}
namespace gles {
<<<<<<< HEAD
int32 ComputePipelineImplGles::getUniformLocation(const char8* uniform)
{
    GLint prog;
    gl::GetIntegerv(GL_CURRENT_PROGRAM, &prog);
    gl::UseProgram(getShaderProgram().getNativeObject());
    int32 ret = gl::GetUniformLocation(getShaderProgram().getNativeObject(), uniform);
    gl::UseProgram(prog);
    return ret;
}

void ComputePipelineImplGles::getUniformLocation(const char8** uniforms, uint32 numUniforms, int32* outLocation)
{
    GLint prog;
    gl::GetIntegerv(GL_CURRENT_PROGRAM, &prog);
    gl::UseProgram(getShaderProgram().getNativeObject());
    for (uint32 i = 0; i < numUniforms; ++i)
    {
        outLocation[i] = gl::GetUniformLocation(getShaderProgram().getNativeObject(), uniforms[i]);
    }
    gl::UseProgram(prog);
}

bool ComputePipelineImplGles::init(const ComputePipelineCreateParam& desc, impl::ComputePipeline_ *owner)
{
	if (m_initialized) { return true; }
    assertion(!desc.pipelineLayout.isNull());
    createStateObjects(desc.computeShader, m_states);
    m_states.pipelineLayout = desc.pipelineLayout;
    m_owner = owner;
    if (m_states.computeShader.isValid())
    {
		if (createProgram() == Result::Success)
		{
			m_initialized = true;
			return true;
    }
	}
	return false;
}

void ComputePipelineImplGles::bind()
{
    pvr::platform::ContextGles& contextES = static_cast<platform::ContextGles&>(*m_context);
    if (!contextES.isLastBoundPipelineCompute() || contextES.getBoundComputePipeline() != m_owner)
    {
        setAll();
        contextES.onBind(m_owner);
    }
}

void ComputePipelineImplGles::setAll()
{
    debugLogApiError("ComputePipeline::setAll entry");

    for (gles::ComputeStateContainer::StateContainerIter it = m_states.states.begin();
         it != m_states.states.end(); ++it)
    {
        (*it)->set(*m_context);
        debugLogApiError("GraphicsPipeline::setFromParent::set");
    }
}

native::HPipeline_& ComputePipelineImplGles::getNativeObject() { return *this; }

const native::HPipeline_& ComputePipelineImplGles::getNativeObject() const { return *this; }

ComputeShaderProgramState ComputePipelineImplGles::getShaderProgram() const
{
    assertion(m_states.states.size() >= 1);
    return *(static_cast<gles::ComputeShaderProgramState*>(m_states.states[0]));
}

const PipelineLayout& ComputePipelineImplGles::getPipelineLayout() const { return m_states.pipelineLayout; }

void ComputePipelineImplGles::destroy()
{
    m_context.reset();
    for (gles::ComputeStateContainer::StateContainer::iterator it = m_states.states.begin();
         it != m_states.states.end(); ++it)
    {
        delete *it;
    }
    m_states.computeShader.reset();
    m_states.pipelineLayout.reset();
    m_states.states.clear();
    m_initialized = false;
}

Result ComputePipelineImplGles::createProgram()
{
    native::HPipeline computeProg;
    gles::ComputeShaderProgramState* program = new gles::ComputeShaderProgramState();
    program->generate();
    native::HShader_ shader = m_states.computeShader->getNativeObject();
    if ((!pvr::utils::createShaderProgram(&shader, 1, 0, 0, 0, program->getNativeObject(), 0,
                                          &m_context->getApiCapabilities())))
    {
        delete program;
        return Result::UnknownError;
    }
    m_states.states.push_back(program);
    return pvr::Result::Success;
}
=======

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
>>>>>>> 1776432f... 4.3
}
}
}
