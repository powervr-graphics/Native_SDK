/*!*********************************************************************************************************************
\File         VulkanImageBasedLighting.cpp
\Title        Introducing the PowerVR Framework
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief        This example demonstrates how to use Physically based rendering using Metallic-Roughness work flow showcasing 2 scenes (helmet and sphere) with Image based lighting
			  (IBL). The Technique presented here is based on Epic Games publication http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
***********************************************************************************************************************/

/*!
IBL Description
	Material: Metallic-Roughness
	============================
	- Albedo map: This is a raw color of the material. This map shouldn't contains any shading information like Ambient Occlusion which is
	very often baked in the diffuse map for phong model.
	It does not only influence the diffuse color, but also the specular color of the material as well.
	When the metallness is one(metallic material) the base color is the specular.

	- MetallicRoughness map: The metallic-roughness texture.
	The metalness values are sampled from the B channel and roughness values are sampled from the G channel, other channels are ignored.

	BRDF
	====
	*Diffuse BRDF: Lambertian diffuse
	f = Cdiff / PI
	Cdiff: Diffuse albedo of the material.

	*Specular BRDF: Cook-Torance
	f = D * F * G / (4 * (N.L) * (N.V));
	D: NDF (Normal Distribution function), It computes the distribution of the microfacets for the shaded surface
	F: Describes how light reflects and refracts at the intersection of two different media (most often in computer graphics : Air and the shaded surface)
	G: Defines the shadowing from the microfacets
	N.L:  is the dot product between the normal of the shaded surface and the light direction.
	N.V is the dot product between the normal of the shaded surface and the view direction.

	IBL workflow
	============
	IBL is one of the most common technique for implmenting global illumination. The idea is that using environmap as light source.

	IBL Diffuse:
	The application load/ generates a diffuse Irradiance map: This is normally done in offline but the code is left here for education
	purpose. Normally when lambert diffuse is used in games, it is the light color multiplied by the visibility factor( N dot L).
	But when using Indirectional lighting (IBL)  the visibility factor is not considered because the light is coming from every where.
	So the diffuse factor is the light color.
	All the pixels in the environment map is a light source, so when shading a point it has to be lit by many pixels from the environment map.
	Sampling multiple texels for shading a single point is not practical for realtime application. Therefore these samples are precomputed
	in the diffuse irradiance map. So at run time it would be a single texture fetch for the given reflection direction.

	IBL Specular & BRDF_LUT:
	Specular reflections looks shiny when the roughness values is low and it becames blurry when the roughness value is high.
	This is encoded in the specular irradiance texture.
	We use the same technique, Split-Sum-Approximation presented by Epics Games, each mip level of this image contains the environment map specular reflectance.
	Mip level 0 contains samples for roughness value 0, and the remaining miplevels get blurry for each mip level as the roughness value increases to 1.

	The samples encoded in this map is the result of the specular BRDF of the environment map. For each pixels in the environemt map,
	computes the Cook-Torrentz microfacet BRDF and stores those results.

	Using the mip map for storing blured images for each roughness value has one draw backs, Specular antialising.
	This happens for the level 0. Since we are using the mip map for different purpose, we can't use mipmapping technique
	to solve the aliasing artifact for high resoultion texture which is level0 of the specular irradiance map.
	Other mip map levels doesn'y have this issue as they are blured and low res.

	To solve this issue we use another texture for doing mipmaping for level 0 of the specular Irradiance map.
*/

#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRCore/cameras/TPSCamera.h"
#include "PVRCore/textureio/TextureWriterPVR.h"
#include "PVRCore/texture/TextureUtils.h"
#include "PVRAssets/fileio/GltfReader.h"

// Content file names
// Shaders
const char VertShaderFileName[] = "VertShader.vsh.spv";
const char SimpleVertShaderFileName[] = "SimpleVertShader.vsh.spv";
const char PBRMaterialFragShaderFileName[] = "PBRMaterialTextureFragShader.fsh.spv";
const char PBRFragShaderFileName[] = "PBRFragShader.fsh.spv";
const char SkyboxVertShaderFileName[] = "SkyboxVertShader.vsh.spv";
const char SkyboxFragShaderFileName[] = "SkyboxFragShader.fsh.spv";
const char IrradianceVertShaderFileName[] = "IrradianceVertShader.vsh.spv";
const char IrradianceFragShaderFileName[] = "IrradianceFragShader.fsh.spv";
const char PreFilterFragShaderFileName[] = "PreFilterFragShader.fsh.spv";

// Scenes
const char HelmetSceneFileName[] = "damagedHelmet.gltf";
const char SphereSceneFileName[] = "sphere.pod";

// Textures
const char SkyboxTexFile[] = "MonValley_baked_lightmap.pvr";
const char DiffuseIrradianceMapTexFile[] = "DiffuseIrradianceMap.pvr";
const char PrefilterEnvMapTexFile[] = "PrefilterEnvMap.pvr";
const char PrefilterL0MipMapTexFile[] = "PrefilterL0MipMap.pvr";
const char BrdfLUTTexFile[] = "brdfLUT.pvr";

const uint32_t IrradianceMapDim = 64;
const uint32_t PrefilterEnvMapDim = 256;

const uint32_t NumSphereRows = 4;
const uint32_t NumSphereColumns = 6;
const uint32_t NumInstances = NumSphereRows * NumSphereColumns;

const bool LoadIrradianceMap = true;
const bool LoadPrefilteredMap = true;
const bool LoadBRDFLUT = true;

const glm::vec3 lightDir[1] = {
	glm::normalize(glm::vec3(0.0f, -0.5f, 0.5f)),
};

struct UBO
{
	pvr::utils::StructuredBufferView view;
	pvrvk::Buffer buffer;
};

struct Pushconstant
{
	uint32_t modelMtxId;
	uint32_t materialIndex;
};

enum class SceneMode
{
	Helmet,
	Sphere,
	NumScenes
};

class SkyBoxPass
{
public:
	void init(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::DescriptorPool& descPool, pvrvk::CommandPool& cmdPool, pvrvk::Queue& queue,
		const pvrvk::RenderPass& renderpass, const pvrvk::PipelineCache& pipelineCache, uint32_t numSwapchains, const pvrvk::Extent2D& viewportDim, const pvrvk::Sampler& sampler,
		pvr::utils::vma::Allocator& allocator, bool loadIrradianceMap, bool loadPrefilteredMap)
	{
		createScreenSpaceQuad(device, allocator);
		createUbo(device, numSwapchains, allocator);

		// create the pipeline layout
		pvrvk::DescriptorSetLayout descSetLayout = initDescriptorSetLayout(device);
		pvrvk::PipelineLayoutCreateInfo pipelineLayoutInfo;
		pipelineLayoutInfo.setDescSetLayout(0, descSetLayout);

		pvrvk::PipelineLayout pipeLayout = device->createPipelineLayout(pipelineLayoutInfo);
		createPipeline(assetProvider, device, renderpass, viewportDim, pipeLayout);

		pvrvk::CommandBuffer cmdBuffer = cmdPool->allocateCommandBuffer();
		cmdBuffer->begin();
		loadTexture(assetProvider, device, cmdBuffer, allocator);

		pvrvk::SubmitInfo submitInfo;
		submitInfo.commandBuffers = &cmdBuffer;
		submitInfo.numCommandBuffers = 1;

		if (!loadIrradianceMap || !loadPrefilteredMap)
		{
			cmdBuffer->end();
			queue->submit(&submitInfo, 1);
			queue->waitIdle();
			cmdBuffer->begin();
		}

		if (loadIrradianceMap)
		{
			irradianceMap = pvr::utils::loadAndUploadImageAndView(device, DiffuseIrradianceMapTexFile, true, cmdBuffer, assetProvider, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
				pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, &allocator, &allocator);
		}
		else
		{
			irradianceMap = device->createImageView(
				pvrvk::ImageViewCreateInfo(generateIrradianceMap(assetProvider, device, cmdPool, descPool, sampler, queue, DiffuseIrradianceMapTexFile, allocator)));
		}
		if (loadPrefilteredMap)
		{
			prefilteredMap = pvr::utils::loadAndUploadImageAndView(device, PrefilterEnvMapTexFile, true, cmdBuffer, assetProvider, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
				pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, &allocator, &allocator);
			prefilteredL0MipMap = pvr::utils::loadAndUploadImageAndView(device, PrefilterL0MipMapTexFile, true, cmdBuffer, assetProvider, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
				pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, &allocator, &allocator);
		}
		else
		{
			pvrvk::Image cubemaps[2];
			generatePrefilteredMap(
				assetProvider, device, cmdPool, descPool, sampler, queue, pipelineCache, PrefilterEnvMapTexFile, PrefilterL0MipMapTexFile, allocator, cubemaps[0], cubemaps[1]);
			prefilteredMap = device->createImageView(pvrvk::ImageViewCreateInfo(cubemaps[0]));
			prefilteredL0MipMap = device->createImageView(pvrvk::ImageViewCreateInfo(cubemaps[1]));
		}

		numPrefilteredMipLevels = prefilteredMap->getImage()->getNumMipLevels();
		cmdBuffer->end();
		queue->submit(&submitInfo, 1);
		queue->waitIdle();

		// create a descriptor set
		descSet = descPool->allocateDescriptorSet(descSetLayout);

		pvrvk::WriteDescriptorSet writeDescSets[2];
		writeDescSets[0]
			.set(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descSet, 0)
			.setImageInfo(0, pvrvk::DescriptorImageInfo(skyBoxMap, sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

		writeDescSets[1].set(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, descSet, 1).setBufferInfo(0, pvrvk::DescriptorBufferInfo(ubo, 0, uboView.getDynamicSliceSize()));

		device->updateDescriptorSets(writeDescSets, ARRAY_SIZE(writeDescSets), nullptr, 0);
	}

	uint32_t getNumPrefilteredMipLevels() const
	{
		return numPrefilteredMipLevels;
	}

	pvrvk::ImageView getDiffuseIrradianceMap()
	{
		return irradianceMap;
	}

	pvrvk::ImageView getPrefilteredMap()
	{
		return prefilteredMap;
	}

	pvrvk::ImageView getPrefilteredMipMap()
	{
		return prefilteredL0MipMap;
	}

	/// <summary>Update Per frame.</summary>
	/// <param name="swapchainIndex">current swapchain index</param>
	/// <param name="invViewProj">inverse view projection matrix.</param>
	/// <param name="eyePos">camera position</param>
	void update(uint32_t swapchainIndex, const glm::mat4& invViewProj, const glm::vec3& eyePos)
	{
		uboView.getElement(0, 0, swapchainIndex).setValue(invViewProj);
		uboView.getElement(1, 0, swapchainIndex).setValue(glm::vec4(eyePos, 0.0f));
		if (uint32_t(ubo->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			ubo->getDeviceMemory()->flushRange(uboView.getDynamicSliceOffset(swapchainIndex), uboView.getDynamicSliceSize());
		}
	}

	/// <summary>Record commands.</summary>
	/// <param name="cmdBuffer">recording commandbuffer</param>
	/// <param name="swapchainIndex">swapchain index.</param>
	void recordCommands(pvrvk::CommandBuffer& cmdBuffer, uint32_t swapchainIndex)
	{
		cmdBuffer->bindPipeline(pipeline);
		cmdBuffer->bindVertexBuffer(vbo, 0, 0);
		uint32_t offset = uboView.getDynamicSliceOffset(swapchainIndex);
		cmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipeline->getPipelineLayout(), 0, descSet, &offset, 1);

		cmdBuffer->draw(0, 6, 0);
	}

private:
	void createScreenSpaceQuad(pvrvk::Device& device, pvr::utils::vma::Allocator& allocator)
	{
		const float quadVertices[] = {
			-1, -1, 1.f, // upper left
			-1, 1, 1.f, // lower left
			1, -1, 1.f, // upper right
			1, -1, 1.f, // upper right
			-1, 1, 1.f, // lower left
			1, 1, 1.f // lower right
		};

		vbo = pvr::utils::createBuffer(device, sizeof(quadVertices), pvrvk::BufferUsageFlags::e_VERTEX_BUFFER_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, &allocator);
		pvr::utils::updateHostVisibleBuffer(vbo, quadVertices, 0, sizeof(quadVertices), true);
	}

	/// <summary>Create uniform buffer objects</summary>
	/// <param name="device">The device the vulkan resources allocated from.</param>
	/// <param name="numSwapchains">Number of swapchains.</param>
	/// <param name="allocator">buffer memory allocator.</param>
	void createUbo(pvrvk::Device& device, uint32_t numSwapchains, pvr::utils::vma::Allocator& allocator)
	{
		// create the structured memory view
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("InvVPMatrix", pvr::GpuDatatypes::mat4x4);
		desc.addElement("EyePos", pvr::GpuDatatypes::vec4);

		uboView.initDynamic(desc, numSwapchains, pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

		ubo = pvr::utils::createBuffer(device, uboView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, &allocator);
		uboView.pointToMappedMemory(ubo->getDeviceMemory()->getMappedData());
	}

	/// <summary>Load skybox texture and its lightmap</summary>
	/// <param name="device">The device the vulkan resources allocated from.</param>
	/// <param name="uploadCmdBuffer">Commandbuffer used for uploading the textures.</param>
	/// <param name="assetProvider">Assetprovider for loading textures from disk.</param>
	/// <param name="allocator">Image memory alloator.</param>
	void loadTexture(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::CommandBuffer& uploadCmdBuffer, pvr::utils::vma::Allocator& allocator)
	{
		// load the  skybox texture
		skyBoxMap = device->createImageView(pvrvk::ImageViewCreateInfo(pvr::utils::loadAndUploadImage(device, SkyboxTexFile, true, uploadCmdBuffer, assetProvider,
			pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, &allocator, &allocator)));
	}

	pvrvk::DescriptorSetLayout initDescriptorSetLayout(pvrvk::Device& device)
	{
		// create skybox descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayout;

		// combined image sampler descriptor
		descSetLayout.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

		// uniform buffer
		descSetLayout.setBinding(1, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);

		return device->createDescriptorSetLayout(descSetLayout);
	}

	void createPipeline(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, const pvrvk::RenderPass& renderpass, const pvrvk::Extent2D& viewportDim,
		const pvrvk::PipelineLayout& pipelineLayout)
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

		// input assembler
		pipeInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);

		// vertex attributes and bindings
		pipeInfo.vertexInput.clear();
		pipeInfo.vertexInput.addInputBinding(pvrvk::VertexInputBindingDescription(0, sizeof(float) * 3));
		pipeInfo.vertexInput.addInputAttribute(pvrvk::VertexInputAttributeDescription(0, 0, pvrvk::Format::e_R32G32B32_SFLOAT, 0));

		pipeInfo.viewport.setViewportAndScissor(0, pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(viewportDim.getWidth()), static_cast<float>(viewportDim.getHeight())),
			pvrvk::Rect2D(0, 0, viewportDim.getWidth(), viewportDim.getHeight()));

		pipeline = device->createGraphicsPipeline(pipeInfo);
	}

	/// <summary>Generate specular irradiance map. Each level of the specular mip map get blured corresponding the roughness value from 0 to 1.0.
	/// The biggest disadvantage to is that we are losing the actual mipmap ing technique and the top mipmap level produces shimmerish artifacts.
	/// Apart from reducing its resolution and loosing its quality we using separate texture with mip mapping for level 0.
	/// So in the fragment shader, sample from the mip mapped texture if the material's roughness value is below 1.0 and mixes with the prefiltered map fixes this issue.
	/// see file: PBRFragShader.fsh  function: prefilteredReflection </summary>
	/// <param name="device">Device the resource allocated from.</param>
	/// <param name="cmdPool">upload commandbuffer allocation pool.</param>
	/// <param name="descPool">descriptor set allocation pool.</param>
	/// <param name="sampler">Trilinear sampler</param>
	/// <param name="queue">upload submission queue.</param>
	/// <param name="assetProvider">Asset provider to read/ write from disk.</param>
	/// <param name="prefilterEnvMapFile">name of the prefiltered map file</param>
	/// <param name="prefilteredEnvL0MipMap">name of the level 0 mipmap file</param>
	/// <param name="allocator">vulkan memory allocator for image and buffers</param>
	/// <param name="outPreFilteredCubeMap">returned prefiltered map</param>
	/// <param name="outMipCubeMap">returned mipmap texture.</param>
	void generatePrefilteredMap(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::CommandPool& cmdPool, pvrvk::DescriptorPool& descPool,
		const pvrvk::Sampler& sampler, pvrvk::Queue& queue, const pvrvk::PipelineCache& pipelineCache, const char* prefilterEnvMapFile, const char* prefilteredEnvL0MipMap,
		pvr::utils::vma::Allocator& allocator, pvrvk::Image& outPreFilteredCubeMap, pvrvk::Image& outMipCubeMap)
	{
		const uint32_t numFaces = 6;
		pvrvk::Image renderTarget[2]; // fbo attachments, prfiltered map and mipmaping texture
		pvrvk::ImageView imageView[2];

		// Discard the last two mipmaps. From our experimentation keeping the last miplevel 4x4 avoids blocky texel artifacts for materials with roughness values of 1.0.
		const uint32_t DISCARD_SPECULAR_MIP_LEVELS = 2;

		// calculate number of mip map levels
		const uint32_t numMipLevels = static_cast<uint32_t>(log2(static_cast<float>(PrefilterEnvMapDim)) + 1.0f - DISCARD_SPECULAR_MIP_LEVELS); // prefilterMap

		struct PushConstant
		{
			float roughnesess;
		};

		// calculate level dimensions.
		std::vector<uint32_t> mipLevelDimensions(numMipLevels);
		for (size_t i = 0; i < mipLevelDimensions.size(); ++i)
		{
			mipLevelDimensions[i] = static_cast<uint32_t>(pow(2, numMipLevels + DISCARD_SPECULAR_MIP_LEVELS - i - 1));
		}

		pvrvk::CommandBuffer cmdBuffer = cmdPool->allocateCommandBuffer();

		// Create descriptor set
		// create skybox descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayout;

		descSetLayout.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // skybox
		descSetLayout.setBinding(1, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT); // uniform buffer
		pvrvk::DescriptorSetLayout setLayout = device->createDescriptorSetLayout(descSetLayout);
		pvrvk::DescriptorSet descSet = descPool->allocateDescriptorSet(setLayout);

		///////////////////////////////////////////////
		// UBO BUFFER AND VIEW
		pvr::utils::StructuredBufferView uboView;
		pvr::utils::StructuredMemoryDescription viewDesc;
		viewDesc.addElement("rotateMtx", pvr::GpuDatatypes::mat4x4);
		uboView.initDynamic(viewDesc, numFaces, pvr::BufferUsageFlags::UniformBuffer, device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment());

		pvrvk::Buffer uboBuffer = pvr::utils::createBuffer(device, uboView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, &allocator);
		uboView.pointToMappedMemory(uboBuffer->getDeviceMemory()->getMappedData());

		// To draw the skybox, we are using a full screen quad with different oritenation matrices for each faces.
		// Face matrices.
		const glm::mat4 cubeView[numFaces] = {
			glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(90.f), glm::vec3(0.0f, 1.0f, 0.f)), // +X
			glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(-90.f), glm::vec3(0.0f, 1.0f, 0.f)), // -X
			glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(90.f), glm::vec3(1.0f, .0f, 0.f)), // +Y
			glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(-90.f), glm::vec3(1.0f, .0f, 0.f)), // -Y
			glm::scale(glm::vec3(1.0f, -1.0f, 1.f)), // +Z
			glm::scale(glm::vec3(-1.0f, -1.0f, -1.f)), // -Z
		};

		uboView.getElement(0, 0, 0).setValue(cubeView[0]);
		uboView.getElement(0, 0, 1).setValue(cubeView[1]);
		uboView.getElement(0, 0, 2).setValue(cubeView[2]);
		uboView.getElement(0, 0, 3).setValue(cubeView[3]);
		uboView.getElement(0, 0, 4).setValue(cubeView[4]);
		uboView.getElement(0, 0, 5).setValue(cubeView[5]);

		if (uint32_t(uboBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			uboBuffer->getDeviceMemory()->flushRange();
		}

		const pvrvk::ClearValue clearValues[] = {
			pvrvk::ClearValue(0.f, 0.f, 0.f, 0.0f),
			pvrvk::ClearValue(0.f, 0.f, 0.f, 0.0f),
		};

		const pvrvk::Format format = pvrvk::Format::e_R8G8B8A8_UNORM;
		const uint32_t stride = sizeof(uint8_t) * 4;

		pvrvk::RenderPass renderpass;
		pvrvk::Framebuffer fbo;
		pvrvk::GraphicsPipeline pipelines;

		cmdBuffer->begin(pvrvk::CommandBufferUsageFlags::e_ONE_TIME_SUBMIT_BIT);
		cmdBuffer->bindVertexBuffer(vbo, 0, 0);

		// Create two buffers (size=numFaces * stride * maxImageDimension)for each cubemaps

		const VkDeviceSize bufferSize = numMipLevels * 6 * stride * mipLevelDimensions[0] * mipLevelDimensions[0];

		pvrvk::Buffer imageDataBuffer[2] = { pvr::utils::createBuffer(device, bufferSize, pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
												 pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, &allocator),

			pvr::utils::createBuffer(device, bufferSize, pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
				pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, &allocator) };

		pvrvk::WriteDescriptorSet descSetUpdate[] = {
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descSet, 0),
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, descSet, 1),
		};
		descSetUpdate[0].setImageInfo(0, pvrvk::DescriptorImageInfo(skyBoxMap, sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
		descSetUpdate[1].setBufferInfo(0, pvrvk::DescriptorBufferInfo(uboBuffer, 0, uboView.getDynamicSliceSize()));

		device->updateDescriptorSets(descSetUpdate, ARRAY_SIZE(descSetUpdate), nullptr, 0);

		// prefiltered target
		renderTarget[0] = pvr::utils::createImage(device, pvrvk::ImageType::e_2D, format, pvrvk::Extent3D(mipLevelDimensions[0], mipLevelDimensions[0], 1),
			pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT, pvrvk::ImageCreateFlags::e_NONE, pvrvk::ImageLayersSize(1, 1),
			pvrvk::SampleCountFlags::e_1_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, &allocator);

		// mip target
		renderTarget[1] = pvr::utils::createImage(device, pvrvk::ImageType::e_2D, format, pvrvk::Extent3D(mipLevelDimensions[0], mipLevelDimensions[0], 1),
			pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT, pvrvk::ImageCreateFlags::e_NONE, pvrvk::ImageLayersSize(1, 1),
			pvrvk::SampleCountFlags::e_1_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, &allocator);

		imageView[0] = device->createImageView(pvrvk::ImageViewCreateInfo(renderTarget[0]));
		imageView[1] = device->createImageView(pvrvk::ImageViewCreateInfo(renderTarget[1]));

		// create the cubemap image

		outPreFilteredCubeMap = pvr::utils::createImage(device, pvrvk::ImageType::e_2D, format, pvrvk::Extent3D(mipLevelDimensions[0], mipLevelDimensions[0], 1),
			pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_DST_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
			pvrvk::ImageCreateFlags::e_CUBE_COMPATIBLE_BIT, pvrvk::ImageLayersSize(6, numMipLevels), pvrvk::SampleCountFlags::e_1_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, &allocator);
		pvr::utils::setImageLayout(outPreFilteredCubeMap, pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, cmdBuffer);

		outMipCubeMap = pvr::utils::createImage(device, pvrvk::ImageType::e_2D, format, pvrvk::Extent3D(mipLevelDimensions[0], mipLevelDimensions[0], 1),
			pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_DST_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
			pvrvk::ImageCreateFlags::e_CUBE_COMPATIBLE_BIT, pvrvk::ImageLayersSize(6, numMipLevels), pvrvk::SampleCountFlags::e_1_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, &allocator);
		pvr::utils::setImageLayout(outMipCubeMap, pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, cmdBuffer);

		// create the renderpass
		pvrvk::RenderPassCreateInfo rpInfo;
		rpInfo.setAttachmentDescription(0,
			pvrvk::AttachmentDescription::createColorDescription(
				format, pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::AttachmentLoadOp::e_DONT_CARE, pvrvk::AttachmentStoreOp::e_STORE));

		rpInfo.setAttachmentDescription(1,
			pvrvk::AttachmentDescription::createColorDescription(
				format, pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::AttachmentLoadOp::e_DONT_CARE, pvrvk::AttachmentStoreOp::e_STORE));

		pvrvk::SubpassDescription subpassDesc;
		subpassDesc.setColorAttachmentReference(0, pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
		subpassDesc.setColorAttachmentReference(1, pvrvk::AttachmentReference(1, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));

		const pvrvk::SubpassDependency dependencies[] = {

			pvrvk::SubpassDependency(pvrvk::SubpassExternal, 0, pvrvk::PipelineStageFlags::e_TOP_OF_PIPE_BIT, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT,
				pvrvk::AccessFlags::e_MEMORY_READ_BIT, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_READ_BIT | pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT,
				pvrvk::DependencyFlags::e_BY_REGION_BIT),

			pvrvk::SubpassDependency(0, pvrvk::SubpassExternal, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_BOTTOM_OF_PIPE_BIT,
				pvrvk::AccessFlags::e_COLOR_ATTACHMENT_READ_BIT | pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_MEMORY_READ_BIT,
				pvrvk::DependencyFlags::e_BY_REGION_BIT),

			pvrvk::SubpassDependency(pvrvk::SubpassExternal, 1, pvrvk::PipelineStageFlags::e_TOP_OF_PIPE_BIT, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT,
				pvrvk::AccessFlags::e_MEMORY_READ_BIT, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_READ_BIT | pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT,
				pvrvk::DependencyFlags::e_BY_REGION_BIT),

			pvrvk::SubpassDependency(1, pvrvk::SubpassExternal, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_BOTTOM_OF_PIPE_BIT,
				pvrvk::AccessFlags::e_COLOR_ATTACHMENT_READ_BIT | pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_MEMORY_READ_BIT,
				pvrvk::DependencyFlags::e_BY_REGION_BIT),

		};
		rpInfo.addSubpassDependencies(dependencies, ARRAY_SIZE(dependencies));
		rpInfo.setSubpass(0, subpassDesc);
		renderpass = device->createRenderPass(rpInfo);

		// Create the fbo
		pvrvk::FramebufferCreateInfo fboInfo;
		fboInfo.setAttachment(0, imageView[0]); // prefilterd map
		fboInfo.setAttachment(1, imageView[1]); // mip map
		fboInfo.setDimensions(mipLevelDimensions[0], mipLevelDimensions[0]);
		fboInfo.setRenderPass(renderpass);
		fbo = device->createFramebuffer(fboInfo);

		pvrvk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.depthStencil.enableAllStates(false);

		pipelineInfo.vertexShader = device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(IrradianceVertShaderFileName)->readToEnd<uint32_t>()));
		pipelineInfo.fragmentShader = device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(PreFilterFragShaderFileName)->readToEnd<uint32_t>()));

		// depth stencil state
		pipelineInfo.depthStencil.enableDepthWrite(false);
		pipelineInfo.depthStencil.enableDepthTest(false);

		// rasterizer state
		pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);
		pipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);
		// blend state
		pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
		pipelineInfo.colorBlend.setAttachmentState(1, pvrvk::PipelineColorBlendAttachmentState());
		// input assembler
		pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);
		pipelineInfo.renderPass = renderpass;

		// vertex attributes and bindings
		pipelineInfo.vertexInput.clear();
		pipelineInfo.vertexInput.addInputBinding(pvrvk::VertexInputBindingDescription(0, sizeof(float) * 3));
		pipelineInfo.vertexInput.addInputAttribute(pvrvk::VertexInputAttributeDescription(0, 0, pvrvk::Format::e_R32G32B32_SFLOAT, 0));
		pipelineInfo.viewport.setViewportAndScissor(0, pvrvk::Viewport(), pvrvk::Rect2D());

		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(descSet->getDescriptorSetLayout());
		pipeLayoutInfo.setPushConstantRange(0, pvrvk::PushConstantRange(pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, 0, sizeof(PushConstant)));

		pipelineInfo.pipelineLayout = device->createPipelineLayout(pipeLayoutInfo);
		pipelineInfo.dynamicStates.setDynamicState(pvrvk::DynamicState::e_VIEWPORT, true);
		pipelineInfo.dynamicStates.setDynamicState(pvrvk::DynamicState::e_SCISSOR, true);

		pipelines = device->createGraphicsPipeline(pipelineInfo, pipelineCache);

		cmdBuffer->debugMarkerBeginEXT("Generate cubemap");
		cmdBuffer->bindPipeline(pipelines);

		VkDeviceSize bufferOffset = 0;
		for (uint32_t level = 0; level < numMipLevels; ++level)
		{
			uint32_t dim = mipLevelDimensions[level];
			cmdBuffer->debugMarkerBeginEXT("Cubemap level");
			// set the viewport of the current dimension
			cmdBuffer->setViewport(pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(dim), static_cast<float>(dim)));
			const pvrvk::Rect2D scissor(pvrvk::Offset2D(), pvrvk::Extent2D(dim, dim));
			cmdBuffer->setScissor(0, 1, &scissor);

			// set the roughness value
			const PushConstant pushConst = { static_cast<float>(level) / static_cast<float>(numMipLevels) };
			cmdBuffer->pushConstants(pipelines->getPipelineLayout(), pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, 0, sizeof(pushConst), &pushConst);

			// draw each face
			for (uint32_t j = 0; j < numFaces; ++j)
			{
				uint32_t offset = uboView.getDynamicSliceOffset(j); // select the right orientation matrix
				cmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipelines->getPipelineLayout(), 0, descSet, &offset, 1);

				// calculate the mip level dimension
				cmdBuffer->beginRenderPass(fbo, true, clearValues, ARRAY_SIZE(clearValues));

				cmdBuffer->draw(0, 6);
				cmdBuffer->endRenderPass();

				// prepear both attachment for transfer src read operation.
				pvrvk::MemoryBarrierSet barriers;
				barriers.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_MEMORY_READ_BIT, pvrvk::AccessFlags::e_TRANSFER_READ_BIT, renderTarget[0],
					pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL,
					queue->getFamilyIndex(), queue->getFamilyIndex()));

				barriers.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_MEMORY_READ_BIT, pvrvk::AccessFlags::e_TRANSFER_READ_BIT, renderTarget[1],
					pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL,
					queue->getFamilyIndex(), queue->getFamilyIndex()));

				cmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_BOTTOM_OF_PIPE_BIT, pvrvk::PipelineStageFlags::e_TRANSFER_BIT, barriers);

				// copy it in to the buffer for  writing the texture in to a file.
				const pvrvk::BufferImageCopy regions(bufferOffset, 0, 0, pvrvk::ImageSubresourceLayers(), pvrvk::Offset3D(), pvrvk::Extent3D(dim, dim, 1));
				cmdBuffer->copyImageToBuffer(renderTarget[0], pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, imageDataBuffer[0], &regions, 1);
				cmdBuffer->copyImageToBuffer(renderTarget[1], pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, imageDataBuffer[1], &regions, 1);

				// copy it in to the final out image
				const pvrvk::ImageCopy blit(pvrvk::ImageSubresourceLayers(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0, 0, 1), pvrvk::Offset3D(),
					pvrvk::ImageSubresourceLayers(pvrvk::ImageAspectFlags::e_COLOR_BIT, level, j, 1), pvrvk::Offset3D(), pvrvk::Extent3D(dim, dim, 1));

				cmdBuffer->copyImage(renderTarget[0], outPreFilteredCubeMap, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, 1, &blit);
				cmdBuffer->copyImage(renderTarget[1], outMipCubeMap, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, 1, &blit);

				barriers.clearAllBarriers();
				barriers.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_TRANSFER_READ_BIT, pvrvk::AccessFlags::e_NONE, renderTarget[0],
					pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL,
					queue->getFamilyIndex(), queue->getFamilyIndex()));

				barriers.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_TRANSFER_READ_BIT, pvrvk::AccessFlags::e_NONE, renderTarget[1],
					pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL,
					queue->getFamilyIndex(), queue->getFamilyIndex()));

				cmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_TRANSFER_BIT, pvrvk::PipelineStageFlags::e_TOP_OF_PIPE_BIT, barriers);
				bufferOffset += stride * dim * dim;
			}
			cmdBuffer->debugMarkerEndEXT();
		}
		pvr::utils::setImageLayout(outPreFilteredCubeMap, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, cmdBuffer);
		pvr::utils::setImageLayout(outMipCubeMap, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, cmdBuffer);

		cmdBuffer->debugMarkerEndEXT();

		cmdBuffer->end();

		pvrvk::SubmitInfo submitInfo;
		submitInfo.commandBuffers = &cmdBuffer;
		submitInfo.numCommandBuffers = 1;

		pvrvk::Fence fence = device->createFence();
		queue->submit(submitInfo, fence);
		fence->wait();
		// Read the diffuse irradiance texture from the buffer and save it in to the file.
		void* data;

		// Read the specular irradiance texture from the buffer and save it in to the file.
		if (imageDataBuffer[0].isValid())
		{
			data = imageDataBuffer[0]->getDeviceMemory()->getMappedData();
			pvr::TextureHeader texHeader;
			texHeader.setChannelType(pvr::VariableType::UnsignedByteNorm);
			texHeader.setColorSpace(pvr::ColorSpace::lRGB);
			texHeader.setDepth(1);
			texHeader.setWidth(mipLevelDimensions[0]);
			texHeader.setHeight(mipLevelDimensions[0]);
			texHeader.setNumMipMapLevels(numMipLevels);
			texHeader.setNumFaces(numFaces);
			texHeader.setNumArrayMembers(1);
			texHeader.setPixelFormat(pvr::PixelFormat::RGBA_8888());

			pvr::Texture tex(texHeader, static_cast<const char*>(data));

			pvr::Stream::ptr_type fileStream = pvr::FileStream::createFileStream(prefilterEnvMapFile, "w");
			pvr::assetWriters::TextureWriterPVR writerPVR;
			writerPVR.openAssetStream(std::move(fileStream));
			writerPVR.writeAsset(tex);
		}

		if (imageDataBuffer[1].isValid())
		{
			data = imageDataBuffer[1]->getDeviceMemory()->getMappedData();
			pvr::TextureHeader texHeader;
			texHeader.setChannelType(pvr::VariableType::UnsignedByteNorm);
			texHeader.setColorSpace(pvr::ColorSpace::lRGB);
			texHeader.setDepth(1);
			texHeader.setWidth(mipLevelDimensions[0]);
			texHeader.setHeight(mipLevelDimensions[0]);
			texHeader.setNumMipMapLevels(numMipLevels);
			texHeader.setNumFaces(numFaces);
			texHeader.setNumArrayMembers(1);
			texHeader.setPixelFormat(pvr::PixelFormat::RGBA_8888());

			pvr::Texture tex(texHeader, static_cast<const char*>(data));

			pvr::Stream::ptr_type fileStream = pvr::FileStream::createFileStream(prefilteredEnvL0MipMap, "w");
			pvr::assetWriters::TextureWriterPVR writerPVR;
			writerPVR.openAssetStream(std::move(fileStream));

			writerPVR.writeAsset(tex);
		}
	}

	// Generate the Diffuse and specular irradiance map.
	pvrvk::Image generateIrradianceMap(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::CommandPool& cmdPool, pvrvk::DescriptorPool& descPool,
		const pvrvk::Sampler& sampler, pvrvk::Queue& queue, const char* irradianceCubeMapFile, pvr::utils::vma::Allocator& allocator)
	{
		const uint32_t numFaces = 6;
		pvrvk::Image renderTarget;
		pvrvk::Image cubeMap;

		uint32_t numMipLevels = static_cast<uint32_t>(log2(static_cast<float>(IrradianceMapDim)) + 1.0);

		// calculate the mip level dimensions.
		std::vector<uint32_t> mipLevelDimensions(numMipLevels);
		for (uint32_t i = 0; i < mipLevelDimensions.size(); ++i)
		{
			mipLevelDimensions[i] = static_cast<uint32_t>(pow(2, numMipLevels - i - 1));
		}

		pvrvk::CommandBuffer cmdBuffer = cmdPool->allocateCommandBuffer();

		// create skybox descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;

		// combined image sampler descriptor
		descSetLayoutInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		// uniform buffer
		descSetLayoutInfo.setBinding(1, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);

		pvrvk::DescriptorSetLayout setLayout = device->createDescriptorSetLayout(descSetLayoutInfo);
		// Create the descriptor set
		pvrvk::DescriptorSet descSet = descPool->allocateDescriptorSet(setLayout);

		///////////////////////////////////////////////
		// UBO BUFFER AND VIEW
		pvr::utils::StructuredBufferView uboView;
		pvr::utils::StructuredMemoryDescription viewDesc;
		viewDesc.addElement("rotateMtx", pvr::GpuDatatypes::mat4x4);
		uboView.initDynamic(viewDesc, numFaces, pvr::BufferUsageFlags::UniformBuffer, device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment());

		pvrvk::Buffer uboBuffer = pvr::utils::createBuffer(device, uboView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, &allocator);
		uboView.pointToMappedMemory(uboBuffer->getDeviceMemory()->getMappedData());

		const glm::mat4 cubeView[numFaces] = {
			glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(90.f), glm::vec3(0.0f, 1.0f, 0.f)), // +X
			glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(-90.f), glm::vec3(0.0f, 1.0f, 0.f)), // -X
			glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(90.f), glm::vec3(1.0f, .0f, 0.f)), // +Y
			glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(-90.f), glm::vec3(1.0f, .0f, 0.f)), // -Y
			glm::scale(glm::vec3(1.0f, -1.0f, 1.f)), // +Z
			glm::scale(glm::vec3(-1.0f, -1.0f, -1.f)), // -Z
		};

		uboView.getElement(0, 0, 0).setValue(cubeView[0]);
		uboView.getElement(0, 0, 1).setValue(cubeView[1]);
		uboView.getElement(0, 0, 2).setValue(cubeView[2]);
		uboView.getElement(0, 0, 3).setValue(cubeView[3]);
		uboView.getElement(0, 0, 4).setValue(cubeView[4]);
		uboView.getElement(0, 0, 5).setValue(cubeView[5]);

		if (uint32_t(uboBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			uboBuffer->getDeviceMemory()->flushRange();
		}

		const pvrvk::ClearValue clearValues[] = {
			pvrvk::ClearValue(1.f, 1.f, 0.f, 1.0f),
			pvrvk::ClearValue(1.f, 1.f, 0.f, 1.0f),
			pvrvk::ClearValue(1.f, 1.f, 0.f, 1.0f),
			pvrvk::ClearValue(1.f, 1.f, 0.f, 1.0f),
			pvrvk::ClearValue(1.f, 1.f, 0.f, 1.0f),
			pvrvk::ClearValue(1.f, 1.f, 0.f, 1.0f),
		};

		const pvrvk::Format formats = { pvrvk::Format::e_R8G8B8A8_UNORM };
		const uint32_t stride = sizeof(uint8_t) * 4;
		pvrvk::ImageView imageViews = {};
		pvrvk::RenderPass renderpass;
		pvrvk::Framebuffer fbo;
		// Create two buffers (size=numFaces * stride * maxImageDimension)for each cubemaps

		const uint32_t bufferSize = mipLevelDimensions[0] * mipLevelDimensions[0] * 6 * stride * numMipLevels;

		// create the image data buffer for cpu read.
		pvrvk::Buffer imageDataBuffer = pvr::utils::createBuffer(device, static_cast<VkDeviceSize>(bufferSize), pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT,
			pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, &allocator);

		pvrvk::WriteDescriptorSet descSetUpdate[] = {
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descSet, 0),
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, descSet, 1),
		};
		descSetUpdate[0].setImageInfo(0, pvrvk::DescriptorImageInfo(skyBoxMap, sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)); // environment map
		descSetUpdate[1].setBufferInfo(0, pvrvk::DescriptorBufferInfo(uboBuffer, 0, uboView.getDynamicSliceSize())); // ubo

		device->updateDescriptorSets(descSetUpdate, ARRAY_SIZE(descSetUpdate), nullptr, 0);

		// create the cubemap image
		pvrvk::Format format = formats;
		renderTarget = pvr::utils::createImage(device, pvrvk::ImageType::e_2D, format, pvrvk::Extent3D(IrradianceMapDim, IrradianceMapDim, 1),
			pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT, pvrvk::ImageCreateFlags::e_NONE, pvrvk::ImageLayersSize(1, 1),
			pvrvk::SampleCountFlags::e_1_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, &allocator);

		imageViews = device->createImageView(pvrvk::ImageViewCreateInfo(renderTarget));

		cubeMap = pvr::utils::createImage(device, pvrvk::ImageType::e_2D, format, pvrvk::Extent3D(IrradianceMapDim, IrradianceMapDim, 1),
			pvrvk::ImageUsageFlags::e_TRANSFER_DST_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageCreateFlags::e_CUBE_COMPATIBLE_BIT,
			pvrvk::ImageLayersSize(6, numMipLevels), pvrvk::SampleCountFlags::e_1_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, &allocator);

		// create the renderpass
		pvrvk::RenderPassCreateInfo rpInfo;
		rpInfo.setAttachmentDescription(0,
			pvrvk::AttachmentDescription::createColorDescription(
				format, pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::AttachmentLoadOp::e_DONT_CARE, pvrvk::AttachmentStoreOp::e_STORE));

		pvrvk::SubpassDescription subpassDesc;
		subpassDesc.setColorAttachmentReference(0, pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));

		const pvrvk::SubpassDependency dependencies[] = {

			pvrvk::SubpassDependency(pvrvk::SubpassExternal, 0, pvrvk::PipelineStageFlags::e_TOP_OF_PIPE_BIT, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT,
				pvrvk::AccessFlags::e_TRANSFER_READ_BIT, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_READ_BIT | pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT,
				pvrvk::DependencyFlags::e_BY_REGION_BIT),

			pvrvk::SubpassDependency(0, pvrvk::SubpassExternal, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_BOTTOM_OF_PIPE_BIT,
				pvrvk::AccessFlags::e_COLOR_ATTACHMENT_READ_BIT | pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_TRANSFER_READ_BIT,
				pvrvk::DependencyFlags::e_BY_REGION_BIT),
		};
		rpInfo.addSubpassDependencies(dependencies, ARRAY_SIZE(dependencies));
		rpInfo.setSubpass(0, subpassDesc);
		renderpass = device->createRenderPass(rpInfo);

		// Create the fbo
		pvrvk::FramebufferCreateInfo fboInfo;
		fboInfo.setAttachment(0, imageViews);
		fboInfo.setDimensions(IrradianceMapDim, IrradianceMapDim);
		fboInfo.setRenderPass(renderpass);
		fbo = device->createFramebuffer(fboInfo);

		// create graphics pipeline.
		pvrvk::GraphicsPipeline pipeline;
		pvrvk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.depthStencil.enableAllStates(false);

		pipelineInfo.vertexShader = device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(IrradianceVertShaderFileName)->readToEnd<uint32_t>()));
		pipelineInfo.fragmentShader = device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(IrradianceFragShaderFileName)->readToEnd<uint32_t>()));

		// depth stencil state
		pipelineInfo.depthStencil.enableDepthWrite(false);
		pipelineInfo.depthStencil.enableDepthTest(false);

		// rasterizer state
		pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);
		pipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);
		// blend state
		pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
		// input assembler
		pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);
		pipelineInfo.renderPass = renderpass;

		// vertex attributes and bindings
		pipelineInfo.vertexInput.clear();
		pipelineInfo.vertexInput.addInputBinding(pvrvk::VertexInputBindingDescription(0, sizeof(float) * 3));
		pipelineInfo.vertexInput.addInputAttribute(pvrvk::VertexInputAttributeDescription(0, 0, pvrvk::Format::e_R32G32B32_SFLOAT, 0));
		pipelineInfo.viewport.setViewportAndScissor(0, pvrvk::Viewport(), pvrvk::Rect2D());

		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(descSet->getDescriptorSetLayout());

		pipelineInfo.pipelineLayout = device->createPipelineLayout(pipeLayoutInfo);
		pipelineInfo.dynamicStates.setDynamicState(pvrvk::DynamicState::e_VIEWPORT, true);
		pipelineInfo.dynamicStates.setDynamicState(pvrvk::DynamicState::e_SCISSOR, true);

		pipeline = device->createGraphicsPipeline(pipelineInfo);

		// record commands
		cmdBuffer->begin(pvrvk::CommandBufferUsageFlags::e_ONE_TIME_SUBMIT_BIT);

		// prepare the cubemap for trasnfer dest.
		pvr::utils::setImageLayout(cubeMap, pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, cmdBuffer);

		cmdBuffer->bindVertexBuffer(vbo, 0, 0);
		cmdBuffer->debugMarkerBeginEXT("Generate cubemap");
		cmdBuffer->bindPipeline(pipeline);

		VkDeviceSize bufferOffset = 0;
		for (uint32_t level = 0; level < numMipLevels; ++level)
		{
			uint32_t dim = mipLevelDimensions[level];
			cmdBuffer->debugMarkerBeginEXT("Cubemap level");
			cmdBuffer->setViewport(pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(dim), static_cast<float>(dim)));
			const pvrvk::Rect2D scissor(pvrvk::Offset2D(), pvrvk::Extent2D(dim, dim));
			cmdBuffer->setScissor(0, 1, &scissor);

			// draw each face
			for (uint32_t j = 0; j < numFaces; ++j)
			{
				uint32_t offset = uboView.getDynamicSliceOffset(j);
				cmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipeline->getPipelineLayout(), 0, descSet, &offset, 1);

				// calculate the mip level dimension
				cmdBuffer->beginRenderPass(fbo, true, clearValues, ARRAY_SIZE(clearValues));

				cmdBuffer->draw(0, 6);
				cmdBuffer->endRenderPass();

				// Prepare the render target for src transfer.
				pvrvk::MemoryBarrierSet barriers;

				// copy it in to the buffer.
				const pvrvk::BufferImageCopy regions(bufferOffset, 0, 0, pvrvk::ImageSubresourceLayers(), pvrvk::Offset3D(), pvrvk::Extent3D(dim, dim, 1));
				cmdBuffer->copyImageToBuffer(renderTarget, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, imageDataBuffer, &regions, 1);

				// copy it in to the final image
				const pvrvk::ImageCopy blit(pvrvk::ImageSubresourceLayers(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0, 0, 1), pvrvk::Offset3D(),
					pvrvk::ImageSubresourceLayers(pvrvk::ImageAspectFlags::e_COLOR_BIT, level, j, 1), pvrvk::Offset3D(), pvrvk::Extent3D(dim, dim, 1));

				cmdBuffer->copyImage(renderTarget, cubeMap, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, 1, &blit);

				barriers.clearAllBarriers();
				barriers.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_TRANSFER_READ_BIT, pvrvk::AccessFlags::e_NONE, renderTarget,
					pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL,
					queue->getFamilyIndex(), queue->getFamilyIndex()));
				cmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_TRANSFER_BIT, pvrvk::PipelineStageFlags::e_TOP_OF_PIPE_BIT, barriers);
				bufferOffset += stride * dim * dim;
			}
			cmdBuffer->debugMarkerEndEXT();
		}
		pvr::utils::setImageLayout(cubeMap, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, cmdBuffer);
		cmdBuffer->debugMarkerEndEXT();

		cmdBuffer->end();

		pvrvk::SubmitInfo submitInfo;
		submitInfo.commandBuffers = &cmdBuffer;
		submitInfo.numCommandBuffers = 1;

		pvrvk::Fence fence = device->createFence();
		queue->submit(submitInfo, fence);
		fence->wait();
		// Read the diffuse irradiance texture from the buffer and save it in to the file.
		void* data;
		if (imageDataBuffer.isValid())
		{
			data = imageDataBuffer->getDeviceMemory()->getMappedData();
			// save the map.
			pvr::TextureHeader texHeader;
			texHeader.setChannelType(pvr::VariableType::UnsignedByteNorm);
			texHeader.setColorSpace(pvr::ColorSpace::lRGB);
			texHeader.setDepth(1);
			texHeader.setWidth(IrradianceMapDim);
			texHeader.setHeight(IrradianceMapDim);
			texHeader.setNumMipMapLevels(numMipLevels);
			texHeader.setNumFaces(6);
			texHeader.setNumArrayMembers(1);
			texHeader.setPixelFormat(pvr::PixelFormat::RGBA_8888());

			pvr::Texture tex(texHeader, static_cast<const char*>(data));
			pvr::Stream::ptr_type fileStream = pvr::FileStream::createFileStream(irradianceCubeMapFile, "w");
			pvr::assetWriters::TextureWriterPVR writerPVR;
			writerPVR.openAssetStream(std::move(fileStream));

			writerPVR.writeAsset(tex);
		}
		return cubeMap;
	}

	pvrvk::GraphicsPipeline pipeline;
	pvrvk::ImageView skyBoxMap;
	pvrvk::ImageView irradianceMap, prefilteredMap, prefilteredL0MipMap;
	pvrvk::DescriptorSet descSet;
	pvrvk::Buffer vbo;
	pvrvk::Buffer ibo;
	pvr::utils::StructuredBufferView uboView;
	pvrvk::Buffer ubo;
	uint32_t numPrefilteredMipLevels;
};

class SpherePass
{
public:
	/// <summary>initialise the sphere's pipeline</summary>
	/// <param name="device">Device for the resource to create from</param>
	/// <param name="assetProvider">Asset provider for loading assets from disk.</param>
	/// <param name="basePipeline">Handle to base pipeline</param>
	/// <param name="uploadCmdBuffer">initial image uploading command buffer</param>
	/// <param name="requireSubmission">Returns boolean flag whether the command buffer need submission.</param>
	void init(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, const pvrvk::GraphicsPipeline& basePipeline, const pvrvk::PipelineCache& pipelineCache,
		pvr::utils::vma::Allocator& allocator, pvrvk::CommandBuffer& uploadCmdBuffer, bool& requireSubmission)
	{
		scene = pvr::assets::Model::createWithReader(pvr::assets::PODReader(assetProvider.getAssetStream(SphereSceneFileName)));

		pvr::utils::appendSingleBuffersFromModel(device, *scene, vbos, ibos, uploadCmdBuffer, requireSubmission, &allocator);

		createPipeline(assetProvider, device, basePipeline, pipelineCache);
	}

	/// <summary>Record commands for rendering the sphere scene</summary>
	/// <param name="cmdBuffer">A command buffer to which sphere commands will be added</param>
	void recordCommands(pvrvk::CommandBuffer& cmdBuffer)
	{
		cmdBuffer->bindPipeline(pipeline);
		uint32_t one = 1;
		cmdBuffer->pushConstants(pipeline->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, 0,
			static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::uinteger)), &one);
		cmdBuffer->pushConstants(pipeline->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT,
			static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::uinteger)), static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::uinteger)), &one);
		for (uint32_t i = 0; i < scene->getNumMeshNodes(); ++i)
		{
			pvr::assets::Node& node = scene->getMeshNode(i);
			pvr::assets::Mesh& mesh = scene->getMesh(static_cast<uint32_t>(node.getObjectId()));

			cmdBuffer->bindVertexBuffer(vbos[i], 0, 0);
			cmdBuffer->bindIndexBuffer(ibos[i], 0, mesh.getFaces().getDataType() == pvr::IndexType::IndexType16Bit ? pvrvk::IndexType::e_UINT16 : pvrvk::IndexType::e_UINT32);
			cmdBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, NumInstances);
		}
	}

private:
	void createPipeline(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, const pvrvk::GraphicsPipeline& basePipeline, const pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::GraphicsPipelineCreateInfo pipeDesc = basePipeline->getCreateInfo();
		pipeDesc.basePipeline = basePipeline;
		pvr::utils::VertexBindings bindingName[] = { { "POSITION", 0 }, { "NORMAL", 1 } };

		pipeDesc.vertexInput.clear();
		pvr::utils::populateInputAssemblyFromMesh(scene->getMesh(0), bindingName, ARRAY_SIZE(bindingName), pipeDesc.vertexInput, pipeDesc.inputAssembler);

		pipeDesc.vertexShader = device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(SimpleVertShaderFileName)->readToEnd<uint32_t>()));

		pipeDesc.fragmentShader = device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(PBRFragShaderFileName)->readToEnd<uint32_t>()));

		pipeline = device->createGraphicsPipeline(pipeDesc, pipelineCache);
	}

	pvr::assets::ModelHandle scene;
	std::vector<pvrvk::Buffer> vbos;
	std::vector<pvrvk::Buffer> ibos;
	pvrvk::GraphicsPipeline pipeline;
};

class HelmetPass
{
public:
	void init(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, const pvrvk::Framebuffer& framebuffer, const pvrvk::PipelineLayout& pipelineLayout,
		const pvrvk::PipelineCache& pipelineCache, pvr::utils::vma::Allocator& allocator, pvrvk::CommandBuffer& uploadCmdBuffer, bool requireSubmission)
	{
		scene = pvr::assets::Model::createWithReader(pvr::assets::GltfReader(assetProvider.getAssetStream(HelmetSceneFileName), assetProvider));

		// create the vbo and ibo for the meshes.
		vbos.resize(scene->getNumMeshes());
		ibos.resize(scene->getNumMeshes());

		pvr::utils::createSingleBuffersFromMesh(device, scene->getMesh(0), vbos[0], ibos[0], uploadCmdBuffer, requireSubmission, &allocator);

		// Load the texture
		loadTextures(assetProvider, device, uploadCmdBuffer, allocator);

		createPipeline(assetProvider, device, framebuffer, pipelineLayout, pipelineCache);
	}

	const pvrvk::GraphicsPipeline& getPipeline()
	{
		return pipeline;
	}

	pvr::assets::ModelHandle& getScene()
	{
		return scene;
	}

	const pvrvk::ImageView& getAlbedoMap()
	{
		return images[0];
	}

	const pvrvk::ImageView& getMetallicRoughnessMap()
	{
		return images[1];
	}

	const pvrvk::ImageView& getNormalMap()
	{
		return images[2];
	}

	const pvrvk::ImageView& getEmissiveMap()
	{
		return images[3];
	}

	void recordCommands(pvrvk::CommandBuffer& cmd)
	{
		cmd->bindPipeline(pipeline);

		const uint32_t numMeshes = scene->getNumMeshes();
		// set the model matrix and material id.
		uint32_t zero = 0;
		cmd->pushConstants(pipeline->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, 0,
			static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::uinteger)), &zero);
		cmd->pushConstants(pipeline->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT,
			static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::uinteger)), static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::uinteger)), &zero);

		for (uint32_t j = 0; j < numMeshes; ++j)
		{
			const pvr::assets::Mesh& mesh = scene->getMesh(j);
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
		pvr::utils::VertexBindings bindingName[] = { { "POSITION", 0 }, { "NORMAL", 1 }, { "UV0", 2 }, { "TANGENT", 3 } };

		pvr::utils::populateViewportStateCreateInfo(framebuffer, pipeDesc.viewport);
		pvr::utils::populateInputAssemblyFromMesh(getScene()->getMesh(0), bindingName, ARRAY_SIZE(bindingName), pipeDesc.vertexInput, pipeDesc.inputAssembler);

		pipeDesc.vertexShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(VertShaderFileName)->readToEnd<uint32_t>())));
		pipeDesc.fragmentShader.setShader(
			device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(PBRMaterialFragShaderFileName)->readToEnd<uint32_t>())));

		pipeDesc.renderPass = framebuffer->getRenderPass();
		pipeDesc.depthStencil.enableDepthTest(true);
		pipeDesc.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);
		pipeDesc.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_LESS);
		pipeDesc.depthStencil.enableDepthWrite(true);
		pipeDesc.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT).setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);
		pipeDesc.subpass = 0;

		pipeDesc.pipelineLayout = pipelineLayout;

		pipeline = device->createGraphicsPipeline(pipeDesc);
	}

	void loadTextures(pvr::IAssetProvider& assetProvider, pvrvk::Device& device, pvrvk::CommandBuffer& uploadCmdBuffer, pvr::utils::vma::Allocator& allocator)
	{
		for (uint32_t i = 0; i < scene->getNumTextures(); ++i)
		{
			pvr::Stream::ptr_type stream = assetProvider.getAssetStream(scene->getTexture(i).getName());
			pvr::Texture tex = pvr::textureLoad(stream, pvr::TextureFileFormat::PVR);
			images.push_back(pvr::utils::uploadImageAndView(device, tex, true, uploadCmdBuffer, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
				pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, &allocator, &allocator, pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT));
		}
	}

	std::vector<pvrvk::ImageView> images;
	std::vector<pvrvk::Buffer> vbos;
	std::vector<pvrvk::Buffer> ibos;
	pvr::assets::ModelHandle scene;
	pvrvk::GraphicsPipeline pipeline;
};

/*!*********************************************************************************************************************
 Class implementing the pvr::Shell functions.
***********************************************************************************************************************/
class VulkanIBL : public pvr::Shell
{
	typedef std::pair<int32_t, pvrvk::DescriptorSet> MaterialDescSet;

	enum DescSetIndex
	{
		PerFrame,
		Scene,
		Material,
	};

	struct DeviceResources
	{
		pvrvk::Instance instance;
		pvrvk::Surface surface;
		pvrvk::Device device;
		pvrvk::Swapchain swapchain;
		pvr::utils::vma::Allocator vmaAllocator;
		pvr::Multi<pvrvk::ImageView> depthStencilImages;
		pvrvk::Queue queue;

		pvrvk::CommandPool commandPool;
		pvrvk::DescriptorPool descriptorPool;

		pvrvk::Semaphore semaphoreImageAcquired[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Fence perFrameAcquireFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Semaphore semaphorePresent[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Fence perFrameCommandBufferFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

		// the framebuffer used in the demo
		pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;

		// main command buffer used to store rendering commands
		pvr::Multi<pvrvk::CommandBuffer> commandBuffers;

		// Pipeline cache
		pvrvk::PipelineCache pipelineCache;

		// descriptor sets
		pvrvk::DescriptorSet descSets[3];

		// structured memory views
		UBO uboPerFrame, uboLights, uboMaterial, uboWorld;

		// samplers
		pvrvk::Sampler samplerBilinear, samplerTrilinear;

		// descriptor set layouts
		pvrvk::DescriptorSetLayout descSetLayouts[3];

		pvrvk::PipelineLayout pipelineLayout;

		pvrvk::ImageView brdfLUT;

		pvr::ui::UIRenderer uiRenderer;

		SkyBoxPass skyBoxPass;
		HelmetPass helmetPass;
		SpherePass spherePass;

		~DeviceResources()
		{
			if (device.isValid())
			{
				device->waitIdle();
			}
			uint32_t l = swapchain->getSwapchainLength();
			for (uint32_t i = 0; i < l; ++i)
			{
				if (perFrameAcquireFence[i].isValid())
					perFrameAcquireFence[i]->wait();
				if (perFrameCommandBufferFence[i].isValid())
					perFrameCommandBufferFence[i]->wait();
			}
		}
	};

	std::unique_ptr<DeviceResources> _deviceResources;

	bool _updateCommands[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	// Projection and Model View matrices
	glm::mat4 _projMtx;
	// Variables to handle the animation in a time-based manner
	float _frame;
	uint32_t _frameId;

	pvr::TPSCamera _camera;
	SceneMode _currentScene;
	bool _pause = false;
	float _cameraLookAngle = 0.0f;

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void createDescriptorSetLayouts();
	void createUbos();
	void createDescriptorSets();
	void recordCommandBuffers(uint32_t swapIndex);
	void createPipelineLayout();
	void generateBRDFLUT(pvrvk::CommandBuffer uploadCmdBufffer);

	virtual void eventMappedInput(pvr::SimplifiedInput action)
	{
		switch (action)
		{
		case pvr::SimplifiedInput::Left:
		{
			uint32_t currentScene = static_cast<uint32_t>(_currentScene);
			currentScene -= 1;
			currentScene = (currentScene + static_cast<uint32_t>(SceneMode::NumScenes)) % static_cast<uint32_t>(SceneMode::NumScenes);
			_currentScene = static_cast<SceneMode>(currentScene);
			memset(_updateCommands, 1, sizeof(_updateCommands));
			break;
		}
		case pvr::SimplifiedInput::Right:
		{
			uint32_t currentScene = static_cast<uint32_t>(_currentScene);
			currentScene += 1;
			currentScene = (currentScene + static_cast<uint32_t>(SceneMode::NumScenes)) % static_cast<uint32_t>(SceneMode::NumScenes);
			_currentScene = static_cast<SceneMode>(currentScene);
			memset(_updateCommands, 1, sizeof(_updateCommands));
			break;
		}
		case pvr::SimplifiedInput::Action1:
		{
			_pause = !_pause;
			break;
		}
		case pvr::SimplifiedInput::ActionClose:
		{
			this->exitShell();
			break;
		}
		default:
			break;
		}
	}
};

/// <summary>
/// Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.). If the rendering
/// context is lost, initApplication() will not be called again.
/// </summary>
pvr::Result VulkanIBL::initApplication()
{
	_frame = 0;
	_frameId = 0;
	_currentScene = SceneMode::Helmet;
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by Shell once per run, just before exiting the program.
/// quitApplication() will not be called every time the rendering context is lost, only before application exit.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanIBL::quitApplication()
{
	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context(e.g.textures, vertex buffers, etc.)</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanIBL::initView()
{
	_deviceResources = std::unique_ptr<DeviceResources>(new DeviceResources());

	// Create vulkan instance and surface
	_deviceResources->instance = pvr::utils::createInstance(this->getApplicationName());
	_deviceResources->surface = pvr::utils::createSurface(_deviceResources->instance, _deviceResources->instance->getPhysicalDevice(0), this->getWindow(), this->getDisplay());

	pvrvk::PhysicalDevice physicalDevice = _deviceResources->instance->getPhysicalDevice(0);

	// Populate queue for rendering and transfer operation
	const pvr::utils::QueuePopulateInfo queuePopulateInfo = { pvrvk::QueueFlags::e_GRAPHICS_BIT, _deviceResources->surface };

	// Create the device and queue
	pvr::utils::QueueAccessInfo queueAccessInfo;
	_deviceResources->device = pvr::utils::createDeviceAndQueues(physicalDevice, &queuePopulateInfo, 1, &queueAccessInfo);

	// Get the queue
	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	// validate the supported swapchain image usage for src trasfer option for capturing screenshots.
	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice->getSurfaceCapabilities(_deviceResources->surface);
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT; // Transfer operation supported.
	}

	// initialse the vma allocator
	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	// Create the swapchain and depth-stencil image views
	pvr::utils::createSwapchainAndDepthStencilImageAndViews(_deviceResources->device, _deviceResources->surface, getDisplayAttributes(), _deviceResources->swapchain,
		_deviceResources->depthStencilImages, swapchainImageUsage, pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT,
		&_deviceResources->vmaAllocator);

	// Create the framebuffer
	pvrvk::RenderPass rp;
	pvr::utils::createOnscreenFramebufferAndRenderpass(_deviceResources->swapchain, &_deviceResources->depthStencilImages[0], _deviceResources->onScreenFramebuffer, rp);

	// Create the Commandpool & Descriptorpool
	_deviceResources->commandPool =
		_deviceResources->device->createCommandPool(pvrvk::CommandPoolCreateInfo(queueAccessInfo.familyId, pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));
	if (!_deviceResources->commandPool.isValid())
	{
		return pvr::Result::UnknownError;
	}

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 16)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 16)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, 16)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_STORAGE_IMAGE, 2)
																						  .setMaxDescriptorSets(16));

	if (!_deviceResources->descriptorPool.isValid())
	{
		return pvr::Result::UnknownError;
	}

	// Create synchronization objects and commandbuffers
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->semaphorePresent[i] = _deviceResources->device->createSemaphore();
		_deviceResources->semaphoreImageAcquired[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameCommandBufferFence[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->perFrameAcquireFence[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->commandBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_updateCommands[i] = true;
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

	_deviceResources->commandBuffers[0]->begin();

	if (LoadBRDFLUT)
	{
		_deviceResources->brdfLUT = _deviceResources->device->createImageView(
			pvrvk::ImageViewCreateInfo(pvr::utils::loadAndUploadImage(_deviceResources->device, BrdfLUTTexFile, true, _deviceResources->commandBuffers[0], *this,
				pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, &_deviceResources->vmaAllocator, &_deviceResources->vmaAllocator)));
	}
	else
	{
		generateBRDFLUT(_deviceResources->commandBuffers[0]);
	}

	createDescriptorSetLayouts();
	createPipelineLayout();

	bool requireSubmission = false;

	_deviceResources->skyBoxPass.init(*this, _deviceResources->device, _deviceResources->descriptorPool, _deviceResources->commandPool, _deviceResources->queue,
		_deviceResources->onScreenFramebuffer[0]->getRenderPass(), _deviceResources->pipelineCache, _deviceResources->swapchain->getSwapchainLength(),
		pvrvk::Extent2D(getWidth(), getHeight()), _deviceResources->samplerTrilinear, _deviceResources->vmaAllocator, LoadIrradianceMap, LoadPrefilteredMap);

	_deviceResources->helmetPass.init(*this, _deviceResources->device, _deviceResources->onScreenFramebuffer[0], _deviceResources->pipelineLayout, _deviceResources->pipelineCache,
		_deviceResources->vmaAllocator, _deviceResources->commandBuffers[0], requireSubmission);

	_deviceResources->spherePass.init(*this, _deviceResources->device, _deviceResources->helmetPass.getPipeline(), _deviceResources->pipelineCache, _deviceResources->vmaAllocator,
		_deviceResources->commandBuffers[0], requireSubmission);

	createUbos();
	createDescriptorSets();

	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queue);

	_deviceResources->commandBuffers[0]->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->commandBuffers[0];
	submitInfo.numCommandBuffers = 1;
	// submit the queue and wait for it to become idle
	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle();
	_deviceResources->commandBuffers[0]->reset(pvrvk::CommandBufferResetFlags::e_RELEASE_RESOURCES_BIT);

	// Calculates the projection matrix
	bool isRotated = this->isScreenRotated() && this->isFullScreen();
	if (isRotated)
	{
		_projMtx = pvr::math::perspective(
			pvr::Api::Vulkan, glm::radians(45.f), static_cast<float>(this->getHeight()) / static_cast<float>(this->getWidth()), 1.f, 2000.f, glm::pi<float>() * .5f);
	}
	else
	{
		_projMtx = pvr::math::perspective(pvr::Api::Vulkan, glm::radians(45.f), static_cast<float>(this->getWidth()) / static_cast<float>(this->getHeight()), 1.f, 2000.f);
	}

	_deviceResources->uiRenderer.getDefaultTitle()->setText("ImageBasedLighting").commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText("Left / Right: to change the scene\n"
															   "Action 1: Enable/Disable Animation\n");
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	// setup the camera
	_camera.setDistanceFromTarget(60.f);
	_camera.setHeight(10.f);

	return pvr::Result::Success;
}

/// <summary>Generates a BRDF integration LUT which stores roughness/ nDotV</summary>
/// <param name="uploadCmdBufffer">Command buffer to which commands may be recorded for loading/generating the BRDF LUT.</param>
/// <param name="loadFromFile">Determines whether regeneration should occur.</param>
void VulkanIBL::generateBRDFLUT(pvrvk::CommandBuffer uploadCmdBufffer)
{
	pvr::Texture tex;
	pvr::assets::generateBRDFLUT(tex);
	_deviceResources->brdfLUT = pvr::utils::uploadImageAndView(_deviceResources->device, tex, true, uploadCmdBufffer, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
		pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, &_deviceResources->vmaAllocator, &_deviceResources->vmaAllocator);
}

/// <summary>Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanIBL::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame</summary>
/// <returns>Result::Success if no error occurred.</summary>
pvr::Result VulkanIBL::renderFrame()
{
	static float emissiveScale = 0.0f;
	static float emissiveStrength = 1.;

	_deviceResources->perFrameAcquireFence[_frameId]->wait();
	_deviceResources->perFrameAcquireFence[_frameId]->reset();

	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->semaphoreImageAcquired[_frameId], _deviceResources->perFrameAcquireFence[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameCommandBufferFence[swapchainIndex]->wait();
	_deviceResources->perFrameCommandBufferFence[swapchainIndex]->reset();

	// Rerecord the commandbuffer if the scene has changed.
	if (_updateCommands[swapchainIndex])
	{
		recordCommandBuffers(swapchainIndex);
		_updateCommands[swapchainIndex] = false;
	}

	emissiveStrength += .15f;
	if (emissiveStrength >= glm::pi<float>())
	{
		emissiveStrength = 0.0f;
	}

	emissiveScale = std::abs(glm::cos(emissiveStrength)) + .75f;

	if (!_pause)
	{
		_cameraLookAngle += 0.15f;
	}

	if (_cameraLookAngle >= 360.0f)
	{
		_cameraLookAngle = _cameraLookAngle - 360.f;
	}
	if (!_pause)
	{
		_camera.setTargetLookAngle(_cameraLookAngle);
	}

	glm::mat4 viewMtx = _camera.getViewMatrix();
	glm::vec3 cameraPos = _camera.getCameraPosition();

	// update the matrix uniform buffer
	{
		// only update the current swapchain ubo
		const glm::mat4 tempMtx = _projMtx * viewMtx;
		_deviceResources->uboPerFrame.view.getElement(0, 0, swapchainIndex).setValue(viewMtx); // view matrix
		_deviceResources->uboPerFrame.view.getElement(1, 0, swapchainIndex).setValue(tempMtx); // view proj
		_deviceResources->uboPerFrame.view.getElement(2, 0, swapchainIndex).setValue(cameraPos); // camera position.
		_deviceResources->uboPerFrame.view.getElement(3, 0, swapchainIndex).setValue(emissiveScale);

		// flush if the buffer memory doesn't support host coherent.
		if (uint32_t(_deviceResources->uboPerFrame.buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_deviceResources->uboPerFrame.buffer->getDeviceMemory()->flushRange(
				_deviceResources->uboPerFrame.view.getDynamicSliceOffset(swapchainIndex), _deviceResources->uboPerFrame.view.getDynamicSliceSize());
		}
	}

	// update the skybox
	_deviceResources->skyBoxPass.update(swapchainIndex, glm::inverse(_projMtx * viewMtx), cameraPos);

	// submit the commandbuffer
	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags waitStage = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.commandBuffers = &_deviceResources->commandBuffers[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitDestStages = &waitStage;
	submitInfo.waitSemaphores = &_deviceResources->semaphoreImageAcquired[_frameId]; // wait for the acquire to be finished.
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->semaphorePresent[_frameId]; // signal the semaphore when it is finish with rendering to the swapchain.
	submitInfo.numSignalSemaphores = 1;

	// submit
	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameCommandBufferFence[swapchainIndex]);

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->swapchain, swapchainIndex, _deviceResources->commandPool, _deviceResources->queue, this->getScreenshotFileName(),
			&_deviceResources->vmaAllocator, &_deviceResources->vmaAllocator);
	}

	// present
	pvrvk::PresentInfo presentInfo;
	presentInfo.waitSemaphores = &_deviceResources->semaphorePresent[_frameId];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.numSwapchains = 1;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.imageIndices = &swapchainIndex;
	_deviceResources->queue->present(presentInfo);

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  Pre-record the rendering commands
***********************************************************************************************************************/
void VulkanIBL::recordCommandBuffers(uint32_t swapIndex)
{
	const pvrvk::ClearValue clearValues[] = { pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 1.0f), pvrvk::ClearValue(1.f, 0) };

	// begin recording commands
	_deviceResources->commandBuffers[swapIndex]->begin();

	// begin the renderpass
	_deviceResources->commandBuffers[swapIndex]->beginRenderPass(
		_deviceResources->onScreenFramebuffer[swapIndex], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, clearValues, ARRAY_SIZE(clearValues));

	// Render the sky box
	_deviceResources->skyBoxPass.recordCommands(_deviceResources->commandBuffers[swapIndex], swapIndex);

	uint32_t offsets[1];
	// get the matrix array offset
	offsets[0] = _deviceResources->uboPerFrame.view.getDynamicSliceOffset(swapIndex);

	// bind the descriptor sets
	_deviceResources->commandBuffers[swapIndex]->bindDescriptorSets(
		pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->pipelineLayout, 0, _deviceResources->descSets, ARRAY_SIZE(_deviceResources->descSets), offsets, 1);

	if (_currentScene == SceneMode::Helmet)
	{
		_deviceResources->helmetPass.recordCommands(_deviceResources->commandBuffers[swapIndex]);
	}
	else
	{
		_deviceResources->spherePass.recordCommands(_deviceResources->commandBuffers[swapIndex]);
	}

	// record the ui renderer.
	_deviceResources->uiRenderer.beginRendering(_deviceResources->commandBuffers[swapIndex]);
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.endRendering();

	_deviceResources->commandBuffers[swapIndex]->endRenderPass();
	_deviceResources->commandBuffers[swapIndex]->end();
}

void VulkanIBL::createDescriptorSetLayouts()
{
	// Create the descriptor set layouts

	// dynamic ubo
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // binding 0
		descSetInfo.setBinding(1, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT); // binding 1
		_deviceResources->descSetLayouts[DescSetIndex::PerFrame] = _deviceResources->device->createDescriptorSetLayout(descSetInfo);
	}

	// create the ubo descriptor set layouts
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // binding 0
		descSetInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // binding 2: Diffuse irradianceMap
		descSetInfo.setBinding(2, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // binding 3: Specular irradianceMap
		descSetInfo.setBinding(3, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // binding 4: Specular irradiance mip Map
		descSetInfo.setBinding(4, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // binding 5: brdfLUTmap
		_deviceResources->descSetLayouts[DescSetIndex::Scene] = _deviceResources->device->createDescriptorSetLayout(descSetInfo);
	}

	// textures
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // binding 0: albedo
		descSetInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // binding 1: metallicRoughness
		descSetInfo.setBinding(2, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // binding 2: emissiveTex
		descSetInfo.setBinding(3, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // binding 3: normal
		descSetInfo.setBinding(4, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); // binding 1
		_deviceResources->descSetLayouts[DescSetIndex::Material] = _deviceResources->device->createDescriptorSetLayout(descSetInfo);
	}
}

void VulkanIBL::createPipelineLayout()
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

/*!*********************************************************************************************************************
\brief  Creates the buffers used throughout the demo.
***********************************************************************************************************************/
void VulkanIBL::createUbos()
{
	// Per frame
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("view", pvr::GpuDatatypes::mat4x4);
		desc.addElement("MVP", pvr::GpuDatatypes::mat4x4);
		desc.addElement("camPos", pvr::GpuDatatypes::vec3);
		desc.addElement("emissiveScale", pvr::GpuDatatypes::Float);

		_deviceResources->uboPerFrame.view.initDynamic(desc, _deviceResources->swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

		const pvrvk::DeviceSize size = _deviceResources->uboPerFrame.view.getSize();
		_deviceResources->uboPerFrame.buffer =
			pvr::utils::createBuffer(_deviceResources->device, size, pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
				pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, &_deviceResources->vmaAllocator);

		_deviceResources->uboPerFrame.view.pointToMappedMemory(_deviceResources->uboPerFrame.buffer->getDeviceMemory()->getMappedData());
	}

	// World matrices (Helmet and spheres)
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("model", pvr::GpuDatatypes::mat4x4, NumInstances + 1);
		desc.addElement("modelInvTranspose", pvr::GpuDatatypes::mat3x3, NumInstances + 1);

		_deviceResources->uboWorld.view.init(desc);

		const pvrvk::DeviceSize size = _deviceResources->uboWorld.view.getSize();
		_deviceResources->uboWorld.buffer =
			pvr::utils::createBuffer(_deviceResources->device, size, pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
				pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, &_deviceResources->vmaAllocator);
		_deviceResources->uboWorld.view.pointToMappedMemory(_deviceResources->uboWorld.buffer->getDeviceMemory()->getMappedData());
		// set the helmet matrix

		glm::mat4 model = glm::eulerAngleXY(glm::radians(0.f), glm::radians(120.f)) * glm::scale(glm::vec3(22.0f));
		_deviceResources->uboWorld.view.getElement(0).setValue(model);
		_deviceResources->uboWorld.view.getElement(1).setValue(glm::inverseTranspose(model));

		// set the sphere matrices
		float positionOffsetX = -25.f;
		float positionOffsetY = 15.f;

		for (uint32_t i = 0; i < NumInstances; ++i)
		{
			if ((i % NumSphereColumns) == 0)
			{
				positionOffsetX = -25.0f;
			}

			if ((i != 0) && (i % NumSphereColumns == 0))
			{
				positionOffsetY -= 10.f;
			}

			const glm::mat4 model = glm::translate(glm::vec3(positionOffsetX, positionOffsetY, 0.0f)) * glm::scale(glm::vec3(4.5f));
			positionOffsetX += 10.f;
			_deviceResources->uboWorld.view.getElement(0, i + 1).setValue(model);
			_deviceResources->uboWorld.view.getElement(1, i + 1).setValue(glm::inverseTranspose(model));
		}

		if ((_deviceResources->uboWorld.buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_deviceResources->uboWorld.buffer->getDeviceMemory()->flushRange();
		}
	}

	// Ubo lights
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("lights", pvr::GpuDatatypes::vec3);
		desc.addElement("numSpecularIrrMapMipLevels", pvr::GpuDatatypes::uinteger);

		_deviceResources->uboLights.view.init(desc);
		const pvrvk::DeviceSize size = _deviceResources->uboLights.view.getSize();
		_deviceResources->uboLights.buffer =
			pvr::utils::createBuffer(_deviceResources->device, size, pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
				pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, &_deviceResources->vmaAllocator);

		_deviceResources->uboLights.view.pointToMappedMemory(_deviceResources->uboLights.buffer->getDeviceMemory()->getMappedData());

		_deviceResources->uboLights.view.getElement(0).setValue(lightDir[0]);
		_deviceResources->uboLights.view.getElement(1).setValue(_deviceResources->skyBoxPass.getNumPrefilteredMipLevels());

		if (uint32_t(_deviceResources->uboLights.buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_deviceResources->uboLights.buffer->getDeviceMemory()->flushRange();
		}
	}

	// ubo material
	{
		const pvr::utils::StructuredMemoryDescription materialDesc("material", NumInstances + 1,
			{ { "roughness", pvr::GpuDatatypes::Float }, { "metallic", pvr::GpuDatatypes::Float }, { "rgb", pvr::GpuDatatypes::vec3 },
				{ "hasMaterialTextures", pvr::GpuDatatypes::boolean } });

		_deviceResources->uboMaterial.view.init(pvr::utils::StructuredMemoryDescription("materials", 1, { materialDesc }));

		_deviceResources->uboMaterial.buffer = pvr::utils::createBuffer(_deviceResources->device, _deviceResources->uboMaterial.view.getSize(),
			pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, &_deviceResources->vmaAllocator);

		_deviceResources->uboMaterial.view.pointToMappedMemory(_deviceResources->uboMaterial.buffer->getDeviceMemory()->getMappedData());

		// update the material buffer
		pvr::assets::Material& material = _deviceResources->helmetPass.getScene()->getMaterial(0);
		pvr::assets::Material::GLTFMetallicRoughnessSemantics metallicRoughness(material);

		// Helmet material
		auto helmetView = _deviceResources->uboMaterial.view.getElement(0, 0);
		helmetView.getElement(0).setValue(metallicRoughness.getRoughness());
		helmetView.getElement(1).setValue(metallicRoughness.getMetallicity());
		helmetView.getElement(2).setValue(metallicRoughness.getBaseColor());
		helmetView.getElement(3).setValue(true);

		// Spheres materials

		// offset the posittion for each sphere instances
		const glm::vec3 color[] = {
			glm::vec3(0.971519, 0.959915, 0.915324), // Silver Metallic
			glm::vec3(1, 0.765557, 0.336057), // Gold Metallic
			glm::vec3(1.0), // White Plastic
			glm::vec3(.05f, .05f, 0.3), // Yellow Plastic
		};

		const float roughness[NumSphereColumns] = { 1.f, 0.8f, 0.45f, 0.3f, 0.15f, 0.0f };

		// set the per sphere materiual property.
		for (uint32_t i = 0; i < NumSphereRows; ++i)
		{
			for (uint32_t j = 0; j < NumSphereColumns; ++j)
			{
				auto sphereView = _deviceResources->uboMaterial.view.getElement(0, i * NumSphereColumns + j + 1);
				sphereView.getElement(0).setValue(roughness[j]);
				sphereView.getElement(1).setValue(float(i < 2) * 1.0f); // set the first 2 row metalicity and the remaining to 0.0
				sphereView.getElement(2).setValue(color[i]);
				sphereView.getElement(3).setValue(false);
			}
		}

		if ((_deviceResources->uboMaterial.buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_deviceResources->uboMaterial.buffer->getDeviceMemory()->flushRange();
		}
	}
}

/*!*********************************************************************************************************************
\brief  Create combined texture and sampler descriptor set for the materials in the _scene
\return Return true on success
***********************************************************************************************************************/
void VulkanIBL::createDescriptorSets()
{
	// Update the descriptor sets

	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;
	// PerFrame
	{
		_deviceResources->descSets[0] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->descSetLayouts[0]);
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->descSets[0], 0));
		writeDescSets.back().setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->uboPerFrame.buffer, 0, _deviceResources->uboPerFrame.view.getDynamicSliceSize()));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->descSets[0], 1));
		writeDescSets.back().setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->uboWorld.buffer, 0, _deviceResources->uboWorld.view.getSize()));
	}
	// Per Scene
	{
		_deviceResources->descSets[1] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->descSetLayouts[1]);

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

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->descSets[1], 3));
		writeDescSets.back().setImageInfo(
			0, pvrvk::DescriptorImageInfo(_deviceResources->skyBoxPass.getPrefilteredMipMap(), _deviceResources->samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

		// BRDF LUT
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->descSets[1], 4));
		writeDescSets.back().setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->brdfLUT, _deviceResources->samplerBilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
	}
	// Material Textures
	{
		_deviceResources->descSets[2] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->descSetLayouts[2]);

		// Albedo Map
		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->descSets[2], 0));
		writeDescSets.back().setImageInfo(
			0, pvrvk::DescriptorImageInfo(_deviceResources->helmetPass.getAlbedoMap(), _deviceResources->samplerBilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->descSets[2], 1));
		writeDescSets.back().setImageInfo(0,
			pvrvk::DescriptorImageInfo(_deviceResources->helmetPass.getMetallicRoughnessMap(), _deviceResources->samplerBilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

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
std::unique_ptr<pvr::Shell> pvr::newDemo()
{
	return std::unique_ptr<pvr::Shell>(new VulkanIBL());
}
