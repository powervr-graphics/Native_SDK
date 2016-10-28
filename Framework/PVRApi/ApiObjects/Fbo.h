/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\Fbo.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief    Contains the definition of the FrameBuffer Object class
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/ApiObjects/Buffer.h"
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRApi/ApiObjects/RenderPass.h"

namespace pvr {

class IGraphicsContext;
namespace api {

/*!****************************************************************************************************************
\brief  Fbo creation descriptor.
*******************************************************************************************************************/
struct FboCreateParam
{
	friend class impl::Fbo_;
	TextureView depthStencilView;
	//< fbo's color attachments the attachments are mapped in the order they are added
	pvr::uint32 layers;
	pvr::uint32 width, height;
	RenderPass	renderPass;
	std::vector<TextureView> colorViews;
public:

	/*!
	   \brief Reset this object
	 */
	void clear()
	{
		width = 0; height = 0; layers = 1; renderPass.reset(); colorViews.clear(); depthStencilView.reset();
	}

	/*!*
	\brief ctor
	 */
    FboCreateParam() : layers(1), width(0), height(0) {}

	/*!*
	\brief Return number of color attachment
	 */
	pvr::uint32 getNumColorAttachements()const { return (uint32)colorViews.size(); }

	/*!*
	\brief Get the color attachment
	\param index Attachment index. Index must be a valid index
	\return
	 */
	const TextureView& getColorAttachment(pvr::uint32 index)const
	{
		debug_assertion(index < colorViews.size() ,  " Invalid Color Attachment index");
		return colorViews[index];
	}

	/*!
	   \brief Get a color attachment TextureView
	   \param index The index of the Colorattachment to retrieve
	   \return This object
	 */
	TextureView& getColorAttachment(pvr::uint32 index)
	{
		return const_cast<TextureView&>(static_cast<const FboCreateParam&>(*this).getColorAttachment(index));
	}

	/*!
	   \brief Get all color attachments (raw datastructure)
	   \return TextureView
	 */
	const std::vector<TextureView>& getColorAttachments()const { return colorViews; }

	/*!*
	\brief Get the Renderpass (const)
	 */
	const RenderPass& getRenderPass()const { return renderPass; }

	/*!*
	\brief Get the Renderpass
	\return
	 */
	RenderPass& getRenderPass() { return renderPass; }

	/*!*
	\brief Get the Depth Stencil attachment
	\return
	 */
	TextureView& getDepthStencilAttachment() { return depthStencilView; }

	/*!*
	\brief Get the Depth Stencil attachment (const)
	 */
	const TextureView& getDepthStencilAttachment() const { return depthStencilView; }

	/*!*
	\brief Get the fbo dimension
	\return
	 */
	glm::ivec2 getDimensions()const { return glm::ivec2(width, height); }

	/*!*
	\brief Set the fbo dimension
	\param width Width
	\param height Height
	\return Return this for chaining
	 */
	FboCreateParam& setDimension(pvr::uint32 width, pvr::uint32 height)
	{
		this->width = width; this->height = height; return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set the depthstencil attachment.
	\param depthStencilView A depthstencil bind info
	\return this (allow chaining)
	***********************************************************************************************************************/
	FboCreateParam& setDepthStencil(const TextureView depthStencilView)
	{
		// validate the attachment format
		const auto& format = depthStencilView->getResource()->getFormat().format;
		if (format == PixelFormat::Depth16 || 
		    format == PixelFormat::Depth24 ||
		    format == PixelFormat::Depth32 || 
		    format == PixelFormat::Depth16Stencil8 ||
		    format == PixelFormat::Depth24Stencil8 ||
		    format == PixelFormat::Depth32Stencil8)
		{
			this->depthStencilView = depthStencilView;
		}
		else
		{
			assertion(false, "Invalid Depth stencil attachment Format");
		}
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Add a color attachment to a specified attachment point.
	\param index The attachment point, the index must be consecutive
	\param colorView The color attachment
	\return this (allow chaining)
	***********************************************************************************************************************/
	FboCreateParam& setColor(pvr::uint32 index, const TextureView colorView)
	{
		if (index >= this->colorViews.size()) { this->colorViews.resize(index + 1); }
		this->colorViews[index] = colorView;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set the number of layers.
	\return this (allow chaining)
	***********************************************************************************************************************/
	FboCreateParam& setNumLayers(pvr::uint32 count) { layers = count; return *this; }

	/*!*********************************************************************************************************************
	\brief Set the Renderpass which this FBO will be invoking when bound.
	\param renderPass A renderpass. When binding this FBO, this renderpass will be the one to be bound.
	\return this (allow chaining)
	***********************************************************************************************************************/
	FboCreateParam& setRenderPass(const RenderPass& renderPass) { this->renderPass = renderPass; return *this; }
};

/*!****************************************************************************************************************
\brief  on screen Fbo creation descriptor - provides limited additional functionality when creating an on screen FBO
primarily the ability to add additional color attachments.
*******************************************************************************************************************/
struct OnScreenFboCreateParam
{

private:
	friend class impl::Fbo_;
	// The user must operate with this structure with non 0 based index but the attachments are stored in 0 based index.
	TextureView colorViews[(uint32)FrameworkCaps::MaxColorAttachments];
	TextureView depthStencilViews[uint32(FrameworkCaps::MaxDepthStencilAttachments)];
	pvr::uint32 colorAttachmentCount;
	uint32 dsAttachmentCount;
public:
	OnScreenFboCreateParam() : colorAttachmentCount(0) {}

	/*!*********************************************************************************************************************
	\return Number of color attachments excluding the presentation image
	***********************************************************************************************************************/
	pvr::uint32 getNumOffScreenColor()const { return colorAttachmentCount; }

	/*!*********************************************************************************************************************
	\param index Index of the attachment. The index must not be 0
	\return The Color attachment of given index
	***********************************************************************************************************************/
	const TextureView& getOffScreenColor(pvr::uint32 index)const
	{
		assertion((index - 1) < colorAttachmentCount, " Invalid Color Attachment index");
		assertion(index > 0u, "Cannot return presentation color attachment");

		return colorViews[index - 1];
	}

	/*!*********************************************************************************************************************
	\brief Add a color attachment to a specified attachment point.
	\param colorView The color attachment
	\return this (allow chaining)
	***********************************************************************************************************************/
	OnScreenFboCreateParam& addOffScreenColor(const TextureView colorView)
	{
		return setOffScreenColor(colorAttachmentCount + 1, colorView);
	}

	/*!*********************************************************************************************************************
	\brief Add a offscreen color attachment to a specified attachment point.
	\param colorView The color attachment
	\param index Attachment index. The index must be consecutive and cannot be 0
	\return this (allow chaining)
	***********************************************************************************************************************/
	OnScreenFboCreateParam& setOffScreenColor(pvr::uint32 index, const TextureView colorView)
	{
		assertion(index > 0u, " Invalid Color Attachment index - Index 0 corresponds to presentation image");
		assertion(index < (uint32)FrameworkCaps::MaxColorAttachments,
                  " Invalid Color Attachment index. Maximum number of color attachments exceeded");

		TextureView oldEntry = this->colorViews[index - 1];
		this->colorViews[index - 1] = colorView;
		// increment the number of attachments for only new entries and not for modifying the existing.
		if (!oldEntry.isValid()) { ++colorAttachmentCount; }
		return *this;
	}

    /*!*********************************************************************************************************************
	\brief Add a offscreen Depth-Stencil attachment to a specified attachment point.
	\param dsView The Depth-Stencil attachment
	\param index Attachment index. The index must be consecutive and cannot be 0
	\return this (allow chaining)
	***********************************************************************************************************************/
    OnScreenFboCreateParam& setOffScreenDepthStencil(pvr::uint32 index, const TextureView dsView)
	{
		assertion(index > 0u, " Invalid Depth-Stencil Attachment index - Index 0 corresponds to presentation Depth-Stencil image");
		assertion(index < (uint32)FrameworkCaps::MaxDepthStencilAttachments,
                  " Invalid Depth-Stencil Attachment index. Maximum number of Depth-Stencil attachments exceeded");

		TextureView oldEntry = this->depthStencilViews[index - 1];
		this->depthStencilViews[index - 1] = dsView;
		// increment the number of attachments for only new entries and not for modifying the existing.
        if (!oldEntry.isValid()) { ++dsAttachmentCount; }
		return *this;
	}
};

namespace impl {
/*!*********************************************************************************************************************
\brief A PVRApi FrameBufferObject implementation. Use through the Reference counted Framework object Fbo. Use a Context to
      create an FBO (IGraphicsContext::createFbo()).
***********************************************************************************************************************/
class Fbo_
{
	friend class ::pvr::api::impl::RenderPass_;
	friend class ::pvr::api::impl::BeginRenderPass;
	friend class ::pvr::api::impl::EndRenderPass;
	Fbo_& operator=(const Fbo_&);
public:

	/*!*********************************************************************************************************************
	\brief  Construct Fbo on a Context.
	***********************************************************************************************************************/
	Fbo_(GraphicsContext& m_context);

	/*!*********************************************************************************************************************
	\brief Destructor. Destroys this object and free all resources owned.
	***********************************************************************************************************************/
	virtual ~Fbo_() { }

	/*!*********************************************************************************************************************
	\brief Construct an FBO on device with create description.
	\param desc Creation information for an FBO
	\param device A Context to create this FBO on
	***********************************************************************************************************************/
	Fbo_(const FboCreateParam& desc, GraphicsContext& device);

	/*!*********************************************************************************************************************
	\brief Return if this is a Default Fbo (onScreen).
	\return True if this is an Fbo that renders on Screen
	***********************************************************************************************************************/
	virtual bool isDefault() const { return false; }

	/*!*********************************************************************************************************************
	\brief Return the renderpass that this fbo uses.
	\return The renderpass that this FBO uses.
	***********************************************************************************************************************/
	const native::HFbo_& getNativeObject() const;

	/*!*********************************************************************************************************************
	\brief Return the renderpass that this fbo uses.
	\return The renderpass that this FBO uses.
	***********************************************************************************************************************/
	const RenderPass& getRenderPass() const { return m_desc.renderPass; }

	/*!*
	\brief Return this object create param
	 */
	const FboCreateParam& getFboCreateParam() const { return m_desc; }

	/*!*
	\brief Get the native handle object
	\return
	 */
	native::HFbo_& getNativeObject();

	/*!*
	\brief Get the Dimension of this fbo
	\return Fbo Dimension
	 */
	glm::ivec2 getDimensions()const { return m_desc.getDimensions(); }

	/*!*
	\brief Get Number of color attachments this fbo has
	 */
	uint32 getNumColorAttachments()const { return m_desc.getNumColorAttachements(); }
	const TextureView& getColorAttachment(pvr::uint32 index)const { return m_desc.getColorAttachment(index); }
	TextureView& getColorAttachment(pvr::uint32 index)
	{
		return const_cast<TextureView&>(static_cast<const Fbo_&>(*this).getColorAttachment(index));
	}
	const TextureView getDepthStencilAttachment()const { return m_desc.getDepthStencilAttachment(); }

	/*!*
	\brief Return true if the fbo has depth stencil attachment
	 */
	bool hasDepthStencilAttachment()const { return m_desc.getDepthStencilAttachment().isValid(); }
protected:
	FboCreateParam m_desc;
	GraphicsContext m_context;
};
}//	namespace impl
typedef RefCountedResource<impl::Fbo_> Fbo;
typedef Multi<Fbo, 4> FboSet;

}// namespace api
}// namespace pvr
