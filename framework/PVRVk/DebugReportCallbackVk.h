/*!
\brief The PVRVk DebugReportCallback class.
\file PVRVk/DebugReportCallbackVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/ObjectHandleVk.h"
#include "PVRVk/DebugMarkerVk.h"
#include "PVRVk/TypesVk.h"

namespace pvrvk {

/// <summary>DebugReportCallback creation descriptor.</summary>
struct DebugReportCallbackCreateInfo
{
public:
	/// <summary>Constructor (zero initialization)</summary>
	DebugReportCallbackCreateInfo() : _flags(DebugReportFlagsEXT::e_ALL_BITS), _callback(nullptr), _userData(nullptr) {}

	/// <summary>Constructor</summary>
	/// <param name="flags">A set of DebugReportFlagsEXT which specify the events causing this callback to be called.</param>
	/// <param name="callback">The application callback function to call.</param>
	/// <param name="pUserData">The userdata which will be passed to the application callback function.</param>
	DebugReportCallbackCreateInfo(DebugReportFlagsEXT flags, PFN_vkDebugReportCallbackEXT callback, void* pUserData) : _flags(flags), _callback(callback), _userData(pUserData) {}

	/// <summary>Get the flags for the creation info</summary>
	/// <returns>The DebugReportFlagsEXT</returns>
	DebugReportFlagsEXT getFlags() const
	{
		return _flags;
	}

	/// <summary>Set the DebugReportFlagsEXT which specify the events causing this callback to be called.</summary>
	/// <param name="flags">A set of DebugReportFlagsEXT which specify the events causing this callback to be called.</param>
	/// <returns>this (allow chaining)</returns>
	DebugReportCallbackCreateInfo& setFlags(const DebugReportFlagsEXT& flags)
	{
		this->_flags = flags;
		return *this;
	}

	/// <summary>Get the application callback function</summary>
	/// <returns>The PFN_vkDebugReportCallbackEXT callback function</returns>
	PFN_vkDebugReportCallbackEXT getCallback() const
	{
		return _callback;
	}

	/// <summary>Set the PFN_vkDebugReportCallbackEXT specifying the callback function which will be called.</summary>
	/// <param name="callback">The application callback function to call.</param>
	/// <returns>this (allow chaining)</returns>
	DebugReportCallbackCreateInfo& setCallback(const PFN_vkDebugReportCallbackEXT& callback)
	{
		this->_callback = callback;
		return *this;
	}

	/// <summary>Get the user data passed to the callback</summary>
	/// <returns>The userdata which will be passed to the application callback function</returns>
	inline void* getPUserData() const
	{
		return _userData;
	}

	/// <summary>Set the user data passed to the callback.</summary>
	/// <param name="pUserData">The userdata which will be passed to the application callback function.</param>
	/// <returns>this (allow chaining)</returns>
	inline void setPUserData(void* pUserData)
	{
		this->_userData = pUserData;
	}

private:
	/// <summary>Indicates which events will cause the callback to be called</summary>
	DebugReportFlagsEXT _flags;
	/// <summary>The application callback function to call</summary>
	PFN_vkDebugReportCallbackEXT _callback;
	/// <summary>User data to be passed to the callback</summary>
	void* _userData;
};

namespace impl {
/// <summary>Vulkan DebugReportCallback wrapper</summary>
class DebugReportCallback_ : public InstanceObjectHandle<VkDebugReportCallbackEXT>
{
public:
	DECLARE_NO_COPY_SEMANTICS(DebugReportCallback_)

	/// <summary>Returns whether the DebugReportCallback object has been enabled.</summary>
	/// <returns>True if the DebugReportCallback has been enabled.</returns>
	bool isEnabled()
	{
		return _enabled;
	}

private:
	template<typename>
	friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Instance_;

	DebugReportCallback_(InstanceWeakPtr instance, const DebugReportCallbackCreateInfo& createInfo);

	~DebugReportCallback_();

	bool _enabled;
};
} // namespace impl
} // namespace pvrvk
