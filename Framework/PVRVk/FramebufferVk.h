/*!
\brief PVRVk Framebuffer Object class
\file PVRVk/FramebufferVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "ImageVk.h"

namespace pvrvk {

/// <summary>Framebuffer creation descriptor.</summary>
struct FramebufferCreateInfo
{
public:
	friend class impl::Framebuffer_;
	/// <summary>The number of array layers of the Framebuffer</summary>
	uint32_t      layers;
	/// <summary>The width (in pixels) of the Framebuffer</summary>
	uint32_t      width;
	/// <summary>The hight (in pixels) of the Framebuffer</summary>
	uint32_t      height;
	/// <summary>The render pass that this Framebuffer will render in</summary>
	RenderPass  renderPass;


	/// <summary>Reset this object</summary>
	void clear()
	{
		width = 0; height = 0; layers = 1; renderPass.reset();
		for (uint32_t i = 0; i < total_max_attachments; ++i)
		{
			attachments[i].reset();
		}
	}

	/// <summary>Constructor (zero initialization)</summary>
	FramebufferCreateInfo() : layers(1), width(0), height(0), numAttachments(0) {}

	/// <summary>Return number of color attachment</summary>
	/// <returns>Number of color attachments</returns>
	uint32_t getNumAttachments()const { return numAttachments; }

	/// <summary>Get a color attachment TextureView</summary>
	/// <param name="index">The index of the Colorattachment to retrieve</param>
	/// <returns>This object</returns>
	const ImageView& getAttachment(uint32_t index)const
	{
		debug_assertion(index < numAttachments, " Invalid attachment index");
		return attachments[index];
	}
	/// <summary>Get a color attachment TextureView</summary>
	/// <param name="index">The index of the Colorattachment to retrieve</param>
	/// <returns>This object</returns>
	ImageView& getAttachment(uint32_t index)
	{
		debug_assertion(index < numAttachments, " Invalid attachment index");
		return attachments[index];
	}

	/// <summary>Get the Renderpass (const)</summary>
	/// <returns>The Renderpass (const)</summary>
	const RenderPass& getRenderPass()const { return renderPass; }

	/// <summary>Get the Renderpass</summary>
	/// <returns>The Renderpass</summary>
	RenderPass& getRenderPass() { return renderPass; }

	/// <summary>Get the dimensions of the framebuffer</summary>
	/// <returns>The framebuffer dimensions</returns>
	Extent2D getDimensions()const { return Extent2D(width, height); }

	/// <summary>Set the framebuffer dimension</summary>
	/// <param name="width">Width</param>
	/// <param name="height">Height</param>
	/// <returns>This object (allow chaining)</returns>
	FramebufferCreateInfo& setDimensions(uint32_t width, uint32_t height)
	{
		this->width = width; this->height = height; return *this;
	}

	/// <summary>Set the framebuffer dimension</summary>
	/// <param name="extent">dimension</param>
	/// <returns>This object (allow chaining)</returns>
	FramebufferCreateInfo& setDimensions(const Extent2D& extent)
	{
		width = extent.width; height = extent.height; return *this;
	}

	/// <summary>Add a color attachment to a specified attachment point.</summary>
	/// <param name="index">The attachment point, the index must be consecutive</param>
	/// <param name="colorView">The color attachment</param>
	/// <returns>this (allow chaining)</returns>
	FramebufferCreateInfo& setAttachment(uint32_t index, const ImageView colorView)
	{
		debug_assertion(index < total_max_attachments, "Index out-of-bound");
		if (index >= numAttachments) { numAttachments = index + 1; }
		this->attachments[index] = colorView;
		return *this;
	}

	/// <summary>Set the number of layers.</summary>
	/// <param name="numLayers">The number of array layers.</param>
	/// <returns>this (allow chaining)</returns>
	FramebufferCreateInfo& setNumLayers(uint32_t numLayers)
	{
		layers = numLayers; return *this;
	}

	/// <summary>Set the Renderpass which this Framebuffer will be invoking when bound.</summary>
	/// <param name="renderPass">A renderpass. When binding this Framebuffer, this renderpass will be the one to be bound.</param>
	/// <returns>this (allow chaining)</returns>
	FramebufferCreateInfo& setRenderPass(const RenderPass& renderPass)
	{
		this->renderPass = renderPass; return *this;
	}

private:
	enum { total_max_attachments = FrameworkCaps::MaxColorAttachments + FrameworkCaps::MaxDepthStencilAttachments };
	ImageView attachments[total_max_attachments];
	uint32_t numAttachments;
};

namespace impl {
/// <summary>Vulkan implementation of the Framebuffer (Framebuffer object) class.</summary>
class Framebuffer_
{
	template<typename> friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;
public:
	DECLARE_NO_COPY_SEMANTICS(Framebuffer_)
	/// <summary>Return the renderpass that this framebuffer uses.</summary>
	/// <returns>The renderpass that this Framebuffer uses.</returns>
	const RenderPass& getRenderPass() const { return _createInfo.renderPass; }

	/// <summary>Return this object create param(const)</summary>
	/// <returns>const FramebufferCreateInfo&</returns>
	const FramebufferCreateInfo& getCreateInfo() const { return _createInfo; }

	/// <summary>Get the Dimension of this framebuffer(const)</summary>
	/// <returns>Framebuffer Dimension</returns>
	Extent2D getDimensions()const { return _createInfo.getDimensions(); }


	/// <summary>Get the color attachment at a specific index</summary>
	/// <param name="index">A color attachment index.</param>
	/// <returns>The Texture that is bound as a color attachment at index.</returns>
	const ImageView& getAttachment(uint32_t index)const
	{
		return _createInfo.getAttachment(index);
	}

	/// <summary>Get the color attachment at a specific index</summary>
	/// <param name="index">A color attachment index.</param>
	/// <returns>The Texture that is bound as a color attachment at index.</returns>
	ImageView& getAttachment(uint32_t index)
	{
		return _createInfo.getAttachment(index);
	}

	/// <summary>getNumAttachments</summary>
	/// <returns>Get number of attachments</returns>
	uint32_t getNumAttachments()const
	{
		return _createInfo.getNumAttachments();
	}

	/// <summary>Get vulkan object</summary>
	/// <returns>VkFramebuffer</returns>
	const VkFramebuffer& getNativeObject()const { return _vkFramebuffer; }
private:
	Framebuffer_(const DeviceWeakPtr& device) : _device(device), _vkFramebuffer(VK_NULL_HANDLE) {}
	~Framebuffer_(){ destroy(); }
	bool init(const FramebufferCreateInfo& createInfo);
	void destroy();
	FramebufferCreateInfo _createInfo;
	VkFramebuffer _vkFramebuffer;
	DeviceWeakPtr _device;
};
}// namespace impl
}// namespace pvrvk
