/*!
\brief Definition of the Vulkan implementation of the DescriptorSet and supporting classes
\file PVRApi/Vulkan/DescriptorSetVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiObjects/DescriptorSet.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRApi/Vulkan/ContextVk.h"
#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"
namespace pvr {
namespace api {
namespace vulkan {

/// <summary>ctor. Vulkan implementation of a DescriptorSet.</summary>
class DescriptorSetLayoutVk_ : public impl::DescriptorSetLayout_, public native::HDescriptorSetLayout_
{
public:
	/// <summary>ctor. Do not use directly, use context->createDescriptorSet.</summary>
	/// <param name="context">The GraphicsContext who owns this descriptor-set-layout.</param>
	/// <param name="desc">This descriptor-set-layout create param</param>
	DescriptorSetLayoutVk_(const GraphicsContext& context, const DescriptorSetLayoutCreateParam& desc) :
		DescriptorSetLayout_(context, desc)
	{
		if (!context->getPlatformContext().isRayTracingSupported())
		{
			debug_assertion(desc.getIndirectRayPipelineCount() == 0,
			                "Context does not support ray tracing");
		}
	}

	/// <summary>Initialize this descriptor-set-layout</summary>
	/// <returns>Return true on success.</returns>
	bool init();

	/// <summary>Free all the resources held by this object</summary>
	void destroy();

	/// <summary>Destructor</summary>
	virtual ~DescriptorSetLayoutVk_();
};

/// <summary>Vulkan implementation of a DescriptorSet.</summary>
class DescriptorSetVk_ : public impl::DescriptorSet_, public native::HDescriptorSet_
{
public:
	/// <summary>ctor.</summary>
	/// <param name="descSetLayout">The DescriptorSetLayout of this descriptor set</param>
	/// <param name="pool">The DescriptorPool this descriptor-set will be allocated from.</param>
	DescriptorSetVk_(const DescriptorSetLayout& descSetLayout, const DescriptorPool& pool) :
		DescriptorSet_(descSetLayout, pool) {}

	/// <summary>Initialize this descriptor-set</summary>
	/// <returns>Return true on success</returns>
	bool init();

	/// <summary>Free all the resources held by this object.</summary>
	void destroy();

	/// <summary>dtor.</summary>
	virtual ~DescriptorSetVk_();
private:
	bool update_(const DescriptorSetUpdate& descSet);
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
	/// <summary>dtor</summary>
	virtual ~DescriptorPoolVk_();

	/// <summary>Initialize this descriptor-pool</summary>
	/// <returns>Return true on success</returns>
	bool init(const DescriptorPoolCreateParam& createParam);

	/// <summary>Destroy this descriptor-pool</summary>
	void destroy();

	/// <summary>Create a new command-pool factory function</summary>
	/// <param name="ctx">The GraphicsContext this commandpool will be created on.</param>
	/// <returns>Return a new Commandpool</returns>
	static DescriptorPoolVk createNew(const GraphicsContext& ctx) { return EmbeddedRefCount<DescriptorPoolVk_>::createNew(ctx); }
private:

	/// <summary>ctor</summary>
	/// <param name="device">The GraphicsContext who owns this pool.</param>
	DescriptorPoolVk_(const GraphicsContext& device) : DescriptorPool_(device) {}

	/* IMPLEMENTING EmbeddedResource */
	void destroyObject() { destroy(); }
	api::DescriptorSet allocateDescriptorSet_(const DescriptorSetLayout& layout);
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
	allocInfo.pSetLayouts = &pvr::api::native_cast(*getDescriptorSetLayout()).handle;
	allocInfo.descriptorSetCount = 1;
	allocInfo.descriptorPool = pvr::api::native_cast(*getDescriptorPool()).handle;
	return (vk::AllocateDescriptorSets(pvr::api::native_cast(*_descSetLayout->getContext()).getDevice(),
	                                   &allocInfo, &pvr::api::native_cast(this)->handle) == VK_SUCCESS);
}
}
}
}
