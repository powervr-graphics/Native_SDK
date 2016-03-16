/*!*********************************************************************************************************************
\file         PVRApi\OGLES\ComputePipelineVk.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the OpenGL ES 2/3 implementation of the all-important pvr::api::GraphicsPipeline object.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/ApiObjects/ComputePipeline.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRNativeApi/ShaderUtils.h"
#include "PVRApi/Vulkan/ShaderVk.h"
#include "PVRApi/Vulkan/ContextVk.h"
#include "PVRNativeApi/Vulkan/PopulateVulkanCreateInfo.h"

namespace pvr {
namespace api {
namespace impl {
//!\cond NO_DOXYGEN
class PushPipeline;
class PopPipeline;
class ResetPipeline;
template<typename> class PackagedBindable;
template<typename, typename> class PackagedBindableWithParam;

class ParentableComputePipeline_;
//!\endcond

//////IMPLEMENTATION INFO/////
/*The desired class hierarchy was
---- OUTSIDE INTERFACE ----
* ParentableGraphicsPipeline(PGP)			: GraphicsPipeline(GP)
-- Inside implementation --
* ParentableGraphicsPipelineGles(PGPGles)	: GraphicsPipelineGles(GPGles)
* GraphicsPipelineGles(GPGles)				: GraphicsPipeline(GP)
---------------------------
This would cause a diamond inheritance, with PGPGles inheriting twice from GP, once through PGP and once through GPGles.
To avoid this issue while maintaining the outside interface, the pImpl idiom is being used instead of the inheritance
chains commonly used for all other PVRApi objects. The same idiom (for the same reasons) is found in the CommandBuffer.
*/////////////////////////////


class ComputePipelineImplementationDetails : public native::HPipeline_
{
public:
	ComputePipelineImplementationDetails(GraphicsContext& context) : m_context(context), m_initialized(false) { }
	~ComputePipelineImplementationDetails() { destroy(); }

	void destroy();
	PipelineLayout pipelineLayout;
	ParentableComputePipeline_* m_parent;
	vulkan::ContextVkWeakRef m_context;
	Result::Enum init(ComputePipelineCreateParam& desc, ParentableComputePipeline_* parent = NULL);
	bool m_initialized;
	bool createPipeline();
};

ComputePipeline_::~ComputePipeline_() { destroy(); }


inline void ComputePipelineImplementationDetails::destroy()
{
	vk::DestroyPipeline(m_context->getDevice(), handle, NULL);
	m_parent = 0;
}

const native::HPipeline_& ComputePipeline_::getNativeObject() const { return *pimpl; }

native::HPipeline_& ComputePipeline_::getNativeObject() { return *pimpl; }

void ComputePipeline_::destroy() { return pimpl->destroy(); }

ComputePipeline_::ComputePipeline_(GraphicsContext& context)
{
	pimpl.reset(new ComputePipelineImplementationDetails(context));
}

Result::Enum ComputePipeline_::init(const ComputePipelineCreateParam& desc)
{
	vulkan::ComputePipelineCreateInfoVulkan createInfoFactory(desc, pimpl->m_context);
	pimpl->pipelineLayout = desc.pipelineLayout;
	vk::CreateComputePipelines(pimpl->m_context->getDevice(), VK_NULL_HANDLE, 1, &createInfoFactory.createInfo, NULL, &pimpl->handle);
	return Result::Success;
}

const pvr::api::PipelineLayout& ComputePipeline_::getPipelineLayout() const
{
	return pimpl->pipelineLayout;
}
}
}
}
//!\endcond
