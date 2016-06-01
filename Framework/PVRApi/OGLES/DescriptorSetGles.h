/*!*********************************************************************************************************************
\file         PVRApi\OGLES\DescriptorSetGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		Definition of the OpenGL ES implementation of the DescriptorSet and supporting classes
***********************************************************************************************************************/

#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRNativeApi/ApiErrors.h"
#include "PVRApi/ApiObjects/DescriptorSet.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRApi/OGLES/SamplerGles.h"
namespace pvr {
namespace api {
namespace gles {
/*!*********************************************************************************************************************
\brief ctor. OpenGL ES implementation of a DescriptorSet.
***********************************************************************************************************************/
class DescriptorSetLayoutGles_ : public native::HDescriptorSetLayout_, public impl::DescriptorSetLayout_
{
public:
	/*!*********************************************************************************************************************
	\brief ctor. Do not use directly, use context->createDescriptorSet.
	***********************************************************************************************************************/
	DescriptorSetLayoutGles_(GraphicsContext& context, const DescriptorSetLayoutCreateParam& desc) :
		DescriptorSetLayout_(context, desc) {}

	bool init() { return true; }

	/*!*********************************************************************************************************************
	\brief dtor.
	***********************************************************************************************************************/
	~DescriptorSetLayoutGles_() {}
};


inline void bindIndexedBuffer(const BufferView& view, IGraphicsContext& context, uint16 index, pvr::uint32 offset, GLenum type)
{
	const Buffer& buffer = view->getResource();
#ifdef GL_UNIFORM_BUFFER
	if (context.hasApiCapability(ApiCapabilities::Ubo))
	{
		const platform::ContextGles::BufferRange& lastBound = static_cast<platform::ContextGles&>(context).getBoundProgramBufferUbo(index);
		if (lastBound.buffer.isValid() && lastBound.offset == view->getOffset() + offset && lastBound.range == view->getRange() - offset && lastBound.buffer == buffer) { return; }
		static_cast<pvr::platform::ContextGles&>(context).onBindUbo(index, buffer, view->getOffset() + offset, view->getRange() - offset);

		gl::BindBufferRange(type, index, *buffer->getNativeObject(), view->getOffset() + offset, view->getRange() - offset);
		debugLogApiError("UboView_::bind exit");
	}
	else
#endif
	{
		assertion(0 , "UBO not supported from underlying API. No effect from UBO::bind");
		Log(Log.Warning, "UBO not supported from underlying API. No effect from UBO::bind");
	}
}


inline void bindTextureView(const TextureView& view, IGraphicsContext& context, uint16 bindIdx, const char* shaderVaribleName)
{
	platform::ContextGles& contextEs = static_cast<platform::ContextGles&>(context);

	const TextureStore& resource = view->getResource();

	if (resource.isNull())
	{
		Log("TextureView_::bind attempted to bind a texture with NULL native texture handle");
		return;
	}
	platform::ContextGles::RenderStatesTracker& renderStates = contextEs.getCurrentRenderStates();

	if (renderStates.texSamplerBindings[bindIdx].lastBoundTex == &(*view)) { return; }
	if (renderStates.lastBoundTexBindIndex != bindIdx)
	{
		gl::ActiveTexture(GL_TEXTURE0 + bindIdx);
	}

	gl::BindTexture(resource->getNativeObject().target, resource->getNativeObject().handle);
	debugLogApiError(strings::createFormatted("TextureView_::bind TARGET%x HANDLE%x",
	                 resource->getNativeObject().target, resource->getNativeObject().handle).c_str());
    contextEs.onBind(*view, bindIdx,shaderVaribleName);
}


/*!*********************************************************************************************************************
\brief OpenGL ES implementation of a DescriptorSet.
***********************************************************************************************************************/
class DescriptorSetGles_ : public impl::DescriptorSet_, public native::HDescriptorSet_
{
public:
	/*!*********************************************************************************************************************
	\brief ctor.
	***********************************************************************************************************************/
	DescriptorSetGles_(const DescriptorSetLayout& descSetLayout, const DescriptorPool& pool) :
		DescriptorSet_(descSetLayout, pool) {}

	bool update_(const pvr::api::DescriptorSetUpdate& descSet)
	{
		m_descParam = descSet;
		return true;
	}
	void bind(IGraphicsContext& device, pvr::uint32 dynamicOffset)const
	{
		platform::ContextGles& contextES = static_cast<platform::ContextGles&>(device);
		// bind the ubos
		for (pvr::uint16 j = 0; j < m_descParam.getBindingList().buffers.size(); ++j)
		{
			auto const& walk = m_descParam.getBindingList().buffers[j];
			if (walk.binding.isValid())
			{
				if (walk.type == types::DescriptorType::UniformBuffer || walk.type == types::DescriptorType::UniformBufferDynamic)
				{
					bindIndexedBuffer(walk.binding, device, walk.bindingId, dynamicOffset, GL_UNIFORM_BUFFER);
				}
#if defined(GL_SHADER_STORAGE_BUFFER)
				else if (walk.type == types::DescriptorType::StorageBuffer || walk.type == types::DescriptorType::StorageBufferDynamic)
				{
					bindIndexedBuffer(walk.binding, device, walk.bindingId, dynamicOffset, GL_SHADER_STORAGE_BUFFER);
				}
#endif
			}
		}
		// bind the combined texture and samplers
		for (pvr::uint16 j = 0; j < m_descParam.getBindingList().images.size(); ++j)
		{
            const DescriptorSetUpdate::ImageBinding& walk = m_descParam.getBindingList().images[j];

			if (!walk.binding.second.isNull())
			{
                bindTextureView(walk.binding.second, device, walk.bindingId,walk.shaderVariableName.c_str());//bind the texture
                if (walk.binding.first.useSampler && walk.binding.first.sampler.isNull())// bind the default sampler if necessary
				{
					static_cast<SamplerGles_&>(*contextES.getDefaultSampler()).bind(device, walk.bindingId);
				}
			}
			if (walk.binding.first.useSampler && !walk.binding.first.sampler.isNull())
			{
				static_cast<const SamplerGles_&>(*walk.binding.first.sampler).bind(device, walk.bindingId);// bind the sampler
			}
		}
	}

	bool init()
	{
		return m_descSetLayout.isValid() && m_descPool.isValid();
	}

	void destroy()
	{
		m_descParam.clear();
		m_descPool.reset();
		m_descSetLayout.reset();
	}

	/*!*********************************************************************************************************************
	\brief dtor.
	***********************************************************************************************************************/
	~DescriptorSetGles_() {}
	DescriptorSetUpdate m_descParam;
};


class DescriptorPoolGles_ : public impl::DescriptorPool_, public native::HDescriptorPool_
{
public:
	DescriptorPoolGles_(GraphicsContext& device) : DescriptorPool_(device) {}
	bool init(const DescriptorPoolCreateParam& createParam)
	{
		m_poolInfo = createParam;
		return true;
	}
private:
	DescriptorPoolCreateParam m_poolInfo;
};

typedef RefCountedResource<gles::DescriptorSetGles_> DescriptorSetGles;
typedef RefCountedResource<gles::DescriptorPoolGles_> DescriptorPoolGles;
typedef RefCountedResource<gles::DescriptorSetLayoutGles_> DescriptorSetLayoutGles;
}

}
}
