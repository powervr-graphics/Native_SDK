/*!
\brief Contains the definition of the FrameBuffer Object class
\file PVRApi/ApiObjects/Fbo.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/ApiObjects/Buffer.h"
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRApi/ApiObjects/RenderPass.h"

namespace pvr {

class IGraphicsContext;
namespace api {

/// <summary>Fbo creation descriptor.</summary>
struct FboCreateParam
{
public:
	friend class impl::Fbo_;
	/// <summary>The number of array layers of the FBO</summary>
	uint32 layers;
	/// <summary>The width (in pixels) of the FBO</summary>
	uint32 width;
	/// <summary>The hight (in pixels) of the FBO</summary>
	uint32 height;
	/// <summary>The render pass that this FBO will render in</summary>
	RenderPass  renderPass;


	/// <summary>Reset this object</summary>
	void clear()
	{
		width = 0; height = 0; layers = 1; renderPass.reset();
		for (uint32 i = 0; i < 4; ++i)
		{
			colorViews[i].reset();
			dsViews[i].reset();
		}
	}

	/// <summary>ctor</summary>
	FboCreateParam() : layers(1), width(0), height(0), colorViewsSize(0), dsViewsSize(0) {}

	/// <summary>Return number of color attachment</summary>
	pvr::uint32 getNumColorAttachements()const { return colorViewsSize; }

	/// <summary>Get the color attachment</summary>
	/// <param name="index">Attachment index. Index must be a valid index</param>
	const TextureView& getColorAttachment(pvr::uint32 index)const
	{
		debug_assertion(index < colorViewsSize,  " Invalid Color Attachment index");
		return colorViews[index];
	}

	/// <summary>Get a color attachment TextureView</summary>
	/// <param name="index">The index of the Colorattachment to retrieve</param>
	/// <returns>This object</returns>
	TextureView& getColorAttachment(pvr::uint32 index)
	{
		return const_cast<TextureView&>(static_cast<const FboCreateParam&>(*this).getColorAttachment(index));
	}

	/// <summary>Get the Renderpass (const)</summary>
	/// <returns>The Renderpass (const)</summary>
	const RenderPass& getRenderPass()const { return renderPass; }

	/// <summary>Get the Renderpass</summary>
	/// <returns>The Renderpass</summary>
	RenderPass& getRenderPass() { return renderPass; }

	/// <summary>Get the Depth Stencil attachment</summary>
	/// <param name="index">The index of the Depth Stencil attachment to retrieve</param>
	/// <returns>The depth-stencil attachment at index <paramref name="index">index</paramref></param>
	TextureView& getDepthStencilAttachment(uint32 index) { return dsViews[index]; }

	/// <summary>Get the Depth Stencil attachment (const)</summary>
	/// <param name="index">The index of the Depth Stencil attachment to retrieve</param>
	/// <returns>The depth-stencil attachment at index <paramref name="index">index</paramref></param>
	const TextureView& getDepthStencilAttachment(uint32 index) const { return dsViews[index]; }

	/// <summary>Get the number of Depth Stencil attachments</summary>
	/// <returns>The number of depth-stencil attachments</param>
	uint32 getNumDepthStencilAttachments() const { return dsViewsSize; }

	/// <summary>Get the dimensions of the fbo</summary>
	/// <summary>The fbo dimensions</summary>
	glm::ivec2 getDimensions()const { return glm::ivec2(width, height); }

	/// <summary>Set the fbo dimension</summary>
	/// <param name="width">Width</param>
	/// <param name="height">Height</param>
	/// <returns>This object (allow chaining)</returns>
	FboCreateParam& setDimensions(pvr::uint32 width, pvr::uint32 height)
	{
		this->width = width; this->height = height; return *this;
	}

	/// <summary>Set the depthstencil attachment.</summary>
	/// <param name="depthStencilView">A depthstencil bind info</param>
	/// <param name="index">The index of the attachment to which to set the depth stencil</param></param>
	/// <returns>This object (allow chaining)</returns>
	FboCreateParam& setDepthStencil(uint32 index, const TextureView depthStencilView)
	{
		debug_assertion(index < 4, "FboCreateParam: Valid attachment indices are 0 to 3.");
		// validate the attachment format
		const auto& format = depthStencilView->getResource()->getFormat().format;
		if (format == PixelFormat::Depth16 ||
		    format == PixelFormat::Depth24 ||
		    format == PixelFormat::Depth32 ||
		    format == PixelFormat::Depth16Stencil8 ||
		    format == PixelFormat::Depth24Stencil8 ||
		    format == PixelFormat::Depth32Stencil8)
		{
			if (index >= dsViewsSize) { dsViewsSize = index + 1; }
			dsViews[index] = depthStencilView;
		}
		else
		{
			assertion(false, "Invalid Depth stencil attachment Format");
		}
		return *this;
	}

	/// <summary>Set the depthstencil attachment at index 0.</summary>
	/// <param name="depthStencilView">The texture to bind as a depth buffer at index 0</param>
	/// <returns>This object (allow chaining)</returns>
	FboCreateParam& setDepthStencil(const TextureView depthStencilView)
	{
		return setDepthStencil(0, depthStencilView);
	}

	/// <summary>Add a color attachment to a specified attachment point.</summary>
	/// <param name="index">The attachment point, the index must be consecutive</param>
	/// <param name="colorView">The color attachment</param>
	/// <returns>this (allow chaining)</returns>
	FboCreateParam& setColor(pvr::uint32 index, const TextureView colorView)
	{
		debug_assertion(index < 4, "FboCreateParam: Valid attachment indices are 0 to 3.");
		if (index >= colorViewsSize) { colorViewsSize = index + 1; }
		this->colorViews[index] = colorView;
		return *this;
	}

	/// <summary>Set the number of layers.</summary>
	/// <param name="count">The number of array layers.</summary>
	/// <returns>this (allow chaining)</returns>
	FboCreateParam& setNumLayers(pvr::uint32 count) { layers = count; return *this; }

	/// <summary>Set the Renderpass which this FBO will be invoking when bound.</summary>
	/// <param name="renderPass">A renderpass. When binding this FBO, this renderpass will be the one to be bound.
	/// </param>
	/// <returns>this (allow chaining)</returns>
	FboCreateParam& setRenderPass(const RenderPass& renderPass) { this->renderPass = renderPass; return *this; }

private:
	TextureView colorViews[4];
	TextureView dsViews[4];
	uint32 colorViewsSize;
	uint32 dsViewsSize;

};

/// <summary>on screen Fbo creation descriptor - provides limited additional functionality when creating an on
/// screen FBO primarily the ability to add additional color attachments.</summary>
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
	/// <summary>Constructor. initializes to zero attachments.</summary>
	OnScreenFboCreateParam() : colorAttachmentCount(0), dsAttachmentCount(0) {}

	/// <summary>Get the number of color attachments excluding the presentation image</summary>
	/// <returns>The number of color attachments excluding the presentation image</returns>
	pvr::uint32 getNumOffScreenColor()const { return colorAttachmentCount; }

	/// <summary>Get the number of depth stencil attachments excluding the presentation image</summary>
	/// <returns>The Number of depth stencil attachments excluding the presentation image</returns>
	uint32 getNumOffScreenDepthStencil()const { return dsAttachmentCount; }

	/// <summary>Get the color attachment for a specified index. Cannot return the presentation image</summary>
	/// <param name="index">Index of the attachment. The index must not be 0</param>
	/// <returns>The Color attachment of given index</returns>
	const TextureView& getOffScreenColor(pvr::uint32 index)const
	{
		assertion((index - 1) < colorAttachmentCount, " Invalid Color Attachment index");
		assertion(index > 0u, "Cannot return presentation color attachment");
		return colorViews[index - 1];
	}

	/// <summary>Get the depth-stencil attachment for a specified index. Cannot return the swapchain's image
	/// </summary>
	/// <param name="index">Index of the attachment. The index must not be 0</param>
	/// <returns>The depth stencil attachment of given index</returns>
	const TextureView& getOffScreenDepthStencil(pvr::uint32 index)const
	{
		assertion((index - 1) < dsAttachmentCount, " Invalid Depth-Stencil Attachment index");
		assertion(index > 0u, "Cannot return Swapchain's Depth-Stencil attachment");
		return depthStencilViews[index - 1];
	}

	/// <summary>Add a color attachment to a specified attachment point.</summary>
	/// <param name="colorView">The color attachment</param>
	/// <returns>this (allow chaining)</returns>
	OnScreenFboCreateParam& addOffScreenColor(const TextureView colorView)
	{
		return setOffScreenColor(colorAttachmentCount + 1, colorView);
	}

	/// <summary>Add a offscreen color attachment to a specified attachment point.</summary>
	/// <param name="colorView">The color attachment</param>
	/// <param name="index">Attachment index. The index must be consecutive and cannot be 0</param>
	/// <returns>this (allow chaining)</returns>
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

	/// <summary>Add a offscreen Depth-Stencil attachment to a specified attachment point.</summary>
	/// <param name="dsView">The Depth-Stencil attachment</param>
	/// <param name="index">Attachment index. The index must be consecutive and cannot be 0</param>
	/// <returns>this (allow chaining)</returns>
	OnScreenFboCreateParam& setOffScreenDepthStencil(pvr::uint32 index, const TextureView dsView)
	{
		assertion(index > 0u, " Invalid Depth-Stencil Attachment index - Index 0 corresponds to presentation Depth-Stencil image");
		assertion(index < (uint32)FrameworkCaps::MaxDepthStencilAttachments,
		          " Invalid Depth-Stencil Attachment index. Maximum number of Depth-Stencil attachments exceeded");
		// increment the number of attachments for only new entries and not for modifying the existing.
		dsAttachmentCount += (uint32)!this->depthStencilViews[index - 1].isValid();
		this->depthStencilViews[index - 1] = dsView;
		return *this;
	}
};

namespace impl {
/// <summary>A PVRApi FrameBufferObject implementation. Use through the Reference counted Framework object Fbo.
/// Use a Context to create an FBO (IGraphicsContext::createFbo()).</summary>
class Fbo_
{
	friend class ::pvr::api::impl::RenderPass_;
	friend class ::pvr::api::impl::BeginRenderPass;
	friend class ::pvr::api::impl::EndRenderPass;
	Fbo_& operator=(const Fbo_&);
public:

	/// <summary>Internal. DO NOT USE. Use context->createFbo</summary>
	Fbo_(const GraphicsContext& _context);

	/// <summary>Destructor. Destroys this object and free all resources owned.</summary>
	virtual ~Fbo_() { }

	/// <summary>Construct an FBO on device with create description.</summary>
	/// <param name="desc">Creation information for an FBO</param>
	/// <param name="device">A Context to create this FBO on</param>
	Fbo_(const FboCreateParam& desc, GraphicsContext& device);

	/// <summary>Return if this is a Default Fbo (onScreen).</summary>
	/// <returns>True if this is an Fbo that renders on Screen</returns>
	virtual bool isDefault() const { return false; }

	/// <summary>Return the renderpass that this fbo uses.</summary>
	/// <returns>The renderpass that this FBO uses.</returns>
	const RenderPass& getRenderPass() const { return _desc.renderPass; }

	/// <summary>Return this object create param</summary>
	const FboCreateParam& getFboCreateParam() const { return _desc; }

	/// <summary>Get the Dimension of this fbo</summary>
	/// <returns>Fbo Dimension</returns>
	glm::ivec2 getDimensions()const { return _desc.getDimensions(); }

	/// <summary>Get the Number of color attachments this fbo has</summary>
	/// <returns>The Number of color attachments this fbo has</returns>
	uint32 getNumColorAttachments()const { return _desc.getNumColorAttachements(); }

	/// <summary>Get the Number of depth-stencil attachments this fbo has</summary>
	/// <returns>The Number of depth-stencil attachments this fbo has</returns>
	uint32 getNumDepthStencilAttachments()const { return _desc.getNumDepthStencilAttachments(); }

	/// <summary>Get the color attachment at a specific index</summary>
	/// <param name="index">A color attachment index.</param>
	/// <returns>The Texture that is bound as a color attachment at index.</returns>
	const TextureView& getColorAttachment(pvr::uint32 index)const { return _desc.getColorAttachment(index); }
	/// <summary>Get the color attachment at a specific index</summary>
	/// <param name="index">A color attachment index.</param>
	/// <returns>The Texture that is bound as a color attachment at index.</returns>
	TextureView& getColorAttachment(pvr::uint32 index)
	{
		return const_cast<TextureView&>(static_cast<const Fbo_&>(*this).getColorAttachment(index));
	}
	/// <summary>Get the depth-stencil attachment at a specific index</summary>
	/// <param name="index">A depth-stencil attachment index.</param>
	/// <returns>The Texture that is bound as a depth-stencil attachment at index.</returns>
	const TextureView getDepthStencilAttachment(uint32 index)const { return _desc.getDepthStencilAttachment(index); }

	/// <summary>Return true if the fbo has depth stencil attachment</summary>
	/// <returns>True if the FBO has at least one depth-stencil attachment, otherwise false.
	bool hasDepthStencilAttachment()const { return _desc.getNumDepthStencilAttachments() > 0; }
protected:
	FboCreateParam _desc;
	GraphicsContext _context;
};
}// namespace impl
/// <summary>A reference counted framework object FBO</summary>
typedef RefCountedResource<impl::Fbo_> Fbo;
/// <summary>A wrapper for a multi-buffered FBO object</summary>
typedef Multi<Fbo, 4> FboSet;

}// namespace api
}// namespace pvr
