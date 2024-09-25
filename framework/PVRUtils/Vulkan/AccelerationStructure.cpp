/*!
\brief Contains Vulkan-specific utilities to facilitate Physically Based Rendering tasks, such as generating irradiance maps and BRDF lookup tables.
\file PVRUtils/Vulkan/PBRUtilsVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

//!\cond NO_DOXYGEN
#include "PVRUtils/Vulkan/AccelerationStructure.h"

namespace pvr {
namespace utils {

void AccelerationStructureWrapper::buildASModelDescription(std::vector<pvrvk::Buffer> vertexBuffers, std::vector<pvrvk::Buffer> indexBuffers, std::vector<int> verticesSize,
	std::vector<int> indicesSize, const std::vector<glm::mat4>& vectorInstanceTransform)
{
	for (uint32_t i = 0; i < vertexBuffers.size(); i++)
	{
		_rtModelInfos.push_back(RTModelInfo{ vertexBuffers[i], indexBuffers[i], static_cast<uint32_t>(indicesSize[i] / 3 + (indicesSize[i] % 3 == 0 ? 0 : 1)),
			static_cast<uint32_t>(verticesSize[i]), sizeof(ASVertexFormat) });

		_instances.push_back(RTInstance{ i, i, 0, 0xFF, pvrvk::GeometryInstanceFlagsKHR::e_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR, glm::mat4(vectorInstanceTransform[i]) });

		_sceneDescriptions.push_back(SceneDescription{ i, glm::mat4(vectorInstanceTransform[i]), glm::transpose(glm::inverse(glm::mat4(vectorInstanceTransform[i]))) });
	}
}

void AccelerationStructureWrapper::clearASModelDescriptionData()
{
	_rtModelInfos.clear();
	_instances.clear();
	_sceneDescriptions.clear();
}

void AccelerationStructureWrapper::buildAS(pvrvk::Device device, pvrvk::Queue queue, pvrvk::CommandBuffer commandBuffer, pvrvk::BuildAccelerationStructureFlagsKHR buildASFlags)
{
	buildBottomLevelASModels(device, commandBuffer, queue);
	buildTopLevelASAndInstances(device, commandBuffer, queue, buildASFlags, false);
}

void AccelerationStructureWrapper::buildBottomLevelASModels(pvrvk::Device device, pvrvk::CommandBuffer commandBuffer, pvrvk::Queue queue)
{
	_blas.resize(_rtModelInfos.size());

	VkDeviceSize maximumScratchSize = 0;

	std::vector<VkAccelerationStructureGeometryKHR> vectorASGeometry(_rtModelInfos.size());
	std::vector<VkAccelerationStructureBuildGeometryInfoKHR> vectorASBuildGeometryInfo(_rtModelInfos.size());

	for (int i = 0; i < _rtModelInfos.size(); ++i)
	{
		VkDeviceAddress vertexBufferAddress = _rtModelInfos[i].vertexBuffer->getDeviceAddress(device);
		VkDeviceAddress indexBufferAddress = _rtModelInfos[i].indexBuffer->getDeviceAddress(device);

		VkAccelerationStructureGeometryTrianglesDataKHR triangleGeometryData = { static_cast<VkStructureType>(pvrvk::StructureType::e_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR) };
		triangleGeometryData.vertexFormat = static_cast<VkFormat>(pvrvk::Format::e_R32G32B32_SFLOAT);
		triangleGeometryData.vertexData = VkDeviceOrHostAddressConstKHR{ vertexBufferAddress };
		triangleGeometryData.vertexStride = static_cast<VkDeviceSize>(_rtModelInfos[i].vertexStride);
		triangleGeometryData.indexType = static_cast<VkIndexType>(pvrvk::IndexType::e_UINT32);
		triangleGeometryData.indexData = VkDeviceOrHostAddressConstKHR{ indexBufferAddress };
		triangleGeometryData.maxVertex = _rtModelInfos[i].vertexCount - 1;
		triangleGeometryData.transformData = {};

		vectorASGeometry[i] = { static_cast<VkStructureType>(pvrvk::StructureType::e_ACCELERATION_STRUCTURE_GEOMETRY_KHR) };
		vectorASGeometry[i].geometryType = static_cast<VkGeometryTypeKHR>(pvrvk::GeometryTypeKHR::e_TRIANGLES_KHR);
		vectorASGeometry[i].flags = static_cast<VkGeometryFlagBitsKHR>(pvrvk::GeometryFlagsKHR::e_OPAQUE_BIT_KHR);
		vectorASGeometry[i].geometry.triangles = triangleGeometryData;

		vectorASBuildGeometryInfo[i] = {};
		vectorASBuildGeometryInfo[i].flags = static_cast<VkBuildAccelerationStructureFlagBitsKHR>(pvrvk::BuildAccelerationStructureFlagsKHR::e_PREFER_FAST_TRACE_BIT_KHR);
		vectorASBuildGeometryInfo[i].sType = static_cast<VkStructureType>(pvrvk::StructureType::e_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR);
		vectorASBuildGeometryInfo[i].geometryCount = 1;
		vectorASBuildGeometryInfo[i].pGeometries = &vectorASGeometry[i];
		vectorASBuildGeometryInfo[i].mode = static_cast<VkBuildAccelerationStructureModeKHR>(pvrvk::BuildAccelerationStructureModeKHR::e_BUILD_KHR);
		vectorASBuildGeometryInfo[i].type = static_cast<VkAccelerationStructureTypeKHR>(pvrvk::AccelerationStructureTypeKHR::e_BOTTOM_LEVEL_KHR);
		vectorASBuildGeometryInfo[i].srcAccelerationStructure = VK_NULL_HANDLE;

		std::vector<uint32_t> maxPrimCount = { _rtModelInfos[i].primitiveCount };
		VkAccelerationStructureBuildSizesInfoKHR asBuildSizesInfo{ static_cast<VkStructureType>(pvrvk::StructureType::e_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR) };
		device->getVkBindings().vkGetAccelerationStructureBuildSizesKHR(device->getVkHandle(),
			static_cast<VkAccelerationStructureBuildTypeKHR>(pvrvk::AccelerationStructureBuildTypeKHR::e_DEVICE_KHR), &vectorASBuildGeometryInfo[i], maxPrimCount.data(),
			&asBuildSizesInfo);

		pvrvk::Buffer blasBuffer = pvr::utils::createBuffer(device,
			pvrvk::BufferCreateInfo(asBuildSizesInfo.accelerationStructureSize,
				pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT | pvrvk::BufferUsageFlags::e_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE,
			pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT);

		pvrvk::AccelerationStructureCreateInfo asCreateInfo;
		asCreateInfo.setType(pvrvk::AccelerationStructureTypeKHR::e_BOTTOM_LEVEL_KHR);
		asCreateInfo.setSize(asBuildSizesInfo.accelerationStructureSize); // Will be used to allocate memory.
		asCreateInfo.setBuffer(blasBuffer->getVkHandle());

		_blas[i] = device->createAccelerationStructure(asCreateInfo, blasBuffer);
		_blas[i]->setFlags(pvrvk::BuildAccelerationStructureFlagsKHR::e_NONE);

		maximumScratchSize = std::max(maximumScratchSize, asBuildSizesInfo.buildScratchSize);

		vectorASBuildGeometryInfo[i].dstAccelerationStructure = _blas[i]->getVkHandle();
	}

	// A scratch buffer with the size of the biggest bottom level acceleration structure geometry element needs to be built and provided
	// to build the bottom level acceleration structure.
	pvrvk::Buffer scratchBuffer = pvr::utils::createBuffer(device,
		pvrvk::BufferCreateInfo(maximumScratchSize, pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT | pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE,
		pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT);

	// Get the address of the scratch buffer.
	VkDeviceAddress scratchAddress = scratchBuffer->getDeviceAddress(device);

	commandBuffer->begin();

	for (int i = 0; i < _rtModelInfos.size(); ++i)
	{
		// The bottom level acceleration structure gets the handle to the bottom level acceleration structure, the
		// vector of AS structure geometries in _blas::_geometry and the address of the scratch buffer.
		vectorASBuildGeometryInfo[i].scratchData.deviceAddress = scratchAddress;

		// Information describing each acceleration structure geometry.
		VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildOffset = { _rtModelInfos[i].primitiveCount, 0, 0, 0 };
		std::vector<const VkAccelerationStructureBuildRangeInfoKHR*> vectorAccelerationStructureBuildOffset(1);
		vectorAccelerationStructureBuildOffset[0] = &accelerationStructureBuildOffset;

		device->getVkBindings().vkCmdBuildAccelerationStructuresKHR(commandBuffer->getVkHandle(), 1, &vectorASBuildGeometryInfo[i], vectorAccelerationStructureBuildOffset.data());

		pvrvk::MemoryBarrierSet barriers;
		barriers.addBarrier(pvrvk::MemoryBarrier(pvrvk::AccessFlags::e_ACCELERATION_STRUCTURE_WRITE_BIT_KHR, pvrvk::AccessFlags::e_ACCELERATION_STRUCTURE_READ_BIT_NV));
		commandBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, pvrvk::PipelineStageFlags::e_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, barriers);
	}

	commandBuffer->end();
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &commandBuffer;
	submitInfo.numCommandBuffers = 1;
	queue->submit(&submitInfo, 1);
	queue->waitIdle();
	commandBuffer->reset();
}

void AccelerationStructureWrapper::buildTopLevelASAndInstances(
	pvrvk::Device device, pvrvk::CommandBuffer commandBuffer, pvrvk::Queue queue, pvrvk::BuildAccelerationStructureFlagsKHR flags, bool update)
{
	// Now, build the information needed by the top level acceleration structure, which is, for each scene element, it's transform and some flags
	std::vector<VkAccelerationStructureInstanceKHR> geometryInstances;
	geometryInstances.reserve(_instances.size()); // Just a single instance is considered for the single acceleration structure geometry which is the triangle to be ray traced
	setupGeometryInstances(device, geometryInstances);

	commandBuffer->begin();

	if (!update)
	{
		// The instance information is put in a buffer
		_instancesBuffer = pvr::utils::createBuffer(device,
			pvrvk::BufferCreateInfo(sizeof(VkAccelerationStructureInstanceKHR) * geometryInstances.size(),
				pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT |
					pvrvk::BufferUsageFlags::e_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE,
			pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT);
	}

	pvr::utils::updateBufferUsingStagingBuffer(
		device, _instancesBuffer, commandBuffer, geometryInstances.data(), 0, sizeof(VkAccelerationStructureInstanceKHR) * geometryInstances.size());

	// As with the scratch buffer, the address of the instance buffer is retrieved and will be used to build the top level acceleration structure.
	VkDeviceAddress instanceBufferAddress = _instancesBuffer->getDeviceAddress(device);

	pvrvk::MemoryBarrierSet barriers;
	barriers.addBarrier(pvrvk::MemoryBarrier(pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT, pvrvk::AccessFlags::e_ACCELERATION_STRUCTURE_WRITE_BIT_KHR));
	commandBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_TRANSFER_BIT, pvrvk::PipelineStageFlags::e_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, barriers);

	VkAccelerationStructureGeometryInstancesDataKHR geometry{ static_cast<VkStructureType>(pvrvk::StructureType::e_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR) };
	geometry.arrayOfPointers = VK_FALSE;
	geometry.data.deviceAddress = instanceBufferAddress;

	VkAccelerationStructureGeometryKHR accelerationStructureGeometryTopLevel{ static_cast<VkStructureType>(pvrvk::StructureType::e_ACCELERATION_STRUCTURE_GEOMETRY_KHR) };
	accelerationStructureGeometryTopLevel.geometryType = static_cast<VkGeometryTypeKHR>(pvrvk::GeometryTypeKHR::e_INSTANCES_KHR);
	accelerationStructureGeometryTopLevel.geometry.instances = geometry;

	// The top level acceleration structure has the handle to the TLAS, the address to the instances buffer, and the address to the scratch buffer.
	// In this case, only one instance will be ray traced, which is a triangle.
	VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryTopLevel{ static_cast<VkStructureType>(
		pvrvk::StructureType::e_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR) };
	accelerationStructureBuildGeometryTopLevel.flags = static_cast<VkBuildAccelerationStructureFlagsKHR>(flags);
	accelerationStructureBuildGeometryTopLevel.geometryCount = 1;
	accelerationStructureBuildGeometryTopLevel.pGeometries = &accelerationStructureGeometryTopLevel;
	accelerationStructureBuildGeometryTopLevel.mode = update ? static_cast<VkBuildAccelerationStructureModeKHR>(pvrvk::BuildAccelerationStructureModeKHR::e_UPDATE_KHR)
															 : static_cast<VkBuildAccelerationStructureModeKHR>(pvrvk::BuildAccelerationStructureModeKHR::e_BUILD_KHR);
	accelerationStructureBuildGeometryTopLevel.type = static_cast<VkAccelerationStructureTypeKHR>(pvrvk::AccelerationStructureTypeKHR::e_TOP_LEVEL_KHR);
	accelerationStructureBuildGeometryTopLevel.srcAccelerationStructure = update ? _tlas->getVkHandle() : VK_NULL_HANDLE;

	uint32_t count = uint32_t(_instances.size());
	VkAccelerationStructureBuildSizesInfoKHR asBuildSizesInfo{ static_cast<VkStructureType>(pvrvk::StructureType::e_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR) };
	device->getVkBindings().vkGetAccelerationStructureBuildSizesKHR(device->getVkHandle(),
		static_cast<VkAccelerationStructureBuildTypeKHR>(pvrvk::AccelerationStructureBuildTypeKHR::e_DEVICE_KHR), &accelerationStructureBuildGeometryTopLevel, &count,
		&asBuildSizesInfo);

	if (!update)
	{
		pvrvk::AccelerationStructureCreateInfo createInfo;
		createInfo.setType(pvrvk::AccelerationStructureTypeKHR::e_TOP_LEVEL_KHR);
		createInfo.setSize(asBuildSizesInfo.accelerationStructureSize);

		pvrvk::Buffer asBuffer = pvr::utils::createBuffer(device,
			pvrvk::BufferCreateInfo(asBuildSizesInfo.accelerationStructureSize,
				pvrvk::BufferUsageFlags::e_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			pvrvk::MemoryPropertyFlags::e_NONE, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE, pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT);

		createInfo.setBuffer(asBuffer->getVkHandle());

		_tlas = device->createAccelerationStructure(createInfo, asBuffer);
		_tlas->setAccelerationStructureBuffer(asBuffer);
	}

	pvrvk::Buffer scratchBuffer = pvr::utils::createBuffer(device,
		pvrvk::BufferCreateInfo(asBuildSizesInfo.buildScratchSize,
			pvrvk::BufferUsageFlags::e_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | pvrvk::BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT |
				pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		pvrvk::MemoryPropertyFlags::e_NONE, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE, pvrvk::MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT);

	VkDeviceAddress scratchAddress = scratchBuffer->getDeviceAddress(device);

	// Update build information
	accelerationStructureBuildGeometryTopLevel.dstAccelerationStructure = _tlas->getVkHandle();
	accelerationStructureBuildGeometryTopLevel.scratchData.deviceAddress = scratchAddress;

	// Build Offsets info: number of instances
	VkAccelerationStructureBuildRangeInfoKHR buildOffsetInfo{ static_cast<uint32_t>(_instances.size()), 0, 0, 0 };
	const VkAccelerationStructureBuildRangeInfoKHR* pBuildOffsetInfo = &buildOffsetInfo;

	device->getVkBindings().vkCmdBuildAccelerationStructuresKHR(commandBuffer->getVkHandle(), 1, &accelerationStructureBuildGeometryTopLevel, &pBuildOffsetInfo);

	commandBuffer->end();
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &commandBuffer;
	submitInfo.numCommandBuffers = 1;
	queue->submit(&submitInfo, 1);
	queue->waitIdle();
	commandBuffer->reset();
}

void AccelerationStructureWrapper::setupGeometryInstances(pvrvk::Device device, std::vector<VkAccelerationStructureInstanceKHR>& geometryInstances)
{
	for (const auto& inst : _instances)
	{
		// Retrieve the address of this bottom level acceleration structure, needed as one of the fields in the struct to be added to geometryInstances
		VkDeviceAddress bottomLevelASAddress = _blas[inst.modelIndex]->getAccelerationStructureDeviceAddress(device);

		// The information for each scene element, expressed through an instance, is added here to geometryInstances. Since only one triangle is
		// present in the scene, the information from _deviceResources::_instance is added to geometryInstances.
		VkAccelerationStructureInstanceKHR accelerationStructureInstance{};
		glm::mat4 transp = glm::transpose(inst.transform);
		memcpy(&accelerationStructureInstance.transform, &transp, sizeof(accelerationStructureInstance.transform));

		accelerationStructureInstance.instanceCustomIndex = inst.instanceId;
		accelerationStructureInstance.mask = inst.mask;
		accelerationStructureInstance.instanceShaderBindingTableRecordOffset = inst.hitGroupId;
		accelerationStructureInstance.flags = static_cast<VkGeometryInstanceFlagsKHR>(inst.flags);
		accelerationStructureInstance.accelerationStructureReference = bottomLevelASAddress;
		geometryInstances.push_back(accelerationStructureInstance);
	}
}

void AccelerationStructureWrapper::updateInstanceTransformData(const std::vector<glm::mat4>& vectorTransform)
{
	assert(_instances.size() == vectorTransform.size());
	for (int i = 0; i < _instances.size(); ++i)
	{
		_instances[i].transform = vectorTransform[i];
		_sceneDescriptions[i].transform = vectorTransform[i];
		_sceneDescriptions[i].transformIT = glm::transpose(glm::inverse(vectorTransform[i]));
	}
}

} // namespace utils
} // namespace pvr
