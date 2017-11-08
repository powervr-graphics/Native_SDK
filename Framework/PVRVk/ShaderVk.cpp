/*!
\brief Function implementations for the Shader class.
\file PVRVk/ShaderVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/ShaderVk.h"

namespace pvrvk {
namespace impl {
bool Shader_::init(const std::vector<uint32_t>& shaderSrc)
{
	if (_vkShaderModule != VK_NULL_HANDLE)
	{
		Log(LogLevel::Warning, "loadShader: Generated shader passed to loadShader. "
		    "Deleting reference to avoid leaking a preexisting shader object.");
		vk::DestroyShaderModule(_device->getNativeObject(), _vkShaderModule, NULL);
	}

	if (shaderSrc.empty()) { return false; }
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VkStructureType::e_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.pNext = nullptr;
	shaderModuleCreateInfo.codeSize = shaderSrc.size();
	shaderModuleCreateInfo.pCode = shaderSrc.data();
	return impl::vkIsSuccessful(vk::CreateShaderModule(_device->getNativeObject(),
	                            &shaderModuleCreateInfo, nullptr, &_vkShaderModule), "Shader Creation Failed");
}

}
}
