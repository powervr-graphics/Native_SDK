/*!
\brief Function definitions for the DescriptorSet class.
\file PVRVk/DescriptorSetVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/DescriptorSetVk.h"
#include "PVRVk/ImageVk.h"
#include "PVRVk/SamplerVk.h"
#include "PVRVk/BufferVk.h"

#ifdef DEBUG
#include <algorithm>
#endif

namespace pvrvk {
// DESCRIPTOR POOL
namespace impl {
pvrvk::DescriptorSet DescriptorPool_::allocateDescriptorSet(const DescriptorSetLayout& layout)
{
	DescriptorSet set;
	set.construct(layout, getReference());
	return set;
}

DescriptorPool_::DescriptorPool_(const DeviceWeakPtr& device, const DescriptorPoolCreateInfo& createInfo)
	: DeviceObjectHandle(device), DeviceObjectDebugMarker(DebugReportObjectTypeEXT::e_DESCRIPTOR_POOL_EXT)
{
	VkDescriptorPoolCreateInfo descPoolInfo;
	descPoolInfo.sType = static_cast<VkStructureType>(StructureType::e_DESCRIPTOR_POOL_CREATE_INFO);
	descPoolInfo.pNext = NULL;
	descPoolInfo.maxSets = createInfo.getMaxDescriptorSets();
	descPoolInfo.flags = static_cast<VkDescriptorPoolCreateFlags>(DescriptorPoolCreateFlags::e_FREE_DESCRIPTOR_SET_BIT);
	VkDescriptorPoolSize poolSizes[static_cast<uint32_t>(DescriptorType::e_RANGE_SIZE)];
	uint32_t poolIndex = 0;
	for (uint32_t i = 0; i < static_cast<uint32_t>(DescriptorType::e_RANGE_SIZE); ++i)
	{
		uint32_t count = createInfo.getNumDescriptorTypes(DescriptorType(i));
		if (count)
		{
			poolSizes[poolIndex].type = static_cast<VkDescriptorType>(i);
			poolSizes[poolIndex].descriptorCount = count;
			++poolIndex;
		}
	} // next type
	descPoolInfo.poolSizeCount = poolIndex;
	descPoolInfo.pPoolSizes = poolSizes;

	vkThrowIfFailed(_device->getVkBindings().vkCreateDescriptorPool(_device->getVkHandle(), &descPoolInfo, nullptr, &_vkHandle), "Create Descriptor Pool failed");
}
void DescriptorPool_::destroy()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (_device.isValid())
		{
			_device->getVkBindings().vkDestroyDescriptorPool(_device->getVkHandle(), getVkHandle(), nullptr);
			_vkHandle = VK_NULL_HANDLE;
			_device.reset();
		}
		else
		{
			reportDestroyedAfterDevice("DescriptorPool");
		}
	}
}
} // namespace impl

// DESCRIPTOR SET
namespace impl {
#ifdef DEBUG
static inline bool bindingIdPairComparison(const std::pair<uint32_t, uint32_t>& a, const std::pair<uint32_t, uint32_t>& b)
{
	return a.first < b.first;
}
#endif

DescriptorSetLayout_::~DescriptorSetLayout_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (_device.isValid())
		{
			_device->getVkBindings().vkDestroyDescriptorSetLayout(_device->getVkHandle(), getVkHandle(), nullptr);
			_vkHandle = VK_NULL_HANDLE;
			_device.reset();
		}
		else
		{
			reportDestroyedAfterDevice("DescriptorSetLayout");
		}
	}
	clearCreateInfo();
}

DescriptorSetLayout_::DescriptorSetLayout_(const DeviceWeakPtr& device, const DescriptorSetLayoutCreateInfo& createInfo)
	: DeviceObjectHandle(device), DeviceObjectDebugMarker(DebugReportObjectTypeEXT::e_DESCRIPTOR_SET_LAYOUT_EXT)
{
	_createInfo = createInfo;
	VkDescriptorSetLayoutCreateInfo vkLayoutCreateInfo = {};
	ArrayOrVector<VkDescriptorSetLayoutBinding, 4> vkBindings(getCreateInfo().getNumBindings());
	const DescriptorSetLayoutCreateInfo::DescriptorSetLayoutBinding* bindings = createInfo.getAllBindings();
	for (uint32_t i = 0; i < createInfo.getNumBindings(); ++i)
	{
		vkBindings[i].descriptorType = static_cast<VkDescriptorType>(bindings[i].descriptorType);
		vkBindings[i].binding = bindings[i].binding;
		vkBindings[i].descriptorCount = bindings[i].descriptorCount;
		vkBindings[i].stageFlags = static_cast<VkShaderStageFlags>(bindings[i].stageFlags);
		vkBindings[i].pImmutableSamplers = nullptr;
		if (bindings[i].immutableSampler.isValid())
		{
			vkBindings[i].pImmutableSamplers = &bindings[i].immutableSampler->getVkHandle();
		}
	}
	vkLayoutCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
	vkLayoutCreateInfo.bindingCount = (uint32_t)getCreateInfo().getNumBindings();
	vkLayoutCreateInfo.pBindings = vkBindings.get();
	vkThrowIfFailed(_device->getVkBindings().vkCreateDescriptorSetLayout(_device->getVkHandle(), &vkLayoutCreateInfo, nullptr, &_vkHandle), "Create Descriptor Set Layout failed");
}
} // namespace impl
} // namespace pvrvk
