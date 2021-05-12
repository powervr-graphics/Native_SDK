#include "PVRVk/AccelerationStructureVk.h"
#include "PVRVk/BufferVk.h"

namespace pvrvk {
namespace impl {

AccelerationStructure_::AccelerationStructure_(make_shared_enabler, const DeviceWeakPtr& device, const AccelerationStructureCreateInfo& createInfo, pvrvk::Buffer asBuffer)
	: PVRVkDeviceObjectBase(device), DeviceObjectDebugUtils(), _flags(pvrvk::BuildAccelerationStructureFlagsKHR::e_NONE)
{
	Device deviceSharedPtr = device.lock();

	_asBuffer = asBuffer;

	VkAccelerationStructureCreateInfoKHR vkCreateInfo = {};
	vkCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_ACCELERATION_STRUCTURE_CREATE_INFO_KHR);
	vkCreateInfo.buffer = _asBuffer->getVkHandle();
	vkCreateInfo.size = createInfo.getSize();
	vkCreateInfo.type = static_cast<VkAccelerationStructureTypeKHR>(createInfo.getType());

	vkThrowIfFailed(static_cast<Result>(deviceSharedPtr->getVkBindings().vkCreateAccelerationStructureKHR(deviceSharedPtr->getVkHandle(), &vkCreateInfo, nullptr, &_vkHandle)),
		"Failed to create Acceleration Structure");
}

AccelerationStructure_::~AccelerationStructure_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (!_device.expired())
		{
			getDevice()->getVkBindings().vkDestroyAccelerationStructureKHR(getDevice()->getVkHandle(), getVkHandle(), NULL);
			_vkHandle = VK_NULL_HANDLE;
		}
		else
		{
			reportDestroyedAfterDevice();
		}
	}
}

} // namespace impl
} // namespace pvrvk
