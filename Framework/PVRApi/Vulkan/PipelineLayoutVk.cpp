/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/SamplerVk.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains Vulkan specific implementation of the Sampler class. Use only if directly using Vulkan calls.
			  Provides the definitions allowing to move from the Framework object Sampler to the underlying Vulkan Sampler.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/Vulkan/PipelineLayoutVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRApi/ApiObjects/DescriptorSet.h"
#include "PVRApi/Vulkan/DescriptorSetVk.h"
#include "PVRApi/Vulkan/ContextVk.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include <vector>
namespace pvr {
namespace api {
namespace impl {
const native::HPipelineLayout_& PipelineLayout_::getNativeObject() const
{
	return native_cast(*this);
}

native::HPipelineLayout_& PipelineLayout_::getNativeObject()
{
	return native_cast(*this);
}

}// namespace impl
namespace vulkan {

bool PipelineLayoutVk_::init(const PipelineLayoutCreateParam& createParam)
{
	VkPipelineLayoutCreateInfo pipeLayoutInfo;
	VkDescriptorSetLayout bindings[4] = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE };
	m_desc = createParam;
    uint32 numLayouts = 0;
    for ( pvr::uint32 i = 0;i < createParam.getNumDescSetLayouts(); ++i)
	{
		//	auto c = native_cast(*createParam.getDescriptorSetLayout(i));
		auto& ref = createParam.getDescriptorSetLayout(i);
		if (ref.isValid())
		{
            bindings[i] = native_cast(*ref); ++numLayouts;
		}
        else
        {
            Log("PipelineLayoutVk_::init Invalid descriptor set layout");
		}
	}

	pipeLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeLayoutInfo.pNext = NULL;
	pipeLayoutInfo.flags = 0;
	pipeLayoutInfo.pSetLayouts = bindings;
    pipeLayoutInfo.setLayoutCount = numLayouts;
	pipeLayoutInfo.pushConstantRangeCount = 0;
	pipeLayoutInfo.pPushConstantRanges = NULL;
	return (vk::CreatePipelineLayout(native_cast(*m_context).getDevice(), &pipeLayoutInfo, NULL, &handle) == VK_SUCCESS);
}
}
}// namespace api
}// namespace pvr

//!\endcond