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
private:
	template<typename>
	friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;

	explicit Semaphore_(const DeviceWeakPtr& device);

	~Semaphore_();
};
} // namespace impl
} // namespace pvrvk
