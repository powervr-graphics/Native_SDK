/*!*********************************************************************************************************************
\file         PVRApi\Vulkan\ShaderVk.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains Vulkan specific implementation of the Shader class. Use only if directly using Vulkan calls.
			  Provides the definitions allowing to move from the Framework object Texture2D to the underlying Vulkan Shader.
***********************************************************************************************************************/

#pragma once
#include "PVRApi/ApiObjects/Shader.h"
#include "PVRApi/Vulkan/ContextVk.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
namespace pvr {
namespace api {
namespace vulkan {
/*!*********************************************************************************************************************
\param  Vulkan shader wrapper
***********************************************************************************************************************/
class ShaderVk_ : public impl::Shader_, public native::HShader_
{
public:
	/*!*********************************************************************************************************************
	\brief ctor. Construct with namtoe shader handle.
	***********************************************************************************************************************/
	ShaderVk_(GraphicsContext& context, const native::HShader_& shader) : Shader_(context) { this->handle = shader.handle; }
	/*!*********************************************************************************************************************
	\brief ctor. Construct with namtoe shader handle.
	***********************************************************************************************************************/
	ShaderVk_(GraphicsContext& context) : Shader_(context), HShader_() { }

	/*!*********************************************************************************************************************
	\brief dtor.
	***********************************************************************************************************************/
	virtual ~ShaderVk_();

	/*!*********************************************************************************************************************
	\brief Destroy this shader object
	***********************************************************************************************************************/
	void destroy();
};
typedef RefCountedResource<vulkan::ShaderVk_> ShaderVk;
}
}
namespace native {
/*!*********************************************************************************************************************
\brief Get the Vulkan Shader object underlying a PVRApi Shader object.
\return A smart pointer wrapper containing the Vulkan Shader.
\description The smart pointer returned by this function works normally with the reference counting, and shares it with the
			 rest of the references to this object, keeping the underlying Vulkan object alive even if all other
             references to it (including the one that was passed to this function) are released. Release when done using it to
			 avoid leaking the object.
***********************************************************************************************************************/
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
	if (handle != VK_NULL_HANDLE && m_context.isValid())
	{
		vk::DestroyShaderModule(native_cast(*m_context).getDevice(), getNativeObject(), NULL);
		getNativeObject() = VK_NULL_HANDLE;
	}
}
}
}
}
