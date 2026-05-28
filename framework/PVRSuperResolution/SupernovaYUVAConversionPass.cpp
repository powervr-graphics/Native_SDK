/*!
\brief Postprocess pass to convert from RGBA to YUVA
\file PVRSuperResolution/SupernovaYUVAConversionPass.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "SupernovaYUVAConversionPass.h"
#include "Log.h"

const std::string RGBToYUVFragmentShaderName = "RGBToYUV.fsh.spv";

namespace pvr {

SupernovaYUVAConversionPass::SupernovaYUVAConversionPass(PostprocessingPassOrder postprocessingPassOrder, bool hasAlphaChannel)
	: VulkanGraphicsPostProcessingPass(postprocessingPassOrder, PostProcessingMethod::YUVAColorConversion)
{
	// Y is the most intensively sampled channel, store it on a specific separate texture to minimize sampling cost

	if (hasAlphaChannel)
	{
		// Store Y on a texture with format VK_FORMAT_R16_SINT, UV on a texture with format VK_FORMAT_R16G16_SINT
		// and alpha on a texture with format VK_FORMAT_R16_SINT (UVA are sampled only once. As VK_FORMAT_R16G16B16_SINT 
		// might not be supported to be used as color attachment, store alpha on a separate texture)
		_vectorOutputImageFormat = { VK_FORMAT_R16_SINT, VK_FORMAT_R16G16_SINT, VK_FORMAT_R16_SINT };
	}
	else
	{
		// Store Y on a separate texture with format VK_FORMAT_R16_SINT and UV on a separate texture with format VK_FORMAT_R16G16_SINT
		_vectorOutputImageFormat = { VK_FORMAT_R16_SINT, VK_FORMAT_R16G16_SINT };
	}
	
	_vectorOutputImageInitialLayout = { VK_IMAGE_LAYOUT_UNDEFINED };
	_vectorOutputImageFinalLayout = { VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
}

void SupernovaYUVAConversionPass::buildPipelines()
{
	_vectorPipeline.push_back(buildPostProcessingPipeline(_renderPass, _vectorPipelineLayout[0], RGBToYUVFragmentShaderName, static_cast<int>(_vectorOutputImageFormat.size())));
}

} // namespace pvr
