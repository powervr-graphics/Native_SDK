/*!
\brief The PVRVk Raytracing Pipeline class, an interface to a VkPipeline that has been
created for the VK_PIPELINE_BINDING_POINT_RAYTRACE
\file PVRVk/RaytracePipelineVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/PipelineVk.h"
#include "PVRVk/DeviceVk.h"
#include "PVRVk/PipelineConfigVk.h"
namespace pvrvk {
/// <summary>Ray tracing pipeline create parameters.</summary>
struct RaytracingPipelineCreateInfo : public PipelineCreateInfo<RaytracingPipeline>
{
public:
	/// <summary>Ray tracing stages used in this pipeline.</summary>
	std::vector<PipelineShaderStageCreateInfo> stages;

	/// <summary>Shader groups matching the ray tracing stages used in this pipeline.</summary>
	std::vector<RayTracingShaderGroupCreateInfo> shaderGroups;

	/// <summary>Maximum recursion depth used in this pipeline.</summary>
	uint32_t maxRecursionDepth;

	RaytracingPipelineCreateInfo() : PipelineCreateInfo(), maxRecursionDepth(0) {}
};

namespace impl {
/// <summary>Vulkan RaytracingPipeline wrapper<summary>
class RaytracingPipeline_ : public Pipeline<RaytracingPipeline, RaytracingPipelineCreateInfo>
{
private:
	friend class Device_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class RaytracingPipeline_;
	};

	static RaytracingPipeline constructShared(const DeviceWeakPtr& device, VkPipeline vkPipeline, const RaytracingPipelineCreateInfo& desc)
	{
		return std::make_shared<RaytracingPipeline_>(make_shared_enabler{}, device, vkPipeline, desc);
	}

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(RaytracingPipeline_)
	RaytracingPipeline_(make_shared_enabler, const DeviceWeakPtr& device, VkPipeline vkPipeline, const RaytracingPipelineCreateInfo& desc) : Pipeline(device, vkPipeline, desc) {}
	//!\endcond
};
} // namespace impl
} // namespace pvrvk
