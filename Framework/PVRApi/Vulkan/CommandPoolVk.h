<<<<<<< HEAD
/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/CommandPoolVk.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Vulkan implementation of the PVRApi CommandPool.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
=======
/*!
\brief Vulkan implementation of the PVRApi CommandPool.
\file PVRApi/Vulkan/CommandPoolVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
>>>>>>> 1776432f... 4.3
#pragma once
#include "PVRApi/ApiObjects/CommandPool.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
namespace pvr {
namespace api {
namespace vulkan {
class CommandPoolVk_;
typedef EmbeddedRefCountedResource<CommandPoolVk_> CommandPoolVk;

/// <summary>Vulkan implementation of the Command Pool class.</summary>
class CommandPoolVk_: public impl::CommandPool_ , public native::HCommandPool_, public EmbeddedRefCount<CommandPoolVk_>
{
	// Implementing EmbeddedRefCount
	template <typename> friend class ::pvr::EmbeddedRefCount;
public:
	/// <summary>dtor</summary>
	virtual ~CommandPoolVk_();

	/// <summary>Initialize this command-pool</summary>
	/// <returns>Return true on success, false in case of error</returns>
	bool init();

	/// <summary>Destroy this command-pool, release all associated resources.</summary>
	void destroy();

	/// <summary>Create a new command-pool factory function</summary>
	/// <param name="ctx">The GraphicsContext this commandpool will be created on.</param>
	/// <returns>Return a new Commandpool. This function is necessary because the CommandPool has its own refcounting
	/// embedded so must always be created with this special factory.</returns>
	static CommandPoolVk createNew(const GraphicsContext& ctx)
	{
		return EmbeddedRefCount<CommandPoolVk_>::createNew(ctx);
	}
private:
	CommandPoolVk_(const CommandPoolVk_&); //deleted

	/// <summary>ctor, Construct a CommandPool</summary>
	/// <param name="context">The GraphicsContext this command pool will be constructed from.</param>
	CommandPoolVk_(const GraphicsContext& context) : CommandPool_(context) {}

	/* IMPLEMENTING EmbeddedResource */
	void destroyObject() { destroy(); }
};

}// namespace vulkan
}// namespace api
}// namespace pvr
PVR_DECLARE_NATIVE_CAST(CommandPool);
<<<<<<< HEAD

//!\endcond
=======
>>>>>>> 1776432f... 4.3
