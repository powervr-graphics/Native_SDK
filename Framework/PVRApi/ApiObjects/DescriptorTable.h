/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\DescriptorTable.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the DescriptorSet class.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/ForwardDecApiObjects.h"
#include "PVRCore/IGraphicsContext.h"
#include "PVRCore/CoreIncludes.h"
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRApi/ApiObjects/Buffer.h"
#include "PVRApi/ApiObjects/Sampler.h"
#include "PVRApi/ApiObjects/GraphicsPipeline.h"
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRApi/ApiObjects/Buffer.h"
#include "PVRApi/ApiObjects/Sampler.h"
#include "PVRApi/ApiObjects/GraphicsPipeline.h"
#include "PVRApi/ApiObjects/PipelineConfigStateCreateParam.h"
namespace pvr {
namespace api {
class UpdateDescriptorSet;
namespace impl {
class DescriptorSetLayoutImpl;
class DescriptorSetGlesImpl;
}//namespace impl

/*!*********************************************************************************************************************
\brief Enumeration of all shader stages.
***********************************************************************************************************************/
namespace ShaderStageFlags {
enum Enum
{
	Vertex = 1, //< Vertex Shader stage
	Fragment = 2,//< Fragment Shader stage
	Compute = 4,//< Compute Shader stage
	AllGraphicsStages = Vertex + Fragment,//< Vertex + Fragment shader stage
	NUM_SHADER_STAGES = AllGraphicsStages + 1
};

/*!*********************************************************************************************************************
\brief Bitwise OR shader stage flags.
\return The Bitwise OR of the operands
***********************************************************************************************************************/
inline ShaderStageFlags::Enum operator|(ShaderStageFlags::Enum lhs, ShaderStageFlags::Enum rhs)
{
	return (ShaderStageFlags::Enum)(static_cast<unsigned char>(lhs) | static_cast<unsigned char>(rhs));
}

/*!*********************************************************************************************************************
\brief Bitwise OR with assignment shader stage flags.
\return The left hand side of the operation, which has been bitwise ORed with the right hand side
***********************************************************************************************************************/
inline ShaderStageFlags::Enum& operator|=(ShaderStageFlags::Enum& lhs, ShaderStageFlags::Enum rhs)
{
	return lhs = (ShaderStageFlags::Enum)(static_cast<unsigned char>(lhs) | static_cast<unsigned char>(rhs));
}
}// namespace ShaderStageFlags
class BindDescriptorSets;

/*!*********************************************************************************************************************
\brief Contains all information required to create a Descriptor Set Layout. This is the number of Textures, Samplers, Uniform
      Buffer Objects, and Shader Storage Buffer Objects bound for any shader stage.
***********************************************************************************************************************/
struct DescriptorSetLayoutCreateParam
{
private:
	struct BindInfo
	{
		DescriptorType::Enum descType;
		ShaderStageFlags::Enum shaderStage;
		pvr::uint32 arraySize;
		BindInfo() {}
		BindInfo(DescriptorType::Enum descType,
		         ShaderStageFlags::Enum shaderStage,
		         pvr::uint32 arraySize) : descType(descType), shaderStage(shaderStage), arraySize(arraySize) {}
	};

	friend class ::pvr::api::impl::DescriptorSetLayoutImpl;
	friend class ::pvr::api::BindDescriptorSets;
	friend struct ::pvr::api::DescriptorSetUpdateParam;
	std::vector<BindInfo> m_bindings;

public:
	/*!*********************************************************************************************************************
	\brief Set the buffer binding of Descriptor Objects in the specified shader stages.
	\param[in] bindIndex The index to which the binding will be added
	\param[in] descType The type of descriptor
	\param[in] arraySize size of the array
	\param[in] stageFlags The shader stages for which the number of bindings is set to (count)
	\return this (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetLayoutCreateParam& addBinding(pvr::uint32 bindIndex, DescriptorType::Enum descType, pvr::uint32 arraySize = 1,
	        ShaderStageFlags::Enum stageFlags = ShaderStageFlags::AllGraphicsStages)
	{
		if (bindIndex >= m_bindings.size())
		{
			m_bindings.resize(bindIndex + 1);
		}
		m_bindings[bindIndex] = BindInfo(descType, stageFlags, arraySize);
		return *this;
	}
};
namespace impl {
/*!*********************************************************************************************************************
\brief API DescriptorSetLayout. Use through the Reference Counted Framework Object pvr::api::DescriptorSetLayout. Create using
       the IGraphicsContext::createDescriptorSetLayout. A Descriptor Set Layout is required both to construct a descriptor
	   set object, and a Pipeline compatible with this object.
***********************************************************************************************************************/
class DescriptorSetLayoutImpl
{
	friend class ::pvr::api::BindDescriptorSets;
	template<typename> friend struct ::pvr::RefCountEntryIntrusive;
	DescriptorSetLayoutCreateParam desc;
	GraphicsContext device;
	native::HDescriptorSetLayout descriptorSetLayout;
	virtual pvr::Result::Enum init() = 0;
	DescriptorSetLayoutImpl& operator=(const DescriptorSetLayoutImpl&);

protected:
	DescriptorSetLayoutImpl(GraphicsContext& context, const DescriptorSetLayoutCreateParam& desc)
		: desc(desc), device(context) {}
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

	virtual ~DescriptorSetLayoutImpl() {}
};

}

/*!*********************************************************************************************************************
\brief  Descriptor Pool create parameter.
***********************************************************************************************************************/
struct DescriptorPoolCreateParam
{
private:
	std::map<DescriptorType::Enum, pvr::uint32> descriptorType;
	pvr::uint32 maxSets;
public:
	/*!*********************************************************************************************************************
	\brief Add the maximum number of the specified descriptor types that the pool will contain.
	\param[in] descType Descriptor type
	\param[in] count Maximum number of descriptors of (type)
	\return this (allow chaining)
	***********************************************************************************************************************/
	DescriptorPoolCreateParam& addDescriptorInfo(DescriptorType::Enum descType, pvr::uint16 count)
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
};

/*!*********************************************************************************************************************
\brief Pipeline Layout create information. The descriptor set layouts must be known to create a Pipeline layout.
***********************************************************************************************************************/
struct PipelineLayoutCreateParam
{
	/*!*********************************************************************************************************************
	\brief Add a descriptor set layout to this pipeline layout. Added to the end of the list of layouts.
	\param[in] descLayout A descriptor set layout
	\return this (allow chaining)
	***********************************************************************************************************************/
	PipelineLayoutCreateParam& addDescSetLayout(const DescriptorSetLayout& descLayout)
	{
		m_descLayout.push_back(descLayout);
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Add a descriptor set layout to this pipeline layout. Added to the specified index.
	\param[in] index The index where the layout will be created on
	\param[in] descLayout A descriptor set layout
	\return this (allow chaining)
	***********************************************************************************************************************/
	PipelineLayoutCreateParam& addDescSetLayout(pvr::uint32 index, const DescriptorSetLayout& descLayout);

	pvr::uint32 getNumDescSetLayouts()const { return (pvr::uint32)m_descLayout.size(); }

	const DescriptorSetLayout& getDescriptorSetLayout(pvr::uint32 index)const
	{
#ifdef DEBUG
		if (index >= m_descLayout.size())
		{
			PVR_ASSERT(false && "Invalid DescriptorSetLayout Index");
			pvr::Log("DescriptorSetLayout::getDescriptorSetLayout - Invalid DescriptorSetLayout Index");
		}
#endif
		return m_descLayout[index];
	}

private:
	friend class ::pvr::api::impl::PipelineLayoutImpl;
	friend class ::pvr::api::impl::GraphicsPipelineImpl;
	std::vector<DescriptorSetLayout> m_descLayout;
};



namespace impl {
/*!*********************************************************************************************************************
\brief Implementation of a PipelineLayout object. A Pipeline Layout API PipelineLayout wrapper.
***********************************************************************************************************************/
class PipelineLayoutImpl
{
	friend ::pvr::api::PipelineLayout IGraphicsContext::createPipelineLayout
	(const api::PipelineLayoutCreateParam& desc);

public:
	/*!*********************************************************************************************************************
	\brief  Create this on device.
	\param[in] device
	***********************************************************************************************************************/
	PipelineLayoutImpl(GraphicsContext& device) : m_context(device) {}

	/*!*********************************************************************************************************************
	\brief dtor
	***********************************************************************************************************************/
	~PipelineLayoutImpl();

	/*!*********************************************************************************************************************
	\brief Get list of descriptor set layout used by this.
	\return std::vector<pvr::api::DescriptorSetLayout>&
	***********************************************************************************************************************/
	const std::vector<pvr::api::DescriptorSetLayout>& getDescriptorSetLayout()const { return m_desc.m_descLayout; }

	const PipelineLayoutCreateParam& getCreateParam()const { return m_desc; }
private:
	pvr::Result::Enum init(const PipelineLayoutCreateParam& createParam);
	GraphicsContext m_context;
	PipelineLayoutCreateParam m_desc;
	native::HPipelineLayout m_pipelineLayout;
};

/*!*********************************************************************************************************************
\brief API DescriptorPool Object wrapper. Access through the framework-managed DescriptorPool object.
***********************************************************************************************************************/
class DescriptorPoolImpl
{
private:
	friend class ::pvr::IGraphicsContext;
	DescriptorPoolCreateParam m_poolInfo;
	native::HDescriptorPool m_descPool;
	GraphicsContext m_context;
	DescriptorPoolUsage::Enum m_usage;
	pvr::Result::Enum init(const DescriptorPoolCreateParam& createParam, DescriptorPoolUsage::Enum usage);
	DescriptorPoolImpl& operator=(const DescriptorPoolImpl&);
	void destroy();
public:
	/*!*********************************************************************************************************************
	\brief Constructor. Do not use directly.
	\param[in] device
	***********************************************************************************************************************/
	DescriptorPoolImpl(GraphicsContext& device) : m_context(device) {}

	/*!*********************************************************************************************************************
	\brief Get native descriptorpool handle.
	\return native handle
	***********************************************************************************************************************/
	const native::HDescriptorPool& getNativeHandle()const { return m_descPool; }
};

}//namespace impl

/*!*********************************************************************************************************************
\brief This class contains all the information necessary to populate a Descriptor Set with the actual API objects. Use with
       the method update of the DescriptorSet. Populate this object with actual Descriptor objects (UBOs, textures etc).
***********************************************************************************************************************/
struct DescriptorSetUpdateParam
{
	/*!*********************************************************************************************************************
	\brief     Constructor.
	***********************************************************************************************************************/
	DescriptorSetUpdateParam(api::DescriptorSetUsage::Enum usage = DescriptorSetUsage::Static) :  m_usage(usage) {}

	/*!*********************************************************************************************************************
	\brief	Add a Ubo to the specified binding index.
	\param	bindingId The index of the indexed binding point
	\param	item The object to add
	\return	this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdateParam& addUbo(pvr::uint16 bindingId, const UboView& item)
	{
		return addUbo(bindingId, 0, item);
	}

	/*!*********************************************************************************************************************
	\brief	Add a Ubo to the specified binding index. Supports array-indexing in the shader.
	\param	bindingId The index of the indexed binding point
	\param	arrayIndex If supported by the underlying API, add to which index of the array binding point.
	\param	item The object to add
	\return	this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdateParam& addUbo(pvr::uint16 bindingId, pvr::uint8 arrayIndex, const UboView& item)
	{
		PVR_ASSERT(item.isValid() && "Invalid Ubo Item");
//		PVR_ASSERT(index < m_descSetLayout->getUboCount(stages));
		//	DescriptorSetLayoutCreateParam::BindInfo const& bindInfo = m_descSetLayout->getCreateParam().getBindInfo(bindingId);
		//	PVR_ASSERT(bindInfo.descType == DescriptorType::Ubo);
		if (bindingId >= m_ubos.size()) { m_ubos.resize(bindingId + 1); }
		m_ubos.push_back(DescriptorBinding<pvr::api::UboView>(bindingId, arrayIndex, item));
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief	Add a Ssbo to the specified binding index.
	\param	bindingId The index of the indexed binding point
	\param	item The object to add
	\return	this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdateParam& addSsbo(pvr::uint16 bindingId, const SsboView& item)
	{
		return addSsbo(bindingId, 0, item);
	}

	/*!*********************************************************************************************************************
	\brief	Add an Ssbo to the specified binding index. Supports array-indexing in the shader.
	\param	bindingId The index of the indexed binding point
	\param	arrayIndex If supported by the underlying API, add to which index of the array binding point.
	\param	item The object to add
	\return	this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdateParam& addSsbo(pvr::uint16 bindingId, pvr::uint8 arrayIndex, const SsboView& item)
	{
		PVR_ASSERT(item.isValid() && "Invalid Ssbo Item");
		//	PVR_ASSERT(index < m_descSetLayout->getSsboCount(stages));
		//DescriptorSetLayoutCreateParam::BindInfo const& bindInfo = m_descSetLayout->getCreateParam().getBindInfo(bindingId);
		//PVR_ASSERT(bindInfo.descType == DescriptorType::Ssbo);
		if (bindingId >= m_ssbos.size()) { m_ssbos.resize(bindingId + 1); }
		m_ssbos.push_back(DescriptorBinding<pvr::api::SsboView>(bindingId, arrayIndex, item));
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief	Add a Texture to the specified binding index.
	\param	bindingId The index of the indexed binding point
	\param	item The object to add
	\return	this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdateParam& addTexture(pvr::uint16 bindingId, const TextureView& item)
	{
		return addTexture(bindingId, 0, item);
	}

	/*!*********************************************************************************************************************
	\brief	Add a Texture to the specified binding index. Supports array-indexing in the shader.
	\param	bindingId The index of the indexed binding point
	\param	arrayIndex If supported by the underlying API, add to which index of the array binding point.
	\param	item The object to add
	\return	this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdateParam& addTexture(pvr::uint16 bindingId, pvr::uint8 arrayIndex, const TextureView& item)
	{
		PVR_ASSERT(item.isValid() && "Invalid Texture Item");
		addImageSampler(bindingId, arrayIndex, item, api::Sampler());
		return *this;
	}


	/*!*********************************************************************************************************************
	\brief	Add a Sampler to the specified binding index.
	\param	bindingId The index of the indexed binding point
	\param	item The object to add
	\return	this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdateParam& addSampler(pvr::uint16 bindingId, const Sampler& item)
	{
		return addSampler(bindingId, 0, item);
	}

	/*!*********************************************************************************************************************
	\brief	Add a Sampler to the specified binding index. Supports array-indexing in the shader.
	\param	bindingId The index of the indexed binding point
	\param	arrayIndex If supported by the underlying API, add to which index of the array binding point.
	\param	item The object to add
	\return	this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdateParam& addSampler(pvr::uint16 bindingId, pvr::uint8 arrayIndex, const Sampler& item)
	{
		PVR_ASSERT(item.isValid() && "Inavlid Sampler Item");
		addImageSampler(bindingId, arrayIndex, api::TextureView(), item);
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief	Create a CombinedImageSampler from the provided texture and sampler, and add it to the specified index.
	\param	bindingId The index of the indexed binding point
	\param	texture The texture to add
	\param	sampler The sampler to add
	\return	this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdateParam& addCombinedImageSampler(pvr::uint16 bindingId,
	        const pvr::api::TextureView& texture, const pvr::api::Sampler& sampler)
	{
		return addCombinedImageSampler(bindingId, 0, texture, sampler);
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
	DescriptorSetUpdateParam& addCombinedImageSampler(pvr::uint16 bindingId, pvr::uint8 arrayIndex,
	        const pvr::api::TextureView& texture, const pvr::api::Sampler& sampler)
	{
		PVR_ASSERT(texture.isValid() && sampler.isValid() && "Invalid Combiend-Image-Sampler Item");
		addImageSampler(bindingId, arrayIndex, texture, sampler);
		return *this;
	}
private:
	template<class _Binding>
	struct DescriptorBinding
	{
		_Binding binding;
		pvr::uint16 bindingId;
		pvr::uint16 arrayIndex;
		DescriptorBinding(pvr::uint16 bindingId, pvr::uint16 index, const _Binding& obj) :
			binding(obj), bindingId(bindingId), arrayIndex(index) {}
		DescriptorBinding() {}
	};

	friend class ::pvr::api::impl::DescriptorSetImpl;
	friend class ::pvr::api::impl::DescriptorSetGlesImpl;

	void addImageSampler(pvr::uint16 bindingId, pvr::uint8 arrayIndex, const pvr::api::TextureView& texture,
	                     const pvr::api::Sampler& sampler);

	//Careful of the digraph: if the space is removed <:: some compilers will interpret it as [:
	typedef std::pair</**/::pvr::api::Sampler, ::pvr::api::TextureView> CombinedImageSampler;
	std::vector<DescriptorBinding</**/::pvr::api::UboView>/**/> m_ubos;
	std::vector<DescriptorBinding</**/::pvr::api::SsboView>/**/> m_ssbos;
	std::vector<DescriptorBinding<CombinedImageSampler>/**/> m_combinedSamplerImage;
	api::DescriptorSetUsage::Enum m_usage;
};

namespace impl {
/*!*********************************************************************************************************************
\brief A descriptor set object. Carries all memory-related API object state like Textures (Images), Samplers, UBOs, Ssbos
       etc. Does NOT carry pipeline specific state such as Vertex/Index buffers, shader programs etc (these are part of the
	   Pipeline objects).
***********************************************************************************************************************/
class DescriptorSetImpl
{
	friend class ::pvr::api::BindDescriptorSets;
	friend class ::pvr::IGraphicsContext;
	friend class ::pvr::api::UpdateDescriptorSet;
public:
	typedef uint16 IndexType;
	/*!*********************************************************************************************************************
	\brief  Create a DescriptorSet on a specific DescriptorPool.
	\brief createParam DescriptorSet creation information.
	\brief pool A DescriptorPool to create the DescriptorSet on.
	***********************************************************************************************************************/
	DescriptorSetImpl(const DescriptorSetLayout& descSetLayout, const DescriptorPool& pool) :
		m_descSetLayout(descSetLayout), m_descPool(pool) {}

	/*!*********************************************************************************************************************
	\brief   Populate this descriptor set with objects.
	\return  pvr::Result::Success if successful, error code otherwise.
	***********************************************************************************************************************/
	virtual pvr::Result::Enum update(const pvr::api::DescriptorSetUpdateParam& descSet) = 0;

	/*!*********************************************************************************************************************
	\brief Destructor. Frees all resources owned by this object.
	***********************************************************************************************************************/
	virtual ~DescriptorSetImpl() { }

	/*!*********************************************************************************************************************
	\brief Return the layout of this DescriptorSet.
	\return This DescriptorSet's DescriptorSetLayout
	***********************************************************************************************************************/
	const DescriptorSetLayout& getDescriptorSetLayout()const {return m_descSetLayout;}

	/*!*********************************************************************************************************************
	\brief Internal. Do not use.
	***********************************************************************************************************************/
	virtual void bind(IGraphicsContext& device, pvr::uint32 dynamicOffset)const = 0;
protected:
	virtual pvr::Result::Enum init() = 0;
	DescriptorSetUpdateParam m_descParam;
	DescriptorSetLayout m_descSetLayout;
	DescriptorPool m_descPool;
	native::HDescriptorSet	m_descriptorSet;
};
}

}
}

#undef ADD_DESCRIPTOR_LAYOUT_PART
