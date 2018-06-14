/*!
\brief PVRVk Fence class.
\file PVRVk/FenceVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/DeviceVk.h"

namespace pvrvk {
namespace impl {
/// <summary>Vulkan implementation of the Fence class
/// Fence can be used by the host to determine completion of execution of subimmisions to queues. The host
/// can be polled for the fence signal
/// .</summary>
class Fence_ : public DeviceObjectHandle<VkFence>, public DeviceObjectDebugMarker<Fence_>
{
public:
	DECLARE_NO_COPY_SEMANTICS(Fence_)

	/// <summary>Host to wait for this fence to be signaled</summary>
	/// <param name="timeoutNanos">Time out period in nanoseconds</param>
	/// <returns>True if successfully waited, false if timed out</returns>
	bool wait(uint64_t timeoutNanos = static_cast<uint64_t>(-1));

	/// <summary>Return true if this fence is signaled</summary>
	/// <returns>True if the fence is signalled, otherwise false</returns>
	bool isSignalled();

	/// <summary>Reset this fence</summary>
	void reset();

private:
	template<typename>
	friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;

	Fence_(const DeviceWeakPtr& device, FenceCreateFlags fenceCreateFlags);
	~Fence_();
};
} // namespace impl
} // namespace pvrvk
