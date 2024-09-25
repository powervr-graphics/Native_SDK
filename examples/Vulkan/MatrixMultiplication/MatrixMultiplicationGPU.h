/*!*********************************************************************************************************************
\File			MatrixMultiplicationGPU.h
\Title			Vulkan Compute implementation of the different multiplication strategies
\Author			PowerVR by Imagination, Developer Technology Team.
\Copyright		Copyright(c) Imagination Technologies Limited.
\brief			Covers the GPU and vulkan side of implementing the benchmark
***********************************************************************************************************************/
#pragma once
#include "PVRUtils/PVRUtilsVk.h"
#include "Matrix.h"

// The compute piplines run sequentially, therefore they can share the same resources
struct DeviceResources
{
	// vulkan instance objects
	pvrvk::Instance instance;
	pvrvk::Device device;
	pvr::utils::DebugUtilsCallbacks debugUtilCallbacks;
	pvr::utils::vma::Allocator vma;

	// vulkan command queue objects
	pvrvk::Queue commandQueue;
	pvrvk::CommandPool commandPool;
	pvrvk::CommandBuffer primaryCommandBuffer;

	// vulkan descriptor sets objects
	pvrvk::DescriptorPool descriptorPool;
	pvrvk::DescriptorSetLayout descriptorSetLayout;
	pvrvk::DescriptorSet descriptorSet;

	// vulkan compute pipeline objects
	// only require one layout as all shaders have the same structure
	// one compute pipeline per test
	pvrvk::PipelineLayout pipelineLayout;
	pvrvk::ComputePipeline computePipeline;

	// Objects to store information about the matrices
	const uint32_t matrixBufferCount = 8;
	// Buffers on the Device, storing a series of transposed versions of the matricies A,B,C
	pvrvk::Buffer matrixBufferSSBOs[8];
	// Mapped memory views of those buffers, so that they can be read and flushed.
	pvr::utils::StructuredBufferView matrixBufferViews[8];

	~DeviceResources()
	{
		if (device) { device->waitIdle(); }
	}
};

/// <summary>
/// Creates the device resources struct and then begins instancing the vulkan variables
/// </summary>
void initiateVulkan(char* pathToExecutable);

/// <summary>
/// Creates the descriptor sets that the pipeline will use to find and use the buffers
/// </summary>
void makeDescriptors();

/// <summary>
/// Allocates the buffers used to send the matrixies to, it would be a good idea to create the matrices before calling this so that their sizes are known and fixed.
/// Must be called before creating a pipeline as it locally stores the matrix dimensions to pass to the shader
/// </summary>
void makeBuffers(uint32_t N, uint32_t M, uint32_t P);

/// <summary>
/// Creates the layout for a pipeline, since they all have the same layout this only needs to be called once,
/// </summary>
void makePipelineLayout();

/// <summary>
/// Creates one pipeline based on the shader at the specified index. The majority of shaders can construct all their defines from the workgroup sizes, so the extra variable is set to a default,
///	this is used for rectangular tiling shaders which need this extra info.
/// </summary>
/// <param name="shaderIndex">The index of the pipeline to be created</param>
/// <param name="xWorkgroupSize">The width of the local workgroups (Tile sizes are reconstructed fom the relationship between this and the matrix sizes)</param>
/// <param name="yWorkgroupSize">The height of the local workgroups(Tile sizes are reconstructed fom the relationship between this and the matrix sizes)</param>
/// <param name="nTileSize">The tile size in the n dimension, this can't be reconstructed from the workgroup sizes</param>
void makePipeline(int shaderIndex, int xWorkgroupSize, int yWorkgroupSize, int nTileSize = 0);

/// <summary>
/// Updates the contents of the buffers, they have to be allocated first.
/// </summary>
void updateBuffers(Matrix LHS, Matrix RHS);

/// <summary>
/// Fetches the result of the compute chader caluclation
/// </summary>
/// <param name="transposed">If the shader stores this multiplication in CT</param>
/// <returns>A matrix holding the product of AB, the result will always be AB</returns>
Matrix fetchResult(bool transposed);

/// <summary>
/// Empties the results buffer so that a correct answer from a previous test does not influence the next test
/// </summary>
void emptyResultBuffers();

/// <summary>
/// Launches the compute pipline created by the most recently compiled shader, and places the result of the multiplication in the mapped memory view
/// </summary>
/// <param name="xWorkgroupNumber">The number of horizontal workgroups that are dispatched </param>
/// <param name="yWorkgroupNumber">The number of vertical workgroups that are dispatched </param>
void doComputeWork(int xWorkgroupNumber, int yWorkgroupNumber);
