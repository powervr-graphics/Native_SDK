/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/DescriptorSetVk.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Definition of the Vulkan implementation of the DescriptorSet and supporting classes
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiObjects/DescriptorSet.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRApi/Vulkan/ContextVk.h"
#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"
namespace pvr {
namespace api {
namespace vulkan {

/*!*********************************************************************************************************************
\brief ctor. Vulkan implementation of a DescriptorSet.
***********************************************************************************************************************/
class DescriptorSetLayoutVk_ : public impl::DescriptorSetLayout_, public native::HDescriptorSetLayout_
{
public:
	/*!*********************************************************************************************************************
	\brief ctor. Do not use directly, use context->createDescriptorSet.
	  \param context The GraphicsContext who owns this descriptor-set-layout.
	  \param desc This descriptor-set-layout create param
	***********************************************************************************************************************/
	DescriptorSetLayoutVk_(GraphicsContext& context, const DescriptorSetLayoutCreateParam& desc) :
		DescriptorSetLayout_(context, desc) {}

	/*!*********************************************************************************************************************
	\brief Initialize this descriptor-set-layout
	\return Return true on success.
	***********************************************************************************************************************/
	bool init();
};

/*!*********************************************************************************************************************
\brief Vulkan implementation of a DescriptorSet.
***********************************************************************************************************************/
class DescriptorSetVk_ : public impl::DescriptorSet_, public native::HDescriptorSet_
{
public:
	/*!*********************************************************************************************************************
	\brief ctor.
	  \param descSetLayout The DescriptorSetLayout of this descriptor set
	  \param pool The DescriptorPool this descriptor-set will be allocated from.
	***********************************************************************************************************************/
	DescriptorSetVk_(const DescriptorSetLayout& descSetLayout, const DescriptorPool& pool) :
		DescriptorSet_(descSetLayout, pool) {}

	/*!*********************************************************************************************************************
	\brief Initialize this descriptor-set
	\return Return true on success
	***********************************************************************************************************************/
	bool init();

	/*!*********************************************************************************************************************
	\brief Update this descriptor set
	\param descSet Descriptor-set update param
	\return Return true on success
	***********************************************************************************************************************/
	bool update(const DescriptorSetUpdate& descSet);

	/*!*********************************************************************************************************************
	\brief dtor.
	***********************************************************************************************************************/
	virtual ~DescriptorSetVk_();
private:
};

class DescriptorPoolVk_;
}
namespace vulkan {
typedef EmbeddedRefCountedResource<vulkan::DescriptorPoolVk_> DescriptorPoolVk;
class DescriptorPoolVk_ : public EmbeddedRefCount<DescriptorPoolVk_>, public impl::DescriptorPool_, public native::HDescriptorPool_
{
	// Implementing EmbeddedRefCount
	template <typename> friend class ::pvr::EmbeddedRefCount;
public:
	/*!*********************************************************************************************************************
	\brief dtor
	***********************************************************************************************************************/
	virtual ~DescriptorPoolVk_();

	/*!*********************************************************************************************************************
	\brief Initialize this descriptor-pool
	\return Return true on success
	***********************************************************************************************************************/
	bool init(const DescriptorPoolCreateParam& createParam);

	/*!*********************************************************************************************************************
	\brief Destroy this descriptor-pool
	***********************************************************************************************************************/
	void destroy();

	/*!*********************************************************************************************************************
	\brief Create a new command-pool factory function
	\param ctx The GraphicsContext this commandpool will be created on.
	\return Return a new Commandpool
	***********************************************************************************************************************/
	static DescriptorPoolVk createNew(GraphicsContext& ctx){ return EmbeddedRefCount<DescriptorPoolVk_>::createNew(ctx); }
private:

	/*!*********************************************************************************************************************
	\brief ctor
	\param device The GraphicsContext who owns this pool.
	***********************************************************************************************************************/
	DescriptorPoolVk_(GraphicsContext& device) : DescriptorPool_(device) {}

	/* IMPLEMENTING EmbeddedResource */
	void destroyObject() { destroy(); }
};

typedef RefCountedResource<DescriptorSetVk_>DescriptorSetVk;
typedef RefCountedResource<DescriptorSetLayoutVk_>DescriptorSetLayoutVk;
}
}
}

PVR_DECLARE_NATIVE_CAST(DescriptorSet);
PVR_DECLARE_NATIVE_CAST(DescriptorPool);
PVR_DECLARE_NATIVE_CAST(DescriptorSetLayout);

namespace pvr {
namespace api {
namespace vulkan {
inline bool DescriptorSetVk_::init()
{
	VkDescriptorSetAllocateInfo allocInfo;
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.pSetLayouts = &native_cast(*getDescriptorSetLayout()).handle;
	allocInfo.descriptorSetCount = 1;
	allocInfo.descriptorPool = native_cast(*getDescriptorPool()).handle;
	return (vk::AllocateDescriptorSets(native_cast(*m_descSetLayout->getContext()).getDevice(), &allocInfo, &native_cast(this)->handle) == VK_SUCCESS);
}
}
}
}