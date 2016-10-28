/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/ShaderVk.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Vulkan implementations for the Shader class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/Vulkan/ShaderVk.h"

namespace pvr {
namespace api {
namespace impl {
native::HShader_& Shader_::getNativeObject()
{
	return native_cast(*this);
}

const native::HShader_& Shader_::getNativeObject() const
{
	return native_cast(*this);
}

}
namespace vulkan {
vulkan::ShaderVk_::~ShaderVk_() { destroy(); }
}
}
}
//!\endcond