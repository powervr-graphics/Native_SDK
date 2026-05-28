/*!
\brief Base post processing class, where Vulkan API classes inherit from
\file PVRSuperResolution/PostProcessingPass.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PostProcessingInitializationData.h"

namespace pvr {

/// <summary>Abstract base class for all techniques implemented in this library.</summary>
class PostProcessingPass
{
public:
	/// <summary>Constructor.</summary>
	/// <param name="postprocessingPassOrder">Enum to know what order this instance has (first, intermediate, last).</param>
	/// <param name="postProcessingMethod">Technique implemented by this instance across all available.</param>
	/// <param name="postProcessingAPI">API and Compute / Graphics approach used by this instance.</param>
	PostProcessingPass(PostprocessingPassOrder postprocessingPassOrder, PostProcessingMethod postProcessingMethod, PostProcessingAPI postProcessingAPI)
		: 
		_postprocessingPassOrder(postprocessingPassOrder), 
		_postProcessingMethod(postProcessingMethod),
		_postProcessingAPI(postProcessingAPI)
	{}

	/// <summary>Generate resources needed by the chosen method.</summary>
	/// <param name="VulkanInitializationData">Struct with all the information required by the method.</param>
	virtual void init(const VulkanInitializationData&) {}

	/// <summary>Method called from the client to allow the library to run any per-frame updates like updating 
	/// Uniform Buffer Object contents before submitting recorded commands from a specific swapchain index.</summary>
	/// <param name="int">Index of the swapchain used byt he current frame update.</param>
	virtual void frameUpdate(int) {}

	/// <summary>Destroy any possible resources allocated by the chosen SuperResolution method.
	/// Important note: This method has to be called once all GPU tasks have finished, otherwise resources 
	/// still in use might be destroy.</summary>
	virtual void shutdown() {}

	/// <summary>Standard destructor.</summary>
	virtual ~PostProcessingPass() {}

	PostProcessingAPI getPostProcessingAPI() { return _postProcessingAPI; }

	/// <summary>Setter for PostProcessingPass::_dynamicMap</summary>
	/// <param name="dynamicMap">Value to set.</param>
	void setDynamicMap(DynamicMap* dynamicMap) { _dynamicMap = dynamicMap; }

protected:
	/// <summary>Enum to know what order each specific SuperResolution pass ocuppies (first, intermediate, last).</summary>
	PostprocessingPassOrder _postprocessingPassOrder{ PostprocessingPassOrder::Size };

	/// <summary>Enum to know the API used by a particular post processing method.</summary>
	PostProcessingAPI _postProcessingAPI{ PostProcessingAPI::PostProcessingAPISize };

	/// <summary>Enum to know what post processing method is this instance implementing.</summary>
	PostProcessingMethod _postProcessingMethod{ PostProcessingMethod::PostProcessingMethodSize };

	/// <summary>Pointer to SuperResolution::_dynamicMap. Pending to implement a better approach to share values from client application to
	/// different techniques in the PVRSupoerResolution library</summary>
	DynamicMap* _dynamicMap{ nullptr };

	/// <summary>Path to the shaders used by each SuperResolution pass.</summary>
#if defined(_WIN32)
	std::string _shaderPath{ "" };
#endif
#if defined(__ANDROID__)
	std::string _shaderPath{ "" };
#endif
#if defined(__linux__) && !defined(__ANDROID__)
	std::string _shaderPath{ "Assets_VulkanSupernova/" };
#endif
};

} // namespace pvr
