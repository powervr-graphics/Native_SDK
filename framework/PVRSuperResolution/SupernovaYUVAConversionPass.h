/*!
\brief Postprocess pass to convert from RGBA to YUVA
\file PVRSuperResolution/SupernovaYUVAConversionPass.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "VulkanGraphicsPostProcessingPass.h"

namespace pvr {

/// <summary>SuperResolution pass to convert image inputs from RGBA to YUVA.</summary>
class SupernovaYUVAConversionPass : public VulkanGraphicsPostProcessingPass
{
public:
	/// <summary>Constructor.</summary>
	/// <param name="postprocessingPassOrder">Enum to know what order each specific post processing pass ocuppies (first, intermediate, last).</param>
	/// <param name="hasAlphaChannel">Whether there is alpha channel in the input texture to be converted to YUVA.</param>
	SupernovaYUVAConversionPass(PostprocessingPassOrder postprocessingPassOrder, bool hasAlphaChannel);

	/// <summary>Standard destructor.</summary>
	virtual ~SupernovaYUVAConversionPass(){}

protected:
	/// <summary>Method to build pipelines used by each Supernova pass instance.</summary>
	void buildPipelines();
};

} // namespace pvr
