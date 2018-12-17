/*!********************************************************************************************
\File         VulkanPostProcessing.cpp
\Title        Bloom
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief      Shows how to do a bloom effect
***********************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRCore/cameras/TPSCamera.h"
#define _USE_MATH_DEFINES
#include <math.h>

namespace BufferEntryNames {
namespace PerMesh {
const char* const MVPMatrix = "mvpMatrix";
const char* const WorldMatrix = "worldMatrix";
} // namespace PerMesh

namespace Scene {
const char* const EyePosition = "eyePosition";
const char* const LightPosition = "lightPosition";
const char* const InverseViewProjectionMatrix = "inverseViewProjectionMatrix";
} // namespace Scene

namespace BloomConfig {
const char* const LuminosityThreshold = "luminosityThreshold";
} // namespace BloomConfig
} // namespace BufferEntryNames

// Bloom modes
enum class BloomMode
{
	NoBloom = 0,
	GaussianOriginal,
	GaussianLinear,
	Compute,
	HybridGaussian,
	GaussianLinearTruncated,
	Kawase,
	DualFilter,
	TentFilter,
	NumBloomModes,
	DefaultMode = GaussianLinearTruncated
};

// Titles for the various bloom modes
const std::string BloomStrings[] = { "Original Image (No Post Processing)", "Gaussian (Reference Implementation)", "Gaussian (Linear Sampling)",
	"Gaussian (Compute Sliding Average)", "Hybrid Gaussian", "Truncated Gaussian (Linear Sampling)", "Kawase", "Dual Filter", "Tent Filter" };

// Files used throughout the demo
namespace Files {
// Shader file names
const char Downsample2x2VertSrcFile[] = "Downsample2x2VertShader.vsh.spv";
const char Downsample2x2FragSrcFile[] = "Downsample2x2FragShader.fsh.spv";
const char Downsample4x4VertSrcFile[] = "Downsample4x4VertShader.vsh.spv";
const char Downsample4x4FragSrcFile[] = "Downsample4x4FragShader.fsh.spv";
const char DualFilterDownSampleFragSrcFile[] = "DualFilterDownSampleFragShader.fsh.spv";
const char DualFilterUpSampleFragSrcFile[] = "DualFilterUpSampleFragShader.fsh.spv";
const char DualFilterUpSampleMergedFinalPassFragSrcFile[] = "DualFilterUpSampleMergedFinalPassFragShader.fsh.spv";
const char DualFilterDownVertSrcFile[] = "DualFilterDownVertShader.vsh.spv";
const char DualFilterUpVertSrcFile[] = "DualFilterUpVertShader.vsh.spv";
const char TentFilterUpSampleVertSrcFile[] = "TentFilterUpSampleVertShader.vsh.spv";
const char TentFilterUpSampleFragSrcFile[] = "TentFilterUpSampleFragShader.fsh.spv";
const char TentFilterFirstUpSampleFragSrcFile[] = "TentFilterFirstUpSampleFragShader.fsh.spv";
const char TentFilterUpSampleMergedFinalPassFragSrcFile[] = "TentFilterUpSampleMergedFinalPassFragShader.fsh.spv";
const char GaussianComputeBlurHorizontal16fSrcFile[] = "GaussianCompHorizontalShader16f.csh.spv";
const char GaussianComputeBlurHorizontal16frgbaSrcFile[] = "GaussianCompHorizontalShader16frgba.csh.spv";
const char GaussianComputeBlurVertical16fSrcFile[] = "GaussianCompVerticalShader16f.csh.spv";
const char GaussianComputeBlurVertical16frgbaSrcFile[] = "GaussianCompVerticalShader16frgba.csh.spv";
const char GaussianHorizontalFragSrcFile[] = "GaussianHorizontalFragShader.fsh.spv";
const char GaussianVerticalFragSrcFile[] = "GaussianVerticalFragShader.fsh.spv";
const char GaussianVertSrcFile[] = "GaussianVertShader.vsh.spv";
const char KawaseVertSrcFile[] = "KawaseVertShader.vsh.spv";
const char KawaseFragSrcFile[] = "KawaseFragShader.fsh.spv";
const char LinearGaussianEvenSamplesFragSrcFile[] = "LinearGaussianEvenSamplesFragShader.fsh.spv";
const char LinearGaussianEvenSamplesHorizontalVertSrcFile[] = "LinearGaussianEvenSamplesHorizontalVertShader.vsh.spv";
const char LinearGaussianEvenSamplesVerticalVertSrcFile[] = "LinearGaussianEvenSamplesVerticalVertShader.vsh.spv";
const char LinearGaussianOddSamplesFragSrcFile[] = "LinearGaussianOddSamplesFragShader.fsh.spv";
const char LinearGaussianOddSamplesHorizontalVertSrcFile[] = "LinearGaussianOddSamplesHorizontalVertShader.vsh.spv";
const char LinearGaussianOddSamplesVerticalVertSrcFile[] = "LinearGaussianOddSamplesVerticalVertShader.vsh.spv";
const char PostBloomVertShaderSrcFile[] = "PostBloomVertShader.vsh.spv";
const char PostBloomFragShaderSrcFile[] = "PostBloomFragShader.fsh.spv";
const char FragShaderSrcFile[] = "FragShader.fsh.spv";
const char VertShaderSrcFile[] = "VertShader.vsh.spv";
const char SkyboxFragShaderSrcFile[] = "SkyboxFragShader.fsh.spv";
const char SkyboxVertShaderSrcFile[] = "SkyboxVertShader.vsh.spv";
} // namespace Files

// POD scene files
const char SceneFile[] = "Satyr.pod";

// Texture files
const char StatueTexFile[] = "Marble.pvr";
const char StatueNormalMapTexFile[] = "MarbleNormalMap.pvr";
const char SkyboxTexFile[] = "MonValley_baked_lightmap.pvr";
const char DiffuseIrradianceMapTexFile[] = "DiffuseIrradianceMap.pvr";

// Various defaults
const float CameraNear = 1.0f;
const float CameraFar = 1000.0f;
const float RotateY = glm::pi<float>() / 150;
const float Fov = 0.80f;
const float BloomLumaThreshold = 0.8f;
const glm::vec3 LightPosition = glm::vec3(100.0, 50.0, 1000.0);
const float MinimumAcceptibleCoefficient = 0.0003f;
const uint8_t MaxDualFilterIteration = 10;
const uint8_t MaxKawaseIteration = 5;
const uint8_t MaxGaussianKernel = 51;
const uint8_t MaxGaussianHalfKernel = (MaxGaussianKernel - 1) / 2 + 1;

const pvr::utils::VertexBindings VertexAttribBindings[] = {
	{ "POSITION", 0 },
	{ "NORMAL", 1 },
	{ "UV0", 2 },
	{ "TANGENT", 3 },
};

// Handles the configurations being used in the demo controlling how the various bloom techniques will operate
namespace DemoConfigurations {
// Wrapper for a Kawase pass including the number of iterations in use and their kernel sizes
struct KawasePass
{
	uint32_t numIterations;
	uint32_t kernel[MaxKawaseIteration];
};

// A wrapper for the demo configuration at any time
struct DemoConfiguration
{
	std::pair<uint32_t, std::string> gaussianConfig;
	std::pair<uint32_t, std::string> linearGaussianConfig;
	std::pair<uint32_t, std::string> computeGaussianConfig;
	std::pair<uint32_t, std::string> truncatedLinearGaussianConfig;
	std::pair<KawasePass, std::string> kawaseConfig;
	std::pair<uint32_t, std::string> dualFilterConfig;
	std::pair<uint32_t, std::string> tentFilterConfig;
	std::pair<uint32_t, std::string> hybridConfig;
};

const uint32_t NumDemoConfigurations = 5;
const uint32_t DefaultDemoConfigurations = 2;
DemoConfiguration Configurations[NumDemoConfigurations]{ // Demo Blur Configurations
	DemoConfiguration{
		std::make_pair(5, "Kernel Size = 5 (5 + 5 taps)"), // Original Gaussian Blur
		std::make_pair(5, "Kernel Size = 5 (3 + 3 taps)"), // Linear Gaussian Blur
		std::make_pair(5, "Kernel Size = 5 (Sliding Average)"), // Compute Gaussian Blur
		std::make_pair(5, "Kernel Size = 5 (3 + 3 taps)"), // Truncated Linear Gaussian Blur
		std::make_pair(KawasePass{ 2, { 0, 0 } }, "2 Iterations: 0, 0"), // Kawase Blur
		std::make_pair(2, "Iterations = 2 (1 downsample, 1 upsample)"), // Dual Filter Blur
		std::make_pair(2, "Iterations = 2 (1 downsample, 1 upsample)"), // Tent Filter
		std::make_pair(0, "Horizontal Compute (5 taps), Vertical Truncated Gaussian (3 taps)"), // Hybrid
	},
	DemoConfiguration{
		std::make_pair(15, "Kernel Size = 15 (15 + 15 taps)"), // Original Gaussian Blur
		std::make_pair(15, "Kernel Size = 15 (8 + 8 taps)"), // Linear Gaussian Blur
		std::make_pair(15, "Kernel Size = 15 (Sliding Average)"), // Compute Gaussian Blur
		std::make_pair(11, "Kernel Size = 11 (6 + 6 taps)"), // Truncated Linear Gaussian Blur
		std::make_pair(KawasePass{ 3, { 0, 0, 1 } }, "3 Iterations: 0, 0, 1"), // Kawase Blur
		std::make_pair(4, "Iterations = 4 (2 downsample, 2 upsample)"), // Dual Filter Blur
		std::make_pair(4, "Iterations = 4 (2 downsample, 2 upsample)"), // Tent Filter
		std::make_pair(0, "Horizontal Compute (15 taps), Vertical Truncated Gaussian (6 taps)"), // Hybrid
	},
	DemoConfiguration{
		std::make_pair(25, "Kernel Size = 25 (25 + 25 taps)"), // Original Gaussian Blur
		std::make_pair(25, "Kernel Size = 25 (13 + 13 taps)"), // Linear Gaussian Blur
		std::make_pair(25, "Kernel Size = 25 (Sliding Average)"), // Compute Gaussian Blur
		std::make_pair(17, "Kernel Size = 17 (9 + 9 taps)"), // Truncated Linear Gaussian Blur
		std::make_pair(KawasePass{ 4, { 0, 0, 1, 1 } }, "4 Iterations: 0, 0, 1, 1"), // Kawase Blur
		std::make_pair(6, "Iterations = 6 (3 downsample, 3 upsample)"), // Dual Filter Blur
		std::make_pair(6, "Iterations = 6 (3 downsample, 3 upsample)"), // Tent Filter
		std::make_pair(0, "Horizontal Compute (25 taps), Vertical Truncated Gaussian (9 taps)"), // Hybrid
	},
	DemoConfiguration{
		std::make_pair(35, "Kernel Size = 35 (35 + 35 taps)"), // Original Gaussian Blur
		std::make_pair(35, "Kernel Size = 35 (18 + 18 taps)"), // Linear Gaussian Blur
		std::make_pair(35, "Kernel Size = 35 (Sliding Average)"), // Compute Gaussian Blur
		std::make_pair(21, "Kernel Size = 21 (11 + 11 taps)"), // Truncated Linear Gaussian Blur
		std::make_pair(KawasePass{ 4, { 0, 1, 1, 1 } }, "4 Iterations: 0, 1, 1, 1"), // Kawase Blur
		std::make_pair(8, "Iterations = 8 (4 downsample, 4 upsample)"), // Dual Filter Blur
		std::make_pair(8, "Iterations = 8 (4 downsample, 4 upsample)"), // Tent Filter
		std::make_pair(0, "Horizontal Compute (35 taps), Vertical Truncated Gaussian (11 taps)"), // Hybrid
	},
	DemoConfiguration{
		std::make_pair(51, "Kernel Size = 51 (51 + 51 taps)"), // Original Gaussian Blur
		std::make_pair(51, "Kernel Size = 51 (26 + 26 taps)"), // Linear Gaussian Blur
		std::make_pair(51, "Kernel Size = 51 (Sliding Average)"), // Compute Gaussian Blur
		std::make_pair(25, "Kernel Size = 25 (13 + 13 taps)"), // Truncated Linear Gaussian Blur
		std::make_pair(KawasePass{ 5, { 0, 0, 1, 1, 2 } }, "5 Iterations: 0, 0, 1, 1, 2"), // Kawase Blur
		std::make_pair(10, "Iterations = 10 (5 downsample, 5 upsample)"), // Dual Filter Blur
		std::make_pair(10, "Iterations = 10 (5 downsample, 5 upsample)"), // Tent Filter
		std::make_pair(0, "Horizontal Compute (51 taps), Vertical Truncated Gaussian (13 taps)"), // Hybrid
	}
};
} // namespace DemoConfigurations

/// <summary>Prints the gaussian weights and offsets provided in the vectors.</summary>
/// <param name="gaussianOffsets">The list of gaussian offsets to print.</param>
/// <param name="gaussianWeights">The list of gaussian weights to print.</param>
void printGaussianWeightsAndOffsets(std::vector<double>& gaussianOffsets, std::vector<double>& gaussianWeights)
{
	Log(LogLevel::Information, "const int maxStepCount = %u;", gaussianWeights.size());
	Log(LogLevel::Information, "const float gWeights[maxStepCount] =");
	Log(LogLevel::Information, "{");

	for (uint32_t i = 0; i < gaussianWeights.size() - 1; i++)
	{
		Log(LogLevel::Information, "%.15f,", gaussianWeights[i]);
	}

	Log(LogLevel::Information, "%.15f", gaussianWeights[gaussianWeights.size() - 1]);
	Log(LogLevel::Information, "};");

	Log(LogLevel::Information, "const float gOffsets[maxStepCount] =");
	Log(LogLevel::Information, "{");

	for (uint32_t i = 0; i < gaussianOffsets.size() - 1; i++)
	{
		Log(LogLevel::Information, "%.15f,", gaussianOffsets[i]);
	}

	Log(LogLevel::Information, "%.15f", gaussianOffsets[gaussianOffsets.size() - 1]);
	Log(LogLevel::Information, "};");
}

/// <summary>Updates the gaussian weights and offsets using the configuration provided.</summary>
/// <param name="kernelSize">The kernel size to generate gaussian weights and offsets for.</param>
/// <param name="useLinearOptimisation">Specifies whether linear sampling will be used when texture sampling using the given weights and offsets,
/// if linear sampling will be used then the weights and offsets must be adjusted accordingly.</param>
/// <param name="truncateCoefficients">Specifies whether to truncate and ignore coefficients which would provide a negligible change in the resulting blurred image.</param>
/// <param name="gaussianOffsets">The returned list of gaussian offsets (as double).</param>
/// <param name="gaussianWeights">The returned list of gaussian weights (as double).</param>
/// <param name="gaussianOffsetsFloats">The returned list of gaussian offsets (as float).</param>
/// <param name="gaussianWeightsFloats">The returned list of gaussian weights (as float).</param>
void updateGaussianWeightsAndOffsets(uint32_t kernelSize, bool useLinearOptimisation, bool truncateCoefficients, std::vector<double>& gaussianOffsets,
	std::vector<double>& gaussianWeights, std::vector<float>& gaussianOffsetsFloats, std::vector<float>& gaussianWeightsFloats)
{
	// Ensure that the kernel given is odd in size. Our utility function requires a central sampling position although this demo also caters for even kernel sizes
	assertion((kernelSize - 1) % 2 == 0);
	assertion(kernelSize <= MaxGaussianKernel);

	// clear the previous set of gaussian weights and offsets
	gaussianWeights.clear();
	gaussianOffsets.clear();
	gaussianWeightsFloats.clear();
	gaussianOffsetsFloats.clear();

	// generate a new set of weights and offsets based on the given configuration
	pvr::math::generateGaussianKernelWeightsAndOffsets(kernelSize, truncateCoefficients, useLinearOptimisation, gaussianWeights, gaussianOffsets, MinimumAcceptibleCoefficient);

	// The following can be used to print the current set of Gaussian Offsets and Weights in use and can be helpful during debugging
	/*
	if (useLinearOptimisation)
	{
		Log(LogLevel::Information, "Linear Sampling Optimized Gaussian Weights and Offsets:");
	}
	else
	{
		Log(LogLevel::Information, "Gaussian Weights and Offsets:");
	}
	printGaussianWeightsAndOffsets(gaussianOffsets, gaussianWeights);
	*/

	// Convert the Gaussian weights from double precision to floating point
	// Only store half of the kernel weights and offsets rather than the full kernel size set of weights and offsets as each side of the kernel will match the other meaning we
	// can save on the amount of data to upload and sample from in the shader
	if (gaussianWeights.size() % 2 == 0)
	{
		uint32_t halfKernelSize = static_cast<uint32_t>(gaussianWeights.size() / 2);

		gaussianWeightsFloats.resize(halfKernelSize);
		gaussianOffsetsFloats.resize(halfKernelSize);
		for (uint32_t i = halfKernelSize; i < gaussianWeights.size(); ++i)
		{
			gaussianWeightsFloats[i - halfKernelSize] = static_cast<float>(gaussianWeights[i]);
			gaussianOffsetsFloats[i - halfKernelSize] = static_cast<float>(gaussianOffsets[i]);
		}
	}
	else
	{
		uint32_t halfKernelSize = static_cast<uint32_t>((gaussianWeights.size() - 1) / 2 + 1);

		gaussianWeightsFloats.resize(halfKernelSize);
		gaussianOffsetsFloats.resize(halfKernelSize);
		for (uint32_t i = halfKernelSize - 1; i < gaussianWeights.size(); ++i)
		{
			gaussianWeightsFloats[i - (halfKernelSize - 1)] = static_cast<float>(gaussianWeights[i]);
			gaussianOffsetsFloats[i - (halfKernelSize - 1)] = static_cast<float>(gaussianOffsets[i]);
		}
	}
}

// A simple pass used for rendering our statue object
struct StatuePass
{
	pvrvk::GraphicsPipeline pipeline;
	pvrvk::PipelineLayout pipelineLayout;
	pvrvk::ImageView albeoImageView;
	pvrvk::ImageView normalMapImageView;
	pvrvk::DescriptorSetLayout descriptorSetLayout;
	pvr::Multi<pvrvk::DescriptorSet> descriptorSets;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> commandBuffers;
	pvr::utils::StructuredBufferView structuredBufferView;
	pvrvk::Buffer buffer;
	std::vector<pvrvk::Buffer> vbos;
	std::vector<pvrvk::Buffer> ibos;

	// 3D Model
	pvr::assets::ModelHandle scene;

	/// <summary>Initialises the Statue pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="commandPool">The command pool from which to allocate command buffers.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="renderpass">The RenderPass to use.</param>
	/// <param name="framebuffers">The framebuffers to use.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	/// <param name="utilityCommandBuffer">A command buffer to use for queueing up all initialisation commands. This command buffer will be submitted later by the main
	/// application.</param>
	/// <param name="samplerBilinear">A bilinear sampler object.</param>
	/// <param name="samplerTrilinear">A trilinear sampler object.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	/// <param name="irrandianceImageView">A diffuse irradiance map to use as an albedo replacement.</param>
	/// <param name="sceneBufferView">Buffer view for the scene buffer.</param>
	/// <param name="sceneBuffer">The scene buffer.</param>
	/// <param name="bloomConfigBufferView">Buffer view for the bloom config buffer.</param>
	/// <param name="bloomConfigBuffer">The bloom config buffer.</param>
	void init(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::CommandPool& commandPool, pvrvk::DescriptorPool& descriptorPool,
		pvrvk::RenderPass& renderpass, pvr::Multi<pvrvk::Framebuffer>& framebuffers, pvr::utils::vma::Allocator& vmaAllocator, pvrvk::CommandBuffer& utilityCommandBuffer,
		pvrvk::Sampler& samplerBilinear, pvrvk::Sampler& samplerTrilinear, pvrvk::PipelineCache& pipelineCache, pvrvk::ImageView& irrandianceImageView,
		pvr::utils::StructuredBufferView& sceneBufferView, pvrvk::Buffer& sceneBuffer, pvr::utils::StructuredBufferView& bloomConfigBufferView, pvrvk::Buffer& bloomConfigBuffer)
	{
		// Load the scene
		pvr::assets::helper::loadModel(assetProvider, SceneFile, scene);

		bool requiresCommandBufferSubmission = false;
		pvr::utils::appendSingleBuffersFromModel(device, *scene, vbos, ibos, utilityCommandBuffer, requiresCommandBufferSubmission, &vmaAllocator);

		createBuffer(device, swapchain, vmaAllocator);
		loadTextures(assetProvider, device, utilityCommandBuffer, vmaAllocator);

		createDescriptorSetLayout(device);
		createPipeline(assetProvider, device, renderpass, swapchain->getDimension(), pipelineCache);

		createDescriptorSets(
			device, swapchain, descriptorPool, irrandianceImageView, samplerBilinear, samplerTrilinear, sceneBufferView, sceneBuffer, bloomConfigBufferView, bloomConfigBuffer);

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			commandBuffers.add(commandPool->allocateSecondaryCommandBuffer());
			// Commands can be recorded up front - these don't change based on changes to the bloom used
			recordCommandBuffer(i, framebuffers, commandBuffers);
		}
	}

	/// <summary>Update the object animation.</summary>
	/// <param name="angle">The angle to use for rotating the statue.</param>
	/// <param name="viewProjectionMatrix">The view projection matrix to use for rendering.</param>
	/// <param name="swapchainIndex">The current swapchain index.</param>
	void updateAnimation(const float angle, glm::mat4& viewProjectionMatrix, uint32_t swapchainIndex)
	{
		// Calculate the model matrix
		const glm::mat4 mModel = glm::translate(glm::vec3(0.0f, 5.0f, 0.0f)) * glm::rotate(angle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(2.2f));

		glm::mat4 worldMatrix = mModel * scene->getWorldMatrix(scene->getNode(0).getObjectId());
		glm::mat4 mvpMatrix = viewProjectionMatrix * worldMatrix;

		structuredBufferView.getElementByName(BufferEntryNames::PerMesh::MVPMatrix, 0, swapchainIndex).setValue(mvpMatrix);
		structuredBufferView.getElementByName(BufferEntryNames::PerMesh::WorldMatrix, 0, swapchainIndex).setValue(worldMatrix);

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			buffer->getDeviceMemory()->flushRange(structuredBufferView.getDynamicSliceOffset(swapchainIndex), structuredBufferView.getDynamicSliceSize());
		}
	}

	/// <summary>Creates any required buffers.</summary>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	void createBuffer(pvrvk::Device device, pvrvk::Swapchain& swapchain, pvr::utils::vma::Allocator& vmaAllocator)
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement(BufferEntryNames::PerMesh::MVPMatrix, pvr::GpuDatatypes::mat4x4);
		desc.addElement(BufferEntryNames::PerMesh::WorldMatrix, pvr::GpuDatatypes::mat4x4);

		structuredBufferView.initDynamic(desc, scene->getNumMeshNodes() * swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
		buffer = pvr::utils::createBuffer(device, structuredBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT, &vmaAllocator,
			pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
		structuredBufferView.pointToMappedMemory(buffer->getDeviceMemory()->getMappedData());
	}

	/// <summary>Creates the textures used for rendering the statue.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="utilityCommandBuffer">A command buffer to use for queueing up all initialisation commands. This command buffer will be submitted later by the main
	/// application.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	void loadTextures(pvr::IAssetProvider& assetProvider, pvrvk::Device device, pvrvk::CommandBuffer& utilityCommandBuffer, pvr::utils::vma::Allocator& vmaAllocator)
	{
		// Load the Texture PVR file from the disk
		pvr::Texture albedoTexture = pvr::textureLoad(assetProvider.getAssetStream(StatueTexFile), pvr::TextureFileFormat::PVR);
		pvr::Texture normalMapTexture = pvr::textureLoad(assetProvider.getAssetStream(StatueNormalMapTexFile), pvr::TextureFileFormat::PVR);

		// Create and Allocate Textures.
		albeoImageView = pvr::utils::uploadImageAndView(
			device, albedoTexture, true, utilityCommandBuffer, pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, &vmaAllocator, &vmaAllocator);

		normalMapImageView = pvr::utils::uploadImageAndView(device, normalMapTexture, true, utilityCommandBuffer, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, &vmaAllocator, &vmaAllocator);
	}

	/// <summary>Creates the descriptor set layouts used for rendering the statue.</summary>
	/// <param name="device">The device from which the descriptor set layouts will be allocated.</param>
	void createDescriptorSetLayout(pvrvk::Device& device)
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayout;

		descSetLayout.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetLayout.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetLayout.setBinding(2, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetLayout.setBinding(3, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetLayout.setBinding(4, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);
		descSetLayout.setBinding(5, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

		descriptorSetLayout = device->createDescriptorSetLayout(descSetLayout);

		pvrvk::PipelineLayoutCreateInfo pipelineLayoutInfo;
		pipelineLayoutInfo.setDescSetLayout(0, descriptorSetLayout);

		pipelineLayout = device->createPipelineLayout(pipelineLayoutInfo);
	}

	/// <summary>Creates the descriptor sets used for rendering the statue.</summary>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="diffuseIrradianceMap">The diffuse irradiance map used as a replacement to a fixed albedo.</param>
	/// <param name="samplerBilinear">A bilinear sampler object.</param>
	/// <param name="samplerTrilinear">A trilinear sampler object.</param>
	/// <param name="sceneBufferView">Buffer view for the scene buffer.</param>
	/// <param name="sceneBuffer">The scene buffer.</param>
	/// <param name="bloomConfigBufferView">Buffer view for the bloom config buffer.</param>
	/// <param name="bloomConfigBuffer">The bloom config buffer.</param>
	void createDescriptorSets(pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::DescriptorPool& descriptorPool, pvrvk::ImageView& diffuseIrradianceMap,
		pvrvk::Sampler& samplerBilinear, pvrvk::Sampler& samplerTrilinear, pvr::utils::StructuredBufferView& sceneBufferView, pvrvk::Buffer& sceneBuffer,
		pvr::utils::StructuredBufferView& bloomConfigBufferView, pvrvk::Buffer& bloomConfigBuffer)
	{
		std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			descriptorSets.add(descriptorPool->allocateDescriptorSet(descriptorSetLayout));

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[i], 0)
										.setImageInfo(0, pvrvk::DescriptorImageInfo(albeoImageView, samplerBilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[i], 1)
										.setImageInfo(0, pvrvk::DescriptorImageInfo(normalMapImageView, samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[i], 2)
										.setImageInfo(0, pvrvk::DescriptorImageInfo(diffuseIrradianceMap, samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, descriptorSets[i], 3)
										.setBufferInfo(0, pvrvk::DescriptorBufferInfo(sceneBuffer, sceneBufferView.getDynamicSliceOffset(i), sceneBufferView.getDynamicSliceSize())));

			writeDescSets.push_back(
				pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, descriptorSets[i], 4)
					.setBufferInfo(0, pvrvk::DescriptorBufferInfo(buffer, structuredBufferView.getDynamicSliceOffset(i), structuredBufferView.getDynamicSliceSize())));

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, descriptorSets[i], 5)
										.setBufferInfo(0, pvrvk::DescriptorBufferInfo(bloomConfigBuffer, 0, bloomConfigBufferView.getSize())));
		}

		device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
	}

	/// <summary>Creates the pipeline.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device to use for allocating the pipelines.</param>
	/// <param name="renderpass">The RenderPass to use.</param>
	/// <param name="viewportDimensions">The viewport dimensions.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipeline.</param>
	void createPipeline(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, const pvrvk::RenderPass& renderpass, const pvrvk::Extent2D& viewportDimensions,
		pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::GraphicsPipelineCreateInfo pipelineInfo;

		pipelineInfo.viewport.setViewportAndScissor(0,
			pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(viewportDimensions.getWidth()), static_cast<float>(viewportDimensions.getHeight())),
			pvrvk::Rect2D(0, 0, viewportDimensions.getWidth(), viewportDimensions.getHeight()));

		pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);

		// depth stencil state
		pipelineInfo.depthStencil.enableDepthWrite(true);
		pipelineInfo.depthStencil.enableDepthTest(true);
		pipelineInfo.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_LESS);
		pipelineInfo.depthStencil.enableStencilTest(false);

		// blend state
		pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
		pipelineInfo.colorBlend.setAttachmentState(1, pvrvk::PipelineColorBlendAttachmentState());

		pipelineInfo.vertexShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::VertShaderSrcFile)->readToEnd<uint32_t>())));
		pipelineInfo.fragmentShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::FragShaderSrcFile)->readToEnd<uint32_t>())));

		const pvr::assets::Mesh& mesh = scene->getMesh(0);
		pipelineInfo.inputAssembler.setPrimitiveTopology(pvr::utils::convertToPVRVk(mesh.getPrimitiveType()));
		pvr::utils::populateInputAssemblyFromMesh(
			mesh, VertexAttribBindings, sizeof(VertexAttribBindings) / sizeof(VertexAttribBindings[0]), pipelineInfo.vertexInput, pipelineInfo.inputAssembler);

		pipelineInfo.renderPass = renderpass;

		pipelineInfo.pipelineLayout = pipelineLayout;

		pipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
	}

	/// <summary>Draws an assets::Mesh after the model view matrix has been set and the material prepared.</summary>
	/// <param name="commandBuffer">The secondary command buffer to record rendering commands to.</param>
	/// <param name="nodeIndex">Node index of the mesh to draw</param>
	void drawMesh(pvrvk::SecondaryCommandBuffer& commandBuffer, int nodeIndex)
	{
		const uint32_t meshId = scene->getNode(nodeIndex).getObjectId();
		const pvr::assets::Mesh& mesh = scene->getMesh(meshId);

		// bind the VBO for the mesh
		commandBuffer->bindVertexBuffer(vbos[meshId], 0, 0);

		//  The geometry can be exported in 4 ways:
		//  - Indexed Triangle list
		//  - Non-Indexed Triangle list
		//  - Indexed Triangle strips
		//  - Non-Indexed Triangle strips
		if (mesh.getNumStrips() == 0)
		{
			// Indexed Triangle list
			if (ibos[meshId].isValid())
			{
				commandBuffer->bindIndexBuffer(ibos[meshId], 0, pvr::utils::convertToPVRVk(mesh.getFaces().getDataType()));
				commandBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
			}
			else
			{
				// Non-Indexed Triangle list
				commandBuffer->draw(0, mesh.getNumFaces() * 3, 0, 1);
			}
		}
		else
		{
			uint32_t offset = 0;
			for (uint32_t i = 0; i < mesh.getNumStrips(); ++i)
			{
				if (ibos[meshId].isValid())
				{
					// Indexed Triangle strips
					commandBuffer->bindIndexBuffer(ibos[meshId], 0, pvr::utils::convertToPVRVk(mesh.getFaces().getDataType()));
					commandBuffer->drawIndexed(0, mesh.getStripLength(i) + 2, offset * 2, 0, 1);
				}
				else
				{
					// Non-Indexed Triangle strips
					commandBuffer->draw(0, mesh.getStripLength(i) + 2, 0, 1);
				}
				offset += mesh.getStripLength(i) + 2;
			}
		}
	}

	/// <summary>Records the secondary command buffers for rendering the statue.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="framebuffers">The framebuffers to render into</param>
	/// <param name="commandBuffers">The secondary command buffers to record into.</param>
	void recordCommandBuffer(uint32_t swapchainIndex, pvr::Multi<pvrvk::Framebuffer>& framebuffers, pvr::Multi<pvrvk::SecondaryCommandBuffer>& commandBuffers)
	{
		commandBuffers[swapchainIndex]->begin(framebuffers[swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
		commandBuffers[swapchainIndex]->debugMarkerBeginEXT("Statue");
		commandBuffers[swapchainIndex]->bindPipeline(pipeline);
		commandBuffers[swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipelineLayout, 0u, descriptorSets[swapchainIndex]);
		drawMesh(commandBuffers[swapchainIndex], 0);
		commandBuffers[swapchainIndex]->debugMarkerEndEXT();
		commandBuffers[swapchainIndex]->end();
	}
};

// A simple pass used for rendering our skybox
struct SkyboxPass
{
	pvrvk::GraphicsPipeline pipeline;
	pvrvk::PipelineLayout pipelineLayout;
	pvrvk::ImageView skyBoxImageView;
	pvrvk::DescriptorSetLayout descriptorSetLayout;
	pvr::Multi<pvrvk::DescriptorSet> descriptorSets;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> commandBuffers;

	/// <summary>Initialises the skybox pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="commandPool">The command pool from which to allocate command buffers.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="renderpass">The RenderPass to use.</param>
	/// <param name="framebuffers">The framebuffers to use.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	/// <param name="utilityCommandBuffer">A command buffer to use for queueing up all initialisation commands. This command buffer will be submitted later by the main
	/// application.</param>
	/// <param name="samplerTrilinear">A trilinear sampler object.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	/// <param name="sceneBufferView">Buffer view for the scene buffer.</param>
	/// <param name="sceneBuffer">The scene buffer.</param>
	/// <param name="bloomConfigBufferView">Buffer view for the bloom config buffer.</param>
	/// <param name="bloomConfigBuffer">The bloom config buffer.</param>
	void init(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::CommandPool& commandPool, pvrvk::DescriptorPool& descriptorPool,
		pvrvk::RenderPass& renderpass, pvr::Multi<pvrvk::Framebuffer>& framebuffers, pvr::utils::vma::Allocator& vmaAllocator, pvrvk::CommandBuffer& utilityCommandBuffer,
		pvrvk::Sampler& samplerTrilinear, pvrvk::PipelineCache& pipelineCache, pvr::utils::StructuredBufferView& sceneBufferView, pvrvk::Buffer& sceneBuffer,
		pvr::utils::StructuredBufferView& bloomConfigBufferView, pvrvk::Buffer& bloomConfigBuffer)
	{
		loadSkyBoxTextures(assetProvider, device, utilityCommandBuffer, vmaAllocator);

		createDescriptorSetLayout(device);
		createPipeline(assetProvider, device, renderpass, swapchain->getDimension(), pipelineCache);

		createDescriptorSets(device, swapchain, descriptorPool, samplerTrilinear, sceneBufferView, sceneBuffer, bloomConfigBufferView, bloomConfigBuffer);

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			commandBuffers.add(commandPool->allocateSecondaryCommandBuffer());
			recordCommandBuffer(i, framebuffers, commandBuffers);
		}
	}

	/// <summary>Creates the textures used for rendering the skybox.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="utilityCommandBuffer">A command buffer to use for queueing up all initialisation commands. This command buffer will be submitted later by the main
	/// application.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	void loadSkyBoxTextures(pvr::IAssetProvider& assetProvider, pvrvk::Device device, pvrvk::CommandBuffer& utilityCommandBuffer, pvr::utils::vma::Allocator& vmaAllocator)
	{
		// Load the Texture PVR file from the disk
		pvr::Texture skyBoxTexture = pvr::textureLoad(assetProvider.getAssetStream(SkyboxTexFile), pvr::TextureFileFormat::PVR);

		// Create and Allocate Textures.
		skyBoxImageView = pvr::utils::uploadImageAndView(
			device, skyBoxTexture, true, utilityCommandBuffer, pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, &vmaAllocator, &vmaAllocator);
	}

	/// <summary>Creates the descriptor set layouts used for rendering the statue.</summary>
	/// <param name="device">The device from which the descriptor set layouts will be allocated.</param>
	void createDescriptorSetLayout(pvrvk::Device& device)
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayout;

		descSetLayout.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetLayout.setBinding(1, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);
		descSetLayout.setBinding(2, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

		descriptorSetLayout = device->createDescriptorSetLayout(descSetLayout);

		pvrvk::PipelineLayoutCreateInfo pipelineLayoutInfo;
		pipelineLayoutInfo.setDescSetLayout(0, descriptorSetLayout);

		pipelineLayout = device->createPipelineLayout(pipelineLayoutInfo);
	}

	/// <summary>Creates the descriptor sets used for rendering the skybox.</summary>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="samplerTrilinear">A trilinear sampler object.</param>
	/// <param name="sceneBufferView">Buffer view for the scene buffer.</param>
	/// <param name="sceneBuffer">The scene buffer.</param>
	/// <param name="bloomConfigBufferView">Buffer view for the bloom config buffer.</param>
	/// <param name="bloomConfigBuffer">The bloom config buffer.</param>
	void createDescriptorSets(pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::DescriptorPool& descriptorPool, pvrvk::Sampler& samplerTrilinear,
		pvr::utils::StructuredBufferView& sceneBufferView, pvrvk::Buffer& sceneBuffer, pvr::utils::StructuredBufferView& bloomConfigBufferView, pvrvk::Buffer& bloomConfigBuffer)
	{
		std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			descriptorSets.add(descriptorPool->allocateDescriptorSet(descriptorSetLayout));

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[i], 0)
										.setImageInfo(0, pvrvk::DescriptorImageInfo(skyBoxImageView, samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, descriptorSets[i], 1)
										.setBufferInfo(0, pvrvk::DescriptorBufferInfo(sceneBuffer, sceneBufferView.getDynamicSliceOffset(i), sceneBufferView.getDynamicSliceSize())));

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, descriptorSets[i], 2)
										.setBufferInfo(0, pvrvk::DescriptorBufferInfo(bloomConfigBuffer, 0, bloomConfigBufferView.getSize())));
		}

		device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
	}

	/// <summary>Creates the pipeline.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device to use for allocating the pipelines.</param>
	/// <param name="renderpass">The RenderPass to use.</param>
	/// <param name="viewportDimensions">The viewport dimensions.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipeline.</param>
	void createPipeline(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, const pvrvk::RenderPass& renderpass, const pvrvk::Extent2D& viewportDimensions,
		pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::GraphicsPipelineCreateInfo pipelineInfo;

		pipelineInfo.viewport.setViewportAndScissor(0,
			pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(viewportDimensions.getWidth()), static_cast<float>(viewportDimensions.getHeight())),
			pvrvk::Rect2D(0, 0, viewportDimensions.getWidth(), viewportDimensions.getHeight()));

		pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);

		pipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

		// depth stencil state
		pipelineInfo.depthStencil.enableDepthWrite(false);
		pipelineInfo.depthStencil.enableDepthTest(true);
		pipelineInfo.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_LESS_OR_EQUAL);
		pipelineInfo.depthStencil.enableStencilTest(false);

		// blend state
		pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
		pipelineInfo.colorBlend.setAttachmentState(1, pvrvk::PipelineColorBlendAttachmentState());

		pipelineInfo.vertexShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::SkyboxVertShaderSrcFile)->readToEnd<uint32_t>())));
		pipelineInfo.fragmentShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::SkyboxFragShaderSrcFile)->readToEnd<uint32_t>())));

		pipelineInfo.vertexInput.clear();
		pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);

		pipelineInfo.renderPass = renderpass;

		pipelineInfo.pipelineLayout = pipelineLayout;

		pipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
	}

	/// <summary>Records the secondary command buffers for rendering the skybox.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="framebuffers">The framebuffers to render into</param>
	/// <param name="commandBuffers">The secondary command buffers to record into.</param>
	void recordCommandBuffer(uint32_t swapchainIndex, pvr::Multi<pvrvk::Framebuffer>& framebuffers, pvr::Multi<pvrvk::SecondaryCommandBuffer>& commandBuffers)
	{
		commandBuffers[swapchainIndex]->begin(framebuffers[swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
		commandBuffers[swapchainIndex]->debugMarkerBeginEXT("Skybox");
		commandBuffers[swapchainIndex]->bindPipeline(pipeline);
		commandBuffers[swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipelineLayout, 0u, descriptorSets[swapchainIndex]);
		commandBuffers[swapchainIndex]->draw(0, 6);
		commandBuffers[swapchainIndex]->debugMarkerEndEXT();
		commandBuffers[swapchainIndex]->end();
	}
};

// A Downsample pass used for downsampling images by 1/4 x 1/4 i.e. 1/16 resolution
struct DownSamplePass
{
	pvrvk::DescriptorSetLayout descriptorSetLayout;
	pvrvk::PipelineLayout pipelineLayout;
	pvr::Multi<pvrvk::DescriptorSet> descriptorSets;
	pvr::Multi<pvrvk::Framebuffer> framebuffers;
	pvrvk::RenderPass renderPass;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> commandBuffers;
	pvrvk::GraphicsPipeline pipeline;
	glm::vec2 blurConfigs[4];

	/// <summary>Initialises the Downsample pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="commandPool">The command pool from which to allocate command buffers.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="blurFramebufferDimensions">The dimensions to use for the downsample pass. These dimensions should be 1/4 x 1/4 the size of the source image.</param>
	/// <param name="inputImageViews">A set of images to downsample.</param>
	/// <param name="colorImageViews">A pre-allocated set of images to render downsampled images to.</param>
	/// <param name="sampler">A bilinear sampler object.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	/// <param name="isComputeDownsample">Determines the destination image layout to use as well as the destination PipelineStageFlags.</param>
	void init(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::CommandPool& commandPool, pvrvk::DescriptorPool& descriptorPool,
		const glm::ivec2& blurFramebufferDimensions, pvr::Multi<pvrvk::ImageView>& inputImageViews, pvr::Multi<pvrvk::ImageView>& colorImageViews, pvrvk::Sampler& sampler,
		pvrvk::PipelineCache& pipelineCache, bool isComputeDownsample)
	{
		// A set of pre-calculated offsets to use for the downsample
		const glm::vec2 offsets[4] = { glm::vec2(-1.0, -1.0), glm::vec2(1.0, -1.0), glm::vec2(-1.0, 1.0), glm::vec2(1.0, 1.0) };

		blurConfigs[0] = glm::vec2(1.0f / (blurFramebufferDimensions.x * 4), 1.0f / (blurFramebufferDimensions.y * 4)) * offsets[0];
		blurConfigs[1] = glm::vec2(1.0f / (blurFramebufferDimensions.x * 4), 1.0f / (blurFramebufferDimensions.y * 4)) * offsets[1];
		blurConfigs[2] = glm::vec2(1.0f / (blurFramebufferDimensions.x * 4), 1.0f / (blurFramebufferDimensions.y * 4)) * offsets[2];
		blurConfigs[3] = glm::vec2(1.0f / (blurFramebufferDimensions.x * 4), 1.0f / (blurFramebufferDimensions.y * 4)) * offsets[3];

		createDescriptorSetLayout(device);
		createDescriptorSets(device, swapchain, descriptorPool, inputImageViews, sampler);
		createFramebuffers(device, swapchain, blurFramebufferDimensions, colorImageViews, isComputeDownsample);
		createPipeline(assetProvider, device, blurFramebufferDimensions, pipelineCache);

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			commandBuffers.add(commandPool->allocateSecondaryCommandBuffer());
		}
	}

	/// <summary>Creates the descriptor set layout.</summary>
	/// <param name="device">The device from which the descriptor set layouts will be allocated.</param>
	void createDescriptorSetLayout(pvrvk::Device& device)
	{
		// create the pre bloom descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

		descriptorSetLayout = device->createDescriptorSetLayout(descSetInfo);

		// create the pipeline layouts
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

		pipeLayoutInfo.setDescSetLayout(0, descriptorSetLayout);

		uint32_t pushConstantsSize = static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec2) * 4);

		pvrvk::PushConstantRange pushConstantsRange;
		pushConstantsRange.setOffset(0);
		pushConstantsRange.setSize(pushConstantsSize);
		pushConstantsRange.setStageFlags(pvrvk::ShaderStageFlags::e_VERTEX_BIT);
		pipeLayoutInfo.setPushConstantRange(0, pushConstantsRange);

		pipelineLayout = device->createPipelineLayout(pipeLayoutInfo);
	}

	/// <summary>Creates the descriptor sets used for the downsample.</summary>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="inputImageViews">A set of images to downsample.</param>
	/// <param name="sampler">A bilinear sampler object.</param>
	void createDescriptorSets(
		pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::DescriptorPool& descriptorPool, pvr::Multi<pvrvk::ImageView>& inputImageViews, pvrvk::Sampler& sampler)
	{
		std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			descriptorSets.add(descriptorPool->allocateDescriptorSet(descriptorSetLayout));

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[i], 0)
										.setImageInfo(0, pvrvk::DescriptorImageInfo(inputImageViews[i], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
		}

		device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
	}

	/// <summary>Creates the pipeline.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device to use for allocating the pipelines.</param>
	/// <param name="blurFramebufferDimensions">The downsampled framebuffer dimensions.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipeline.</param>
	void createPipeline(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, const glm::ivec2& blurFramebufferDimensions, pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.viewport.setViewportAndScissor(0, pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(blurFramebufferDimensions.x), static_cast<float>(blurFramebufferDimensions.y)),
			pvrvk::Rect2D(0, 0, blurFramebufferDimensions.x, blurFramebufferDimensions.y));

		pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_FRONT_BIT);
		pipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

		// disable depth writing and depth testing
		pipelineInfo.depthStencil.enableDepthWrite(false);
		pipelineInfo.depthStencil.enableDepthTest(false);

		// disable stencil testing
		pipelineInfo.depthStencil.enableStencilTest(false);

		pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

		// load and create appropriate shaders
		pipelineInfo.vertexShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::Downsample4x4VertSrcFile)->readToEnd<uint32_t>())));
		pipelineInfo.fragmentShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::Downsample4x4FragSrcFile)->readToEnd<uint32_t>())));

		pipelineInfo.vertexInput.clear();
		pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);

		pipelineInfo.pipelineLayout = pipelineLayout;

		// renderpass/subpass
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;

		pipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
	}

	/// <summary>Allocates the framebuffers used for the downsample.</summary>
	/// <param name="device">The device from which the framebuffers will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="blurFramebufferDimensions">The downsampled framebuffer dimensions.</param>
	/// <param name="colorImageViews">A pre-allocated set of images to render downsampled images to.</param>
	/// <param name="isComputeDownsample">Determines the destination image layout to use as well as the destination PipelineStageFlags.</param>
	void createFramebuffers(
		pvrvk::Device& device, pvrvk::Swapchain& swapchain, const glm::ivec2& blurFramebufferDimensions, pvr::Multi<pvrvk::ImageView>& colorImageViews, bool isComputeDownsample)
	{
		pvrvk::RenderPassCreateInfo renderPassInfo;

		if (isComputeDownsample)
		{
			renderPassInfo.setAttachmentDescription(0,
				pvrvk::AttachmentDescription::createColorDescription(colorImageViews[0]->getImage()->getFormat(), pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_GENERAL,
					pvrvk::AttachmentLoadOp::e_DONT_CARE, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT));
		}
		else
		{
			renderPassInfo.setAttachmentDescription(0,
				pvrvk::AttachmentDescription::createColorDescription(colorImageViews[0]->getImage()->getFormat(), pvrvk::ImageLayout::e_UNDEFINED,
					pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_DONT_CARE, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT));
		}

		pvrvk::SubpassDescription subpass;
		subpass.setColorAttachmentReference(0, pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
		renderPassInfo.setSubpass(0, subpass);

		// Add external subpass dependencies to avoid the implicit subpass depedencies
		pvrvk::SubpassDependency externalDependencies[2];
		externalDependencies[0] = pvrvk::SubpassDependency(pvrvk::SubpassExternal, 0, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT,
			pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::DependencyFlags::e_NONE);

		if (isComputeDownsample)
		{
			externalDependencies[1] =
				pvrvk::SubpassDependency(0, pvrvk::SubpassExternal, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT,
					pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::DependencyFlags::e_NONE);
		}
		else
		{
			externalDependencies[1] =
				pvrvk::SubpassDependency(0, pvrvk::SubpassExternal, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT,
					pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::DependencyFlags::e_NONE);
		}

		renderPassInfo.addSubpassDependency(externalDependencies[0]);
		renderPassInfo.addSubpassDependency(externalDependencies[1]);

		renderPass = device->createRenderPass(renderPassInfo);

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			pvrvk::FramebufferCreateInfo createInfo;
			createInfo.setAttachment(0, colorImageViews[i]);
			createInfo.setDimensions(blurFramebufferDimensions.x, blurFramebufferDimensions.y);
			createInfo.setRenderPass(renderPass);

			framebuffers.add(device->createFramebuffer(createInfo));
		}
	}

	/// <summary>Records the commands required for the downsample.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	void recordCommands(uint32_t swapchainIndex)
	{
		commandBuffers[swapchainIndex]->begin(framebuffers[swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
		commandBuffers[swapchainIndex]->debugMarkerBeginEXT("Downsample");
		commandBuffers[swapchainIndex]->bindPipeline(pipeline);
		commandBuffers[swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipelineLayout, 0u, descriptorSets[swapchainIndex]);
		commandBuffers[swapchainIndex]->pushConstants(
			pipelineLayout, pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec2) * 4), &blurConfigs[0]);
		commandBuffers[swapchainIndex]->draw(0, 3);
		commandBuffers[swapchainIndex]->debugMarkerEndEXT();
		commandBuffers[swapchainIndex]->end();
	}
};

// A Downsample pass used for downsampling images by 1/2 x 1/2 i.e. 1/4 resolution making use of vkCmdBlitImage
struct BlitDownSamplePass
{
	void recordCommands(pvrvk::SecondaryCommandBuffer& commandBuffer, pvrvk::Image& sourceImage, pvrvk::Image& destinationImage, pvrvk::Queue& queue)
	{
		pvrvk::MemoryBarrierSet preBlitLayoutTransition;
		pvrvk::MemoryBarrierSet postBlitLayoutTransition;

		// Setup the image layout transitions
		{
			// Transition the source image ready to be used as a blit source
			preBlitLayoutTransition.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT, sourceImage,
				pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0u, 1u, 0u, 1u), pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL,
				pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, queue->getFamilyIndex(), queue->getFamilyIndex()));

			// Transition the destination image ready to be used as a blit destination
			preBlitLayoutTransition.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT,
				destinationImage, pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0u, 1u, 0u, 1u), pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL,
				pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, queue->getFamilyIndex(), queue->getFamilyIndex()));

			// Transition the source image back, ready to be used as a SHADER_READ_ONLY image
			postBlitLayoutTransition.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_TRANSFER_READ_BIT, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, sourceImage,
				pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0u, 1u, 0u, 1u), pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL,
				pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, queue->getFamilyIndex(), queue->getFamilyIndex()));

			// Transition the destination image back, ready to be used as a SHADER_READ_ONLY image
			postBlitLayoutTransition.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT,
				destinationImage, pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0u, 1u, 0u, 1u), pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL,
				pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, queue->getFamilyIndex(), queue->getFamilyIndex()));
		}

		// Perform the blit:
		//		1. Transition the source and destination images ready for the blit
		//		2. Perform the blit using a linear filter
		//		3. Transition the source and destination images after the blit ready to be used as SHADER_READY_ONLY images
		{
			// Transition the source and destination images ready for the blit
			commandBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_TRANSFER_BIT, preBlitLayoutTransition);

			// Setup the blit region
			pvrvk::Offset3D sourceOffsets[] = { pvrvk::Offset3D(0, 0, 0), pvrvk::Offset3D(sourceImage->getWidth(), sourceImage->getHeight(), 1) };
			pvrvk::Offset3D destinationOffsets[] = { pvrvk::Offset3D(0, 0, 0), pvrvk::Offset3D(destinationImage->getWidth(), destinationImage->getHeight(), 1) };
			pvrvk::ImageBlit blitRegion = pvrvk::ImageBlit(pvrvk::ImageSubresourceLayers(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0, 0, 1), &sourceOffsets[0],
				pvrvk::ImageSubresourceLayers(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0, 0, 1), &destinationOffsets[0]);

			// Perform the blit using a linear filter
			commandBuffer->blitImage(
				sourceImage, destinationImage, &blitRegion, 1, pvrvk::Filter::e_LINEAR, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL);

			// Transition the source and destination images after the blit ready to be used as SHADER_READY_ONLY images
			commandBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_TRANSFER_BIT, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, postBlitLayoutTransition);
		}

		// Clear up the image layout transitions ready for the next downsample pass
		{
			preBlitLayoutTransition.clearAllBarriers();
			postBlitLayoutTransition.clearAllBarriers();
		}
	}
};

// Developed by Masaki Kawase, Bunkasha Games
// Used in DOUBLE-S.T.E.A.L. (aka Wreckless)
// From his GDC2003 Presentation: Frame Buffer Postprocessing Effects in  DOUBLE-S.T.E.A.L (Wreckless)
// Multiple iterations of fixed (per iteration) offset sampling
struct KawaseBlurPass
{
	pvrvk::GraphicsPipeline pipeline;
	pvrvk::DescriptorSetLayout descriptorSetLayout;
	pvrvk::PipelineLayout pipelineLayout;

	// 2 Descriptor sets are created. The descriptor sets are ping-ponged between for each Kawase blur iteration:
	//		iteration 0: (read 0 -> write 1), iteration 1: (read 1 -> write 0), iteration 2: (read 0 -> write 1) etc.
	pvr::Multi<pvrvk::DescriptorSet> descriptorSets[2];

	// Command buffers are recorded individually for each Kawase blur iteration
	pvr::Multi<pvrvk::SecondaryCommandBuffer> commandBuffers[MaxKawaseIteration];

	// Per iteration fixed size offset
	std::vector<uint32_t> blurKernels;

	// The number of Kawase blur iterations
	uint32_t blurIterations;

	// Push constants used for the per iteration Kawase blur configuration
	glm::vec2 pushConstants[MaxKawaseIteration][4];

	// The per swapchain blurred images.
	// Note that these blurred images are not new images and are instead pointing at one of the per swapchain ping-ponged image sets depending on the number of Kawase blur iterations.
	pvr::Multi<pvrvk::ImageView> blurredImages;

	glm::ivec2 blurFramebufferDimensions;

	/// <summary>Initialises the Kawase blur pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="commandPool">The command pool from which to allocate command buffers.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="blurRenderPass">The RenderPasses used for the Kawase blur passes</param>
	/// <param name="blurFramebufferDimensions">The dimensions to use for the Kawase blur iterations.</param>
	/// <param name="imageViews">The ping-ponged image views to use as sources/targets for the Kawase blur passes.</param>
	/// <param name="numImageViews">The number of ping-ponged image views to use as sources/targets for the
	/// Kawase blur passes - this must be 2.</param>
	/// <param name="sampler">The sampler object to use when sampling from the ping-ponged images during the Kawase blur
	/// passes.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	void init(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::CommandPool& commandPool, pvrvk::DescriptorPool& descriptorPool,
		pvrvk::RenderPass& blurRenderPass, const glm::ivec2& blurFramebufferDimensions, pvr::Multi<pvrvk::ImageView>* imageViews, uint32_t numImageViews, pvrvk::Sampler& sampler,
		pvrvk::PipelineCache& pipelineCache)
	{
		// The image views provided must be "ping-ponged" i.e. two of them which are swapped (in terms of read/write) each Kawase blur iteration
		// Using more than 2 image views would be inefficient
		assertion(numImageViews == 2);

		this->blurFramebufferDimensions = blurFramebufferDimensions;

		createDescriptorSetLayout(device);

		createPipeline(assetProvider, device, blurRenderPass, blurFramebufferDimensions, pipelineCache);

		// Create the ping-ponged descriptor sets
		createDescriptorSets(device, swapchain, descriptorPool, imageViews, sampler);

		// Pre-allocate all of the potential Kawase blur per swapchain command buffers
		// Recording of commands will take place later
		for (uint32_t i = 0; i < MaxKawaseIteration; ++i)
		{
			for (uint32_t j = 0; j < swapchain->getSwapchainLength(); ++j)
			{
				commandBuffers[i].add(commandPool->allocateSecondaryCommandBuffer());
			}
		}
	}

	/// <summary>Returns the blurred image for the given swapchain index.</summary>
	/// <param name="swapchainIndex">The swapchain index of the blurred image to retrieve.</param>
	/// <returns>The blurred image for the specified swapchain index.</returns>
	pvrvk::ImageView& getBlurredImage(uint32_t swapchainIndex)
	{
		return blurredImages[swapchainIndex];
	}

	/// <summary>Update the Kawase blur configuration.</summary>
	/// <param name="iterationsOffsets">The offsets to use in the Kawase blur passes.</param>
	/// <param name="numIterations">The number of iterations of Kawase blur passes.</param>
	/// <param name="imageViews">The set of ping-pong images.</param>
	/// <param name="numImageViews">The number of ping-pong images.</param>
	void updateConfig(uint32_t* iterationsOffsets, uint32_t numIterations, pvr::Multi<pvrvk::ImageView>* imageViews, uint32_t numImageViews)
	{
		// reset/clear the kernels and number of iterations
		blurKernels.clear();
		blurIterations = 0;

		// calculate texture sample offsets based on the number of iteratios and the kernel offset currently in use for the given iteration
		glm::vec2 pixelSize = glm::vec2(1.0f / blurFramebufferDimensions.x, 1.0f / blurFramebufferDimensions.y);

		glm::vec2 halfPixelSize = pixelSize / 2.0f;

		for (uint32_t i = 0; i < numIterations; ++i)
		{
			blurKernels.push_back(iterationsOffsets[i]);

			glm::vec2 dUV = pixelSize * glm::vec2(blurKernels[i], blurKernels[i]) + halfPixelSize;

			pushConstants[i][0] = glm::vec2(-dUV.x, dUV.y);
			pushConstants[i][1] = glm::vec2(dUV);
			pushConstants[i][2] = glm::vec2(dUV.x, -dUV.y);
			pushConstants[i][3] = glm::vec2(-dUV.x, -dUV.y);
		}
		blurIterations = numIterations;
		assertion(blurIterations <= MaxKawaseIteration);

		assertion(numImageViews == 2);

		blurredImages = imageViews[numIterations % 2];
	}

	/// <summary>Creates the Kawase blur descriptor set layout.</summary>
	/// <param name="device">The device to use for allocating the descriptor set layout.</param>
	void createDescriptorSetLayout(pvrvk::Device& device)
	{
		// Create the descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descriptorSetLayout = device->createDescriptorSetLayout(descSetInfo);

		// Push constants are used for uploading the texture sample offsets
		pvrvk::PushConstantRange pushConstantsRange;
		pushConstantsRange.setOffset(0);
		pushConstantsRange.setSize(static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec2) * 4));
		pushConstantsRange.setStageFlags(pvrvk::ShaderStageFlags::e_VERTEX_BIT);

		// Create the pipeline layout
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo.setDescSetLayout(0, descriptorSetLayout);
		pipeLayoutInfo.setPushConstantRange(0, pushConstantsRange);
		pipelineLayout = device->createPipelineLayout(pipeLayoutInfo);
	}

	/// <summary>Creates the Kawase blur descriptor sets.</summary>
	/// <param name="device">The device to use for allocating the descriptor sets.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="imageViews">The ping-ponged images to use for reading.</param>
	/// <param name="sampler">The sampler to use.</param>
	void createDescriptorSets(
		pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::DescriptorPool& descriptorPool, pvr::Multi<pvrvk::ImageView>* imageViews, pvrvk::Sampler& sampler)
	{
		std::vector<pvrvk::WriteDescriptorSet> writeDescSets;
		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			// The number of ping-pong images is fixed at 2
			for (uint32_t j = 0; j < 2; ++j)
			{
				descriptorSets[j].add(descriptorPool->allocateDescriptorSet(descriptorSetLayout));

				writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[j][i], 0)
											.setImageInfo(0, pvrvk::DescriptorImageInfo(imageViews[j][i], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
			}
		}

		device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
	}

	/// <summary>Creates the Kawase pipeline.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device to use for allocating the pipelines.</param>
	/// <param name="blurRenderPass">The RenderPass to use for the Kawase blur iterations.</param>
	/// <param name="blurFramebufferDimensions">The dimensions of the Framebuffers used in the Kawase blur iterations.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	void createPipeline(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::RenderPass& blurRenderPass, const glm::ivec2& blurFramebufferDimensions,
		pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.viewport.setViewportAndScissor(0, pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(blurFramebufferDimensions.x), static_cast<float>(blurFramebufferDimensions.y)),
			pvrvk::Rect2D(0, 0, blurFramebufferDimensions.x, blurFramebufferDimensions.y));

		pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_FRONT_BIT);
		pipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

		// Disable depth writing and depth testing
		pipelineInfo.depthStencil.enableDepthWrite(false);
		pipelineInfo.depthStencil.enableDepthTest(false);

		// Disable stencil testing
		pipelineInfo.depthStencil.enableStencilTest(false);

		pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

		// Load and create the Kawase blur shader modules
		pipelineInfo.vertexShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::KawaseVertSrcFile)->readToEnd<uint32_t>())));
		pipelineInfo.fragmentShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::KawaseFragSrcFile)->readToEnd<uint32_t>())));

		// We use attributeless rendering
		pipelineInfo.vertexInput.clear();
		pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);

		pipelineInfo.pipelineLayout = pipelineLayout;
		pipelineInfo.renderPass = blurRenderPass;
		pipelineInfo.subpass = 0;

		pipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
	}

	/// <summary>Records the commands required for the Kawase blur iterations based on the current configuration.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="blurFramebuffers">The ping-ponged Framebuffers to use in the Kawase blur iterations.</param>
	void recordCommands(uint32_t swapchainIndex, pvr::Multi<pvrvk::Framebuffer>* blurFramebuffers)
	{
		// Iterate through the Kawase blur iterations
		for (uint32_t i = 0; i < blurIterations; i++)
		{
			// Calculate the ping pong index based on the current iteration
			uint32_t pingPongIndex = i % 2;

			commandBuffers[i][swapchainIndex]->begin(blurFramebuffers[pingPongIndex][swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
			commandBuffers[i][swapchainIndex]->debugMarkerBeginEXT(pvr::strings::createFormatted("Kawase Blur - swapchain (%i): %i", swapchainIndex, i));
			commandBuffers[i][swapchainIndex]->pushConstants(
				pipelineLayout, pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec2) * 4), &pushConstants[i]);
			commandBuffers[i][swapchainIndex]->bindPipeline(pipeline);
			commandBuffers[i][swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipelineLayout, 0u, descriptorSets[pingPongIndex][swapchainIndex]);
			commandBuffers[i][swapchainIndex]->draw(0, 3);
			commandBuffers[i][swapchainIndex]->debugMarkerEndEXT();
			commandBuffers[i][swapchainIndex]->end();
		}
	}

	/// <summary>Records the command buffers into the given main rendering command buffer.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="commandBuffer">The main command buffer into which the pre-recorded command buffers will be recorded into.</param>
	/// <param name="queue">The queue to which the command buffer will be submitted.</param>
	/// <param name="blurRenderPass">The RenderPass to use.</param>
	/// <param name="blurFramebuffers">The ping-ponged Framebuffers to use in the Kawase blur iterations.</param>
	void recordCommandsToMainCommandBuffer(
		uint32_t swapchainIndex, pvrvk::CommandBuffer& commandBuffer, pvrvk::Queue& queue, pvrvk::RenderPass& blurRenderPass, pvr::Multi<pvrvk::Framebuffer>* blurFramebuffers)
	{
		pvrvk::ClearValue clearValue = pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 1.f);

		// Iterate through the current set of Kawase blur iterations
		for (uint32_t i = 0; i < blurIterations; i++)
		{
			uint32_t pingPongIndex = i % 2;

			commandBuffer->beginRenderPass(blurFramebuffers[pingPongIndex][swapchainIndex], blurRenderPass,
				pvrvk::Rect2D(0, 0, blurFramebuffers[pingPongIndex][swapchainIndex]->getDimensions().getWidth(),
					blurFramebuffers[pingPongIndex][swapchainIndex]->getDimensions().getHeight()),
				false, &clearValue, 1);
			commandBuffer->executeCommands(commandBuffers[i][swapchainIndex]);
			commandBuffer->endRenderPass();
		}
	}
};

// Developed by Marius Bjorge (ARM)
// Bandwidth-Efficient Rendering - siggraph2015-mmg-marius
// Filters images whilst Downsampling and Upsampling
struct DualFilterBlurPass
{
	// We only need (MaxDualFilterIteration - 1) images as the first image is an input to the blur pass
	// We also special case the final pass as this requires either a different pipeline or a different descriptor set/layout

	// Special cased final pass pipeline where the final upsample pass and compositing occurs in the same pipeline. This lets us avoid an extra write to memory/read from memory pass
	pvrvk::GraphicsPipeline finalPassPipeline;
	pvrvk::GraphicsPipeline finalPassBloomOnlyPipeline;

	// Pre allocated Up and Down sample pipelines for the iterations up to MaxDualFilterIteration
	pvrvk::GraphicsPipeline pipelines[MaxDualFilterIteration - 1];

	// The current set of pipelines in use for the currently selected configuration
	pvrvk::GraphicsPipeline currentPipelines[MaxDualFilterIteration];

	// The special cased final pass descriptor set layout
	pvrvk::DescriptorSetLayout finalPassDescriptorSetLayout;

	// The descriptor set layout used during the up and down sample passes
	pvrvk::DescriptorSetLayout descriptorSetLayout;

	// The special cased final pass pipeline layout
	pvrvk::PipelineLayout finalPassPipelineLayout;

	// The pipeline layout used during the up and down sample passes
	pvrvk::PipelineLayout pipelineLayout;

	// The special cased final pass per swapchain descriptor sets
	pvr::Multi<pvrvk::DescriptorSet> finalPassDescriptorSets;

	// The per swapchain descriptor sets for the up and down sample passes
	pvr::Multi<pvrvk::DescriptorSet> descriptorSets[MaxDualFilterIteration - 1];

	// The pre allocated framebuffers for the iterations up to MaxDualFilterIteration
	pvr::Multi<pvrvk::Framebuffer> framebuffers[MaxDualFilterIteration - 1];

	// The current set of framebuffers in use for the currently selected configuration
	pvr::Multi<pvrvk::Framebuffer> currentFramebuffers[MaxDualFilterIteration - 1];

	// The pre allocated image views for the iterations up to MaxDualFilterIteration
	pvr::Multi<pvrvk::ImageView> imageViews[MaxDualFilterIteration - 1];

	// The current set of image views in use for the currently selected configuration
	pvr::Multi<pvrvk::ImageView> currentImageViews[MaxDualFilterIteration - 1];

	// The set of command buffers where commands will be recorded for the current configuration
	pvr::Multi<pvrvk::SecondaryCommandBuffer> commandBuffers[MaxDualFilterIteration];

	// The framebuffer dimensions for the current configuration
	std::vector<glm::vec2> currentIterationDimensions;

	// The framebuffer inverse dimensions for the current configuration
	std::vector<glm::vec2> currentIterationInverseDimensions;

	// The full set of framebuffer dimensions
	std::vector<glm::vec2> maxIterationDimensions;

	// The full set of framebuffer inverse dimensions
	std::vector<glm::vec2> maxIterationInverseDimensions;

	// The number of Dual Filter iterations currently in use
	uint32_t blurIterations;

	// The current set of push constants for the current configuration
	glm::vec2 pushConstants[MaxDualFilterIteration][8];

	// The final full resolution framebuffer dimensions
	glm::ivec2 framebufferDimensions;

	// The color image format in use
	pvrvk::Format colorImageFormat;

	// The source image currently being blurred
	pvr::Multi<pvrvk::ImageView> currentImageToBlur;

	/// <summary>Initialises the Dual Filter blur.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="commandPool">The command pool from which to allocate command buffers.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="blurRenderPass">The RenderPass to use.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	/// <param name="colorImageFormat">The color image format to use for the Dual Filter blur.</param>
	/// <param name="framebufferDimensions">The full size resolution framebuffer dimensions.</param>
	/// <param name="sampler">The sampler object to use when sampling from the images during the Dual Filter blur passes.</param>
	/// <param name="utilityCommandBuffer">A command buffer to use for queueing up all initialisation commands. This command buffer will be submitted later by the main
	/// application.</param>
	/// <param name="onScreenRenderPass">The main RenderPass used for rendering to the screen.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	void init(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::CommandPool& commandPool, pvrvk::DescriptorPool& descriptorPool,
		pvrvk::RenderPass& blurRenderPass, pvr::utils::vma::Allocator& vmaAllocator, pvrvk::Format colorImageFormat, const glm::ivec2& framebufferDimensions,
		pvrvk::Sampler& sampler, pvrvk::CommandBuffer& utilityCommandBuffer, pvrvk::RenderPass& onScreenRenderPass, pvrvk::PipelineCache& pipelineCache)
	{
		this->colorImageFormat = colorImageFormat;
		this->framebufferDimensions = framebufferDimensions;
		this->blurIterations = -1;

		createBuffers(device, swapchain, vmaAllocator);
		createDescriptorSetLayouts(device);
		createDescriptorSets(swapchain, descriptorPool);

		// Calculate the maximum set of per iteration framebuffer dimensions
		// The maximum set will start from framebufferDimensions and allow for MaxDualFilterIteration. Note that this includes down and up sample passes
		calculateIterationDimensions();

		// Allocates the images used for each of the down/up sample passes
		allocatePingPongImages(device, swapchain, vmaAllocator);

		// Transition the images
		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			for (uint32_t j = 0; j < MaxDualFilterIteration - 1; j++)
			{
				pvr::utils::setImageLayout(imageViews[j][i]->getImage(), pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, utilityCommandBuffer);
			}
		}

		// Create the dual filter framebuffers
		createFramebuffers(device, swapchain, blurRenderPass);

		// Create the up and down sample pipelines
		createPipelines(assetProvider, device, blurRenderPass, onScreenRenderPass, pipelineCache);

		for (uint32_t i = 0; i < MaxDualFilterIteration; ++i)
		{
			for (uint32_t j = 0; j < swapchain->getSwapchainLength(); ++j)
			{
				commandBuffers[i].add(commandPool->allocateSecondaryCommandBuffer());
			}
		}
	}

	/// <summary>Creates any required buffers - The Dual Filter blur pass doesn't use any buffers.</summary>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	virtual void createBuffers(pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvr::utils::vma::Allocator& vmaAllocator) {}

	/// <summary>Returns the blurred image for the given swapchain index.</summary>
	/// <param name="swapchainIndex">The swapchain index of the blurred image to retrieve.</param>
	/// <returns>The blurred image for the specified swapchain index.</returns>
	pvrvk::ImageView& getBlurredImage(uint32_t swapchainIndex)
	{
		return currentImageViews[blurIterations - 1][swapchainIndex];
	}

	/// <summary>Update the Dual Filter blur configuration.</summary>
	/// <param name="numIterations">The number of iterations of Kawase blur passes.</param>
	/// <param name="initial">Specifies whether the Dual Filter pass is being initialised.</param>
	void updateConfig(uint32_t numIterations, bool initial = false)
	{
		// We only update the Dual Filter configuration if the number of iterations has actually been modified
		if (numIterations != blurIterations || initial)
		{
			blurIterations = numIterations;

			assertion(blurIterations % 2 == 0);

			// Calculate the Dual Filter iteration dimensions based on the current Dual Filter configuration
			getIterationDimensions(currentIterationDimensions, currentIterationInverseDimensions, blurIterations);

			// Configure the Dual Filter push constant values based on the current Dual Filter configuration
			configurePushConstants();

			// Configure the set of Dual Filter ping pong images based on the current Dual Filter configuration
			configurePingPongImages();

			// Configure the set of Framebuffers based on the current Dual Filter configuration
			configureFramebuffers();

			// Configure the set of up/down samples pipelines based on the current Dual Filter configuration
			configurePipelines();
		}
	}

	/// <summary>Configure the set of up/down samples pipelines based on the current Dual Filter configuration.</summary>
	void configurePipelines()
	{
		uint32_t index = 0;
		for (; index < blurIterations / 2; ++index)
		{
			currentPipelines[index] = pipelines[index];
		}

		for (uint32_t i = MaxDualFilterIteration - (blurIterations / 2); i < MaxDualFilterIteration - 1; ++i)
		{
			currentPipelines[index] = pipelines[i];
			index++;
		}
	}

	/// <summary>Configure the set of Framebuffers based on the current Dual Filter configuration.</summary>
	void configureFramebuffers()
	{
		for (uint32_t i = 0; i < MaxDualFilterIteration - 1; ++i)
		{
			currentFramebuffers[i].clear();
		}

		uint32_t index = 0;
		for (; index < blurIterations / 2; ++index)
		{
			currentFramebuffers[index] = framebuffers[index];
		}

		for (uint32_t i = MaxDualFilterIteration - (blurIterations / 2); i < MaxDualFilterIteration - 1; ++i)
		{
			currentFramebuffers[index] = framebuffers[i];
			index++;
		}
	}

	/// <summary>Configure the set of Dual Filter ping pong images based on the current Dual Filter configuration.</summary>
	virtual void configurePingPongImages()
	{
		for (uint32_t i = 0; i < MaxDualFilterIteration - 1; ++i)
		{
			currentImageViews[i].clear();
		}

		uint32_t index = 0;
		for (; index < blurIterations / 2; ++index)
		{
			currentImageViews[index] = imageViews[index];
		}

		for (uint32_t i = MaxDualFilterIteration - (blurIterations / 2); i < MaxDualFilterIteration - 1; ++i)
		{
			currentImageViews[index] = imageViews[i];
			index++;
		}
	}

	/// <summary>Calculate the full set of Dual Filter iteration dimensions.</summary>
	void calculateIterationDimensions()
	{
		maxIterationDimensions.resize(MaxDualFilterIteration);
		maxIterationInverseDimensions.resize(MaxDualFilterIteration);

		// Determine the dimensions and inverse dimensions for each iteration of the Dual Filter
		// If the original texture size is 800x600 and we are using a 4 pass Dual Filter then:
		//		Iteration 0: 400x300
		//		Iteration 1: 200x150
		//		Iteration 2: 400x300
		//		Iteration 3: 800x600
		glm::ivec2 dimension = glm::ivec2(framebufferDimensions.x, framebufferDimensions.y);

		// Calculate the dimensions and inverse dimensions top down
		for (uint32_t i = 0; i < MaxDualFilterIteration / 2; ++i)
		{
			dimension = glm::ivec2(glm::ceil(dimension.x / 2.0f), glm::ceil(dimension.y / 2.0f));
			maxIterationDimensions[i] = dimension;
			glm::vec2 inverseDimensions = glm::vec2(1.0f / dimension.x, 1.0f / dimension.y);
			maxIterationInverseDimensions[i] = inverseDimensions;
		}

		dimension = glm::ivec2(framebufferDimensions.x, framebufferDimensions.y);

		for (uint32_t i = MaxDualFilterIteration - 1; i >= MaxDualFilterIteration / 2; --i)
		{
			maxIterationDimensions[i] = dimension;
			glm::vec2 inverseDimensions = glm::vec2(1.0f / dimension.x, 1.0f / dimension.y);
			maxIterationInverseDimensions[i] = inverseDimensions;
			dimension = glm::ivec2(glm::ceil(dimension.x / 2.0f), glm::ceil(dimension.y / 2.0f));
		}
	}

	/// <summary>Calculate the Dual Filter iteration dimensions based on the current Dual Filter configuration.</summary>
	/// <param name="iterationDimensions">A returned list of dimensions to use for the current configuration.</param>
	/// <param name="iterationInverseDimensions">A returned list of inverse dimensions to use for the current configuration.</param>
	/// <param name="numIterations">Specifies the number of iterations used in the current configuration.</param>
	void getIterationDimensions(std::vector<glm::vec2>& iterationDimensions, std::vector<glm::vec2>& iterationInverseDimensions, uint32_t numIterations)
	{
		// Determine the dimensions and inverse dimensions for each iteration of the Dual Filter
		// If the original texture size is 800x600 and we are using a 4 pass Dual Filter then:
		//		Iteration 0: 400x300
		//		Iteration 1: 200x150
		//		Iteration 2: 400x300
		//		Iteration 3: 800x600
		iterationDimensions.clear();
		iterationInverseDimensions.clear();

		for (uint32_t i = 0; i < numIterations / 2; ++i)
		{
			iterationDimensions.push_back(maxIterationDimensions[i]);
			iterationInverseDimensions.push_back(maxIterationInverseDimensions[i]);
		}

		uint32_t index = MaxDualFilterIteration - (numIterations / 2);
		for (uint32_t i = numIterations / 2; i < numIterations; ++i)
		{
			iterationDimensions.push_back(maxIterationDimensions[index]);
			iterationInverseDimensions.push_back(maxIterationInverseDimensions[index]);
			index++;
		}
	}

	/// <summary>Allocates the images used for each of the down/up sample passes.</summary>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	virtual void allocatePingPongImages(pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvr::utils::vma::Allocator& vmaAllocator)
	{
		pvrvk::ImageUsageFlags imageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT;

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			for (uint32_t j = 0; j < MaxDualFilterIteration / 2; ++j)
			{
				pvrvk::Extent3D extent =
					pvrvk::Extent3D(static_cast<uint32_t>(maxIterationDimensions[j].x), static_cast<uint32_t>(maxIterationDimensions[j].y), static_cast<uint32_t>(1.0f));

				pvrvk::Image blurColorTexture = pvr::utils::createImage(device, pvrvk::ImageType::e_2D, colorImageFormat, extent, imageUsage, static_cast<pvrvk::ImageCreateFlags>(0),
					pvrvk::ImageLayersSize(), pvrvk::SampleCountFlags::e_1_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, &vmaAllocator);

				imageViews[j].add(device->createImageView(pvrvk::ImageViewCreateInfo(blurColorTexture)));
			}

			// We're able to reuse images between up/down sample passes. This can help us keep down the total number of images in flight
			uint32_t k = 0;
			for (uint32_t j = MaxDualFilterIteration / 2; j < MaxDualFilterIteration - 1; ++j)
			{
				uint32_t reuseIndex = (MaxDualFilterIteration / 2) - 1 - (k + 1);

				imageViews[j].add(imageViews[reuseIndex][i]);
				k++;
			}
		}
	}

	/// <summary>Allocates the framebuffers used for each of the down/up sample passes.</summary>
	/// <param name="device">The device from which the framebuffers will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="blurRenderPass">The RenderPass to use.</param>
	void createFramebuffers(pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::RenderPass& blurRenderPass)
	{
		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			pvrvk::FramebufferCreateInfo createInfo;
			createInfo.setRenderPass(blurRenderPass);

			for (uint32_t j = 0; j < MaxDualFilterIteration - 1; ++j)
			{
				createInfo.setDimensions(static_cast<uint32_t>(maxIterationDimensions[j].x), static_cast<uint32_t>(maxIterationDimensions[j].y));
				createInfo.setAttachment(0, imageViews[j][i]);
				framebuffers[j].add(device->createFramebuffer(createInfo));
			}
		}
	}

	/// <summary>Creates the descriptor set layouts used for the various up/down sample passes.</summary>
	/// <param name="device">The device from which the descriptor set layouts will be allocated.</param>
	virtual void createDescriptorSetLayouts(pvrvk::Device& device)
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descriptorSetLayout = device->createDescriptorSetLayout(descSetInfo);

		// The final pass uses a number of extra resources compared to the other passes
		descSetInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetInfo.setBinding(2, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		finalPassDescriptorSetLayout = device->createDescriptorSetLayout(descSetInfo);

		// Push constants are used for uploading the texture sample offsets
		pvrvk::PushConstantRange pushConstantsRange;
		pushConstantsRange.setOffset(0);
		pushConstantsRange.setSize(static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec2) * 8));
		pushConstantsRange.setStageFlags(pvrvk::ShaderStageFlags::e_VERTEX_BIT);

		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo.setDescSetLayout(0, descriptorSetLayout);
		pipeLayoutInfo.setPushConstantRange(0, pushConstantsRange);
		pipelineLayout = device->createPipelineLayout(pipeLayoutInfo);

		pipeLayoutInfo.setDescSetLayout(0, finalPassDescriptorSetLayout);
		finalPassPipelineLayout = device->createPipelineLayout(pipeLayoutInfo);
	}

	/// <summary>Creates the descriptor sets used for up/down sample passes.</summary>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	virtual void createDescriptorSets(pvrvk::Swapchain& swapchain, pvrvk::DescriptorPool& descriptorPool)
	{
		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			for (uint32_t j = 0; j < MaxDualFilterIteration - 1; ++j)
			{
				descriptorSets[j].add(descriptorPool->allocateDescriptorSet(descriptorSetLayout));
			}
			finalPassDescriptorSets.add(descriptorPool->allocateDescriptorSet(finalPassDescriptorSetLayout));
		}
	}

	/// <summary>Updates the descriptor sets used for each of the down/up sample passes.</summary>
	/// <param name="device">The device from which the updateDescriptorSets call will be piped.</param>
	/// <param name="swapchainIndex">The swapchain index of the descriptor set to update.</param>
	/// <param name="originalImageView">The original unblurred image view.</param>
	/// <param name="imageToBlur">The original unblurred luminance image view.</param>
	/// <param name="sampler">The sampler object to use when sampling from the images during the Dual Filter blur passes.</param>
	virtual void updateDescriptorSets(pvrvk::Device& device, uint32_t swapchainIndex, pvrvk::ImageView& originalImageView, pvrvk::ImageView& imageToBlur, pvrvk::Sampler& sampler)
	{
		// The source image to blur/apply bloom to
		currentImageToBlur[swapchainIndex] = imageToBlur;

		// First pass
		std::vector<pvrvk::WriteDescriptorSet> writeDescSets;
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[0][swapchainIndex], 0)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(currentImageToBlur[swapchainIndex], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		// Down/up sample passes
		for (uint32_t j = 1; j < blurIterations - 1; ++j)
		{
			writeDescSets.push_back(
				pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[j][swapchainIndex], 0)
					.setImageInfo(0, pvrvk::DescriptorImageInfo(currentImageViews[j - 1][swapchainIndex], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
		}

		// Final pass
		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, finalPassDescriptorSets[swapchainIndex], 0)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(currentImageViews[blurIterations - 2][swapchainIndex], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, finalPassDescriptorSets[swapchainIndex], 1)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(originalImageView, sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
	}

	/// <summary>Creates the pipelines.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device to use for allocating the pipelines.</param>
	/// <param name="blurRenderPass">The RenderPass to use.</param>
	/// <param name="onScreenRenderPass">The on screen RenderPass used for the final Dual Filter pass.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	virtual void createPipelines(
		pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::RenderPass& blurRenderPass, pvrvk::RenderPass& onScreenRenderPass, pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_FRONT_BIT);
		pipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

		// Disable depth writing and depth testing
		pipelineInfo.depthStencil.enableDepthWrite(false);
		pipelineInfo.depthStencil.enableDepthTest(false);

		// Disable stencil testing
		pipelineInfo.depthStencil.enableStencilTest(false);
		pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

		// We use attributeless rendering
		pipelineInfo.vertexInput.clear();
		pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);

		pipelineInfo.renderPass = blurRenderPass;
		pipelineInfo.subpass = 0;

		// Create the up and down sample pipelines using the appropriate dimensions and shaders
		for (uint32_t j = 0; j < MaxDualFilterIteration - 1; ++j)
		{
			// Downsample
			if (j < MaxDualFilterIteration / 2)
			{
				pipelineInfo.vertexShader.setShader(
					device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::DualFilterDownVertSrcFile)->readToEnd<uint32_t>())));
				pipelineInfo.fragmentShader.setShader(
					device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::DualFilterDownSampleFragSrcFile)->readToEnd<uint32_t>())));
			}
			// Upsample
			else
			{
				pipelineInfo.vertexShader.setShader(
					device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::DualFilterUpVertSrcFile)->readToEnd<uint32_t>())));
				pipelineInfo.fragmentShader.setShader(
					device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::DualFilterUpSampleFragSrcFile)->readToEnd<uint32_t>())));
			}

			pipelineInfo.viewport.setViewportAndScissor(0,
				pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(maxIterationDimensions[j].x), static_cast<float>(maxIterationDimensions[j].y)),
				pvrvk::Rect2D(0, 0, static_cast<uint32_t>(maxIterationDimensions[j].x), static_cast<uint32_t>(maxIterationDimensions[j].y)));

			pipelineInfo.pipelineLayout = pipelineLayout;
			pipelines[j] = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
			pipelineInfo.viewport.clear();
		}

		// Create the final Dual filter pass pipeline
		{
			pipelineInfo.renderPass = onScreenRenderPass;

			pipelineInfo.viewport.setViewportAndScissor(0,
				pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(maxIterationDimensions.back().x), static_cast<float>(maxIterationDimensions.back().y)),
				pvrvk::Rect2D(0, 0, static_cast<uint32_t>(maxIterationDimensions.back().x), static_cast<uint32_t>(maxIterationDimensions.back().y)));

			pipelineInfo.pipelineLayout = finalPassPipelineLayout;
			pipelineInfo.fragmentShader.setShader(
				device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::DualFilterUpSampleMergedFinalPassFragSrcFile)->readToEnd<uint32_t>())));
			finalPassPipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);

			// Enable bloom only
			int enabled = 1;
			pipelineInfo.fragmentShader.setShaderConstant(0, pvrvk::ShaderConstantInfo(0, &enabled, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Integer))));
			finalPassBloomOnlyPipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
		}
	}

	/// <summary>Configure the Dual Filter push constant values based on the current Dual Filter configuration.</summary>
	virtual void configurePushConstants()
	{
		for (uint32_t i = 0; i < blurIterations; i++)
		{
			// Downsample
			if (i < blurIterations / 2)
			{
				glm::vec2 pixelSize = glm::vec2(currentIterationInverseDimensions[i]);

				glm::vec2 halfPixelSize = pixelSize / 2.0f;
				glm::vec2 dUV = pixelSize + halfPixelSize;

				pushConstants[i][0] = glm::vec2(-dUV);
				pushConstants[i][1] = glm::vec2(dUV);
				pushConstants[i][2] = glm::vec2(dUV.x, -dUV.y);
				pushConstants[i][3] = glm::vec2(-dUV.x, dUV.y);
			}
			// Upsample
			else
			{
				glm::vec2 pixelSize = glm::vec2(currentIterationInverseDimensions[i]);

				glm::vec2 halfPixelSize = pixelSize / 2.0f;
				glm::vec2 dUV = pixelSize + halfPixelSize;

				pushConstants[i][0] = glm::vec2(-dUV.x * 2.0, 0.0);
				pushConstants[i][1] = glm::vec2(-dUV.x, dUV.y);
				pushConstants[i][2] = glm::vec2(0.0, dUV.y * 2.0);
				pushConstants[i][3] = glm::vec2(dUV.x, dUV.y);
				pushConstants[i][4] = glm::vec2(dUV.x * 2.0, 0.0);
				pushConstants[i][5] = glm::vec2(dUV.x, -dUV.y);
				pushConstants[i][6] = glm::vec2(0.0, -dUV.y * 2.0);
				pushConstants[i][7] = glm::vec2(-dUV.x, -dUV.y);
			}
		}
	}

	/// <summary>Records the commands required for the Dual Filter blur iterations based on the current configuration.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="onScreenFramebuffer">The on screen Framebuffer.</param>
	virtual void recordCommands(uint32_t swapchainIndex, pvrvk::Framebuffer& onScreenFramebuffer, bool renderBloomOnly)
	{
		for (uint32_t i = 0; i < blurIterations; i++)
		{
			// Special case the final Dual Filter iteration
			if (i == blurIterations - 1)
			{
				commandBuffers[i][swapchainIndex]->begin(onScreenFramebuffer, 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
				commandBuffers[i][swapchainIndex]->debugMarkerBeginEXT(pvr::strings::createFormatted("Dual Filter Blur (Final Pass) - swapchain (%i): %i", swapchainIndex, i));
				commandBuffers[i][swapchainIndex]->pushConstants(finalPassPipeline->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0,
					static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec2) * 8), &pushConstants[i]);

				if (renderBloomOnly)
				{
					commandBuffers[i][swapchainIndex]->bindPipeline(finalPassBloomOnlyPipeline);
				}
				else
				{
					commandBuffers[i][swapchainIndex]->bindPipeline(finalPassPipeline);
				}

				commandBuffers[i][swapchainIndex]->bindDescriptorSet(
					pvrvk::PipelineBindPoint::e_GRAPHICS, finalPassPipeline->getPipelineLayout(), 0u, finalPassDescriptorSets[swapchainIndex]);
			}
			// Down/Up sample passes
			else
			{
				commandBuffers[i][swapchainIndex]->begin(currentFramebuffers[i][swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
				commandBuffers[i][swapchainIndex]->debugMarkerBeginEXT(pvr::strings::createFormatted("Dual filter Blur - swapchain (%i): %i", swapchainIndex, i));
				commandBuffers[i][swapchainIndex]->pushConstants(currentPipelines[i]->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0,
					static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec2) * 8), &pushConstants[i]);
				commandBuffers[i][swapchainIndex]->bindPipeline(currentPipelines[i]);
				commandBuffers[i][swapchainIndex]->bindDescriptorSet(
					pvrvk::PipelineBindPoint::e_GRAPHICS, currentPipelines[i]->getPipelineLayout(), 0u, descriptorSets[i][swapchainIndex]);
			}

			commandBuffers[i][swapchainIndex]->draw(0, 3);
			commandBuffers[i][swapchainIndex]->debugMarkerEndEXT();
			commandBuffers[i][swapchainIndex]->end();
		}
	}

	/// <summary>Records the command buffers into the given main rendering command buffer.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="commandBuffer">The main command buffer into which the pre-recorded command buffers will be recorded into.</param>
	/// <param name="queue">The queue to which the command buffer will be submitted.</param>
	/// <param name="blurRenderPass">The RenderPass to use for down/up sampling.</param>
	/// <param name="onScreenRenderPass">The on screen RenderPass to use in the final Dual Filter pass.</param>
	/// <param name="onScreenFramebuffer">The on screen Framebuffers to use in the final Dual Filter pass.</param>
	/// <param name="onScreenClearValues">The clear values used in the final Dual Filter pass.</param>
	/// <param name="numOnScreenClearValues">The number of clear color values to use in the final Dual Filter pass.</param>
	virtual void recordCommandsToMainCommandBuffer(uint32_t swapchainIndex, pvrvk::CommandBuffer& commandBuffer, pvrvk::Queue& queue, pvrvk::RenderPass& blurRenderPass,
		pvrvk::RenderPass& onScreenRenderPass, pvrvk::Framebuffer& onScreenFramebuffer, pvrvk::ClearValue* onScreenClearValues, uint32_t numOnScreenClearValues)
	{
		pvrvk::ClearValue clearValue = pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 1.f);

		for (uint32_t i = 0; i < blurIterations; i++)
		{
			// Special Case the final Dual Filter pass
			if (i == blurIterations - 1)
			{
				commandBuffer->beginRenderPass(onScreenFramebuffer, onScreenRenderPass,
					pvrvk::Rect2D(0, 0, static_cast<uint32_t>(currentIterationDimensions[i].x), static_cast<uint32_t>(currentIterationDimensions[i].y)), false, onScreenClearValues,
					numOnScreenClearValues);
				commandBuffer->executeCommands(commandBuffers[i][swapchainIndex]);
			}
			// Down/Up sample passes
			else
			{
				commandBuffer->beginRenderPass(currentFramebuffers[i][swapchainIndex], blurRenderPass,
					pvrvk::Rect2D(0, 0, static_cast<uint32_t>(currentIterationDimensions[i].x), static_cast<uint32_t>(currentIterationDimensions[i].y)), false, &clearValue, 1);
				commandBuffer->executeCommands(commandBuffers[i][swapchainIndex]);
				commandBuffer->endRenderPass();
			}
		}
	}
};

// Presented in "Next Generation Post Processing In Call Of Duty Advanced Warfare" by Jorge Jimenez
// Filters whilst Downsampling and Upsampling
// Downsamples:
//	Used for preventing aliasing artifacts
//		A = downsample2(FullRes)
//		B = downsample2(A)
//		C = downsample2(B)
//		D = downsample2(C)
//		E = downsample2(D)
// Upsamples:
//	Used for image quality and smooth results
//	Upsampling progressively using bilinear filtering is equivalent to bi-quadratic b-spline filtering
//	We do the sum with the previous mip as we upscale
//		E' = E
//		D' = D + blur(E')
//		C' = C + blur(D')
//		B' = B + blur(C')
//		A' = A + blur(B')
//	The tent filter (3x3) - uses a radius parameter : 1 2 1
//												      2 4 2 * 1/16
//													  1 2 1
// Described here: http://www.iryoku.com/next-generation-post-processing-in-call-of-duty-advanced-warfare
// We make use of the DualFilterBlurPass as these passes share many similarities
struct DownAndTentFilterBlurPass : public DualFilterBlurPass
{
	// The descriptor set layout for the up sample iterations
	pvrvk::DescriptorSetLayout upSampleDescriptorSetLayout;

	// The pipeline layout for the up sample iterations
	pvrvk::PipelineLayout upSamplePipelineLayout;

	// A special cased up sample pipeline - we have one per potential Dual Filter iteration as the dimension used is important
	pvrvk::GraphicsPipeline firstUpSamplePipelines[MaxDualFilterIteration / 2 - 1];

	// Up sample pass image dependencies - these are dependencies on the downsampled mipmap i.e upsample D' is dependent on down sampled image D
	pvr::Multi<std::vector<pvrvk::ImageView> > upSampleIterationImageDependencies[MaxDualFilterIteration / 2 - 1];

	// Defines a scale to use for offsetting the tent offsets
	glm::vec2 tentScale;

	// Determines whether the images of the specifies color format can be used as BLIT_SRC and BLIT_DST
	bool supportsBlit;

	// Provides the capability of downsampling images using a linear filter
	BlitDownSamplePass blitDownSamplePass;

	/// <summary>Initialises the Dual Filter blur.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="commandPool">The command pool from which to allocate command buffers.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="blurRenderPass">The RenderPass to use for down/up sampling.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	/// <param name="colorImageFormat">The color image format to use for the Dual Filter blur.</param>
	/// <param name="framebufferDimensions">The full size resolution framebuffer dimensions.</param>
	/// <param name="sampler">The sampler object to use when sampling from the images during the Dual Filter blur passes.</param>
	/// <param name="utilityCommandBuffer">A command buffer to use for queueing up all initialisation commands. This command buffer will be submitted later by the main
	/// application.</param>
	/// <param name="onScreenRenderPass">The main RenderPass used for rendering to the screen.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	void init(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::CommandPool& commandPool, pvrvk::DescriptorPool& descriptorPool,
		pvrvk::RenderPass& blurRenderPass, pvr::utils::vma::Allocator& vmaAllocator, pvrvk::Format colorImageFormat, const glm::ivec2& framebufferDimensions,
		pvrvk::Sampler& sampler, pvrvk::CommandBuffer& utilityCommandBuffer, pvrvk::RenderPass& onScreenRenderPass, pvrvk::PipelineCache& pipelineCache, bool supportsBlit)
	{
		tentScale = glm::vec2(3.0f, 3.0f);

		this->supportsBlit = supportsBlit;

		DualFilterBlurPass::init(assetProvider, device, swapchain, commandPool, descriptorPool, blurRenderPass, vmaAllocator, colorImageFormat, framebufferDimensions, sampler,
			utilityCommandBuffer, onScreenRenderPass, pipelineCache);
	}

	/// <summary>Creates the descriptor sets used for up/down sample passes.</summary>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	void createDescriptorSets(pvrvk::Swapchain& swapchain, pvrvk::DescriptorPool& descriptorPool) override
	{
		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			for (uint32_t j = 0; j < MaxDualFilterIteration - 1; ++j)
			{
				if (j < MaxDualFilterIteration / 2 + 1)
				{
					descriptorSets[j].add(descriptorPool->allocateDescriptorSet(descriptorSetLayout));
				}
				else
				{
					descriptorSets[j].add(descriptorPool->allocateDescriptorSet(upSampleDescriptorSetLayout));
				}
			}
			finalPassDescriptorSets.add(descriptorPool->allocateDescriptorSet(finalPassDescriptorSetLayout));
		}
	}

	/// <summary>Allocates the images used for each of the down/up sample passes.</summary>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	void allocatePingPongImages(pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvr::utils::vma::Allocator& vmaAllocator) override
	{
		pvrvk::ImageUsageFlags imageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT;
		if (supportsBlit)
		{
			imageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_DST_BIT;
		}

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			for (uint32_t j = 0; j < MaxDualFilterIteration - 1; ++j)
			{
				pvrvk::Extent3D extent = pvrvk::Extent3D(static_cast<uint32_t>(maxIterationDimensions[j].x), static_cast<uint32_t>(maxIterationDimensions[j].y), 1);

				pvrvk::Image blurColorTexture = pvr::utils::createImage(device, pvrvk::ImageType::e_2D, colorImageFormat, extent, imageUsage, static_cast<pvrvk::ImageCreateFlags>(0),
					pvrvk::ImageLayersSize(), pvrvk::SampleCountFlags::e_1_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, &vmaAllocator);

				imageViews[j].add(device->createImageView(pvrvk::ImageViewCreateInfo(blurColorTexture)));
			}
		}
	}

	/// <summary>Configure the set of Tent Filter ping pong images based on the current Tent Filter configuration.</summary>
	void configurePingPongImages() override
	{
		for (uint32_t i = 0; i < MaxDualFilterIteration - 1; ++i)
		{
			currentImageViews[i].clear();
		}

		uint32_t index = 0;
		for (; index < blurIterations / 2; ++index)
		{
			currentImageViews[index] = imageViews[index];
		}

		for (uint32_t i = 0; i < (blurIterations / 2) - 1; ++i)
		{
			currentImageViews[index] = imageViews[(MaxDualFilterIteration - (blurIterations / 2)) + i];
			index++;
		}
	}

	/// <summary>Creates the descriptor set layouts used for up/down sample passes.</summary>
	/// <param name="device">The device from which the descriptor set layouts will be allocated.</param>
	virtual void createDescriptorSetLayouts(pvrvk::Device& device) override
	{
		// create the pre bloom descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descriptorSetLayout = device->createDescriptorSetLayout(descSetInfo);

		descSetInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		upSampleDescriptorSetLayout = device->createDescriptorSetLayout(descSetInfo);

		descSetInfo.setBinding(2, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetInfo.setBinding(3, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		finalPassDescriptorSetLayout = device->createDescriptorSetLayout(descSetInfo);

		// create the pipeline layouts
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo.setDescSetLayout(0, descriptorSetLayout);
		pipelineLayout = device->createPipelineLayout(pipeLayoutInfo);

		// Push constants are used for uploading the texture sample offsets
		pvrvk::PushConstantRange pushConstantsRange;
		pushConstantsRange.setOffset(0);
		pushConstantsRange.setStageFlags(pvrvk::ShaderStageFlags::e_VERTEX_BIT);
		pushConstantsRange.setSize(static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec2) * 8));
		pipeLayoutInfo.setPushConstantRange(0, pushConstantsRange);

		pipeLayoutInfo.setDescSetLayout(0, upSampleDescriptorSetLayout);
		upSamplePipelineLayout = device->createPipelineLayout(pipeLayoutInfo);

		pipeLayoutInfo.setDescSetLayout(0, finalPassDescriptorSetLayout);
		finalPassPipelineLayout = device->createPipelineLayout(pipeLayoutInfo);
	}

	/// <summary>Updates the descriptor sets used for each of the down/up sample passes.</summary>
	/// <param name="device">The device from which the updateDescriptorSets call will be piped.</param>
	/// <param name="swapchainIndex">The swapchain index of the descriptor set to update.</param>
	/// <param name="originalImageView">The original unblurred image view.</param>
	/// <param name="imageToBlur">The original unblurred luminance image view.</param>
	/// <param name="sampler">The sampler object to use when sampling from the images during the Tent Filter blur passes.</param>
	virtual void updateDescriptorSets(pvrvk::Device& device, uint32_t swapchainIndex, pvrvk::ImageView& originalImageView, pvrvk::ImageView& imageToBlur, pvrvk::Sampler& sampler) override
	{
		// The source image to blur/apply bloom to
		currentImageToBlur[swapchainIndex] = imageToBlur;

		std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

		// First pass
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[0][swapchainIndex], 0)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(currentImageToBlur[swapchainIndex], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		// downsample
		for (uint32_t i = 1; i < blurIterations / 2; ++i)
		{
			writeDescSets.push_back(
				pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[i][swapchainIndex], 0)
					.setImageInfo(0, pvrvk::DescriptorImageInfo(currentImageViews[i - 1][swapchainIndex], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
		}

		uint32_t upSampleDescriptorIndex = 0;

		// first up sample
		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[MaxDualFilterIteration / 2 + upSampleDescriptorIndex][swapchainIndex], 0)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(currentImageViews[blurIterations / 2 - 1][swapchainIndex], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
		upSampleDescriptorIndex++;

		uint32_t downsampledImageIndex = 1;

		// upsample
		for (uint32_t i = blurIterations / 2 + 1; i < blurIterations - 1; ++i)
		{
			writeDescSets.push_back(
				pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[MaxDualFilterIteration / 2 + upSampleDescriptorIndex][swapchainIndex], 0)
					.setImageInfo(0, pvrvk::DescriptorImageInfo(currentImageViews[i - 1][swapchainIndex], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

			writeDescSets.push_back(
				pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[MaxDualFilterIteration / 2 + upSampleDescriptorIndex][swapchainIndex], 1)
					.setImageInfo(0,
						pvrvk::DescriptorImageInfo(
							currentImageViews[blurIterations / 2 - 1 - downsampledImageIndex][swapchainIndex], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
			downsampledImageIndex++;
			upSampleDescriptorIndex++;
		}

		// Final pass
		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, finalPassDescriptorSets[swapchainIndex], 0)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(currentImageViews[blurIterations - 2][swapchainIndex], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, finalPassDescriptorSets[swapchainIndex], 1)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(currentImageViews[0][swapchainIndex], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, finalPassDescriptorSets[swapchainIndex], 2)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(originalImageView, sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);

		upSampleIterationImageDependencies->clear();
		downsampledImageIndex = 0;

		// The last entry into the downSampledImageViews array
		uint32_t lastDownSampledImageIndex = blurIterations / 2 - 1;
		// Ignore the last entry as this pass is special cased as the "first up sample"
		uint32_t currentDownSampledImageIndex = lastDownSampledImageIndex - 1;

		for (uint32_t i = blurIterations / 2 + 1; i < blurIterations; ++i)
		{
			upSampleIterationImageDependencies[downsampledImageIndex][swapchainIndex].push_back(currentImageViews[currentDownSampledImageIndex][swapchainIndex]);
			currentDownSampledImageIndex--;
			downsampledImageIndex++;
		}
	}

	/// <summary>Creates the pipelines.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device to use for allocating the pipelines.</param>
	/// <param name="blurRenderPass">The RenderPass to use for down/up sampling.</param>
	/// <param name="onScreenRenderPass">The on screen RenderPass used for the final Tent Filter pass.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	virtual void createPipelines(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::RenderPass& blurRenderPass, pvrvk::RenderPass& onScreenRenderPass,
		pvrvk::PipelineCache& pipelineCache) override
	{
		pvrvk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_FRONT_BIT);
		pipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

		// Disable depth writing and depth testing
		pipelineInfo.depthStencil.enableDepthWrite(false);
		pipelineInfo.depthStencil.enableDepthTest(false);

		// Disable stencil testing
		pipelineInfo.depthStencil.enableStencilTest(false);

		pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

		// We use attributeless rendering
		pipelineInfo.vertexInput.clear();
		pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);

		pipelineInfo.renderPass = blurRenderPass;
		pipelineInfo.subpass = 0;

		uint32_t upSamplePipelineIndex = 0;

		// Create the up and down sample pipelines using the appropriate dimensions and shaders
		for (uint32_t i = 0; i < MaxDualFilterIteration - 1; ++i)
		{
			// Downsample
			if (i < MaxDualFilterIteration / 2)
			{
				pipelineInfo.vertexShader.setShader(
					device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::Downsample2x2VertSrcFile)->readToEnd<uint32_t>())));
				pipelineInfo.fragmentShader.setShader(
					device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::Downsample2x2FragSrcFile)->readToEnd<uint32_t>())));
				pipelineInfo.pipelineLayout = pipelineLayout;
			}
			// Upsample
			else
			{
				pipelineInfo.vertexShader.setShader(
					device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::TentFilterUpSampleVertSrcFile)->readToEnd<uint32_t>())));
				pipelineInfo.fragmentShader.setShader(
					device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::TentFilterUpSampleFragSrcFile)->readToEnd<uint32_t>())));
				pipelineInfo.pipelineLayout = upSamplePipelineLayout;
			}

			pipelineInfo.viewport.setViewportAndScissor(0,
				pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(maxIterationDimensions[i].x), static_cast<float>(maxIterationDimensions[i].y)),
				pvrvk::Rect2D(0, 0, static_cast<uint32_t>(maxIterationDimensions[i].x), static_cast<uint32_t>(maxIterationDimensions[i].y)));

			pipelines[i] = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
			pipelineInfo.viewport.clear();

			// Special cased first up sample pipeline. This may seem strange but we need to handle first up sample pipelines
			// for various numbers of iterations as they will have different dimensions
			if (i >= MaxDualFilterIteration / 2)
			{
				// Note we use the Downsample2x2VertSrcFile as it's a simple vertex shader just passing texture coordinates through
				pipelineInfo.vertexShader.setShader(
					device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::Downsample2x2VertSrcFile)->readToEnd<uint32_t>())));
				pipelineInfo.fragmentShader.setShader(
					device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::TentFilterFirstUpSampleFragSrcFile)->readToEnd<uint32_t>())));
				pipelineInfo.pipelineLayout = pipelineLayout;

				pipelineInfo.viewport.setViewportAndScissor(0,
					pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(maxIterationDimensions[i].x), static_cast<float>(maxIterationDimensions[i].y)),
					pvrvk::Rect2D(0, 0, static_cast<uint32_t>(maxIterationDimensions[i].x), static_cast<uint32_t>(maxIterationDimensions[i].y)));

				firstUpSamplePipelines[upSamplePipelineIndex] = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
				pipelineInfo.viewport.clear();
				upSamplePipelineIndex++;
			}
		}

		// Create the final Tent filter pass pipeline
		{
			pipelineInfo.renderPass = onScreenRenderPass;

			pipelineInfo.viewport.setViewportAndScissor(0,
				pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(maxIterationDimensions.back().x), static_cast<float>(maxIterationDimensions.back().y)),
				pvrvk::Rect2D(0, 0, static_cast<uint32_t>(maxIterationDimensions.back().x), static_cast<uint32_t>(maxIterationDimensions.back().y)));

			pipelineInfo.pipelineLayout = finalPassPipelineLayout;
			pipelineInfo.vertexShader.setShader(
				device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::TentFilterUpSampleVertSrcFile)->readToEnd<uint32_t>())));
			pipelineInfo.fragmentShader.setShader(
				device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::TentFilterUpSampleMergedFinalPassFragSrcFile)->readToEnd<uint32_t>())));
			finalPassPipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);

			// Enable bloom only
			int enabled = 1;
			pipelineInfo.fragmentShader.setShaderConstant(0, pvrvk::ShaderConstantInfo(0, &enabled, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Integer))));
			finalPassBloomOnlyPipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
		}
	}

	/// <summary>Configure the Tent Filter push constant values based on the current Tent Filter configuration.</summary>
	virtual void configurePushConstants() override
	{
		const glm::vec2 offsets[8] = { glm::vec2(-1.0, 1.0), glm::vec2(0.0, 1.0), glm::vec2(1.0, 1.0), glm::vec2(1.0, 0.0), glm::vec2(1.0, -1.0), glm::vec2(0.0, -1.0),
			glm::vec2(-1.0, -1.0), glm::vec2(-1.0, 0.0) };

		for (uint32_t i = 0; i < blurIterations; i++)
		{
			pushConstants[i][0] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[0] * tentScale;
			pushConstants[i][1] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[1] * tentScale;
			pushConstants[i][2] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[2] * tentScale;
			pushConstants[i][3] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[3] * tentScale;
			pushConstants[i][4] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[4] * tentScale;
			pushConstants[i][5] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[5] * tentScale;
			pushConstants[i][6] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[6] * tentScale;
			pushConstants[i][7] = glm::vec2(1.0f / (currentIterationDimensions[i].x * 0.5), 1.0f / (currentIterationDimensions[i].y * 0.5)) * offsets[7] * tentScale;
		}
	}

	using DualFilterBlurPass::recordCommands;
	/// <summary>Records the commands required for the Tent Filter blur iterations based on the current configuration.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="onScreenFramebuffer">The on screen Framebuffer.</param>
	/// <param name="queue">The queue being used.</param>
	/// <param name="sourceImageView">The source image to blur.</param>
	void recordCommands(uint32_t swapchainIndex, pvrvk::Framebuffer& onScreenFramebuffer, bool renderBloomOnly, pvrvk::Queue& queue, pvrvk::ImageView& sourceImageView)
	{
		uint32_t index = 0;

		uint32_t i = 0;

		if (supportsBlit)
		{
			commandBuffers[0][swapchainIndex]->begin();

			// Downsample using vkCmdBlitImage
			for (; i < blurIterations / 2; i++)
			{
				pvrvk::Image sourceImage = sourceImageView->getImage();

				if (i > 0)
				{
					sourceImage = currentImageViews[i - 1][swapchainIndex]->getImage();
				}

				blitDownSamplePass.recordCommands(commandBuffers[0][swapchainIndex], sourceImage, currentImageViews[i][swapchainIndex]->getImage(), queue);
			}

			commandBuffers[0][swapchainIndex]->end();
		}
		else
		{
			// Perform downsamples using separate passes
			for (; i < blurIterations / 2; i++)
			{
				commandBuffers[i][swapchainIndex]->begin(currentFramebuffers[i][swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
				commandBuffers[i][swapchainIndex]->debugMarkerBeginEXT(pvr::strings::createFormatted("Tent Blur (Down Sample Pass) - swapchain (%i): %i", swapchainIndex, i));
				commandBuffers[i][swapchainIndex]->bindPipeline(currentPipelines[i]);
				commandBuffers[i][swapchainIndex]->bindDescriptorSet(
					pvrvk::PipelineBindPoint::e_GRAPHICS, currentPipelines[i]->getPipelineLayout(), 0u, descriptorSets[i][swapchainIndex]);
				commandBuffers[i][swapchainIndex]->draw(0, 3);
				commandBuffers[i][swapchainIndex]->debugMarkerEndEXT();
				commandBuffers[i][swapchainIndex]->end();
			}
		}

		if (blurIterations > 2)
		{
			// Handle the first up sample pass
			commandBuffers[i][swapchainIndex]->begin(currentFramebuffers[i][swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
			commandBuffers[i][swapchainIndex]->debugMarkerBeginEXT(pvr::strings::createFormatted("Tent Blur (First Up Sample Pass) - swapchain (%i): %i", swapchainIndex, 0));
			commandBuffers[i][swapchainIndex]->bindPipeline(firstUpSamplePipelines[MaxDualFilterIteration / 2 - i]);
			commandBuffers[i][swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, firstUpSamplePipelines[MaxDualFilterIteration / 2 - i]->getPipelineLayout(),
				0u, descriptorSets[(MaxDualFilterIteration / 2) + index][swapchainIndex]);
			commandBuffers[i][swapchainIndex]->draw(0, 3);
			commandBuffers[i][swapchainIndex]->debugMarkerEndEXT();
			commandBuffers[i][swapchainIndex]->end();
			index++;
			i++;

			// Handle the other up sample passes
			for (; i < blurIterations - 1; i++)
			{
				commandBuffers[i][swapchainIndex]->begin(currentFramebuffers[i][swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
				commandBuffers[i][swapchainIndex]->debugMarkerBeginEXT(pvr::strings::createFormatted("Tent Blur (Up Sample Pass) - swapchain (%i): %i", swapchainIndex, i));
				commandBuffers[i][swapchainIndex]->bindPipeline(currentPipelines[i]);
				commandBuffers[i][swapchainIndex]->pushConstants(currentPipelines[i]->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0,
					static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec2) * 8), &pushConstants[i]);
				commandBuffers[i][swapchainIndex]->bindDescriptorSet(
					pvrvk::PipelineBindPoint::e_GRAPHICS, currentPipelines[i]->getPipelineLayout(), 0u, descriptorSets[(MaxDualFilterIteration / 2) + index][swapchainIndex]);
				commandBuffers[i][swapchainIndex]->draw(0, 3);
				commandBuffers[i][swapchainIndex]->debugMarkerEndEXT();
				commandBuffers[i][swapchainIndex]->end();
				index++;
			}
		}

		// Special case the final up sample pass
		commandBuffers[i][swapchainIndex]->begin(onScreenFramebuffer, 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
		commandBuffers[i][swapchainIndex]->debugMarkerBeginEXT(pvr::strings::createFormatted("Tent Blur (Final Pass) - swapchain (%i): %i", swapchainIndex, i));
		if (renderBloomOnly)
		{
			commandBuffers[i][swapchainIndex]->bindPipeline(finalPassBloomOnlyPipeline);
		}
		else
		{
			commandBuffers[i][swapchainIndex]->bindPipeline(finalPassPipeline);
		}
		commandBuffers[i][swapchainIndex]->pushConstants(
			finalPassPipeline->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec2) * 8), &pushConstants[i]);
		commandBuffers[i][swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, finalPassPipeline->getPipelineLayout(), 0u, finalPassDescriptorSets[swapchainIndex]);
		commandBuffers[i][swapchainIndex]->draw(0, 3);
		commandBuffers[i][swapchainIndex]->debugMarkerEndEXT();
		commandBuffers[i][swapchainIndex]->end();
	}

	/// <summary>Records the command buffers into the given main rendering command buffer.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="commandBuffer">The main command buffer into which the pre-recorded command buffers will be recorded into.</param>
	/// <param name="queue">The queue to which the command buffer will be submitted.</param>
	/// <param name="blurRenderPass">The RenderPass to use for down/up sampling.</param>
	/// <param name="onScreenRenderPass">The on screen RenderPass to use in the final Tent Filter pass.</param>
	/// <param name="onScreenFramebuffer">The on screen Framebuffers to use in the final Tent Filter pass.</param>
	/// <param name="onScreenClearValues">The clear values used in the final Tent Filter pass.</param>
	/// <param name="numOnScreenClearValues">The number of clear color values to use in the final Tent Filter pass.</param>
	void recordCommandsToMainCommandBuffer(uint32_t swapchainIndex, pvrvk::CommandBuffer& commandBuffer, pvrvk::Queue& queue, pvrvk::RenderPass& blurRenderPass,
		pvrvk::RenderPass& onScreenRenderPass, pvrvk::Framebuffer& onScreenFramebuffer, pvrvk::ClearValue* onScreenClearValues, uint32_t numOnScreenClearValues) override
	{
		pvrvk::ClearValue clearValue = pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 1.f);

		pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL;
		pvrvk::ImageLayout destinationImageLayout = pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL;

		uint32_t i = 0;

		if (supportsBlit)
		{
			// Perform the downsamples using vkCmdBlitImage
			commandBuffer->executeCommands(commandBuffers[0][swapchainIndex]);

			// skip to the up samples passes
			i = blurIterations / 2;
		}
		uint32_t upSampleIndex = 0;
		for (; i < blurIterations; i++)
		{
			// Take care of the extra image dependencies the up sample passes require
			if (i > blurIterations / 2)
			{
				for (uint32_t j = 0; j < upSampleIterationImageDependencies[upSampleIndex][swapchainIndex].size(); ++j)
				{
					pvrvk::MemoryBarrierSet layoutTransition;
					layoutTransition.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT,
						upSampleIterationImageDependencies[upSampleIndex][swapchainIndex][j]->getImage(),
						pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0u, 1u, 0u, 1u), sourceImageLayout, destinationImageLayout, queue->getFamilyIndex(),
						queue->getFamilyIndex()));
					commandBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, layoutTransition);
				}

				upSampleIndex++;
			}

			// Handle the special cased final up sample whereby we merge the upsample blur with the tonemapping pass to save on write/reads
			if (i == blurIterations - 1)
			{
				commandBuffer->beginRenderPass(onScreenFramebuffer, onScreenRenderPass,
					pvrvk::Rect2D(0, 0, static_cast<uint32_t>(currentIterationDimensions[i].x), static_cast<uint32_t>(currentIterationDimensions[i].y)), false, onScreenClearValues,
					numOnScreenClearValues);
				commandBuffer->executeCommands(commandBuffers[i][swapchainIndex]);
			}
			else
			{
				commandBuffer->beginRenderPass(currentFramebuffers[i][swapchainIndex], blurRenderPass,
					pvrvk::Rect2D(0, 0, static_cast<uint32_t>(currentIterationDimensions[i].x), static_cast<uint32_t>(currentIterationDimensions[i].y)), false, &clearValue, 1);
				commandBuffer->executeCommands(commandBuffers[i][swapchainIndex]);
				commandBuffer->endRenderPass();
			}
		}
	}
};

// A Gaussian Blur Pass
struct GaussianBlurPass
{
	// Horizontal and Vertical graphics pipelines
	pvrvk::GraphicsPipeline horizontalPipeline;
	pvrvk::GraphicsPipeline verticalPipeline;

	pvrvk::DescriptorSetLayout descriptorSetLayout;
	pvrvk::PipelineLayout pipelineLayout;

	// Horizontal and Vertical descriptor sets
	pvr::Multi<pvrvk::DescriptorSet> horizontalDescriptorSets;
	pvr::Multi<pvrvk::DescriptorSet> verticalDescriptorSets;

	// Horizontal and Vertical command buffers
	pvr::Multi<pvrvk::SecondaryCommandBuffer> horizontalBlurCommandBuffers;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> verticalBlurCommandBuffers;

	// Gaussian offsets and weights
	std::vector<double> gaussianOffsets;
	std::vector<double> gaussianWeights;

	std::vector<float> gaussianOffsetsFloats;
	std::vector<float> gaussianWeightsFloats;
	// stores details regarding the Gaussian blur configuration currently in use
	glm::vec4 blurConfig;
	// the size of the gaussian kernel currently in use
	uint32_t kernelSize;
	pvr::Multi<pvrvk::Buffer> bloomConfigBuffers;
	uint32_t ssboPerSwapchainSize;
	pvr::Multi<pvrvk::ImageView> blurredImages;

	/// <summary>Initialises the Gaussian blur pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="commandPool">The command pool from which to allocate command buffers.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	/// <param name="blurRenderPass">The RenderPass to use.</param>
	/// <param name="blurFramebufferDimensions">The dimensions used for the blur.</param>
	/// <param name="horizontalBlurImageViews">A set of images (per swapchain) to use as the destination for a horizontal blur.</param>
	/// <param name="verticalBlurImageViews">A set of images (per swapchain) to use as the destination for a vertical blur.</param>
	/// <param name="sampler">A bilinear sampler object.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	void init(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::CommandPool& commandPool, pvrvk::DescriptorPool& descriptorPool,
		pvr::utils::vma::Allocator& vmaAllocator, pvrvk::RenderPass& blurRenderPass, const glm::ivec2& blurFramebufferDimensions,
		pvr::Multi<pvrvk::ImageView>& horizontalBlurImageViews, pvr::Multi<pvrvk::ImageView>& verticalBlurImageViews, pvrvk::Sampler& sampler, pvrvk::PipelineCache& pipelineCache)
	{
		createBuffers(device, swapchain, vmaAllocator);

		blurConfig = glm::vec4(1.0f / blurFramebufferDimensions.x, 1.0f / blurFramebufferDimensions.y, 0.0f, 0.0f);

		createDescriptorSetLayout(device);

		createPipelines(assetProvider, device, blurRenderPass, blurFramebufferDimensions, pipelineCache);
		createDescriptorSets(device, swapchain, descriptorPool, horizontalBlurImageViews, verticalBlurImageViews, sampler);

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			horizontalBlurCommandBuffers.add(commandPool->allocateSecondaryCommandBuffer());
			verticalBlurCommandBuffers.add(commandPool->allocateSecondaryCommandBuffer());
		}

		blurredImages = verticalBlurImageViews;
	}

	/// <summary>Updates the kernel configuration currently in use.</summary>
	/// <param name="kernelSizeConfig">The kernel size.</param>
	/// <param name="useLinearOptimisation">Specifies whether the offsets and weights used for the current kernel size should be modified
	/// (optimised) for use with linear sampling.</param>
	/// <param name="truncateCoefficients">Provides us with an efficient and convenient mechanism for achieving blurs approximating
	/// blurs with larger kernel sizes Note that using making use "truncateCoefficients" can result in a loss of a low value high
	/// precision tail in the blur. Depending on the scene/image being blurred the effect of using "truncateCoefficients" may not be
	/// particularly visible however in some cases it may be required that high precision low value tails are present - it may be artistically
	/// required that no loss is observed. Using a traditional 8 bit per channel color buffer we could very well get away with the loss of the
	/// very low value tails. Using a HDR 16 bit per channel color buffer we may get away with ignoring negligible coefficients depending on
	/// the minimum coefficient we deem to be non-negligible. For the most accurate of blur then we would recommened against its use however
	/// if speed is of the utmost importance and if you can get away with the quality degradation then ignoring negligible coefficients
	/// is a great way to reduce the number of texture samples. We will leave this decision to the reader..</param>
	virtual void updateKernelConfig(uint32_t kernelSizeConfig, bool useLinearOptimisation, bool truncateCoefficients)
	{
		kernelSize = kernelSizeConfig;
		updateGaussianWeightsAndOffsets(kernelSize, useLinearOptimisation, truncateCoefficients, gaussianOffsets, gaussianWeights, gaussianOffsetsFloats, gaussianWeightsFloats);
		blurConfig.z = static_cast<float>(gaussianOffsetsFloats.size());
	}

	/// <summary>Updates the kernel buffers for the specified swapchain index.</summary>
	/// <param name="swapchainIndex">The swapchain index of the buffer to update.</param>
	virtual void updateKernelBuffer(uint32_t swapchainIndex)
	{
		void* memory = bloomConfigBuffers[swapchainIndex]->getDeviceMemory()->getMappedData();

		memcpy(static_cast<char*>(memory), &blurConfig, pvr::getSize(pvr::GpuDatatypes::vec4));
		memcpy(static_cast<char*>(memory) + pvr::getSize(pvr::GpuDatatypes::vec4), gaussianWeightsFloats.data(), pvr::getSize(pvr::GpuDatatypes::Float) * MaxGaussianHalfKernel);
		memcpy(static_cast<char*>(memory) + pvr::getSize(pvr::GpuDatatypes::vec4) + pvr::getSize(pvr::GpuDatatypes::Float) * MaxGaussianHalfKernel, gaussianOffsetsFloats.data(),
			pvr::getSize(pvr::GpuDatatypes::Float) * MaxGaussianHalfKernel);

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(bloomConfigBuffers[swapchainIndex]->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			bloomConfigBuffers[swapchainIndex]->getDeviceMemory()->flushRange();
		}
	}

	/// <summary>Creates any required buffers.</summary>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	virtual void createBuffers(pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvr::utils::vma::Allocator& vmaAllocator)
	{
		ssboPerSwapchainSize = static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4) + pvr::getSize(pvr::GpuDatatypes::Float) * MaxGaussianHalfKernel * 2);

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			bloomConfigBuffers[i] = pvr::utils::createBuffer(device, ssboPerSwapchainSize, pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT,
				pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT, &vmaAllocator);
		}
	}

	/// <summary>Creates the descriptor set layouts used for up/down sample passes.</summary>
	/// <param name="device">The device from which the descriptor set layouts will be allocated.</param>
	virtual void createDescriptorSetLayout(pvrvk::Device& device)
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetInfo.setBinding(1, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descriptorSetLayout = device->createDescriptorSetLayout(descSetInfo);

		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo.setDescSetLayout(0, descriptorSetLayout);
		pipelineLayout = device->createPipelineLayout(pipeLayoutInfo);
	}

	/// <summary>Creates the descriptor sets.</summary>
	/// <param name="device">The device to use for allocating the descriptor sets.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="horizontalBlurImageViews">A set of images (per swapchain) to use as the destination for a horizontal blur.</param>
	/// <param name="verticalBlurImageViews">A set of images (per swapchain) to use as the destination for a vertical blur (this is also the destination for a downsample or at
	/// least is the source image to do a horizontal blur).</param>
	/// <param name="sampler">A bilinear sampler object.</param>
	virtual void createDescriptorSets(pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::DescriptorPool& descriptorPool,
		pvr::Multi<pvrvk::ImageView>& horizontalBlurImageViews, pvr::Multi<pvrvk::ImageView>& verticalBlurImageViews, pvrvk::Sampler& sampler)
	{
		std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			// Descriptor sets for the Horizontal Blur Pass
			horizontalDescriptorSets.add(descriptorPool->allocateDescriptorSet(descriptorSetLayout));
			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, horizontalDescriptorSets[i], 0)
										.setImageInfo(0, pvrvk::DescriptorImageInfo(verticalBlurImageViews[i], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, horizontalDescriptorSets[i], 1)
										.setBufferInfo(0, pvrvk::DescriptorBufferInfo(bloomConfigBuffers[i], 0, ssboPerSwapchainSize)));

			// Descriptor sets for the Vertical Blur Pass
			verticalDescriptorSets.add(descriptorPool->allocateDescriptorSet(descriptorSetLayout));
			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, verticalDescriptorSets[i], 0)
										.setImageInfo(0, pvrvk::DescriptorImageInfo(horizontalBlurImageViews[i], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, verticalDescriptorSets[i], 1)
										.setBufferInfo(0, pvrvk::DescriptorBufferInfo(bloomConfigBuffers[i], 0, ssboPerSwapchainSize)));
		}

		device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
	}

	/// <summary>Creates the pipelines.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device to use for allocating the pipelines.</param>
	/// <param name="renderPass">The RenderPass to use for gaussian blur.</param>
	/// <param name="blurFramebufferDimensions">The dimensions of the Framebuffers used in the Gaussian blur iterations.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	virtual void createPipelines(
		pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::RenderPass& renderPass, const glm::ivec2& blurFramebufferDimensions, pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.viewport.setViewportAndScissor(0, pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(blurFramebufferDimensions.x), static_cast<float>(blurFramebufferDimensions.y)),
			pvrvk::Rect2D(0, 0, blurFramebufferDimensions.x, blurFramebufferDimensions.y));

		pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_FRONT_BIT);
		pipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

		// disable depth writing and depth testing
		pipelineInfo.depthStencil.enableDepthWrite(false);
		pipelineInfo.depthStencil.enableDepthTest(false);

		// disable stencil testing
		pipelineInfo.depthStencil.enableStencilTest(false);

		pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

		pipelineInfo.vertexInput.clear();
		pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);

		pipelineInfo.pipelineLayout = pipelineLayout;

		// renderpass/subpass
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;

		pipelineInfo.vertexShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::GaussianVertSrcFile)->readToEnd<uint32_t>())));
		pipelineInfo.fragmentShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::GaussianHorizontalFragSrcFile)->readToEnd<uint32_t>())));

		horizontalPipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);

		pipelineInfo.vertexShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::GaussianVertSrcFile)->readToEnd<uint32_t>())));
		pipelineInfo.fragmentShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::GaussianVerticalFragSrcFile)->readToEnd<uint32_t>())));
		verticalPipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
	}

	/// <summary>Records the commands required for the Gaussian blur.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="horizontalBlurFramebuffers">The framebuffers to use in the horizontal blur pass.</param>
	/// <param name="verticalBlurFramebuffers">The framebuffers to use in the vertical blur pass.</param>
	virtual void recordCommands(uint32_t swapchainIndex, pvr::Multi<pvrvk::Framebuffer>& horizontalBlurFramebuffers, pvr::Multi<pvrvk::Framebuffer>& verticalBlurFramebuffers)
	{
		// Record the commands to use for carrying out the horizontal gaussian blur pass
		{
			horizontalBlurCommandBuffers[swapchainIndex]->begin(horizontalBlurFramebuffers[swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

			horizontalBlurCommandBuffers[swapchainIndex]->debugMarkerBeginEXT(pvr::strings::createFormatted("Gaussian Blur (horizontal) - swapchain (%i)", swapchainIndex));

			horizontalBlurCommandBuffers[swapchainIndex]->bindPipeline(horizontalPipeline);
			horizontalBlurCommandBuffers[swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipelineLayout, 0u, horizontalDescriptorSets[swapchainIndex]);

			horizontalBlurCommandBuffers[swapchainIndex]->draw(0, 3);
			horizontalBlurCommandBuffers[swapchainIndex]->debugMarkerEndEXT();
			horizontalBlurCommandBuffers[swapchainIndex]->end();
		}

		// Record the commands to use for carrying out the vertical gaussian blur pass
		{
			verticalBlurCommandBuffers[swapchainIndex]->begin(verticalBlurFramebuffers[swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

			verticalBlurCommandBuffers[swapchainIndex]->debugMarkerBeginEXT(pvr::strings::createFormatted("Gaussian Blur (vertical) - swapchain (%i)", swapchainIndex));

			verticalBlurCommandBuffers[swapchainIndex]->bindPipeline(verticalPipeline);
			verticalBlurCommandBuffers[swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipelineLayout, 0u, verticalDescriptorSets[swapchainIndex]);

			verticalBlurCommandBuffers[swapchainIndex]->draw(0, 3);
			verticalBlurCommandBuffers[swapchainIndex]->debugMarkerEndEXT();
			verticalBlurCommandBuffers[swapchainIndex]->end();
		}
	}

	/// <summary>Records the command buffers into the given main rendering command buffer.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="commandBuffer">The main command buffer into which the pre-recorded command buffers will be recorded into.</param>
	/// <param name="queue">The queue to which the command buffer will be submitted.</param>
	/// <param name="blurRenderPass">The RenderPass to use for the blur.</param>
	/// <param name="horizontalBlurFramebuffers">The framebuffers to use in the horizontal blur pass.</param>
	/// <param name="verticalBlurFramebuffers">The framebuffers to use in the vertical blur pass.</param>
	virtual void recordCommandsToMainCommandBuffer(uint32_t swapchainIndex, pvrvk::CommandBuffer& commandBuffer, pvrvk::Queue& queue, pvrvk::RenderPass& blurRenderPass,
		pvr::Multi<pvrvk::Framebuffer>& horizontalBlurFramebuffers, pvr::Multi<pvrvk::Framebuffer>& verticalBlurFramebuffers)
	{
		pvrvk::ClearValue clearValue = pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 1.f);

		// Horizontal Blur
		{
			commandBuffer->beginRenderPass(horizontalBlurFramebuffers[swapchainIndex], blurRenderPass,
				pvrvk::Rect2D(0, 0, horizontalBlurFramebuffers[swapchainIndex]->getDimensions().getWidth(), horizontalBlurFramebuffers[swapchainIndex]->getDimensions().getHeight()),
				false, &clearValue, 1);

			commandBuffer->executeCommands(horizontalBlurCommandBuffers[swapchainIndex]);
			commandBuffer->endRenderPass();
		}

		// Note the use of explicit external subpass dependencies which ensure the vertical blur occurs after the horizontal blur

		// Vertical Blur
		{
			commandBuffer->beginRenderPass(verticalBlurFramebuffers[swapchainIndex], blurRenderPass,
				pvrvk::Rect2D(0, 0, verticalBlurFramebuffers[swapchainIndex]->getDimensions().getWidth(), verticalBlurFramebuffers[swapchainIndex]->getDimensions().getHeight()),
				false, &clearValue, 1);

			commandBuffer->executeCommands(verticalBlurCommandBuffers[swapchainIndex]);
			commandBuffer->endRenderPass();
		}
	}

	/// <summary>Returns the blurred image for the given swapchain index.</summary>
	/// <param name="swapchainIndex">The swapchain index of the blurred image to retrieve.</param>
	/// <returns>The blurred image for the specified swapchain index.</returns>
	pvrvk::ImageView& getBlurredImage(uint32_t swapchainIndex)
	{
		return blurredImages[swapchainIndex];
	}
};

// A Compute shader based Gaussian Blur Pass
struct ComputeBlurPass : public GaussianBlurPass
{
	// Horizontal and Vertical compute pipelines
	pvrvk::ComputePipeline horizontalComputePipeline;
	pvrvk::ComputePipeline verticalComputePipeline;

	// For our compute shader based Gaussian blur we duplicate the Gaussian weights so that we don't need special shader logic to handle buffer overruns
	std::vector<float> duplicatedGaussianWeightsFloats;

	virtual void createBuffers(pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvr::utils::vma::Allocator& vmaAllocator)
	{
		ssboPerSwapchainSize = static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4) + pvr::getSize(pvr::GpuDatatypes::Float) * MaxGaussianKernel * 2);

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			bloomConfigBuffers[i] = pvr::utils::createBuffer(device, ssboPerSwapchainSize, pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT,
				pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT, &vmaAllocator);
		}
	}

	virtual void updateKernelBuffer(uint32_t swapchainIndex)
	{
		void* memory = bloomConfigBuffers[swapchainIndex]->getDeviceMemory()->getMappedData();

		memcpy(static_cast<char*>(memory), &blurConfig, pvr::getSize(pvr::GpuDatatypes::vec4));
		memcpy(static_cast<char*>(memory) + pvr::getSize(pvr::GpuDatatypes::vec4), duplicatedGaussianWeightsFloats.data(),
			pvr::getSize(pvr::GpuDatatypes::Float) * MaxGaussianKernel * 2);

		if (static_cast<uint32_t>(bloomConfigBuffers[swapchainIndex]->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			bloomConfigBuffers[swapchainIndex]->getDeviceMemory()->flushRange();
		}
	}

	virtual void updateKernelConfig(uint32_t kernelSizeConfig, bool useLinearOptimisation, bool ignoreNegligibleCoefficients)
	{
		kernelSize = kernelSizeConfig;
		updateGaussianWeightsAndOffsets(
			kernelSize, useLinearOptimisation, ignoreNegligibleCoefficients, gaussianOffsets, gaussianWeights, gaussianOffsetsFloats, gaussianWeightsFloats);

		duplicatedGaussianWeightsFloats.clear();

		blurConfig.z = static_cast<float>(gaussianWeights.size());

		for (uint32_t duplications = 0; duplications < 2; duplications++)
		{
			for (uint32_t i = 0; i < gaussianWeightsFloats.size(); i++)
			{
				duplicatedGaussianWeightsFloats.push_back(gaussianWeightsFloats[gaussianWeightsFloats.size() - 1 - i]);
			}

			for (uint32_t i = 1; i < gaussianWeightsFloats.size(); i++)
			{
				duplicatedGaussianWeightsFloats.push_back(gaussianWeightsFloats[i]);
			}
		}
	}

	virtual void createDescriptorSetLayout(pvrvk::Device& device)
	{
		// create the descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_STORAGE_IMAGE, 1u, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		descSetInfo.setBinding(1, pvrvk::DescriptorType::e_STORAGE_IMAGE, 1u, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		descSetInfo.setBinding(2, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1u, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);

		descriptorSetLayout = device->createDescriptorSetLayout(descSetInfo);

		// create the pipeline layouts
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

		pipeLayoutInfo.setDescSetLayout(0, descriptorSetLayout);

		pipelineLayout = device->createPipelineLayout(pipeLayoutInfo);
	}

	virtual void createDescriptorSets(pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::DescriptorPool& descriptorPool,
		pvr::Multi<pvrvk::ImageView>& horizontalBlurImageViews, pvr::Multi<pvrvk::ImageView>& verticalBlurImageViews, pvrvk::Sampler& sampler)
	{
		std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			// Descriptor sets for the Horizontal Blur Pass
			horizontalDescriptorSets.add(descriptorPool->allocateDescriptorSet(descriptorSetLayout));
			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_IMAGE, horizontalDescriptorSets[i], 0)
										.setImageInfo(0, pvrvk::DescriptorImageInfo(verticalBlurImageViews[i], sampler, pvrvk::ImageLayout::e_GENERAL)));

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_IMAGE, horizontalDescriptorSets[i], 1)
										.setImageInfo(0, pvrvk::DescriptorImageInfo(horizontalBlurImageViews[i], sampler, pvrvk::ImageLayout::e_GENERAL)));

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, horizontalDescriptorSets[i], 2)
										.setBufferInfo(0, pvrvk::DescriptorBufferInfo(bloomConfigBuffers[i], 0, ssboPerSwapchainSize)));

			// Descriptor sets for the Vertical Blur Pass
			verticalDescriptorSets.add(descriptorPool->allocateDescriptorSet(descriptorSetLayout));
			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_IMAGE, verticalDescriptorSets[i], 0)
										.setImageInfo(0, pvrvk::DescriptorImageInfo(horizontalBlurImageViews[i], sampler, pvrvk::ImageLayout::e_GENERAL)));

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_IMAGE, verticalDescriptorSets[i], 1)
										.setImageInfo(0, pvrvk::DescriptorImageInfo(verticalBlurImageViews[i], sampler, pvrvk::ImageLayout::e_GENERAL)));

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, verticalDescriptorSets[i], 2)
										.setBufferInfo(0, pvrvk::DescriptorBufferInfo(bloomConfigBuffers[i], 0, ssboPerSwapchainSize)));
		}

		device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
	}

	virtual void createPipelines(
		pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::RenderPass& renderPass, const glm::ivec2& blurFramebufferDimensions, pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::ComputePipelineCreateInfo pipelineInfo;

		std::string horizontalComputeShader;
		std::string verticalComputeShader;

		if (device->getPhysicalDevice()->getFeatures().getShaderStorageImageExtendedFormats())
		{
			horizontalComputeShader = Files::GaussianComputeBlurHorizontal16fSrcFile;
			verticalComputeShader = Files::GaussianComputeBlurVertical16fSrcFile;
		}
		// Special case platforms without support for shader storage image extended formats (features.ShaderStorageImageExtendedFormats)
		// if features.ShaderStorageImageExtendedFormats is not supported then fallback to the less efficient rgba16f shaders
		else
		{
			horizontalComputeShader = Files::GaussianComputeBlurHorizontal16frgbaSrcFile;
			verticalComputeShader = Files::GaussianComputeBlurVertical16frgbaSrcFile;
		}

		pipelineInfo.computeShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(horizontalComputeShader)->readToEnd<uint32_t>())));

		pipelineInfo.pipelineLayout = pipelineLayout;
		horizontalComputePipeline = device->createComputePipeline(pipelineInfo, pipelineCache);

		pipelineInfo.computeShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(verticalComputeShader)->readToEnd<uint32_t>())));
		verticalComputePipeline = device->createComputePipeline(pipelineInfo, pipelineCache);
	}

	using GaussianBlurPass::recordCommands;
	virtual void recordCommands(uint32_t swapchainIndex, pvr::Multi<pvrvk::ImageView>& horizontalBlurImages, pvr::Multi<pvrvk::ImageView>& verticalBlurImages, pvrvk::Queue& queue)
	{
		// horizontal
		{
			horizontalBlurCommandBuffers[swapchainIndex]->begin();
			horizontalBlurCommandBuffers[swapchainIndex]->debugMarkerBeginEXT("Compute Blur Horizontal");
			horizontalBlurCommandBuffers[swapchainIndex]->bindPipeline(horizontalComputePipeline);
			horizontalBlurCommandBuffers[swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_COMPUTE, pipelineLayout, 0u, horizontalDescriptorSets[swapchainIndex]);

			// Draw a quad
			// dispatch x = image.height / 32
			// dispatch y = 1
			// dispatch z = 1
			horizontalBlurCommandBuffers[swapchainIndex]->dispatch(static_cast<uint32_t>(glm::ceil(horizontalBlurImages[swapchainIndex]->getImage()->getHeight() / 32.0f)), 1, 1);
			horizontalBlurCommandBuffers[swapchainIndex]->debugMarkerEndEXT();

			pvrvk::MemoryBarrierSet layoutTransition;
			// Set up a barrier to pass the image from our horizontal compute shader to our vertical compute shader.
			layoutTransition.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT,
				horizontalBlurImages[swapchainIndex]->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_GENERAL,
				pvrvk::ImageLayout::e_GENERAL, queue->getFamilyIndex(), queue->getFamilyIndex()));
			horizontalBlurCommandBuffers[swapchainIndex]->pipelineBarrier(
				pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, layoutTransition);

			horizontalBlurCommandBuffers[swapchainIndex]->end();
		}

		// vertical
		{
			verticalBlurCommandBuffers[swapchainIndex]->begin();
			verticalBlurCommandBuffers[swapchainIndex]->debugMarkerBeginEXT("Compute Blur Vertical");
			verticalBlurCommandBuffers[swapchainIndex]->bindPipeline(verticalComputePipeline);
			verticalBlurCommandBuffers[swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_COMPUTE, pipelineLayout, 0u, verticalDescriptorSets[swapchainIndex]);

			// Draw a quad
			// dispatch x = image.width / 32
			// dispatch y = 1
			// dispatch z = 1
			verticalBlurCommandBuffers[swapchainIndex]->dispatch(static_cast<uint32_t>(glm::ceil(verticalBlurImages[swapchainIndex]->getImage()->getWidth() / 32.0f)), 1, 1);
			verticalBlurCommandBuffers[swapchainIndex]->debugMarkerEndEXT();

			pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_GENERAL;
			pvrvk::ImageLayout destinationImageLayout = pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL;

			pvrvk::MemoryBarrierSet layoutTransitions;
			layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT,
				horizontalBlurImages[swapchainIndex]->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), sourceImageLayout, destinationImageLayout,
				queue->getFamilyIndex(), queue->getFamilyIndex()));
			layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT,
				verticalBlurImages[swapchainIndex]->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), sourceImageLayout, destinationImageLayout,
				queue->getFamilyIndex(), queue->getFamilyIndex()));

			verticalBlurCommandBuffers[swapchainIndex]->pipelineBarrier(
				pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, layoutTransitions);

			verticalBlurCommandBuffers[swapchainIndex]->end();
		}
	}

	using GaussianBlurPass::recordCommandsToMainCommandBuffer;
	virtual void recordCommandsToMainCommandBuffer(uint32_t swapchainIndex, pvrvk::CommandBuffer& commandBuffer)
	{
		commandBuffer->executeCommands(horizontalBlurCommandBuffers[swapchainIndex]);
		commandBuffer->executeCommands(verticalBlurCommandBuffers[swapchainIndex]);
	}
};

// A Linear sampler optimised Gaussian Blur Pass
struct LinearGaussianBlurPass : public GaussianBlurPass
{
	// Horizontal and Vertical graphics pipelines to handle special cases where the number of samples will be an even number
	pvrvk::GraphicsPipeline evenSampleHorizontalPipeline;
	pvrvk::GraphicsPipeline evenSampleVerticalPipeline;

	virtual void createDescriptorSetLayout(pvrvk::Device& device) override
	{
		// create the descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetInfo.setBinding(1, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1u, pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

		descriptorSetLayout = device->createDescriptorSetLayout(descSetInfo);

		// create the pipeline layouts
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

		pipeLayoutInfo.setDescSetLayout(0, descriptorSetLayout);

		pipelineLayout = device->createPipelineLayout(pipeLayoutInfo);
	}

	virtual void createPipelines(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::RenderPass& renderPass, const glm::ivec2& blurFramebufferDimensions,
		pvrvk::PipelineCache& pipelineCache) override
	{
		pvrvk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.viewport.setViewportAndScissor(0, pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(blurFramebufferDimensions.x), static_cast<float>(blurFramebufferDimensions.y)),
			pvrvk::Rect2D(0, 0, blurFramebufferDimensions.x, blurFramebufferDimensions.y));

		pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_FRONT_BIT);
		pipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

		// disable depth writing and depth testing
		pipelineInfo.depthStencil.enableDepthWrite(false);
		pipelineInfo.depthStencil.enableDepthTest(false);

		// disable stencil testing
		pipelineInfo.depthStencil.enableStencilTest(false);

		pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

		pipelineInfo.vertexInput.clear();
		pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);

		pipelineInfo.pipelineLayout = pipelineLayout;

		// renderpass/subpass
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;

		// Handle odd and even number samples using specifically optimised shaders for each case
		// Generally an application will know beforehand which of the two approaches they would favour and would stick to one or the other implementation
		// In our case we provide implementations for both so that we aren't limited by one or the other kernel sizes
		pipelineInfo.vertexShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::LinearGaussianOddSamplesHorizontalVertSrcFile)->readToEnd<uint32_t>())));
		pipelineInfo.fragmentShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::LinearGaussianOddSamplesFragSrcFile)->readToEnd<uint32_t>())));

		horizontalPipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);

		pipelineInfo.vertexShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::LinearGaussianOddSamplesVerticalVertSrcFile)->readToEnd<uint32_t>())));
		pipelineInfo.fragmentShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::LinearGaussianOddSamplesFragSrcFile)->readToEnd<uint32_t>())));
		verticalPipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);

		pipelineInfo.vertexShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::LinearGaussianEvenSamplesHorizontalVertSrcFile)->readToEnd<uint32_t>())));
		pipelineInfo.fragmentShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::LinearGaussianEvenSamplesFragSrcFile)->readToEnd<uint32_t>())));

		evenSampleHorizontalPipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);

		pipelineInfo.vertexShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::LinearGaussianEvenSamplesVerticalVertSrcFile)->readToEnd<uint32_t>())));
		pipelineInfo.fragmentShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::LinearGaussianEvenSamplesFragSrcFile)->readToEnd<uint32_t>())));
		evenSampleVerticalPipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
	}

	virtual void recordCommands(uint32_t swapchainIndex, pvr::Multi<pvrvk::Framebuffer>& horizontalBlurFramebuffers, pvr::Multi<pvrvk::Framebuffer>& verticalBlurFramebuffers) override
	{
		// Record the commands to use for carrying out the horizontal gaussian blur pass
		{
			horizontalBlurCommandBuffers[swapchainIndex]->begin(horizontalBlurFramebuffers[swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

			horizontalBlurCommandBuffers[swapchainIndex]->debugMarkerBeginEXT(pvr::strings::createFormatted("Linear Gaussian Blur (horizontal) - swapchain (%i)", swapchainIndex));

			// appropriately handle odd vs even numbers of taps
			if (gaussianWeights.size() % 2 == 0)
			{
				horizontalBlurCommandBuffers[swapchainIndex]->bindPipeline(evenSampleHorizontalPipeline);
			}
			else
			{
				horizontalBlurCommandBuffers[swapchainIndex]->bindPipeline(horizontalPipeline);
			}

			horizontalBlurCommandBuffers[swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipelineLayout, 0u, horizontalDescriptorSets[swapchainIndex]);
			horizontalBlurCommandBuffers[swapchainIndex]->draw(0, 3);
			horizontalBlurCommandBuffers[swapchainIndex]->debugMarkerEndEXT();
			horizontalBlurCommandBuffers[swapchainIndex]->end();
		}

		// Record the commands to use for carrying out the vertical gaussian blur pass
		{
			verticalBlurCommandBuffers[swapchainIndex]->begin(verticalBlurFramebuffers[swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

			verticalBlurCommandBuffers[swapchainIndex]->debugMarkerBeginEXT(pvr::strings::createFormatted("Linear Gaussian Blur (vertical) - swapchain (%i)", swapchainIndex));

			if (gaussianWeights.size() % 2 == 0)
			{
				verticalBlurCommandBuffers[swapchainIndex]->bindPipeline(evenSampleVerticalPipeline);
			}
			else
			{
				verticalBlurCommandBuffers[swapchainIndex]->bindPipeline(verticalPipeline);
			}

			verticalBlurCommandBuffers[swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipelineLayout, 0u, verticalDescriptorSets[swapchainIndex]);

			verticalBlurCommandBuffers[swapchainIndex]->draw(0, 3);
			verticalBlurCommandBuffers[swapchainIndex]->debugMarkerEndEXT();
			verticalBlurCommandBuffers[swapchainIndex]->end();
		}
	}
};

// A Hybrid Gaussian Blur pass making use of a horizontal Compute shader pass followed by a Fragment based Vertical Gaussian Blur Pass
struct HybridGaussianBlurPass
{
	// The Compute shader based Gaussian Blur pass - we will only be making use of the horizontal blur resources
	ComputeBlurPass* computeBlurPass;
	// The Fragment shader based Gaussian Blur pass - we will only be making use of the vertical blur resources
	LinearGaussianBlurPass* linearBlurPass;

	// Command buffers for the horizontal and vertical blur passes
	pvr::Multi<pvrvk::SecondaryCommandBuffer> horizontalBlurCommandBuffers;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> verticalBlurCommandBuffers;

	/// <summary>A minimal initialisation function as no extra resources are created for this type of blur pass and instead we make use of the compute and fragment based passes.</summary>
	/// <param name="computeBlurPass">The Compute shader based Gaussian Blur pass - we will only be making use of the horizontal blur resources.</param>
	/// <param name="linearBlurPass">The Fragment shader based Gaussian Blur pass - we will only be making use of the vertical blur resources.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="commandPool">The command pool from which to allocate command buffers.</param>
	void init(ComputeBlurPass* computeBlurPass, LinearGaussianBlurPass* linearBlurPass, pvrvk::Swapchain& swapchain, pvrvk::CommandPool& commandPool)
	{
		this->computeBlurPass = computeBlurPass;
		this->linearBlurPass = linearBlurPass;

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			horizontalBlurCommandBuffers.add(commandPool->allocateSecondaryCommandBuffer());
			verticalBlurCommandBuffers.add(commandPool->allocateSecondaryCommandBuffer());
		}
	}

	/// <summary>Records the commands required for the Hybrid Gaussian blur.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="horizontalBlurFramebuffers">The framebuffers to use in the horizontal blur pass.</param>
	/// <param name="verticalBlurFramebuffers">The framebuffers to use in the vertical blur pass.</param>
	/// <param name="queue">The queue to which the command buffer will be submitted.</param>
	void recordCommands(
		uint32_t swapchainIndex, pvr::Multi<pvrvk::Framebuffer>& horizontalBlurFramebuffers, pvr::Multi<pvrvk::Framebuffer>& verticalBlurFramebuffers, pvrvk::Queue& queue)
	{
		// horizontal compute based gaussian blur pass
		{
			horizontalBlurCommandBuffers[swapchainIndex]->begin();
			horizontalBlurCommandBuffers[swapchainIndex]->debugMarkerBeginEXT("Compute Blur Horizontal");
			horizontalBlurCommandBuffers[swapchainIndex]->bindPipeline(computeBlurPass->horizontalComputePipeline);
			horizontalBlurCommandBuffers[swapchainIndex]->bindDescriptorSet(
				pvrvk::PipelineBindPoint::e_COMPUTE, computeBlurPass->pipelineLayout, 0u, computeBlurPass->horizontalDescriptorSets[swapchainIndex]);

			// Draw a quad
			// dispatch x = image.height / 32
			// dispatch y = 1
			// dispatch z = 1
			horizontalBlurCommandBuffers[swapchainIndex]->dispatch(
				static_cast<uint32_t>(glm::ceil(horizontalBlurFramebuffers[swapchainIndex]->getDimensions().getHeight() / 32.0f)), 1, 1);
			horizontalBlurCommandBuffers[swapchainIndex]->debugMarkerEndEXT();

			pvrvk::MemoryBarrierSet layoutTransition;
			// Set up a barrier to pass the image from our horizontal compute shader to our vertical fragment shader.
			layoutTransition.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT,
				horizontalBlurFramebuffers[swapchainIndex]->getAttachment(0)->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT),
				pvrvk::ImageLayout::e_GENERAL, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, queue->getFamilyIndex(), queue->getFamilyIndex()));
			horizontalBlurCommandBuffers[swapchainIndex]->pipelineBarrier(
				pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, layoutTransition);

			horizontalBlurCommandBuffers[swapchainIndex]->end();
		}

		// vertical fragment based gaussian blur pass
		{
			verticalBlurCommandBuffers[swapchainIndex]->begin(verticalBlurFramebuffers[swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
			verticalBlurCommandBuffers[swapchainIndex]->debugMarkerBeginEXT(pvr::strings::createFormatted("Linear Gaussian Blur (vertical) - swapchain (%i)", swapchainIndex));

			// appropriately handle odd vs even numbers of taps
			if (linearBlurPass->gaussianWeights.size() % 2 == 0)
			{
				verticalBlurCommandBuffers[swapchainIndex]->bindPipeline(linearBlurPass->evenSampleVerticalPipeline);
			}
			else
			{
				verticalBlurCommandBuffers[swapchainIndex]->bindPipeline(linearBlurPass->verticalPipeline);
			}

			verticalBlurCommandBuffers[swapchainIndex]->bindDescriptorSet(
				pvrvk::PipelineBindPoint::e_GRAPHICS, linearBlurPass->pipelineLayout, 0u, linearBlurPass->verticalDescriptorSets[swapchainIndex]);

			verticalBlurCommandBuffers[swapchainIndex]->draw(0, 3);
			verticalBlurCommandBuffers[swapchainIndex]->debugMarkerEndEXT();
			verticalBlurCommandBuffers[swapchainIndex]->end();
		}
	}

	/// <summary>Records the command buffers into the given main rendering command buffer.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="commandBuffer">The main command buffer into which the pre-recorded command buffers will be recorded into.</param>
	/// <param name="blurRenderPass">The RenderPass to use for the blur.</param>
	/// <param name="horizontalBlurFramebuffers">The framebuffers to use in the horizontal blur pass.</param>
	/// <param name="verticalBlurFramebuffers">The framebuffers to use in the vertical blur pass.</param>
	void recordCommandsToMainCommandBuffer(uint32_t swapchainIndex, pvrvk::CommandBuffer& commandBuffer, pvrvk::RenderPass& blurRenderPass,
		pvr::Multi<pvrvk::Framebuffer>& horizontalBlurFramebuffers, pvr::Multi<pvrvk::Framebuffer>& verticalBlurFramebuffers)
	{
		// Compute horizontal pass
		commandBuffer->executeCommands(horizontalBlurCommandBuffers[swapchainIndex]);

		// Fragment vertical pass
		pvrvk::ClearValue clearValue = pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 1.f);
		commandBuffer->beginRenderPass(verticalBlurFramebuffers[swapchainIndex], blurRenderPass,
			pvrvk::Rect2D(0, 0, verticalBlurFramebuffers[swapchainIndex]->getDimensions().getWidth(), verticalBlurFramebuffers[swapchainIndex]->getDimensions().getHeight()), false,
			&clearValue, 1);
		commandBuffer->executeCommands(verticalBlurCommandBuffers[swapchainIndex]);
		commandBuffer->endRenderPass();
	}
};

// Post bloom composition pass
struct PostBloomPass
{
	pvrvk::PipelineLayout pipelineLayout;
	pvrvk::GraphicsPipeline defaultPipeline;
	pvrvk::GraphicsPipeline bloomOnlyPipeline;
	pvrvk::GraphicsPipeline offscreenOnlyPipeline;
	pvrvk::DescriptorSetLayout descriptorSetLayout;
	pvr::Multi<pvrvk::DescriptorSet> descriptorSets;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> commandBuffers;

	/// <summary>Initialises the Post Bloom Pass.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="device">The device from which the resources will be allocated.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="commandPool">The command pool from which to allocate command buffers.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	/// <param name="vmaAllocator">A VMA allocator to use for allocating images and buffers.</param>
	/// <param name="renderPass">The RenderPass to use.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	void init(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::CommandPool& commandPool, pvrvk::DescriptorPool& descriptorPool,
		pvr::utils::vma::Allocator& vmaAllocator, pvrvk::RenderPass& renderPass, pvrvk::PipelineCache& pipelineCache)
	{
		createDescriptorSetLayout(device);
		createDescriptorSets(swapchain, descriptorPool);
		createPipeline(assetProvider, device, swapchain, renderPass, pipelineCache);

		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			commandBuffers.add(commandPool->allocateSecondaryCommandBuffer());
		}
	}

	/// <summary>Creates the descriptor set layout.</summary>
	/// <param name="device">The device from which the descriptor set layouts will be allocated.</param>
	void createDescriptorSetLayout(pvrvk::Device& device)
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

		descriptorSetLayout = device->createDescriptorSetLayout(descSetInfo);

		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo.setDescSetLayout(0, descriptorSetLayout);
		pipelineLayout = device->createPipelineLayout(pipeLayoutInfo);
	}

	/// <summary>Creates the descriptor sets.</summary>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="descriptorPool">The descriptor pool from which to allocate descriptor sets.</param>
	void createDescriptorSets(pvrvk::Swapchain& swapchain, pvrvk::DescriptorPool& descriptorPool)
	{
		for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
		{
			descriptorSets.add(descriptorPool->allocateDescriptorSet(descriptorSetLayout));
		}
	}

	/// <summary>Updates the descriptor sets used based on the original image and bloomed image.</summary>
	/// <param name="device">The device from which the updateDescriptorSets call will be piped.</param>
	/// <param name="swapchainIndex">The swapchain index of the descriptor set to update.</param>
	/// <param name="originalImageView">The original unblurred image view.</param>
	/// <param name="blurredImageView">The blurred luminance image view.</param>
	/// <param name="sampler">The sampler object to use when sampling from the images.</param>
	void updateDescriptorSets(pvrvk::Device& device, uint32_t swapchainIndex, pvrvk::ImageView& originalImageView, pvrvk::ImageView& blurredImageView, pvrvk::Sampler& sampler)
	{
		std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[swapchainIndex], 0)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(blurredImageView, sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSets[swapchainIndex], 1)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(originalImageView, sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
	}

	/// <summary>Creates the Post bloom pipeline.</summary>
	/// <param name="assetProvider">The pvr::IAssetProvider which will be used for loading resources from memory.</param>
	/// <param name="swapchain">The swapchain which will determine the number of per swapchain resources to allocate.</param>
	/// <param name="renderPass">The RenderPass to use.</param>
	/// <param name="pipelineCache">A pipeline cache object to use when creating pipelines.</param>
	void createPipeline(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::Swapchain& swapchain, pvrvk::RenderPass& renderPass, pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.viewport.setViewportAndScissor(0,
			pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(swapchain->getDimension().getWidth()), static_cast<float>(swapchain->getDimension().getHeight())),
			pvrvk::Rect2D(0, 0, swapchain->getDimension().getWidth(), swapchain->getDimension().getHeight()));

		pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_FRONT_BIT);
		pipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

		// disable depth writing and depth testing
		pipelineInfo.depthStencil.enableDepthWrite(false);
		pipelineInfo.depthStencil.enableDepthTest(false);

		// disable stencil testing
		pipelineInfo.depthStencil.enableStencilTest(false);

		pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

		// load and create appropriate shaders
		pipelineInfo.vertexShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::PostBloomVertShaderSrcFile)->readToEnd<uint32_t>())));
		pipelineInfo.fragmentShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(Files::PostBloomFragShaderSrcFile)->readToEnd<uint32_t>())));

		pipelineInfo.vertexInput.clear();
		pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);

		pipelineInfo.pipelineLayout = pipelineLayout;

		// renderpass/subpass
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;

		defaultPipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);

		// Enable bloom only
		int enabled = 1;
		pipelineInfo.fragmentShader.setShaderConstant(0, pvrvk::ShaderConstantInfo(0, &enabled, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Integer))));
		bloomOnlyPipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);

		// Disable bloom only
		// Enable Offscreen only
		int disabled = 0;
		pipelineInfo.fragmentShader.setShaderConstant(0, pvrvk::ShaderConstantInfo(0, &disabled, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Integer))));
		pipelineInfo.fragmentShader.setShaderConstant(1, pvrvk::ShaderConstantInfo(1, &enabled, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Integer))));

		offscreenOnlyPipeline = device->createGraphicsPipeline(pipelineInfo, pipelineCache);
	}

	/// <summary>Records the secondary command buffers for the post bloom composition pass.</summary>
	/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
	/// <param name="framebuffer">The framebuffer to render into</param>
	void recordCommandBuffer(uint32_t swapchainIndex, pvrvk::Framebuffer& framebuffer, bool renderBloomOnly, bool renderOffScreenOnly)
	{
		commandBuffers[swapchainIndex]->begin(framebuffer, 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
		commandBuffers[swapchainIndex]->debugMarkerBeginEXT("PostBloom");
		if (renderOffScreenOnly)
		{
			commandBuffers[swapchainIndex]->bindPipeline(offscreenOnlyPipeline);
		}
		else if (renderBloomOnly)
		{
			commandBuffers[swapchainIndex]->bindPipeline(bloomOnlyPipeline);
		}
		else
		{
			commandBuffers[swapchainIndex]->bindPipeline(defaultPipeline);
		}
		commandBuffers[swapchainIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipelineLayout, 0u, descriptorSets[swapchainIndex]);
		commandBuffers[swapchainIndex]->draw(0, 3);
		commandBuffers[swapchainIndex]->debugMarkerEndEXT();
		commandBuffers[swapchainIndex]->end();
	}
};

struct DeviceResources
{
	pvrvk::Instance instance;
	pvrvk::DebugReportCallback debugCallbacks[2];
	pvrvk::Device device;
	pvrvk::DescriptorPool descriptorPool;
	pvrvk::CommandPool commandPool;
	pvrvk::Swapchain swapchain;
	pvr::utils::vma::Allocator vmaAllocator;
	pvrvk::Queue queues[2];
	pvrvk::PipelineCache pipelineCache;

	// On screen resources
	pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffers;
	pvrvk::RenderPass onScreenRenderPass;

	// Off screen resources
	pvr::Multi<pvrvk::Framebuffer> offScreenFramebuffers;
	pvrvk::RenderPass offScreenRenderPass;
	pvr::Multi<pvrvk::ImageView> depthStencilImages;

	// Synchronisation primitives
	pvrvk::Semaphore semaphoreImageAcquired[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Fence perFrameAcquireFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Semaphore semaphorePresent[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Fence perFrameCommandBufferFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	// Textures
	pvr::Multi<pvrvk::ImageView> luminanceImageViews;
	pvr::Multi<pvrvk::ImageView> offScreenColorImageViews;
	pvr::Multi<pvrvk::ImageView> pingPongImageViews[2];
	pvr::Multi<pvrvk::ImageView> storageImagePingPongImageViews[2];
	pvrvk::ImageView diffuseIrradianceMapImageView;

	// Bloom resources
	pvrvk::RenderPass blurRenderPass;
	pvrvk::RenderPass hybridBlurRenderPass;
	pvr::Multi<pvrvk::Framebuffer> blurFramebuffers[2];
	pvr::Multi<pvrvk::Framebuffer> hybridBlurFramebuffers[2];

	// Samplers
	pvrvk::Sampler samplerNearest;
	pvrvk::Sampler samplerBilinear;
	pvrvk::Sampler samplerTrilinear;

	// Command Buffers
	pvr::Multi<pvrvk::CommandBuffer> mainCommandBuffers;
	pvrvk::CommandBuffer utilityCommandBuffer;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> bloomUiRendererCommandBuffers;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> uiRendererCommandBuffers;

	// Passes
	StatuePass statuePass;
	SkyboxPass skyBoxPass;
	DownSamplePass downsamplePass;
	DownSamplePass computeDownsamplePass;
	GaussianBlurPass gaussianBlurPass;
	LinearGaussianBlurPass linearGaussianBlurPass;
	LinearGaussianBlurPass truncatedLinearGaussianBlurPass;
	DualFilterBlurPass dualFilterBlurPass;
	DownAndTentFilterBlurPass downAndTentFilterBlurPass;
	ComputeBlurPass computeBlurPass;
	HybridGaussianBlurPass hybridGaussianBlurPass;
	KawaseBlurPass kawaseBlurPass;
	PostBloomPass postBloomPass;

	// UIRenderers used to display text
	pvr::ui::UIRenderer uiRenderer;

	// Buffers and their views
	pvr::utils::StructuredBufferView sceneBufferView;
	pvrvk::Buffer sceneBuffer;
	pvr::utils::StructuredBufferView lightBufferView;
	pvrvk::Buffer lightBuffer;
	pvr::utils::StructuredBufferView bloomConfigBufferView;
	pvrvk::Buffer bloomConfigBuffer;

	~DeviceResources()
	{
		if (device.isValid())
		{
			device->waitIdle();
			uint32_t l = swapchain->getSwapchainLength();
			for (uint32_t i = 0; i < l; ++i)
			{
				if (perFrameAcquireFence[i].isValid())
					perFrameAcquireFence[i]->wait();
				if (perFrameCommandBufferFence[i].isValid())
					perFrameCommandBufferFence[i]->wait();
			}
		}
	}
};

/*!********************************************************************************************
Class implementing the Shell functions.
***********************************************************************************************/
class VulkanPostProcessing : public pvr::Shell
{
	std::unique_ptr<DeviceResources> _deviceResources;

	pvrvk::Format _luminanceColorFormat;
	pvrvk::Format _storageImageLuminanceColorFormat;
	pvrvk::ImageTiling _storageImageTiling;

	glm::ivec2 _blurFramebufferDimensions;
	glm::vec2 _blurInverseFramebufferDimensions;
	uint32_t _blurScale;

	// Synchronisation counters
	uint32_t _numSwapImages;
	uint32_t _swapchainIndex;
	uint32_t _frameId;
	uint32_t _queueIndex;

	bool _animateObject;
	bool _animateCamera;
	float _objectAngleY;
	float _cameraAngle;
	pvr::TPSCamera _camera;
	float _logicTime;
	float _modeSwitchTime;
	bool _isManual;
	float _modeDuration;

	glm::vec3 _lightPosition;
	glm::mat4 _viewMatrix;
	glm::mat4 _projectionMatrix;
	glm::mat4 _viewProjectionMatrix;

	BloomMode _blurMode;

	bool _useThreshold;
	float _bloomLumaThreshold;

	uint32_t _currentDemoConfiguration;

	bool _mustRecordPrimaryCommandBuffer[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	bool _mustUpdatePerSwapchainDemoConfig[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	bool _renderOnlyBloom;

	std::string _currentBlurString;

	bool _supportsBlit;

public:
	VulkanPostProcessing() {}

	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void recordBlurCommands(BloomMode blurMode, uint32_t swapchainIndex);
	void createBuffers();
	void createSceneBuffers();
	void createBloomThresholdBuffer();
	void createBlurRenderPass();
	void createHybridBlurRenderPass();
	void createBlurFramebuffers();
	void createHybridBlurFramebuffers();
	void createSamplers();
	void allocatePingPongImages();
	void createOffScreenFramebuffers();
	void createUiRenderer();
	void updateBlurDescription();
	void recordUIRendererCommands(uint32_t swapchainIndex, pvr::Multi<pvrvk::SecondaryCommandBuffer>& commandBuffers);
	void updateDemoConfigs();
	void eventMappedInput(pvr::SimplifiedInput e);
	void updateBloomConfiguration();
	void updateAnimation();
	void updateDynamicSceneData();
	void recordMainCommandBuffer(uint32_t swapchainIndex);
};

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.) If the rendering
/// context is lost, initApplication() will not be called again.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanPostProcessing::initApplication()
{
	this->setStencilBitsPerPixel(0);

	// Default demo properties
	_animateObject = true;
	_animateCamera = false;
	_lightPosition = LightPosition;
	_useThreshold = true;
	_objectAngleY = 0.0f;
	_cameraAngle = 240.0f;
	_camera.setDistanceFromTarget(200.f);
	_camera.setHeight(-15.f);
	_blurScale = 4;
	_frameId = 0;
	_queueIndex = 0;
	_logicTime = 0.0f;
	_modeSwitchTime = 0.0f;
	_isManual = false;
	_modeDuration = 1.5f;

	// Handle command line arguments including "blurmode", "blursize" and "bloom"
	const pvr::CommandLine& commandOptions = getCommandLine();
	int32_t intBlurMode = -1;
	if (commandOptions.getIntOption("-blurmode", intBlurMode))
	{
		if (intBlurMode > static_cast<int32_t>(BloomMode::NumBloomModes))
		{
			_blurMode = BloomMode::DefaultMode;
		}
		else
		{
			_isManual = true;
			_blurMode = static_cast<BloomMode>(intBlurMode);
		}
	}
	else
	{
		_blurMode = BloomMode::DefaultMode;
	}

	int32_t intConfigSize = -1;
	if (commandOptions.getIntOption("-blursize", intConfigSize))
	{
		if (intConfigSize > static_cast<int32_t>(DemoConfigurations::NumDemoConfigurations))
		{
			_currentDemoConfiguration = DemoConfigurations::DefaultDemoConfigurations;
		}
		else
		{
			_isManual = true;
			_currentDemoConfiguration = intConfigSize;
		}
	}
	else
	{
		_currentDemoConfiguration = DemoConfigurations::DefaultDemoConfigurations;
	}

	_renderOnlyBloom = false;
	commandOptions.getBoolOptionSetTrueIfPresent("-bloom", _renderOnlyBloom);

	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context(e.g.textures, vertex buffers, etc.)</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanPostProcessing::initView()
{
	_deviceResources = std::unique_ptr<DeviceResources>(new DeviceResources());

	// Create instance and retrieve compatible physical devices
	_deviceResources->instance = pvr::utils::createInstance(this->getApplicationName());

	if (_deviceResources->instance->getNumPhysicalDevices() == 0)
	{
		setExitMessage("Unable not find a compatible Vulkan physical device.");
		return pvr::Result::UnknownError;
	}

	// Create the surface
	pvrvk::Surface surface = pvr::utils::createSurface(_deviceResources->instance, _deviceResources->instance->getPhysicalDevice(0), this->getWindow(), this->getDisplay());

	// Add Debug Report Callbacks
	// Add a Debug Report Callback for logging messages for events of all supported types.
	_deviceResources->debugCallbacks[0] = pvr::utils::createDebugReportCallback(_deviceResources->instance);
	// Add a second Debug Report Callback for throwing exceptions for Error events.
	_deviceResources->debugCallbacks[1] =
		pvr::utils::createDebugReportCallback(_deviceResources->instance, pvrvk::DebugReportFlagsEXT::e_ERROR_BIT_EXT, pvr::utils::throwOnErrorDebugReportCallback);

	pvr::utils::QueuePopulateInfo queueCreateInfos[] = {
		{ pvrvk::QueueFlags::e_GRAPHICS_BIT | pvrvk::QueueFlags::e_COMPUTE_BIT, surface }, // Queue 0
		{ pvrvk::QueueFlags::e_GRAPHICS_BIT | pvrvk::QueueFlags::e_COMPUTE_BIT, surface } // Queue 1
	};
	pvr::utils::QueueAccessInfo queueAccessInfos[2];
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), queueCreateInfos, 2, queueAccessInfos);

	_deviceResources->queues[0] = _deviceResources->device->getQueue(queueAccessInfos[0].familyId, queueAccessInfos[0].queueId);
	_deviceResources->queues[1] = _deviceResources->device->getQueue(queueAccessInfos[1].familyId, queueAccessInfos[1].queueId);

	// Currently we're requiring that both queues use the same queue family id
	assertion(_deviceResources->queues[0]->getFamilyIndex() == _deviceResources->queues[1]->getFamilyIndex());

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT;
	}

	_luminanceColorFormat = pvrvk::Format::e_R16_SFLOAT;

	// Determine support for using images created using the given image format as a blit src and blit dst
	{
		pvrvk::FormatProperties properties = _deviceResources->device->getPhysicalDevice()->getFormatProperties(_luminanceColorFormat);
		pvrvk::FormatFeatureFlags flags = properties.getOptimalTilingFeatures();

		if ((static_cast<uint32_t>(flags & pvrvk::FormatFeatureFlags::e_BLIT_SRC_BIT) != 0) && (static_cast<uint32_t>(flags & pvrvk::FormatFeatureFlags::e_BLIT_DST_BIT) != 0))
		{
			_supportsBlit = true;
		}
		else
		{
			_supportsBlit = false;
		}
	}

	// Create memory allocator
	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	pvr::utils::createSwapchainAndDepthStencilImageAndViews(_deviceResources->device, surface, getDisplayAttributes(), _deviceResources->swapchain,
		_deviceResources->depthStencilImages, swapchainImageUsage, pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT,
		&_deviceResources->vmaAllocator);

	// calculate the frame buffer width and heights
	_blurFramebufferDimensions = glm::ivec2(this->getWidth() / _blurScale, this->getHeight() / _blurScale);
	_blurInverseFramebufferDimensions = glm::vec2(1.0f / _blurFramebufferDimensions.x, 1.0f / _blurFramebufferDimensions.y);

	// Calculates the projection matrices
	bool bRotate = isFullScreen() && isScreenRotated();
	if (bRotate)
	{
		_projectionMatrix =
			pvr::math::perspectiveFov(pvr::Api::Vulkan, Fov, static_cast<float>(getHeight()), static_cast<float>(getWidth()), CameraNear, CameraFar, glm::pi<float>() * .5f);
	}
	else
	{
		_projectionMatrix = pvr::math::perspectiveFov(pvr::Api::Vulkan, Fov, static_cast<float>(getWidth()), static_cast<float>(getHeight()), CameraNear, CameraFar);
	}

	// Get the number of swap images
	_numSwapImages = _deviceResources->swapchain->getSwapchainLength();

	// Get current swap index
	_swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	// Determine whether shader storage image extended formats is supported on the current platform
	// Ideally we would choose e_R16_SFLOAT but the physical device must have support for features.ShaderStorageImageExtendedFormats
	// If features.ShaderStorageImageExtendedFormats isn't supported then we'll fallback to e_R16G16B16A16_SFLOAT
	// e_R16G16B16A16_SFLOAT may already be preferred as it allows for the handling of colored blooms i.e. colored light sources
	if (_deviceResources->instance->getPhysicalDevice(0)->getFeatures().getShaderStorageImageExtendedFormats())
	{
		pvrvk::Format extendedFormat = _luminanceColorFormat;
		pvrvk::FormatProperties prop = _deviceResources->instance->getPhysicalDevice(0)->getFormatProperties(extendedFormat);
		if ((prop.getOptimalTilingFeatures() & pvrvk::FormatFeatureFlags::e_STORAGE_IMAGE_BIT) != 0)
		{
			_storageImageLuminanceColorFormat = extendedFormat;
			_storageImageTiling = pvrvk::ImageTiling::e_OPTIMAL;
		}
		else if ((prop.getLinearTilingFeatures() & pvrvk::FormatFeatureFlags::e_STORAGE_IMAGE_BIT) != 0)
		{
			_storageImageLuminanceColorFormat = extendedFormat;
			_storageImageTiling = pvrvk::ImageTiling::e_LINEAR;
		}

		// Ensure that the format being used supports Linear Sampling
		if ((prop.getOptimalTilingFeatures() & pvrvk::FormatFeatureFlags::e_SAMPLED_IMAGE_FILTER_LINEAR_BIT) == 0)
		{
			assertion(0);
		}
	}
	else
	{
		pvrvk::Format format = pvrvk::Format::e_R16G16B16A16_SFLOAT;
		pvrvk::FormatProperties prop = _deviceResources->instance->getPhysicalDevice(0)->getFormatProperties(format);
		if ((prop.getOptimalTilingFeatures() & pvrvk::FormatFeatureFlags::e_STORAGE_IMAGE_BIT) != 0)
		{
			_storageImageLuminanceColorFormat = format;
			_storageImageTiling = pvrvk::ImageTiling::e_OPTIMAL;
		}
		else if ((prop.getLinearTilingFeatures() & pvrvk::FormatFeatureFlags::e_STORAGE_IMAGE_BIT) != 0)
		{
			_storageImageLuminanceColorFormat = format;
			_storageImageTiling = pvrvk::ImageTiling::e_LINEAR;
		}

		// Ensure that the format being used supports Linear Sampling
		if ((prop.getOptimalTilingFeatures() & pvrvk::FormatFeatureFlags::e_SAMPLED_IMAGE_FILTER_LINEAR_BIT) == 0)
		{
			assertion(0);
		}
	}

	// create the commandpool and the descriptor pool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(
		pvrvk::CommandPoolCreateInfo(_deviceResources->queues[0]->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	// This demo application makes use of quite a large number of Images and Buffers and therefore we're making possible for the descriptor pool to allocate descriptors with various limits.maxDescriptorSet*
	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(
		pvrvk::DescriptorPoolCreateInfo()
			.setMaxDescriptorSets(75)
			.addDescriptorInfo(pvrvk::DescriptorType::e_INPUT_ATTACHMENT, 10)
			.addDescriptorInfo(
				pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMaxDescriptorSetSampledImages())
			.addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC,
				_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMaxDescriptorSetUniformBuffersDynamic())
			.addDescriptorInfo(pvrvk::DescriptorType::e_STORAGE_IMAGE, 20)
			.addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMaxDescriptorSetUniformBuffers())
			.addDescriptorInfo(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMaxDescriptorSetStorageBuffers()));

	// create the utility commandbuffer which will be used for image layout transitions and buffer/image uploads.
	_deviceResources->utilityCommandBuffer = _deviceResources->commandPool->allocateCommandBuffer();
	_deviceResources->utilityCommandBuffer->begin();

	// Create the framebuffers and main rendering images
	// Note the use of the color attachment load operation (pvrvk::AttachmentLoadOp::e_DONT_CARE). The final composition pass will be a full screen render
	// so we don't need to clear the attachment prior to rendering to the image as each pixel will get a new value either way
	pvr::utils::createOnscreenFramebufferAndRenderpass(_deviceResources->swapchain, nullptr, _deviceResources->onScreenFramebuffers, _deviceResources->onScreenRenderPass,
		pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_UNDEFINED, pvrvk::AttachmentLoadOp::e_DONT_CARE);

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	// create demo buffers
	createBuffers();

	// Allocate two images to use which can be "ping-ponged" between when applying various filters/blurs
	// Pass 1: Read From 1, Render to 0
	// Pass 2: Read From 0, Render to 1
	allocatePingPongImages();

	// Create the HDR offscreen framebuffers
	createOffScreenFramebuffers();

	// Create the samplers used for various texture sampling
	createSamplers();

	// transition the blur pingpong images ready for their first use
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		for (uint32_t j = 0; j < 2; j++)
		{
			pvr::utils::setImageLayout(_deviceResources->pingPongImageViews[j][i]->getImage(), pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL,
				_deviceResources->utilityCommandBuffer);
			if (_storageImageLuminanceColorFormat != _luminanceColorFormat)
			{
				pvr::utils::setImageLayout(_deviceResources->storageImagePingPongImageViews[j][i]->getImage(), pvrvk::ImageLayout::e_UNDEFINED,
					pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, _deviceResources->utilityCommandBuffer);
			}
		}
	}

	pvr::Texture diffuseIrradianceMapTexture = pvr::textureLoad(this->getAssetStream(DiffuseIrradianceMapTexFile), pvr::TextureFileFormat::PVR);

	// Create and Allocate Textures
	_deviceResources->diffuseIrradianceMapImageView =
		pvr::utils::uploadImageAndView(_deviceResources->device, diffuseIrradianceMapTexture, true, _deviceResources->utilityCommandBuffer, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, &_deviceResources->vmaAllocator, &_deviceResources->vmaAllocator);

	// Create the main scene rendering passes
	_deviceResources->statuePass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool, _deviceResources->descriptorPool,
		_deviceResources->offScreenRenderPass, _deviceResources->offScreenFramebuffers, _deviceResources->vmaAllocator, _deviceResources->utilityCommandBuffer,
		_deviceResources->samplerBilinear, _deviceResources->samplerTrilinear, _deviceResources->pipelineCache, _deviceResources->diffuseIrradianceMapImageView,
		_deviceResources->sceneBufferView, _deviceResources->sceneBuffer, _deviceResources->bloomConfigBufferView, _deviceResources->bloomConfigBuffer);

	_deviceResources->skyBoxPass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool, _deviceResources->descriptorPool,
		_deviceResources->offScreenRenderPass, _deviceResources->offScreenFramebuffers, _deviceResources->vmaAllocator, _deviceResources->utilityCommandBuffer,
		_deviceResources->samplerTrilinear, _deviceResources->pipelineCache, _deviceResources->sceneBufferView, _deviceResources->sceneBuffer,
		_deviceResources->bloomConfigBufferView, _deviceResources->bloomConfigBuffer);

	// Create bloom RenderPasses and Framebuffers
	createBlurRenderPass();
	createBlurFramebuffers();

	createHybridBlurRenderPass();
	createHybridBlurFramebuffers();

	// Create the downsample passes
	{
		_deviceResources->downsamplePass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool, _deviceResources->descriptorPool,
			_blurFramebufferDimensions, _deviceResources->luminanceImageViews, _deviceResources->pingPongImageViews[0], _deviceResources->samplerBilinear,
			_deviceResources->pipelineCache, false);

		_deviceResources->computeDownsamplePass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool, _deviceResources->descriptorPool,
			_blurFramebufferDimensions, _deviceResources->luminanceImageViews, _deviceResources->storageImagePingPongImageViews[0], _deviceResources->samplerBilinear,
			_deviceResources->pipelineCache, true);
	}

	// Create the post bloom composition pass
	_deviceResources->postBloomPass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool, _deviceResources->descriptorPool,
		_deviceResources->vmaAllocator, _deviceResources->onScreenRenderPass, _deviceResources->pipelineCache);

	// Initialise the Blur Passes
	// Gaussian Blurs
	{
		uint32_t horizontalPassPingPongImageIndex = 1;
		uint32_t verticalPassPingPongImageIndex = 0;

		_deviceResources->gaussianBlurPass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool, _deviceResources->descriptorPool,
			_deviceResources->vmaAllocator, _deviceResources->blurRenderPass, _blurFramebufferDimensions, _deviceResources->pingPongImageViews[horizontalPassPingPongImageIndex],
			_deviceResources->pingPongImageViews[verticalPassPingPongImageIndex], _deviceResources->samplerNearest, _deviceResources->pipelineCache);

		_deviceResources->linearGaussianBlurPass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool, _deviceResources->descriptorPool,
			_deviceResources->vmaAllocator, _deviceResources->blurRenderPass, _blurFramebufferDimensions, _deviceResources->pingPongImageViews[horizontalPassPingPongImageIndex],
			_deviceResources->pingPongImageViews[verticalPassPingPongImageIndex], _deviceResources->samplerBilinear, _deviceResources->pipelineCache);

		_deviceResources->truncatedLinearGaussianBlurPass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool,
			_deviceResources->descriptorPool, _deviceResources->vmaAllocator, _deviceResources->blurRenderPass, _blurFramebufferDimensions,
			_deviceResources->pingPongImageViews[horizontalPassPingPongImageIndex], _deviceResources->pingPongImageViews[verticalPassPingPongImageIndex],
			_deviceResources->samplerBilinear, _deviceResources->pipelineCache);

		_deviceResources->computeBlurPass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool, _deviceResources->descriptorPool,
			_deviceResources->vmaAllocator, _deviceResources->blurRenderPass, _blurFramebufferDimensions,
			_deviceResources->storageImagePingPongImageViews[horizontalPassPingPongImageIndex], _deviceResources->storageImagePingPongImageViews[verticalPassPingPongImageIndex],
			_deviceResources->samplerNearest, _deviceResources->pipelineCache);

		_deviceResources->hybridGaussianBlurPass.init(
			&_deviceResources->computeBlurPass, &_deviceResources->truncatedLinearGaussianBlurPass, _deviceResources->swapchain, _deviceResources->commandPool);
	}

	// Kawase Blur
	{
		_deviceResources->kawaseBlurPass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool, _deviceResources->descriptorPool,
			_deviceResources->blurRenderPass, _blurFramebufferDimensions, _deviceResources->pingPongImageViews, 2, _deviceResources->samplerBilinear, _deviceResources->pipelineCache);
	}

	// Dual Filter Blur
	{
		_deviceResources->dualFilterBlurPass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool, _deviceResources->descriptorPool,
			_deviceResources->blurRenderPass, _deviceResources->vmaAllocator, _luminanceColorFormat, glm::ivec2(this->getWidth(), this->getHeight()),
			_deviceResources->samplerBilinear, _deviceResources->utilityCommandBuffer, _deviceResources->onScreenRenderPass, _deviceResources->pipelineCache);
	}

	// Down Sample and Tent filter blur pass
	{
		_deviceResources->downAndTentFilterBlurPass.init(*this, _deviceResources->device, _deviceResources->swapchain, _deviceResources->commandPool, _deviceResources->descriptorPool,
			_deviceResources->blurRenderPass, _deviceResources->vmaAllocator, _luminanceColorFormat, glm::ivec2(this->getWidth(), this->getHeight()),
			_deviceResources->samplerBilinear, _deviceResources->utilityCommandBuffer, _deviceResources->onScreenRenderPass, _deviceResources->pipelineCache, _supportsBlit);
	}

	_deviceResources->utilityCommandBuffer->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->utilityCommandBuffer;
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queues[0]->submit(&submitInfo, 1);
	_deviceResources->queues[0]->waitIdle(); // wait

	// signal that buffers need updating and command buffers need recording
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_mustRecordPrimaryCommandBuffer[i] = true;
		_mustUpdatePerSwapchainDemoConfig[i] = true;
	}

	// Update the demo configuration
	updateDemoConfigs();

	// create the synchronisation primitives
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->semaphorePresent[i] = _deviceResources->device->createSemaphore();
		_deviceResources->semaphoreImageAcquired[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameCommandBufferFence[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->perFrameAcquireFence[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
	}

	// initalise the UI Renderers
	createUiRenderer();

	// Record UI renderer command buffers
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		// record bloom command buffers
		recordUIRendererCommands(i, _deviceResources->uiRendererCommandBuffers);
		recordUIRendererCommands(i, _deviceResources->bloomUiRendererCommandBuffers);
	}

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->mainCommandBuffers.add(_deviceResources->commandPool->allocateCommandBuffer());
	}

	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame</summary>
/// <returns>Result::Success if no error occurred.</summary>
pvr::Result VulkanPostProcessing::renderFrame()
{
	_deviceResources->perFrameAcquireFence[_frameId]->wait();
	_deviceResources->perFrameAcquireFence[_frameId]->reset();
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->semaphoreImageAcquired[_frameId], _deviceResources->perFrameAcquireFence[_frameId]);

	_swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameCommandBufferFence[_swapchainIndex]->wait();
	_deviceResources->perFrameCommandBufferFence[_swapchainIndex]->reset();

	// update dynamic buffers
	updateDynamicSceneData();

	// Re-record command buffers on demand
	if (_mustRecordPrimaryCommandBuffer[_swapchainIndex])
	{
		recordMainCommandBuffer(_swapchainIndex);
		_mustRecordPrimaryCommandBuffer[_swapchainIndex] = false;
	}

	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags submitWaitFlags = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.waitDestStages = &submitWaitFlags;
	submitInfo.waitSemaphores = &_deviceResources->semaphoreImageAcquired[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->semaphorePresent[_frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.numCommandBuffers = 1;
	submitInfo.commandBuffers = &_deviceResources->mainCommandBuffers[_swapchainIndex];

	// Ping pong between multiple VkQueues
	// It's important to realise that in Vulkan, pipeline barriers only observe their barriers within the VkQueue they are submitted to.
	// When we use BloomMode::Compute || BloomMode::HybridGaussian we are introducing a Fragment -> Compute -> Fragment chain, which if left
	// unattended can cause compute pipeline bubbles meaning we can quite easily hit into per frame workload serialisation as shown below:
	// Compute Workload             |1----|                  |2----|
	// Fragment Workload     |1----|       |1---||1--||2----|       |2---||2--|

	// The Compute -> Fragment pipeline used after our Compute pipeline stage for synchronising between the pipeline stages has further, less obvious unintended consequences
	// in that when using only a single VkQueue this pipeline barrier enforces a barrier between all Compute work *before* the barrier and all Fragment work *after* the
	// barrier. This barrier means that even though we can see compute pipeline bubbles that could potentially be interleaved with Fragment work the barrier enforces against
	// this behaviour. This is where Vulkan really shines over OpenGL ES in terms of giving explicit control of work submission to the application. We make use of two Vulkan
	// VkQueue objects which are submitted to in a ping-ponged fashion. Each VkQueue only needs to observe barriers used in command buffers which are submitted to them meaning
	// there are no barriers enforced between the two sets of separate commands other than the presentation synchronisation logic. This simple change allows us to observe the
	// following workload scheduling: Compute Workload                |1----|    |2----| Fragment Workload      |1----||2----|  |1---||1--||2---||2--|
	_deviceResources->queues[_queueIndex]->submit(&submitInfo, 1, _deviceResources->perFrameCommandBufferFence[_swapchainIndex]);

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->swapchain, _swapchainIndex, _deviceResources->commandPool, _deviceResources->queues[_queueIndex],
			this->getScreenshotFileName(), &_deviceResources->vmaAllocator, &_deviceResources->vmaAllocator);
	}

	pvrvk::PresentInfo presentInfo;
	presentInfo.waitSemaphores = &_deviceResources->semaphorePresent[_frameId];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.imageIndices = &_swapchainIndex;

	// As above we must present using the same VkQueue as submitted to previously
	_deviceResources->queues[_queueIndex]->present(presentInfo);

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();
	_queueIndex = (_queueIndex + 1) % 2;

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanPostProcessing::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by Shell once per run, just before exiting the program.
/// quitApplication() will not be called every time the rendering context is lost, only before application exit.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanPostProcessing::quitApplication()
{
	return pvr::Result::Success;
}

/// <summary>Creates The UI renderer.</summary>
void VulkanPostProcessing::createUiRenderer()
{
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenRenderPass, 0, getBackBufferColorspace() == pvr::ColorSpace::sRGB,
		_deviceResources->commandPool, _deviceResources->queues[0]);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("PostProcessing");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText("Left / right: Blur Mode\n"
															   "Up / Down: Blur Size\n"
															   "Action 1: Enable/Disable Bloom\n"
															   "Action 2: Enable/Disable Animation\n");
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	updateBlurDescription();
	_deviceResources->uiRenderer.getDefaultDescription()->setText(_currentBlurString);
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
}

/// <summary>Updates the description for the currently used blur technique.</summary>
void VulkanPostProcessing::updateBlurDescription()
{
	switch (_blurMode)
	{
	case (BloomMode::NoBloom):
	{
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)];
		break;
	}
	case (BloomMode::GaussianOriginal):
	{
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" + DemoConfigurations::Configurations[_currentDemoConfiguration].gaussianConfig.second;
		break;
	}
	case (BloomMode::GaussianLinear):
	{
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" + DemoConfigurations::Configurations[_currentDemoConfiguration].linearGaussianConfig.second;
		break;
	}
	case (BloomMode::GaussianLinearTruncated):
	{
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" + DemoConfigurations::Configurations[_currentDemoConfiguration].truncatedLinearGaussianConfig.second;
		break;
	}
	case (BloomMode::Compute):
	{
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" + DemoConfigurations::Configurations[_currentDemoConfiguration].computeGaussianConfig.second;
		break;
	}
	case (BloomMode::DualFilter):
	{
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" + DemoConfigurations::Configurations[_currentDemoConfiguration].dualFilterConfig.second;
		break;
	}
	case (BloomMode::TentFilter):
	{
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" + DemoConfigurations::Configurations[_currentDemoConfiguration].tentFilterConfig.second;
		break;
	}
	case (BloomMode::HybridGaussian):
	{
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" + DemoConfigurations::Configurations[_currentDemoConfiguration].hybridConfig.second;
		break;
	}
	case (BloomMode::Kawase):
	{
		_currentBlurString = BloomStrings[static_cast<uint32_t>(_blurMode)] + "\n" + DemoConfigurations::Configurations[_currentDemoConfiguration].kawaseConfig.second;
		break;
	}
	default:
		throw pvr::UnsupportedOperationError("Unsupported BlurMode.");
	}

	Log(LogLevel::Information, "Current blur mode: \"%s\"", BloomStrings[static_cast<int32_t>(_blurMode)].c_str());
	Log(LogLevel::Information, "Current blur size configiuration: \"%u\"", _currentDemoConfiguration);
}

/// <summary>Creates the main scene buffer.</summary>
void VulkanPostProcessing::createSceneBuffers()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(BufferEntryNames::Scene::InverseViewProjectionMatrix, pvr::GpuDatatypes::mat4x4);
	desc.addElement(BufferEntryNames::Scene::EyePosition, pvr::GpuDatatypes::vec3);
	desc.addElement(BufferEntryNames::Scene::LightPosition, pvr::GpuDatatypes::vec3);

	_deviceResources->sceneBufferView.initDynamic(desc, _numSwapImages, pvr::BufferUsageFlags::UniformBuffer,
		_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment());

	_deviceResources->sceneBuffer = pvr::utils::createBuffer(_deviceResources->device, _deviceResources->sceneBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT,
		pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT, &_deviceResources->vmaAllocator);

	_deviceResources->sceneBufferView.pointToMappedMemory(_deviceResources->sceneBuffer->getDeviceMemory()->getMappedData());
}

/// <summary>Creates the bloom threshold buffer.</summary>
void VulkanPostProcessing::createBloomThresholdBuffer()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement(BufferEntryNames::BloomConfig::LuminosityThreshold, pvr::GpuDatatypes::Float);

	_deviceResources->bloomConfigBufferView.initDynamic(desc, _deviceResources->swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
		_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment());

	_deviceResources->bloomConfigBuffer =
		pvr::utils::createBuffer(_deviceResources->device, _deviceResources->bloomConfigBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT,
			pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT, &_deviceResources->vmaAllocator);

	_deviceResources->bloomConfigBufferView.pointToMappedMemory(_deviceResources->bloomConfigBuffer->getDeviceMemory()->getMappedData());
}

/// <summary>Creates main application buffers.</summary>
void VulkanPostProcessing::createBuffers()
{
	createSceneBuffers();
	createBloomThresholdBuffer();

	// update the bloom threshold buffer
	if (_useThreshold)
	{
		// This threshold values controls the minimum luminosity value any fragment must have to be used as part of the bloom
		_bloomLumaThreshold = BloomLumaThreshold;
	}
	else
	{
		_bloomLumaThreshold = 0.0;
	}

	_deviceResources->bloomConfigBufferView.getElementByName(BufferEntryNames::BloomConfig::LuminosityThreshold).setValue(_bloomLumaThreshold);
}

/// <summary>Records the main command buffers used for rendering the demo.</summary>
/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
void VulkanPostProcessing::recordMainCommandBuffer(uint32_t swapchainIndex)
{
	_deviceResources->mainCommandBuffers[swapchainIndex]->begin();

	_deviceResources->mainCommandBuffers[swapchainIndex]->debugMarkerBeginEXT(pvr::strings::createFormatted("Render Scene - swapchain: %i", swapchainIndex));

	const pvrvk::ClearValue offScreenClearValues[] = { pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 1.f), pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 1.f),
		pvrvk::ClearValue::createDefaultDepthStencilClearValue() };

	// Render the main scene
	_deviceResources->mainCommandBuffers[swapchainIndex]->beginRenderPass(_deviceResources->offScreenFramebuffers[swapchainIndex], _deviceResources->offScreenRenderPass,
		pvrvk::Rect2D(0, 0, getWidth(), getHeight()), false, offScreenClearValues, ARRAY_SIZE(offScreenClearValues));
	_deviceResources->mainCommandBuffers[swapchainIndex]->executeCommands(_deviceResources->statuePass.commandBuffers[swapchainIndex]);
	_deviceResources->mainCommandBuffers[swapchainIndex]->executeCommands(_deviceResources->skyBoxPass.commandBuffers[swapchainIndex]);
	_deviceResources->mainCommandBuffers[swapchainIndex]->endRenderPass();

	_deviceResources->mainCommandBuffers[swapchainIndex]->debugMarkerEndEXT();

	pvrvk::ClearValue clearValues = pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 1.f);

	// Downsample the luminance image view
	if (_blurMode != BloomMode::NoBloom)
	{
		// When using Dual/Tent filter no downsample is required as they take care of downsampling theirselves
		if (!(_blurMode == BloomMode::DualFilter || _blurMode == BloomMode::TentFilter))
		{
			// Use a special cased downsample pass when the next pass will be using compute
			if (_blurMode == BloomMode::Compute || _blurMode == BloomMode::HybridGaussian)
			{
				_deviceResources->computeDownsamplePass.recordCommands(swapchainIndex);

				_deviceResources->mainCommandBuffers[swapchainIndex]->beginRenderPass(_deviceResources->computeDownsamplePass.framebuffers[swapchainIndex],
					_deviceResources->computeDownsamplePass.renderPass, pvrvk::Rect2D(0, 0, _blurFramebufferDimensions.x, _blurFramebufferDimensions.y), false, &clearValues, 1);
				_deviceResources->mainCommandBuffers[swapchainIndex]->executeCommands(_deviceResources->computeDownsamplePass.commandBuffers[swapchainIndex]);
				_deviceResources->mainCommandBuffers[swapchainIndex]->endRenderPass();
			}
			else
			{
				_deviceResources->downsamplePass.recordCommands(swapchainIndex);

				_deviceResources->mainCommandBuffers[swapchainIndex]->beginRenderPass(_deviceResources->downsamplePass.framebuffers[swapchainIndex],
					_deviceResources->downsamplePass.renderPass, pvrvk::Rect2D(0, 0, _blurFramebufferDimensions.x, _blurFramebufferDimensions.y), false, &clearValues, 1);
				_deviceResources->mainCommandBuffers[swapchainIndex]->executeCommands(_deviceResources->downsamplePass.commandBuffers[swapchainIndex]);
				_deviceResources->mainCommandBuffers[swapchainIndex]->endRenderPass();
			}
		}

		// Record the current set of commands for bloom
		recordBlurCommands(_blurMode, swapchainIndex);

		switch (_blurMode)
		{
		case (BloomMode::GaussianOriginal):
		{
			_deviceResources->gaussianBlurPass.recordCommandsToMainCommandBuffer(swapchainIndex, _deviceResources->mainCommandBuffers[swapchainIndex], _deviceResources->queues[0],
				_deviceResources->blurRenderPass, _deviceResources->blurFramebuffers[0], _deviceResources->blurFramebuffers[1]);
			break;
		}
		case (BloomMode::GaussianLinear):
		{
			_deviceResources->linearGaussianBlurPass.recordCommandsToMainCommandBuffer(swapchainIndex, _deviceResources->mainCommandBuffers[swapchainIndex],
				_deviceResources->queues[0], _deviceResources->blurRenderPass, _deviceResources->blurFramebuffers[0], _deviceResources->blurFramebuffers[1]);
			break;
		}
		case (BloomMode::GaussianLinearTruncated):
		{
			_deviceResources->truncatedLinearGaussianBlurPass.recordCommandsToMainCommandBuffer(swapchainIndex, _deviceResources->mainCommandBuffers[swapchainIndex],
				_deviceResources->queues[0], _deviceResources->blurRenderPass, _deviceResources->blurFramebuffers[0], _deviceResources->blurFramebuffers[1]);
			break;
		}
		case (BloomMode::Compute):
		{
			// Graphics to Compute pipeline barrier (Downsample -> Compute Blur (horizontal))
			// Add a pipelineBarrier between fragment write (Downsample) -> shader read (Compute Blur (horizontal))
			{
				pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL;
				pvrvk::ImageLayout destinationImageLayout = pvrvk::ImageLayout::e_GENERAL;

				pvrvk::MemoryBarrierSet layoutTransitions;
				layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_WRITE_BIT,
					_deviceResources->storageImagePingPongImageViews[1][swapchainIndex]->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT),
					sourceImageLayout, destinationImageLayout, _deviceResources->queues[0]->getFamilyIndex(), _deviceResources->queues[0]->getFamilyIndex()));

				_deviceResources->mainCommandBuffers[swapchainIndex]->pipelineBarrier(
					pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, layoutTransitions);
			}

			_deviceResources->computeBlurPass.recordCommandsToMainCommandBuffer(swapchainIndex, _deviceResources->mainCommandBuffers[swapchainIndex]);
			break;
		}
		case (BloomMode::Kawase):
		{
			_deviceResources->kawaseBlurPass.recordCommandsToMainCommandBuffer(swapchainIndex, _deviceResources->mainCommandBuffers[swapchainIndex], _deviceResources->queues[0],
				_deviceResources->blurRenderPass, _deviceResources->blurFramebuffers);
			break;
		}
		case (BloomMode::DualFilter):
		{
			_deviceResources->dualFilterBlurPass.recordCommandsToMainCommandBuffer(swapchainIndex, _deviceResources->mainCommandBuffers[swapchainIndex], _deviceResources->queues[0],
				_deviceResources->blurRenderPass, _deviceResources->onScreenRenderPass, _deviceResources->onScreenFramebuffers[swapchainIndex], &clearValues, 1);
			break;
		}
		case (BloomMode::TentFilter):
		{
			_deviceResources->downAndTentFilterBlurPass.recordCommandsToMainCommandBuffer(swapchainIndex, _deviceResources->mainCommandBuffers[swapchainIndex],
				_deviceResources->queues[0], _deviceResources->blurRenderPass, _deviceResources->onScreenRenderPass, _deviceResources->onScreenFramebuffers[swapchainIndex],
				&clearValues, 1);
			break;
		}
		case (BloomMode::HybridGaussian):
		{
			// Graphics to Compute pipeline barrier (Downsample -> Compute Blur (horizontal))
			// Add a pipelineBarrier between fragment write (Downsample) -> shader read (Compute Blur (horizontal))
			{
				pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL;
				pvrvk::ImageLayout destinationImageLayout = pvrvk::ImageLayout::e_GENERAL;

				pvrvk::MemoryBarrierSet layoutTransitions;
				layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_WRITE_BIT,
					_deviceResources->storageImagePingPongImageViews[1][swapchainIndex]->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT),
					sourceImageLayout, destinationImageLayout, _deviceResources->queues[0]->getFamilyIndex(), _deviceResources->queues[0]->getFamilyIndex()));

				_deviceResources->mainCommandBuffers[swapchainIndex]->pipelineBarrier(
					pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, layoutTransitions);
			}

			_deviceResources->hybridGaussianBlurPass.recordCommandsToMainCommandBuffer(swapchainIndex, _deviceResources->mainCommandBuffers[swapchainIndex],
				_deviceResources->blurRenderPass, _deviceResources->hybridBlurFramebuffers[0], _deviceResources->hybridBlurFramebuffers[1]);

			break;
		}
		default:
			throw pvr::UnsupportedOperationError("Unsupported BlurMode.");
		}
	}

	// If Dual or Tent filter then the composition is taken care of during the final up sample
	if (_blurMode != BloomMode::DualFilter && _blurMode != BloomMode::TentFilter)
	{
		_deviceResources->mainCommandBuffers[swapchainIndex]->beginRenderPass(
			_deviceResources->onScreenFramebuffers[swapchainIndex], _deviceResources->onScreenRenderPass, pvrvk::Rect2D(0, 0, getWidth(), getHeight()), false, &clearValues, 1);

		// Ensure the post bloom pass uses the correct blurred image for the current blur mode
		switch (_blurMode)
		{
		case (BloomMode::GaussianOriginal):
		{
			_deviceResources->postBloomPass.updateDescriptorSets(_deviceResources->device, swapchainIndex, _deviceResources->offScreenColorImageViews[swapchainIndex],
				_deviceResources->gaussianBlurPass.getBlurredImage(swapchainIndex), _deviceResources->samplerBilinear);
			break;
		}
		case (BloomMode::GaussianLinear):
		{
			_deviceResources->postBloomPass.updateDescriptorSets(_deviceResources->device, swapchainIndex, _deviceResources->offScreenColorImageViews[swapchainIndex],
				_deviceResources->linearGaussianBlurPass.getBlurredImage(swapchainIndex), _deviceResources->samplerBilinear);
			break;
		}
		case (BloomMode::Compute):
		{
			_deviceResources->postBloomPass.updateDescriptorSets(_deviceResources->device, swapchainIndex, _deviceResources->offScreenColorImageViews[swapchainIndex],
				_deviceResources->computeBlurPass.getBlurredImage(swapchainIndex), _deviceResources->samplerBilinear);
			break;
		}
		case (BloomMode::GaussianLinearTruncated):
		{
			_deviceResources->postBloomPass.updateDescriptorSets(_deviceResources->device, swapchainIndex, _deviceResources->offScreenColorImageViews[swapchainIndex],
				_deviceResources->truncatedLinearGaussianBlurPass.getBlurredImage(swapchainIndex), _deviceResources->samplerBilinear);
			break;
		}
		case (BloomMode::Kawase):
		{
			_deviceResources->postBloomPass.updateDescriptorSets(_deviceResources->device, swapchainIndex, _deviceResources->offScreenColorImageViews[swapchainIndex],
				_deviceResources->kawaseBlurPass.getBlurredImage(swapchainIndex), _deviceResources->samplerBilinear);
			break;
		}
		case (BloomMode::HybridGaussian):
		{
			_deviceResources->postBloomPass.updateDescriptorSets(_deviceResources->device, swapchainIndex, _deviceResources->offScreenColorImageViews[swapchainIndex],
				_deviceResources->hybridGaussianBlurPass.linearBlurPass->getBlurredImage(swapchainIndex), _deviceResources->samplerBilinear);
			break;
		}
		case (BloomMode::NoBloom):
		{
			_deviceResources->postBloomPass.updateDescriptorSets(_deviceResources->device, swapchainIndex, _deviceResources->offScreenColorImageViews[swapchainIndex],
				_deviceResources->luminanceImageViews[swapchainIndex], _deviceResources->samplerBilinear);
			break;
		}
		default:
			throw pvr::UnsupportedOperationError("Unsupported BlurMode.");
		}

		_deviceResources->postBloomPass.recordCommandBuffer(swapchainIndex, _deviceResources->onScreenFramebuffers[swapchainIndex], _renderOnlyBloom, _blurMode == BloomMode::NoBloom);
		_deviceResources->mainCommandBuffers[swapchainIndex]->executeCommands(_deviceResources->postBloomPass.commandBuffers[swapchainIndex]);
	}

	_deviceResources->mainCommandBuffers[swapchainIndex]->executeCommands(_deviceResources->bloomUiRendererCommandBuffers[swapchainIndex]);
	_deviceResources->mainCommandBuffers[swapchainIndex]->endRenderPass();
	_deviceResources->mainCommandBuffers[swapchainIndex]->end();
}

/// <summary>Records the commands necessary for the current bloom technique.</summary>
/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
void VulkanPostProcessing::recordBlurCommands(BloomMode blurMode, uint32_t swapchainIndex)
{
	switch (blurMode)
	{
	case (BloomMode::GaussianOriginal):
	{
		_deviceResources->gaussianBlurPass.recordCommands(swapchainIndex, _deviceResources->blurFramebuffers[0], _deviceResources->blurFramebuffers[1]);
		break;
	}
	case (BloomMode::GaussianLinear):
	{
		_deviceResources->linearGaussianBlurPass.recordCommands(swapchainIndex, _deviceResources->blurFramebuffers[0], _deviceResources->blurFramebuffers[1]);
		break;
	}
	case (BloomMode::GaussianLinearTruncated):
	{
		_deviceResources->truncatedLinearGaussianBlurPass.recordCommands(swapchainIndex, _deviceResources->blurFramebuffers[0], _deviceResources->blurFramebuffers[1]);
		break;
	}
	case (BloomMode::Compute):
	{
		_deviceResources->computeBlurPass.recordCommands(
			swapchainIndex, _deviceResources->storageImagePingPongImageViews[0], _deviceResources->storageImagePingPongImageViews[1], _deviceResources->queues[_queueIndex]);
		break;
	}
	case (BloomMode::Kawase):
	{
		_deviceResources->kawaseBlurPass.recordCommands(swapchainIndex, _deviceResources->blurFramebuffers);
		break;
	}
	case (BloomMode::DualFilter):
	{
		_deviceResources->dualFilterBlurPass.recordCommands(swapchainIndex, _deviceResources->onScreenFramebuffers[swapchainIndex], _renderOnlyBloom);
		break;
	}
	case (BloomMode::TentFilter):
	{
		_deviceResources->downAndTentFilterBlurPass.recordCommands(swapchainIndex, _deviceResources->onScreenFramebuffers[swapchainIndex], _renderOnlyBloom,
			_deviceResources->queues[_queueIndex], _deviceResources->luminanceImageViews[_deviceResources->swapchain->getSwapchainIndex()]);
		break;
	}
	case (BloomMode::HybridGaussian):
	{
		_deviceResources->hybridGaussianBlurPass.recordCommands(
			swapchainIndex, _deviceResources->hybridBlurFramebuffers[0], _deviceResources->hybridBlurFramebuffers[1], _deviceResources->queues[_queueIndex]);
		break;
	}
	default:
		throw pvr::UnsupportedOperationError("Unsupported BlurMode.");
	}
}

/// <summary>Allocates the various ping pong image views used throughout the demo.</summary>
void VulkanPostProcessing::allocatePingPongImages()
{
	pvrvk::Extent3D dimension = pvrvk::Extent3D(_blurFramebufferDimensions.x, _blurFramebufferDimensions.y, 1u);

	// If the storage image luminance color format matches the standard luminance color format then we only need allocate 2 images
	if (_storageImageLuminanceColorFormat == _luminanceColorFormat)
	{
		// Allocate the luminance render targets (we need to ping pong between 2 targets)
		pvrvk::ImageUsageFlags imageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT | pvrvk::ImageUsageFlags::e_STORAGE_BIT;

		for (uint32_t i = 0; i < 2; ++i)
		{
			for (uint32_t j = 0; j < _numSwapImages; ++j)
			{
				pvrvk::Image blurColorTexture = pvr::utils::createImage(_deviceResources->device, pvrvk::ImageType::e_2D, _luminanceColorFormat, dimension, imageUsage,
					static_cast<pvrvk::ImageCreateFlags>(0), pvrvk::ImageLayersSize(), pvrvk::SampleCountFlags::e_1_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
					pvrvk::MemoryPropertyFlags::e_NONE, &_deviceResources->vmaAllocator);

				_deviceResources->pingPongImageViews[i].add(_deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(blurColorTexture)));
				_deviceResources->storageImagePingPongImageViews[i] = _deviceResources->pingPongImageViews[i];
			}
		}
	}
	// else 2 sets of images need allocating
	else
	{
		// Allocate the luminance render targets (we need to ping pong between 2 targets)
		pvrvk::ImageUsageFlags imageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT;
		pvrvk::ImageUsageFlags storageImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT | pvrvk::ImageUsageFlags::e_STORAGE_BIT;

		for (uint32_t i = 0; i < 2; ++i)
		{
			for (uint32_t j = 0; j < _numSwapImages; ++j)
			{
				pvrvk::Image blurColorTexture = pvr::utils::createImage(_deviceResources->device, pvrvk::ImageType::e_2D, _luminanceColorFormat, dimension, imageUsage,
					static_cast<pvrvk::ImageCreateFlags>(0), pvrvk::ImageLayersSize(), pvrvk::SampleCountFlags::e_1_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
					pvrvk::MemoryPropertyFlags::e_NONE, &_deviceResources->vmaAllocator);

				_deviceResources->pingPongImageViews[i].add(_deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(blurColorTexture)));

				pvrvk::Image storageImageColorTexture = pvr::utils::createImage(_deviceResources->device, pvrvk::ImageType::e_2D, _storageImageLuminanceColorFormat, dimension,
					storageImageUsage, static_cast<pvrvk::ImageCreateFlags>(0), pvrvk::ImageLayersSize(), pvrvk::SampleCountFlags::e_1_BIT,
					pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, &_deviceResources->vmaAllocator,
					pvr::utils::vma::AllocationCreateFlags::e_NONE, pvrvk::SharingMode::e_EXCLUSIVE, _storageImageTiling);
				_deviceResources->storageImagePingPongImageViews[i].add(_deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(storageImageColorTexture)));
			}
		}
	}
}

/// <summary>Creates the various samplers used throughout the demo.</summary>
void VulkanPostProcessing::createSamplers()
{
	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.wrapModeU = samplerInfo.wrapModeV = samplerInfo.wrapModeW = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;

	samplerInfo.minFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.magFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
	_deviceResources->samplerBilinear = _deviceResources->device->createSampler(samplerInfo);

	samplerInfo.minFilter = pvrvk::Filter::e_NEAREST;
	samplerInfo.magFilter = pvrvk::Filter::e_NEAREST;
	_deviceResources->samplerNearest = _deviceResources->device->createSampler(samplerInfo);

	samplerInfo.magFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.minFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_LINEAR;
	_deviceResources->samplerTrilinear = _deviceResources->device->createSampler(samplerInfo);
}

/// <summary>Create the framebuffers which will be used in the various bloom passes.</summary>
void VulkanPostProcessing::createBlurFramebuffers()
{
	for (uint32_t i = 0; i < 2; ++i)
	{
		for (uint32_t j = 0; j < _numSwapImages; ++j)
		{
			pvrvk::FramebufferCreateInfo createInfo;
			createInfo.setAttachment(0, _deviceResources->pingPongImageViews[!i][j]);
			createInfo.setDimensions(_blurFramebufferDimensions.x, _blurFramebufferDimensions.y);
			createInfo.setRenderPass(_deviceResources->blurRenderPass);

			_deviceResources->blurFramebuffers[i].add(_deviceResources->device->createFramebuffer(createInfo));
		}
	}
}

/// <summary>Create the framebuffers which will be used in the hybrid bloom pass.</summary>
void VulkanPostProcessing::createHybridBlurFramebuffers()
{
	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		pvrvk::FramebufferCreateInfo createInfo;
		createInfo.setAttachment(0, _deviceResources->storageImagePingPongImageViews[1][i]);
		createInfo.setDimensions(_blurFramebufferDimensions.x, _blurFramebufferDimensions.y);
		createInfo.setRenderPass(_deviceResources->hybridBlurRenderPass);

		_deviceResources->hybridBlurFramebuffers[0].add(_deviceResources->device->createFramebuffer(createInfo));
	}

	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		pvrvk::FramebufferCreateInfo createInfo;
		createInfo.setAttachment(0, _deviceResources->pingPongImageViews[0][i]);
		createInfo.setDimensions(_blurFramebufferDimensions.x, _blurFramebufferDimensions.y);
		createInfo.setRenderPass(_deviceResources->blurRenderPass);

		_deviceResources->hybridBlurFramebuffers[1].add(_deviceResources->device->createFramebuffer(createInfo));
	}
}

/// <summary>Create the RenderPasses used in the various bloom passes.</summary>
void VulkanPostProcessing::createBlurRenderPass()
{
	pvrvk::RenderPassCreateInfo renderPassInfo;

	renderPassInfo.setAttachmentDescription(0,
		pvrvk::AttachmentDescription::createColorDescription(_deviceResources->pingPongImageViews[0][0]->getFormat(), pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_DONT_CARE, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT));

	pvrvk::SubpassDescription subpass;
	subpass.setColorAttachmentReference(0, pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	renderPassInfo.setSubpass(0, subpass);

	// Add external subpass dependencies to avoid the implicit subpass depedencies and to provide more optimal pipeline stage task synchronisation
	pvrvk::SubpassDependency externalDependencies[2];
	externalDependencies[0] = pvrvk::SubpassDependency(pvrvk::SubpassExternal, 0, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT,
		pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::DependencyFlags::e_NONE);

	externalDependencies[1] = pvrvk::SubpassDependency(0, pvrvk::SubpassExternal, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT,
		pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::DependencyFlags::e_NONE);

	renderPassInfo.addSubpassDependency(externalDependencies[0]);
	renderPassInfo.addSubpassDependency(externalDependencies[1]);

	_deviceResources->blurRenderPass = _deviceResources->device->createRenderPass(renderPassInfo);
}

/// <summary>Create the RenderPasses used in the hybrid bloom passes.</summary>
void VulkanPostProcessing::createHybridBlurRenderPass()
{
	pvrvk::RenderPassCreateInfo renderPassInfo;

	renderPassInfo.setAttachmentDescription(0,
		pvrvk::AttachmentDescription::createColorDescription(_deviceResources->storageImagePingPongImageViews[0][0]->getFormat(), pvrvk::ImageLayout::e_GENERAL,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_DONT_CARE, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT));

	pvrvk::SubpassDescription subpass;
	subpass.setColorAttachmentReference(0, pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	renderPassInfo.setSubpass(0, subpass);

	// Add external subpass dependencies to avoid the implicit subpass depedencies and to provide more optimal pipeline stage task synchronisation
	pvrvk::SubpassDependency externalDependencies[2];
	externalDependencies[0] = pvrvk::SubpassDependency(pvrvk::SubpassExternal, 0, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT,
		pvrvk::AccessFlags::e_SHADER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::DependencyFlags::e_NONE);

	externalDependencies[1] = pvrvk::SubpassDependency(0, pvrvk::SubpassExternal, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT,
		pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::DependencyFlags::e_NONE);

	renderPassInfo.addSubpassDependency(externalDependencies[0]);
	renderPassInfo.addSubpassDependency(externalDependencies[1]);

	_deviceResources->hybridBlurRenderPass = _deviceResources->device->createRenderPass(renderPassInfo);
}

/// <summary>Create the offscreen framebuffers used in the application.</summary>
void VulkanPostProcessing::createOffScreenFramebuffers()
{
	pvrvk::RenderPassCreateInfo renderPassInfo;

	// Off-Screen attachment
	renderPassInfo.setAttachmentDescription(0,
		pvrvk::AttachmentDescription::createColorDescription(pvrvk::Format::e_R16G16B16A16_SFLOAT, pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL,
			pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT));

	renderPassInfo.setAttachmentDescription(1,
		pvrvk::AttachmentDescription::createColorDescription(_luminanceColorFormat, pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL,
			pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT));

	renderPassInfo.setAttachmentDescription(2,
		pvrvk::AttachmentDescription::createDepthStencilDescription(_deviceResources->depthStencilImages[0]->getImage()->getFormat(), pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE, pvrvk::AttachmentLoadOp::e_CLEAR,
			pvrvk::AttachmentStoreOp::e_DONT_CARE, pvrvk::SampleCountFlags::e_1_BIT));

	pvrvk::SubpassDescription localMemorySubpasses[2];

	// Subpass 0
	localMemorySubpasses[0].setColorAttachmentReference(0, pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	localMemorySubpasses[0].setColorAttachmentReference(1, pvrvk::AttachmentReference(1, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	localMemorySubpasses[0].setDepthStencilAttachmentReference(pvrvk::AttachmentReference(2, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
	renderPassInfo.setSubpass(0, localMemorySubpasses[0]);

	// Add external subpass dependencies to avoid the implicit subpass depedencies and to provide more optimal pipeline stage task synchronisation
	pvrvk::SubpassDependency externalDependencies[2];
	externalDependencies[0] = pvrvk::SubpassDependency(pvrvk::SubpassExternal, 0, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT,
		pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::DependencyFlags::e_NONE);

	externalDependencies[1] = pvrvk::SubpassDependency(0, pvrvk::SubpassExternal, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT,
		pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::DependencyFlags::e_NONE);

	renderPassInfo.addSubpassDependency(externalDependencies[0]);
	renderPassInfo.addSubpassDependency(externalDependencies[1]);

	// Create the renderpass
	_deviceResources->offScreenRenderPass = _deviceResources->device->createRenderPass(renderPassInfo);

	const pvrvk::Extent3D& dimension = pvrvk::Extent3D(_deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight(), 1u);
	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		pvrvk::ImageUsageFlags imageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT;
		if (_supportsBlit)
		{
			imageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_DST_BIT;
		}

		// Allocate the HDR luminance texture
		pvrvk::Image luminanceColorTexture = pvr::utils::createImage(_deviceResources->device, pvrvk::ImageType::e_2D, _luminanceColorFormat, dimension, imageUsage,
			static_cast<pvrvk::ImageCreateFlags>(0), pvrvk::ImageLayersSize(), pvrvk::SampleCountFlags::e_1_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			pvrvk::MemoryPropertyFlags::e_NONE, &_deviceResources->vmaAllocator);

		_deviceResources->luminanceImageViews.add(_deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(luminanceColorTexture)));

		pvrvk::FramebufferCreateInfo offScreenFramebufferCreateInfo;

		// Allocate the HDR color texture
		pvrvk::Image colorTexture = pvr::utils::createImage(_deviceResources->device, pvrvk::ImageType::e_2D, pvrvk::Format::e_R16G16B16A16_SFLOAT, dimension,
			pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT | pvrvk::ImageUsageFlags::e_INPUT_ATTACHMENT_BIT,
			static_cast<pvrvk::ImageCreateFlags>(0), pvrvk::ImageLayersSize(), pvrvk::SampleCountFlags::e_1_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			pvrvk::MemoryPropertyFlags::e_NONE, &_deviceResources->vmaAllocator);

		_deviceResources->offScreenColorImageViews.add(_deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(colorTexture)));

		offScreenFramebufferCreateInfo.setAttachment(0, _deviceResources->offScreenColorImageViews[i]);
		offScreenFramebufferCreateInfo.setAttachment(1, _deviceResources->luminanceImageViews[i]);
		offScreenFramebufferCreateInfo.setAttachment(2, _deviceResources->depthStencilImages[i]);
		offScreenFramebufferCreateInfo.setDimensions(_deviceResources->swapchain->getDimension());
		offScreenFramebufferCreateInfo.setRenderPass(_deviceResources->offScreenRenderPass);

		_deviceResources->offScreenFramebuffers[i] = _deviceResources->device->createFramebuffer(offScreenFramebufferCreateInfo);
	}
}

/// <summary>Update the various dynamic scene data used in the application.</summary>
void VulkanPostProcessing::updateDynamicSceneData()
{
	// Update object animations
	updateAnimation();

	// Update the animation data used in the statue pass
	_deviceResources->statuePass.updateAnimation(_objectAngleY, _viewProjectionMatrix, _deviceResources->swapchain->getSwapchainIndex());

	// Update the Scene Buffer
	_deviceResources->sceneBufferView.getElementByName(BufferEntryNames::Scene::InverseViewProjectionMatrix, 0, _swapchainIndex).setValue(glm::inverse(_viewProjectionMatrix));
	_deviceResources->sceneBufferView.getElementByName(BufferEntryNames::Scene::EyePosition, 0, _swapchainIndex).setValue(_camera.getCameraPosition());
	_deviceResources->sceneBufferView.getElementByName(BufferEntryNames::Scene::LightPosition, 0, _swapchainIndex).setValue(_lightPosition);

	// Update any bloom configuration buffers currently required
	if (_mustUpdatePerSwapchainDemoConfig[_deviceResources->swapchain->getSwapchainIndex()])
	{
		switch (_blurMode)
		{
		case (BloomMode::GaussianOriginal):
		{
			_deviceResources->gaussianBlurPass.updateKernelBuffer(_deviceResources->swapchain->getSwapchainIndex());
			break;
		}
		case (BloomMode::GaussianLinear):
		{
			_deviceResources->linearGaussianBlurPass.updateKernelBuffer(_deviceResources->swapchain->getSwapchainIndex());
			break;
		}
		case (BloomMode::GaussianLinearTruncated):
		{
			_deviceResources->truncatedLinearGaussianBlurPass.updateKernelBuffer(_deviceResources->swapchain->getSwapchainIndex());
			break;
		}
		case (BloomMode::Compute):
		{
			_deviceResources->computeBlurPass.updateKernelBuffer(_deviceResources->swapchain->getSwapchainIndex());
			break;
		}
		case (BloomMode::DualFilter):
		{
			_deviceResources->dualFilterBlurPass.updateDescriptorSets(_deviceResources->device, _deviceResources->swapchain->getSwapchainIndex(),
				_deviceResources->offScreenColorImageViews[_deviceResources->swapchain->getSwapchainIndex()],
				_deviceResources->luminanceImageViews[_deviceResources->swapchain->getSwapchainIndex()], _deviceResources->samplerBilinear);
			break;
		}
		case (BloomMode::TentFilter):
		{
			_deviceResources->downAndTentFilterBlurPass.updateDescriptorSets(_deviceResources->device, _deviceResources->swapchain->getSwapchainIndex(),
				_deviceResources->offScreenColorImageViews[_deviceResources->swapchain->getSwapchainIndex()],
				_deviceResources->luminanceImageViews[_deviceResources->swapchain->getSwapchainIndex()], _deviceResources->samplerBilinear);
			break;
		}
		case (BloomMode::HybridGaussian):
		{
			_deviceResources->hybridGaussianBlurPass.linearBlurPass->updateKernelBuffer(_deviceResources->swapchain->getSwapchainIndex());
			_deviceResources->hybridGaussianBlurPass.computeBlurPass->updateKernelBuffer(_deviceResources->swapchain->getSwapchainIndex());
			break;
		}
		default:
			break;
		}

		_mustUpdatePerSwapchainDemoConfig[_deviceResources->swapchain->getSwapchainIndex()] = false;
	}
}

/// <summary>Update the animations for the current frame.</summary>
void VulkanPostProcessing::updateAnimation()
{
	if (_animateCamera)
	{
		_cameraAngle += 0.15f;

		if (_cameraAngle >= 360.0f)
		{
			_cameraAngle = _cameraAngle - 360.f;
		}
	}

	_camera.setTargetLookAngle(_cameraAngle);

	_viewMatrix = _camera.getViewMatrix();
	_viewProjectionMatrix = _projectionMatrix * _viewMatrix;

	if (_animateObject)
	{
		_objectAngleY += RotateY * 0.03f * getFrameTime();
	}

	float dt = getFrameTime() * 0.001f;
	_logicTime += dt;
	if (_logicTime > 10000000)
	{
		_logicTime = 0;
	}

	if (!_isManual)
	{
		if (_logicTime > _modeSwitchTime + _modeDuration)
		{
			_modeSwitchTime = _logicTime;

			if (_blurMode != BloomMode::NoBloom)
			{
				// Increase the demo configuration
				_currentDemoConfiguration = (_currentDemoConfiguration + 1) % DemoConfigurations::NumDemoConfigurations;
			}
			// Change to the next bloom mode
			if (_currentDemoConfiguration == 0 || _blurMode == BloomMode::NoBloom)
			{
				uint32_t currentBlurMode = static_cast<uint32_t>(_blurMode);
				currentBlurMode += 1;
				currentBlurMode = (currentBlurMode + static_cast<uint32_t>(BloomMode::NumBloomModes)) % static_cast<uint32_t>(BloomMode::NumBloomModes);
				_blurMode = static_cast<BloomMode>(currentBlurMode);
			}

			updateBloomConfiguration();
		}
	}
}

/// <summary>Update the demo configuration in use. Calculates gaussian weights and offsets, images being used, framebuffers being used etc.</summary>
void VulkanPostProcessing::updateDemoConfigs()
{
	switch (_blurMode)
	{
	case (BloomMode::GaussianOriginal):
	{
		_deviceResources->gaussianBlurPass.updateKernelConfig(DemoConfigurations::Configurations[_currentDemoConfiguration].gaussianConfig.first, false, false);
		break;
	}
	case (BloomMode::GaussianLinear):
	{
		_deviceResources->linearGaussianBlurPass.updateKernelConfig(DemoConfigurations::Configurations[_currentDemoConfiguration].linearGaussianConfig.first, true, false);
		break;
	}
	case (BloomMode::GaussianLinearTruncated):
	{
		_deviceResources->truncatedLinearGaussianBlurPass.updateKernelConfig(
			DemoConfigurations::Configurations[_currentDemoConfiguration].truncatedLinearGaussianConfig.first, true, true);
		break;
	}
	case (BloomMode::Kawase):
	{
		_deviceResources->kawaseBlurPass.updateConfig(DemoConfigurations::Configurations[_currentDemoConfiguration].kawaseConfig.first.kernel,
			DemoConfigurations::Configurations[_currentDemoConfiguration].kawaseConfig.first.numIterations, _deviceResources->pingPongImageViews, 2);
		break;
	}
	case (BloomMode::Compute):
	{
		_deviceResources->computeBlurPass.updateKernelConfig(DemoConfigurations::Configurations[_currentDemoConfiguration].computeGaussianConfig.first, false, false);
		break;
	}
	case (BloomMode::DualFilter):
	{
		_deviceResources->dualFilterBlurPass.updateConfig(DemoConfigurations::Configurations[_currentDemoConfiguration].dualFilterConfig.first);
		break;
	}
	case (BloomMode::TentFilter):
	{
		_deviceResources->downAndTentFilterBlurPass.updateConfig(DemoConfigurations::Configurations[_currentDemoConfiguration].dualFilterConfig.first);
		break;
	}
	case (BloomMode::HybridGaussian):
	{
		_deviceResources->truncatedLinearGaussianBlurPass.updateKernelConfig(
			DemoConfigurations::Configurations[_currentDemoConfiguration].truncatedLinearGaussianConfig.first, true, true);
		_deviceResources->computeBlurPass.updateKernelConfig(DemoConfigurations::Configurations[_currentDemoConfiguration].computeGaussianConfig.first, false, false);
		break;
	}
	default:
		break;
	}
}

/// <summary>Update the bloom configuration.</summary>
void VulkanPostProcessing::updateBloomConfiguration()
{
	updateDemoConfigs();

	updateBlurDescription();
	_deviceResources->uiRenderer.getDefaultDescription()->setText(_currentBlurString);
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();

	for (uint32_t i = 0; i < _numSwapImages; ++i)
	{
		_mustRecordPrimaryCommandBuffer[i] = true;
		_mustUpdatePerSwapchainDemoConfig[i] = true;
	}
}

/// <summary>Handles user input and updates live variables accordingly.</summary>
void VulkanPostProcessing::eventMappedInput(pvr::SimplifiedInput e)
{
	switch (e)
	{
	case pvr::SimplifiedInput::Up:
	{
		_currentDemoConfiguration = (_currentDemoConfiguration + 1) % DemoConfigurations::NumDemoConfigurations;
		updateBloomConfiguration();
		_isManual = true;
		break;
	}
	case pvr::SimplifiedInput::Down:
	{
		if (_currentDemoConfiguration == 0)
		{
			_currentDemoConfiguration = DemoConfigurations::NumDemoConfigurations;
		}
		_currentDemoConfiguration = (_currentDemoConfiguration - 1) % DemoConfigurations::NumDemoConfigurations;
		updateBloomConfiguration();
		_isManual = true;
		break;
	}
	case pvr::SimplifiedInput::Left:
	{
		uint32_t currentBlurMode = static_cast<uint32_t>(_blurMode);
		currentBlurMode -= 1;
		currentBlurMode = (currentBlurMode + static_cast<uint32_t>(BloomMode::NumBloomModes)) % static_cast<uint32_t>(BloomMode::NumBloomModes);
		_blurMode = static_cast<BloomMode>(currentBlurMode);
		updateBloomConfiguration();
		_isManual = true;
		break;
	}
	case pvr::SimplifiedInput::Right:
	{
		uint32_t currentBlurMode = static_cast<uint32_t>(_blurMode);
		currentBlurMode += 1;
		currentBlurMode = (currentBlurMode + static_cast<uint32_t>(BloomMode::NumBloomModes)) % static_cast<uint32_t>(BloomMode::NumBloomModes);
		_blurMode = static_cast<BloomMode>(currentBlurMode);
		updateBloomConfiguration();
		_isManual = true;
		break;
	}
	case pvr::SimplifiedInput::ActionClose:
	{
		this->exitShell();
		break;
	}
	case pvr::SimplifiedInput::Action1:
	{
		_renderOnlyBloom = !_renderOnlyBloom;
		for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
		{
			_mustRecordPrimaryCommandBuffer[i] = true;
		}
		break;
	}
	case pvr::SimplifiedInput::Action2:
	{
		_animateObject = !_animateObject;
		for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
		{
			_mustRecordPrimaryCommandBuffer[i] = true;
		}
		break;
	}
	default:
	{
		break;
	}
	}
}

/// <summary>Records the UI Renderer commands.</summary>
/// <param name="swapchainIndex">The swapchain index for which the recorded command buffer will be used.</param>
/// <param name="commandBuffers">The secondary command buffer to which UI Renderer commands should be recorded.</param>
void VulkanPostProcessing::recordUIRendererCommands(uint32_t swapchainIndex, pvr::Multi<pvrvk::SecondaryCommandBuffer>& commandBuffers)
{
	commandBuffers.add(_deviceResources->commandPool->allocateSecondaryCommandBuffer());

	commandBuffers[swapchainIndex]->begin(_deviceResources->onScreenFramebuffers[swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

	_deviceResources->uiRenderer.beginRendering(commandBuffers[swapchainIndex]);

	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.endRendering();
	commandBuffers[swapchainIndex]->end();
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo()
{
	return std::unique_ptr<pvr::Shell>(new VulkanPostProcessing());
}
