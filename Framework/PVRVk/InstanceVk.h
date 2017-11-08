/*!
\brief The PVRVk Instance class
\file PVRVk/InstanceVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/ExtensionsVk.h"
#include "PVRVk/PhysicalDeviceVk.h"
#include "PVRVk/ForwardDecObjectsVk.h"
#include "PVRVk/LayersVk.h"
#include "PVRVk/SurfaceVk.h"

namespace pvrvk {
namespace impl {
class InstanceHelperFactory_;

/// <summary>The Instance is a system-wide vulkan "implementation", similar in concept to the
/// "installation" of Vulkan libraries on a system. Contrast with the "Physical Device" which
/// for example represents a particular driver implementing Vulkan for a specific Device.
/// Conceptually, the Instance "Forwards" to the "Physical Device / Device"</summary>
class Instance_ : public EmbeddedRefCount<Instance_>
{
public:
	DECLARE_NO_COPY_SEMANTICS(Instance_)

	/// <summary>Get Vulkan object (const)</summary>
	/// <returns>const VkInstance&</returns>
	const VkInstance& getNativeObject()const
	{
		return _instance;
	}

	/// <summary>Get instance create info(const)</summary>
	/// <returns>const InstanceCreateInfo&</returns>
	const InstanceCreateInfo& getInfo()const
	{
		return  _createInfo;
	}

	/// <summary>Get surface if exists(const)</summary>
	/// <returns> Surface</returns>
	Surface getSurface() const;

	/// <summary>Get physical device (const)</summary>
	/// <param name="id">Physcialdevice id</param>
	/// <returns>const PhysicalDevice&</returns>
	const PhysicalDevice& getPhysicalDevice(uint32_t id)const;

	/// <summary>Get physical device (const)</summary>
	/// <param name="id">Physcialdevice id</param>
	/// <returns>const PhysicalDevice&</returns>
	PhysicalDevice getPhysicalDevice(uint32_t id);

	/// <summary>Create surface</summary>
	/// <param name="physicalDevice"> Physical device Reuired for creating NullWS surface</param>
	/// <param name="window">Window handle</param>
	/// <param name="display">dusplay handle</param>
	/// <returns>Valid Surface object if success.</returns>
	Surface createSurface(const PhysicalDevice& physicalDevice, void* window, void* display);

	/// <summary>Get all enabled extensions names</summary>
	/// <returns>std::vector<const char*>&</returns>
	const std::vector<std::string>& getEnabledInstanceExtensions()
	{
		return _createInfo.enabledExtensionNames;
	}

	/// <summary>Get all enabled layers names</summary>
	/// <returns>std::vector<const char*>&</returns>
	const std::vector<const char*>& getEnabledInstanceLayers()
	{
		return _enabledInstanceLayers;
	}

	/// <summary>Check if extension is enabled</summary>
	/// <param name="extensionName">Extension name</param>
	/// <returns>Return true if it is enabled</returns>
	bool isInstanceExtensionEnabled(const char* extensionName)
	{
		for (uint32_t i = 0; i < _createInfo.enabledExtensionNames.size(); i++)
		{
			if (!strcmp(_createInfo.enabledExtensionNames[i].c_str(), extensionName))
			{
				return true;
			}
		}

		return false;
	}

	/// <summary>Check if layer is enabled</summary>
	/// <param name="layerName">Layer name</param>
	/// <returns>Return true if enabled</returns>
	bool isInstanceLayerEnabled(const char* layerName)
	{
		for (uint32_t i = 0; i < _enabledInstanceLayers.size(); i++)
		{
			if (!strcmp(_enabledInstanceLayers[i], layerName))
			{
				return true;
			}
		}
		return false;
	}

	/// <summary>Init debug call backs</summary>
	/// <returns>Return true if success</returns>
	bool initDebugCallbacks();

	/// <summary>Get number of physcial device available</summary>
	/// <returns>uint32_t</returns>
	uint32_t getNumPhysicalDevices()const
	{
		return static_cast<uint32_t>(_physicalDevice.size());
	}

private:
	friend class InstanceHelperFactory_;
	friend class ::pvrvk::EmbeddedRefCount<Instance_>;

	void destroyObject()
	{
		_surface.reset();
		_physicalDevice.clear();
		if (_instance != VK_NULL_HANDLE)
		{
#ifdef DEBUG
			if (_debugReportCallback && _supportsDebugReport)
			{
				vk::DestroyDebugReportCallbackEXT(_instance, _debugReportCallback, NULL);
			}
#endif

			vk::DestroyInstance(_instance, NULL);
			_instance = VK_NULL_HANDLE;
		}
	}

	static Instance createNew()
	{
		return EmbeddedRefCount<Instance_>::createNew();
	}

	bool init(const InstanceCreateInfo& instanceCreateInfo);

	Instance_() : _instance(VK_NULL_HANDLE), _supportsDebugReport(false) {}

	std::vector<PhysicalDevice> _physicalDevice;
	Surface _surface;
	VkInstance _instance;
	VkDebugReportCallbackEXT _debugReportCallback;
	bool _supportsDebugReport;
	std::vector<const char*> _enabledInstanceLayers;
	InstanceCreateInfo _createInfo;
};

inline Surface Instance_::createSurface(const PhysicalDevice& physicalDevice,
                                        void* window, void* display)
{
	if (_surface.isValid())
	{
		Log(LogLevel::Debug, "Render surface allready being created. Destroying the current surface and re-creating a new one");
	}
	_surface.construct();
	if (!_surface->init(getWeakReference(), physicalDevice, window, display))
	{
		_surface.reset();
	}
	return _surface;
}

inline Surface Instance_::getSurface()const
{
	return _surface;
}

inline PhysicalDevice Instance_::getPhysicalDevice(uint32_t id)
{
	return _physicalDevice[id];
}

inline const PhysicalDevice& Instance_::getPhysicalDevice(uint32_t id)const
{
	return _physicalDevice[id];
}

}

/// <summary>Create a PVRVk Instance</summary>
/// <param name="createInfo">The Create Info object for created Instance</param>
/// <returns>A newly created instance. In case of failure, null instance</returns>
Instance createInstance(const InstanceCreateInfo& createInfo);
}// namespace pvrvk

