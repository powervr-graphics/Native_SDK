/*!
\brief SuperNova V1 Mode 2X output pass
\file PVRSuperResolution/SupernovaV1Mode2XPassOutput.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "SupernovaV1Mode2XPassOutput.h"
#include "Log.h"

namespace pvr {

const std::string supernovaV1Mode2XOutputFragmentShaderName = "SupernovaV1Mode2XOutput.fsh.spv";

SupernovaV1Mode2XPassOutput::SupernovaV1Mode2XPassOutput(PostprocessingPassOrder postprocessingPassOrder, bool hasAlphaChannel) : 
	VulkanGraphicsPostProcessingPass(postprocessingPassOrder, PostProcessingMethod::SupernovaV1Mode2X)
{}

void SupernovaV1Mode2XPassOutput::buildPipelines()
{
	_vectorPipeline.push_back(buildPostProcessingPipeline(_renderPass, _vectorPipelineLayout[0], supernovaV1Mode2XOutputFragmentShaderName, static_cast<int>(_vectorOutputImageFormat.size())));
}

} // namespace pvr
