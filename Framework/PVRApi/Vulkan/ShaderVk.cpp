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