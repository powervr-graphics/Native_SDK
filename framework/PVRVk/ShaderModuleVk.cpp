/*!
\brief Function implementations for the ShaderModule class.
\file PVRVk/ShaderModuleVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/ShaderModuleVk.h"

namespace pvrvk {
namespace impl {
ShaderModule_::ShaderModule_(const DeviceWeakPtr& device, const ShaderModuleCreateInfo& createInfo)
	: DeviceObjectHandle(device), DeviceObjectDebugMarker(DebugReportObjectTypeEXT::e_SHADER_MODULE_EXT), _createInfo(createInfo)
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		Log(LogLevel::Debug,
			"loadShader: Generated shader passed to loadShader. "
			"Deleting reference to avoid leaking a preexisting shader object.");
		_device->getVkBindings().vkDestroyShaderModule(_device->getVkHandle(), getVkHandle(), NULL);
	}

	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_SHADER_MODULE_CREATE_INFO);
	shaderModuleCreateInfo.codeSize = _createInfo.getCodeSize();
	shaderModuleCreateInfo.pCode = _createInfo.getShaderSources().data();
	vkThrowIfFailed(_device->getVkBindings().vkCreateShaderModule(_device->getVkHandle(), &shaderModuleCreateInfo, nullptr, &_vkHandle), "Failed to create ShaderModule");
}

ShaderModule_::~ShaderModule_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (_device.isValid())
		{
			_device->getVkBindings().vkDestroyShaderModule(_device->getVkHandle(), getVkHandle(), nullptr);
			_vkHandle = VK_NULL_HANDLE;
			_device.reset();
		}
		else
		{
			reportDestroyedAfterDevice("ShaderModule");
		}
	}
}
} // namespace impl
} // namespace pvrvk
