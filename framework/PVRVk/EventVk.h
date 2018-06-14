/*!
\brief PVRVk Event class.
\file PVRVk/EventVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/DeviceVk.h"

namespace pvrvk {
namespace impl {
/// <summary>Vulkan implementation of the Event class.
/// Event can be used by the host to do fine-grained synchronization of commands, and it
/// can be signalled either from the host (calling set()) or the device (submitting a setEvent() command).</summary>
class Event_ : public DeviceObjectHandle<VkEvent>, public DeviceObjectDebugMarker<Event_>
{
public:
	DECLARE_NO_COPY_SEMANTICS(Event_)

	/// <summary>Set this event</summary>
	void set();

	/// <summary>Reset this event</summary>
	void reset();

	/// <summary>Return true if this event is set</summary>
	/// <returns>Return true if this event is set</returns>
	bool isSet();

private:
	template<typename>
	friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;

	explicit Event_(const DeviceWeakPtr& device);

	~Event_();
};
} // namespace impl
} // namespace pvrvk