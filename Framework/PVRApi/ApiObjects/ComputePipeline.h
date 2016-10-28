/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\ComputePipeline.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the Compute Pipeline.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/ApiObjects/PipelineLayout.h"
#include "PVRApi/ApiObjects/PipelineConfig.h"
#include <vector>

namespace pvr {
namespace api {
/*!****************************************************************************************************************
\brief Compute pipeline create parameters.
*******************************************************************************************************************/
struct ComputePipelineCreateParam
{
public:
	pipelineCreation::ComputeShaderStageCreateParam computeShader;	//!< Compute shader information
	PipelineLayout pipelineLayout;									//!< Compute pipeline information
};

namespace impl {

class ComputePipelineImplBase
{
public:
	/*!
	   \brief ~ComputePipelineImplBase
	 */
	virtual ~ComputePipelineImplBase() {}

	/*!
	   \brief getUniformLocation
	   \param uniforms
	   \param numUniforms
	   \param outLocation
	 */
	virtual void getUniformLocation(const char8** uniforms, uint32 numUniforms, int32* outLocation) = 0;

	/*!
	   \brief getUniformLocation
	   \param uniform
	   \return
	 */
	virtual int32 getUniformLocation(const char8* uniform) = 0;

	/*!
	   \brief getPipelineLayout
	   \return
	 */
	virtual const PipelineLayout& getPipelineLayout()const = 0;

	/*!
	   \brief getNativeObject
	   \return
	 */
	virtual const native::HPipeline_& getNativeObject() const = 0;

	/*!
	   \brief getNativeObject
	   \return
	 */
	virtual native::HPipeline_& getNativeObject() = 0;

	/*!
	   \brief destroy
	 */
	virtual void destroy() = 0;

	/*!
	   \brief bind
	 */
	virtual void bind() = 0;
};

	/*!*********************************************************************************************************************
\brief A configuration of the Compute Pipeline which must be bound before launching a Compute operation.
	***********************************************************************************************************************/
class ComputePipeline_
	{
protected:
	friend class PopPipeline;
	friend class CommandBufferBase_;
	friend class ComputePipelineImplementationDetails;
	template<typename> friend class PackagedBindable;
	template<typename> friend struct ::pvr::RefCountEntryIntrusive;
	friend class ::pvr::IGraphicsContext;
public:

	/*!****************************************************************************************************************
	\brief Destroy this pipeline. Releases all resources held by the pipeline.
	******************************************************************************************************************/
	void destroy() { pimpl->destroy(); }

	/*!****************************************************************************************************************
	\brief Get If uniforms are supported by the underlying API, get the shader locations of several uniform variables
	at once. If a uniform does not exist or is inactive, returns -1
	\param[in] uniforms An array of uniform variable names
	\param[in] numUniforms The number of uniforms in the array
	\param[out] outLocation An array where the locations will be saved. Writes -1 for inactive uniforms.
	    ******************************************************************************************************************/
	void getUniformLocation(const char8** uniforms, uint32 numUniforms, int32* outLocation)
{
		pimpl->getUniformLocation(uniforms, numUniforms, outLocation);
	}

	/*!****************************************************************************************************************
	\brief Get If uniforms are supported by the underlying API, get the shader location of a uniform variables. If a
	uniform does not exist or is inactive, return -1
	\param uniform The name of a shader uniform variable name
	\return The location of the uniform, -1 if not found/inactive.
	******************************************************************************************************************/
	int32 getUniformLocation(const char8* uniform)
{
		return pimpl->getUniformLocation(uniform);
	}

	/*!****************************************************************************************************************
	\brief Return pipeline layout.
	\return const PipelineLayout&
	******************************************************************************************************************/
	const PipelineLayout& getPipelineLayout()const { return pimpl->getPipelineLayout(); }

	/*!
	\brief Return a handle to the native obejct (const)
	\return
	 */
	const native::HPipeline_& getNativeObject() const { return pimpl->getNativeObject(); }

	/*!
	\brief Return a handle to the native obejct
	 */
	native::HPipeline_& getNativeObject() { return pimpl->getNativeObject(); }

	// INTERNAL USE ONLY
	ComputePipelineImplBase& getImpl() { return *pimpl; }
protected:
	ComputePipeline_(std::auto_ptr<ComputePipelineImplBase>& pimpl) : pimpl(pimpl) {}
	std::auto_ptr<ComputePipelineImplBase> pimpl;
	void bind(IGraphicsContext&) { pimpl->bind(); }
};
}
inline native::HPipeline_& native_cast(pvr::api::impl::ComputePipeline_& object) { return object.getNativeObject(); }
inline const native::HPipeline_& native_cast(const pvr::api::impl::ComputePipeline_& object) { return object.getNativeObject(); }
}
}
