<<<<<<< HEAD
/*!*********************************************************************************************************************
\file         PVRApi\Vulkan\DescriptorSetVk.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Vulkan implementation of the DescriptorSet class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
=======
/*!
\brief Vulkan implementation of the DescriptorSet class.
\file PVRApi/Vulkan/DescriptorSetVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
>>>>>>> 1776432f... 4.3
#include "PVRApi/Vulkan/DescriptorSetVk.h"
#include "PVRApi/Vulkan/TextureVk.h"
#include "PVRApi/Vulkan/SamplerVk.h"
#include "PVRApi/Vulkan/BufferVk.h"
#include "PVRApi/Vulkan/IndirectRayPipelineVk.h"

#ifdef DEBUG
#include <algorithm>
#endif

#ifdef DEBUG
#include <algorithm>
#endif

namespace pvr {
namespace api {

// HELPER FUNCTIONS
namespace {

#ifdef DEBUG
static inline void validateBufferEntries(const pvr::uint32 totalNumberOfBuffers, const VkDescriptorBufferInfo* bufferInfo,
    const pvr::uint32& descSetWritesIndex, const VkWriteDescriptorSet* descSetWritesVk)
{
	debug_assertion(descSetWritesIndex >= totalNumberOfBuffers, "Expected total number of buffers does not match the number of buffers.");
	// validate total number of buffer entries
	pvr::uint32 validBufferInfoEntries = 0;
	for (pvr::uint32 i = 0; i < totalNumberOfBuffers; ++i)
	{
		if (bufferInfo[i].buffer != 0 && bufferInfo[i].range != 0)
		{
			validBufferInfoEntries++;
		}
	}
	debug_assertion(validBufferInfoEntries == totalNumberOfBuffers, "Expected total number of buffers does not match the number of buffers.");
}
#endif
#ifdef DEBUG
static inline void validateLayoutBindings(pvr::api::DescriptorSetLayout& descSetLayout, const pvr::api::DescriptorSetUpdate& descSetUpdate)
{
	const pvr::types::DescriptorBindingLayout* imageLayout = descSetLayout->getCreateParam().getImages();
	const pvr::types::DescriptorBindingLayout* uboLayout = descSetLayout->getCreateParam().getUbos();
	const pvr::types::DescriptorBindingLayout* ssboLayout = descSetLayout->getCreateParam().getSsbos();
	const pvr::types::DescriptorBindingLayout* indirectPipelinesLayout = descSetLayout->getCreateParam().getIndirectRayPipelines();

	struct DescriptorLayout
	{
		pvr::uint16 bindingId;
		types::DescriptorType type;
		pvr::int16 arrayIndex;
	};

	std::vector<DescriptorLayout> layoutTypes;

	pvr::uint16 numDescriptors = 0;
	numDescriptors += descSetLayout->getCreateParam().getTotalArrayElementImageCount();
	numDescriptors += descSetLayout->getCreateParam().getTotalArrayElementUboCount();
	numDescriptors += descSetLayout->getCreateParam().getTotalArrayElementSsboCount();
	numDescriptors += descSetLayout->getCreateParam().getTotalArrayElementIndirectRayPipelineCount();

	layoutTypes.reserve(numDescriptors);

	// add each of the descriptor array elements
	for (pvr::uint16 i = 0; i < descSetLayout->getCreateParam().getImageCount(); i++)
	{
		for (pvr::uint16 j = 0; j < imageLayout[i]._arraySize; j++)
		{
			layoutTypes.push_back(DescriptorLayout{ imageLayout[i]._bindingId, imageLayout[i]._descType, static_cast<pvr::int16>(j) });
		}
	}
<<<<<<< HEAD
}

void DescriptorSetLayoutVk_::destroy()
{
	if (device.isValid())
=======
	for (pvr::uint16 i = 0; i < descSetLayout->getCreateParam().getUboCount(); i++)
>>>>>>> 1776432f... 4.3
	{
		for (pvr::uint16 j = 0; j < uboLayout[i]._arraySize; j++)
		{
			layoutTypes.push_back(DescriptorLayout{ uboLayout[i]._bindingId, uboLayout[i]._descType, static_cast<pvr::int16>(j) });
		}
	}
<<<<<<< HEAD
	desc.clear();
}

DescriptorSetVk_::~DescriptorSetVk_()
{
	if (m_descPool.isValid())
=======
	for (pvr::uint16 i = 0; i < descSetLayout->getCreateParam().getSsboCount(); i++)
>>>>>>> 1776432f... 4.3
	{
		for (pvr::uint16 j = 0; j < ssboLayout[i]._arraySize; j++)
		{
			layoutTypes.push_back(DescriptorLayout{ ssboLayout[i]._bindingId, ssboLayout[i]._descType, static_cast<pvr::int16>(j) });
		}
	}
	for (pvr::uint16 i = 0; i < descSetLayout->getCreateParam().getIndirectRayPipelineCount(); i++)
	{
		for (pvr::uint16 j = 0; j < indirectPipelinesLayout[i]._arraySize; j++)
		{
			layoutTypes.push_back(DescriptorLayout{ indirectPipelinesLayout[i]._bindingId, indirectPipelinesLayout[i]._descType, static_cast<pvr::int16>(j) });
		}
	}

	auto* updateImages = descSetUpdate.getImages();
	auto* updateUbos = descSetUpdate.getUbos();
	auto* updateSsbos = descSetUpdate.getSsbos();
	auto* updateIndirectPipelines = descSetUpdate.getIndirectRayPipelines();
	auto* updateAccumulationImages = descSetUpdate.getAccumulationImages();

	std::vector<DescriptorLayout> updateTypes;
	updateTypes.reserve(descSetUpdate.getBindingCount());

	for (pvr::uint16 i = 0; i < descSetUpdate.getImageCount(); i++)
	{
		updateTypes.push_back(DescriptorLayout{ updateImages[i]._bindingId, updateImages[i]._descType, updateImages[i]._arrayIndex });
	}
	for (pvr::uint16 i = 0; i < descSetUpdate.getUboCount(); i++)
	{
		updateTypes.push_back(DescriptorLayout{ updateUbos[i]._bindingId, updateUbos[i]._descType, updateUbos[i]._arrayIndex });
	}
	for (pvr::uint16 i = 0; i < descSetUpdate.getSsboCount(); i++)
	{
		updateTypes.push_back(DescriptorLayout{ updateSsbos[i]._bindingId, updateSsbos[i]._descType, updateSsbos[i]._arrayIndex });
	}
	for (pvr::uint16 i = 0; i < descSetUpdate.getIndirectRayPipelineCount(); i++)
	{
		updateTypes.push_back(DescriptorLayout{ updateIndirectPipelines[i]._bindingId, updateIndirectPipelines[i]._descType, updateIndirectPipelines[i]._arrayIndex });
	}
	for (pvr::uint16 i = 0; i < descSetUpdate.getAccumulationImageCount(); i++)
	{
		updateTypes.push_back(DescriptorLayout{ updateAccumulationImages[i]._bindingId, updateAccumulationImages[i]._descType, updateAccumulationImages[i]._arrayIndex });
	}

	debug_assertion(layoutTypes.size() >= updateTypes.size(), "The number of descriptors updated must be less than or equal to the number of Descriptor set layout items.");

	bool currentUpdateValidated = false;

	for (pvr::uint32 i = 0; i < updateTypes.size(); i++)
	{
		auto currentUpdate = updateTypes[i];
		currentUpdateValidated = false;

<<<<<<< HEAD
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

=======
		for (pvr::uint32 j = 0; j < layoutTypes.size(); j++)
		{
			if (currentUpdate.bindingId == layoutTypes[j].bindingId && currentUpdate.arrayIndex == layoutTypes[j].arrayIndex)
			{
				debug_assertion(currentUpdate.type == layoutTypes[j].type, "The descriptor set layout type and update types must match.");
				currentUpdateValidated = true;
			}
		}

		debug_assertion(currentUpdateValidated, "Could not validate the current descriptor update type.");
	}
}
#endif
>>>>>>> 1776432f... 4.3
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
<<<<<<< HEAD
	imageInfos[index].imageView = native_cast(*imageBinding.second).handle;
	if (imageBinding.first.useSampler)
	{
		imageInfos[index].sampler = native_cast(*imageBinding.first.sampler).handle;
	}
	else
	{
=======
	imageInfos[index].imageView = pvr::api::native_cast(*imageBinding.second).handle;
	if (imageBinding.first._useSampler)
	{
		imageInfos[index].sampler = pvr::api::native_cast(*imageBinding.first._sampler).handle;
	}
	else
	{
>>>>>>> 1776432f... 4.3
		imageInfos[index].sampler = VK_NULL_HANDLE;
	}
	imageInfos[index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

<<<<<<< HEAD
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
=======
static inline void createImageDescriptorSet(const pvr::uint16 numberOfImageBindings, const pvr::types::DescriptorItemBinding<DescriptorSetUpdate::Image>* imageList,
    const VkDescriptorSet& handle, pvr::types::DescriptorBindingType expectedType, pvr::uint32& descSetWritesIndex, VkDescriptorImageInfo* imageInfos,
    VkWriteDescriptorSet* descSetWritesVk)
{
	if (numberOfImageBindings > 0)
	{
		VkWriteDescriptorSet currentDescSet = initializeWriteDescSet();

		int16 lastBindingId = -1;
		uint16 imageArrayInfoIndex = 0;
		uint16 numArrayElements = 0;

		for (pvr::uint8 i = 0; i < numberOfImageBindings; ++i)
>>>>>>> 1776432f... 4.3
		{
			auto& bindingInfo = imageList[i];
			if (!bindingInfo.isValid()) { continue; }

<<<<<<< HEAD
			assertion(i == 0 ||
			          bindingInfo.bindingId != (int16)lastBindingId ||
			          bindingInfo.arrayIndex == (int16)lastArrayIndex + 1);

#ifdef DEBUG
			// verify we aren't trampling on an existing imageinfo entry
			assertion(imageInfos[i].imageLayout == 0);
			assertion(imageInfos[i].imageView == 0);
#endif

			fillImageInfos(i, bindingInfo.binding, imageInfos);
=======
			// verify we aren't trampling on an existing imageinfo entry
			debug_assertion(imageInfos[imageArrayInfoIndex].imageLayout == 0, "Overwriting existing image infos element.");
			debug_assertion(imageInfos[imageArrayInfoIndex].imageView == 0, "Overwriting existing image infos element.");

			fillImageInfos(imageArrayInfoIndex, bindingInfo._binding, imageInfos);
			imageArrayInfoIndex++;
>>>>>>> 1776432f... 4.3

			if (bindingInfo._bindingId == lastBindingId) // if its a new binding or end of array of the binding
			{
				numArrayElements++;
			}
			else
			{
<<<<<<< HEAD
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
=======
				numArrayElements = 1;
				currentDescSet.dstSet = handle;
				currentDescSet.dstBinding = bindingInfo._bindingId;
				lastBindingId = bindingInfo._bindingId;

				currentDescSet.dstArrayElement = bindingInfo._arrayIndex;
				currentDescSet.descriptorType = nativeVk::ConvertToVk::descriptorType(bindingInfo._descType);
				currentDescSet.pImageInfo = &imageInfos[imageArrayInfoIndex - 1];

				debug_assertion(expectedType == pvr::types::getDescriptorTypeBinding(bindingInfo._descType), "Descriptor must be an image");

				descSetWritesVk[descSetWritesIndex] = currentDescSet;
				descSetWritesIndex++;
			}

			descSetWritesVk[descSetWritesIndex - 1].descriptorCount = numArrayElements;
>>>>>>> 1776432f... 4.3
		}
	}
}

<<<<<<< HEAD
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
=======
static inline void createBufferDescriptorSet(const pvr::uint8 numberOfBufferBindings, const pvr::types::DescriptorItemBinding<BufferView>* bufferList,
    const VkDescriptorSet& handle, pvr::types::DescriptorBindingType expectedType, pvr::uint32& descSetWritesIndex,
    VkDescriptorBufferInfo* bufferInfo, VkWriteDescriptorSet* descSetWritesVk, uint16& bufferArrayInfoIndex)
{
	if (numberOfBufferBindings > 0)
	{
		VkWriteDescriptorSet currentDescSet = initializeWriteDescSet();

		int16 lastBindingId = -1;
>>>>>>> 1776432f... 4.3

		uint16 numArrayElements = 0;

		// for each descriptor
		// - Validate the buffer binding id and the array index are linear.
		// - add a new descriptor set if its a new binding or end of array of the binding
<<<<<<< HEAD
		for (pvr::uint32 i = 0; i < maxNumberOfBuffers; ++i)
=======
		for (pvr::uint8 i = 0; i < numberOfBufferBindings; ++i)
>>>>>>> 1776432f... 4.3
		{
			auto& bindingInfo = bufferList[i];
			if (!bindingInfo.isValid()) { continue; }

<<<<<<< HEAD
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
=======
			// verify we aren't trampling on an existing bufferinfo entry
			debug_assertion(bufferInfo[bufferArrayInfoIndex].buffer == 0, "Overwriting existing image infos element.");
			debug_assertion(bufferInfo[bufferArrayInfoIndex].range == 0, "Overwriting existing image infos element.");

			auto arrayBinding = bindingInfo._binding;
			bufferInfo[bufferArrayInfoIndex].buffer = pvr::api::native_cast(*arrayBinding->getResource()).buffer;
			bufferInfo[bufferArrayInfoIndex].offset = arrayBinding->getOffset();
			bufferInfo[bufferArrayInfoIndex].range = arrayBinding->getRange();
			bufferArrayInfoIndex++;

			if (bindingInfo._bindingId == lastBindingId) // if its a new binding or end of array of the binding
			{
				numArrayElements++;
			}
			else
			{
				numArrayElements = 1;
				currentDescSet.dstSet = handle;
				currentDescSet.dstBinding = bindingInfo._bindingId;
				lastBindingId = bindingInfo._bindingId;

				currentDescSet.dstArrayElement = bindingInfo._arrayIndex;
				currentDescSet.descriptorType = nativeVk::ConvertToVk::descriptorType(bindingInfo._descType);
				currentDescSet.pBufferInfo = &bufferInfo[bufferArrayInfoIndex - 1];

				debug_assertion(expectedType == pvr::types::getDescriptorTypeBinding(bindingInfo._descType), "Descriptor must be a buffer");

				descSetWritesVk[descSetWritesIndex] = currentDescSet;
				descSetWritesIndex++;
			}

			descSetWritesVk[descSetWritesIndex - 1].descriptorCount = numArrayElements;
		}
	}
}


static inline void addDescriptorBindingLayout(pvr::uint32 arrayIndex, const pvr::types::DescriptorBindingLayout& bindInfo,
    std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	if (bindInfo.isValid())
	{
		VkDescriptorSetLayoutBinding vkLayoutBinding;
		vkLayoutBinding.descriptorType = nativeVk::ConvertToVk::descriptorType(bindInfo._descType);
		vkLayoutBinding.descriptorCount = bindInfo._arraySize;
		vkLayoutBinding.pImmutableSamplers = NULL;
		vkLayoutBinding.stageFlags = nativeVk::ConvertToVk::shaderStage(bindInfo._shaderStage);
		vkLayoutBinding.binding = bindInfo._bindingId;
		bindings[arrayIndex] = vkLayoutBinding;
>>>>>>> 1776432f... 4.3
	}
}
}

// DESCRIPTOR POOL
namespace vulkan {
api::DescriptorSet DescriptorPoolVk_::allocateDescriptorSet_(const DescriptorSetLayout& layout)
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

bool DescriptorPoolVk_::init(const DescriptorPoolCreateParam& createParam)
{
	VkDescriptorPoolCreateInfo descPoolInfo;
	descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descPoolInfo.pNext = NULL;
	descPoolInfo.maxSets = createParam.getMaxSetCount();
	descPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
<<<<<<< HEAD
	VkDescriptorPoolSize poolSize[(uint32)types::DescriptorType::Count];
	pvr::uint32 i = 0, poolIndex = 0;
	for (; i < (uint32)types::DescriptorType::Count; ++i)
=======
	VkDescriptorPoolSize poolSize[pvr::uint32(pvr::types::DescriptorType::Count)];
	pvr::uint32 poolIndex = 0;
	for (pvr::uint32 i = 0; i < pvr::uint32(pvr::types::DescriptorType::Count); ++i)
>>>>>>> 1776432f... 4.3
	{
		pvr::uint32 count = createParam.getDescriptorTypeCount(static_cast<types::DescriptorType>(i));
		if (count)
		{
<<<<<<< HEAD
			poolSize[poolIndex].type = ConvertToVk::descriptorType(static_cast<types::DescriptorType>(i));
=======
			poolSize[poolIndex].type = nativeVk::ConvertToVk::descriptorType(static_cast<types::DescriptorType>(i));
>>>>>>> 1776432f... 4.3
			poolSize[poolIndex].descriptorCount = count;
			++poolIndex;
		}
	}// next type
	descPoolInfo.poolSizeCount = poolIndex;
	descPoolInfo.pPoolSizes = poolSize;

	VkDevice dev = native_cast(*getContext()).getDevice();
	VkDescriptorPool pool;
	VkResult res = vk::CreateDescriptorPool(dev, &descPoolInfo, NULL, &pool);
	pvr::api::native_cast(this)->handle = pool;
	return res == VK_SUCCESS;
}
void DescriptorPoolVk_::destroy()
{
	if (handle)
	{
		vk::DestroyDescriptorPool(native_cast(*getContext()).getDevice(), this->handle, NULL);
	}
	handle = 0;
}

DescriptorPoolVk_::~DescriptorPoolVk_()
{
	if (getContext().isValid())
	{
		destroy();
	}
	else
{
		Log(Log.Warning, "Attempted to free DescriptorPool after its corresponding context was destroyed.");
	}
}
}

// DESCRIPTOR SET
namespace vulkan {
#ifdef DEBUG
static inline bool bindingIdPairComparison(const std::pair<pvr::uint32, uint32>& a, const std::pair<uint32, uint32>& b)
{
	return a.first < b.first;
}
#endif

bool DescriptorSetVk_::update_(const DescriptorSetUpdate& descSet)
{
	_descParam = descSet;
	std::vector<VkDescriptorImageInfo> imageInfos;
	imageInfos.resize(descSet.getImageCount());
	std::vector<VkDescriptorBufferInfo> bufferInfo;
	bufferInfo.resize(descSet.getUboCount() + descSet.getSsboCount());


	std::vector<VkWriteDescriptorSet> descSetWritesVk;
	descSetWritesVk.resize(descSet.getImageCount() + descSet.getUboCount() + descSet.getSsboCount()
	                      );


#ifdef DEBUG
	validateLayoutBindings(_descSetLayout, descSet);
#endif

	pvr::uint32 descSetWritesIndex = 0;
	pvr::uint16 bufferArrayInfoIndex = 0;

	createImageDescriptorSet(descSet.getImageCount(), descSet.getImages(), handle, pvr::types::DescriptorBindingType::Image,
	                         descSetWritesIndex, imageInfos.data(), descSetWritesVk.data());

	createBufferDescriptorSet(descSet.getUboCount(), descSet.getUbos(), handle, pvr::types::DescriptorBindingType::UniformBuffer,
	                          descSetWritesIndex, bufferInfo.data(), descSetWritesVk.data(), bufferArrayInfoIndex);

	createBufferDescriptorSet(descSet.getSsboCount(), descSet.getSsbos(), handle, pvr::types::DescriptorBindingType::StorageBuffer,
	                          descSetWritesIndex, bufferInfo.data(), descSetWritesVk.data(), bufferArrayInfoIndex);


#ifdef DEBUG
	validateBufferEntries(descSet.getUboCount() + descSet.getSsboCount(), bufferInfo.data(), descSetWritesIndex, descSetWritesVk.data());

	// validate that image and buffer entries have been added linearly starting from zero
	std::vector<std::pair<pvr::uint32, pvr::uint32>> bindingIndices(0);

	for (pvr::uint32 i = 0; i < descSet.getImageCount(); i++)
	{
		bindingIndices.push_back(std::make_pair(descSet.getImages()[i]._bindingId, descSet.getImages()[i]._arrayIndex));
	}

	for (pvr::uint32 i = 0; i < descSet.getUboCount(); i++)
	{
		bindingIndices.push_back(std::make_pair(descSet.getUbos()[i]._bindingId, descSet.getUbos()[i]._arrayIndex));
	}

	for (pvr::uint32 i = 0; i < descSet.getSsboCount(); i++)
	{
		bindingIndices.push_back(std::make_pair(descSet.getSsbos()[i]._bindingId, descSet.getSsbos()[i]._arrayIndex));
	}

	std::sort(std::begin(bindingIndices), std::end(bindingIndices), bindingIdPairComparison);

	pvr::uint32 lastBindingIndex = 0;
	pvr::uint32 lastArrayBindingIndex = 0;
	for (pvr::uint32 i = 0; i < descSetWritesIndex; ++i)
	{
		if (i == 0)
		{
			debug_assertion(bindingIndices[i].first == 0, "Binding indices must be linear and must start from zero.");
		}
		else
		{
			debug_assertion(bindingIndices[i].first - 1 == lastBindingIndex || bindingIndices[i].second - 1 == lastArrayBindingIndex,
			                "Binding indices must be linear and must start from zero.");
			lastBindingIndex = bindingIndices[i].first;
			lastArrayBindingIndex = bindingIndices[i].second;
		}
	}
#endif

	if (descSetWritesIndex > 0)
	{
		vk::UpdateDescriptorSets(native_cast(*_descSetLayout->getContext()).getDevice(), descSetWritesIndex, descSetWritesVk.data(), 0, 0);
	}
	return true;
}

void DescriptorSetVk_::destroy()
{
	auto& vkobj = native_cast(*this);
	if (vkobj.handle != VK_NULL_HANDLE)
	{
		if (_descPool->getContext().isValid())
		{
			vk::FreeDescriptorSets(native_cast(*getContext()).getDevice(), native_cast(_descPool)->handle, 1, &native_cast(*this).handle);
		}
		vkobj.handle = VK_NULL_HANDLE;
		_descPool.reset();
		_descSetLayout.reset();
	}
}

void DescriptorSetLayoutVk_::destroy()
{
	if (getContext().isValid())
	{
		if (handle != VK_NULL_HANDLE)
		{
			vk::DestroyDescriptorSetLayout(native_cast(*getContext()).getDevice(), native_cast(*this), NULL);
		}
		getContext().reset();
	}
	clearCreateParam();
}

DescriptorSetVk_::~DescriptorSetVk_()
{
	if (_descPool.isValid())
	{
		if (_descPool->getContext().isValid())
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
	if (getContext().isValid())
	{
		destroy();
	}
	else
	{
		Log(Log.Warning, "Attempted to free DescriptorSetLayout after its corresponding device was destroyed");
	}
}

static inline void addDescriptorBindingLayout(const pvr::uint32 bindingIndex, const pvr::types::DescriptorBindingLayout& bindInfo,
    std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
<<<<<<< HEAD
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
=======
	platform::ContextVk& contextVk = pvr::api::native_cast(*getContext());
	VkDescriptorSetLayoutCreateInfo vkLayoutCreateInfo;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.resize(getCreateParam().getBindingCount());

	pvr::uint32 arrayIndex = 0;

	auto imageBindings = getCreateParam().getImages();
	for (pvr::uint8 i = 0; i < getCreateParam().getImageCount(); ++i)
	{
		auto& binding = imageBindings[i];
		addDescriptorBindingLayout(arrayIndex++, binding, bindings);
	}

	auto uboBindings = getCreateParam().getUbos();
	for (pvr::uint8 i = 0; i < getCreateParam().getUboCount(); ++i)
	{
		auto& binding = uboBindings[i];
		addDescriptorBindingLayout(arrayIndex++, binding, bindings);
	}

	auto ssboBindings = getCreateParam().getSsbos();
	for (pvr::uint8 i = 0; i < getCreateParam().getSsboCount(); ++i)
	{
		auto& binding = ssboBindings[i];
		addDescriptorBindingLayout(arrayIndex++, binding, bindings);
	}

>>>>>>> 1776432f... 4.3
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
