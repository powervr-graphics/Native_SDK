/*!
\brief Contains the RenderPass API Object class.
\file PVRApi/ApiObjects/RenderPass.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/ApiObjects/Texture.h"

namespace pvr {
namespace api {
namespace impl { class RenderPass_;}

/// <summary>Contains information on the Color configuration of a renderpass (format, loadop, storeop, samples).
/// </summary>
struct RenderPassColorInfo
{
	friend class ::pvr::api::impl::RenderPass_;

	ImageDataFormat format;//!< Color buffer attachment format
	types::LoadOp loadOpColor;//!< Color attachment load operation
	types::StoreOp  storeOpColor;//!< Color attachment store operation
	pvr::uint32 numSamples;//!< Number of samples
	pvr::types::ImageLayout initialLayout;  //!< initial image layout
	pvr::types::ImageLayout finalLayout;    //!< final image layout

	/// <summary>ctor</summary>
	RenderPassColorInfo() :
		loadOpColor(types::LoadOp::Load), storeOpColor(types::StoreOp::Store), numSamples(1),
		initialLayout(pvr::types::ImageLayout::ColorAttachmentOptimal),
		finalLayout(pvr::types::ImageLayout::ColorAttachmentOptimal)
	{
		format.format = PixelFormat::Unknown;
	}

	/// <summary>Constructor</summary>
	/// <param name="format">Color format</param>
	/// <param name="loadOpColor">Color load operation. Default is Load. For performance, prefer Ignore if possible for
	/// your application.</param>
	/// <param name="storeOpColor">Color Store operator. Default is Store. For performance, prefer Ignore if possible
	/// for your application.</param>
	/// <param name="numSamples">Number of samples. Default is 1.</param>
	/// <param name="initialLayout">The initial layout that the output image will be on. Must match the actual layout
	/// of the Image. Default is ColorAttachmentOptimal.</param>
	/// <param name="finalLayout">A layout to transition the image to at the end of this renderpass. Default is
	/// ColorAttachmentOptimal.</param>
	RenderPassColorInfo(const ImageDataFormat& format, types::LoadOp  loadOpColor = types::LoadOp::Load,
	                    types::StoreOp  storeOpColor = types::StoreOp::Store, pvr::uint32  numSamples = 1u,
	                    pvr::types::ImageLayout initialLayout = pvr::types::ImageLayout::ColorAttachmentOptimal,
	                    pvr::types::ImageLayout finalLayout = pvr::types::ImageLayout::ColorAttachmentOptimal) :
		format(format), loadOpColor(loadOpColor), storeOpColor(storeOpColor),
		numSamples(numSamples), initialLayout(initialLayout), finalLayout(finalLayout)
	{}

	RenderPassColorInfo(const ImageStorageFormat& format, types::LoadOp  loadOpColor = types::LoadOp::Load,
	                    types::StoreOp  storeOpColor = types::StoreOp::Store,
	                    pvr::types::ImageLayout initialLayout = pvr::types::ImageLayout::ColorAttachmentOptimal,
	                    pvr::types::ImageLayout finalLayout = pvr::types::ImageLayout::ColorAttachmentOptimal) :
		format(format), loadOpColor(loadOpColor), storeOpColor(storeOpColor),
		numSamples(format.numSamples), initialLayout(initialLayout), finalLayout(finalLayout)
	{}


};
/// <summary>Contains information on the Depth/Stencil configuration of a renderpass (format, loadops, storeops,
/// samples).</summary>
struct RenderPassDepthStencilInfo
{
public:
	ImageDataFormat format;//!< Depth stencil buffer format
	types::LoadOp loadOpDepth;//!< Depth attachment load operations
	types::StoreOp  storeOpDepth;//!< Depth attachment store operation
	types::LoadOp loadOpStencil;//!< Stencil attachment load operation
	types::StoreOp  storeOpStencil;//!< Stencil attachment store operation
	pvr::uint32 numSamples;//!< number of samples

	/// <summary>RenderPassDepthStencilInfo</summary>
	RenderPassDepthStencilInfo() : format(PixelFormat::Unknown), loadOpDepth(types::LoadOp::Load),
		storeOpDepth(types::StoreOp::Store), loadOpStencil(types::LoadOp::Load),
		storeOpStencil(types::StoreOp::Store), numSamples(1) {}

	/// <summary>ctor</summary>
	/// <param name="format">Deprh stencil format</param>
	/// <param name="loadOpDepth">Depth load operator</param>
	/// <param name="storeOpDepth">Depth Store operator</param>
	/// <param name="loadOpStencil">Stencil load operator</param>
	/// <param name="storeOpStencil">Stencil Store operator</param>
	/// <param name="numSamples">Number of samples</param>
	RenderPassDepthStencilInfo(const ImageDataFormat& format, types::LoadOp  loadOpDepth = types::LoadOp::Load,
	                           types::StoreOp  storeOpDepth = types::StoreOp::Store, types::LoadOp  loadOpStencil = types::LoadOp::Load,
	                           types::StoreOp storeOpStencil = types::StoreOp::Store, pvr::uint32 numSamples = 1) :
		format(format), loadOpDepth(loadOpDepth), storeOpDepth(storeOpDepth), loadOpStencil(loadOpStencil),
		storeOpStencil(storeOpStencil), numSamples(numSamples) {}

	RenderPassDepthStencilInfo(const ImageStorageFormat& format, types::LoadOp loadOpDepth = types::LoadOp::Load,
	                           types::StoreOp  storeOpDepth = types::StoreOp::Store, types::LoadOp loadOpStencil = types::LoadOp::Load,
	                           types::StoreOp storeOpStencil = types::StoreOp::Store) :
		format(format), loadOpDepth(loadOpDepth), storeOpDepth(storeOpDepth), loadOpStencil(loadOpStencil),
		storeOpStencil(storeOpStencil), numSamples(format.numSamples) {}

};

/// <summary>Render pass subpass. Subpasses allow intermediate draws to be chained and communicating with techniques
/// like Pixel Local Storage without outputting to the FrameBuffer until the end of the RenderPass.</summary>
struct SubPass
{
public:
	/// <summary>Constructor</summary>
	/// <param name="pipeBindPoint">The binding point for this subpass (Graphics, Compute). Default Graphics.</param>
	SubPass(types::PipelineBindPoint pipeBindPoint = types::PipelineBindPoint::Graphics)
		: pipelineBindPoint(pipeBindPoint), numInputAttachment(0), numColorAttachment(0),
		  numResolveColorAttachment(0), numResolveDsAttachment(0),  numPreserveAttachment(0),
		  depthStencilAttachment(-1), enableDepthStencil(false)
	{
		memset(inputAttachment, -1, sizeof(inputAttachment));
		memset(colorAttachment, -1, sizeof(colorAttachment));
		memset(resolveColorAttachment, -1, sizeof(resolveColorAttachment));
		memset(resolveDsAttachment, -1, sizeof(resolveDsAttachment));
		memset(preserveAttachment, -1, sizeof(preserveAttachment));
	}

	/// <summary>Activate the specified color output attachment of the fbo.</summary>
	/// <param name="bindingIndex">Corresponding fragment shader output location. The Index must start from 0 and must be
	/// consective.</param>
	/// <param name="attachmentIndex">Attachment to activate as output</param>
	/// <returns>Reference to this(allows chaining)</returns>
	SubPass& setColorAttachment(uint32 bindingIndex, pvr::uint8 attachmentIndex)
	{
		numColorAttachment += (uint8)setAttachment(bindingIndex, attachmentIndex, colorAttachment,
		                      (uint32)FrameworkCaps::MaxColorAttachments);
		return *this;
	}

	/// <summary>Set the specified color attachment as input.</summary>
	/// <param name="attachmentIndex">Attachment to set as input</param>
	/// <param name="bindingIndex">Corresponding fragment shader input location. The Index must start from 0 and must be
	/// consective.</param>
	/// <returns>Reference to this(allows chaining)</returns>
	SubPass& setInputAttachment(uint32 bindingIndex, pvr::uint8 attachmentIndex)
	{
		numInputAttachment += (uint8)setAttachment(bindingIndex, attachmentIndex, inputAttachment,
		                      (uint32)FrameworkCaps::MaxInputAttachments);
		return *this;
	}

	/// <summary>Activate the specified Resolve attachment of the fbo.</summary>
	/// <param name="attachmentIndex">Attachment to set as resolve</param>
	/// <param name="bindingIndex">Corresponding fragment shader input location. The Index must start from 0 and must be
	/// consective.</param>
	/// <returns>this (allow chaining)</returns>
	SubPass& setResolveColorAttachment(uint32 bindingIndex, pvr::uint8 attachmentIndex)
	{
		numResolveColorAttachment += (uint8)setAttachment(bindingIndex, attachmentIndex, resolveColorAttachment,
		                             (uint32)FrameworkCaps::MaxResolveAttachments);
		return *this;
	}


	SubPass& setResolveDepthStencilAttachment(uint32 bindingIndex, pvr::uint8 attachmentIndex)
	{
		numResolveDsAttachment += (uint8)setAttachment(bindingIndex, attachmentIndex, resolveDsAttachment,
		                          (uint32)FrameworkCaps::MaxResolveAttachments);
		return *this;
	}



	/// <summary>Set preserve attachment from the fbo.</summary>
	/// <param name="attachmentIndex">Attachment to set as preserve</param>
	/// <param name="bindingIndex">The Index must start from 0 and must be consective.</param>
	/// <returns>Reference to this(allows chaining)</returns>
	/// <returns>this (allow chaining)</returns>
	SubPass& setPreserveAttachment(uint32 bindingIndex, pvr::uint8 attachmentIndex)
	{
		numPreserveAttachment += (uint8)setAttachment(bindingIndex, attachmentIndex, preserveAttachment,
		                         (uint32)FrameworkCaps::MaxPreserveAttachments);
		return *this;
	}

	/// <summary>Set the pipeline binding point.</summary>
	/// <returns>Reference to this(allows chaining)</returns>
	SubPass& setPipelineBindPoint(types::PipelineBindPoint bindingPoint)
	{
		pipelineBindPoint = bindingPoint;
		return *this;
	}

	/// <summary>Sets that the subpass uses the depth stencil image.</summary>
	/// <returns>Reference to this(allows chaining)</returns>
	SubPass& setDepthStencilAttachment(uint8 index)
	{
		depthStencilAttachment = index;
		return *this;
	}

	SubPass& enableDepthStencilAttachment(bool flag)
	{
		enableDepthStencil = flag;
		return *this;
	}

	/// <summary>Return number of color attachments</summary>
	uint8 getNumColorAttachment()const
	{
		return numColorAttachment;
	}

	/// <summary>Return number of input attachments</summary>
	uint8 getNumInputAttachment()const
	{
		return numInputAttachment;
	}

	/// <summary>Return number of resolve attachments</summary>
	uint8 getNumResolveDepthStencilAttachment()const { return numResolveDsAttachment; }

	uint8 getNumResolveColorAttachment()const { return numResolveColorAttachment; }

	/// <summary>Return number of preserve attachments</summary>
	uint8 getNumPreserveAttachment()const
	{
		return numPreserveAttachment;
	}

	/// <summary>Return pipeline binding point</summary>
	types::PipelineBindPoint getPipelineBindPoint()const
	{
		return pipelineBindPoint;
	}

	/// <summary>Get input attachment id</summary>
	/// <param name="index">Attachment index</param>
	/// <returns>Input attachment id</returns>
	uint8 getInputAttachmentId(pvr::uint8 index)const
	{
		assertion(index < numInputAttachment,  "Invalid index");
		return inputAttachment[index];
	}

	/// <summary>Return true if this subpass uses depth stencil attachment.</summary>
	uint8 getDepthStencilAttachmentId() const { return this->depthStencilAttachment; }

	bool usesDepthStencilAttachment()const { return enableDepthStencil && depthStencilAttachment != -1; }

	/// <summary>Get color attachment id</summary>
	/// <param name="index">Attachment index</param>
	/// <returns>Color attachment id</returns>
	uint8 getColorAttachmentId(pvr::uint8 index)const
	{
		assertion(index < numColorAttachment,  "Invalid index");
		return colorAttachment[index];
	}

	/// <summary>Get resolve attachment id</summary>
	/// <param name="index">Attachment index</param>
	/// <returns>Resolve attachment id</returns>
	uint8 getResolveColorAttachmentId(pvr::uint8 index)const
	{
		assertion(index < numResolveColorAttachment,  "Invalid index");
		return resolveColorAttachment[index];
	}


	/// <summary>Get resolve attachment id</summary>
	/// <param name="index">Attachment index</param>
	/// <returns>Resolve attachment id</returns>
	uint8 getResolveDepthStencilAttachmentId(pvr::uint8 index)const
	{
		assertion(index < numResolveDsAttachment,  "Invalid index");
		return resolveDsAttachment[index];
	}

	/// <summary>Get preserve attachment id</summary>
	/// <param name="index">Attachment index</param>
	/// <returns>Preserve attachment id</returns>
	uint8 getPreserveAttachmentId(pvr::uint8 index)const
	{
		assertion(index < numPreserveAttachment,  "Invalid index");
		return preserveAttachment[index];
	}

	/// <summary>clear all entries</summary>
	SubPass& clear()
	{
		numInputAttachment = numResolveColorAttachment = numPreserveAttachment = numColorAttachment = 0;
		memset(inputAttachment, 0, sizeof(inputAttachment[0]) * (uint32)FrameworkCaps::MaxInputAttachments);
		memset(colorAttachment, 0, sizeof(colorAttachment[0]) * (uint32)FrameworkCaps::MaxColorAttachments);
		memset(resolveDsAttachment, 0, sizeof(resolveDsAttachment[0]) * (uint32) FrameworkCaps::MaxResolveAttachments);
		memset(preserveAttachment, 0, sizeof(preserveAttachment[0]) * (uint32)FrameworkCaps::MaxPreserveAttachments);
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
	int8 resolveColorAttachment[(uint32)FrameworkCaps::MaxResolveAttachments];
	int8 resolveDsAttachment[(uint32)FrameworkCaps::MaxResolveAttachments];
	int8 preserveAttachment[(uint32)FrameworkCaps::MaxPreserveAttachments];
	int8 depthStencilAttachment;
	uint8 numInputAttachment;
	uint8 numColorAttachment;
	uint8 numResolveColorAttachment;
	uint8 numResolveDsAttachment;
	uint8 numPreserveAttachment;
	bool enableDepthStencil;
};

/// <summary>The SubPassDependency struct Describes the dependecy between pair of sub passes.</summary>
struct SubPassDependency
{
	uint32 srcSubPass;//!< Producer sub pass index
	uint32 dstSubPass;//!< Consumer sub pass index
	types::PipelineStageFlags   srcStageMask;//!<
	types::PipelineStageFlags dstStageMask;
	types::AccessFlags srcAccessMask;
	types::AccessFlags dstAccessMask;
	bool dependencyByRegion;

	/// <summary>SubPassDependency</summary>
	SubPassDependency() {}

	/// <summary>Constructor. "Source" refers to the operations that must be completed before "destination" operations
	/// </summary>
	/// <param name="srcSubPass">The subpass for the source of the dependency</param>
	/// <param name="dstSubPass">The subpass for the destination of the dependency.</param>
	/// <param name="srcStageMask">A bitmask describing which stages of source must be completed before destination
	/// </param>
	/// <param name="dstStageMask">A bitmask describing which stages of the destination subpass must not start
	/// before source</param>
	/// <param name="srcAccessMask">A bitmask describing which kinds of operations of source must be completed
	/// before destination</param>
	/// <param name="dstAccessMask">A bitmask describing which kinds of operations of destination must be completed
	/// before source</param>
	/// <param name="dependencyByRegion">Set to "true" if the dependencies are only relevant on a region by region
	/// base, set to false if it is possible that source calculations from a part of the image may be used in another
	/// part of the render in destination.</param>
	SubPassDependency(uint32 srcSubPass, uint32 dstSubPass, types::PipelineStageFlags srcStageMask,
	                  types::PipelineStageFlags dstStageMask, types::AccessFlags srcAccessMask,
	                  types::AccessFlags dstAccessMask, bool dependencyByRegion) :
		srcSubPass(srcSubPass), dstSubPass(dstSubPass), srcStageMask(srcStageMask), dstStageMask(dstStageMask),
		srcAccessMask(srcAccessMask), dstAccessMask(dstAccessMask), dependencyByRegion(dependencyByRegion)
	{}

};

/// <summary>RenderPass creation parameters. Fill this object and then use it to create a RenderPass through you
/// IGraphicsContext.</summary>
struct RenderPassCreateParam
{
private:
	friend class impl::RenderPass_;
	RenderPassDepthStencilInfo  depthStencil[(uint32)FrameworkCaps::MaxColorAttachments];
	RenderPassColorInfo   color[(uint32)FrameworkCaps::MaxColorAttachments];
	std::vector<SubPass>  subPass;
	std::vector<SubPassDependency>  subPassDependency;
	uint32 numColorInfo;
	uint32 numDepthStencilInfo;
	types::SampleCount multiSampleCount;
public:
	/// <summary>RenderPassCreateParam</summary>
	RenderPassCreateParam() : numColorInfo(0), numDepthStencilInfo(0), multiSampleCount(types::SampleCount::Count1) {}

	/// <summary>Clear all entries</summary>
	void clear()
	{
		subPass.clear();
		subPassDependency.clear();
	}

	/// <summary>Return number of subpasses</summary>
	pvr::uint32 getNumSubPass()const
	{
		return (uint32) subPass.size();
	}

	/// <summary>Get subpass</summary>
	/// <param name="index">Subpass index</param>
	/// <returns>Subpas</returns>
	const SubPass& getSubPass(pvr::uint32 index)const
	{
		debug_assertion(index < getNumSubPass() ,  "Invalid subpass index");
		return subPass[index];
	}

	/// <summary>Get number of subpass dependency</summary>
	pvr::uint32 getNumSubPassDependencies()const
	{
		return (pvr::uint32)subPassDependency.size();
	}

	/// <summary>Get subpass dependency</summary>
	/// <param name="index">Subpass dependency index</param>
	/// <returns>SubPassDependency</returns>
	const SubPassDependency& getSubPassDependency(pvr::uint32 index)const
	{
		debug_assertion(index < getNumSubPassDependencies(), "Invalid subpass dependency index");
		return subPassDependency[index];
	}

	/// <summary>Return number of color info</summary>
	pvr::uint32 getNumColorInfo()const
	{
		return numColorInfo;
	}

	/// <summary>Return number of depthstencil info</summary>
	pvr::uint32 getNumDepthStencilInfo()const { return numDepthStencilInfo; }

	/// <summary>Get render pass color info</summary>
	/// <param name="index">Color info index</param>
	/// <returns>RenderPassColorInfo</returns>
	const RenderPassColorInfo& getColorInfo(pvr::uint32 index)const
	{
		debug_assertion(index < getNumColorInfo() ,  "Invalid color info index");
		return color[index];
	}

	/// <summary>Get render pass depth stencil info</summary>
	/// <param name="index">The index to which to send the depth stencil</param>
	/// <returns>RenderPassDepthStencilInfo</returns>
	const RenderPassDepthStencilInfo& getDepthStencilInfo(uint32 index)const
	{
		debug_assertion(index < getNumDepthStencilInfo() ,  "Invalid depth stencil info index");
		return depthStencil[index];
	}

	/// <summary>Add color info to the specified color attachment point.</summary>
	/// <param name="index">The color attachment point to add the color info, index must be consecutive</param>
	/// <param name="color">The color info to add to the attachment point</param>
	/// <returns>Reference to this object. (allow chaining)</returns>
	RenderPassCreateParam& setColorInfo(pvr::uint32 index, const RenderPassColorInfo& color)
	{
		if (index >= (uint32)FrameworkCaps::MaxColorAttachments)
		{
			debug_assertion(false, "Color attachment exceeds the max color attachment limit");
			Log("Color attachment exceeds the max color attachment limit %d",
			    (uint32)FrameworkCaps::MaxColorAttachments);
		}
		numColorInfo += (uint32)(this->color[index].format.format == PixelFormat::Unknown);
		this->color[index] = color;
		multiSampleCount = types::SampleCount(glm::max((uint32)multiSampleCount, color.numSamples));
		return *this;
	}

	types::SampleCount getNumRasterizationSamples()const { return multiSampleCount; }

	/// <summary>Add depth and stencil attachment info to this object.</summary>
	/// <param name="index">The index of the attachment</param>
	/// <param name="dsInfo">The depth/stencil info to add</param>
	/// <returns>Reference to this object. (allow chaining)</returns>
	RenderPassCreateParam& setDepthStencilInfo(uint32 index, const RenderPassDepthStencilInfo& dsInfo)
	{
		if (index >= (uint32)FrameworkCaps::MaxDepthStencilAttachments)
		{
			debug_assertion(false, "Color attachment exceeds the max depth stencil attachment limit");
			Log("Color attachment exceeds the max depth stencil attachment limit %d",
			    (uint32)FrameworkCaps::MaxDepthStencilAttachments);
		}
		numDepthStencilInfo += (uint32)(depthStencil[index].format.format == PixelFormat::Unknown);
		depthStencil[index] = dsInfo;
		return *this;
	}

	RenderPassCreateParam& setDepthStencilInfo(const RenderPassDepthStencilInfo& dsInfo)
	{
		return setDepthStencilInfo(0, dsInfo);
	}

	/// <summary>Add a subpass to this renderpass</summary>
	/// <param name="index">Index where to add the subpass, the index must be consective</param>
	/// <param name="subPass">The SubPass to add</param>
	/// <returns>Reference to this object. (allow chaining)</returns>
	RenderPassCreateParam& setSubPass(pvr::uint32 index, const SubPass& subPass)
	{
		if (index >= this->subPass.size())
		{
			this->subPass.resize(index + 1);
		}
		this->subPass[index] = subPass;
		return *this;
	}

	/// <summary>Add a subpass dependecy to this renderpass</summary>
	/// <param name="subPassDependency">The SubPass dependency to add</param>
	/// <returns>Reference to this object. (allow chaining)</returns>
	RenderPassCreateParam& addSubPassDependency(const SubPassDependency& subPassDependency)
	{
		if ((subPassDependency.srcSubPass != types::SubpassExternal) &&
		    (subPassDependency.srcSubPass > subPassDependency.dstSubPass))
		{
			debug_assertion(false, " Source Sub pass must be less than or equal to destination Sub pass");
		}
		this->subPassDependency.push_back(subPassDependency);
		return *this;
	}

	RenderPassCreateParam& addSubPassDependencies(const SubPassDependency* subPassDependencies, uint32 numDependencies)
	{
		for (uint32 i = 0; i < numDependencies; ++i) { addSubPassDependency(subPassDependencies[i]); }
		return *this;
	}

};



namespace impl {
class BeginRenderPass;
class EndRenderPass;
/// <summary>The implementation of the RenderPass. Use through the Reference counted framework object
/// pvr::api::RenderPass. RenderPass Compatibility: Framebuffers and graphics pipelines are created based on a
/// specific render pass object. They must only be used with that render pass object, or one compatible with it.
/// Two attachment references are compatible if they have matching format and sample count, Two arrays of
/// attachment references are compatible if all corresponding pairs of attachments are compatible. If the arrays
/// are of different lengths, attachment references not present in the smaller array are treated as unused. Two
/// render passes that contain only a single subpass are compatible if their corresponding color, input, resolve,
/// and depth/stencil attachment references are compatible. If two render passes contain more than one subpass,
/// they are compatible if they are identical except for: - Initial and final image layout in attachment
/// descriptions - Load and store operations in attachment descriptions - Image layout in attachment references A
/// framebuffer is compatible with a render pass if it was created using the same render pass or a compatible
/// render pass.</summary>
class RenderPass_
{
public:
	/// <summary>~RenderPass_</summary>
	virtual ~RenderPass_() { }

	/// <summary>Return const reference to the context which own this object</summary>
	const GraphicsContext& getContext()const
	{
		return _context;
	}

	/// <summary>Return reference to the context which own this object</summary>
	GraphicsContext& getContext()
	{
		return _context;
	}

	const RenderPassCreateParam& getCreateParam()const { return _createParam; }


protected:
	/// <summary>Creates a new RenderPass object. Use through the IGraphicsContext::createRenderPass.</summary>
	RenderPass_(const GraphicsContext& device) : _context(device) {}
	GraphicsContext _context;
	RenderPassCreateParam _createParam;
};
}
typedef RefCountedResource<impl::RenderPass_> RenderPass;
}// namespace api
}// namespace pvr
