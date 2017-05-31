/*!
\brief Contains Vulkan specific implementation of the Sampler class. Use only if directly using Vulkan calls. Provides
the definitions allowing to move from the Framework object Sampler to the underlying Vulkan Sampler.
\file PVRApi/Vulkan/SamplerVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRApi/Vulkan/PipelineLayoutVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRApi/ApiObjects/DescriptorSet.h"
#include "PVRApi/Vulkan/DescriptorSetVk.h"
#include "PVRApi/Vulkan/ContextVk.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include <vector>
namespace pvr {
namespace api {
namespace vulkan {

bool PipelineLayoutVk_::init(const PipelineLayoutCreateParam& createParam)
{
	VkPipelineLayoutCreateInfo pipeLayoutInfo = {};
	VkDescriptorSetLayout bindings[4] = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE };
	_desc = createParam;
	uint32 numLayouts = 0;
	for (pvr::uint32 i = 0; i < createParam.getNumDescSetLayouts(); ++i)
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
	pipeLayoutInfo.pSetLayouts = bindings;
	pipeLayoutInfo.setLayoutCount = numLayouts;
	std::vector<VkPushConstantRange> vkPushConstantRange(createParam.getNumPushConstantRange());
	for(uint32 i = 0; i < createParam.getNumPushConstantRange(); ++i)
	{
		if(createParam.getPushConstantRange(i).size == 0)
		{
			debug_assertion(false, "Push constant range index must be consecutive and have valid data");
			Log("Push constant range index is not consecutive or have invalid data");
			return false;
		}

		vkPushConstantRange[i].stageFlags = nativeVk::ConvertToVk::shaderStage(createParam.getPushConstantRange(i).stage);
		vkPushConstantRange[i].offset = createParam.getPushConstantRange(i).offset;
		vkPushConstantRange[i].size = createParam.getPushConstantRange(i).size;
	}
	pipeLayoutInfo.pushConstantRangeCount = (uint32)vkPushConstantRange.size();
	pipeLayoutInfo.pPushConstantRanges = vkPushConstantRange.size() ? vkPushConstantRange.data() : NULL;
	return (vk::CreatePipelineLayout(native_cast(*_context).getDevice(), &pipeLayoutInfo, NULL, &handle) == VK_SUCCESS);
}
}
}// namespace api
}// namespace pvr
