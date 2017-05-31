<<<<<<< HEAD
/*!*********************************************************************************************************************
\file         PVRApi/ApiObjects/PipelineLayout.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains the PipelineLayout class. A PipelineLayout is an object that describes the types and amounts
              of any memory objects (Buffers, Textures etc) that a pipeline will be using. It comprises of DescriptorSetLayouts.
***********************************************************************************************************************/
=======
/*!
\brief Contains the PipelineLayout class. A PipelineLayout is an object that describes the types and amounts of any
memory objects (Buffers, Textures etc) that a pipeline will be using. It comprises of DescriptorSetLayouts.
\file         PVRApi/ApiObjects/PipelineLayout.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
*/
>>>>>>> 1776432f... 4.3
#pragma once
#include "PVRApi/ApiIncludes.h"
namespace pvr {
namespace api {

/// <summary>Pipeline Layout create information. The descriptor set layouts must be known to create a Pipeline layout.
/// </summary>
struct PipelineLayoutCreateParam
{
<<<<<<< HEAD
	/*!*********************************************************************************************************************
	\brief Add a descriptor set layout to this pipeline layout. Added to the end of the list of layouts.
	\param[in] descLayout A descriptor set layout
	\return this (allow chaining)
	***********************************************************************************************************************/
	PipelineLayoutCreateParam& addDescSetLayout(const api::DescriptorSetLayout& descLayout)
	{
		assertion(size < (uint32)FrameworkCaps::MaxDescriptorSetBindings, "PipelineLayoutCreateParam: Descriptor Set index cannot be 4 or greater");
		m_descLayout[size++] = descLayout;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Add a descriptor set layout to this pipeline layout. Added to the specified index.
	\param[in] index The index where the layout will be created on
	\param[in] descLayout A descriptor set layout
	\return this (allow chaining)
	***********************************************************************************************************************/
	PipelineLayoutCreateParam& setDescSetLayout(pvr::uint32 index, const api::DescriptorSetLayout& descLayout)
	{
#ifdef DEBUG
		if (index >= (uint32)FrameworkCaps::MaxDescriptorSetBindings)
		{
			assertion(index < (uint32)FrameworkCaps::MaxDescriptorSetBindings, "PipelineLayoutCreateParam: Descriptor Set index cannot be 4 or greater");
			pvr::Log("PipelineLayoutCreateParam::getDescriptorSetLayout - Invalid DescriptorSetLayout Index");
		}
#endif
		if (index >= (uint32)size) { size = (uint8)index + 1; }
		m_descLayout[index] = descLayout;
		return *this;
	}

	/*!*********************************************************************************************************************
	  \brief Return number of descriptor set layouts
	  ***********************************************************************************************************************/
	pvr::uint32 getNumDescSetLayouts()const { return size; }

	/*!*********************************************************************************************************************
	  \brief Get descriptor set layout
	\param index Descriptor set layout index
	\return Descritpor set layout
	  ***********************************************************************************************************************/
	const api::DescriptorSetLayout& getDescriptorSetLayout(pvr::uint32 index)const
	{
#ifdef DEBUG
		if (index >= size)
		{
			assertion(false ,  "Invalid DescriptorSetLayout Index");
			pvr::Log("PipelineLayoutCreateParam::getDescriptorSetLayout - Invalid DescriptorSetLayout Index");
		}
#endif
		return m_descLayout[index];
	}

	/*!
	   \brief Clear the entries
	 */
=======
	/// <summary>Add a descriptor set layout to this pipeline layout. Added to the end of the list of layouts.
	/// </summary>
	/// <param name="descLayout">A descriptor set layout</param>
	/// <returns>this (allow chaining)</returns>
	PipelineLayoutCreateParam& addDescSetLayout(const api::DescriptorSetLayout& descLayout)
	{
		assertion(size < (uint32)FrameworkCaps::MaxDescriptorSetBindings, "PipelineLayoutCreateParam: Descriptor Set index cannot be 4 or greater");
		_descLayout[size++] = descLayout;
		return *this;
	}

	/// <summary>Add a descriptor set layout to this pipeline layout. Added to the specified index.</summary>
	/// <param name="index">The index where the layout will be created on</param>
	/// <param name="descLayout">A descriptor set layout</param>
	/// <returns>this (allow chaining)</returns>
	PipelineLayoutCreateParam& setDescSetLayout(pvr::uint32 index, const api::DescriptorSetLayout& descLayout)
	{
#ifdef DEBUG
		if (index >= (uint32)FrameworkCaps::MaxDescriptorSetBindings)
		{
			assertion(index < (uint32)FrameworkCaps::MaxDescriptorSetBindings, "PipelineLayoutCreateParam: Descriptor Set index cannot be 4 or greater");
			pvr::Log("PipelineLayoutCreateParam::getDescriptorSetLayout - Invalid DescriptorSetLayout Index");
		}
#endif
		if (index >= (uint32)size) { size = (uint8)index + 1; }
		_descLayout[index] = descLayout;
		return *this;
	}

	/// <summary>Return number of descriptor set layouts</summary>
	pvr::uint32 getNumDescSetLayouts()const { return size; }

	/// <summary>Get descriptor set layout</summary>
	/// <param name="index">Descriptor set layout index</param>
	/// <returns>Descritpor set layout</returns>
	const api::DescriptorSetLayout& getDescriptorSetLayout(pvr::uint32 index)const
	{
#ifdef DEBUG
		if (index >= size)
		{
			assertion(false, "Invalid DescriptorSetLayout Index");
			pvr::Log("PipelineLayoutCreateParam::getDescriptorSetLayout - Invalid DescriptorSetLayout Index");
		}
#endif
		return _descLayout[index];
	}

	/// <summary>Clear the entries</summary>
>>>>>>> 1776432f... 4.3
	void clear()
	{
		for (size_t i = 0; i < size; ++i)
		{
<<<<<<< HEAD
			m_descLayout[i].reset();
=======
			_descLayout[i].reset();
>>>>>>> 1776432f... 4.3
		}
		size = 0;
	}

<<<<<<< HEAD
	/*!*********************************************************************************************************************
	\brief Get all descriptor set layouts
	\return An array of 4 descriptor set layouts. Unused one will be empty references (isNull() returns true)
	***********************************************************************************************************************/
	const std::array<api::DescriptorSetLayout, (uint32)FrameworkCaps::MaxDescriptorSetBindings>& getDescriptorSetLayouts()const
	{
		return m_descLayout;
	}

	/*!
	   \brief operator ==
	   \param rhs
	   \return Return true if equal
	 */
=======
	/// <summary>Get all descriptor set layouts</summary>
	/// <returns>An array of 4 descriptor set layouts. Unused one will be empty references (isNull() returns true)
	/// </returns>
	const std::array<api::DescriptorSetLayout, (uint32)FrameworkCaps::MaxDescriptorSetBindings>& getDescriptorSetLayouts()const
	{
		return _descLayout;
	}

>>>>>>> 1776432f... 4.3
	bool operator==(const PipelineLayoutCreateParam& rhs) const
	{
		if (size != rhs.size) { return false; }
		for (size_t i = 0; i < size; ++i)
		{
<<<<<<< HEAD
			if (m_descLayout[i] != rhs.m_descLayout[i]) { return false; }
		}
		return true;
	}

	/*!
	   \brief PipelineLayoutCreateParam
	 */
	PipelineLayoutCreateParam(): size(0) {}
private:
	friend class ::pvr::api::impl::PipelineLayout_;
	friend class ::pvr::api::impl::GraphicsPipeline_;
	std::array<api::DescriptorSetLayout, (uint32)FrameworkCaps::MaxDescriptorSetBindings> m_descLayout;
	uint8 size;
};

namespace impl {
/*!*********************************************************************************************************************
\brief Implementation of a PipelineLayout object. A Pipeline Layout API PipelineLayout wrapper.
***********************************************************************************************************************/
=======
			if (_descLayout[i] != rhs._descLayout[i]) { return false; }
		}
		return true;
	}

	void setPushConstantRange(uint32 index, const types::PushConstantRange& pushConstantRange)
	{
		if (pushConstantRange.size == 0)
		{
			debug_assertion(false, "Push constant range size must not be be 0");
			Log("Push constant range size must not be be 0");
		}

		if (index >= _pushConstantRange.size()) { _pushConstantRange.resize((index + 1) - _pushConstantRange.size()); }
		_pushConstantRange[index] = pushConstantRange;
	}

	const types::PushConstantRange& getPushConstantRange(uint32 index)const
	{
		return _pushConstantRange.at(index);
	}

	uint32 getNumPushConstantRange()const { return static_cast<pvr::uint32>(_pushConstantRange.size()); }

	/// <summary>PipelineLayoutCreateParam</summary>
	PipelineLayoutCreateParam() : size(0) {}
private:
	bool isValidPushConstantRange(uint32 index) { return _pushConstantRange[index].size != 0; }
	friend class ::pvr::api::impl::PipelineLayout_;
	friend class ::pvr::api::impl::GraphicsPipeline_;
	std::array<api::DescriptorSetLayout, (uint32)FrameworkCaps::MaxDescriptorSetBindings> _descLayout;
	uint8 size;
	std::vector<types::PushConstantRange> _pushConstantRange;
};

namespace impl {
/// <summary>Implementation of a PipelineLayout object. A Pipeline Layout API PipelineLayout wrapper.</summary>
>>>>>>> 1776432f... 4.3
class PipelineLayout_
{
	friend ::pvr::api::PipelineLayout IGraphicsContext::createPipelineLayout
	(const api::PipelineLayoutCreateParam& desc);

public:
<<<<<<< HEAD
	/*!*********************************************************************************************************************
	\brief  INTERNAL. Use context->createPipelineLayout to create this object
	\param[in] context
	***********************************************************************************************************************/
	PipelineLayout_(GraphicsContext& context) : m_context(context) {}

	/*!*********************************************************************************************************************
	\brief dtor
	***********************************************************************************************************************/
	virtual ~PipelineLayout_() { }

	/*!*********************************************************************************************************************
	\brief Get a descriptor set layout used by this pipeline layout
	\param index Layout index
	\return std::vector<pvr::api::DescriptorSetLayout>&
	***********************************************************************************************************************/
	const pvr::api::DescriptorSetLayout& getDescriptorSetLayout(pvr::uint32 index)const
	{
		assertion(index < m_desc.size && "Invalid Index");
		return m_desc.m_descLayout[index];
	}

	/*!*********************************************************************************************************************
	\brief Get all the descriptor set layouts used by this object as (raw datastructure)
	\return The underlying container of all descriptor set layouts used.
	***********************************************************************************************************************/
	const std::array<pvr::api::DescriptorSetLayout, (uint32)FrameworkCaps::MaxDescriptorSetBindings>& getDescriptorSetLayouts()const
	{
		return m_desc.m_descLayout;
	}

	/*!
	   \brief Get number of descriptorSet layout
	   \return the number of descriptor set layouts
	 */
	pvr::uint32 getNumDescritporSetLayout()const { return m_desc.size; }

	/*!*********************************************************************************************************************
	  \brief Return create param
	***********************************************************************************************************************/
	const PipelineLayoutCreateParam& getCreateParam()const { return m_desc; }

	/*!*********************************************************************************************************************
	  \brief Return const reference to the underlying api object
	  ***********************************************************************************************************************/
	const native::HPipelineLayout_& getNativeObject() const;

	/*!*********************************************************************************************************************
	  \brief Return reference to the underlying api object
	  ***********************************************************************************************************************/
	native::HPipelineLayout_& getNativeObject();
protected:
	bool init(const PipelineLayoutCreateParam& createParam);
	GraphicsContext m_context;
	PipelineLayoutCreateParam m_desc;
=======
	/// <summary>INTERNAL. Use context->createPipelineLayout to create this object</summary>
	PipelineLayout_(const GraphicsContext& context) : _context(context) {}

	/// <summary>dtor</summary>
	virtual ~PipelineLayout_() { }

	/// <summary>Get a descriptor set layout used by this pipeline layout</summary>
	/// <param name="index">Layout index</param>
	/// <returns>std::vector<pvr::api::DescriptorSetLayout>&</returns>
	const pvr::api::DescriptorSetLayout& getDescriptorSetLayout(pvr::uint32 index)const
	{
		assertion(index < _desc.size && "Invalid Index");
		return _desc._descLayout[index];
	}

	/// <summary>Get all the descriptor set layouts used by this object as (raw datastructure)</summary>
	/// <returns>The underlying container of all descriptor set layouts used.</returns>
	const DescriptorSetLayoutSet& getDescriptorSetLayouts()const
	{
		return _desc._descLayout;
	}

	/// <summary>Get number of descriptorSet layout</summary>
	/// <returns>the number of descriptor set layouts</returns>
	pvr::uint32 getNumDescritporSetLayout()const { return _desc.size; }

	/// <summary>Return create param</summary>
	const PipelineLayoutCreateParam& getCreateParam()const { return _desc; }

protected:
	bool init(const PipelineLayoutCreateParam& createParam);
	GraphicsContext _context;
	PipelineLayoutCreateParam _desc;
>>>>>>> 1776432f... 4.3
};

}
}
}
