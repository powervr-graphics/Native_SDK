/*!
\brief The PVRVk Shader class.
\file PVRVk/ShaderVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/DeviceVk.h"
#include "PVRVk/BindingsVk.h"
namespace pvrvk {
namespace impl {
/// <summary>Vulkan shader wrapper</summary>
class Shader_
{
public:
	DECLARE_NO_COPY_SEMANTICS(Shader_)

	/// <summary>Get vulkan object</summary>
	/// <returns>const VkShaderModule&</returns>
	const VkShaderModule& getNativeObject()const { return _vkShaderModule; }
private:
	template<typename> friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;

	bool init(const std::vector<uint32_t>& shaderSrc);

	/// <summary>dtor.</summary>
	~Shader_() { destroy(); }

	/// <summary>Destroy this shader object</summary>
	void destroy()
	{
		if (_vkShaderModule != VK_NULL_HANDLE)
		{
			if (_device.isValid())
			{
				vk::DestroyShaderModule(_device->getNativeObject(), _vkShaderModule, nullptr);
				_device.reset();
			}
			else
			{
				reportDestroyedAfterContext("ShaderModule");
			}
		}
	}

	Shader_(const DeviceWeakPtr& device) : _device(device), _vkShaderModule(VK_NULL_HANDLE) { }
	VkShaderModule        _vkShaderModule;
	DeviceWeakPtr  _device;
};
}
}
