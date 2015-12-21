/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\ComputeStateCreateParam.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Internal file for the Compute Pipeline.
***********************************************************************************************************************/
#pragma once
namespace pvr {
namespace api {
namespace impl {
class ComputePipelineImpl;
class ComputePipelineImplState;
}

/*!*********************************************************************************************************************
\brief  INTERNAL Contains all the states for compute pipeline.
***********************************************************************************************************************/
struct ComputeStateContainer
{
public:
/*! \cond NO_DOXYGEN*/
	typedef std::vector<impl::ComputePipelineImplState*>StateContainer;

	pvr::api::Shader computeShader;
	StateContainer states;
	PipelineLayout pipelineLayout;
	bool hasComputeShader()const { return computeShader.isValid(); }
/*! \endcond*/
};

/*!*********************************************************************************************************************
\brief Contains structs required to set different parameters of the GraphicsPipeline and ComputePipeline objects
***********************************************************************************************************************/
namespace pipelineCreation {

/*!*********************************************************************************************************************
\brief Computer shader stage creator.
***********************************************************************************************************************/
struct ComputeShaderStageCreateParam
{
	friend class ::pvr::api::impl::ComputePipelineImpl;
	friend class ::pvr::api::impl::ComputePipelineImplState;
	friend struct ::pvr::api::ComputePipelineCreateParam;
public:
	/*!*********************************************************************************************************************
	\brief Set the compute shader object
	\param[in] shader The compute shader object.
	***********************************************************************************************************************/
	void setShader(const pvr::api::Shader& shader) { m_shader = shader; }

	/*!*********************************************************************************************************************
	\brief Return if it has valid compute shader.
	\return true If the compute shader exists, false otherwise.
	***********************************************************************************************************************/
	bool hasComputeShader() { return m_shader.isValid(); }
private:
	void createStateObjects(ComputeStateContainer& state, ComputeShaderStageCreateParam* parent_state = NULL)const;
	pvr::api::Shader m_shader;
};
}
}// api
}// pvr
