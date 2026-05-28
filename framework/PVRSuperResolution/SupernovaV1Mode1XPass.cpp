/*!
\brief SuperNova V1 Mode 1X pass
\file PVRSuperResolution/SupernovaV1Mode1XPass.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "SupernovaV1Mode1XPass.h"
#include "Log.h"

namespace pvr {

const std::string supernovaV1Mode1XFragmentShaderName = "SupernovaV1Mode1X.fsh.spv";

void SupernovaV1Mode1XPass::buildPipelines()
{
	_vectorPipeline.push_back(buildPostProcessingPipeline(_renderPass, _vectorPipelineLayout[0], supernovaV1Mode1XFragmentShaderName, static_cast<int>(_vectorOutputImageFormat.size())));
}

} // namespace pvr
