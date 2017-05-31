<<<<<<< HEAD
/*!*********************************************************************************************************************
\file         PVRApi\Vulkan\ShaderVk.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains Vulkan specific implementation of the Shader class. Use only if directly using Vulkan calls.
			  Provides the definitions allowing to move from the Framework object Texture2D to the underlying Vulkan Shader.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
=======
/*!
\brief Contains Vulkan specific implementation of the Shader class. Use only if directly using Vulkan calls. Provides
the definitions allowing to move from the Framework object Texture2D to the underlying Vulkan Shader.
\file PVRApi/Vulkan/ShaderVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
>>>>>>> 1776432f... 4.3

#pragma once
#include "PVRApi/ApiObjects/Shader.h"
#include "PVRApi/Vulkan/ContextVk.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
namespace pvr {
namespace api {
namespace vulkan {
/// <summary>Vulkan shader wrapper</summary>
class ShaderVk_ : public impl::Shader_, public native::HShader_
{
public:
	/// <summary>ctor. Construct with namtoe shader handle.</summary>
	ShaderVk_(const GraphicsContext& context, const native::HShader_& shader) : Shader_(context) { this->handle = shader.handle; }
	/// <summary>ctor. Construct with namtoe shader handle.</summary>
	ShaderVk_(const GraphicsContext& context) : Shader_(context), HShader_() { }

	/// <summary>dtor.</summary>
	virtual ~ShaderVk_();

	/// <summary>Destroy this shader object</summary>
	void destroy();
};
typedef RefCountedResource<vulkan::ShaderVk_> ShaderVk;
}
}
namespace native {
/// <summary>Get the Vulkan Shader object underlying a PVRApi Shader object.</summary>
/// <returns>A smart pointer wrapper containing the Vulkan Shader.</returns>
/// <remarks>The smart pointer returned by this function works normally with the reference counting, and shares it
/// with the rest of the references to this object, keeping the underlying Vulkan object alive even if all other
/// references to it (including the one that was passed to this function) are released. Release when done using it
/// to avoid leaking the object.</remarks>
inline HShader createNativeHandle(const RefCountedResource<api::impl::Shader_>& Shader)
{
	return static_cast<RefCountedResource<native::HShader_>/**/>(static_cast<RefCountedResource<api::vulkan::ShaderVk_>/**/>(Shader));
}

}
}

PVR_DECLARE_NATIVE_CAST(Shader)

namespace pvr {
namespace api {
namespace vulkan {
inline void pvr::api::vulkan::ShaderVk_::destroy()
{
	if (handle != VK_NULL_HANDLE && _context.isValid())
	{
		vk::DestroyShaderModule(native_cast(*_context).getDevice(), handle, NULL);
		handle = VK_NULL_HANDLE;
	}
}
}
}
}

<<<<<<< HEAD
//!\endcond
=======
>>>>>>> 1776432f... 4.3
