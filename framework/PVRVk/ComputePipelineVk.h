/*!
\brief The PVRVk Compute Pipeline class, an interface to a VkPipeline that has been
created for the VK_PIPELINE_BINDING_POINT_COMPUTE
\file PVRVk/ComputePipelineVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/PipelineVk.h"
#include "PVRVk/DeviceVk.h"
#include "PVRVk/PipelineConfigVk.h"
namespace pvrvk {
/// <summary>Compute pipeline create parameters.</summary>
struct ComputePipelineCreateInfo : public PipelineCreateInfo<ComputePipeline>
{
public:
	PipelineShaderStageCreateInfo computeShader; //!< Compute shader information

	ComputePipelineCreateInfo() : PipelineCreateInfo() {}
};

namespace impl {
/// <summary> Vulkan Computepipeline wrapper<summary>
class ComputePipeline_ : public Pipeline<ComputePipeline, ComputePipelineCreateInfo>
{
public:
	DECLARE_NO_COPY_SEMANTICS(ComputePipeline_)

private:
	template<typename>
	friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;

	ComputePipeline_(DeviceWeakPtr device, VkPipeline vkPipeline, const ComputePipelineCreateInfo& desc) : Pipeline(device, vkPipeline, desc) {}
};
} // namespace impl
} // namespace pvrvk
