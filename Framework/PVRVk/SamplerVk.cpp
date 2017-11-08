/*!
\brief Function definitions for the Sampler class.
\file PVRVk/SamplerVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/SamplerVk.h"

namespace pvrvk {
namespace impl {
bool Sampler_::init(const pvrvk::SamplerCreateInfo& samplerDesc)
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VkStructureType::e_SAMPLER_CREATE_INFO;
	samplerInfo.addressModeU = (samplerDesc.wrapModeU);
	samplerInfo.addressModeV = (samplerDesc.wrapModeV);
	samplerInfo.addressModeW = (samplerDesc.wrapModeW);
	samplerInfo.borderColor = (samplerDesc.borderColor);
	samplerInfo.compareEnable = samplerDesc.compareOpEnable;
	samplerInfo.compareOp = (samplerDesc.compareOp);
	samplerInfo.magFilter = (samplerDesc.magFilter);
	samplerInfo.minFilter = (samplerDesc.minFilter);
	samplerInfo.maxAnisotropy = samplerDesc.anisotropyMaximum;
	samplerInfo.anisotropyEnable = samplerDesc.enableAnisotropy;
	samplerInfo.maxLod = samplerDesc.lodMaximum;
	samplerInfo.minLod = samplerDesc.lodMinimum;
	samplerInfo.mipLodBias = samplerDesc.lodBias;
	samplerInfo.mipmapMode = (samplerDesc.mipMapMode);
	samplerInfo.unnormalizedCoordinates = samplerDesc.unnormalizedCoordinates;
	return vkIsSuccessful(vk::CreateSampler(_device->getNativeObject(), &samplerInfo,
	                                        nullptr, &_vkSampler), "Sampler creation failed");
}
}// namespace impl
}// namespace pvrvk
