/*!
\brief The PVRVk ShaderModule class.
\file PVRVk/ShaderModuleVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/DeviceVk.h"

namespace pvrvk {
namespace impl {
/// <summary>Vulkan shader module wrapper</summary>
class ShaderModule_ : public DeviceObjectHandle<VkShaderModule>, public DeviceObjectDebugMarker<ShaderModule_>
{
public:
	DECLARE_NO_COPY_SEMANTICS(ShaderModule_)

private:
	template<typename>
	friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;

	ShaderModule_(const DeviceWeakPtr& device, const std::vector<uint32_t>& shaderSrc);

	~ShaderModule_();
};
} // namespace impl
} // namespace pvrvk
