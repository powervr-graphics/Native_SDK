/*!*********************************************************************************************************************
\file         PVRApi\OGLES\ComputePipelineGles.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         OpenGL ES Implementation of the ComputePipeline class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/ApiObjects/ComputePipeline.h"
#include "PVRApi/OGLES/StateContainerGles.h"
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRApi/OGLES/ShaderGles.h"
#include "PVRNativeApi/ShaderUtils.h"


namespace pvr {
namespace api {
namespace pipelineCreation {
void createStateObjects(const ComputeShaderStageCreateParam& thisobject, gles::ComputeStateContainer& storage);
}
namespace impl {

/*!*********************************************************************************************************************
\brief A configuration of the Compute Pipeline which must be bound before launching a Compute operation.
***********************************************************************************************************************/
class ComputePipelineImplementationDetails : public native::HPipeline_
{
public:
	template<typename MyClass_> friend struct ::pvr::RefCountEntryIntrusive;
	template<typename any> friend class ::pvr::api::impl::PackagedBindable;
	ComputePipelineImplementationDetails(const GraphicsContext& context) : m_context(context), m_initialized(false) {}
	friend ComputePipeline IGraphicsContext::createComputePipeline(const api::ComputePipelineCreateParam&);
	Result::Enum init(const ComputePipelineCreateParam& desc);

	void bind(IGraphicsContext& context);

	void setAll();

	native::HPipeline_& getNativeObject() { return *this; }
	const native::HPipeline_& getNativeObject() const { return *this; }

	inline gles::ComputeShaderProgramState getShaderProgram()const;

	/*!*********************************************************************************************************************
	\brief Get the PipelineLayout of this pipeline.
	\return The PipelineLayout of this pipeline.
	***********************************************************************************************************************/
	const PipelineLayout& getPipelineLayout()const { return m_states.pipelineLayout; }

	/*!*********************************************************************************************************************
	\brief If free standing uniforms are supported by the underlying API, get the location of a uniform.
	If a uniform is not found (does not exist or is not active in the shader program), -1 is returned.
	\param uniform uniform name
	\return The location of a uniform variable
	***********************************************************************************************************************/
	int32 getUniformLocation(const char8* uniform);

	/*!*********************************************************************************************************************
	\brief Release all resources held by this pipeline
	***********************************************************************************************************************/
	void destroy()
	{
		m_context.reset();
		for (gles::ComputeStateContainer::StateContainer::iterator it = m_states.states.begin(); it != m_states.states.end(); ++it)
		{
			delete *it;
		}
		m_states.computeShader.reset();
		m_states.pipelineLayout.reset();
		m_states.states.clear();
		m_initialized = false;
	}

	/*!*********************************************************************************************************************
	\brief Destructor. Releases all resources held by this pipeline
	***********************************************************************************************************************/
	~ComputePipelineImplementationDetails()
	{
		destroy();
	}
	Result::Enum createProgram();

	GraphicsContext m_context;
	gles:: ComputeStateContainer m_states;
	bool m_initialized;
};

int32 ComputePipeline_::getUniformLocation(const char8* uniform) { return pimpl->getUniformLocation(uniform); }

ComputePipeline_::ComputePipeline_(GraphicsContext& context)
{
	pimpl.reset(new ComputePipelineImplementationDetails(context));
}

native::HPipeline_& ComputePipeline_::getNativeObject() { return *pimpl; }
const native::HPipeline_& ComputePipeline_::getNativeObject() const { return *pimpl; }

void ComputePipeline_::destroy()
{
	return pimpl->destroy();
}

const PipelineLayout& ComputePipeline_::getPipelineLayout() const { return pimpl->getPipelineLayout(); }

inline void ComputePipelineImplementationDetails::setAll()
{
	debugLogApiError("ComputePipeline::setAll entry");

	for (gles::ComputeStateContainer::StateContainer::iterator it = m_states.states.begin();
	     it != m_states.states.end(); ++it)
	{
		(*it)->set(*m_context);
		debugLogApiError("GraphicsPipeline::setFromParent::set");
	}
}

inline Result::Enum ComputePipelineImplementationDetails::init(const ComputePipelineCreateParam& desc)
{
	if (m_initialized) { return Result::AlreadyInitialized; }
	assertion(!desc.pipelineLayout.isNull());
	createStateObjects(desc.computeShader, m_states);
	m_states.pipelineLayout = desc.pipelineLayout;
	if (m_states.computeShader.isValid())
	{
		return createProgram();
	}
	return Result::NotInitialized;
}

Result::Enum ComputePipeline_::init(const ComputePipelineCreateParam& desc)
{
	return pimpl->init(desc);
}

inline gles::ComputeShaderProgramState ComputePipelineImplementationDetails::getShaderProgram()const
{
	assertion(m_states.states.size() >= 1);
	return *(static_cast<gles::ComputeShaderProgramState*>(m_states.states[0]));
}

ComputePipeline_::~ComputePipeline_()
{
	destroy();
}





Result::Enum ComputePipelineImplementationDetails::createProgram()
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

void ComputePipeline_::bind(IGraphicsContext& context)
{
	pvr::platform::ContextGles& contextES = static_cast<pvr::platform::ContextGles&>(context);
	if (!contextES.isLastBoundPipelineCompute() || contextES.getBoundComputePipeline() != this)
	{
		pimpl->setAll();
		static_cast<pvr::platform::ContextGles&>(context).onBind(this);
	}
}

int32 ComputePipelineImplementationDetails::getUniformLocation(const char8* uniform)
{
	GLint prog;
	gl::GetIntegerv(GL_CURRENT_PROGRAM, &prog);
	gl::UseProgram(getShaderProgram().getNativeObject());
	int32 ret = gl::GetUniformLocation(getShaderProgram().getNativeObject(), uniform);
	gl::UseProgram(prog);
	return ret;
}

}
}
}
//!\endcond
