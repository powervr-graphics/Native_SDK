<<<<<<< HEAD
/*!*********************************************************************************************************************
\file         PVRApi\OGLES\DescriptorSetGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		Definition of the OpenGL ES implementation of the DescriptorSet and supporting classes
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
=======
/*!
\brief Definition of the OpenGL ES implementation of the DescriptorSet and supporting classes
\file PVRApi/OGLES/DescriptorSetGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
>>>>>>> 1776432f... 4.3
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRNativeApi/OGLES/ApiErrorsGles.h"
#include "PVRApi/ApiObjects/DescriptorSet.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRApi/OGLES/SamplerGles.h"
#include "PVRApi/OGLES/TextureGles.h"
namespace pvr {
namespace api {
namespace gles {


/// <summary>ctor. OpenGL ES implementation of a DescriptorSet.</summary>
class DescriptorSetLayoutGles_ : public native::HDescriptorSetLayout_, public impl::DescriptorSetLayout_
{
public:
	/// <summary>ctor. Do not use directly, use context->createDescriptorSet.</summary>
	DescriptorSetLayoutGles_(const GraphicsContext& context, const DescriptorSetLayoutCreateParam& desc) :
		DescriptorSetLayout_(context, desc) {}

	/// <summary>Initialize this descriptor-set-layout</summary>
	/// <returns>Return true on success.</returns>
	bool init();

<<<<<<< HEAD
inline void bindIndexedBuffer(const BufferView& view, IGraphicsContext& context, uint16 index, pvr::uint32 offset, GLenum type)
{
	const Buffer& buffer = view->getResource();
#ifdef GL_UNIFORM_BUFFER
	if (context.hasApiCapability(ApiCapabilities::Ubo))
	{
		const platform::ContextGles::BufferRange& lastBound = static_cast<platform::ContextGles&>(context).getBoundProgramBufferUbo(index);
		if (lastBound.buffer.isValid() && lastBound.offset == view->getOffset() + offset && lastBound.range == view->getRange() && lastBound.buffer == buffer) { return; }
		static_cast<pvr::platform::ContextGles&>(context).onBindUbo(index, buffer, view->getOffset() + offset, view->getRange());

		gl::BindBufferRange(type, index, *buffer->getNativeObject(), view->getOffset() + offset, view->getRange());
		debugLogApiError("UboView_::bind exit");
	}
	else
#endif
	{
		assertion(0 , "UBO not supported from underlying API. No effect from UBO::bind");
		Log(Log.Warning, "UBO not supported from underlying API. No effect from UBO::bind");
	}
}


inline void bindTextureView(const TextureView& view, IGraphicsContext& context, uint16 bindIdx)
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
	contextEs.onBind(*view, bindIdx);
}
=======
	/// <summary>Free all the resources held by this object</summary>
	void destroy();
>>>>>>> 1776432f... 4.3

	/// <summary>dtor.</summary>
	virtual ~DescriptorSetLayoutGles_();
};

/// <summary>OpenGL ES implementation of a DescriptorSet.</summary>
class DescriptorSetGles_ : public impl::DescriptorSet_, public native::HDescriptorSet_
{
public:
	/// <summary>ctor.</summary>
	DescriptorSetGles_(const DescriptorSetLayout& descSetLayout, const DescriptorPool& pool) :
		DescriptorSet_(descSetLayout, pool) {}

<<<<<<< HEAD
	bool update_(const pvr::api::DescriptorSetUpdate& descSet);
	void bind(IGraphicsContext& device, pvr::uint32*& dynamicOffsets)const
	{
		platform::ContextGles& contextES = static_cast<platform::ContextGles&>(device);
		// bind the buffers
		for (pvr::uint16 i = 0; i < pvr::types::DescriptorBindingDefaults::MaxStorageBuffers; ++i)
		{
			auto const& walk = m_descParam.getBindingList().storageBuffers[i];
			if (walk.binding.isValid())
			{
#if defined(GL_SHADER_STORAGE_BUFFER)
				if (pvr::types::getDescriptorTypeBinding(walk.descType) == pvr::types::DescriptorBindingType::StorageBuffer)
				{
					uint32 dynamicOffset = 0;
					if (walk.descType == types::DescriptorType::StorageBufferDynamic)
					{
						dynamicOffset = *dynamicOffsets++;
					}
					bindIndexedBuffer(walk.binding, device, walk.bindingId, dynamicOffset, GL_SHADER_STORAGE_BUFFER);
				}
#endif
			}
		}
		for (pvr::uint16 i = 0; i < pvr::types::DescriptorBindingDefaults::MaxUniformBuffers; ++i)
		{
			auto const& walk = m_descParam.getBindingList().uniformBuffers[i];
			if (walk.binding.isValid())
			{
				if (pvr::types::getDescriptorTypeBinding(walk.descType) == pvr::types::DescriptorBindingType::UniformBuffer)
				{
					uint32 dynamicOffset = 0;
					if (walk.descType == types::DescriptorType::UniformBufferDynamic)
					{
						dynamicOffset = *dynamicOffsets++;
					}
					bindIndexedBuffer(walk.binding, device, walk.bindingId, dynamicOffset, GL_UNIFORM_BUFFER);
				}
			}
		}
		// bind the combined texture and samplers
		for (pvr::uint16 i = 0; i < pvr::types::DescriptorBindingDefaults::MaxImages; ++i)
		{
			const DescriptorSetUpdate::ImageBinding& walk = m_descParam.getBindingList().images[i];

			if (!walk.binding.second.isNull())
			{
				bindTextureView(walk.binding.second, device, walk.bindingId); //bind the texture
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
};
=======
	void bind(IGraphicsContext& device, uint32*& dynamicOffsets)const;
>>>>>>> 1776432f... 4.3

	/// <summary>Initialize this descriptor-set</summary>
	/// <returns>Return true on success</returns>
	bool init();

	/// <summary>Free all the resources held by this object.</summary>
	void destroy();

	/// <summary>dtor.</summary>
	virtual ~DescriptorSetGles_();
private:
	bool update_(const DescriptorSetUpdate& descSet);
};

class DescriptorPoolGles_ : public impl::DescriptorPool_, public native::HDescriptorPool_
{
public:
	DescriptorPoolGles_(const GraphicsContext& device) : DescriptorPool_(device) {}

	/// <summary>dtor</summary>
	virtual ~DescriptorPoolGles_();

	/// <summary>Initialize this descriptor-pool</summary>
	/// <returns>Return true on success</returns>
	bool init(const DescriptorPoolCreateParam& createParam);

	/// <summary>Destroy this descriptor-pool</summary>
	void destroy();

private:
	DescriptorSet allocateDescriptorSet_(const DescriptorSetLayout& layout);
	DescriptorPoolCreateParam _createParam;
};

typedef RefCountedResource<gles::DescriptorSetGles_> DescriptorSetGles;
typedef RefCountedResource<gles::DescriptorPoolGles_> DescriptorPoolGles;
typedef RefCountedResource<gles::DescriptorSetLayoutGles_> DescriptorSetLayoutGles;
}

}
}
<<<<<<< HEAD
//!\endcond
=======
PVR_DECLARE_NATIVE_CAST(DescriptorSet);
PVR_DECLARE_NATIVE_CAST(DescriptorPool);
>>>>>>> 1776432f... 4.3
