/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\PipelineState.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Internal use. Contains objects used by the Graphics Pipelines.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/ApiObjects/PipelineStateCreateParam.h"

namespace pvr {
namespace api {
namespace impl {
/*!**********************************************************************************
\brief Base interface for a pipeline state object.
************************************************************************************/
class PipelineState
{
	friend class ::pvr::api::impl::GraphicsPipelineImpl;
	friend class ::pvr::api::impl::ComputePipelineImpl;
	friend struct ::pvr::api::ComputePipelineCreateParam;
	friend struct ::pvr::api::GraphicsPipelineCreateParam;
public:
	typedef PipelineState* ptr_type;
	virtual ptr_type createClone() const = 0;
	virtual ptr_type createDefault() const = 0;
	static void destroyClone(ptr_type clone)
	{
		delete clone;
	}
	PipelineState() : m_parent(NULL) { }

	/*!**********************************************************************************
	\brief Set default state.
	************************************************************************************/
	virtual void setDefault(pvr::IGraphicsContext& device) = 0;

	/*!**********************************************************************************
	\brief Destructor.
	************************************************************************************/
	virtual ~PipelineState() {}

	/*!**********************************************************************************
	\brief Set this state.
	************************************************************************************/
	virtual void set(pvr::IGraphicsContext& device) = 0;

	/*!**********************************************************************************
	\brief Unset this state.
	************************************************************************************/
	virtual void unset(pvr::IGraphicsContext& device) = 0;

	/*!**********************************************************************************
	\brief Reset this state.
	************************************************************************************/
	virtual void reset(pvr::IGraphicsContext& device) = 0;

	/*!******************************************************************************
	\brief	Type of this state.
	\return	void
	********************************************************************************/
	virtual GraphicsStateType::Enum getStateType() const = 0;

	/*!******************************************************************************
	\brief	Check is a valid state.
	\return	bool
	********************************************************************************/
	virtual bool isValid() { return m_isValid; }
protected:
	PipelineState*  m_parent;
	bool			m_isValid;
};

/*!******************************************************************************
\brief Base class for graphics pipeline state.
********************************************************************************/
class GraphicsPipelineImplState : public PipelineState
{
public:
	virtual ~GraphicsPipelineImplState() {}
protected:
};

/*!******************************************************************************
\brief Base class for compute pipeline states.
********************************************************************************/
class ComputePipelineImplState : public PipelineState
{
public:
	virtual ~ComputePipelineImplState() {}
};
}
}
}//namespace pvr
