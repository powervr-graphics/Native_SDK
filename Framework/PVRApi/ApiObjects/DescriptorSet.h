/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\DescriptorSet.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the DescriptorSet class.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRApi/ApiObjects/Buffer.h"
#include "PVRApi/ApiObjects/Sampler.h"
#include "PVRApi/ApiObjects/GraphicsPipeline.h"
#include "PVRApi/ApiObjects/PipelineConfig.h"
namespace pvr {
namespace api {

class BindDescriptorSets;

/*!*********************************************************************************************************************
\brief Contains all information required to create a Descriptor Set Layout. This is the number of Textures, Samplers, Uniform
      Buffer Objects, and Shader Storage Buffer Objects bound for any shader stage.
***********************************************************************************************************************/
struct DescriptorSetLayoutCreateParam
{
public:
	struct BindInfo
	{
		types::DescriptorType::Enum descType;
		types::ShaderStageFlags::Enum shaderStage;
		pvr::uint32 arraySize;
		BindInfo() {}
		BindInfo(types::DescriptorType::Enum descType, types::ShaderStageFlags::Enum shaderStage, pvr::uint32 arraySize) :
			descType(descType), shaderStage(shaderStage), arraySize(arraySize) {}
	};
	/*!*********************************************************************************************************************
	\brief Set the buffer binding of Descriptor Objects in the specified shader stages.
	\param[in] bindIndex The index to which the binding will be added
	\param[in] descType The type of descriptor
	\param[in] arraySize size of the array
	\param[in] stageFlags The shader stages for which the number of bindings is set to (count)
	\return this (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetLayoutCreateParam& setBinding(pvr::uint32 bindIndex, types::DescriptorType::Enum descType, pvr::uint32 arraySize = 1,
	    types::ShaderStageFlags::Enum stageFlags = types::ShaderStageFlags::AllGraphicsStages)
	{
		assertion(arraySize != 0, "Array size cannot be zero");
		if (bindIndex >= m_bindings.size())
		{
			m_bindings.resize(bindIndex + 1);
		}
		m_bindings[bindIndex] = BindInfo(descType, stageFlags, arraySize);
		return *this;
	}

	void clear()
	{
		m_bindings.clear();
	}
private:

	friend class ::pvr::api::impl::DescriptorSetLayout_;
	friend class ::pvr::api::BindDescriptorSets;
	friend struct ::pvr::api::DescriptorSetUpdate;
	std::vector<BindInfo> m_bindings;
};

namespace impl {
/*!*********************************************************************************************************************
\brief API DescriptorSetLayout. Use through the Reference Counted Framework Object pvr::api::DescriptorSetLayout. Create using
       the IGraphicsContext::createDescriptorSetLayout. A Descriptor Set Layout is required both to construct a descriptor
	   set object, and a Pipeline compatible with this object.
***********************************************************************************************************************/
class DescriptorSetLayout_
{
protected:
	friend class ::pvr::api::BindDescriptorSets;
	template<typename> friend struct ::pvr::RefCountEntryIntrusive;
	bool init();
	DescriptorSetLayout_& operator=(const DescriptorSetLayout_&);
	std::vector<DescriptorSetLayoutCreateParam::BindInfo>& getDescriptorDescBindings() { return desc.m_bindings; }

	DescriptorSetLayoutCreateParam desc;
	GraphicsContext device;
	DescriptorSetLayout_(GraphicsContext& context, const DescriptorSetLayoutCreateParam& desc) : desc(desc), device(context) {}
public:

	/*!*********************************************************************************************************************
	\brief Get the DescriptorSetCreateParam object that was used to create this layout.
	\return The DescriptorSetCreateParam object that was used to create this layout.
	***********************************************************************************************************************/
	const DescriptorSetLayoutCreateParam& getCreateParam()const { return desc; }

	/*!*********************************************************************************************************************
	\brief Get the context that this layout belongs to.
	\return The context that this layout belongs to.
	***********************************************************************************************************************/
	GraphicsContext& getContext() { return device; }

	const GraphicsContext& getContext()const { return device; }

	virtual ~DescriptorSetLayout_() { }

	const native::HDescriptorSetLayout_& getNativeObject()const;

	native::HDescriptorSetLayout_& getNativeObject();
};
}// impl

/*!*********************************************************************************************************************
\brief  Descriptor Pool create parameter.
***********************************************************************************************************************/
struct DescriptorPoolCreateParam
{
private:
	std::map<types::DescriptorType::Enum, pvr::uint32> descriptorType;
	pvr::uint32 maxSets;
public:
	DescriptorPoolCreateParam() : maxSets(200) {}

	/*!*********************************************************************************************************************
	\brief Add the maximum number of the specified descriptor types that the pool will contain.
	\param[in] descType Descriptor type
	\param[in] count Maximum number of descriptors of (type)
	\return this (allow chaining)
	***********************************************************************************************************************/
	DescriptorPoolCreateParam& addDescriptorInfo(types::DescriptorType::Enum descType, pvr::uint16 count)
	{
		descriptorType[descType] = count;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set the maximum number of descriptor sets.
	\param[in] maxSets The maximum number of descriptor sets
	\return this (allow chaining)
	***********************************************************************************************************************/
	DescriptorPoolCreateParam& setMaxDescriptorSets(pvr::uint32 maxSets)
	{
		this->maxSets = maxSets; return *this;
	}

	pvr::uint32 getDescriptorTypeCount(types::DescriptorType::Enum descType) const
	{
		std::map<types::DescriptorType::Enum, pvr::uint32>::const_iterator found = descriptorType.find(descType);
		return (found != descriptorType.end() ? found->second  : 0);
	}

	pvr::uint32 getMaxSets() const { return maxSets; }

};

namespace impl {
/*!*********************************************************************************************************************
\brief API DescriptorPool Object wrapper. Access through the framework-managed DescriptorPool object.
***********************************************************************************************************************/
class DescriptorPool_
{
protected:
	friend class ::pvr::IGraphicsContext;
	GraphicsContext m_context;
	types::DescriptorPoolUsage::Enum m_usage;
	DescriptorPool_& operator=(const DescriptorPool_&);
public:
	const GraphicsContext& getContext()const { return m_context; }
	GraphicsContext& getContext() { return m_context; }
	/*!*********************************************************************************************************************
	\brief Constructor. Do not use directly.
	\param[in] device
	***********************************************************************************************************************/
	DescriptorPool_(const GraphicsContext& device) : m_context(device) {}

	api::DescriptorSet allocateDescriptorSet(const DescriptorSetLayout& layout);

	/*!*********************************************************************************************************************
	\brief Get native descriptorpool handle.
	\return native handle
	***********************************************************************************************************************/
	const native::HDescriptorPool_& getNativeObject()const;
	native::HDescriptorPool_& getNativeObject();
};

}//namespace impl

/*!*********************************************************************************************************************
\brief This class contains all the information necessary to populate a Descriptor Set with the actual API objects. Use with
       the method update of the DescriptorSet. Populate this object with actual Descriptor objects (UBOs, textures etc).
***********************************************************************************************************************/
struct DescriptorSetUpdate
{
	template<class _Binding>
	struct DescriptorBinding
	{
		_Binding    binding;
		pvr::int16 bindingId;
		pvr::uint16 arrayIndex;
		types::DescriptorType::Enum type;
		DescriptorBinding(pvr::uint16 bindingId, pvr::uint16 index, types::DescriptorType::Enum type, const _Binding& obj) :
			binding(obj), bindingId(bindingId), type(type), arrayIndex(index) {}
		DescriptorBinding() : bindingId(-1) {}
	};
	typedef DescriptorBinding<pvr::api::BufferView> BufferViewBinding;
	typedef std::pair<pvr::api::Sampler, pvr::api::TextureView/**/> CombinedImageSampler;
	typedef DescriptorBinding<CombinedImageSampler> CombImageSamplerBinding;
	typedef std::vector<BufferViewBinding> BufferBindingList;
	typedef std::vector<CombImageSamplerBinding> CombImageSamplerBindingList;
	struct Bindings
	{
		BufferBindingList buffers;
		CombImageSamplerBindingList combinedSamplerImage;
	};

	/*!*********************************************************************************************************************
	\brief     Constructor.
	***********************************************************************************************************************/
	DescriptorSetUpdate(types::DescriptorSetUsage::Enum usage = types::DescriptorSetUsage::Static) :  m_usage(usage) {}

	/*!*********************************************************************************************************************
	\brief	Add a Ubo to the specified binding index.
	\param	bindingId The index of the indexed binding point
	\param	item The object to add
	\return	this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setUbo(pvr::uint16 bindingId, const api::BufferView& item)
	{
		return addBuffer(bindingId, 0, types::DescriptorType::UniformBuffer, item);
	}

	/*!*********************************************************************************************************************
	\brief	Add a Ubo to the specified binding index. Supports array-indexing in the shader.
	\param	bindingId The index of the indexed binding point
	\param	arrayIndex If supported by the underlying API, add to which index of the array binding point.
	\param	item The object to add
	\return	this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setUboAtIndex(pvr::uint16 bindingId, pvr::uint8 arrayIndex, const api::BufferView& item)
	{
		return addBuffer(bindingId, arrayIndex, types::DescriptorType::UniformBuffer, item);
	}

	/*!*********************************************************************************************************************
		\brief	Add a Ubo to the specified binding index.
		\param	bindingId The index of the indexed binding point
		\param	item The object to add
		\return	this object (allow chaining)
		***********************************************************************************************************************/
	DescriptorSetUpdate& setDynamicUbo(pvr::uint16 bindingId, const api::BufferView& item)
	{
		return addBuffer(bindingId, 0, types::DescriptorType::UniformBufferDynamic, item);
	}

	/*!*********************************************************************************************************************
		\brief	Add a Ubo to the specified binding index. Supports array-indexing in the shader.
		\param	bindingId The index of the indexed binding point
		\param	arrayIndex If supported by the underlying API, add to which index of the array binding point.
		\param	item The object to add
		\return	this object (allow chaining)
		***********************************************************************************************************************/
	DescriptorSetUpdate& setDynamicUboAtIndex(pvr::uint16 bindingId, pvr::uint8 arrayIndex, const api::BufferView& item)
	{
		return addBuffer(bindingId, arrayIndex, types::DescriptorType::UniformBufferDynamic, item);
	}

	/*!*********************************************************************************************************************
	\brief	Add a Ssbo to the specified binding index.
	\param	bindingId The index of the indexed binding point
	\param	item The object to add
	\return	this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setSsbo(pvr::uint16 bindingId, const api::BufferView& item)
	{
		return addBuffer(bindingId, 0, types::DescriptorType::StorageBuffer, item);
	}

	/*!*********************************************************************************************************************
	\brief	Add an Ssbo to the specified binding index. Supports array-indexing in the shader.
	\param	bindingId The index of the indexed binding point
	\param	arrayIndex If supported by the underlying API, add to which index of the array binding point.
	\param	item The object to add
	\return	this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setSsboAtIndex(pvr::uint16 bindingId, pvr::uint8 arrayIndex, const api::BufferView& item)
	{
		return addBuffer(bindingId, arrayIndex, types::DescriptorType::StorageBuffer, item);
	}

	/*!*********************************************************************************************************************
	\brief	Add a Ssbo to the specified binding index.
	\param	bindingId The index of the indexed binding point
	\param	item The object to add
	\return	this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setDynamicSsbo(pvr::uint16 bindingId, const api::BufferView& item)
	{
		return addBuffer(bindingId, 0, types::DescriptorType::StorageBufferDynamic, item);
	}

	/*!*********************************************************************************************************************
	\brief	Add an Ssbo to the specified binding index. Supports array-indexing in the shader.
	\param	bindingId The index of the indexed binding point
	\param	arrayIndex If supported by the underlying API, add to which index of the array binding point.
	\param	item The object to add
	\return	this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setDynamicSsboAtIndex(pvr::uint16 bindingId, pvr::uint8 arrayIndex, const api::BufferView& item)
	{
		return addBuffer(bindingId, arrayIndex, types::DescriptorType::StorageBufferDynamic, item);
	}

	/*!*********************************************************************************************************************
	\brief	Create a CombinedImageSampler from the provided texture and sampler, and add it to the specified index.
	\param	bindingId The index of the indexed binding point
	\param	texture The texture to add
	\param	sampler The sampler to add
	\return	this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setCombinedImageSampler(pvr::uint16 bindingId, const pvr::api::TextureView& texture, const pvr::api::Sampler& sampler)
	{
		return addImageSampler(bindingId, 0, texture, sampler, types::DescriptorType::CombinedImageSampler);
	}

	/*!*********************************************************************************************************************
	\brief	Create a CombinedImageSampler from the provided texture and sampler, and add it to the specified index. Supports
	        array-indexing in the shader
	\param	bindingId The index of the indexed binding point
	\param	arrayIndex If supported by the underlying API, add to which index of the array binding point.
	\param	texture The texture to add
	\param	sampler The sampler to add
	\return	this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setCombinedImageSamplerAtIndex(pvr::uint16 bindingId, pvr::uint8 arrayIndex,
	    const pvr::api::TextureView& texture, const pvr::api::Sampler& sampler)
	{
		return addImageSampler(bindingId, arrayIndex, texture, sampler, types::DescriptorType::CombinedImageSampler);
	}

	/*!*********************************************************************************************************************
	\brief	Create a Input Attachment from the provided texture and sampler, and add it to the specified index.
	\param	bindingId The index of the indexed binding point
	\param	texture The texture to add
	\param	sampler The sampler to add
	\return	this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setInputImageAttachment(pvr::uint16 bindingId, const pvr::api::TextureView& texture, const pvr::api::Sampler& sampler)
	{
		return addImageSampler(bindingId, 0, texture, sampler, types::DescriptorType::InputAttachment);
	}

	/*!*********************************************************************************************************************
	\brief	Create a Input Attachment from the provided texture and sampler, and add it to the specified index. Supports
	array-indexing in the shader
	\param	bindingId The index of the indexed binding point
	\param	arrayIndex If supported by the underlying API, add to which index of the array binding point.
	\param	texture The texture to add
	\param	sampler The sampler to add
	\return	this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setInputImageAttachmentAtIndex(pvr::uint16 bindingId, pvr::uint8 arrayIndex,
	    const pvr::api::TextureView& texture, const pvr::api::Sampler& sampler)
	{
		return addImageSampler(bindingId, arrayIndex, texture, sampler, types::DescriptorType::InputAttachment);
	}

	const DescriptorSetUpdate::Bindings& getBindingList() const { return m_bindings; }

	void clear()
    {
        this->m_bindings.combinedSamplerImage.clear();
        this->m_bindings.buffers.clear();
    }
    
	template<class _Binding> static bool cmpDescriptorBinding(const _Binding& a, const _Binding& b)
	{
		if (a.bindingId < b.bindingId) { return true; }
		if (a.bindingId == b.bindingId && a.arrayIndex < b.arrayIndex) { return true; }
		return false;
	}
private:
	friend class ::pvr::api::impl::DescriptorSet_;

	DescriptorSetUpdate& addBuffer(pvr::uint16 bindingId, pvr::uint8 arrayIndex, types::DescriptorType::Enum type, const api::BufferView& item)
	{
		assertion(item.isValid(), "Invalid Ubo Item");
		if (bindingId >= m_bindings.buffers.size()) { m_bindings.buffers.resize(bindingId + 1); }
		m_bindings.buffers[bindingId] = DescriptorBinding<pvr::api::BufferView>(bindingId, arrayIndex, type, item);
		return *this;
	}

	DescriptorSetUpdate& addImageSampler(pvr::uint16 bindingId, pvr::uint8 arrayIndex, const pvr::api::TextureView& texture,
	                                     const pvr::api::Sampler& sampler, const pvr::types::DescriptorType::Enum type)
	{
		assertion(texture.isValid() && sampler.isValid(), "Invalid Combined-Image-Sampler Item");
		if (bindingId >= m_bindings.combinedSamplerImage.size())
		{
			m_bindings.combinedSamplerImage.resize(bindingId + 1);
		}
		m_bindings.combinedSamplerImage[bindingId] = DescriptorBinding<CombinedImageSampler>(bindingId, arrayIndex, type,
		    std::make_pair(sampler, texture));
		return *this;
	}

	Bindings m_bindings;
	types::DescriptorSetUsage::Enum m_usage;
};

namespace impl {
/*!*********************************************************************************************************************
\brief A descriptor set object. Carries all memory-related API object state like Textures (Images), Samplers, UBOs, Ssbos
       etc. Does NOT carry pipeline specific state such as Vertex/Index buffers, shader programs etc (these are part of the
	   Pipeline objects).
***********************************************************************************************************************/
class DescriptorSet_
{
public:
	typedef uint16 IndexType;
	/*!*********************************************************************************************************************
	\brief  Create a DescriptorSet on a specific DescriptorPool.
	\brief createParam DescriptorSet creation information.
	\brief pool A DescriptorPool to create the DescriptorSet on.
	***********************************************************************************************************************/
	DescriptorSet_(const api::DescriptorSetLayout& descSetLayout, const api::DescriptorPool& pool) :
		m_descSetLayout(descSetLayout), m_descPool(pool) {}

	/*!*********************************************************************************************************************
	\brief Destructor. Frees all resources owned by this object.
	***********************************************************************************************************************/
	virtual ~DescriptorSet_() { }

	/*!*********************************************************************************************************************
	\brief Return the layout of this DescriptorSet.
	\return This DescriptorSet's DescriptorSetLayout
	***********************************************************************************************************************/
	const api::DescriptorSetLayout& getDescriptorSetLayout()const {return m_descSetLayout;}

	const native::HDescriptorSet_& getNativeObject() const;
	native::HDescriptorSet_& getNativeObject();

	const DescriptorPool& getDescriptorPool()const { return m_descPool; }
	DescriptorPool& getDescriptorPool() { return m_descPool; }

	const GraphicsContext& getContext()const { return m_descPool->getContext(); }
	GraphicsContext& getContext() { return m_descPool->getContext(); }

	bool update(const DescriptorSetUpdate& descSet);
protected:
	DescriptorSetLayout m_descSetLayout;
	DescriptorPool m_descPool;
};
}// namespace impl
}// namspace api

}