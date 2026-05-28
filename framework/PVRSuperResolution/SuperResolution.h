/*!
\brief Class to instantiate any SuperResolution algorithm implemented
\file PVRSuperResolution/SuperResolution.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "../include/vulkan/vulkan.h"
#include "../include/vk_bindings.h"
#include <vector>
#include <string>
#include "PostProcessingPass.h"
#include "DynamicMap.h"

namespace pvr {

/// <summary>Base class for all super resolution techniques.</summary>
class SuperResolution
{
public:
	/// <summary>Constructor.</summary>
	SuperResolution() {}

	/// <summary>Generate Vulkan resources and post processing passes needed by the chosen method.</summary>
	/// <param name="initializationData">Struct with all the information required by the method.</param>
	/// <returns>Pointer to SuperResolution::_dynamicMap so client application can set values that might be required by the algorithm
	/// from PVRSuperResolution.</returns>
	DynamicMap* init(const VulkanInitializationData& initializationData);

	/// <summary>Command buffer to record to the SuperResolution commands for later submission.</summary>
	/// <param name="commandBuffer">Command buffer to record to.</param>
	/// <param name="commandBufferIndex">When the SuperResolution algorithm is initialized, a value is provided in 
	/// VulkanInitializationData::numberCommandBuffer with the amount of command buffers that will be recorded for 
	/// the algorithm (to cover all the swapchain images / internal amount of frames an engine could have). This parameter indicates
	/// what command buffer index in the range [0, VulkanInitializationData::numberCommandBuffer - 1] to record to.</param>
	virtual void recordCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferIndex);

	/// <summary>Method called from the client to allow the library to run any per-frame updates like updating
	/// Uniform Buffer Object contents before submitting recorded commands from a specific swapchain index.</summary>
	/// <param name="swapchainIndex">Index of the swapchain used byt he current frame update.</param>
	virtual void frameUpdate(int swapchainIndex);

	/// <summary>Destroy any possible resources allocated by the chosen SuperResolution method.
	/// Important note: This method has to be called once all GPU tasks have finished, otherwise resources 
	/// still in use might be destroy.</summary>
	virtual void shutdown();

	/// <summary>Getter of PostProcessingMethod::_supernovaMethod.</summary>
	/// <returns>Enum with the current post processing method instanced.</returns>
	PostProcessingMethod getMethod();

	/// <summary>Standard destructor.</summary>
	virtual ~SuperResolution() { shutdown(); }

protected:
	/// <summary>Helper function to decide whether a specific Vulkan texture format has alpha channel.</summary>
	/// <param name="format">Format to analyse.</param>
	/// <returns>True if the provided format has alpha channrl, false otherwise.</returns>
	static bool textureHasAlphaChannel(VkFormat)
	{
		// TODO: Implement
		return false;
	}

	/// <summary>Number of command buffers that will be used to record to the commands for the chosen SuperResolution
	/// algorithm. Each time a call to recordCommands() is done, an internal counter increases and if it surpasses the
	/// value of numberCommandBuffer, an assert will be triggered.</summary>
	uint32_t _numberCommandBuffer{ 0 };

	/// <summary>Number of times the call to record command buffer for the chosen SuperResolution
	/// algorithm, calling recordCommands(), has been done. If this value surpasses _numberCommandBuffer an
	/// assert will be triggered.</summary>
	uint32_t _recordedCommandBuffer{ 0 };

	/// <summary>Vector with all the SuperNovaPass instances for the SuperResolution method currently used.</summary>
	std::vector<PostProcessingPass*> _vectorPass;

	PostProcessingMethod _postProcessingMethod{ PostProcessingMethod::PostProcessingMethodSize };

	/// <summary>Used to allow client applications to provide an heterogeneous set of values to this library in case there are
	/// shared values which change. It replaces the need to access getters / setters from specific parts of a PVRSuperResolution
	/// pass. The client application should declare a set of variables defined beforehand which each postprocessing pass in
	/// the PVRSuperResolution library will query in the call to frameUpdate (the client appliation must update any shared valued
	/// before this call for the PVRSuperResolution library to get updatd values.
	/// There are better ways to implement this, like a way to "enqueue" value updates of a specific property defined by a string
	/// which are received by the PVRSuperResolution library from the clien tinstead of sharing a common data strucutre and that
	/// could be a proper implementation in future iterations.</summary>
	DynamicMap _dynamicMap;
};

} // namespace pvr
