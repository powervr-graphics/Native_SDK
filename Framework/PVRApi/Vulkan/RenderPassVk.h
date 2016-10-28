/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/RenderPassVk.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains Vulkan specific implementation of the RenderPass class. Use only if directly using Vulkan calls.
			  Provides the definitions allowing to move from the Framework object RenderPass to the underlying Vulkan RenderPass.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN

#pragma once
#include "PVRApi/ApiObjects/RenderPass.h"
#include "PVRApi/Vulkan/ContextVk.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRNativeApi/ApiErrors.h"

namespace pvr {
namespace api {
namespace vulkan {
/*!*********************************************************************************************************************
\brief Vulkan implementation of the RenderPass class.
***********************************************************************************************************************/
class RenderPassVk_ : public impl::RenderPass_, public native::HRenderPass_
{
public:

	/*!*********************************************************************************************************************
	\brief ctor, Construct a RenderPass
	\param context The GraphicsContext this pipeline-layout will be constructed from.
	***********************************************************************************************************************/
	RenderPassVk_(GraphicsContext& device) : impl::RenderPass_(device){}
	
	/*!*********************************************************************************************************************
	\brief Initialize this RenderPass
	\param createParam RenderPass create parameters
	\return Return true on success, false in case of error
	***********************************************************************************************************************/
	bool init(const RenderPassCreateParam& createParam);

	/*!*********************************************************************************************************************
	\brief Release all resources held by this object
	***********************************************************************************************************************/
	void destroy();

	/*!
		\brief destructor
	*/
	~RenderPassVk_();
};

typedef RefCountedResource<RenderPassVk_> RenderPassVk;
}// namespace vulkan
}// namespace api
}// namespace pvr

PVR_DECLARE_NATIVE_CAST(RenderPass);
//!\endcond