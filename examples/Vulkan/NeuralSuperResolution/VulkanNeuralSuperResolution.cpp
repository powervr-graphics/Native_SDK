/*!
\brief This example demonstrates how to use the Mentis v2 Neural Super Resolution upscaler to generate a FullHD 1920x1080 image from
\ an input conformed by a color image half its size (940x540), motion vectors image, depth image and the previous, FullHD upscaled image.
\ NOTE: The screen size has to be 1920x1080. Run with command line parameters -width=1920 -height=1080 -fullscreen=1
\file  VulkanNeuralSuperResolution.cpp
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
#include "PVRSuperResolution/SuperResolution.h"
#include "PVRSuperResolution/VulkanComputePostProcessingPass.h"
#include "PVRSuperResolution/DynamicMap.h"

// Content file names

// Models
const char HelmetModelFileName[]{ "damagedHelmet.gltf" };

// Textures
const std::string SkyboxTexFile{ "quarry_r9g9b9e5" };
const char BrdfLUTTexFile[]{ "brdfLUT.pvr" };

/// <summary>Camera rotation speed.</summary>
const float rotationSpeed{ 0.01f };

/// <summary>Camera field of view.</summary>
const float fov{ 65.f };

/// <summary>Light direction.</summary>
const glm::vec3 lightDir{ glm::normalize(glm::vec3(-0.5f, -0.5f, -0.5f)) };

/// <summary>Number of points following a halton sequence of index 2 and base 3 generated (should be a power of 2*3 = 6).</summary>
const int numberHaltonSequenceValues{ 36 };

/// <summary>Array containing the Halton sequence jitter values generated</summary>
glm::vec2 arrayHaltonSequenceJitter[numberHaltonSequenceValues];

/// <summary>Struct to group a uniform buffer object and its associated structured view.</summary>
struct UBO
{
	pvr::utils::StructuredBufferView view;
	pvrvk::Buffer buffer;
};

class SkyBoxMaterial
{
public:
	pvrvk::ImageView getPrefilteredMipMap() const { return skyBoxMap; }
	pvrvk::ImageView getDiffuseIrradianceMap() const { return irradianceMap; }
	pvrvk::ImageView getPrefilteredMap() const { return prefilteredMap; }
	uint32_t getNumPrefilteredMipLevels() const { return numPrefilteredMipLevels; }
	const std::string& getVertexShaderFileName() { return vertexShaderFileName; }
	const std::string& getFragmentShaderFileName() { return fragmentShaderFileName; }

	void setSkyboxImage(pvr::IAssetProvider& assetProvider, pvrvk::Queue queue, pvrvk::CommandPool commandPool, pvr::utils::vma::Allocator& allocator)
	{
		pvrvk::CommandBuffer cmdBuffer = commandPool->allocateCommandBuffer();
		pvrvk::Device device = commandPool->getDevice();

		cmdBuffer->begin();

		skyBoxMap = device->createImageView(pvrvk::ImageViewCreateInfo(pvr::utils::loadAndUploadImage(device, SkyboxTexFile + ".pvr", true, cmdBuffer, assetProvider,
			pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, allocator, allocator)));

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

		irradianceMap = pvr::utils::loadAndUploadImageAndView(device, diffuseMapFilename.c_str(), true, cmdBuffer, assetProvider,
			pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, allocator, allocator);
		prefilteredMap = pvr::utils::loadAndUploadImageAndView(device, prefilteredMapFilename.c_str(), true, cmdBuffer, assetProvider,
			pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, allocator, allocator);

		numPrefilteredMipLevels = prefilteredMap->getImage()->getNumMipLevels();

		cmdBuffer->end();
		queue->submit(&submitInfo, 1);
		queue->waitIdle();
	}
	
private:
	pvrvk::ImageView skyBoxMap;
	pvrvk::ImageView irradianceMap;
	pvrvk::ImageView prefilteredMap;
	uint32_t numPrefilteredMipLevels{ 0 };
	const std::string vertexShaderFileName { "SkyboxVertShader.vsh.spv" };
	const std::string fragmentShaderFileName { "SkyboxFragShader.fsh.spv" };
};

class SkyBoxPass
{
public:
	/// <summary>Initialize Vulkan objects for the Sky box pass.</summary>
	/// <param name="assetProvider">Asset provider for asset loading.</param>
	/// <param name="device">Logical device.</param>
	/// <param name="descPool">Descriptor set pool to allocate descriptor sets from.</param>
	/// <param name="commandPool">Command pool to allocate commands from.</param>
	/// <param name="queue">Queue where to submit command buffers.</param>
	/// <param name="renderpass">Render pass where this sky box pass will be called.</param>
	/// <param name="pipelineCache">Pipeline cache used in in the graphics pipeline.</param>
	/// <param name="viewportDim">Dimensions of the viewport.</param>
	/// <param name="allocator">VMA allocator.</param>
	void init(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::DescriptorPool& descPool, pvrvk::CommandPool& commandPool, pvrvk::Queue& queue,
		const pvrvk::RenderPass& renderpass, const pvrvk::PipelineCache& pipelineCache, const pvrvk::Extent2D& viewportDim,
		pvr::utils::vma::Allocator& allocator)
	{
		buildDescriptorSetLayout(device, descPool);
		createPipeline(assetProvider, device, renderpass, viewportDim, pipelineCache);
		skyBoxMaterial.setSkyboxImage(assetProvider, queue, commandPool, allocator);
	}

	/// <summary>Record commands.</summary>
	/// <param name="device">Logical device.</param>
	/// <param name="descPool">Descriptor set pool to allocate descriptor sets from.</param>
	void buildDescriptorSetLayout(const pvrvk::Device& device, pvrvk::DescriptorPool& descPool)
	{
		// create skybox descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
		descriptorSetLayoutCreateInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descriptorSetLayoutCreateInfo.setBinding(1, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

		descriptorSetLayout = device->createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

		// Build Skybox descriptor set
		descriptorSet = descPool->allocateDescriptorSet(descriptorSetLayout);
		descriptorSet->setObjectName("SkyBoxDescriptorSet");
	}

	/// <summary>Update the descriptor set used in the sky box pass.</summary>
	/// <param name="device">Logical device.</param>
	/// <param name="sampler">Sampler used in the descriptor set.</param>
	/// <param name="buffer">Buffer used in the descriptor set.</param>
	/// <param name="bufferView">Buffer view of the Buffer used in the descriptor set.</param>
	void updateDescriptorSetSkyBoxPass(const pvrvk::Device& device, const pvrvk::Sampler& sampler, const pvrvk::Buffer& buffer, const pvr::utils::StructuredBufferView& bufferView)
	{
		pvrvk::WriteDescriptorSet writeDescSets[2];
		writeDescSets[0]
			.set(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descriptorSet, 0)
			.setImageInfo(0, pvrvk::DescriptorImageInfo(skyBoxMaterial.getPrefilteredMipMap(), sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

		writeDescSets[1].set(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, descriptorSet, 1).setBufferInfo(0, pvrvk::DescriptorBufferInfo(buffer, 0, bufferView.getDynamicSliceSize()));

		device->updateDescriptorSets(writeDescSets, ARRAY_SIZE(writeDescSets), nullptr, 0);
	}

	/// <summary>Record commands.</summary>
	/// <param name="cmdBuffer">recording commandbuffer</param>
	/// <param name="swapchainIndex">swapchain index.</param>
	/// <param name="structuredBufferView">Structured buffer view to store the swapchain index value.</param>
	void recordCommands(pvrvk::CommandBuffer& cmdBuffer, uint32_t swapchainIndex, const pvr::utils::StructuredBufferView& structuredBufferView)
	{
		cmdBuffer->bindPipeline(pipeline);
		uint32_t offset = structuredBufferView.getDynamicSliceOffset(swapchainIndex);
		cmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipeline->getPipelineLayout(), 0, descriptorSet, &offset, 1);

		cmdBuffer->draw(0, 6, 0);
	}

	SkyBoxMaterial& getSkyBoxTexture() { return skyBoxMaterial; }

private:
	/// <summary>Build the graphics pipeline.</summary>
	/// <param name="assetProvider">Asset provider for asset loading.</param>
	/// <param name="device">Logical device.</param>
	/// <param name="renderpass">Render pass where this sky box pass will be called.</param>
	/// <param name="viewportDim">Dimensions of the viewport.</param>
	/// <param name="pipelineCache">Pipeline cache used in in the graphics pipeline.</param>
	void createPipeline(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, const pvrvk::RenderPass& renderpass, const pvrvk::Extent2D& viewportDim,
		const pvrvk::PipelineCache& pipelineCache)
	{
		{
			pvrvk::PipelineLayoutCreateInfo pipelineLayoutInfo;
			pipelineLayoutInfo.setDescSetLayout(0, descriptorSetLayout);
			pipeLayout = device->createPipelineLayout(pipelineLayoutInfo);
		}

		{
			pvrvk::GraphicsPipelineCreateInfo pipeInfo;

			// on screen renderpass
			pipeInfo.renderPass = renderpass;

			pipeInfo.vertexShader.setShader(
				device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(skyBoxMaterial.getVertexShaderFileName())->readToEnd<uint32_t>())));
			pipeInfo.fragmentShader.setShader(
				device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(skyBoxMaterial.getFragmentShaderFileName())->readToEnd<uint32_t>())));

			pipeInfo.pipelineLayout = pipeLayout;

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
	}

	pvrvk::GraphicsPipeline pipeline;
	pvrvk::PipelineLayout pipeLayout;
	pvrvk::DescriptorSetLayout descriptorSetLayout;
	pvrvk::DescriptorSet descriptorSet;
	SkyBoxMaterial skyBoxMaterial;
};

class PBRMaterial
{
public:
	/// <summary>Load textures used by this PBR material.</summary>
	/// <param name="assetProvider">Asset provider for asset loading.</param>
	/// <param name="device">Logical device.</param>
	/// <param name="model">Scene model where to load textures from.</param>
	/// <param name="uploadCmdBuffer">Command buffer to submit to the GPU.</param>
	/// <param name="allocator">VMA allocator.</param>
	void loadTextures(
		pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvr::assets::ModelHandle model, pvrvk::CommandBuffer& uploadCmdBuffer, pvr::utils::vma::Allocator& allocator)
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

	const pvrvk::ImageView& getAlbedoMap() { return images[0]; }
	const pvrvk::ImageView& getOcclusionMetallicRoughnessMap() { return images[1]; }
	const pvrvk::ImageView& getNormalMap() { return images[2]; }
	const pvrvk::ImageView& getEmissiveMap() { return images[3]; }

	const std::string& getVertexShaderFileName() { return vertexShaderFileName; }
	const std::string& getFragmentShaderFileName() { return fragmentShaderFileName; }
	
	void setIsASTCSupported(bool value) { isASTCSupported = value; }

private:
	std::vector<pvrvk::ImageView> images;
	bool isASTCSupported{ false };
	const std::string vertexShaderFileName{ "VertShader.vsh.spv" };
	const std::string fragmentShaderFileName{ "PBRFragShader.fsh.spv" };
};

class HelmetPass
{
public:
	/// <summary>Initialize Vulkan objects for the Sky box pass.</summary>
	/// <param name="assetProvider">Asset provider for asset loading.</param>
	/// <param name="device">Logical device.</param>
	/// <param name="framebuffer">Framebuffer to draw to.</param>
	/// <param name="pipelineLayout">Pipeline layout to be used in the graphics pipeline.</param>
	/// <param name="pipelineCache">Pipeline cache used in in the graphics pipeline.</param>
	/// <param name="allocator">VMA allocator.</param>
	/// <param name="uploadCmdBuffer">Command buffer to submit to the GPU.</param>
	/// <param name="requireSubmission">Whether the commands done require to be submitted.</param>
	/// <param name="astcSupported">Whether ASTC texture compression is supported.</param>
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

		pbrMaterial.setIsASTCSupported(astcSupported);

		// Load the texture
		pbrMaterial.loadTextures(assetProvider, device, model, uploadCmdBuffer, allocator);

		createPipeline(assetProvider, device, framebuffer, pipelineLayout, pipelineCache);
	}

	const pvrvk::GraphicsPipeline& getPipeline() { return pipeline; }

	pvr::assets::ModelHandle& getModel() { return model; }

	PBRMaterial& getPBRMaterial() { return pbrMaterial; }

	/// <summary>Record commands.</summary>
	/// <param name="cmd">recording commandbuffer</param>
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
	/// <summary>Build the graphics pipeline.</summary>
	/// <param name="assetProvider">Asset provider for asset loading.</param>
	/// <param name="device">Logical device.</param>
	/// <param name="framebuffer">Framebuffer to draw to.</param>
	/// <param name="pipelineLayout">Pipeline layout to be used in the graphics pipeline.</param>
	/// <param name="pipelineCache">Pipeline cache used in in the graphics pipeline.</param>
	void createPipeline(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, const pvrvk::Framebuffer& framebuffer, const pvrvk::PipelineLayout& pipelineLayout,
		const pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::GraphicsPipelineCreateInfo pipeDesc;
		pipeDesc.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
		pipeDesc.colorBlend.setAttachmentState(1, pvrvk::PipelineColorBlendAttachmentState());
		pvr::utils::VertexBindings bindingName[] = { { "POSITION", 0 }, { "NORMAL", 1 }, { "UV0", 2 }, { "TANGENT", 3 } };

		pvr::utils::populateViewportStateCreateInfo(framebuffer, pipeDesc.viewport);
		pvr::utils::populateInputAssemblyFromMesh(getModel()->getMesh(0), bindingName, ARRAY_SIZE(bindingName), pipeDesc.vertexInput, pipeDesc.inputAssembler);

		pipeDesc.vertexShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(pbrMaterial.getVertexShaderFileName())->readToEnd<uint32_t>())));
		pipeDesc.fragmentShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(pbrMaterial.getFragmentShaderFileName())->readToEnd<uint32_t>())));

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
	
	std::vector<pvrvk::Buffer> vbos;
	std::vector<pvrvk::Buffer> ibos;
	pvr::assets::ModelHandle model;
	pvrvk::GraphicsPipeline pipeline;
	PBRMaterial pbrMaterial;
};

/// <summary>Implementing the pvr::Shell functions.</summary>
class VulkanNeuralSuperResolution: public pvr::Shell
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

		/// <summary>Fences to wait in the host for the compute command buffers to complete execution.</summary>
		std::vector<pvrvk::Fence> vectorLastSubmitFence;

		/// <summary>Command buffer to draw the environment and scene mesh.</summary>
		std::vector<pvrvk::CommandBuffer> vectorGraphicsCommandBuffers;

		/// <summary>Command buffer to draw the UI after the compute pass (used for the cases where the environment is done with a compute pass).</summary>
		std::vector<pvrvk::CommandBuffer> vectorGraphicsUICommandBuffers;

		/// <summary> Command buffer to draw the environment with compute shaders where each cooperative matrix are used to evaluate pixels.</summary>
		std::vector<pvrvk::CommandBuffer> vectorComputeCooperativeMatrixCommandBuffers;

		/// <summary>Secondary command buffer for the compute pass inferring the environment pixels where cooperative matrices are used to evaluate pixels.</summary>
		std::vector<pvrvk::SecondaryCommandBuffer> vectorNeuralSuperResolutionCooperativeMatrixCommandBuffer;

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

		/// <summary>Render pass drawing the skybox.</summary>
		SkyBoxPass skyBoxPass;

		/// <summary>Render pass drawing the helmet mesh.</summary>
		HelmetPass helmetPass;

		/// <summary>Framebuffer used to draw the scene ofscreen.</summary>
		std::vector<pvrvk::Framebuffer> offscreenFramebuffer;

		/// <summary>Framebuffer used to draw the UI on top of the compute pass results.</summary>
		std::vector<pvrvk::Framebuffer> uiFramebuffer;

		/// <summary>Image used to draw the scene at 1/4 resolution.</summary>
		std::vector<pvrvk::Image> offscreenColorAttachmentImage;

		/// <summary>Image views for the offscreenColorAttachmentMaskImage.</summary>
		std::vector<pvrvk::ImageView> offscreenColorAttachmentImageView;

		/// <summary>Image used to store the motion vectors in the scene offscreen pass.</summary>
		std::vector<pvrvk::Image> offscreenMotionVectorAttachmentImage;

		/// <summary>Image views for the offscreenMotionVectorAttachmentImage.</summary>
		std::vector<pvrvk::ImageView> offscreenMotionVectorAttachmentImageView;

		/// <summary>Depth attachments for the scene pass.</summary>
		std::vector<pvrvk::Image> offscreenDepthAttachmentImage;

		/// <summary>Image views of offscreenDepthAttachmentImage.</summary>
		std::vector<pvrvk::ImageView> offscreenDepthAttachmentImageView;

		/// <summary>Image used to store the previous frame upscaled result.</summary>
		std::vector<pvrvk::Image> previousFrameResultImage;

		/// <summary>Image views for the previousFrameResultImage.</summary>
		std::vector<pvrvk::ImageView> previousFrameResultImageView;

		/// <summary>Render pass used to draw the scene offscreen.</summary>
		pvrvk::RenderPass offScreenGeometryRenderPass;

		/// <summary>Render pass used to draw the UI on top of compute results.</summary>
		pvrvk::RenderPass uiRenderPass;

		/// <summary>Depth images.</summary>
		std::vector<pvrvk::ImageView> depthImages;

		/// <summary>Vector with the descriptor sets for the compute shader generating pixels for the environment after loading a specific patch of the neural 
		/// network that approximates it.</summary>
		std::vector<pvrvk::DescriptorSet> vectorNeuralSuperResolutionDescriptorSet;

		pvr::SuperResolution* superResolutionNSR{ nullptr };

		~DeviceResources()
		{
			if (device) { device->waitIdle(); }
			computeQueue->waitIdle();
			graphicsQueue->waitIdle();

			if (superResolutionNSR != nullptr)
			{
				delete superResolutionNSR;
			}
		}
	};

	std::unique_ptr<DeviceResources> _deviceResources;

	/// <summary>Camera projection matrix.</summary>
	glm::mat4 _projectionMatrix;

	/// <summary>View matrix from current frame.</summary>
	glm::mat4 _viewMatrixCurrentFrame;

	/// <summary>View matrix from previous frame.</summary>
	glm::mat4 _viewMatrixPreviousFrame;

	/// <summary>Iterates from [0, swapchain length - 1].</summary>
	uint32_t _frameId{ 0 };

	/// <summary>Value of _frameId used in the last frame.</summary>
	uint32_t _lastFrameID{ 0 };

	/// <summary>Counts the  number of frames.</summary>
	uint32_t _frameCounter{ 0 };

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

	/// <summary>Width of the texture used to draw the scene offscreen.</summary>
	uint32_t _textureWidth { 0 };

	/// <summary>Height of the texture used to draw the scene offscreen.</summary>
	uint32_t _textureHeight{ 0 };

	/// <summary>Off screen width when drawing the scene offscreen.</summary>
	uint32_t _offscreenWidth{ 0 };

	/// <summary>Off screen height when drawing the scene offscreen.</summary>
	uint32_t _offscreenHeight{ 0 };

	/// <summary>Struct with the cooperative matrix properties used for the compute shader variant inderring nn values this way.</summary>
	std::vector<VkCooperativeMatrixPropertiesKHR> _vectorCooperativeMatrixPropertiesKHR;

	/// <summary> Texture LOD bias applied to the trilinear samplers used to sample textures to draw the scene in an offscreen texture.</summary>
	float _textureLODBias{ -10.0f };

	/// <summary> Offscreen pass color format, has to be compatible with MentisV2NeuralSuperResolution preferences (8-bit sRGB or linear RGB).</summary>
	pvrvk::Format _offscreenColorFormat{ pvrvk::Format::e_R8G8B8A8_UNORM };

	/// <summary> Offscreen pass depth format, has to be compatible with MentisV2NeuralSuperResolution preferences (VK_FORMAT_D32_SFLOAT).</summary>
	pvrvk::Format _offscreenDepthFormat{ pvrvk::Format::e_D32_SFLOAT_S8_UINT };

	/// <summary> Pointer to the dynamic map in SuperResolution::_dynamicMap (not owned by this application, just used to set values required by the MentisV2NeuralSuperResolution algorithm).</summary>
	pvr::DynamicMap* dynamicMap{ nullptr };

	/// <summary>Platform-independent command line argument parser.</summary>
	pvr::CommandLine _cmdLine{};

	/// <summary>Get comand line option to draw the scene at full screen resolution without using the MentisV2NeuralSuperResolution upscaler.</summary>
	bool _useNativeFullScreenRasterization { false };

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void processCommandlineOptions();
	void queryCooperativeMatrixInformation();
	void createDescriptorSetLayouts();
	void initializeSuperResolution();
	void createUbos();
	void updateDescriptors();
	void recordCommandBuffers();
	void recordGraphicsCommandBuffers(std::vector<pvrvk::CommandBuffer>& vectorCommandBuffer);
	void recordGraphicsUICommandBuffers(std::vector<pvrvk::CommandBuffer>& vectorCommandBuffer);
	void recordSecondaryComputeCommandBuffers();
	void recordNeuralSuperResolutionCooperativeMatrixCommandBuffer(uint32_t swapIndex);
	void recordComputeCommandBuffers();
	void createPipelineLayout();
	void updateUniformBuffer(uint32_t swapchainIndex);
	void changeInitialImageLayout(pvrvk::CommandBuffer commandBuffer);
	void createOffScreenGeometryRenderPass();
	void createUIRenderPass();
	float generateHaltonSequence(unsigned int index, int base);
	void buildPixelJitteringValues();
	pvrvk::RenderPass createTechniqueRenderPass(const std::vector<pvrvk::AttachmentDescription>& vectorAttachmentDescription);
	void fillAttachmentDescription(int numColorAttachments, const std::vector<pvrvk::Format>& vectorColorFormat, bool addDepthAttachment,
		pvrvk::SampleCountFlags numSamplesPerPixel, bool keepColorAttachmentContent, std::vector<pvrvk::AttachmentDescription>& vectorAttachmentDescription);

	bool verifyColorFormatSupported();
	bool verifyDepthFormatSupported();
	void createImagesAndFramebuffers();
	void createRenderPass();

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
pvr::Result VulkanNeuralSuperResolution::initApplication()
{
	_frameId = 0;
	_cmdLine = this->getCommandLine();
	setBackBufferColorspace(pvr::ColorSpace::lRGB);
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by Shell once per run, just before exiting the program.
/// quitApplication() will not be called every time the rendering context is lost, only before application exit.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanNeuralSuperResolution::quitApplication() { return pvr::Result::Success; }

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context(e.g.textures, vertex buffers, etc.)</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanNeuralSuperResolution::initView()
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
		VK_NV_COOPERATIVE_MATRIX_EXTENSION_NAME,
		VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME,
		VK_KHR_16BIT_STORAGE_EXTENSION_NAME, 
		VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME,
		VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME,
		VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME
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

	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_DST_BIT)) // Transfer operation for screenshots
	{
		swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_DST_BIT;
	}
	else
	{
		Log(LogLevel::Information, "Error: swapchain images do not support VK_IMAGE_USAGE_TRANSFER_DST_BIT, needed for native full screen rasterization.");
		return pvr::Result::InitializationError;
	}

	// initialise the vma allocator
	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage));

	_deviceResources->swapchain = swapChainCreateOutput.swapchain;

	_swapchainLength = _deviceResources->swapchain->getSwapchainLength();

	_offscreenWidth = getWidth() / 2;
	_offscreenHeight = getHeight() / 2;

	Log(LogLevel::Warning, "\nINFO: To use upscaler, window dimensions should be (1920,1080). Run with command line parameters -width=1920 -height=1080 -fullscreen=1",
		_offscreenWidth, _offscreenHeight);
	Log(LogLevel::Warning, "\nINFO: To use native rasterization instead of upscaler, run with command line parameters -width=1920 -height=1080 -fullscreen=1 -nativeFullScreenRasterization");

	if ((getWidth() != 1920) || (getHeight() != 1080))
	{
		Log(LogLevel::Warning, "\nError: Window dimensions are (%d, %d), they should be (1920,1080). Please run with command line parameters -width=1920 -height=1080 -fullscreen=1", _offscreenWidth, _offscreenHeight);
		return pvr::Result::InitializationError;
	}

	_deviceResources->vectorImageAcquiredSemaphores.resize(_swapchainLength);
	_deviceResources->vectorPresentationSemaphores.resize(_swapchainLength);
	_deviceResources->vectorComputeSemaphores.resize(_swapchainLength);
	_deviceResources->vectorUISemaphores.resize(_swapchainLength);
	_deviceResources->vectorLastSubmitFence.resize(_swapchainLength);
	_deviceResources->vectorGraphicsCommandBuffers.resize(_swapchainLength);
	_deviceResources->vectorGraphicsUICommandBuffers.resize(_swapchainLength);
	_deviceResources->vectorComputeCooperativeMatrixCommandBuffers.resize(_swapchainLength);
	_deviceResources->vectorNeuralSuperResolutionCooperativeMatrixCommandBuffer.resize(_swapchainLength);

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

		_deviceResources->vectorLastSubmitFence[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->vectorLastSubmitFence[i]->setObjectName("LastSubmitFenceSwapchain" + std::to_string(i));

		_deviceResources->vectorGraphicsCommandBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->vectorGraphicsCommandBuffers[i]->setObjectName("GraphicsCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->vectorGraphicsCommandBuffers[i]->setVKSynchronization2IsSupported(true);

		_deviceResources->vectorGraphicsUICommandBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->vectorGraphicsUICommandBuffers[i]->setObjectName("GraphicsUICommandBufferSwapchain" + std::to_string(i));
		_deviceResources->vectorGraphicsUICommandBuffers[i]->setVKSynchronization2IsSupported(true);

		_deviceResources->vectorComputeCooperativeMatrixCommandBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->vectorComputeCooperativeMatrixCommandBuffers[i]->setObjectName("ComputeCooperativeMatrixCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->vectorComputeCooperativeMatrixCommandBuffers[i]->setVKSynchronization2IsSupported(true);

		_deviceResources->vectorNeuralSuperResolutionCooperativeMatrixCommandBuffer[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->vectorNeuralSuperResolutionCooperativeMatrixCommandBuffer[i]->setObjectName("NeuralSuperResolutionCooperativeMatrixSwapchain" + std::to_string(i));
		_deviceResources->vectorNeuralSuperResolutionCooperativeMatrixCommandBuffer[i]->setVKSynchronization2IsSupported(true);
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
	samplerInfo.lodBias = _textureLODBias;
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
	buildPixelJitteringValues();

	if (!verifyColorFormatSupported())
	{
		Log(LogLevel::Error, "Error: Color format for offscreen rendering is not compatible with MentisV2NeuralSuperResolution preferences (8-bit sRGB or linear RGB");
		return pvr::Result::UnknownError;
	}

	if (!verifyDepthFormatSupported())
	{
		Log(LogLevel::Error, "Error: Depth format required by MentisV2NeuralSuperResolution for offscreen rendering is not supported (VK_FORMAT_D32_SFLOAT).");
		return pvr::Result::UnknownError;
	}

	processCommandlineOptions();
	createImagesAndFramebuffers();
	createDescriptorSetLayouts();
	initializeSuperResolution();
	createPipelineLayout();

	// Create Descriptor Sets
	_deviceResources->descSets[0] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->descSetLayouts[0]);
	_deviceResources->descSets[1] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->descSetLayouts[1]);
	_deviceResources->descSets[2] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->descSetLayouts[2]);

	_deviceResources->descSets[0]->setObjectName("DynamicUBODescriptorSet");
	_deviceResources->descSets[1]->setObjectName("StaticUBODescriptorSet");
	_deviceResources->descSets[2]->setObjectName("PerObjectUBODescriptorSet");

	bool requireSubmission = false;

	uint32_t width = _useNativeFullScreenRasterization ? getWidth() : _offscreenWidth;
	uint32_t height = _useNativeFullScreenRasterization ? getHeight() : _offscreenHeight;

	_deviceResources->skyBoxPass.init(*this, _deviceResources->device, _deviceResources->descriptorPool, _deviceResources->commandPool, _deviceResources->graphicsQueue,
		_deviceResources->offScreenGeometryRenderPass, _deviceResources->pipelineCache, pvrvk::Extent2D(width, height), _deviceResources->vmaAllocator);

	_deviceResources->helmetPass.init(*this, _deviceResources->device, _deviceResources->offscreenFramebuffer[0],
		_deviceResources->pipelineLayout,
		_deviceResources->pipelineCache,
		_deviceResources->vmaAllocator, _deviceResources->vectorGraphicsCommandBuffers[0], requireSubmission, _isASTCSupported);

	createUbos();

	updateDescriptors(); // Actually populate the data

	VkPhysicalDeviceSubgroupProperties physicalDeviceSubgroupProperties{};
	_deviceResources->device->getPhysicalDevice()->populateExtensionPropertiesVk(pvrvk::StructureType::e_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES, &physicalDeviceSubgroupProperties);

	_deviceResources->skyBoxPass.updateDescriptorSetSkyBoxPass(
		_deviceResources->device, _deviceResources->samplerTrilinear, _deviceResources->uboPerFrame.buffer, _deviceResources->uboPerFrame.view);

	changeInitialImageLayout(_deviceResources->vectorGraphicsCommandBuffers[0]);

	if (_useNativeFullScreenRasterization)
	{
		_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->offscreenFramebuffer[0]->getRenderPass(), 0,
			getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->graphicsQueue);
	}
	else
	{
		_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->uiFramebuffer[0]->getRenderPass(), 0,
			getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->graphicsQueue);
	}

	_deviceResources->uiRenderer.getDefaultTitle()->setText("IMG Neural Super Resolution").commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText("Action: Pause");
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

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
		_projectionMatrix = pvr::math::perspective(
			pvr::Api::Vulkan, glm::radians(fov), static_cast<float>(this->getHeight()) / static_cast<float>(this->getWidth()), 1.f, 2000.f, glm::pi<float>() * .5f);
	}
	else
	{
		_projectionMatrix = pvr::math::perspective(pvr::Api::Vulkan, glm::radians(fov), static_cast<float>(this->getWidth()) / static_cast<float>(this->getHeight()), 1.f, 2000.f);
	}

	_deviceResources->uiRenderer.getDefaultTitle()->setText("Neural Super Resolution").commitUpdates();
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
	if (!_useNativeFullScreenRasterization)
	{
		recordGraphicsUICommandBuffers(_deviceResources->vectorGraphicsUICommandBuffers);
	}

	return pvr::Result::Success;
}

/// <summary>Verify the required MentisV2NeuralSuperResolution input texture color format (sRGBA with eight bit per channel, or linear RGB lke R10G11B11)
/// is supported.</summary>
/// <returns>True of the format is supported, false otherwise.</returns>
bool VulkanNeuralSuperResolution::verifyColorFormatSupported()
{
	// MentisV2NeuralSuperResolution requires color input pixel format to be sRGB or linear RGB
	switch (_offscreenColorFormat)
	{
		case pvrvk::Format::e_R8G8B8_UINT:
		case pvrvk::Format::e_R8G8B8_SINT:
		case pvrvk::Format::e_R8G8B8_SRGB:
		case pvrvk::Format::e_B8G8R8_UNORM:
		case pvrvk::Format::e_B8G8R8_SNORM:
		case pvrvk::Format::e_B8G8R8_UINT:
		case pvrvk::Format::e_B8G8R8_SINT:
		case pvrvk::Format::e_B8G8R8_SRGB:
		case pvrvk::Format::e_R8G8B8A8_UNORM:
		case pvrvk::Format::e_R8G8B8A8_SNORM:
		case pvrvk::Format::e_R8G8B8A8_UINT:
		case pvrvk::Format::e_R8G8B8A8_SINT:
		case pvrvk::Format::e_R8G8B8A8_SRGB:
		case pvrvk::Format::e_B8G8R8A8_UNORM:
		case pvrvk::Format::e_B8G8R8A8_SNORM:
		case pvrvk::Format::e_B8G8R8A8_UINT:
		case pvrvk::Format::e_B8G8R8A8_SINT:
		case pvrvk::Format::e_B8G8R8A8_SRGB:
		case pvrvk::Format::e_R32G32B32_SFLOAT:
		case pvrvk::Format::e_R32G32B32A32_SFLOAT:
		case pvrvk::Format::e_R16G16B16_SFLOAT:
		case pvrvk::Format::e_R16G16B16A16_SFLOAT:
		case pvrvk::Format::e_B10G11R11_UFLOAT_PACK32:
		{
			return true;
		}
		default:
		{
			Log(LogLevel::Error, "Error: No depth format matching in VulkanNeuralSuperResolution::verifyColorFormatSupported.");
		}
	}

	return false;
}

/// <summary>Verify the required depth format VK_FORMAT_D32_SFLOAT is supported.</summary>
/// <returns>True of the format is supported, false otherwise.</returns>
bool VulkanNeuralSuperResolution::verifyDepthFormatSupported()
{
	pvrvk::ImageFormatProperties imageFormatProperties = _deviceResources->device->getPhysicalDevice()->getImageFormatProperties(_offscreenDepthFormat, pvrvk::ImageType::e_2D,
		pvrvk::ImageTiling::e_OPTIMAL,
		pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageCreateFlags::e_NONE);

	if (imageFormatProperties.getMaxResourceSize() == 0)
	{
		return false;
	}
	return true;
}

/// <summary>Build the images and framebuffers used in the offscreen pass.</summary>
void VulkanNeuralSuperResolution::createImagesAndFramebuffers()
{	
	uint32_t offscreenWidth = _useNativeFullScreenRasterization ? getWidth() : _offscreenWidth;
	uint32_t offscreenHeight = _useNativeFullScreenRasterization ? getHeight() : _offscreenHeight;
	
	pvrvk::ImageCreateInfo colorImageHalfResolutionInfoMask = pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, _offscreenColorFormat,
		pvrvk::Extent3D(offscreenWidth, offscreenHeight, 1u),
		pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT);

	pvrvk::ImageCreateInfo colorImagePreviousFrameResultInfoMask = pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, _offscreenColorFormat,
		pvrvk::Extent3D(getWidth(), getHeight(), 1u), pvrvk::ImageUsageFlags::e_TRANSFER_DST_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT);

	pvrvk::ImageCreateInfo motionVectorImageInfoMask = pvrvk::ImageCreateInfo(
		pvrvk::ImageType::e_2D, pvrvk::Format::e_R32G32_SFLOAT,
		pvrvk::Extent3D(offscreenWidth, offscreenHeight, 1u), pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT);

	pvrvk::ImageCreateInfo depthImageInfo = pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, _offscreenDepthFormat,
		pvrvk::Extent3D(offscreenWidth, offscreenHeight, 1u),
		pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT);

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		// Build color, motion vector and depth attachment image and image views for the offscreen pass of FXAA (1 sample per pixel)
		pvrvk::Image colorImage = pvr::utils::createImage(_deviceResources->device, colorImageHalfResolutionInfoMask, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT, _deviceResources->vmaAllocator,
			pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);
		pvrvk::ImageView colorImageView = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(colorImage));

		colorImage->setObjectName("offscreenColorAttachmentImageSwapchain" + std::to_string(i));
		colorImageView->setObjectName("offscreenColorAttachmentImageViewSwapchain" + std::to_string(i));

		_deviceResources->offscreenColorAttachmentImage.push_back(colorImage);
		_deviceResources->offscreenColorAttachmentImageView.push_back(colorImageView);
		
		// Motion vector image
		pvrvk::Image motionVectorImage = pvr::utils::createImage(_deviceResources->device, motionVectorImageInfoMask, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT, _deviceResources->vmaAllocator,
			pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);
		pvrvk::ImageView motionVectorImageView = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(motionVectorImage));

		colorImage->setObjectName("offscreenColorAttachmentImageSwapchain" + std::to_string(i));
		colorImageView->setObjectName("offscreenColorAttachmentImageViewSwapchain" + std::to_string(i));

		_deviceResources->offscreenMotionVectorAttachmentImage.push_back(motionVectorImage);
		_deviceResources->offscreenMotionVectorAttachmentImageView.push_back(motionVectorImageView);

		// Depth image
		pvrvk::Image depthImage = pvr::utils::createImage(
			_deviceResources->device, depthImageInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, _deviceResources->vmaAllocator);
		pvrvk::ImageView depthImageView1SPP = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(depthImage));
		_deviceResources->offscreenDepthAttachmentImage.push_back(depthImage);
		_deviceResources->offscreenDepthAttachmentImageView.push_back(depthImageView1SPP);

		// Previous result frame image
		pvrvk::Image previousFrameImage = pvr::utils::createImage(_deviceResources->device, colorImagePreviousFrameResultInfoMask, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT, _deviceResources->vmaAllocator,
			pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);
		pvrvk::ImageView previousFrameImageView = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(previousFrameImage));

		previousFrameImage->setObjectName("previousFrameImage" + std::to_string(i));
		previousFrameImageView->setObjectName("previousFrameImageView" + std::to_string(i));

		_deviceResources->previousFrameResultImage.push_back(previousFrameImage);
		_deviceResources->previousFrameResultImageView.push_back(previousFrameImageView);

		pvrvk::FramebufferCreateInfo offscreenFramebufferCreateInfo;
		offscreenFramebufferCreateInfo.setAttachment(0, _deviceResources->offscreenColorAttachmentImageView[i]);
		offscreenFramebufferCreateInfo.setAttachment(1, _deviceResources->offscreenMotionVectorAttachmentImageView[i]);
		offscreenFramebufferCreateInfo.setAttachment(2, _deviceResources->offscreenDepthAttachmentImageView[i]);
		offscreenFramebufferCreateInfo.setDimensions(offscreenWidth, offscreenHeight);
		offscreenFramebufferCreateInfo.setRenderPass(_deviceResources->offScreenGeometryRenderPass);
		_deviceResources->offscreenFramebuffer.push_back(_deviceResources->device->createFramebuffer(offscreenFramebufferCreateInfo));

		pvrvk::FramebufferCreateInfo offscreenFramebufferCreateInfo2;
		offscreenFramebufferCreateInfo2.setAttachment(0, _deviceResources->swapchain->getImageView(i));
		offscreenFramebufferCreateInfo2.setDimensions(getWidth(), getHeight());
		if (_useNativeFullScreenRasterization)
		{
			offscreenFramebufferCreateInfo2.setRenderPass(_deviceResources->offScreenGeometryRenderPass);
		}
		else
		{
			offscreenFramebufferCreateInfo2.setRenderPass(_deviceResources->uiRenderPass);
		}
		
		_deviceResources->uiFramebuffer.push_back(_deviceResources->device->createFramebuffer(offscreenFramebufferCreateInfo2));
	}
}

/// <summary>Build the images and framebuffers used in the offscreen pass.</summary>
/// <param name="numColorAttachments">Number of color attachments to fill.</param>
/// <param name="vectorColorFormat">Vector with the color formats of the attachments.</param>
/// <param name="numSamplesPerPixel">Vector with the number of samples per pixel.</param>
/// <param name="keepColorAttachmentContent">Flag to keep or not the contents of the color attachment.</param>
/// <param name="vectorAttachmentDescription">Vector with the final attachment description.</param>
void VulkanNeuralSuperResolution::fillAttachmentDescription(int numColorAttachments, const std::vector<pvrvk::Format>& vectorColorFormat, bool addDepthAttachment,
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
		vectorAttachmentDescription.push_back(pvrvk::AttachmentDescription::createDepthStencilDescription(_offscreenDepthFormat,
			pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE,
			pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE, numSamplesPerPixel));
	}
}

/// <summary>Build the render pass used to draw the scene offscreen.</summary>
void VulkanNeuralSuperResolution::createOffScreenGeometryRenderPass()
{
	std::vector<pvrvk::AttachmentDescription> vectorAttachmentDescription;
	std::vector<pvrvk::Format> vectorColorFormat = { _deviceResources->swapchain->getImageFormat(), pvrvk::Format::e_R32G32_SFLOAT };
	fillAttachmentDescription(2, vectorColorFormat, true, pvrvk::SampleCountFlags::e_1_BIT, false, vectorAttachmentDescription);
	_deviceResources->offScreenGeometryRenderPass = createTechniqueRenderPass(vectorAttachmentDescription);
	_deviceResources->offScreenGeometryRenderPass->setObjectName("offScreenGeometryRenderPass");
}

/// <summary>Generate Halton sequence for index and basde iven as parameter.</summary>
/// <param name="index">Index of the Halton sequence.</param>
/// <param name="base">Base of the Halton sequence.</param>
float VulkanNeuralSuperResolution::generateHaltonSequence(unsigned int index, int base)
{
	float f = 1;
	float r = 0;

	int current = index;
	do {
		f = f / base;
		r = r + f * (current % base);
		current = static_cast<int>(glm::floor(static_cast<float>(current) / static_cast<float>(base)));
	} while (current > 0);

	return r;
}

/// <summary>Fill the arrayHaltonSequenceJitter array with the final pixel jittering values.</summary>
void VulkanNeuralSuperResolution::buildPixelJitteringValues()
{
	float screenWidth = float(_offscreenWidth);
	float screenHeight = float(_offscreenHeight);

	float screenWidthInverse = 1.0f / screenWidth;
	float screenHeightInverse = 1.0f / screenHeight;

	for (int i = 0; i < numberHaltonSequenceValues; i++)
	{
		float x = generateHaltonSequence(i + 1, 2);
		float y = generateHaltonSequence(i + 1, 3);

		arrayHaltonSequenceJitter[i].x = x;
		arrayHaltonSequenceJitter[i].y = y;

		arrayHaltonSequenceJitter[i].x = (((x - 0.5f) / screenWidth) * 2.0f) * screenWidthInverse;
		arrayHaltonSequenceJitter[i].y = (((y - 0.5f) / screenHeight) * 2.0f) * screenHeightInverse;
	}
}

/// <summary>Build the render pass used to draw the UI on top of the compute-generated environment.</summary>
void VulkanNeuralSuperResolution::createUIRenderPass()
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
pvrvk::RenderPass VulkanNeuralSuperResolution::createTechniqueRenderPass(const std::vector<pvrvk::AttachmentDescription>& vectorAttachmentDescription)
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
pvr::Result VulkanNeuralSuperResolution::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Render vurrent frame.</summary>
pvr::Result VulkanNeuralSuperResolution::renderFrame()
{
	// As sometimes the swapchain index can be repeated at the beginning of the application, let's use a new set of command buffers 
	// for this repeated swapchain index to allow multiple frames in flight
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->vectorImageAcquiredSemaphores[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->vectorLastSubmitFence[swapchainIndex]->wait();
	_deviceResources->vectorLastSubmitFence[swapchainIndex]->reset();

	if (!_pause)
	{
		_camera.addAzimuth(getFrameTime() * rotationSpeed);
	}

	if (this->isKeyPressed(pvr::Keys::A)) { _camera.addAzimuth(getFrameTime() * -.1f); }
	if (this->isKeyPressed(pvr::Keys::D)) { _camera.addAzimuth(getFrameTime() * .1f); }

	if (this->isKeyPressed(pvr::Keys::W)) { _camera.addInclination(getFrameTime() * .1f); }
	if (this->isKeyPressed(pvr::Keys::S)) { _camera.addInclination(getFrameTime() * -.1f); }

	_viewMatrixCurrentFrame = _camera.getViewMatrix();

	updateUniformBuffer(swapchainIndex);

	_viewMatrixPreviousFrame = _viewMatrixCurrentFrame;
	// Make sure to update any Dynamic map variables by now as the techniques in SuperResolution will update their values at this point
	// NOTE: Design a better approach to share information with the PVRSuperResolution library.
	// Ideally, a way to "send packages" of information to avoid sharing a pointer to a custom data structure in the library.
	// Simplest approach would be a set of structs, one per data type (float array, int array, double array, string, etc) with an enum
	// and some methods "sendInt(), sendFloat(), ... etc).
	// Each technique in SuperResolution::_vectorPass would register the string it wants to know about, and the sendXYZ method would
	// notify accordingly without the technique knowing anything outside itself
	dynamicMap->setValue("jitterX", arrayHaltonSequenceJitter[_frameCounter % numberHaltonSequenceValues].x);
	dynamicMap->setValue("jitterY", arrayHaltonSequenceJitter[_frameCounter % numberHaltonSequenceValues].y);
	dynamicMap->setValue("frameCounter", float(_frameCounter));
	_deviceResources->superResolutionNSR->frameUpdate(swapchainIndex);

	if (_useNativeFullScreenRasterization)
	{
		// submit the graphics command buffer
		pvrvk::SubmitInfo graphicsSubmitInfo;
		pvrvk::PipelineStageFlags graphicsWaitStage = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
		graphicsSubmitInfo.commandBuffers = &_deviceResources->vectorGraphicsCommandBuffers[swapchainIndex];
		graphicsSubmitInfo.numCommandBuffers = 1;
		graphicsSubmitInfo.waitDstStageMask = &graphicsWaitStage;
		graphicsSubmitInfo.waitSemaphores = &_deviceResources->vectorImageAcquiredSemaphores[swapchainIndex]; // wait for the acquire to be finished.
		graphicsSubmitInfo.numWaitSemaphores = 1;
		graphicsSubmitInfo.signalSemaphores = &_deviceResources->vectorPresentationSemaphores[swapchainIndex]; // signal the compute sempahore when finished.
		graphicsSubmitInfo.numSignalSemaphores = 1;
		_deviceResources->graphicsQueue->submit(&graphicsSubmitInfo, 1, _deviceResources->vectorLastSubmitFence[swapchainIndex]);
	}
	else
	{
		// submit the graphics command buffer
		pvrvk::SubmitInfo graphicsSubmitInfo;
		pvrvk::PipelineStageFlags graphicsWaitStage = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
		graphicsSubmitInfo.commandBuffers = &_deviceResources->vectorGraphicsCommandBuffers[swapchainIndex];
		graphicsSubmitInfo.numCommandBuffers = 1;
		graphicsSubmitInfo.waitDstStageMask = &graphicsWaitStage;
		graphicsSubmitInfo.waitSemaphores = &_deviceResources->vectorImageAcquiredSemaphores[swapchainIndex]; // wait for the acquire to be finished.
		graphicsSubmitInfo.numWaitSemaphores = 1;
		graphicsSubmitInfo.signalSemaphores = &_deviceResources->vectorComputeSemaphores[swapchainIndex]; // signal the compute sempahore when finished.
		graphicsSubmitInfo.numSignalSemaphores = 1;
		_deviceResources->graphicsQueue->submit(&graphicsSubmitInfo, 1, nullptr);

		// submit the compute command buffer in charge of the upscaling
		pvrvk::SubmitInfo computeSubmitInfo;
		pvrvk::PipelineStageFlags computeWaitStage = pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT;
		computeSubmitInfo.commandBuffers = &_deviceResources->vectorComputeCooperativeMatrixCommandBuffers[swapchainIndex];
		computeSubmitInfo.numCommandBuffers = 1;
		computeSubmitInfo.waitDstStageMask = &computeWaitStage;
		computeSubmitInfo.waitSemaphores = &_deviceResources->vectorComputeSemaphores[swapchainIndex]; // wait for the graphics command buffer to be finished.
		computeSubmitInfo.numWaitSemaphores = 1;
		computeSubmitInfo.signalSemaphores = &_deviceResources->vectorUISemaphores[swapchainIndex]; // signal the UI semaphore when it is finish with compute.
		computeSubmitInfo.numSignalSemaphores = 1;
		_deviceResources->computeQueue->submit(&computeSubmitInfo, 1, nullptr);

		// submit the graphics command buffer in charge of the UI
		pvrvk::SubmitInfo UISubmitInfo;
		pvrvk::PipelineStageFlags UIWaitStage = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
		UISubmitInfo.commandBuffers = &_deviceResources->vectorGraphicsUICommandBuffers[swapchainIndex];
		UISubmitInfo.numCommandBuffers = 1;
		UISubmitInfo.waitDstStageMask = &UIWaitStage;
		UISubmitInfo.waitSemaphores = &_deviceResources->vectorUISemaphores[swapchainIndex]; // wait for the compute command buffer to be finished.
		UISubmitInfo.numWaitSemaphores = 1;
		UISubmitInfo.signalSemaphores = &_deviceResources->vectorPresentationSemaphores[swapchainIndex]; // signal the UI semaphore when it is finish with compute.
		UISubmitInfo.numSignalSemaphores = 1;
		_deviceResources->graphicsQueue->submit(&UISubmitInfo, 1, _deviceResources->vectorLastSubmitFence[swapchainIndex]);
	}
		
	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->computeQueue, _deviceResources->commandPool, _deviceResources->swapchain, swapchainIndex, this->getScreenshotFileName(),
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

	_frameCounter++;

	return pvr::Result::Success;
}

/// <summary>Pre-record the graphics commands to draw the scene with environment, and without environment.</summary>
void VulkanNeuralSuperResolution::recordCommandBuffers()
{
	recordGraphicsCommandBuffers(_deviceResources->vectorGraphicsCommandBuffers);
}

/// <summary>Pre-record the rendering commands.</summary>
/// <param name="vectorCommandBuffer">Vector with the command buffers to record to.</param>
void VulkanNeuralSuperResolution::recordGraphicsCommandBuffers(std::vector<pvrvk::CommandBuffer>& vectorCommandBuffer)
{
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		const pvrvk::ClearValue clearValues[] = { pvrvk::ClearValue(0.6f, 0.64f, 0.66f, 1.0f), pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 0.0f),
			pvrvk::ClearValue(1.0f, 0) };

		// begin recording commands
		vectorCommandBuffer[i]->begin();

		pvr::utils::beginCommandBufferDebugLabel(vectorCommandBuffer[i], pvrvk::DebugUtilsLabel("MainRenderPass"));

		// begin the renderpass
		uint32_t width = _useNativeFullScreenRasterization ? getWidth() : _offscreenWidth;
		uint32_t height = _useNativeFullScreenRasterization ? getHeight() : _offscreenHeight;
		vectorCommandBuffer[i]->beginRenderPass(_deviceResources->offscreenFramebuffer[i], pvrvk::Rect2D(0, 0, width, height), true, clearValues, ARRAY_SIZE(clearValues));

		// Render the sky box
		_deviceResources->skyBoxPass.recordCommands(vectorCommandBuffer[i], i, _deviceResources->uboPerFrame.view);

		uint32_t offsets[1];
		// get the matrix array offset
		offsets[0] = _deviceResources->uboPerFrame.view.getDynamicSliceOffset(i);

		// bind the descriptor sets
		vectorCommandBuffer[i]->bindDescriptorSets(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->pipelineLayout, 0, _deviceResources->descSets, ARRAY_SIZE(_deviceResources->descSets), offsets, 1);

		_deviceResources->helmetPass.recordCommands(vectorCommandBuffer[i]);

		// If natively drawing at full screen resolution and not at 1/4, UI can be drawn as well (as UI renderer does not provide the
		// information MentisV2NeuralSuperResolution needs like motion vectors, it cannot be upscaled properly)
		if (_useNativeFullScreenRasterization)
		{
			// record the ui renderer.
			_deviceResources->uiRenderer.beginRendering(vectorCommandBuffer[i]);
			_deviceResources->uiRenderer.getDefaultTitle()->render();
			_deviceResources->uiRenderer.getDefaultControls()->render();
			_deviceResources->uiRenderer.getSdkLogo()->render();
			_deviceResources->uiRenderer.endRendering();
		}

		vectorCommandBuffer[i]->endRenderPass();

		if (_useNativeFullScreenRasterization)
		{
			// Until a dedicated pipeline is done for native full screen rasterization, copy the output of the offscreen texture to
			// the swapchain image

			// Change the offscreenColorAttachmentImage image layout from color attachment to transfer source to make a copy
			pvrvk::MemoryBarrierSet2 swapchainCopyBarriers;
			pvrvk::ImageMemoryBarrier2 imageBarrier1;
			imageBarrier1.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
			imageBarrier1.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_TRANSFER_BIT_KHR);
			imageBarrier1.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_COLOR_ATTACHMENT_WRITE_BIT_KHR);
			imageBarrier1.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_TRANSFER_READ_BIT_KHR);
			imageBarrier1.setOldLayout(pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
			imageBarrier1.setNewLayout(pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL);
			imageBarrier1.setSrcQueueFamilyIndex(_deviceResources->graphicsQueue->getFamilyIndex());
			imageBarrier1.setDstQueueFamilyIndex(_deviceResources->graphicsQueue->getFamilyIndex());
			imageBarrier1.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));
			imageBarrier1.setImage(_deviceResources->offscreenColorAttachmentImage[i]);
			swapchainCopyBarriers.addBarrier(imageBarrier1);

			// Change the swapchain image layout from present to transfer destination to make a copy
			imageBarrier1.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_BOTTOM_OF_PIPE_BIT_KHR);
			imageBarrier1.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_ALL_TRANSFER_BIT_KHR);
			imageBarrier1.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_NONE_KHR);
			imageBarrier1.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_TRANSFER_WRITE_BIT_KHR);
			imageBarrier1.setOldLayout(pvrvk::ImageLayout::e_PRESENT_SRC_KHR);
			imageBarrier1.setNewLayout(pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL);
			imageBarrier1.setImage(_deviceResources->swapchain->getImage(i));
			swapchainCopyBarriers.addBarrier(imageBarrier1);
			vectorCommandBuffer[i]->pipelineBarrier2(swapchainCopyBarriers);
			
			pvrvk::ImageSubresourceLayers imageSubresourceLayers = pvrvk::ImageSubresourceLayers(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0, 0, 1);
			pvrvk::Offset3D offset = pvrvk::Offset3D(0, 0, 0);
			pvrvk::Extent3D extent = pvrvk::Extent3D(getWidth(), getHeight(), 1);
			pvrvk::ImageCopy imageCopyInformation = pvrvk::ImageCopy(imageSubresourceLayers, offset, imageSubresourceLayers, offset, extent);
			vectorCommandBuffer[i]->copyImage(_deviceResources->offscreenColorAttachmentImage[i], _deviceResources->swapchain->getImage(i), 
				pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, 1, &imageCopyInformation);

			// Change the swapchain image layout from transfer destination back to present
			pvrvk::MemoryBarrierSet2 restoreLayoutBarriers;
			pvrvk::ImageMemoryBarrier2 imageBarrier3;
			imageBarrier3.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_ALL_TRANSFER_BIT_KHR);
			imageBarrier3.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_BOTTOM_OF_PIPE_BIT_KHR);
			imageBarrier3.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_TRANSFER_WRITE_BIT_KHR);
			imageBarrier3.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_NONE_KHR);
			imageBarrier3.setOldLayout(pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL);
			imageBarrier3.setNewLayout(pvrvk::ImageLayout::e_PRESENT_SRC_KHR);
			imageBarrier3.setSrcQueueFamilyIndex(_deviceResources->graphicsQueue->getFamilyIndex());
			imageBarrier3.setDstQueueFamilyIndex(_deviceResources->graphicsQueue->getFamilyIndex());
			imageBarrier3.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));
			imageBarrier3.setImage(_deviceResources->swapchain->getImage(i));
			restoreLayoutBarriers.addBarrier(imageBarrier3);

			imageBarrier3.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_TRANSFER_BIT_KHR);
			imageBarrier3.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
			imageBarrier3.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_TRANSFER_READ_BIT_KHR);
			imageBarrier3.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_COLOR_ATTACHMENT_WRITE_BIT_KHR);
			imageBarrier3.setOldLayout(pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL);
			imageBarrier3.setNewLayout(pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
			imageBarrier3.setImage(_deviceResources->offscreenColorAttachmentImage[i]);
			restoreLayoutBarriers.addBarrier(imageBarrier3);
			vectorCommandBuffer[i]->pipelineBarrier2(restoreLayoutBarriers);
		}
		else
		{
			// Transition the layout of the color attachments to be used for sampling in the MentisV2NeuralSuperResolution compute dispatch
			pvrvk::MemoryBarrierSet2 graphicsToComputeBarrier;

			// Transition the mask image from e_COLOR_ATTACHMENT_OPTIMAL to draw to it to e_SHADER_READ_ONLY_OPTIMAL for sampling in the upcoming compute pass
			pvrvk::ImageMemoryBarrier2 imageBarrier;
			imageBarrier.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
			imageBarrier.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COMPUTE_SHADER_BIT_KHR);
			imageBarrier.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_COLOR_ATTACHMENT_WRITE_BIT_KHR);
			imageBarrier.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_SHADER_READ_BIT_KHR);
			imageBarrier.setOldLayout(pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
			imageBarrier.setNewLayout(pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL);
			imageBarrier.setSrcQueueFamilyIndex(_deviceResources->graphicsQueue->getFamilyIndex());
			imageBarrier.setDstQueueFamilyIndex(_deviceResources->computeQueue->getFamilyIndex());
			imageBarrier.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));
			imageBarrier.setImage(_deviceResources->offscreenColorAttachmentImage[i]);

			graphicsToComputeBarrier.addBarrier(imageBarrier);

			imageBarrier.setImage(_deviceResources->offscreenMotionVectorAttachmentImage[i]);
			graphicsToComputeBarrier.addBarrier(imageBarrier);

			imageBarrier.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_LATE_FRAGMENT_TESTS_BIT_KHR);
			imageBarrier.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT_KHR);
			imageBarrier.setOldLayout(pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
			imageBarrier.setNewLayout(pvrvk::ImageLayout::e_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
			imageBarrier.setImage(_deviceResources->offscreenDepthAttachmentImage[i]);
			imageBarrier.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_DEPTH_BIT | pvrvk::ImageAspectFlags::e_STENCIL_BIT));
			graphicsToComputeBarrier.addBarrier(imageBarrier);

			vectorCommandBuffer[i]->pipelineBarrier2(graphicsToComputeBarrier);
		}

		pvr::utils::endCommandBufferDebugLabel(vectorCommandBuffer[i]);
		vectorCommandBuffer[i]->end();
	}
}

/// <summary>Pre-record the rendering commands for the UI.</summary>
/// <param name="vectorCommandBuffer">Vector with the command buffers to record to.</param>
void VulkanNeuralSuperResolution::recordGraphicsUICommandBuffers(std::vector<pvrvk::CommandBuffer>& vectorCommandBuffer)
{
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		// begin recording commands
		vectorCommandBuffer[i]->begin();

		pvr::utils::beginCommandBufferDebugLabel(vectorCommandBuffer[i], pvrvk::DebugUtilsLabel("UIGraphicsPass"));

		// begin the renderpass
		vectorCommandBuffer[i]->beginRenderPass(_deviceResources->uiFramebuffer[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, nullptr, 0);

		// record the ui renderer.
		_deviceResources->uiRenderer.beginRendering(vectorCommandBuffer[i]);
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getDefaultControls()->render();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.endRendering();

		vectorCommandBuffer[i]->endRenderPass();

		pvrvk::MemoryBarrierSet2 graphicsToComputeBarrier;
		pvrvk::ImageMemoryBarrier2 imageBarrier3;
		imageBarrier3.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COMPUTE_SHADER_BIT_KHR);
		imageBarrier3.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_BOTTOM_OF_PIPE_BIT_KHR);
		imageBarrier3.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_SHADER_STORAGE_WRITE_BIT_KHR);
		imageBarrier3.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_NONE_KHR);
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
void VulkanNeuralSuperResolution::recordSecondaryComputeCommandBuffers()
{
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->vectorNeuralSuperResolutionCooperativeMatrixCommandBuffer[i]->begin();
		recordNeuralSuperResolutionCooperativeMatrixCommandBuffer(i);
		_deviceResources->vectorNeuralSuperResolutionCooperativeMatrixCommandBuffer[i]->end();
	}
}

/// <summary>Record command buffer for the compute pass which infers the values of the pixels for a specific region of the screen.
/// This compute pass uses compute shaders and cooperative matrices to infer the pixel values through loading a specific neural network which 
/// approximates a specific part of the environment texture.</summary>
/// <param name="swapIndex">Swapchain index.</param>
void VulkanNeuralSuperResolution::recordNeuralSuperResolutionCooperativeMatrixCommandBuffer(uint32_t swapIndex)
{
	// Third dispatch: Each workgroup will read the information of one slot from the organizePatchWorkload buffer, load the information of the
	// neural network patch to be approximated from the NeuralSuperResolutionBuffer and infer the pixels in the screen-space box belonging to the patch

	// Change the swapchain image layour from present to general
	pvrvk::MemoryBarrierSet2 computeToGraphicsBarrier;
	pvrvk::ImageMemoryBarrier2 presentBarrier;
	presentBarrier.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_TOP_OF_PIPE_BIT_KHR);
	presentBarrier.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COMPUTE_SHADER_BIT_KHR);
	presentBarrier.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_NONE_KHR);
	presentBarrier.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_SHADER_STORAGE_WRITE_BIT_KHR);
	presentBarrier.setOldLayout(pvrvk::ImageLayout::e_PRESENT_SRC_KHR);
	presentBarrier.setNewLayout(pvrvk::ImageLayout::e_GENERAL);
	presentBarrier.setSrcQueueFamilyIndex(_deviceResources->graphicsQueue->getFamilyIndex());
	presentBarrier.setDstQueueFamilyIndex(_deviceResources->computeQueue->getFamilyIndex());
	presentBarrier.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));
	presentBarrier.setImage(_deviceResources->swapchain->getImage(swapIndex));
	computeToGraphicsBarrier.addBarrier(presentBarrier);
	_deviceResources->vectorNeuralSuperResolutionCooperativeMatrixCommandBuffer[swapIndex]->pipelineBarrier2(computeToGraphicsBarrier);

	_deviceResources->vectorNeuralSuperResolutionCooperativeMatrixCommandBuffer[swapIndex]->setObjectName(
		"NeuralSuperResolutionCooperativeMatrixSwapchain" + std::to_string(swapIndex));

	_deviceResources->superResolutionNSR->recordCommands(_deviceResources->vectorNeuralSuperResolutionCooperativeMatrixCommandBuffer[swapIndex]->getVkHandle(), swapIndex);

	// Do a copy of the swapchain image from the previous frame as it is an input of the current frame

	// Change the previous image layout from shader read to transfer source
	pvrvk::MemoryBarrierSet2 computeToGraphicsBarrier_;
	pvrvk::ImageMemoryBarrier2 presentBarrier_;
	presentBarrier_.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_FRAGMENT_SHADER_BIT_KHR);
	presentBarrier_.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_TRANSFER_BIT_KHR);
	presentBarrier_.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_SHADER_SAMPLED_READ_BIT_KHR);
	presentBarrier_.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_TRANSFER_WRITE_BIT_KHR);
	presentBarrier_.setOldLayout(pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL);
	presentBarrier_.setNewLayout(pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL);
	presentBarrier_.setSrcQueueFamilyIndex(_deviceResources->graphicsQueue->getFamilyIndex());
	presentBarrier_.setDstQueueFamilyIndex(_deviceResources->computeQueue->getFamilyIndex());
	presentBarrier_.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));
	presentBarrier_.setImage(_deviceResources->previousFrameResultImage[swapIndex]);
	computeToGraphicsBarrier_.addBarrier(presentBarrier_);
	_deviceResources->vectorNeuralSuperResolutionCooperativeMatrixCommandBuffer[swapIndex]->pipelineBarrier2(computeToGraphicsBarrier_);
	
	// Change the swapchain image layour from general to transfer source to make a copy (previous frame is kept)
	pvrvk::MemoryBarrierSet2 computeToGraphicsBarrier2;
	pvrvk::ImageMemoryBarrier2 presentBarrier2;
	presentBarrier2.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COMPUTE_SHADER_BIT_KHR);
	presentBarrier2.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_TRANSFER_BIT_KHR);
	presentBarrier2.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_SHADER_STORAGE_WRITE_BIT_KHR);
	presentBarrier2.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_TRANSFER_READ_BIT_KHR);
	presentBarrier2.setOldLayout(pvrvk::ImageLayout::e_GENERAL);
	presentBarrier2.setNewLayout(pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL);
	presentBarrier2.setSrcQueueFamilyIndex(_deviceResources->computeQueue->getFamilyIndex());
	presentBarrier2.setDstQueueFamilyIndex(_deviceResources->computeQueue->getFamilyIndex());
	presentBarrier2.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));
	presentBarrier2.setImage(_deviceResources->swapchain->getImage(swapIndex));
	computeToGraphicsBarrier2.addBarrier(presentBarrier2);
	_deviceResources->vectorNeuralSuperResolutionCooperativeMatrixCommandBuffer[swapIndex]->pipelineBarrier2(computeToGraphicsBarrier2);

	pvrvk::ImageSubresourceLayers imageSubresourceLayers = pvrvk::ImageSubresourceLayers(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0, 0, 1);
	pvrvk::Offset3D offset = pvrvk::Offset3D(0, 0, 0);
	pvrvk::Extent3D extent = pvrvk::Extent3D(getWidth(), getHeight(), 1);
	pvrvk::ImageCopy imageCopyInformation = pvrvk::ImageCopy(imageSubresourceLayers, offset, imageSubresourceLayers, offset, extent);
	_deviceResources->vectorNeuralSuperResolutionCooperativeMatrixCommandBuffer[swapIndex]->copyImage(_deviceResources->swapchain->getImage(swapIndex),
		_deviceResources->previousFrameResultImage[swapIndex], pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL,
		pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, 1, &imageCopyInformation);

	// Change the swapchain image layout from transfer destination to color attachment, as the UI still needs to be rendered
	// in a different command buffer run after this one
	pvrvk::MemoryBarrierSet2 computeToGraphicsBarrier3;
	pvrvk::ImageMemoryBarrier2 presentBarrier3;
	presentBarrier3.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_TRANSFER_BIT_KHR);
	presentBarrier3.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
	presentBarrier3.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_TRANSFER_READ_BIT_KHR);
	presentBarrier3.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_COLOR_ATTACHMENT_WRITE_BIT_KHR);
	presentBarrier3.setOldLayout(pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL);
	presentBarrier3.setNewLayout(pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
	presentBarrier3.setSrcQueueFamilyIndex(_deviceResources->computeQueue->getFamilyIndex());
	presentBarrier3.setDstQueueFamilyIndex(_deviceResources->graphicsQueue->getFamilyIndex());
	presentBarrier3.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));
	presentBarrier3.setImage(_deviceResources->swapchain->getImage(swapIndex));
	computeToGraphicsBarrier3.addBarrier(presentBarrier3);
	_deviceResources->vectorNeuralSuperResolutionCooperativeMatrixCommandBuffer[swapIndex]->pipelineBarrier2(computeToGraphicsBarrier3);

	// Change the previous image layout from transfer source to shader read
	pvrvk::MemoryBarrierSet2 computeToGraphicsBarrier4;
	pvrvk::ImageMemoryBarrier2 presentBarrier4;
	presentBarrier4.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_TRANSFER_BIT_KHR);
	presentBarrier4.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_FRAGMENT_SHADER_BIT_KHR);
	presentBarrier4.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_TRANSFER_WRITE_BIT_KHR);
	presentBarrier4.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_SHADER_SAMPLED_READ_BIT_KHR);
	presentBarrier4.setOldLayout(pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL);
	presentBarrier4.setNewLayout(pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL);
	presentBarrier4.setSrcQueueFamilyIndex(_deviceResources->computeQueue->getFamilyIndex());
	presentBarrier4.setDstQueueFamilyIndex(_deviceResources->graphicsQueue->getFamilyIndex());
	presentBarrier4.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));
	presentBarrier4.setImage(_deviceResources->previousFrameResultImage[swapIndex]);
	computeToGraphicsBarrier4.addBarrier(presentBarrier4);
	_deviceResources->vectorNeuralSuperResolutionCooperativeMatrixCommandBuffer[swapIndex]->pipelineBarrier2(computeToGraphicsBarrier4);
}

/// <summary>Record command buffer for the two types of implementations to infer pixels (plain compute, and using cooperative matrix).</summary>
void VulkanNeuralSuperResolution::recordComputeCommandBuffers()
{
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->vectorComputeCooperativeMatrixCommandBuffers[i]->setObjectName("ComputeCommandBufferCooperativeMatrixSwapchain" + std::to_string(i));
		_deviceResources->vectorComputeCooperativeMatrixCommandBuffers[i]->begin();
		_deviceResources->vectorComputeCooperativeMatrixCommandBuffers[i]->executeCommands(_deviceResources->vectorNeuralSuperResolutionCooperativeMatrixCommandBuffer[i]);
		_deviceResources->vectorComputeCooperativeMatrixCommandBuffers[i]->end();
	}
}

/// <summary>Get comand line option to draw the scene at full screen resolution without using the MentisV2NeuralSuperResolution upscaler.</summary>
void VulkanNeuralSuperResolution::processCommandlineOptions()
{
	_cmdLine.getBoolOptionSetTrueIfPresent("-nativeFullScreenRasterization", _useNativeFullScreenRasterization);
}

/// <summary>Query and show information on the possible cooperative matrix configurations availables on the GPU the application is running.</summary>
void VulkanNeuralSuperResolution::queryCooperativeMatrixInformation()
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

/// <summary>Build the SuperResolution library Mentis v2 Neural Super Resolution instance taking care of upscaling.</summary>
void VulkanNeuralSuperResolution::initializeSuperResolution()
{
	_deviceResources->superResolutionNSR = new pvr::SuperResolution();

	pvr::VulkanInitializationData postProcessingInitializationData = {};
	postProcessingInitializationData.device = _deviceResources->device->getVkHandle();
	postProcessingInitializationData.physicalDevice = _deviceResources->device->getPhysicalDevice()->getVkHandle();
	std::vector<std::vector<VkImageView>> vectorInputImageView;
	std::vector<VkImageView> vectorOutputImageView;
	for (size_t i = 0; i < _deviceResources->offscreenColorAttachmentImageView.size(); ++i)
	{
		std::vector<VkImageView> vectorInput;
		vectorInput.push_back(_deviceResources->offscreenColorAttachmentImageView[i]->getVkHandle());
		vectorInput.push_back(_deviceResources->offscreenMotionVectorAttachmentImageView[i]->getVkHandle());
		vectorInput.push_back(_deviceResources->offscreenDepthAttachmentImageView[i]->getVkHandle());
		vectorInput.push_back(_deviceResources->previousFrameResultImageView[i]->getVkHandle());
		postProcessingInitializationData.vectorInputImageView.push_back(vectorInput);

		postProcessingInitializationData.vectorOutputImageView.push_back(_deviceResources->swapchain->getImageView(static_cast<uint32_t>(i))->getVkHandle());
	}

	postProcessingInitializationData.inputImageFormat.push_back(static_cast<VkFormat>(_deviceResources->offscreenColorAttachmentImageView[0]->getFormat()));
	postProcessingInitializationData.inputImageFormat.push_back(static_cast<VkFormat>(_deviceResources->offscreenMotionVectorAttachmentImageView[0]->getFormat()));
	postProcessingInitializationData.inputImageFormat.push_back(static_cast<VkFormat>(_deviceResources->offscreenDepthAttachmentImageView[0]->getFormat()));
	postProcessingInitializationData.inputImageFormat.push_back(static_cast<VkFormat>(_deviceResources->previousFrameResultImage[0]->getFormat()));

	postProcessingInitializationData.inputImageLayout.push_back(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	postProcessingInitializationData.inputImageLayout.push_back(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	postProcessingInitializationData.inputImageLayout.push_back(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
	postProcessingInitializationData.inputImageLayout.push_back(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	postProcessingInitializationData.inputImageExtent = { _offscreenWidth, _offscreenHeight };
	postProcessingInitializationData.outputImageFormat = static_cast<VkFormat>(_deviceResources->swapchain->getImageFormat());
	postProcessingInitializationData.outputImageInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	postProcessingInitializationData.outputImageFinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	postProcessingInitializationData.outputImageExtent = { getWidth(), getHeight() };
	postProcessingInitializationData.queueFlagBits = VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT;
	postProcessingInitializationData.queue = _deviceResources->graphicsQueue->getVkHandle();
	postProcessingInitializationData.queueFamilyIndex = _deviceResources->graphicsQueue->getFamilyIndex();
	postProcessingInitializationData.numberCommandBuffer = _deviceResources->swapchain->getSwapchainLength();
	postProcessingInitializationData.vk = &_deviceResources->device->getVkBindings();
	postProcessingInitializationData.vkInstance = &_deviceResources->instance->getVkBindings();
	postProcessingInitializationData.application = static_cast<void*>(getOSApplication());
	postProcessingInitializationData.postProcessingMethod = pvr::PostProcessingMethod::MentisV2NeuralSuperResolution;
	postProcessingInitializationData.dynamicMap = dynamicMap;
	dynamicMap = _deviceResources->superResolutionNSR->init(postProcessingInitializationData);
}

/// <summary>Build descriptor set layouts for some uniform buffers used in the sample.</summary>
void VulkanNeuralSuperResolution::createDescriptorSetLayouts()
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

/// <summary>Build the pipeline layout object to use the descriptor set layouts built in VulkanNeuralSuperResolution::createDescriptorSetLayouts().</summary>
void VulkanNeuralSuperResolution::createPipelineLayout()
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

/// <summary>Update the compute uniform buffer with per-frame information.</summary>
/// <param name="swapchainIndex">Swapchain index.</param>
void VulkanNeuralSuperResolution::updateUniformBuffer(uint32_t swapchainIndex)
{
	static float emissiveScale = 0.0f;
	static float emissiveStrength = 1.0f;
	emissiveStrength += .15f;
	if (emissiveStrength >= glm::pi<float>()) { emissiveStrength = 0.0f; }
	emissiveScale = std::abs(glm::cos(emissiveStrength)) + .75f;

	_deviceResources->uboPerFrame.view.getElementByName("inverseViewProjectionMatrix", 0, swapchainIndex).setValue(glm::inverse(_projectionMatrix * _viewMatrixCurrentFrame));
	_deviceResources->uboPerFrame.view.getElementByName("cameraPosition", 0, swapchainIndex).setValue(_camera.getCameraPosition());
	_deviceResources->uboPerFrame.view.getElementByName("projectionMatrix", 0, swapchainIndex).setValue(_projectionMatrix);
	_deviceResources->uboPerFrame.view.getElementByName("exposure", 0, swapchainIndex).setValue(exposure);
	_deviceResources->uboPerFrame.view.getElementByName("viewMatrixCurrentFrame", 0, swapchainIndex).setValue(_viewMatrixCurrentFrame);
	_deviceResources->uboPerFrame.view.getElementByName("viewMatrixPreviousFrame", 0, swapchainIndex).setValue(_viewMatrixPreviousFrame);
	_deviceResources->uboPerFrame.view.getElementByName("emissiveIntensity", 0, swapchainIndex).setValue(emissiveScale);
	_deviceResources->uboPerFrame.view.getElementByName("jitter", 0, swapchainIndex).setValue(arrayHaltonSequenceJitter[_frameCounter % numberHaltonSequenceValues]);

	// if the memory property flags used by the buffers' device memory does not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->uboPerFrame.buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->uboPerFrame.buffer->getDeviceMemory()->flushRange(
			_deviceResources->uboPerFrame.view.getDynamicSliceOffset(swapchainIndex), _deviceResources->uboPerFrame.view.getDynamicSliceSize());
	}
}

/// <summary>Change the layout of the swapchain images from undefined to shader read only optimal .</summary>
/// <param name="commandBuffer">Command buffer to record the changes against.</param>
void VulkanNeuralSuperResolution::changeInitialImageLayout(pvrvk::CommandBuffer commandBuffer)
{
	// Transition the swapchain images from from e_PRESENT_SRC_KHR layout to e_GENERAL
	pvrvk::MemoryBarrierSet2 swapchainBarrier2;

	pvrvk::ImageMemoryBarrier2 imageMemoryBarrier;

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		imageMemoryBarrier.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_NONE_KHR);
		imageMemoryBarrier.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
		imageMemoryBarrier.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_NONE_KHR);
		imageMemoryBarrier.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_COLOR_ATTACHMENT_WRITE_BIT_KHR);
		imageMemoryBarrier.setOldLayout(pvrvk::ImageLayout::e_UNDEFINED);
		imageMemoryBarrier.setNewLayout(pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
		imageMemoryBarrier.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));
		imageMemoryBarrier.setImage(_deviceResources->offscreenColorAttachmentImage[i]);
		swapchainBarrier2.addBarrier(imageMemoryBarrier);

		imageMemoryBarrier.setImage(_deviceResources->offscreenMotionVectorAttachmentImage[i]);
		swapchainBarrier2.addBarrier(imageMemoryBarrier);

		imageMemoryBarrier.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_NONE_KHR);
		imageMemoryBarrier.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_LATE_FRAGMENT_TESTS_BIT_KHR);
		imageMemoryBarrier.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_NONE_KHR);
		imageMemoryBarrier.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT_KHR);
		imageMemoryBarrier.setNewLayout(pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		imageMemoryBarrier.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_DEPTH_BIT | pvrvk::ImageAspectFlags::e_STENCIL_BIT));
		imageMemoryBarrier.setImage(_deviceResources->offscreenDepthAttachmentImage[i]);
		swapchainBarrier2.addBarrier(imageMemoryBarrier);

		imageMemoryBarrier.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_NONE_KHR);
		imageMemoryBarrier.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_NONE_KHR);
		imageMemoryBarrier.setOldLayout(pvrvk::ImageLayout::e_UNDEFINED);
		imageMemoryBarrier.setNewLayout(pvrvk::ImageLayout::e_PRESENT_SRC_KHR);
		imageMemoryBarrier.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));
		imageMemoryBarrier.setImage(_deviceResources->swapchain->getImage(i));
		imageMemoryBarrier.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_NONE_KHR);
		imageMemoryBarrier.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
		swapchainBarrier2.addBarrier(imageMemoryBarrier);

		imageMemoryBarrier.setSrcAccessMask(pvrvk::AccessFlagBits2KHR::e_2_NONE_KHR);
		imageMemoryBarrier.setDstAccessMask(pvrvk::AccessFlagBits2KHR::e_2_SHADER_READ_BIT_KHR);
		imageMemoryBarrier.setOldLayout(pvrvk::ImageLayout::e_UNDEFINED);
		imageMemoryBarrier.setNewLayout(pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL);
		imageMemoryBarrier.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT));
		imageMemoryBarrier.setImage(_deviceResources->previousFrameResultImage[i]);
		imageMemoryBarrier.setSrcStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_NONE_KHR);
		imageMemoryBarrier.setDstStageMask(pvrvk::PipelineStageFlagBits2KHR::e_2_FRAGMENT_SHADER_BIT_KHR);
		swapchainBarrier2.addBarrier(imageMemoryBarrier);
	}

	commandBuffer->pipelineBarrier2(swapchainBarrier2);
}

/// <summary>Build several uniform buffers used in the sample.</summary>
void VulkanNeuralSuperResolution::createUbos()
{
	// Per frame
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("inverseViewProjectionMatrix", pvr::GpuDatatypes::mat4x4);
		desc.addElement("projectionMatrix", pvr::GpuDatatypes::mat4x4);
		desc.addElement("viewMatrixCurrentFrame", pvr::GpuDatatypes::mat4x4);
		desc.addElement("viewMatrixPreviousFrame", pvr::GpuDatatypes::mat4x4);
		desc.addElement("cameraPosition", pvr::GpuDatatypes::vec3);
		desc.addElement("exposure", pvr::GpuDatatypes::Float);
		desc.addElement("screenWidth", pvr::GpuDatatypes::Integer);
		desc.addElement("screenHeight", pvr::GpuDatatypes::Integer);
		desc.addElement("emissiveIntensity", pvr::GpuDatatypes::Float);
		desc.addElement("textureLODBias", pvr::GpuDatatypes::Float);
		desc.addElement("jitter", pvr::GpuDatatypes::vec2);

		_deviceResources->uboPerFrame.view.initDynamic(desc, _swapchainLength, pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

		_deviceResources->uboPerFrame.buffer = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->uboPerFrame.view.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
		_deviceResources->uboPerFrame.buffer->setObjectName("NNSettingsUBO");

		_deviceResources->uboPerFrame.view.pointToMappedMemory(_deviceResources->uboPerFrame.buffer->getDeviceMemory()->getMappedData());

		glm::vec3 cameraPos = _camera.getCameraPosition();
		for (uint32_t i = 0; i < _swapchainLength; ++i)
		{
			_deviceResources->uboPerFrame.view.getElementByName("screenWidth", 0, i).setValue(_offscreenWidth);
			_deviceResources->uboPerFrame.view.getElementByName("screenHeight", 0, i).setValue(_offscreenHeight);
			_deviceResources->uboPerFrame.view.getElementByName("textureLODBias", 0, i).setValue(_textureLODBias);

			// if the memory property flags used by the buffers' device memory does not contain e_HOST_COHERENT_BIT then we must flush the memory
			if (static_cast<uint32_t>(_deviceResources->uboPerFrame.buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
			{
				_deviceResources->uboPerFrame.buffer->getDeviceMemory()->flushRange(
					_deviceResources->uboPerFrame.view.getDynamicSliceOffset(i), _deviceResources->uboPerFrame.view.getDynamicSliceSize());
			}
		}
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
		_deviceResources->uboLights.view.getElement(2).setValue(_deviceResources->skyBoxPass.getSkyBoxTexture().getNumPrefilteredMipLevels());

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

/// <summary>Build descritpor sets for the textures used in the graphics pass for drawing the environment and the scene mesh.</summary>
void VulkanNeuralSuperResolution::updateDescriptors()
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
			pvrvk::DescriptorImageInfo(
				_deviceResources->skyBoxPass.getSkyBoxTexture().getDiffuseIrradianceMap(), _deviceResources->samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

		// Specular Irradiance
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->descSets[1], 2));
		writeDescSets.back().setImageInfo(0,
			pvrvk::DescriptorImageInfo(
				_deviceResources->skyBoxPass.getSkyBoxTexture().getPrefilteredMap(), _deviceResources->samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

		// Environment map
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->descSets[1], 3));
		writeDescSets.back().setImageInfo(0,
			pvrvk::DescriptorImageInfo(_deviceResources->skyBoxPass.getSkyBoxTexture().getPrefilteredMipMap(), _deviceResources->samplerTrilinearLodClamped,
				pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

		// BRDF LUT
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->descSets[1], 4));
		writeDescSets.back().setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->brdfLUT, _deviceResources->samplerBilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
	}
	// Per object ubo: Material textures.
	{
		// Albedo Map
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->descSets[2], 0));
		writeDescSets.back().setImageInfo(0,
			pvrvk::DescriptorImageInfo(_deviceResources->helmetPass.getPBRMaterial().getAlbedoMap(), _deviceResources->samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->descSets[2], 1));
		writeDescSets.back().setImageInfo(0,
			pvrvk::DescriptorImageInfo(_deviceResources->helmetPass.getPBRMaterial().getOcclusionMetallicRoughnessMap(), _deviceResources->samplerTrilinear,
				pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->descSets[2], 2));
		writeDescSets.back().setImageInfo(0,
			pvrvk::DescriptorImageInfo(_deviceResources->helmetPass.getPBRMaterial().getNormalMap(), _deviceResources->samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->descSets[2], 3));
		writeDescSets.back().setImageInfo(0,
			pvrvk::DescriptorImageInfo(
				_deviceResources->helmetPass.getPBRMaterial().getEmissiveMap(), _deviceResources->samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

		// Materials buffers
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->descSets[2], 4));
		writeDescSets.back().setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->uboMaterial.buffer, 0, _deviceResources->uboMaterial.view.getDynamicSliceSize()));
	}

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanNeuralSuperResolution>(); }
