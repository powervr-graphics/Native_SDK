/*!
\brief Contains the Compute Pipeline.
\file PVRApi/ApiObjects/ComputePipeline.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiObjects/PipelineLayout.h"
#include "PVRApi/ApiObjects/PipelineConfig.h"
#include <vector>

namespace pvr {
namespace api {
/// <summary>Compute pipeline create parameters.</summary>
struct ComputePipelineCreateParam
{
public:
	pipelineCreation::ComputeShaderStageCreateParam computeShader;  //!< Compute shader information
	PipelineLayout pipelineLayout; //!< Compute pipeline information
};
//!\cond NO_DOXYGEN
namespace impl {
class ComputePipelineImplBase
{
	friend class ComputePipeline_;
public:
	virtual ~ComputePipelineImplBase() {}

	virtual void getUniformLocation(const char8** uniforms, uint32 numUniforms, int32* outLocation)const = 0;

	virtual int32 getUniformLocation(const char8* uniform)const = 0;

	virtual const PipelineLayout& getPipelineLayout()const = 0;

	virtual const ComputePipelineCreateParam& getCreateParam()const = 0;
};
//!\endcond

/// <summary>A configuration of the Compute Pipeline which must be bound before launching a Compute operation.
/// </summary>
class ComputePipeline_
{
protected:
	friend class PopPipeline;
	friend class CommandBufferBase_;
	template<typename> friend class PackagedBindable;
	template<typename> friend struct ::pvr::RefCountEntryIntrusive;
	friend class ::pvr::IGraphicsContext;
public:
	virtual ~ComputePipeline_() { }

	/// <summary>Get If uniforms are supported by the underlying API, get the shader locations of several uniform
	/// variables at once. If a uniform does not exist or is inactive, returns -1</summary>
	/// <param name="uniforms">An array of uniform variable names</param>
	/// <param name="numUniforms">The number of uniforms in the array</param>
	/// <param name="outLocation">An array where the locations will be saved. Writes -1 for inactive uniforms.
	/// </param>
	void getUniformLocation(const char8** uniforms, uint32 numUniforms, int32* outLocation)
	{
		pimpl->getUniformLocation(uniforms, numUniforms, outLocation);
	}

	/// <summary>Get If uniforms are supported by the underlying API, get the shader location of a uniform variables.
	/// If a uniform does not exist or is inactive, return -1</summary>
	/// <param name="uniform">The name of a shader uniform variable name</param>
	/// <returns>The location of the uniform, -1 if not found/inactive.</returns>
	int32 getUniformLocation(const char8* uniform)
	{
		return pimpl->getUniformLocation(uniform);
	}

	/// <summary>Return pipeline layout.</summary>
	/// <returns>const PipelineLayout&</returns>
	const PipelineLayout& getPipelineLayout()const
	{
		return pimpl->getPipelineLayout();
	}

	/// <summary>return pipeline create param used to create the child pipeline</summary>
	/// <returns>const ComputePipelineCreateParam&</returns>
	const ComputePipelineCreateParam& getCreateParam()const { return pimpl->getCreateParam(); }

	// INTERNAL USE ONLY
	const ComputePipelineImplBase& getImpl()const { return *pimpl; }
	ComputePipelineImplBase& getImpl() { return *pimpl; }
protected:
	ComputePipeline_(std::auto_ptr<ComputePipelineImplBase>& pimpl) : pimpl(pimpl) {}
	std::auto_ptr<ComputePipelineImplBase> pimpl;
};
}
}
}