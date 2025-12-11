/*!
\brief This example demonstrates how approximate images with neural networks, in this case the environment of the sample. This sample
\ is a clone of the Image Based Lighting sample. For any information regarding Physically Based Rendering, please review that sample.
\file  VulkanNeuralNetworkEnvironment.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include <regex>
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRUtils/Vulkan/PBRUtilsVk.h"
#include "PVRCore/cameras/TPSCamera.h"
#include "PVRCore/textureio/TextureWriterPVR.h"
#include "PVRAssets/fileio/GltfReader.h"

// Content file names
// Shaders
const char VertShaderFileName[]{ "VertShader.vsh.spv" };
const char PBRFragShaderFileName[]{ "PBRFragShader.fsh.spv" };
const char SkyboxVertShaderFileName[]{ "SkyboxVertShader.vsh.spv" };
const char SkyboxFragShaderFileName[]{ "SkyboxFragShader.fsh.spv" };
const char ComputeShaderScreenSpaceBoxSrcFile[]{ "ScreenSpaceBox.csh" };
const char ComputeShaderOrganizePatchWorkload[]{ "OrganizePatchWorkload.csh" };
const char ComputeShaderNeuralNetworkEnvironment[]{ "NeuralNetworkEnvironment.csh" };
const char ComputeShaderNeuralNetworkEnvironmentCooperativeMatrix[]{ "NeuralNetworkEnvironmentCooperativeMatrix.csh.spv" };

// Models
const char HelmetModelFileName[]{ "damagedHelmet.gltf" };

// Textures
const std::string SkyboxTexFile{ "quarry_r9g9b9e5" };
const std::string equirectangularEnvironmentTexture{ "quarry_equirectangular_r9g9b9e5" };
const std::string neuralNetworkEnvironmentBinaryBlob{ "quarry_equirectangular_r9g9b9e5.blob" };
const char BrdfLUTTexFile[]{ "brdfLUT.pvr" };

/// <summary>Maximum number of layers supported for inference.</summary>
const uint32_t maxNeuralNetworkLayers{ 5 };

/// <summary>Size of the header of the neural network file.</summary>
const uint32_t neuralNetworkHeaderFileByteSize{ 22 * sizeof(uint32_t) };

/// <summary>Camera rotation speed.</summary>
const float rotationSpeed{ 0.01f };

/// <summary>Time in seconds each rendering technique to draw the environment is active.</summary>
const float environmentDrawTechniqueShowTime{ 10.0f };

/// <summary>Camera field of view.</summary>
const float fov{ 65.f };

/// <summary>Light direction.</summary>
const glm::vec3 lightDir{ glm::normalize(glm::vec3(-0.5f, -0.5f, -0.5f)) };

const int maxNumberScreenSpaceBoxSlots{ 20 };
const int numberElementsPerScreenSpaceBoxSlot{ 5 };

/// <summary>Struct to group a uniform buffer object and its associated structured view.</summary>
struct UBO
{
	pvr::utils::StructuredBufferView view;
	pvrvk::Buffer buffer;
};

/// <summary>Enum to indicate what precission to use for the neural network biases and weights.</summary>
enum class InferenceFloatPrecission
{
	IFP_32_BIT_FLOAT = 0,
	IFP_16_BIT_FLOAT = 1,
	IFP_SIZE = 3,
};

/// <summary>Enum to indicate what method is used to draw the environment map.</summary>
enum class CurrentEnvironmentTechnique
{
	CET_NEURAL_NETWORK_COMPUTE_COOPERATIVE = 0, /// <summary>Use cooperative matrix in compute shaders to draw the environment map.</summary>
	CET_NEURAL_NETWORK_COMPUTE = 1, /// <summary>Use compute shaders to draw the environment map.</summary>
	CET_RASTERIZATION = 2, /// <summary>Use rasterizaiton to draw the environment map.</summary>
	CET_SIZE = 3,
};

/// <summary>Used for the 32-bit float to 16-bit float conversion.</summary>
/// <param name="x">value to return as unsigned int</param>
/// <returns>Value converted to unsigned int.</returns>
uint32_t as_uint(const float x) { return *(uint32_t*)&x; }

/// <summary>Convert from 32-bit float to 16-bit float.</summary>
/// https://stackoverflow.com/questions/1659440/32-bit-to-16-bit-floating-point-conversion
/// <param name="x">value to convert</param>
/// <returns>Value converted, encoded as a 16-bit uint.</returns>
uint16_t float32ToFloat16(const float x)
{
	// IEEE-754 16-bit format:
	// 1 bit sign
	// 5 bits exponent
	// 10 bits mantissa
	const uint32_t b = as_uint(x) + 0x00001000;
	const uint32_t e = (b & 0x7F800000) >> 23;
	const uint32_t m = b & 0x007FFFFF;
	return static_cast<uint16_t>((b & 0x80000000) >> 16 | (e > 112) * ((((e - 112) << 10) & 0x7C00) | m >> 13) |
		((e < 113) & (e > 101)) * ((((0x007FF000 + m) >> (125 - e)) + 1) >> 1) | (e > 143) * 0x7FFF);
}

/// <summary>Convert a byte vector formed of 32-bit floats to 16-bit floats.</summary>
/// <param name="vectorByteData">Vector to convert</param>
static void convertByteArrayFloat32ToFloat16(std::vector<uint8_t>& vectorByteData)
{
	float* pData32Bit = reinterpret_cast<float*>(vectorByteData.data());

	std::vector<uint8_t> vectorResult(vectorByteData.size() / 2);

	for (int i = 0; i < vectorByteData.size() / 4; ++i)
	{
		uint16_t result = float32ToFloat16(pData32Bit[i]);
		uint8_t partA = static_cast<uint8_t>(result & 0x00FF);
		uint8_t partB = static_cast<uint8_t>((result & 0xFF00) >> 8);
		vectorResult[2 * i + 0] = partA;
		vectorResult[2 * i + 1] = partB;
	}

	vectorByteData = vectorResult;
}

class SkyBoxPass
{
public:
	void init(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::DescriptorPool& descPool, pvrvk::CommandPool& commandPool, pvrvk::Queue& queue,
		const pvrvk::RenderPass& renderpass, const pvrvk::PipelineCache& pipelineCache, const pvrvk::Extent2D& viewportDim,
		pvr::utils::vma::Allocator& allocator)
	{
		// /// CREATE THE PIPELINE OBJECT FOR THE SKYBOX /// //
		// create skybox descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
		descSetLayoutInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetLayoutInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descSetLayoutInfo.setBinding(2, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

		pvrvk::DescriptorSetLayout descSetLayout = device->createDescriptorSetLayout(descSetLayoutInfo);

		pvrvk::PipelineLayoutCreateInfo pipelineLayoutInfo;
		pipelineLayoutInfo.setDescSetLayout(0, descSetLayout);

		pvrvk::PipelineLayout pipeLayout = device->createPipelineLayout(pipelineLayoutInfo);
		createPipeline(assetProvider, device, renderpass, viewportDim, pipeLayout, pipelineCache);

		// /// CREATE THE SKYBOX DESCRIPTOR SET /// //
		descSet = descPool->allocateDescriptorSet(descSetLayout);
		descSet->setObjectName("SkyBoxDescriptorSet");

		setSkyboxImage(assetProvider, queue, commandPool, descPool, allocator);
	}

	void setSkyboxImage(pvr::IAssetProvider& assetProvider, pvrvk::Queue queue, pvrvk::CommandPool commandPool, pvrvk::DescriptorPool descPool,
		pvr::utils::vma::Allocator& allocator)
	{
		// /// LOAD THE SKYBOX TEXTURE /// //
		pvrvk::CommandBuffer cmdBuffer = commandPool->allocateCommandBuffer();
		pvrvk::Device device = commandPool->getDevice();

		cmdBuffer->begin();

		skyBoxMap = device->createImageView(pvrvk::ImageViewCreateInfo(pvr::utils::loadAndUploadImage(device, SkyboxTexFile + ".pvr", true, cmdBuffer, assetProvider,
			pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, allocator, allocator)));

		equirectangularMap = device->createImageView(pvrvk::ImageViewCreateInfo(pvr::utils::loadAndUploadImage(device, equirectangularEnvironmentTexture + ".pvr",
			true, cmdBuffer, assetProvider, pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, allocator, allocator)));

		skyBoxMapWidth = static_cast<float>(equirectangularMap->getImage()->getExtent().getWidth());
		skyBoxMapHeight = static_cast<float>(equirectangularMap->getImage()->getExtent().getHeight());

		cmdBuffer->end();

		pvrvk::SubmitInfo submitInfo;
		submitInfo.commandBuffers = &cmdBuffer;
		submitInfo.numCommandBuffers = 1;
		queue->submit(&submitInfo, 1);
		queue->waitIdle();

		cmdBuffer->begin();

		// Load (or generate) the other image based lighting files (diffuse/irradiance, specular/pre-filtered)

		std::string diffuseMapFilename = SkyboxTexFile + "_Irradiance.pvr";
		std::string prefilteredMapFilename = SkyboxTexFile + "_Prefiltered.pvr";

		irradianceMap = pvr::utils::loadAndUploadImageAndView(device, diffuseMapFilename.c_str(), true, cmdBuffer, assetProvider, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, allocator, allocator);
		prefilteredMap = pvr::utils::loadAndUploadImageAndView(device, prefilteredMapFilename.c_str(), true, cmdBuffer, assetProvider, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, allocator, allocator);

		numPrefilteredMipLevels = prefilteredMap->getImage()->getNumMipLevels();

		cmdBuffer->end();
		queue->submit(&submitInfo, 1);
		queue->waitIdle();
	}

	void updateDescriptorSetSkyBoxPass(const pvrvk::Device& device, const pvrvk::Sampler& sampler, const pvrvk::Buffer& buffer, const pvr::utils::StructuredBufferView& bufferView)
	{
		pvrvk::WriteDescriptorSet writeDescSets[3];
		writeDescSets[0]
			.set(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descSet, 0)
			.setImageInfo(0, pvrvk::DescriptorImageInfo(skyBoxMap, sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

		writeDescSets[1]
			.set(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descSet, 1)
			.setImageInfo(0, pvrvk::DescriptorImageInfo(equirectangularMap, sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

		writeDescSets[2].set(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, descSet, 2).setBufferInfo(0, pvrvk::DescriptorBufferInfo(buffer, 0, bufferView.getDynamicSliceSize()));

		device->updateDescriptorSets(writeDescSets, ARRAY_SIZE(writeDescSets), nullptr, 0);
	}

	uint32_t getNumPrefilteredMipLevels() const { return numPrefilteredMipLevels; }

	pvrvk::ImageView getDiffuseIrradianceMap() { return irradianceMap; }

	pvrvk::ImageView getPrefilteredMap() { return prefilteredMap; }

	pvrvk::ImageView getPrefilteredMipMap() { return skyBoxMap; }

	/// <summary>Record commands.</summary>
	/// <param name="cmdBuffer">recording commandbuffer</param>
	/// <param name="swapchainIndex">swapchain index.</param>
	void recordCommands(pvrvk::CommandBuffer& cmdBuffer, uint32_t swapchainIndex, const pvr::utils::StructuredBufferView& structuredBufferView)
	{
		cmdBuffer->bindPipeline(pipeline);
		uint32_t offset = structuredBufferView.getDynamicSliceOffset(swapchainIndex);
		cmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipeline->getPipelineLayout(), 0, descSet, &offset, 1);

		cmdBuffer->draw(0, 6, 0);
	}

	float getSkyBoxMapWidth() const { return skyBoxMapWidth; }
	float getSkyBoxMapHeight() const { return skyBoxMapHeight; }

private:
	void createPipeline(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, const pvrvk::RenderPass& renderpass, const pvrvk::Extent2D& viewportDim,
		const pvrvk::PipelineLayout& pipelineLayout, const pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::GraphicsPipelineCreateInfo pipeInfo;

		// on screen renderpass
		pipeInfo.renderPass = renderpass;

		pipeInfo.vertexShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(SkyboxVertShaderFileName)->readToEnd<uint32_t>())));
		pipeInfo.fragmentShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(SkyboxFragShaderFileName)->readToEnd<uint32_t>())));

		pipeInfo.pipelineLayout = pipelineLayout;

		// depth stencil state
		pipeInfo.depthStencil.enableDepthWrite(false);
		pipeInfo.depthStencil.enableDepthTest(false);

		// rasterizer state
		pipeInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);

		// blend state
		pipeInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
		pipeInfo.colorBlend.setAttachmentState(1, pvrvk::PipelineColorBlendAttachmentState());

		// input assembler
		pipeInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);

		// vertex attributes and bindings
		pipeInfo.vertexInput.clear();

		pipeInfo.viewport.setViewportAndScissor(0, pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(viewportDim.getWidth()), static_cast<float>(viewportDim.getHeight())),
			pvrvk::Rect2D(0, 0, viewportDim.getWidth(), viewportDim.getHeight()));

		pipeline = device->createGraphicsPipeline(pipeInfo, pipelineCache);
		pipeline->setObjectName("SkyBoxGraphicsPipeline");
	}

	pvrvk::GraphicsPipeline pipeline;
	pvrvk::ImageView skyBoxMap;
	float skyBoxMapWidth{ 0.0f };
	float skyBoxMapHeight{ 0.0f };
	pvrvk::ImageView irradianceMap, prefilteredMap;
	pvrvk::ImageView equirectangularMap;
	pvrvk::DescriptorSet descSet;
	uint32_t numPrefilteredMipLevels{ 0 };
};

class HelmetPass
{
public:
	void init(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, const pvrvk::Framebuffer& framebuffer, const pvrvk::PipelineLayout& pipelineLayout,
		const pvrvk::PipelineCache& pipelineCache, pvr::utils::vma::Allocator& allocator, pvrvk::CommandBuffer& uploadCmdBuffer, bool requireSubmission, bool astcSupported)
	{
		model = pvr::assets::loadModel(assetProvider, HelmetModelFileName);

		// create the vbo and ibo for the meshes.
		uint32_t numMeshes = model->getNumMeshes();
		vbos.resize(numMeshes);
		ibos.resize(numMeshes);

		for (uint32_t m = 0; m < numMeshes; ++m)
		{
			pvr::utils::createSingleBuffersFromMesh(device, model->getMesh(m), vbos[m], ibos[m], uploadCmdBuffer, requireSubmission, allocator);
		}

		isASTCSupported = astcSupported;

		// Load the texture
		loadTextures(assetProvider, device, uploadCmdBuffer, allocator);

		createPipeline(assetProvider, device, framebuffer, pipelineLayout, pipelineCache);
	}

	const pvrvk::GraphicsPipeline& getPipeline() { return pipeline; }

	pvr::assets::ModelHandle& getModel() { return model; }

	const pvrvk::ImageView& getAlbedoMap() { return images[0]; }

	const pvrvk::ImageView& getOcclusionMetallicRoughnessMap() { return images[1]; }

	const pvrvk::ImageView& getNormalMap() { return images[2]; }

	const pvrvk::ImageView& getEmissiveMap() { return images[3]; }

	void recordCommands(pvrvk::CommandBuffer& cmd)
	{
		cmd->bindPipeline(pipeline);
		const uint32_t numMeshes = model->getNumMeshes();

		for (uint32_t j = 0; j < numMeshes; ++j)
		{
			const pvr::assets::Mesh& mesh = model->getMesh(j);
			// find the texture descriptor set which matches the current material

			// bind the vbo and ibos for the current mesh node
			cmd->bindVertexBuffer(vbos[j], 0, 0);

			cmd->bindIndexBuffer(ibos[j], 0, mesh.getFaces().getDataType() == pvr::IndexType::IndexType16Bit ? pvrvk::IndexType::e_UINT16 : pvrvk::IndexType::e_UINT32);

			// draws
			cmd->drawIndexed(0, mesh.getNumFaces() * 3);
		}
	}

private:
	void createPipeline(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, const pvrvk::Framebuffer& framebuffer, const pvrvk::PipelineLayout& pipelineLayout,
		const pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::GraphicsPipelineCreateInfo pipeDesc;
		pipeDesc.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
		pipeDesc.colorBlend.setAttachmentState(1, pvrvk::PipelineColorBlendAttachmentState());
		pvr::utils::VertexBindings bindingName[] = { { "POSITION", 0 }, { "NORMAL", 1 }, { "UV0", 2 }, { "TANGENT", 3 } };

		pvr::utils::populateViewportStateCreateInfo(framebuffer, pipeDesc.viewport);
		pvr::utils::populateInputAssemblyFromMesh(getModel()->getMesh(0), bindingName, ARRAY_SIZE(bindingName), pipeDesc.vertexInput, pipeDesc.inputAssembler);

		pipeDesc.vertexShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(VertShaderFileName)->readToEnd<uint32_t>())));
		pipeDesc.fragmentShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(PBRFragShaderFileName)->readToEnd<uint32_t>())));

		static VkBool32 shaderConstantHasTextures = 1;
		pipeDesc.vertexShader.setShaderConstant(0, pvrvk::ShaderConstantInfo(0, &shaderConstantHasTextures, sizeof(VkBool32)));
		pipeDesc.fragmentShader.setShaderConstant(0, pvrvk::ShaderConstantInfo(0, &shaderConstantHasTextures, sizeof(VkBool32)));

		pipeDesc.renderPass = framebuffer->getRenderPass();
		pipeDesc.depthStencil.enableDepthTest(true);
		pipeDesc.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);
		pipeDesc.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_LESS);
		pipeDesc.depthStencil.enableDepthWrite(true);
		pipeDesc.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT).setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);
		pipeDesc.subpass = 0;

		pipeDesc.pipelineLayout = pipelineLayout;

		pipeDesc.flags = pvrvk::PipelineCreateFlags::e_ALLOW_DERIVATIVES_BIT;

		pipeline = device->createGraphicsPipeline(pipeDesc, pipelineCache);
		pipeline->setObjectName("HelmetPassGraphicsPipeline");
	}

	void loadTextures(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::CommandBuffer& uploadCmdBuffer, pvr::utils::vma::Allocator& allocator)
	{
		for (uint32_t i = 0; i < model->getNumTextures(); ++i)
		{
			std::string textureName = model->getTexture(i).getName();
			pvr::assets::helper::getTextureNameWithExtension(textureName, isASTCSupported);
			std::unique_ptr<pvr::Stream> stream = assetProvider.getAssetStream(textureName.c_str());
			pvr::Texture tex = pvr::textureLoad(*stream, pvr::TextureFileFormat::PVR);
			images.push_back(pvr::utils::uploadImageAndView(device, tex, true, uploadCmdBuffer, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
				pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, allocator, allocator, pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT));
		}
	}

	std::vector<pvrvk::ImageView> images;
	std::vector<pvrvk::Buffer> vbos;
	std::vector<pvrvk::Buffer> ibos;
	pvr::assets::ModelHandle model;
	pvrvk::GraphicsPipeline pipeline;
	bool isASTCSupported{ false };
};

struct NeuralNetworkConfiguration
{
	/// <summary>Width of the original image approximated.</summary>
	uint32_t imageWidth{ 0 };

	/// <summary>Height of the original image approximated.</summary>
	uint32_t imageHeight{ 0 };

	/// <summary>Number of column patches for the neural network blob loaded from file containing the patches which approximate the environment texture.</summary>
	uint32_t numberColumnPatches{ 0 };

	/// <summary>Number of row patches for the neural network blob loaded from file containing the patches which approximate the environment texture.</summary>
	uint32_t numberRowPatches{ 0 };

	/// <summary>Size in bytes of the neural network weights buffer each neual network patch has, required in the compute shader to evaluate the neural network.</summary>
	uint32_t nnWeightsBufferSize{ 0 };

	/// <summary>Number elements of the weights buffer.</summary>
	uint32_t nnWeightsNumberElement{ 0 };

	/// <summary>Size in bytes of the neural network biases buffer each neual network patch has, required in the compute shader to evaluate the neural network.</summary>
	uint32_t nnBiasesBufferSize{ 0 };

	/// <summary>Number elements of the biasesbuffer.</summary>
	uint32_t nnBiasesNumberElement{ 0 };

	/// <summary>Number of layers in the neural network.</summary>
	uint32_t layerCount{ 0 };

	/// <summary>Maximum number of neurons per layer in the current configuration.</summary>
	uint32_t maxNeuronsPerLayer{ 0 };

	/// <summary>Number of neurons per layer.</summary>
	std::vector<uint32_t> vectorNeuronsPerLayer;

	/// <summary>Offsets for the connections for all neurons in each layer.</summary>
	std::vector<uint32_t> vectorConnectionOffsetsPerLayer;

	/// <summary>Offsets for the index of each neuron.</summary>
	std::vector<uint32_t> vectorNeuronOffsets;
};

/// <summary>Implementing the pvr::Shell functions.</summary>
class VulkanNeuralNetworkEnvironment : public pvr::Shell
{
	/// <summary>Usage of various descritpor sets used.</summary>
	enum DescSetIndex
	{
		PerFrame,
		Model,
		Material,
	};

	struct DeviceResources
	{
		/// <summary>Encapsulation of a Vulkan instance.</summary>
		pvrvk::Instance instance;

		/// <summary>Callbacks and messengers for debug messages.</summary>
		pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;

		/// <summary>Encapsulation of a Vulkan logical device.</summary>
		pvrvk::Device device;

		/// <summary>Encapsulation of a Vulkan swapchain.</summary>
		pvrvk::Swapchain swapchain;

		/// <summary>vma memory allocator used to build some buffers.</summary>
		pvr::utils::vma::Allocator vmaAllocator;

		/// <summary>Graphics queue where to submit commands.</summary>
		pvrvk::Queue graphicsQueue;

		/// <summary>Compute queue where to submit commands.</summary>
		pvrvk::Queue computeQueue;

		/// <summary>Command pool to allocate command buffers.</summary>
		pvrvk::CommandPool commandPool;

		/// <summary>Descriptor pool where to get descriptor sets allocated from.</summary>
		pvrvk::DescriptorPool descriptorPool;

		/// <summary>Semaphores signaled when the next swap chain image has been acquired.</summary>
		std::vector<pvrvk::Semaphore> vectorImageAcquiredSemaphores;

		/// <summary>Semaphores signaled when the graphics command buffers of the scene have completed on the GPU.</summary>
		std::vector<pvrvk::Semaphore> vectorPresentationSemaphores;

		/// <summary>Semaphores signaled when the compute command buffers to draw the environment have completed on the GPU.</summary>
		std::vector<pvrvk::Semaphore> vectorComputeSemaphores;

		/// <summary>Semaphores signaled when the UI is drawn (only when compute is used).</summary>
		std::vector<pvrvk::Semaphore> vectorUISemaphores;

		/// <summary>Fences to wait in the host for the graphics command buffers to complete execution.</summary>
		std::vector<pvrvk::Fence> vectorGraphicsFence;

		/// <summary>Fences to wait in the host for the compute command buffers to complete execution.</summary>
		std::vector<pvrvk::Fence> vectorComputeFence;

		/// <summary>Command buffer to draw the environment and scene mesh.</summary>
		std::vector<pvrvk::CommandBuffer> vectorGraphicsCommandBuffers;

		/// <summary>Command buffer to draw just the scene mesh.</summary>
		std::vector<pvrvk::CommandBuffer> vectorGraphicsNoEnvironmentCommandBuffers;

		/// <summary>Command buffer to draw the UI after the compute pass (used for the cases where the environment is done with a compute pass).</summary>
		std::vector<pvrvk::CommandBuffer> vectorGraphicsUICommandBuffers;

		/// <summary>Command buffer to draw the environment with compute shaders where each thread evaluates at least one pixel.</summary>
		std::vector<pvrvk::CommandBuffer> vectorComputeCommandBuffers;

		/// <summary> Command buffer to draw the environment with compute shaders where each cooperative matrix are used to evaluate pixels.</summary>
		std::vector<pvrvk::CommandBuffer> vectorComputeCooperativeMatrixCommandBuffers;

		/// <summary>Secondary command buffers for the compute pass where the screen is divided into screen-space boxes containing the different environment patches.</summary>
		std::vector<pvrvk::SecondaryCommandBuffer> vectorScreenSpaceBoxCommandBuffer;

		/// <summary>Secondary command buffers for the compute pass where the screen-space boxes containing the different environment patches are subdivided.</summary>
		/// into smaller parts so workgroups can generate the pixels of each part
		std::vector<pvrvk::SecondaryCommandBuffer> vectorOrganizePatchWorkloadCommandBuffer;

		/// <summary>Secondary command buffer for the compute pass inferring the environment pixels where each compute thread evaluates at least one pixel.</summary>
		std::vector<pvrvk::SecondaryCommandBuffer> vectorNeuralNetworkEnvironmentCommandBuffer;

		/// <summary>Secondary command buffer for the compute pass inferring the environment pixels where cooperative matrices are used to evaluate pixels.</summary>
		std::vector<pvrvk::SecondaryCommandBuffer> vectorNeuralNetworkEnvironmentCooperativeMatrixCommandBuffer;

		/// <summary>Pipeline cache.</summary>
		pvrvk::PipelineCache pipelineCache;

		/// <summary>Descriptor sets with uniform buffers used in the sample.</summary>
		pvrvk::DescriptorSet descSets[3];

		/// <summary>Uniform buffer objects with structured memory views for information changing per frame.</summary>
		UBO uboPerFrame;
		
		/// <summary>Uniform buffer objects with structured memory views for light information.</summary>
		UBO uboLights;

		/// <summary>Uniform buffer objects with structured memory views for material information.</summary>
		UBO uboMaterial;

		/// <summary>Uniform buffer objects with structured memory views for scene information.</summary>
		UBO uboWorld;

		/// <summary>Bilinar sampler.</summary>
		pvrvk::Sampler samplerBilinear;

		/// <summary>Trilinear sampler.</summary>
		pvrvk::Sampler samplerTrilinear;

		/// <summary>Trilinear sampler with LOD clamped.</summary>
		pvrvk::Sampler samplerTrilinearLodClamped;

		/// <summary>Descriptor set layout for the uniform buffers used.</summary>
		pvrvk::DescriptorSetLayout descSetLayouts[3];

		/// <summary>Pipeline layout for the scene geometry pass.</summary>
		pvrvk::PipelineLayout pipelineLayout;

		/// <summary>Look up table texture for the BRDF.</summary>
		pvrvk::ImageView brdfLUT;

		/// <summary>UI Renderer object.</summary>
		pvr::ui::UIRenderer uiRenderer;

		/// <summary>UI Renderer object for the compute passes.</summary>
		pvr::ui::UIRenderer uiRendererCompute;

		/// <summary>Render pass drawing the skybox.</summary>
		SkyBoxPass skyBoxPass;

		/// <summary>Render pass drawing the helmet mesh.</summary>
		HelmetPass helmetPass;

		/// <summary>Framebuffer used to draw the scene ofscreen.</summary>
		std::vector<pvrvk::Framebuffer> offscreenFramebuffer;

		/// <summary>Framebuffer used to draw the UI on top of the compute pass results.</summary>
		std::vector<pvrvk::Framebuffer> uiFramebuffer;

		/// <summary>Image used as a mark to know what pixels have scene geometry drawn.</summary>
		std::vector<pvrvk::Image> offscreenColorAttachmentMaskImage;

		/// <summary>Image views for the offscreenColorAttachmentMaskImage.</summary>
		std::vector<pvrvk::ImageView> offscreenColorAttachmentMaskImageView;

		/// <summary>Depth attachments for the scene pass.</summary>
		std::vector<pvrvk::Image> offscreenDepthAttachmentImage;

		/// <summary>Image views of offscreenDepthAttachmentImage.</summary>
		std::vector<pvrvk::ImageView> offscreenDepthAttachmentImageView;

		/// <summary>Render pass used to draw the scene offscreen.</summary>
		pvrvk::RenderPass offScreenGeometryRenderPass;

		/// <summary>Render pass used to draw the UI on top of compute results.</summary>
		pvrvk::RenderPass uiRenderPass;

		/// <summary>Depth images.</summary>
		std::vector<pvrvk::ImageView> depthImages;

		/// <summary>Uniform Buffer used in compute passes.</summary>
		pvrvk::Buffer computeSettingsBuffer;

		/// <summary>Structured view of computeSettingsBuffer.</summary>
		pvr::utils::StructuredBufferView computeSettingsBufferView;

		/// <summary>Buffer with the screen space boxes of each enviroment patch visible from screen.</summary>
		std::vector<pvrvk::Buffer> vectorComputeScreenSpaceBoxBuffer;

		/// <summary>Temporal debug buffer
		std::vector<pvrvk::Buffer> vectorComputeScreenSpaceBoxDebugBuffer;

		/// <summary>Buffer with the sreen-space squares and patch index used by each workgroup to infer through the neural network the pixel value
		/// of each pixel in this square for the neural netowrk patch approximating the environment determined by the patch index.</summary>
		std::vector<pvrvk::Buffer> vectorComputeOrganizePatchWorkloadBuffer;

		/// <summary>For debug purposes.</summary>
		std::vector<pvrvk::Buffer> vectorNNEnvironmentDebugBuffer;

		/// <summary>Temporal debug buffer.</summary>
		std::vector<pvrvk::Buffer> vectorComputeOrganizePatchWorkloadDebugBuffer;

		/// <summary>Buffer used as atomic counter for the dispatch where creen-space squares with each environment's patch information are split into balanced
		/// workloads that can be tackled by a workgroup to load a neural network and approximate the pixels of a specific patch within the screen-space square,
		/// putting the information in vectorComputeOrganizePatchWorkloadBuffer.</summary>
		std::vector<pvrvk::Buffer> vectorComputeAtomicCounterBuffer;

		/// <summary>Descriptor set layout for the compute shader computing what neural network patches are visible from screen.</summary>
		pvrvk::DescriptorSetLayout computeScreenSpaceBoxDescriptorSetLayout;

		/// <summary>Descriptor set layout for the compute shader reading the vectorComputeScreenSpaceBoxBuffer buffer and organizing 
		/// the final screen-space boxes the workgroups in another dispatch will work with to load a neural network and approximate the pixels.</summary>
		pvrvk::DescriptorSetLayout computeOrganizePatchWorkloadDescriptorSetLayout;

		/// <summary>Descriptor set layout for the compute shader inferrig pixels from a specific patch.</summary>
		pvrvk::DescriptorSetLayout computeNeuralNetworkEnvironmentDescriptorSetLayout;

		/// <summary>Vector with the descriptor sets for the compute pass computing what screen space boxes contain environment texture neural network patches.</summary>
		std::vector<pvrvk::DescriptorSet> vectorComputeScreenSpaceBoxDescriptorSet;

		/// <summary>Vector with the descriptor sets for the compute shader reading the vectorComputeScreenSpaceBoxBuffer
		/// buffer and organizing the final screen-space boxes the workgroups in another dispatch will work with to load a neural network and approximate the pixels.</summary>
		std::vector<pvrvk::DescriptorSet> vectorComputeOrganizePatchWorkloadDescriptorSet;

		/// <summary>Vector with the descriptor sets for the compute shader generating pixels for the environment after loading a specific patch of the neural 
		/// network that approximates it.</summary>
		std::vector<pvrvk::DescriptorSet> vectorNeuralNetworkEnvironmentDescriptorSet;

		/// <summary>Pipeline layout for the screen space box compute pass.</summary>
		pvrvk::PipelineLayout computeScreenSpaceBoxPipelinelayout;

		/// <summary>Pipeline layout for the compute pass establishing what regions in the screen will be analysed by workgroups to infe pixels for a specific patch.</summary>
		pvrvk::PipelineLayout computeOrganizePatchWorkloadPipelinelayout;

		/// <summary>Pipeline layout for the compute pass inferring pixels from a neural network covering a part (patch) of the environment texture.</summary>
		pvrvk::PipelineLayout computeNeuralNetworkEnvironmentPipelinelayout;

		/// <summary>Compute pipeline for the screen space box compute pass.</summary>
		pvrvk::ComputePipeline computeScreenSpaceBoxComputePipeline;

		/// <summary>Compute pipeline for the compute pass establishing what regions in the screen will be analysed by workgroups to infe pixels for a specific patch.</summary>
		pvrvk::ComputePipeline computeOrganizePatchWorkloadComputePipeline;

		/// <summary>Compute pipeline for the compute pass where the environment pixels are inferred using classic compute approach.</summary>
		pvrvk::ComputePipeline computeNeuralNetworkEnvironmentComputePipeline;

		/// <summary>Compute pipeline for the compute pass where the environment pixels are inferred using cooperative matrix.</summary>
		pvrvk::ComputePipeline computeNeuralNetworkEnvironmentCooperativeMatrixComputePipeline;

		/// <summary>Buffer used to store the buffer with the information for all neural network patches used to approximate the environment texture.</summary>
		pvrvk::Buffer neuralNetworkEnvironmentBuffer;

		~DeviceResources()
		{
			if (device) { device->waitIdle(); }
			computeQueue->waitIdle();
			graphicsQueue->waitIdle();
		}
	};

	std::unique_ptr<DeviceResources> _deviceResources;

	/// <summary>Camera projection matrix.</summary>
	glm::mat4 _projMtx;

	/// <summary>Model view matrix.</summary>
	glm::mat4 _viewMtx;

	/// <summary>Iterates from [0, swapchain length - 1].</summary>
	uint32_t _frameId{ 0 };

	/// <summary>Value of _frameId used in the last frame.</summary>
	uint32_t _lastFrameID{ 0 };

	/// <summary>Camera object used in the scene.</summary>
	pvr::TPSOrbitCamera _camera;

	/// <summary>Flag to pause the camera rotation.</summary>
	bool _pause{ false };

	/// <summary>Exposure value for tonemapping effect.</summary>
	float exposure{ 1.0f };

	/// <summary>Flag to knwo whether ASTC is supported.</summary>
	bool _isASTCSupported{ false };

	/// <summary>How many images the swap chain has.</summary>
	uint32_t _swapchainLength{ 0 };

	/// <summary>Side size of the patches of the neural network used to approximate the environment texture.</summary>
	uint32_t _patchSide{ 0 };

	/// <summary>Width of the texture used to draw the scene offscreen.</summary>
	uint32_t _textureWidth { 0 };

	/// <summary>Height of the texture used to draw the scene offscreen.</summary>
	uint32_t _textureHeight{ 0 };

	/// <summary>Screen width.</summary>
	uint32_t _screenWidth{ 0 };

	/// <summary>Screen height.</summary>
	uint32_t _screenHeight{ 0 };

	/// <summary>Value used to divide _screenWidth and _screenHeight to compute the screen-space boxes containing patches from the environment texture.</summary>
	uint32_t _screenSpaceBoxScreenFactor{ 4 };

	/// <summary>Width of the screen for the screen space box pass where a smaller factor of the screen size might be analised.</summary>
	uint32_t _screenWidthScreenSpaceBox{ 0 };

	/// <summary>Height of the screen for the screen space box pass where a smaller factor of the screen size might be analised.</summary>
	uint32_t _screenHeightScreenSpaceBox{ 0 };

	/// <summary>Number of patches in the x dimension when partitioning the texture to be approximated with nn in squares of side _patchSide.</summary>
	uint32_t _textureNumberPatchXDimension{ 0 };

	/// <summary>Number of patches in the y dimension when partitioning the texture to be approximated with nn in squares of side _patchSide.</summary>
	uint32_t _textureNumberPatchYDimension{ 0 };

	/// <summary>Equal to _textureNumberPatchXDimension * _textureNumberPatchYDimension.</summary>
	uint32_t _textureNumPatches{ 0 };

	/// <summary>Size of the compute subgroup used in the GPU used by the application.</summary>
	uint32_t _subgroupSize{ 0 };

	/// <summary>Number of threads per workgroup in the compute shader where each thread computes the screen space box of the environment texture patches.</summary>
	uint32_t _screenSpaceBoxWorkgroupSize{ 0 };

	/// <summary>Number of pixels each thread will process in the compute shader where each thread computes the screen space box of the environment texture patches.</summary>
	uint32_t _screenSpaceBoxNumberPixelPerThread{ 0 };

	/// <summary>Number of workgroups to dipatch in the x dimension to cover all the pixels specified by _screenSpaceBoxScreenWidth * _screenSpaceBoxScreenHeight.</summary>
	uint32_t _screenSpaceBoxXWorkgroupNumber{ 0 };

	/// <summary>Number of workgroups to dispatch for the compute shader where each thread prepares the screen space regions where to infer environment texture pixels corresponding to a specific environment patch approximated by a neural network.</summary>
	uint32_t _organizePatchWorkloadXWorkgroupNumber{ 0 };

	/// <summary>Number slots processed by each thread in the comnpute dispatch which splits the screen-space boxes containing a environment patch onto smaller pixel regions to later be generated with neural networks.</summary>
	uint32_t _organizePatchWorkloadNumberSlotPerThread{ 1 };

	/// <summary>Once the screen space boxes with patches inside are computed, those screen space boxes are split into regions with a number of pixels similar to this
	/// value for later a workgroup to load the neural network approximating the patch and inferring the pixels in it.</summary>
	uint32_t _organizePatchWorkloadNumberPixelScreenSpaceRegion{ 0 };

	/// <summary>Struct with the information of the neural network used to replace the environment texture.</summary>
	NeuralNetworkConfiguration nnConfiguration;

	/// <summary>Struct with the cooperative matrix properties used for the compute shader variant inderring nn values this way.</summary>
	std::vector<VkCooperativeMatrixPropertiesKHR> _vectorCooperativeMatrixPropertiesKHR;

	/// <summary>Vector to know whether each graphics fence has been reset.</summary>
	std::vector<bool> _vectorGraphicsFenceHasBeenReset;

	/// <summary>Vector to know whether each compute fence has been reset.</summary>
	std::vector<bool> _vectorComputeFenceHasBeenReset;

	/// <summary> Floating point format for the biases and weights used for inference (the loaded data is in 32-bit floating precission and will be converted to 16-bit if required).</summary>
	InferenceFloatPrecission _inferenceFloatPrecission{ InferenceFloatPrecission::IFP_16_BIT_FLOAT };

	/// <summary> To know what current technique is being displayed (raster, compute neural network inference or compute neural network cooperative matrix inference).</summary>
	CurrentEnvironmentTechnique _currentEnvironmentTechnique{ CurrentEnvironmentTechnique::CET_NEURAL_NETWORK_COMPUTE_COOPERATIVE };

	/// <summary> Flag to know whether there were command buffers sent to the compute queue last frame.</summary>
	bool _lastFrameUsedComputeQueue{ true };

	/// <summary> Time counter to change through the different ways to draw the environment, defined by CurrentEnvironmentTechnique.</summary>
	float _currentModeRemainingTime{ environmentDrawTechniqueShowTime };

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void queryCooperativeMatrixInformation();
	void createDescriptorSetLayouts();
	void createUbos();
	void updateDescriptors();
	void recordCommandBuffers();
	void recordGraphicsCommandBuffers(std::vector<pvrvk::CommandBuffer>& vectorCommandBuffer, bool includeEnvironmentPass);
	void recordGraphicsUICommandBuffers(std::vector<pvrvk::CommandBuffer>& vectorCommandBuffer);
	void recordSecondaryComputeCommandBuffers();
	void recordScreenSpaceBoxCommandBuffer(uint32_t swapIndex);
	void recordOrganizePatchWorkloadCommandBuffer(uint32_t swapIndex);
	void recordNeuralNetworkEnvironmentCommandBuffer(uint32_t swapIndex);
	void recordNeuralNetworkEnvironmentCooperativeMatrixCommandBuffer(uint32_t swapIndex);
	void recordComputeCommandBuffers();
	void createPipelineLayout();
	void buildComputeUniformBuffer();
	void updateGraphicsUniformBuffers(int swapchainIndex);
	void updateComputeUniformBuffer(uint32_t swapchainIndex);
	void updateEnvironmentTechnique();
	void buildComputeScreenSpaceBoxBuffer();
	void buildComputeOrganizePatchWorkloadBuffer();
	void buildComputeDescriptorSetLayout();
	void updateComputeScreenSpaceBoxDescriptorSet();
	void updateComputeOrganizePatchWorkloadDescriptorSet();
	void updateVectorNeuralNetworkEnvironmentDescriptorSet();
	void buildScreenSpaceBoxComputePipeline();
	void buildOrganizePatchWorkloadComputePipeline();
	void buildNeuralNetworkEnvironmentInferenceComputePipeline();
	void buildNeuralNetworkEnvironmentInferenceCooperativeMatrixComputePipeline();
	void changeSwapchainImageLayout(pvrvk::CommandBuffer commandBuffer);
	void buildComputeNeuralNetworkBuffer(pvrvk::CommandBuffer commandBuffer);
	void initializeAtomicCounterBuffer(pvrvk::CommandBuffer commandBuffer);
	void createOffScreenGeometryRenderPass();
	void createUIRenderPass();
	pvrvk::RenderPass createTechniqueRenderPass(const std::vector<pvrvk::AttachmentDescription>& vectorAttachmentDescription);
	void fillAttachmentDescription(int numColorAttachments, const std::vector<pvrvk::Format>& vectorColorFormat, bool addDepthAttachment,
		pvrvk::SampleCountFlags numSamplesPerPixel, bool keepColorAttachmentContent, std::vector<pvrvk::AttachmentDescription>& vectorAttachmentDescription);

	void createImagesAndFramebuffers();

	virtual void eventMappedInput(pvr::SimplifiedInput action)
	{
		switch (action)
		{
		case pvr::SimplifiedInput::Action1:
		case pvr::SimplifiedInput::Action2:
		case pvr::SimplifiedInput::Action3:
		{
			_pause = !_pause;
			break;
		}
		case pvr::SimplifiedInput::ActionClose: {
			this->exitShell();
			break;
		}
		default: break;
		}
	}
};

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.). If the rendering
/// context is lost, initApplication() will not be called again.</summary>
pvr::Result VulkanNeuralNetworkEnvironment::initApplication()
{
	_frameId = 0;
	setBackBufferColorspace(pvr::ColorSpace::lRGB);
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by Shell once per run, just before exiting the program.
/// quitApplication() will not be called every time the rendering context is lost, only before application exit.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanNeuralNetworkEnvironment::quitApplication() { return pvr::Result::Success; }

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context(e.g.textures, vertex buffers, etc.)</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanNeuralNetworkEnvironment::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();

	// Create a Vulkan 1.3 instance and retrieve compatible physical devices
	pvr::utils::VulkanVersion VulkanVersion(1, 3, 0);

	_deviceResources->instance = pvr::utils::createInstance(this->getApplicationName(), VulkanVersion, pvr::utils::InstanceExtensions(VulkanVersion));
	pvrvk::Surface surface =
		pvr::utils::createSurface(_deviceResources->instance, _deviceResources->instance->getPhysicalDevice(0), this->getWindow(), this->getDisplay(), this->getConnection());

	// Create a default set of debug utils messengers or debug callbacks using either VK_EXT_debug_utils or VK_EXT_debug_report respectively
	_deviceResources->debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(_deviceResources->instance);

	pvrvk::PhysicalDevice physicalDevice = _deviceResources->instance->getPhysicalDevice(0);

	// Populate queues for rendering and compute
	pvr::utils::QueuePopulateInfo queuePopulateInfos[] = {
		{ pvrvk::QueueFlags::e_GRAPHICS_BIT, surface }, // Queue 0 for Graphics
		{ pvrvk::QueueFlags::e_COMPUTE_BIT } // Queue 1 For Compute
	};

	std::vector<std::string> vectorExtensionNames{ 
		VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, 
		VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME, 
		VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME,
		VK_KHR_16BIT_STORAGE_EXTENSION_NAME, 
	};

	std::vector<int> vectorPhysicalDevicesIndex = pvr::utils::validatePhysicalDeviceExtensions(_deviceResources->instance, vectorExtensionNames);

	if (vectorPhysicalDevicesIndex.size() == 0)
	{
		throw pvrvk::ErrorFeatureNotPresent("Required extensions VK_KHR_synchronization2, VK_KHR_cooperative_matrix, VK_KHR_shader_float16_int8 or VK_KHR_16bit_storage are not suported.");
	}

	physicalDevice = _deviceResources->instance->getPhysicalDevice(vectorPhysicalDevicesIndex[0]);

	pvr::utils::DeviceExtensions deviceExtensions = pvr::utils::DeviceExtensions();
	for (const std::string& extensionName : vectorExtensionNames) { deviceExtensions.addExtension(extensionName); }

	VkPhysicalDeviceFeatures2 deviceFeatures{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_FEATURES_2) };

	VkPhysicalDeviceSynchronization2FeaturesKHR physicalDeviceSynchronization2FeaturesKHR{ static_cast<VkStructureType>(
		pvrvk::StructureType::e_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES) };
	
	VkPhysicalDeviceCooperativeMatrixFeaturesKHR coopMatFeatures = {
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR, nullptr,
		VK_TRUE,  // cooperativeMatrix
		VK_FALSE, // cooperativeMatrixRobustBufferAccess
	};

	VkPhysicalDeviceVulkanMemoryModelFeaturesKHR memoryModel = {
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES,
		nullptr,
		VK_TRUE, // vulkanMemoryModel
		VK_TRUE, // vulkanMemoryModelDeviceScope
		VK_FALSE // vulkanMemoryModelAvailabilityVisibilityChains
	};

	VkPhysicalDevice16BitStorageFeatures storage16BitFeatures = {
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES,
		nullptr,
		VK_TRUE,  // storageBuffer16BitAccess
		VK_FALSE, // uniformAndStorageBuffer16BitAccess
		VK_FALSE, // storagePushConstant16
		VK_FALSE  // storageInputOutput16
	};

	VkPhysicalDeviceShaderFloat16Int8Features shaderFloat16Int8Features = {
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES,
		nullptr,
		VK_TRUE,  // shaderFloat16
		VK_FALSE, // shaderInt8
	};

	deviceFeatures.pNext = &physicalDeviceSynchronization2FeaturesKHR;
	physicalDeviceSynchronization2FeaturesKHR.pNext = &coopMatFeatures;
	coopMatFeatures.pNext = &memoryModel;
	memoryModel.pNext = &storage16BitFeatures;
	storage16BitFeatures.pNext = &shaderFloat16Int8Features;

	// Fill in all of these device features with one call
	_deviceResources->instance->getVkBindings().vkGetPhysicalDeviceFeatures2(_deviceResources->instance->getPhysicalDevice(vectorPhysicalDevicesIndex[0])->getVkHandle(), &deviceFeatures);

	// Add these device features to the physical device, since they're all connected by a pNext chain, we only need to explicitly attach the top feature
	deviceExtensions.addExtensionFeatureVk<VkPhysicalDeviceSynchronization2FeaturesKHR>(&physicalDeviceSynchronization2FeaturesKHR);

	// Create the device and queue
	pvr::utils::QueueAccessInfo queueAccessInfos[2];
	_deviceResources->device = pvr::utils::createDeviceAndQueues(physicalDevice, queuePopulateInfos, 2, queueAccessInfos, deviceExtensions);

	// Obtain the cooperative matrix information supported.
	queryCooperativeMatrixInformation();

	// Get the graphics queue
	_deviceResources->graphicsQueue = _deviceResources->device->getQueue(queueAccessInfos[0].familyId, queueAccessInfos[0].queueId);
	_deviceResources->graphicsQueue->setObjectName("GraphicsQueue");

	if (queueAccessInfos[1].familyId != static_cast<uint32_t>(-1) && queueAccessInfos[1].queueId != static_cast<uint32_t>(-1))
	{
		Log(LogLevel::Information, "Multiple queues supported e_GRAPHICS_BIT + e_COMPUTE_BIT + WSI");

		_deviceResources->computeQueue = _deviceResources->device->getQueue(queueAccessInfos[1].familyId, queueAccessInfos[1].queueId);
		_deviceResources->computeQueue->setObjectName("ComputeQueue");
	}
	else
	{
		Log(LogLevel::Information, "Only a single queue supports e_GRAPHICS_BIT + e_COMPUTE_BIT + WSI");

		_deviceResources->computeQueue = _deviceResources->graphicsQueue;
	}

	// validate the supported swapchain image usage for source transfer option for capturing screenshots.
	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice->getSurfaceCapabilities(surface);
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT)) // Transfer operation for screenshots
	{
		swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT;
	}
	else
	{
		Log(LogLevel::Information, "Error: swapchain images do not support VK_IMAGE_USAGE_TRANSFER_SRC_BIT, needed for screenshots.");
		return pvr::Result::InitializationError;
	}

	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT)) // Color attachment for offscreen rendering
	{
		swapchainImageUsage |= pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	}
	else
	{
		Log(LogLevel::Information, "Error: swapchain images do not support VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, needed for offscreen rendering.");
		return pvr::Result::InitializationError;
	}

	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_STORAGE_BIT)) // Color attachment for offscreen rendering
	{
		swapchainImageUsage |= pvrvk::ImageUsageFlags::e_STORAGE_BIT;
	}
	else
	{
		Log(LogLevel::Information, "Error: swapchain images do not support VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, needed for offscreen rendering.");
		return pvr::Result::InitializationError;
	}

	// initialise the vma allocator
	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage));

	_deviceResources->swapchain = swapChainCreateOutput.swapchain;

	_swapchainLength = _deviceResources->swapchain->getSwapchainLength();

	_deviceResources->vectorImageAcquiredSemaphores.resize(_swapchainLength);
	_deviceResources->vectorPresentationSemaphores.resize(_swapchainLength);
	_deviceResources->vectorComputeSemaphores.resize(_swapchainLength);
	_deviceResources->vectorUISemaphores.resize(_swapchainLength);
	_deviceResources->vectorGraphicsFence.resize(_swapchainLength);
	_deviceResources->vectorComputeFence.resize(_swapchainLength);
	_vectorGraphicsFenceHasBeenReset.resize(_swapchainLength);
	_vectorComputeFenceHasBeenReset.resize(_swapchainLength);
	_deviceResources->vectorGraphicsCommandBuffers.resize(_swapchainLength);
	_deviceResources->vectorGraphicsNoEnvironmentCommandBuffers.resize(_swapchainLength);
	_deviceResources->vectorGraphicsUICommandBuffers.resize(_swapchainLength);
	_deviceResources->vectorComputeCommandBuffers.resize(_swapchainLength);
	_deviceResources->vectorComputeCooperativeMatrixCommandBuffers.resize(_swapchainLength);
	_deviceResources->vectorScreenSpaceBoxCommandBuffer.resize(_swapchainLength);
	_deviceResources->vectorOrganizePatchWorkloadCommandBuffer.resize(_swapchainLength);
	_deviceResources->vectorNeuralNetworkEnvironmentCommandBuffer.resize(_swapchainLength);
	_deviceResources->vectorNeuralNetworkEnvironmentCooperativeMatrixCommandBuffer.resize(_swapchainLength);
	_deviceResources->vectorComputeScreenSpaceBoxBuffer.resize(_swapchainLength);
	_deviceResources->vectorComputeScreenSpaceBoxDebugBuffer.resize(_swapchainLength);
	_deviceResources->vectorComputeOrganizePatchWorkloadBuffer.resize(_swapchainLength);
	_deviceResources->vectorComputeOrganizePatchWorkloadDebugBuffer.resize(_swapchainLength);
	_deviceResources->vectorComputeAtomicCounterBuffer.resize(_swapchainLength);
	_deviceResources->vectorNNEnvironmentDebugBuffer.resize(_swapchainLength);

	// Create the Command pool & Descriptor pool
	_deviceResources->commandPool =
		_deviceResources->device->createCommandPool(pvrvk::CommandPoolCreateInfo(queueAccessInfos[0].familyId, pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));
	if (!_deviceResources->commandPool) { return pvr::Result::UnknownError; }

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, static_cast<uint16_t>(10 * _swapchainLength))
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, static_cast<uint16_t>(10 * _swapchainLength))
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, static_cast<uint16_t>(10 * _swapchainLength))
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_STORAGE_IMAGE, static_cast<uint16_t>(10 * _swapchainLength))
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_STORAGE_BUFFER, static_cast<uint16_t>(10 * _swapchainLength))
																						  .setMaxDescriptorSets(static_cast<uint16_t>(20 * _swapchainLength)));

	if (!_deviceResources->descriptorPool) { return pvr::Result::UnknownError; }
	_deviceResources->descriptorPool->setObjectName("DescriptorPool");

	// Create synchronization objects and command buffers
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->vectorPresentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->vectorImageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->vectorComputeSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->vectorUISemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->vectorPresentationSemaphores[i]->setObjectName("PresentationSemaphoreSwapchain" + std::to_string(i));
		_deviceResources->vectorImageAcquiredSemaphores[i]->setObjectName("ImageAcquiredSemaphoreSwapchain" + std::to_string(i));
		_deviceResources->vectorComputeSemaphores[i]->setObjectName("ComputeSemaphoreSwapchain" + std::to_string(i));
		_deviceResources->vectorUISemaphores[i]->setObjectName("GraphicsUISemaphoreSwapchain" + std::to_string(i));

		_deviceResources->vectorGraphicsFence[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->vectorGraphicsFence[i]->setObjectName("GraphicsFenceSwapchain" + std::to_string(i));

		_deviceResources->vectorComputeFence[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->vectorComputeFence[i]->setObjectName("ComputeFenceSwapchain" + std::to_string(i));

		_deviceResources->vectorGraphicsCommandBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->vectorGraphicsCommandBuffers[i]->setObjectName("GraphicsCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->vectorGraphicsCommandBuffers[i]->setVKSynchronization2IsSupported(true);

		_deviceResources->vectorGraphicsUICommandBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->vectorGraphicsUICommandBuffers[i]->setObjectName("GraphicsUICommandBufferSwapchain" + std::to_string(i));
		_deviceResources->vectorGraphicsUICommandBuffers[i]->setVKSynchronization2IsSupported(true);

		_deviceResources->vectorGraphicsNoEnvironmentCommandBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->vectorGraphicsNoEnvironmentCommandBuffers[i]->setObjectName("GraphicsNoEnvironmentCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->vectorGraphicsNoEnvironmentCommandBuffers[i]->setVKSynchronization2IsSupported(true);

		_deviceResources->vectorComputeCommandBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->vectorComputeCommandBuffers[i]->setObjectName("ComputeCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->vectorComputeCommandBuffers[i]->setVKSynchronization2IsSupported(true);

		_deviceResources->vectorComputeCooperativeMatrixCommandBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->vectorComputeCooperativeMatrixCommandBuffers[i]->setObjectName("ComputeCooperativeMatrixCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->vectorComputeCooperativeMatrixCommandBuffers[i]->setVKSynchronization2IsSupported(true);

		_deviceResources->vectorScreenSpaceBoxCommandBuffer[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->vectorScreenSpaceBoxCommandBuffer[i]->setObjectName("ScreenSpaceBoxSwapchain" + std::to_string(i));
		_deviceResources->vectorScreenSpaceBoxCommandBuffer[i]->setVKSynchronization2IsSupported(true);

		_deviceResources->vectorOrganizePatchWorkloadCommandBuffer[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->vectorOrganizePatchWorkloadCommandBuffer[i]->setObjectName("OrganizePatchWorkloadSwapchain" + std::to_string(i));
		_deviceResources->vectorOrganizePatchWorkloadCommandBuffer[i]->setVKSynchronization2IsSupported(true);

		_deviceResources->vectorNeuralNetworkEnvironmentCommandBuffer[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->vectorNeuralNetworkEnvironmentCommandBuffer[i]->setObjectName("NeuralNetworkEnvironmentSwapchain" + std::to_string(i));
		_deviceResources->vectorNeuralNetworkEnvironmentCommandBuffer[i]->setVKSynchronization2IsSupported(true);

		_deviceResources->vectorNeuralNetworkEnvironmentCooperativeMatrixCommandBuffer[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->vectorNeuralNetworkEnvironmentCooperativeMatrixCommandBuffer[i]->setObjectName("NeuralNetworkEnvironmentCooperativeMatrixSwapchain" + std::to_string(i));
		_deviceResources->vectorNeuralNetworkEnvironmentCooperativeMatrixCommandBuffer[i]->setVKSynchronization2IsSupported(true);
	}

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	// create the sampler object
	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.minFilter = samplerInfo.magFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
	samplerInfo.wrapModeU = samplerInfo.wrapModeV = samplerInfo.wrapModeW = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;
	_deviceResources->samplerBilinear = _deviceResources->device->createSampler(samplerInfo);

	// trilinear
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_LINEAR;
	_deviceResources->samplerTrilinear = _deviceResources->device->createSampler(samplerInfo);

	// trilinear with max lod clamping
	samplerInfo.lodMinimum = 2.f;
	_deviceResources->samplerTrilinearLodClamped = _deviceResources->device->createSampler(samplerInfo);

	_deviceResources->vectorGraphicsCommandBuffers[0]->begin();

	_isASTCSupported = pvr::utils::isSupportedFormat(_deviceResources->device->getPhysicalDevice(), pvrvk::Format::e_ASTC_4x4_UNORM_BLOCK);

	_deviceResources->brdfLUT = _deviceResources->device->createImageView(
		pvrvk::ImageViewCreateInfo(pvr::utils::loadAndUploadImage(_deviceResources->device, BrdfLUTTexFile, true, _deviceResources->vectorGraphicsCommandBuffers[0], *this,
			pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator)));

	_deviceResources->depthImages.resize(_swapchainLength);
	pvr::utils::createAttachmentImages(_deviceResources->depthImages, _deviceResources->device, _swapchainLength,
		pvr::utils::getSupportedDepthStencilFormat(_deviceResources->device, getDisplayAttributes()), _deviceResources->swapchain->getDimension(),
		pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT, pvrvk::SampleCountFlags::e_1_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT, "DepthStencilBufferImages");
	createOffScreenGeometryRenderPass();
	createUIRenderPass();
	createImagesAndFramebuffers();
	createDescriptorSetLayouts();
	createPipelineLayout();

	// Create Descriptor Sets
	_deviceResources->descSets[0] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->descSetLayouts[0]);
	_deviceResources->descSets[1] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->descSetLayouts[1]);
	_deviceResources->descSets[2] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->descSetLayouts[2]);

	_deviceResources->descSets[0]->setObjectName("DynamicUBODescriptorSet");
	_deviceResources->descSets[1]->setObjectName("StaticUBODescriptorSet");
	_deviceResources->descSets[2]->setObjectName("PerObjectUBODescriptorSet");

	bool requireSubmission = false;

	_deviceResources->skyBoxPass.init(*this, _deviceResources->device, _deviceResources->descriptorPool, _deviceResources->commandPool, _deviceResources->graphicsQueue,
		_deviceResources->offScreenGeometryRenderPass, _deviceResources->pipelineCache, pvrvk::Extent2D(getWidth(), getHeight()), _deviceResources->vmaAllocator);

	_deviceResources->helmetPass.init(*this, _deviceResources->device, _deviceResources->offscreenFramebuffer[0],
		_deviceResources->pipelineLayout,
		_deviceResources->pipelineCache,
		_deviceResources->vmaAllocator, _deviceResources->vectorGraphicsCommandBuffers[0], requireSubmission, _isASTCSupported);

	createUbos();

	buildComputeNeuralNetworkBuffer(_deviceResources->vectorGraphicsCommandBuffers[0]);

	updateDescriptors(); // Actually populate the data

	_screenWidth = getWidth();
	_screenHeight = getHeight();

	_textureWidth = static_cast<uint32_t>(_deviceResources->skyBoxPass.getSkyBoxMapWidth());
	_textureHeight = static_cast<uint32_t>(_deviceResources->skyBoxPass.getSkyBoxMapHeight());

	assertion((_textureWidth == nnConfiguration.imageWidth), "Texture width is not as in the baked neural network.");
	assertion((_textureHeight == nnConfiguration.imageHeight), "Texture height is not as in the baked neural network.");

	_textureNumberPatchXDimension = nnConfiguration.imageWidth / _patchSide;
	assertion((_textureWidth % _patchSide) == 0, "Texture width is not a multiple of patch side");

	_textureNumberPatchYDimension = nnConfiguration.imageHeight / _patchSide;
	assertion((_textureHeight % _patchSide) == 0, "Texture height is not a multiple of patch side");

	_textureNumPatches = _textureNumberPatchXDimension * _textureNumberPatchYDimension;

	VkPhysicalDeviceSubgroupProperties physicalDeviceSubgroupProperties{};
	_deviceResources->device->getPhysicalDevice()->populateExtensionPropertiesVk(pvrvk::StructureType::e_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES, &physicalDeviceSubgroupProperties);
	_subgroupSize = physicalDeviceSubgroupProperties.subgroupSize;
	_organizePatchWorkloadNumberPixelScreenSpaceRegion = 4 * _subgroupSize; // Each thread in a workgroup will be in charge of generating four pixels

	_screenSpaceBoxNumberPixelPerThread = 1;
	_screenSpaceBoxWorkgroupSize = _subgroupSize;

	_screenWidthScreenSpaceBox = int(ceil(float(_screenWidth) / float(_screenSpaceBoxScreenFactor)));
	_screenHeightScreenSpaceBox = int(ceil(float(_screenHeight) / float(_screenSpaceBoxScreenFactor)));

	_screenSpaceBoxXWorkgroupNumber = int(ceil(float(_screenWidthScreenSpaceBox * _screenHeightScreenSpaceBox) / float(_screenSpaceBoxWorkgroupSize)));
	assertion(((_patchSide * _patchSide) % _screenSpaceBoxWorkgroupSize) == 0, "Workgroup size is not a multiple of patch side");

	// Each thread will process a set of slots in vectorComputeOrganizePatchWorkloadBuffer defined by _organizePatchWorkloadNumberSlotPerThread
	_organizePatchWorkloadXWorkgroupNumber = int(ceil(float(_textureNumPatches) / float(_subgroupSize * _organizePatchWorkloadNumberSlotPerThread)));

	// Compute initialization
	buildComputeUniformBuffer();
	buildComputeScreenSpaceBoxBuffer();
	buildComputeOrganizePatchWorkloadBuffer();
	buildComputeDescriptorSetLayout();
	updateComputeScreenSpaceBoxDescriptorSet();
	updateComputeOrganizePatchWorkloadDescriptorSet();
	updateVectorNeuralNetworkEnvironmentDescriptorSet();
	buildScreenSpaceBoxComputePipeline();
	buildOrganizePatchWorkloadComputePipeline();
	buildNeuralNetworkEnvironmentInferenceComputePipeline();
	buildNeuralNetworkEnvironmentInferenceCooperativeMatrixComputePipeline();
	initializeAtomicCounterBuffer(_deviceResources->vectorGraphicsCommandBuffers[0]);

	_deviceResources->skyBoxPass.updateDescriptorSetSkyBoxPass(
		_deviceResources->device, _deviceResources->samplerTrilinear, _deviceResources->computeSettingsBuffer, _deviceResources->computeSettingsBufferView);

	changeSwapchainImageLayout(_deviceResources->vectorGraphicsCommandBuffers[0]);

	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->offscreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->graphicsQueue);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("Environment: Rasterization").commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText("Action: Pause");
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	_deviceResources->uiRendererCompute.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->uiFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->graphicsQueue);

	std::string uiMessage = "Environment: Compute cooperative matrix";
	_deviceResources->uiRendererCompute.getDefaultTitle()->setText(uiMessage.c_str()).commitUpdates();
	_deviceResources->uiRendererCompute.getDefaultControls()->setText("Action: Pause");
	_deviceResources->uiRendererCompute.getDefaultControls()->commitUpdates();

	_deviceResources->vectorGraphicsCommandBuffers[0]->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->vectorGraphicsCommandBuffers[0];
	submitInfo.numCommandBuffers = 1;

	// submit the graphicsQueue and wait for it to become idle
	_deviceResources->graphicsQueue->submit(&submitInfo, 1);
	_deviceResources->graphicsQueue->waitIdle();
	_deviceResources->vectorGraphicsCommandBuffers[0]->reset(pvrvk::CommandBufferResetFlags::e_RELEASE_RESOURCES_BIT);

	// Calculates the projection matrix
	bool isRotated = this->isScreenRotated() && this->isFullScreen();
	if (isRotated)
	{
		_projMtx = pvr::math::perspective(
			pvr::Api::Vulkan, glm::radians(fov), static_cast<float>(this->getHeight()) / static_cast<float>(this->getWidth()), 1.f, 2000.f, glm::pi<float>() * .5f);
	}
	else
	{
		_projMtx = pvr::math::perspective(pvr::Api::Vulkan, glm::radians(fov), static_cast<float>(this->getWidth()) / static_cast<float>(this->getHeight()), 1.f, 2000.f);
	}

	_deviceResources->uiRenderer.getDefaultTitle()->setText("Environment: Rasterization").commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText("Action: Pause");
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	// setup the camera
	_camera.setDistanceFromTarget(50.f);
	_camera.setInclination(10.f);

	_deviceResources->uboWorld.view.getElement(0, 0).setValue(glm::eulerAngleXY(glm::radians(0.f), glm::radians(120.f)) * glm::scale(glm::vec3(22.0f)));
	
	if ((_deviceResources->uboWorld.buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->uboWorld.buffer->getDeviceMemory()->flushRange();
	}

	recordCommandBuffers();
	recordSecondaryComputeCommandBuffers();
	recordComputeCommandBuffers();

	return pvr::Result::Success;
}

/// <summary>Buld the images and framebuffers used in the offscreen pass.</summary>
void VulkanNeuralNetworkEnvironment::createImagesAndFramebuffers()
{
	pvrvk::ImageCreateInfo colorImageInfoMask = pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, pvrvk::Format::e_R8_UNORM,
		pvrvk::Extent3D(getWidth(), getHeight(), 1u), pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT);

	pvrvk::ImageCreateInfo colorDebugImageInfo = pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, pvrvk::Format::e_R32G32B32A32_SFLOAT,
		pvrvk::Extent3D(getWidth(), getHeight(), 1u), pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT);

	pvrvk::ImageCreateInfo depthImageInfo = pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, _deviceResources->depthImages[0]->getFormat(),
		pvrvk::Extent3D(getWidth(), getHeight(), 1u), pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT);

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		// Build color and depth attachment image and image views for the offscreen pass of FXAA (1 sample per pixel)
		pvrvk::Image colorImage = pvr::utils::createImage(_deviceResources->device, colorImageInfoMask, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT, _deviceResources->vmaAllocator,
			pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);
		pvrvk::ImageView colorImageView = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(colorImage));

		colorImage->setObjectName("offscreenColorAttachmentMaskImageSwapchain" + std::to_string(i));
		colorImageView->setObjectName("offscreenColorAttachmentMaskImageViewSwapchain" + std::to_string(i));

		_deviceResources->offscreenColorAttachmentMaskImage.push_back(colorImage);
		_deviceResources->offscreenColorAttachmentMaskImageView.push_back(colorImageView);

		pvrvk::Image depthImage = pvr::utils::createImage(
			_deviceResources->device, depthImageInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, _deviceResources->vmaAllocator);
		pvrvk::ImageView depthImageView1SPP = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(depthImage));
		_deviceResources->offscreenDepthAttachmentImage.push_back(depthImage);
		_deviceResources->offscreenDepthAttachmentImageView.push_back(depthImageView1SPP);

		pvrvk::FramebufferCreateInfo offscreenFramebufferCreateInfo;
		offscreenFramebufferCreateInfo.setAttachment(0, _deviceResources->swapchain->getImageView(i));
		offscreenFramebufferCreateInfo.setAttachment(1, _deviceResources->offscreenColorAttachmentMaskImageView[i]);
		offscreenFramebufferCreateInfo.setAttachment(2, _deviceResources->offscreenDepthAttachmentImageView[i]);
		offscreenFramebufferCreateInfo.setDimensions(getWidth(), getHeight());
		offscreenFramebufferCreateInfo.setRenderPass(_deviceResources->offScreenGeometryRenderPass);
		_deviceResources->offscreenFramebuffer.push_back(_deviceResources->device->createFramebuffer(offscreenFramebufferCreateInfo));

		pvrvk::FramebufferCreateInfo offscreenFramebufferCreateInfo2;
		offscreenFramebufferCreateInfo2.setAttachment(0, _deviceResources->swapchain->getImageView(i));
		offscreenFramebufferCreateInfo2.setDimensions(getWidth(), getHeight());
		offscreenFramebufferCreateInfo2.setRenderPass(_deviceResources->uiRenderPass);
		_deviceResources->uiFramebuffer.push_back(_deviceResources->device->createFramebuffer(offscreenFramebufferCreateInfo2));
	}
}

/// <summary>Build the images and framebuffers used in the offscreen pass.</summary>
/// <param name="numColorAttachments">Number of color attachments to fill.</param>
/// <param name="vectorColorFormat">Vector with the color formats of the attachments.</param>
/// <param name="numSamplesPerPixel">Vector with the number of samples per pixel.</param>
/// <param name="keepColorAttachmentContent">Flag to keep or not the contents of the color attachment.</param>
/// <param name="vectorAttachmentDescription">Vector with the final attachment description.</param>
void VulkanNeuralNetworkEnvironment::fillAttachmentDescription(int numColorAttachments, const std::vector<pvrvk::Format>& vectorColorFormat, bool addDepthAttachment,
	pvrvk::SampleCountFlags numSamplesPerPixel, bool keepColorAttachmentContent, std::vector<pvrvk::AttachmentDescription>& vectorAttachmentDescription)
{
	vectorAttachmentDescription.clear();

	for (int i = 0; i < numColorAttachments; ++i)
	{
		vectorAttachmentDescription.push_back(pvrvk::AttachmentDescription::createColorDescription(vectorColorFormat[i],
			keepColorAttachmentContent ? pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL : pvrvk::ImageLayout::e_UNDEFINED, 
			keepColorAttachmentContent ? pvrvk::ImageLayout::e_GENERAL: pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL,
			keepColorAttachmentContent ? pvrvk::AttachmentLoadOp::e_LOAD : pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, numSamplesPerPixel));
	}

	if (addDepthAttachment)
	{
		vectorAttachmentDescription.push_back(pvrvk::AttachmentDescription::createDepthStencilDescription(_deviceResources->depthImages[0]->getFormat(),
			pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE,
			pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE, numSamplesPerPixel));
	}
}

/// <summary>Build the render pass used to draw the scene offscreen.</summary>
void VulkanNeuralNetworkEnvironment::createOffScreenGeometryRenderPass()
{
	std::vector<pvrvk::AttachmentDescription> vectorAttachmentDescription;
	std::vector<pvrvk::Format> vectorColorFormat = { _deviceResources->swapchain->getImageFormat(), pvrvk::Format::e_R8_UNORM };
	fillAttachmentDescription(2, vectorColorFormat, true, pvrvk::SampleCountFlags::e_1_BIT, false, vectorAttachmentDescription);
	_deviceResources->offScreenGeometryRenderPass = createTechniqueRenderPass(vectorAttachmentDescription);
	_deviceResources->offScreenGeometryRenderPass->setObjectName("offScreenGeometryRenderPass");
}

/// <summary>Build the render pass used to draw the UI on top of the compute-generated environment.</summary>
void VulkanNeuralNetworkEnvironment::createUIRenderPass()
{
	std::vector<pvrvk::AttachmentDescription> vectorAttachmentDescription;
	std::vector<pvrvk::Format> vectorColorFormat = { _deviceResources->swapchain->getImageFormat() };
	fillAttachmentDescription(1, vectorColorFormat, false, pvrvk::SampleCountFlags::e_1_BIT, true, vectorAttachmentDescription);
	_deviceResources->uiRenderPass = createTechniqueRenderPass(vectorAttachmentDescription);
	_deviceResources->uiRenderPass->setObjectName("uiRenderPass");
}

/// <summary>Build the render pass object based on the information provided in the attachment description parameter.</summary>
/// <param name="vectorAttachmentDescription">Attachment information for the render pass.</param>
/// <returns>The generated render pass.</returns>
pvrvk::RenderPass VulkanNeuralNetworkEnvironment::createTechniqueRenderPass(const std::vector<pvrvk::AttachmentDescription>& vectorAttachmentDescription)
{
	pvrvk::RenderPassCreateInfo renderPassInfo;
	pvrvk::SubpassDescription subpass;

	bool depthAttachmentPresent = false;

	for (uint32_t i = 0; i < vectorAttachmentDescription.size(); ++i)
	{
		renderPassInfo.setAttachmentDescription(i, vectorAttachmentDescription[i]);

		pvrvk::ImageLayout finalLayout = vectorAttachmentDescription[i].getFinalLayout();

		if (finalLayout == pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			subpass.setDepthStencilAttachmentReference(pvrvk::AttachmentReference(i, finalLayout));
			depthAttachmentPresent = true;
		}
		else
		{
			// Assuming the only other value used in this sample, e_COLOR_ATTACHMENT_OPTIMAL
			subpass.setColorAttachmentReference(i, pvrvk::AttachmentReference(i, finalLayout));
		}
	}

	if (depthAttachmentPresent && vectorAttachmentDescription.back().getFinalLayout() != pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		assertion(false, "Render pass depth attachment not present at last index of vectorAttachmentDescription");
	}

	renderPassInfo.setSubpass(0, subpass);

	// Add external subpass dependencies to avoid the implicit subpass dependencies and to provide more optimal pipeline stage task synchronisation
	pvrvk::SubpassDependency dependencies[2];

	dependencies[0].setSrcSubpass(VK_SUBPASS_EXTERNAL);
	dependencies[0].setDstSubpass(0);
	dependencies[0].setSrcStageMask(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT);
	dependencies[0].setDstStageMask(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT);
	dependencies[0].setSrcAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
	dependencies[0].setDstAccessMask(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT);
	dependencies[0].setDependencyFlags(pvrvk::DependencyFlags::e_BY_REGION_BIT);

	dependencies[1].setSrcSubpass(0);
	dependencies[1].setDstSubpass(VK_SUBPASS_EXTERNAL);
	dependencies[1].setSrcStageMask(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT);
	dependencies[1].setDstStageMask(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT);
	dependencies[1].setSrcAccessMask(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT);
	dependencies[1].setDstAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
	dependencies[1].setDependencyFlags(pvrvk::DependencyFlags::e_BY_REGION_BIT);

	renderPassInfo.addSubpassDependency(dependencies[0]);
	renderPassInfo.addSubpassDependency(dependencies[1]);

	return _deviceResources->device->createRenderPass(renderPassInfo);
}

/// <summary>Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanNeuralNetworkEnvironment::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame</summary>
/// <returns>Result::Success if no error occurred.</summary>
pvr::Result VulkanNeuralNetworkEnvironment::renderFrame()
{
	_deviceResources->vectorGraphicsFence[_lastFrameID]->wait();
	_deviceResources->vectorGraphicsFence[_lastFrameID]->reset();

	if (_lastFrameUsedComputeQueue)
	{
		_deviceResources->vectorComputeFence[_lastFrameID]->wait();
		_deviceResources->vectorComputeFence[_lastFrameID]->reset();
	}

	_vectorGraphicsFenceHasBeenReset[_lastFrameID] = true;
	_vectorComputeFenceHasBeenReset[_lastFrameID] = true;

	// As sometimes the swapchain index can be repeated at the beginning of the application, let's use a new set of command buffers 
	// for this repeated swapchain index to allow multiple frames in flight
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->vectorImageAcquiredSemaphores[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	if ((swapchainIndex != _lastFrameID) && !_vectorGraphicsFenceHasBeenReset[swapchainIndex])
	{
		// Sometimes the same swapchain image can be used in two consecutive frames
		_deviceResources->vectorGraphicsFence[swapchainIndex]->wait();
		_deviceResources->vectorGraphicsFence[swapchainIndex]->reset();
		
		if (_lastFrameUsedComputeQueue) { _deviceResources->vectorComputeFence[swapchainIndex]->wait(); }
		_deviceResources->vectorComputeFence[swapchainIndex]->reset();

		_vectorGraphicsFenceHasBeenReset[swapchainIndex] = true;
		_vectorComputeFenceHasBeenReset[swapchainIndex] = true;
	}

	if (!_pause) { _camera.addAzimuth(getFrameTime() * rotationSpeed); }

	if (this->isKeyPressed(pvr::Keys::A)) { _camera.addAzimuth(getFrameTime() * -.1f); }
	if (this->isKeyPressed(pvr::Keys::D)) { _camera.addAzimuth(getFrameTime() * .1f); }

	if (this->isKeyPressed(pvr::Keys::W)) { _camera.addInclination(getFrameTime() * .1f); }
	if (this->isKeyPressed(pvr::Keys::S)) { _camera.addInclination(getFrameTime() * -.1f); }

	_viewMtx = _camera.getViewMatrix();

	updateGraphicsUniformBuffers(swapchainIndex);
	updateComputeUniformBuffer(swapchainIndex);

	switch (_currentEnvironmentTechnique)
	{
		case CurrentEnvironmentTechnique::CET_RASTERIZATION:
		{
			// _lastFrameUsedComputeQueue
			pvrvk::SubmitInfo graphicsSubmitInfo;
			pvrvk::PipelineStageFlags graphicsWaitStage = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
			graphicsSubmitInfo.commandBuffers = &_deviceResources->vectorGraphicsCommandBuffers[swapchainIndex];
			graphicsSubmitInfo.numCommandBuffers = 1;
			graphicsSubmitInfo.waitDstStageMask = &graphicsWaitStage;
			graphicsSubmitInfo.waitSemaphores = &_deviceResources->vectorImageAcquiredSemaphores[_frameId]; // wait for the acquire to be finished.
			graphicsSubmitInfo.numWaitSemaphores = 1;
			graphicsSubmitInfo.signalSemaphores = &_deviceResources->vectorPresentationSemaphores[swapchainIndex]; // signal the compute sempahore when finished.
			graphicsSubmitInfo.numSignalSemaphores = 1;
			_deviceResources->graphicsQueue->submit(&graphicsSubmitInfo, 1, _deviceResources->vectorGraphicsFence[swapchainIndex]);

			_lastFrameUsedComputeQueue = false;
			break;
		}
		case CurrentEnvironmentTechnique::CET_NEURAL_NETWORK_COMPUTE:
		case CurrentEnvironmentTechnique::CET_NEURAL_NETWORK_COMPUTE_COOPERATIVE: {
			// submit the graphics command buffer
			pvrvk::SubmitInfo graphicsSubmitInfo;
			pvrvk::PipelineStageFlags graphicsWaitStage = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
			graphicsSubmitInfo.commandBuffers = &_deviceResources->vectorGraphicsNoEnvironmentCommandBuffers[swapchainIndex];
			graphicsSubmitInfo.numCommandBuffers = 1;
			graphicsSubmitInfo.waitDstStageMask = &graphicsWaitStage;
			graphicsSubmitInfo.waitSemaphores = &_deviceResources->vectorImageAcquiredSemaphores[_frameId]; // wait for the acquire to be finished.
			graphicsSubmitInfo.numWaitSemaphores = 1;
			graphicsSubmitInfo.signalSemaphores = &_deviceResources->vectorComputeSemaphores[swapchainIndex]; // signal the compute sempahore when finished.
			graphicsSubmitInfo.numSignalSemaphores = 1;
			_deviceResources->graphicsQueue->submit(&graphicsSubmitInfo, 1, nullptr);

			// submit the compute command buffer
			pvrvk::SubmitInfo computeSubmitInfo;
			pvrvk::PipelineStageFlags computeWaitStage = pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT;
			computeSubmitInfo.commandBuffers = (_currentEnvironmentTechnique == CurrentEnvironmentTechnique::CET_NEURAL_NETWORK_COMPUTE)
				? &_deviceResources->vectorComputeCommandBuffers[swapchainIndex]
				: &_deviceResources->vectorComputeCooperativeMatrixCommandBuffers[swapchainIndex];
			computeSubmitInfo.numCommandBuffers = 1;
			computeSubmitInfo.waitDstStageMask = &computeWaitStage;
			computeSubmitInfo.waitSemaphores = &_deviceResources->vectorComputeSemaphores[swapchainIndex]; // wait for the graphics command buffer to be finished.
			computeSubmitInfo.numWaitSemaphores = 1;
			computeSubmitInfo.signalSemaphores =
				&_deviceResources->vectorUISemaphores[swapchainIndex]; // signal the UI semaphore when it is finish with compute.
			computeSubmitInfo.numSignalSemaphores = 1;
			_deviceResources->computeQueue->submit(&computeSubmitInfo, 1, _deviceResources->vectorComputeFence[swapchainIndex]);

			pvrvk::SubmitInfo graphicsUISubmitInfo;
			graphicsUISubmitInfo.commandBuffers = &_deviceResources->vectorGraphicsUICommandBuffers[swapchainIndex];
			graphicsUISubmitInfo.numCommandBuffers = 1;
			graphicsUISubmitInfo.waitDstStageMask = &graphicsWaitStage;
			graphicsUISubmitInfo.waitSemaphores = &_deviceResources->vectorUISemaphores[swapchainIndex]; // wait for the acquire to be finished.
			graphicsUISubmitInfo.numWaitSemaphores = 1;
			graphicsUISubmitInfo.signalSemaphores = &_deviceResources->vectorPresentationSemaphores[swapchainIndex]; // signal the compute sempahore when finished.
			graphicsUISubmitInfo.numSignalSemaphores = 1;
			_deviceResources->graphicsQueue->submit(&graphicsUISubmitInfo, 1, _deviceResources->vectorGraphicsFence[swapchainIndex]);

			_lastFrameUsedComputeQueue = true;
			break;
		}
	}

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->graphicsQueue, _deviceResources->commandPool, _deviceResources->swapchain, swapchainIndex, this->getScreenshotFileName(),
			_deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	// present
	pvrvk::PresentInfo presentInfo;
	presentInfo.waitSemaphores = &_deviceResources->vectorPresentationSemaphores[swapchainIndex];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.numSwapchains = 1;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.imageIndices = &swapchainIndex;
	_deviceResources->graphicsQueue->present(presentInfo);

	_frameId = (_frameId + 1) % _swapchainLength;

	_lastFrameID = swapchainIndex;

	updateEnvironmentTechnique();

	return pvr::Result::Success;
}

/// <summary>Pre-record the graphics commands to draw the scene with environment, and without environment.</summary>
void VulkanNeuralNetworkEnvironment::recordCommandBuffers()
{
	recordGraphicsCommandBuffers(_deviceResources->vectorGraphicsCommandBuffers, true);
	recordGraphicsCommandBuffers(_deviceResources->vectorGraphicsNoEnvironmentCommandBuffers, false);
	recordGraphicsUICommandBuffers(_deviceResources->vectorGraphicsUICommandBuffers);
}

/// <summary>Pre-record the rendering commands.</summary>
/// <param name="vectorCommandBuffer">Vector with the command buffers to record to.</param>
/// <param name="includeEnvironmentPass">Flag to add the drawing of the environment to the commands.</param>
void VulkanNeuralNetworkEnvironment::recordGraphicsCommandBuffers(std::vector<pvrvk::CommandBuffer>& vectorCommandBuffer, bool includeEnvironmentPass)
{
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		const pvrvk::ClearValue clearValues[] = { pvrvk::ClearValue(0.6f, 0.64f, 0.66f, 1.0f), pvrvk::ClearValue(1.0f, 0.0f, 0.0f, 1.0f), pvrvk::ClearValue(1.f, 0) };

		// begin recording commands
		vectorCommandBuffer[i]->begin();

		// Transition the mask image from e_SHADER_READ_ONLY_OPTIMAL for sampling in the previous compute pass to e_COLOR_ATTACHMENT_OPTIMAL to draw to it
		pvrvk::MemoryBarrierSet2 computeToGraphicsBarrier;
		pvrvk::ImageMemoryBarrier2 imageBarrier2;
		imageBarrier2.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_NONE_KHR);
		imageBarrier2.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_COLOR_ATTACHMENT_READ_BIT_KHR);
		imageBarrier2.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
		imageBarrier2.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
		imageBarrier2.setOldLayout(pvrvk::ImageLayout::e_PRESENT_SRC_KHR);
		imageBarrier2.setNewLayout(pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
		imageBarrier2.setSrcQueueFamilyIndex(_deviceResources->graphicsQueue->getFamilyIndex());
		imageBarrier2.setDstQueueFamilyIndex(_deviceResources->graphicsQueue->getFamilyIndex());
		imageBarrier2.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));
		imageBarrier2.setImage(_deviceResources->swapchain->getImage(i));
		computeToGraphicsBarrier.addBarrier(imageBarrier2);
		vectorCommandBuffer[i]->pipelineBarrier2(computeToGraphicsBarrier);

		pvr::utils::beginCommandBufferDebugLabel(vectorCommandBuffer[i], pvrvk::DebugUtilsLabel("MainRenderPass"));

		// begin the renderpass
		vectorCommandBuffer[i]->beginRenderPass(
			_deviceResources->offscreenFramebuffer[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, clearValues, ARRAY_SIZE(clearValues));

		if (includeEnvironmentPass)
		{
			// Render the sky box
			_deviceResources->skyBoxPass.recordCommands(vectorCommandBuffer[i], i, _deviceResources->computeSettingsBufferView);
		}

		uint32_t offsets[1];
		// get the matrix array offset
		offsets[0] = _deviceResources->uboPerFrame.view.getDynamicSliceOffset(i);

		// bind the descriptor sets
		vectorCommandBuffer[i]->bindDescriptorSets(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->pipelineLayout, 0, _deviceResources->descSets, ARRAY_SIZE(_deviceResources->descSets), offsets, 1);

		_deviceResources->helmetPass.recordCommands(vectorCommandBuffer[i]);

		if (includeEnvironmentPass)
		{
			// record the ui renderer.
			_deviceResources->uiRenderer.beginRendering(vectorCommandBuffer[i]);
			_deviceResources->uiRenderer.getDefaultTitle()->render();
			_deviceResources->uiRenderer.getDefaultControls()->render();
			_deviceResources->uiRenderer.getSdkLogo()->render();
			_deviceResources->uiRenderer.endRendering();
		}

		vectorCommandBuffer[i]->endRenderPass();

		pvrvk::MemoryBarrierSet2 graphicsToComputeBarrier;
		pvrvk::ImageMemoryBarrier2 imageBarrier3;
		imageBarrier3.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_COLOR_ATTACHMENT_WRITE_BIT_KHR);
		if (includeEnvironmentPass)
		{
			imageBarrier3.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_NONE_KHR);
		}
		else
		{
			imageBarrier3.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_COLOR_ATTACHMENT_READ_BIT_KHR);
		}
		imageBarrier3.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
		imageBarrier3.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
		imageBarrier3.setOldLayout(pvrvk::ImageLayout::e_ATTACHMENT_OPTIMAL_KHR);
		if (includeEnvironmentPass)
		{
			imageBarrier3.setNewLayout(pvrvk::ImageLayout::e_PRESENT_SRC_KHR);
		}
		else
		{
			imageBarrier3.setNewLayout(pvrvk::ImageLayout::e_GENERAL);
		}
		imageBarrier3.setSrcQueueFamilyIndex(_deviceResources->graphicsQueue->getFamilyIndex());
		imageBarrier3.setDstQueueFamilyIndex(_deviceResources->graphicsQueue->getFamilyIndex());
		imageBarrier3.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));
		imageBarrier3.setImage(_deviceResources->swapchain->getImage(i));
		graphicsToComputeBarrier.addBarrier(imageBarrier3);

		// Transition the mask image from e_COLOR_ATTACHMENT_OPTIMAL to draw to it to e_SHADER_READ_ONLY_OPTIMAL for sampling in the upcoming compute pass
		pvrvk::ImageMemoryBarrier2 imageBarrier4;
		imageBarrier4.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_COLOR_ATTACHMENT_WRITE_BIT_KHR);
		imageBarrier4.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_NONE_KHR);
		imageBarrier4.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
		imageBarrier4.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COMPUTE_SHADER_BIT_KHR);
		imageBarrier4.setOldLayout(pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
		if (includeEnvironmentPass)
		{
			imageBarrier4.setNewLayout(pvrvk::ImageLayout::e_PRESENT_SRC_KHR);
		}
		else
		{
			imageBarrier4.setNewLayout(pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL);
		}
		imageBarrier4.setSrcQueueFamilyIndex(_deviceResources->graphicsQueue->getFamilyIndex());
		imageBarrier4.setDstQueueFamilyIndex(_deviceResources->computeQueue->getFamilyIndex());
		imageBarrier4.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));
		imageBarrier4.setImage(_deviceResources->offscreenColorAttachmentMaskImage[i]);

		graphicsToComputeBarrier.addBarrier(imageBarrier4);
		vectorCommandBuffer[i]->pipelineBarrier2(graphicsToComputeBarrier);

		pvr::utils::endCommandBufferDebugLabel(vectorCommandBuffer[i]);
		vectorCommandBuffer[i]->end();
	}
}

/// <summary>Pre-record the rendering commands for the UI.</summary>
/// <param name="vectorCommandBuffer">Vector with the command buffers to record to.</param>
void VulkanNeuralNetworkEnvironment::recordGraphicsUICommandBuffers(std::vector<pvrvk::CommandBuffer>& vectorCommandBuffer)
{
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		// begin recording commands
		vectorCommandBuffer[i]->begin();

		pvr::utils::beginCommandBufferDebugLabel(vectorCommandBuffer[i], pvrvk::DebugUtilsLabel("UIGraphicsPass"));

		// begin the renderpass
		vectorCommandBuffer[i]->beginRenderPass(_deviceResources->uiFramebuffer[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, nullptr, 0);

		// record the ui renderer.
		_deviceResources->uiRendererCompute.beginRendering(vectorCommandBuffer[i]);
		_deviceResources->uiRendererCompute.getDefaultTitle()->render();
		_deviceResources->uiRendererCompute.getDefaultControls()->render();
		_deviceResources->uiRendererCompute.getSdkLogo()->render();
		_deviceResources->uiRendererCompute.endRendering();

		vectorCommandBuffer[i]->endRenderPass();

		pvrvk::MemoryBarrierSet2 graphicsToComputeBarrier;
		pvrvk::ImageMemoryBarrier2 imageBarrier3;
		imageBarrier3.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_COLOR_ATTACHMENT_WRITE_BIT_KHR);
		imageBarrier3.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_NONE_KHR);
		imageBarrier3.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
		imageBarrier3.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
		imageBarrier3.setOldLayout(pvrvk::ImageLayout::e_GENERAL);
		imageBarrier3.setNewLayout(pvrvk::ImageLayout::e_PRESENT_SRC_KHR);
		imageBarrier3.setSrcQueueFamilyIndex(_deviceResources->graphicsQueue->getFamilyIndex());
		imageBarrier3.setDstQueueFamilyIndex(_deviceResources->graphicsQueue->getFamilyIndex());
		imageBarrier3.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));
		imageBarrier3.setImage(_deviceResources->swapchain->getImage(i));
		graphicsToComputeBarrier.addBarrier(imageBarrier3);
		vectorCommandBuffer[i]->pipelineBarrier2(graphicsToComputeBarrier);

		pvr::utils::endCommandBufferDebugLabel(vectorCommandBuffer[i]);

		vectorCommandBuffer[i]->end();
	}
}

/// <summary>Pre-record secondary command buffers for the different stages of the compute part of the sample.</summary>
void VulkanNeuralNetworkEnvironment::recordSecondaryComputeCommandBuffers()
{
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->vectorScreenSpaceBoxCommandBuffer[i]->begin(pvrvk::CommandBufferUsageFlags::e_SIMULTANEOUS_USE_BIT);
		recordScreenSpaceBoxCommandBuffer(i);
		_deviceResources->vectorScreenSpaceBoxCommandBuffer[i]->end();

		_deviceResources->vectorOrganizePatchWorkloadCommandBuffer[i]->begin(pvrvk::CommandBufferUsageFlags::e_SIMULTANEOUS_USE_BIT);
		recordOrganizePatchWorkloadCommandBuffer(i);
		_deviceResources->vectorOrganizePatchWorkloadCommandBuffer[i]->end();

		_deviceResources->vectorNeuralNetworkEnvironmentCommandBuffer[i]->begin();
		recordNeuralNetworkEnvironmentCommandBuffer(i);
		_deviceResources->vectorNeuralNetworkEnvironmentCommandBuffer[i]->end();

		_deviceResources->vectorNeuralNetworkEnvironmentCooperativeMatrixCommandBuffer[i]->begin();
		recordNeuralNetworkEnvironmentCooperativeMatrixCommandBuffer(i);
		_deviceResources->vectorNeuralNetworkEnvironmentCooperativeMatrixCommandBuffer[i]->end();
	}
}

/// <summary>Record command buffer for the compute pass which builds screen-space boxes for the visible patches of the environment.</summary>
/// <param name="swapIndex">Swapchain index.</param>
void VulkanNeuralNetworkEnvironment::recordScreenSpaceBoxCommandBuffer(uint32_t swapIndex)
{
	_deviceResources->vectorScreenSpaceBoxCommandBuffer[swapIndex]->setObjectName("ScreenSpaceBoxSwapchain" + std::to_string(swapIndex));

	// Reset the _deviceResources->vectorComputeScreenSpaceBoxBuffer[swapIndex] buffer so current data is not mixed with prevous executions
	_deviceResources->vectorScreenSpaceBoxCommandBuffer[swapIndex]->fillBuffer(_deviceResources->vectorComputeScreenSpaceBoxBuffer[swapIndex], 0, 0, VK_WHOLE_SIZE);

	pvr::utils::beginCommandBufferDebugLabel(_deviceResources->vectorScreenSpaceBoxCommandBuffer[swapIndex], pvrvk::DebugUtilsLabel("ComputePatch"));

	_deviceResources->vectorScreenSpaceBoxCommandBuffer[swapIndex]->bindPipeline(_deviceResources->computeScreenSpaceBoxComputePipeline);

	uint32_t offset = _deviceResources->computeSettingsBufferView.getDynamicSliceOffset(swapIndex);

	_deviceResources->vectorScreenSpaceBoxCommandBuffer[swapIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_COMPUTE, _deviceResources->computeScreenSpaceBoxPipelinelayout, 0,
		_deviceResources->vectorComputeScreenSpaceBoxDescriptorSet[swapIndex], &offset, 1);

	// First dispatch: tag all the patches from the environment visible from camera in a buffer
	_deviceResources->vectorScreenSpaceBoxCommandBuffer[swapIndex]->dispatch(_screenSpaceBoxXWorkgroupNumber, 1, 1);

	// Add a barrier for vectorComputeScreenSpaceBoxBuffer[swapIndex] so the next compute dispatch, dependant on the results, sees the updated information
	pvrvk::MemoryBarrierSet2 computeBarrier;
	pvrvk::BufferMemoryBarrier2 computeBufferBarrier;
	computeBufferBarrier.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_SHADER_WRITE_BIT_KHR);
	computeBufferBarrier.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_SHADER_READ_BIT_KHR);
	computeBufferBarrier.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COMPUTE_SHADER_BIT_KHR);
	computeBufferBarrier.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COMPUTE_SHADER_BIT_KHR);
	computeBufferBarrier.setOffset(0);
	computeBufferBarrier.setSize(static_cast<uint32_t>(_deviceResources->vectorComputeScreenSpaceBoxBuffer[swapIndex]->getDeviceMemory()->getSize()));
	computeBufferBarrier.setSrcQueueFamilyIndex(_deviceResources->computeQueue->getFamilyIndex());
	computeBufferBarrier.setDstQueueFamilyIndex(_deviceResources->computeQueue->getFamilyIndex());
	computeBufferBarrier.setBuffer(_deviceResources->vectorComputeScreenSpaceBoxBuffer[swapIndex]);
	computeBarrier.addBarrier(computeBufferBarrier);
	_deviceResources->vectorScreenSpaceBoxCommandBuffer[swapIndex]->pipelineBarrier2(computeBarrier);

	pvr::utils::endCommandBufferDebugLabel(_deviceResources->vectorScreenSpaceBoxCommandBuffer[swapIndex]);
}

/// <summary>Record command buffer for the compute pass which splits the screen-space boxes for the visible patches of the environment.
/// into smaller regions that each workgroup will take care of, infering the values of each region pixel.</summary>
/// <param name="swapIndex">Swapchain index.</param>
void VulkanNeuralNetworkEnvironment::recordOrganizePatchWorkloadCommandBuffer(uint32_t swapIndex)
{
	// Second dispatch: For each not null element in vectorComputeScreenSpaceBoxBuffer[swapIndex], compute the size of the screen box to use later to generate pixels
	// with the neural network and write in the final buffer screen space boxes with a balanced amount of pixels each workgroup
	// later will take care of.

	_deviceResources->vectorOrganizePatchWorkloadCommandBuffer[swapIndex]->setObjectName("OrganizePatchWorkloadSwapchain" + std::to_string(swapIndex));

	// Reset the vectorComputeOrganizePatchWorkloadBuffer[swapIndex] and vectorComputeAtomicCounterBuffer[swapIndex] buffers
	_deviceResources->vectorOrganizePatchWorkloadCommandBuffer[swapIndex]->fillBuffer(_deviceResources->vectorComputeOrganizePatchWorkloadBuffer[swapIndex], 0, 0, VK_WHOLE_SIZE);

	// Initialize only the first four bytes as those are the ones changed in the below dispatch (the second and third remaining values can stay as with initialization value, "1, 1")
	_deviceResources->vectorOrganizePatchWorkloadCommandBuffer[swapIndex]->fillBuffer(_deviceResources->vectorComputeAtomicCounterBuffer[swapIndex], 0, 0, 4);

	_deviceResources->vectorOrganizePatchWorkloadCommandBuffer[swapIndex]->fillBuffer(_deviceResources->vectorComputeOrganizePatchWorkloadDebugBuffer[swapIndex], 0, 0, VK_WHOLE_SIZE);

	_deviceResources->vectorOrganizePatchWorkloadCommandBuffer[swapIndex]->bindPipeline(_deviceResources->computeOrganizePatchWorkloadComputePipeline);

	uint32_t offset = _deviceResources->computeSettingsBufferView.getDynamicSliceOffset(swapIndex);

	_deviceResources->vectorOrganizePatchWorkloadCommandBuffer[swapIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_COMPUTE,
		_deviceResources->computeOrganizePatchWorkloadPipelinelayout,
		0, _deviceResources->vectorComputeOrganizePatchWorkloadDescriptorSet[swapIndex], &offset, 1);

	_deviceResources->vectorOrganizePatchWorkloadCommandBuffer[swapIndex]->dispatch(_organizePatchWorkloadXWorkgroupNumber, 1, 1);

	// Add a barrier for vectorComputeOrganizePatchWorkloadBuffer[swapIndex] so the next compute dispatch, dependant on the results, sees the updated information
	pvrvk::MemoryBarrierSet2 computeBarrier2;
	pvrvk::BufferMemoryBarrier2 computeBufferBarrier2;
	computeBufferBarrier2.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_SHADER_WRITE_BIT_KHR);
	computeBufferBarrier2.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_SHADER_READ_BIT_KHR);
	computeBufferBarrier2.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COMPUTE_SHADER_BIT_KHR);
	computeBufferBarrier2.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COMPUTE_SHADER_BIT_KHR);
	computeBufferBarrier2.setOffset(0);
	computeBufferBarrier2.setSize(static_cast<uint32_t>(_deviceResources->vectorComputeOrganizePatchWorkloadBuffer[swapIndex]->getDeviceMemory()->getSize()));
	computeBufferBarrier2.setSrcQueueFamilyIndex(_deviceResources->computeQueue->getFamilyIndex());
	computeBufferBarrier2.setDstQueueFamilyIndex(_deviceResources->computeQueue->getFamilyIndex());
	computeBufferBarrier2.setBuffer(_deviceResources->vectorComputeOrganizePatchWorkloadBuffer[swapIndex]);
	computeBarrier2.addBarrier(computeBufferBarrier2);

	// Add a barrier for vectorComputeAtomicCounterBuffer[swapIndex] so the compute dispatch indirect has updated results
	pvrvk::BufferMemoryBarrier2 computeBufferBarrier3;
	computeBufferBarrier3.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_SHADER_WRITE_BIT_KHR);
	computeBufferBarrier3.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_INDIRECT_COMMAND_READ_BIT_KHR);
	computeBufferBarrier3.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COMPUTE_SHADER_BIT_KHR);
	computeBufferBarrier3.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_DRAW_INDIRECT_BIT_KHR);
	computeBufferBarrier3.setOffset(0);
	computeBufferBarrier3.setSize(4); // Update only the first four bytes, which are the only ones modified
	computeBufferBarrier3.setSrcQueueFamilyIndex(_deviceResources->computeQueue->getFamilyIndex());
	computeBufferBarrier3.setDstQueueFamilyIndex(_deviceResources->computeQueue->getFamilyIndex());
	computeBufferBarrier3.setBuffer(_deviceResources->vectorComputeAtomicCounterBuffer[swapIndex]);
	computeBarrier2.addBarrier(computeBufferBarrier3);
	_deviceResources->vectorOrganizePatchWorkloadCommandBuffer[swapIndex]->pipelineBarrier2(computeBarrier2);
}

/// <summary>Record command buffer for the compute pass which infers the values of the pixels for a specific region of the screen.
/// This compute pass uses compute shaders to infer the pixel values through loading a specific neural network which approximates a 
/// specific part of the environment texture. Each thread will infer at least one pixel.</summary>
/// <param name="swapIndex">Swapchain index.</param>
void VulkanNeuralNetworkEnvironment::recordNeuralNetworkEnvironmentCommandBuffer(uint32_t swapIndex)
{
	// Third dispatch: Each workgroup will read the information of one slot from the organizePatchWorkload buffer, load the information of the
	// neural network patch to be approximated from the neuralNetworkEnvironmentBuffer and infer the pixels in the screen-space box belonging to the patch

	_deviceResources->vectorNeuralNetworkEnvironmentCommandBuffer[swapIndex]->setObjectName("NeuralNetworkEnvironmentSwapchain" + std::to_string(swapIndex));

	_deviceResources->vectorNeuralNetworkEnvironmentCommandBuffer[swapIndex]->fillBuffer(_deviceResources->vectorNNEnvironmentDebugBuffer[swapIndex], 0, 0, VK_WHOLE_SIZE);

	_deviceResources->vectorNeuralNetworkEnvironmentCommandBuffer[swapIndex]->bindPipeline(_deviceResources->computeNeuralNetworkEnvironmentComputePipeline);

	uint32_t offset = _deviceResources->computeSettingsBufferView.getDynamicSliceOffset(swapIndex);

	_deviceResources->vectorNeuralNetworkEnvironmentCommandBuffer[swapIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_COMPUTE,
		_deviceResources->computeNeuralNetworkEnvironmentPipelinelayout, 0, _deviceResources->vectorNeuralNetworkEnvironmentDescriptorSet[swapIndex], &offset, 1);

	_deviceResources->vectorNeuralNetworkEnvironmentCommandBuffer[swapIndex]->dispatchIndirect(_deviceResources->vectorComputeAtomicCounterBuffer[swapIndex], 0);

	// Change the swapchain image layour from general to present
	pvrvk::MemoryBarrierSet2 computeToGraphicsBarrier;
	pvrvk::ImageMemoryBarrier2 presentBarrier;
	presentBarrier.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_SHADER_STORAGE_WRITE_BIT_KHR);
	presentBarrier.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_NONE_KHR);
	presentBarrier.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COMPUTE_SHADER_BIT_KHR);
	presentBarrier.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
	presentBarrier.setOldLayout(pvrvk::ImageLayout::e_GENERAL);
	presentBarrier.setNewLayout(pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
	presentBarrier.setSrcQueueFamilyIndex(_deviceResources->computeQueue->getFamilyIndex());
	presentBarrier.setDstQueueFamilyIndex(_deviceResources->graphicsQueue->getFamilyIndex());
	presentBarrier.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));
	presentBarrier.setImage(_deviceResources->swapchain->getImage(swapIndex));
	computeToGraphicsBarrier.addBarrier(presentBarrier);
	_deviceResources->vectorNeuralNetworkEnvironmentCommandBuffer[swapIndex]->pipelineBarrier2(computeToGraphicsBarrier);
}

/// <summary>Record command buffer for the compute pass which infers the values of the pixels for a specific region of the screen.
/// This compute pass uses compute shaders and cooperative matrices to infer the pixel values through loading a specific neural network which 
/// approximates a specific part of the environment texture.</summary>
/// <param name="swapIndex">Swapchain index.</param>
void VulkanNeuralNetworkEnvironment::recordNeuralNetworkEnvironmentCooperativeMatrixCommandBuffer(uint32_t swapIndex)
{
	// Third dispatch: Each workgroup will read the information of one slot from the organizePatchWorkload buffer, load the information of the
	// neural network patch to be approximated from the neuralNetworkEnvironmentBuffer and infer the pixels in the screen-space box belonging to the patch

	_deviceResources->vectorNeuralNetworkEnvironmentCooperativeMatrixCommandBuffer[swapIndex]->setObjectName("NeuralNetworkEnvironmentCooperativeMatrixSwapchain" + std::to_string(swapIndex));

	_deviceResources->vectorNeuralNetworkEnvironmentCooperativeMatrixCommandBuffer[swapIndex]->fillBuffer(
		_deviceResources->vectorNNEnvironmentDebugBuffer[swapIndex], 0, 0, VK_WHOLE_SIZE);

	_deviceResources->vectorNeuralNetworkEnvironmentCooperativeMatrixCommandBuffer[swapIndex]->bindPipeline(
		_deviceResources->computeNeuralNetworkEnvironmentCooperativeMatrixComputePipeline);

	uint32_t offset = _deviceResources->computeSettingsBufferView.getDynamicSliceOffset(swapIndex);

	_deviceResources->vectorNeuralNetworkEnvironmentCooperativeMatrixCommandBuffer[swapIndex]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_COMPUTE,
		_deviceResources->computeNeuralNetworkEnvironmentPipelinelayout, 0, _deviceResources->vectorNeuralNetworkEnvironmentDescriptorSet[swapIndex], &offset, 1);

	_deviceResources->vectorNeuralNetworkEnvironmentCooperativeMatrixCommandBuffer[swapIndex]->dispatchIndirect(_deviceResources->vectorComputeAtomicCounterBuffer[swapIndex], 0);

	// Change the swapchain image layour from general to present
	pvrvk::MemoryBarrierSet2 computeToGraphicsBarrier;
	pvrvk::ImageMemoryBarrier2 presentBarrier;
	presentBarrier.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_SHADER_STORAGE_WRITE_BIT_KHR);
	presentBarrier.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_NONE_KHR);
	presentBarrier.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COMPUTE_SHADER_BIT_KHR);
	presentBarrier.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
	presentBarrier.setOldLayout(pvrvk::ImageLayout::e_GENERAL);
	presentBarrier.setNewLayout(pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
	presentBarrier.setSrcQueueFamilyIndex(_deviceResources->computeQueue->getFamilyIndex());
	presentBarrier.setDstQueueFamilyIndex(_deviceResources->graphicsQueue->getFamilyIndex());
	presentBarrier.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));
	presentBarrier.setImage(_deviceResources->swapchain->getImage(swapIndex));
	computeToGraphicsBarrier.addBarrier(presentBarrier);
	_deviceResources->vectorNeuralNetworkEnvironmentCooperativeMatrixCommandBuffer[swapIndex]->pipelineBarrier2(computeToGraphicsBarrier);
}

/// <summary>Record command buffer for the two types of implementations to infer pixels (plain compute, and using cooperative matrix).</summary>
void VulkanNeuralNetworkEnvironment::recordComputeCommandBuffers()
{
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->vectorComputeCommandBuffers[i]->setObjectName("ComputeCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->vectorComputeCommandBuffers[i]->begin();
		_deviceResources->vectorComputeCommandBuffers[i]->executeCommands(_deviceResources->vectorScreenSpaceBoxCommandBuffer[i]);
		_deviceResources->vectorComputeCommandBuffers[i]->executeCommands(_deviceResources->vectorOrganizePatchWorkloadCommandBuffer[i]);
		_deviceResources->vectorComputeCommandBuffers[i]->executeCommands(_deviceResources->vectorNeuralNetworkEnvironmentCommandBuffer[i]);
		_deviceResources->vectorComputeCommandBuffers[i]->end();

		_deviceResources->vectorComputeCooperativeMatrixCommandBuffers[i]->setObjectName("ComputeCommandBufferCooperativeMatrixSwapchain" + std::to_string(i));
		_deviceResources->vectorComputeCooperativeMatrixCommandBuffers[i]->begin();
		_deviceResources->vectorComputeCooperativeMatrixCommandBuffers[i]->executeCommands(_deviceResources->vectorScreenSpaceBoxCommandBuffer[i]);
		_deviceResources->vectorComputeCooperativeMatrixCommandBuffers[i]->executeCommands(_deviceResources->vectorOrganizePatchWorkloadCommandBuffer[i]);
		_deviceResources->vectorComputeCooperativeMatrixCommandBuffers[i]->executeCommands(_deviceResources->vectorNeuralNetworkEnvironmentCooperativeMatrixCommandBuffer[i]);
		_deviceResources->vectorComputeCooperativeMatrixCommandBuffers[i]->end();
	}
}

/// <summary>Query and show information on the possible cooperative matrix configurations availables on the GPU the application is running.</summary>
void VulkanNeuralNetworkEnvironment::queryCooperativeMatrixInformation()
{
	uint32_t numCooperativeMatrixProperties = 0;
	
	pvrvk::impl::vkThrowIfFailed(_deviceResources->instance->getVkBindings().vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR(
		_deviceResources->device->getPhysicalDevice()->getVkHandle(), &numCooperativeMatrixProperties, NULL));

	_vectorCooperativeMatrixPropertiesKHR.resize(numCooperativeMatrixProperties);
	for (uint32_t i = 0; i < numCooperativeMatrixProperties; ++i)
	{
		_vectorCooperativeMatrixPropertiesKHR[i].sType = VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_PROPERTIES_KHR;
		_vectorCooperativeMatrixPropertiesKHR[i].pNext = NULL;
	}

	pvrvk::impl::vkThrowIfFailed(_deviceResources->instance->getVkBindings().vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR(
		_deviceResources->device->getPhysicalDevice()->getVkHandle(), &numCooperativeMatrixProperties, &_vectorCooperativeMatrixPropertiesKHR[0]));
}

/// <summary>Build descriptor set layouts for some uniform buffers used in the sample.</summary>
void VulkanNeuralNetworkEnvironment::createDescriptorSetLayouts()
{
	// Create the descriptor set layouts

	// Dynamic UBO: Transformation matrix etc.
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // binding 0
		descSetInfo.setBinding(1, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT); // binding 1
		_deviceResources->descSetLayouts[DescSetIndex::PerFrame] = _deviceResources->device->createDescriptorSetLayout(descSetInfo);
	}

	// "Static" UBO: Scene maps (environment, irradiance)
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // binding 0
		descSetInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // binding 2: Diffuse irradianceMap
		descSetInfo.setBinding(2, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // binding 3: Specular irradianceMap
		descSetInfo.setBinding(3, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // binding 4: Environment map (for perfect reflections)
		descSetInfo.setBinding(4, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // binding 5: brdfLUTmap
		_deviceResources->descSetLayouts[DescSetIndex::Model] = _deviceResources->device->createDescriptorSetLayout(descSetInfo);
	}

	// Material textures
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // binding 0: Albedo
		descSetInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // binding 1: MetallicRoughness
		descSetInfo.setBinding(2, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // binding 2: Normal
		descSetInfo.setBinding(3, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // binding 3: Emissive
		descSetInfo.setBinding(4, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // binding 1
		_deviceResources->descSetLayouts[DescSetIndex::Material] = _deviceResources->device->createDescriptorSetLayout(descSetInfo);
	}
}

/// <summary>Build the pipeline layout object to use the descriptor set layouts built in VulkanNeuralNetworkEnvironment::createDescriptorSetLayouts().</summary>
void VulkanNeuralNetworkEnvironment::createPipelineLayout()
{
	// create the pipeline layout
	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
	pipeLayoutInfo.addDescSetLayout(_deviceResources->descSetLayouts[0]);
	pipeLayoutInfo.addDescSetLayout(_deviceResources->descSetLayouts[1]);
	pipeLayoutInfo.addDescSetLayout(_deviceResources->descSetLayouts[2]);

	pipeLayoutInfo.setPushConstantRange(0,
		pvrvk::PushConstantRange(
			pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, 0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::Integer) * 2)));

	_deviceResources->pipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
}

/// <summary>Build the uniform buffer used in all compute shaders of the sample.</summary>
void VulkanNeuralNetworkEnvironment::buildComputeUniformBuffer()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement("inverseViewMatrix", pvr::GpuDatatypes::mat4x4);
	desc.addElement("inverseProjectionMatrix", pvr::GpuDatatypes::mat4x4);
	desc.addElement("viewMatrix", pvr::GpuDatatypes::mat4x4);
	desc.addElement("projectionMatrix", pvr::GpuDatatypes::mat4x4);
	desc.addElement("eyePos", pvr::GpuDatatypes::vec4);
	desc.addElement("exposure", pvr::GpuDatatypes::Float);
	desc.addElement("patchSide", pvr::GpuDatatypes::Integer);
	desc.addElement("textureWidth", pvr::GpuDatatypes::Integer);
	desc.addElement("textureHeight", pvr::GpuDatatypes::Integer);
	desc.addElement("screenSpaceBoxNumberPixelPerThread", pvr::GpuDatatypes::Integer);
	desc.addElement("textureNumberPatchXDimension", pvr::GpuDatatypes::Integer);
	desc.addElement("textureNumberPatchYDimension", pvr::GpuDatatypes::Integer);
	desc.addElement("screenWidthScreenSpaceBox", pvr::GpuDatatypes::Integer);
	desc.addElement("screenHeightScreenSpaceBox", pvr::GpuDatatypes::Integer);
	desc.addElement("screenWidth", pvr::GpuDatatypes::Integer);
	desc.addElement("screenHeight", pvr::GpuDatatypes::Integer);
	desc.addElement("organizePatchWorkloadNumberPixelScreenSpaceRegion", pvr::GpuDatatypes::Integer);
	desc.addElement("neuronsPerLayer0", pvr::GpuDatatypes::Integer);
	desc.addElement("neuronsPerLayer1", pvr::GpuDatatypes::Integer);
	desc.addElement("neuronsPerLayer2", pvr::GpuDatatypes::Integer);
	desc.addElement("neuronsPerLayer3", pvr::GpuDatatypes::Integer);
	desc.addElement("neuronsPerLayer4", pvr::GpuDatatypes::Integer);
	desc.addElement("connectionOffset0", pvr::GpuDatatypes::Integer);
	desc.addElement("connectionOffset1", pvr::GpuDatatypes::Integer);
	desc.addElement("connectionOffset2", pvr::GpuDatatypes::Integer);
	desc.addElement("connectionOffset3", pvr::GpuDatatypes::Integer);
	desc.addElement("connectionOffset4", pvr::GpuDatatypes::Integer);
	desc.addElement("neuronOffset0", pvr::GpuDatatypes::Integer);
	desc.addElement("neuronOffset1", pvr::GpuDatatypes::Integer);
	desc.addElement("neuronOffset2", pvr::GpuDatatypes::Integer);
	desc.addElement("neuronOffset3", pvr::GpuDatatypes::Integer);
	desc.addElement("neuronOffset4", pvr::GpuDatatypes::Integer);
	desc.addElement("layerCount", pvr::GpuDatatypes::Integer);
	desc.addElement("nnWeightsNumberElement", pvr::GpuDatatypes::Integer);
	desc.addElement("nnBiasesNumberElement", pvr::GpuDatatypes::Integer);
	
	_deviceResources->computeSettingsBufferView.initDynamic(desc, _swapchainLength, pvr::BufferUsageFlags::UniformBuffer,
		static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

	_deviceResources->computeSettingsBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->computeSettingsBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->computeSettingsBuffer->setObjectName("NNSettingsUBO");

	_deviceResources->computeSettingsBufferView.pointToMappedMemory(_deviceResources->computeSettingsBuffer->getDeviceMemory()->getMappedData());

	glm::vec3 cameraPos = _camera.getCameraPosition();
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->computeSettingsBufferView.getElementByName("eyePos", 0, i).setValue(glm::vec4(cameraPos, 0.0f));
		_deviceResources->computeSettingsBufferView.getElementByName("patchSide", 0, i).setValue(_patchSide);
		_deviceResources->computeSettingsBufferView.getElementByName("textureWidth", 0, i).setValue(_textureWidth);
		_deviceResources->computeSettingsBufferView.getElementByName("textureHeight", 0, i).setValue(_textureHeight);
		_deviceResources->computeSettingsBufferView.getElementByName("screenSpaceBoxNumberPixelPerThread", 0, i).setValue(_screenSpaceBoxNumberPixelPerThread);
		_deviceResources->computeSettingsBufferView.getElementByName("textureNumberPatchXDimension", 0, i).setValue(_textureNumberPatchXDimension);
		_deviceResources->computeSettingsBufferView.getElementByName("textureNumberPatchYDimension", 0, i).setValue(_textureNumberPatchYDimension);
		_deviceResources->computeSettingsBufferView.getElementByName("screenWidthScreenSpaceBox", 0, i).setValue(_screenWidthScreenSpaceBox);
		_deviceResources->computeSettingsBufferView.getElementByName("screenHeightScreenSpaceBox", 0, i).setValue(_screenHeightScreenSpaceBox);
		_deviceResources->computeSettingsBufferView.getElementByName("screenWidth", 0, i).setValue(_screenWidth);
		_deviceResources->computeSettingsBufferView.getElementByName("screenHeight", 0, i).setValue(_screenHeight);
		_deviceResources->computeSettingsBufferView.getElementByName("organizePatchWorkloadNumberPixelScreenSpaceRegion", 0, i).setValue(_organizePatchWorkloadNumberPixelScreenSpaceRegion);
		_deviceResources->computeSettingsBufferView.getElementByName("neuronsPerLayer0", 0, i).setValue(nnConfiguration.vectorNeuronsPerLayer[0]);
		_deviceResources->computeSettingsBufferView.getElementByName("neuronsPerLayer1", 0, i).setValue(nnConfiguration.vectorNeuronsPerLayer[1]);
		_deviceResources->computeSettingsBufferView.getElementByName("neuronsPerLayer2", 0, i).setValue(nnConfiguration.vectorNeuronsPerLayer[2]);
		_deviceResources->computeSettingsBufferView.getElementByName("neuronsPerLayer3", 0, i).setValue(nnConfiguration.vectorNeuronsPerLayer[3]);
		_deviceResources->computeSettingsBufferView.getElementByName("neuronsPerLayer4", 0, i).setValue(nnConfiguration.vectorNeuronsPerLayer[4]);
		_deviceResources->computeSettingsBufferView.getElementByName("connectionOffset0", 0, i).setValue(nnConfiguration.vectorConnectionOffsetsPerLayer[0]);
		_deviceResources->computeSettingsBufferView.getElementByName("connectionOffset1", 0, i).setValue(nnConfiguration.vectorConnectionOffsetsPerLayer[1]);
		_deviceResources->computeSettingsBufferView.getElementByName("connectionOffset2", 0, i).setValue(nnConfiguration.vectorConnectionOffsetsPerLayer[2]);
		_deviceResources->computeSettingsBufferView.getElementByName("connectionOffset3", 0, i).setValue(nnConfiguration.vectorConnectionOffsetsPerLayer[3]);
		_deviceResources->computeSettingsBufferView.getElementByName("connectionOffset4", 0, i).setValue(nnConfiguration.vectorConnectionOffsetsPerLayer[4]);
		_deviceResources->computeSettingsBufferView.getElementByName("neuronOffset0", 0, i).setValue(nnConfiguration.vectorNeuronOffsets[0]);
		_deviceResources->computeSettingsBufferView.getElementByName("neuronOffset1", 0, i).setValue(nnConfiguration.vectorNeuronOffsets[1]);
		_deviceResources->computeSettingsBufferView.getElementByName("neuronOffset2", 0, i).setValue(nnConfiguration.vectorNeuronOffsets[2]);
		_deviceResources->computeSettingsBufferView.getElementByName("neuronOffset3", 0, i).setValue(nnConfiguration.vectorNeuronOffsets[3]);
		_deviceResources->computeSettingsBufferView.getElementByName("neuronOffset4", 0, i).setValue(nnConfiguration.vectorNeuronOffsets[4]);
		_deviceResources->computeSettingsBufferView.getElementByName("layerCount", 0, i).setValue(nnConfiguration.layerCount);
		_deviceResources->computeSettingsBufferView.getElementByName("nnWeightsNumberElement", 0, i).setValue(nnConfiguration.nnWeightsNumberElement);
		_deviceResources->computeSettingsBufferView.getElementByName("nnBiasesNumberElement", 0, i).setValue(nnConfiguration.nnBiasesNumberElement);

		// if the memory property flags used by the buffers' device memory does not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(_deviceResources->computeSettingsBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_deviceResources->computeSettingsBuffer->getDeviceMemory()->flushRange(
				_deviceResources->computeSettingsBufferView.getDynamicSliceOffset(i), _deviceResources->computeSettingsBufferView.getDynamicSliceSize());
		}
	}
}

/// <summary>Called from VulkanNeuralNetworkEnvironment::renderFrame() each frame to update per-frame information in various uniform buffers.</summary>
void VulkanNeuralNetworkEnvironment::updateGraphicsUniformBuffers(int swapchainIndex)
{
	static float emissiveScale = 0.0f;
	static float emissiveStrength = 1.;

	_deviceResources->uboWorld.view.getElement(0, 0).setValue(glm::eulerAngleXY(glm::radians(0.f), glm::radians(120.f)) * glm::scale(glm::vec3(22.0f)));

	if ((_deviceResources->uboWorld.buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->uboWorld.buffer->getDeviceMemory()->flushRange();
	}

	emissiveStrength += .15f;
	if (emissiveStrength >= glm::pi<float>()) { emissiveStrength = 0.0f; }

	emissiveScale = std::abs(glm::cos(emissiveStrength)) + .75f;

	glm::mat4 inverseViewMatrix = glm::inverse(_viewMtx);
	glm::mat4 inverseProjectionMatrix = glm::inverse(_projMtx);
	glm::mat4 inverseViewProjectionMatrix = glm::inverse(_projMtx * _viewMtx);
	glm::vec3 cameraPos = _camera.getCameraPosition();

	// update the matrix uniform buffer
	{
		// only update the current swapchain ubo
		const glm::mat4 tempMtx = _projMtx * _viewMtx;
		_deviceResources->uboPerFrame.view.getElement(0, 0, swapchainIndex).setValue(tempMtx); // view proj
		_deviceResources->uboPerFrame.view.getElement(1, 0, swapchainIndex).setValue(cameraPos); // camera position.
		_deviceResources->uboPerFrame.view.getElement(2, 0, swapchainIndex).setValue(emissiveScale);
		_deviceResources->uboPerFrame.view.getElement(3, 0, swapchainIndex).setValue(exposure);

		// flush if the buffer memory doesn't support host coherent.
		if (uint32_t(_deviceResources->uboPerFrame.buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_deviceResources->uboPerFrame.buffer->getDeviceMemory()->flushRange(
				_deviceResources->uboPerFrame.view.getDynamicSliceOffset(swapchainIndex), _deviceResources->uboPerFrame.view.getDynamicSliceSize());
		}
	}
}

/// <summary>Update the compute uniform buffer used in all compute passes in the sample.</summary>
/// <param name="swapchainIndex">Swapchain index.</param>
void VulkanNeuralNetworkEnvironment::updateComputeUniformBuffer(uint32_t swapchainIndex)
{
	glm::mat4 inverseViewMatrix = glm::inverse(_viewMtx);
	glm::mat4 inverseProjectionMatrix = glm::inverse(_projMtx);

	_deviceResources->computeSettingsBufferView.getElementByName("inverseViewMatrix", 0, swapchainIndex).setValue(inverseViewMatrix);
	_deviceResources->computeSettingsBufferView.getElementByName("inverseProjectionMatrix", 0, swapchainIndex).setValue(inverseProjectionMatrix);
	_deviceResources->computeSettingsBufferView.getElementByName("viewMatrix", 0, swapchainIndex).setValue(_viewMtx);
	_deviceResources->computeSettingsBufferView.getElementByName("projectionMatrix", 0, swapchainIndex).setValue(_projMtx);
	_deviceResources->computeSettingsBufferView.getElementByName("exposure", 0, swapchainIndex).setValue(exposure);

	// if the memory property flags used by the buffers' device memory does not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->computeSettingsBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->computeSettingsBuffer->getDeviceMemory()->flushRange(
			_deviceResources->computeSettingsBufferView.getDynamicSliceOffset(swapchainIndex), _deviceResources->computeSettingsBufferView.getDynamicSliceSize());
	}
}

/// <summary>Update the technique used to draw the environment.</summary>
void VulkanNeuralNetworkEnvironment::updateEnvironmentTechnique()
{
	// Change current method to draw the eenvironment if current one has been for longer than environmentDrawTechniqueShowTime seconds
	_currentModeRemainingTime -= getFrameTime() * 0.001f;
	if (_currentModeRemainingTime <= 0.0f)
	{
		_currentModeRemainingTime = environmentDrawTechniqueShowTime;
		int enumCast = static_cast<int>(_currentEnvironmentTechnique);
		enumCast += 1;

		if (enumCast >= static_cast<int>(CurrentEnvironmentTechnique::CET_SIZE)) { enumCast = 0; }

		_currentEnvironmentTechnique = static_cast<CurrentEnvironmentTechnique>(enumCast);

		if (_currentEnvironmentTechnique != CurrentEnvironmentTechnique::CET_RASTERIZATION)
		{
			std::string uiMessage = "Environment: Compute ";
			if (_currentEnvironmentTechnique == CurrentEnvironmentTechnique::CET_NEURAL_NETWORK_COMPUTE) { uiMessage += "non-cooperative"; }
			else { uiMessage += "cooperative matrix"; }
			_deviceResources->uiRendererCompute.getDefaultTitle()->setText(uiMessage.c_str()).commitUpdates();
			_deviceResources->uiRendererCompute.getDefaultControls()->setText("Action: Pause");
			_deviceResources->uiRendererCompute.getDefaultControls()->commitUpdates();
		}
	}
}

/// <summary>Build the buffer where to store the screen-space box information on what environment textures are visible from camera.</summary>
void VulkanNeuralNetworkEnvironment::buildComputeScreenSpaceBoxBuffer()
{
	uint32_t bufferSize = _textureNumPatches * sizeof(uint32_t) * 4;
	uint32_t debugBufferSize = _screenSpaceBoxXWorkgroupNumber * sizeof(uint32_t) * 50 * 64;
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->vectorComputeScreenSpaceBoxBuffer[i] = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(static_cast<VkDeviceSize>(bufferSize), pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE);
		_deviceResources->vectorComputeScreenSpaceBoxBuffer[i]->setObjectName("computeScreenSpaceBoxBufferSB_Swapchain_" + std::to_string(i));

		_deviceResources->vectorComputeScreenSpaceBoxDebugBuffer[i] =
			pvr::utils::createBuffer(_deviceResources->device, pvrvk::BufferCreateInfo(static_cast<VkDeviceSize>(debugBufferSize), pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT),
				pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE);
		_deviceResources->vectorComputeScreenSpaceBoxDebugBuffer[i]->setObjectName("computeScreenSpaceBoxDebugBufferSB_Swapchain_" + std::to_string(i));
	}
}

/// <summary>Build the buffer where the screen-space boxes with the information on what environment textures are visible from camera
/// are split into regions so later compute subgroups can generate the pixels from that region.</summary>
void VulkanNeuralNetworkEnvironment::buildComputeOrganizePatchWorkloadBuffer()
{
	uint32_t organizePatchWorkloadBufferBufferSize = _textureNumPatches * sizeof(uint32_t) * maxNumberScreenSpaceBoxSlots * numberElementsPerScreenSpaceBoxSlot;
	uint32_t atomicBufferSize = 3 * sizeof(uint32_t); // This buffer will be used for indirect dispatch so it has sàce for three integers
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->vectorComputeOrganizePatchWorkloadBuffer[i] = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(
				static_cast<VkDeviceSize>(organizePatchWorkloadBufferBufferSize), pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE);
		_deviceResources->vectorComputeOrganizePatchWorkloadBuffer[i]->setObjectName("computeOrganizePatchWorkloadSB_Swapchain_" + std::to_string(i));

		_deviceResources->vectorComputeOrganizePatchWorkloadDebugBuffer[i] = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(static_cast<VkDeviceSize>(_textureNumPatches * sizeof(uint32_t) * 100),
				pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE);
		_deviceResources->vectorComputeOrganizePatchWorkloadDebugBuffer[i]->setObjectName("computeOrganizePatchWorkloadDebugSB_Swapchain_" + std::to_string(i));

		_deviceResources->vectorComputeAtomicCounterBuffer[i] = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(static_cast<VkDeviceSize>(atomicBufferSize),
				pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT | pvrvk::BufferUsageFlags ::e_INDIRECT_BUFFER_BIT),
				pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE);
		_deviceResources->vectorComputeAtomicCounterBuffer[i]->setObjectName("computeAtomicCounterBufferSB_Swapchain_" + std::to_string(i));

		_deviceResources->vectorNNEnvironmentDebugBuffer[i] = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(static_cast<VkDeviceSize>(1024 * 1024 * 4),
				pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE);
		_deviceResources->vectorComputeOrganizePatchWorkloadDebugBuffer[i]->setObjectName("vectorNNEnvironmentDebugBufferSB_Swapchain_" + std::to_string(i));
	}
}

/// <summary>Build the descriptor set layouts used in the different compute passes.</summary>
void VulkanNeuralNetworkEnvironment::buildComputeDescriptorSetLayout()
{
	{
		pvrvk::DescriptorSetLayoutCreateInfo layoutCreateInfo;
		layoutCreateInfo.setBinding(0, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		layoutCreateInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		layoutCreateInfo.setBinding(2, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		layoutCreateInfo.setBinding(3, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		_deviceResources->computeScreenSpaceBoxDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(layoutCreateInfo);
	}

	{
		pvrvk::DescriptorSetLayoutCreateInfo layoutCreateInfo;
		layoutCreateInfo.setBinding(0, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		layoutCreateInfo.setBinding(1, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		layoutCreateInfo.setBinding(2, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		layoutCreateInfo.setBinding(3, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		layoutCreateInfo.setBinding(4, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		_deviceResources->computeOrganizePatchWorkloadDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(layoutCreateInfo);
	}

	{
		pvrvk::DescriptorSetLayoutCreateInfo layoutCreateInfo;
		layoutCreateInfo.setBinding(0, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		layoutCreateInfo.setBinding(1, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		layoutCreateInfo.setBinding(2, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		layoutCreateInfo.setBinding(3, pvrvk::DescriptorType::e_STORAGE_IMAGE, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		layoutCreateInfo.setBinding(4, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		layoutCreateInfo.setBinding(5, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		_deviceResources->computeNeuralNetworkEnvironmentDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(layoutCreateInfo);
	}
}

/// <summary>Update the descritor set used in the compute pass where screen-space box information 
/// on what environment textures are visible from camera is built.</summary>
void VulkanNeuralNetworkEnvironment::updateComputeScreenSpaceBoxDescriptorSet()
{
	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

	_deviceResources->vectorComputeScreenSpaceBoxDescriptorSet.resize(_swapchainLength);

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->vectorComputeScreenSpaceBoxDescriptorSet[i] =
			_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->computeScreenSpaceBoxDescriptorSetLayout);
		_deviceResources->vectorComputeScreenSpaceBoxDescriptorSet[i]->setObjectName("ComputePatchesSpwachain" + std::to_string(i) + "DescriptorSet");

		// Binding 0: vectorComputeScreenSpaceBoxBuffer
		pvrvk::DescriptorBufferInfo descriptorBufferInfoNNBiasesBuffer =
			pvrvk::DescriptorBufferInfo(_deviceResources->vectorComputeScreenSpaceBoxBuffer[i], 0, _deviceResources->vectorComputeScreenSpaceBoxBuffer[i]->getSize());
		pvrvk::WriteDescriptorSet writeDescriptorSetNNBiasesBuffer =
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->vectorComputeScreenSpaceBoxDescriptorSet[i], 0);
		writeDescriptorSetNNBiasesBuffer.setBufferInfo(0, descriptorBufferInfoNNBiasesBuffer);

		writeDescSets.push_back(writeDescriptorSetNNBiasesBuffer);

		// Binding 1: offscreenColorAttachmentMaskImageView
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->vectorComputeScreenSpaceBoxDescriptorSet[i], 1)
				.setImageInfo(0,
					pvrvk::DescriptorImageInfo(
						_deviceResources->offscreenColorAttachmentMaskImageView[i], _deviceResources->samplerBilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		// Binding 2: computeSettingsBuffer
		pvrvk::DescriptorBufferInfo descriptorBufferInfoUniform =
			pvrvk::DescriptorBufferInfo(_deviceResources->computeSettingsBuffer, 0, _deviceResources->computeSettingsBufferView.getDynamicSliceSize());
		pvrvk::WriteDescriptorSet writeDescriptorSetUniform =
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->vectorComputeScreenSpaceBoxDescriptorSet[i], 2);
		writeDescriptorSetUniform.setBufferInfo(0, descriptorBufferInfoUniform);
		writeDescSets.push_back(writeDescriptorSetUniform);

		// Binding 3: vectorComputeScreenSpaceBoxDebugBuffer
		pvrvk::DescriptorBufferInfo descriptorComputeScreenSpaceBoxDebugBuffer =
			pvrvk::DescriptorBufferInfo(_deviceResources->vectorComputeScreenSpaceBoxDebugBuffer[i], 0, _deviceResources->vectorComputeScreenSpaceBoxDebugBuffer[i]->getSize());
		pvrvk::WriteDescriptorSet writeComputeScreenSpaceBoxDebugBuffer =
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->vectorComputeScreenSpaceBoxDescriptorSet[i], 3);
		writeComputeScreenSpaceBoxDebugBuffer.setBufferInfo(0, descriptorComputeScreenSpaceBoxDebugBuffer);
		writeDescSets.push_back(writeComputeScreenSpaceBoxDebugBuffer);
	}

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

/// <summary>Update the descriptor sets of the compute pass where the screen-space boxes with the information on what environment textures are 
/// visible from camera are split into regions so later compute subgroups can generate the pixels from that region.</summary>
void VulkanNeuralNetworkEnvironment::updateComputeOrganizePatchWorkloadDescriptorSet()
{
	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

	_deviceResources->vectorComputeOrganizePatchWorkloadDescriptorSet.resize(_swapchainLength);

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->vectorComputeOrganizePatchWorkloadDescriptorSet[i] =
			_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->computeOrganizePatchWorkloadDescriptorSetLayout);
		_deviceResources->vectorComputeOrganizePatchWorkloadDescriptorSet[i]->setObjectName("ComputeOrganizePatchesSpwachain" + std::to_string(i) + "DescriptorSet");

		// Binding 0: vectorComputeScreenSpaceBoxBuffer
		pvrvk::DescriptorBufferInfo descriptorBufferInfoComputePatchesBuffer =
			pvrvk::DescriptorBufferInfo(_deviceResources->vectorComputeScreenSpaceBoxBuffer[i], 0, _deviceResources->vectorComputeScreenSpaceBoxBuffer[i]->getSize());
		pvrvk::WriteDescriptorSet writeDescriptorSetComputePatchesBuffer =
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->vectorComputeOrganizePatchWorkloadDescriptorSet[i], 0);
		writeDescriptorSetComputePatchesBuffer.setBufferInfo(0, descriptorBufferInfoComputePatchesBuffer);
		writeDescSets.push_back(writeDescriptorSetComputePatchesBuffer);

		// Binding 1: vectorComputeOrganizePatchWorkloadBuffer
		pvrvk::DescriptorBufferInfo descriptorBufferInfoComputeOrganizePatchWorkloadBuffer =
			pvrvk::DescriptorBufferInfo(_deviceResources->vectorComputeOrganizePatchWorkloadBuffer[i], 0, _deviceResources->vectorComputeOrganizePatchWorkloadBuffer[i]->getSize());
		pvrvk::WriteDescriptorSet writeDescriptorSetComputeOrganizePatchWorkloadBuffer =
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->vectorComputeOrganizePatchWorkloadDescriptorSet[i], 1);
		writeDescriptorSetComputeOrganizePatchWorkloadBuffer.setBufferInfo(0, descriptorBufferInfoComputeOrganizePatchWorkloadBuffer);
		writeDescSets.push_back(writeDescriptorSetComputeOrganizePatchWorkloadBuffer);

		// Binding 2: vectorComputeAtomicCounterBuffer
		pvrvk::DescriptorBufferInfo descriptorBufferInfoComputeAtomicCounterBuffer =
			pvrvk::DescriptorBufferInfo(_deviceResources->vectorComputeAtomicCounterBuffer[i], 0, _deviceResources->vectorComputeAtomicCounterBuffer[i]->getSize());
		pvrvk::WriteDescriptorSet writeDescriptorSetComputeAtomicCounterBuffer =
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->vectorComputeOrganizePatchWorkloadDescriptorSet[i], 2);
		writeDescriptorSetComputeAtomicCounterBuffer.setBufferInfo(0, descriptorBufferInfoComputeAtomicCounterBuffer);
		writeDescSets.push_back(writeDescriptorSetComputeAtomicCounterBuffer);

		// Binding 3: computeSettingsBuffer
		pvrvk::DescriptorBufferInfo descriptorBufferInfoUniform =
			pvrvk::DescriptorBufferInfo(_deviceResources->computeSettingsBuffer, 0, _deviceResources->computeSettingsBufferView.getDynamicSliceSize());
		pvrvk::WriteDescriptorSet writeDescriptorSetUniform =
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->vectorComputeOrganizePatchWorkloadDescriptorSet[i], 3);
		writeDescriptorSetUniform.setBufferInfo(0, descriptorBufferInfoUniform);
		writeDescSets.push_back(writeDescriptorSetUniform);

		// Binding 4: vectorComputeOrganizePatchWorkloadDebugBuffer
		pvrvk::DescriptorBufferInfo descriptorBufferInfoDebug = pvrvk::DescriptorBufferInfo(
			_deviceResources->vectorComputeOrganizePatchWorkloadDebugBuffer[i], 0, _deviceResources->vectorComputeOrganizePatchWorkloadDebugBuffer[i]->getSize());
		pvrvk::WriteDescriptorSet writeDescriptorSetDebug =
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->vectorComputeOrganizePatchWorkloadDescriptorSet[i], 4);
		writeDescriptorSetDebug.setBufferInfo(0, descriptorBufferInfoDebug);
		writeDescSets.push_back(writeDescriptorSetDebug);
	}

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

/// <summary>Update the descriptor sets of the compute pass where each subroup takes a screen-space region to infer pixels it contains.</summary>
void VulkanNeuralNetworkEnvironment::updateVectorNeuralNetworkEnvironmentDescriptorSet()
{
	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

	_deviceResources->vectorNeuralNetworkEnvironmentDescriptorSet.resize(_swapchainLength);

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->vectorNeuralNetworkEnvironmentDescriptorSet[i] =
			_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->computeNeuralNetworkEnvironmentDescriptorSetLayout);
		_deviceResources->vectorNeuralNetworkEnvironmentDescriptorSet[i]->setObjectName("ComputeNeuralNetworkEnvironment" + std::to_string(i) + "DescriptorSet");

		// Binding 0: vectorComputeOrganizePatchWorkloadBuffer
		pvrvk::DescriptorBufferInfo descriptor1 =
			pvrvk::DescriptorBufferInfo(_deviceResources->vectorComputeOrganizePatchWorkloadBuffer[i], 0, _deviceResources->vectorComputeOrganizePatchWorkloadBuffer[i]->getSize());
		pvrvk::WriteDescriptorSet writeDescriptorSet1 =
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->vectorNeuralNetworkEnvironmentDescriptorSet[i], 0);
		writeDescriptorSet1.setBufferInfo(0, descriptor1);
		writeDescSets.push_back(writeDescriptorSet1);

		// Binding 1: neuralNetworkEnvironmentBuffer
		pvrvk::DescriptorBufferInfo descriptor2 =
			pvrvk::DescriptorBufferInfo(_deviceResources->neuralNetworkEnvironmentBuffer, 0, _deviceResources->neuralNetworkEnvironmentBuffer->getSize());
		pvrvk::WriteDescriptorSet writeDescriptorSet2 =
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->vectorNeuralNetworkEnvironmentDescriptorSet[i], 1);
		writeDescriptorSet2.setBufferInfo(0, descriptor2);
		writeDescSets.push_back(writeDescriptorSet2);

		// Binding 2: computeSettingsBuffer
		pvrvk::DescriptorBufferInfo descriptorBufferInfoUniform =
			pvrvk::DescriptorBufferInfo(_deviceResources->computeSettingsBuffer, 0, _deviceResources->computeSettingsBufferView.getDynamicSliceSize());
		pvrvk::WriteDescriptorSet writeDescriptorSetUniform =
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->vectorNeuralNetworkEnvironmentDescriptorSet[i], 2);
		writeDescriptorSetUniform.setBufferInfo(0, descriptorBufferInfoUniform);
		writeDescSets.push_back(writeDescriptorSetUniform);

		// Binding 3: Swapchain image
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_IMAGE, _deviceResources->vectorNeuralNetworkEnvironmentDescriptorSet[i], 3)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->swapchain->getImageView(i), pvrvk::ImageLayout::e_GENERAL)));

		// Binding 4: vectorNNEnvironmentDebugBuffer
		pvrvk::DescriptorBufferInfo descriptor3 =
			pvrvk::DescriptorBufferInfo(_deviceResources->vectorNNEnvironmentDebugBuffer[i], 0, _deviceResources->vectorNNEnvironmentDebugBuffer[i]->getSize());
		pvrvk::WriteDescriptorSet writeDescriptorSet3 =
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->vectorNeuralNetworkEnvironmentDescriptorSet[i], 4);
		writeDescriptorSet3.setBufferInfo(0, descriptor3);
		writeDescSets.push_back(writeDescriptorSet3);

		// Binding 5: offscreenColorAttachmentMaskImageView
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->vectorNeuralNetworkEnvironmentDescriptorSet[i], 5)
				.setImageInfo(0,
					pvrvk::DescriptorImageInfo(
						_deviceResources->offscreenColorAttachmentMaskImageView[i], _deviceResources->samplerBilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));
	}

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

/// <summary>Build the compute pippeline used in the compute pass where screen-space box information 
/// on what environment textures are visible from camera is built.</summary>
void VulkanNeuralNetworkEnvironment::buildScreenSpaceBoxComputePipeline()
{
	{
		pvrvk::PipelineLayoutCreateInfo createInfo;
		createInfo.addDescSetLayout(_deviceResources->computeScreenSpaceBoxDescriptorSetLayout);
		_deviceResources->computeScreenSpaceBoxPipelinelayout = _deviceResources->device->createPipelineLayout(createInfo);
	}

	{
		std::string shaderSource;
		getAssetStream(ComputeShaderScreenSpaceBoxSrcFile)->readIntoString(shaderSource);

		// Add the workgroup local thread dimensions
		std::string replaceString = "layout(local_size_x = " + std::to_string(_screenSpaceBoxWorkgroupSize) + ", local_size_y = 1, local_size_z = 1) in;\n";
		shaderSource = std::regex_replace(shaderSource, std::regex("\\%s1"), replaceString);	

		pvrvk::ShaderModule computeShaderModule = pvr::utils::createShaderModule(_deviceResources->device, shaderSource, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		pvrvk::ComputePipelineCreateInfo createInfo;
		createInfo.computeShader.setShader(computeShaderModule);
		createInfo.pipelineLayout = _deviceResources->computeScreenSpaceBoxPipelinelayout;
		_deviceResources->computeScreenSpaceBoxComputePipeline = _deviceResources->device->createComputePipeline(createInfo, _deviceResources->pipelineCache);
		_deviceResources->computeScreenSpaceBoxComputePipeline->setObjectName("computePatchesComputePipeline");
	}
}

/// <summary>Build the compute pippeline used in the compute pass where the screen-space boxes with the information on what environment textures are 
/// visible from camera are split into regions so later compute subgroups can generate the pixels from that region.</summary>
void VulkanNeuralNetworkEnvironment::buildOrganizePatchWorkloadComputePipeline()
{
	{
		pvrvk::PipelineLayoutCreateInfo createInfo;
		createInfo.addDescSetLayout(_deviceResources->computeOrganizePatchWorkloadDescriptorSetLayout);
		_deviceResources->computeOrganizePatchWorkloadPipelinelayout = _deviceResources->device->createPipelineLayout(createInfo);
	}

	{
		std::string shaderSource;
		getAssetStream(ComputeShaderOrganizePatchWorkload)->readIntoString(shaderSource);

		// Add the workgroup local thread dimensions
		std::string replaceString = "layout(local_size_x = " + std::to_string(_subgroupSize) + ", local_size_y = 1, local_size_z = 1) in;\n";
		shaderSource = std::regex_replace(shaderSource, std::regex("\\%s1"), replaceString);

		pvrvk::ShaderModule computeShaderModule = pvr::utils::createShaderModule(_deviceResources->device, shaderSource, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		pvrvk::ComputePipelineCreateInfo createInfo;
		createInfo.computeShader.setShader(computeShaderModule);
		createInfo.pipelineLayout = _deviceResources->computeOrganizePatchWorkloadPipelinelayout;
		_deviceResources->computeOrganizePatchWorkloadComputePipeline = _deviceResources->device->createComputePipeline(createInfo, _deviceResources->pipelineCache);
		_deviceResources->computeOrganizePatchWorkloadComputePipeline->setObjectName("organizePatchWorkloadComputePipeline");
	}
}

/// <summary>Build the compute pipeline for the compute pass where pixels from each screen-space region are generated (each thread will generate
/// at least one pixel).</summary>
void VulkanNeuralNetworkEnvironment::buildNeuralNetworkEnvironmentInferenceComputePipeline()
{
	{
		pvrvk::PipelineLayoutCreateInfo createInfo;
		createInfo.addDescSetLayout(_deviceResources->computeNeuralNetworkEnvironmentDescriptorSetLayout);
		_deviceResources->computeNeuralNetworkEnvironmentPipelinelayout = _deviceResources->device->createPipelineLayout(createInfo);
	}

	{
		std::string shaderSource;
		getAssetStream(ComputeShaderNeuralNetworkEnvironment)->readIntoString(shaderSource);

		// Add two shared variable arrays which can hold the neural network information
		std::string replaceString = "shared float sharedArrayNNBiases[" + std::to_string(nnConfiguration.nnBiasesNumberElement) + "];";
		shaderSource = std::regex_replace(shaderSource, std::regex("\\%s0"), replaceString);

		replaceString = "shared float sharedArrayNNWeights[" + std::to_string(nnConfiguration.nnWeightsNumberElement) + "];";
		shaderSource = std::regex_replace(shaderSource, std::regex("\\%s1"), replaceString);

		// Add the workgroup local thread dimensions
		replaceString = "layout(local_size_x = " + std::to_string(_subgroupSize) + ", local_size_y = 1, local_size_z = 1) in;\n";
		shaderSource = std::regex_replace(shaderSource, std::regex("\\%s3"), replaceString);

		pvrvk::ShaderModule computeShaderModule = pvr::utils::createShaderModule(_deviceResources->device, shaderSource, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		pvrvk::ComputePipelineCreateInfo createInfo;
		createInfo.computeShader.setShader(computeShaderModule);
		createInfo.pipelineLayout = _deviceResources->computeNeuralNetworkEnvironmentPipelinelayout;
		_deviceResources->computeNeuralNetworkEnvironmentComputePipeline = _deviceResources->device->createComputePipeline(createInfo, _deviceResources->pipelineCache);
		_deviceResources->computeNeuralNetworkEnvironmentComputePipeline->setObjectName("NeuralNetworkEnvironment");
	}
}

/// <summary>Build the compute pipeline for the compute pass where pixels from each screen-space region are generated using 
/// cooperative matrices.</summary>
void VulkanNeuralNetworkEnvironment::buildNeuralNetworkEnvironmentInferenceCooperativeMatrixComputePipeline()
{
	pvrvk::ShaderModule computeShaderModule =
		_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(ComputeShaderNeuralNetworkEnvironmentCooperativeMatrix)->readToEnd<uint32_t>()));

	pvrvk::ComputePipelineCreateInfo createInfo;
	createInfo.computeShader.setShader(computeShaderModule);
	createInfo.pipelineLayout = _deviceResources->computeNeuralNetworkEnvironmentPipelinelayout;
	_deviceResources->computeNeuralNetworkEnvironmentCooperativeMatrixComputePipeline = _deviceResources->device->createComputePipeline(createInfo, _deviceResources->pipelineCache);
	_deviceResources->computeNeuralNetworkEnvironmentCooperativeMatrixComputePipeline->setObjectName("NeuralNetworkEnvironmentCooperativeMatrix");
}

/// <summary>Change the layout of the swapchain images from undefined to shader read only optimal .</summary>
/// <param name="commandBuffer">Command buffer to record the changes against.</param>
void VulkanNeuralNetworkEnvironment::changeSwapchainImageLayout(pvrvk::CommandBuffer commandBuffer)
{
	// Transition the swapchain images from from e_PRESENT_SRC_KHR layout to e_GENERAL
	pvrvk::MemoryBarrierSet2 swapchainBarrier2;

	std::vector<pvrvk::ImageMemoryBarrier2> vectorImageMemoryBarrier2(_swapchainLength);

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		vectorImageMemoryBarrier2[i].setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_NONE_KHR);
		vectorImageMemoryBarrier2[i].setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_COLOR_ATTACHMENT_WRITE_BIT_KHR);
		vectorImageMemoryBarrier2[i].setOldLayout(pvrvk::ImageLayout::e_UNDEFINED);
		vectorImageMemoryBarrier2[i].setNewLayout(pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
		vectorImageMemoryBarrier2[i].setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));
		vectorImageMemoryBarrier2[i].setImage(_deviceResources->offscreenColorAttachmentMaskImage[i]);
		vectorImageMemoryBarrier2[i].setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_NONE_KHR);
		vectorImageMemoryBarrier2[i].setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
		swapchainBarrier2.addBarrier(vectorImageMemoryBarrier2[i]);

		vectorImageMemoryBarrier2[i].setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_NONE_KHR);
		vectorImageMemoryBarrier2[i].setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_NONE_KHR);
		vectorImageMemoryBarrier2[i].setOldLayout(pvrvk::ImageLayout::e_UNDEFINED);
		vectorImageMemoryBarrier2[i].setNewLayout(pvrvk::ImageLayout::e_PRESENT_SRC_KHR);
		vectorImageMemoryBarrier2[i].setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));
		vectorImageMemoryBarrier2[i].setImage(_deviceResources->swapchain->getImage(i));
		vectorImageMemoryBarrier2[i].setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_NONE_KHR);
		vectorImageMemoryBarrier2[i].setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
		swapchainBarrier2.addBarrier(vectorImageMemoryBarrier2[i]);
	}

	commandBuffer->pipelineBarrier2(swapchainBarrier2);
}

/// <summary>Build several uniform buffers used in the sample.</summary>
void VulkanNeuralNetworkEnvironment::createUbos()
{
	// Per frame
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("VPMatrix", pvr::GpuDatatypes::mat4x4);
		desc.addElement("camPos", pvr::GpuDatatypes::vec3);
		desc.addElement("emissiveIntensity", pvr::GpuDatatypes::Float);
		desc.addElement("exposure", pvr::GpuDatatypes::Float);

		_deviceResources->uboPerFrame.view.initDynamic(desc, _swapchainLength, pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

		const pvrvk::DeviceSize size = _deviceResources->uboPerFrame.view.getSize();
		_deviceResources->uboPerFrame.buffer = pvr::utils::createBuffer(_deviceResources->device, pvrvk::BufferCreateInfo(size, pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
			pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			_deviceResources->vmaAllocator);
		_deviceResources->uboPerFrame.buffer->setObjectName("SceneCameraParamsUBO");

		_deviceResources->uboPerFrame.view.pointToMappedMemory(_deviceResources->uboPerFrame.buffer->getDeviceMemory()->getMappedData());
	}

	// World matrix for Helmet
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("modelMatrix", pvr::GpuDatatypes::mat4x4);

		_deviceResources->uboWorld.view.init(desc);

		const pvrvk::DeviceSize size = _deviceResources->uboWorld.view.getSize();
		_deviceResources->uboWorld.buffer = pvr::utils::createBuffer(_deviceResources->device, pvrvk::BufferCreateInfo(size, pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
			pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			_deviceResources->vmaAllocator);
		_deviceResources->uboWorld.buffer->setObjectName("ModelUBO");
		_deviceResources->uboWorld.view.pointToMappedMemory(_deviceResources->uboWorld.buffer->getDeviceMemory()->getMappedData());
	}

	// Ubo lights
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("lightDirection", pvr::GpuDatatypes::vec3);
		desc.addElement("lightColor", pvr::GpuDatatypes::vec3);
		desc.addElement("numSpecularIrrMapMipLevels", pvr::GpuDatatypes::uinteger);

		_deviceResources->uboLights.view.init(desc);
		_deviceResources->uboLights.buffer = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->uboLights.view.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _deviceResources->vmaAllocator);
		_deviceResources->uboLights.buffer->setObjectName("LightsUBO");

		_deviceResources->uboLights.view.pointToMappedMemory(_deviceResources->uboLights.buffer->getDeviceMemory()->getMappedData());

		_deviceResources->uboLights.view.getElement(0).setValue(lightDir);
		_deviceResources->uboLights.view.getElement(1).setValue(glm::vec3(1.f, 1.f, 1.f));
		_deviceResources->uboLights.view.getElement(2).setValue(_deviceResources->skyBoxPass.getNumPrefilteredMipLevels());

		if (uint32_t(_deviceResources->uboLights.buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_deviceResources->uboLights.buffer->getDeviceMemory()->flushRange();
		}
	}

	// ubo material
	{
		const pvr::utils::StructuredMemoryDescription materialDesc("material", 1,
			{
				{ "albedo", pvr::GpuDatatypes::vec3 },
				{ "roughness", pvr::GpuDatatypes::Float },
				{ "metallic", pvr::GpuDatatypes::Float },
			});

		_deviceResources->uboMaterial.view.init(pvr::utils::StructuredMemoryDescription("materials", 1, { materialDesc }));

		_deviceResources->uboMaterial.buffer = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->uboMaterial.view.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, _deviceResources->vmaAllocator);
		_deviceResources->uboMaterial.buffer->setObjectName("MaterialUBO");

		_deviceResources->uboMaterial.view.pointToMappedMemory(_deviceResources->uboMaterial.buffer->getDeviceMemory()->getMappedData());

		// update the material buffer
		pvr::assets::Material& material = _deviceResources->helmetPass.getModel()->getMaterial(0);
		pvr::assets::Material::GLTFMetallicRoughnessSemantics metallicRoughness(material);

		// Helmet material
		auto helmetView = _deviceResources->uboMaterial.view.getElement(0, 0);
		helmetView.getElement(0).setValue(metallicRoughness.getBaseColor());
		helmetView.getElement(1).setValue(metallicRoughness.getRoughness());
		helmetView.getElement(2).setValue(metallicRoughness.getMetallicity());

		if ((_deviceResources->uboMaterial.buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_deviceResources->uboMaterial.buffer->getDeviceMemory()->flushRange();
		}
	}
}

/// <summary>Load the binary blob containing the neural network used to approximate the environment texture.</summary>
/// <param name="commandBuffer">Command buffer to record the upload of the information to a buffer.</param>
void VulkanNeuralNetworkEnvironment::buildComputeNeuralNetworkBuffer(pvrvk::CommandBuffer commandBuffer)
{
	// Structure of the header information in the binary blobs containing the neural network patches to approximate the environment pixels
	//		uint32_t _imageWidth
	//		uint32_t _imageHeight
	//		uint32_t _numberColumnsBake
	//		uint32_t _numberRowsBake
	//		uint32_t _nnBiasesBufferByteSize
	//		uint32_t _nnWeightsBufferByteSize
	//		uint32_t layerCount
	//		uint32_t vectorNeuronsPerLayer
	//		uint32_t vectorNeuronsPerLayer
	//		uint32_t vectorNeuronsPerLayer
	//		uint32_t vectorNeuronsPerLayer
	//		uint32_t vectorNeuronsPerLayer
	//		uint32_t vectorConnectionOffsetsPerLayer
	//		uint32_t vectorConnectionOffsetsPerLayer
	//		uint32_t vectorConnectionOffsetsPerLayer
	//		uint32_t vectorConnectionOffsetsPerLayer
	//		uint32_t vectorConnectionOffsetsPerLayer
	//		uint32_t vectorNeuronOffsets
	//		uint32_t vectorNeuronOffsets
	//		uint32_t vectorNeuronOffsets
	//		uint32_t vectorNeuronOffsets
	//		uint32_t vectorNeuronOffsets

	std::vector<uint8_t> vectorByteData = getAssetStream(neuralNetworkEnvironmentBinaryBlob)->readToEnd<uint8_t>();

	// Read the blob header information
	assertion(vectorByteData.size() >= neuralNetworkHeaderFileByteSize, "The binary blob with the neural network environment information does not have a header");

	nnConfiguration.vectorNeuronsPerLayer.resize(maxNeuralNetworkLayers);
	nnConfiguration.vectorConnectionOffsetsPerLayer.resize(maxNeuralNetworkLayers);
	nnConfiguration.vectorNeuronOffsets.resize(maxNeuralNetworkLayers);

	uint32_t* pHeaderData = reinterpret_cast<uint32_t*>(vectorByteData.data());
	nnConfiguration.imageWidth = pHeaderData[0];
	nnConfiguration.imageHeight = pHeaderData[1];
	nnConfiguration.numberColumnPatches = pHeaderData[2];
	nnConfiguration.numberRowPatches = pHeaderData[3];
	nnConfiguration.nnBiasesBufferSize = pHeaderData[4];
	nnConfiguration.nnWeightsBufferSize = pHeaderData[5];
	nnConfiguration.layerCount = pHeaderData[6];
	nnConfiguration.vectorNeuronsPerLayer[0] = pHeaderData[7];
	nnConfiguration.vectorNeuronsPerLayer[1] = pHeaderData[8];
	nnConfiguration.vectorNeuronsPerLayer[2] = pHeaderData[9];
	nnConfiguration.vectorNeuronsPerLayer[3] = pHeaderData[10];
	nnConfiguration.vectorNeuronsPerLayer[4] = pHeaderData[11];
	nnConfiguration.vectorConnectionOffsetsPerLayer[0] = pHeaderData[12];
	nnConfiguration.vectorConnectionOffsetsPerLayer[1] = pHeaderData[13];
	nnConfiguration.vectorConnectionOffsetsPerLayer[2] = pHeaderData[14];
	nnConfiguration.vectorConnectionOffsetsPerLayer[3] = pHeaderData[15];
	nnConfiguration.vectorConnectionOffsetsPerLayer[4] = pHeaderData[16];
	nnConfiguration.vectorNeuronOffsets[0] = pHeaderData[17];
	nnConfiguration.vectorNeuronOffsets[1] = pHeaderData[18];
	nnConfiguration.vectorNeuronOffsets[2] = pHeaderData[19];
	nnConfiguration.vectorNeuronOffsets[3] = pHeaderData[20];
	nnConfiguration.vectorNeuronOffsets[4] = pHeaderData[21];

	nnConfiguration.nnWeightsNumberElement = nnConfiguration.nnWeightsBufferSize / sizeof(uint32_t);
	nnConfiguration.nnBiasesNumberElement = nnConfiguration.nnBiasesBufferSize / sizeof(uint32_t);

	nnConfiguration.maxNeuronsPerLayer = std::max(nnConfiguration.vectorNeuronsPerLayer[0], nnConfiguration.vectorNeuronsPerLayer[1]);
	nnConfiguration.maxNeuronsPerLayer = std::max(nnConfiguration.maxNeuronsPerLayer, nnConfiguration.vectorNeuronsPerLayer[2]);
	nnConfiguration.maxNeuronsPerLayer = std::max(nnConfiguration.maxNeuronsPerLayer, nnConfiguration.vectorNeuronsPerLayer[3]);
	nnConfiguration.maxNeuronsPerLayer = std::max(nnConfiguration.maxNeuronsPerLayer, nnConfiguration.vectorNeuronsPerLayer[4]);

	vectorByteData.erase(vectorByteData.begin(), vectorByteData.begin() + neuralNetworkHeaderFileByteSize);
	uint32_t bufferSize = static_cast<uint32_t>(vectorByteData.size()) - neuralNetworkHeaderFileByteSize; // Avoid the header information, which is 16 bytes
	
	if (_inferenceFloatPrecission == InferenceFloatPrecission::IFP_16_BIT_FLOAT)
	{
		convertByteArrayFloat32ToFloat16(vectorByteData);
		bufferSize = static_cast<uint32_t>(vectorByteData.size());
	}

	_deviceResources->neuralNetworkEnvironmentBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(static_cast<VkDeviceSize>(bufferSize), pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE);
	_deviceResources->neuralNetworkEnvironmentBuffer->setObjectName("neuralNetworkEnvironmentBuffer");

	pvr::utils::updateBufferUsingStagingBuffer(
		_deviceResources->device, _deviceResources->neuralNetworkEnvironmentBuffer, commandBuffer, vectorByteData.data(), 0, bufferSize, _deviceResources->vmaAllocator);

	_patchSide = nnConfiguration.imageWidth / nnConfiguration.numberColumnPatches;
}

/// <summary>Initalize a buffer used as atomic counter (there is one per swapchain).</summary>
/// <param name="commandBuffer">Command buffer to record the upload of the information to a buffer.</param>
void VulkanNeuralNetworkEnvironment::initializeAtomicCounterBuffer(pvrvk::CommandBuffer commandBuffer)
{
	std::vector<uint32_t> vectorAtomicConter{ 0, 1, 1};
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, _deviceResources->vectorComputeAtomicCounterBuffer[i], commandBuffer, vectorAtomicConter.data(), 0,
			vectorAtomicConter.size() * sizeof(uint32_t), _deviceResources->vmaAllocator);
	}
}

/// <summary>Build descritpor sets for the textures used in the graphics pass for drawing the environment and the scene mesh.</summary>
void VulkanNeuralNetworkEnvironment::updateDescriptors()
{
	// Update the descriptor sets

	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;
	// Dynamic ubo (per frame/object data) : Transformation matrices
	{
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->descSets[0], 0));
		writeDescSets.back().setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->uboPerFrame.buffer, 0, _deviceResources->uboPerFrame.view.getDynamicSliceSize()));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->descSets[0], 1));
		writeDescSets.back().setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->uboWorld.buffer, 0, _deviceResources->uboWorld.view.getSize()));
	}

	// Static ubo (per scene data) : Environment maps etc., BRDF
	{
		// Light
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->descSets[1], 0));
		writeDescSets.back().setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->uboLights.buffer, 0, _deviceResources->uboLights.view.getDynamicSliceSize()));

		// Diffuse Irradiance
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->descSets[1], 1));
		writeDescSets.back().setImageInfo(0,
			pvrvk::DescriptorImageInfo(_deviceResources->skyBoxPass.getDiffuseIrradianceMap(), _deviceResources->samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

		// Specular Irradiance
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->descSets[1], 2));
		writeDescSets.back().setImageInfo(
			0, pvrvk::DescriptorImageInfo(_deviceResources->skyBoxPass.getPrefilteredMap(), _deviceResources->samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

		// Environment map
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->descSets[1], 3));
		writeDescSets.back().setImageInfo(0,
			pvrvk::DescriptorImageInfo(_deviceResources->skyBoxPass.getPrefilteredMipMap(), _deviceResources->samplerTrilinearLodClamped, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

		// BRDF LUT
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->descSets[1], 4));
		writeDescSets.back().setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->brdfLUT, _deviceResources->samplerBilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
	}
	// Per object ubo: Material textures.
	{
		// Albedo Map
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->descSets[2], 0));
		writeDescSets.back().setImageInfo(
			0, pvrvk::DescriptorImageInfo(_deviceResources->helmetPass.getAlbedoMap(), _deviceResources->samplerBilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->descSets[2], 1));
		writeDescSets.back().setImageInfo(0,
			pvrvk::DescriptorImageInfo(
				_deviceResources->helmetPass.getOcclusionMetallicRoughnessMap(), _deviceResources->samplerBilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->descSets[2], 2));
		writeDescSets.back().setImageInfo(
			0, pvrvk::DescriptorImageInfo(_deviceResources->helmetPass.getNormalMap(), _deviceResources->samplerBilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->descSets[2], 3));
		writeDescSets.back().setImageInfo(
			0, pvrvk::DescriptorImageInfo(_deviceResources->helmetPass.getEmissiveMap(), _deviceResources->samplerBilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

		// Materials buffers
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->descSets[2], 4));
		writeDescSets.back().setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->uboMaterial.buffer, 0, _deviceResources->uboMaterial.view.getDynamicSliceSize()));
	}

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanNeuralNetworkEnvironment>(); }
