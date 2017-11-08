/*!
\brief Function definitions for the Pipeline Layout class
\file PVRVk/SamplerVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/PipelineLayoutVk.h"
#include "PVRVk/DescriptorSetVk.h"
#include "PVRVk/DeviceVk.h"
#include <vector>
namespace pvrvk {
namespace impl {

bool PipelineLayout_::init(const PipelineLayoutCreateInfo& createInfo)
{
	VkPipelineLayoutCreateInfo pipeLayoutInfo = {};
	VkDescriptorSetLayout bindings[4] = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE };
	_createInfo = createInfo;
	uint32_t numLayouts = 0;
	for (uint32_t i = 0; i < createInfo.getNumDescriptorSetLayouts(); ++i)
	{
		auto& ref = createInfo.getDescriptorSetLayout(i);
		if (ref.isValid())
		{
			bindings[i] = ref->getNativeObject(); ++numLayouts;
		}
		else
		{
			Log("PipelineLayoutVk_::init Invalid descriptor set layout");
		}
	}

	pipeLayoutInfo.sType = VkStructureType::e_PIPELINE_LAYOUT_CREATE_INFO;
	pipeLayoutInfo.pSetLayouts = bindings;
	pipeLayoutInfo.setLayoutCount = numLayouts;
	std::vector<VkPushConstantRange> vkPushConstantRange(createInfo.getNumPushConstantRanges());
	for (uint32_t i = 0; i < createInfo.getNumPushConstantRanges(); ++i)
	{
		if (createInfo.getPushConstantRange(i).size == 0)
		{
			debug_assertion(false, "Push constant range index must be consecutive and have valid data");
			Log("Push constant range index is not consecutive or have invalid data");
			return false;
		}

		vkPushConstantRange[i].stageFlags = createInfo.getPushConstantRange(i).stage;
		vkPushConstantRange[i].offset = createInfo.getPushConstantRange(i).offset;
		vkPushConstantRange[i].size = createInfo.getPushConstantRange(i).size;
	}
	pipeLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(vkPushConstantRange.size());
	pipeLayoutInfo.pPushConstantRanges = vkPushConstantRange.size() ? vkPushConstantRange.data() : NULL;
	return (vk::CreatePipelineLayout(_device->getNativeObject(), &pipeLayoutInfo, NULL, &_vkPipeLayout) == VkResult::e_SUCCESS);
}
}
}// namespace pvrvk
