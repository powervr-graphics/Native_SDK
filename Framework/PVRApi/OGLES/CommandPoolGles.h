<<<<<<< HEAD
/*!*********************************************************************************************************************
\file         PVRApi/OGLES/CommandPoolGles.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        OpenGL ES Implementation of the CommandPool class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
=======
/*!
\brief OpenGL ES Implementation of the CommandPool class.
\file PVRApi/OGLES/CommandPoolGles.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
>>>>>>> 1776432f... 4.3
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
	static CommandPoolGles createNew(const GraphicsContext& ctx)
	{
		return EmbeddedRefCount<CommandPoolGles_>::createNew(ctx);
	}
private:
	CommandPoolGles_(const GraphicsContext& context) : CommandPool_(context) {}
	CommandPoolGles_(const CommandPoolGles_&); //deleted
	/// <summary>ctor, Construct a CommandPool</summary>
	/// <param name="context">The GraphicsContext this command pool will be constructed from.</param>

	/* IMPLEMENTING EmbeddedResource */
	void destroyObject() { }

};
}
}
}
<<<<<<< HEAD
//!\endcond
=======
>>>>>>> 1776432f... 4.3
