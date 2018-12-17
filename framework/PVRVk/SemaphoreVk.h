/*!
\brief PVRVk Semaphore class.
\file PVRVk/SemaphoreVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/DeviceVk.h"

namespace pvrvk {
namespace impl {
/// <summary>Vulkan implementation of the Semaphore class.
/// Use to "serialize" access between CommandBuffer submissions and /Queues</summary>
class Semaphore_ : public DeviceObjectHandle<VkSemaphore>, public DeviceObjectDebugMarker<Semaphore_>
{
public:
	DECLARE_NO_COPY_SEMANTICS(Semaphore_)

	/// <summary>Get the Semaphore creation flags</summary>
	/// <returns>The set of Semaphore creation flags</returns>
	inline SemaphoreCreateFlags getFlags() const
	{
		return _createInfo.getFlags();
	}

	/// <summary>Get this Semaphore's create flags</summary>
	/// <returns>SemaphoreCreateInfo</returns>
	SemaphoreCreateInfo getCreateInfo() const
	{
		return _createInfo;
	}

private:
	template<typename>
	friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;

	Semaphore_(const DeviceWeakPtr& device, const SemaphoreCreateInfo& createInfo);

	~Semaphore_();

	/// <summary>Creation information used when creating the Semaphore.</summary>
	SemaphoreCreateInfo _createInfo;
};
} // namespace impl
} // namespace pvrvk
