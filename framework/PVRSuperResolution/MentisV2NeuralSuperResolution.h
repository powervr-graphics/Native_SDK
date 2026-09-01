/*!
\brief Post processing pass implementing Mentis v2 (Neural Super Resolution) through a compute pass
\file PVRSuperResolution/MentisV2NeuralSuperResolution.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "glm/glm.hpp"
#include "VulkanComputePostProcessingPass.h"

namespace pvr {

/// <summary>SuperResolution base postprocessing pass.</summary>
class MentisV2NeuralSuperResolution : public VulkanComputePostProcessingPass
{
public:
	/// <summary>Constructor.</summary>
	/// <param name="postprocessingPassOrder">Enum to know what order each specific post processing pass ocuppies (first, intermediate, last).</param>
	/// <param name="postProcessingMethod">Technique implemented by this instance across all available.</param>
	MentisV2NeuralSuperResolution(PostprocessingPassOrder postprocessingPassOrder, PostProcessingMethod postProcessingMethod)
		: VulkanComputePostProcessingPass(postprocessingPassOrder, PostProcessingMethod::MentisV2NeuralSuperResolution)
	{
		logMessage("\nNOTE: MentisV2NeuralSuperResolution requires in the client application the DynamicMap pointer to declare a jitter variable and update it every frame with the "
				   "jitter value used to draw the scene");
		logMessage("    Declare the variables like:");
		logMessage("        dynamicMap->setValue(\" jitterX\", 0.001234f);");
		logMessage("        dynamicMap->setValue(\" jitterY\", 0.005678f);");
		logMessage("    The values will be retrieved in the PVRSuperResolution library with:");
		logMessage("        float jitterX = _dynamicMap.getValue(\" jitterX\", -999.0f);");
		logMessage("        float jitterY = _dynamicMap.getValue(\" jitterY\", -999.0f);\n");
	}

	/// <summary>Build a descriptor pool to allocate descriptor sets from.</summary>
	void buildDescriptorPool();

	/// <summary>Command buffer to record to the SuperResolution commands for later submission.</summary>
	/// <param name="commandBuffer">Command buffer to record to.</param>
	/// <param name="commandBufferIndex">When the SuperResolution algorithm is initialized, a value is provided in 
	/// VulkanComputeInitializationData::numberCommandBuffer with the amount of command buffers that will be recorded for 
	/// the algorithm (to cover all the swapchain images / internal amount of frames an engine could have). This parameter indicates
	/// what command buffer index in the range [0, VulkanComputeInitializationData::numberCommandBuffer - 1] to record to.</param>
	void recordCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferIndex);

	/// <summary>Method called from the client to allow the library to run any per-frame updates like updating
	/// Uniform Buffer Object contents before submitting recorded commands from a specific swapchain index.</summary>
	/// <param name="swapchainIndex">Index of the swapchain used byt he current frame update.</param>
	void frameUpdate(int swapchainIndex);

	/// <summary>Standard destructor.</summary>
	virtual ~MentisV2NeuralSuperResolution() { shutdown(); }

	/// <summary>Setter for _jitter.</summary>
	void setJitter(glm::vec2 jitter) { _jitter = jitter; }

protected:
	/// <summary>Method to build a sampler used by each SuperResolution pass instance.</summary>
	void buildSampler();

	/// <summary>Method to build any buffers required.</summary>
	void buildBuffers();

	/// <summary>Method to build pipelines used by each SuperResolution pass instance.</summary>
	void buildPipelines();

	/// <summary>Pixel jitter applied by the application using this library, needs to be updated each frame.</summary>
	glm::vec2 _jitter;
};

} // namespace pvr
