/*!
\brief The PVRVk RenderPass class.
\file PVRVk/RenderPassVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/DeviceVk.h"

namespace pvrvk {
/// <summary>Contains attachment configuration of a renderpass (format, loadop, storeop, samples).
/// </summary>
struct AttachmentDescription
{
	friend class ::pvrvk::impl::RenderPass_;

	VkFormat              format;//!< Color buffer attachment format
	VkSampleCountFlags    samples;//!< Number of samples

	VkAttachmentLoadOp    loadOp;//!< Attachment load operation (Color or Depth)
	VkAttachmentStoreOp   storeOp;//!< Attachment store operation (Color or Depth)
	VkAttachmentLoadOp    stencilLoadOp;//!< Stencil loadop (ONLY for stencil attachment)
	VkAttachmentStoreOp   stencilStoreOp;//!< Stencil storeop (ONLY for stencil attachment)
	VkImageLayout         initialLayout;  //!< initial image layout
	VkImageLayout         finalLayout;    //!< final image layout

	/// <summary>Constructor to undefined layouts/clear/store</summary>
	AttachmentDescription() :
		format(VkFormat::e_UNDEFINED),
		initialLayout(VkImageLayout::e_UNDEFINED),
		finalLayout(VkImageLayout::e_UNDEFINED),
		loadOp(VkAttachmentLoadOp::e_CLEAR),
		storeOp(VkAttachmentStoreOp::e_STORE),
		stencilLoadOp(VkAttachmentLoadOp::e_CLEAR),
		stencilStoreOp(VkAttachmentStoreOp::e_STORE),
		samples(VkSampleCountFlags::e_1_BIT)
	{}

	/// <summary>Constructor</summary>
	/// <param name="format">Attachment format</param>
	/// <param name="loadOp">Color/Depth attachment looad operation. Default is Clear. For performance, prefer Ignore if possible for
	/// your application.</param>
	/// <param name="storeOp">Color/Depth attachment store operator. Default is Store. For performance, prefer Ignore if possible
	/// for your application.</param>
	/// <param name="stencilLoadOp">Stencil load operation. Default is Clear. For performance, prefer Ignore if possible for
	/// your application.</param>
	/// <param name="stencilStoreOp">Stencil store operator. Default is Store. For performance, prefer Ignore if possible
	/// for your application.</param>
	/// <param name="numSamples">Number of samples. Default is 1.</param>
	/// <param name="initialLayout">The initial layout that the output image will be on. Must match the actual layout
	/// of the Image. Default is ColorAttachmentOptimal.</param>
	/// <param name="finalLayout">A layout to transition the image to at the end of this renderpass. Default is
	/// ColorAttachmentOptimal.</param>
	AttachmentDescription(VkFormat format, VkImageLayout initialLayout, VkImageLayout finalLayout,
	                      VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkAttachmentLoadOp stencilLoadOp,
	                      VkAttachmentStoreOp stencilStoreOp, VkSampleCountFlags numSamples):
		format(format), initialLayout(initialLayout), finalLayout(finalLayout),
		loadOp(loadOp), storeOp(storeOp), stencilLoadOp(stencilLoadOp), stencilStoreOp(stencilStoreOp),
		samples(numSamples)
	{}

	/// <summary>Create color description</summary>
	/// <param name="format">Color format</param>
	/// <param name="initialLayout">Color initial layout</param>
	/// <param name="finalLayout">Color final layout</param>
	/// <param name="loadOp">Attachment loadop</param>
	/// <param name="storeOp">Attachment storeop</param>
	/// <param name="numSamples">Number of samples</param>
	/// <returns>AttachmentDescription</returns>
	static AttachmentDescription createColorDescription(VkFormat format,
	    VkImageLayout initialLayout = VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL,
	    VkImageLayout finalLayout = VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL,
	    VkAttachmentLoadOp  loadOp = VkAttachmentLoadOp::e_CLEAR,
	    VkAttachmentStoreOp  storeOp = VkAttachmentStoreOp::e_STORE,
	    VkSampleCountFlags  numSamples = VkSampleCountFlags::e_1_BIT);

	/// <summary>Create depthstencil description</summary>
	/// <param name="format">Attachment format</param>
	/// <param name="initialLayout">Attachment initial layout</param>
	/// <param name="finalLayout">Attachment final layout</param>
	/// <param name="loadOp">Depth load op.</param>
	/// <param name="storeOp">Depth store op</param>
	/// <param name="stencilLoadOp">Stencil load op </param>
	/// <param name="stencilStoreOp">Stencil store op</param>
	/// <param name="numSamples">Number of samples</param>
	/// <returns>AttachmentDescription</returns>
	static AttachmentDescription createDepthStencilDescription(VkFormat format,
	    VkImageLayout initialLayout = VkImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	    VkImageLayout finalLayout = VkImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	    VkAttachmentLoadOp loadOp = VkAttachmentLoadOp::e_CLEAR,
	    VkAttachmentStoreOp storeOp = VkAttachmentStoreOp::e_DONT_CARE,
	    VkAttachmentLoadOp stencilLoadOp = VkAttachmentLoadOp::e_CLEAR,
	    VkAttachmentStoreOp stencilStoreOp = VkAttachmentStoreOp::e_DONT_CARE,
	    VkSampleCountFlags numSamples = VkSampleCountFlags::e_1_BIT);
};

/// <summary>Contains attachment reference of a subpass</summary>
struct AttachmentReference
{
	uint32_t       attachment;//!< The index of the AttachmentDescription of the render pass
	VkImageLayout  layout;//!< The layout the attachment uses during the subpass.

	/// <summary>AttachmentReference</summary>
	AttachmentReference() : layout(VkImageLayout::e_UNDEFINED), attachment(static_cast<uint32_t>(-1)) {}

	/// <summary>Constructor from individual elements</summary>
	/// <param name="attachment">The index of the AttachmentDescription of the render pass</param>
	/// <param name="layout">The layout the attachment uses during the subpass.</param>
	AttachmentReference(uint32_t attachment, VkImageLayout layout) : attachment(attachment), layout(layout) {}

	/// <summary>Return true if this attachment reference is valid</summary>
	/// <returns>Return true if this is a valid attachment reference</returns>
	bool isValid()const { return attachment != uint32_t(-1) && layout != VkImageLayout::e_UNDEFINED; }
};

/// <summary>Render pass subpass. Subpasses allow intermediate draws to be chained and communicating with techniques
/// like Pixel Local Storage without outputting to the FrameBuffer until the end of the RenderPass.</summary>
struct SubPassDescription
{
public:
	/// <summary>Constructor</summary>
	/// <param name="pipeBindPoint">The binding point for this subpass (Graphics, Compute). Default Graphics.</param>
	SubPassDescription(VkPipelineBindPoint pipeBindPoint = VkPipelineBindPoint::e_GRAPHICS) :
		_pipelineBindPoint(pipeBindPoint), _numInputAttachments(0), _numColorAttachments(0),
		_numResolveAttachments(0),  _numPreserveAttachments(0)
	{
		for (uint32_t i = 0; i < FrameworkCaps::MaxPreserveAttachments; ++i)
		{
			_preserveAttachment[i] = static_cast<uint32_t>(-1);
		}
	}

	/// <summary>Set the pipeline binding point.</summary>
	/// <param name="bindingPoint">New pipeline binding point</param>
	/// <returns>Reference to this(allows chaining)</returns>
	SubPassDescription& setPipelineBindPoint(VkPipelineBindPoint bindingPoint)
	{
		_pipelineBindPoint = bindingPoint;
		return *this;
	}

	/// <summary>Activate the specified color output attachment of the framebuffer.</summary>
	/// <param name="bindingIndex">Corresponding fragment shader output location. The Index must start from 0 and must be
	/// consective.</param>
	/// <param name="attachmentReference">Attachment to activate as output</param>
	/// <returns>Reference to this(allows chaining)</returns>
	SubPassDescription& setColorAttachmentReference(uint32_t bindingIndex, const AttachmentReference& attachmentReference)
	{
		_numColorAttachments += static_cast<uint8_t>(setAttachment(bindingIndex, attachmentReference, _colorAttachment,
		                        static_cast<uint32_t>(FrameworkCaps::MaxColorAttachments)));
		return *this;
	}

	/// <summary>Set the specified color attachment as input.</summary>
	/// <param name="bindingIndex">Corresponding fragment shader input location. The Index must start from 0 and must be
	/// consective.</param>
	/// <param name="attachmentReference">Attachment to set as input</param>
	/// <returns>Reference to this(allows chaining)</returns>
	SubPassDescription& setInputAttachmentReference(uint32_t bindingIndex, const AttachmentReference& attachmentReference)
	{
		_numInputAttachments += static_cast<uint8_t>(setAttachment(bindingIndex, attachmentReference, _inputAttachment,
		                        static_cast<uint32_t>(FrameworkCaps::MaxInputAttachments)));
		return *this;
	}

	/// <summary>Activate the specified Resolve attachment of the framebuffer.</summary>
	/// <param name="bindingIndex">Corresponding fragment shader input location. The Index must start from 0 and must be
	/// consective.</param>
	/// <param name="attachmentReference">Attachment to set as resolve</param>
	/// <returns>this (allow chaining)</returns>
	SubPassDescription& setResolveAttachmentReference(uint32_t bindingIndex, const AttachmentReference& attachmentReference)
	{
		_numResolveAttachments += static_cast<uint8_t>(setAttachment(bindingIndex, attachmentReference, _resolveAttachments,
		                          static_cast<uint32_t>(FrameworkCaps::MaxResolveAttachments)));
		return *this;
	}

	/// <summary>Set preserve attachment from the framebuffer.</summary>
	/// <param name="bindingIndex">The Index must start from 0 and must be consective.</param>
	/// <param name="preserveAttachment">Attachment to set as preserve</param>
	/// <returns>Reference to this(allows chaining)</returns>
	/// <returns>this (allow chaining)</returns>
	SubPassDescription& setPreserveAttachmentReference(uint32_t bindingIndex, const uint32_t preserveAttachment)
	{
		_numPreserveAttachments += (this->_preserveAttachment[bindingIndex] == static_cast<uint32_t>(-1) && preserveAttachment != static_cast<uint32_t>(-1)) ? 1 : 0;
		this->_preserveAttachment[bindingIndex] = preserveAttachment;
		return *this;
	}

	/// <summary>Sets depth stencil attachment reference.</summary>
	/// <param name="attachmentReference">New depth-stencil attachment reference</param>
	/// <returns>Reference to this(allows chaining)</returns>
	SubPassDescription& setDepthStencilAttachmentReference(const AttachmentReference& attachmentReference)
	{
		_depthStencilAttachment = attachmentReference;
		return *this;
	}

	/// <summary>Return number of color attachments</summary>
	/// <returns>Return number of color attachment references</returns>
	uint8_t getNumColorAttachmentReference()const
	{
		return _numColorAttachments;
	}

	/// <summary>Return number of input attachments</summary>
	/// <returns>Return number of input attachment references</returns>
	uint8_t getNumInputAttachmentReference()const
	{
		return _numInputAttachments;
	}

	/// <summary>Get number of resolve attachments (const)</summary>
	/// <returns>Number of resolve attachments</returns>
	uint8_t getNumResolveAttachmentReference()const { return _numResolveAttachments; }

	/// <summary>Return number of preserve attachments (const)</summary>
	//// <returns>Number of preserve attachments</returns>
	uint8_t getNumPreserveAttachmentReference()const
	{
		return _numPreserveAttachments;
	}

	/// <summary>Get pipeline binding point (const)</summary>
	/// <returns>Returns Pipeline binding point</returns>
	VkPipelineBindPoint getPipelineBindPoint()const
	{
		return _pipelineBindPoint;
	}

	/// <summary>Get input attachment id (const)</summary>
	/// <param name="index">Attachment index</param>
	/// <returns>Input attachment id</returns>
	const AttachmentReference& getInputAttachmentReference(uint8_t index)const
	{
		assertion(index < _numInputAttachments,  "Invalid index");
		return _inputAttachment[index];
	}

	/// <summary>Get depth stencil attachment reference (const).</summary>
	/// <returns> Return depth-stencil attachment reference id</returns>
	const AttachmentReference& getDepthStencilAttachmentReference() const { return this->_depthStencilAttachment; }

	/// <summary>Get color attachment id (const)</summary>
	/// <param name="index">Attachment index</param>
	/// <returns>Color attachment id</returns>
	const AttachmentReference& getColorAttachmentReference(uint8_t index)const
	{
		assertion(index < _numColorAttachments,  "Invalid index");
		return _colorAttachment[index];
	}

	/// <summary>Get resolve attachment id (const)</summary>
	/// <param name="index">Attachment index</param>
	/// <returns>Resolve attachment id</returns>
	const AttachmentReference& getResolveAttachmentReference(uint8_t index)const
	{
		assertion(index < _numResolveAttachments,  "Invalid index");
		return _resolveAttachments[index];
	}

	/// <summary>Get preserve attachment id (const)</summary>
	/// <param name="index">Attachment index</param>
	/// <returns>Preserve attachment id</returns>
	uint32_t getPreserveAttachmentReference(uint8_t index)const
	{
		assertion(index < _numPreserveAttachments,  "Invalid index");
		return _preserveAttachment[index];
	}

	/// <summary>Get all preserve attahments ids (const)</summary>(const)
	/// <returns>Return all preserve attachments id</returns>
	const uint32_t* getAllPreserveAttachments()const
	{
		return _preserveAttachment;
	}

	/// <summary>clear all entries</summary>
	/// <returns>Returns this object (allows chained calls)</returns>
	SubPassDescription& clear()
	{
		_numInputAttachments = _numResolveAttachments = _numPreserveAttachments = _numColorAttachments = 0;
		memset(_inputAttachment, 0, sizeof(_inputAttachment[0]) * (FrameworkCaps::MaxInputAttachments));
		memset(_colorAttachment, 0, sizeof(_colorAttachment[0]) * (FrameworkCaps::MaxColorAttachments));
		memset(_preserveAttachment, 0, sizeof(_preserveAttachment[0]) * (FrameworkCaps::MaxPreserveAttachments));
		return *this;
	}
private:
	uint32_t setAttachment(uint32_t bindingId, const AttachmentReference& newAttachment, AttachmentReference* attachments, uint32_t maxAttachment)
	{
		assertion(bindingId < maxAttachment, "Binding Id exceeds the max limit");
		const uint32_t oldId = attachments[bindingId].attachment;
		attachments[bindingId] = newAttachment;
		return (oldId == static_cast<uint32_t>(-1) ? 1 : 0);
	}

	VkPipelineBindPoint  _pipelineBindPoint;
	AttachmentReference _inputAttachment[FrameworkCaps::MaxInputAttachments];
	AttachmentReference _colorAttachment[FrameworkCaps::MaxColorAttachments];
	AttachmentReference _resolveAttachments[FrameworkCaps::MaxResolveAttachments];
	uint32_t _preserveAttachment[FrameworkCaps::MaxPreserveAttachments];
	AttachmentReference _depthStencilAttachment;
	uint8_t _numInputAttachments;
	uint8_t _numColorAttachments;
	uint8_t _numResolveAttachments;
	uint8_t _numPreserveAttachments;
};
/// <summary>The SubPassDependency struct Describes the dependecy between pair of sub passes.</summary>
struct SubPassDependency
{
	uint32_t srcSubPass;//!< Subpass index of the first subpass in the dependency, or SubpassExternal
	uint32_t dstSubPass;//!< Subpass index of the second subpass in the dependency, or VK_SUBPASS_EXTERNAL
	VkPipelineStageFlags   srcStageMask;//!< Bitmask of VkPipelineStageFlagBits specifying the source stage mask
	VkPipelineStageFlags dstStageMask;//!< Bitmask of VkPipelineStageFlagBits specifying the destination stage mask
	VkAccessFlags srcAccessMask;//!< Bitmask of VkAccessFlagBits specifying a source access mask
	VkAccessFlags dstAccessMask;//!< Bitmask of VkAccessFlagBits specifying a destination access mask
	VkDependencyFlags dependencyByRegion;//!< Bitmask of VkDependencyFlagBits.

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
	SubPassDependency(uint32_t srcSubPass, uint32_t dstSubPass, VkPipelineStageFlags srcStageMask,
	                  VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask,
	                  VkAccessFlags dstAccessMask, VkDependencyFlags dependencyByRegion) :
		srcSubPass(srcSubPass), dstSubPass(dstSubPass), srcStageMask(srcStageMask), dstStageMask(dstStageMask),
		srcAccessMask(srcAccessMask), dstAccessMask(dstAccessMask), dependencyByRegion(dependencyByRegion)
	{}

};

/// <summary>RenderPass creation parameters.</summary>
struct RenderPassCreateInfo
{
private:
	enum {total_max_attachments = static_cast<uint32_t>(FrameworkCaps::MaxColorAttachments) + static_cast<uint32_t>(FrameworkCaps::MaxDepthStencilAttachments)};
	friend class impl::RenderPass_;
	AttachmentDescription   _attachmentDescriptions[total_max_attachments];
	std::vector<SubPassDescription> _subpass;
	std::vector<SubPassDependency> _subPassDependency;
	uint32_t _numAttachmentDescription;
public:
	/// <summary>RenderPassCreateInfo</summary>
	RenderPassCreateInfo() : _numAttachmentDescription(0) {}

	/// <summary>Clear all entries</summary>
	void clear()
	{
		_subpass.clear();
		_subPassDependency.clear();
	}

	/// <summary>Return number of subpasses (const)</summary>
	/// <returns>Number of subpasses</returns>
	uint32_t getNumSubPasses()const
	{
		return static_cast<uint32_t>(_subpass.size());
	}

	/// <summary>Get subpass (const)</summary>
	/// <param name="index">Subpass index</param>
	/// <returns>Subpas</returns>
	const SubPassDescription& getSubPass(uint32_t index)const
	{
		debug_assertion(index < getNumSubPasses(),  "Invalid subpass index");
		return _subpass[index];
	}

	/// <summary>Get number of subpass dependencies (const)</summary>
	/// <returns>Number of subpass dependencies</returns>
	uint32_t getNumSubPassDependencies()const
	{
		return static_cast<uint32_t>(_subPassDependency.size());
	}

	/// <summary>Get subpass dependency (const)</summary>
	/// <param name="index">Subpass dependency index</param>
	/// <returns>SubPassDependency</returns>
	const SubPassDependency& getSubPassDependency(uint32_t index)const
	{
		debug_assertion(index < getNumSubPassDependencies(), "Invalid subpass dependency index");
		return _subPassDependency[index];
	}

	/// <summary>Return number of color attachments (const)</summary>
	/// <returns>Number of color attachments</returns>
	uint32_t getNumAttachmentDescription()const
	{
		return _numAttachmentDescription;
	}

	/// <summary>Get render pass color info (const)</summary>
	/// <param name="index">Color info index</param>
	/// <returns>RenderPassColorInfo</returns>
	const AttachmentDescription& getAttachmentDescription(uint32_t index)const
	{
		debug_assertion(index < getNumAttachmentDescription(),  "Invalid color info index");
		return _attachmentDescriptions[index];
	}


	/// <summary>Add color info to the specified color attachment point.</summary>
	/// <param name="index">The color attachment point to add the color info, index must be consecutive</param>
	/// <param name="attachmentDescription">The attachment description to add to the attachment point</param>
	/// <returns>Reference to this object. (allow chaining)</returns>
	RenderPassCreateInfo& setAttachmentDescription(uint32_t index, const AttachmentDescription& attachmentDescription)
	{
		if (index >= total_max_attachments)
		{
			debug_assertion(false, "AttachmentDescription exceeds the max attachment limit");
			Log("AttachmentDescription exceeds the max attachment limit %d",
			    total_max_attachments);
		}
		_numAttachmentDescription += static_cast<uint32_t>(this->_attachmentDescriptions[index].format == VkFormat::e_UNDEFINED);
		this->_attachmentDescriptions[index] = attachmentDescription;
		return *this;
	}

	/// <summary>Add a subpass to this renderpass</summary>
	/// <param name="index">Index where to add the subpass, the index must be consective</param>
	/// <param name="subpass">The SubPass to add</param>
	/// <returns>Reference to this object. (allow chaining)</returns>
	RenderPassCreateInfo& setSubPass(uint32_t index, const SubPassDescription& subpass)
	{
		if (index >= this->_subpass.size())
		{
			this->_subpass.resize(index + 1);
		}
		this->_subpass[index] = subpass;
		return *this;
	}

	/// <summary>Add a subpass dependecy to this renderpass</summary>
	/// <param name="subPassDependency">The SubPass dependency to add</param>
	/// <returns>Reference to this object. (allow chaining)</returns>
	RenderPassCreateInfo& addSubPassDependency(const SubPassDependency& subPassDependency)
	{
		if ((subPassDependency.srcSubPass != pvrvk::SubpassExternal) &&
		    (subPassDependency.srcSubPass > subPassDependency.dstSubPass))
		{
			debug_assertion(false, " Source Sub pass must be less than or equal to destination Sub pass");
		}
		_subPassDependency.push_back(subPassDependency);
		return *this;
	}

	/// <summary>Add subpass dependencies</summary>
	/// <param name="subPassDependencies">Pointer to supass dependencies</param>
	/// <param name="numDependencies">Number of supass dependencies</param>
	/// <returns>Return this.</returns>
	RenderPassCreateInfo& addSubPassDependencies(const SubPassDependency* subPassDependencies, uint32_t numDependencies)
	{
		for (uint32_t i = 0; i < numDependencies; ++i) { addSubPassDependency(subPassDependencies[i]); }
		return *this;
	}
};

namespace impl {

/// <summary>Vulkan implementation of the RenderPass class.
/// Use through the Reference counted framework object
/// pvrvk::api::RenderPass. RenderPass Compatibility: Framebuffers and graphics pipelines are created based on a
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
	DECLARE_NO_COPY_SEMANTICS(RenderPass_)

	/// <summary>getCreateInfo</summary>
	/// <returns></returns>
	const RenderPassCreateInfo& getCreateInfo()const { return _createInfo; }

	/// <summary>Get vulkan object </summary>(const)
	/// <returns>VkRenderPass&</returns>
	const VkRenderPass& getNativeObject()const {return _vkRenderPass; }

	/// <summary>Get the device which owns this resource</summary>
	/// <returns>DeviceWeakPtr</returns>
	DeviceWeakPtr getDevice() const
	{
		return _device;
	}

private:
	template<typename> friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;

	RenderPass_(const DeviceWeakPtr& device) : _device(device), _vkRenderPass(VK_NULL_HANDLE) {}

	bool init(const RenderPassCreateInfo& createInfo);

	/// <summary>destructor</summary>
	~RenderPass_() { destroy(); }

	/// <summary>Release all resources held by this object</summary>
	void destroy();

	DeviceWeakPtr _device;
	VkRenderPass _vkRenderPass;
	RenderPassCreateInfo _createInfo;
};
}// namespace impl
}// namespace pvrvk
