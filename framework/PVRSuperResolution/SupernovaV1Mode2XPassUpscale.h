/*!
\brief SuperNova V1 Mode 2X upscale pass
\file PVRSuperResolution/SupernovaV1Mode2XPassUpscale.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include <vector>
#include "VulkanGraphicsPostProcessingPass.h"

namespace pvr {

/// <summary>SuperResolution pass to perform the Supernova V1 X1 Mode pass.</summary>
class SupernovaV1Mode2XPassUpscale : public VulkanGraphicsPostProcessingPass
{
public:
	/// <summary>Constructor.</summary>
	/// <param name="postprocessingPassOrder">Enum to know what order each specific post processing pass ocuppies (first, intermediate, last).</param>
	/// <param name="hasAlphaChannel">Whether there is alpha channel in the input texture to be converted to YUVA.</param>
	SupernovaV1Mode2XPassUpscale(PostprocessingPassOrder supernovaPassOrder, bool hasAlphaChannel);

	/// <summary>Standard destructor.</summary>
	virtual ~SupernovaV1Mode2XPassUpscale() {}

protected:
	/// <summary>Method to build pipelines used by each Supernova pass instance.</summary>
	void buildPipelines();	
};

} // namespace pvr
