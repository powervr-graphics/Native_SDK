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
namespace pvr {

class IGraphicsContext;
namespace api {
namespace impl {
class RenderPass_;
class BeginRenderPass;
class EndRenderPass;
}
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
	FboCreateParam() : width(0), height(0), layers(1) {}

	pvr::uint32 getNumColorAttachements()const { return (uint32)colorViews.size(); }

	const TextureView& getColorAttachment(pvr::uint32 index)const
	{
		assertion(index < colorViews.size() ,  " Invalid Color Attachment index");
		return colorViews[index];
	}
	const std::vector<TextureView>& getColorAttachments()const
	{
		return colorViews;
	}

	const RenderPass& getRenderPass()const { return renderPass; }


	RenderPass& getRenderPass() { return renderPass; }

	TextureView& getDepthStencilAttachment() { return depthStencilView; }

	const TextureView& getDepthStencilAttachment() const { return depthStencilView; }

	glm::ivec2 getDimension()const {	return glm::vec2(width, height); }

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
		//const auto& format = depthStencilView->getResource()->getFormat().format.;
		//if (format == PixelFormat::Depth16 || format == PixelFormat::Depth24 &&
		//    format == PixelFormat::Depth32 || format == PixelFormat::Depth24Stencil8 &&
		//    format == PixelFormat::Depth32Stencil8)
		{
			this->depthStencilView = depthStencilView; return *this;
		}
		//	assertion(false, "Invalid Depth stencil attachemnt");
	}

	/*!*********************************************************************************************************************
	\brief Add a color attachment to a specified attachment point.
	\param index The attachment point
	\param colorView The color attachment
	\return this (allow chaining)
	***********************************************************************************************************************/
	FboCreateParam& addColor(pvr::uint32 index, const TextureView colorView)
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
	friend class impl::Fbo_;
	std::map<pvr::uint32, TextureView> colorViews;
public:
	OnScreenFboCreateParam() {}

	pvr::uint32 getNumColorAttachements()const { return (uint32)colorViews.size(); }

	const TextureView& getColorAttachment(pvr::uint32 index)const
	{
		assertion(index < colorViews.size(), " Invalid Color Attachment index");
		assertion(index > 0u, " Invalid Color Attachment - Index 0 corresponds to presentation image");
		auto foundColorView = colorViews.find(index);
		assertion(foundColorView != colorViews.end(), " Invalid Color Attachment index");
		return foundColorView->second;
	}

	const std::map<pvr::uint32, TextureView>& getColorAttachments()const
	{
		return colorViews;
	}

	/*!*********************************************************************************************************************
	\brief Add a color attachment to a specified attachment point.
	\param index The attachment point
	\param colorView The color attachment
	\return this (allow chaining)
	***********************************************************************************************************************/
	OnScreenFboCreateParam& addColor(pvr::uint32 index, const TextureView colorView)
	{
		assertion(index > 0u, " Invalid Color Attachment index - Index 0 corresponds to presentation image");
		this->colorViews[index] = colorView;
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
	\brief Destroy this object and free all resources owned.
	***********************************************************************************************************************/
	void destroy();

	/*!*********************************************************************************************************************
	\brief Destructor. Destroys this object and free all resources owned.
	***********************************************************************************************************************/
	virtual ~Fbo_() {  destroy(); }

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

	native::HFbo_& getNativeObject();
protected:
	FboCreateParam m_desc;
	GraphicsContext m_context;
};
}//	namespace impl
typedef RefCountedResource<impl::Fbo_> Fbo;

}// namespace api
}// namespace pvr
