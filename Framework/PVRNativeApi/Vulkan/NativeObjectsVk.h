/*!
\brief Contain Vulkan object wrappers and conversion from Framework object type to Vulkan.
\file PVRNativeApi/Vulkan/NativeObjectsVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#pragma once
#include "PVRCore/Interfaces/ForwardDecApiObjects.h"
#include "PVRNativeApi/Vulkan/HeadersVk.h"
namespace pvr {
namespace native {
struct VKdummy {};

#define DECLARE_NATIVE_TYPE(_framework_type_, _native_type_) \
struct _framework_type_\
{\
  typedef _native_type_ NativeType;\
  NativeType handle;\
  _framework_type_() : handle(_native_type_()) {} \
  _framework_type_(NativeType handle) : handle(handle) {} \
  operator const NativeType&() const { return handle; }\
  operator NativeType&() { return handle; }\
  const NativeType& operator*() const { return handle; }\
  NativeType& operator*() { return handle; }\
};

/// <summary>A native wrapper for VkBufferView</summary>
DECLARE_NATIVE_TYPE(HBufferView_, VkBufferView);

/// <summary>A native wrapper for VkImageView</summary>
DECLARE_NATIVE_TYPE(HDepthStencilView_, VkImageView);

/// <summary>A native wrapper for VkDescriptorPool</summary>
DECLARE_NATIVE_TYPE(HDescriptorPool_, VkDescriptorPool);

/// <summary>A native wrapper for VkDescriptorSet</summary>
DECLARE_NATIVE_TYPE(HDescriptorSet_, VkDescriptorSet);

/// <summary>A native wrapper for VkDescriptorSetLayout</summary>
DECLARE_NATIVE_TYPE(HDescriptorSetLayout_, VkDescriptorSetLayout);

/// <summary>A native wrapper for VkImageView</summary>
DECLARE_NATIVE_TYPE(HColorAttachmentView_, VkImageView);

/// <summary>A native wrapper for VkCommandBuffer</summary>
DECLARE_NATIVE_TYPE(HCommandBuffer_, VkCommandBuffer);

/// <summary>A native wrapper for VkCommandPool</summary>
DECLARE_NATIVE_TYPE(HCommandPool_, VkCommandPool);

/// <summary>A native wrapper for VkFramebuffer</summary>
DECLARE_NATIVE_TYPE(HFbo_, VkFramebuffer);

/// <summary>A native wrapper for VkPipeline</summary>
DECLARE_NATIVE_TYPE(HPipeline_, VkPipeline);

/// <summary>A native wrapper for VkPipelineLayout</summary>
DECLARE_NATIVE_TYPE(HPipelineLayout_, VkPipelineLayout);


/// <summary>A native wrapper for VkRenderPass</summary>
DECLARE_NATIVE_TYPE(HRenderPass_, VkRenderPass);

/// <summary>A native wrapper for VkSampler</summary>
DECLARE_NATIVE_TYPE(HSampler_, VkSampler);

/// <summary>A native wrapper for VkShaderModule</summary>
DECLARE_NATIVE_TYPE(HShader_, VkShaderModule);


/// <summary>A native wrapper for VkSemaphore</summary>
DECLARE_NATIVE_TYPE(HSemaphore_, VkSemaphore);

/// <summary>A native wrapper for VkEvent</summary>
DECLARE_NATIVE_TYPE(HEvent_, VkEvent);

/// <summary>A native wrapper for VkPipelineCache</summary>
DECLARE_NATIVE_TYPE(HPipelineCache_, VkPipelineCache);

/// <summary>A aggregated native wrapper for VkBuffer and VkDeviceMemory</summary>
struct HBuffer_
{
	VkBuffer        buffer;
	VkDeviceMemory  memory;
	HBuffer_() : buffer(VK_NULL_HANDLE), memory(VK_NULL_HANDLE) {}
};

/// <summary>A aggregated native wrapper for VkImage and VkDeviceMemory</summary>
struct HTexture_
{
	VkImage         image;
	VkDeviceMemory  memory;
	bool undeletable;
	HTexture_() : image(VK_NULL_HANDLE), memory(VK_NULL_HANDLE), undeletable(false) {}
};


/// <summary>A native wrapper for VkFence</summary>
struct HFence_
{
	typedef VkFence NativeType;
	NativeType handle;
	bool undeletable;
	HFence_(VkFence fence, bool undeletable) : handle(fence), undeletable(undeletable)
	{
	}
	HFence_() : handle(VK_NULL_HANDLE), undeletable(false)
	{
	}
	operator const NativeType& () const { return handle; }
	operator NativeType& () { return handle; }
	const NativeType& operator*() const { return handle; }
	NativeType& operator*() { return handle; }
};

/// <summary>A native wrapper for VkImageView</summary>
struct HImageView_
{
	typedef VkImageView NativeType;
	NativeType handle;
	bool undeletable;
	HImageView_() : handle(VkImageView()), undeletable(false) {}
	HImageView_(NativeType handle) : handle(handle), undeletable(false) {}
	HImageView_(NativeType handle, bool undeletable) : handle(handle), undeletable(undeletable) {}
	operator const NativeType& () const { return handle; }
	operator NativeType& () { return handle; }
	const NativeType& operator*() const { return handle; }
	NativeType& operator*() { return handle; }
};

/// <summary>A aggregated native wrapper for VkPhysicalDevice, VkDevice and VkInstance</summary>
struct HContext_
{
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkInstance instance;
};

#undef DECLARE_NATIVE_TYPE
}
}

#define PVR_DECLARE_NATIVE_CAST(_naked_name_) \
namespace pvr { namespace api { \
inline const vulkan::_naked_name_##Vk_& native_cast(const pvr::api::impl::_naked_name_##_& object) { return static_cast<const pvr::api::vulkan::_naked_name_##Vk_&>(object); } \
inline const vulkan::_naked_name_##Vk_* native_cast(const pvr::api::_naked_name_& object) { return &native_cast(*object); } \
inline const vulkan::_naked_name_##Vk_* native_cast(const pvr::api::impl::_naked_name_##_* object) { return &native_cast(*object); } \
inline vulkan::_naked_name_##Vk_& native_cast(pvr::api::impl::_naked_name_##_& object) { return static_cast<pvr::api::vulkan::_naked_name_##Vk_&>(object); } \
inline vulkan::_naked_name_##Vk_* native_cast(pvr::api::_naked_name_& object) { return &native_cast(*object); } \
inline vulkan::_naked_name_##Vk_* native_cast(pvr::api::impl::_naked_name_##_* object) { return &native_cast(*object); } \
} }

//!\endcond