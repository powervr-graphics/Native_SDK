/*!
\brief Acceleration structure wrappers to use with the Vulkan Ray Tracing extension.
\file PVRUtils/AccelerationStructure.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "PVRCore/glm.h"
#include "PVRVk/BufferVk.h"
#include "PVRVk/AccelerationStructureVk.h"
#include "PVRVk/CommandPoolVk.h"
#include "PVRUtils/Vulkan/HelperVk.h"
#include <PVRAssets/Model.h>

#pragma once
namespace pvr {
namespace utils {

/// <summary>Struct to store scene instance elements information, will be copied to a GPU buffer.</summary>
struct SceneDescription
{
	uint32_t modelIndex = 0; //!< Reference to the top level instance by index
	glm::mat4 transform = glm::mat4(1); //!< RTS (rotation, translation and scale) matrix of the instance
	glm::mat4 transformIT = glm::mat4(1); //!< Inverse transpose of RTS
};

/// <summary>Information about each scene element that can be ray traced at high level (instance).</summary>
struct RTInstance
{
	uint32_t modelIndex = 0; //!< Index of the corresponding bottom element in the bottom level acceleration structure
	uint32_t instanceId = 0; //!< Index of the instance, at shader level given by gl_InstanceID
	uint32_t hitGroupId = 0; //!< Index of the hit group
	uint32_t mask = 0xFF; //!< Visibility mask
	pvrvk::GeometryInstanceFlagsKHR flags = pvrvk::GeometryInstanceFlagsKHR::e_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR; //!< Ray Traced instance flags
	glm::mat4 transform = glm::mat4(1.0); //!< Instance RTS (rotation, translation and scale) matrix
};

/// <summary>Information per bottom level acceleration structure element.</summary>
struct RTModelInfo
{
	pvrvk::Buffer vertexBuffer; //!< Vertex buffer with the botom level geometry information
	pvrvk::Buffer indexBuffer; //!< Index buffer with the botom level geometry information
	uint32_t primitiveCount; //!< Amount of primitives this geometry has
	uint32_t vertexCount; //!< Amount of vertices in this geometry
	size_t vertexStride; //!< Stride for this geometry
};

/// <summary>Vertex format of the acceleration primitive elements, in the future this vertex information wil be customizable.</summary>
struct ASVertexFormat
{
	glm::vec3 pos; //!< Vertex position
	glm::vec3 nrm; //!< Vertex normal
	glm::vec2 texCoord; //!< Vertex texture coordinate
	glm::vec3 tangent; //!< Vertex tangent
};

/// <summary>A wrapper for an acceleration structure for the Vulkan Khronos Ray Tracing extension.</summary>
class AccelerationStructureWrapper
{
private:
	/// <summary>Top level acceleration structure.</summary>
	pvrvk::AccelerationStructure _tlas;

	/// <summary>Vector with all the bottom level acceleration structures used to generate the whole bottom acceleration structure.</summary>
	std::vector<pvrvk::AccelerationStructure> _blas;

	/// <summary>Bottom level information about the geometries in the acceleration structure representing scene elements.</summary>
	std::vector<RTModelInfo> _rtModelInfos;

	/// <summary>Top level information about the instances in the scene.</summary>
	std::vector<RTInstance> _instances;

	/// <summary>Top level information about the instances in the scene for the scnee descriptor buffer used.</summary>
	std::vector<SceneDescription> _sceneDescriptions;

public:
	/// <summary>Constructor.</summary>
	explicit AccelerationStructureWrapper() {}

	/// <summary>Fills the member variables _rtModelInfos, _instances and _sceneDescriptions used for the top level and bottom
	/// level acceleration structures needed.</summary>
	/// <param name="vertexBuffers">Array with a vertex buffer for each scene model.</param>
	/// <param name="indexBuffers">Array with an index buffer for each scene model.</param>
	/// <param name="verticesSize">Array with the amount of vertices of the geometry for this scene model.</param>
	/// <param name="indicesSize">Array with the amount of indices of the geometry for this scene model.</param>
	void buildASModelDescription(std::vector<pvrvk::Buffer> vertexBuffers, std::vector<pvrvk::Buffer> indexBuffers, std::vector<int> verticesSize, std::vector<int> indicesSize);

	/// <summary>Clear the information in _rtModelInfos, _instances and _sceneDescriptions filled in the call to buildASModelDescription once that
	/// information is no longer needed.</summary>
	void clearASModelDescriptionData();

	/// <summary>Build the acceleration structures _tlas and _blas (both the top and the bottom level ones).</summary>
	/// <param name="device">Device to build this acceleration structure for.</param>
	/// <param name="queue">Queue to submit commands.</param>
	/// <param name="buildASFlags">Build options for the acceleration structure. NOTE: Some flags are not implemented yet like e_ALLOW_COMPACTION_BIT_KHR, currently intended use
	/// is e_PREFER_FAST_TRACE_BIT_KHR.</param> <param name="commandBuffer">Command buffer to record commands submitted to the provided queue.</param>
	void buildAS(pvrvk::Device device, pvrvk::Queue queue, pvrvk::CommandBuffer commandBuffer,
		pvrvk::BuildAccelerationStructureFlagsKHR buildASFlags = pvrvk::BuildAccelerationStructureFlagsKHR::e_PREFER_FAST_TRACE_BIT_KHR);

	/// <summary>Build a bottom level acceleration structure for each scene element.</summary>
	/// <param name="device">Device to build the buffers for the bottom level scene elements being ray traced.</param>
	/// <param name="commandBuffer">Command buffer to record commands submitted to the provided queue.</param>
	/// <param name="queue">Queue to submit commands.</param>
	void buildBottomLevelASModels(pvrvk::Device device, pvrvk::CommandBuffer commandBuffer, pvrvk::Queue queue);

	/// <summary>Build the top level acceleration structure and the instances used for ray tracing.</summary>
	/// <param name="device">Device to build this top level acceration structure with.</param>
	/// <param name="commandBuffer">Command buffer to record commands submitted to the provided queue.</param>
	/// <param name="queue">Queue to submit commands.</param>
	/// <param name="flags">Flags for the acceleration struct to be built.</param>
	void buildTopLevelASAndInstances(pvrvk::Device device, pvrvk::CommandBuffer commandBuffer, pvrvk::Queue queue, pvrvk::BuildAccelerationStructureFlagsKHR flags);

	/// <summary>Helper function to convert information from elements in _instances to VkAccelerationStructureInstanceKHR equivalents.</summary>
	/// <param name="device">Device needed for the buffer address for the instance.</param>
	/// <param name="geometryInstances">Vector where to store the VkAccelerationStructureInstanceKHR elements generated.</param>
	void setupGeometryInstances(pvrvk::Device device, std::vector<VkAccelerationStructureInstanceKHR>& geometryInstances);

	/// <summary>Get the top level acceleration structure.</summary>
	/// <returns>The top level information about the instances in the scene for the scene descriptor buffer used.</returns>
	inline pvrvk::AccelerationStructure getTopLevelAccelerationStructure() { return _tlas; }

	/// <summary>Get the top level information about the instances in the scene for the scene descriptor buffer used.</summary>
	/// <returns>The top level information about the instances in the scene for the scene descriptor buffer used.</returns>
	inline std::vector<SceneDescription>& getSceneDescriptions() { return _sceneDescriptions; }

	/// <summary>Get the array with the bottom level acceleration structures.</summary>
	/// <returns>The array with the bottom level acceleration structures.</returns>
	inline std::vector<pvrvk::AccelerationStructure>& getBlas() { return _blas; }
};

} // namespace utils
} // namespace pvr
