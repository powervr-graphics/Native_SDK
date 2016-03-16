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
class ComputePipelineImplementationDetails;

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
	\brief Destructor. Destroys all resources held by this pipeline.
	******************************************************************************************************************/
	virtual ~ComputePipeline_();

	/*!****************************************************************************************************************
	\brief Destroy this pipeline. Releases all resources held by the pipeline.
	******************************************************************************************************************/
	void destroy();

	/*!****************************************************************************************************************
	\brief Get If uniforms are supported by the underlying API, get the shader locations of several uniform variables
	at once. If a uniform does not exist or is inactive, returns -1
	\param[in] uniforms An array of uniform variable names
	\param[in] numUniforms The number of uniforms in the array
	\param[out] outLocation An array where the locations will be saved. Writes -1 for inactive uniforms.
	******************************************************************************************************************/
	void getUniformLocation(const char8** uniforms, uint32 numUniforms, int32* outLocation)
	{
		for (uint32 i = 0; i < numUniforms; ++i) { outLocation[i] = getUniformLocation(uniforms[i]); }
	}

	/*!****************************************************************************************************************
	\brief Get If uniforms are supported by the underlying API, get the shader location of a uniform variables. If a
	uniform does not exist or is inactive, return -1
	\param uniform The name of a shader uniform variable name
	\return The location of the uniform, -1 if not found/inactive.
	******************************************************************************************************************/
	int32 getUniformLocation(const char8* uniform);

	/*!****************************************************************************************************************
	\brief Return pipeline layout.
	\return const PipelineLayout&
	******************************************************************************************************************/
	const PipelineLayout& getPipelineLayout()const;

	const native::HPipeline_& getNativeObject() const;
	native::HPipeline_& getNativeObject();
protected:
	Result::Enum init(const ComputePipelineCreateParam& desc);

	ComputePipeline_(GraphicsContext& device);
	std::auto_ptr<ComputePipelineImplementationDetails> pimpl;
	void bind(IGraphicsContext& context);
};
}
inline native::HPipeline_& native_cast(pvr::api::impl::ComputePipeline_& object) { return object.getNativeObject(); }
inline const native::HPipeline_& native_cast(const pvr::api::impl::ComputePipeline_& object) { return object.getNativeObject(); }
}
}
