/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\RenderPass.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the RenderPass API Object class.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/RefCounted.h"
#include "PVRApi/ApiObjects/Fbo.h"
#include "PVRCore/Rectangle.h"
namespace pvr {
namespace api {
namespace impl { class RenderPassImpl; }

/*!*********************************************************************************************************************
\brief    Contains information on the Color configuration of a renderpass (format, loadop, storeop, samples).
***********************************************************************************************************************/
struct RenderPassColorInfo
{
	friend class ::pvr::api::impl::RenderPassImpl;

	api::ImageDataFormat format;//!< Color buffer attachment format
	LoadOp::Enum         loadOpColor;//!< Color attachment load operation
	StoreOp::Enum        storeOpColor;//!< Color attachment store operation
	pvr::uint32          numSamples;//!< Number of samples
	RenderPassColorInfo() : loadOpColor(LoadOp::Load), storeOpColor(StoreOp::Store), numSamples(1)
	{
		format.format = PixelFormat::Unknown;
	}
	RenderPassColorInfo(const api::ImageDataFormat& format, LoadOp::Enum  loadOpColor = LoadOp::Load,
	                     StoreOp::Enum  storeOpColor = StoreOp::Store, pvr::uint32  numSamples = 0) : format(format),
		loadOpColor(loadOpColor), storeOpColor(storeOpColor), numSamples(numSamples)
	{
	}
};
/*!*********************************************************************************************************************
\brief    Contains information on the Depth/Stencil configuration of a renderpass (format, loadops, storeops, samples).
***********************************************************************************************************************/
struct RenderPassDepthStencilInfo
{
public:
	api::ImageDataFormat format;//!< Depth stencil buffer format
	LoadOp::Enum         loadOpDepth;//!< Depth attachment load operations
	StoreOp::Enum        storeOpDepth;//!< Depth attachment store operation
	LoadOp::Enum         loadOpStencil;//!< Stencil attachment load operation
	StoreOp::Enum        storeOpStencil;//!< Stencil attachment store operation
	pvr::uint32          numSamples;//!< number of samples
	RenderPassDepthStencilInfo() : loadOpDepth(LoadOp::Load), storeOpDepth(StoreOp::Store),
		loadOpStencil(LoadOp::Load), storeOpStencil(StoreOp::Store), numSamples(1) {}

	RenderPassDepthStencilInfo(const api::ImageDataFormat& format, LoadOp::Enum  loadOpDepth = LoadOp::Load,
	                           StoreOp::Enum  storeOpDepth = StoreOp::Store, LoadOp::Enum  loadOpStencil = LoadOp::Load,
	                           StoreOp::Enum storeOpStencil = StoreOp::Store, pvr::uint32 numSamples = 1) :
		format(format), loadOpDepth(loadOpDepth), storeOpDepth(storeOpDepth), loadOpStencil(loadOpStencil),
		storeOpStencil(storeOpStencil), numSamples(numSamples) {}
};

/*!*********************************************************************************************************************
\brief Render pass subpass. Subpasses allow intermediate draws to be chained and communicating with techniques like
       Pixel Local Storage without outputting to the FrameBuffer until the end of the RenderPass.
***********************************************************************************************************************/
struct SubPass
{
public:
	SubPass(PipelineBindingPoint::Enum pipeBindingPoint = PipelineBindingPoint::Graphics)
		: pipelineBindingPoint(pipeBindingPoint) {}

	/*!****************************************************************************************************************
	\brief	Activate the specified color output attachment of the fbo.
	\return	Reference to this(allows chaining)
	\param	attachmentIndex Attachment to activate as output
	*******************************************************************************************************************/
	SubPass& setColorAttachment(pvr::uint32 attachmentIndex) { colorAttachment.push_back(attachmentIndex); return *this; }

	/*!****************************************************************************************************************
	\brief	Set the specified color attachment as input.
	\return	Reference to this(allows chaining)
	\param	attachmentIndex Attachment to set as input
	*******************************************************************************************************************/
	SubPass& setInputAttachment(pvr::uint32 attachmentIndex) { inputAttachment.push_back(attachmentIndex); return *this; }

	/*!****************************************************************************************************************
	\brief	Activate the specified Resolve attachment of the fbo.
	\return	Reference to this(allows chaining)
	\param	attachmentIndex Attachment to set as resolve
	*******************************************************************************************************************/
	SubPass& setResolveAttachment(pvr::uint32 attachmentIndex) { resolveAttachment.push_back(attachmentIndex); return *this; }

	/*!****************************************************************************************************************
	\brief	Set preserve attachment from the fbo.
	\return	Reference to this(allows chaining)
	\param	attachmentIndex  Attachment to set as preserve
	*******************************************************************************************************************/
	SubPass& setPreserveAttachment(pvr::uint32 attachmentIndex) { preserveAttachment.push_back(attachmentIndex); return *this; }

	/*!*********************************************************************************************************************
	\brief Set the pipeline binding point.
	\return	Reference to this(allows chaining)
	***********************************************************************************************************************/
	SubPass& setPipelineBindingPoint(PipelineBindingPoint::Enum bindingPoint) { pipelineBindingPoint = bindingPoint; return *this;}
private:
	PipelineBindingPoint::Enum  pipelineBindingPoint;
	std::vector<pvr::uint32> inputAttachment;
	std::vector<pvr::uint32> colorAttachment;
	std::vector<pvr::uint32> resolveAttachment;
	std::vector<pvr::uint32> preserveAttachment;
};

/*!*********************************************************************************************************************
\brief  RenderPass creation parameters. Fill this object and then use it to create a RenderPass through you IGraphicsContext.
***********************************************************************************************************************/
struct RenderPassCreateParam
{
private:
	friend class impl::RenderPassImpl;
	std::vector<RenderPassColorInfo> color;
	RenderPassDepthStencilInfo        depthStencil;
	std::vector<SubPass>			  subPass;
public:
	/*!*********************************************************************************************************************
	\brief Add color info to the specified color attachment point.
	\param index The color attachment point to add the color info
	\param color The color info to add to the attachment point
	\return Reference to this object. (allow chaining)
	***********************************************************************************************************************/
	RenderPassCreateParam& addColorInfo(pvr::uint32 index, const RenderPassColorInfo& color)
	{
		if (index >= this->color.size()) { this->color.resize(index + 1); }
		this->color[index] = color;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Add depth and stencil attachment info to this object.
	\param dsInfo The depth/stencil info to add
	\return Reference to this object. (allow chaining)
	***********************************************************************************************************************/
	RenderPassCreateParam& setDepthStencilInfo(const RenderPassDepthStencilInfo& dsInfo) { depthStencil = dsInfo; return *this; }

	/*!*********************************************************************************************************************
	\brief Add a subpass to this renderpass
	\param index Index where to add the subpass
	\param subPass The SubPass to add
	\return Reference to this object. (allow chaining)
	***********************************************************************************************************************/
	RenderPassCreateParam& addSubPass(pvr::uint32 index, const SubPass& subPass)
	{
		if (index >= this->subPass.size()) { this->subPass.resize(index + 1); }
		this->subPass[index] = subPass;
		return *this;
	}
};


namespace impl {
class BeginRenderPass;
class EndRenderPass;
/*!********************************************************************************************************************
\brief The implementation of the RenderPass. Use through the Reference counted framework object pvr::api::RenderPass.
**********************************************************************************************************************/
class RenderPassImpl
{
	friend class ::pvr::api::impl::BeginRenderPass;
	friend class ::pvr::api::impl::EndRenderPass;
	friend class ::pvr::IGraphicsContext;
public:
	enum BindScope {BindBegin, BindEnd};
	typedef void* isBindable;

	/*!******************************************************************************************************************
	\brief Creates a new RenderPass object. Use through the IGraphicsContext::createRenderPass.
	*********************************************************************************************************************/
	RenderPassImpl(GraphicsContext& device) : m_context(device) {}

	~RenderPassImpl();

	/*!******************************************************************************************************************
	\brief Get the list of color info of this RenderPass.
	\return The list of color info of this RenderPass
	********************************************************************************************************************/
	const std::vector<RenderPassColorInfo>& getColorInfo()const {	return m_desc.color;}

	/*!******************************************************************************************************************
	\brief Get the depth stencil info of this RenderPass.
	\return The Depth Stencil info of this RenderPass
	*********************************************************************************************************************/
	const RenderPassDepthStencilInfo& getDepthStencilInfo()const {return m_desc.depthStencil;}
private:

	pvr::Result::Enum init(const RenderPassCreateParam& descriptor);

	void destroy();

	void begin(IGraphicsContext& context, const api::Fbo& fbo, const pvr::Rectanglei& drawRegion, glm::vec4* clearColor,
	           pvr::uint32 numClearColor, pvr::float32 clearDepth, pvr::int32 clearStencil)const;
	void end(IGraphicsContext& context)const;
	RenderPassCreateParam m_desc;
	GraphicsContext m_context;
	native::HRenderPass m_renderPass;
};
}
typedef RefCountedResource<impl::RenderPassImpl>	RenderPass;
}//	namespace api
}//	namespace pvr

