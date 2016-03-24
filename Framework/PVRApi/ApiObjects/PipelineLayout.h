#pragma once
#include "PVRApi/ApiIncludes.h"
namespace pvr
{
namespace api
{

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
    PipelineLayoutCreateParam& addDescSetLayout(const api::DescriptorSetLayout& descLayout)
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
    PipelineLayoutCreateParam& setDescSetLayout(pvr::uint32 index, const api::DescriptorSetLayout& descLayout)
    {
        if (index >= this->m_descLayout.size()) { this->m_descLayout.resize(index + 1); }
        this->m_descLayout[index] = descLayout;
        return *this;
    }

	/*!*********************************************************************************************************************
    \brief Return number of descriptor set layouts
    ***********************************************************************************************************************/
    pvr::uint32 getNumDescSetLayouts()const { return (pvr::uint32)m_descLayout.size(); }

	/*!*********************************************************************************************************************
    \brief Get descriptor set layout
	\param index Descriptor set layout index
	\return Descritpor set layout
    ***********************************************************************************************************************/
    const api::DescriptorSetLayout& getDescriptorSetLayout(pvr::uint32 index)const
    {
#ifdef DEBUG
        if (index >= m_descLayout.size())
        {
            assertion(false ,  "Invalid DescriptorSetLayout Index");
            pvr::Log("DescriptorSetLayout::getDescriptorSetLayout - Invalid DescriptorSetLayout Index");
        }
#endif
        return m_descLayout[index];
    }

private:
    friend class ::pvr::api::impl::PipelineLayout_;
    friend class ::pvr::api::impl::GraphicsPipeline_;
    std::vector<api::DescriptorSetLayout> m_descLayout;
};

namespace impl
{
/*!*********************************************************************************************************************
\brief Implementation of a PipelineLayout object. A Pipeline Layout API PipelineLayout wrapper.
***********************************************************************************************************************/
class PipelineLayout_
{
    friend ::pvr::api::PipelineLayout IGraphicsContext::createPipelineLayout
    (const api::PipelineLayoutCreateParam& desc);

public:
    /*!*********************************************************************************************************************
    \brief  Create this on device.
    \param[in] device
    ***********************************************************************************************************************/
    PipelineLayout_(GraphicsContext& context) : m_context(context) {}

    /*!*********************************************************************************************************************
    \brief dtor
    ***********************************************************************************************************************/
    virtual ~PipelineLayout_(){ }

    /*!*********************************************************************************************************************
    \brief Get list of descriptor set layout used by this.
    \return std::vector<pvr::api::DescriptorSetLayout>&
    ***********************************************************************************************************************/
    const std::vector<pvr::api::DescriptorSetLayout>& getDescriptorSetLayout()const { return m_desc.m_descLayout; }

	/*!*********************************************************************************************************************
	\brief Get a descriptor set layout used by this.
	\param index Layout index
	\return std::vector<pvr::api::DescriptorSetLayout>&
	***********************************************************************************************************************/
	const pvr::api::DescriptorSetLayout& getDescriptorSetLayout(pvr::uint32 index)const 
	{ 
		assertion(index < m_desc.m_descLayout.size() && "Invalid Index");
		return m_desc.m_descLayout[index]; 
	}

	pvr::uint32 getNumDescritporSetLayout()const{ return m_desc.m_descLayout.size(); }

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
};

}
}
}

