/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/SamplerVk.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Vulkan implementation of the Sampler class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/Vulkan/SamplerVk.h"
#include "PVRApi/ApiIncludes.h"
#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"
#include "PVRNativeApi/ApiErrors.h"
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRApi/Vulkan/ContextVk.h"
namespace pvr {
namespace api {
namespace impl {
native::HSampler_& Sampler_::getNativeObject()
{
	return native_cast(*this);
}

const native::HSampler_& Sampler_::getNativeObject() const
{
	return native_cast(*this);
}
}// namespace impl

namespace vulkan {
void SamplerVk_::destroy()
{
	VkSampler& s = native_cast(*this).handle;
	if (getContext().isValid())
	{
		if (s != VK_NULL_HANDLE)
		{
			vk::DestroySampler(native_cast(*getContext()).getDevice(), getNativeObject(), NULL);
			s = VK_NULL_HANDLE;
		}
	}
}



bool SamplerVk_::init(const api::SamplerCreateParam& samplerDesc)
{
	VkSamplerCreateInfo samplerInfo;
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.pNext = NULL;
	samplerInfo.addressModeU = ConvertToVk::samplerWrap(samplerDesc.wrapModeU);
	samplerInfo.addressModeV = ConvertToVk::samplerWrap(samplerDesc.wrapModeV);
	samplerInfo.addressModeW = ConvertToVk::samplerWrap(samplerDesc.wrapModeW);
	samplerInfo.borderColor = ConvertToVk::borderColor(samplerDesc.borderColor);
	samplerInfo.compareEnable = (samplerDesc.compareMode != types::ComparisonMode::None);
	samplerInfo.compareOp = ConvertToVk::compareMode(pvr::types::ComparisonMode::Enum(samplerDesc.compareMode % 8));
	samplerInfo.flags = 0;
	samplerInfo.magFilter = ConvertToVk::samplerFilter(samplerDesc.magnificationFilter);
	samplerInfo.minFilter = ConvertToVk::samplerFilter(samplerDesc.minificationFilter);
	samplerInfo.maxAnisotropy = samplerDesc.anisotropyMaximum;
	samplerInfo.anisotropyEnable = (samplerDesc.anisotropyMaximum > 0);
	samplerInfo.maxLod = samplerDesc.lodMaximum;
	samplerInfo.minLod = samplerDesc.lodMinimum;
	samplerInfo.mipLodBias = samplerDesc.lodBias;
	samplerInfo.mipmapMode = ConvertToVk::mipmapFilter(samplerDesc.mipMappingFilter);
	samplerInfo.unnormalizedCoordinates = samplerDesc.unnormalizedCoordinates;
	return vkIsSuccessful(vk::CreateSampler(native_cast(*m_context).getDevice(), &samplerInfo, NULL, &native_cast(this)->handle), "Sampler creation failed");
}
}// namespace vulkan
}// namespace api
}// namespace pvr
//!\endcond
