/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/CommandPoolVk.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Vulkan implementation of the PVRApi CommandPool.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiObjects/CommandPool.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
namespace pvr {
namespace api {
namespace vulkan {
class CommandPoolVk_;
typedef EmbeddedRefCountedResource<CommandPoolVk_> CommandPoolVk;

/*!*********************************************************************************************************************
\brief Vulkan implementation of the Command Pool class.
***********************************************************************************************************************/
class CommandPoolVk_: public impl::CommandPool_ , public native::HCommandPool_, public EmbeddedRefCount<CommandPoolVk_>
{
	// Implementing EmbeddedRefCount
	template <typename> friend class ::pvr::EmbeddedRefCount;
public:
	/*!*********************************************************************************************************************
	\brief dtor
	***********************************************************************************************************************/
	virtual ~CommandPoolVk_();

	/*!*********************************************************************************************************************
	\brief Initialize this command-pool
	\return Return true on success, false in case of error
	***********************************************************************************************************************/
	bool init();

	/*!*********************************************************************************************************************
	\brief Destroy this command-pool, release all associated resources.
	***********************************************************************************************************************/
	void destroy();

	/*!*********************************************************************************************************************
	\brief Create a new command-pool factory function
	\param ctx The GraphicsContext this commandpool will be created on.
	\return Return a new Commandpool. This function is necessary because the CommandPool has its own refcounting embedded
	so must always be created with this special factory.
	***********************************************************************************************************************/
	static CommandPoolVk createNew(const GraphicsContext& ctx)
	{
		return EmbeddedRefCount<CommandPoolVk_>::createNew(ctx);
	}
private:
	CommandPoolVk_(const CommandPoolVk_&); //deleted

	/*!*********************************************************************************************************************
	\brief ctor, Construct a CommandPool
	\param context The GraphicsContext this command pool will be constructed from.
	***********************************************************************************************************************/
	CommandPoolVk_(const GraphicsContext& context) : CommandPool_(context) {}

	/* IMPLEMENTING EmbeddedResource */
	void destroyObject() { destroy(); }
};

}// namespace vulkan
}// namespace api
}// namespace pvr
PVR_DECLARE_NATIVE_CAST(CommandPool);