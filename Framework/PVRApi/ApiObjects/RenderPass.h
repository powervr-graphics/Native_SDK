/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\RenderPass.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the RenderPass API Object class.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/ApiObjects/Texture.h"

namespace pvr{
namespace api{
namespace impl{ class RenderPass_;}

/*!*********************************************************************************************************************
\brief    Contains information on the Color configuration of a renderpass (format, loadop, storeop, samples).
***********************************************************************************************************************/
struct RenderPassColorInfo
{
	friend class ::pvr::api::impl::RenderPass_;

	ImageDataFormat	format;//!< Color buffer attachment format
	types::LoadOp	loadOpColor;//!< Color attachment load operation
	types::StoreOp	storeOpColor;//!< Color attachment store operation
	pvr::uint32	numSamples;//!< Number of samples
	pvr::types::ImageLayout	initialLayout;  //!< initial image layout
	pvr::types::ImageLayout	finalLayout;    //!< final image layout

	/*!****************************************************************************************************************
	\brief ctor
	*******************************************************************************************************************/
	RenderPassColorInfo() :
		loadOpColor(types::LoadOp::Load), storeOpColor(types::StoreOp::Store), numSamples(1),
		initialLayout(pvr::types::ImageLayout::ColorAttachmentOptimal),
		finalLayout(pvr::types::ImageLayout::ColorAttachmentOptimal)
	{
		format.format = PixelFormat::Unknown;
	}

	/*!****************************************************************************************************************
	\brief Constructor
	\param format Color format
	\param loadOpColor Color load operation. Default is Load. For performance, prefer Ignore if possible for your application.
	\param storeOpColor Color Store operator. Default is Store. For performance, prefer Ignore if possible for your application.
	\param numSamples Number of samples. Default is 1.
	\param initialLayout The initial layout that the output image will be on. Must match the actual layout of the Image. Default is ColorAttachmentOptimal.
	\param finalLayout A layout to transition the image to at the end of this renderpass. Default is ColorAttachmentOptimal.
	*******************************************************************************************************************/
	RenderPassColorInfo(const api::ImageDataFormat& format, types::LoadOp  loadOpColor = types::LoadOp::Load,
	                    types::StoreOp  storeOpColor = types::StoreOp::Store, pvr::uint32  numSamples = 1u,
	                    pvr::types::ImageLayout initialLayout = pvr::types::ImageLayout::ColorAttachmentOptimal,
		pvr::types::ImageLayout finalLayout = pvr::types::ImageLayout::ColorAttachmentOptimal) :
		format(format), loadOpColor(loadOpColor), storeOpColor(storeOpColor),
		numSamples(numSamples), initialLayout(initialLayout), finalLayout(finalLayout)
	{}
};
/*!*********************************************************************************************************************
\brief    Contains information on the Depth/Stencil configuration of a renderpass (format, loadops, storeops, samples).
***********************************************************************************************************************/
struct RenderPassDepthStencilInfo
{
public:
	api::ImageDataFormat format;//!< Depth stencil buffer format
	types::LoadOp	loadOpDepth;//!< Depth attachment load operations
	types::StoreOp	storeOpDepth;//!< Depth attachment store operation
	types::LoadOp	loadOpStencil;//!< Stencil attachment load operation
	types::StoreOp	storeOpStencil;//!< Stencil attachment store operation
	pvr::uint32	numSamples;//!< number of samples

	/*!
	   \brief RenderPassDepthStencilInfo
	 */
	RenderPassDepthStencilInfo() : format(PixelFormat::Unknown), loadOpDepth(types::LoadOp::Load),
		storeOpDepth(types::StoreOp::Store), loadOpStencil(types::LoadOp::Load),
		storeOpStencil(types::StoreOp::Store), numSamples(1) {}

	/*!****************************************************************************************************************
	\brief ctor
	\param format Deprh stencil format
	\param loadOpDepth Depth load operator
	\param storeOpDepth Depth Store operator
	\param loadOpStencil Stencil load operator
	\param storeOpStencil Stencil Store operator
	\param numSamples Number of samples
	*******************************************************************************************************************/
	RenderPassDepthStencilInfo(const api::ImageDataFormat& format, types::LoadOp  loadOpDepth = types::LoadOp::Load,
							   types::StoreOp  storeOpDepth = types::StoreOp::Store, types::LoadOp  loadOpStencil = types::LoadOp::Load,
						       types::StoreOp storeOpStencil = types::StoreOp::Store, pvr::uint32 numSamples = 1) :
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
	/*!
	   \brief SubPass
	   \param pipeBindPoint
	 */
	SubPass(types::PipelineBindPoint pipeBindPoint = types::PipelineBindPoint::Graphics)
		: pipelineBindPoint(pipeBindPoint), numInputAttachment(0), numColorAttachment(0),
		  numResolveAttachment(0), numPreserveAttachment(0)
	{
		this->useDepthStencil = true;
		memset(inputAttachment, -1, sizeof(inputAttachment));
		memset(colorAttachment, -1, sizeof(colorAttachment));
		memset(resolveAttachment, -1, sizeof(resolveAttachment));
		memset(preserveAttachment, -1, sizeof(preserveAttachment));
	}

	/*!****************************************************************************************************************
	\brief	Activate the specified color output attachment of the fbo.
	\param  bindingIndex Corresponding fragment shader output location. The Index must start from 0 and must be consective.
	\return	Reference to this(allows chaining)
	\param	attachmentIndex Attachment to activate as output
	*******************************************************************************************************************/
	SubPass& setColorAttachment(uint32 bindingIndex, pvr::uint8 attachmentIndex)
	{
		numColorAttachment += (uint8)setAttachment(bindingIndex, attachmentIndex, colorAttachment,
							  (uint32)FrameworkCaps::MaxColorAttachments);
		return *this;
	}

	/*!****************************************************************************************************************
	\brief	Set the specified color attachment as input.
	\return	Reference to this(allows chaining)
	\param	attachmentIndex Attachment to set as input
	\param  bindingIndex Corresponding fragment shader input location. The Index must start from 0 and must be consective.
	*******************************************************************************************************************/
	SubPass& setInputAttachment(uint32 bindingIndex, pvr::uint8 attachmentIndex)
	{
		numInputAttachment += (uint8)setAttachment(bindingIndex, attachmentIndex, inputAttachment,
							  (uint32)FrameworkCaps::MaxInputAttachments);
		return *this;
	}

	/*!****************************************************************************************************************
	\brief	Activate the specified Resolve attachment of the fbo.
	\param	attachmentIndex Attachment to set as resolve
	\param  bindingIndex Corresponding fragment shader input location. The Index must start from 0 and must be consective.
	\return this (allow chaining)
	*******************************************************************************************************************/
	SubPass& setResolveAttachment(uint32 bindingIndex, pvr::uint8 attachmentIndex)
	{
		numResolveAttachment += (uint8)setAttachment(bindingIndex, attachmentIndex, resolveAttachment,
								(uint32)FrameworkCaps::MaxResolveAttachments);
		return *this;
	}

	/*!****************************************************************************************************************
	\brief	Set preserve attachment from the fbo.
	\return	Reference to this(allows chaining)
	\param	attachmentIndex  Attachment to set as preserve
	\param bindingIndex The Index must start from 0 and must be consective.
	\return this (allow chaining)
	*******************************************************************************************************************/
	SubPass& setPreserveAttachment(uint32 bindingIndex, pvr::uint8 attachmentIndex)
	{
		numPreserveAttachment += (uint8)setAttachment(bindingIndex, attachmentIndex, preserveAttachment,
								 (uint32)FrameworkCaps::MaxPreserveAttachments);
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set the pipeline binding point.
	\return	Reference to this(allows chaining)
	***********************************************************************************************************************/
	SubPass& setPipelineBindPoint(types::PipelineBindPoint bindingPoint)
	{
		pipelineBindPoint = bindingPoint;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Sets that the subpass uses the depth stencil image.
	\return	Reference to this(allows chaining)
	***********************************************************************************************************************/
	SubPass& setDepthStencilAttachment(bool useDepthStencil)
	{
		this->useDepthStencil = useDepthStencil;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Return number of color attachments
	***********************************************************************************************************************/
	uint8 getNumColorAttachment()const
	{
		return numColorAttachment;
	}

	/*!*********************************************************************************************************************
	\brief Return number of input attachments
	***********************************************************************************************************************/
	uint8 getNumInputAttachment()const
	{
		return numInputAttachment;
	}

	/*!*********************************************************************************************************************
	\brief Return number of resolve attachments
	***********************************************************************************************************************/
	uint8 getNumResolveAttachment()const
	{
		return numResolveAttachment;
	}

	/*!*********************************************************************************************************************
	\brief Return number of preserve attachments
	***********************************************************************************************************************/
	uint8 getNumPreserveAttachment()const
	{
		return numPreserveAttachment;
	}

	/*!*********************************************************************************************************************
	\brief Return pipeline binding point
	***********************************************************************************************************************/
	types::PipelineBindPoint getPipelineBindPoint()const
	{
		return pipelineBindPoint;
	}

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

	/*!
	   \brief Return true if this subpass uses depth stencil attachment.
	 */
	bool usesDepthStencilAttachment() const { return this->useDepthStencil; }

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

	/*!
	   \brief clear all entries
	   \return
	 */
	SubPass& clear()
	{
		numInputAttachment = numResolveAttachment = numPreserveAttachment = numColorAttachment = 0;
		memset(inputAttachment, 0,sizeof(inputAttachment[0]) * (uint32)FrameworkCaps::MaxInputAttachments);
		memset(colorAttachment, 0,sizeof(colorAttachment[0]) * (uint32)FrameworkCaps::MaxColorAttachments);
		memset(resolveAttachment, 0,sizeof(resolveAttachment[0]) *(uint32) FrameworkCaps::MaxResolveAttachments);
		memset(preserveAttachment, 0,sizeof(preserveAttachment[0]) * (uint32)FrameworkCaps::MaxPreserveAttachments);
		return *this;
	}
private:
	uint32 setAttachment(uint32 bindingId, uint32 attachmentId, int8* attachment, uint32 maxAttachment)
	{
		assertion(bindingId < maxAttachment, "Binding Id exceeds the max limit");
		int8 oldId = attachment[bindingId];
		attachment[bindingId] = (uint8)attachmentId;
		return (oldId >= 0 ? 0 : 1);
	}

	types::PipelineBindPoint  pipelineBindPoint;
	int8 inputAttachment[(uint32)FrameworkCaps::MaxInputAttachments];
	int8 colorAttachment[(uint32)FrameworkCaps::MaxColorAttachments];
	int8 resolveAttachment[(uint32)FrameworkCaps::MaxResolveAttachments];
	int8 preserveAttachment[(uint32)FrameworkCaps::MaxPreserveAttachments];
	uint8 numInputAttachment;
	uint8 numColorAttachment;
	uint8 numResolveAttachment;
	uint8 numPreserveAttachment;
	bool useDepthStencil;
};

/*!***************************************************************************************************************
\brief The SubPassDependency struct
	   Describes the dependecy between pair of sub passes.
******************************************************************************************************************/
struct SubPassDependency
{
	uint32 srcSubPass;//!< Producer sub pass index
	uint32 dstSubPass;//!< Consumer sub pass index
	types::ShaderStageFlags   srcStageMask;//!<
	types::ShaderStageFlags dstStageMask;
	types::AccessFlags srcAccessMask;
	types::AccessFlags dstAccessMask;
	bool dependencyByRegion;

	/*!
	   \brief SubPassDependency
	 */
	SubPassDependency() {}

	/*!***********************************************************************************************************
	\brief ctor
	\param srcSubPass
	\param dstSubPass
	\param srcStageMask
	\param dstStageMask
	\param srcAccessMask
	\param dstAccessMask
	\param dependencyByRegion
	 ************************************************************************************************************/
	SubPassDependency(uint32 srcSubPass, uint32 dstSubPass, types::ShaderStageFlags   srcStageMask,
	                  types::ShaderStageFlags dstStageMask, types::AccessFlags srcAccessMask,
		types::AccessFlags dstAccessMask, bool dependencyByRegion) :
		srcSubPass(srcSubPass), dstSubPass(dstSubPass), srcStageMask(srcStageMask), dstStageMask(dstStageMask),
		srcAccessMask(srcAccessMask), dstAccessMask(dstAccessMask), dependencyByRegion(dependencyByRegion)
	{}

};

/*!*********************************************************************************************************************
\brief  RenderPass creation parameters. Fill this object and then use it to create a RenderPass through you IGraphicsContext.
***********************************************************************************************************************/
struct RenderPassCreateParam
{
private:
	enum { MaxColorAttachments = 8};
	friend class impl::RenderPass_;
	RenderPassDepthStencilInfo	depthStencil;
	RenderPassColorInfo		color[MaxColorAttachments];
	std::vector<SubPass>	subPass;
	std::vector<SubPassDependency>	subPassDependency;
	uint32 numColorInfo;
public:
	/*!
	   \brief RenderPassCreateParam
	 */
	RenderPassCreateParam() : numColorInfo(0) {}

	/*!
	   \brief Clear all entries
	 */
	void clear()
	{
		subPass.clear();
		subPassDependency.clear();
	}

	/*!*********************************************************************************************************************
	\brief Return number of subpasses
	***********************************************************************************************************************/
	pvr::uint32 getNumSubPass()const
	{
		return (uint32) subPass.size();
	}

	/*!*********************************************************************************************************************
	\brief Get subpass
	\param index Subpass index
	\return Subpas
	***********************************************************************************************************************/
	const SubPass& getSubPass(pvr::uint32 index)const
	{
		debug_assertion(index < getNumSubPass() ,  "Invalid subpass index");
		return subPass[index];
	}

	/*!*********************************************************************************************************************
	\brief Get number of subpass dependency
	***********************************************************************************************************************/
	pvr::uint32 getNumSubPassDependencies()const
	{
		return (pvr::uint32)subPassDependency.size();
	}

	/*!*********************************************************************************************************************
	\brief Get subpass dependency
	\param index Subpass dependency index
	\return SubPassDependency
	***********************************************************************************************************************/
	const SubPassDependency& getSubPassDependency(pvr::uint32 index)const
	{
		debug_assertion(index < getNumSubPassDependencies(), "Invalid subpass dependency index");
		return subPassDependency[index];
	}

	/*!*********************************************************************************************************************
	\brief Return number of color info
	***********************************************************************************************************************/
	pvr::uint32 getNumColorInfo()const
	{
		return numColorInfo;
	}

	/*!*********************************************************************************************************************
	\brief Get render pass color info
	\param index Color info index
	\return RenderPassColorInfo
	***********************************************************************************************************************/
	const RenderPassColorInfo& getColorInfo(pvr::uint32 index)const
	{
		debug_assertion(index < getNumColorInfo() ,  "Invalid color info index");
		return color[index];
	}

	/*!*********************************************************************************************************************
	\brief Get render pass depth stencil info
	\return RenderPassDepthStencilInfo
	***********************************************************************************************************************/
	const RenderPassDepthStencilInfo& getDepthStencilInfo()const
	{
		return depthStencil;
	}

	/*!*********************************************************************************************************************
	\brief Add color info to the specified color attachment point.
	\param index The color attachment point to add the color info, index must be consecutive
	\param color The color info to add to the attachment point
	\return Reference to this object. (allow chaining)
	***********************************************************************************************************************/
	RenderPassCreateParam& setColorInfo(pvr::uint32 index, const RenderPassColorInfo& color)
	{
		if (index >= MaxColorAttachments)
		{
			debug_assertion(false, "Color attachment exceeds the max color attachment limit");
			Log("Color attachment exceeds the max color attachment limit %d", MaxColorAttachments);
		}
		numColorInfo += (uint32)(this->color[index].format.format == PixelFormat::Unknown);
		this->color[index] = color;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Add depth and stencil attachment info to this object.
	\param dsInfo The depth/stencil info to add
	\return Reference to this object. (allow chaining)
	***********************************************************************************************************************/
	RenderPassCreateParam& setDepthStencilInfo(const RenderPassDepthStencilInfo& dsInfo)
	{
		depthStencil = dsInfo;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Add a subpass to this renderpass
	\param index Index where to add the subpass, the index must be consective
	\param subPass The SubPass to add
	\return Reference to this object. (allow chaining)
	***********************************************************************************************************************/
	RenderPassCreateParam& setSubPass(pvr::uint32 index, const SubPass& subPass)
	{
		if (index >= this->subPass.size())
		{
			this->subPass.resize(index + 1);
		}
		this->subPass[index] = subPass;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Add a subpass dependecy to this renderpass
	\param subPassDependency The SubPass dependency to add
	\return Reference to this object. (allow chaining)
	***********************************************************************************************************************/
	RenderPassCreateParam& addSubPassDependency(const SubPassDependency& subPassDependency)
	{
		if(subPassDependency.srcSubPass > subPassDependency.dstSubPass)
		{
			debug_assertion(false, " Source Sub pass must be less than or equal to destination Sub pass");
		}
		this->subPassDependency.push_back(subPassDependency);
		return *this;
	}
};




namespace impl{
class BeginRenderPass;
class EndRenderPass;
/*!********************************************************************************************************************
\brief The implementation of the RenderPass. Use through the Reference counted framework object pvr::api::RenderPass.

RenderPass Compatibility: Framebuffers and graphics pipelines are created based on a specific render pass object.
They must only be used with that render pass object, or one compatible with it.

Two attachment references are compatible if they have matching format and sample count,

Two arrays of attachment references are compatible if all corresponding pairs of attachments are compatible.
If the arrays are of different lengths, attachment references not present in the smaller array are treated as unused.

Two render passes that contain only a single subpass are compatible if their corresponding color, input,
resolve, and depth/stencil attachment references are compatible.

If two render passes contain more than one subpass, they are compatible if they are identical except for:
	- Initial and final image layout in attachment descriptions
	- Load and store operations in attachment descriptions
	- Image layout in attachment references

A framebuffer is compatible with a render pass if it was created using the same render pass or a compatible render pass.
**********************************************************************************************************************/
class RenderPass_
{
public:
	/*!
	   \brief ~RenderPass_
	 */
	virtual ~RenderPass_() { }

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
	const GraphicsContext& getContext()const
	{
		return m_context;
	}

	/*!*********************************************************************************************************************
	\brief Return reference to the context which own this object
	***********************************************************************************************************************/
	GraphicsContext& getContext()
	{
		return m_context;
	}
protected:
	/*!******************************************************************************************************************
	\brief Creates a new RenderPass object. Use through the IGraphicsContext::createRenderPass.
	*********************************************************************************************************************/
	RenderPass_(GraphicsContext& device) : m_context(device) {}
	GraphicsContext m_context;
};
}
typedef RefCountedResource<impl::RenderPass_>	RenderPass;
}//	namespace api
}//	namespace pvr
