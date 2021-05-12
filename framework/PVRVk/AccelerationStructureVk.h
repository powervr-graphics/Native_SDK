/*!
\brief Wraps a VkAccelerationStructureKHR Vulkan object from the Vulkan Ray Tracing extension, can be used to build top and bottom level acceleration structures.
\file PVRVk/AccelerationStructureVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/DeviceVk.h"
#include "PVRVk/DeviceMemoryVk.h"

namespace pvrvk {

struct AccelerationStructureCreateGeometryTypeInfo
{
private:
	GeometryTypeKHR _geometryType;
	uint32_t _maxPrimitiveCount;
	IndexType _indexType;
	uint32_t _maxVertexCount;
	Format _vertexFormat;
	Bool32 _allowsTransforms;

public:
	/// <summary>Constructor (zero initialization)</summary>
	AccelerationStructureCreateGeometryTypeInfo()
		: _geometryType(GeometryTypeKHR::e_TRIANGLES_KHR), _maxPrimitiveCount(0), _indexType(IndexType::e_UINT32), _maxVertexCount(0), _vertexFormat(Format::e_UNDEFINED),
		  _allowsTransforms(false)
	{}

	/// <summary>Constructor</summary>
	/// <param name="size">The buffer creation size</param>
	/// <param name="usageFlags">The buffer creation usage flags</param>
	/// <param name="flags">The buffer creation flags</param>
	/// <param name="sharingMode">The buffer creation sharing mode</param>
	/// <param name="queueFamilyIndices">A pointer to a list of supported queue families</param>
	/// <param name="numQueueFamilyIndices">The number of supported queue family indices</param>
	AccelerationStructureCreateGeometryTypeInfo(
		GeometryTypeKHR geometryType, uint32_t maxPrimitiveCount, IndexType indexType, uint32_t maxVertexCount, Format vertexFormat, Bool32 allowsTransforms)
		: _geometryType(geometryType), _maxPrimitiveCount(maxPrimitiveCount), _indexType(indexType), _maxVertexCount(maxVertexCount), _vertexFormat(vertexFormat),
		  _allowsTransforms(allowsTransforms)
	{}

	inline GeometryTypeKHR getGeometryType() const { return _geometryType; }
	inline void setGeometryType(GeometryTypeKHR geometryType) { this->_geometryType = geometryType; }
	inline uint32_t getMaxPrimitiveCount() const { return _maxPrimitiveCount; }
	inline void setMaxPrimitiveCount(uint32_t maxPrimitiveCount) { this->_maxPrimitiveCount = maxPrimitiveCount; }
	inline IndexType getIndexType() const { return _indexType; }
	inline void setIndexType(IndexType indexType) { this->_indexType = indexType; }
	inline uint32_t getMaxVertexCount() const { return _maxVertexCount; }
	inline void setMaxVertexCount(uint32_t maxVertexCount) { this->_maxVertexCount = maxVertexCount; }
	inline Format getVertexFormat() const { return _vertexFormat; }
	inline void setVertexFormat(Format vertexFormat) { this->_vertexFormat = vertexFormat; }
	inline Bool32 getAllowsTransforms() const { return _allowsTransforms; }
	inline void setAllowsTransforms(Bool32 allowsTransforms) { this->_allowsTransforms = allowsTransforms; }
};

struct AccelerationStructureCreateInfo
{
public:
	/// <summary>Constructor (zero initialization)</summary>
	AccelerationStructureCreateInfo()
		: _sType(StructureType::e_ACCELERATION_STRUCTURE_CREATE_INFO_KHR), _pNext(nullptr), _createFlags(0), _buffer(VK_NULL_HANDLE), _offset(0), _size(0),
		  _type(AccelerationStructureTypeKHR::e_MAX_ENUM), _deviceAddress(0)
	{}

	inline StructureType getSType() const { return _sType; }
	inline void setSType(StructureType sType) { _sType = sType; }

	inline VkAccelerationStructureCreateFlagsKHR getCreateFlags() const { return _createFlags; }
	inline void setCompactedSize(VkAccelerationStructureCreateFlagsKHR createFlags) { _createFlags = createFlags; }

	inline VkBuffer getBuffer() const { return _buffer; }
	inline void setBuffer(VkBuffer buffer) { _buffer = buffer; }

	inline VkDeviceSize getOffset() const { return _offset; }
	inline void setOffset(VkDeviceSize offset) { _offset = offset; }

	inline VkDeviceSize getSize() const { return _size; }
	inline void setSize(VkDeviceSize size) { _size = size; }

	inline AccelerationStructureTypeKHR getType() const { return _type; }
	inline void setType(AccelerationStructureTypeKHR type) { _type = type; }

	inline VkDeviceAddress getDeviceAddress() const { return _deviceAddress; }
	inline void setDeviceAddress(VkDeviceAddress deviceAddress) { _deviceAddress = deviceAddress; }

	inline const void* getpNext() const { return _pNext; }
	inline void setpNext(const void* pNext) { _pNext = pNext; }

private:
	StructureType _sType;
	const void* _pNext;
	VkAccelerationStructureCreateFlagsKHR _createFlags;
	VkBuffer _buffer;
	VkDeviceSize _offset;
	VkDeviceSize _size;
	AccelerationStructureTypeKHR _type;
	VkDeviceAddress _deviceAddress = 0;
};

namespace impl {
/// <summary>Top level acceleration structure implementation for the Vulkan Ray Tracing extension</summary>
class AccelerationStructure_ : public PVRVkDeviceObjectBase<VkAccelerationStructureKHR, ObjectType::e_ACCELERATION_STRUCTURE_KHR>, public DeviceObjectDebugUtils<AccelerationStructure_>
{
private:
	friend class Device_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class AccelerationStructure_;
	};

	static AccelerationStructure constructShared(const DeviceWeakPtr& device, const AccelerationStructureCreateInfo& createInfo, pvrvk::Buffer asBuffer)
	{
		return std::make_shared<AccelerationStructure_>(make_shared_enabler{}, device, createInfo, asBuffer);
	}

	/// <summary>Buffer sused to build the acceleration structure</summary>
	pvrvk::Buffer _asBuffer;

	/// <summary>Flags used during the aceeleration structure building</summary>
	pvrvk::BuildAccelerationStructureFlagsKHR _flags;

	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(AccelerationStructure_)

public:
	/// <summary>Destructor. Checks if the device is valid</summary>
	~AccelerationStructure_();
	//!\endcond

	/// <summary>Constructor for instances of this class, will call buildAcelerationStructure with the createInfo parammeter</summary>
	/// <param name="make_shared_enabler">Shared pointer helper.</param>
	/// <param name="device">The device used to build this acceleration structure.</param>
	/// <param name="createInfo">The information used to build this acceleration structure.</param>
	/// <param name="asBuffer">Acceleration Structure buffer built previously.</param>
	AccelerationStructure_(make_shared_enabler, const DeviceWeakPtr& device, const AccelerationStructureCreateInfo& createInfo, pvrvk::Buffer asBuffer);

	/// <summary>Get the acceleration structure buffer used to build the acceleration structure</summary>
	/// <returns>The acceleration structure buffer used to build the acceleration structure</returns>
	inline const pvrvk::Buffer getAccelerationStructureBuffer() const { return _asBuffer; }

	/// <summary>Get the acceleration structure buffer used to build the acceleration structure</summary>
	/// <param name="deviceMemory">The acceleration structure buffer used to build the acceleration structure</param>
	inline const void setAccelerationStructureBuffer(pvrvk::Buffer asBuffer) { _asBuffer = asBuffer; }

	/// <summary>Get flags used for the acceleration structure building</summary>
	/// <returns>The flags used for the acceleration structure building</returns>
	inline const pvrvk::BuildAccelerationStructureFlagsKHR getFlags() const { return _flags; }

	/// <summary>Set flags used for the acceleration structure building</summary>
	/// <param name="flags">The flags used for the acceleration structure building</param>
	inline const void setFlags(pvrvk::BuildAccelerationStructureFlagsKHR flags) { _flags = flags; }

	/// <summary>Retrieve the address of this acceleration structure</summary>
	/// <param name="device">Device this buffer was built with.</param>
	/// <returns>The device address of the acceleration structure given as parameter.</returns>
	VkDeviceAddress getAccelerationStructureDeviceAddress(const Device device)
	{
		VkAccelerationStructureDeviceAddressInfoKHR accelerationStructureDeviceAddress{
			static_cast<VkStructureType>(pvrvk::StructureType::e_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR), nullptr, getVkHandle()
		};

		return device->getVkBindings().vkGetAccelerationStructureDeviceAddressKHR(device->getVkHandle(), &accelerationStructureDeviceAddress);
	}
};

} // namespace impl
} // namespace pvrvk
