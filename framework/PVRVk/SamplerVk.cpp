/*!
\brief Function definitions for the Sampler class.
\file PVRVk/SamplerVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/SamplerVk.h"

namespace pvrvk {
namespace impl {
Sampler_::Sampler_(const DeviceWeakPtr& device, const pvrvk::SamplerCreateInfo& samplerDesc)
	: DeviceObjectHandle(device), DeviceObjectDebugMarker(DebugReportObjectTypeEXT::e_SAMPLER_EXT)
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = static_cast<VkStructureType>(StructureType::e_SAMPLER_CREATE_INFO);
	samplerInfo.addressModeU = static_cast<VkSamplerAddressMode>(samplerDesc.wrapModeU);
	samplerInfo.addressModeV = static_cast<VkSamplerAddressMode>(samplerDesc.wrapModeV);
	samplerInfo.addressModeW = static_cast<VkSamplerAddressMode>(samplerDesc.wrapModeW);
	samplerInfo.borderColor = static_cast<VkBorderColor>(samplerDesc.borderColor);
	samplerInfo.compareEnable = samplerDesc.compareOpEnable;
	samplerInfo.compareOp = static_cast<VkCompareOp>(samplerDesc.compareOp);
	samplerInfo.magFilter = static_cast<VkFilter>(samplerDesc.magFilter);
	samplerInfo.minFilter = static_cast<VkFilter>(samplerDesc.minFilter);
	samplerInfo.maxAnisotropy = samplerDesc.anisotropyMaximum;
	samplerInfo.anisotropyEnable = samplerDesc.enableAnisotropy;
	samplerInfo.maxLod = samplerDesc.lodMaximum;
	samplerInfo.minLod = samplerDesc.lodMinimum;
	samplerInfo.mipLodBias = samplerDesc.lodBias;
	samplerInfo.mipmapMode = static_cast<VkSamplerMipmapMode>(samplerDesc.mipMapMode);
	samplerInfo.unnormalizedCoordinates = samplerDesc.unnormalizedCoordinates;
	vkThrowIfFailed(_device->getVkBindings().vkCreateSampler(_device->getVkHandle(), &samplerInfo, nullptr, &_vkHandle), "Sampler Creation failed");
}

Sampler_::~Sampler_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (_device.isValid())
		{
			_device->getVkBindings().vkDestroySampler(_device->getVkHandle(), getVkHandle(), nullptr);
			_vkHandle = VK_NULL_HANDLE;
			_device.reset();
		}
		else
		{
			reportDestroyedAfterDevice("Sampler");
		}
	}
}

} // namespace impl
} // namespace pvrvk
