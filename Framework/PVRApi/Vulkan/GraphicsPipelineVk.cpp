/*!*********************************************************************************************************************
\file         PVRApi\OGLES\GraphicsPipelineGles.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the OpenGL ES 2/3 implementation of the all-important pvr::api::GraphicsPipeline object.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/ApiObjects/GraphicsPipeline.h"
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
class ParentableGraphicsPipeline_;
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


class GraphicsPipelineImplementationDetails : public native::HPipeline_
{
public:
	struct PipelineRelation
	{
		enum Enum
		{
			Unrelated,
			Identity,
			Null_Null,
			Null_NotNull,
			NotNull_Null,
			Father_Child,
			Child_Father,
			Siblings
		};
	};

	PipelineLayout pipelineLayout;

	static PipelineRelation::Enum getRelation(GraphicsPipelineImplementationDetails* first, GraphicsPipelineImplementationDetails* second);

	VertexInputBindingInfo const* getInputBindingInfo(uint16 bindingId)const;
	VertexAttributeInfo const* getAttributesInfo(uint16 bindId)const;

	GraphicsPipelineImplementationDetails(GraphicsContext& context) : m_context(context), m_initialized(false) { }
	~GraphicsPipelineImplementationDetails() { destroy(); }

	void destroy();
	int32 getAttributeLocation(const char8* attribute);
	uint8 getNumAttributes(uint16 bindingId);
	ParentableGraphicsPipeline_* m_parent;
	vulkan::ContextVkWeakRef m_context;
	Result::Enum init(GraphicsPipelineCreateParam& desc, ParentableGraphicsPipeline_* parent = NULL);
	bool m_initialized;
	bool createPipeline();
};

GraphicsPipeline_::~GraphicsPipeline_()
{
	destroy();
}

inline GraphicsPipelineImplementationDetails::PipelineRelation::Enum GraphicsPipelineImplementationDetails::getRelation(GraphicsPipelineImplementationDetails* lhs,
        GraphicsPipelineImplementationDetails* rhs)
{
	if (lhs)
	{
		if (rhs)
		{
			GraphicsPipelineImplementationDetails* first = lhs;
			GraphicsPipelineImplementationDetails* firstFather = lhs->m_parent ? lhs->m_parent->pimpl.get() : NULL;
			GraphicsPipelineImplementationDetails* second = rhs;
			GraphicsPipelineImplementationDetails* secondFather = rhs->m_parent ? rhs->m_parent->pimpl.get() : NULL;
			return first == second ? PipelineRelation::Identity :
			       firstFather == second ? PipelineRelation::Child_Father :
			       firstFather == secondFather ? firstFather == NULL ? PipelineRelation::Unrelated : PipelineRelation::Siblings :
			       first == secondFather ? PipelineRelation::Father_Child :
			       PipelineRelation::Unrelated;
		}
		else { return PipelineRelation::NotNull_Null; }
	}
	else { return rhs ? PipelineRelation::Null_NotNull : PipelineRelation::Null_Null; }
}
inline void GraphicsPipelineImplementationDetails::destroy()
{
	if (m_context.isValid() && handle)
	{
		vk::DestroyPipeline(m_context->getDevice(), handle, NULL);
		handle = VK_NULL_HANDLE;
	}
	m_parent = 0;
}

const native::HPipeline_& GraphicsPipeline_::getNativeObject() const { return *pimpl; }

native::HPipeline_& GraphicsPipeline_::getNativeObject() { return *pimpl; }

void GraphicsPipeline_::destroy()
{
	return pimpl.reset();
}

GraphicsPipeline_::GraphicsPipeline_(GraphicsContext& context)
{
	pimpl.reset(new GraphicsPipelineImplementationDetails(context));
}

Result::Enum GraphicsPipeline_::init(const GraphicsPipelineCreateParam& desc, ParentableGraphicsPipeline_* parent)
{
	//pimpl->pipelineLayout = (desc.pipelineLayout.isValid() ? desc.pipelineLayout :
	//						 (parent ? parent->getPipelineLayout() : PipelineLayout()));
	pimpl->pipelineLayout = desc.pipelineLayout;
	if (!getPipelineLayout().isValid())
	{
		assertion(0, "Invalid PipelineLayout");
		return Result::UnknownError;
	}
	vulkan::GraphicsPipelineCreateInfoVulkan createInfoFactory(desc, pimpl->m_context, parent);
	return pvr::vkIsSuccessful(vk::CreateGraphicsPipelines(pimpl->m_context->getDevice(), VK_NULL_HANDLE, 1, &createInfoFactory.createInfo, NULL, &pimpl->handle),
	                           "Create GraphicsPipeline") ? Result::Success : Result::UnknownError;
}

int32 GraphicsPipeline_::getAttributeLocation(const char8* attribute)
{
	assertion(false, "VULKAN DOES NOT SUPPORT REFLECTION");
	return 0;
}

int32 GraphicsPipeline_::getUniformLocation(const char8* uniform)
{
	assertion(false, "VULKAN DOES NOT SUPPORT REFLECTION");
	return 0;
}

pvr::uint8 GraphicsPipeline_::getNumAttributes(pvr::uint16 bindingId)const
{
	assertion(false, "VULKAN DOES NOT SUPPORT SHADER/PIPELINE REFLECTION");
	return 0;
}

VertexInputBindingInfo const* GraphicsPipeline_::getInputBindingInfo(pvr::uint16 bindingId) const
{
	assertion(false, "VULKAN DOES NOT SUPPORT SHADER REFLECTION");
	return NULL;
}

VertexAttributeInfoWithBinding const* GraphicsPipeline_::getAttributesInfo(pvr::uint16 bindId)const
{
	assertion(false, "VULKAN DOES NOT SUPPORT SHADER REFLECTION");
	return NULL;
}

const pvr::api::PipelineLayout& GraphicsPipeline_::getPipelineLayout() const
{
	return pimpl->pipelineLayout;
}

Result::Enum ParentableGraphicsPipeline_::init(const GraphicsPipelineCreateParam& desc)
{
	pimpl->pipelineLayout = desc.pipelineLayout;
	if (!getPipelineLayout().isValid())
	{
		assertion(false, "Invalid PipelineLayout");
		return Result::UnknownError;
	}

	vulkan::GraphicsPipelineCreateInfoVulkan createInfoFactory(desc, pimpl->m_context, NULL);
	createInfoFactory.createInfo.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
	return pvr::vkIsSuccessful(vk::CreateGraphicsPipelines(pimpl->m_context->getDevice(), VK_NULL_HANDLE, 1,
	                           &createInfoFactory.createInfo, NULL, &pimpl->handle),
	                           "Create GraphicsPipeline") ? Result::Success : Result::UnknownError;
}
}
}
}
//!\endcond
