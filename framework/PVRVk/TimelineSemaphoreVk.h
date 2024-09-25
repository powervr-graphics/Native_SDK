/*!
\brief PVRVk Timelinesemaphore class.
\file PVRVk/TimelineSemaphoreVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#ifndef VULKANTIMELINESEMAPHORES_TIMELINESEMAPHOREVK_H
#define VULKANTIMELINESEMAPHORES_TIMELINESEMAPHOREVK_H

#include "SemaphoreVk.h"

#include <iostream>
namespace pvrvk {
namespace impl {
class TimelineSemaphore_ : public Semaphore_
{
private:
	friend class Device_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class TimelineSemaphore_;
	};
	
	static TimelineSemaphore constructShared(const DeviceWeakPtr& device, SemaphoreCreateInfo& createInfo)
	{
		createInfo.setSemaphoreType(SemaphoreType::e_TIMELINE);
		return std::make_shared<TimelineSemaphore_>(make_shared_enabler{}, device, createInfo);
	}

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(TimelineSemaphore_)
	TimelineSemaphore_(const TimelineSemaphore_::make_shared_enabler& enabler, const DeviceWeakPtr& device, const SemaphoreCreateInfo& createInfo);

	virtual ~TimelineSemaphore_();
	//!\endcond

	// <summary> Host waits for semaphore /summary>
	bool wait(const uint64_t& waitValue, uint64_t timeoutNanos = static_cast<uint64_t>(-1));
};
} // namespace impl
/// <summary>Timeline Semaphore submit info. Contains the information on timeline semaphores</summary>
struct TimelineSemaphoreSubmitInfo
{
	uint32_t waitSemaphoreValueCount ;
	const uint64_t* waitSemaphoreValues;
	uint32_t signalSemaphoreValueCount;
	const uint64_t* signalSemaphoreValues;

	/// <summary>Constructor. Default initialised to 0.</summary>
	TimelineSemaphoreSubmitInfo()
		: waitSemaphoreValueCount(0), waitSemaphoreValues(nullptr), signalSemaphoreValueCount(0), signalSemaphoreValues(nullptr)
	{}
	TimelineSemaphoreSubmitInfo(uint32_t waitSemaphoreValueCount, const uint64_t *waitSemaphoreValues, uint32_t signalSemaphoreValueCount, const uint64_t *signalSemaphoreValues):waitSemaphoreValueCount(waitSemaphoreValueCount),waitSemaphoreValues(waitSemaphoreValues),signalSemaphoreValueCount(signalSemaphoreValueCount),signalSemaphoreValues(signalSemaphoreValues){}
};
} // namespace pvrvk

#endif // VULKANTIMELINESEMAPHORES_TIMELINESEMAPHOREVK_H
