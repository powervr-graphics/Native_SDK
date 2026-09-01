/*!
\brief Class to instantiate any SuperResolution algorithm implemented
\file PVRSuperResolution/PVRSuperResolution.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include <vector>
#include <string>
#include "SuperResolution.h"
#include "VulkanGraphicsPostProcessingPass.h"
#include "VulkanComputePostProcessingPass.h"
#include "SupernovaYUVAConversionPass.h"
#include "SupernovaV1Mode1XPass.h"
#include "SupernovaV1Mode2XPassUpscale.h"
#include "SupernovaV1Mode2XPassOutput.h"
#include "MentisV2NeuralSuperResolution.h"
#include "Log.h"
#include "FileIO.h"

namespace pvr {

DynamicMap* SuperResolution::init(const VulkanInitializationData& initializationData)
{
	_postProcessingMethod = initializationData.postProcessingMethod;

	bool useAlphaChannel = textureHasAlphaChannel(initializationData.inputImageFormat[0]) && (initializationData.outputImageFormat);

	switch (_postProcessingMethod)
	{
	case PostProcessingMethod::SupernovaV1Mode1X:
	{
		// Add a RGBA -> YUVA conversion pass
		SupernovaYUVAConversionPass* supernovaYUVAConversionPass = new SupernovaYUVAConversionPass(PostprocessingPassOrder::FirstPass, useAlphaChannel);
		supernovaYUVAConversionPass->init(initializationData);

		// Supernova v1 x1 Mode pass
		SupernovaV1Mode1XPass* supernovaV1Mode1XPass = new SupernovaV1Mode1XPass(PostprocessingPassOrder::LastPass, useAlphaChannel);
		// The input of SupernovaV1Mode1XPass is the output of the previous pass, SupernovaYUVAConversionPass
		supernovaV1Mode1XPass->setVectorVectorInputImageView(supernovaYUVAConversionPass->refVectorVectorOutputImageView());
		supernovaV1Mode1XPass->init(initializationData);

		_vectorPass.push_back(supernovaYUVAConversionPass);
		_vectorPass.push_back(supernovaV1Mode1XPass);

		break;
	}
	case PostProcessingMethod::SupernovaV1Mode2X:
	{
		// Add a RGBA -> YUVA conversion pass
		SupernovaYUVAConversionPass* supernovaYUVAConversionPass = new SupernovaYUVAConversionPass(PostprocessingPassOrder::FirstPass, useAlphaChannel);
		supernovaYUVAConversionPass->init(initializationData);

		// Supernova v1 2X Mode upscale pass
		SupernovaV1Mode2XPassUpscale* supernovaV1Mode2XPassUpscale = new SupernovaV1Mode2XPassUpscale(PostprocessingPassOrder::IntermediatePass, useAlphaChannel);
		// The input of SupernovaV1Mode2XPassUpscale is the output of the previous pass, SupernovaYUVAConversionPass
		supernovaV1Mode2XPassUpscale->setVectorVectorInputImageView(supernovaYUVAConversionPass->refVectorVectorOutputImageView());
		supernovaV1Mode2XPassUpscale->init(initializationData);

		// Supernova v1 2X Mode output pass
		SupernovaV1Mode2XPassOutput* supernovaV1Mode2XPassOutput = new SupernovaV1Mode2XPassOutput(PostprocessingPassOrder::LastPass, useAlphaChannel);
		// The input of SupernovaV1Mode2XPassOutput is the output of the previous pass, SupernovaV1Mode2XPassUpscale
		supernovaV1Mode2XPassOutput->setVectorVectorInputImageView(supernovaV1Mode2XPassUpscale->refVectorVectorOutputImageView());
		supernovaV1Mode2XPassOutput->init(initializationData);

		_vectorPass.push_back(supernovaYUVAConversionPass);
		_vectorPass.push_back(supernovaV1Mode2XPassUpscale);
		_vectorPass.push_back(supernovaV1Mode2XPassOutput);

		break;
	}
	case PostProcessingMethod::MentisV2NeuralSuperResolution: {
		// Mentis spatial upscaler
		VulkanComputePostProcessingPass* vulkanComputePass = new MentisV2NeuralSuperResolution(PostprocessingPassOrder::SinglePass, PostProcessingMethod::MentisV2NeuralSuperResolution);
		vulkanComputePass->setDynamicMap(&_dynamicMap);
		vulkanComputePass->init(initializationData);

		_vectorPass.push_back(vulkanComputePass);
		break;
	}
	default:
	{
		assertCondition(false, "ERROR: Not recognized SuperResolution method");
		break;
	}
	}

	return &_dynamicMap;
}

void SuperResolution::recordCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferIndex)
{
	for (size_t i = 0; i < _vectorPass.size(); ++i)
	{
		if ((_vectorPass[i]->getPostProcessingAPI() != PostProcessingAPI::PostProcessingGraphicsAPIVulkan) &&
			(_vectorPass[i]->getPostProcessingAPI() != PostProcessingAPI::PostProcessingComputeAPIVulkan))
		{
			assertCondition(false, "ERROR: Using a non-Vulkan API pass");
		}
		
		VulkanPostProcessingPass* pass = static_cast<VulkanPostProcessingPass*>(_vectorPass[i]);
		pass->recordCommands(commandBuffer, commandBufferIndex);
	}
}

void SuperResolution::frameUpdate(int swapchainIndex)
{
	for (size_t i = 0; i < _vectorPass.size(); ++i)
	{
		if ((_vectorPass[i]->getPostProcessingAPI() != PostProcessingAPI::PostProcessingGraphicsAPIVulkan) && 
			(_vectorPass[i]->getPostProcessingAPI() != PostProcessingAPI::PostProcessingComputeAPIVulkan))
		{
			assertCondition(false, "ERROR: Using a non-Vulkan API pass");
		}

		_vectorPass[i]->frameUpdate(swapchainIndex);
	}
}

void SuperResolution::shutdown()
{
	for (size_t i = 0; i < _vectorPass.size(); ++i)
	{
		delete _vectorPass[i];
		_vectorPass[i] = nullptr;
	}
}

PostProcessingMethod SuperResolution::getMethod() {
	return _postProcessingMethod;
}

} // namespace pvr
