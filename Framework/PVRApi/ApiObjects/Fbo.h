/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\Fbo.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief    Contains the definition of the FrameBuffer Object class
***********************************************************************************************************************/
#pragma once
#include "PVRCore/IGraphicsContext.h"
#include "PVRCore/RefCounted.h"
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRApi/ApiObjects/FboCreateParam.h"
namespace pvr {
namespace platform {
class ContextGles;
}
class IGraphicsContext;
namespace api {
namespace impl {
class RenderPassImpl;
class BeginRenderPass;
class EndRenderPass;
/*!*********************************************************************************************************************
\brief  PVR Api Color attachment View. Use through the Reference Counted API object pvr::api::ColorAttachmentView.
        Contains a Color Attachment of an image.
***********************************************************************************************************************/
struct ColorAttachmentViewImpl
{
	TextureView texture;	//!< Texture to be rendered to
	uint32 mipLevel;			//!< Mip-map level to use
	pvr::uint32 baseArraySlice;	//!< Array Slice starting index to use
	pvr::uint32 arraySize;		//!< Number of Array Slices to use
	TextureView msaaResolveImage;	//!< Image to use for MSAA resolve
	ImageSubResourceRange msaaResolveSubResRange;//!< SubResourceRange to use for MSAA resolve

	mutable uint32     samples;//!< number of samples, used for MultiSampling
	/*!************************************************************************************
	\brief Which attachment point to attach this object to.
	***************************************************************************************/
	void attachTo(uint32 attachment)const;

	/*!************************************************************************************
	\brief Create an empty ColorAttachmentView object.
	***************************************************************************************/
	ColorAttachmentViewImpl(GraphicsContext& device) :
		mipLevel(0), samples(0), device(device) {}
private:
	friend class ::pvr::IGraphicsContext;
	Result::Enum init(const pvr::api::ColorAttachmentViewCreateParam& createParam);
	native::HColorAttachmentView colorAttachmentView;
	GraphicsContext device;
};

/*!********************************************************************************************
\brief  Api DepthStencilView wrapper. Use through the Reference Counted API object pvr::api::DepthStencilView.
        Contains a Color Attachment of an image.
***********************************************************************************************/
struct DepthStencilViewImpl
{
	enum DepthStencilBit
	{
		Depth,
		Stencil
	};
	TextureView texture;	//!< texture to be attached
	DepthStencilBit bitFlag;	//!< Depth, Stencil, DepthStencil
	uint32 mipLevel;			//!< Mip-map level to use
	pvr::uint32 baseArraySlice;	//!< Array Slice starting index to use
	pvr::uint32 arraySize;		//!< Number of Array Slices to use
	TextureView msaaResolveImage;	//!< Image to use for MSAA resolve
	ImageSubResourceRange msaaResolveSubResRange;//!< SubResourceRange to use for MSAA resolve

	/*!************************************************************************************
	\brief Which attachment point to attach this object to.
	***************************************************************************************/
	void attachTo()const;
	DepthStencilViewImpl(api::FboAttachmentType::Enum attachmentType) : type(attachmentType) {}
private:
	friend class pvr::IGraphicsContext;
	pvr::Result::Enum init(const pvr::api::DepthStencilViewCreateParam& createParam);
	api::FboAttachmentType::Enum	type;
	pvr::native::HDepthStencilView depthStencilView;
};
}// namespace impl
typedef RefCountedResource<impl::ColorAttachmentViewImpl> ColorAttachmentView;
typedef RefCountedResource<impl::DepthStencilViewImpl> DepthStencilView;
namespace impl {

/*!*********************************************************************************************************************
\brief A PVRApi FrameBufferObject implementation. Use through the Reference counted Framework object Fbo. Use a Context to
      create an FBO (IGraphicsContext::createFbo()).
***********************************************************************************************************************/
class FboImpl
{
	friend class ::pvr::api::impl::RenderPassImpl;
	friend class ::pvr::api::impl::BeginRenderPass;
	friend class ::pvr::api::impl::EndRenderPass;
	friend class ::pvr::platform::ContextGles;
	FboImpl& operator=(const FboImpl&);
public:
	typedef RefCountedResource<FboImpl> Fbo_;

	/*!*********************************************************************************************************************
	\brief  Construct default on a device.
	***********************************************************************************************************************/
	FboImpl(GraphicsContext& device);

	/*!*********************************************************************************************************************
	\brief Destroy this object and free all resources owned.
	***********************************************************************************************************************/
	virtual void destroy();

	/*!*********************************************************************************************************************
	\brief Destructor. Destroys this object and free all resources owned.
	***********************************************************************************************************************/
	virtual ~FboImpl() {  destroy(); }

	/*!*********************************************************************************************************************
	\brief Construct an FBO on device with create description.
	\param desc Creation information for an FBO
	\param device A Context to create this FBO on
	***********************************************************************************************************************/
	FboImpl(const FboCreateParam& desc, GraphicsContext& device);

	/*!*********************************************************************************************************************
	\brief Return if this is a Default Fbo (onScreen).
	\return True if this is an Fbo that renders on Screen
	***********************************************************************************************************************/
	virtual bool isDefault()const { return false; }

	/*!*********************************************************************************************************************
	\brief Return the renderpass that this fbo uses.
	\return The renderpass that this FBO uses.
	***********************************************************************************************************************/
	const RenderPass& getRenderPass()const { return m_desc.renderPass; }

	/*!*********************************************************************************************************************
	\brief Return the renderpass that this fbo uses.
	\return The renderpass that this FBO uses.
	***********************************************************************************************************************/
	const native::HFbo& getNativeHandle()const { return m_fbo; }
protected:
	native::HFbo m_fbo;
	mutable FboBindingTarget::Enum m_target;
	virtual void bind(IGraphicsContext& context, api::FboBindingTarget::Enum target)const;
	virtual Result::Enum init(const FboCreateParam& desc);
	virtual bool checkFboStatus();
	std::vector<ColorAttachmentView> m_colorAttachments;
	std::vector<DepthStencilView> m_depthStencilAttachment;
	GraphicsContext m_context;
	FboCreateParam m_desc;
};
}//	namespace impl
typedef impl::FboImpl::Fbo_ Fbo;

}// namespace api
}// namespace pvr
