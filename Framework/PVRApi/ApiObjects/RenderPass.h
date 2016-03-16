/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\RenderPass.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the RenderPass API Object class.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/ApiObjects/Fbo.h"
#include "PVRApi/ApiObjects/Texture.h"

namespace pvr {
namespace api {
namespace impl { class RenderPass_; }

/*!*********************************************************************************************************************
\brief    Contains information on the Color configuration of a renderpass (format, loadop, storeop, samples).
***********************************************************************************************************************/
struct RenderPassColorInfo
{
	friend class ::pvr::api::impl::RenderPass_;

	ImageDataFormat format;//!< Color buffer attachment format
	types::LoadOp::Enum         loadOpColor;//!< Color attachment load operation
	types::StoreOp::Enum        storeOpColor;//!< Color attachment store operation
	pvr::uint32          numSamples;//!< Number of samples
	
	/*!****************************************************************************************************************
	\brief ctor
	*******************************************************************************************************************/
	RenderPassColorInfo() : loadOpColor(types::LoadOp::Load), storeOpColor(types::StoreOp::Store), numSamples(1)
	{
		format.format = PixelFormat::Unknown;
	}
	
	/*!****************************************************************************************************************
	\brief ctor
	\param format Color format
	\param loadOpColor Color load operator
	\param storeOpColor Color Store operator
	\param numSamples Number of samples
	*******************************************************************************************************************/
	RenderPassColorInfo(const api::ImageDataFormat& format, types::LoadOp::Enum  loadOpColor = types::LoadOp::Load,
	                    types::StoreOp::Enum  storeOpColor = types::StoreOp::Store, pvr::uint32  numSamples = 0) : 
						format(format),	loadOpColor(loadOpColor), storeOpColor(storeOpColor), numSamples(numSamples)
	{}
};
/*!*********************************************************************************************************************
\brief    Contains information on the Depth/Stencil configuration of a renderpass (format, loadops, storeops, samples).
***********************************************************************************************************************/
struct RenderPassDepthStencilInfo
{
public:
	api::ImageDataFormat format;//!< Depth stencil buffer format
	types::LoadOp::Enum         loadOpDepth;//!< Depth attachment load operations
	types::StoreOp::Enum        storeOpDepth;//!< Depth attachment store operation
	types::LoadOp::Enum         loadOpStencil;//!< Stencil attachment load operation
	types::StoreOp::Enum        storeOpStencil;//!< Stencil attachment store operation
	pvr::uint32          numSamples;//!< number of samples
	RenderPassDepthStencilInfo() : loadOpDepth(types::LoadOp::Load), storeOpDepth(types::StoreOp::Store),
		loadOpStencil(types::LoadOp::Load), storeOpStencil(types::StoreOp::Store), numSamples(1) {}

	/*!****************************************************************************************************************
	\brief ctor
	\param format Deprh stencil format
	\param loadOpDepth Depth load operator
	\param storeOpDepth Depth Store operator
	\param loadOpStencil Stencil load operator
	\param storeOpStencil Stencil Store operator
	\param numSamples Number of samples
	*******************************************************************************************************************/	
	RenderPassDepthStencilInfo(const api::ImageDataFormat& format, types::LoadOp::Enum  loadOpDepth = types::LoadOp::Load,
	                           types::StoreOp::Enum  storeOpDepth = types::StoreOp::Store, types::LoadOp::Enum  loadOpStencil = types::LoadOp::Load,
	                           types::StoreOp::Enum storeOpStencil = types::StoreOp::Store, pvr::uint32 numSamples = 1) :
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
	SubPass(types::PipelineBindPoint::Enum pipeBindPoint = types::PipelineBindPoint::Graphics)
		: pipelineBindPoint(pipeBindPoint),
		  numColorAttachment(0), numInputAttachment(0), numPreserveAttachment(0), numResolveAttachment(0) { }

	/*!****************************************************************************************************************
	\brief	Activate the specified color output attachment of the fbo.
	\return	Reference to this(allows chaining)
	\param	attachmentIndex Attachment to activate as output
	*******************************************************************************************************************/
	SubPass& setColorAttachment(pvr::uint8 attachmentIndex) { colorAttachment[numColorAttachment++] = attachmentIndex; return *this; }

	/*!****************************************************************************************************************
	\brief	Set the specified color attachment as input.
	\return	Reference to this(allows chaining)
	\param	attachmentIndex Attachment to set as input
	*******************************************************************************************************************/
	SubPass& setInputAttachment(pvr::uint8 attachmentIndex) { inputAttachment[numInputAttachment++] = attachmentIndex; return *this; }

	/*!****************************************************************************************************************
	\brief	Activate the specified Resolve attachment of the fbo.
	\return	Reference to this(allows chaining)
	\param	attachmentIndex Attachment to set as resolve
	*******************************************************************************************************************/
	SubPass& setResolveAttachment(pvr::uint8 attachmentIndex) { resolveAttachment[numResolveAttachment++] = attachmentIndex; return *this; }

	/*!****************************************************************************************************************
	\brief	Set preserve attachment from the fbo.
	\return	Reference to this(allows chaining)
	\param	attachmentIndex  Attachment to set as preserve
	*******************************************************************************************************************/
	SubPass& setPreserveAttachment(pvr::uint8 attachmentIndex) { preserveAttachment[numPreserveAttachment++] = attachmentIndex; return *this; }

	/*!*********************************************************************************************************************
	\brief Set the pipeline binding point.
	\return	Reference to this(allows chaining)
	***********************************************************************************************************************/
	SubPass& setPipelineBindPoint(types::PipelineBindPoint::Enum bindingPoint) { pipelineBindPoint = bindingPoint; return *this;}

	/*!*********************************************************************************************************************
	\brief Return number of color attachments
	***********************************************************************************************************************/
	uint8 getNumColorAttachment()const { return numColorAttachment; }
	
	/*!*********************************************************************************************************************
	\brief Return number of input attachments
	***********************************************************************************************************************/
	uint8 getNumInputAttachment()const { return numInputAttachment; }
	
	/*!*********************************************************************************************************************
	\brief Return number of resolve attachments
	***********************************************************************************************************************/
	uint8 getNumResolveAttachment()const { return numResolveAttachment; }
	
	/*!*********************************************************************************************************************
	\brief Return number of preserve attachments
	***********************************************************************************************************************/
	uint8 getNumPreserveAttachment()const { return numPreserveAttachment; }

	/*!*********************************************************************************************************************
	\brief Return pipeline binding point
	***********************************************************************************************************************/
	types::PipelineBindPoint::Enum getPipelineBindPoint()const { return pipelineBindPoint; }

	/*!*********************************************************************************************************************
	\brief Get input attachment id
	\param index Attachment index
	\return Input attachment id
	***********************************************************************************************************************/
	uint8 getInputAttachmentId(pvr::uint8 index)const
	{
		assertion(index < numInputAttachment,  "Invalid index");
		return inputAttachment[index];
	}

	/*!*********************************************************************************************************************
	\brief Get color attachment id
	\param index Attachment index
	\return Color attachment id
	***********************************************************************************************************************/
	uint8 getColorAttachmentId(pvr::uint8 index)const
	{
		assertion(index < numColorAttachment,  "Invalid index");
		return colorAttachment[index];
	}

	/*!*********************************************************************************************************************
	\brief Get resolve attachment id
	\param index Attachment index
	\return Resolve attachment id
	***********************************************************************************************************************/
	uint8 getResolveAttachmentId(pvr::uint8 index)const
	{
		assertion(index < numResolveAttachment,  "Invalid index");
		return resolveAttachment[index];
	}

	/*!*********************************************************************************************************************
	\brief Get preserve attachment id
	\param index Attachment index
	\return Preserve attachment id
	***********************************************************************************************************************/
	uint8 getPreserveAttachmentId(pvr::uint8 index)const
	{
		assertion(index < numPreserveAttachment,  "Invalid index");
		return preserveAttachment[index];
	}
private:
	types::PipelineBindPoint::Enum  pipelineBindPoint;
	uint8 inputAttachment[8];
	uint8 colorAttachment[8];
	uint8 resolveAttachment[8];
	uint8 preserveAttachment[8];
	uint8 numInputAttachment;
	uint8 numColorAttachment;
	uint8 numResolveAttachment;
	uint8 numPreserveAttachment;
};

struct SubPassDependency
{
	uint32 srcSubPass;
	uint32 dstSubPass;
	types::ShaderStageFlags::Enum srcStageMask;
	types::ShaderStageFlags::Enum dstStageMask;
	types::AccessFlags::Enum srcAccessMask;
	types::AccessFlags::Enum dstAccessMask;
	bool dependencyByRegion;
};


/*!*********************************************************************************************************************
\brief  RenderPass creation parameters. Fill this object and then use it to create a RenderPass through you IGraphicsContext.
***********************************************************************************************************************/
struct RenderPassCreateParam
{
private:
	friend class impl::RenderPass_;
public:
	RenderPassDepthStencilInfo        depthStencil;
	std::vector<RenderPassColorInfo>  color;
	std::vector<SubPass>			  subPass;
	std::vector<SubPassDependency>	  subPassDependency;

	/*!*********************************************************************************************************************
	\brief Return number of subpasses
	***********************************************************************************************************************/
	pvr::uint32 getNumSubPass()const { return (uint32) subPass.size(); }

	/*!*********************************************************************************************************************
	\brief Get subpass
	\param index Subpass index
	\return Subpas
	***********************************************************************************************************************/
	const SubPass& getSubPass(pvr::uint32 index)const
	{
		assertion(index < getNumSubPass() ,  "Invalid subpass index");
		return subPass[index];
	}

	/*!*********************************************************************************************************************
	\brief Get number of subpass dependency
	***********************************************************************************************************************/
	pvr::uint32 getNumSubPassDependencies()const { return (pvr::uint32)subPassDependency.size(); }

	/*!*********************************************************************************************************************
	\brief Get subpass dependency
	\param index Subpass dependency index
	\return SubPassDependency
	***********************************************************************************************************************/
	const SubPassDependency& getSubPassDependency(pvr::uint32 index)const
	{
		assertion(index < getNumSubPassDependencies(), "Invalid subpass dependency index");
		return subPassDependency[index];
	}

	/*!*********************************************************************************************************************
	\brief Return number of color info
	***********************************************************************************************************************/
	pvr::uint32 getNumColorInfo()const { return (pvr::uint32)color.size(); }

	/*!*********************************************************************************************************************
	\brief Get render pass color info
	\param index Color info index
	\return RenderPassColorInfo
	***********************************************************************************************************************/
	const RenderPassColorInfo& getColorInfo(pvr::uint32 index)const
	{
		assertion(index < getNumColorInfo() ,  "Invalid color info index");
		return color[index];
	}

	/*!*********************************************************************************************************************
	\brief Get render pass depth stencil info
	\param index Depth stencil info index
	\return RenderPassDepthStencilInfo
	***********************************************************************************************************************/
	const RenderPassDepthStencilInfo& getDepthStencilInfo()const { return depthStencil; }

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

	/*!*********************************************************************************************************************
	\brief Add a subpass dependecy to this renderpass
	\param index Index where to add the subpass dependency
	\param subPassDependency The SubPass dependency to add
	\return Reference to this object. (allow chaining)
	***********************************************************************************************************************/
	RenderPassCreateParam& addSubPassDependency(const SubPassDependency& subPassDependency)
	{
		assertion(subPassDependency.srcSubPass <= subPassDependency.dstSubPass, " Source Sub pass must be less than or equal to destination Sub pass");
		this->subPassDependency.push_back(subPassDependency);
		return *this;
	}
};


namespace impl {
class BeginRenderPass;
class EndRenderPass;
/*!********************************************************************************************************************
\brief The implementation of the RenderPass. Use through the Reference counted framework object pvr::api::RenderPass.
**********************************************************************************************************************/
class RenderPass_
{
public:
	virtual ~RenderPass_() { destroy(); }

	/*!*********************************************************************************************************************
	\brief Return const reference to the underlying native object
	***********************************************************************************************************************/
	const native::HRenderPass_& getNativeObject() const;
	
	/*!*********************************************************************************************************************
	\brief Return reference to the underlying native object
	***********************************************************************************************************************/
	native::HRenderPass_& getNativeObject();
	
	/*!*********************************************************************************************************************
	\brief Return const reference to the context which own this object
	***********************************************************************************************************************/
	const GraphicsContext& getContext()const { return m_context; }
	
	/*!*********************************************************************************************************************
	\brief Return reference to the context which own this object
	***********************************************************************************************************************/
	GraphicsContext& getContext() { return m_context; }
protected:
	/*!******************************************************************************************************************
	\brief Creates a new RenderPass object. Use through the IGraphicsContext::createRenderPass.
	*********************************************************************************************************************/
	RenderPass_(GraphicsContext& device) : m_context(device) {}
	void destroy();
	GraphicsContext m_context;
};
}
typedef RefCountedResource<impl::RenderPass_>	RenderPass;
}//	namespace api
}//	namespace pvr


