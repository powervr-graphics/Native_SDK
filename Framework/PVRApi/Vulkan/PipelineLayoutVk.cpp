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
	std::vector<VkDescriptorSetLayout> bindings;

	for (pvr::uint32 i = 0; i < createParam.getNumDescSetLayouts(); ++i)
	{
		//	auto c = native_cast(*createParam.getDescriptorSetLayout(i));
		bindings.push_back(native_cast(*createParam.getDescriptorSetLayout(i)).handle);
	}

	pipeLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeLayoutInfo.pNext = NULL;
	pipeLayoutInfo.flags = 0;
	pipeLayoutInfo.pSetLayouts = bindings.data();
	pipeLayoutInfo.setLayoutCount = (uint32)bindings.size();
	pipeLayoutInfo.pushConstantRangeCount = 0;
	pipeLayoutInfo.pPushConstantRanges = NULL;
	return (vk::CreatePipelineLayout(native_cast(*m_context).getDevice(), &pipeLayoutInfo, NULL, &handle) == VK_SUCCESS);
}
}
}// namespace api
}// namespace pvr
