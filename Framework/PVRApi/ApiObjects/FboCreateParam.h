/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\FboCreateParam.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains Creation object and supporting classes for FrameBufferObjects
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiObjects/Buffer.h"
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRCore/RefCounted.h"
namespace pvr {

// forward declarations
class ApiExtensions;

namespace api {
/*!*********************************************************************************************************************
\brief Wrap a TextureView with this class in order to use it as a Color Attachment in an FBO.
\description  --- Defaults ---
             Image: NULL, Mip level: 0, Base Array slice:0, Array size:1, MSAA resolve image:NULL, MSAA subresource range:NULL
***********************************************************************************************************************/
struct ColorAttachmentViewCreateParam
{
	TextureView	image;          //!< texture view
	pvr::uint32		mipLevel;       //!< Which mip level to use as attachment
	pvr::uint32		baseArraySlice; //!< Which array slice to use as attachment
	pvr::uint32		arraySize;
	TextureView	msaaResolveImage;//!< Image to use for MSAA resolve
	ImageSubResourceRange msaaResolveSubResRange;//!< SubResource range to use for MSAA resolve

	/*!*********************************************************************************************************************
	\brief Default constructor. Empty textures, miplevel:0, baseArraySlice:0, arraySize(1).
	***********************************************************************************************************************/
	ColorAttachmentViewCreateParam() : mipLevel(0), baseArraySlice(0), arraySize(1)
	{}

	/*!*********************************************************************************************************************
	\brief Construct a Color Attachment View with the specified parameters.
	\param image The image to use as attachment
	\param mipLevel The mipmap level of (image) to use as attachment
	\param baseArraySlice The starting index in (image) - if array - of array slices to use
	\param arraySize The number of array slices of (image) to use
	\param msaaResolveImage The image to use as MSAA Resolve image
	\param msaaResolveSubResRange The Subresource Range of (msaaResolveImage) to use for MSAA resolve
	***********************************************************************************************************************/
	ColorAttachmentViewCreateParam(const TextureView& image, pvr::uint32 mipLevel = 0,
	                               pvr::uint32 baseArraySlice = 0, pvr::uint32	arraySize = 1, const TextureView& msaaResolveImage = TextureView(),
	                               const ImageSubResourceRange& msaaResolveSubResRange = ImageSubResourceRange()) :
		image(image), mipLevel(mipLevel), baseArraySlice(baseArraySlice),
		arraySize(arraySize), msaaResolveSubResRange(msaaResolveSubResRange)
	{
	}
};

/*!*********************************************************************************************************************
\brief Wrap a TextureView with this class in order to use it as a Depth/Stencil Attachment in an FBO.
\description  --- Defaults ---
             Image: NULL, Mip level: 0, Base Array slice:0, Array size:1, MSAA resolve image:NULL, MSAA subresource range:NULL
***********************************************************************************************************************/
struct DepthStencilViewCreateParam
{
	TextureView image;//< texture image
	pvr::uint32		mipLevel;//< miplevel to use
	pvr::uint32		baseArraySlice;
	pvr::uint32		arraySize;
	TextureView msaaResolveImage;
	ImageSubResourceRange msaaResolveSubResRange;
	DepthStencilViewCreateParam(TextureView image, pvr::uint32	mipLevel = 0, pvr::uint32 baseArraySlice = 0,
	                            pvr::uint32	arraySize = 1, const TextureView& 	msaaResolveImage = TextureView(),
	                            const ImageSubResourceRange& msaaResolveSubResRange = ImageSubResourceRange()) :
		image(image), mipLevel(mipLevel), baseArraySlice(baseArraySlice), arraySize(arraySize),
		msaaResolveSubResRange(msaaResolveSubResRange)
	{
	}

	/*!*********************************************************************************************************************
	\brief Default constructor. Empty textures, miplevel:0, baseArraySlice:0, arraySize(1).
	***********************************************************************************************************************/
	DepthStencilViewCreateParam() : mipLevel(0), baseArraySlice(0), arraySize(1) {}
};

/*!****************************************************************************************************************
\brief  Fbo creation descriptor.
*******************************************************************************************************************/
struct FboCreateParam
{
	friend class impl::FboImpl;
private:
	DepthStencilView depthStencilView;
	//< fbo's color attachments the attachments are mapped in the order they are added
	std::vector<ColorAttachmentView> colorViews;
	RenderPass	renderPass;
	pvr::uint32 layers;
	const ApiExtensions* extensions;
	pvr::uint32 width, height;
public:
	FboCreateParam() : width(0), height(0) {}

	FboCreateParam& setDimension(pvr::uint32 width, pvr::uint32 height)
	{
		this->width = width; this->height = height; return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set the depthstencil attachment.
	\param depthStencilView A depthstencil bind info
	\return this (allow chaining)
	***********************************************************************************************************************/
	FboCreateParam& setDepthStencil(const DepthStencilView& depthStencilView) { this->depthStencilView = depthStencilView; return *this; }

	/*!*********************************************************************************************************************
	\brief Add a color attachment to a specified attachment point.
	\param index The attachment point
	\param colorView The color attachment
	\return this (allow chaining)
	***********************************************************************************************************************/
	FboCreateParam& addColor(pvr::uint32 index, const ColorAttachmentView& colorView)
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
}
}
