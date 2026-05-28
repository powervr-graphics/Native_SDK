/*!
\brief SuperNova V1 Mode 2X upscale pass
\file PVRSuperResolution/SupernovaV1Mode2XPassUpscale.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "SupernovaV1Mode2XPassUpscale.h"
#include "Log.h"

namespace pvr {

const std::string supernovaV1Mode2XFragmentShaderName = "SupernovaV1Mode2XUpscale.fsh.spv";

SupernovaV1Mode2XPassUpscale::SupernovaV1Mode2XPassUpscale(PostprocessingPassOrder postprocessingPassOrder, bool hasAlphaChannel)
	: VulkanGraphicsPostProcessingPass(postprocessingPassOrder, PostProcessingMethod::SupernovaV1Mode2X)
{
	// Y is the most intensively sampled channel, store it on a specific separate texture to minimize sampling cost

	if (hasAlphaChannel)
	{
		// For each one of the four pixels generated store:
		// + Y channel on a texture with format VK_FORMAT_R16G16B16A16_SINT
		// + U channel on a texture with format VK_FORMAT_R16G16B16A16_SINT
		// + V channel on a texture with format VK_FORMAT_R16G16B16A16_SINT
		// + A channel on a texture with format VK_FORMAT_R16G16B16A16_SINT
		_vectorOutputImageFormat = { VK_FORMAT_R16G16B16A16_SINT, VK_FORMAT_R16G16B16A16_SINT, VK_FORMAT_R16G16B16A16_SINT, VK_FORMAT_R16G16B16A16_SINT };
	}
	else
	{
		// For each one of the four pixels generated store:
		// + Y channel on a texture with format VK_FORMAT_R16G16B16A16_SINT
		// + U channel on a texture with format VK_FORMAT_R16G16B16A16_SINT
		// + V channel on a texture with format VK_FORMAT_R16G16B16A16_SINT
		_vectorOutputImageFormat = { VK_FORMAT_R16G16B16A16_SINT, VK_FORMAT_R16G16B16A16_SINT, VK_FORMAT_R16G16B16A16_SINT };
	}

	_vectorOutputImageInitialLayout = { VK_IMAGE_LAYOUT_UNDEFINED };
	_vectorOutputImageFinalLayout = { VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
}

void SupernovaV1Mode2XPassUpscale::buildPipelines()
{
	_vectorPipeline.push_back(buildPostProcessingPipeline(_renderPass, _vectorPipelineLayout[0], supernovaV1Mode2XFragmentShaderName, static_cast<int>(_vectorOutputImageFormat.size())));
}

} // namespace pvr
