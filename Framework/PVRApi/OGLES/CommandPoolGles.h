#pragma once
#include "PVRApi/ApiObjects/CommandPool.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
namespace pvr {
namespace api {
namespace gles {
class CommandPoolGles_;
typedef EmbeddedRefCountedResource<CommandPoolGles_> CommandPoolGles;
class CommandPoolGles_ : public impl::CommandPool_, public native::HCommandPool_, public EmbeddedRefCount<CommandPoolGles_>
{
	template <typename> friend class ::pvr::EmbeddedRefCount;
public:
	bool init() { return true; }
	void destroy() {/*DO NOTHING*/}
	static CommandPoolGles createNew(GraphicsContext& ctx)
	{
		return EmbeddedRefCount<CommandPoolGles_>::createNew(ctx);
	}
private:
	CommandPoolGles_(GraphicsContext& context) : CommandPool_(context) {}
	CommandPoolGles_(const CommandPoolGles_&); //deleted
	/*!*********************************************************************************************************************
	\brief ctor, Construct a CommandPool
	\param context The GraphicsContext this command pool will be constructed from.
	***********************************************************************************************************************/

	/* IMPLEMENTING EmbeddedResource */
	void destroyObject() { destroy(); }

};
}
}
}