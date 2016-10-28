/*!*********************************************************************************************************************
\file         PVRApi\Vulkan\DescriptorSetVk.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Vulkan implementation of the DescriptorSet class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/Vulkan/DescriptorSetVk.h"
#include "PVRApi/Vulkan/TextureVk.h"
#include "PVRApi/Vulkan/SamplerVk.h"
#include "PVRApi/Vulkan/BufferVk.h"

#ifdef DEBUG
#include <algorithm>
#endif

namespace pvr {
namespace api {
namespace impl {
api::DescriptorSet DescriptorPool_::allocateDescriptorSet(const DescriptorSetLayout& layout)
{
	DescriptorPool this_ref = static_cast<vulkan::DescriptorPoolVk_*>(this)->getReference();

	api::vulkan::DescriptorSetVk set;
	set.construct(layout, this_ref);
	if (!set->init())
	{
		set.reset();
	}
	return set;
}
bool DescriptorSet_::update(const DescriptorSetUpdate& descSet)
{
	return native_cast(*this).update(descSet);
}

const native::HDescriptorSet_& impl::DescriptorSet_::getNativeObject() const
{
	return native_cast(*this);
}

native::HDescriptorSet_& impl::DescriptorSet_::getNativeObject()
{
	return native_cast(*this);
}

const native::HDescriptorPool_& DescriptorPool_::getNativeObject()const
{
	return native_cast(*this);
}

native::HDescriptorPool_& DescriptorPool_::getNativeObject()
{
	return native_cast(*this);
}

}

namespace vulkan {

void DescriptorSetVk_::destroy()
{
	auto& vkobj = native_cast(*this);
	if (vkobj.handle != VK_NULL_HANDLE)
	{
		if (m_descPool->getContext().isValid())
		{
			vk::FreeDescriptorSets(native_cast(*getContext()).getDevice(), native_cast(m_descPool)->handle, 1, &native_cast(*this).handle);
		}
		vkobj.handle = VK_NULL_HANDLE;
		m_descPool.reset();
		m_descSetLayout.reset();
	}
}

void DescriptorSetLayoutVk_::destroy()
{
	if (device.isValid())
	{
		if (handle != VK_NULL_HANDLE)
		{
			vk::DestroyDescriptorSetLayout(native_cast(*getContext()).getDevice(), native_cast(*this), NULL);
		}
		device.reset();
	}
	desc.clear();
}

DescriptorSetVk_::~DescriptorSetVk_()
{
	if (m_descPool.isValid())
	{
		if (m_descPool->getContext().isValid())
		{
			destroy();
		}
		else
		{
			Log(Log.Warning, "Attempted to free DescriptorSet after its corresponding device was destroyed");
		}
	}
	else
	{
		Log(Log.Warning, "Attempted to free DescriptorSet after its corresponding pool was destroyed");
	}
}

DescriptorSetLayoutVk_::~DescriptorSetLayoutVk_()
{
	if (device.isValid())
	{
		destroy();
	}
	else
	{
		Log(Log.Warning, "Attempted to free DescriptorSetLayout after its corresponding device was destroyed");
	}
}

// initialize a VkWriteDescriptorSet
static inline VkWriteDescriptorSet initializeWriteDescSet()
{
	VkWriteDescriptorSet descSet;
	memset(&descSet, 0, sizeof(descSet));
	descSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descSet.pNext = NULL;

	return descSet;
}

// Fill in an ImageInfo entry
static inline void fillImageInfos(const pvr::uint32& index, const DescriptorSetUpdate::Image& imageBinding, VkDescriptorImageInfo* imageInfos)
{
	imageInfos[index].imageView = native_cast(*imageBinding.second).handle;
	if (imageBinding.first.useSampler)
	{
		imageInfos[index].sampler = native_cast(*imageBinding.first.sampler).handle;
	}
	else
	{
		imageInfos[index].sampler = VK_NULL_HANDLE;
	}
	imageInfos[index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

static inline void createImageDescriptorSet(
  const pvr::uint8 numberOfImages, const pvr::uint8 maxNumberOfImages,
  const api::DescriptorSetUpdate::ImageBinding* imageList, const VkDescriptorSet& handle,
  pvr::types::DescriptorBindingType expectedType, pvr::uint32& descSetWritesIndex,
  VkDescriptorImageInfo* imageInfos, VkWriteDescriptorSet* descSetWritesVk)
{
	// update the combined image samplers
	if (numberOfImages > 0)
	{
		// Current Implementation assumes that array indexes are linear, and they are in the order of binding id.
		// bindingId : arrayIndex
		// 0:0, 0:1, 0:2, 1:0, 1:1, 1:2, ....
		VkWriteDescriptorSet currentDescSet = initializeWriteDescSet();

		int32 lastBindingId = -1;
		uint32 lastArrayIndex = (uint32) - 1;

		// for each descriptor
		// - Validate the image binding id and the array index are linear.
		// - add a new descriptor set if its a new binding or end of array of the binding
		for (pvr::uint32 i = 0; i < maxNumberOfImages; ++i)
		{
			auto& bindingInfo = imageList[i];
			if (!bindingInfo.isValid()) { continue; }

			assertion(i == 0 ||
			          bindingInfo.bindingId != (int16)lastBindingId ||
			          bindingInfo.arrayIndex == (int16)lastArrayIndex + 1);

#ifdef DEBUG
			// verify we aren't trampling on an existing imageinfo entry
			assertion(imageInfos[i].imageLayout == 0);
			assertion(imageInfos[i].imageView == 0);
#endif

			fillImageInfos(i, bindingInfo.binding, imageInfos);

			if (bindingInfo.bindingId != lastBindingId) // if its a new binding or end of array of the binding
			{
				currentDescSet.descriptorCount = 0;
				currentDescSet.dstBinding = lastBindingId = bindingInfo.bindingId;
				currentDescSet.dstArrayElement = lastArrayIndex = bindingInfo.arrayIndex;
				currentDescSet.dstSet = handle;
				currentDescSet.pImageInfo = &imageInfos[i];
				currentDescSet.descriptorType = ConvertToVk::descriptorType(bindingInfo.descType);
#ifdef DEBUG
				assertion(expectedType ==
				          pvr::types::getDescriptorTypeBinding(bindingInfo.descType),
				          "Descriptor must be an image");
#endif
				descSetWritesVk[descSetWritesIndex] = currentDescSet;
				descSetWritesIndex++;
			}
			descSetWritesVk[descSetWritesIndex - 1].descriptorCount++;
			}
		}
	}

static inline void createBufferDescriptorSet(
  const pvr::uint8 numberOfBuffers,
    const pvr::uint8 maxNumberOfBuffers,
  const api::DescriptorSetUpdate::BufferViewBinding* bufferList,
    const VkDescriptorSet& handle,
    pvr::types::DescriptorBindingType expectedType,
    pvr::uint32& descSetWritesIndex,
    VkDescriptorBufferInfo* bufferInfo,
    VkWriteDescriptorSet* descSetWritesVk)
{
	if (numberOfBuffers > 0)
	{
		// Current Implementation assumes that array indexes are linear, and they are in the order of binding id.
		// bindingId : arrayIndex
		// 0:0, 0:1, 0:2, 1:0, 1:1, 1:2, ....
		VkWriteDescriptorSet currentDescSet = initializeWriteDescSet();

		int32 lastBindingId = -1;
		uint32 lastArrayIndex = (uint32) - 1;

		// for each descriptor
		// - Validate the buffer binding id and the array index are linear.
		// - add a new descriptor set if its a new binding or end of array of the binding
		for (pvr::uint32 i = 0; i < maxNumberOfBuffers; ++i)
		{
			auto& bindingInfo = bufferList[i];
			if (!bindingInfo.isValid()) { continue; }

			auto binding = bindingInfo.binding;

#ifdef DEBUG
			// verify we aren't trampling on an existing bufferinfo entry
			assertion(bufferInfo[i].buffer == 0);
			assertion(bufferInfo[i].range == 0);
#endif

			bufferInfo[i].buffer = native_cast(*binding->getResource()).buffer;
			bufferInfo[i].offset = binding->getOffset();
			bufferInfo[i].range = binding->getRange();
			if (bindingInfo.bindingId != lastBindingId) // if its a new binding or end of array of the binding
			{
				currentDescSet.descriptorCount = 0;
				currentDescSet.dstBinding = lastBindingId = bindingInfo.bindingId;
				currentDescSet.dstArrayElement = lastArrayIndex = bindingInfo.arrayIndex;
				currentDescSet.dstSet = handle;
				currentDescSet.pBufferInfo = &bufferInfo[i];
#ifdef DEBUG
				assertion(expectedType ==
				          pvr::types::getDescriptorTypeBinding(bindingInfo.descType));
#endif
				currentDescSet.descriptorType = ConvertToVk::descriptorType(bindingInfo.descType);
				descSetWritesVk[descSetWritesIndex] = currentDescSet;
				descSetWritesIndex++;
			}
			++descSetWritesVk[descSetWritesIndex - 1].descriptorCount;
		}
	}
}

#ifdef DEBUG
static inline void validateBufferEntries(const pvr::uint32 totalNumberOfBuffers,
    const VkDescriptorBufferInfo* bufferInfo,
    const pvr::uint32& descSetWritesIndex,
    const VkWriteDescriptorSet* descSetWritesVk)
{
	// validate total number of buffer entries
	pvr::uint32 validBufferInfoEntries = 0;
	for (pvr::uint32 i = 0; i < pvr::types::DescriptorBindingDefaults::MaxStorageBuffers +
	     pvr::types::DescriptorBindingDefaults::MaxUniformBuffers; ++i)
	{
		if (bufferInfo[i].buffer != 0 && bufferInfo[i].range != 0)
		{
			validBufferInfoEntries++;
		}
	}
	assertion(validBufferInfoEntries == totalNumberOfBuffers);
}
#endif

//--- DescriptorSet Implementation
bool DescriptorSetVk_::update(const DescriptorSetUpdate& descSet)
{
	m_descParam = descSet;
	VkDescriptorImageInfo imageInfos[pvr::types::DescriptorBindingDefaults::MaxImages];
	VkDescriptorBufferInfo bufferInfo[pvr::types::DescriptorBindingDefaults::MaxStorageBuffers +
	                                  pvr::types::DescriptorBindingDefaults::MaxUniformBuffers];
	memset(&imageInfos, 0, sizeof(imageInfos));
	memset(&bufferInfo, 0, sizeof(bufferInfo));
	VkWriteDescriptorSet descSetWritesVk[pvr::types::DescriptorBindingDefaults::MaxImages +
	                                     pvr::types::DescriptorBindingDefaults::MaxStorageBuffers +
	                                     pvr::types::DescriptorBindingDefaults::MaxUniformBuffers];

	pvr::uint32 descSetWritesIndex = 0;

	createImageDescriptorSet(descSet.getNumImages(), pvr::types::DescriptorBindingDefaults::MaxImages,
	                         descSet.getBindingList().images, handle, pvr::types::DescriptorBindingType::Image,
	                         descSetWritesIndex, imageInfos, descSetWritesVk);

	createBufferDescriptorSet(descSet.getNumUbos(), pvr::types::DescriptorBindingDefaults::MaxUniformBuffers,
	                          descSet.getBindingList().uniformBuffers, handle, pvr::types::DescriptorBindingType::UniformBuffer,
	                          descSetWritesIndex, bufferInfo, descSetWritesVk);

	createBufferDescriptorSet(descSet.getNumSsbos(), pvr::types::DescriptorBindingDefaults::MaxStorageBuffers,
	                          descSet.getBindingList().storageBuffers, handle, pvr::types::DescriptorBindingType::StorageBuffer,
	                          descSetWritesIndex, bufferInfo, descSetWritesVk);

#ifdef DEBUG
	validateBufferEntries(descSet.getNumUbos() + descSet.getNumSsbos(),
	                      bufferInfo, descSetWritesIndex, descSetWritesVk);

	// validate that image and buffer entries have been added linearly starting from zero
	std::vector<pvr::uint32> bindingIndices(0);
	for (pvr::uint32 i = 0; i < descSetWritesIndex; ++i)
	{
		if (descSet.getBindingList().images[i].isValid())
		{
			bindingIndices.push_back(descSet.getBindingList().images[i].bindingId);
		}
		else if (descSet.getBindingList().storageBuffers[i].isValid())
		{
			bindingIndices.push_back(descSet.getBindingList().storageBuffers[i].bindingId);
		}
		else if (descSet.getBindingList().uniformBuffers[i].isValid())
		{
			bindingIndices.push_back(descSet.getBindingList().uniformBuffers[i].bindingId);
		}
		else
		{
			debug_assertion(false, "Binding indices must be linear and must start from zero.");
		}
	}
	std::sort(std::begin(bindingIndices), std::end(bindingIndices));

	pvr::uint32 lastIndex = 0;
	for (pvr::uint32 i = 0; i < descSetWritesIndex; ++i)
	{
		if (i == 0)
		{
			debug_assertion(bindingIndices[i] == 0,
			                "Binding indices must be linear and must start from zero.");
			}
		else
		{
			debug_assertion(bindingIndices[i] - 1 == lastIndex,
			                "Binding indices must be linear and must start from zero.");
			lastIndex = bindingIndices[i];
		}
	}
#endif

	if (descSetWritesIndex > 0)
	{
		vk::UpdateDescriptorSets(native_cast(*m_descSetLayout->getContext()).getDevice(), descSetWritesIndex, descSetWritesVk, 0, 0);
	}
	return true;
}

//--- DescriptorPool Implementation
bool DescriptorPoolVk_::init(const DescriptorPoolCreateParam& createParam)
{
	VkDescriptorPoolCreateInfo descPoolInfo;
	descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descPoolInfo.pNext = NULL;
	descPoolInfo.maxSets = createParam.getMaxSets();
	descPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	VkDescriptorPoolSize poolSize[(uint32)types::DescriptorType::Count];
	pvr::uint32 i = 0, poolIndex = 0;
	for (; i < (uint32)types::DescriptorType::Count; ++i)
	{
		pvr::uint32 count = createParam.getDescriptorTypeCount(static_cast<types::DescriptorType>(i));
		if (count)
		{
			poolSize[poolIndex].type = ConvertToVk::descriptorType(static_cast<types::DescriptorType>(i));
			poolSize[poolIndex].descriptorCount = count;
			++poolIndex;
		}
	}// next type
	descPoolInfo.poolSizeCount = poolIndex;
	descPoolInfo.pPoolSizes = poolSize;

	VkDevice dev = native_cast(*m_context).getDevice();
	VkDescriptorPool pool;
	VkResult res = vk::CreateDescriptorPool(dev, &descPoolInfo, NULL, &pool);
	native_cast(this)->handle = pool;
	return res == VK_SUCCESS;
}
void DescriptorPoolVk_::destroy()
{
	if (handle)
	{
		vk::DestroyDescriptorPool(native_cast(*m_context).getDevice(), this->handle, NULL);
	}
	handle = 0;
}

DescriptorPoolVk_::~DescriptorPoolVk_()
{
	if (m_context.isValid())
	{
		destroy();
	}
	else
{
		Log(Log.Warning, "Attempted to free DescriptorPool after its corresponding context was destroyed.");
	}
}

static inline void addDescriptorBindingLayout(const pvr::uint32 bindingIndex, const pvr::types::DescriptorBindingLayout& bindInfo,
    std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	if (bindInfo.isValid())
	{
		VkDescriptorSetLayoutBinding vkLayoutBinding;
		vkLayoutBinding.descriptorType =  ConvertToVk::descriptorType(bindInfo.descType);
		vkLayoutBinding.descriptorCount = bindInfo.arraySize;
		vkLayoutBinding.pImmutableSamplers = NULL;
		vkLayoutBinding.stageFlags = ConvertToVk::shaderStage(bindInfo.shaderStage);
		vkLayoutBinding.binding = bindingIndex;
		bindings[bindingIndex] = vkLayoutBinding;
	}
}

bool DescriptorSetLayoutVk_::init()
{
	platform::ContextVk& contextVk = native_cast(*getContext());
	VkDescriptorSetLayoutCreateInfo vkLayoutCreateInfo;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.resize(getCreateParam().getNumBindings());
	for (pvr::uint8 i = 0; i < pvr::types::DescriptorBindingDefaults::MaxImages; ++i)
	{
		const pvr::types::DescriptorBindingLayout& bindInfo = getCreateParam().getBindingImage(i);
		addDescriptorBindingLayout(i, bindInfo, bindings);
	}
	for (pvr::uint8 i = 0; i < pvr::types::DescriptorBindingDefaults::MaxUniformBuffers; ++i)
	{
		const pvr::types::DescriptorBindingLayout& bindInfo = getCreateParam().getBindingUbo(i);
		addDescriptorBindingLayout(i, bindInfo, bindings);
	}
	for (pvr::uint8 i = 0; i < pvr::types::DescriptorBindingDefaults::MaxStorageBuffers; ++i)
	{
		const pvr::types::DescriptorBindingLayout& bindInfo = getCreateParam().getBindingSsbo(i);
		addDescriptorBindingLayout(i, bindInfo, bindings);
	}
	vkLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	vkLayoutCreateInfo.pNext = NULL;
	vkLayoutCreateInfo.flags = 0;
	vkLayoutCreateInfo.bindingCount = (uint32)bindings.size();
	vkLayoutCreateInfo.pBindings = bindings.data();
	return (vk::CreateDescriptorSetLayout(contextVk.getDevice(), &vkLayoutCreateInfo, NULL, &handle) == VK_SUCCESS);
}
}// namespace vulkan
}// namespace api
}// namespace pvr
//!\endcond
