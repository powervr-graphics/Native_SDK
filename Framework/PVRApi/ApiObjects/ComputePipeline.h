/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\ComputePipeline.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the Compute Pipeline.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRApi/ApiObjects/PipelineState.h"
#include "PVRApi/ApiObjects/PipelineStateCreateParam.h"
#include "PVRApi/ApiObjects/ShaderProgramState.h"
#include "PVRApi/ApiErrors.h"
#include <vector>

namespace pvr {
namespace api {
namespace impl {
//!\cond NO_DOXYGEN
template<typename> class PackagedBindable;
template<typename, typename> class PackagedBindableWithParam;
//!\endcond

/*!*********************************************************************************************************************
\brief A configuration of the Compute Pipeline which must be bound before launching a Compute operation.
***********************************************************************************************************************/
class ComputePipelineImpl
{
    template<typename MyClass_> friend struct ::pvr::RefCountEntryIntrusive;
	template<typename any> friend class ::pvr::api::impl::PackagedBindable;
	ComputePipelineImpl(GraphicsContext& context) : m_context(context), m_initialised(false) {}
	friend ComputePipeline IGraphicsContext::createComputePipeline(const api::ComputePipelineCreateParam&);
	friend class ::pvr::api::impl::PopPipeline;
	Result::Enum init(const ComputePipelineCreateParam& desc);

	void bind(IGraphicsContext& context);

	void setAll();

public:
	typedef int isBindable;

	inline ComputeShaderProgramState getShaderProgram()const;

	/*!*********************************************************************************************************************
	\brief Get the PipelineLayout of this pipeline.
	\return The PipelineLayout of this pipeline.
	***********************************************************************************************************************/
	const PipelineLayout& getPipelineLayout()const { return m_states.pipelineLayout; }

	/*!*********************************************************************************************************************
	\brief If free standing uniforms are supported by the underlying API, get the location of a several uniforms at once.
	       If a uniform is not found (does not exist or is not active in the shader program), -1 is returned.
	\param[in] uniforms An array of uniform names
	\param[in] numUniforms The number of uniforms
	\param[out] outLocation A pointer to enough positions to store the uniform location.
	***********************************************************************************************************************/
	void getUniformLocation(const char8** uniforms, uint32 numUniforms, int32* outLocation);

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
		for (ComputeStateContainer::StateContainer::iterator it = m_states.states.begin(); it != m_states.states.end(); ++it)
		{
			delete *it;
		}
		m_states.computeShader.reset();
		m_states.pipelineLayout.reset();
		m_states.states.clear();
		m_initialised = false;
	}

	/*!*********************************************************************************************************************
	\brief Destructor. Releases all resources held by this pipeline
	***********************************************************************************************************************/
	~ComputePipelineImpl()
	{
		destroy();
	}
private:
	Result::Enum createProgram();
	GraphicsContext m_context;
	ComputeStateContainer m_states;
	bool m_initialised;
};

inline void ComputePipelineImpl::setAll()
{
	debugLogApiError("ComputePipeline::setAll entry");

	for (ComputeStateContainer::StateContainer::iterator it = m_states.states.begin();
	        it != m_states.states.end(); ++it)
	{
		(*it)->set(*m_context);
		debugLogApiError("GraphicsPipeline::setFromParent::set");
	}
}

inline Result::Enum ComputePipelineImpl::init(const ComputePipelineCreateParam& desc)
{
	if (m_initialised) { return Result::AlreadyInitialised; }
	PVR_ASSERT(!desc.pipelineLayout.isNull());
	desc.computeShader.createStateObjects(m_states);
	m_states.pipelineLayout = desc.pipelineLayout;
	if (m_states.computeShader.isValid())
	{
		return createProgram();
	}
	return Result::NotInitialised;
}

inline ComputeShaderProgramState ComputePipelineImpl::getShaderProgram()const
{
	PVR_ASSERT(m_states.states.size() >= 1);
	return *(static_cast<ComputeShaderProgramState*>(m_states.states[0]));
}

inline void ComputePipelineImpl::getUniformLocation(const char8** uniforms, uint32 numUniforms, int32* outLocation)
{
	for (uint32 i = 0; i < numUniforms; ++i) { outLocation[i] = getUniformLocation(uniforms[i]); }
}


}
}
}
