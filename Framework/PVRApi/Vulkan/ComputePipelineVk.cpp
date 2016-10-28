/*!*********************************************************************************************************************
\file         PVRApi\Vulkan\ComputePipelineVk.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the OpenGL ES 2/3 implementation of the all-important pvr::api::GraphicsPipeline object.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/Vulkan/ComputePipelineVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRNativeApi/ShaderUtils.h"
#include "PVRApi/Vulkan/ShaderVk.h"
#include "PVRApi/Vulkan/ContextVk.h"
#include "PVRApi/Vulkan/PopulateVulkanCreateInfo.h"
//!\cond NO_DOXYGEN

namespace pvr {
namespace api {
namespace vulkan {
class PushPipeline;
class PopPipeline;
class ResetPipeline;
template<typename> class PackagedBindable;
template<typename, typename> class PackagedBindableWithParam;

class ParentableComputePipeline_;
//////IMPLEMENTATION INFO/////
/*The desired class hierarchy was
---- OUTSIDE INTERFACE ----
* ParentableGraphicsPipeline(PGP)			: GraphicsPipeline(GP)
-- Inside implementation --
* ParentableGraphicsPipelineGles(PGPGles)	: GraphicsPipelineGles(GPGles)
* GraphicsPipelineGles(GPGles)				: GraphicsPipeline(GP)
---------------------------
This would cause a diamond inheritance, with PGPGles inheriting twice from GP, once through PGP and once through GPGles.
To avoid this issue while maintaining the outside interface, the bridge pattern is used instead of the inheritance
chains commonly used for all other PVRApi objects. The same idiom is found in the CommandBuffer (for the same 
reasons: double inheritance between the CommandBuffer, SecondaryCommandBuffer, CommandBufferVk, SecondaryCommandBufferVk etc.) .
*/////////////////////////////


void ComputePipelineImplVk::destroy()
{
	if (m_context.isValid() && handle)
	{
		vk::DestroyPipeline(m_context->getDevice(), handle, NULL);
	}
	handle = VK_NULL_HANDLE;
}


bool ComputePipelineImplVk::init(const ComputePipelineCreateParam& desc)
{
	vulkan::ComputePipelineCreateInfoVulkan createInfoFactory(desc, m_context);
	m_desc = desc;
	return (vk::CreateComputePipelines(m_context->getDevice(), VK_NULL_HANDLE, 1, &createInfoFactory.createInfo, NULL, &handle) == VK_SUCCESS);
}

}
}
}
//!\endcond
