/*!
\brief Function implementations for the DebugMarkerImpl class
\file PVRVk/DebugMarkerVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRVk/DebugMarkerVk.h"
#include "PVRVk/DeviceVk.h"

/// <summary>Main PowerVR Framework Namespace</summary>
namespace pvrvk {
/// <summary>Contains internal objects and wrapped versions of the PVRVk module</summary>
namespace impl {
void DebugMarkerImpl::setObjectName(const Device_& device, uint64_t vkHandle, const std::string& objectName)
{
	assertion(device.getVkHandle() != VK_NULL_HANDLE);
	assertion(vkHandle != VK_NULL_HANDLE);
	if (this->hasName())
	{
		Log(LogLevel::Debug, "Changing object name from: '%s' to: '%s'", _objectName.c_str(), objectName.c_str());
	}
	_objectName = objectName;
	// if the extension is supported then set the object name
	if (device.isExtensionEnabled(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
	{
		VkDebugMarkerObjectNameInfoEXT objectNameInfo = {};
		objectNameInfo.sType = static_cast<VkStructureType>(StructureType::e_DEBUG_MARKER_OBJECT_NAME_INFO_EXT);
		// The VK_DEBUG_REPORT_OBJECT_TYPE type of the object to be named
		objectNameInfo.objectType = static_cast<VkDebugReportObjectTypeEXT>(_objectType);
		// The actual object handle of the object to name
		objectNameInfo.object = vkHandle;
		// The name to use for the object
		objectNameInfo.pObjectName = _objectName.c_str();

		vkThrowIfFailed(device.getVkBindings().vkDebugMarkerSetObjectNameEXT(device.getVkHandle(), &objectNameInfo), "Failed to set ObjectName with vkDebugMarkerSetObjectNameEXT");
	}
	// otherwise we return success
}

/// <summary>Makes use of the extension VK_EXT_debug_marker to provide a tag for a specified object.</summary>
/// <param name="tagName">A numerical identifier of the tag for the object</param>
/// <param name="tagSize">The number of bytes of data to attach to the object.</param>
/// <param name="tag">An array of tagSize bytes containing the data to be associated with the object.</param>
/// <returns>VkResult specifying the result of the call to DebugMarkerSetObjectNameEXT</returns>
void DebugMarkerImpl::setObjectTag(const Device_& device, uint64_t vkHandle, uint64_t tagName, size_t tagSize, const void* tag)
{
	assertion(device.getVkHandle() != VK_NULL_HANDLE);
	assertion(vkHandle != VK_NULL_HANDLE);
	if (device.isExtensionEnabled(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
	{
		VkDebugMarkerObjectTagInfoEXT objectTagInfo = {};
		objectTagInfo.sType = static_cast<VkStructureType>(StructureType::e_DEBUG_MARKER_OBJECT_TAG_INFO_EXT);
		// The VK_DEBUG_REPORT_OBJECT_TYPE type of the object to be tagged
		objectTagInfo.objectType = static_cast<VkDebugReportObjectTypeEXT>(_objectType);
		// The actual object handle of the object to tag
		objectTagInfo.object = vkHandle;
		// The tag name to use for the object
		objectTagInfo.tagName = tagName;
		// The number of bytes of data to attach to the object
		objectTagInfo.tagSize = tagSize;
		// An array of tagSize bytes containing the data to be associated with the object
		objectTagInfo.pTag = tag;
		vkThrowIfFailed(device.getVkBindings().vkDebugMarkerSetObjectTagEXT(device.getVkHandle(), &objectTagInfo), "Failed to set ObjectTag with vkDebugMarkerSetObjectTagEXT");
	}
}
} // namespace impl
} // namespace pvrvk
//!\endcond
