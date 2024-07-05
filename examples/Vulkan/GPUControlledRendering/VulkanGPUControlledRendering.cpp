/*!
\brief Shows how to perform GPU Controlled Rendering using compute and indirect drawing
\file VulkanGpuControlledRendering.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"

#define M_PI 3.14159265358979323846 // pi
const float RotateY = glm::pi<float>() / 150;

// Grid size for number of total instance elements ( sphere + torus )
#define INSTANCES 5
#define XSIZE INSTANCES
#define YSIZE INSTANCES
#define ZSIZE INSTANCES
#define NUM_INSTANCES_PER_DRAW XSIZE* YSIZE* ZSIZE
#define SCENE_NUM_MESHES 2 // num gltf sub-meshes in the scene
#define TOTAL_NUM_INSTANCES NUM_INSTANCES_PER_DRAW* SCENE_NUM_MESHES

// Source and binary shaders
const char ForwardPassIndirectDrawFragShaderSrcFile[] = "FragShader.fsh.spv";
const char ForwardPassIndirectDrawVertShaderSrcFile[] = "VertShader.vsh.spv";
const char IndirectCullCompShaderFileName[] = "IndirectCullCompute.csh.spv";
const char OnScreenQuadFrag[] = "FullScreenQuadFrag.fsh.spv";
const char OnScreenQuadVert[] = "FullScreenQuadVert.vsh.spv";

// PVR texture files
const std::string sphereTexFileName = "sphereTex";
const std::string torusTexFileName = "torusTex";

// gltf _scene files, contains 2 sub meshes( sphere, torus )
const char SceneFile[] = "sphereTorus.gltf";

// light constants
const glm::vec3 lightDir = glm::vec3(.24f, .685f, -.685f);
const glm::vec3 lightCol = glm::vec3(0.5f, 0.5f, 0.5f);

// vertex bindings
const pvr::utils::VertexBindings VertexAttribBindings[] = {
	{ "POSITION", 0 },
	{ "NORMAL", 1 },
	{ "UV0", 2 },
};

/// <summary>
/// Struct used to hold per object instance data
/// pos : per instance postion in object space
/// scale : per instance scale in object space
/// </summary>
struct InstanceData
{
	glm::vec3 pos;
	float scale;
};

/// <summary>
/// Struct used to hold bounding spehere mesh bounds
/// origin : center of the mesh in its local space
/// radius : radius of the sphere
/// </summary>
struct MeshBounds
{
	glm::vec3 origin;
	float radius;
};

/// <summary>
/// Struct used to hold per instance object transform and mesh bounds
/// modelMatrix : per instance model transform
/// center_rad : bounding sphere mesh bounds packed in a vec4
/// pos : per instance local space position
/// </summary>
struct GPUSSBOMeshData
{
	glm::mat4 modelMatrix;
	glm::vec4 center_rad;
	glm::vec3 pos;
	float scale;
};

/// <summary>
/// Struct used to hold per instance input for indirect cull dispatch stage
/// objectID : every instance is assigned an unique object ID
/// batchID : every set of instances are assigned a draw ID, basically, gl_DrawID, max is equal to total draw count.
/// </summary>
struct GPUPerInstanceInput
{
	uint32_t objectID;
	uint32_t batchID;
};

/// <summary>
/// Struct used to store the VkDrawIndexedIndirectCommand
/// </summary>
struct GPUIndirectDrawCommandObject
{
	VkDrawIndexedIndirectCommand command;
};

/// <summary>
/// Struct used to hold per frame constants for the indirect cull stage
/// frustrumPlanes : front, back, top, bottom, left, right planes used of frustrum culling
/// cullingEnabled : toggle to switch culling ON/OFF
/// drawCount : total number of VkDrawIndexedIndirectCommand / draws
/// zNear : the near clip value
/// </summary>
struct DrawCullData
{
	glm::vec4 frustrumPlanes[6];
	uint32_t cullingEnabled;
	uint32_t drawCount;
	float zNear;
};

/// <summary>
/// Struct used to hold the light constants for the forward indirect draw pass
/// lightDir : the light direction / position
/// lightCol : the color of the light
/// </summary>
struct LightConstants
{
	glm::vec4 lightDir;
	glm::vec4 lightCol;
};

/// <summary>
/// Struct used to hold the shader uniforms
/// proj : holds the perspective projection matrix
/// </summary>
struct UboPerMeshData
{
	glm::mat4 proj;
};

struct ForwardIndirectPass;
struct IndirectCullComputePass;
struct OnScreenPass;

struct DeviceResources
{
	pvrvk::Instance instance;
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
	pvrvk::Device device;
	pvrvk::Swapchain swapchain;
	pvrvk::CommandPool commandPoolGraphics, commandPoolCompute;
	pvrvk::DescriptorPool descriptorPool;
	pvrvk::Queue graphicsQueue, computeQueue;
	pvr::utils::vma::Allocator vmaAllocator;
	std::vector<pvrvk::Semaphore> imageAcquiredSemaphores;
	std::vector<pvrvk::Semaphore> computeSemaphores;
	std::vector<pvrvk::Semaphore> presentationSemaphores;
	std::vector<pvrvk::Fence> perFrameResourcesFences;
	std::vector<pvrvk::Fence> perFrameResourcesFencesCompute;

	// merged VBO & IBO buffers
	pvrvk::Buffer batchedVBO;
	pvrvk::Buffer batchedIBO;

	// 2 main primary command buffers and framebuffer resources
	std::vector<pvrvk::CommandBuffer> mainCommandBuffers; // per swapchain
	std::vector<pvrvk::CommandBuffer> computeCommandBuffers; // per swapchain
	std::vector<pvrvk::Framebuffer> onScreenFramebuffer; // per swapchain

	pvrvk::PipelineCache pipelineCache;

	// Scene Passes
	std::shared_ptr<ForwardIndirectPass> forwardIndirectPass;
	std::shared_ptr<IndirectCullComputePass> indirectCullComputePass;
	std::shared_ptr<OnScreenPass> onScreenPass;

	// UIRenderer used to display text
	pvr::ui::UIRenderer uiRenderer;

	// ForwardIndirectPass resources
	pvrvk::DescriptorSetLayout texLayout;
	pvrvk::DescriptorSetLayout vertexSSBOLayout;
	pvrvk::DescriptorSetLayout uboLayoutDynamic;
	std::vector<pvrvk::DescriptorSet> uboDescSets;
	pvrvk::DescriptorSet vertexSSBODescSet;
	pvrvk::DescriptorSet texDescSet;
	pvr::utils::StructuredBufferView uboStructuredBufferView;
	pvrvk::Buffer uboBuffer;
	pvrvk::Buffer lightConstantUboBuffer;
	uint32_t indirectDrawCount;

	// IndirectCullComputePass Resources
	pvrvk::DescriptorSetLayout indirectCullDescSetLayout;
	pvrvk::DescriptorSet indirectCullDescSet;
	pvrvk::Buffer drawCullDataUBOBuffer;
	pvrvk::Buffer gpuInstanceInputBuffer;
	pvrvk::Buffer gpuInstanceOutputBuffer;
	pvrvk::Buffer gpuInstanceOutputCopyBuffer;

	// Common Resources
	pvrvk::Buffer gpuObjectSSBOBuffer;
	pvrvk::Buffer gpuIndirectCommandsBuffer;

	~DeviceResources()
	{
		if (device) { device->waitIdle(); }
		uint32_t l = swapchain->getSwapchainLength();
		for (uint32_t i = 0; i < l; ++i)
		{
			if (perFrameResourcesFences[i]) perFrameResourcesFences[i]->wait();
			if (perFrameResourcesFencesCompute[i]) perFrameResourcesFencesCompute[i]->wait();
		}
	}
};

/// <summary>A simple rendering pass for forward indirect drawing </summary>
struct ForwardIndirectPass
{
	/// <summary>Initialises the ForwardIndirect pass.</summary>
	void init(pvr::Shell& shell, DeviceResources* deviceResources, const pvr::assets::ModelHandle& scene)
	{
		createRenderPasses(deviceResources);
		createImages(deviceResources, shell);
		createImageViews(deviceResources);
		createFramebuffers(deviceResources);
		createShaderModules(deviceResources, shell);
		createPipelineLayouts(deviceResources);
		createPipelines(deviceResources, scene);
		createDescriptorSet(deviceResources);
	};

	/// <summary>Create and update the descriptor sets.</summary>
	void createDescriptorSet(DeviceResources* deviceResources)
	{
		deviceResources->vertexSSBODescSet = deviceResources->descriptorPool->allocateDescriptorSet(deviceResources->vertexSSBOLayout);
		deviceResources->vertexSSBODescSet->setObjectName(std::string("SSBO DescriptorSet"));
		uint32_t gpuInstanceOutputBufferSize = static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::uinteger)) * TOTAL_NUM_INSTANCES;

		pvrvk::DescriptorBufferInfo bufferInfos[] = { pvrvk::DescriptorBufferInfo(deviceResources->gpuInstanceOutputBuffer, 0, gpuInstanceOutputBufferSize),
			pvrvk::DescriptorBufferInfo(deviceResources->gpuObjectSSBOBuffer, 0, TOTAL_NUM_INSTANCES * sizeof(GPUSSBOMeshData)),
			pvrvk::DescriptorBufferInfo(deviceResources->lightConstantUboBuffer, 0, sizeof(LightConstants)) };

		pvrvk::WriteDescriptorSet writeDescSets[] = { pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, deviceResources->vertexSSBODescSet, 0),
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, deviceResources->vertexSSBODescSet, 1),
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, deviceResources->vertexSSBODescSet, 2) };
		writeDescSets[0].setBufferInfo(0, bufferInfos[0]);
		writeDescSets[1].setBufferInfo(0, bufferInfos[1]);
		writeDescSets[2].setBufferInfo(0, bufferInfos[2]);

		deviceResources->device->updateDescriptorSets(writeDescSets, ARRAY_SIZE(writeDescSets), nullptr, 0);
	}

	/// <summary> Record indirect draw render commands.</summary>
	void render(pvrvk::CommandBuffer& cmdBuffer, DeviceResources* deviceResources, uint32_t swapChainIndex, uint32_t queueIndex, pvr::Shell& shell)
	{
		// Setup clear color & depth.
		pvrvk::ClearValue clearValues[2] = { pvrvk::ClearValue(0.0f, 0.45f, 0.41f, 1.f), pvrvk::ClearValue(0.f, 0u) };

		// Start render pass.
		cmdBuffer->beginRenderPass(_fbo[queueIndex], pvrvk::Rect2D(0, 0, shell.getWidth(), shell.getHeight()), true, clearValues, ARRAY_SIZE(clearValues));

		// Insert a debug label.
		pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Forward Indirect Pass - Swapchain (%i)", swapChainIndex)));

		// calculate the dynamic offset to use
		const uint32_t dynamicOffsetUbo = deviceResources->uboStructuredBufferView.getDynamicSliceOffset(swapChainIndex);

		// enqueue the static states which wont be changed through out the frame
		cmdBuffer->bindPipeline(_pipe);

		cmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _pipelineLayout, 0, deviceResources->uboDescSets[swapChainIndex], &dynamicOffsetUbo, 1);
		cmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _pipelineLayout, 1, deviceResources->vertexSSBODescSet, nullptr);
		cmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _pipelineLayout, 2, deviceResources->texDescSet, nullptr);

		// Render all mesh nodes.
		drawMeshIndirect(cmdBuffer, deviceResources);

		// End debug label region.
		pvr::utils::endCommandBufferDebugLabel(cmdBuffer);

		cmdBuffer->endRenderPass();
	}

	/// <summary> Create color and depth attachment images.</summary>
	void createImages(DeviceResources* deviceResources, pvr::Shell& shell)
	{
		pvrvk::Extent3D texExtents = pvrvk::Extent3D(shell.getWidth(), shell.getHeight(), 1);

		for (int i = 0; i < 2; i++)
		{
			_colorImages[i] = pvr::utils::createImage(deviceResources->device,
				pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, deviceResources->swapchain->getImageFormat(), texExtents,
					pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT),
				pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, deviceResources->vmaAllocator);

			_depthImages[i] = pvr::utils::createImage(deviceResources->device,
				pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, pvrvk::Format::e_D32_SFLOAT, texExtents, pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT),
				pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, deviceResources->vmaAllocator);
		}
	}

	/// <summary> Create color and depth attachment image views.</summary>
	void createImageViews(DeviceResources* deviceResources)
	{
		for (int i = 0; i < 2; i++)
		{
			_colorImageViews[i] = deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(_colorImages[i]));
			_depthImageViews[i] = deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(_depthImages[i]));
		}
	}

	/// <summary> Create offscreen framebuffer for indirect draw pass.</summary>
	void createFramebuffers(DeviceResources* deviceResources)
	{
		std::vector<pvrvk::FramebufferCreateInfo> framebufferInfo;
		framebufferInfo.resize(2);
		for (int i = 0; i < 2; i++)
		{
			pvrvk::FramebufferCreateInfo& info = framebufferInfo[i];
			info.setAttachment(0, _colorImageViews[i]);
			info.setAttachment(1, _depthImageViews[i]);
			info.setRenderPass(_renderPass);
			info.setDimensions(deviceResources->swapchain->getDimension());
			_fbo[i] = deviceResources->device->createFramebuffer(pvrvk::FramebufferCreateInfo(info));
		}
	}

	/// <summary> Create the renderpass to be used.</summary>
	void createRenderPasses(DeviceResources* deviceResources)
	{
		pvrvk::RenderPassCreateInfo renderPassCreateInfo;
		pvrvk::SubpassDescription subPassDesc;
		// Color attachment
		pvrvk::AttachmentDescription colorAttachment = pvrvk::AttachmentDescription::createColorDescription(deviceResources->swapchain->getImageFormat(), pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT);

		// Depth attachment
		pvrvk::AttachmentDescription depthAttachment = pvrvk::AttachmentDescription::createDepthStencilDescription(pvrvk::Format::e_D32_SFLOAT, pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::AttachmentLoadOp::e_DONT_CARE,
			pvrvk::AttachmentStoreOp::e_DONT_CARE, pvrvk::SampleCountFlags::e_1_BIT);

		pvrvk::AttachmentReference colorAttachmentRef = pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
		pvrvk::AttachmentReference depthAttachmentRef = pvrvk::AttachmentReference(1, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		subPassDesc.setColorAttachmentReference(0, colorAttachmentRef);
		subPassDesc.setDepthStencilAttachmentReference(depthAttachmentRef);

		pvrvk::SubpassDependency dependencies[2];

		dependencies[0].setSrcSubpass(VK_SUBPASS_EXTERNAL);
		dependencies[0].setDstSubpass(0);
		dependencies[0].setSrcStageMask(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT);
		dependencies[0].setSrcAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
		dependencies[0].setDstStageMask(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT);
		dependencies[0].setDstAccessMask(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT);
		dependencies[0].setDependencyFlags(pvrvk::DependencyFlags::e_BY_REGION_BIT);

		dependencies[1].setSrcSubpass(0);
		dependencies[1].setDstSubpass(VK_SUBPASS_EXTERNAL);
		dependencies[1].setSrcStageMask(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT);
		dependencies[1].setDstStageMask(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT);
		dependencies[1].setSrcAccessMask(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT);
		dependencies[1].setDstAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
		dependencies[1].setDependencyFlags(pvrvk::DependencyFlags::e_BY_REGION_BIT);

		renderPassCreateInfo.setAttachmentDescription(0, colorAttachment);
		renderPassCreateInfo.setAttachmentDescription(1, depthAttachment);
		renderPassCreateInfo.setSubpass(0, subPassDesc);
		renderPassCreateInfo.addSubpassDependencies(dependencies, 2);

		_renderPass = deviceResources->device->createRenderPass(renderPassCreateInfo);
		_renderPass->setObjectName("ForwardIndirectRenderPass");
	}

	/// <summary> Create the vertex and fragment shader modules to be used with the pipeline.</summary>
	void createShaderModules(DeviceResources* deviceResources, pvr::Shell& shell)
	{
		_vs = deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(shell.getAssetStream(ForwardPassIndirectDrawVertShaderSrcFile)->readToEnd<uint32_t>()));
		_fs = deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(shell.getAssetStream(ForwardPassIndirectDrawFragShaderSrcFile)->readToEnd<uint32_t>()));
	}

	/// <summary> Create the graphics pipeline to be used with the indirect draw pass.</summary>
	void createPipelines(DeviceResources* deviceResources, const pvr::assets::ModelHandle& scene)
	{
		pvrvk::GraphicsPipelineCreateInfo pipelineCreateInfo;

		const pvrvk::Rect2D rect(0, 0, deviceResources->swapchain->getDimension().getWidth(), deviceResources->swapchain->getDimension().getHeight());
		pipelineCreateInfo.viewport.setViewportAndScissor(0,
			pvrvk::Viewport(static_cast<float>(rect.getOffset().getX()), static_cast<float>(rect.getOffset().getY()), static_cast<float>(rect.getExtent().getWidth()),
				static_cast<float>(rect.getExtent().getHeight())),
			rect);
		// enable front face culling
		pipelineCreateInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);

		// set counter clockwise winding order for front faces
		pipelineCreateInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);
		// renderShadowPipelineCreateInfo.rasterizer.setDepthClip = false;

		// enable depth testing
		pipelineCreateInfo.depthStencil.enableDepthTest(true);
		pipelineCreateInfo.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_GREATER);
		pipelineCreateInfo.depthStencil.enableDepthWrite(true);

		// load and create appropriate shaders
		pipelineCreateInfo.vertexShader.setShader(_vs);
		pipelineCreateInfo.fragmentShader.setShader(_fs);

		const pvr::assets::Mesh& mesh = scene->getMesh(0);
		pvr::utils::populateInputAssemblyFromMesh(
			mesh, VertexAttribBindings, sizeof(VertexAttribBindings) / sizeof(VertexAttribBindings[0]), pipelineCreateInfo.vertexInput, pipelineCreateInfo.inputAssembler);

		// renderpass/subpass
		pipelineCreateInfo.renderPass = _renderPass;

		pvrvk::PipelineColorBlendAttachmentState colorAttachmentState;

		colorAttachmentState.setBlendEnable(false);
		pipelineCreateInfo.colorBlend.setAttachmentState(0, colorAttachmentState);

		pipelineCreateInfo.pipelineLayout = _pipelineLayout;

		pipelineCreateInfo.inputAssembler.setPrimitiveTopology(pvr::utils::convertToPVRVk(mesh.getPrimitiveType()));

		_pipe = deviceResources->device->createGraphicsPipeline(pipelineCreateInfo, deviceResources->pipelineCache);
		_pipe->setObjectName("ForwardIndirectGraphicsPipeline");
	}

	/// <summary> Create the pipeline layouts to be used with the graphics pipeline.</summary>
	void createPipelineLayouts(DeviceResources* deviceResources)
	{
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

		//--- create the pipeline layout
		{
			pipeLayoutInfo
				.addDescSetLayout(deviceResources->uboLayoutDynamic) /*set 0*/
				.addDescSetLayout(deviceResources->vertexSSBOLayout) /*set 1*/
				.addDescSetLayout(deviceResources->texLayout); /*set 2*/
		}
		_pipelineLayout = deviceResources->device->createPipelineLayout(pipeLayoutInfo);
	}

	/// <summary> Records commands for binding vertex and index buffer and vkCmdDrawIndexedIndirect().</summary>
	void drawMeshIndirect(pvrvk::CommandBuffer& cmdBuffers, DeviceResources* deviceResources)
	{
		// bind the merged VBO &* IBO for the scene
		cmdBuffers->bindVertexBuffer(deviceResources->batchedVBO, 0, 0);
		cmdBuffers->bindIndexBuffer(deviceResources->batchedIBO, 0, pvrvk::IndexType::e_UINT16);
		cmdBuffers->drawIndexedIndirect(deviceResources->gpuIndirectCommandsBuffer, 0, deviceResources->indirectDrawCount, sizeof(GPUIndirectDrawCommandObject));
	}

	pvrvk::ShaderModule _vs;
	pvrvk::ShaderModule _fs;
	pvrvk::GraphicsPipeline _pipe;
	pvrvk::PipelineLayout _pipelineLayout;
	pvrvk::RenderPass _renderPass;
	pvrvk::Framebuffer _fbo[2];

	pvrvk::Image _colorImages[2];
	pvrvk::ImageView _colorImageViews[2];

	pvrvk::Image _depthImages[2];
	pvrvk::ImageView _depthImageViews[2];
};

struct OnScreenPass
{
	OnScreenPass() {}
	~OnScreenPass() {}

	/// <summary>Initialises the OnScreenPass pass.</summary>
	void init(pvr::Shell& shell, DeviceResources* deviceResources)
	{
		createShaderModules(deviceResources, shell);
		createDescriptorSetLayoutAndDescriptorSet(deviceResources);
		createPipelineLayouts(deviceResources);
		createPipelines(deviceResources);
	}

	/// <summary> Record full screen quad draw render commands.</summary>
	void render(pvrvk::CommandBuffer& cmdBuffer, DeviceResources* deviceResources, uint32_t swapChainIndex, pvr::Shell& shell, pvrvk::ImageView& offScreenImageView, uint32_t queueIndex)
	{
		{
			pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL;
			pvrvk::ImageLayout destinationImageLayout = pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL;

			pvrvk::MemoryBarrierSet layoutTransitions;
			layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT,
				offScreenImageView->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), sourceImageLayout, destinationImageLayout,
				deviceResources->graphicsQueue->getFamilyIndex(), deviceResources->graphicsQueue->getFamilyIndex()));

			cmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, layoutTransitions);
		}

		// Setup clear color & depth.
		pvrvk::ClearValue clearValues[2] = { pvrvk::ClearValue(0.5f, 0.5f, 0.5f, 1.f), pvrvk::ClearValue(0.f, 0u) };

		// Start render pass.
		cmdBuffer->beginRenderPass(
			deviceResources->onScreenFramebuffer[swapChainIndex], pvrvk::Rect2D(0, 0, shell.getWidth(), shell.getHeight()), true, clearValues, ARRAY_SIZE(clearValues));

		// Insert a debug label.
		pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Draw On Screen - Swapchain (%i)", swapChainIndex)));

		// enqueue the static states which wont be changed through out the frame
		cmdBuffer->bindPipeline(_pipe);
		cmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _pipelineLayout, 0, _descSet[queueIndex], nullptr);

		// Render all mesh nodes.
		drawFullScreen(cmdBuffer);
		pvr::utils::endCommandBufferDebugLabel(cmdBuffer);

		deviceResources->uiRenderer.beginRendering(cmdBuffer);
		deviceResources->uiRenderer.getDefaultTitle()->render();
		deviceResources->uiRenderer.getDefaultControls()->render();
		deviceResources->uiRenderer.getSdkLogo()->render();
		deviceResources->uiRenderer.endRendering();

		// End debug label region.

		cmdBuffer->endRenderPass();

		{
			pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL;
			pvrvk::ImageLayout destinationImageLayout = pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL;

			pvrvk::MemoryBarrierSet layoutTransitions;
			layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT,
				offScreenImageView->getImage(), pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), sourceImageLayout, destinationImageLayout,
				deviceResources->graphicsQueue->getFamilyIndex(), deviceResources->graphicsQueue->getFamilyIndex()));

			cmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, layoutTransitions);
		}
	}

	/// <summary> Create the full screen quad vertex and fragment shader modules to be used with the pipeline.</summary>
	void createShaderModules(DeviceResources* deviceResources, pvr::Shell& shell)
	{
		_vs = deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(shell.getAssetStream(OnScreenQuadVert)->readToEnd<uint32_t>()));
		_fs = deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(shell.getAssetStream(OnScreenQuadFrag)->readToEnd<uint32_t>()));
	}

	/// <summary> Create the graphics pipeline to be used with the OnScreenPass.</summary>
	void createPipelines(DeviceResources* deviceResources)
	{
		pvrvk::GraphicsPipelineCreateInfo pipelineCreateInfo;

		const pvrvk::Rect2D rect(0, 0, deviceResources->swapchain->getDimension().getWidth(), deviceResources->swapchain->getDimension().getHeight());
		pipelineCreateInfo.viewport.setViewportAndScissor(0,
			pvrvk::Viewport(static_cast<float>(rect.getOffset().getX()), static_cast<float>(rect.getOffset().getY()), static_cast<float>(rect.getExtent().getWidth()),
				static_cast<float>(rect.getExtent().getHeight())),
			rect);
		// enable front face culling
		pipelineCreateInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_NONE);

		// set counter clockwise winding order for front faces
		pipelineCreateInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);
		// renderShadowPipelineCreateInfo.rasterizer.setDepthClip = false;

		// enable depth testing
		pipelineCreateInfo.depthStencil.enableDepthTest(false);
		pipelineCreateInfo.depthStencil.enableDepthWrite(false);

		// load and create appropriate shaders
		pipelineCreateInfo.vertexShader.setShader(_vs);
		pipelineCreateInfo.fragmentShader.setShader(_fs);

		// renderpass/subpass
		pipelineCreateInfo.renderPass = deviceResources->onScreenFramebuffer[0]->getRenderPass();

		pvrvk::PipelineColorBlendAttachmentState colorAttachmentState;

		colorAttachmentState.setBlendEnable(false);
		pipelineCreateInfo.colorBlend.setAttachmentState(0, colorAttachmentState);

		pipelineCreateInfo.pipelineLayout = _pipelineLayout;

		pipelineCreateInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);

		_pipe = deviceResources->device->createGraphicsPipeline(pipelineCreateInfo, deviceResources->pipelineCache);
		_pipe->setObjectName("OnScreenGraphicsPipeline");
	}

	/// <summary> Create the pipeline layouts to be used with the graphics pipeline.</summary>
	void createPipelineLayouts(DeviceResources* deviceResources)
	{
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

		//--- create the pipeline layout
		{
			pipeLayoutInfo.addDescSetLayout(_descSetLayout); /*set 0*/
		}
		_pipelineLayout = deviceResources->device->createPipelineLayout(pipeLayoutInfo);
	}

	/// <summary>Create descriptor set layout, descriptor set, sampler and update the descriptor sets.</summary>
	void createDescriptorSetLayoutAndDescriptorSet(DeviceResources* deviceResources)
	{
		{
			pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
			descSetLayoutInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); /*binding 0*/
			_descSetLayout = deviceResources->device->createDescriptorSetLayout(descSetLayoutInfo);
		}

		pvrvk::SamplerCreateInfo samplerInfo;
		samplerInfo.magFilter = pvrvk::Filter::e_LINEAR;
		samplerInfo.minFilter = pvrvk::Filter::e_LINEAR;
		samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
		_samplerMipBilinear = deviceResources->device->createSampler(samplerInfo);

		for (int i = 0; i < 2; i++)
		{
			_descSet[i] = deviceResources->descriptorPool->allocateDescriptorSet(_descSetLayout);
			_descSet[i]->setObjectName("OnScreen" + std::to_string(i) + "DescriptorSet");
			// Update descriptor sets
			std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _descSet[i], 0));
			writeDescSets.back().setImageInfo(
				0, pvrvk::DescriptorImageInfo(deviceResources->forwardIndirectPass->_colorImageViews[i], _samplerMipBilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

			deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
		}
	}

	/// <summary>Record draw commands for drawing full screen triangle.</summary>
	void drawFullScreen(pvrvk::CommandBuffer& cmdBuffers) { cmdBuffers->draw(0, 3, 0, 1); }

	pvrvk::Sampler _samplerMipBilinear;
	pvrvk::ShaderModule _vs;
	pvrvk::ShaderModule _fs;
	pvrvk::GraphicsPipeline _pipe;
	pvrvk::PipelineLayout _pipelineLayout;
	pvrvk::DescriptorSetLayout _descSetLayout;
	pvrvk::DescriptorSet _descSet[2];
};

struct IndirectCullComputePass
{
	/// <summary>Initialises the IndirectCullComputePass pass.</summary>
	void init(pvr::Shell& shell, DeviceResources* deviceResources)
	{
		createShaderModules(shell, deviceResources);
		createUBOBuffer(deviceResources);
		createDescriptorSetLayout(deviceResources);
		createAndUpdateDescriptorSets(deviceResources);
		createPipelineLayouts(deviceResources);
		createPipeline(deviceResources);
	}

	/// <summary>Record commands for performing indirect cull compute tasks .</summary>
	void indirectCullDispatch(DeviceResources* deviceResources, pvrvk::CommandBuffer& cmdBuffer)
	{
		pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel("Indirect Cull Pass"));
		cmdBuffer->bindPipeline(_pipeline);

		cmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_COMPUTE, _pipelineLayout, 0, deviceResources->indirectCullDescSet, nullptr, 0);

		cmdBuffer->dispatch(static_cast<uint32_t>((TOTAL_NUM_INSTANCES / 32) + 1), 1, 1);

		pvrvk::AccessFlags srcAccessMask = pvrvk::AccessFlags::e_SHADER_WRITE_BIT;
		pvrvk::AccessFlags dstAccessMask1 = pvrvk::AccessFlags::e_INDIRECT_COMMAND_READ_BIT;
		pvrvk::AccessFlags dstAccessMask2 = pvrvk::AccessFlags::e_SHADER_READ_BIT;
		pvrvk::MemoryBarrierSet bufferMemoryBarrier1;
		pvrvk::MemoryBarrierSet bufferMemoryBarrier2;

		bufferMemoryBarrier1.addBarrier(pvrvk::BufferMemoryBarrier(
			srcAccessMask, dstAccessMask1, deviceResources->gpuIndirectCommandsBuffer, 0, static_cast<uint32_t>(deviceResources->gpuIndirectCommandsBuffer->getSize())));
		bufferMemoryBarrier2.addBarrier(pvrvk::BufferMemoryBarrier(
			srcAccessMask, dstAccessMask2, deviceResources->gpuInstanceOutputBuffer, 0, static_cast<uint32_t>(deviceResources->gpuInstanceOutputBuffer->getSize())));

		cmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_DRAW_INDIRECT_BIT, bufferMemoryBarrier1);
		cmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_VERTEX_SHADER_BIT, bufferMemoryBarrier2);
		pvr::utils::endCommandBufferDebugLabel(cmdBuffer);
	}

	/// <summary> Create the indirect cull compute shader modules to be used with the pipeline.</summary>
	void createShaderModules(pvr::Shell& shell, DeviceResources* deviceResources)
	{
		_cs = deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(shell.getAssetStream(IndirectCullCompShaderFileName)->readToEnd<uint32_t>()));
	}

	/// <summary>Create the indirect cull compute descriptor set layout.</summary>
	void createDescriptorSetLayout(DeviceResources* deviceResources)
	{
		//--- create the input & output depth texture-sampler descriptor set layout
		{
			pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
			descSetLayoutInfo.setBinding(0, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1u, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
			descSetLayoutInfo.setBinding(1, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1u, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
			descSetLayoutInfo.setBinding(2, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1u, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
			descSetLayoutInfo.setBinding(3, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1u, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
			descSetLayoutInfo.setBinding(4, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1u, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
			deviceResources->indirectCullDescSetLayout = deviceResources->device->createDescriptorSetLayout(descSetLayoutInfo);
		}
	}

	/// <summary>Create and update the descriptor sets.</summary>
	void createAndUpdateDescriptorSets(DeviceResources* deviceResources)
	{
		deviceResources->indirectCullDescSet = deviceResources->descriptorPool->allocateDescriptorSet(deviceResources->indirectCullDescSetLayout);
		deviceResources->indirectCullDescSet->setObjectName("IndirectCullDescriptorSet");

		uint32_t gpuInstanceOutputBufferSize = static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::uinteger)) * TOTAL_NUM_INSTANCES;
		uint32_t uboSize = static_cast<uint32_t>(sizeof(DrawCullData));

		pvrvk::DescriptorBufferInfo bufferInfos[] = { pvrvk::DescriptorBufferInfo(deviceResources->gpuObjectSSBOBuffer, 0, TOTAL_NUM_INSTANCES * sizeof(GPUSSBOMeshData)),
			pvrvk::DescriptorBufferInfo(deviceResources->gpuIndirectCommandsBuffer, 0, deviceResources->indirectDrawCount * sizeof(GPUIndirectDrawCommandObject)),
			pvrvk::DescriptorBufferInfo(deviceResources->gpuInstanceInputBuffer, 0, TOTAL_NUM_INSTANCES * sizeof(GPUPerInstanceInput)),
			pvrvk::DescriptorBufferInfo(deviceResources->gpuInstanceOutputBuffer, 0, gpuInstanceOutputBufferSize),
			pvrvk::DescriptorBufferInfo(deviceResources->drawCullDataUBOBuffer, 0, uboSize) };

		pvrvk::WriteDescriptorSet writeDescSets[5] = { pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, deviceResources->indirectCullDescSet, 0),
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, deviceResources->indirectCullDescSet, 1),
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, deviceResources->indirectCullDescSet, 2),
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, deviceResources->indirectCullDescSet, 3),
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, deviceResources->indirectCullDescSet, 4) };

		writeDescSets[0].setBufferInfo(0, bufferInfos[0]);
		writeDescSets[1].setBufferInfo(0, bufferInfos[1]);
		writeDescSets[2].setBufferInfo(0, bufferInfos[2]);
		writeDescSets[3].setBufferInfo(0, bufferInfos[3]);
		writeDescSets[4].setBufferInfo(0, bufferInfos[4]);

		deviceResources->device->updateDescriptorSets(writeDescSets, ARRAY_SIZE(writeDescSets), nullptr, 0);
	}

	/// <summary> Create the pipeline layouts to be used with the compute pipeline.</summary>
	void createPipelineLayouts(DeviceResources* deviceResources)
	{
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(deviceResources->indirectCullDescSetLayout); // set 0
		_pipelineLayout = deviceResources->device->createPipelineLayout(pipeLayoutInfo);
	}

	/// <summary> Create the compute pipeline to be used with the indirect cull compute pass.</summary>
	void createPipeline(DeviceResources* deviceResources)
	{
		pvrvk::ComputePipelineCreateInfo pipelineCreateInfo;

		pipelineCreateInfo.computeShader.setShader(_cs);
		pipelineCreateInfo.pipelineLayout = _pipelineLayout;

		_pipeline = deviceResources->device->createComputePipeline(pipelineCreateInfo, deviceResources->pipelineCache);
		_pipeline->setObjectName("IndirectCullComputePipeline");
	}

	/// <summary> Create the compute shader constant buffer to be used with the indirect cull compute stage.</summary>
	void createUBOBuffer(DeviceResources* deviceResources)
	{
		deviceResources->drawCullDataUBOBuffer = pvr::utils::createBuffer(deviceResources->device,
			pvrvk::BufferCreateInfo(sizeof(DrawCullData), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, deviceResources->vmaAllocator);
		deviceResources->drawCullDataUBOBuffer->setObjectName("DrawCullDataUBO");
	}

	uint32_t _depthPyramidWidth;
	uint32_t _depthPyramidHeight;

	pvrvk::ShaderModule _cs;
	pvrvk::ComputePipeline _pipeline;
	pvrvk::PipelineLayout _pipelineLayout;
};

/// <summary>Class implementing the Shell functions.</summary>
class VulkanGpuControlledRendering : public pvr::Shell
{
	// 3D Model
	pvr::assets::ModelHandle _scene;

	// store the swapchain length as it is frequently accessed
	uint32_t _swapchainLength;

	// frame and queue counters
	uint32_t _frameId;
	uint32_t _queueIndex;

	// The translation and Rotate parameter of Model
	float _angleYSphere;

	// Putting all API objects, resources into a pointer
	std::unique_ptr<DeviceResources> _deviceResources;

	// Store the per object instance data
	std::vector<InstanceData> _instanceData;

	// Store the calculated mesh bounds
	std::vector<MeshBounds> _sceneMeshBounds;

	// toggle to switch culling ON/OFF
	bool _cullingEnabled = true;

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	/// <summary> Loads scene textures and create texture sampler and descriptor set </summary>
	void createImageSamplerDescriptor(pvrvk::CommandBuffer& imageUploadCmd);

	/// <summary> Creates the dynamic ubo & structured buffer view for our forward indirect scene pass </summary>
	void createUbo();

	/// <summary> Creates all the common buffers and desc set layouts </summary>
	void createCommonResources();

	/// <summary> Initializes all the scene passes that contribute to the final frame buffer </summary>
	void createScenePasses();

	/// <summary> Populates the per instance transform data like position and scales into a grid like fashion for TOTAL_NUM_INSTANCES </summary>
	void createInstanceData();

	/// <summary> Combines the sub-meshes vertex and index data from the 3D model (.gltf) into single vertex and index buffer </summary>
	void mergeSceneIbosVbos(pvrvk::CommandBuffer uploadBuffer);

	/// <summary> Calculates object space sphere bounds </summary>
	/// <returns> calculated Meshbounds in object space
	MeshBounds calculateBoundingSphereMeshBounds(pvr::assets::Model::Mesh& mesh);

	/// <summary> Populates the vertexSSBO data </summary>
	void refreshBoundsAndUpdateObjectSSBOData(pvrvk::CommandBuffer& cmdBuffer);

	/// <summary> Updates indirect cull const data </summary>
	void updateDrawCullDataAndUBO(const uint32_t swapchainIndex);

	/// <summary> Updates GPU Indirect Object data </summary>
	void updateGPUIndirectObjectData(pvrvk::CommandBuffer& cmdBuffer);

	/// <summary> Updates per object instance data and also resets the final visibilty buffer every frame </summary>
	void updateGPUInstanceData();

	/// <summary> Updates & resets per draw, per instance & the indirect command buffer data </summary>
	void prepareCullData(pvrvk::CommandBuffer& cmdBuffer);

	/// <summary> Reads the culling results from the output instance buffer and logs the queried results </summary>
	void logDebugData();

	/// <summary> Normalizes a 3D vector representing a plane </summary>
	/// <returns> a normalized 3D vector representing a plane
	glm::vec4 normalizePlane(glm::vec4 p);

	/// <summary> Generates a perspective projection matrix with infinite far plane </summary>
	/// <returns> Return a perspective projection matrix with infinite far plane with reversed z
	glm::mat4 perspectiveProjectionInifiniteFarPlane(float fovY, float aspectWbyH, float zNear);

	/// <summary>Utility method to allocate a new command buffer and start recording, returning it.</summary>
	/// <returns>Return the allocated command buffer.</returns>
	pvrvk::CommandBuffer beginCommandBuffer();

	/// <summary>Utility method to finish recording and submit a command buffer.</summary>
	void endAndSubmitCommandBuffer(pvrvk::CommandBuffer commandBuffer);

private:
	/// <summary>Handles user input and updates live variables accordingly.</summary>
	void eventMappedInput(pvr::SimplifiedInput action);

	/// <summary>Based on user input  toggles the CULLING_MODE ON/OFF </summary>
	void toggleCulling();
};

void VulkanGpuControlledRendering::createInstanceData()
{
	_instanceData.resize(TOTAL_NUM_INSTANCES);

	std::vector<glm::vec3> instancePoses;
	std::vector<float> instanceScales;

	instancePoses.reserve(TOTAL_NUM_INSTANCES);
	instanceScales.reserve(TOTAL_NUM_INSTANCES);

	// Generate instancePoses for 1st mesh sphere
	float gridXOffset = 25.0f;
	float gridYOffset = 25.0f;
	float gridZOffset = 25.0f;

	for (int y = 0, index = 0; y < YSIZE; y++)
	{
		for (int x = 0; x < XSIZE; x++)
		{
			for (int z = 0; z < ZSIZE; z++, index++)
			{
				glm::vec3 sphereSpawnPos = glm::vec3(x * gridXOffset, y * gridYOffset, z * gridZOffset);
				instancePoses.emplace_back(sphereSpawnPos);
				instanceScales.emplace_back(7.0f);
			}
		}
	}

	// Generate instancePoses for 2nd mesh torus
	gridXOffset = -20.f;

	for (int y = 0; y < YSIZE; y++)
	{
		for (int x = 0; x < XSIZE; x++)
		{
			for (int z = 0; z < ZSIZE; z++)
			{
				glm::vec3 torusSpawnPos = glm::vec3(x * gridXOffset, y * gridYOffset, z * gridZOffset);
				instancePoses.emplace_back(torusSpawnPos);
				instanceScales.emplace_back(7.0f);
			}
		}
	}

	const int numInstances = static_cast<int>(_instanceData.size());
	for (int i = 0; i < numInstances; i++)
	{
		_instanceData[i].pos = instancePoses[i];
		_instanceData[i].scale = instanceScales[i];
	}
}

void VulkanGpuControlledRendering::updateGPUIndirectObjectData(pvrvk::CommandBuffer& cmdBuffer)
{
	// reset GPUIndirectObject buffer
	std::vector<GPUIndirectDrawCommandObject> gpuIndirectObjects;
	int startIndexOffset = 0;
	int startVertexOffset = 0;

	int m = 0;
	for (uint32_t i = 0; i < _scene->getNumMeshes(); i++)
	{
		GPUIndirectDrawCommandObject temp{};
		VkDrawIndexedIndirectCommand indirectCommand;
		pvr::assets::Mesh& mesh = _scene->getMesh(i);
		indirectCommand.instanceCount = 0; // actuals will be updated by the indirect cull pass on surviving instances
		indirectCommand.firstInstance = m * NUM_INSTANCES_PER_DRAW;
		indirectCommand.firstIndex = startIndexOffset;
		indirectCommand.vertexOffset = startVertexOffset;
		indirectCommand.indexCount = mesh.getNumFaces() * 3;
		temp.command = indirectCommand;
		gpuIndirectObjects.push_back(temp);

		startIndexOffset += mesh.getNumIndices();
		startVertexOffset += mesh.getNumVertices();
		m++;
	}
	_deviceResources->indirectDrawCount = static_cast<uint32_t>(gpuIndirectObjects.size());

	// copy to gpuIndirectCommandBuffer to be used for indirect cull and final rendering with actual instance count
	pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, _deviceResources->gpuIndirectCommandsBuffer, cmdBuffer, gpuIndirectObjects.data(), 0,
		sizeof(GPUIndirectDrawCommandObject) * gpuIndirectObjects.size(), _deviceResources->vmaAllocator);

	// buffer memory barrier to consume for indirect cull compute pass
	pvrvk::AccessFlags srcAccessMask = pvrvk::AccessFlags::e_TRANSFER_READ_BIT | pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT;
	pvrvk::AccessFlags dstAccessMask = pvrvk::AccessFlags::e_SHADER_READ_BIT | pvrvk::AccessFlags::e_SHADER_WRITE_BIT;
	pvrvk::MemoryBarrierSet bufferMemoryBarrier;
	bufferMemoryBarrier.addBarrier(pvrvk::BufferMemoryBarrier(
		srcAccessMask, dstAccessMask, _deviceResources->gpuIndirectCommandsBuffer, 0, static_cast<uint32_t>(_deviceResources->gpuIndirectCommandsBuffer->getSize())));
	cmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_TRANSFER_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, bufferMemoryBarrier);
}

void VulkanGpuControlledRendering::updateGPUInstanceData()
{
	std::vector<GPUPerInstanceInput> gpuInstanceObjects;
	uint32_t objectIDCounter = 0;
	for (int i = 0; i < SCENE_NUM_MESHES; i++)
	{
		for (int j = 0; j < NUM_INSTANCES_PER_DRAW; j++)
		{
			GPUPerInstanceInput temp{};
			temp.batchID = i;
			temp.objectID = objectIDCounter++;
			gpuInstanceObjects.push_back(temp);
		}
	}
	// reset the final visibility buffer
	uint32_t gpuInstanceOutputBufferSize = static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::uinteger)) * TOTAL_NUM_INSTANCES;
	std::vector<uint32_t> gpuInstanceOutputBufferData;
	for (int i = 0; i < TOTAL_NUM_INSTANCES; i++) { gpuInstanceOutputBufferData.push_back(UINT32_MAX); }

	pvrvk::CommandBuffer uploadCmd = beginCommandBuffer();
	pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, _deviceResources->gpuInstanceInputBuffer, uploadCmd, gpuInstanceObjects.data(), 0,
		sizeof(GPUPerInstanceInput) * gpuInstanceObjects.size(), _deviceResources->vmaAllocator);

	pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, _deviceResources->gpuInstanceOutputBuffer, uploadCmd, gpuInstanceOutputBufferData.data(), 0,
		gpuInstanceOutputBufferSize, _deviceResources->vmaAllocator);

	endAndSubmitCommandBuffer(uploadCmd);
}

void VulkanGpuControlledRendering::prepareCullData(pvrvk::CommandBuffer& cmdBuffer)
{
	// update model matrix & mesh bounds
	refreshBoundsAndUpdateObjectSSBOData(cmdBuffer);

	// reset IndirectDrawCommand buffer
	updateGPUIndirectObjectData(cmdBuffer);

	// update gpu instance input and output buffer
	updateGPUInstanceData();
}

void VulkanGpuControlledRendering::mergeSceneIbosVbos(pvrvk::CommandBuffer uploadBuffer)
{
	pvrvk::DeviceSize batchVBOSize = 0;
	pvrvk::DeviceSize batchIBOSize = 0;

	for (uint32_t i = 0; i < _scene->getNumMeshes(); i++)
	{
		const pvr::assets::Mesh& mesh = _scene->getMesh(i);
		batchVBOSize += mesh.getDataSize(0);
		batchIBOSize += mesh.getFaces().getDataSize();
	}

	// batched VBO and IBO buffer creation
	pvrvk::MemoryPropertyFlags requiredMemoryFlags = pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT;

	_deviceResources->batchedVBO = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(static_cast<uint32_t>(batchVBOSize), pvrvk::BufferUsageFlags::e_VERTEX_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT),
		requiredMemoryFlags, pvrvk::MemoryPropertyFlags::e_NONE, _deviceResources->vmaAllocator);
	_deviceResources->batchedVBO->setObjectName("BatchedVBO");

	_deviceResources->batchedIBO = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(static_cast<uint32_t>(batchIBOSize), pvrvk::BufferUsageFlags::e_INDEX_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT),
		requiredMemoryFlags, pvrvk::MemoryPropertyFlags::e_NONE, _deviceResources->vmaAllocator);
	_deviceResources->batchedIBO->setObjectName("BatchedIBO");

	uint32_t currVertexOffset = 0;
	uint32_t currIndexOffset = 0;
	for (uint32_t i = 0; i < _scene->getNumMeshes(); i++)
	{
		const pvr::assets::Mesh& mesh = _scene->getMesh(i);
		pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, _deviceResources->batchedVBO, uploadBuffer, (const void*)mesh.getData(0), currVertexOffset,
			mesh.getDataSize(0), _deviceResources->vmaAllocator);

		pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, _deviceResources->batchedIBO, uploadBuffer, (const void*)mesh.getFaces().getData(), currIndexOffset,
			mesh.getFaces().getDataSize(), _deviceResources->vmaAllocator);

		currVertexOffset += static_cast<uint32_t>(mesh.getDataSize(0));
		currIndexOffset += mesh.getFaces().getDataSize();
	}
}

void VulkanGpuControlledRendering::createImageSamplerDescriptor(pvrvk::CommandBuffer& imageUploadCmd)
{
	pvrvk::Device& device = _deviceResources->device;
	pvrvk::ImageView texBase1;
	pvrvk::ImageView texBase2;
	// create the bilinear sampler
	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.magFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.minFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
	pvrvk::Sampler samplerMipBilinear = device->createSampler(samplerInfo);

	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_LINEAR;
	pvrvk::Sampler samplerTrilinear = device->createSampler(samplerInfo);

	bool astcSupported = pvr::utils::isSupportedFormat(_deviceResources->device->getPhysicalDevice(), pvrvk::Format::e_ASTC_4x4_UNORM_BLOCK);

	texBase1 = pvr::utils::loadAndUploadImageAndView(_deviceResources->device, (sphereTexFileName + (astcSupported ? "_astc.pvr" : ".pvr")).c_str(), true, imageUploadCmd, *this,
		pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	texBase1->setObjectName("Sphere Texture");

	texBase2 = pvr::utils::loadAndUploadImageAndView(_deviceResources->device, (torusTexFileName + (astcSupported ? "_astc.pvr" : ".pvr")).c_str(), true, imageUploadCmd, *this,
		pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	texBase2->setObjectName("Torus Texture");

	// create the descriptor set
	_deviceResources->texDescSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->texLayout);
	_deviceResources->texDescSet->setObjectName("Texture DescriptorSet");

	pvrvk::DescriptorImageInfo imageInfos[2] = { pvrvk::DescriptorImageInfo(texBase1, nullptr), pvrvk::DescriptorImageInfo(texBase2, nullptr) };

	pvrvk::WriteDescriptorSet writeDescSets[2] = { pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_SAMPLER, _deviceResources->texDescSet, 0),
		pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_SAMPLED_IMAGE, _deviceResources->texDescSet, 1) };
	writeDescSets[0].setImageInfo(0, pvrvk::DescriptorImageInfo(nullptr, samplerMipBilinear));
	writeDescSets[1].setImageInfo(0, imageInfos[0]);
	writeDescSets[1].setImageInfo(1, imageInfos[1]);

	_deviceResources->device->updateDescriptorSets(writeDescSets, ARRAY_SIZE(writeDescSets), nullptr, 0);
}

void VulkanGpuControlledRendering::createUbo()
{
	std::vector<pvrvk::WriteDescriptorSet> descUpdate{ _swapchainLength };
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("proj", pvr::GpuDatatypes::mat4x4);

		_deviceResources->uboStructuredBufferView.initDynamic(desc, _scene->getNumMeshNodes() * _swapchainLength, pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
		_deviceResources->uboBuffer = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->uboStructuredBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
		_deviceResources->uboStructuredBufferView.pointToMappedMemory(_deviceResources->uboBuffer->getDeviceMemory()->getMappedData());
		_deviceResources->uboBuffer->setObjectName("ObjectUBO");
	}

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->uboDescSets.push_back(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->uboLayoutDynamic));
		_deviceResources->uboDescSets[i]->setObjectName("UBOSwapchain" + std::to_string(i) + "DescriptorSet");

		descUpdate[i]
			.set(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->uboDescSets[i])
			.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->uboBuffer, 0, _deviceResources->uboStructuredBufferView.getDynamicSliceSize()));
	}
	_deviceResources->device->updateDescriptorSets(static_cast<const pvrvk::WriteDescriptorSet*>(descUpdate.data()), _swapchainLength, nullptr, 0);
}

MeshBounds VulkanGpuControlledRendering::calculateBoundingSphereMeshBounds(pvr::assets::Model::Mesh& mesh)
{
	MeshBounds bounds = {};

	auto positions = mesh.getVertexAttributeByName("POSITION");
	auto numVertices = mesh.getNumVertices();
	const uint8_t* data = (const uint8_t*)mesh.getData(positions->getDataIndex()); // Use a byte-sized pointer so that offsets work properly.
	uint32_t stride = mesh.getStride(positions->getDataIndex());
	uint32_t offset = positions->getOffset();

	// Center
	glm::vec3 center = glm::vec3(0);
	// go through the vertices to calculate the origin
	for (uint32_t i = 0; i < numVertices; i++)
	{
		glm::vec3 pos_tmp;
		memcpy(&pos_tmp, data + offset + i * stride, sizeof(float) * 3);

		glm::vec3 pos = glm::vec3(glm::vec4(pos_tmp, 1.f));
		center += glm::vec3(pos);
	}
	center /= float(numVertices);

	// Radius
	float radius = 0;
	// go through the vertices again to calculate the bounding sphere radius
	for (uint32_t i = 0; i < numVertices; i++)
	{
		glm::vec3 pos_tmp;
		memcpy(&pos_tmp, data + offset + i * stride, sizeof(float) * 3);

		glm::vec3 pos = glm::vec3(glm::vec4(pos_tmp, 1.f));
		radius = std::max(radius, glm::distance(center, glm::vec3(pos)));
	}

	bounds.origin = center;
	bounds.radius = radius;

	return bounds;
}

void VulkanGpuControlledRendering::refreshBoundsAndUpdateObjectSSBOData(pvrvk::CommandBuffer& cmdBuffer)
{
	_angleYSphere += -RotateY * 0.05f * getFrameTime();

	std::vector<GPUSSBOMeshData> meshDataList;
	// generate model matrices for each instance of scene elments using instance data (pos & scales)
	for (int i = 0; i < TOTAL_NUM_INSTANCES; i++)
	{
		GPUSSBOMeshData temp;
		glm::mat4 transMat;
		MeshBounds meshBound;
		glm::mat4 modelMat;
		glm::vec3 posOffset;
		float posOffsetZ = glm::lerp(50.f, 100.f, (float)std::sin(-RotateY * getTime() * 0.01));

		//    know for the fact we have equal instances in each sub draw
		if (i < NUM_INSTANCES_PER_DRAW)
		{
			posOffset = glm::vec3(10, -50.0f, posOffsetZ);
			transMat = glm::translate(glm::mat4(1.0f), _instanceData[i].pos + posOffset);
			modelMat = transMat * glm::rotate(-_angleYSphere, glm::vec3(1, 1, 0)) * glm::scale(glm::vec3(_instanceData[i].scale));
			meshBound = _sceneMeshBounds[0];
		}
		else
		{
			posOffset = glm::vec3(-15, -50.0f, posOffsetZ);
			transMat = glm::translate(glm::mat4(1.0f), _instanceData[i].pos + posOffset);
			modelMat = transMat * glm::rotate(-_angleYSphere, glm::vec3(0, 1, 1)) * glm::scale(glm::vec3(_instanceData[i].scale));
			meshBound = _sceneMeshBounds[1];
		}
		temp.pos = _instanceData[i].pos + posOffset;
		temp.scale = _instanceData[i].scale;
		temp.center_rad = glm::vec4(meshBound.origin, meshBound.radius);
		temp.modelMatrix = modelMat;
		meshDataList.push_back(temp);
	}

	pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, _deviceResources->gpuObjectSSBOBuffer, cmdBuffer, meshDataList.data(), 0,
		sizeof(GPUSSBOMeshData) * meshDataList.size(), _deviceResources->vmaAllocator);

	pvrvk::AccessFlags srcAccessMask = pvrvk::AccessFlags::e_TRANSFER_READ_BIT | pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT;
	pvrvk::AccessFlags dstAccessMask = pvrvk::AccessFlags::e_SHADER_READ_BIT;
	pvrvk::MemoryBarrierSet bufferMemoryBarrier;
	bufferMemoryBarrier.addBarrier(
		pvrvk::BufferMemoryBarrier(srcAccessMask, dstAccessMask, _deviceResources->gpuObjectSSBOBuffer, 0, static_cast<uint32_t>(_deviceResources->gpuObjectSSBOBuffer->getSize())));
	cmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_TRANSFER_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, bufferMemoryBarrier);
}

void VulkanGpuControlledRendering::updateDrawCullDataAndUBO(const uint32_t swapchainIndex)
{
	float fov = glm::radians(70.f);
	float nearClip = 0.01f;
	float farClip = 1000;

	glm::mat4 mProj = perspectiveProjectionInifiniteFarPlane(fov, float(static_cast<float>(this->getWidth()) / static_cast<float>(this->getHeight())), nearClip);

	UboPerMeshData srcWrite;
	srcWrite.proj = mProj;
	_deviceResources->uboStructuredBufferView.getElementByName("proj", 0, swapchainIndex).setValue(srcWrite.proj);

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->uboBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->uboBuffer->getDeviceMemory()->flushRange(
			_deviceResources->uboStructuredBufferView.getDynamicSliceOffset(swapchainIndex), _deviceResources->uboStructuredBufferView.getDynamicSliceSize());
	}

	glm::mat4 projectionT = glm::transpose(mProj);

	// update DrawCullData
	glm::vec4 frustums[6];

	// left plane
	frustums[0] = normalizePlane(projectionT[3] + projectionT[0]);

	// right plane
	frustums[1] = normalizePlane(projectionT[3] - projectionT[0]);

	// bottom plane
	frustums[2] = normalizePlane(projectionT[3] + projectionT[1]);

	// top plane
	frustums[3] = normalizePlane(projectionT[3] - projectionT[1]);

	// near plane
	frustums[4] = normalizePlane(projectionT[3] - projectionT[2]);

	// far plane
	frustums[5] = glm::vec4(0, 0, -1, farClip);
	uint32_t uboSize = static_cast<uint32_t>(sizeof(DrawCullData));

	DrawCullData cullData;
	cullData.cullingEnabled = uint32_t(_cullingEnabled);
	cullData.drawCount = static_cast<uint32_t>(TOTAL_NUM_INSTANCES);
	cullData.zNear = nearClip;
	cullData.frustrumPlanes[0] = frustums[0];
	cullData.frustrumPlanes[1] = frustums[1];
	cullData.frustrumPlanes[2] = frustums[2];
	cullData.frustrumPlanes[3] = frustums[3];
	cullData.frustrumPlanes[4] = frustums[4];
	cullData.frustrumPlanes[5] = frustums[5];

	pvrvk::CommandBuffer uploadCmd = beginCommandBuffer();
	pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, _deviceResources->drawCullDataUBOBuffer, uploadCmd, &cullData, 0, uboSize, _deviceResources->vmaAllocator);
	endAndSubmitCommandBuffer(uploadCmd);
}

void VulkanGpuControlledRendering::createCommonResources()
{
	//--- create the texture-sampler descriptor set layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
		descSetLayoutInfo.setBinding(0, pvrvk::DescriptorType::e_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); /*binding 0*/
		descSetLayoutInfo.setBinding(1, pvrvk::DescriptorType::e_SAMPLED_IMAGE, 2, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); /*binding 1*/
		_deviceResources->texLayout = _deviceResources->device->createDescriptorSetLayout(descSetLayoutInfo);
	}

	//--- create the ubo descriptor set layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
		descSetLayoutInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT); /*binding 0*/
		_deviceResources->uboLayoutDynamic = _deviceResources->device->createDescriptorSetLayout(descSetLayoutInfo);
	}

	//--- create the vertexSSBO descriptor set layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
		descSetLayoutInfo.setBinding(0, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT); /*binding 0*/
		descSetLayoutInfo.setBinding(1, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT); /*binding 1*/
		descSetLayoutInfo.setBinding(2, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); /*binding 2*/
		_deviceResources->vertexSSBOLayout = _deviceResources->device->createDescriptorSetLayout(descSetLayoutInfo);
	}

	// copy and gpu side buffers to be consumed by indirect cull compute stage and final indirect forward rendering pass
	pvr::utils::StructuredMemoryDescription ssboDesc;
	ssboDesc.addElement("modelMatrix", pvr::GpuDatatypes::mat4x4);
	ssboDesc.addElement("origin_rad", pvr::GpuDatatypes::vec4);
	ssboDesc.addElement("extents", pvr::GpuDatatypes::vec4);

	pvrvk::BufferCreateInfo bufferCreateInfo;
	pvrvk::DeviceSize bufferSize;
	pvrvk::BufferUsageFlags bufferUsageFlags;

	// gpuObjectSSBOBuffer creation
	bufferSize = static_cast<VkDeviceSize>(TOTAL_NUM_INSTANCES * sizeof(GPUSSBOMeshData));
	bufferUsageFlags = pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT;
	bufferCreateInfo.setSize(bufferSize);
	bufferCreateInfo.setUsageFlags(bufferUsageFlags);

	_deviceResources->gpuObjectSSBOBuffer = pvr::utils::createBuffer(
		_deviceResources->device, bufferCreateInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, _deviceResources->vmaAllocator);
	_deviceResources->gpuObjectSSBOBuffer->setObjectName("Object SSBO");

	// gpuIndirectCommandsBuffer creation
	bufferSize = static_cast<VkDeviceSize>(SCENE_NUM_MESHES * sizeof(GPUIndirectDrawCommandObject));
	bufferCreateInfo.setSize(bufferSize);
	bufferUsageFlags = pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT | pvrvk::BufferUsageFlags::e_INDIRECT_BUFFER_BIT;
	bufferCreateInfo.setUsageFlags(bufferUsageFlags);

	_deviceResources->gpuIndirectCommandsBuffer = pvr::utils::createBuffer(
		_deviceResources->device, bufferCreateInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, _deviceResources->vmaAllocator);
	_deviceResources->gpuIndirectCommandsBuffer->setObjectName("GPU Indirect Command Buffer");

	// gpuInstanceInputBuffer creation
	bufferSize = static_cast<VkDeviceSize>(TOTAL_NUM_INSTANCES * sizeof(GPUPerInstanceInput));
	bufferCreateInfo.setSize(bufferSize);
	bufferUsageFlags = pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT;
	bufferCreateInfo.setUsageFlags(bufferUsageFlags);

	_deviceResources->gpuInstanceInputBuffer = pvr::utils::createBuffer(
		_deviceResources->device, bufferCreateInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, _deviceResources->vmaAllocator);
	_deviceResources->gpuInstanceInputBuffer->setObjectName("GPU Instance Input Buffer");

	// gpuInstanceOutputBuffer & gpuInstanceOutputCopyBuffer creation
	bufferSize = static_cast<VkDeviceSize>(pvr::getSize(pvr::GpuDatatypes::uinteger)) * TOTAL_NUM_INSTANCES;
	bufferCreateInfo.setSize(bufferSize);
	bufferUsageFlags = pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_SRC_BIT;
	bufferCreateInfo.setUsageFlags(bufferUsageFlags);

	_deviceResources->gpuInstanceOutputBuffer = pvr::utils::createBuffer(
		_deviceResources->device, bufferCreateInfo, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, _deviceResources->vmaAllocator);
	_deviceResources->gpuInstanceOutputBuffer->setObjectName("GPU Instance Output Buffer");

	bufferUsageFlags = pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT;
	bufferCreateInfo.setUsageFlags(bufferUsageFlags);

	_deviceResources->gpuInstanceOutputCopyBuffer = pvr::utils::createBuffer(_deviceResources->device, bufferCreateInfo, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->gpuInstanceOutputCopyBuffer->setObjectName("GPU Instance Output Copy Buffer");

	// lightConstantUboBuffer creation
	bufferSize = static_cast<VkDeviceSize>(sizeof(LightConstants));
	bufferCreateInfo.setSize(bufferSize);
	bufferUsageFlags = pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT;
	bufferCreateInfo.setUsageFlags(bufferUsageFlags);

	_deviceResources->lightConstantUboBuffer = pvr::utils::createBuffer(_deviceResources->device, bufferCreateInfo, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT, _deviceResources->vmaAllocator,
		pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->lightConstantUboBuffer->setObjectName("Light Constant Buffer");

	// push constant data for once
	LightConstants data;
	data.lightCol = glm::vec4(lightCol, 1.0);
	data.lightDir = glm::vec4(lightDir, 1.0);
	pvr::utils::updateHostVisibleBuffer(_deviceResources->lightConstantUboBuffer, &data, 0, sizeof(LightConstants), true);
}

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanGpuControlledRendering::initApplication()
{
	// Load the scene
	_scene = pvr::assets::loadModel(*this, SceneFile);
	_angleYSphere = 0.0f;
	_frameId = 0;
	_queueIndex = 0;
	_instanceData.clear();
	_sceneMeshBounds.clear();
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
///	If the rendering context is lost, quitApplication() will not be called.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanGpuControlledRendering::quitApplication()
{
	_scene.reset();
	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context (e.g. scene passes, physical & logical devices, swapchains, textures, vertex buffers, etc.).</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanGpuControlledRendering::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();

	// Create Vulkan 1.2 instance and retrieve compatible physical devices
	auto version = pvr::utils::VulkanVersion(1u, 1u);
	_deviceResources->instance = pvr::utils::createInstance(this->getApplicationName(), version);

	if (_deviceResources->instance->getNumPhysicalDevices() == 0)
	{
		setExitMessage("Unable not find a compatible Vulkan physical device.");
		return pvr::Result::UnknownError;
	}

	// Create the surface
	pvrvk::Surface surface =
		pvr::utils::createSurface(_deviceResources->instance, _deviceResources->instance->getPhysicalDevice(0), this->getWindow(), this->getDisplay(), this->getConnection());

	// Create a default set of debug utils messengers or debug callbacks using either VK_EXT_debug_utils or VK_EXT_debug_report respectively
	_deviceResources->debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(_deviceResources->instance);

	// create device and queue(s)
	pvr::utils::QueuePopulateInfo queueCreateInfos[2] = {
		{ pvrvk::QueueFlags::e_GRAPHICS_BIT | pvrvk::QueueFlags::e_TRANSFER_BIT, surface },
		{ pvrvk::QueueFlags::e_COMPUTE_BIT },
	};
	pvr::utils::QueueAccessInfo queueAccessInfos[2];

	// check for multiDrawIndirect feature support
	if (_deviceResources->instance->getPhysicalDevice(0)->getFeatures().getMultiDrawIndirect() == 0)
	{
		throw pvrvk::ErrorInitializationFailed("No physical device with multiDrawIndirect feature support is found.");
		return pvr::Result::UnsupportedRequest;
	}

	if (_deviceResources->instance->getPhysicalDevice(0)->getFeatures().getDrawIndirectFirstInstance() == 0)
	{
		throw pvrvk::ErrorInitializationFailed("No physical device with DrawIndirectFirstInstance feature support is found.");
		return pvr::Result::UnsupportedRequest;
	}

	VkPhysicalDeviceShaderDrawParameterFeatures shaderDrawParametersFeature = { static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES) };

	VkPhysicalDeviceFeatures2 deviceFeatures2{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_FEATURES_2) };
	deviceFeatures2.pNext = &shaderDrawParametersFeature;

	// Fill in all of these device features with one call
	_deviceResources->instance->getPhysicalDevice(0)->getInstance()->getVkBindings().vkGetPhysicalDeviceFeatures2(
		_deviceResources->instance->getPhysicalDevice(0)->getVkHandle(), &deviceFeatures2);

	if (shaderDrawParametersFeature.shaderDrawParameters == VK_FALSE)
	{
		throw pvrvk::ErrorInitializationFailed("No physical device with shader draw parameters feature support is found.");
		return pvr::Result::UnsupportedRequest;
	}
	pvr::utils::DeviceExtensions deviceExtensions = pvr::utils::DeviceExtensions();

	// Add these device features to the physical device, since they're all connected by a pNext chain, we only need to explicitly attach the top feature
	deviceExtensions.addExtensionFeatureVk<VkPhysicalDeviceShaderDrawParameterFeatures>(&shaderDrawParametersFeature);

	_deviceResources->device =
		pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), queueCreateInfos, ARRAY_SIZE(queueCreateInfos), queueAccessInfos, deviceExtensions);
	// on PowerVR devices these queues will be the same but we'll design the application around the possibility that they aren't for compatibility with other platforms
	_deviceResources->graphicsQueue = _deviceResources->device->getQueue(queueAccessInfos[0].familyId, queueAccessInfos[0].queueId);
	_deviceResources->computeQueue = queueAccessInfos[0].familyId == queueAccessInfos[1].familyId
		? _deviceResources->graphicsQueue
		: _deviceResources->device->getQueue(queueAccessInfos[1].familyId, queueAccessInfos[1].queueId);

	_deviceResources->graphicsQueue->setObjectName("GraphicsQueue");
	_deviceResources->computeQueue->setObjectName("ComputeQueue");

	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT;
	} //---------------

	// Create the swapchain, on screen framebuffers and renderpass
	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage));

	_deviceResources->swapchain = swapChainCreateOutput.swapchain;

	// Store the swapchain length for repeated use
	_swapchainLength = _deviceResources->swapchain->getSwapchainLength();
	;

	_deviceResources->onScreenFramebuffer = swapChainCreateOutput.framebuffer;

	// resize resource vectors
	_deviceResources->imageAcquiredSemaphores.resize(_swapchainLength);
	_deviceResources->presentationSemaphores.resize(_swapchainLength);
	_deviceResources->computeSemaphores.resize(_swapchainLength);
	_deviceResources->perFrameResourcesFences.resize(_swapchainLength);
	_deviceResources->perFrameResourcesFencesCompute.resize(_swapchainLength);
	_deviceResources->mainCommandBuffers.resize(_swapchainLength);
	_deviceResources->computeCommandBuffers.resize(_swapchainLength);

	//---------------
	// Create the command pool and descriptor set pool
	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 64)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 16)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, 16)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_STORAGE_BUFFER_DYNAMIC, 128)
																						  .setMaxDescriptorSets(256));
	_deviceResources->descriptorPool->setObjectName("DescriptorPool");

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	//---------------
	// create command pools
	_deviceResources->commandPoolGraphics = _deviceResources->device->createCommandPool(
		pvrvk::CommandPoolCreateInfo(_deviceResources->graphicsQueue->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));
	_deviceResources->commandPoolCompute = _deviceResources->device->createCommandPool(
		pvrvk::CommandPoolCreateInfo(_deviceResources->computeQueue->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		// create the per swapchain command buffers
		_deviceResources->mainCommandBuffers[i] = _deviceResources->commandPoolGraphics->allocateCommandBuffer();
		_deviceResources->mainCommandBuffers[i]->setObjectName(std::string("Main CommandBuffer [") + std::to_string(i) + "]");

		_deviceResources->computeCommandBuffers[i] = _deviceResources->commandPoolCompute->allocateCommandBuffer();
		_deviceResources->computeCommandBuffers[i]->setObjectName(std::string("Compute CommandBuffer [") + std::to_string(i) + "]");

		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->computeSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->presentationSemaphores[i]->setObjectName("PresentationSemaphoreSwapchain" + std::to_string(i));
		_deviceResources->imageAcquiredSemaphores[i]->setObjectName("ImageAcquiredSemaphoreSwapchain" + std::to_string(i));
		_deviceResources->computeSemaphores[i]->setObjectName("ComputeSemaphoreSwapchain" + std::to_string(i));

		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->perFrameResourcesFencesCompute[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->perFrameResourcesFences[i]->setObjectName("FenceSwapchain" + std::to_string(i));
		_deviceResources->perFrameResourcesFences[i]->setObjectName("ComputeFenceSwapchain" + std::to_string(i));
	}

	// Create a single time submit command buffer for uploading resources
	pvrvk::CommandBuffer uploadBuffer = _deviceResources->commandPoolGraphics->allocateCommandBuffer();
	uploadBuffer->setObjectName("InitView : Upload Command Buffer");
	uploadBuffer->begin(pvrvk::CommandBufferUsageFlags::e_ONE_TIME_SUBMIT_BIT);

	// merge meshes into single IBO and VBOs
	mergeSceneIbosVbos(uploadBuffer);

	// create buffers and desc set layouts
	createCommonResources();

	// create the image samplers
	createImageSamplerDescriptor(uploadBuffer);
	uploadBuffer->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &uploadBuffer;
	submitInfo.numCommandBuffers = 1;
	_deviceResources->graphicsQueue->submit(&submitInfo, 1);
	_deviceResources->graphicsQueue->waitIdle();

	//  Initialize UIRenderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPoolGraphics, _deviceResources->graphicsQueue);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("GpuControlledRendering");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();

	_deviceResources->commandPoolGraphics->reset(pvrvk::CommandPoolResetFlags::e_RELEASE_RESOURCES_BIT);

	createUbo();

	createInstanceData();

	pvrvk::CommandBuffer uploadCmd = beginCommandBuffer();

	updateGPUIndirectObjectData(uploadCmd);

	endAndSubmitCommandBuffer(uploadCmd);

	createScenePasses();

	// generate model space sphere mesh bounds
	for (uint32_t i = 0; i < _scene->getNumMeshes(); i++)
	{
		pvr::assets::Model::Mesh mesh = _scene->getMesh(i);
		_sceneMeshBounds.push_back(calculateBoundingSphereMeshBounds(mesh));
	}

	_queueIndex = _queueIndex == 0 ? 1 : 0;

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanGpuControlledRendering::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

void VulkanGpuControlledRendering::logDebugData()
{
	// Iterate over the culling results for last frame and count the culled objects from the final visibility buffer from compute pass
	uint32_t gpuInstanceOutputBufferSize = static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::uinteger)) * TOTAL_NUM_INSTANCES;

	pvrvk::CommandBuffer uploadCmd = beginCommandBuffer();
	pvrvk::BufferCopy bufferCopy(0, 0, static_cast<VkDeviceSize>(gpuInstanceOutputBufferSize));
	uploadCmd->copyBuffer(_deviceResources->gpuInstanceOutputBuffer, _deviceResources->gpuInstanceOutputCopyBuffer, 1, &bufferCopy);
	endAndSubmitCommandBuffer(uploadCmd);

	uint32_t* finalInstanceBufferData = reinterpret_cast<uint32_t*>(_deviceResources->gpuInstanceOutputCopyBuffer->getDeviceMemory()->getMappedData());

	uint32_t numCulledInstancesSphere = 0;
	uint32_t numCulledInstancesTorus = 0;

	if (_cullingEnabled)
	{
		for (uint32_t i = 0; i < TOTAL_NUM_INSTANCES; ++i)
		{
			if (finalInstanceBufferData[i] == UINT32_MAX && i < NUM_INSTANCES_PER_DRAW) { ++numCulledInstancesSphere; }
			else if (finalInstanceBufferData[i] == UINT32_MAX)
			{
				++numCulledInstancesTorus;
			}
		}
	}
	std::string cullMode = {};
	cullMode = _cullingEnabled ? "ON" : "OFF";
	std::string uiDescription = "Total Instanced Objects : " + std::to_string(TOTAL_NUM_INSTANCES);
	std::string culledDescription = "\n Num Culled Instances: " + std::to_string(numCulledInstancesSphere + numCulledInstancesTorus);
	std::string numPrimitivesCulledDesc = "\n Primitives Culled :" +
		std::to_string(numCulledInstancesSphere * _scene->getMesh(0).getNumVertices() / 3 + numCulledInstancesTorus * _scene->getMesh(1).getNumVertices() / 3);
	std::string cullingMode = "\n Culling Mode :" + cullMode;

	uiDescription += culledDescription;
	uiDescription += numPrimitivesCulledDesc;
	uiDescription += cullingMode;

	_deviceResources->uiRenderer.getDefaultControls()->setText(uiDescription);
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();
}

void VulkanGpuControlledRendering::eventMappedInput(pvr::SimplifiedInput action)
{
	switch (action)
	{
	case pvr::SimplifiedInput::Action1: toggleCulling(); break;
	case pvr::SimplifiedInput::ActionClose: // quit the application
		this->exitShell();
		break;
	default: break;
	}
}

void VulkanGpuControlledRendering::toggleCulling() { _cullingEnabled = !_cullingEnabled; }

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanGpuControlledRendering::renderFrame()
{
	pvr::utils::beginQueueDebugLabel(_deviceResources->graphicsQueue, pvrvk::DebugUtilsLabel("renderFrame"));

	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();
	_deviceResources->perFrameResourcesFencesCompute[swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFencesCompute[swapchainIndex]->reset();

	// log culling results on screen after we are done waiting on last frame fence
	logDebugData();

	//----------------------------------------------
	// main compute command buffer record and submit
	auto computeCommandBuffer = _deviceResources->computeCommandBuffers[swapchainIndex];

	computeCommandBuffer->begin();

	prepareCullData(computeCommandBuffer);

	updateDrawCullDataAndUBO(swapchainIndex);

	_deviceResources->indirectCullComputePass->indirectCullDispatch(_deviceResources.get(), computeCommandBuffer);

	computeCommandBuffer->end();

	pvrvk::SubmitInfo submitInfoCompute;
	submitInfoCompute.commandBuffers = &computeCommandBuffer;
	submitInfoCompute.numCommandBuffers = 1;
	submitInfoCompute.signalSemaphores = &_deviceResources->computeSemaphores[_frameId];
	submitInfoCompute.numSignalSemaphores = 1;
	_deviceResources->computeQueue->submit(&submitInfoCompute, 1, _deviceResources->perFrameResourcesFencesCompute[swapchainIndex]);

	//--------------------------------------------------------------------------------------
	// main graphics command buffer record and submit : forward indirect draw + UI rendering
	_deviceResources->commandPoolGraphics->reset(pvrvk::CommandPoolResetFlags::e_RELEASE_RESOURCES_BIT);

	_deviceResources->perFrameResourcesFences[swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[swapchainIndex]->reset();

	auto mainCommandBuffer = _deviceResources->mainCommandBuffers[swapchainIndex];
	mainCommandBuffer->begin();
	_deviceResources->forwardIndirectPass->render(mainCommandBuffer, _deviceResources.get(), swapchainIndex, _queueIndex, *this);

	// blit to onScreen framebuffer
	_deviceResources->onScreenPass->render(
		mainCommandBuffer, _deviceResources.get(), swapchainIndex, *this, _deviceResources->forwardIndirectPass->_colorImageViews[_queueIndex], _queueIndex);

	mainCommandBuffer->end();

	pvrvk::Semaphore waitSemaphores[2] = { _deviceResources->imageAcquiredSemaphores[_frameId], _deviceResources->computeSemaphores[_frameId] };
	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags waitStages[2] = { pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT };
	submitInfo.commandBuffers = &_deviceResources->mainCommandBuffers[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = waitSemaphores;
	submitInfo.numWaitSemaphores = 2;
	submitInfo.signalSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.waitDstStageMask = waitStages;
	_deviceResources->graphicsQueue->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[swapchainIndex]);

	pvr::utils::endQueueDebugLabel(_deviceResources->graphicsQueue);

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->graphicsQueue, _deviceResources->commandPoolGraphics, _deviceResources->swapchain, swapchainIndex,
			this->getScreenshotFileName(), _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	//---------------
	// PRESENT
	pvr::utils::beginQueueDebugLabel(_deviceResources->graphicsQueue, pvrvk::DebugUtilsLabel("Presenting swapchain image to the screen"));

	pvrvk::PresentInfo presentInfo;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.imageIndices = &swapchainIndex;
	_deviceResources->graphicsQueue->present(presentInfo);

	pvr::utils::endQueueDebugLabel(_deviceResources->graphicsQueue);

	_frameId = (_frameId + 1) % _swapchainLength;
	_queueIndex = _queueIndex == 0 ? 1 : 0;

	return pvr::Result::Success;
}

void VulkanGpuControlledRendering::createScenePasses()
{
	_deviceResources->forwardIndirectPass = std::make_shared<ForwardIndirectPass>();
	_deviceResources->forwardIndirectPass->init(*this, _deviceResources.get(), _scene);

	_deviceResources->indirectCullComputePass = std::make_shared<IndirectCullComputePass>();
	_deviceResources->indirectCullComputePass->init(*this, _deviceResources.get());

	_deviceResources->onScreenPass = std::make_shared<OnScreenPass>();
	_deviceResources->onScreenPass->init(*this, _deviceResources.get());
}

pvrvk::CommandBuffer VulkanGpuControlledRendering::beginCommandBuffer()
{
	pvrvk::CommandBuffer uploadCmd = _deviceResources->commandPoolGraphics->allocateCommandBuffer();
	uploadCmd->begin();
	return uploadCmd;
}

void VulkanGpuControlledRendering::endAndSubmitCommandBuffer(pvrvk::CommandBuffer commandBuffer)
{
	commandBuffer->end();
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &commandBuffer;
	submitInfo.numCommandBuffers = 1;
	_deviceResources->graphicsQueue->submit(&submitInfo, 1);
	_deviceResources->graphicsQueue->waitIdle();
}

glm::vec4 VulkanGpuControlledRendering::normalizePlane(glm::vec4 p) { return p / glm::length(glm::vec3(p)); }

glm::mat4 VulkanGpuControlledRendering::perspectiveProjectionInifiniteFarPlane(float fovY, float aspectWbyH, float zNear)
{
	float f = 1.0f / tanf(fovY / 2.0f);
	return glm::mat4(f / aspectWbyH, 0.0f, 0.0f, 0.0f, 0.0f, f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, zNear, 0.0f);
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanGpuControlledRendering>(); }
