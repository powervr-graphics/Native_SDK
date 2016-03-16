/*!*********************************************************************************************************************
\file         PVRNativeApi\Vulkan\NativeObjectsVk.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contain Vulkan object wrappers and conversion from Framework object type to Vulkan.
***********************************************************************************************************************/
#pragma once
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

/*!
\brief A native wrapper for VkBufferView
*/
DECLARE_NATIVE_TYPE(HBufferView_, VkBufferView);

/*!
\brief A native wrapper for VkImageView
*/
DECLARE_NATIVE_TYPE(HDepthStencilView_, VkImageView);

/*!
\brief A native wrapper for VkDescriptorPool
*/
DECLARE_NATIVE_TYPE(HDescriptorPool_, VkDescriptorPool);

/*!
\brief A native wrapper for VkDescriptorSet
*/
DECLARE_NATIVE_TYPE(HDescriptorSet_, VkDescriptorSet);

/*!
\brief A native wrapper for VkDescriptorSetLayout
*/
DECLARE_NATIVE_TYPE(HDescriptorSetLayout_, VkDescriptorSetLayout);

/*!
\brief A native wrapper for VkImageView
*/
DECLARE_NATIVE_TYPE(HColorAttachmentView_, VkImageView);

/*!
\brief A native wrapper for VkCommandBuffer
*/
DECLARE_NATIVE_TYPE(HCommandBuffer_, VkCommandBuffer);

/*!
\brief A native wrapper for VkCommandPool
*/
DECLARE_NATIVE_TYPE(HCommandPool_, VkCommandPool);

/*!
\brief A native wrapper for VkFramebuffer
*/
DECLARE_NATIVE_TYPE(HFbo_, VkFramebuffer);

/*!
\brief A native wrapper for VkPipeline
*/
DECLARE_NATIVE_TYPE(HPipeline_, VkPipeline);

/*!
\brief A native wrapper for VkPipelineLayout
*/
DECLARE_NATIVE_TYPE(HPipelineLayout_, VkPipelineLayout);

/*!
\brief A native wrapper for VkRenderPass
*/
DECLARE_NATIVE_TYPE(HRenderPass_, VkRenderPass);

/*!
\brief A native wrapper for VkSampler
*/
DECLARE_NATIVE_TYPE(HSampler_, VkSampler);

/*!
\brief A native wrapper for VkShaderModule
*/
DECLARE_NATIVE_TYPE(HShader_, VkShaderModule);

/*!
\brief A native wrapper for VkFence
*/
DECLARE_NATIVE_TYPE(HFence_, VkFence);

/*!
\brief A native wrapper for VkSemaphore
*/
DECLARE_NATIVE_TYPE(HSemaphore_, VkSemaphore);

/*!
\brief A native wrapper for VkEvent
*/
DECLARE_NATIVE_TYPE(HEvent_, VkEvent);

/*!
\brief A native wrapper for VkPipelineCache
*/
DECLARE_NATIVE_TYPE(HPipelineCache_, VkPipelineCache);

/*!
\brief A aggregated native wrapper for VkBuffer and VkDeviceMemory
*/
struct HBuffer_
{
	VkBuffer        buffer;
	VkDeviceMemory  memory;
};

/*!
\brief A aggregated native wrapper for VkImage and VkDeviceMemory
*/
struct HTexture_
{
	VkImage         image;
	VkDeviceMemory  memory;
	bool undeletable;
	HTexture_() : undeletable(false) {}
};

/*!
\brief A native wrapper for VkImageView
*/
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

/*!
\brief A aggregated native wrapper for VkPhysicalDevice, VkDevice and VkInstance
*/
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
