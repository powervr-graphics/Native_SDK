/*!
\brief OpenGL ES implementation of the DescriptorPool class.
\file PVRApi/OGLES/DescriptorSetGles.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRApi/OGLES/DescriptorSetGles.h"
<<<<<<< HEAD
namespace {
using namespace pvr;
template<typename type>
bool validateDescriptorBinding(const types::DescriptorBinding<type>& updateBinding,
=======
#include "PVRApi/OGLES/BufferGles.h"
#include "PVRApi/OGLES/TextureGles.h"
namespace {
using namespace pvr;
template<typename type>
bool validateDescriptorBinding(const types::DescriptorItemBinding<type>& updateBinding,
>>>>>>> 1776432f... 4.3
                               const types::DescriptorBindingLayout& layoutBinding)
{
	if (!layoutBinding.isValid())
	{
		std::string msg = strings::createFormatted("DescriptorsetUpdate descriptor type does not match with the layout for binding %d",
<<<<<<< HEAD
		                  updateBinding.bindingId);
=======
		                  updateBinding._bindingId);
>>>>>>> 1776432f... 4.3
		assertion(false, msg.c_str());
		Log(msg.c_str());
		return false;
	}
<<<<<<< HEAD
	if (updateBinding.arrayIndex > (int16)layoutBinding.arraySize)
	{
		std::string msg = strings::createFormatted("DescriptorsetUpdate array index is %d but the layout only supports array size %d",
		                  updateBinding.bindingId, layoutBinding.arraySize);
=======
	if (updateBinding._arrayIndex > (int16)layoutBinding._arraySize)
	{
		std::string msg = strings::createFormatted("DescriptorsetUpdate array index is %d but the layout only supports array size %d",
		                  updateBinding._bindingId, layoutBinding._arraySize);
>>>>>>> 1776432f... 4.3
		assertion(false, msg.c_str());
		Log(msg.c_str());
		return false;
	}
	return true;
}
<<<<<<< HEAD
=======

>>>>>>> 1776432f... 4.3
}

namespace pvr {
namespace api {

// DESCRIPTOR POOL
namespace gles {

inline void bindIndexedBuffer(const BufferView& view, IGraphicsContext& context, uint16 index, uint32 offset, GLenum type)
{
	const Buffer& buffer = view->getResource();
#ifdef GL_UNIFORM_BUFFER
	if (context.hasApiCapability(ApiCapabilities::Ubo))
	{
		const platform::ContextGles::BufferRange& lastBound = native_cast(context).getBoundProgramBufferUbo(index);
		if (lastBound.buffer.isValid() && lastBound.offset == view->getOffset() + offset && lastBound.range == view->getRange() && lastBound.buffer == buffer) { return; }
		native_cast(context).onBindUbo(index, view->getResource(), view->getOffset() + offset, view->getRange());

		gl::BindBufferRange(type, index, native_cast(*buffer).handle, view->getOffset() + offset, view->getRange());
		debugLogApiError("UboView_::bind exit");
	}
	else
#endif
	{
		assertion(0, "UBO not supported from underlying API. No effect from UBO::bind");
		Log(Log.Warning, "UBO not supported from underlying API. No effect from UBO::bind");
	}
}

inline void bindTextureView(const TextureView& view, IGraphicsContext& context, uint16 bindIdx)
{
	const TextureStore& resource = view->getResource();

	if (resource.isNull())
	{
		Log("TextureView_::bind attempted to bind a texture with NULL native texture handle");
		return;
	}
	native_cast(*resource).bind(context, bindIdx);
}

inline void bindImage(const TextureView& view, IGraphicsContext& context, uint16 bindIdx)
{
	const TextureStore& resource = view->getResource();

	if (resource.isNull())
	{
		Log("TextureView_::bind attempted to bind a texture with NULL native texture handle");
		return;
	}
	native_cast(*resource).bindImage(context, bindIdx);
}


bool DescriptorPoolGles_::init(const DescriptorPoolCreateParam& createParam)
{
	_createParam = createParam;
	return true;
}

void DescriptorPoolGles_::destroy()
{
	_createParam = pvr::api::DescriptorPoolCreateParam();
}

DescriptorPoolGles_::~DescriptorPoolGles_()
{
	if (getContext().isValid())
	{
		destroy();
	}
	else
	{
		Log(Log.Warning, "Attempted to free DescriptorPool after its corresponding context was destroyed.");
	}
}

api::DescriptorSet DescriptorPoolGles_::allocateDescriptorSet_(const api::DescriptorSetLayout& layout)
{
	//For OpenGL ES, DescriptorPool is dummy.
	return getContext()->createDescriptorSetOnDefaultPool(layout);
}
}

// DESCRIPTOR SET
namespace gles {

void DescriptorSetGles_::bind(IGraphicsContext& device, uint32*& dynamicOffsets)const
{
	platform::ContextGles& contextES = native_cast(device);
	// bind the buffers
#if defined(GL_SHADER_STORAGE_BUFFER)
	for (uint16 i = 0; i < _descParam.getSsboCount(); ++i)
	{
		auto const& walk = _descParam.getSsbos()[i];
		if (walk._binding.isValid())
		{
			if (types::getDescriptorTypeBinding(walk._descType) == types::DescriptorBindingType::StorageBuffer)
			{
				uint32 dynamicOffset = 0;
				if (walk._descType == types::DescriptorType::StorageBufferDynamic)
				{
					dynamicOffset = *dynamicOffsets++;
				}
				bindIndexedBuffer(walk._binding, device, walk._bindingId, dynamicOffset, GL_SHADER_STORAGE_BUFFER);
			}
		}
	}
#endif
	for (uint16 i = 0; i < _descParam.getUboCount(); ++i)
	{
		auto const& walk = _descParam.getUbos()[i];
		if (walk._binding.isValid())
		{
			if (types::getDescriptorTypeBinding(walk._descType) == types::DescriptorBindingType::UniformBuffer)
			{
				uint32 dynamicOffset = 0;
				if (walk._descType == types::DescriptorType::UniformBufferDynamic)
				{
					dynamicOffset = *dynamicOffsets++;
				}
				bindIndexedBuffer(walk._binding, device, walk._bindingId, dynamicOffset, GL_UNIFORM_BUFFER);
			}
		}
	}
	// bind the combined texture and samplers
	for (uint16 i = 0; i < _descParam.getImageCount(); ++i)
	{
		auto const& walk = _descParam.getImages()[i];
		if (walk._descType != types::DescriptorType::StorageImage)
		{
			if (!walk._binding.second.isNull())
			{
				bindTextureView(walk._binding.second, device, walk._bindingId); //bind the texture
				if (walk._binding.first._useSampler && walk._binding.first._sampler.isNull())// bind the default sampler if necessary
				{
					static_cast<SamplerGles_&>(*contextES.getDefaultSampler()).bind(device, walk._bindingId);
				}
			}
			if (walk._binding.first._useSampler && !walk._binding.first._sampler.isNull())
			{
				static_cast<const SamplerGles_&>(*walk._binding.first._sampler).bind(device, walk._bindingId);// bind the sampler
			}
		}
		else
		{
			bindImage(walk._binding.second, device, walk._bindingId); //bind the texture
		}
	}
}

bool DescriptorSetGles_::init()
{
	return _descSetLayout.isValid() && _descPool.isValid();
}

void DescriptorSetGles_::destroy()
{
	_descParam.clear();
	_descPool.reset();
	_descSetLayout.reset();
}

DescriptorSetGles_::~DescriptorSetGles_()
{
	if (_descPool.isValid())
	{
		if (_descPool->getContext().isValid())
		{
			destroy();
		}
		else
		{
			Log(Log.Warning, "Attempted to free DescriptorSet after its corresponding device was destroyed");
		}
	}
	else
	{
		Log(Log.Warning, "Attempted to free DescriptorSet after its corresponding pool was destroyed");
	}
}

bool DescriptorSetGles_::update_(const DescriptorSetUpdate& descSet)
{
#ifdef DEBUG
	// validate
	const api::DescriptorSetLayoutCreateParam& layoutInfo = _descSetLayout->getCreateParam();

	if (layoutInfo.getSsboCount() != descSet.getSsboCount())
	{
		return false;
	}
	for (uint32 i = 0; i < layoutInfo.getSsboCount(); ++i)
	{
		auto& layoutBinding = layoutInfo.getSsbos()[i];
		auto& updateBinding = descSet.getSsbos()[i];

		if (layoutBinding.isValid() && updateBinding.isValid())
		{
			if (!validateDescriptorBinding(updateBinding, layoutBinding)) { return false; }
		}
		else
		{
			return false;
		}
	}

	if (layoutInfo.getUboCount() != descSet.getUboCount())
	{
		return false;
	}
	for (uint32 i = 0; i < layoutInfo.getUboCount(); ++i)
	{
		auto& layoutBinding = layoutInfo.getUbos()[i];
		auto& updateBinding = descSet.getUbos()[i];

		if (layoutBinding.isValid() && updateBinding.isValid())
		{
			if (!validateDescriptorBinding(updateBinding, layoutBinding)) { return false; }
		}
		else
		{
			return false;
		}
	}

	if (layoutInfo.getImageCount() != descSet.getImageCount())
	{
		return false;
	}
	for (uint32 i = 0; i < layoutInfo.getImageCount(); ++i)
	{
		auto& layoutBinding = layoutInfo.getImages()[i];
		auto& updateBinding = descSet.getImages()[i];

		if (layoutBinding.isValid() && updateBinding.isValid())
		{
			if (!validateDescriptorBinding(updateBinding, layoutBinding)) { return false; }
		}
		else
		{
			return false;
		}
	}
#endif
	_descParam = descSet;
	return true;
}
}

// DESCRIPTOR SET LAYOUT
namespace gles {

bool DescriptorSetLayoutGles_::init()
{
	return true;
}

<<<<<<< HEAD
bool gles::DescriptorSetGles_::update_(const DescriptorSetUpdate& descSet)
{
#ifdef DEBUG
	// validate
	const api::DescriptorSetLayoutCreateParam& layoutInfo = m_descSetLayout->getCreateParam();
	const DescriptorSetUpdate::Bindings& updateBindings = descSet.getBindingList();
	for (uint32 i = 0; i < pvr::types::DescriptorBindingDefaults::MaxStorageBuffers; ++i)
	{
		if (updateBindings.storageBuffers[i].isValid())
		{
			auto& binding = layoutInfo.getBinding(updateBindings.storageBuffers[i].bindingId, updateBindings.storageBuffers[i].descType);
			if (!validateDescriptorBinding(updateBindings.storageBuffers[i], binding)) { return false; }
		}
	}
	for (uint32 i = 0; i < pvr::types::DescriptorBindingDefaults::MaxUniformBuffers; ++i)
	{
		if (updateBindings.uniformBuffers[i].isValid())
		{
			auto& binding = layoutInfo.getBinding(updateBindings.uniformBuffers[i].bindingId, updateBindings.uniformBuffers[i].descType);
			if (!validateDescriptorBinding(updateBindings.uniformBuffers[i], binding)) { return false; }
		}
	}
	for (uint32 i = 0; i < pvr::types::DescriptorBindingDefaults::MaxImages; ++i)
	{
		if (updateBindings.images[i].binding.second.isValid() && updateBindings.images[i].isValid())
		{
			auto& binding = layoutInfo.getBinding(updateBindings.images[i].bindingId, updateBindings.images[i].descType);
			if (!validateDescriptorBinding(updateBindings.images[i], binding))
			{
				return false;
			}
		}
	}
#endif
	m_descParam = descSet;
	return true;
}
=======
void DescriptorSetLayoutGles_::destroy()
{
	if (getContext().isValid())
	{
		getContext().reset();
	}
	clearCreateParam();
>>>>>>> 1776432f... 4.3
}

DescriptorSetLayoutGles_::~DescriptorSetLayoutGles_()
{
	if (getContext().isValid())
	{
		destroy();
	}
	else
	{
		Log(Log.Warning, "Attempted to free DescriptorSetLayout after its corresponding device was destroyed");
	}
}
}
}
}
