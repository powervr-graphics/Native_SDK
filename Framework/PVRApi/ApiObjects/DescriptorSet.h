/*!
\brief Contains the DescriptorSet class.
\file PVRApi/ApiObjects/DescriptorSet.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRApi/ApiObjects/Buffer.h"
#include "PVRApi/ApiObjects/Sampler.h"
#include "PVRApi/ApiObjects/PipelineConfig.h"
namespace pvr {
namespace api {

/// <summary>Contains all information required to create a Descriptor Set Layout. This is the number of Textures,
/// Samplers, Uniform Buffer Objects, and Shader Storage Buffer Objects bound for any shader stage.</summary>
struct DescriptorSetLayoutCreateParam
{
<<<<<<< HEAD
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
=======
private:

	// the layout bindings for the images used in a descriptor set
	pvr::types::DescriptorLayoutBindingStore _images;
	// the layout bindings for the ubos used in a descriptor set
	pvr::types::DescriptorLayoutBindingStore _ubos;
	// the layout bindings for the ssbos used in a descriptor set
	pvr::types::DescriptorLayoutBindingStore _ssbos;
	// the layout bindings for the indirect ray pipelines used in a descriptor set
	pvr::types::DescriptorLayoutBindingStore _indirectRayPipelines;

public:

	/// <summary>Set the buffer binding of Descriptor Objects in the specified shader stages.</summary>
	/// <param name="bindIndex">The index to which the binding will be added</param>
	/// <param name="descType">The type of descriptor</param>
	/// <param name="arraySize">size of the array</param>
	/// <param name="stageFlags">The shader stages for which the number of bindings is set to (count)</param>
	/// <returns>this (allow chaining)</returns>
	DescriptorSetLayoutCreateParam& setBinding(pvr::uint16 bindIndex, types::DescriptorType descType, pvr::uint16 arraySize = 1,
>>>>>>> 1776432f... 4.3
	    types::ShaderStageFlags stageFlags = types::ShaderStageFlags::AllGraphicsStages)
	{
		assertion(arraySize != 0, "DescriptorSetLayoutCreateParam::setBinding - Array size cannot be zero");

<<<<<<< HEAD
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
=======
		// fetch the generic descriptor type (image, storage buffer, uniform buffer or indirect ray pipeline)
		pvr::types::DescriptorBindingType descriptorType = pvr::types::getDescriptorTypeBinding(descType);

		// based on the descriptor type add the new binding layout to the appropriate store
		switch (descriptorType)
		{
		case pvr::types::DescriptorBindingType::Image:
			_images.add(pvr::types::DescriptorBindingLayout(bindIndex, arraySize, descType, stageFlags));
			debug_assertion(_images.retrieveDescriptor(bindIndex).isValid(), "Added image is not valid");
			break;
		case pvr::types::DescriptorBindingType::StorageBuffer:
			_ssbos.add(pvr::types::DescriptorBindingLayout(bindIndex, arraySize, descType, stageFlags));
			debug_assertion(_ssbos.retrieveDescriptor(bindIndex).isValid(), "Added storage buffer is not valid");
			break;
		case pvr::types::DescriptorBindingType::UniformBuffer:
			_ubos.add(pvr::types::DescriptorBindingLayout(bindIndex, arraySize, descType, stageFlags));
			debug_assertion(_ubos.retrieveDescriptor(bindIndex).isValid(), "Added uniform buffer is not valid");
			break;
		case pvr::types::DescriptorBindingType::IndirectRayPipeline:
			_indirectRayPipelines.add(pvr::types::DescriptorBindingLayout(bindIndex, arraySize, descType, stageFlags));
			debug_assertion(_indirectRayPipelines.retrieveDescriptor(bindIndex).isValid(), "Added indirect ray pipeline buffer is not valid");
>>>>>>> 1776432f... 4.3
			break;
		default:
			assertion(false, "Unsupported descriptor type");
			Log(pvr::Logger::Severity::Error, "Unsupported descriptor type");
		}

		return *this;
	}

<<<<<<< HEAD
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
=======
	/// <summary>Clear all entries</summary>
	/// <returns>Return this for chaining</returns>
	DescriptorSetLayoutCreateParam& clear()
	{
		_images.clear();
		_ssbos.clear();
		_ubos.clear();
		_indirectRayPipelines.clear();

		return *this;
	}

	/// <summary>get descriptor binding</summary>
	/// <param name="bindingId">Binding Index</param>
	/// <param name="descType">The descriptor type to get the layout for.</param>
	/// <returns>The binding layout object (DescriptorBindingLayout)</returns>
	const pvr::types::DescriptorTypeBinding& getBinding(uint16 bindingId, pvr::types::DescriptorType descType)const
	{
		// fetch the generic descriptor type (image, storage buffer, uniform buffer or indirect ray pipeline)
		pvr::types::DescriptorBindingType descriptorType = pvr::types::getDescriptorTypeBinding(descType);

		const pvr::types::DescriptorLayoutBindingStore* bindings[] =
		{
			&_images,
			&_ubos,
			&_ssbos,
			&_indirectRayPipelines,
		};
		return bindings[(uint16)descriptorType]->retrieveDescriptor(bindingId);
	}

	/// <summary>Return the number of images in this object</summary>
	/// <returns>the number of images in this object</returns>
	uint16 getImageCount()const { return _images.itemCount(); }

	/// <summary>Return the total number of array element images in this object</summary>
	/// <returns>the total number of array element images in this object</returns>
	uint16 getTotalArrayElementImageCount()const
	{
		pvr::uint16 numArrayElementImages = 0;
		for (pvr::uint16 i = 0; i < getImageCount(); i++)
		{
			for (pvr::uint16 j = 0; j < getImages()[i]._arraySize; j++)
			{
				numArrayElementImages++;
			}
		}

		return numArrayElementImages;
	}

	/// <summary>Return the list of images</summary>
	const pvr::types::DescriptorBindingLayout* getImages() const { return _images.descriptorBindings(); }

	/// <summary>Return number of ubos in this object</summary>
	/// <returns>The number of ubos in this object</returns>
	uint16 getUboCount()const { return _ubos.itemCount(); }

	/// <summary>Return the total number of array element ubos in this object</summary>
	/// <returns>the total number of array element ubos in this object</returns>
	uint16 getTotalArrayElementUboCount()const
	{
		pvr::uint16 numArrayElementUbos = 0;
		const auto& ubos = getUbos();
		for (pvr::uint16 i = 0; i < getUboCount(); i++)
		{
			numArrayElementUbos += ubos[i]._arraySize;
		}
		return numArrayElementUbos;
	}

	/// <summary>Return the list of ubos</summary>
	const pvr::types::DescriptorBindingLayout* getUbos()const { return _ubos.descriptorBindings(); }

	/// <summary>Return number of ssbos in this object</summary>
	/// <returns>The number of ssbos in this object</returns>
	uint16 getSsboCount()const { return _ssbos.itemCount(); }

	/// <summary>Return the total number of array element ssbos in this object</summary>
	/// <returns>the total number of array element ssbos in this object</returns>
	uint16 getTotalArrayElementSsboCount()const
	{
		pvr::uint16 numArrayElementSsbos = 0;
		for (pvr::uint16 i = 0; i < getSsboCount(); i++)
		{
			for (pvr::uint16 j = 0; j < getSsbos()[i]._arraySize; j++)
			{
				numArrayElementSsbos++;
			}
		}

		return numArrayElementSsbos;
	}

	/// <summary>Return the list of ssbos</summary>
	const pvr::types::DescriptorBindingLayout* getSsbos()const { return _ssbos.descriptorBindings(); }

	/// <summary>Return number of indirect ray pipelines in this object</summary>
	/// <returns>The number of indirect ray pipelines in this object</returns>
	uint16 getIndirectRayPipelineCount()const { return _indirectRayPipelines.itemCount(); }

	/// <summary>Return the total number of array element indirect ray pipelines in this object</summary>
	/// <returns>the total number of array element indirect ray pipelines in this object</returns>
	uint16 getTotalArrayElementIndirectRayPipelineCount()const
	{
		pvr::uint16 numArrayElementIndirectRayPipelines = 0;
		for (pvr::uint16 i = 0; i < getIndirectRayPipelineCount(); i++)
		{
			for (pvr::uint16 j = 0; j < getIndirectRayPipelines()[i]._arraySize; j++)
			{
				numArrayElementIndirectRayPipelines++;
			}
		}

		return numArrayElementIndirectRayPipelines;
	}

	/// <summary>Return the list of indirect ray pipelines</summary>
	const pvr::types::DescriptorBindingLayout* getIndirectRayPipelines()const { return _indirectRayPipelines.descriptorBindings(); }

	/// <summary>Get the total number of bindings (objects) in this object</summary>
	/// <returns>The total number of bindings (objects) in this object</returns>
	uint16 getBindingCount()const { return (uint16)(getImageCount() + getUboCount() + getSsboCount() + getIndirectRayPipelineCount()); }

	/// <summary>Equality operator. Does deep comparison of the contents.</summary>
	/// <param name="rhs">The right-hand side argument of the operator.</param>
	/// <returns>True if the layouts have identical bindings</returns>
	bool operator==(const DescriptorSetLayoutCreateParam& rhs)
	{
		if (getBindingCount() != rhs.getBindingCount()) { return false; }

		return _images == rhs._images && _ubos == rhs._ubos && _ssbos == rhs._ssbos && _indirectRayPipelines == rhs._indirectRayPipelines;
>>>>>>> 1776432f... 4.3
	}
private:
	friend class ::pvr::api::impl::DescriptorSetLayout_;
	friend struct ::pvr::api::DescriptorSetUpdate;
};

namespace impl {
<<<<<<< HEAD
/*!*********************************************************************************************************************
\brief API DescriptorSetLayout. Use through the Reference Counted Framework Object pvr::api::DescriptorSetLayout. Create using
       the IGraphicsContext::createDescriptorSetLayout. A Descriptor Set Layout is required both to construct a descriptor
       set object, and a Pipeline compatible with this object.
***********************************************************************************************************************/
=======
/// <summary>API DescriptorSetLayout. Use through the Reference Counted Framework Object
/// pvr::api::DescriptorSetLayout. Create using the IGraphicsContext::createDescriptorSetLayout. A Descriptor Set
/// Layout is required both to construct a descriptor set object, and a Pipeline compatible with this object.
/// </summary>
>>>>>>> 1776432f... 4.3
class DescriptorSetLayout_
{
private:
	DescriptorSetLayoutCreateParam _desc;
	GraphicsContext _device;

protected:
	template<typename> friend struct ::pvr::RefCountEntryIntrusive;
<<<<<<< HEAD

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
=======

	/// <summary>Constructor</summary>
	/// <param name="context">Context who owns this resource</param>
	/// <param name="desc">Initialization info</param>
	DescriptorSetLayout_(const GraphicsContext& context, const DescriptorSetLayoutCreateParam& desc) : _desc(desc), _device(context)
	{
#ifdef DEBUG
		// only validate uniqueness for the Vulkan api
		if (getContext()->getApiType() > pvr::Api::OpenGLESMaxVersion)
		{
			// for the number of bindings check that there exists a binding to validate linear binding indices
			for (pvr::uint16 i = 0; i < desc.getBindingCount(); i++)
			{
				if (desc._images.hasBinding(i) || desc._ubos.hasBinding(i) || desc._ssbos.hasBinding(i) || desc._indirectRayPipelines.hasBinding(i))
				{
					// in vulkan binding ids must be unique in the descriptor set
					if ((desc._images.hasBinding(i) && desc._ubos.hasBinding(i)) ||
					    (desc._images.hasBinding(i) && desc._ssbos.hasBinding(i)) ||
					    (desc._images.hasBinding(i) && desc._indirectRayPipelines.hasBinding(i)) ||
					    (desc._ubos.hasBinding(i) && desc._ssbos.hasBinding(i)) ||
					    (desc._ubos.hasBinding(i) && desc._indirectRayPipelines.hasBinding(i)) ||
					    (desc._ssbos.hasBinding(i) && desc._indirectRayPipelines.hasBinding(i)))
					{
						debug_assertion(false, "Vulkan requires that descriptor set layout bindings have unique indices within a single set.");
>>>>>>> 1776432f... 4.3
					}
				}
				else
				{
<<<<<<< HEAD
					debug_assertion(false,
					                "Vulkan requires that descriptor set layouts have linear bindings starting at 0");
=======
					debug_assertion(false, "Vulkan requires that descriptor set layouts have linear bindings starting at 0");
>>>>>>> 1776432f... 4.3
				}
			}
		}
#endif
	}
<<<<<<< HEAD
	DescriptorSetLayoutCreateParam desc;
	GraphicsContext device;
public:
=======
>>>>>>> 1776432f... 4.3

public:

	/// <summary>Get the DescriptorSetCreateParam object that was used to create this layout.</summary>
	/// <returns>The DescriptorSetCreateParam object that was used to create this layout.</returns>
	const DescriptorSetLayoutCreateParam& getCreateParam()const { return _desc; }

<<<<<<< HEAD
	/*!*********************************************************************************************************************
	\brief Get the context that this layout belongs to. (const)
	\return The context that this layout belongs to.
	***********************************************************************************************************************/
	const GraphicsContext& getContext()const { return device; }

	/*!
	   \brief destructor
	 */
=======
	/// <summary>Clear the descriptor set layout create param list.</summary>
	void clearCreateParam() { _desc.clear(); }

	/// <summary>Get the context that this layout belongs to.</summary>
	/// <returns>The context that this layout belongs to.</returns>
	GraphicsContext& getContext() { return _device; }

	/// <summary>Get the context that this layout belongs to. (const)</summary>
	/// <returns>The context that this layout belongs to.</returns>
	const GraphicsContext& getContext()const { return _device; }

	/// <summary>destructor</summary>
>>>>>>> 1776432f... 4.3
	virtual ~DescriptorSetLayout_() { }
};
}// impl

/// <summary>Descriptor Pool create parameter.</summary>
struct DescriptorPoolCreateParam
{
private:
<<<<<<< HEAD
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
=======
	std::pair<types::DescriptorType, pvr::uint16> _descriptorTypes[pvr::uint32(pvr::types::DescriptorType::Count)];
	pvr::uint16 _numberOfDescriptorTypes;
	pvr::uint16 _maxSets;
public:

	/// <summary>Constructor</summary>
	DescriptorPoolCreateParam() : _numberOfDescriptorTypes(0), _maxSets(200) {}

	/// <summary>Add the maximum number of the specified descriptor types that the pool will contain.</summary>
	/// <param name="descType">Descriptor type</param>
	/// <param name="count">Maximum number of descriptors of (type)</param>
	/// <returns>this (allow chaining)</returns>
>>>>>>> 1776432f... 4.3
	DescriptorPoolCreateParam& addDescriptorInfo(types::DescriptorType descType, pvr::uint16 count)
	{
		_descriptorTypes[_numberOfDescriptorTypes] = std::make_pair(descType, count);
		_numberOfDescriptorTypes++;
		return *this;
	}

	/// <summary>Set the maximum number of descriptor sets.</summary>
	/// <param name="maxSets">The maximum number of descriptor sets</param>
	/// <returns>this (allow chaining)</returns>
	DescriptorPoolCreateParam& setMaxDescriptorSets(pvr::uint16 maxSets)
	{
		this->_maxSets = maxSets; return *this;
	}

<<<<<<< HEAD
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
=======
	/// <summary>Get the number of allocations of a descriptor type is supported on this pool.</summary>
	/// <param name="descType">DescriptorType</param>
	/// <returns>Number of allocations.</returns>
	pvr::uint16 getDescriptorTypeCount(types::DescriptorType descType) const
	{
		for (pvr::uint16 i = 0; i < _numberOfDescriptorTypes; i++)
		{
			if (_descriptorTypes[i].first == descType)
			{
				return _descriptorTypes[i].second;
			}
		}
		return 0;
	}

	/// <summary>Get maximum sets supported on this pool.</summary>
	pvr::uint16 getMaxSetCount() const { return _maxSets; }
>>>>>>> 1776432f... 4.3

};

namespace impl {
/// <summary>API DescriptorPool Object wrapper. Access through the framework-managed DescriptorPool object.
/// </summary>
class DescriptorPool_
{
<<<<<<< HEAD
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
=======
public:
	virtual ~DescriptorPool_() {}
	/// <summary>Return the context</summary>
	const GraphicsContext& getContext()const { return _context; }

	/// <summary>Return the context</summary>
	GraphicsContext& getContext() { return _context; }

	/// <summary>Constructor. Do not use directly.</summary>
	/// <param name="device">the device</param>
	DescriptorPool_(const GraphicsContext& device) : _context(device) {}

	/// <summary>Allocate descriptor set</summary>
	/// <param name="layout">Descriptor set layout</param>
	/// <returns>Return DescritptorSet else null if fails.</returns>
	api::DescriptorSet allocateDescriptorSet(const DescriptorSetLayout& layout)
	{ return allocateDescriptorSet_(layout); }

private:
	virtual api::DescriptorSet allocateDescriptorSet_(const DescriptorSetLayout& layout) = 0;

	GraphicsContext _context;
>>>>>>> 1776432f... 4.3
};
}//namespace impl

/// <summary>This class contains all the information necessary to populate a Descriptor Set with the actual API
/// objects. Use with the method update of the DescriptorSet. Populate this object with actual Descriptor objects
/// (UBOs, textures etc).</summary>
struct DescriptorSetUpdate
{
<<<<<<< HEAD
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
=======
	/// <summary>Internal class</summary>
	struct DescriptorSampler
	{
		pvr::api::Sampler _sampler; //!<Internal object
		bool _useSampler; //!<Internal object
	};

	typedef std::pair<DescriptorSampler, pvr::api::TextureView> Image;

	pvr::types::DescriptorUpdateBindingStore<Image> _images;//!<Internal
	pvr::types::DescriptorUpdateBindingStore<api::BufferView> _ubos;//!<Internal
	pvr::types::DescriptorUpdateBindingStore<api::BufferView> _ssbos;//!<Internal
	pvr::types::DescriptorUpdateBindingStore<std::pair<pvr::api::TextureView, pvr::uint16>> _accumulationImages;//!<Internal
	pvr::types::DescriptorUpdateBindingStore<api::IndirectRayPipeline> _indirectRayPipelines;//!<Internal

public:
	/// <summary>Constructor.</summary>
	DescriptorSetUpdate(types::DescriptorSetUsage usage = types::DescriptorSetUsage::Static) :  _usage(usage)
	{
		_images.clear();
		_ubos.clear();
		_ssbos.clear();
		_accumulationImages.clear();
		_indirectRayPipelines.clear();
	}

	/// <summary>Return number of images</summary>
	uint16 getImageCount()const { return _images.itemCount(); }

	/// <summary>Return the list of images</summary>
	const pvr::types::DescriptorItemBinding<Image>* getImages()const { return _images.descriptorBindings(); }

	/// <summary>Return number of ubos</summary>
	uint16 getUboCount()const { return _ubos.itemCount(); }

	/// <summary>Return the list of ubos</summary>
	const pvr::types::DescriptorItemBinding<BufferView>* getUbos()const { return _ubos.descriptorBindings(); }

	/// <summary>Return number of ssbos</summary>
	uint16 getSsboCount()const { return _ssbos.itemCount(); }

	/// <summary>Return the list of ssbos</summary>
	const pvr::types::DescriptorItemBinding<BufferView>* getSsbos()const { return _ssbos.descriptorBindings(); }

	/// <summary>Return number of accumulation images</summary>
	uint16 getAccumulationImageCount()const { return _accumulationImages.itemCount(); }

	/// <summary>Return the list of accumulation images</summary>
	const pvr::types::DescriptorItemBinding<std::pair<pvr::api::TextureView, pvr::uint16>>* getAccumulationImages()const { return _accumulationImages.descriptorBindings(); }

	/// <summary>Return number of indirect ray pipelines</summary>
	uint16 getIndirectRayPipelineCount()const { return _indirectRayPipelines.itemCount(); }

	/// <summary>Return the list of indirect ray pipelines</summary>
	const pvr::types::DescriptorItemBinding<IndirectRayPipeline>* getIndirectRayPipelines()const { return _indirectRayPipelines.descriptorBindings(); }

	/// <summary>Get the total number of bindings (objects) in this object</summary>
	/// <returns>The total number of bindings (objects) in this object</returns>
	uint16 getBindingCount()const { return (uint16)(getImageCount() + getUboCount() + getSsboCount() + getIndirectRayPipelineCount() + getAccumulationImageCount()); }

	/// <summary>Add a Ubo to the specified binding index.</summary>
	/// <param name="bindingId">The index of the indexed binding point</param>
	/// <param name="item">The object to add</param>
	/// <returns>this object (allow chaining)</returns>
	DescriptorSetUpdate& setUbo(pvr::uint16 bindingId, const api::BufferView& item)
	{
		assertion(static_cast<pvr::uint16>(item->getResource()->getBufferUsage() & pvr::types::BufferBindingUse::UniformBuffer) != 0,
>>>>>>> 1776432f... 4.3
		          "DescriptorSetUpdate::setUbo - buffer doesn't support ubo binding");
		return addBuffer(bindingId, 0, types::DescriptorType::UniformBuffer, item);
	}

<<<<<<< HEAD
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
=======
	/// <summary>Add a Ubo to the specified binding index. Supports array-indexing in the shader.</summary>
	/// <param name="bindingId">The index of the indexed binding point</param>
	/// <param name="arrayIndex">If supported by the underlying API, add to which index of the array binding point.
	/// </param>
	/// <param name="item">The object to add</param>
	/// <returns>this object (allow chaining)</returns>
	DescriptorSetUpdate& setUboAtIndex(pvr::uint16 bindingId, pvr::uint16 arrayIndex, const api::BufferView& item)
	{
		assertion(static_cast<pvr::uint16>(item->getResource()->getBufferUsage() & pvr::types::BufferBindingUse::UniformBuffer) != 0,
>>>>>>> 1776432f... 4.3
		          "DescriptorSetUpdate::setUboAtIndex - buffer doesn't support ubo binding");
		return addBuffer(bindingId, arrayIndex, types::DescriptorType::UniformBuffer, item);
	}

<<<<<<< HEAD
	/*!*********************************************************************************************************************
	\brief  Add a Ubo to the specified binding index.
	\param  bindingId The index of the indexed binding point
	\param  item The object to add
	\return this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setDynamicUbo(pvr::uint8 bindingId, const api::BufferView& item)
	{
		assertion(static_cast<pvr::uint32>(item->getResource()->getBufferUsage() & pvr::types::BufferBindingUse::UniformBuffer) != 0,
=======
	/// <summary>Add a Ubo to the specified binding index.</summary>
	/// <param name="bindingId">The index of the indexed binding point</param>
	/// <param name="item">The object to add</param>
	/// <returns>this object (allow chaining)</returns>
	DescriptorSetUpdate& setDynamicUbo(pvr::uint16 bindingId, const api::BufferView& item)
	{
		assertion(static_cast<pvr::uint16>(item->getResource()->getBufferUsage() & pvr::types::BufferBindingUse::UniformBuffer) != 0,
>>>>>>> 1776432f... 4.3
		          "DescriptorSetUpdate::setDynamicUbo - buffer doesn't support ubo binding");
		return addBuffer(bindingId, 0, types::DescriptorType::UniformBufferDynamic, item);
	}

<<<<<<< HEAD
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
=======
	/// <summary>Add a Ubo to the specified binding index. Supports array-indexing in the shader.</summary>
	/// <param name="bindingId">The index of the indexed binding point</param>
	/// <param name="arrayIndex">If supported by the underlying API, add to which index of the array binding point.
	/// </param>
	/// <param name="item">The object to add</param>
	/// <returns>this object (allow chaining)</returns>
	DescriptorSetUpdate& setDynamicUboAtIndex(pvr::uint16 bindingId, pvr::uint16 arrayIndex, const api::BufferView& item)
	{
		assertion(static_cast<pvr::uint16>(item->getResource()->getBufferUsage() & pvr::types::BufferBindingUse::UniformBuffer) != 0,
>>>>>>> 1776432f... 4.3
		          "DescriptorSetUpdate::setDynamicUboAtIndex - buffer doesn't support ubo binding");
		return addBuffer(bindingId, arrayIndex, types::DescriptorType::UniformBufferDynamic, item);
	}

<<<<<<< HEAD
	/*!*********************************************************************************************************************
	\brief  Add a Ssbo to the specified binding index.
	\param  bindingId The index of the indexed binding point
	\param  item The object to add
	\return this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setSsbo(pvr::uint8 bindingId, const api::BufferView& item)
	{
		assertion(static_cast<pvr::uint32>(item->getResource()->getBufferUsage() & pvr::types::BufferBindingUse::StorageBuffer) != 0,
=======
	/// <summary>Add a Ssbo to the specified binding index.</summary>
	/// <param name="bindingId">The index of the indexed binding point</param>
	/// <param name="item">The object to add</param>
	/// <returns>this object (allow chaining)</returns>
	DescriptorSetUpdate& setSsbo(pvr::uint16 bindingId, const api::BufferView& item)
	{
		assertion(static_cast<pvr::uint16>(item->getResource()->getBufferUsage() & pvr::types::BufferBindingUse::StorageBuffer) != 0,
>>>>>>> 1776432f... 4.3
		          "DescriptorSetUpdate::setSsbo - buffer doesn't support ssbo binding");
		return addBuffer(bindingId, 0, types::DescriptorType::StorageBuffer, item);
	}

<<<<<<< HEAD
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
=======
	/// <summary>Add an Ssbo to the specified binding index. Supports array-indexing in the shader.</summary>
	/// <param name="bindingId">The index of the indexed binding point</param>
	/// <param name="arrayIndex">If supported by the underlying API, add to which index of the array binding point.
	/// </param>
	/// <param name="item">The object to add</param>
	/// <returns>this object (allow chaining)</returns>
	DescriptorSetUpdate& setSsboAtIndex(pvr::uint16 bindingId, pvr::uint16 arrayIndex, const api::BufferView& item)
	{
		assertion(static_cast<pvr::uint16>(item->getResource()->getBufferUsage() & pvr::types::BufferBindingUse::StorageBuffer) != 0,
>>>>>>> 1776432f... 4.3
		          "DescriptorSetUpdate::setSsboAtIndex - buffer doesn't support ssbo binding");
		return addBuffer(bindingId, arrayIndex, types::DescriptorType::StorageBuffer, item);
	}

<<<<<<< HEAD
	/*!*********************************************************************************************************************
	\brief  Add a Ssbo to the specified binding index.
	\param  bindingId The index of the indexed binding point
	\param  item The object to add
	\return this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setDynamicSsbo(pvr::uint8 bindingId, const api::BufferView& item)
	{
		assertion(static_cast<pvr::uint32>(item->getResource()->getBufferUsage() & pvr::types::BufferBindingUse::StorageBuffer) != 0,
=======
	/// <summary>Add a Ssbo to the specified binding index.</summary>
	/// <param name="bindingId">The index of the indexed binding point</param>
	/// <param name="item">The object to add</param>
	/// <returns>this object (allow chaining)</returns>
	DescriptorSetUpdate& setDynamicSsbo(pvr::uint16 bindingId, const api::BufferView& item)
	{
		assertion(static_cast<pvr::uint16>(item->getResource()->getBufferUsage() & pvr::types::BufferBindingUse::StorageBuffer) != 0,
>>>>>>> 1776432f... 4.3
		          "DescriptorSetUpdate::setDynamicSsbo - buffer doesn't support ssbo binding");
		return addBuffer(bindingId, 0, types::DescriptorType::StorageBufferDynamic, item);
	}

<<<<<<< HEAD
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
=======
	/// <summary>Add an Ssbo to the specified binding index. Supports array-indexing in the shader.</summary>
	/// <param name="bindingId">The index of the indexed binding point</param>
	/// <param name="arrayIndex">If supported by the underlying API, add to which index of the array binding point.
	/// </param>
	/// <param name="item">The object to add</param>
	/// <returns>this object (allow chaining)</returns>
	DescriptorSetUpdate& setDynamicSsboAtIndex(pvr::uint16 bindingId, pvr::uint16 arrayIndex, const api::BufferView& item)
	{
		assertion(static_cast<pvr::uint16>(item->getResource()->getBufferUsage() & pvr::types::BufferBindingUse::StorageBuffer) != 0,
>>>>>>> 1776432f... 4.3
		          "DescriptorSetUpdate::setUbo - buffer doesn't support ssbo binding");
		return addBuffer(bindingId, arrayIndex, types::DescriptorType::StorageBufferDynamic, item);
	}

<<<<<<< HEAD
	/*!*********************************************************************************************************************
	\brief  Create a CombinedImageSampler from the provided texture and sampler, and add it to the specified index.
	\param  bindingId The index of the indexed binding point
	\param  texture The texture to add
	\param  sampler The sampler to add
	\return this object (allow chaining)
	***********************************************************************************************************************/
	DescriptorSetUpdate& setCombinedImageSampler(pvr::uint8 bindingId, const pvr::api::TextureView& texture,
=======
	/// <summary>Create a CombinedImageSampler from the provided texture and sampler, and add it to the specified index.
	/// </summary>
	/// <param name="bindingId">The index of the indexed binding point</param>
	/// <param name="texture">The texture to add</param>
	/// <param name="sampler">The sampler to add</param>
	/// <returns>this object (allow chaining)</returns>
	DescriptorSetUpdate& setCombinedImageSampler(pvr::uint16 bindingId, const pvr::api::TextureView& texture,
>>>>>>> 1776432f... 4.3
	    const pvr::api::Sampler& sampler)
	{
		return addImageSampler(bindingId, 0, texture, sampler, types::DescriptorType::CombinedImageSampler);
	}

<<<<<<< HEAD
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
=======
	/// <summary>Create a CombinedImageSampler from the provided texture and sampler, and add it to the specified index.
	/// Supports array-indexing in the shader</summary>
	/// <param name="bindingId">The index of the indexed binding point</param>
	/// <param name="arrayIndex">If supported by the underlying API, add to which index of the array binding point.
	/// </param>
	/// <param name="texture">The texture to add</param>
	/// <param name="sampler">The sampler to add</param>
	/// <returns>this object (allow chaining)</returns>
	DescriptorSetUpdate& setCombinedImageSamplerAtIndex(pvr::uint16 bindingId, pvr::uint16 arrayIndex,
	    const pvr::api::TextureView& texture, const pvr::api::Sampler& sampler)
	{
		return addImageSampler(bindingId, arrayIndex, texture, sampler, types::DescriptorType::CombinedImageSampler);
	}

	/// <summary>Create a Input Attachment from the provided texture and sampler, and add it to the specified index.
	/// </summary>
	/// <param name="bindingId">The index of the indexed binding point</param>
	/// <param name="texture">The texture to add</param>
	/// <returns>this object (allow chaining)</returns>
	DescriptorSetUpdate& setInputImageAttachment(pvr::uint16 bindingId, const pvr::api::TextureView& texture)
	{
		return addInputAttachment(bindingId, 0, texture, types::DescriptorType::InputAttachment);
	}

	/// <summary>Create a Input Attachment from the provided texture and sampler, and add it to the specified index.
	/// Supports array-indexing in the shader</summary>
	/// <param name="bindingId">The index of the indexed binding point</param>
	/// <param name="arrayIndex">If supported by the underlying API, add to which index of the array binding point.
	/// </param>
	/// <param name="texture">The texture to add</param>
	/// <returns>this object (allow chaining)</returns>
	DescriptorSetUpdate& setInputImageAttachmentAtIndex(pvr::uint16 bindingId, pvr::uint16 arrayIndex,
	    const pvr::api::TextureView& texture)
	{
		return addInputAttachment(bindingId, arrayIndex, texture, types::DescriptorType::InputAttachment);
	}

	/// <summary>Create a storage image from the provided texture and add it to the specified index.</summary>
	/// <param name="bindingId">The index of the indexed binding point</param>
	/// <param name="texture">The texture to add</param>
	/// <returns>this object (allow chaining)</returns>
	DescriptorSetUpdate& setStorageImage(pvr::uint16 bindingId, const pvr::api::TextureView& texture)
	{
		return addStorageImage(bindingId, 0, texture, types::DescriptorType::StorageImage);
	}

	/// <summary>Create a storage image from the provided texture and add it to the specified index.
	/// Supports array-indexing in the shader</summary>
	/// <param name="bindingId">The index of the indexed binding point</param>
	/// <param name="arrayIndex">If supported by the underlying API, add to which index of the array binding point.
	/// </param>
	/// <param name="texture">The texture to add</param>
	/// <returns>this object (allow chaining)</returns>
	DescriptorSetUpdate& setStorageImageAtIndex(pvr::uint16 bindingId, pvr::uint16 arrayIndex,
	    const pvr::api::TextureView& texture)
	{
		return addStorageImage(bindingId, arrayIndex, texture, types::DescriptorType::StorageImage);
	}

	/// <summary>Add an indirect ray pipeline to the specified index.</summary>
	/// <param name="bindingId">The index of the indexed binding point</param>
	/// <param name="indirectRayPipeline">The ray pipeline to add</param>
	/// <returns>this object (allow chaining)</returns>
	DescriptorSetUpdate& setIndirectRayPipeline(pvr::uint16 bindingId, const pvr::api::IndirectRayPipeline& indirectRayPipeline)
	{
		return addIndirectRayPipeline(bindingId, 0, indirectRayPipeline, types::DescriptorType::IndirectRayPipeline);
	}

	/// <summary>Add an indirect ray pipeline to the specified index.
	/// Supports array-indexing in the shader</summary>
	/// <param name="bindingId">The index of the indexed binding point</param>
	/// <param name="arrayIndex">If supported by the underlying API, add to which index of the array binding point.
	/// </param>
	/// <param name="indirectRayPipeline">The ray pipeline to add</param>
	/// <returns>this object (allow chaining)</returns>
	DescriptorSetUpdate& setIndirectRayPipelineAtIndex(pvr::uint16 bindingId, pvr::uint16 arrayIndex,
	    const pvr::api::IndirectRayPipeline& indirectRayPipeline)
	{
		return addIndirectRayPipeline(bindingId, arrayIndex, indirectRayPipeline, types::DescriptorType::IndirectRayPipeline);
	}

	/// <summary>Add an accumulation image to the specified index.</summary>
	/// <param name="bindingId">The index of the indexed binding point</param>
	/// <param name="texture">The Accumulation image to add</param>
	/// <param name="imageAttachmentPoint">The global binding point for the accumulation image</param>
	/// <returns>this object (allow chaining)</returns>
	DescriptorSetUpdate& setAccumulationImage(pvr::uint16 bindingId, const pvr::api::TextureView& texture, pvr::uint16 imageAttachmentPoint)
	{
		return addAccumulationImage(bindingId, 0, texture, types::DescriptorType::StorageImage, imageAttachmentPoint);
	}

	/// <summary>Add an accumulation image to the specified index.
	/// Supports array-indexing in the shader</summary>
	/// <param name="bindingId">The index of the indexed binding point</param>
	/// <param name="arrayIndex">If supported by the underlying API, add to which index of the array binding point.
	/// </param>
	/// <param name="texture">The Accumulation image to add</param>
	/// <param name="imageAttachmentPoint">The global binding point for the accumulation image</param>
	/// <returns>this object (allow chaining)</returns>
	DescriptorSetUpdate& setAccumulationImageAtIndex(pvr::uint16 bindingId, pvr::uint16 arrayIndex,
	    const pvr::api::TextureView& texture, pvr::uint16 imageAttachmentPoint)
	{
		return addAccumulationImage(bindingId, arrayIndex, texture, types::DescriptorType::StorageImage, imageAttachmentPoint);
	}

	/// <summary>Clear all entries</summary>
	/// <returns>Return this object for chaining</returns>
	DescriptorSetUpdate& clear()
	{
		_images.clear();
		_ssbos.clear();
		_ubos.clear();
		_indirectRayPipelines.clear();
		_accumulationImages.clear();
>>>>>>> 1776432f... 4.3

		return *this;
	}

<<<<<<< HEAD
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
=======
private:
	friend class ::pvr::api::impl::DescriptorSet_;

	DescriptorSetUpdate& addBuffer(pvr::uint16 bindingId, pvr::uint16 arrayIndex, types::DescriptorType type, const api::BufferView& item)
	{
		assertion(item.isValid(), "Invalid Buffer Item");

		// fetch the generic descriptor type (image, storage buffer, uniform buffer, accumulation image or indirect ray pipeline)
		pvr::types::DescriptorBindingType descriptorType = pvr::types::getDescriptorTypeBinding(type);

		// only increment the count for a new binding at a specific binding index
		switch (descriptorType)
		{
		case pvr::types::DescriptorBindingType::StorageBuffer:
			_ssbos.add(pvr::types::DescriptorItemBinding<pvr::api::BufferView>(bindingId, arrayIndex, type, item));
			debug_assertion(_ssbos.retrieveDescriptor(bindingId, arrayIndex).isValid(), "Added storage buffer is not valid");
			break;
		case pvr::types::DescriptorBindingType::UniformBuffer:
			_ubos.add(pvr::types::DescriptorItemBinding<pvr::api::BufferView>(bindingId, arrayIndex, type, item));
			debug_assertion(_ubos.retrieveDescriptor(bindingId, arrayIndex).isValid(), "Added uniform buffer is not valid");
			break;
		default:
			assertion(false, "Unsupported descriptor type");
			Log(pvr::Logger::Severity::Error, "Unsupported descriptor type");
		}

		return *this;
	}

	inline DescriptorSetUpdate& addImage(
	  pvr::uint16 bindingId, pvr::uint16 arrayIndex, const pvr::api::TextureView& texture,
	  const pvr::api::Sampler& sampler, const pvr::types::DescriptorType type, bool useSampler)
	{
		assertion(texture.isValid(), "DescriptorSet update addImage invalid texture object");
		assertion(!useSampler || sampler.isValid(), "DescriptorSet update addImage invalid sampler object");

		DescriptorSampler descriptorSampler{ sampler, useSampler };

		// fetch the generic descriptor type (image, storage buffer, uniform buffer, accumulation image or indirect ray pipeline)
		pvr::types::DescriptorBindingType descriptorType = pvr::types::getDescriptorTypeBinding(type);

		assertion(descriptorType == pvr::types::DescriptorBindingType::Image, "Unsupported descriptor type");

		_images.add(pvr::types::DescriptorItemBinding<Image>(bindingId, arrayIndex, type, std::make_pair(descriptorSampler, texture)));
		debug_assertion(_images.retrieveDescriptor(bindingId, arrayIndex).isValid(), "Added image is not valid");
>>>>>>> 1776432f... 4.3

		return *this;
	}

<<<<<<< HEAD
	DescriptorSetUpdate& addImageSampler(pvr::uint8 bindingId, pvr::uint8 arrayIndex, const pvr::api::TextureView& texture,
	                                     const pvr::api::Sampler& sampler, const pvr::types::DescriptorType type)
	{
		return addImage(bindingId, arrayIndex, texture, sampler, type, true);
	}

	DescriptorSetUpdate& addInputAttachment(pvr::uint8 bindingId, pvr::uint8 arrayIndex, const pvr::api::TextureView& texture,
	                                        const pvr::types::DescriptorType type)
=======
	DescriptorSetUpdate& addImageSampler(pvr::uint16 bindingId, pvr::uint16 arrayIndex, const pvr::api::TextureView& texture,
	                                     const pvr::api::Sampler& sampler, const pvr::types::DescriptorType type)
	{
		return addImage(bindingId, arrayIndex, texture, sampler, type, true);
	}

	DescriptorSetUpdate& addInputAttachment(pvr::uint16 bindingId, pvr::uint16 arrayIndex, const pvr::api::TextureView& texture,
	                                        const pvr::types::DescriptorType type)
	{
		return addImage(bindingId, arrayIndex, texture, pvr::api::Sampler(), type, false);
	}

	DescriptorSetUpdate& addStorageImage(pvr::uint16 bindingId, pvr::uint16 arrayIndex, const pvr::api::TextureView& texture,
	                                     const pvr::types::DescriptorType type)
>>>>>>> 1776432f... 4.3
	{
		return addImage(bindingId, arrayIndex, texture, pvr::api::Sampler(), type, false);
	}

<<<<<<< HEAD
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
=======
	DescriptorSetUpdate& addAccumulationImage(pvr::uint16 bindingId, pvr::uint16 arrayIndex, const pvr::api::TextureView& accumImage,
	    const pvr::types::DescriptorType type, pvr::uint16 imageAttachmentPoint)
	{
		if (!accumImage.isValid())
		{
			assertion(accumImage.isValid(), "Invalid Accumulation image Item");
			Log("DescriptorSet update addAccumulationImage invalid accumulation image object");
			return *this;
		}

		assertion(type == pvr::types::DescriptorType::StorageImage, "An accumulation image must be used as descriptor of type storage image");

		_accumulationImages.add(pvr::types::DescriptorItemBinding<std::pair<pvr::api::TextureView, pvr::uint16>>(bindingId, arrayIndex, type, std::make_pair(accumImage, imageAttachmentPoint)));
		debug_assertion(_accumulationImages.retrieveDescriptor(bindingId, arrayIndex).isValid(), "Added accumulation image is not valid");

		return *this;
	}

	DescriptorSetUpdate& addIndirectRayPipeline(pvr::uint16 bindingId, pvr::uint16 arrayIndex, const pvr::api::IndirectRayPipeline& indirectRayPipeline,
	    const pvr::types::DescriptorType type)
	{
		if (!indirectRayPipeline.isValid())
		{
			assertion(indirectRayPipeline.isValid(), "Invalid indirectRayPipeline Item");
			Log("DescriptorSet update addIndirectRayPipeline invalid indirectRayPipeline object");
			return *this;
		}

		_indirectRayPipelines.add(pvr::types::DescriptorItemBinding<pvr::api::IndirectRayPipeline>(bindingId, arrayIndex, type, indirectRayPipeline));
		debug_assertion(_indirectRayPipelines.retrieveDescriptor(bindingId, arrayIndex).isValid(), "Added indirect ray pipeline is not valid");

		return *this;
	}

	types::DescriptorSetUsage _usage;
};

namespace impl {
/// <summary>A descriptor set object. Carries all memory-related API object state like Textures (Images),
/// Samplers, UBOs, Ssbos etc. Does NOT carry pipeline specific state such as Vertex/Index buffers, shader programs
/// etc (these are part of the Pipeline objects).</summary>
class DescriptorSet_
{
public:
	typedef uint16 IndexType;
	/// <summary>Create a DescriptorSet on a specific DescriptorPool.</summary>
	/// <summary>createParam DescriptorSet creation information.</summary>
	/// <summary>pool A DescriptorPool to create the DescriptorSet on.</summary>
>>>>>>> 1776432f... 4.3
	DescriptorSet_(const api::DescriptorSetLayout& descSetLayout, const api::DescriptorPool& pool) :
		_descSetLayout(descSetLayout), _descPool(pool) {}

	/// <summary>Destructor. Frees all resources owned by this object.</summary>
	virtual ~DescriptorSet_() { }

	/// <summary>Return the layout of this DescriptorSet.</summary>
	/// <returns>This DescriptorSet's DescriptorSetLayout</returns>
	const api::DescriptorSetLayout& getDescriptorSetLayout()const {return _descSetLayout;}

<<<<<<< HEAD
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
=======
	/// <summary>Return the descriptor pool (const)</summary>
	const DescriptorPool& getDescriptorPool()const { return _descPool; }

	/// <summary>Return the descriptor pool</summary>
	DescriptorPool& getDescriptorPool() { return _descPool; }

	/// <summary>Return the graphics context (const)</summary>
	const GraphicsContext& getContext()const { return _descPool->getContext(); }

	/// <summary>Return the graphics context (const)</summary>
	GraphicsContext& getContext() { return _descPool->getContext(); }

	/// <summary>Update this</summary>
	/// <param name="descSet">Descriptor set update param. Note application should externally synchronize if this descriptor
	/// set maybe used by the gpu durring the update.</param>
	/// <returns>Return true on success</returns>
	bool update(const DescriptorSetUpdate& descSet)
	{ return update_(descSet); }
protected:
	DescriptorSetLayout _descSetLayout;
	DescriptorPool _descPool;
	DescriptorSetUpdate _descParam;
private:
	virtual bool update_(const DescriptorSetUpdate& descSet) = 0;
>>>>>>> 1776432f... 4.3
};
}// namespace impl
}// namspace api

}
