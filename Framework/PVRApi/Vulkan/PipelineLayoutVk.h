<<<<<<< HEAD
/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/PipelineLayoutVk.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains Vulkan specific implementation of the PipelineLayout class. Use only if directly using Vulkan calls.
			  Provides the definitions allowing to move from the Framework object PipelineLayout to the underlying Vulkan PipelineLayout.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
=======
/*!
\brief Contains Vulkan specific implementation of the PipelineLayout class. Use only if directly using Vulkan calls.
Provides the definitions allowing to move from the Framework object PipelineLayout to the underlying Vulkan
PipelineLayout.
\file PVRApi/Vulkan/PipelineLayoutVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
>>>>>>> 1776432f... 4.3

#pragma once
#include "PVRApi/ApiObjects/PipelineLayout.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRApi/Vulkan/ContextVk.h"

namespace pvr {
namespace api {
namespace vulkan {
/// <summary>Vulkan implementation of the PipelineLayout class.</summary>
class PipelineLayoutVk_ : public impl::PipelineLayout_, public native::HPipelineLayout_
{
public:
	/// <summary>Initialize this PipelineLayout</summary>
	/// <param name="createParam">PipelineLayout create parameters</param>
	/// <returns>Return true on success, false in case of error</returns>
	bool init(const PipelineLayoutCreateParam& createParam);

	/// <summary>ctor, Construct a PipelineLayout</summary>
	/// <param name="context">The GraphicsContext this pipeline-layout will be constructed from.</param>
	PipelineLayoutVk_(const GraphicsContext& device) : PipelineLayout_(device) {}

	/// <summary>ctor, Release all resources held by this object</summary>
	inline void destroy();

<<<<<<< HEAD
	/*!
	\brief destructor
	*/
	~PipelineLayoutVk_()
	{
#ifdef DEBUG
		if (m_context.isValid())
=======
	/// <summary>destructor</summary>
	~PipelineLayoutVk_()
	{
#ifdef DEBUG
		if (_context.isValid())
>>>>>>> 1776432f... 4.3
		{
			destroy();
		}
		else
		{
			Log(Log.Warning, "PipelineLayout attempted to destroy after corresponding context destruction.");
		}
#else
		destroy();
#endif
	}

};
typedef RefCountedResource<PipelineLayoutVk_> PipelineLayoutVk;
}// namespace impl
}
}

PVR_DECLARE_NATIVE_CAST(PipelineLayout)

inline void pvr::api::vulkan::PipelineLayoutVk_::destroy()
{
	if (_context.isValid())
	{
		if (handle != VK_NULL_HANDLE)
		{
			vk::DestroyPipelineLayout(native_cast(*_context).getDevice(), handle, NULL);
		}
		_context.reset();
	}
	handle = VK_NULL_HANDLE;
}
<<<<<<< HEAD


//!\endcond
=======
>>>>>>> 1776432f... 4.3
