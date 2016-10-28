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
	// Binding list for images, uniform buffers and storage buffers
	typedef pvr::types::DescriptorBindingLayout ImagesList[pvr::types::DescriptorBindingDefaults::MaxImages];
	typedef pvr::types::DescriptorBindingLayout UniformBuffersList[pvr::types::DescriptorBindingDefaults::MaxUniformBuffers];
	typedef pvr::types::DescriptorBindingLayout StorageBuffersList[pvr::types::DescriptorBindingDefaults::MaxStorageBuffers];
	struct Bindings
	{
		ImagesList images;
		UniformBuffersList uniformBuffers;
		StorageBuffersList storageBuffers;
	};
private:
	Bindings m_bindings;
	pvr::uint8 numberOfImages;
	pvr::uint8 numberOfUniformBuffers;
	pvr::uint8 numberOfStorageBuffers;
public:
	/*!
	   \brief Constructor
	 */
	DescriptorSetLayoutCreateParam() :  numberOfImages(0), numberOfUniformBuffers(0), numberOfStorageBuffers(0)
	{}

	/*!*********************************************************************************************************************
	\brief Set the buffer binding of Descriptor Objects in the specified shader stages.
	\param[in] bindIndex The index to which the binding will be added
	\param[in] descType The type of descriptor
	\param[in] arraySize size of the array
	\param[in] stageFlags The shader stages for which the number of bindings is set to (count)
	\return this (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetLayoutCreateParam& setBinding(pvr::uint8 bindIndex, types::DescriptorType descType, pvr::uint8 arraySize = 1,
	    types::ShaderStageFlags stageFlags = types::ShaderStageFlags::AllGraphicsStages)
	{
		assertion(arraySize != 0, "DescriptorSetLayoutCreateParam::setBinding - Array size cannot be zero");

		// fetch the generic descriptor type (image, storage buffer or uniform buffer)
		pvr::types::DescriptorBindingType descriptorType = pvr::types::getDescriptorTypeBinding(descType);

		// only increment the count for a new binding at a specific binding index
		switch (descriptorType)
		{
		case pvr::types::DescriptorBindingType::Image:
			if (!m_bindings.images[bindIndex].isValid()) { ++numberOfImages; }
			m_bindings.images[bindIndex] = pvr::types::DescriptorBindingLayout(arraySize, descType, stageFlags);
			break;
		case pvr::types::DescriptorBindingType::StorageBuffer:
			if (!m_bindings.storageBuffers[bindIndex].isValid()) { ++numberOfStorageBuffers; }
			m_bindings.storageBuffers[bindIndex] = pvr::types::DescriptorBindingLayout(arraySize, descType, stageFlags);
			break;
		case pvr::types::DescriptorBindingType::UniformBuffer:
			if (!m_bindings.uniformBuffers[bindIndex].isValid()) { ++numberOfUniformBuffers; }
			m_bindings.uniformBuffers[bindIndex] = pvr::types::DescriptorBindingLayout(arraySize, descType, stageFlags);
			break;
		default:
			assertion(false, "Unsupported descriptor type");
			Log(pvr::Logger::Severity::Error, "Unsupported descriptor type");
		}

		return *this;
	}

	/*!
	   \brief Clear all entries
	   \return Return this for chaining
	 */
	DescriptorSetLayoutCreateParam& clear()
	{
		for (unsigned int i = 0; i < pvr::types::DescriptorBindingDefaults::MaxImages; i++)
		{
			m_bindings.images[i] = pvr::types::DescriptorBindingLayout();
		}
		for (unsigned int i = 0; i < pvr::types::DescriptorBindingDefaults::MaxStorageBuffers; i++)
		{
			m_bindings.storageBuffers[i] = pvr::types::DescriptorBindingLayout();
		}
		for (unsigned int i = 0; i < pvr::types::DescriptorBindingDefaults::MaxUniformBuffers; i++)
		{
			m_bindings.uniformBuffers[i] = pvr::types::DescriptorBindingLayout();
		}

		numberOfImages = 0;
		numberOfUniformBuffers = 0;
		numberOfStorageBuffers = 0;

		return *this;
	}

	const types::DescriptorBindingLayout& getBindingImage(uint8 index)const
	{
		return m_bindings.images[index];
	}

	const types::DescriptorBindingLayout& getBindingUbo(uint8 index)const
	{
		return m_bindings.uniformBuffers[index];
	}

	const types::DescriptorBindingLayout& getBindingSsbo(uint8 index)const
	{
		return m_bindings.storageBuffers[index];
	}

	/*!********************************************************************************************************************
	\brief get descriptor binding
	\param index Binding Index
	\param descType The descriptor type to get the layout for.
	\return The binding layout object (DescriptorBindingLayout)
	**********************************************************************************************************************/
	const types::DescriptorBindingLayout& getBinding(uint8 index, pvr::types::DescriptorType descType)const
	{
		// fetch the generic descriptor type (image, storage buffer or uniform buffer)
		pvr::types::DescriptorBindingType descriptorType = pvr::types::getDescriptorTypeBinding(descType);

		const pvr::types::DescriptorBindingLayout* bindings[] =
		{
			&m_bindings.images[0],
			&m_bindings.uniformBuffers[0],
			&m_bindings.storageBuffers[0]
		};
		return bindings[(uint32)descriptorType][index];
	}

	/*!
	\brief Return the number of images in this object
	\return the number of images in this object
	*/
	uint8 getNumImages()const { return numberOfImages; }

	/*!
	\brief Return number of ubos in this object
	\return The number of ubos in this object
	*/
	uint8 getNumUbos()const { return numberOfUniformBuffers; }

	/*!
	\brief Return number of ssbos in this object
	\return The number of ssbos in this object
	*/
	uint8 getNumSsbos()const { return numberOfStorageBuffers; }

	/*!*********************************************************************************************************************
	\brief Get the total number of bindings (objects) in this object
	\return The total number of bindings (objects) in this object
	*********************************************************************************************************************/
	uint32 getNumBindings()const { return (uint32)(numberOfImages + numberOfStorageBuffers + numberOfUniformBuffers); }

	/*!
	   \brief Equality operator. Does deep comparison of the contents.
	   \param rhs The right-hand side argument of the operator.
	   \return True if the layouts have identical bindings
	 */
	bool operator==(const DescriptorSetLayoutCreateParam& rhs)
	{
		if (getNumBindings() != rhs.getNumBindings()) { return false; }

		for (unsigned int i = 0; i < pvr::types::DescriptorBindingDefaults::MaxImages; i++)
		{
			if (m_bindings.images[i].arraySize != rhs.m_bindings.images[i].arraySize ||
			    m_bindings.images[i].descType != rhs.m_bindings.images[i].descType ||
			    m_bindings.images[i].shaderStage != rhs.m_bindings.images[i].shaderStage ||
			    !m_bindings.images[i].isValid() ||
			    !rhs.m_bindings.images[i].isValid())
			{
				return false;
			}
		}
		for (unsigned int i = 0; i < pvr::types::DescriptorBindingDefaults::MaxStorageBuffers; i++)
		{
			if (m_bindings.storageBuffers[i].arraySize != rhs.m_bindings.storageBuffers[i].arraySize ||
			    m_bindings.storageBuffers[i].descType != rhs.m_bindings.storageBuffers[i].descType ||
			    m_bindings.storageBuffers[i].shaderStage != rhs.m_bindings.storageBuffers[i].shaderStage ||
			    !m_bindings.storageBuffers[i].isValid() ||
			    !rhs.m_bindings.storageBuffers[i].isValid())
			{
				return false;
			}
		}
		for (unsigned int i = 0; i < pvr::types::DescriptorBindingDefaults::MaxUniformBuffers; i++)
		{
			if (m_bindings.uniformBuffers[i].arraySize != rhs.m_bindings.uniformBuffers[i].arraySize ||
			    m_bindings.uniformBuffers[i].descType != rhs.m_bindings.uniformBuffers[i].descType ||
			    m_bindings.uniformBuffers[i].shaderStage != rhs.m_bindings.uniformBuffers[i].shaderStage ||
			    !m_bindings.uniformBuffers[i].isValid() ||
			    !rhs.m_bindings.uniformBuffers[i].isValid())
			{
				return false;
			}
		}

		return true;
	}
private:
	friend class ::pvr::api::impl::DescriptorSetLayout_;
	friend class ::pvr::api::BindDescriptorSets;
	friend struct ::pvr::api::DescriptorSetUpdate;
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

	/*!
	   \brief operator =
	   \return Return true if equal
	 */
	DescriptorSetLayout_& operator=(const DescriptorSetLayout_&);

	/*!
	   \brief Return the DescriptorLayout bindings
	 */
	const DescriptorSetLayoutCreateParam::Bindings& getDescriptorDescBindings()const { return desc.m_bindings; }

	/*!
	   \brief Constructor
	   \param context Context who owns this resource
	   \param desc Initialization info
	 */
	DescriptorSetLayout_(GraphicsContext& context, const DescriptorSetLayoutCreateParam& desc) : desc(desc), device(context)
	{
#ifdef DEBUG
		if (getContext()->getApiType() > pvr::Api::OpenGLESMaxVersion)
		{
			for (pvr::uint8 i = 0; i < desc.getNumBindings(); i++)
			{
				if (desc.getBindingImage(i).isValid() ||
				    desc.getBindingSsbo(i).isValid() ||
				    desc.getBindingUbo(i).isValid())
				{
					if ((desc.getBindingImage(i).isValid() && desc.getBindingSsbo(i).isValid()) ||
					    (desc.getBindingUbo(i).isValid() && desc.getBindingSsbo(i).isValid()) ||
					    (desc.getBindingUbo(i).isValid() && desc.getBindingSsbo(i).isValid()))
					{
						debug_assertion(false,
						                "Vulkan requires that descriptor set layout bindings have unique indices within a single set.");
					}
				}
				else
				{
					debug_assertion(false,
					                "Vulkan requires that descriptor set layouts have linear bindings starting at 0");
				}
			}
		}
#endif
	}
	DescriptorSetLayoutCreateParam desc;
	GraphicsContext device;
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

	/*!*********************************************************************************************************************
	\brief Get the context that this layout belongs to. (const)
	\return The context that this layout belongs to.
	***********************************************************************************************************************/
	const GraphicsContext& getContext()const { return device; }

	/*!
	   \brief destructor
	 */
	virtual ~DescriptorSetLayout_() { }
};
}// impl

/*!*********************************************************************************************************************
\brief  Descriptor Pool create parameter.
***********************************************************************************************************************/
struct DescriptorPoolCreateParam
{
private:
	std::map<types::DescriptorType, pvr::uint32> descriptorType;
	pvr::uint32 maxSets;
public:

	/*!
	   \brief Constructor
	 */
	DescriptorPoolCreateParam() : maxSets(200) {}

	/*!*********************************************************************************************************************
	\brief Add the maximum number of the specified descriptor types that the pool will contain.
	\param[in] descType Descriptor type
	\param[in] count Maximum number of descriptors of (type)
	\return this (allow chaining)
	***********************************************************************************************************************/
	DescriptorPoolCreateParam& addDescriptorInfo(types::DescriptorType descType, pvr::uint16 count)
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

	/*!
	   \brief Get the number of allocations of a descriptor type is supported on this pool.
	   \param descType DescriptorType
	   \return Number of allocations.
	 */
	pvr::uint32 getDescriptorTypeCount(types::DescriptorType descType) const
	{
		std::map<types::DescriptorType, pvr::uint32>::const_iterator found = descriptorType.find(descType);
		return (found != descriptorType.end() ? found->second  : 0);
	}

	/*!
	   \brief Get maximum sets supported on this pool.
	 */
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
	DescriptorPool_& operator=(const DescriptorPool_&);
public:
	/*!
	   \brief Return the context
	 */
	const GraphicsContext& getContext()const { return m_context; }

	/*!
	   \brief Return the context
	 */
	GraphicsContext& getContext() { return m_context; }

	/*!*********************************************************************************************************************
	\brief Constructor. Do not use directly.
	\param[in] device the device
	***********************************************************************************************************************/
	DescriptorPool_(const GraphicsContext& device) : m_context(device) {}

	/*!
	   \brief Allocate descriptor set
	   \param layout Descriptor set layout
	   \return Return DescritptorSet else null if fails.
	 */
	api::DescriptorSet allocateDescriptorSet(const DescriptorSetLayout& layout);

	/*!*********************************************************************************************************************
	\brief Get the underlying API object for this descriptor pool.
	\return An API-specific native handle. Type will be different for each API.
	***********************************************************************************************************************/
	const native::HDescriptorPool_& getNativeObject()const;

	/*!*********************************************************************************************************************
	\brief Get the underlying API object for this descriptor pool.
	\return An API-specific native handle. Type will be different for each API.
	***********************************************************************************************************************/
	native::HDescriptorPool_& getNativeObject();
};
}//namespace impl

/*!*********************************************************************************************************************
\brief This class contains all the information necessary to populate a Descriptor Set with the actual API objects. Use with
       the method update of the DescriptorSet. Populate this object with actual Descriptor objects (UBOs, textures etc).
***********************************************************************************************************************/
struct DescriptorSetUpdate
{
	/*!
	\brief Internal class
	*/
	struct DescriptorSampler
	{
		pvr::api::Sampler sampler; //!<Internal object
		bool useSampler; //!<Internal object
	};

	typedef pvr::types::DescriptorBinding<pvr::api::BufferView> BufferViewBinding;
	typedef std::pair<DescriptorSampler, pvr::api::TextureView> Image;
	typedef pvr::types::DescriptorBinding<Image> ImageBinding;
	typedef BufferViewBinding StorageBufferBindingList[pvr::types::DescriptorBindingDefaults::MaxStorageBuffers];
	typedef BufferViewBinding UniformBufferBindingList[pvr::types::DescriptorBindingDefaults::MaxUniformBuffers];
	typedef ImageBinding ImageBindingList[pvr::types::DescriptorBindingDefaults::MaxImages];
	/*!
	\brief Internal class
	*/
	struct Bindings
	{
		StorageBufferBindingList storageBuffers; //!<Internal
		StorageBufferBindingList uniformBuffers; //!<Internal
		ImageBindingList images; //!<Internal
	};
private:
	uint8 numberOfImages;
	uint8 numberOfUniformBuffers;
	uint8 numberOfStorageBuffers;
public:
	/*!*********************************************************************************************************************
	\brief     Constructor.
	***********************************************************************************************************************/
	DescriptorSetUpdate(types::DescriptorSetUsage usage = types::DescriptorSetUsage::Static) :  m_usage(usage)
	{
		numberOfImages = 0;
		numberOfUniformBuffers = 0;
		numberOfStorageBuffers = 0;
	}

	/*!
	   \brief Return number of images
	 */
	uint8 getNumImages()const { return numberOfImages; }

	/*!
	   \brief Return number of ubos
	 */
	uint8 getNumUbos()const { return numberOfUniformBuffers; }

	/*!
	   \brief Return number of ssbos
	 */
	uint8 getNumSsbos()const { return numberOfStorageBuffers; }

	/*!*********************************************************************************************************************
	\brief  Add a Ubo to the specified binding index.
	\param  bindingId The index of the indexed binding point
	\param  item The object to add
	\return this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setUbo(pvr::uint8 bindingId, const api::BufferView& item)
	{
		assertion(static_cast<pvr::uint32>(item->getResource()->getBufferUsage() & pvr::types::BufferBindingUse::UniformBuffer) != 0,
		          "DescriptorSetUpdate::setUbo - buffer doesn't support ubo binding");
		return addBuffer(bindingId, 0, types::DescriptorType::UniformBuffer, item);
	}

	/*!*********************************************************************************************************************
	\brief  Add a Ubo to the specified binding index. Supports array-indexing in the shader.
	\param  bindingId The index of the indexed binding point
	\param  arrayIndex If supported by the underlying API, add to which index of the array binding point.
	\param  item The object to add
	\return this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setUboAtIndex(pvr::uint8 bindingId, pvr::uint8 arrayIndex, const api::BufferView& item)
	{
		assertion(static_cast<pvr::uint32>(item->getResource()->getBufferUsage() & pvr::types::BufferBindingUse::UniformBuffer) != 0,
		          "DescriptorSetUpdate::setUboAtIndex - buffer doesn't support ubo binding");
		return addBuffer(bindingId, arrayIndex, types::DescriptorType::UniformBuffer, item);
	}

	/*!*********************************************************************************************************************
	\brief  Add a Ubo to the specified binding index.
	\param  bindingId The index of the indexed binding point
	\param  item The object to add
	\return this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setDynamicUbo(pvr::uint8 bindingId, const api::BufferView& item)
	{
		assertion(static_cast<pvr::uint32>(item->getResource()->getBufferUsage() & pvr::types::BufferBindingUse::UniformBuffer) != 0,
		          "DescriptorSetUpdate::setDynamicUbo - buffer doesn't support ubo binding");
		return addBuffer(bindingId, 0, types::DescriptorType::UniformBufferDynamic, item);
	}

	/*!*********************************************************************************************************************
	\brief  Add a Ubo to the specified binding index. Supports array-indexing in the shader.
	\param  bindingId The index of the indexed binding point
	\param  arrayIndex If supported by the underlying API, add to which index of the array binding point.
	\param  item The object to add
	\return this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setDynamicUboAtIndex(pvr::uint8 bindingId, pvr::uint8 arrayIndex, const api::BufferView& item)
	{
		assertion(static_cast<pvr::uint32>(item->getResource()->getBufferUsage() & pvr::types::BufferBindingUse::UniformBuffer) != 0,
		          "DescriptorSetUpdate::setDynamicUboAtIndex - buffer doesn't support ubo binding");
		return addBuffer(bindingId, arrayIndex, types::DescriptorType::UniformBufferDynamic, item);
	}

	/*!*********************************************************************************************************************
	\brief  Add a Ssbo to the specified binding index.
	\param  bindingId The index of the indexed binding point
	\param  item The object to add
	\return this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setSsbo(pvr::uint8 bindingId, const api::BufferView& item)
	{
		assertion(static_cast<pvr::uint32>(item->getResource()->getBufferUsage() & pvr::types::BufferBindingUse::StorageBuffer) != 0,
		          "DescriptorSetUpdate::setSsbo - buffer doesn't support ssbo binding");
		return addBuffer(bindingId, 0, types::DescriptorType::StorageBuffer, item);
	}

	/*!*********************************************************************************************************************
	\brief  Add an Ssbo to the specified binding index. Supports array-indexing in the shader.
	\param  bindingId The index of the indexed binding point
	\param  arrayIndex If supported by the underlying API, add to which index of the array binding point.
	\param  item The object to add
	\return this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setSsboAtIndex(pvr::uint8 bindingId, pvr::uint8 arrayIndex, const api::BufferView& item)
	{
		assertion(static_cast<pvr::uint32>(item->getResource()->getBufferUsage() & pvr::types::BufferBindingUse::StorageBuffer) != 0,
		          "DescriptorSetUpdate::setSsboAtIndex - buffer doesn't support ssbo binding");
		return addBuffer(bindingId, arrayIndex, types::DescriptorType::StorageBuffer, item);
	}

	/*!*********************************************************************************************************************
	\brief  Add a Ssbo to the specified binding index.
	\param  bindingId The index of the indexed binding point
	\param  item The object to add
	\return this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setDynamicSsbo(pvr::uint8 bindingId, const api::BufferView& item)
	{
		assertion(static_cast<pvr::uint32>(item->getResource()->getBufferUsage() & pvr::types::BufferBindingUse::StorageBuffer) != 0,
		          "DescriptorSetUpdate::setDynamicSsbo - buffer doesn't support ssbo binding");
		return addBuffer(bindingId, 0, types::DescriptorType::StorageBufferDynamic, item);
	}

	/*!*********************************************************************************************************************
	\brief  Add an Ssbo to the specified binding index. Supports array-indexing in the shader.
	\param  bindingId The index of the indexed binding point
	\param  arrayIndex If supported by the underlying API, add to which index of the array binding point.
	\param  item The object to add
	\return this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setDynamicSsboAtIndex(pvr::uint8 bindingId, pvr::uint8 arrayIndex, const api::BufferView& item)
	{
		assertion(static_cast<pvr::uint32>(item->getResource()->getBufferUsage() & pvr::types::BufferBindingUse::StorageBuffer) != 0,
		          "DescriptorSetUpdate::setUbo - buffer doesn't support ssbo binding");
		return addBuffer(bindingId, arrayIndex, types::DescriptorType::StorageBufferDynamic, item);
	}

	/*!*********************************************************************************************************************
	\brief  Create a CombinedImageSampler from the provided texture and sampler, and add it to the specified index.
	\param  bindingId The index of the indexed binding point
	\param  texture The texture to add
	\param  sampler The sampler to add
	\return this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setCombinedImageSampler(pvr::uint8 bindingId, const pvr::api::TextureView& texture,
	    const pvr::api::Sampler& sampler)
	{
		return addImageSampler(bindingId, 0, texture, sampler, types::DescriptorType::CombinedImageSampler);
	}

	/*!*********************************************************************************************************************
	\brief  Create a CombinedImageSampler from the provided texture and sampler, and add it to the specified index. Supports
	                array-indexing in the shader
	\param  bindingId The index of the indexed binding point
	\param  arrayIndex If supported by the underlying API, add to which index of the array binding point.
	\param  texture The texture to add
	\param  sampler The sampler to add
	\return this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setCombinedImageSamplerAtIndex(pvr::uint8 bindingId, pvr::uint8 arrayIndex,
	    const pvr::api::TextureView& texture, const pvr::api::Sampler& sampler)
	{
		return addImageSampler(bindingId, arrayIndex, texture, sampler, types::DescriptorType::CombinedImageSampler);
	}

	/*!*********************************************************************************************************************
	\brief  Create a Input Attachment from the provided texture and sampler, and add it to the specified index.
	\param  bindingId The index of the indexed binding point
	\param  texture The texture to add
	\return this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setInputImageAttachment(pvr::uint8 bindingId, const pvr::api::TextureView& texture)
	{
		return addInputAttachment(bindingId, 0, texture, types::DescriptorType::InputAttachment);
	}

	/*!*********************************************************************************************************************
	\brief  Create a Input Attachment from the provided texture and sampler, and add it to the specified index. Supports
	array-indexing in the shader
	\param  bindingId The index of the indexed binding point
	\param  arrayIndex If supported by the underlying API, add to which index of the array binding point.
	\param  texture The texture to add
	\return this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setInputImageAttachmentAtIndex(pvr::uint8 bindingId, pvr::uint8 arrayIndex,
	    const pvr::api::TextureView& texture)
	{
		return addInputAttachment(bindingId, arrayIndex, texture, types::DescriptorType::InputAttachment);
	}

	/*!
	   \brief Return the binding list
	 */
	const DescriptorSetUpdate::Bindings& getBindingList() const { return m_bindings; }

	/*!
	   \brief Clear all entries
	   \return Return this object for chaining
	 */
	DescriptorSetUpdate& clear()
	{
		for (uint32 i = 0; i < pvr::types::DescriptorBindingDefaults::MaxImages; ++i)
		{
			m_bindings.images[i] = ImageBinding();
		}
		for (uint32 i = 0; i < pvr::types::DescriptorBindingDefaults::MaxStorageBuffers; ++i)
		{
			m_bindings.storageBuffers[i] = BufferViewBinding();
		}
		for (uint32 i = 0; i < pvr::types::DescriptorBindingDefaults::MaxUniformBuffers; ++i)
		{
			m_bindings.storageBuffers[i] = BufferViewBinding();
		}
		numberOfImages = 0;
		numberOfUniformBuffers = 0;
		numberOfStorageBuffers = 0;
		return *this;
	}

private:
	friend class ::pvr::api::impl::DescriptorSet_;

	DescriptorSetUpdate& addBuffer(pvr::uint8 bindingId, pvr::uint8 arrayIndex, types::DescriptorType type, const api::BufferView& item)
	{
		assertion(item.isValid(), "Invalid Buffer Item");

		// fetch the generic descriptor type (image, storage buffer or uniform buffer)
		pvr::types::DescriptorBindingType descriptorType = pvr::types::getDescriptorTypeBinding(type);

		// only increment the count for a new binding at a specific binding index
		switch (descriptorType)
		{
		case pvr::types::DescriptorBindingType::StorageBuffer:
			if (m_bindings.storageBuffers[bindingId].bindingId == pvr::types::DescriptorBindingDefaults::BindingId)
			{
				++numberOfStorageBuffers;
			}
			m_bindings.storageBuffers[bindingId] = pvr::types::DescriptorBinding<pvr::api::BufferView>(bindingId, arrayIndex, type, item);
			assertion(m_bindings.storageBuffers[bindingId].isValid(), "Added storage buffer is not valid");
			break;
		case pvr::types::DescriptorBindingType::UniformBuffer:
			if (m_bindings.uniformBuffers[bindingId].bindingId == pvr::types::DescriptorBindingDefaults::BindingId)
			{
				++numberOfUniformBuffers;
			}
			m_bindings.uniformBuffers[bindingId] = pvr::types::DescriptorBinding<pvr::api::BufferView>(bindingId, arrayIndex, type, item);
			assertion(m_bindings.uniformBuffers[bindingId].isValid(), "Added uniform buffer is not valid");
			break;
		default:
			assertion(false, "Unsupported descriptor type");
			Log(pvr::Logger::Severity::Error, "Unsupported descriptor type");
		}

		return *this;
	}

	inline DescriptorSetUpdate& addImage(
	  pvr::uint8 bindingId, pvr::uint8 arrayIndex, const pvr::api::TextureView& texture,
	  const pvr::api::Sampler& sampler, const pvr::types::DescriptorType type, bool useSampler)
	{
		if (!texture.isValid())
		{
			assertion(texture.isValid(), "Invalid Image Item");
			Log("DescriptorSet update addImage invalid texture object");
			return *this;
		}
		if (useSampler && !sampler.isValid())
		{
			assertion(sampler.isValid(), "Invalid Sampler Item");
			Log("DescriptorSet update addImage invalid sampler object");
			return *this;
		}

		DescriptorSampler descriptorSampler{ sampler, useSampler };

		// fetch the generic descriptor type (image, storage buffer or uniform buffer)
		pvr::types::DescriptorBindingType descriptorType = pvr::types::getDescriptorTypeBinding(type);

		switch (descriptorType)
		{
		case pvr::types::DescriptorBindingType::Image:
			if (m_bindings.images[bindingId].bindingId == pvr::types::DescriptorBindingDefaults::BindingId)
			{
				numberOfImages++;
			}
			m_bindings.images[bindingId] = pvr::types::DescriptorBinding<Image>(bindingId, arrayIndex, type,
			                               std::make_pair(descriptorSampler, texture));
			assertion(m_bindings.images[bindingId].isValid(), "Added image is not valid");
			break;
		default:
			assertion(false, "Unsupported descriptor type");
			Log(pvr::Logger::Severity::Error, "Unsupported descriptor type");
		}

		return *this;
	}

	DescriptorSetUpdate& addImageSampler(pvr::uint8 bindingId, pvr::uint8 arrayIndex, const pvr::api::TextureView& texture,
	                                     const pvr::api::Sampler& sampler, const pvr::types::DescriptorType type)
	{
		return addImage(bindingId, arrayIndex, texture, sampler, type, true);
	}

	DescriptorSetUpdate& addInputAttachment(pvr::uint8 bindingId, pvr::uint8 arrayIndex, const pvr::api::TextureView& texture,
	                                        const pvr::types::DescriptorType type)
	{
		return addImage(bindingId, arrayIndex, texture, pvr::api::Sampler(), type, false);
	}

	Bindings m_bindings;
	types::DescriptorSetUsage m_usage;
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
	typedef uint8 IndexType;
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

	/*!
	   \brief Return a handle to the native object (const)
	 */
	const native::HDescriptorSet_& getNativeObject() const;

	/*!
	   \brief Return a handle to the native object
	 */
	native::HDescriptorSet_& getNativeObject();

	/*!
	   \brief Return the descriptor pool (const)
	 */
	const DescriptorPool& getDescriptorPool()const { return m_descPool; }

	/*!
	    \brief Return the descriptor pool
	 */
	DescriptorPool& getDescriptorPool() { return m_descPool; }

	/*!
	   \brief Return the graphics context (const)
	 */
	const GraphicsContext& getContext()const { return m_descPool->getContext(); }

	/*!
	   \brief \brief Return the graphics context (const)
	 */
	GraphicsContext& getContext() { return m_descPool->getContext(); }

	/*!
	   \brief Update this
	   \param descSet Descriptor set update param. Note application should externally synchronize
	          if this descriptor set maybe used by the gpu durring the update.
	   \return Return true on success
	 */
	bool update(const DescriptorSetUpdate& descSet);
protected:
	DescriptorSetLayout m_descSetLayout;
	DescriptorPool m_descPool;
	DescriptorSetUpdate m_descParam;
};
}// namespace impl
}// namspace api

}
