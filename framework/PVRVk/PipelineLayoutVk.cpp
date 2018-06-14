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

PipelineLayout_::PipelineLayout_(const DeviceWeakPtr& device, const PipelineLayoutCreateInfo& createInfo)
	: DeviceObjectHandle(device), DeviceObjectDebugMarker(DebugReportObjectTypeEXT::e_PIPELINE_LAYOUT_EXT)
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
			bindings[i] = ref->getVkHandle();
			++numLayouts;
		}
		else
		{
			throw ErrorValidationFailedEXT("PipelineLayout constructor: Push constant range index must be consecutive and have valid data");
		}
	}

	pipeLayoutInfo.sType = static_cast<VkStructureType>(StructureType::e_PIPELINE_LAYOUT_CREATE_INFO);
	pipeLayoutInfo.pSetLayouts = bindings;
	pipeLayoutInfo.setLayoutCount = numLayouts;
	std::vector<VkPushConstantRange> vkPushConstantRange(createInfo.getNumPushConstantRanges());
	for (uint32_t i = 0; i < createInfo.getNumPushConstantRanges(); ++i)
	{
		if (createInfo.getPushConstantRange(i).getSize() == 0)
		{
			throw ErrorValidationFailedEXT("PipelineLayout constructor: Push constant range index must be consecutive and have valid data");
		}

		vkPushConstantRange[i].stageFlags = static_cast<VkShaderStageFlags>(createInfo.getPushConstantRange(i).getStageFlags());
		vkPushConstantRange[i].offset = createInfo.getPushConstantRange(i).getOffset();
		vkPushConstantRange[i].size = createInfo.getPushConstantRange(i).getSize();
	}
	pipeLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(vkPushConstantRange.size());
	pipeLayoutInfo.pPushConstantRanges = vkPushConstantRange.size() ? vkPushConstantRange.data() : NULL;
	vkThrowIfFailed(
		_device->getVkBindings().vkCreatePipelineLayout(_device->getVkHandle(), &pipeLayoutInfo, NULL, &_vkHandle), "PipelineLayout constructor: Failed to create pipeline layout");
}
} // namespace impl
} // namespace pvrvk
