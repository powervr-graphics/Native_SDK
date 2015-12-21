/*!*********************************************************************************************************************
\file         PVRApi\Context.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Common implementation functions for the PVRCore::IGraphicsContext interface.
\cond NO_DOXYGEN
***********************************************************************************************************************/
#include "PVRCore/IGraphicsContext.h"
#include "PVRApi/ApiObjects/GraphicsPipeline.h"
#include "PVRApi/ApiObjects/ComputePipeline.h"
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRApi/ApiObjects/Sampler.h"
#include "PVRApi/ApiObjects/FboCreateParam.h"
#include "PVRApi/ApiObjects/Fbo.h"
#include "PVRApi/ApiObjects/RenderPass.h"
#include "PVRApi/ApiObjects/DescriptorTable.h"
#include "PVRApi/EffectApi.h"
namespace pvr {
api::GraphicsPipeline IGraphicsContext::createGraphicsPipeline(api::GraphicsPipelineCreateParam& desc)
{
	return createGraphicsPipeline(desc, api::ParentableGraphicsPipeline());
}

api::GraphicsPipeline IGraphicsContext::createGraphicsPipeline(api::GraphicsPipelineCreateParam& desc,
        api::ParentableGraphicsPipeline parent)
{
	api::GraphicsPipeline gp;
	api::impl::GraphicsPipelineImpl* gpi = new api::impl::GraphicsPipelineImpl(m_this_shared);
	Result::Enum result = (parent).isValid() ? gpi->init(desc, parent.get()) : gpi->init(desc);
	if (result == Result::Success)
	{
		gp.reset(gpi);
	}
	else
	{
		Log(Log.Error, "Failed to create graphics pipeline. Error value was: %s", Log.getResultCodeString(result));
		delete gpi;
	}
	return gp;
}

api::ComputePipeline IGraphicsContext::createComputePipeline(const api::ComputePipelineCreateParam& desc)
{
	api::ComputePipeline cp;
	cp.construct(m_this_shared);
	Result::Enum result = cp->init(desc);
	if (result != Result::Success)
	{
		Log(Log.Error, "Failed to create graphics pipeline. Error value was: %s",
		    Log.getResultCodeString(result));
		cp.release();
	}
	return cp;
}

api::ParentableGraphicsPipeline IGraphicsContext::createParentableGraphicsPipeline(const api::GraphicsPipelineCreateParam& desc)
{
	api::ParentableGraphicsPipeline gp;
	gp.construct(m_this_shared);
	Result::Enum result = gp->init(desc);
	if (result != Result::Success)
	{
		Log(Log.Error, "Failed to create parentable graphics pipeline. Error value was: %s",
		    Log.getResultCodeString(result));
		gp.release();
	}
	return gp;
}

api::PipelineLayout IGraphicsContext::createPipelineLayout(const api::PipelineLayoutCreateParam& desc)
{
	pvr::api::PipelineLayout pipelayout;
	pipelayout.construct(m_this_shared);
	if (pipelayout->init(desc) != pvr::Result::Success)
	{
		pipelayout.release();
	}
	return pipelayout;
}

api::Sampler IGraphicsContext::createSampler(const assets::SamplerCreateParam& desc)
{
	api::Sampler sampler;
	sampler.construct(m_this_shared, desc);
	return sampler;
}

api::EffectApi IGraphicsContext::createEffectApi(assets::Effect& effectDesc, api::GraphicsPipelineCreateParam& pipeDesc,
        api::AssetLoadingDelegate& effectDelegate)
{
	api::EffectApi effect;
	effect.construct(m_this_shared, effectDelegate);
	if (effect->init(effectDesc, pipeDesc) != Result::Success)
	{
		effect.release();
	}
	return effect;
}

api::TextureView IGraphicsContext::createTexture()
{
	api::TextureView tex;
	tex.construct(m_this_shared);
	return tex;
}

api::UboView IGraphicsContext::createUbo(const pvr::api::Buffer& buffer, pvr::uint32 offset, pvr::uint32 range)
{
	api::UboView ubo;
	if (hasApiCapability(ApiCapabilities::Ubo))
	{
		PVR_ASSERT(buffer->getBufferUsage() & api::BufferBindingUse::UniformBuffer);
		ubo.construct(buffer, offset, range);
	}
	else
	{
		pvr::Log("ubo not supported by this api");
	}
	return ubo;
}

api::SsboView IGraphicsContext::createSsbo(const api::Buffer& buffer, pvr::uint32 offset, pvr::uint32 range)
{
	api::SsboView ssbo;
	if (hasApiCapability(ApiCapabilities::Ssbo))
	{
		ssbo.construct(buffer, offset, range);
	}
	return ssbo;
}

api::Fbo IGraphicsContext::createFbo(const api::FboCreateParam& desc)
{
	api::Fbo fbo;
	// create fbo
	fbo.construct(desc, m_this_shared);
	if (fbo.isNull()) { fbo.release(); }
	return fbo;
}

api::RenderPass IGraphicsContext::createRenderPass(const api::RenderPassCreateParam& renderPass)
{
	api::RenderPass rp;
	rp.construct(m_this_shared);
	if (rp->init(renderPass))
	{
		rp.release();
	}
	return rp;
}

api::ColorAttachmentView IGraphicsContext::createColorAttachmentView(
    const api::ColorAttachmentViewCreateParam& createParam)
{
	api::ColorAttachmentView attachment;
	attachment.construct(m_this_shared);
	if (attachment->init(createParam) != Result::Success) {	attachment.release(); }
	return attachment;
}

api::DescriptorPool IGraphicsContext::createDescriptorPool(
    const api::DescriptorPoolCreateParam& createParam, api::DescriptorPoolUsage::Enum poolUsage)
{
	api::DescriptorPool descPool;
	descPool.construct(m_this_shared);
	if (descPool->init(createParam, poolUsage) != Result::Success) { descPool.release(); }
	return descPool;
}

api::DepthStencilView IGraphicsContext::createDepthStencilView(
    const api::DepthStencilViewCreateParam& createParam)
{
	api::DepthStencilView attachment;
	attachment.construct(api::FboAttachmentType::DepthStencil);
	if (attachment->init(createParam) != Result::Success) { attachment.release(); }
	return attachment;
}

api::DepthStencilView IGraphicsContext::createDepthView(const api::DepthStencilViewCreateParam& createParam)
{
	api::DepthStencilView attachment;
	attachment.construct(api::FboAttachmentType::Depth);
	if (attachment->init(createParam) != Result::Success) { attachment.release(); }
	return attachment;
}


api::CommandBuffer IGraphicsContext::createCommandBuffer()
{
	api::CommandBuffer commandBuffer;
	commandBuffer.construct(m_this_shared);
	return commandBuffer;
}

api::SecondaryCommandBuffer IGraphicsContext::createSecondaryCommandBuffer()
{
	api::SecondaryCommandBuffer commandBuffer;
	commandBuffer.construct(m_this_shared);
	return commandBuffer;
}

api::DescriptorSet IGraphicsContext::allocateDescriptorSet(const api::DescriptorSetLayout& layout)
{
	return allocateDescriptorSet(layout, m_defaultPool);
}

}
//!\endcond