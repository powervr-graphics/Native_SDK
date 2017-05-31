/*!
\brief Contains Vulkan specific implementation of the RenderPass class. Use only if directly using Vulkan calls.
Provides the definitions allowing to move from the Framework object RenderPass to the underlying Vulkan
RenderPass.
\file PVRApi/Vulkan/RenderPassVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRApi/ApiObjects/RenderPass.h"
#include "PVRApi/Vulkan/ContextVk.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"

namespace pvr {
namespace api {
namespace vulkan {
/// <summary>Vulkan implementation of the RenderPass class.</summary>
class RenderPassVk_ : public impl::RenderPass_, public native::HRenderPass_
{
public:

	/// <summary>ctor, Construct a RenderPass</summary>
	/// <param name="context">The GraphicsContext this pipeline-layout will be constructed from.</param>
	RenderPassVk_(const GraphicsContext& device) : impl::RenderPass_(device){}
	
	/// <summary>Initialize this RenderPass</summary>
	/// <param name="createParam">RenderPass create parameters</param>
	/// <returns>Return true on success, false in case of error</returns>
	bool init(const RenderPassCreateParam& createParam);

	/// <summary>Release all resources held by this object</summary>
	void destroy();

	/// <summary>destructor</summary>
	~RenderPassVk_();
};

typedef RefCountedResource<RenderPassVk_> RenderPassVk;
}// namespace vulkan
}// namespace api
}// namespace pvr

PVR_DECLARE_NATIVE_CAST(RenderPass);
