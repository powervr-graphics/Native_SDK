/*!
\brief Post processing pass using Vulkan API and compute
\file PVRSuperResolution/VulkanComputePostProcessingPass.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "VulkanPostProcessingPass.h"

namespace pvr {

/// <summary>SuperResolution base postprocessing pass.</summary>
class VulkanComputePostProcessingPass : public VulkanPostProcessingPass
{
public:
	/// <summary>Constructor.</summary>
	/// <param name="postprocessingPassOrder">Enum to know what order each specific post processing pass ocuppies (first, intermediate, last).</param>
	/// <param name="postProcessingMethod">Technique implemented by this instance across all available.</param>
	VulkanComputePostProcessingPass(PostprocessingPassOrder postprocessingPassOrder, PostProcessingMethod postProcessingMethod)
		: VulkanPostProcessingPass(postprocessingPassOrder, postProcessingMethod, PostProcessingAPI::PostProcessingComputeAPIVulkan)
	{}

	/// <summary>Standard destructor.</summary>
	virtual ~VulkanComputePostProcessingPass() { shutdown(); }

protected:
	/// <summary>Method to build descriptor set layouts used by each SuperResolution pass instance.</summary>
	virtual void buildDescriptorSetLayouts();
};

} // namespace pvr
